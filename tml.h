#ifndef TML_H
#define TML_H 1


#include <stdint.h>
#include <stdatomic.h>
#include "tmalloc.h"

#include <setjmp.h>

typedef struct _Thread Thread;


#ifdef __cplusplus
extern "C" {
#endif

void TxOnce ();
void TxShutdown ();
Thread* TxNewThread ();
void TxFreeThread (Thread*);
void TxInitThread (Thread*, long id);
void txCommitReset (Thread*);
void TxAbort (Thread*);

int     TxStart       (Thread*);
void     TxStore       (Thread*, volatile intptr_t*, intptr_t, int);
void     TxStore_F       (Thread*, volatile intptr_t*, intptr_t);
void     TxStore_P       (Thread*, volatile intptr_t*, intptr_t);
//int     TxStore       (Thread*,  atomic_int*, intptr_t);
intptr_t TxLoad       (Thread*, volatile intptr_t*, int);
intptr_t TxLoad_P       (Thread*, volatile intptr_t*);
float TxLoad_F       (Thread*, float*);
int     TxCommit      (Thread*);

void*    TxAlloc       (Thread*, size_t);
void     TxFree        (Thread*, void*);




#ifdef __cplusplus
}
#endif



#endif