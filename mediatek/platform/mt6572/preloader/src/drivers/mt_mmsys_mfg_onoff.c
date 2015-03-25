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
 
#include "mt_mmmfg_on_off.h"
#include "typedefs.h"
#include "mt_emi.h"
#include "mt_mmmfg_sram_repair.h"
#include "mt_mmmfg_on_off.h"

#define BIT(_bit_)                  (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BITMASK(_bits_)             (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

#define SPM_BASE                    0x10006000
#define SPM_POWERON_CONFIG_SET      (SPM_BASE + 0x0000)
#define  SPM_POWER_ON_VAL0          (SPM_BASE + 0x0010)
#define SPM_MFG_PWR_CON             (SPM_BASE + 0x0214)
#define SPM_DIS_PWR_CON             (SPM_BASE + 0x023c)
#define SPM_CONN_PWR_CON            (SPM_BASE + 0x0280)
#define SPM_MD_PWR_CON              (SPM_BASE + 0x0284)
#define SPM_PCM_REG13_DATA          (SPM_BASE + 0x03b4)
#define SPM_PWR_STATUS              (SPM_BASE + 0x060c)
#define SPM_PWR_STATUS_S            (SPM_BASE + 0x0610)

#define INFRA_SYS_CFG_AO_BASE        0x10001000
#define INFRACFG_AO_BASE            INFRA_SYS_CFG_AO_BASE
#define TOPAXI_SI0_CTL              (INFRACFG_AO_BASE + 0x0200) // TODO: review it
#define INFRA_TOPAXI_PROTECTEN      (INFRACFG_AO_BASE + 0x0220)
#define INFRA_TOPAXI_PROTECTSTA1    (INFRACFG_AO_BASE + 0x0228)

#define TOPCKGEN_BASE                0x10000000	//MT6572
#define TOP_CLOCK_CTRL_BASE         TOPCKGEN_BASE

#define CLK_MUX_SEL                 (TOP_CLOCK_CTRL_BASE + 0x0000)
#define CLK_GATING_CTRL0            (TOP_CLOCK_CTRL_BASE + 0x0020)
#define CLK_GATING_CTRL1            (TOP_CLOCK_CTRL_BASE + 0x0024)
#define MPLL_FREDIV_EN              (TOP_CLOCK_CTRL_BASE + 0x0030)
#define UPLL_FREDIV_EN              (TOP_CLOCK_CTRL_BASE + 0x0034)
#define SET_CLK_GATING_CTRL0        (TOP_CLOCK_CTRL_BASE + 0x0050)
#define SET_CLK_GATING_CTRL1        (TOP_CLOCK_CTRL_BASE + 0x0054)
#define SET_MPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0060)
#define SET_UPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0064)
#define CLR_CLK_GATING_CTRL0        (TOP_CLOCK_CTRL_BASE + 0x0080)
#define CLR_CLK_GATING_CTRL1        (TOP_CLOCK_CTRL_BASE + 0x0084)
#define CLR_MPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0090)
#define CLR_UPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0094)

#define SPM_PROJECT_CODE            0xb16

#define PWR_RST_B_BIT               BIT(0)          // @ SPM_XXX_PWR_CON
#define PWR_ISO_BIT                 BIT(1)          // @ SPM_XXX_PWR_CON
#define PWR_ON_BIT                  BIT(2)          // @ SPM_XXX_PWR_CON
#define PWR_ON_S_BIT                BIT(3)          // @ SPM_XXX_PWR_CON
#define PWR_CLK_DIS_BIT             BIT(4)          // @ SPM_XXX_PWR_CON
#define SRAM_CKISO_BIT              BIT(5)          // @ SPM_FC0_PWR_CON or SPM_CPU_PWR_CON
#define SRAM_ISOINT_B_BIT           BIT(6)          // @ SPM_FC0_PWR_CON or SPM_CPU_PWR_CON
#define SRAM_PDN_BITS               BITMASK(11:8)   // @ SPM_XXX_PWR_CON
#define SRAM_PDN_ACK_BITS           BITMASK(15:12)  // @ SPM_XXX_PWR_CON

#define DIS_PWR_STA_MASK            BIT(3)
#define MFG_PWR_STA_MASK    BIT(4)

//
// MMSYS_CONFIG
//
#define MMSYS_CONFIG_BASE            0x14000000	//MT6572
#define MMSYS_CG_CON0           (MMSYS_CONFIG_BASE + 0x100)
#define MMSYS_CG_SET0           (MMSYS_CONFIG_BASE + 0x104)
#define MMSYS_CG_CLR0           (MMSYS_CONFIG_BASE + 0x108)
#define MMSYS_CG_CON1           (MMSYS_CONFIG_BASE + 0x110)
#define MMSYS_CG_SET1           (MMSYS_CONFIG_BASE + 0x114)
#define MMSYS_CG_CLR1           (MMSYS_CONFIG_BASE + 0x118)

//
// MFG_CONFIG
//
#define G3D_CONFIG_BASE              0x13000000 //MT6572
#define MFG_CONFIG_BASE         G3D_CONFIG_BASE

#define MFG_CG_CON              (MFG_CONFIG_BASE + 0x0000)
#define MFG_CG_SET              (MFG_CONFIG_BASE + 0x0004)
#define MFG_CG_CLR              (MFG_CONFIG_BASE + 0x0008)

//
// AUDIO_SYS_TOP
//
#define AUDIO_BASE                   0x11140000	//MT6572
#define AUDIO_SYS_TOP_BASE      AUDIO_BASE

#define AUDIO_TOP_CON0          (AUDIO_SYS_TOP_BASE + 0x0000)

// CG_MPLL
#define MPLL_D2_EN_BIT          BIT(0)
#define MPLL_D3_EN_BIT          BIT(1)
#define MPLL_D5_EN_BIT          BIT(2)
#define MPLL_D7_EN_BIT          BIT(3)

#define MPLL_D4_EN_BIT          BIT(8)
#define MPLL_D6_EN_BIT          BIT(9)
#define MPLL_D10_EN_BIT         BIT(10)

#define MPLL_D8_EN_BIT          BIT(16)
#define MPLL_D12_EN_BIT         BIT(17)
#define MPLL_D20_EN_BIT         BIT(18)

#define MPLL_D24_EN_BIT         BIT(24)

// TODO: set @ init is better
#define CG_MPLL_MASK           (MPLL_D2_EN_BIT  \
                              | MPLL_D3_EN_BIT  \
                              | MPLL_D5_EN_BIT  \
                              | MPLL_D7_EN_BIT  \
                              | MPLL_D4_EN_BIT  \
                              | MPLL_D6_EN_BIT  \
                              | MPLL_D10_EN_BIT \
                              | MPLL_D8_EN_BIT  \
                              | MPLL_D12_EN_BIT \
                              | MPLL_D20_EN_BIT \
                              | MPLL_D24_EN_BIT)

// CG_UPLL
#define UPLL_D2_EN_BIT          BIT(0)
#define UPLL_D3_EN_BIT          BIT(1)
#define UPLL_D5_EN_BIT          BIT(2)
#define UPLL_D7_EN_BIT          BIT(3)

#define UPLL_D4_EN_BIT          BIT(8)
#define UPLL_D6_EN_BIT          BIT(9)
#define UPLL_D10_EN_BIT         BIT(10)

#define UPLL_D8_EN_BIT          BIT(16)
#define UPLL_D12_EN_BIT         BIT(17)
#define UPLL_D20_EN_BIT         BIT(18)

#define UPLL_D16_EN_BIT         BIT(24)
#define UPLL_D24_EN_BIT         BIT(25)

// TODO: set @ init is better
#define CG_UPLL_MASK           (UPLL_D2_EN_BIT  \
                              | UPLL_D3_EN_BIT  \
                              | UPLL_D5_EN_BIT  \
                              | UPLL_D7_EN_BIT  \
                              | UPLL_D4_EN_BIT  \
                              | UPLL_D6_EN_BIT  \
                              | UPLL_D10_EN_BIT \
                              | UPLL_D8_EN_BIT  \
                              | UPLL_D12_EN_BIT \
                              | UPLL_D20_EN_BIT \
                              | UPLL_D16_EN_BIT \
                              | UPLL_D24_EN_BIT)

// CG_CTRL0 (clk_swcg_0)
#define PWM_MM_SW_CG_BIT        BIT(0)
#define CAM_MM_SW_CG_BIT        BIT(1)
#define MFG_MM_SW_CG_BIT        BIT(2)
#define SPM_52M_SW_CG_BIT       BIT(3)
#define MIPI_26M_DBG_EN_BIT     BIT(4)
#define DBI_BCLK_SW_CG_BIT      BIT(5)
#define SC_26M_CK_SEL_EN_BIT    BIT(6)
#define SC_MEM_CK_OFF_EN_BIT    BIT(7)

#define DBI_PAD0_SW_CG_BIT      BIT(16)
#define DBI_PAD1_SW_CG_BIT      BIT(17)
#define DBI_PAD2_SW_CG_BIT      BIT(18)
#define DBI_PAD3_SW_CG_BIT      BIT(19)
#define GPU_491P52M_EN_BIT      BIT(20)
#define GPU_500P5M_EN_BIT       BIT(21)

#define ARMDCM_CLKOFF_EN_BIT    BIT(31)

// TODO: set @ init is better
#define CG_CTRL0_MASK          (PWM_MM_SW_CG_BIT        \
                              | CAM_MM_SW_CG_BIT        \
                              | MFG_MM_SW_CG_BIT        \
                              | SPM_52M_SW_CG_BIT       \
                              | MIPI_26M_DBG_EN_BIT     \
                              | DBI_BCLK_SW_CG_BIT      \
                              | SC_26M_CK_SEL_EN_BIT    \
                              | SC_MEM_CK_OFF_EN_BIT    \
                              | DBI_PAD0_SW_CG_BIT      \
                              | DBI_PAD1_SW_CG_BIT      \
                              | DBI_PAD2_SW_CG_BIT      \
                              | DBI_PAD3_SW_CG_BIT      \
                              | GPU_491P52M_EN_BIT      \
                              | GPU_500P5M_EN_BIT       \
                              | ARMDCM_CLKOFF_EN_BIT)

// enable bit @ CG_CTRL0
#define CG_CTRL0_EN_MASK       (MIPI_26M_DBG_EN_BIT     \
                              | SC_26M_CK_SEL_EN_BIT    \
                              | SC_MEM_CK_OFF_EN_BIT    \
                              | GPU_491P52M_EN_BIT      \
                              | GPU_500P5M_EN_BIT       \
                              | ARMDCM_CLKOFF_EN_BIT)

// CG_CTRL1 (rg_peri_sw_cg)
#define EFUSE_SW_CG_BIT         BIT(0)
#define THEM_SW_CG_BIT          BIT(1)
#define APDMA_SW_CG_BIT         BIT(2)
#define I2C0_SW_CG_BIT          BIT(3)
#define I2C1_SW_CG_BIT          BIT(4)
#define AUX_SW_CG_MD_BIT        BIT(5) // XXX: NOT USED @ AP SIDE
#define NFI_SW_CG_BIT           BIT(6)
#define NFIECC_SW_CG_BIT        BIT(7)

#define DEBUGSYS_SW_CG_BIT      BIT(8)
#define PWM_SW_CG_BIT           BIT(9)
#define UART0_SW_CG_BIT         BIT(10)
#define UART1_SW_CG_BIT         BIT(11)
#define BTIF_SW_CG_BIT          BIT(12)
#define USB_SW_CG_BIT           BIT(13)
#define FHCTL_SW_CG_BIT         BIT(14)
#define AUX_SW_CG_THERM_BIT     BIT(15)

#define SPINFI_SW_CG_BIT        BIT(16)
#define MSDC0_SW_CG_BIT         BIT(17)
#define MSDC1_SW_CG_BIT         BIT(18)

#define PMIC_SW_CG_AP_BIT       BIT(20)
#define SEJ_SW_CG_BIT           BIT(21)
#define MEMSLP_DLYER_SW_CG_BIT  BIT(22)

#define APXGPT_SW_CG_BIT        BIT(24)
#define AUDIO_SW_CG_BIT         BIT(25)
#define SPM_SW_CG_BIT           BIT(26)
#define PMIC_SW_CG_MD_BIT       BIT(27) // XXX: NOT USED @ AP SIDE
#define PMIC_SW_CG_CONN_BIT     BIT(28) // XXX: NOT USED @ AP SIDE
#define PMIC_26M_SW_CG_BIT      BIT(29)
#define AUX_SW_CG_ADC_BIT       BIT(30)
#define AUX_SW_CG_TP_BIT        BIT(31)

// TODO: set @ init is better
#define CG_CTRL1_MASK          (EFUSE_SW_CG_BIT         \
                              | THEM_SW_CG_BIT          \
                              | APDMA_SW_CG_BIT         \
                              | I2C0_SW_CG_BIT          \
                              | I2C1_SW_CG_BIT          \
                              | NFI_SW_CG_BIT           \
                              | NFIECC_SW_CG_BIT        \
                              | DEBUGSYS_SW_CG_BIT      \
                              | PWM_SW_CG_BIT           \
                              | UART0_SW_CG_BIT         \
                              | UART1_SW_CG_BIT         \
                              | BTIF_SW_CG_BIT          \
                              | USB_SW_CG_BIT           \
                              | FHCTL_SW_CG_BIT         \
                              | AUX_SW_CG_THERM_BIT     \
                              | SPINFI_SW_CG_BIT        \
                              | MSDC0_SW_CG_BIT         \
                              | MSDC1_SW_CG_BIT         \
                              | PMIC_SW_CG_AP_BIT       \
                              | SEJ_SW_CG_BIT           \
                              | MEMSLP_DLYER_SW_CG_BIT  \
                              | APXGPT_SW_CG_BIT        \
                              | AUDIO_SW_CG_BIT         \
                              | SPM_SW_CG_BIT           \
                              | PMIC_26M_SW_CG_BIT      \
                              | AUX_SW_CG_ADC_BIT       \
                              | AUX_SW_CG_TP_BIT)

// | AUX_SW_CG_MD_BIT        \
// | PMIC_SW_CG_MD_BIT       \
// | PMIC_SW_CG_CONN_BIT     \

// CG_MMSYS0
#define SMI_COMMON_SW_CG_BIT        BIT(0)
#define SMI_LARB0_SW_CG_BIT         BIT(1)
#define MM_CMDQ_SW_CG_BIT           BIT(2)
#define MM_CMDQ_SMI_IF_SW_CG_BIT    BIT(3)
#define DISP_COLOR_SW_CG_BIT        BIT(4)
#define DISP_BLS_SW_CG_BIT          BIT(5)
#define DISP_WDMA_SW_CG_BIT         BIT(6)
#define DISP_RDMA_SW_CG_BIT         BIT(7)
#define DISP_OVL_SW_CG_BIT          BIT(8)
#define MDP_TDSHP_SW_CG_BIT         BIT(9)
#define MDP_WROT_SW_CG_BIT          BIT(10)
#define MDP_WDMA_SW_CG_BIT          BIT(11)
#define MDP_RSZ1_SW_CG_BIT          BIT(12)
#define MDP_RSZ0_SW_CG_BIT          BIT(13)
#define MDP_RDMA_SW_CG_BIT          BIT(14)
#define MDP_BLS_26M_SW_CG_BIT       BIT(15)
#define MM_CAM_SW_CG_BIT            BIT(16)
#define MM_SENINF_SW_CG_BIT         BIT(17)
#define MM_CAMTG_SW_CG_BIT          BIT(18)
#define MM_CODEC_SW_CG_BIT          BIT(19)
#define DISP_FAKE_ENG_SW_CG_BIT     BIT(20)
#define MUTEX_SLOW_CLOCK_SW_CG_BIT  BIT(21)

// TODO: set @ init is better
#define CG_MMSYS0_MASK         (SMI_COMMON_SW_CG_BIT        \
                              | SMI_LARB0_SW_CG_BIT         \
                              | MM_CMDQ_SW_CG_BIT           \
                              | MM_CMDQ_SMI_IF_SW_CG_BIT    \
                              | DISP_COLOR_SW_CG_BIT        \
                              | DISP_BLS_SW_CG_BIT          \
                              | DISP_WDMA_SW_CG_BIT         \
                              | DISP_RDMA_SW_CG_BIT         \
                              | DISP_OVL_SW_CG_BIT          \
                              | MDP_TDSHP_SW_CG_BIT         \
                              | MDP_WROT_SW_CG_BIT          \
                              | MDP_WDMA_SW_CG_BIT          \
                              | MDP_RSZ1_SW_CG_BIT          \
                              | MDP_RSZ0_SW_CG_BIT          \
                              | MDP_RDMA_SW_CG_BIT          \
                              | MDP_BLS_26M_SW_CG_BIT       \
                              | MM_CAM_SW_CG_BIT            \
                              | MM_SENINF_SW_CG_BIT         \
                              | MM_CAMTG_SW_CG_BIT          \
                              | MM_CODEC_SW_CG_BIT          \
                              | DISP_FAKE_ENG_SW_CG_BIT     \
                              | MUTEX_SLOW_CLOCK_SW_CG_BIT)

// CG_MMSYS1
#define DSI_ENGINE_SW_CG_BIT        BIT(0)
#define DSI_DIGITAL_SW_CG_BIT       BIT(1)
#define DISP_DPI_ENGINE_SW_CG_BIT   BIT(2)
#define DISP_DPI_IF_SW_CG_BIT       BIT(3)
#define DISP_DBI_ENGINE_SW_CG_BIT   BIT(4)
#define DISP_DBI_SMI_SW_CG_BIT      BIT(5)
#define DISP_DBI_IF_SW_CG_BIT       BIT(6)

// TODO: set @ init is better
#define CG_MMSYS1_MASK         (DSI_ENGINE_SW_CG_BIT        \
    | DSI_DIGITAL_SW_CG_BIT       \
    | DISP_DPI_ENGINE_SW_CG_BIT   \
    | DISP_DPI_IF_SW_CG_BIT       \
    | DISP_DBI_ENGINE_SW_CG_BIT   \
    | DISP_DBI_SMI_SW_CG_BIT      \
    | DISP_DBI_IF_SW_CG_BIT)

// CG_MFG
#define MFG_PDN_BG3D_SW_CG_BIT  BIT(0)

// TODO: set @ init is better
#define CG_MFG_MASK            (MFG_PDN_BG3D_SW_CG_BIT)

// CG_AUDIO
#define AUD_PDN_AFE_EN_BIT          BIT(2)
#define AUD_PDN_I2S_EN_BIT          BIT(6)
#define AUD_PDN_ADC_EN_BIT          BIT(24)
#define AUD_PDN_DAC_EN_BIT          BIT(25)
#define AUD_PDN_DAC_PREDIS_EN_BIT   BIT(26)
#define AUD_PDN_TML_EN_BIT          BIT(27)

// TODO: set @ init is better
#define CG_AUDIO_MASK          (AUD_PDN_AFE_EN_BIT          \
                              | AUD_PDN_I2S_EN_BIT          \
                              | AUD_PDN_ADC_EN_BIT          \
                              | AUD_PDN_DAC_EN_BIT          \
                              | AUD_PDN_DAC_PREDIS_EN_BIT   \
                              | AUD_PDN_TML_EN_BIT)

#define spm_write(addr, data)       (*(volatile unsigned int *)(addr) = (data))
#define spm_read(addr)              (*(volatile unsigned int *)(addr))
#define clk_writel                  spm_write

void mmsys_on(void)
{
    int i;
         
    (*(volatile unsigned long*)(0x10000084)) |= 0x04000000;
       
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    /// spm_write(SPM_POWER_ON_VAL0, spm_read(SPM_POWER_ON_VAL0) & ~BIT(4));

    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_BIT);                 // PWR_ON = 1
    udelay(1);                                                                          // delay 1 us
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S_BIT);               // PWR_ON_S = 1
    udelay(3);                                                                          // delay 3 us
    
    while (   !(spm_read(SPM_PWR_STATUS)   & DIS_PWR_STA_MASK)                          // wait until PWR_ACK = 1
           || !(spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)
           );
    
    // SRAM_PDN                                                                         // MEM power on
    for (i = BIT(8); i <= BITMASK(11:8); i = (i << 1) + BIT(8))                         // set SRAM_PDN 0 one by one
    {
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~i);
    }

    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);           // PWR_CLK_DIS = 0
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO_BIT);               // PWR_ISO = 0
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_CLK_DIS_BIT);            // PWR_CLK_DIS = 1
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B_BIT);              // PWR_RST_B = 1
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);           // PWR_CLK_DIS = 0
    
    while (   BITMASK(15:12)
           && (spm_read(SPM_DIS_PWR_CON) & BITMASK(15:12))                              // wait until SRAM_PDN_ACK all 0
           );
    
    // BUS_PROTECT
    if (0 != BIT(11))
    {
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~BIT(11));
        while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & BIT(11));
    }
    
    clk_writel(MMSYS_CG_CLR0, CG_MMSYS0_MASK);
    clk_writel(MMSYS_CG_CLR1, CG_MMSYS1_MASK);
    
    (*(volatile unsigned long*)(0x14000108)) |= 03;
}

void mmsys_off(void)
{
    int i;    
    
    (*(volatile unsigned long*)(0x14000104)) |= 0x03;
    
    // BUS_PROTECT                                                                      // enable BUS protect
    if (0 != BIT(11))
    {
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) | BIT(11));
        while ((spm_read(INFRA_TOPAXI_PROTECTSTA1) & BIT(11)) != BIT(11));
    }

    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_CLK_DIS_BIT);            // PWR_CLK_DIS = 1
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO_BIT);                // PWR_ISO = 1
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_RST_B_BIT);             // PWR_RST_B = 0

    // SRAM_PDN                                                                         // MEM power off
    for (i = BIT(8); i <= BITMASK(11:8); i = (i << 1) + BIT(8))                         // set SRAM_PDN 1 one by one
    {
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | i);
    }
      
    while (   BITMASK(15:12)
           && ((spm_read(SPM_DIS_PWR_CON) & BITMASK(15:12)) != BITMASK(15:12))          // wait until SRAM_PDN_ACK all 1
           );
    
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON_BIT));              // PWR_ON = 0
    udelay(1);                                                                          // delay 1 us
    spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON_S_BIT));            // PWR_ON_S = 0
    udelay(1);                                                                          // delay 1 us

    while (   (spm_read(SPM_PWR_STATUS)   & DIS_PWR_STA_MASK)                           // wait until PWR_ACK = 0
           || (spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)
           );
           
    /// spm_write(SPM_POWER_ON_VAL0, spm_read(SPM_POWER_ON_VAL0) | BIT(4));    
}

void mfg_on(void)
{
    int i;

    clk_writel(CLR_CLK_GATING_CTRL0, MFG_MM_SW_CG_BIT);

    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_BIT);                             // PWR_ON = 1
    udelay(1);                                                                                  // delay 1 us
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_S_BIT);                           // PWR_ON_S = 1
    udelay(3);                                                                                  // delay 3 us
    
    while (   !(spm_read(SPM_PWR_STATUS)   & MFG_PWR_STA_MASK)                                     // wait until PWR_ACK = 1
           || !(spm_read(SPM_PWR_STATUS_S) & MFG_PWR_STA_MASK)
           );
    
    // SRAM_PDN                                                                                 // MEM power on
    for (i = BIT(8); i <= BITMASK(8:8); i = (i << 1) + BIT(8))                            // set SRAM_PDN 0 one by one
    {
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~i);
    }

    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_CLK_DIS_BIT);                       // PWR_CLK_DIS = 0
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_ISO_BIT);                           // PWR_ISO = 0
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_CLK_DIS_BIT);                        // PWR_CLK_DIS = 1
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_RST_B_BIT);                          // PWR_RST_B = 1
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_CLK_DIS_BIT);                       // PWR_CLK_DIS = 0
    
    while (   BITMASK(12:12)
           && (spm_read(SPM_MFG_PWR_CON) & BITMASK(12:12))                                // wait until SRAM_PDN_ACK all 0
           );
   
    
    // BUS_PROTECT
    if (0 != 0)
    {
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~0);
        while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & 0);
    }
}

void mfg_off(void)
{
    int i;

    // BUS_PROTECT                                                                              // enable BUS protect
    if (0 != 0)
    {
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) | 0);
        while ((spm_read(INFRA_TOPAXI_PROTECTSTA1) & 0) != 0);
    }

    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_CLK_DIS_BIT);                        // PWR_CLK_DIS = 1
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ISO_BIT);                            // PWR_ISO = 1
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_RST_B_BIT);                         // PWR_RST_B = 0

    // SRAM_PDN                                                                                 // MEM power off
    for (i = BIT(8); i <= BITMASK(8:8); i = (i << 1) + BIT(8))                            // set SRAM_PDN 1 one by one
    {
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | i);
    }

    while (   BITMASK(12:12)
           && ((spm_read(SPM_MFG_PWR_CON) & BITMASK(12:12)) != BITMASK(12:12))    // wait until SRAM_PDN_ACK all 1
           );

    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON_BIT));                          // PWR_ON = 0
    udelay(1);                                                                                  // delay 1 us
    spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON_S_BIT));                        // PWR_ON_S = 0
    udelay(1);                                                                                  // delay 1 us

    while (   (spm_read(SPM_PWR_STATUS)   & MFG_PWR_STA_MASK)                                      // wait until PWR_ACK = 0
           || (spm_read(SPM_PWR_STATUS_S) & MFG_PWR_STA_MASK)
           );

    clk_writel(SET_CLK_GATING_CTRL0, MFG_MM_SW_CG_BIT);
}
