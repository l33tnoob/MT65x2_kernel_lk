/** Commands for TA memory **/

#ifndef __TRUSTZONE_TA_SYS_MEM__
#define __TRUSTZONE_TA_SYS_MEM__

#include "tz_cross/ta_mem.h"
#include "tz_private/sys_mem.h"

/**
 * Memory object define
 *
 * @param buffer    A pointer to memory buffer.
 * @param size    memory size in bytes.
 * @param alignment    Memory alignment in bytes. 
 * @param reference    reference count.
 */
typedef struct {
    void *buffer;
    uint32_t size;
    uint32_t alignment;
    uint32_t reference;
} MTEE_MEM_OBJ;

/**
 * Memory object define
 *
 * @param buffer    A pointer to memory buffer.
 * @param size    memory size in bytes.
 * @param mmutbl    A pointer to MMU table.
 */
typedef struct {
    void *buffer;
    uint32_t size;
    uint32_t offset;
    uint32_t page_num;
    uint32_t *mmutbl; 
    int control; // 0 = no remap, 1 = remap
} MTEE_SHAREDMEM_OBJ;

/**
 * Query registered shared memory
 *
 * To query shared memory buffer by handle.
 * It is used when handle is send from REE to get the memory parameters.
 *
 * @param shaerdMem    Shared Memory handle.
 * @param param    The pointer to the shared memory parameters.
 * @return    return code.
 */
TZ_RESULT TA_Mem_QuerySharedmem (MTEE_SHAREDMEM_HANDLE handle, MTEE_SHAREDMEM_PARAM *param);

/**
 * Query registered shared memory
 *
 * To query memory buffer by handle.
 * It queries memory, which is allocated in secure world by related APIs.
 * It is used when handle is send from REE to get the memory parameters.
 *
 * @param handle    Memory handle.
 * @param param    The pointer to the shared memory parameters.
 * @return    return code.

 */
TZ_RESULT TA_Mem_QueryMem (uint32_t handle, MTEE_MEM_PARAM *param);

/**
 * Shared memory
 *
 * A shared memory is normal memory, which can be seen by Normal world and Secure world.
 * It is used to create the comminicattion between two worlds.
 * Currently, zero-copy data transfer is supportted, for simple and efficient design.
 *
 * The shared memory lifetime:
 * 1. CA (Client Application at REE) prepares memory
 * 2. CA registers it to TEE scope.
 * 3. A handle is returned. CA can use it to communicate with TEE.
 * 4. If shared memory is not used, CA deregisters it.
 * 5. CA frees memory.
 *
 * Note: Because shared memory can be seen by both Normal and Secure world.
 * It is a possible weakpoint to bed attacked or leak secure data.
 */

/**
 * Register shared memory
 *
 * @param handle    A pointer to shared memory handle.
 * @param param    A pointer to shared memory paramete, defined as MTEE_SHAREDMEM_PARAM
 * @return    If success, return address. If fail, return NULL.
 */ 
void *TA_Mem_RegisterSharedMem (MTEE_SHAREDMEM_HANDLE *handle, MTEE_SHAREDMEM *param);

/**
 * Unregister shared memory
 *
 * @param shaerdMem    Shared memory handle.
 * @return    return code.
 */ 
TZ_RESULT TA_Mem_UnregisterSharedmem (MTEE_SHAREDMEM_HANDLE handle);

/**
 * Secure memory
 *
 * A secure memory can be seen only in Secure world.
 * Secure memory, here, is defined as external memory (ex: external DRAM) protected by trustzone.
 * It can protect from software attack very well, but can not protect from physical attack, like memory probe.
 *
 * Secure memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. External memory (ex: external DRAM).
 * 3. With cache.
 * 4. Physically continuous.
 *
 * All secure memory APIs are defined at <stdlib.h> as well-known memory APIs, ex: malloc.
 * Here, for programming style, some APIs are provided as TEE API style by macro define.
 */

/**
 * Secure memory allocation
 *
 * Allocate one memory.
 * If memory is allocated successfully, a pointer to the memory will be provided.
 * 
 * Memory lifetime:
 * 1. Allocate memory, and get the memory handle and pointer. Reference count will be 1 after allocating.
 * Memory pointer can be used directly, and memory handle is used to pass memory between sessions.
 * 2. If another session wants to use the same memory, reference it. 
 * 3. If it stops to use it, unreference it. Notice that, reference number must be equal to unreference number 
 * to prevent memory leak.
 * 4. Unreference it (for free memory), if the memory is not used. If reference count is 1, do unreference will free it.
 *
 * Simple rules:
 * 1. start by allocate, end by unreference (free).
 * 2. start by reference, end by unreference.
 *
 * @param handle    [out] A pointer to memory handle
 * @param align    Memory alignment in bytes. 
 * @param size    The size of the buffer to be allocated in bytes. 
 * @return    If success, return address. If fail, return NULL.
 */  
void *TA_Mem_AllocMem (MTEE_SECUREMEM_HANDLE *handle, uint32_t alignment, uint32_t size);

/**
 * Secure memory reference
 *
 * Reference memory.
 * Referen count will be increased by 1 after reference.
 * 
 * Reference lifetime:
 * 1. Reference the memory before using it.
 * 2. Unreference it if it is not used.
 *
 * @param buffer    The pointer to the memory block to be referenced
 * @return    If success, return address. If fail, return NULL.
 */ 
void *TA_Mem_ReferenceMem (MTEE_SECUREMEM_HANDLE handle);

/**
 * Secure memory unreference
 *
 * Unreference memory.
 * Reference count will be decreased 1 after unreference.
 * After unreference, if reference count is zero, memory will be freed.
 *
 * @param buffer    The pointer to the memory block to be referenced
 * @return    0 if success. <0 for error. otherwise for reference count 
 */
int TA_Mem_UnreferenceMem (MTEE_SECUREMEM_HANDLE handle);

/**
 * Secure chunk memory
 *
 * A secure chunk memory can be seen only in Secure world.
 * It is a kind of secure memory but with difference characteristic:
 * 1. It is designed and optimized for chunk memory usage.
 * 2. For future work, it can be released as normal memory for more flexible memory usage.
 *
 * Secure chunk memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. External memory (ex: external DRAM).
 * 3. With cache.
 * 4. For future, it can be released to normal world.
 */

/**
 * Secure chunk memory allocation
 *
 * Allocate one memory.
 * If memory is allocated successfully, a pointer to the memory will be provided.
 * 
 * Memory lifetime:
 * 1. Allocate memory, and get the memory handle and pointer. Reference count will be 1 after allocating.
 * Memory pointer can be used directly, and memory handle is used to pass memory between sessions.
 * 2. If another session wants to use the same memory, reference it. 
 * 3. If it stops to use it, unreference it. Notice that, reference number must be equal to unreference number 
 * to prevent memory leak.
 * 4. Unreference it (for free memory), if the memory is not used. If reference count is 1, do unreference will free it.
 *
 * Simple rules:
 * 1. start by allocate, end by unreference (free).
 * 2. start by reference, end by unreference.
 *
 * @param handle    [out] A pointer to memory handle
 * @param align    Memory alignment in bytes. 
 * @param size    The size of the buffer to be allocated in bytes. 
 * @return    If success, return address. If fail, return NULL.
 */  
void *TA_Mem_AllocChunkmem (MTEE_SECUREMEM_HANDLE *handle, uint32_t alignment, uint32_t size);

/**
 * Secure chunk memory reference
 *
 * Reference memory.
 * Referen count will be increased by 1 after reference.
 * 
 * Reference lifetime:
 * 1. Reference the memory before using it.
 * 2. Unreference it if it is not used.
 *
 * @param buffer    The pointer to the memory block to be referenced
 * @return    If success, return address. If fail, return NULL.
 */ 
void *TA_Mem_ReferenceChunkmem (MTEE_SECUREMEM_HANDLE handle);

/**
 * Secure chunk memory unreference
 *
 * Unreference memory.
 * Reference count will be decreased 1 after unreference.
 * After unreference, if reference count is zero, memory will be freed.
 *
 * @param buffer    The pointer to the memory block to be referenced
 * @return    0 if success. <0 for error. otherwise for reference count 
 */
int TA_Mem_UnreferenceChunkmem (MTEE_SECUREMEM_HANDLE handle);

#endif /* __TRUSTZONE_TA_SYSMEM__ */

