/* =============================================================================
 *
 * stm.h
 *
 * User program interface for STM. For an STM to interface with STAMP, it needs
 * to have its own stm.h for which it redefines the macros appropriately.
 *
 * =============================================================================
 *
 * Author: Chi Cao Minh
 *
 * =============================================================================
 */


#ifndef STM_H
#define STM_H 1


#include "tml.h"
#include "util.h"



#define STM_THREAD_T                    Thread
#define STM_SELF                        Self
#define STM_RO_FLAG                     ROFlag

#define STM_MALLOC(size)                TxAlloc(STM_SELF, size)
#define STM_FREE(ptr)                   TxFree(STM_SELF, ptr)


#  define malloc(size)                  tmalloc_reserve(size)

//#  define calloc(n, size)               /*({ \
                                            size_t numByte = (n) * (size); \
                                            void* ptr = tmalloc_reserve(numByte); \
                                            if (ptr) { \
                                                memset(ptr, 0, numByte); \
                                            } \
                                            ptr; \
                                        })*/
                                        
#  define realloc(ptr, size)           tmalloc_reserveAgain(ptr, size)
#  define free(ptr)                    tmalloc_release(ptr)


#  include <setjmp.h>
#  define STM_JMPBUF_T                  sigjmp_buf
#  define STM_JMPBUF                    buf

#define STM_VALID()                     (1)
#define STM_RESTART()                   TxAbort(STM_SELF)

#define STM_STARTUP()                   TxOnce()
#define STM_SHUTDOWN()                  TxShutdown()

#define STM_NEW_THREAD()                TxNewThread()
#define STM_INIT_THREAD(t, id)          TxInitThread(t, id)
#define STM_FREE_THREAD(t)              TxFreeThread(t)


#  define STM_BEGIN()                   TxStart(STM_SELF)
#  define STM_BEGIN_WR()                TxStart(STM_SELF)
#  define STM_BEGIN_RD()                TxStart(STM_SELF)

#define STM_END()                       TxCommit(STM_SELF)


typedef volatile intptr_t               vintp;



#define STM_READ(var)                   TxLoad(STM_SELF, (vintp*)(void*)&(var), 0)

#define STM_READ_F(var)                 IP2F(TxLoad(STM_SELF, \
                                                    (vintp*)FP2IPP(&(var)), 2))
#define STM_READ_P(var)                 IP2VP(TxLoad(STM_SELF, \
                                                     (vintp*)(void*)&(var), 1))

#define STM_WRITE(var, val)             TxStore(STM_SELF, \
                                                (vintp*)(void*)&(var), \
                                                (intptr_t)(val), 0)
#define STM_WRITE_F(var, val)           TxStore(STM_SELF, \
                                                (vintp*)FP2IPP(&(var)), \
                                                F2IP(val), 2)
#define STM_WRITE_P(var, val)           TxStore(STM_SELF, \
                                                (vintp*)(void*)&(var), \
                                                VP2IP(val), 1)



/*#define STM_READ(var)                   TxLoad(STM_SELF, (vintp*)(void*)&(var))

#define STM_READ_P(var)                 TxLoad_P(STM_SELF, (vintp*)(void*)&(var))

#define STM_READ_F(var)                 TxLoad_F(STM_SELF, (float*)(void*)&(var))*/

/*#define STM_READ_F(var)                 IP2F(TxLoad(STM_SELF, \
                                                    (vintp*)FP2IPP(&(var))))
#define STM_READ_P(var)                 IP2VP(TxLoad(STM_SELF, \
                                                     (vintp*)(void*)&(var)))*/


/*#define STM_READ_F(var)                 TxLoad(STM_SELF, (vintp*)(void*)&(var))
#define STM_READ_P(var)                 TxLoad(STM_SELF, (vintp*)(void*)&(var))*/

/*#define STM_WRITE(var, val)             TxStore(STM_SELF, \
                                                (vintp*)(void*)&(var), \
                                                (intptr_t)(val))
#define STM_WRITE_F(var, val)             TxStore_F(STM_SELF, \
                                                (vintp*)(void*)&(var), \
                                                (intptr_t)(val))
#define STM_WRITE_P(var, val)             TxStore_P(STM_SELF, \
                                                (vintp*)(void*)&(var), \
                                                (intptr_t)(val))*/


#define STM_LOCAL_WRITE(var, val)       ({var = val; var;})
#define STM_LOCAL_WRITE_D(var, val)     ({var = val; var;})
#define STM_LOCAL_WRITE_P(var, val)     ({var = val; var;})

#endif /* STM_H */