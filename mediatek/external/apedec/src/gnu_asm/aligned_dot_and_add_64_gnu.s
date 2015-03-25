    .text

    x .req r0 @ input array x[]
    c .req r1 @ input array c[]
    v .req r2 @ input array v[]
    acc .req r4 @
    xx .req r5 @
    cc .req r6 @
    v_0 .req r7 @
    v_1 .req r12 @
    one .req r3 @
    cnt .req r8 @

    .macro PROCESS_TWO_SAMPLES
    LDR xx, [x], #4
    LDR cc, [c], #4
    LDRSH v_0, [v], #2
    LDRSH v_1, [v], #2
    smlabb acc, xx, cc, acc
    smlatt acc, xx, cc, acc
    smlabb v_0, xx, one, v_0
    ADD v_1, v_1, xx, asr #16
    STRH v_0, [x, #-4]
    STRH v_1, [x, #-2]
    .endm

    .global aligned_dot_and_add_16
    @ int aligned_dot_and_add_16(short *x, short *c, short *v)
aligned_dot_and_add_16:
    STMFD sp!, {r4-r7, lr}
    MOV acc, #0
    MOV one, #1

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r7, pc}

    .global aligned_dot_and_add_32
    @ int aligned_dot_and_add_32(short *x, short *c, short *v)
aligned_dot_and_add_32:
    STMFD sp!, {r4-r7, lr}
    MOV acc, #0
    MOV one, #1

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r7, pc}

    .global aligned_dot_and_add_64
    @ int aligned_dot_and_add_64(short *x, short *c, short *v)
aligned_dot_and_add_64:
    STMFD sp!, {r4-r7, lr}
    MOV acc, #0
    MOV one, #1

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r7, pc}

    .global aligned_dot_and_add_256
    @ int aligned_dot_and_add_256(short *x, short *c, short *v)
aligned_dot_and_add_256:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0
    MOV one, #1
    MOV cnt, #4

loop_256:
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    subs cnt, cnt, #1
    bne loop_256

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}

    .global aligned_dot_and_add_1280
    @ int aligned_dot_and_add_1280(short *x, short *c, short *v)
aligned_dot_and_add_1280:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0
    MOV one, #1
    MOV cnt, #20

loop:
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES

    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    PROCESS_TWO_SAMPLES
    subs cnt, cnt, #1
    bne loop

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}
