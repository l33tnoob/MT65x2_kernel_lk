x RN 0 ; input array x[]
c RN 1 ; input array c[]
v RN 2 ; input array v[]
acc RN 3 ;
xx RN 4 ;
cc RN 5 ;
v_0 RN 6 ;
v_1 RN 12 ;
cnt RN 7;
xxx RN 8;
ccc RN 14;

    MACRO
    PROCESS_FOUR_SAMPLES
    ldmia v!,{v_0,v_1}
    ldmia x,{xx,xxx}    
    ldmia c!,{cc,ccc}        
    sadd16 v_0, v_0, xx
    sadd16 v_1, v_1, xxx
    stmia x!,{v_0,v_1}
    smlad acc, xx, cc, acc
    smlad acc,xxx,ccc, acc    
    ;ori
    ;ldr v_0,[v], #4
    ;ldr xx, [x], #4
    ;ldr cc, [c], #4 
    ;ldr v_1,[v], #4
    ;ldr xxx,[x], #4
    ;ldr ccc,[c], #4
    ;sadd16 v_0, v_0, xx
    ;sadd16 v_1, v_1, xxx    
    ;smlad acc, xx, cc, acc
    ;str v_0, [x, #-8]
    ;smlad acc,xxx,ccc, acc
    ;str v_1, [x, #-4]
    MEND

    AREA |s1.text|, CODE, READONLY
    EXPORT aligned_dot_and_add_16
    ; int aligned_dot_and_add_16(short *x, short *c, short *v)
aligned_dot_and_add_16
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0

    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
    PROCESS_FOUR_SAMPLES
        
    MOV r0, acc
    LDMFD sp!, {r4-r8, pc}

	AREA |s2.text|, CODE, READONLY
    EXPORT aligned_dot_and_add_32
    ; int aligned_dot_and_add_32(short *x, short *c, short *v)
aligned_dot_and_add_32
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

	AREA |s3.text|, CODE, READONLY
    EXPORT aligned_dot_and_add_64
    ; int aligned_dot_and_add_64(short *x, short *c, short *v)
aligned_dot_and_add_64
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

	AREA |s4.text|, CODE, READONLY
    EXPORT aligned_dot_and_add_256
    ; int aligned_dot_and_add_256(short *x, short *c, short *v)
aligned_dot_and_add_256
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0
    MOV cnt, #4
    
loop_256
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

	AREA |s5.text|, CODE, READONLY
    EXPORT aligned_dot_and_add_1280
    ; int aligned_dot_and_add_1280(short *x, short *c, short *v)
aligned_dot_and_add_1280
    STMFD sp!, {r4-r8, lr}
    MOV acc, #0
    MOV cnt, #20
    
loop
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

    END
