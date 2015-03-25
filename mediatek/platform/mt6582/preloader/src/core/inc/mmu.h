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
#ifndef __ARCH_ARM_MMU_H
#define __ARCH_ARM_MMU_H

#include "typedefs.h"

void arm_mmu_init(void);

/* C, B and TEX[2:0] encodings without TEX remap */
                                                       /* TEX      |    CB    */
#define MMU_MEMORY_TYPE_STRONGLY_ORDERED              ((0x0 << 12) | (0x0 << 2))
#define MMU_MEMORY_TYPE_DEVICE_SHARED                 ((0x0 << 12) | (0x1 << 2))
#define MMU_MEMORY_TYPE_DEVICE_NON_SHARED             ((0x2 << 12) | (0x0 << 2))
#define MMU_MEMORY_TYPE_NORMAL                        ((0x1 << 12) | (0x0 << 2))
#define MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH          ((0x0 << 12) | (0x2 << 2))
#define MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE ((0x0 << 12) | (0x3 << 2))
#define MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE    ((0x1 << 12) | (0x3 << 2))

#define MMU_MEMORY_AP_NO_ACCESS     (0x0 << 10)
#define MMU_MEMORY_AP_READ_ONLY     (0x7 << 10)
#define MMU_MEMORY_AP_READ_WRITE    (0x3 << 10)

#define MMU_MEMORY_XN               (0x1 << 4)

///////////////////////////////////////////////////
#define NON_SECURE              0x00080000
#define SECURE                  0x00000000

#define NON_GLOBAL              0x00020000
#define GLOBAL                  0x00000000

#define SHARE                   0x00010000
#define NON_SHARE               0x00000000

#define STRONGLY_ORDER          0x00000000
#define SHARED_DEVICE           0x00000004
#define NONCACHABLE             0x00001000
#define WRITEBACK               0x0000000C
#define WRITETHROUGH            0x00000008
#define NON_SHARED_DEVICE       0x00002000

#define PRIVILEGED_READ_WRITE   0x00000400
#define PRIVILEGED_READ         0x00008400

#define EXECUTE                 0x00000000
#define NON_EXECUTE             0x00000010

//-------------------------------------------------
// Strongly Order Memory (SOM)
//-------------------------------------------------
#define MMUv6_SOM       (SECURE|NON_GLOBAL|SHARE|STRONGLY_ORDER)

//-------------------------------------------------
// Non-Shared Normal Memory (NSNM)
//-------------------------------------------------
#define MMUv6_NSNM_WB   (SECURE|NON_GLOBAL|NON_SHARE|WRITEBACK)     // cacheable (write-back)
#define MMUv6_NSNM_NC   (SECURE|NON_GLOBAL|NON_SHARE|NONCACHABLE)   // non-cacheable (bufferable)
#define MMUv6_NSNM_WT   (SECURE|NON_GLOBAL|NON_SHARE|WRITETHROUGH)  // cacheable (write-through) 


void arm_mmu_map_section(ulong paddr, ulong vaddr, u32 flags);
#endif
