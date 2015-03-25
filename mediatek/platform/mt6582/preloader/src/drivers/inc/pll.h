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

#ifndef PLL_H
#define PLL_H

/* APMIXEDSYS Register */
#define AP_PLL_CON0             (0x10209000)
#define AP_PLL_CON1             (0x10209004)
#define AP_PLL_CON2             (0x10209008)
#define AP_PLL_CON3             (0x1020900C)
#define AP_PLL_CON4             (0x10209010)
#define PLL_HP_CON0             (0x10209014)
#define CLKSQ_STB_CON0          (0x10209018)
#define PLL_ISO_CON0            (0x10209024)
#define PLL_TEST_CON0           (0x10209038)
#define ARMPLL_CON0             (0x10209200)
#define ARMPLL_CON1             (0x10209204)
#define ARMPLL_PWR_CON0         (0x1020920C)
#define MAINPLL_CON0            (0x10209210)
#define MAINPLL_CON1            (0x10209214)
#define MAINPLL_PWR_CON0        (0x1020921C)
#define UNIVPLL_CON0            (0x10209220)
#define UNIVPLL_CON1            (0x10209224)
#define UNIVPLL_PWR_CON0        (0x1020922C)
#define MMPLL_CON0              (0x10209230)
#define MMPLL_CON1              (0x10209234)
#define MMPLL_PWR_CON0          (0x1020923C)
#define MSDCPLL_CON0            (0x10209240)
#define MSDCPLL_CON1            (0x10209244)
#define MSDCPLL_PWR_CON0        (0x1020924C)
#define AP_AUXADC_CON0          (0x10209400)
#define AP_AUXADC_CON1          (0x10209404)
#define TS_CON0                 (0x10209600)
#define TS_CON1                 (0x10209604)
#define AP_ABIST_MON_CON0       (0x10209E00)
#define AP_ABIST_MON_CON1       (0x10209E04)
#define AP_ABIST_MON_CON2       (0x10209E08)
#define AP_ABIST_MON_CON3       (0x10209E0C)
#define AP_ACIF_WR_PATH_CON0    (0x10209E10)
#define AP_ACIF_WR_PATH_CON1    (0x10209E14)
#define CLKDIV_CON0             (0x10209E1C)

#define VENCPLL_CON0            (0x1000F800)
#define VENCPLL_CON1            (0x1000F804)
#define VENCPLL_PWR_CON0        (0x1000F80C)

/* TOPCKGEN Register */
#define CLK_MODE                (0x10000000)
#define DCM_CFG                 (0x10000004)
#define TST_SEL_0               (0x10000020)
#define TST_SEL_1               (0x10000024)
#define TST_SEL_2               (0x10000028)
#define CLK_CFG_0               (0x10000040)
#define CLK_CFG_1               (0x10000050)
#define CLK_CFG_2               (0x10000060)
#define CLK_CFG_3               (0x10000070)
#define CLK_CFG_4               (0x10000080)
#define CLK_CFG_8               (0x10000100)
#define CLK_CFG_9               (0x10000104)
#define CLK_CFG_10              (0x10000108)
#define CLK_CFG_11              (0x1000010C)
#define CLK_SCP_CFG_0           (0x10000200)
#define CLK_SCP_CFG_1           (0x10000204)
#define CLK_MISC_CFG_0          (0x10000210)
#define CLK_MISC_CFG_1          (0x10000214)
#define CLK26CALI_0             (0x10000220)
#define CLK26CALI_1             (0x10000224)
#define CLK26CALI_2             (0x10000228)
#define CKSTA_REG               (0x1000022C)
#define TEST_MODE_CFG           (0x10000230)
#define MBIST_CFG_0             (0x10000308)
#define MBIST_CFG_1             (0x1000030C)
#define MBIST_CFG_2             (0x10000310)
#define MBIST_CFG_3             (0x10000314)

/* INFRASYS Register */
#define TOP_CKMUXSEL            (0x10001000)
#define TOP_CKDIV1              (0x10001008)
#define TOP_DCMCTL              (0x10001010)
#define TOP_DCMDBC              (0x10001014)

/* MCUSS Register */
#define ACLKEN_DIV              (0x10200060)

/* MEMPLL Configuration */
#define MEMPLL_CLK_533

/* DISP Register*/
#define DISP_CG_CON0        (0x14000100)
#define DISP_CG_SET0        (0x14000104)
#define DISP_CG_CLR0        (0x14000108)
#define DISP_CG_CON1        (0x14000110)
#define DISP_CG_SET1        (0x14000114)
#define DISP_CG_CLR1        (0x14000118)


#if 0
#define DRAMC_ASYNC
#endif

#if 1
#define DDRPHY_3PLL_MODE
#endif

#ifdef DDRPHY_3PLL_MODE
#if 1
#define DDRPHY_2PLL
#endif
#endif

#if 0
#define DDR_533
#endif
#if 0
#define DDR_800
#endif
#if 0
#define DDR_912
#endif
#if 0
#define DDR_1000
#endif
#if 0
#define DDR_1110
#endif
#if 0
#define DDR_1172
#endif
#endif


/* for MTCMOS */
#define STA_POWER_DOWN  0
#define STA_POWER_ON    1

#define VDE_PWR_STA_MASK    (0x1 << 7)
#define IFR_PWR_STA_MASK    (0x1 << 6)
#define ISP_PWR_STA_MASK    (0x1 << 5)
#define DIS_PWR_STA_MASK    (0x1 << 3)
#define MFG_PWR_STA_MASK    (0x1 << 4)
#define DPY_PWR_STA_MASK    (0x1 << 2)
#define CONN_PWR_STA_MASK    (0x1 << 1)
#define MD1_PWR_STA_MASK    (0x1 << 0)

#define PWR_RST_B           (0x1 << 0)
#define PWR_ISO             (0x1 << 1)
#define PWR_ON              (0x1 << 2)
#define PWR_ON_S            (0x1 << 3)
#define PWR_CLK_DIS         (0x1 << 4)

#define SRAM_PDN            (0xf << 8)

#define MD_SRAM_PDN         (0x1 << 8)

#define VDE_SRAM_ACK        (0x1 << 12)
#define IFR_SRAM_ACK        (0xf << 12)
#define ISP_SRAM_ACK        (0x3 << 12)
#define DIS_SRAM_ACK        (0xf << 12)
#define MFG_SRAM_ACK        (0x1 << 12)

#define TOPAXI_PROT_EN      (INFRACFG_BASE + 0x0220)
#define TOPAXI_PROT_STA1    (INFRACFG_BASE + 0x0228)

#define MD1_PROT_MASK   0x00B8
#define CONN_PROT_MASK   0x0104

int spm_mtcmos_ctrl_disp(int state);
