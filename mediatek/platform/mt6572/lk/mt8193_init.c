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


#include <platform/mt8193.h>
#include <platform/mt_typedefs.h>
#include <platform/mt_i2c.h>


int mt8193_io_init(void)
{
    printf("lk mt8193_io_init() enter\n");

    u32 u4Tmp = 0;

    /* Modify some pad multi function as function 1*/
 
    u4Tmp = CKGEN_READ32(REG_RW_PMUX1);
    u4Tmp |= (1<<PMUX1_PAD_G0_FUNC);
    u4Tmp |= (1<<PMUX1_PAD_B5_FUNC);
    u4Tmp |= (1<<PMUX1_PAD_B4_FUNC);
    CKGEN_WRITE32(REG_RW_PMUX1, u4Tmp);

    u4Tmp = CKGEN_READ32(REG_RW_PMUX2);
    u4Tmp |= (1<<PMUX2_PAD_B3_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_B2_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_B1_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_B0_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_DE_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_VCLK_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_HSYNC_FUNC);
    u4Tmp |= (1<<PMUX2_PAD_VSYNC_FUNC);
    CKGEN_WRITE32(REG_RW_PMUX2, u4Tmp);
    

    printf("lk mt8193_io_init() exit\n");

    return 0;
}


int mt8193_init(void)
{
	  printf("uboot mt8193_init() enter\n");
	  
      mt8193_io_init();
      
	  printf("uboot mt8193_init() exit\n");
	  
    return (0);
}

