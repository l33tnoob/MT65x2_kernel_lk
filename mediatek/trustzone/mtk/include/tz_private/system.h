
#ifndef __MTEE_SYSTEM_H__
#define __MTEE_SYSTEM_H__

#include "tz_cross/trustzone.h"

#ifndef __TRUSTZONE_TEE__
#error This file(tz_private/system.h) is for TEE use only.
#endif /* __TRUSTZONE_TEE__ */

/**
 * Paramter define
 *
 * @param mem    Parameter for memory. Parameter types are TZPT_MEM_XXX.
 * @param value    Parameter for value. Parameter types are TZPT_VALUE_XXX.
 */
typedef union {
    struct {
        void *buffer;
        uint32_t size;
    } mem;
    
    struct {
        uint32_t a, b;
    } value;
} MTEE_PARAM;

/// TEE session handle type.
typedef uint32_t MTEE_SESSION_HANDLE;

/* Session management */
/**
 * Create a new TEE sesssion 
 * This will be called in service call when REE asked to create a new session.
 *
 * @param ta_uuid UUID of the TA to connect to.
 * @param pHandle Handle for the new session. Return MTEE_SESSION_HANDLE_FAIL if fail.
 * @return return code
 */
TZ_RESULT MTEE_CreateSession(const char *ta_uuid, MTEE_SESSION_HANDLE *pHandle);

/**
 * Close TEE session 
 *
 * @param handle Handle for session to close.
 * @return 0 if success. <0 for error.
 */
TZ_RESULT MTEE_CloseSession(MTEE_SESSION_HANDLE handle);

/**
 * Set session user data.
 * 
 * @prarm handle Handle for the session
 * @param pUserData New user data.
 * @return return code
 */
TZ_RESULT MTEE_SetSessionUserData(MTEE_SESSION_HANDLE handle, void *pUserData);

/**
 * Get session user data.
 *
 * @prarm handle Handle for the session
 * @param ppUserData Return user data set to the session.
 * @return return code
 */
TZ_RESULT MTEE_GetSessionUserData(MTEE_SESSION_HANDLE handle, void **ppUserData);


/* TA Register & Service Call */
struct MTEE_TA_Function;

/**
 * Function prototype for TA init.
 * Function to be called at TA register time.
 *
 * @param ta_func Point to MTEE_TA_Function data structure.
 * @return 0 if init OK. <0 if TA init failed and is not usable.
 */
typedef TZ_RESULT (*MTEE_TaInit)(const struct MTEE_TA_Function *ta_func);

/**
 * Function prototype for TA service call function.
 */
typedef TZ_RESULT (*MTEE_ServiceCallFunction)(MTEE_SESSION_HANDLE handle,
                    uint32_t cmd, uint32_t paramTypes, MTEE_PARAM param[4]);

/**
 * Session create callback function prototype
 * When user create a new session to the TA, tee system will create a new 
 * handle and call this callback with the newly created handle.
 *
 * If this callback return error, the session created fail. Tee system will
 * still free all registered resource, but will not call session close 
 * callback. After resource freed, the return error code of this callback 
 * will be return to REE as the return code for session create.
 *
 * @param handle The session about to be created
 * @param ta_func TA function table.
 * @return 0 if success. <0 if not success. If not success, the session
 *         will not be created and the handle will be freed.
 */
typedef TZ_RESULT (*MTEE_SessionCreateCallback)(MTEE_SESSION_HANDLE handle,
                                                const struct MTEE_TA_Function *ta_func);

/**
 * Session close callback function prototype
 * This function will be called when REE request to close a session. This
 * callback function will be called before all registered resource is freed.
 *
 * @param handle The session about to be closed
 * @param ta_func TA function table.
 * @return 0 if success. <0 if not success. If not success, the session
 *         will not be created and the handle will be freed.
 */
typedef void (*MTEE_SessionCloseCallback)(MTEE_SESSION_HANDLE handle,
                                          const struct MTEE_TA_Function *ta_func);

/**
 * TA function table used to register to TEE system.
 *
 * @param TAName Name of this TA
 * @param UUID UUID of this TA
 * @param init_func Called at TA register time. This will be called when TEE
 *                  system initailze. NULL if TA doesn't need init
 * @param create_func Called when create a new session to this TA.
 *                    NULL if doesn't need create callback
 * @param close_func Called when a session connected to this TA is closed.
 *                   NULL if doesn't need close callback
 * @param service_func TA command service call function. Can't be NULL.
 */
struct MTEE_TA_Function
{
    const char *TAName;
    const char *UUID;
    MTEE_TaInit init_func;
    MTEE_SessionCreateCallback create_func;
    MTEE_SessionCloseCallback close_func;
    MTEE_ServiceCallFunction service_func;
};

/**
 * Register TA
 *
 * Usage:
 *     struct MTEE_TA_Function ta1 = {...};
 *     TA_REGISTER(ta1);
 *
 * @param ta_func MTEE_TA_Function to register.
 */
#define TA_REGISTER(ta_func)   \
    static const struct MTEE_TA_Function *ta_register __attribute__((__used__)) \
        __attribute__((__section__(".ta_register"))) = &ta_func;

/* Init functions */
/**
 * Register init function to be called when system init.
 *
 * Init functions are spearate into 4 different levels.
 * The init order will be:
 *   plafrom_init, driver_init, module_init, TA init, late_init
 * platform_init & driver_init will be called in TEE init, after TEE init 
 * all cpu cores.
 * module_init & TA_init & late_init will be called when Linux kernel TZ
 * module is ready.
 * The order for init functions in the same levels are not defined.
 *
 * Usage:
 *   void libabc_init();
 *   module_init(libabc_init);
 */
typedef void (*MTEE_InitFunc)(void);
#define register_init(init_func, num) \
    static const MTEE_InitFunc __init_ptr_ ## init_func  __attribute__((__used__)) \
        __attribute__((__section__(".initfunc" #num))) = init_func;
#define platform_init(init_func) register_init(init_func, 0)
#define driver_init(init_func)   register_init(init_func, 1)
#define module_init(init_func)   register_init(init_func, 2)
#define late_init(init_func)     register_init(init_func, 3)


/* Resource management */

/** 
 * Free resource function prototype. 
 * Used by generic resource managment MTEE_RegisterSessionResource()
 *
 * @param user_data The resource to be freed
 */
typedef void (*MTEE_ResourceFreeFunction)(void *user_data);

/**
 * Register resource used by a session.
 * The resource free function will be called when the session is closed
 *
 * NOTE:
 *   If you register with the same session & resource more than once,
 * it will be called the same times it was registered when the session
 * is closed. This is so it can handle resource with reference count
 * correctly.
 *
 * Usage example:
 *    ptr = malloc(1024);
 *    MTEE_RegisterSessionResource(session, free, ptr);
 *
 * @param handle Session handle that own the resource
 * @param func Resource free callback to be called when the session is closed
 * @param user_data Pointer to the resource
 * @return 0 if success. <0 for error.
 */
TZ_RESULT MTEE_RegisterSessionResource(MTEE_SESSION_HANDLE handle,
                                       MTEE_ResourceFreeFunction func,
                                       void *user_data);

/**
 * Unregister resource used by a session.
 * If the resource is freed before the session is closed, use this function
 * to unregister it in generic resource manager.
 *
 * @param handle Session handle that own the resource
 * @param func Resource free callback to be called when the session is closed
 * @param user_data Pointer to the resource
 * @return 0 if success. <0 for error
 */
TZ_RESULT MTEE_UnregisterSessionResource(MTEE_SESSION_HANDLE handle,
                                         MTEE_ResourceFreeFunction func,
                                         void *user_data);

/**
 * Query if a resource is used by the session.
 *
 * @param handle Session handle that own the resource
 * @param func Resource free callback to be called when the session is closed
 * @param user_data Pointer to the resource
 * @return 0 if the resource was not registered. !=0 if it was registered.
 */
int MTEE_QuerySessionResource(MTEE_SESSION_HANDLE handle,
                              MTEE_ResourceFreeFunction func,
                              void *user_data);


/* Generic Handle Manager */
typedef void *MTEE_HANDLE_TYPE_ID;

/**
 * Init/create a new generic handle manager handle type id
 *
 * @see MTEE_HandleAllocate()
 * @param hname Name for new handle namespace, must be uniq & always there
 *             ex,  MTEE_HandleInit("Session")
 * @return Type ID for the handle. 0 if the name is already used.
 */
MTEE_HANDLE_TYPE_ID MTEE_HandleInit(const char *hname);

/**
 * Generate & allocate a new handle for the object
 *
 * Some API in TEE need to return a handle to REE world to represent a 
 * resouce or object. To prevent information leaking, TEE must not use
 * object address in TEE as a handle. Generic handle manager is a generic
 * solution for this problem.
 *
 * Each user need to create its own MTEE_HANDLE_TYPE_ID, so user can make
 * sure the handle object type is correct. The generated handled is a 32
 * bits integer. It is guarantee the generated handle will not be zero,
 * and the most significant bit will be zero.
 *
 * Call MTEE_HandleAllocate() when there is a new object that need a new 
 * handle. Handler manager will create a new handle and keep the mapping
 * of the handle and object for future reference.
 *
 * @param hid Handle type id.
 * @param object The object to create a new handle.
 * @param pHandle Pointer to place for returned handle.
 * @return 0 if success. <0 for error
 */
TZ_RESULT MTEE_HandleAllocate(MTEE_HANDLE_TYPE_ID hid, void *object, 
                              uint32_t *pHandle);

/**
 * Get the handle's object
 * When success, return the object register by MTEE_HandleAllocate.
 * The object will be locked until user call MTEE_HandlePutObject
 * or MTEE_HandleFree. The other user will busy-waiting in MTEE_HandleGetObject
 * so please keep the section as small as possible.
 *
 * This limitation is to fix handle object used after free race
 * condition issue. If without lock, when Thread A do this:
 *     obj = MTEE_HandleGetObject();
 *     access obj A;
 * and Thread B doing a MTEE_HandleFree, the access in Thread A might
 * be invalid.
 *
 * @param hid Handle type id.
 * @param handle The handle returned by MTEE_HandleAllocate()
 * @return The handle object. NULL if the handle was not registered or
 *         type id error. If not NULL, user must call MTEE_HandlePutObject
 *         after use complete.
 */
void* MTEE_HandleGetObject(MTEE_HANDLE_TYPE_ID hid, uint32_t handle);

/**
 * Put the handle's object after use. 
 * Allow other to access this object.
 *
 * @param hid Handle type id.
 * @param handle The handle returned by MTEE_HandleAllocate()
 */
void MTEE_HandlePutObject(MTEE_HANDLE_TYPE_ID hid, uint32_t handle);

/**
 * Remove the object handle
 * User must call MTEE_HandleGetObject() before using this function.
 * Please note this only free the handle allocated for the object and does
 * not free the object itself.
 *
 * @param hid Handle type id.
 * @param handle The handle returned by MTEE_HandleAllocate()
 * @return 0 if success. <0 for error
 */
TZ_RESULT MTEE_HandleFree(MTEE_HANDLE_TYPE_ID hid, uint32_t handle);


/* IRQ Support */
/** Irq handler */
typedef TZ_RESULT (*MTEE_IrqHandler)(unsigned int irq, void *user_data);

/* IRQ Flags */
#define MTIRQF_SHARED          (1<<0)   /// Share with other handlers
#define MTIRQF_TRIGGER_LOW     (1<<1)   /// IRQ is triggered by low signal
#define MTIRQF_TRIGGER_HIGH    (1<<2)   /// IRQ is triggered by high signal
#define MTIRQF_TRIGGER_RISING  (1<<3)   /// IRQ is triggered by rising edge
#define MTIRQF_TRIGGER_FALLING (1<<4)   /// IRQ is triggered by falling edge

#define MTIRQF_NORMAL          0        /// Deprecated, please set TRIGGER flags

/**
 * Request IRQ
 * The irq number will be setup to be received as IRQ interrupt and
 * call the irq handler in interrupt context. If the IRQ handler 
 * return TZ_RESULT_SUCCESS, the interrupt is consider handled.
 *
 * NOTE: In IRQ handler, you can't sleep or trying to get semaphore lock.
 * NOTE: Currently, we only support share IRQ between REE & TEE world.
 *       Multiple handlers sharing one IRQ in TEE is not supported.
 *
 * @param irq_no The irq number to receive.
 * @param handler Irq handler function.
 * @param flags Bit-ored flags of the irq. See MIRQF_ for flags.
 * @param user_data When IRQ occured, handler will be called with this
 *                  user_data value. If MTIRQF_SHARED is set in flags,
 *                  this must be a uniq value.
 * @return 0 if success. <0 for error
 */ 
TZ_RESULT MTEE_RequestIrq(unsigned int irq_no, MTEE_IrqHandler handler,
                          unsigned int flags, void *user_data);

/**
 * Enable the irq line 
 *
 * @param irq_no The irq number
 */
void MTEE_EnableIrq(unsigned int irq_no);

/**
 * Disable the irq line 
 *
 * @param irq_no The irq number
 */
void MTEE_DisableIrq(unsigned int irq_no);

/**
 * Check if we are in irq handler.
 *
 * @return 0 if not in irq. !=0 if in irq handler.
 */
int MTEE_InIrqHanlder();


/* Clock Support */
/**
 * Enable clock
 *
 * @param id The clock id. The clock ID & clock name are platform dependent
 * @param name clock name
 * @return 0 if success. <0 for error
 */
TZ_RESULT MTEE_EnableClock(unsigned int id, char *name);

/**
 * Disable clock
 *
 * @param id The clock id. The clock ID & clock name are platform dependent
 * @param name clock name
 * @return 0 if success. <0 for error
 */
TZ_RESULT MTEE_DisableClock(unsigned int id, char *name);



/* IRQ Mask/Restore */

/**
 * Mask IRQ/FIQ and return previous state.
 *
 * @return Previous IRQ/FIQ state.
 */
static inline uint32_t MTEE_MaskIrq()
{
    unsigned int ret;
    __asm__ volatile(
        "    mrs    %0, cpsr\n"
        "    cpsid  if\n"
        : "=r" (ret)
        :
        : "cc", "memory");
    return ret;    
}

/**
 * Restore IRQ/FIQ to previous state.
 *
 * @param state Previous IRQ/FIQ state to restore to.
 */
static inline void MTEE_RestoreIrq(uint32_t state)
{
    __asm__ volatile(
        "    msr    cpsr_c, %0\n"
        :
        : "r" (state)
        : "cc", "memory");
}

/**
 * Check if both IRQ and FIQ were disabled
 *
 * @return If both IRQ and FIQ were disabled, return non-0
 */
static inline uint32_t MTEE_IrqIsDisabled()
{
    unsigned int value;
    __asm__ volatile(
        "    mrs    %0, cpsr\n"
        : "=r" (value)
        :
        : "cc", "memory");
    return (value & (0xc0)) == 0xc0;
}

/** 
 * Secure function prototype. 
 * Used by secure function MTEE_SecureFunction()
 *
 * @param user_data A pointer to the user data.  
 */
typedef void (*MTEE_SecureFunctionPrototype)(void *user_data);

/**
 * Secure function call
 *
 * In secure function, all memory MUST be in onChip memory.
 * This function call will help to replace stack in onChip memory.
 * But programmer MUST take care other memory usage, and make sure they are in onChip memroy.
 *
 * Notice, because the resource of sram stack is limited, it will be failed if out of mmeory.
 * In this case, the function will return fail directly.
 *
 * For secure consideration, in secure function, Ree servicecall related APIs can not be used.
 * It will abort if ree service is used in secure function.
 * Simply, only spinlock can be used for IPC.
 * 
 * Interrupt will be disabled during the secure function.
 *
 * @param func      A pointer to the function.
 * @param user_data      A pointer to the user data.  
 * @return      return value.  
 */ 
uint32_t MTEE_SecureFunction (MTEE_SecureFunctionPrototype func, void *user_data);


/* Cache Maintenance */
/** Specify DMA direction */
typedef enum
{
    MTEE_DMA_FROM_DEVICE,
    MTEE_DMA_TO_DEVICE,
    MTEE_DMA_BIDIRECTION,
} MTEE_DMA_DIRECTION;

/**
 * Map a memory for device use.
 * This is for cache coherent operation for memory region used by both
 * CPU & HW device. Memory region is either used by CPU or HW device at
 * any given time. Before starting HW DMA, device driver must use MAP
 * function to mark the memory region as used by HW. After HW DMA is
 * completed and before CPU trying to access this memory, device driver
 * must call UNMAP function. These functions will do necessary cache
 * operation to ensure cache coherent between CPU & HW.
 *
 * NOTE:
 *   1. This function can only be used on secure DRAM and secure CM.
 *   2. The cpu_addr & size will be aligned to cache line.
 *
 * @param cpu_addr CPU address for the memory
 * @param size of the memory
 * @param direction The DMA direction.
 */
void MTEE_DmaMapMemory(void *cpu_addr, unsigned int size,
                       MTEE_DMA_DIRECTION direction);

/**
 * Unmap a previous mapped memory for CPU use.
 *
 * @see MTEE_DmaMapMemory
 * @param cpu_addr CPU address for the memory
 * @param size of the memory
 * @param direction The DMA direction.
 */
void MTEE_DmaUnmapMemory(void *cpu_addr, unsigned int size,
                         MTEE_DMA_DIRECTION direction);


/* Physical/Virtual */
/**
 * Translate from physical address to virtual address
 * Only support translate for Secure DRAM & Secure Chunk Memory
 *
 * @param phys_addr Physical memory address to translate.
 * @return Virutal address pointer. NULL if input physical address can't 
 *         not be translate
 */
void *MTEE_Virtual(unsigned long phys_addr);

/**
 * Translate from virtual address to physical address
 * Only support already mapped virtual address & Secure DRAM &
 * Secure Chunk Memory
 *
 * @param virt_addr Virtual address to translate
 * @return Physical address. 0 if input virtual is not known.
 */
unsigned long MTEE_Physical(void *virt_addr);


/* Misc */
/**
 * Puts a message in REE log system
 *
 * @param buf Message to print
 */
void MTEE_Puts(const char *buf);

/**
 * Sleep a little while
 *
 * @param ustime Sleep time in micro-sconds
 */
void MTEE_USleep(unsigned long ustime);

/**
 * Busy-looping delay for a little while
 *
 * @param ustime Delay time in micro-sconds
 */
void MTEE_UDelay(unsigned long ustime);

/* Thread function */
typedef uint32_t MTEE_THREAD_HANDLE;

/**
 * Thread function
 *
 * TEE can create kthread through ree service.
 * This is kthread function prototype.
 *
 * @param user_data A pointer to the user data. 
 */ 
typedef void *(*MTEE_ThreadFunc)(void *user_data);

/*
 * Create a new thread, the thread_func will be called in new thread context
 * The thread will be terminated if the function return or MTEE_ExitThread
 * is called.
 *
 * @param pHandle Returned thread handle.
 * @param thread_func Function for new thread.
 * @param user_data Pass to thread_func when excute.
 * @return return value.
 */
TZ_RESULT MTEE_CreateThread (MTEE_THREAD_HANDLE *handle, MTEE_ThreadFunc fuc, void *user_data, char *name);

/*
 * Wait for a specific thread to terminated.
 * This function must be called for terminated thread, otherwise thread
 * resource might not be freed.
 *
 * @param handle The thread to wait.
 * @param result Return value returned by thread.
 * @return return value.
 */
TZ_RESULT MTEE_JoinThread (MTEE_THREAD_HANDLE handle, void **result);


/* power management */
/*
* Suspend/Resume function prototype
* This function is used for saving the device state.
*/
typedef int (*MTEE_PM_Func)(void *user_data);

struct MTEE_PM_DEV_OP
{
    const char *DevName;
    void *user_data;
    MTEE_PM_Func suspend_func;
    MTEE_PM_Func suspend_late_func;
    MTEE_PM_Func resume_func;
    MTEE_PM_Func resume_early_func;
};

/*
* Register the pm suspend/resume functions. They will be called when system is
* going into standby/hibernation state or out from it.
* The TZ driver is installed at arch_initcall(), so the suspend execution order will
* be almost the last one. (only eariler than etb, etm, arm-pmu, CCI)
*
* @param ppmop the function/data struct for registeration.
* @return 0 if success. <0 for error.
*/
TZ_RESULT MTEE_PM_RegisterDevOp(const struct MTEE_PM_DEV_OP *ppmop);


uint32_t MTEE_getReleaseCmSize (void);


/*
* Set the function to be called after initialization and before leaving secure world.
* It will be used to install function for the platform dependent flow. 
* (L2/sram sharing behavior after 6582)
*/
void MTEE_SetPlatformSecureWorldLeaveFunction(void (*func)(void));

#if 0
TBD function. Not implement, not sure will be implemented now.

/*
 * Exit calling thread.
 *
 * @param result The result to return to MTEE_JoinThread.
 * @return Return value. Only return if current threaed is not created by
 *         MTEE_CreateThread().
 */
TZ_RESULT MTEE_ExitThread(void *result);


/**
 * Query the object's handle
 *
 * @param hid Handle type id.
 * @param object The object to query
 * @return Registered handle of the object. 0 if the object was not
 *         registered or type id error.
 */
uint32_t MTEE_HandleQueryHandle(MTEE_HANDLE_TYPE_ID hid, void *object);

#endif /* TBD functions*/

#endif /* __MTEE_SYSTEM_H__ */
