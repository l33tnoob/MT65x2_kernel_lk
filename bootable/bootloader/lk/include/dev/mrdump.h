#if !defined (__MRDUMP_H__)
#define __MRDUMP_H__

#include <sys/types.h>

struct mrdump_regset
{

    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    
    uint32_t fp; /* r11 */
    uint32_t r12;
    uint32_t sp; /* r13 */
    uint32_t lr; /* r14 */
    uint32_t pc; /* r15 */
    
    uint32_t cpsr;
};

struct mrdump_regpair {
    uint32_t addr;
    uint32_t val;
};

int aee_kdump_detection(void);

void mrdump_run(const struct mrdump_regset *per_cpu_regset, const struct mrdump_regpair *regpairs);

#endif
