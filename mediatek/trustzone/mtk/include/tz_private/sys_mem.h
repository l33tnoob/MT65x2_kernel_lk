
#ifndef __MTEE_MEM_H__
#define __MTEE_MEM_H__

#include "tz_cross/trustzone.h"
#include "tz_private/system.h"
#include "tz_private/sys_mem.h"
#include <malloc.h>

/**
 * Memory handle define
 *
 * Handle is used to communicate with normal world:
 * 1. Memory information can not expose to normal world. (Major, important!)
 * 2. Too many informations, and thet can be grouped by handle.
 *
 * All kinds of memory use the same handle define.
 * According to their different purpose, they are redefined to specific name.
 * Just for easy programming.
 */

// Shared memory handle define
typedef uint32_t MTEE_SHAREDMEM_HANDLE;  

// Secure memory handle define
typedef uint32_t MTEE_SECUREMEM_HANDLE; 

// Secure chunk memory handle define
typedef uint32_t MTEE_SECURECM_HANDLE; 

typedef uint32_t MTEE_RELEASECM_HANDLE; 


/**
 * Shared memory parameter
 *
 * It defines the types for shared memory.
 *
 * @param buffer    A pointer to shared memory buffer
 * @param size    shared memory size
 */
typedef struct { 
    void* buffer; 
    uint32_t size; 
} MTEE_SHAREDMEM_PARAM;

/**
 * Memory parameter
 *
 * It defines the parameters for memory.
 *
 * @param buffer    A pointer to shared memory buffer
 * @param size    shared memory size
 */
typedef struct { 
    void* buffer; 
    int size; 
} MTEE_MEM_PARAM;

/**
 * Shared memory information
 *
 * It defines the information structure for shared memory.
 *
 * @param parameter    A pointer to shared memory parameter
 * @param page_num    Page number
 * @param mmuTable     A pointer to mmu table
 */
typedef struct { 
    MTEE_SHAREDMEM_PARAM parameter;
    uint32_t page_num;
    uint32_t *mmuTable;
} MTEE_SHAREDMEM;


/**
 * TEE memory map
 *
 * Four levels/kinds of TEE memory:
 * 1. On-chip secure memory
 * 2. External secure memory (secure memory, in brief)
 * 3. External secure chunk memory (secure chunk memory, in brief)
 * 4. Non-secure memory (includes shared memory)
 * 
 * Each memory has different secure strength.
 * Here, followed GP defined TEE resource property.
 */

/**
 * Query memory rank
 *
 * To query memory rank, which is memory pool ID.
 * It is used for checking memory if it secure enough or not.
 *
 * @param buffer    a pointer to memory.
 * @return    memory rank, defined as KERNELMEM_ID. If not secure memory, return 0xFFFFFFFF.
 */
uint32_t MTEE_QueryMemoryrank (void *buffer);

/**
 * Get physical address
 *
 * Get physical address according to virtual address.
 * Currently, all memory in TEE is physical continous. Only VA to PA converting is needed.
 *
 * @param buffer    A pointer to memory with virtual address.
 * @return    A pointer to memory with physical address. If fail, it is NULL.
 */ 
void *get_physicaladdr (void *buffer);
#define MTEE_GetPhysicaladdr(buffer) get_physicaladdr(buffer);

/**
 * On-chip secure memory
 *
 * An On-chip secure memory can be seen only in Secure world.
 * On-chip secure memory, here, is defined as on-chip memory (ex: On-chip SRAM) protected by trustzone.
 * It can protect from software attack and some physical very well, but can not protect from tamper attack, like decap.
 *
 * Secure memory spec.:
 * 1. Protected by trustzone (NS = 0).
 * 2. On-chip memory (ex: On-chip SRAM).
 * 3. With cache. 
 * 4. Physically continuous.
 */
 
/**
 * Allocate onchip secure memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_AllocOnchipMem(size) onchip_malloc(size)

/**
 * Free onchip secure memory
 *
 * @param buffer    The pointer to the memory block to be referenced
 */
#define MTEE_FreeOnchipMem(buffer) onchip_free(buffer)  

/**
 * Query allocated onchip secure memory size
 *
 * @param buffer    The pointer to the memory block to be queried.
 * @return    memoy size in bytes. It may be slightly larger than allcated because of alignment.
 */
#define MTEE_QueryOnchipMemsize(buffer) secure_memsize(buffer)  


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
 * Allocate secure memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_AllocMem(size) secure_malloc(size)

/**
 * Free secure memory
 *
 * @param buffer    The pointer to the memory block to be referenced
 */
#define MTEE_FreeMem(buffer) secure_free(buffer)  

/**
 * Reallocation secure memory 
 *
 * @param buffer    The pointer to the memory block to be realocated
 * @param size    The size of the buffer to be allocated in bytes 
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_ReallocMem(old_buffer, size) secure_realloc(old_buffer, size)

/**
 * Calloc secure memory 
 *
 * @param element    number of element wanted to allocate
 * @param elem_size    The size of one element in bytes 
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_CallocMem(element, elem_size) secure_calloc(element, elem_size)

/**
 * Allocate secure memory with alignment
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param alignment    Memory alignment in bytes. 
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_AllocMemAlign(alignment, size) secure_memalign(alignment, size)

/**
 * Query allocated secure memory size
 *
 * @param buffer    The pointer to the memory block to be queried.
 * @return    memoy size in bytes. It may be slightly larger than allcated because of alignment.
 */
#define MTEE_QueryMemsize(buffer) secure_memsize(buffer)  

/**
 * Query information of secure memory
 *
 * @param info    The pointer to the memory information defined as MTEE_KERNELMEM.
 */
#define MTEE_QueryMem(info) secure_meminfo(info)  

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
 * 4. Physically continuous.
 * 5. For future, it can be released to normal world.
 */

/**
 * Allocate secure chunk memory
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_AllocChunkmem(size) cm_malloc(size)

/**
 * Free secure chunk memory
 *
 * @param buffer    The pointer to the memory block to be referenced
 */
#define MTEE_FreeChunkmem(buffer) cm_free(buffer)  

/**
 * Reallocation secure chunk memory 
 *
 * @param buffer    The pointer to the memory block to be realocated
 * @param size    The size of the buffer to be allocated in bytes 
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_ReallocChunkmem(old_buffer, size) cm_realloc(old_buffer, size)

/**
 * Allocate secure chunk memory with alignment
 *
 * @param size    The size of the buffer to be allocated in bytes
 * @param alignment    Memory alignment in bytes. 
 * @return    If success, return address. If fail, return NULL.
 */ 
#define MTEE_AllocChunkmemAlign(alignment, size) cm_memalign(alignment, size)

/**
 * Query allocated secure chunk memory size
 *
 * @param buffer    The pointer to the memory block to be queried.
 * @return    memoy size in bytes. It may be slightly larger than allcated because of alignment.
 */
#define MTEE_QueryChunkmemsize(buffer) secure_memsize(buffer)  

/**
 * Query information of secure chunk memory
 *
 * @param info    The pointer to the memory information defined as MTEE_KERNELMEM.
 */
#define MTEE_QueryChunkmem(info) cm_meminfo(info)  

/**
 * Get Releasable chunk memory size
 *
 * @return    Releasable chunk memory size.
 */
#define MTEE_GetReleasablesize() cm_get_releasablesize()



/* Memory TA used only
*/

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

int MTEE_InitSharedmem (void *start, uint32_t size);
void *MTEE_AllocSharedmem (uint32_t size);
int MTEE_FreeSharedmem (void *buffer);
uint32_t *MTEE_CreateMmutbl (MTEE_SHAREDMEM *param);
void MTEE_DestroyMmutbl (uint32_t *mmutbl);
void MTEE_SetupMmutbl (uint32_t *mmutbl, uint32_t va_base, uint32_t num);
void MTEE_ClearMmutbl (uint32_t virt_addr, int size);

void MTEE_ReleaseCmMmu (void *cm_va, void *shared_va, void *cm_pa, uint32_t size);
void MTEE_AppendMmu (void *cm_va, void *cm_pa, void *shared_va, void *shared_pa, uint32_t size);

#endif /* __MTEE_MEM_H__ */

