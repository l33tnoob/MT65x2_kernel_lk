/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <dev/uart.h>
#include <arch/arm/mmu.h>
#include <arch/ops.h>

#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <mt_partition.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#include <video.h>
#include <stdlib.h>
#include <string.h>
#include <target/board.h>
#include <platform/mt_logo.h>
#include <platform/mt_gpio.h>
#include <platform/mtk_key.h>
#include <platform/mt_pmic_wrap_init.h>
#include <platform/pll.h>
//#define LK_DL_CHECK
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#include <platform/mt_rtc.h>
#include <platform/mt_leds.h>
#endif
#include <dev/mrdump.h>
#include <platform/env.h>
#include <platform/mtk_wdt.h>
#include <platform/disp_drv.h>
#include <platform/mt_disp_drv.h>
#include <platform/mmc_common_inter.h>
#include <target/cust_key.h>    //MT65XX_MENU_OK_KEY

#ifdef LK_DL_CHECK
/*block if check dl fail*/
#undef LK_DL_CHECK_BLOCK_LEVEL
#endif

extern void platform_early_init_timer();
extern void isink0_init(void);
extern int mboot_common_load_logo(unsigned long logo_addr, char* filename);
extern int sec_func_init(int dev_type);
extern int sec_usbdl_enabled (void);
extern void mtk_wdt_disable(void);
extern void platform_deinit_interrupts(void);

void platform_uninit(void);

/* Transparent to DRAM customize */
int g_nr_bank;
int g_rank_size[4];
unsigned int g_fb_base;
unsigned int g_fb_size;
BOOT_ARGUMENT *g_boot_arg;
BOOT_ARGUMENT  boot_addr;
BI_DRAM bi_dram[MAX_NR_BANK];
BOOT_ARGUMENT g_addr;

extern void jump_da(u32 addr, u32 arg1, u32 arg2);
extern void enable_DA_sram(void);

int clk_init(void)
{
    {
        #define BIT(_bit_)                  (unsigned int)(1 << (_bit_))
        #define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
        #define BITMASK(_bits_)             (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

        #define SPM_BASE                    0x10006000
        #define  SPM_POWERON_CONFIG_SET     (SPM_BASE + 0x0000)
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
#ifdef CLK_GATING_CTRL1
        #undef CLK_GATING_CTRL1
#endif
        #define CLK_GATING_CTRL1            (TOP_CLOCK_CTRL_BASE + 0x0024)
        #define MPLL_FREDIV_EN              (TOP_CLOCK_CTRL_BASE + 0x0030)
        #define UPLL_FREDIV_EN              (TOP_CLOCK_CTRL_BASE + 0x0034)
        #define SET_CLK_GATING_CTRL0        (TOP_CLOCK_CTRL_BASE + 0x0050)
#ifdef SET_CLK_GATING_CTRL1
        #undef SET_CLK_GATING_CTRL1
#endif
        #define SET_CLK_GATING_CTRL1        (TOP_CLOCK_CTRL_BASE + 0x0054)
        #define SET_MPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0060)
        #define SET_UPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0064)
        #define CLR_CLK_GATING_CTRL0        (TOP_CLOCK_CTRL_BASE + 0x0080)
#ifdef CLR_CLK_GATING_CTRL1
        #undef CLR_CLK_GATING_CTRL1
#endif
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

                              // | AUX_SW_CG_MD_BIT        
                              // | PMIC_SW_CG_MD_BIT       
                              // | PMIC_SW_CG_CONN_BIT     

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

        unsigned int i;

        clk_writel(CLR_CLK_GATING_CTRL0,
                   0
                 | GPU_491P52M_EN_BIT       // disable
                 | GPU_500P5M_EN_BIT
                 | SC_MEM_CK_OFF_EN_BIT
                   );

        clk_writel(SET_CLK_GATING_CTRL0,
                   0
                                            // enable
                 | SPM_52M_SW_CG_BIT        // disable
                   );
#if 0
        // default off
        | PWM_MM_SW_CG_BIT
        | CAM_MM_SW_CG_BIT
        | MFG_MM_SW_CG_BIT
        | DBI_BCLK_SW_CG_BIT
        | DBI_PAD0_SW_CG_BIT
        | DBI_PAD1_SW_CG_BIT
        | DBI_PAD2_SW_CG_BIT
        | DBI_PAD3_SW_CG_BIT

        // default on
        | MIPI_26M_DBG_EN_BIT
        | SC_26M_CK_SEL_EN_BIT
        | ARMDCM_CLKOFF_EN_BIT
#endif

        clk_writel(CLR_CLK_GATING_CTRL1,
                   0
                 | SPM_SW_CG_BIT            // enable
                 | PMIC_SW_CG_AP_BIT
                   );

        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | EFUSE_SW_CG_BIT          // disable
                 | MEMSLP_DLYER_SW_CG_BIT
                 | PMIC_26M_SW_CG_BIT
                 | AUDIO_SW_CG_BIT
                 | SPINFI_SW_CG_BIT
                   );

// EMMC
#ifdef MTK_EMMC_SUPPORT
        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | NFI_SW_CG_BIT            // disable
                 | NFIECC_SW_CG_BIT
                   );
// NAND
#else
    #ifdef MTK_SPI_NAND_SUPPORT // SPI-NAND
        clk_writel(CLR_CLK_GATING_CTRL1,
                   0
                 | SPINFI_SW_CG_BIT         // enable
                   );
    #endif

        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | MSDC0_SW_CG_BIT          // disable
                   );
#endif

#if 0
        // default off
        | UART1_SW_CG_BIT
        | THEM_SW_CG_BIT
        | APDMA_SW_CG_BIT
        | I2C0_SW_CG_BIT
        | I2C1_SW_CG_BIT
        | PWM_SW_CG_BIT
        | BTIF_SW_CG_BIT
        | USB_SW_CG_BIT
        | FHCTL_SW_CG_BIT
        | AUX_SW_CG_THERM_BIT
        | MSDC1_SW_CG_BIT
        | AUX_SW_CG_ADC_BIT
        | AUX_SW_CG_TP_BIT

        // default on
        | SEJ_SW_CG_BIT
        | DEBUGSYS_SW_CG_BIT
        | UART0_SW_CG_BIT
        | APXGPT_SW_CG_BIT
#endif

        spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

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

        while (   BITMASK(15:12)
               && (spm_read(SPM_DIS_PWR_CON) & BITMASK(15:12))                              // wait until SRAM_PDN_ACK all 0
               );

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_CLK_DIS_BIT);            // PWR_CLK_DIS = 1
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B_BIT);              // PWR_RST_B = 1
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);           // PWR_CLK_DIS = 0

        // BUS_PROTECT
        if (0 != BIT(11))
        {
            spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~BIT(11));
            while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & BIT(11));
        }

        clk_writel(MMSYS_CG_CLR0,
                   SMI_COMMON_SW_CG_BIT
                 | SMI_LARB0_SW_CG_BIT
                 | MM_CMDQ_SW_CG_BIT
                 | MM_CMDQ_SMI_IF_SW_CG_BIT
                   );
    }
}

int dram_init(void)
{
    int i;
    unsigned int dram_rank_num;

    /* Get parameters from pre-loader. Get as early as possible
     * The address of BOOT_ARGUMENT_LOCATION will be used by Linux later
     * So copy the parameters from BOOT_ARGUMENT_LOCATION to LK's memory region
     */
    g_boot_arg = &boot_addr;
    memcpy(g_boot_arg, BOOT_ARGUMENT_LOCATION, sizeof(BOOT_ARGUMENT));

#ifdef MACH_FPGA
    g_nr_bank = 1;
    bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
    bi_dram[0].size = 0xCB00000;
    //bi_dram[1].start = 0x10000000;
    //bi_dram[1].size = 0x10000000;
#else

    dram_rank_num = g_boot_arg->dram_rank_num;

    g_nr_bank = dram_rank_num;

    for (i = 0; i < g_nr_bank; i++)
    {
        g_rank_size[i] = g_boot_arg->dram_rank_size[i];
    }

    if (g_nr_bank == 1)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
    } else if (g_nr_bank == 2)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];
    } else if (g_nr_bank == 3)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];
        bi_dram[2].start = bi_dram[1].start + bi_dram[1].size;
        bi_dram[2].size = g_rank_size[2];
    } else if (g_nr_bank == 4)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];
        bi_dram[2].start = bi_dram[1].start + bi_dram[1].size;
        bi_dram[2].size = g_rank_size[2];
        bi_dram[3].start = bi_dram[2].start + bi_dram[2].size;
        bi_dram[3].size = g_rank_size[3];
    } else
    {
        //dprintf(CRITICAL, "[LK ERROR] DRAM bank number is not correct!!!");
        while (1) ;
    }
#endif
    return 0;
}

/*******************************************************
 * Routine: memory_size
 * Description: return DRAM size to LCM driver
 ******************************************************/
u32 memory_size(void)
{
    int nr_bank = g_nr_bank;
    int i, size = 0;

    for (i = 0; i < nr_bank; i++)
        size += bi_dram[i].size;
    size += RESERVE_MEM_SIZE;

    return size;
}

void sw_env()
{
#ifdef LK_DL_CHECK
    int dl_status = 0;
#ifdef MTK_EMMC_SUPPORT
    dl_status = mmc_get_dl_info();
#else
    dl_status = nand_get_dl_info();
#endif
    printf("mt65xx_sw_env--dl_status: %d\n", dl_status);
    if (dl_status != 0)
    {
        video_printf("=> TOOL DL image Fail!\n");
        printf("TOOL DL image Fail\n");
#ifdef LK_DL_CHECK_BLOCK_LEVEL
        printf("uboot is blocking by dl info\n");
        while (1) ;
#endif
    }
#endif

#ifndef USER_BUILD
    switch (g_boot_mode)
    {
      case META_BOOT:
          video_printf(" => META MODE\n");
          break;
      case FACTORY_BOOT:
          video_printf(" => FACTORY MODE\n");
          break;
      case RECOVERY_BOOT:
          video_printf(" => RECOVERY MODE\n");
          break;
      case SW_REBOOT:
          //video_printf(" => SW RESET\n");
          break;
      case NORMAL_BOOT:
          if(g_boot_arg->boot_reason != BR_RTC && get_env("hibboot") != NULL && atoi(get_env("hibboot")) == 1)
              video_printf(" => HIBERNATION BOOT\n");
          else
              video_printf(" => NORMAL BOOT\n");
          break;
      case ADVMETA_BOOT:
          video_printf(" => ADVANCED META MODE\n");
          break;
      case ATE_FACTORY_BOOT:
          video_printf(" => ATE FACTORY MODE\n");
          break;
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	case KERNEL_POWER_OFF_CHARGING_BOOT:
		video_printf(" => POWER OFF CHARGING MODE\n");
		break;
	case LOW_POWER_OFF_CHARGING_BOOT:
		video_printf(" => LOW POWER OFF CHARGING MODE\n");
		break;
#endif
      case ALARM_BOOT:
          video_printf(" => ALARM BOOT\n");
          break;
      case FASTBOOT:
          video_printf(" => FASTBOOT mode...\n");
          break;
      default:
          video_printf(" => UNKNOWN BOOT\n");
    }
    return;
#endif

#ifdef USER_BUILD
    if(g_boot_mode == FASTBOOT)
        video_printf(" => FASTBOOT mode...\n");
#endif

}

void platform_init_mmu_mappings(void)
{
  /* configure available RAM banks */
  dram_init();

/* Enable D-cache  */

  unsigned int offset;
  unsigned int dram_size = 0;

  dram_size = memory_size();

  /* do some memory map initialization */
  for (offset = 0; offset < dram_size; offset += (1024*1024))
  {
    /*virtual to physical 1-1 mapping*/
    arm_mmu_map_section(DRAM_PHY_ADDR+offset, DRAM_PHY_ADDR+offset, MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE | MMU_MEMORY_AP_READ_WRITE);
  }

}

void platform_early_init(void)
{
#ifdef LK_PROFILING
    unsigned int time_led_init;
    unsigned int time_pmic6329_init;
    unsigned int time_i2c_init;
    unsigned int time_disp_init;
    unsigned int time_platform_early_init;
    unsigned int time_set_clock;
    unsigned int time_disp_preinit;
    unsigned int time_misc_init;
    unsigned int time_clock_init;
    unsigned int time_wdt_init;

    time_platform_early_init = get_timer(0);
    time_set_clock = get_timer(0);
#endif
    //mt_gpio_set_default();

    //Specific for MT6572. ARMPLL can't set to max speed when L2 is configured as SRAM.
    //preloader won't reach max speed. It will done by LK.
    if (g_boot_arg->boot_mode != DOWNLOAD_BOOT)
    {
    mtk_set_arm_clock();
    }

    /* initialize the uart */
    uart_init_early();

    printf("arm clock set finished\n");

#ifdef LK_PROFILING
    printf("[PROFILE] ------- set clock takes %d ms -------- \n", get_timer(time_set_clock));
    time_misc_init = get_timer(0);
#endif

    platform_init_interrupts();
    platform_early_init_timer();
    mt_gpio_set_default();

    /* initialize the uart */
    uart_init_early();
#ifdef LK_PROFILING
    printf("[PROFILE] -------misc init takes %d ms -------- \n", get_timer(time_misc_init));
    time_i2c_init = get_timer(0);
#endif

    mt_i2c_init();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- i2c init takes %d ms -------- \n", get_timer(time_i2c_init));
    time_clock_init = get_timer(0);
#endif

    clk_init();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- clock init takes %d ms -------- \n", get_timer(time_clock_init));
    time_wdt_init = get_timer(0);
#endif

    mtk_wdt_init();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- wdt init takes %d ms -------- \n", get_timer(time_wdt_init));
    time_disp_preinit = get_timer(0);
#endif

    /* initialize the frame buffet information */
    g_fb_size = mt_disp_get_vram_size();
    g_fb_base = memory_size() - g_fb_size + DRAM_PHY_ADDR;

    dprintf(INFO, "FB base = 0x%x, FB size = %d\n", g_fb_base, g_fb_size);

#ifdef LK_PROFILING
    printf("[PROFILE] -------disp preinit takes %d ms -------- \n", get_timer(time_disp_preinit));
    time_led_init = get_timer(0);
#endif

#ifndef MACH_FPGA
    leds_init();
#endif

    isink0_init();              //turn on PMIC6329 isink0

#ifdef LK_PROFILING
    printf("[PROFILE] ------- led init takes %d ms -------- \n", get_timer(time_led_init));
    time_disp_init = get_timer(0);
#endif

    mt_disp_init((void *)g_fb_base);

#ifdef CONFIG_CFB_CONSOLE
    drv_video_init();
#endif

#ifdef LK_PROFILING
    printf("[PROFILE] ------- disp init takes %d ms -------- \n", get_timer(time_disp_init));
    time_pmic6329_init = get_timer(0);
#endif

//#ifndef MACH_FPGA
//    pwrap_init_lk();
//    pwrap_init_for_early_porting();
//#endif

    //pmic6320_init();  //[TODO] init PMIC

#ifdef LK_PROFILING
    printf("[PROFILE] ------- pmic6329_init takes %d ms -------- \n", get_timer(time_pmic6329_init));
    printf("[PROFILE] ------- platform_early_init takes %d ms -------- \n", get_timer(time_platform_early_init));
#endif
}

extern void mt65xx_bat_init(void);
#ifdef MTK_MT8193_SUPPORT
extern int mt8193_init(void);
#endif

#if defined (MTK_KERNEL_POWER_OFF_CHARGING)

int kernel_charging_boot(void)
{
	if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_TRUE)
	{
		printf("[%s] Kernel Power Off Charging with Charger/Usb \n", __func__);
		return  1;
	}
	else if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_FALSE)
	{
		printf("[%s] Kernel Power Off Charging without Charger/Usb \n", __func__);
		return -1;
	}
	else
		return 0;
}
#endif

void platform_init(void)
{

    /* init timer */
    //mtk_timer_init();
#ifdef LK_PROFILING
    unsigned int time_nand_emmc;
    unsigned int time_load_logo;
    unsigned int time_bat_init;
    unsigned int time_backlight;
    unsigned int time_show_logo;
    unsigned int time_boot_mode;
    unsigned int time_sw_env;
    unsigned int time_download_boot_check;
    unsigned int time_rtc_check;
    unsigned int time_platform_init;
    unsigned int time_sec_init;
    unsigned int time_env;
    time_platform_init = get_timer(0);
    time_nand_emmc = get_timer(0);
#endif
    dprintf(INFO, "platform_init()\n");

#ifdef MTK_MT8193_SUPPORT
	mt8193_init();
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
    // init eMMC/NAND driver only in normal boot, not in memory dump
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
#ifdef MTK_EMMC_SUPPORT
        mmc_legacy_init(1);
#else
        nand_init();
        nand_driver_test();
#endif
    }

#ifdef LK_PROFILING
    printf("[PROFILE] ------- NAND/EMMC init takes %d ms -------- \n", get_timer(time_nand_emmc));
    time_env = get_timer(0);
#endif

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if((g_boot_arg->boot_reason == BR_USB) && (upmu_is_chr_det() == KAL_FALSE))
	{
		printf("[%s] Unplugged Charger/Usb between Pre-loader and Uboot in Kernel Charging Mode, Power Off \n", __func__);
		mt6575_power_off();
	}
#endif

	env_init();
	print_env();
#ifdef LK_PROFILING
	printf("[PROFILE] ------- ENV init takes %d ms -------- \n", get_timer(time_env));
	time_load_logo = get_timer(0);
#endif
#if defined(MEM_PRESERVED_MODE_ENABLE)
    // only init eMMC/NAND driver in normal boot, not in memory dump
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
        mboot_common_load_logo((unsigned long)mt_get_logo_db_addr(), "logo");
    }
    dprintf(INFO, "Show BLACK_PICTURE\n");
    mt_disp_power(TRUE);
    mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
#ifdef LK_PROFILING
    printf("[PROFILE] ------- load_logo takes %d ms -------- \n", get_timer(time_load_logo));
    time_backlight = get_timer(0);
#endif

    /*for kpd pmic mode setting*/
    set_kpd_pmic_mode();
#ifndef DISABLE_FOR_BRING_UP
    mt65xx_backlight_on();
#endif

#ifdef LK_PROFILING
    printf("[PROFILE] ------- backlight takes %d ms -------- \n", get_timer(time_backlight));
    time_boot_mode = get_timer(0);
#endif
    enable_PMIC_kpd_clock();
#if defined(MEM_PRESERVED_MODE_ENABLE)
    // when memory preserved mode, do not check KPOC (in boot_mode_selec()), which will power off the target
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
        boot_mode_select();
    }
#ifdef LK_PROFILING
    printf("[PROFILE] ------- boot mode select takes %d ms -------- \n", get_timer(time_boot_mode));
    time_sec_init = get_timer(0);
#endif

    /* initialize security library */
#ifdef MTK_EMMC_SUPPORT
    sec_func_init(1);
#else
    sec_func_init(0);
#endif

#ifdef LK_PROFILING
        printf("[PROFILE] ------- sec init takes %d ms -------- \n", get_timer(time_sec_init));
        time_download_boot_check = get_timer(0);
#endif

    /*Show download logo & message on screen */
    if (g_boot_arg->boot_mode == DOWNLOAD_BOOT)
    {
	printf("[LK] boot mode is DOWNLOAD_BOOT\n");
	/* verify da before jumping to da*/
	if (sec_usbdl_enabled()) {
	    u8  *da_addr = (unsigned char *)g_boot_arg->da_info.addr;
	    u32 da_sig_len = DRV_Reg32(SRAMROM_BASE + 0x30);
	    u32 da_len   = da_sig_len >> 10;
	    u32 sig_len  = da_sig_len & 0x3ff;
	    u8  *sig_addr = (unsigned char *)da_addr + (da_len - sig_len);

	    if (da_len == 0 || sig_len == 0) {
		printf("[LK] da argument is invalid\n");
		printf("da_addr = 0x%x\n", da_addr);
		printf("da_len  = 0x%x\n", da_len);
		printf("sig_len = 0x%x\n", sig_len);
	    }

	    if (sec_usbdl_verify_da(da_addr, (da_len - sig_len), sig_addr, sig_len)) {
		/* da verify fail */
                video_printf(" => Not authenticated tool, download stop...\n");
		DRV_WriteReg32(SRAMROM_BASE + 0x30, 0x0);
		while(1); /* fix me, should not be infinite loop in lk */
	    }
	}
	else {
	    printf(" DA verification disabled...\n");
	}

	/* clear da length and da signature length information */
	DRV_WriteReg32(SRAMROM_BASE + 0x30, 0x0);

        mt_disp_show_boot_logo();
        video_printf(" => Downloading...\n");
#ifndef DISABLE_FOR_BRING_UP
        mt65xx_backlight_on();
#endif
        mtk_wdt_disable();//Disable wdt before jump to DA
	platform_uninit();
#ifdef HAVE_CACHE_PL310
        l2_disable();
#endif
        arch_disable_cache(UCACHE);
        arch_disable_mmu();

        //enable sram for DA usage
        enable_DA_sram();
        jump_da(g_boot_arg->da_info.addr, g_boot_arg->da_info.arg1, g_boot_arg->da_info.arg2);
    }


#ifdef LK_PROFILING
    printf("[PROFILE] ------- download boot check takes %d ms -------- \n", get_timer(time_download_boot_check));
    time_bat_init = get_timer(0);
#endif
    mt65xx_bat_init();
#ifdef LK_PROFILING
    printf("[PROFILE] ------- battery init takes %d ms -------- \n", get_timer(time_bat_init));
    time_rtc_check = get_timer(0);
#endif

#ifndef CFG_POWER_CHARGING
    /* NOTE: if define CFG_POWER_CHARGING, will rtc_boot_check() in mt65xx_bat_init() */
    rtc_boot_check(false);
#endif

#ifdef LK_PROFILING
    printf("[PROFILE] ------- rtc check takes %d ms -------- \n", get_timer(time_rtc_check));
    time_show_logo = get_timer(0);
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
    // when memory preserved mode, do not check KPOC (in boot_mode_selec()), which will power off the target
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    	if(kernel_charging_boot() == 1)
    	{

    		mt_disp_power(TRUE);
    		mt_disp_show_low_battery();
    		mt_disp_wait_idle();
#ifndef DISABLE_FOR_BRING_UP
    		mt65xx_leds_brightness_set(6, 110);
#endif
    	}
    	else if(g_boot_mode != KERNEL_POWER_OFF_CHARGING_BOOT && g_boot_mode != LOW_POWER_OFF_CHARGING_BOOT)
    	{
#ifndef MACH_FPGA
    		if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
    		{
    			mt_disp_show_boot_logo();
    		}
#endif
    	}
#else
#ifndef MACH_FPGA
    	if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
    	{
    		mt_disp_show_boot_logo();
    	}
#endif
#endif
    }   //if (TRUE != mtk_wdt_is_mem_preserved())


#ifdef LK_PROFILING
    printf("[PROFILE] ------- show logo takes %d ms -------- \n", get_timer(time_show_logo));
    time_sw_env= get_timer(0);
#endif
	//mt_i2c_test();

    sw_env();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- sw_env takes %d ms -------- \n", get_timer(time_sw_env));
    printf("[PROFILE] ------- platform_init takes %d ms -------- \n", get_timer(time_platform_init));
#endif
}

void platform_uninit(void)
{
#ifndef DISABLE_FOR_BRING_UP
    leds_deinit();
#endif
	platform_deinit_interrupts();
}

#ifdef ENABLE_L2_SHARING
#define L2C_SIZE_CFG_OFF 5
/* config L2 cache and sram to its size */
void config_L2_size(void)
{
    volatile unsigned int cache_cfg;
    /* set L2C size to 256KB */
    cache_cfg = DRV_Reg(MCUSYS_CFGREG_BASE);
    cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFF);
    cache_cfg |= 0x1 << L2C_SIZE_CFG_OFF;
    DRV_WriteReg(MCUSYS_CFGREG_BASE, cache_cfg);
}

void enable_DA_sram(void)
{
    volatile unsigned int cache_cfg;

    //enable L2 sram for DA
    cache_cfg = DRV_Reg(MCUSYS_CFGREG_BASE);
    cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFF);
    DRV_WriteReg(MCUSYS_CFGREG_BASE, cache_cfg);

    //enable audio sysram clk for DA
    *(volatile unsigned int *)(0x10000084) = 0x02000000;
}
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
/*
 * 10200000
 * 4		L2RSTDISABLE	0: L2 cache is reset by hardware.
 *                          1: L2 cache is not reset by hardware..
 * 1:0		L1RSTDISABLE	0: L1 cache is reset by hardware.
 *                          1: L1 cache is not reset by hardware..
*/

#define MCUSYS_L2RSTDISABLE_MASK    (0x10)
#define MCUSYS_L1RSTDISABLE_MASK    (0x03)
#define EMI_GENA                    (EMI_BASE + 0x70)
#define MEM_PSRV_REQ_EN             (0x4)
#define MEM_PRESERVED_MAGIC         (0x504D454D)

static void mtk_mcusys_l1_rst_config(unsigned int rst_enable)
{
    volatile unsigned int cfgreg;

    cfgreg  = DRV_Reg32(MCUSYS_CFGREG_BASE);
    if (0 == rst_enable)    //reset disable
        cfgreg |= MCUSYS_L1RSTDISABLE_MASK;
    else
        cfgreg &= (~MCUSYS_L1RSTDISABLE_MASK);

    DRV_WriteReg32(MCUSYS_CFGREG_BASE, cfgreg);
}

static void mtk_mcusys_l2_rst_config(unsigned int rst_enable)
{
    volatile unsigned int cfgreg;

    cfgreg  = DRV_Reg32(MCUSYS_CFGREG_BASE);
    if (0 == rst_enable)    //reset disable
        cfgreg |= MCUSYS_L2RSTDISABLE_MASK;
    else
        cfgreg &= (~MCUSYS_L2RSTDISABLE_MASK);

    DRV_WriteReg32(MCUSYS_CFGREG_BASE, cfgreg);
}

void platform_mem_preserved_config(unsigned int enable)
{
    volatile unsigned int cfgreg;

/*
    printf("====WDT register 0x80, 0x02====\n");
    printf("REG(0x%X)=0x%X\n", TOP_RGU_BASE, DRV_Reg32(TOP_RGU_BASE));
    printf("REG(0x%X)=0x%X\n", (TOP_RGU_BASE + 0x40), DRV_Reg32((TOP_RGU_BASE + 0x40)));
    printf("====MCUSYS register 0x13====\n");
    printf("REG(0x%X)=0x%X\n", MCUSYS_CFGREG_BASE, DRV_Reg32(MCUSYS_CFGREG_BASE));
    printf("====EMI GENA register 0x4 ====\n");
    printf("REG(0x%X)=0x%X\n", EMI_GENA, DRV_Reg32(EMI_GENA));
    printf("====SRAMROM_BASE 0x24, 0x28 ====\n");
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x24, DRV_Reg32(SRAMROM_BASE + 0x24));
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x28, DRV_Reg32(SRAMROM_BASE + 0x28));
*/

    mtk_wdt_presrv_mode_config(enable,enable);

    //enable memory preserved mode, do not reset
    mtk_mcusys_l1_rst_config(!(enable));
    mtk_mcusys_l2_rst_config(!(enable));

    cfgreg  = DRV_Reg32(EMI_GENA);
    if (0 == enable)
        cfgreg &= (~MEM_PSRV_REQ_EN);
    else
        cfgreg |= MEM_PSRV_REQ_EN;

    DRV_WriteReg32(EMI_GENA, cfgreg);

    if (0 == enable){
        //prepare the entry for BROM jump(0x10001424)
   		DRV_WriteReg32(SRAMROM_BASE + 0x24, 0x0);
        DRV_WriteReg32(SRAMROM_BASE + 0x28, 0x0);
    }else{
        //prepare the entry for BROM jump(0x10001424)
        DRV_WriteReg32(SRAMROM_BASE + 0x28, MEM_PRESERVED_MAGIC);
   		DRV_WriteReg32(SRAMROM_BASE + 0x24, MEM_SRAM_PRELOADER_START);
    }
/*
    printf("====WDT register 0x80, 0x02====\n");
    printf("REG(0x%X)=0x%X\n", TOP_RGU_BASE, DRV_Reg32(TOP_RGU_BASE));
    printf("REG(0x%X)=0x%X\n", (TOP_RGU_BASE + 0x40), DRV_Reg32((TOP_RGU_BASE + 0x40)));
    printf("====MCUSYS register 0x13====\n");
    printf("REG(0x%X)=0x%X\n", MCUSYS_CFGREG_BASE, DRV_Reg32(MCUSYS_CFGREG_BASE));
    printf("====EMI GENA register 0x4 ====\n");
    printf("REG(0x%X)=0x%X\n", EMI_GENA, DRV_Reg32(EMI_GENA));
    printf("====SRAMROM_BASE 0x24, 0x28 ====\n");
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x24, DRV_Reg32(SRAMROM_BASE + 0x24));
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x28, DRV_Reg32(SRAMROM_BASE + 0x28));
*/
}
void platform_mem_preserved_load_img(void)
{
    //enter memory preserved mode
    //if ((TRUE == mtk_wdt_mem_preserved_is_enabled()) && (TRUE != mtk_wdt_is_mem_preserved()))
    //always load sram/mem preloader when sysfs decide enter memory preserved mode
    if (TRUE != mtk_wdt_is_mem_preserved())
    {
        char * name;
        unsigned int start_addr;
        unsigned int size;
        unsigned int tmp;

        tmp = DRV_Reg32(TOP_RGU_BASE);
        tmp = (tmp & ~(0x1));
        tmp = (tmp | 0x22000000);
        DRV_WriteReg32(TOP_RGU_BASE,tmp);
        printf ("wdt_mode=0x%x\n",mtk_wdt_mem_preserved_is_enabled());
#if 1
        mboot_mem_preserved_load_part(PART_UBOOT, MEM_SRAM_PRELOADER_START, MEM_PRELOADER_START);
#else
        name = PART_SRAM_PRELD;
        start_addr = MEM_PRELOADER_START;
        size = MEM_SRAM_PRELOADER_SIZE;
        printf("[MEM] ------- start DMA and Copy part[%s] start=0x%x, size=0x%x -------- \n",name,start_addr, size );
        mboot_recovery_load_raw_part(name, start_addr, size );


        /* relocate mem sram preloader into On-Chip SRAM */
        printf("[MEM] ------- start memcpy part[%s] start=0x%x, size=0x%x -------- \n",name,MEM_SRAM_PRELOADER_START, size );
        memcpy((char *)MEM_SRAM_PRELOADER_START, (char *)start_addr, size);

        name = PART_MEM_PRELD;
        start_addr = MEM_PRELOADER_START;
        size = MEM_PRELOADER_SIZE;
        printf("[MEM] ------- start DMA part[%s] start=0x%x, size=0x%x -------- \n",name,start_addr, size );
        mboot_recovery_load_raw_part(name, start_addr, size );
#endif
#if 0
        printf("[MEM] write pattern into EMI through MMU/cache \n");
        for (tmp=0;tmp<0x100;tmp=tmp+4)
        {
            *(volatile u32 *)(DRAM_PHY_ADDR + 0x120000 + tmp) = tmp;
        }
#endif
#if 0
        printf("[MEM] 72dead loop, waiting for WDT \n");
        mtk_wdt_mem_preserved_hw_reset();
        while(1);
#endif
    }
}

void platform_mem_preserved_dump_mem(void)
{
    //enter memory preserved mode
    if (TRUE == mtk_wdt_is_mem_preserved())
    {
        struct mrdump_regset per_cpu_regs[NR_CPUS];
        struct mrdump_regpair regpairs[9];

        unsigned int tmp, i, reg_addr;

#if 0
        // do not disable WDT
        tmp = DRV_Reg32(TOP_RGU_BASE);
        tmp = (tmp & ~(0x1));
        tmp = (tmp | 0x22000000);
        DRV_WriteReg32(TOP_RGU_BASE,tmp);
#endif
        printf ("wdt_flag=0x%x\n",mtk_wdt_is_mem_preserved());
        mtk_wdt_clear_mem_preserved_status();
        printf ("after clear wdt_flag=0x%x\n",mtk_wdt_is_mem_preserved());

        memset(per_cpu_regs, 0, sizeof(per_cpu_regs));
        per_cpu_regs[0].pc = DRV_Reg32(DBG_CORE0_PC);
        per_cpu_regs[0].fp = DRV_Reg32(DBG_CORE0_FP);
        per_cpu_regs[0].sp = DRV_Reg32(DBG_CORE0_SP);

        per_cpu_regs[1].pc = DRV_Reg32(DBG_CORE1_PC);
        per_cpu_regs[1].fp = DRV_Reg32(DBG_CORE1_FP);
        per_cpu_regs[1].sp = DRV_Reg32(DBG_CORE1_SP);

        //dump AHBABT Monitor register
        memset(regpairs, 0, sizeof(regpairs));

        for ( i = 0 ; i < 8 ; i++)
        {
            reg_addr = AHBABT_ADDR1 + (i*4);
            regpairs[0].addr = reg_addr;
            regpairs[0].val = DRV_Reg32(reg_addr);
        }
        // end of regpairs, set addr and val to 0
        regpairs[8].addr = 0;
        regpairs[8].val = 0;

        printf ("==== 72we are in memory preserved mode====\n");

        mt_set_gpio_mode(CARD_DETECT_PIN,4);
        // if no card is insert, remind inser sd card
        if (0 != mt_get_gpio_in(CARD_DETECT_PIN))
        {
            printf("Please Insert SD card for dump\n");
            printf("Press[VOL DOWN] to continue\n");
#if defined(MEM_PRESERVED_MODE_VIDEO_PRINT)
            video_printf("Please Insert SD card for dump\n");
            video_printf("Press[VOL DOWN] to continue\n");
#endif
            mtk_wdt_restart();

            while(1) {
                // VOL_DOWN
                if (mtk_detect_key(MT65XX_MENU_OK_KEY)) {
                    break;
                }
//                printf ("card status=0x%x\n",mt_get_gpio_in(CARD_DETECT_PIN));
            }
        }

#if defined(MEM_PRESERVED_MODE_VIDEO_PRINT)
        video_printf("[MEM_PRE]Start Dump\n");
#endif
        mrdump_run(&per_cpu_regs, regpairs);
    }
}
#endif ////#if defined(MEM_PRESERVED_MODE_ENABLE)
