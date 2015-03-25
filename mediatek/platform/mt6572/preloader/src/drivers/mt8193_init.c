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

#include "typedefs.h"
#include "platform.h"
#include "uart.h"
#include "meta.h"
#include "mt8193.h"
#include "mt_i2c.h"


int mt8193_pllgp_en()
{
    printf("mt8193_pllgp_en() enter\n");

    /* PLL1 setup of 75MHZ. AD_PLLGP_CLK=450MHZ*/

    CKGEN_WRITE32(REG_RW_PLL_GPANACFG0, 0xAD300982);

    CKGEN_WRITE32(REG_RW_PLL_GPANACFG3, 0x80008000);

    CKGEN_WRITE32(REG_RW_PLL_GPANACFG2, 0x2500000);

    printf("mt8193_pllgp_en() exit\n");

    return 0;
}

int mt8193_vopll_en()
{
    printf("mt8193_pllgp_en() enter\n");

    /* PLL2 setup of 75MHZ. AD_PLLGP_CLK=450MHZ*/

    IO_WRITE32(0, 0x44c, 0x1);

    // CKGEN_WRITE32(REG_RW_PLL_GPANACFG0, 0xAD300982);

    CKGEN_WRITE32(REG_RW_LVDS_ANACFG2, 0x32215000);

    CKGEN_WRITE32(REG_RW_LVDS_ANACFG3, 0x410c0);

    CKGEN_WRITE32(REG_RW_LVDS_ANACFG4, 0x300);

    printf("mt8193_pllgp_en() exit\n");

    return 0;
}

int mt8193_i2c_init(void)
{   
    u32 ret_code;
    
    printf("mt8193_i2c_init() enter\n");    

    /* Sset I2C speed mode */
    ret_code = mt_i2c_set_speed(I2C2, I2C_CLK_RATE, ST_MODE, MAX_ST_MODE_SPEED);
    if( ret_code !=  I2C_OK)
    {
        printf("[mt8193_i2c_init] mt_i2c_set_speed error (%d)\n", ret_code);
        return ret_code;
    }

    printf("mt8193_i2c_init() exit\n"); 

    return (0);
}



int mt8193_init(void)
{
    printf("mt8193_init() enter\n");

	u32 u4Tmp = 0;

    mt8193_i2c_init();

	
	u4Tmp = CKGEN_READ32(REG_RW_LVDSWRAP_CTRL1);
	u4Tmp |= (CKGEN_LVDSWRAP_CTRL1_NFIPLL_MON_EN | CKGEN_LVDSWRAP_CTRL1_DCXO_POR_MON_EN);
	CKGEN_WRITE32(REG_RW_LVDSWRAP_CTRL1, u4Tmp);
	  
	/* close pad_int trapping function*/
	u4Tmp = 0x0;
	CKGEN_WRITE32(REG_RW_PMUX7, u4Tmp);


#if 0
    /*  dcxo enable */
	u4Tmp = CKGEN_READ32(REG_RW_CKMISC_CTRL);
	u4Tmp &= (~CKGEN_CKMISC_CTRL_DCXO_MODE_EN);
	CKGEN_WRITE32(REG_RW_CKMISC_CTRL, u4Tmp);
#endif
	  
    mt8193_pllgp_en();

    mt8193_vopll_en();
      
	printf("mt8193_init() exit\n");
	  

    return (0);
}

