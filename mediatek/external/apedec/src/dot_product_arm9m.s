x RN 0 ; input array x[]
c RN 1 ; input array c[]
N RN 2 ; number of samples (a multiple of 16)
acc RN 3 ; accumulator
x_0 RN 4 ; elements from array x[]
x_1 RN 5
c_0 RN 9 ; elements from array c[]
c_1 RN 10

    AREA |.text|, CODE, READONLY
    EXPORT dot_16by16_arm9m

    ; int dot_16by16_arm9m(short *x, short *c, unsigned N)
dot_16by16_arm9m
    STMFD sp!, {r4-r5, r9-r10, lr}
    MOV acc, #0
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
loop_9m ; accumulate 16 products
    SUBS N, N, #16
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRSH x_0, [x], #2
    LDRSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    LDRSH x_1, [x], #2
    LDRSH c_1, [c], #2
    MLA acc, x_0, c_0, acc
    LDRGTSH x_0, [x], #2
    LDRGTSH c_0, [c], #2
    MLA acc, x_1, c_1, acc
    BGT loop_9m
    MOV r0, acc
    LDMFD sp!, {r4-r5, r9-r10, pc}

    END
