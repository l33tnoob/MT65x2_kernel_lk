/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2011. All rights reserved.
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

#ifndef MT8193_H
#define MT8193_H

#include "mt_typedefs.h"

// #include "platform.h"
#include "mt_i2c.h"

#define CKGEN_BASE                       0x1000



#define REG_RW_BUS_CKCFG                 0x000 
#define CLK_BUS_SEL_XTAL                 0
#define CLK_BUS_SEL_NFIPLL_D2                 1
#define CLK_BUS_SEL_NFIPLL_D3                 2
#define CLK_BUS_SEL_XTAL_D2                 3
#define CLK_BUS_SEL_32K                4
#define CLK_BUS_SEL_PAD_DPI0                 5
#define CLK_BUS_SEL_PAD_DPI1                 6
#define CLK_BUS_SEL_ROSC                 7


#define REG_RW_NFI_CKCFG                 0x008 
#define CLK_NFI_SEL_NFIPLL               0
#define CLK_NFI_SEL_NFIPLL_D2            1
#define CLK_NFI_SEL_NFIPLL_D3            2
#define CLK_NFI_SEL_XTAL_D1              3
#define CLK_PDN_NFI                      (1U<<7) 


#define REG_RW_HDMI_PLL_CKCFG             0x00c
#define CLK_HDMI_PLL_SEL_HDMIPLL               0
#define CLK_HDMI_PLL_SEL_32K                  1
#define CLK_HDMI_PLL_SEL_XTAL_D1            2
#define CLK_HDMI_PLL_SEL_NFIPLL             3
#define CLK_PDN_HDMI_PLL                    (1U<<7) 




#define REG_RW_HDMI_DISP_CKCFG           0x010
#define CLK_HDMI_DISP_SEL_DISP               0
#define CLK_HDMI_DISP_SEL_32K                  1
#define CLK_HDMI_DISP_SEL_XTAL_D1            2
#define CLK_HDMI_DISP_SEL_NFIPLL             3
#define CLK_PDN_HDMI_DISP                      (1U<<7) 


#define REG_RW_LVDS_DISP_CKCFG           0x014
#define CLK_LVDS_DISP_SEL_AD_VPLL_DPIX         0
#define CLK_LVDS_DISP_SEL_32K                  1
#define CLK_LVDS_DISP_SEL_XTAL_D1            2
#define CLK_LVDS_DISP_SEL_NFIPLL             3
#define CLK_PDN_LVDS_DISP                      (1U<<7) 



#define REG_RW_LVDS_CTS_CKCFG            0x018
#define CLK_LVDS_CTS_SEL_AD_VPLL_DPIX         0
#define CLK_LVDS_CTS_SEL_32K                  1
#define CLK_LVDS_CTS_SEL_XTAL_D1            2
#define CLK_LVDS_CTS_SEL_NFIPLL             3
#define CLK_PDN_LVDS_CTS                      (1U<<7)

#define REG_RW_CKMISC_CTRL            0x050
#define CKGEN_CKMISC_CTRL_DCXO_MODE_EN         1


#define REG_RW_PMUX0                 0x200
#define REG_RW_PMUX1                 0x204
#define PMUX1_PAD_G0_FUNC            21
#define PMUX1_PAD_B5_FUNC            24
#define PMUX1_PAD_B4_FUNC            27


#define REG_RW_PMUX2                 0x208
#define PMUX2_PAD_B3_FUNC            0
#define PMUX2_PAD_B2_FUNC            3
#define PMUX2_PAD_B1_FUNC            6
#define PMUX2_PAD_B0_FUNC            9
#define PMUX2_PAD_DE_FUNC            12
#define PMUX2_PAD_VCLK_FUNC            15
#define PMUX2_PAD_HSYNC_FUNC            18
#define PMUX2_PAD_VSYNC_FUNC            21

#define REG_RW_PMUX3                 0x20c
#define REG_RW_PMUX4                 0x210
#define REG_RW_PMUX5                 0x214
#define REG_RW_PMUX6                 0x218
#define REG_RW_PMUX7                 0x21c
#define REG_RW_PMUX8                 0x220
#define REG_RW_PMUX9                 0x224


#define REG_RW_LVDSWRAP_CTRL1                0x254
#define CKGEN_LVDSWRAP_CTRL1_NFIPLL_MON_EN   (1U<<7)
#define CKGEN_LVDSWRAP_CTRL1_DCXO_POR_MON_EN   (1U<<8)

#define REG_RW_GPIO_EN_1             0x128
#define REG_RW_GPIO_OUT_1             0x11c
#define REG_RW_GPIO_IN_1             0x138


#define REG_RW_GPIO_EN_0             0x128        // need get register addr
#define REG_RW_GPIO_OUT_0             0x11c       // need get register addr
#define REG_RW_GPIO_IN_0             0x138       // need get register addr

#define REG_RW_GPIO_EN_2             0x128        // need get register addr
#define REG_RW_GPIO_OUT_2             0x11c       // need get register addr
#define REG_RW_GPIO_IN_2            0x138       // need get register addr


/* PLLGP */
#define REG_RW_PLLGP_ANACFG0              0x34c
#define PLLGP_ANACFG0_PLL1_RESERVED       1
#define PLLGP_ANACFG0_PLL1_NFIPLL_EN      (1U<<1)
#define PLLGP_ANACFG0_PLL1_EN             (1U<<31)

#define REG_RW_PLLGP_ANACFG2              0x354
#define PLLGP_ANACFG2_PLLGP_BIAS_EN       (1U<<20)

#define REG_RW_DCXO_ANACFG9              0x388
#define DCXO_ANACFG9_BUS_CK_SOURCE_SEL_SHIFT   9
#define DCXO_ANACFG9_BUS_CK_SOURCE_SEL_MASK    0x7

/* DCXO */

#define REG_RW_DCXO_ANACFG2              0x308
#define DCXO_ANACFG2_LDO4_EN             (1U<<2)
#define DCXO_ANACFG2_LDO4_MAN_EN         (1U<<3)
#define DCXO_ANACFG2_LDO3_EN             (1U<<4)
#define DCXO_ANACFG2_LDO3_MAN_EN         (1U<<5)
#define DCXO_ANACFG2_LDO2_EN             (1U<<6)
#define DCXO_ANACFG2_LDO2_MAN_EN         (1U<<7)
#define DCXO_ANACFG2_LDO1_EN             (1U<<8)
#define DCXO_ANACFG2_LDO1_MAN_EN         (1U<<9)
#define DCXO_ANACFG2_PO_MAN              (1U<<29)




#define REG_RW_DCXO_ANACFG4              0x370
#define DCXO_ANACFG4_BT_MAN             (1U<<18)
#define DCXO_ANACFG4_EXT2_MAN           (1U<<19)
#define DCXO_ANACFG4_EXT1_MAN           (1U<<20)



#define REG_LVDS_PWR_CTRL (0x10c)
#define REG_LVDS_PWR_RST_B (0x108)

#define REG_HDMI_PWR_CTRL (0x104)
#define REG_HDMI_PWR_RST_B (0x100)

#if 0
int mt8193_i2c_write(u16 addr, u32 data)
{
    
    return 0;
}

u32 mt8193_i2c_read(u16 addr)
{
    return 0;
}

#endif


#define IO_READ8(base, offset)                          mt8193_i2c_read8((base) + (offset))
#define IO_READ16(base, offset)                         mt8193_i2c_read16((base) + (offset))
#define IO_READ32(base, offset)                         mt8193_i2c_read32((base) + (offset))

//============================================================================
// Macros for register write
//============================================================================
extern int mt8193_i2c_write32(u16 addr, u32 data);
extern u32 mt8193_i2c_read32(u16 addr);

#define IO_WRITE8(base, offset, value)                  mt8193_i2c_write8((base) + (offset), (value))
#define IO_WRITE16(base, offset, value)                 mt8193_i2c_write16((base) + (offset), (value))
#define IO_WRITE32(base, offset, value)                 mt8193_i2c_write32((base) + (offset), (value))


#define CKGEN_READ8(offset)            IO_READ8(CKGEN_BASE, (offset))
#define CKGEN_READ16(offset)           IO_READ16(CKGEN_BASE, (offset))
#define CKGEN_READ32(offset)           IO_READ32(CKGEN_BASE, (offset))

#define CKGEN_WRITE8(offset, value)    IO_WRITE8(CKGEN_BASE, (offset), (value))
#define CKGEN_WRITE16(offset, value)   IO_WRITE16(CKGEN_BASE, (offset), (value))
#define CKGEN_WRITE32(offset, value)   IO_WRITE32(CKGEN_BASE, (offset), (value))



/*=======================================================================*/
/* Constant Definitions                                                  */
/*=======================================================================*/

typedef enum
{
    SRC_CK_APLL,
    SRC_CK_ARMPLL,
    SRC_CK_VDPLL,
    SRC_CK_DMPLL,
    SRC_CK_SYSPLL1,
    SRC_CK_SYSPLL2,
    SRC_CK_USBCK,
    SRC_CK_MEMPLL,
    SRC_CK_MCK
} SRC_CK_T;

typedef enum
{
    e_CLK_NFI,             //0   0x70.3
    e_CLK_HDMIPLL,            //1   0x70.7
    e_CLK_HDMIDISP,
    e_CLK_LVDSDISP,
    e_CLK_LVDSCTS,
    e_CLK_MAX              //2
} e_CLK_T;

typedef enum
{
    e_CKEN_NFI,            //0   0x300.31
    e_CKEN_HDMI,          //1   0x300.29
    e_CKEN_MAX               //2
} e_CKEN_T;


#endif // MT8193_H

