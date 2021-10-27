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

void     TxOnce          ();
void     TxShutdown      ();
Thread*  TxNewThread     ();
void     TxFreeThread    (Thread*);
void     TxInitThread    (Thread*, long id);
void     txCommitReset   (Thread*);
void     TxAbort         (Thread*);

int      TxStart         (Thread*);
void     TxStore         (Thread*, volatile atomic_intptr_t*, atomic_intptr_t, int);
intptr_t TxLoad          (Thread*, volatile atomic_intptr_t*, int);
int      TxCommit        (Thread*);

void*    TxAlloc         (Thread*, size_t);
void     TxFree          (Thread*, void*);




#ifdef __cplusplus
}
#endif



#endif