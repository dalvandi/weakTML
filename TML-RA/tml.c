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
#include <time.h>
#include <errno.h>

#define OK 1
#define ABORT 0
#define COMMIT 2
#define NOTSTARTED 3

#define MS 0.005
#define IF 30

#define nop() __asm__ volatile("nop")

//#define DEBUG

#define FALSE 0
#define TRUE 1

#define EVEN(VAL) (VAL % 2) == 0

#define CAS_RA(VAR, EV, NV) atomic_compare_exchange_weak_explicit(&VAR, &EV, NV, memory_order_acq_rel, memory_order_relaxed)
//#define CAS(m,c,s)  cas((intptr_t)(s),(intptr_t)(c),(intptr_t*)(m))
char ty[3] = {'N', 'P', 'F'};

struct _Thread
{
    long UniqID;
    long transId;
    pthread_t tid;
    int loc;
    intptr_t val;
    int hasRead;
    int status;
    long Starts;
    long Aborts; /* Tally of # of aborts */
    volatile long Retries;
    tmalloc_t *allocPtr; /* CCM: speculatively allocated */
    tmalloc_t *freePtr;  /* CCM: speculatively free'd */
    sigjmp_buf *envPtr;
};

typedef struct _Thread Thread;

static pthread_key_t global_key_self;

volatile long StartTally = 0;
volatile long AbortTally = 0;
volatile long ReadOverflowTally = 0;
volatile long WriteOverflowTally = 0;
volatile long LocalOverflowTally = 0;

atomic_int glb = 0;

intptr_t nglb = 0;


atomic_int nma = 0;


inline void spin64()
{
    for (int i = 0; i < 64; i++)
        nop();
}

inline void sleepy(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        //return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

intptr_t
AtomicAdd(volatile intptr_t *addr, intptr_t dx)
{
    //printf("AtomicAdd \n");
    intptr_t v;
    for (v = *addr; CAS(addr, v, v + dx) != v; v = *addr)
    {
    }
    return (v + dx);
}

void TxOnce()
{
    //printf("TxOnce \n");
    pthread_key_create(&global_key_self, NULL); /* CCM: do before we register handler */
}

void TxShutdown()
{
    printf("TML-RA system shutdown.\n Starts: %li  Aborts: %li \n NoA: %d \n", StartTally, AbortTally, nma);

    pthread_key_delete(global_key_self);
}

Thread *
TxNewThread()
{
    //printf("TxNewThread \n");
    Thread *t = (Thread *)malloc(sizeof(Thread));
    assert(t);

    return t;
}

void TxFreeThread(Thread *t)
{
    //printf("TxFreeThread \n");
    AtomicAdd((volatile intptr_t *)((void *)(&ReadOverflowTally)), 0);

    long wrSetOvf = 0;

    AtomicAdd((volatile intptr_t *)((void *)(&WriteOverflowTally)), wrSetOvf);

    AtomicAdd((volatile intptr_t *)((void *)(&StartTally)), t->Starts);
    AtomicAdd((volatile intptr_t *)((void *)(&AbortTally)), t->Aborts);

    free(t);
}

void TxInitThread(Thread *t, long id)
{
    //printf("TxInitThread (id = %ld) \n", id);
    /* CCM: so we can access NOREC's thread metadata in signal handlers */
    pthread_setspecific(global_key_self, (void *)t);

    memset(t, 0, sizeof(*t)); /* Default value for most members */

    t->UniqID = id;
    t->allocPtr = tmalloc_alloc(1);
    assert(t->allocPtr);
    t->freePtr = tmalloc_alloc(1);
    assert(t->freePtr);

    //sleep(300);
}

void txCommitReset(Thread *Self)
{
    //printf("txCommitReset \n");
    //txReset(Self);
    Self->Retries = 0;
}

void TxAbort(Thread *Self)
{
#ifdef DEBUG
    printf("TxAbort (tid = %ld, transId=%ld) \n", Self->UniqID, Self->transId);
#endif

    Self->Retries++;
    Self->Aborts++;
    //Self->status = ABORT;
    SIGLONGJMP(*Self->envPtr, 1);
}

void *
TxAlloc(Thread *Self, size_t size)
{
    //printf("TxAlloc \n");
    void *ptr = tmalloc_reserve(size);
    if (ptr)
    {
        tmalloc_append(Self->allocPtr, ptr);
    }

    return ptr;
}

void TxFree(Thread *Self, void *ptr)
{
    tmalloc_append(Self->freePtr, ptr);
}

void TxStart(Thread *Self, sigjmp_buf *envPtr, int *ROFlag)
{
    Self->hasRead = FALSE;
    Self->loc = 0;
    Self->val = 0;
    Self->status = NOTSTARTED;

    long ms = MS;
    long sleepc = 1;

    Self->loc = atomic_load_explicit(&glb, memory_order_acquire);
    ++nma;

    while (!EVEN(Self->loc))
    {
        sleepy(sleepc * ms);

        sleepc *= IF;
        Self->loc = atomic_load_explicit(&glb, memory_order_acquire);
       ++nma;
    }

    Self->Starts++;
    Self->status = OK;
    Self->envPtr = envPtr;

#ifdef DEBUG
    printf("\nTMBegin (loc=%d, status=%d, transId=%ld) \n", Self->loc, Self->status, Self->transId);
#endif
}

void TxStore(Thread *Self, volatile atomic_intptr_t *addr, atomic_intptr_t v, int tt)
{
    if (EVEN(Self->loc))
    {
        ++nma;
        if (!CAS_RA(glb, Self->loc, Self->loc + 1))
        {
#ifdef DEBUG
            printf("TxStore_%c *ABORTED* (addr = %ld, valu = %ld, tid = %ld, transId=%ld)\n", ty[tt], &addr, v, Self->UniqID, Self->transId);
#endif

            Self->status = ABORT;
            TxAbort(Self);
            return;
        }
        else
            Self->loc++;
    }

    //*addr = v;
    ++nma;
    atomic_store_explicit(addr, v, memory_order_release);
#ifdef DEBUG
    printf("TxStore_%c (addr = %ld, valu = %ld, tid = %ld, transId=%ld)\n", ty[tt], &addr, v, Self->UniqID, Self->transId);
#endif
    return;
}

intptr_t
TxLoad(Thread *Self, volatile atomic_intptr_t *addr, int tt)
{
    Self->val = atomic_load_explicit(addr, memory_order_relaxed); //(*(addr));
    ++nma;

    if (!Self->hasRead && EVEN(Self->loc))
    {
        if (CAS_RA(glb, Self->loc, Self->loc))
        {
            ++nma;
            Self->hasRead = TRUE;

#ifdef DEBUG
            printf("TxLoad_%c (val = %ld, tid = %ld, transId=%ld)  \n", ty[tt], Self->val, Self->UniqID, Self->transId);
#endif

            return Self->val;
        }
    }
    else if (Self->loc == atomic_load_explicit(&glb, memory_order_relaxed))
    {
       ++nma;

#ifdef DEBUG
        printf("TxLoad_%c (val = %ld, tid = %ld, transId=%ld)  \n", ty[tt], Self->val, Self->UniqID, Self->transId);
#endif

        return Self->val;
    }

#ifdef DEBUG
    printf("TMRead(tid=%ld, transId=%ld) returns ABORTED\n", Self->UniqID, Self->transId);
#endif

    Self->status = ABORT;
    TxAbort(Self);
    return ABORT;
}

int TxCommit(Thread *Self)
{
    if (!EVEN(Self->loc)){
        atomic_store_explicit(&glb, Self->loc + 1, memory_order_release);
       ++nma;
    }
#ifdef DEBUG
    printf("TxCommit(tid = %ld, transId=%ld)\n", Self->UniqID, Self->transId);
#endif

    return 1;
}
