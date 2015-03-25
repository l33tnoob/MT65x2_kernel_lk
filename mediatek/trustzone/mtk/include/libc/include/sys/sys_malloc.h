/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __SYS_MALLOC_H
#define __SYS_MALLOC_H

//#define DBG_MEM_ALLOCATOR

/**
 * Kernel memory pools define
 *
 * @see MTEE_KERNELMEM_ID 
 */
typedef enum {
    KERNELMEM_ID_SECUREONCHIP = 0,
    KERNELMEM_ID_SECUREMEM,
    KERNELMEM_ID_SECURECM,
#ifdef DBG_MEM_ALLOCATOR
    KERNELMEM_ID_DBGMEM,
#endif
    KERNELMEM_ID_NUM
} KERNELMEM_ID;

typedef struct {
    uint32_t initial;
    void *pool;
    uint32_t minAlign;
    void *start;
    uint32_t size;
    uint32_t used_size;
} KERNELMEM_PARAM;

int secure_memInit (void *start, uint32_t size);
uint32_t secure_memrank (void *buffer);   

void secure_meminfo (KERNELMEM_PARAM *outInfo);
void *secure_malloc (uint32_t size);
void secure_free (void *buffer);
void *secure_realloc (void *old_buffer, uint32_t size);
void *secure_memalign (uint32_t alignment, uint32_t size);
uint32_t secure_memsize (void *buffer);
void *secure_calloc (uint32_t element, uint32_t elem_size);

int onchip_memInit (void *start, uint32_t size);
void *onchip_malloc (uint32_t size);
void onchip_free (void *buffer);
void *onchip_realloc (void *old_buffer, uint32_t size);
void *onchip_memalign (uint32_t alignment, uint32_t size);

void cm_meminfo (KERNELMEM_PARAM *outInfo);
uint32_t cm_get_releasablesize (void);
int cm_memInit (void *start, uint32_t size);
void *cm_malloc (uint32_t size);
void cm_free (void *buffer);
void *cm_realloc (void *old_buffer, uint32_t size);
void *cm_memalign (uint32_t alignment, uint32_t size);

#ifdef DBG_MEM_ALLOCATOR
int dbg_memInit (void *start, uint32_t size);
void *dbg_malloc (uint32_t size, char *string);
void dbg_free (void *buffer, char *string);
void *dbg_realloc (void *old_buffer, uint32_t size, char *string);
void *dbg_memalign (uint32_t alignment, uint32_t size, char *string);
void *dbg_calloc (uint32_t element, uint32_t elem_size, char *string);
#endif

#endif /* __SYS_MALLOC_H */

