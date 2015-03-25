/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "platform.h"

#include "mt6572.h"
#include "mtk_pll.h"
#include "mtk_timer.h"
#include "custom_emi.h"

kal_uint32 mtk_get_bus_freq(void)
{
    kal_uint32 bus_clk = 26000;

    #if !CFG_FPGA_PLATFORM

    /* For MT6572, check CLK_MUX_SEL(CLK_SEL_0) to get such information */

    kal_uint32 mainpll_con1 = 0, main_diff = 0;
    kal_uint32 clk_sel = 0, pre_div = 1, post_div = 0, vco_div = 1;
    float      n_info = 0, output_freq = 0;

    clk_sel = DRV_Reg32(CLK_SEL_0);

    mainpll_con1 = DRV_Reg32(MAINPLL_CON1);

    post_div = (mainpll_con1 >> 24) & 0x07;
    if(post_div > 0x100)
    {
        output_freq = 0;
    }
    else
    {
        post_div = 0x1 << post_div;
        n_info = (mainpll_con1 & 0x3FFF);
        n_info /= (0x1 << 14);
        n_info += ((mainpll_con1 >> 14) & 0x7F);
        output_freq = 26*n_info*vco_div/pre_div/post_div; //Mhz

        clk_sel = (clk_sel >> 5) & 0x7;
        if( 0x2 == clk_sel )
        {
            bus_clk = output_freq*1000/10;
        }
        else if( 0x4 == clk_sel )
        {
            bus_clk = output_freq*1000/12;
        }
        else
        {
            bus_clk = 26*1000;
        }
    }

    #endif

    return bus_clk; // Khz
}

void mtk_pll_post_init(void)
{
    return;
}

#define BIT(_bit_)                  (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BITMASK(_bits_)             (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

int mtk_pll_init_emi(unsigned int freq)
{
    kal_uint32 reg_val = 0;

    switch (freq)
    {
    case 200:
        /* rg_emi2x_gfmux_sel = 0xA: main pll/4 */
        reg_val = DRV_Reg32(CLK_SEL_0) & ~BITMASK(4:1);
        reg_val |= BITS(4:1, 0xA);
        DRV_WriteReg32(CLK_SEL_0, reg_val);
        break;

    case 266:
        /* rg_emi2x_gfmux_sel = 0x9: main pll/3 */
        reg_val = DRV_Reg32(CLK_SEL_0) & ~BITMASK(4:1);
        reg_val |= BITS(4:1, 0x9);
        DRV_WriteReg32(CLK_SEL_0, reg_val);
        break;

    case 333:
        /* adjust main pll frequency for EMI @ 667Mhz */
        // POSDIV: 1, VCO: 1326.0, PLL: 1326.0
        reg_val = 0x800CC000; // | ((DRV_Reg32(MAINPLL_CON0) & 1) << 31);
        DRV_WriteReg32(MAINPLL_CON1, reg_val);

        //5. Wait 100us for ARMPLL, MAINPLL and UNIVPLL settle
        /* wait for 1ms */
        gpt_busy_wait_us(1000);

        reg_val = DRV_Reg32(CLK_SEL_0) & ~(BITMASK(4:1) | BITMASK(7:5));
        /* rg_emi2x_gfmux_sel = 0xC: main pll/2 */
        reg_val |= BITS(4:1, 0xC);
        /* rg_axibus_gfmux_sel = 0x2: main pll/10 */
        reg_val |= BITS(7:5, 0x2);
        DRV_WriteReg32(CLK_SEL_0, reg_val);
        break;

    default:
        return -1;
    }

    return 0;
}

void mtk_pll_init(void)
{
#if !(CFG_FPGA_PLATFORM)

    kal_uint32 reg_val = 0;

    if (reg_val = (DRV_Reg32(0x10009100) >> 4) & 0xF)
    {
        reg_val = DRV_Reg32(0x10205600) & 0xFFFF0FFF | (reg_val << 12);
        DRV_WriteReg32(0x10205600, reg_val);
    }

    if (reg_val = (DRV_Reg32(0x10009100) >> 0) & 0xF)
    {
        reg_val = DRV_Reg32(0x10205600) & 0xFFFFF0FF | (reg_val << 8);
        DRV_WriteReg32(0x10205600, reg_val);
    }

    /* Diable bypass delay */
    reg_val = DRV_Reg32(AP_PLL_CON3);
    reg_val &= 0xFFFFFC00;
    DRV_WriteReg32(AP_PLL_CON3, reg_val);

    reg_val = DRV_Reg32(AP_PLL_CON3);
    reg_val |= 0x80;
    DRV_WriteReg32(AP_PLL_CON3, reg_val);

    /* check if SRAM safe mode is enabled */
    if (0) // (EFUSE_PLL_Safe_IsEnabled()) // TODO: add EFUSE check API for preloader
    {
        DRV_WriteReg32(ARMPLL_CON1, 0x0109A000);
    }
    else
    {
        DRV_WriteReg32(ARMPLL_CON1, 0x01114000);
    }

    /* switch to HW mode */
    reg_val = DRV_Reg32(AP_PLL_CON1);
    reg_val &= 0xF8F8CF8C;
    DRV_WriteReg32(AP_PLL_CON1, reg_val);

    /* wait for 1ms */
    gpt_busy_wait_us(1000);

    /* CPU clock */
    DRV_WriteReg32(ACLKEN_DIV, 0x12); // CPU bus clock is MCU clock /2
    DRV_WriteReg32(PCLKEN_DIV, 0x15); // CPU debug APB bus clock is MCU clokc /5

    DRV_WriteReg32(CLK_SWCG_3, 0x80000000);
    DRV_WriteReg32(CLK_SEL_0, 0x10000480); // rg_axibus_gfmux_sel: main pll/12

    /* clock switch - switch AP MCU clock */
    reg_val = DRV_Reg32(INFRA_TOPCKGEN_CKMUXSEL);
    reg_val |= 0x4;
    DRV_WriteReg32(INFRA_TOPCKGEN_CKMUXSEL, reg_val);

#endif // !defined(CFG_FPGA_PLATFORM)
}

kal_uint32 mtk_get_uart_clock(pll_uart_id uart_num)
{
    #if !CFG_FPGA_PLATFORM

    kal_uint32 reg_val = 0;
    if(uart_num >= NUM_OF_UART)
    {
        return 0;
    }
    else
    {
        reg_val = DRV_Reg32(CLK_SEL_0);
        if(PLL_UART0 == uart_num)
        {
            if(reg_val & 0x1)
            {
                return 52*1000;
            }
            else
            {
                return 26*1000;
            }
        }
        else if(PLL_UART0 == uart_num)
        {
            if(reg_val & 0x80000)
            {
                return 52*1000;
            }
            else
            {
                return 26*1000;
            }
        }
    }
    #else
    return 26*1000;
    #endif /* !CFG_FPGA_PLATFORM */
}

