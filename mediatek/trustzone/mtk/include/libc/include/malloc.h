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
#ifndef __MALLOC_H
#define __MALLOC_H

#include <sys/types.h>
#include <compiler.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/sys_malloc.h>

#ifdef DBG_MEM_ALLOCATOR
//#define DBG_MEM_ALLOCATOR_SYS
#endif

#if 1
#ifndef DBG_MEM_ALLOCATOR_SYS
#define malloc(size) secure_malloc(size)
#define memalign(alignment, size) secure_memalign(alignment, size)
#define free(buffer) secure_free(buffer)
#define realloc(old_buffer, size) secure_realloc(old_buffer, size)
#define calloc(element, elem_size) secure_calloc(element, elem_size)
#else
#define DBG_MEM_SYS_ID "SYS"
#define malloc(size) dbg_malloc(size, DBG_MEM_SYS_ID)
#define memalign(alignment, size) dbg_memalign(alignment, size, DBG_MEM_SYS_ID)
#define free(buffer) dbg_free(buffer, DBG_MEM_SYS_ID)
#define realloc(old_buffer, size) dbg_realloc(old_buffer, size, DBG_MEM_SYS_ID)
#define calloc(element, elem_size) dbg_calloc(element, elem_size, DBG_MEM_SYS_ID)
#endif
#else
void *malloc(size_t size) __MALLOC;
void *memalign(size_t boundary, size_t size) __MALLOC;
void *calloc(size_t count, size_t size) __MALLOC;
void free(void *ptr);
void *realloc(void *ptr, size_t size);
#endif

#if defined(__cplusplus)
}
#endif

#endif

