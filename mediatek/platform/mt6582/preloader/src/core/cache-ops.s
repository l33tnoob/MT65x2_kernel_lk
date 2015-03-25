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


.text

/* uint32_t arm_read_cr1(void) */
.global arm_read_cr1; arm_read_cr1:
	mrc		p15, 0, r0, c1, c0, 0
	bx		lr

/* void arm_write_cr1(uint32_t val) */
.global arm_write_cr1; arm_write_cr1:
	mcr		p15, 0, r0, c1, c0, 0
	bx	lr
	
/* void arm_invalidate_tlb(void) */
.global arm_invalidate_tlb; arm_invalidate_tlb:
	mov		r0, #0
	mcr	p15, 0, r0, c8, c7, 0
	bx		lr
	
/* void arm_write_ttbr(uint32_t val) */
.global arm_write_ttbr; arm_write_ttbr:
	mcr	p15, 0, r0, c2, c0, 0
	bx		lr

/* void arm_write_dacr(uint32_t val) */
.global arm_write_dacr; arm_write_dacr:
	mcr	p15, 0, r0, c3, c0, 0
	bx		lr

.global  arm_cp15_dcache_invalidate; arm_cp15_dcache_invalidate:    
    push    {r4,r5,r7,r9,r10,r11}
    dmb                                     @@; ensure ordering with previous memory accesses
    mrc     p15, 1, r0, c0, c0, 1           @; read clidr
    ands    r3, r0, #0x7000000              @; extract loc from clidr
    mov     r3, r3, lsr #23                 @; left align loc bit field
    beq     .d_invalidate_finished                        @; if loc is 0, then no need to clean
    mov     r10, #0                         @; start clean at cache level 0
.d_invalidate_loop1:
    add     r2, r10, r10, lsr #1            @; work out 3x current cache level
    mov     r1, r0, lsr r2                  @; extract cache type bits from clidr
    and     r1, r1, #7                      @; mask of the bits for current cache only
    cmp     r1, #2                          @; see what cache we have at this level
    blt     .d_invalidate_skip                            @; skip if no cache, or just i-cache
    mcr     p15, 2, r10, c0, c0, 0          @; select current cache level in cssr
    isb                                     @; isb to sych the new cssr&csidr
    mrc     p15, 1, r1, c0, c0, 0           @; read the new csidr
    and     r2, r1, #7                      @; extract the length of the cache lines
    add     r2, r2, #4                      @; add 4 (line length offset)
    ldr     r4, =0x3ff
    ands    r4, r4, r1, lsr #3              @; find maximum number on the way size
    clz     r5, r4                          @; find bit position of way size increment
    ldr     r7, =0x7fff
    ands    r7, r7, r1, lsr #13             @; extract max number of the index size
.d_invalidate_loop2:
    mov     r9, r4                          @; create working copy of max way size
.d_invalidate_loop3:
    orr     r11, r10, r9, lsl r5            @; factor way and cache number into r11
    orr     r11, r11, r7, lsl r2            @; factor index number into r11
    mcr     p15, 0,  r11, c7, c6, 2         @; invalidate by set/way    
    subs    r9, r9, #1                      @; decrement the way
    bge     .d_invalidate_loop3                           @;
    subs    r7, r7, #1                      @; decrement the index
    bge     .d_invalidate_loop2
.d_invalidate_skip:
    add     r10, r10, #2                    @; increment cache number
    cmp     r3, r10
    bgt     .d_invalidate_loop1
.d_invalidate_finished:
    mov     r10, #0                         @; swith back to cache level 0
    mcr     p15, 2, r10, c0, c0, 0          @; select current cache level in cssr
    dsb
    isb
    pop     {r4,r5,r7,r9,r10,r11}
    bx      lr
	
.globl apmcu_icache_invalidate
apmcu_icache_invalidate:
    MOV r0, #0
    MCR p15, 0, r0, c7, c5, 0  /* CHECKME: c5 or c1 */
    BX  lr

.globl apmcu_dsb
apmcu_dsb:
    DSB
    BX  lr

.globl apmcu_isb
apmcu_isb:
    ISB
    BX  lr

.globl apmcu_disable_dcache
apmcu_disable_dcache:
    MRC p15,0,r0,c1,c0,0
    BIC r0,r0,#0x4
    MCR p15,0,r0,c1,c0,0
    BX  lr

.globl apmcu_disable_icache
apmcu_disable_icache:
    MOV r0,#0
    MCR p15,0,r0,c7,c5,6   /* Flush entire branch target cache */
    MRC p15,0,r0,c1,c0,0
    BIC r0,r0,#0x1800      /* I+Z bits */
    MCR p15,0,r0,c1,c0,0
    BX  lr

.globl apmcu_disable_smp
apmcu_disable_smp:
    MRC p15,0,r0,c1,c0,1
    BIC r0,r0,#0x040       /* SMP bit */
    MCR p15,0,r0,c1,c0,1
    BX  lr
 
.globl apmcu_enable_dcache
apmcu_enable_dcache:
    MRC p15,0,r0,c1,c0,0
    ORR r0,r0,#(1<<2)
    MCR p15,0,r0,c1,c0,0
    BX lr
	
.globl apmcu_dcache_clean_invalidate
apmcu_dcache_clean_invalidate:
    push    {r4,r5,r7,r9,r10,r11}
    dmb                                     /* ensure ordering with previous memory accesses */
    mrc     p15, 1, r0, c0, c0, 1           /* read clidr */
    ands    r3, r0, #0x7000000              /* extract loc from clidr */
    mov     r3, r3, lsr #23                 /* left align loc bit field */
    beq     ci_finished                     /* if loc is 0, then no need to clean */
    mov     r10, #0                         /* start clean at cache level 0 */
ci_loop1:
    add     r2, r10, r10, lsr #1            /* work out 3x current cache level */
    mov     r1, r0, lsr r2                  /* extract cache type bits from clidr */
    and     r1, r1, #7                      /* mask of the bits for current cache only */
    cmp     r1, #2                          /* see what cache we have at this level */
    blt     ci_skip                         /* skip if no cache, or just i-cache */
    mcr     p15, 2, r10, c0, c0, 0          /* select current cache level in cssr */
    isb                                     /* isb to sych the new cssr&csidr */
    mrc     p15, 1, r1, c0, c0, 0           /* read the new csidr */
    and     r2, r1, #7                      /* extract the length of the cache lines */
    add     r2, r2, #4                      /* add 4 (line length offset) */
    ldr     r4, =0x3ff
    ands    r4, r4, r1, lsr #3              /* find maximum number on the way size */
    clz     r5, r4                          /* find bit position of way size increment */
    ldr     r7, =0x7fff
    ands    r7, r7, r1, lsr #13             /* extract max number of the index size */
ci_loop2:
    mov     r9, r4                          /* create working copy of max way size */
ci_loop3:
    orr     r11, r10, r9, lsl r5            /* factor way and cache number into r11 */
    orr     r11, r11, r7, lsl r2            /* factor index number into r11 */
    mcr     p15, 0, r11, c7, c14, 2         /* clean & invalidate by set/way */
    subs    r9, r9, #1                      /* decrement the way */
    bge     ci_loop3
    subs    r7, r7, #1                      /* decrement the index */
    bge     ci_loop2
ci_skip:
    add     r10, r10, #2                    /* increment cache number */
    cmp     r3, r10
    bgt     ci_loop1
ci_finished:
    mov     r10, #0                         /* swith back to cache level 0 */
    mcr     p15, 2, r10, c0, c0, 0          /* select current cache level in cssr */
    dsb
    isb
    pop     {r4,r5,r7,r9,r10,r11}
    bx      lr