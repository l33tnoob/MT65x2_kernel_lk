
#ifndef __MTEE_SYSTEM_IPC_H__
#define __MTEE_SYSTEM_IPC_H__

#include "tz_cross/trustzone.h"
#include "tz_private/system.h"
#include <stddef.h>

//#define MTEE_LOCK_DEBUG

/**
 * Spinlock define
 *
 * @param lock    locker.
 */
typedef struct {
    volatile uint32_t lock;
#ifdef MTEE_LOCK_DEBUG
    void *owner_stack, *owner_pc;
#endif
} MTEE_SPINLOCK;

/**
 * Mutex define
 *
 * @param mutex    mutex id.
 */
typedef uint32_t MTEE_MUTEX;

/**
 * Semaphore define
 *
 * @param semaphore    semaphore id.
 */
typedef uint32_t MTEE_SEMAPHORE;

/**
 * Wait queue define
 *
 * @param waitqueue    waitqueue id.
 */
typedef uint32_t MTEE_WAITQUEUE;

/**
 * TEE built-in IPC
 *
 * These IPC functions are implemented in TEE.
 */

/* Spinlock
*/

/**
 * Spinlock initializer, used in lock declare.
 *
 * Usage:
 *    MTEE_SPINLOCK mylock = MTEE_SPINLOCK_INIT_UNLOCK;
 */
#ifndef MTEE_LOCK_DEBUG
#define MTEE_SPINLOCK_INIT_UNLOCK    {0}
#else
#define MTEE_SPINLOCK_INIT_UNLOCK    {0,0,0}
#endif

/**
 * Init spinlock, used in code
 *
 * @param lock The lock to init
 * @param init_lock Init lock state.
 */
static inline void MTEE_InitSpinlock(MTEE_SPINLOCK *lock, int init_lock)
{
    lock->lock = (init_lock) ? 1 : 0;
#ifdef MTEE_LOCK_DEBUG
    lock->owner_stack = lock->owner_pc = 0;
#endif
}

/**
 * Check if the spin lock is locked.
 *
 * @param lock The lock to check
 */
static inline int MTEE_SpinIsLocked(MTEE_SPINLOCK *lock)
{
    return lock->lock != 0;
}

/* 
    __MTEE_SpinLock()/__MTEE_SpinUnlock() are internally used
    TA should not use those function except you know there is no priority inversion cases
    Example:
      A kernel thread with low priority get the spinlock.
      B/C/D kernel threads with real time priority cannot get the spin lock but occupy the CPU forever
*/
/**
 * Grab the spin lock
 *
 * @param lock    The spin lock to take.
 */
static inline void __MTEE_SpinLock(MTEE_SPINLOCK *lock)
{
    unsigned int tmp;

    __asm__ volatile(
        "1:    ldrex    %0, [%1]\n"
        "      teq      %0, #0\n"
        "      wfene    \n"
        "      strexeq  %0, %2, [%1]\n"
        "      teqeq    %0, #0\n"
        "      bne      1b\n"
        "      dmb      \n"
        : "=&r" (tmp)
        : "r" (&lock->lock), "r" (1)
        : "cc", "memory");
#ifdef MTEE_LOCK_DEBUG
{
    register unsigned long sp asm ("sp");
    lock->owner_stack = (void*)sp;
here:
    lock->owner_pc = (void*)&&here;
}
#endif
}

/**
 * Unlock spin lock
 *
 * @param lock    The spin lock to unlock
 */
static inline void __MTEE_SpinUnlock(MTEE_SPINLOCK *lock)
{
#ifdef MTEE_LOCK_DEBUG
    lock->owner_stack = lock->owner_pc = 0;
#endif
    __asm__ volatile(
        "      dmb      \n"
        "      str      %1, [%0]\n"
        "      dsb      \n"
        "      sev      \n"
        :
        : "r" (&lock->lock), "r" (0)
        : "memory");
}

/**
 * Grab the spin lock and mask IRQ/FIQ
 *
 * @param lock    The spin lock to take.
 * @param state    Previous IRQ/FIQ state to restore to.
 */
static inline uint32_t MTEE_SpinLockMaskIrq(MTEE_SPINLOCK *lock)
{
    uint32_t state = MTEE_MaskIrq();
    __MTEE_SpinLock(lock);
    return state;
}

/**
 * Unlock spin lock and restore IRQ to previous state.
 *
 * @param lock    The spin lock to unlock
 * @param state    Previous IRQ state to restore to.
 */
static inline void MTEE_SpinUnlockRestoreIrq(MTEE_SPINLOCK *lock, uint32_t state)
{
    __MTEE_SpinUnlock(lock);
    MTEE_RestoreIrq(state);
}

/**
 * IPC through REE service
 *
 * These IPC functions are implemented through REE service.
 * By REE service, use normal world OS's IPC functions.
 *
 * NOTE: It is simple, but not as secure as built-in solution.
 * It may be used for attacking. The major weapon will be race condition.
 */
 
/* Mutex
*/
/**
 * Mutex, create
 *
 * @return    A pointer to the the mutex. If fail, return 0.
 */ 
MTEE_MUTEX *MTEE_CreateMutex (void);

/**
 * Mutex, destory
 *
 * @param mutex    A pointer to the mutex.
 * @return    0 if success. <0 for error.
 */ 
int MTEE_DestoryMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, lock
 *
 * @param mutex    A pointer to the mutex.
 */ 
void MTEE_LockMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, unlock
 *
 * @param mutex    A pointer to the mutex.
 */     
void MTEE_UnlockMutex (MTEE_MUTEX *mutex);

/**
 * Mutex, try lock
 *
 * @param mutex    A pointer to the mutex.
 * @return    1 if the mutex has been acquired successfully, and 0 on contention.
 */ 

int MTEE_TryLockMutex (MTEE_MUTEX *mutex); 

/**
 * Mutex, querey if it is locked or not
 *
 * @param mutex    A pointer to the mutex.
 * @return    1 if the mutex is locked, 0 if unlocked..
 */     
int MTEE_IsLockedMutex (MTEE_MUTEX *mutex);     

/* Semaphore
*/
/**
 * Semaphore, create
 *
 * @param val      initial value. 
 * @return    A pointer to the the semaphore. If fail, return 0.
 */    
MTEE_SEMAPHORE *MTEE_CreateSemaphore (int val);

/**
 * Semaphore, free
 *
 * @param semaphore    A pointer to the semaphore.
 * @return    0 if success. <0 for error.
 */    
int MTEE_DestroySemaphore (MTEE_SEMAPHORE *semaphore);     
        
/**
 * Semaphore, down
 *
 * @param semaphore    A pointer to the semaphore.
 */   
void MTEE_DownSemaphore (MTEE_SEMAPHORE *semaphore);    

/**
 * Semaphore, down with time-out
 *
 * @param semaphore    A pointer to the semaphore.
 * @param timeout    timeout value, in jiffies   
 * @return    0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 */   
int MTEE_DownSemaphore_Timeout (MTEE_SEMAPHORE *semaphore, int timeout);

/**
 * Semaphore, down try lock
 *
 * @param semaphore    A pointer to the semaphore.
 * @return    0 if semaphore is avalibale.
 */      
int MTEE_DownTryLockSemaphore (MTEE_SEMAPHORE *semaphore);     

/**
 * Semaphore, up
 *
 * @param semaphore    A pointer to the semaphore.
 */  
void MTEE_UpSemaphore (MTEE_SEMAPHORE *semaphore); 

#if 0  
/* Wait queue
*/    
/**
 * Wait queue, create
 *
 * @return    A pointer to the waitqueue. If fail, return 0.
 */ 
MTEE_WAITQUEUE *MTEE_CreateWaitQueue (void); 

/**
 * Wait queue, free
 *
 * @param waitqueue    A pointer to the waitqueue.
 * @return    0 if success. <0 for error. 
 */ 
int MTEE_DestroyWaitQueue (MTEE_WAITQUEUE *waitqueue); 

/**
 * Wait queue, wait event
 *
 * @param waitqueue    A pointer to the waitqueue.
 * @condition    The event to wait for
 */ 
void MTEE_WaitEvent (MTEE_WAITQUEUE *waitqueue, int condition); 
    
/**
 * Wait queue, wait event with timeout
 *
 * @param waitqueue    A pointer to the waitqueue.
 * @condition    The event to wait for
 * @timeout    timeout value, in jiffies  
 * @return     0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 */ 
int MTEE_WaitEvent_Timeout (MTEE_WAITQUEUE *waitqueue, int condition, int timeout);   

/**
 * Wait queue, wake up
 *
 * @param waitqueue    A pointer to the waitqueue.
 */ 
void MTEE_WakeUpWaitQueue (MTEE_WAITQUEUE *waitqueue);
#endif

#endif /* __MTEE_SYSTEM_IPC_H__ */
