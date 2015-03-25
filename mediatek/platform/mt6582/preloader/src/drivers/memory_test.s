/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


     
                 
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.globl store_8word
store_8word:

   STMDB   sp!, {r4,r5,r6,r7,r8,r9}
   MVN r3,r1
   ADD r4, r1, r1
   ADD r5, r3, r3
   ADD r6, r4, r4
   ADD r7, r5, r5
   ADD r8, r6, r6
   ADD r9, r7, r7
   STMIA   r0, {r1,r3,r4,r5,r6,r7,r8,r9}   

StoreEnd:
   LDMIA   sp!, {r4,r5,r6,r7,r8,r9}
;@  BX lr
   mov	pc,	lr


;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.globl load_8word   
load_8word:

   STMDB   sp!, {r4-r12}
   MOV r12,#0
   MVN r3,r1
   LDMIA   r0, {r4,r5,r6,r7,r8,r9,r10,r11}   
   CMP r4, r1
   MOVNE r12, #9
   BNE LoadEnd
   CMP r5, r3
   MOVNE r12, #10
   BNE LoadEnd
   ADD r1, r1, r1
   CMP r6, r1
   MOVNE r12, #11
   BNE LoadEnd
   ADD r3, r3, r3
   CMP r7, r3
   MOVNE r12, #12
   BNE LoadEnd
   
   ADD r1, r1, r1
   CMP r8, r1
   MOVNE r12, #13
   BNE LoadEnd
   ADD r3, r3, r3
   CMP r9, r3
   MOVNE r12, #14
   BNE LoadEnd
   ADD r1, r1, r1
   CMP r10, r1
   MOVNE r12, #15
   BNE LoadEnd
   ADD r3, r3, r3
   CMP r11, r3
   MOVNE r12, #16
   
LoadEnd:
   mov r0, r12
   LDMIA   sp!, {r4-r12}
;@   BX lr
   mov	pc,	lr
   