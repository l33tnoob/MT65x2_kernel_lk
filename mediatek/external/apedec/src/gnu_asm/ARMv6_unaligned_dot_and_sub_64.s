    .text

    x .req r0 @ input array x[]
    c .req r1 @ input array c[]
    v .req r2 @ input array v[]
    @cnt  .req r3@
    acc  .req r4@
    c_0  .req r5@
    c_1  .req r6@
    @v_0  .req r7@
    @v_1  .req r8@
    x_0  .req r9@
    x_1  .req r10@
    cc   .req r11@
    vv   .req r12@
    vv_1 .req r14@
    @===mark if not eight sample
   cnt  .req r11@
   c_2  .req r11@
   c_3  .req r12@
   v_0  .req r3@
   v_1  .req r7@
   v_2  .req r8@
   v_3  .req r14@

    @sub c, c, #2
    @sub v, v, #2
    @ldr c_0, [c], #4
    @ldr v_0, [v], #4

    .macro PROCESS_FOUR_SAMPLES
    ldr c_1, [c], #4
    ldr v_1, [v], #4        
    ldmia x,{x_0,x_1}   
    pkhtb cc, c_0, c_1
    pkhtb vv, v_0, v_1
    ldr c_0, [c], #4
    ldr v_0, [v], #4
    smladx acc, x_0, cc, acc
    ror vv, vv, #16
    ssub16 vv, x_0, vv    
    pkhtb cc, c_1, c_0
    pkhtb vv_1, v_1, v_0
    smladx acc, x_1, cc, acc
    ror vv_1, vv_1, #16
    ssub16 vv_1, x_1, vv_1
    stmia x!,{vv,vv_1}
    .endm

 
    .macro PROCESS_EIGHT_SAMPLES    
    @ c_0 = c[0]xx, c_1 = x[2]c[1]
    @ c_v = v[0]xx, v_1 = v[2]v[1]   
    ldmia  v!,{v_2,v_3}         @ v_2 = v[4]v[3] , v_3 =v[6]v[5]
    ldmia  c!,{c_2,c_3}         @ c_2 = c[4]c[3] , c_3 =c[6]c[5]
    ldmia  x,{x_0,x_1}          @ x_0 = x[1]x[0] , x_1 = x[3]x[2]
    ror    c_1,c_1,#16          @ c_1 = c[1]c[2] 
    ror    v_1,v_1,#16          @ v_1 = v[1]v[2]    
    pkhtb  c_0,c_1,c_0,asr #16  @ c_0 = c[1]c[0]
    pkhtb  v_0,v_1,v_0,asr #16  @ v_0 = v[1]v[0]   
    smlad acc,x_0,c_0,acc       @ acc = acc + x[1]*c[1]+ x[0]*c[0]
    ssub16 x_0,x_0,v_0          @ x_0 = x[1]+v[1], x[0]+v[0] 
    pkhbt  c_1,c_1,c_2,lsl #16  @ c_1 = c[3]c[2]  
    pkhbt  v_1,v_1,v_2,lsl #16  @ v_1 = v[3]v[2]
    smlad  acc,x_1,c_1,acc      @ acc = acc + x[3]*c[3]+ x[2]*c[2]
    ssub16 x_1,x_1,v_1          @ x_1 = x[3]+v[3], x[2]+v[2]
    stmia  x!,{x_0,x_1}         @ store x[0]=x[0]+v[0] ~x[3]=x[3]+v[3] 
    ldmia  v!,{v_0,v_1}         @ v_0 = v[8]v[7],v_1 = v[10]v[9] 
    ldmia  c!,{c_0,c_1}         @ c_0 = c[8]c[7],c_1 = c[10]c[9] 
    ldmia  x,{x_0,x_1}          @ x_0 = x[5]x[4] ,x_1=x[7]x[6]
    ror    c_3,c_3,#16          @ c_3 = c[5]c[6] 
    ror    v_3,v_3,#16          @ v_3 = v[5]v[6]       
    pkhtb  c_2,c_3,c_2,asr #16  @ c_2 = c[5]c[4]
    pkhtb  v_2,v_3,v_2,asr #16  @ v_2 = v[5]v[4]
    smlad acc,x_0,c_2,acc       @ acc = acc + x[5]*c[5] + x[4]*c[4]
    ssub16 x_0,x_0,v_2          @ x_0 = x[5]+v[5], x[4]+v[4]
    pkhbt  c_3,c_3,c_0,lsl #16  @ c_3 = c[7]c[6]  
    pkhbt  v_3,v_3,v_0,lsl #16  @ v_3 = v[7]v[6]
    smlad acc,x_1,c_3,acc       @ acc = acc + x[7]*c[7] + x[6]*c[6]
    ssub16 x_1,x_1,v_3          @ x_1 = x[7]+v[7], x[6]+v[6]
    stmia  x!,{x_0,x_1}         @ store x[4]=x[4]+v[4]~x[4]=x[7]+v[7]
    .endm

    .global unaligned_dot_and_sub_16
    @ int unaligned_dot_and_sub_16(short *x, short *c, short *v)
unaligned_dot_and_sub_16:
    STMFD sp!, {r4-r12, lr}
    MOV acc, #0
    @SUB c, c, #2
    @SUB v, v, #2
    @ldr c_0, [c], #4
    @ldr v_0, [v], #4  
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES

    SUB    v,v,#2
    SUB    c,c,#2  
    ldmia  v!,{v_0,v_1}         @v_0 = v[0]xx   , v_1 =v[2]v[1]
    ldmia  c!,{c_0,c_1}         @ c_0 = c[0]xx   , c_1 =c[2]c[1]
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    MOV r0, acc
    LDMFD sp!, {r4-r12, pc}

    .global unaligned_dot_and_sub_32
    @ int unaligned_dot_and_sub_32(short *x, short *c, short *v)
unaligned_dot_and_sub_32:
    STMFD sp!, {r4-r12, lr}
    MOV acc, #0
   @ SUB c, c, #2
   @ SUB v, v, #2
   @ ldr c_0, [c], #4
   @ ldr v_0, [v], #4
   @
   @ PROCESS_FOUR_SAMPLES
   @ PROCESS_FOUR_SAMPLES
   @ PROCESS_FOUR_SAMPLES
   @ PROCESS_FOUR_SAMPLES
   @
   @ PROCESS_FOUR_SAMPLES
   @ PROCESS_FOUR_SAMPLES
   @ PROCESS_FOUR_SAMPLES
   @ PROCESS_FOUR_SAMPLES

    SUB    v,v,#2
    SUB    c,c,#2  
    ldmia  v!,{v_0,v_1}         @ v_0 = v[0]xx   , v_1 =v[2]v[1]
    ldmia  c!,{c_0,c_1}         @ c_0 = c[0]xx   , c_1 =c[2]c[1]
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r12, pc}

    .global unaligned_dot_and_sub_64
    @ int unaligned_dot_and_sub_64(short *x, short *c, short *v)
unaligned_dot_and_sub_64:
    STMFD sp!, {r4-r12, lr}
    MOV acc, #0
    @SUB c, c, #2
    @SUB v, v, #2
    @ldr c_0, [c], #4
    @ldr v_0, [v], #4
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES

    SUB    v,v,#2
    SUB    c,c,#2  
    ldmia  v!,{v_0,v_1}         @ v_0 = v[0]xx   , v_1 =v[2]v[1]
    ldmia  c!,{c_0,c_1}         @ c_0 = c[0]xx   , c_1 =c[2]c[1]
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES

    MOV r0, acc
    LDMFD sp!, {r4-r12, pc}

    .global unaligned_dot_and_sub_256
    @ int unaligned_dot_and_sub_256(short *x, short *c, short *v)
unaligned_dot_and_sub_256:
    STMFD sp!, {r4-r12, lr}
    MOV acc, #0
    @SUB c, c, #2
    @SUB v, v, #2
    @ldr c_0, [c], #4
    @ldr v_0, [v], #4
    SUB    v,v,#2
    SUB    c,c,#2  
    ldmia  v!,{v_0,v_1}         @ v_0 = v[0]xx   , v_1 =v[2]v[1]
    ldmia  c!,{c_0,c_1}         @ c_0 = c[0]xx   , c_1 =c[2]c[1]
    MOV cnt, #4

loop_256:
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES

    STMFD sp!,{cnt}
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    LDMFD sp!,{cnt}
    subs cnt, cnt, #1
    bne loop_256

    MOV r0, acc
    LDMFD sp!, {r4-r12, pc}

    .global unaligned_dot_and_sub_1280
    @ int unaligned_dot_and_sub_1280(short *x, short *c, short *v)
unaligned_dot_and_sub_1280:
    STMFD sp!, {r4-r12, lr}
    MOV acc, #0
    @SUB c, c, #2
    @SUB v, v, #2
    @ldr c_0, [c], #4
    @ldr v_0, [v], #4
    SUB  v, v, #2
    SUB    c,c,#2  
    ldmia  v!,{v_0,v_1}         @ v_0 = v[0]xx   , v_1 =v[2]v[1]
    ldmia  c!,{c_0,c_1}         @ c_0 = c[0]xx   , c_1 =c[2]c[1]    
    MOV cnt, #20

loop:
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES
    @PROCESS_FOUR_SAMPLES

    STMFD sp!,{cnt}
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES
    LDMFD sp!,{cnt}

    subs cnt, cnt, #1
    bne loop

    MOV r0, acc
    LDMFD sp!, {r4-r12, pc}
