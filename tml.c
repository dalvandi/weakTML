#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include "tml.h"
#include "tmalloc.h"
#include "platform.h"


#define OK 1
#define ABORT 0
#define COMMIT 2
#define NOTSTARTED 3

#define FALSE 0
#define TRUE 1

#define EVEN(VAL) (VAL % 2) == 0

#define CAS_RA(VAR, EV, NV) atomic_compare_exchange_weak_explicit(&VAR, &EV, NV, memory_order_acq_rel, memory_order_relaxed)
//#define CAS(m,c,s)  cas((intptr_t)(s),(intptr_t)(c),(intptr_t*)(m))
char ty[3] = {'N', 'P', 'F'};

struct _Thread
{
    long UniqID;
    pthread_t tid;
    int loc;
    intptr_t val;
    int hasRead;
    int status;
    long Starts;
    long Aborts; /* Tally of # of aborts */
    volatile long Retries;
    tmalloc_t* allocPtr; /* CCM: speculatively allocated */
    tmalloc_t* freePtr;  /* CCM: speculatively free'd */
};


typedef  struct _Thread Thread;

static pthread_key_t    global_key_self;

volatile long StartTally         = 0;
volatile long AbortTally         = 0;
volatile long ReadOverflowTally  = 0;
volatile long WriteOverflowTally = 0;
volatile long LocalOverflowTally = 0;

atomic_int glb = 0;

intptr_t
AtomicAdd (volatile intptr_t* addr, intptr_t dx)
{
        //printf("AtomicAdd \n");
   intptr_t v;
    for (v = *addr; CAS(addr, v, v+dx) != v; v = *addr) {}
    return (v+dx);
}


void
TxOnce ()
{
       //printf("TxOnce \n");

    pthread_key_create(&global_key_self, NULL); /* CCM: do before we register handler */

}


void
TxShutdown ()
{
    printf("TML system shutdown.\n Starts: %li  Aborts: %li \n", StartTally, AbortTally);

    pthread_key_delete(global_key_self);
}



Thread*
TxNewThread ()
{
       //printf("TxNewThread \n");
   Thread* t = (Thread*)malloc(sizeof(Thread));
    assert(t);

    return t;
}




void
TxFreeThread (Thread* t)
{
      //printf("TxFreeThread \n");
   AtomicAdd((volatile intptr_t*)((void*)(&ReadOverflowTally)), 0);

    long wrSetOvf = 0;

    AtomicAdd((volatile intptr_t*)((void*)(&WriteOverflowTally)), wrSetOvf);

    AtomicAdd((volatile intptr_t*)((void*)(&StartTally)),         t->Starts);
    AtomicAdd((volatile intptr_t*)((void*)(&AbortTally)),         t->Aborts);


    free(t);
}


void
TxInitThread (Thread* t, long id)
{
     //printf("TxInitThread (id = %ld) \n", id);
   /* CCM: so we can access NOREC's thread metadata in signal handlers */
    pthread_setspecific(global_key_self, (void*)t);

    memset(t, 0, sizeof(*t));     /* Default value for most members */

    t->UniqID = id;
    t->allocPtr = tmalloc_alloc(1);
    assert(t->allocPtr);
    t->freePtr = tmalloc_alloc(1);
    assert(t->freePtr);

    //sleep(300);
}


void
txCommitReset (Thread* Self)
{
      //printf("txCommitReset \n");
    //txReset(Self);
    Self->Retries = 0;
}


void
TxAbort (Thread* Self)
{
      printf("TxAbort \n");
    Self->Retries++;
    Self->Aborts++;
}

void*
TxAlloc (Thread* Self, size_t size)
{
      //printf("TxAlloc \n");
    void* ptr = tmalloc_reserve(size);
    if (ptr) {
        tmalloc_append(Self->allocPtr, ptr);
    }

    return ptr;
}


void
TxFree (Thread* Self, void* ptr)
{
    tmalloc_append(Self->freePtr, ptr);
}


int
TxStart(Thread* Self)
{
    Self->hasRead = FALSE;
    Self->loc = 0;
    Self->val = 0;
    Self->status = NOTSTARTED;

    do
    {
        Self->loc = atomic_load_explicit(&glb, memory_order_acquire);
    } while (!EVEN(Self->loc));

    Self->Starts++;

    Self->status = OK;
       printf("\nTMBegin (loc=%d, status=%d) \n",  Self->loc, Self->status);
    return OK; 

}

void
TxStore(Thread* Self, volatile intptr_t* addr, intptr_t v, int tt)
{
    if(Self->status != OK) {
         TxAbort(Self);
        }

    if(EVEN(Self->loc))
    {
        if(!CAS_RA(glb, Self->loc, Self->loc+1))
        {
            printf("TxStore_%c (addr = %ld, valu = %ld\n", ty[tt], &addr, v);
            Self->status = ABORT;
                TxAbort(Self);
        }
        else
            Self->loc++;
    }

    *addr = v;
    //atomic_store_explicit(addr, v, memory_order_release);
    printf("TxStore_%c (addr = %ld, valu = %ld\n", ty[tt], &addr, v);

}



intptr_t
TxLoad (Thread* Self, volatile intptr_t* addr, int tt)
{
    if(Self->status != OK) return 0;

    Self->val = (*(addr));

    int temp = atomic_load_explicit(&glb, memory_order_relaxed);
    if(!Self->hasRead && EVEN(Self->loc))
    {
        if(CAS_RA(glb, Self->loc, Self->loc))
        {
            Self->hasRead = TRUE;
            printf("TxLoad_%c (val = %ld)  \n", ty[tt], Self->val);
            return Self->val;
        }
    }
    else if(Self->loc == temp)
    {
            printf("TxLoad_%c (val = %ld)  \n", ty[tt], Self->val);
        return Self->val;
    }

    printf("TMRead(tid=%ld) returns ABORT\n", Self->UniqID);
    Self->status = ABORT;
    TxAbort(Self);
  return ABORT;
}


int
TxCommit(Thread* Self)
{
   if(Self->status != OK) return 0;

   if (!EVEN(Self->loc))
        atomic_store_explicit(&glb, Self->loc+1, memory_order_release);

    int tmp = atomic_load_explicit(&glb, memory_order_relaxed);

    printf("TxCommit\n");

   return COMMIT;
}




/*****************************/
void
TxStore_P(Thread* Self, volatile intptr_t* addr, intptr_t v)
{
    if(Self->status != OK) {
         TxAbort(Self);
        }

    if(EVEN(Self->loc))
    {
        if(!CAS_RA(glb, Self->loc, Self->loc+1))
        {
            printf("TxStore_P (ABORTED)");
            Self->status = ABORT;
            TxAbort(Self);
        }
        else
            Self->loc++;
    }

    addr = v;
    //atomic_store_explicit(addr, v, memory_order_release);
    printf("TxStore_P (addr = %ld, valu = %ld)\n", &addr, v);

}



void
TxStore_F(Thread* Self, volatile intptr_t* addr, intptr_t v)
{
    if(Self->status != OK) {
         TxAbort(Self);
        }

    if(EVEN(Self->loc))
    {
        if(!CAS_RA(glb, Self->loc, Self->loc+1))
        {
            printf("TxStore_F (Aborted)\n");
            Self->status = ABORT;
                TxAbort(Self);
        }
        else
            Self->loc++;
    }

    addr = v;
    printf("TxStore_F (addr = %ld, valu = %f)\n", &addr, v);

}


float
TxLoad_F (Thread* Self, float* addr)
{
    if(Self->status != OK) return 0;


    //printf("*** %f", *addr);

    Self->val = *addr;

    int temp = atomic_load_explicit(&glb, memory_order_relaxed);
    if(!Self->hasRead && EVEN(Self->loc))
    {
        if(CAS_RA(glb, Self->loc, Self->loc))
        {
            Self->hasRead = TRUE;
            printf("TxLoad_F (val = %f)  \n", Self->val);
            return Self->val;
        }
    }
    else if(Self->loc == temp)
    {
            printf("TxLoad_F (val = %f)  \n", Self->val);
        return Self->val;
    }

    printf("TMRead(tid=%ld) returns ABORT\n", Self->UniqID);
    Self->status = ABORT;
    TxAbort(Self);
  return ABORT;
}


intptr_t
TxLoad_P (Thread* Self, volatile intptr_t* addr)
{
    if(Self->status != OK) { 
        TxAbort(Self);
        return 0;
        }

    Self->val = *addr;

    int temp = atomic_load_explicit(&glb, memory_order_relaxed);
    if(!Self->hasRead && EVEN(Self->loc))
    {
        if(CAS_RA(glb, Self->loc, Self->loc))
        {
            Self->hasRead = TRUE;
            printf("TxLoad_P (val = %ld)  \n", Self->val);
            return Self->val;
        }
    }
    else if(Self->loc == temp)
    {
            printf("TxLoad_P (val = %ld)  \n", Self->val);
        return Self->val;
    }

    printf("TMRead(tid=%ld) returns ABORT\n", Self->UniqID);
    Self->status = ABORT;
    TxAbort(Self);
  return ABORT;
}
