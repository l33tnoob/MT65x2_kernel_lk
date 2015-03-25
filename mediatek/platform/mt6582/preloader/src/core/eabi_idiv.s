.macro ARMDIV divend, div, res, curb


        tst     \div, #0xe0000000
        moveq   \div, \div, lsl #3
        moveq   \curb, #8
        movne   \curb, #1

1:      cmp     \div, #0x10000000
        cmplo   \div, \divend
        movlo   \div, \div, lsl #4
        movlo   \curb, \curb, lsl #4
        blo     1b

1:      cmp     \div, #0x80000000
        cmplo   \div, \divend
        movlo   \div, \div, lsl #1
        movlo   \curb, \curb, lsl #1
        blo     1b
        mov     \res, #0

1:      cmp     \divend, \div
        subhs   \divend, \divend, \div
        orrhs   \res,   \res,   \curb
        cmp     \divend, \div,  lsr #1
        subhs   \divend, \divend, \div, lsr #1
        orrhs   \res,   \res,   \curb,  lsr #1
        cmp     \divend, \div,  lsr #2
        subhs   \divend, \divend, \div, lsr #2
        orrhs   \res,   \res,   \curb,  lsr #2
        cmp     \divend, \div,  lsr #3
        subhs   \divend, \divend, \div, lsr #3
        orrhs   \res,   \res,   \curb,  lsr #3
        cmp     \divend, #0                   
        movnes  \curb,   \curb,  lsr #4     
        movne   \div,  \div, lsr #4
        bne     1b
.endm
.macro ARMDIV2 div, order

        cmp     \div, #(1 << 16)
        movhs   \div, \div, lsr #16
        movhs   \order, #16
        movlo   \order, #0
        cmp     \div, #(1 << 8)
        movhs   \div, \div, lsr #8
        addhs   \order, \order, #8
        cmp     \div, #(1 << 4)
        movhs   \div, \div, lsr #4
        addhs   \order, \order, #4
        cmp     \div, #(1 << 2)
        addhi   \order, \order, #3
        addls   \order, \order, \div, lsr #1
.endm
        .align  5

.globl __divsi3

.globl __aeabi_idiv

__divsi3:

__aeabi_idiv:
        cmp     r1, #0
        eor     ip, r0, r1
        beq     Ldiv0
        rsbmi   r1, r1, #0
        subs    r2, r1, #1
        beq     10f
        movs    r3, r0
        rsbmi   r3, r0, #0
        cmp     r3, r1
        bls     11f
        tst     r1, r2    
        beq     12f
        ARMDIV r3, r1, r0, r2
        cmp     ip, #0
        rsbmi   r0, r0, #0
        mov     pc, lr
10:     teq     ip, r0    
        rsbmi   r0, r0, #0
        mov     pc, lr
11:     movlo   r0, #0
        moveq   r0, ip, asr #31
        orreq   r0, r0, #1
        mov     pc, lr
12:     ARMDIV2 r1, r2
        cmp     ip, #0
        mov     r0, r3, lsr r2
        rsbmi   r0, r0, #0
        mov     pc, lr

Ldiv0:
        str     lr, [sp, #-4]!
        bl      __div0
        mov     r0, #0    
        ldr     pc, [sp], #4

