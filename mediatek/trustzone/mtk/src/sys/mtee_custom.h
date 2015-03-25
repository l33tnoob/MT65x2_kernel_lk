/* Define custom part. */

#ifndef __MTEE_CUSTOM_H__
#define __MTEE_CUSTOM_H__

#define VIRT_OFFSET (0x10000000)
struct tee_memory_config {
        unsigned long phys_memory_base;	     // Start of secure memory
        unsigned int total_memory_size;      // Total secure memory (including chunk)
        unsigned int chunk_buffer_size;      // Chunk memory size.
        unsigned long phys_boot_share;       // Pysical memory for boot shared memory.
        unsigned int version;                // Memory config version
};
extern const struct tee_memory_config tee_memory_config;

struct tee_secure_func_config {
        unsigned int stack_num;    // stack number
        unsigned int stack_size;    // stack size
};
extern const struct tee_secure_func_config tee_secure_func_config;

struct tee_tzmem_release_config {
        unsigned int cm_size;    // chunkmem release size
};
extern const struct tee_tzmem_release_config tee_tzmem_release_config;


#endif /* __MTEE_CUSTOM_H__ */
