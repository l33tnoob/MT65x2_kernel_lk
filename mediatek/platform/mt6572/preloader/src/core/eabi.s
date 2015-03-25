divend        .req    r0
divsor        .req    r1
res           .req    r2
curb          .req    r3

        .text
        .globl   __udivsi3
        .type   __udivsi3 ,function
        .globl  __aeabi_uidiv
        .type   __aeabi_uidiv ,function
        .align  0

 __udivsi3:

 __aeabi_uidiv:
        cmp     divsor, #0
        beq     LD0
        mov     curb, #1
        mov     res, #0
        cmp     divend, divsor
        bcc     Lgot_res

L1:
        cmp     divsor, #0x10000000
        cmpcc   divsor, divend
        movcc   divsor, divsor, lsl #4
        movcc   curb, curb, lsl #4
        bcc     L1

LB:
        cmp     divsor, #0x80000000
        cmpcc   divsor, divend
        movcc   divsor, divsor, lsl #1
        movcc   curb, curb, lsl #1
        bcc     LB

L3:

        cmp     divend, divsor
        subcs   divend, divend, divsor
        orrcs   res, res, curb
        cmp     divend, divsor, lsr #1
        subcs   divend, divend, divsor, lsr #1
        orrcs   res, res, curb, lsr #1
        cmp     divend, divsor, lsr #2
        subcs   divend, divend, divsor, lsr #2
        orrcs   res, res, curb, lsr #2
        cmp     divend, divsor, lsr #3
        subcs   divend, divend, divsor, lsr #3
        orrcs   res, res, curb, lsr #3
        cmp     divend, #0                   
        movnes  curb, curb, lsr #4          
        movne   divsor, divsor, lsr #4
        bne     L3

Lgot_res:
        mov     r0, res
        mov     pc, lr

LD0:
        str     lr, [sp, #-4]!
        bl       __div0       (PLT)
        mov     r0, #0                  
        ldmia   sp!, {pc}
        .size  __udivsi3       , . -  __udivsi3

.globl __aeabi_uidivmod
__aeabi_uidivmod:
        stmfd   sp!, {r0, r1, ip, lr}
        bl      __aeabi_uidiv
        ldmfd   sp!, {r1, r2, ip, lr}
        mul     r3, r0, r2
        sub     r1, r1, r3
        mov     pc, lr

.globl __aeabi_idivmod

__aeabi_idivmod:

        stmfd   sp!, {r0, r1, ip, lr}
        bl      __aeabi_idiv
        ldmfd   sp!, {r1, r2, ip, lr}
        mul     r3, r0, r2
        sub     r1, r1, r3
	mov     pc, lr

