.section .text.start

.equ MODE_USR       ,0x10
.equ MODE_FIQ       ,0x11
.equ MODE_IRQ       ,0x12
.equ MODE_SVC       ,0x13
.equ MODE_MON       ,0x16
.equ MODE_ABT       ,0x17
.equ MODE_UNDEF     ,0x1B
.equ MODE_SYS       ,0x1F
.equ I_BIT          ,0x80
.equ F_BIT          ,0x40
.equ INT_BIT        ,0xC0

.extern sys_stack
.extern sys_stack_sz

.globl _start
_start:
    b resethandler
bss_start:
    .word _bss_start
bss_end:
    .word _bss_end
stack:
    .long sys_stack
stacksz:
    .long sys_stack_sz

resethandler:
    MOV r0, #0
    MOV r1, #0
    MOV r2, #0
    MOV r3, #0
    MOV r4, #0
    MOV r5, #0
    MOV r6, #0
    MOV r7, #0
    MOV r8, #0
    MOV r9, #0
    MOV r10, #0
    MOV r11, #0
    MOV r12, #0
    MOV sp, #0
    MOV lr, #0

    /* set the cpu to SVC32 mode */
    MRS	r0,cpsr
    BIC	r0,r0,#0x1f
    ORR	r0,r0,#0xd3
    MSR	cpsr,r0

    /* disable interrupt */
    MRS r0, cpsr
    MOV r1, #INT_BIT
    ORR r0, r0, r1
    MSR cpsr_cxsf, r0

    BL apmcu_disable_dcache
    BL apmcu_dcache_clean_invalidate
    BL apmcu_dsb
    BL apmcu_icache_invalidate
    BL apmcu_disable_icache
    BL apmcu_isb
    BL apmcu_disable_smp

    /* enable I+Z+SMP bits and disable D bit */
    MRC p15, 0, ip, c1, c0, 0
    ORR ip, ip, #0x1840   /* I+Z+SMP bits */
    BIC ip, ip, #0x4      /* C bit */
    MCR p15, 0, ip, c1, c0, 0

clear_bss :
    LDR r0, bss_start  /* find start of bss segment */
    LDR r1, bss_end    /* stop here */
    MOV r2, #0x00000000 /* clear */

    CMP r0, r1
    BEQ setup_stk

    /*  clear loop... */
clbss_l :
    STR r2, [r0]
    ADD r0, r0, #4
    CMP r0, r1
    BNE clbss_l

setup_stk :
    /* setup stack */
    LDR r0, stack
    LDR r1, stacksz

    /* buffer overflow detect pattern */
    LDR r2, =0xDEADBEFF
    STR r2, [r0]

    LDR r1, [r1]
    SUB r1, r1, #0x04
    ADD r1, r0, r1

    MOV sp, r1

entry :
    B   main

.globl jump
jump:
    MOV r4, r1   /* r4 argument */
    MOV r5, r2   /* r5 argument */
    MOV pc, r0   /* jump to addr */

.section .text.armapi
.globl apmcu_icache_invalidate
apmcu_icache_invalidate:
    MOV r0, #0
    MCR p15, 0, r0, c7, c5, 0  /* CHECKME: c5 or c1 */
    BX  lr

.globl apmcu_dcache_clean_invalidate
.align 4
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

.global __switch_to_smp
.equ ACTLR_SMP_BIT ,0x00000040

__switch_to_smp:
    MRC p15,0,r0,c1,c0,1
    ORR r0,r0,#ACTLR_SMP_BIT
    MCR p15,0,r0,c1,c0,1
    BX lr

.equ CA9_ICCIAR, 0x1021200c
.equ CA9_ICCEOIR, 0x10212010

.align 4
OtherCoreHandler:
.global    OtherCoreHandler
    /*
     * The master CPU duplicate the CTP code to SLAVE_CPU1_CTP_START
     * and then activate the slave CPU to run from SLAVE_CPU1_CTP_START.
     * Since the LMA and VMA from the slave CPU's view is different,
     * need to create VMA-to-LMA mapping for the slave CPU.
     */

    mrc p15, 0, r4, c0, c0, 5
    and r4, r4, #0xf
    ldr r0, =CA9_ICCIAR
    ldr r0, [r0]
    lsl r1, r0,#22
    lsr r1, r1,#22
    ldr r0, =CA9_ICCEOIR
    str r1, [r0]
    cmp r4, #1
    beq cpu1_init

    b deadloop

/*
    cmp r4, #2
    beq cpu2_init
    cmp r4, #3
    beq cpu3_init
*/
/*
 *==================================================================
 * CPU1-3 initialize
 *==================================================================
 */

cpu1_init:
    bl __switch_to_smp
    bl apmcu_dcache_clean_invalidate

deadloop:
    nop
    nop
    nop
    nop
    b    deadloop

