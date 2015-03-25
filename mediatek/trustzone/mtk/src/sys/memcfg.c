/* Memory configuration customize for TEE */
/* This is non-private, build by config */

#include "mtee_custom.h"

const struct tee_memory_config tee_memory_config = { 0, MEMSIZE, FBSIZE, BOOTSHARE, 0};
const struct tee_secure_func_config tee_secure_func_config = {SECURE_FUNC_STACK_NUM, SECURE_FUNC_STACK_SIZE};
const struct tee_tzmem_release_config tee_tzmem_release_config = {TZMEM_RELEASECM_SIZE};


