#include <stddef.h>

#ifndef _TZ_MEM_H_
#define _TZ_MEM_H_

#define SRAM_BASE_ADDRESS   0x00100000
#define SRAM_START_ADDR     0x00102140
#define VECTOR_START        (SRAM_START_ADDR + 0xBAC0)

typedef struct tz_memory_t {
    short next, previous;
} tz_memory_t;

#define FREE            ((short)(0x0001))
#define IS_FREE(x)      ((x)->next & FREE)
#define CLEAR_FREE(x)   ((x)->next &= ~FREE)
#define SET_FREE(x)     ((x)->next |= FREE)
#define FROM_ADDR(x)    ((short)(ptrdiff_t)(x))
#define TO_ADDR(x)      ((tz_memory_t *)(SRAM_BASE_ADDRESS + ((x) & ~FREE)))

#endif