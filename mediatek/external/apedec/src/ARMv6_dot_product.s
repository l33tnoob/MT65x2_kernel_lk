x RN 0 ; input array x[]
c RN 1 ; input array c[]
N RN 2 ; number of samples (a multiple of 16)
acc  RN 3 ;@ accumulator
x_0  RN 4 ;@ elements from array x[]
x_1  RN 5
x_2  RN 6
x_3  RN 7 
c_0  RN 14 ;@ elements from array c[]
c_1  RN 8
c_2  RN 9
c_3  RN 10
acc_2  RN 11


    MACRO
    PROCESS_EIGHT_SAMPLES
    ldmia x!,{x_0,x_1}
    LDMIA c!,{r8,r9}       
    LDMIA x!,{x_2,x_3}   
    LDMIA c!,{r10,r14}    
    SMLAD acc,x_0, r8, acc
    SMLAD acc,x_1, r9, acc  
    SMLAD acc,x_2, r10, acc  
    SMLAD acc,x_3, r14, acc    
    MEND
    
	AREA |s1.text|, CODE, READONLY
    EXPORT dot_aligned_16by16_arm9m
    ; int dot_aligned_16by16_arm9m *x, short *c, unsigned N)
dot_aligned_16by16_arm9m
    STMFD sp!, {r4-r10, lr}    
    MOV acc, #0   
loop_aligned   ; @ accumulate 16 products 
    SUBS N, N, #16        
    PROCESS_EIGHT_SAMPLES
    PROCESS_EIGHT_SAMPLES           
    BGT loop_aligned
    MOV r0, acc
    LDMFD sp!, {r4-r10, pc}


    AREA |s2.text|, CODE, READONLY
    EXPORT dot_16by16_arm9m

    ; int dot_16by16_arm9m(short *x, short *c, unsigned N)
dot_16by16_arm9m
    STMFD sp!, {r4-r11, lr}
    LDRSH c_0, [c], #2    
   ; and  c,c,#0x0FC
    MOV   acc, #0               
    ;LDMFD sp!, {r4-r11, pc}       
    MOV   acc_2,#0
    ROR   c_0,c_0,#16  
loop_9m_1 ; accumulate 16 products           
    ; first c_0 = c[0]
    ; c_1 = c[2] c[1] , x_0 = x[1] x[0]
    ; c_2 = c[4] c[3] , x_1 = x[3] x[2]
    ; c_3 = c[6] c[5] , x_2 = x[5] x[4]
    ; c_0 = c[0] c[7] , x_3 = x[7] x[6]    
    ldmia x!,{x_0,x_1}    
    ldmia  c!,{c_1,c_2}    
    ;ldr   c_1,[c],#4
    ;ldr   c_2,[c],#4
    ldmia  x!,{x_2,x_3}    
    SUBS N, N, #16  
    SMLABT acc,x_0,c_0,acc    ; x[0]*c[0]      
    SMLATB acc_2,x_0,c_1,acc_2    ; x[1]*c[1]        
    SMLABT acc,x_1,c_1,acc    ; x[2]*c[2]
    ldmia  c!,{c_3,c_0}
    ;ldr   c_3,[c],#4
    ;ldr   c_0,[c],#4  
    SMLATB acc,x_1,c_2,acc    ; x[3]*c[3]  
    SMLABT acc_2,x_2,c_2,acc_2    ; x[4]*c[4] 
    SMLATB acc,x_2,c_3,acc    ; x[5]*c[5] 
    ldmia  x!,{x_0,x_1}            
    SMLABT acc,x_3,c_3,acc    ; x[6]*c[6]
    ldmia  c!,{c_1,c_2}
    ;ldr   c_1,[c],#4
    ;ldr   c_2,[c],#4  
    SMLATB acc,x_3,c_0,acc    ; x[7]*c[7]
        
    SMLABT acc_2,x_0,c_0,acc_2    ; x[0]*c[0]
    SMLATB acc,x_0,c_1,acc    ; x[1]*c[1]
    ldmia  x!,{x_2,x_3}    
    SMLABT acc,x_1,c_1,acc    ; x[2]*c[2]
    ldmia  c!,{c_3,c_0}
    ;ldr   c_3,[c],#4
    ;ldr   c_0,[c],#4  
    SMLATB acc,x_1,c_2,acc    ; x[3]*c[3]  
    SMLABT acc_2,x_2,c_2,acc_2    ; x[4]*c[4] 
    SMLATB acc,x_2,c_3,acc    ; x[5]*c[5] 
    ;ldmia  x!,{x_0,x_1}            
    SMLABT acc_2,x_3,c_3,acc_2    ; x[6]*c[6]     
    SMLATB acc,x_3,c_0,acc    ; x[7]*c[7]         
    BGT loop_9m_1  
    ADDS acc,acc,acc_2 
    MOV r0, acc
    LDMFD sp!, {r4-r11, pc}

 AREA |s3.text|, CODE, READONLY
    EXPORT dot_16by16_arm9m_ori

    ; int dot_16by16_arm9m(short *x, short *c, unsigned N)
dot_16by16_arm9m_ori
    STMFD sp!, {r4-r5, r8-r9, lr}
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
    LDMFD sp!, {r4-r5, r8-r9, pc}


    END

    