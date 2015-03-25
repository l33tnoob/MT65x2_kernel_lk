#ifndef __MT_MEMORY_H__
#define __MT_MEMORY_H__

/*
 * Define constants.
 */
#define RESERVED_MEM_SIZE_FOR_CONNSYS (0x100000)
#define DRAM_PHYS_ADDR_START (0x80000000)
#define PHYS_OFFSET (DRAM_PHYS_ADDR_START + RESERVED_MEM_SIZE_FOR_CONNSYS)

/*
 * Define macros.
 */

/* IO_VIRT = 0xF0000000 | IO_PHYS[27:0] */
#define IO_VIRT_TO_PHYS(v) (0x10000000 | ((v) & 0x0fffffff))
#define IO_PHYS_TO_VIRT(p) (0xf0000000 | ((p) & 0x0fffffff))

#endif  /* !__MT_MEMORY_H__ */
