    .text

    x .req r0 @ input array x[]
    c .req r1 @ input array c[]
    v .req r2 @ input array v[]
    acc .req r3 @
    xx .req r4 @
    cc .req r5 @
    v_0 .req r6 @
    v_1 .req r12 @
    cnt .req r7@
    xxx .req r8@
    ccc .req r14@

    .macro PROCESS_FOUR_SAMPLES
    ldmia v!,{v_0,v_1}
    ldmia x,{xx,xxx}    
    ldmia c!,{cc,ccc}        
    ssub16 v_0, xx,v_0 
    ssub16 v_1,  xxx, v_1
    stmia x!,{v_0,v_1}
    smlad acc, xx, cc, acc
    smlad acc,xxx,ccc, acc  
    .endm

    .global aligned_dot_and_sub_16
    @ int aligned_dot_and_sub_16(short *x, short *c, short *v)
aligned_dot_and_sub_16:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}

    .global aligned_dot_and_sub_32
    @ int aligned_dot_and_sub_32(short *x, short *c, short *v)
aligned_dot_and_sub_32:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}

    .global aligned_dot_and_sub_64
    @ int aligned_dot_and_sub_64(short *x, short *c, short *v)
aligned_dot_and_sub_64:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}

    .global aligned_dot_and_sub_256
    @ int aligned_dot_and_sub_256(short *x, short *c, short *v)
aligned_dot_and_sub_256:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0
    MOV cnt, #4

loop_256:
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    subs cnt, cnt, #1
    bne loop_256

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}

    .global aligned_dot_and_sub_1280
    @ int aligned_dot_and_sub_1280(short *x, short *c, short *v)
aligned_dot_and_sub_1280:
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0
    MOV cnt, #20

loop:
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES

    subs cnt, cnt, #1
    bne loop

    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}
