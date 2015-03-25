/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2010
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include "msdc.h"

#if defined(MMC_MSDC_DRV_CTP)
#if !MSDC_USE_FPGA
#include <mach/mt_clkmgr.h>
#include <pmic.h>
#endif
#endif

#if defined(MMC_MSDC_DRV_PRELOADER) ||defined(MMC_MSDC_DRV_LK)
//To include g_mmcTable[]
#include "../../../kernel/drivers/mmc-host/emmc_device_list.h"
#endif

#if defined(MMC_MSDC_DRV_LK)
#include "upmu_common.h"
#endif

#define PERI_MSDC_SRCSEL    (0xc100000c)

static msdc_priv_t msdc_priv[MSDC_MAX_NUM];

static int msdc_rsp[] = {
    0,  /* RESP_NONE */
    1,  /* RESP_R1 */
    2,  /* RESP_R2 */
    3,  /* RESP_R3 */
    4,  /* RESP_R4 */
    1,  /* RESP_R5 */
    1,  /* RESP_R6 */
    1,  /* RESP_R7 */
    7,  /* RESP_R1b */
};

#if MSDC_DEBUG
static struct msdc_regs *msdc_reg[MSDC_MAX_NUM];
#endif

static int msdc_tune_forcing[MSDC_MAX_NUM] = {0, 0}; //force tuning for debugging purpose
//{133250000, 160000000, 200000000, 178280000, 189420000, 0, 26000000, 208000000};
//Note: 189MHz cannot be used for DDR533
//Since clock source index is not determined by clock frequency,
// we use an array to store index according sorting on clock frequency value
#if MSDC_USE_CLKSRC_IN_DATCRC
static u32 msdc_clock_tune_seq[] = {MSDC_CLKSRC_208MHZ, MSDC_CLKSRC_200MHZ,
                                    MSDC_CLKSRC_178MHZ, MSDC_CLKSRC_160MHZ, MSDC_CLKSRC_133MHZ};
static u32 msdc_clock_tune_seq_index[MSDC_MAX_NUM];
#endif
//#define MSDC_CLKSRC_HIGH_INDEX      MSDC_CLKSRC_208MHZ
//#define MSDC_CLKSRC_LOW_INDEX       MSDC_CLKSRC_133MHZ
#if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
static int msdc_clock_divisor[MSDC_MAX_NUM];
#endif

#if defined(MMC_MSDC_DRV_CTP)
#if 0 //Light: turn if off before I verify it
u32 sg_autocmd_crc_tuning_blkno = 0xFFFFFFFF;
u32 sg_autocmd_crc_tuning_count = 0;
#endif
#endif

void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks)
{
    u32 base = host->base;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout >>TMO_IN_CLK_2POWER;

    timeout = timeout > 1 ? timeout - 1 : 0;
    timeout = timeout > 255 ? 255 : timeout;

    MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, timeout);

    MSG(OPS_MMC, "[SD%d] Set data timeout with sclk(%d): %dns %dclks -> %d x %d cycles\n",
        host->id, host->sclk, ns, clks, timeout + 1, 1<<TMO_IN_CLK_2POWER);
}

void msdc_set_blklen(struct mmc_host *host, u32 blklen)
{
    u32 base = host->base;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    host->blklen     = blklen;
    #if defined(MSDC_USE_DMA_MODE)
    priv->cfg.blklen = blklen;
    #endif
    MSDC_CLR_FIFO();
}

void msdc_set_blknum(struct mmc_host *host, u32 blknum)
{
    u32 base = host->base;

    MSDC_WRITE32(SDC_BLK_NUM, blknum);
}

void msdc_set_dmode(struct mmc_host *host, int mode)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    #if defined(MSDC_USE_DMA_MODE)
    priv->cfg.mode = mode;
    #endif

    if (mode == MSDC_MODE_PIO) {
        host->blk_read  = msdc_pio_bread;
        host->blk_write = msdc_pio_bwrite;
    #if defined(MSDC_USE_MMC_STREAM)
    } else if (mode == MSDC_MODE_MMC_STREAM) {
        host->blk_read  = msdc_stream_bread;
        host->blk_write = msdc_stream_bwrite;
    #endif
    #if defined(MSDC_USE_DMA_MODE)
    } else {
        host->blk_read  = msdc_dma_bread;
        host->blk_write = msdc_dma_bwrite;
    #endif
    }
}

void msdc_set_pio_bits(struct mmc_host *host, int bits)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    priv->pio_bits = bits;
}

void msdc_set_autocmd(struct mmc_host *host, int cmd, int on)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    if (on) {
        priv->autocmd |= cmd;
    } else {
        priv->autocmd &= ~cmd;
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// IO Config Control
//
////////////////////////////////////////////////////////////////////////////////

void msdc_set_smt(struct mmc_host *host, int set_smt)
{
    u32 reg_addr;

    switch (host->id) {
        case 0:
            if (set_smt)
                reg_addr=MSDC0_SMT_CFG_SET;
            else
                reg_addr=MSDC0_SMT_CFG_CLR;

            #if defined(FEATURE_MMC_BOOT_MODE)
                MSDC_WRITE32(reg_addr, MSDC0_SMT_CK_CM_DAT|MSDC0_SMT_DAT7_4);
            #else
                MSDC_WRITE32(reg_addr, MSDC0_SMT_CK_CM_DAT);
            #endif
            break;

        #if defined(MMC_MSDC_DRV_CTP) || defined(FEATURE_MMC_MEM_PRESERVE_MODE)
        case 1:
            if (set_smt) {
                MSDC_WRITE32(MSDC1_SMT_CFG_SET, MSDC1_SMT_CK_CM_DAT);
            } else {
                MSDC_WRITE32(MSDC1_SMT_CFG_CLR, MSDC1_SMT_CK_CM_DAT);
            }
            break;
        #endif

        default:
            break;
    }
}

void msdc_set_rdtdsel(struct mmc_host *host,bool sd_18)
{
    switch (host->id) {
        case 0:
            MSDC_SET_FIELD(MSDC0_TDSEL_BASE, MSDC0_TDSEL ,0x0);
            MSDC_SET_FIELD(MSDC0_RDSEL_BASE, MSDC0_RDSEL ,0x0);
            break;
        case 1:
            if(sd_18){
                MSDC_SET_FIELD(MSDC1_TDSEL_BASE, MSDC1_TDSEL ,0x0);
                MSDC_SET_FIELD(MSDC1_RDSEL_BASE, MSDC1_RDSEL ,0x0);
            } else{
                MSDC_SET_FIELD(MSDC1_TDSEL_BASE, MSDC1_TDSEL ,0x5);
                MSDC_SET_FIELD(MSDC1_RDSEL_BASE, MSDC1_RDSEL ,0xc);
            }
            break;
        default:
            break;
    }
}

void msdc_set_driving(struct mmc_host *host, u8 clkdrv, u8 cmddrv, u8 datdrv)
{
    //Light: Set driving is not necessary for BROM/Preloader/LK,
    //                   but necessary for CTP
    //Light Make sure if tune driving is performed in msdc_dev_bread()

    switch (host->id) {
        //For 6572, CLK, CMD, DAT3~0 share one driving setting
        case 0:
            MSDC_SET_FIELD(MSDC0_DRVING_BASE, MSDC0_CK_CM_DAT_DRVING, clkdrv);
            #if defined(FEATURE_MMC_BOOT_MODE)
            MSDC_SET_FIELD(MSDC0_DRVING_BASE, MSDC0_DAT7_4_DRVING, clkdrv);
            #endif

            break;

        #if defined(MMC_MSDC_DRV_CTP) || defined(FEATURE_MMC_MEM_PRESERVE_MODE)
        case 1:
            MSDC_SET_FIELD(MSDC1_DRVING_BASE, MSDC1_CK_CM_DAT_DRVING, clkdrv);
            break;
        #endif

        default:
            break;
    }
}

void msdc_pad_ctrl_init(struct mmc_host *host)
{
    u32 tmpReg;

    switch (host->id) {
        case 0:
            #if defined(FEATURE_MMC_BOOT_MODE)
            MSDC_SET_FIELD(MSDC0_PULL_EN_CFG_BASE, 0x1FFFFF, 0x155556);

            //
            // set pull_sel cmd/dat/rst. (designer comment: when rstb switch to msdc mode, need gpio pull up to drive high)
            //
            MSDC_SET_FIELD(MSDC0_PULL_SEL_CFG_BASE, MSDC0_PULL_SEL_ALL_MASK,
                     MSDC0_CMD_PULL_SEL  |
                     MSDC0_DAT0_PULL_SEL | MSDC0_DAT1_PULL_SEL | MSDC0_DAT2_PULL_SEL | MSDC0_DAT3_PULL_SEL |
                     MSDC0_DAT4_PULL_SEL | MSDC0_DAT5_PULL_SEL | MSDC0_DAT6_PULL_SEL | MSDC0_DAT7_PULL_SEL |
                     MSDC0_RST_PULL_SEL);

            //
            // enable SMT for glitch filter. Default is off
            //
            msdc_set_smt(host, 1);

            //
            // set clk, cmd, dat pad driving
            //
            msdc_set_driving(host, msdc_cap.clk_drv, msdc_cap.cmd_drv, msdc_cap.dat_drv);

            //
            // set rdsel/tdsel
            //
            msdc_set_rdtdsel(host,1);

            //
            // set msdc mode. (MC0_DAT2~6, MC0_RST)
            //
            MSDC_SET_FIELD(MSDC0_DAT2_PINMUX_ADDR,
                     MSDC0_DAT2_PINMUX_BITS | MSDC0_DAT3_PINMUX_BITS | MSDC0_DAT4_PINMUX_BITS | MSDC0_DAT5_PINMUX_BITS |
                     MSDC0_DAT6_PINMUX_BITS | MSDC0_DAT7_PINMUX_BITS | MSDC0_RST_PINMUX_BITS,
                     0x01111111);

            //
            // set msdc mode. (MC0_CK, MC0_CMD, MC0_DAT0, MC0_DAT1)
            //
            MSDC_SET_FIELD(MSDC0_CLK_PINMUX_ADDR,
                     MSDC0_CLK_PINMUX_BITS | MSDC0_CMD_PINMUX_BITS | MSDC0_DAT0_PINMUX_BITS | MSDC0_DAT1_PINMUX_BITS ,
                     0x1111);
            #else

            //
            // set pull_en to select 10K or 50K resistor. (sd default uses 50K for clk/cmd/dat)
            //
            MSDC_WRITE32(MSDC0_PULL_EN_CFG_SET,
                     MSDC0_CLK_PULL_EN  | MSDC0_CMD_PULL_EN  |
                     MSDC0_DAT0_PULL_EN | MSDC0_DAT1_PULL_EN | MSDC0_DAT2_PULL_EN | MSDC0_DAT3_PULL_EN);

            //
            // set pull_sel to pull up cmd/dat. pull low clk.
            //
            MSDC_WRITE32(MSDC0_PULL_SEL_CFG_CLR, MSDC0_CLK_PULL_SEL);
            MSDC_WRITE32(MSDC0_PULL_SEL_CFG_SET,
                    MSDC0_CMD_PULL_SEL  |
                    MSDC0_DAT0_PULL_SEL | MSDC0_DAT1_PULL_SEL | MSDC0_DAT2_PULL_SEL | MSDC0_DAT3_PULL_SEL);

            //
            // enable SMT for glitch filter. Default is off
            //
            msdc_set_smt(host, 1);

            //
            // set clk, cmd, dat pad driving
            //
            msdc_set_driving(host, msdc_cap.clk_drv, msdc_cap.cmd_drv, msdc_cap.dat_drv);

            //
            // set rdsel/tdsel
            //
            msdc_set_rdtdsel(host, 0);

            //
            // set gpio to msdc mode. (cmd/dat1/dat2/dat3 in GPIO_MODE7)
            //
            tmpReg = MSDC_READ32(MSDC0_CMD_PINMUX_ADDR);
            tmpReg &= (~(MSDC0_CMD_PINMUX_BITS  |
                 MSDC0_DAT1_PINMUX_BITS | MSDC0_DAT2_PINMUX_BITS | MSDC0_DAT3_PINMUX_BITS));

            tmpReg |= (MSDC0_CMD_PINMUX_VAL  |
                   MSDC0_DAT1_PINMUX_VAL | MSDC0_DAT2_PINMUX_VAL | MSDC0_DAT3_PINMUX_VAL);
            MSDC_WRITE32(MSDC0_CMD_PINMUX_ADDR, tmpReg);

            //
            // set gpio to msdc mode. (dat0 in GPIO_MODE6)
            //
            tmpReg = MSDC_READ32(MSDC0_DAT0_PINMUX_ADDR);
            tmpReg &= ~(MSDC0_DAT0_PINMUX_BITS);
            tmpReg |= (MSDC0_DAT0_PINMUX_VAL);
            MSDC_WRITE32(MSDC0_DAT0_PINMUX_ADDR, tmpReg);

            //
            // set gpio to msdc mode. (clk)
            //
            tmpReg = MSDC_READ32(MSDC0_CLK_PINMUX_ADDR);
            tmpReg &= ~(MSDC0_CLK_PINMUX_BITS);
            tmpReg |= (MSDC0_CLK_PINMUX_VAL);
            MSDC_WRITE32(MSDC0_CLK_PINMUX_ADDR, tmpReg);
            #endif

            break;

        #if defined(MMC_MSDC_DRV_CTP) || defined(FEATURE_MMC_MEM_PRESERVE_MODE)
        case 1:
            #if 1
            MSDC_SET_FIELD(MSDC1_IES_CFG_BASE, MSDC1_IES_CK_CM_DAT, 1); //Cool's solution for SD card: Enable IES bit
            //
            // set pull_en to for 50K resistor. (sd default uses 50K for clk/cmd/dat)
            //
            MSDC_WRITE32(MSDC1_PULL_EN_CFG_SET,
                     MSDC1_CLK_PULL_EN  | MSDC1_CMD_PULL_EN  |
                     MSDC1_DAT0_PULL_EN | MSDC1_DAT1_PULL_EN |
                     MSDC1_DAT2_PULL_EN | MSDC1_DAT3_PULL_EN);

            //
            // set pull_sel to pull up cmd/dat. pull low clk.
            //
            MSDC_WRITE32(MSDC1_PULL_SEL_CFG_CLR, MSDC1_CLK_PULL_SEL);
            MSDC_WRITE32(MSDC1_PULL_SEL_CFG_SET,
                 MSDC1_CMD_PULL_SEL  |
                 MSDC1_DAT0_PULL_SEL | MSDC1_DAT1_PULL_SEL |
                 MSDC1_DAT2_PULL_SEL | MSDC1_DAT3_PULL_SEL);

            //
            // enable SMT for glitch filter. Default is off
            //
            msdc_set_smt(host, 1);

            //
            // set clk, cmd, dat pad driving
            //

            msdc_set_driving(host, msdc_cap_removable.clk_drv, msdc_cap_removable.cmd_drv, msdc_cap_removable.dat_drv);

            //
            // set gpio to msdc mode. (cmd/dat0/dat1/dat2 in GPIO_MODE11)
            //
            tmpReg = MSDC_READ32(MSDC1_CMD_PINMUX_ADDR);
            tmpReg &= (~(MSDC1_CMD_PINMUX_BITS  |
                 MSDC1_DAT0_PINMUX_BITS | MSDC1_DAT1_PINMUX_BITS | MSDC1_DAT2_PINMUX_BITS));

            tmpReg |= (MSDC1_CMD_PINMUX_VAL  |
                   MSDC1_DAT0_PINMUX_VAL | MSDC1_DAT1_PINMUX_VAL | MSDC1_DAT2_PINMUX_VAL);
            MSDC_WRITE32(MSDC1_CMD_PINMUX_ADDR, tmpReg);

            //
            // set gpio to msdc mode. (dat3 in GPIO_MODE12)
            //
            tmpReg = MSDC_READ32(MSDC1_DAT3_PINMUX_ADDR);
            tmpReg &= ~(MSDC1_DAT3_PINMUX_BITS);
            tmpReg |= (MSDC1_DAT3_PINMUX_VAL);
            MSDC_WRITE32(MSDC1_DAT3_PINMUX_ADDR, tmpReg);

            //
            // set gpio to msdc mode. (clk)
            //
            tmpReg = MSDC_READ32(MSDC1_CLK_PINMUX_ADDR);
            tmpReg &= ~(MSDC1_CLK_PINMUX_BITS);
            tmpReg |= (MSDC1_CLK_PINMUX_VAL);
            MSDC_WRITE32(MSDC1_CLK_PINMUX_ADDR, tmpReg);
            break;
            #endif
        #endif

        default:
            break;
    }
}

#if defined(MSDC_WITH_DEINIT)
void msdc_save_pin_mux(struct mmc_host *host)
{
    //
    // io config right side.
    //
    host->default_reg[0] = MSDC_READ32(MSDC0_PULL_EN_CFG_BASE);
    host->default_reg[1] = MSDC_READ32(MSDC0_PULL_SEL_CFG_BASE);
    host->default_reg[2] = MSDC_READ32(MSDC0_SMT_CFG_BASE);
    host->default_reg[3] = MSDC_READ32(MSDC0_DRVING_BASE);

    //
    // gpio mode selection.
    //
    host->default_reg[4] = MSDC_READ32(MSDC0_CMD_PINMUX_ADDR);
    host->default_reg[5] = MSDC_READ32(MSDC0_DAT0_PINMUX_ADDR);
}

void msdc_restore_pin_mux(struct mmc_host *host)
{
    u32 tmpReg;

    tmpReg = MSDC_READ32(MSDC0_CMD_PINMUX_ADDR);
    tmpReg &= 0xFFFF0FFF;
    tmpReg |= (host->default_reg[4] & 0x0F000);//MSDC0_CLK_PINMUX_BITS
    MSDC_WRITE32(MSDC0_CMD_PINMUX_ADDR, tmpReg);

    tmpReg = MSDC_READ32(MSDC0_CMD_PINMUX_ADDR);
    tmpReg &= 0xFFF0F000;//(~(MSDC0_CMD_PINMUX_BITS|MSDC0_DAT0_PINMUX_BITS|MSDC0_DAT1_PINMUX_BITS|MSDC0_DAT2_PINMUX_BITS));
    tmpReg |= (host->default_reg[4] & 0xF0FFF);//& (MSDC0_CMD_PINMUX_BITS|MSDC0_DAT0_PINMUX_BITS|MSDC0_DAT1_PINMUX_BITS|MSDC0_DAT2_PINMUX_BITS));
    MSDC_WRITE32(MSDC0_CMD_PINMUX_ADDR, tmpReg);

    tmpReg = MSDC_READ32(MSDC0_DAT0_PINMUX_ADDR);
    tmpReg &= (~(MSDC0_DAT0_PINMUX_BITS));
    tmpReg |= (host->default_reg[5] & (MSDC0_DAT0_PINMUX_BITS));
    MSDC_WRITE32(MSDC0_DAT0_PINMUX_ADDR, tmpReg);

    tmpReg = MSDC_READ32(MSDC0_SMT_CFG_BASE);
    tmpReg &= (0xFFFFFFFE);
    tmpReg |= (host->default_reg[2] & (0x1));
    MSDC_WRITE32(MSDC0_SMT_CFG_BASE, tmpReg);

    tmpReg = MSDC_READ32(MSDC0_DRVING_BASE);
    tmpReg &= (~MSDC0_CK_CM_DAT_DRVING);
    tmpReg |= (host->default_reg[3] & (MSDC0_CK_CM_DAT_DRVING));
    MSDC_WRITE32(MSDC0_DRVING_BASE, tmpReg);

    tmpReg = MSDC_READ32(MSDC0_PULL_SEL_CFG_BASE);
    tmpReg &= (~(0x3F));
    tmpReg |= (host->default_reg[1] & (0x3F));
    MSDC_WRITE32(MSDC0_PULL_SEL_CFG_BASE, tmpReg);
}
#endif


void msdc_pin_clk_pud(struct mmc_host *host, int mode)
{
    switch(host->id){
    	case 0:
    	    MSDC_SET_FIELD(MSDC0_PULL_SEL_CFG_BASE, MSDC0_CLK_PULL_SEL, mode);
    	    break;
    	#if defined(MMC_MSDC_DRV_CTP) || defined(FEATURE_MMC_MEM_PRESERVE_MODE)
    	case 1:
    	    MSDC_SET_FIELD(MSDC1_PULL_SEL_CFG_BASE, MSDC1_CLK_PULL_SEL, mode);
    	    break;
    	#endif
    	default:
    	    break;
    }
}

void msdc_pin_pud(struct mmc_host *host, int mode)
{
    u32 reg;
    //mode: 1 -> pull up
    //      0 -> pull down

    switch(host->id){
    	case 0:
    	    if ( mode==1 )
    	        reg=MSDC0_PULL_SEL_CFG_SET;
    	    else
    	        reg=MSDC0_PULL_SEL_CFG_CLR;

    	    #if defined(FEATURE_MMC_BOOT_MODE)
        		MSDC_WRITE32(
        			MSDC0_PULL_SEL_CFG_SET,
        			MSDC0_CLK_PULL_SEL|MSDC0_CMD_PULL_SEL|
        			MSDC0_DAT0_PULL_SEL|MSDC0_DAT1_PULL_SEL|MSDC0_DAT2_PULL_SEL|MSDC0_DAT3_PULL_SEL|
        			MSDC0_DAT4_PULL_SEL|MSDC0_DAT5_PULL_SEL|MSDC0_DAT6_PULL_SEL|MSDC0_DAT7_PULL_SEL
        		    );
    	    #else
    	        MSDC_WRITE32(
        			MSDC0_PULL_SEL_CFG_SET,
        			MSDC0_CLK_PULL_SEL|MSDC0_CMD_PULL_SEL|
        			MSDC0_DAT0_PULL_SEL|MSDC0_DAT1_PULL_SEL|MSDC0_DAT2_PULL_SEL|MSDC0_DAT3_PULL_SEL
        		    );
    	    #endif
    	    break;
    	#if defined(MMC_MSDC_DRV_CTP) || defined(FEATURE_MMC_MEM_PRESERVE_MODE)
    	case 1:
            if ( mode==1 )
                reg=MSDC1_PULL_SEL_CFG_SET;
            else
                reg=MSDC1_PULL_SEL_CFG_CLR;

            // set pull_sel to pull up cmd/dat. pull low clk.
            MSDC_WRITE32(
                    reg,
                    MSDC1_CMD_PULL_SEL|
                    MSDC1_DAT0_PULL_SEL|MSDC1_DAT1_PULL_SEL|MSDC1_DAT2_PULL_SEL|MSDC1_DAT3_PULL_SEL
                    );
            MSDC_WRITE32(MSDC1_PULL_EN_CFG_SET,
                    MSDC1_CLK_PULL_EN|MSDC1_CMD_PULL_EN|
                    MSDC1_DAT0_PULL_EN|MSDC1_DAT1_PULL_EN|MSDC1_DAT2_PULL_EN|MSDC1_DAT3_PULL_EN);
            break;
        #endif
    	default:
    	    break;
	}
}

void msdc_pin_pnul(struct mmc_host *host, int mode)
{
    #if defined(MMC_MSDC_DRV_CTP)
    switch(host->id){
    	case 0:
    	    if ( !mode )
                #if defined(FEATURE_MMC_BOOT_MODE)
                MSDC_WRITE32(MSDC0_PULL_EN_CFG_CLR,
                    MSDC0_CLK_PULL10K_EN|MSDC0_CMD_PULL10K_EN|
                    MSDC0_DAT0_PULL10K_EN|MSDC0_DAT1_PULL10K_EN|MSDC0_DAT2_PULL10K_EN|MSDC0_DAT3_PULL10K_EN|
                    MSDC0_DAT4_PULL10K_EN|MSDC0_DAT5_PULL10K_EN|MSDC0_DAT6_PULL10K_EN|MSDC0_DAT7_PULL10K_EN);
                #else
                MSDC_WRITE32(MSDC0_PULL_EN_CFG_CLR,
                    MSDC0_CLK_PULL_EN|MSDC0_CMD_PULL_EN|
                    MSDC0_DAT0_PULL_EN|MSDC0_DAT1_PULL_EN|MSDC0_DAT2_PULL_EN|MSDC0_DAT3_PULL_EN);
                #endif
            break;
        case 1:
            if ( !mode )
                MSDC_WRITE32(MSDC1_PULL_EN_CFG_CLR,
                    MSDC1_CLK_PULL_EN|MSDC1_CMD_PULL_EN|
                    MSDC1_DAT0_PULL_EN|MSDC1_DAT1_PULL_EN|MSDC1_DAT2_PULL_EN|MSDC1_DAT3_PULL_EN);
			break;
    	default:
    	    break;
	}
	#endif
}


////////////////////////////////////////////////////////////////////////////////
//
// Config CLKSRC, BUS width, Sample edge, PIN pull selection
//
////////////////////////////////////////////////////////////////////////////////

void msdc_config_clksrc(struct mmc_host *host, clk_source_t clksrc)
{
#if !defined(__FPGA__)
    #if defined(MMC_MSDC_DRV_CTP)
    cg_clk_id clk_id;
    clkmux_id mux_id;
    char name[6];
    #endif

    host->clksrc = clksrc;
    host->clk    = msdc_src_clks[clksrc];
    #if MSDC_USE_CLKSRC_IN_DATCRC
    for(i=0; i<ARRAY_SIZE(msdc_clock_tune_seq); i++ ) {
        if ( msdc_clock_tune_seq[i]==clksrc ) {
            msdc_clock_tune_seq_index[host->id]=i;
            break;
        }
    }
    #endif

    #if defined(MMC_MSDC_DRV_PRELOADER) || defined(MMC_MSDC_DRV_LK)
    if ( host->id==0 )
        MSDC_SET_FIELD(CLK_MUX_SEL_ADDR, MSDC0_CLK_MUX_SEL, clksrc);
    else if ( host->id==1 )
        MSDC_SET_FIELD(CLK_MUX_SEL_ADDR, MSDC1_CLK_MUX_SEL, clksrc);
    #endif

    #if defined(MMC_MSDC_DRV_CTP)
    //
    // Top clock control.
    //
    if (host->id == 0) {
        mux_id = MT_CLKMUX_MSDC0_MUX_SEL;
        sprintf(name, "MSDC0");
    }
    else if (host->id == 1) {
        mux_id = MT_CLKMUX_MSDC1_MUX_SEL;
        sprintf(name, "MSDC1");
    }

    switch (clksrc) {
        case 0:
            clk_id = MT_CG_MPLL_D12;
            break;
        case 1:
            clk_id = MT_CG_MPLL_D10;
            break;
        case 2:
            clk_id = MT_CG_MPLL_D8;
            break;
        case 3:
            clk_id = MT_CG_UPLL_D7;
            break;
        case 4:
            clk_id = MT_CG_MPLL_D7;
            break;
        case 5:
            clk_id = MT_CG_MPLL_D8;
            break;
        case 6:
            clk_id = MT_CG_SYS_26M;
            break;
        case 7:
            clk_id = MT_CG_UPLL_D6;
            break;
        default:
            break;
    }

    clkmux_sel(mux_id, clk_id, name);

    //host->sclk   = MSDC_MIN_SCLK;
    #endif

#else
    u32 i;
    host->clksrc = clksrc; //non-sense in FPGA
    host->clk    = MSDC_SRC_CLK;
    #if MSDC_USE_CLKSRC_IN_DATCRC
    msdc_clock_tune_seq_index[host->id]=0;
    #endif

#endif
}

void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz)
{
    msdc_priv_t *priv = host->priv;
    u32 base = host->base;
    u32 mode;
    u32 div;
    u32 sclk;
    //u32 orig_clksrc = host->clksrc;

    //printf("[SD%d] SET_CLK original target: %dHz\n", host->id, hz);

    if (hz >= host->f_max) {
        hz = host->f_max;
    } else if (hz < host->f_min) {
        hz = host->f_min;
    }

    if (ddr) {
        mode = 0x2; /* ddr mode and use divisor */
        if (hz >= (host->clk >> 2)) {
            div  = 0;           /* mean div = 1/2 */
            sclk = host->clk >> 2; /* sclk = clk/div/2. 2: internal divisor */
        } else {
            div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
            sclk = (host->clk >> 2) / div;
            div  = (div >> 1);     /* since there is 1/2 internal divisor */
        }
    } else if (hz >= host->clk) {
        mode = 0x1; /* no divisor and divisor is ignored */
        div  = 0;
        sclk = host->clk;
    } else {
        mode = 0x0; /* use divisor */
        if (hz >= (host->clk >> 1)) {
            div  = 0;           /* mean div = 1/2 */
            sclk = host->clk >> 1; /* sclk = clk / 2 */
        } else {
            div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
            sclk = (host->clk >> 2) / div;
            //printf("host->clk(%d),hz(%d),div(%d),sclk(%d)\n",host->clk,hz,div,sclk);
        }
    }
    host->sclk = sclk;

#if defined(FEATURE_MMC_UHS1)
    //This section may be unnecessary if 1.8V and 3.3V can use the same driving strength
    //The necessity depend on IOCUP and HQA result.
    #if 0
    if (hz > 100000000 && mmc_card_uhs1(host->card)) {
        msdc_set_driving(host, msdc_cap.clk_18v_drv, msdc_cap.cmd_18v_drv, msdc_cap.dat_18v_drv);
        /* don't enable cksel for ddr mode */
        //if (mmc_card_ddr(host->card) == 0)
            //MSDC_SET_FIELD(MSDC_PATCH_BIT0,MSDC_CKGEN_MSDC_CK_SEL, 1);
    }
    #endif
#endif

    //msdc_config_clksrc(host, MSDC_CLKSRC_DEFAULT); //Comment out by Yuchi in 6589 JB

    /* set clock mode and divisor */
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD|MSDC_CFG_CKDIV, (mode << 8)|div);

    //Comment out by Cool: an invocation for setting propoer clock source may be needed before invoking msdc_config_clock()
    //msdc_config_clksrc(host, orig_clksrc);

    /* wait clock stable */
    while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));

    #if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
    if ( mode==1 )
        msdc_clock_divisor[host->id]=-1;
    else
        msdc_clock_divisor[host->id]=div;
    #endif

    printf("[SD%d] SET_CLK(%dkHz): SCLK(%dkHz) SRCLK(%dkHz) MODE(%d) DDR(%d) DIV(%d) DS(%d) RS(%d)\n",
        host->id, hz/1000, sclk/1000, host->clk/1000, mode, ddr > 0 ? 1 : 0, div, priv->dsmpl, priv->rsmpl);
}

void msdc_clock(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s clock \n", host->id, on ? "on" : "off", "host");
}

void msdc_config_bus(struct mmc_host *host, u32 width)
{
    u32 base = host->base;
    u32 val  = MSDC_READ32(SDC_CFG);

    val &= ~SDC_CFG_BUSWIDTH;

    switch (width) {
        case HOST_BUS_WIDTH_1:
            val |= (MSDC_BUS_1BITS << 16);
            //MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 16, 0);
            break;
        case HOST_BUS_WIDTH_4:
            val |= (MSDC_BUS_4BITS << 16);
            //MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 16, 1);
            break;
        case HOST_BUS_WIDTH_8:
            val |= (MSDC_BUS_8BITS << 16);
            //MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 16, 1);
            break;
        default:
            width = HOST_BUS_WIDTH_1;
            break;
    }
    MSDC_WRITE32(SDC_CFG, val);

    printf("[SD%d] Bus Width: %d\n", host->id, width);
}

void msdc_config_smpl(struct mmc_host *host, u8 dsmpl, u8 rsmpl)
{
    u32 base = host->base;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    /* set sampling edge */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, rsmpl);
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, dsmpl);

    /* wait clock stable */
    while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));

    priv->rsmpl = rsmpl;
    priv->dsmpl = dsmpl;
}

void msdc_config_pin(struct mmc_host *host, int mode)
{
    MSG(CFG, "[SD%d] Pins mode(%d), none(0), down(1), up(2), keep(3)\n", host->id, mode);

    switch (mode) {
        case MSDC_PIN_PULL_UP:
            msdc_pin_pud(host,1);
            break;
        case MSDC_PIN_PULL_DOWN:
            msdc_pin_pud(host,0);
            break;
        case MSDC_PIN_PULL_NONE:
        default:
            msdc_pin_pnul(host,0);
            break;
    }
}


#if defined(__FPGA__)
////////////////////////////////////////////////////////////////////////////////
//
// FPGA Power Control
//
////////////////////////////////////////////////////////////////////////////////
#define GPIO_INPUT_ADDR     (0x10001E80)
#define GPIO_OUTPUT_ADDR    (0x10001E84)
#define GPIO_DIR_ADDR       (0x10001E88)

#define GPIO_CARD_PWR       (0x1 << 8)
#define GPIO_LVL_PWR_18V    (0x1 << 9)
#define GPIO_LVL_PWR_33V    (0x1 << 10)
#define GPIO_L4_DIR         (0x1 << 11)

static u32 gpio_val = 0;

#define FPGA_GPIO_DEBUG
static void msdc_clr_gpio(u32 bits)
{
    u32 l_val = 0;
    //gpio_val &= ~mask;

    switch (bits){
        case GPIO_CARD_PWR:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_CARD_PWR, l_val);
            //printf("====GPIO_CARD_PWR====%d\n", l_val);
            if (0 == l_val){
                printf("check me! [clr]gpio for card pwr is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_CARD_PWR;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (GPIO_CARD_PWR &  MSDC_READ32(GPIO_OUTPUT_ADDR)){
                printf("clear card pwr:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val &= ~GPIO_CARD_PWR;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
            }
            break;
        case GPIO_LVL_PWR_18V:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_LVL_PWR_18V, l_val);
            //printf("====GPIO_LVL_PWR_18V====%d\n", l_val);
            if (0 == l_val){
                printf("check me! [clr]gpio for card 1.8 pwr is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_LVL_PWR_18V;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (GPIO_LVL_PWR_18V &  MSDC_READ32(GPIO_OUTPUT_ADDR)){
                printf("clear card 1.8v pwr:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val &= ~GPIO_LVL_PWR_18V;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        case GPIO_LVL_PWR_33V:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_LVL_PWR_33V, l_val);
            //printf("====GPIO_LVL_PWR_33V====%d\n", l_val);
            if (0 == l_val){
                printf("check me! gpio for card 3.3v pwr is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_LVL_PWR_33V;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (GPIO_LVL_PWR_33V &  MSDC_READ32(GPIO_OUTPUT_ADDR)){
                printf("clear card 3.3v pwr:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val &= ~GPIO_LVL_PWR_33V;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        case GPIO_L4_DIR:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_L4_DIR, l_val);
            //printf("====GPIO_L4_DIR====%d\n", l_val);
            if (0 == l_val){
                printf("check me! gpio for l4 dir is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_L4_DIR;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (GPIO_L4_DIR &  MSDC_READ32(GPIO_OUTPUT_ADDR)){
                printf("clear l4 dir:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val &= ~GPIO_L4_DIR;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        default:
            printf("[%s:%d]invalid value: 0x%x\n", __FILE__, __func__, bits);
            break;
    }

    #ifdef FPGA_GPIO_DEBUG
    {
        u32 val = 0;
        val = MSDC_READ32(GPIO_INPUT_ADDR);
        printf("[clr]GPIO_INPUT_ADDR[8-11] :0x%x\n", val);
        val = MSDC_READ32(GPIO_OUTPUT_ADDR);
        printf("[clr]GPIO_OUTPUT_ADDR[8-11]:0x%x\n", val);
        val = MSDC_READ32(GPIO_DIR_ADDR);
        printf("[clr]GPIO_DIR[8-11]        :0x%x\n", val);
    }
    #endif
}

static void msdc_set_gpio(u32 bits)
{
    u32 l_val = 0;
    gpio_val |= bits;

    switch (bits){
        case GPIO_CARD_PWR:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_CARD_PWR, l_val);
            //printf("====GPIO_CARD_PWR====%d\n", l_val);
            if (0 == l_val){
                printf("check me! [set]gpio for card pwr is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_CARD_PWR;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (0 == (GPIO_CARD_PWR &  MSDC_READ32(GPIO_OUTPUT_ADDR))){
                printf("set card pwr:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val |= GPIO_CARD_PWR;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        case GPIO_LVL_PWR_18V:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_LVL_PWR_18V, l_val);
            //printf("====GPIO_LVL_PWR_18V====%d\n", l_val);
            if (0 == l_val){
                printf("check me! gpio for card 1.8v pwr is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_LVL_PWR_18V;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (0 == (GPIO_LVL_PWR_18V &  MSDC_READ32(GPIO_OUTPUT_ADDR))){
                printf("set card 1.8v pwr:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val |= GPIO_LVL_PWR_18V;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        case GPIO_LVL_PWR_33V:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_LVL_PWR_33V, l_val);
            //printf("====GPIO_LVL_PWR_33V====%d\n", l_val);
            if (0 == l_val){
                printf("check me! gpio for card 3.3v pwr is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_LVL_PWR_33V;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (0 == (GPIO_LVL_PWR_33V &  MSDC_READ32(GPIO_OUTPUT_ADDR))){
                printf("set card 3.3v pwr:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val |= GPIO_LVL_PWR_33V;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        case GPIO_L4_DIR:
            MSDC_GET_FIELD(GPIO_DIR_ADDR, GPIO_L4_DIR, l_val);
            //printf("====GPIO_L4_DIR====%d\n", l_val);
            if (0 == l_val){
                printf("check me! gpio for l4 dir is input\n");
                l_val = MSDC_READ32(GPIO_DIR_ADDR);
                l_val |= GPIO_L4_DIR;
                MSDC_WRITE32(GPIO_DIR_ADDR, l_val);
            }

            /* check for set before */
            if (0 == (GPIO_L4_DIR &  MSDC_READ32(GPIO_OUTPUT_ADDR))){
                printf("set l4 dir:\n");
                l_val = MSDC_READ32(GPIO_OUTPUT_ADDR);
                l_val |= GPIO_L4_DIR;
                MSDC_WRITE32(GPIO_OUTPUT_ADDR, l_val);
            }
            break;
        default:
            printf("[%s:%d]invalid value: 0x%x\n", __FILE__, __func__, bits);
            break;
    }

    #ifdef FPGA_GPIO_DEBUG
    {
        u32 val = 0;
        val = MSDC_READ32(GPIO_INPUT_ADDR);
        printf("[set]GPIO_INPUT_ADDR[8-11] :0x%x\n", val);
        val = MSDC_READ32(GPIO_OUTPUT_ADDR);
        printf("[set]GPIO_OUTPUT_ADDR[8-11]:0x%x\n", val);
        val = MSDC_READ32(GPIO_DIR_ADDR);
        printf("[set]GPIO_DIR[8-11]        :0x%x\n", val);
    }
    #endif

}

void msdc_triger_gpio(void)
{
    msdc_set_gpio(GPIO_CARD_PWR);
    msdc_set_gpio(GPIO_LVL_PWR_33V);
}

void msdc_set_host_level_pwr(struct mmc_host *host, u32 on, u32 level)
{
    //Parameter host is currently not used. Reserve it for future usage

    // GPO[3:2] = {LVL_PWR33, LVL_PWR18};
    msdc_clr_gpio(GPIO_LVL_PWR_18V);
    msdc_clr_gpio(GPIO_LVL_PWR_33V);

    if ( on ) {
        if (level)
            msdc_set_gpio(GPIO_LVL_PWR_18V);
        else
            msdc_set_gpio(GPIO_LVL_PWR_33V);
    }

    //add for fpga debug
    msdc_set_gpio(GPIO_L4_DIR);
}

void msdc_set_card_pwr(int on)
{
    if (on){
#if MSDC_USE_EMMC45_POWER
        msdc_set_gpio(GPIO_CARD_PWR);
        msdc_set_gpio(GPIO_LVL_PWR_18V);
#else
        msdc_set_gpio(GPIO_CARD_PWR);
        msdc_set_gpio(GPIO_LVL_PWR_33V);
#endif
        /* add for fpga debug */
        msdc_set_gpio(GPIO_L4_DIR);
    } else {
        msdc_clr_gpio(GPIO_CARD_PWR);
        msdc_clr_gpio(GPIO_LVL_PWR_33V);
        msdc_clr_gpio(GPIO_LVL_PWR_18V);

        /* add for fpga debug */
        msdc_clr_gpio(GPIO_L4_DIR);
    }
    mdelay(10);
}

#else
////////////////////////////////////////////////////////////////////////////////
//
// ASIC Power Control
//
////////////////////////////////////////////////////////////////////////////////
//Power id default on when power on, therefore Preloader/LK need not control power
#if defined(MMC_MSDC_DRV_CTP)
typedef enum MT65XX_POWER_VOL_TAG
{
    VOL_DEFAULT,
    VOL_0900 = 900,
    VOL_1000 = 1000,
    VOL_1100 = 1100,
    VOL_1200 = 1200,
    VOL_1300 = 1300,
    VOL_1350 = 1350,
    VOL_1500 = 1500,
    VOL_1800 = 1800,
    VOL_2000 = 2000,
    VOL_2100 = 2100,
    VOL_2500 = 2500,
    VOL_2800 = 2800,
    VOL_3000 = 3000,
    VOL_3300 = 3300,
    VOL_3400 = 3400,
    VOL_3500 = 3500,
    VOL_3600 = 3600
} MT65XX_POWER_VOLTAGE;

typedef enum MT65XX_POWER_TAG {

	//MT6323 Digital LDO
	MT6323_POWER_LDO_VIO28=0,
	MT6323_POWER_LDO_VUSB,
	MT6323_POWER_LDO_VMC,
	MT6323_POWER_LDO_VMCH,
	MT6323_POWER_LDO_VEMC_3V3,
	MT6323_POWER_LDO_VGP1,
	MT6323_POWER_LDO_VGP2,
	MT6323_POWER_LDO_VGP3,
	MT6323_POWER_LDO_VCN_1V8,
	MT6323_POWER_LDO_VSIM1,
	MT6323_POWER_LDO_VSIM2,
	MT6323_POWER_LDO_VRTC,
	MT6323_POWER_LDO_VCAM_AF,
	MT6323_POWER_LDO_VIBR,
	MT6323_POWER_LDO_VM,
	MT6323_POWER_LDO_VRF18,
	MT6323_POWER_LDO_VIO18,
	MT6323_POWER_LDO_VCAMD,
	MT6323_POWER_LDO_VCAM_IO,

	//MT6323 Analog LDO
	MT6323_POWER_LDO_VTCXO,
	MT6323_POWER_LDO_VA,
	MT6323_POWER_LDO_VCAMA,
	MT6323_POWER_LDO_VCN33,
	MT6323_POWER_LDO_VCN28,

	//MT6320 Digital LDO
	MT65XX_POWER_LDO_VIO28,
	MT65XX_POWER_LDO_VUSB,
	MT65XX_POWER_LDO_VMC1,
	MT65XX_POWER_LDO_VMCH1,
	MT65XX_POWER_LDO_VEMC_3V3,
	MT65XX_POWER_LDO_VEMC_1V8,
	MT65XX_POWER_LDO_VGP1,
	MT65XX_POWER_LDO_VGP2,
	MT65XX_POWER_LDO_VGP3,
	MT65XX_POWER_LDO_VGP4,
	MT65XX_POWER_LDO_VGP5,
	MT65XX_POWER_LDO_VGP6,
	MT65XX_POWER_LDO_VSIM1,
	MT65XX_POWER_LDO_VSIM2,
	MT65XX_POWER_LDO_VIBR,
	MT65XX_POWER_LDO_VRTC,
	MT65XX_POWER_LDO_VAST,

	//MT6320 Analog LDO
	MT65XX_POWER_LDO_VRF28,
	MT65XX_POWER_LDO_VRF28_2,
	MT65XX_POWER_LDO_VTCXO,
	MT65XX_POWER_LDO_VTCXO_2,
	MT65XX_POWER_LDO_VA,
	MT65XX_POWER_LDO_VA28,
	MT65XX_POWER_LDO_VCAMA,

	MT65XX_POWER_COUNT_END,
	MT65XX_POWER_NONE = -1
} MT65XX_POWER;

void msdc_pmic_VEMC_3V3_sel(MT65XX_POWER_VOLTAGE volt)
{
    if(volt == VOL_3000)    { upmu_set_rg_vemc_3v3_vosel(0);}
    else if(volt == VOL_3300)    { upmu_set_rg_vemc_3v3_vosel(1);}
    else{ printf("Not support to Set VEMC_3V3 power to %d\n", volt);}
}

void msdc_pmic_VMC_sel(MT65XX_POWER_VOLTAGE volt)
{
    if(volt == VOL_3300) { upmu_set_rg_vmc_vosel(1);}
    else if(volt == VOL_1800) { upmu_set_rg_vmc_vosel(0);}
    else{ printf("Not support to Set VMC1 power to %d\n", volt);}
}

void msdc_pmic_VMCH_sel(MT65XX_POWER_VOLTAGE volt)
{
    if(volt == VOL_3000)  { upmu_set_rg_vmch_vosel(0);}
    else if(volt == VOL_3300)  { upmu_set_rg_vmch_vosel(1);}
    else{ printf("Not support to Set VMCH1 power to %d\n", volt);}
}

u32 hwPowerOn(MT65XX_POWER powerId, MT65XX_POWER_VOLTAGE powerVolt)
{
    switch (powerId){
        //case MT6323_POWER_LDO_VEMC_1V8:
        //    msdc_pmic_VEMC_1V8_sel(powerVolt);
        //    upmu_set_rg_vemc_1v8_en(1);
        //    break;
        case MT6323_POWER_LDO_VEMC_3V3:
            msdc_pmic_VEMC_3V3_sel(powerVolt);
            upmu_set_rg_vemc_3v3_en(1);
            break;
        case MT6323_POWER_LDO_VMC:
            msdc_pmic_VMC_sel(powerVolt);
            upmu_set_rg_vmc_en(1);
            break;
        case MT6323_POWER_LDO_VMCH:
            msdc_pmic_VMCH_sel(powerVolt);
            upmu_set_rg_vmch_en(1);
            break;
        //case MT65XX_POWER_LDO_VGP6:
        //    msdc_pmic_VGP6_sel(powerVolt);
        //    upmu_set_rg_vgp6_en(1);
        //    break;
        default:
            printf("Not support to Set %d power on\n", powerId);
            break;
    }

    CTP_Wait_msec(100); /* requires before voltage stable */

    return 0;
}

u32 hwPowerDown(MT65XX_POWER powerId)
{
    switch (powerId){
        case MT6323_POWER_LDO_VEMC_3V3:
            upmu_set_rg_vemc_3v3_en(0);
            break;
        case MT6323_POWER_LDO_VMC:
            upmu_set_rg_vmc_en(0);
            break;
        case MT6323_POWER_LDO_VMCH:
            upmu_set_rg_vmch_en(0);
            break;
        default:
            printf("Not support to Set %d power down\n", powerId);
            break;
    }
    return 0;
}

static u32 msdc_ldo_power(u32 on, MT65XX_POWER powerId, MT65XX_POWER_VOLTAGE powerVolt, u32 *status)
{
    if (on) { // want to power on
        if (*status == 0) {  // can power on
            printf("msdc LDO<%d> power on<%d>\n", powerId, powerVolt);
            hwPowerOn(powerId, powerVolt);
            *status = powerVolt;
        } else if (*status == powerVolt) {
            printf("msdc LDO<%d><%d> power on again!\n", powerId, powerVolt);
        } else { // for sd3.0 later
            printf("msdc LDO<%d> change<%d> to <%d>\n", powerId, *status, powerVolt);
            hwPowerDown(powerId);
            hwPowerOn(powerId, powerVolt);
            *status = powerVolt;
        }
    } else {  // want to power off
        if (*status != 0) {  // has been powerred on
            printf("msdc LDO<%d> power off\n", powerId);
            hwPowerDown(powerId);
            *status = 0;
        } else {
            printf("LDO<%d> not power on\n", powerId);
        }
    }

    return 0;
}

u32 g_msdc0_io;
u32 g_msdc1_io;
u32 g_msdc0_flash;
u32 g_msdc1_flash;

void msdc_set_host_level_pwr(struct mmc_host *host, u32 on, u32 level)
{
    switch (host->id) {

        case 0:
            //do nothing
            break;
        case 1:
            msdc_ldo_power(on, MT6323_POWER_LDO_VMC, VOL_1800, &g_msdc1_io);
            //msdc_ldo_power(on, MT6323_POWER_LDO_VMCH, VOL_3300, &g_msdc1_flash);
            //
            // to do. set driving and rdtdsel.
            //
            break;

        default:
            break;

    }
}
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Power Control -- Common for ASIC and FPGA
//
////////////////////////////////////////////////////////////////////////////////
void msdc_set_host_pwr(struct mmc_host *host, int on)
{
    #if !defined(__FPGA__)
        #if  defined(MMC_MSDC_DRV_CTP)
        switch(host->id){
            case 0:
                //do nothing since it is always on
                break;
            case 1:
                msdc_ldo_power(on, MT6323_POWER_LDO_VMC, VOL_3300, &g_msdc1_io);
                break;
            default:
                break;
        }
        mdelay(50);
        #endif
    #else
        msdc_set_host_level_pwr(host, on, 0);
    #endif
}

void msdc_host_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "host");

    if (on) {
        msdc_config_pin(host, MSDC_PIN_PULL_UP);
        msdc_set_host_pwr(host, 1);
        msdc_clock(host, 1);
    } else {
        msdc_clock(host, 0);
        msdc_set_host_pwr(host, 0);
        msdc_config_pin(host, MSDC_PIN_PULL_DOWN);
    }
}

void msdc_card_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn1 %s %s power \n", host->id, on ? "on" : "off", "card");

    //Note: host is not used

    #if !defined(__FPGA__)
        #if defined(MMC_MSDC_DRV_CTP)
        switch(host->id) {
            case 0:
                //Do nothing since it is always on
                break;
            case 1:
                msdc_ldo_power(on, MT6323_POWER_LDO_VMCH, VOL_3300, &g_msdc1_flash);
                mdelay(10);
                break;
            default:
                break;
        }
        #endif
    #else
        switch(host->id) {
            case 0:
                if (on) {
                    msdc_set_card_pwr(1);
                } else {
                    msdc_set_card_pwr(0);
                }
                mdelay(10);
                break;
            default:
                //No MSDC1 in FPGA
                break;
        }
    #endif
}

void msdc_power(struct mmc_host *host, u8 mode)
{
    if (mode == MMC_POWER_ON || mode == MMC_POWER_UP) {
        msdc_host_power(host, 1);
        msdc_card_power(host, 1);
    } else {
        msdc_card_power(host, 0);
        msdc_host_power(host, 0);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Command/Response, Abort
//
////////////////////////////////////////////////////////////////////////////////
void msdc_intr_unmask(struct mmc_host *host, u32 bits)
{
    u32 base = host->base;
    u32 val;

    val  = MSDC_READ32(MSDC_INTEN);
    val |= bits;
    MSDC_WRITE32(MSDC_INTEN, val);
}

void msdc_intr_mask(struct mmc_host *host, u32 bits)
{
    u32 base = host->base;
    u32 val;

    val  = MSDC_READ32(MSDC_INTEN);
    val &= ~bits;
    MSDC_WRITE32(MSDC_INTEN, val);
}


int msdc_send_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base   = host->base;
    u32 opcode = cmd->opcode;
    u32 rsptyp = cmd->rsptyp;
    u32 rawcmd;
    u32 timeout = cmd->timeout;
    u32 error = MMC_ERR_NONE;

    /* rawcmd :
     * vol_swt << 30 | auto_cmd << 28 | blklen << 16 | go_irq << 15 |
     * stop << 14 | rw << 13 | dtype << 11 | rsptyp << 7 | brk << 6 | opcode
     */
    rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) |
            msdc_rsp[rsptyp] << 7 | host->blklen << 16;

    if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
        rawcmd |= ((2 << 11) | (1 << 13));
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if ( opcode == MMC_CMD_WRITE_BLOCK || opcode == MMC_CMD50 ) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK
                || opcode == SD_ACMD_SEND_SCR
                || opcode == SD_CMD_SWITCH
                || opcode == MMC_CMD_SEND_EXT_CSD
                || opcode == MMC_CMD_SEND_WRITE_PROT      //For MMC and SDSC only
                || opcode == MMC_CMD_SEND_WRITE_PROT_TYPE //For MMC only
                || opcode == MMC_CMD21
                ) {
        rawcmd |= (1 << 11);
    } else if (opcode == MMC_CMD_STOP_TRANSMISSION) {
        rawcmd |= (1 << 14);
        rawcmd &= ~(0x0FFF << 16);
    #if defined(FEATURE_MMC_UHS1)
    } else if (opcode == SD_CMD_VOL_SWITCH) {
        rawcmd |= (1 << 30);
    #endif
    } else if (opcode == SD_CMD_SEND_TUNING_BLOCK) {
        rawcmd |= (1 << 11); /* CHECKME */
        if (priv->autocmd & MSDC_AUTOCMD19)
            rawcmd |= (3 << 28);
    } else if (opcode == MMC_CMD_GO_IRQ_STATE) {
        rawcmd |= (1 << 15);
    #if defined(MSDC_USE_MMC_STREAM)
    } else if (opcode == MMC_CMD_WRITE_DAT_UNTIL_STOP) {
        rawcmd |= ((1<< 13) | (3 << 11));
    } else if (opcode == MMC_CMD_READ_DAT_UNTIL_STOP) {
        rawcmd |= (3 << 11);
    #endif
    #if defined(FEATURE_MMC_SDIO)
    } else if (opcode == SD_IO_RW_EXTENDED) {
        if (cmd->arg & 0x80000000)  /* R/W flag */
            rawcmd |= (1 << 13);
        if ((cmd->arg & 0x08000000) && ((cmd->arg & 0x1FF) > 1))
            rawcmd |= (2 << 11); /* multiple block mode */
        else
            rawcmd |= (1 << 11);
    } else if (opcode == SD_IO_RW_DIRECT) {
        if ((cmd->arg & 0x80000000) && ((cmd->arg >> 9) & 0x1FFFF))/* I/O abt */
            rawcmd |= (1 << 14);
    #endif
    }

    MSG(CMD, "[SD%d] CMD(%d): ARG(0x%x), RAW(0x%x), RSP(%d)\n",
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), cmd->arg, rawcmd, rsptyp);

    if (!priv->stream_stop) {
        /* FIXME. Need to check if SDC is busy before data read/write transfer */
        if (opcode == MMC_CMD_SEND_STATUS) {
            if (SDC_IS_CMD_BUSY()) {
                WAIT_COND(!SDC_IS_CMD_BUSY(), cmd->timeout, timeout);
                if (timeout == 0) {
                    error = MMC_ERR_TIMEOUT;
                    printf("[SD%d] CMD(%d): SDC_IS_CMD_BUSY timeout\n",
                        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
                    goto end;
                }
            }
        } else {
            if (SDC_IS_BUSY()) {
                WAIT_COND(!SDC_IS_BUSY(), 1000, timeout);
                if (timeout == 0) {
                    error = MMC_ERR_TIMEOUT;
                    printf("[SD%d] CMD(%d): SDC_IS_BUSY timeout\n",
                        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
                    goto end;
                }
            }
        }
    }
    //printf("Before SEND CMD-------------------------------\n");
    //msdc_dump_register(host);
    SDC_SEND_CMD(rawcmd, cmd->arg);
    //printf("After SEND CMD--------------------------------\n");
    //msdc_dump_register(host);

end:
    cmd->error = error;

    return error;
}

int msdc_wait_rsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base   = host->base;
    u32 rsptyp = cmd->rsptyp;
    u32 status;
    u32 opcode = (cmd->opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT));
    u32 error = MMC_ERR_NONE;
    u32 wints = MSDC_INT_CMDTMO | MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR |
                MSDC_INT_ACMDRDY | MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO |
                MSDC_INT_ACMD19_DONE;

    if (cmd->opcode == MMC_CMD_GO_IRQ_STATE)
        wints |= MSDC_INT_MMCIRQ;

    //status = msdc_intr_wait(host, wints, cmd);
    status = msdc_intr_wait(host, wints);

    if (status == 0) {
        error = MMC_ERR_TIMEOUT;
        goto end;
    }

    if ((status & MSDC_INT_CMDRDY) || (status & MSDC_INT_ACMDRDY) ||
        (status & MSDC_INT_ACMD19_DONE)) {
        u32 *resp = &cmd->resp[0];

        switch (rsptyp) {
            case RESP_NONE:
                MSG(RSP, "[SD%d] CMD(%d): RSP(%d)\n", host->id, opcode, rsptyp);
                break;
            case RESP_R2:
                *resp++ = MSDC_READ32(SDC_RESP3);
                *resp++ = MSDC_READ32(SDC_RESP2);
                *resp++ = MSDC_READ32(SDC_RESP1);
                *resp++ = MSDC_READ32(SDC_RESP0);
                MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x 0x%x 0x%x 0x%x\n",
                    host->id, opcode, cmd->rsptyp, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
                break;
            default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
                if ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE))
                    *resp = MSDC_READ32(SDC_ACMD_RESP);
                else
                    *resp = MSDC_READ32(SDC_RESP0);
                MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x AUTO(%d)\n", host->id, opcode, cmd->rsptyp, *resp,
                    ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE)) ? 1 : 0);
                break;
        }
    } else if ((status & MSDC_INT_RSPCRCERR) || (status & MSDC_INT_ACMDCRCERR)) {
        error = MMC_ERR_BADCRC;
        MSG(RSP, "[SD%d] CMD(%d): RSP(%d) ERR(BADCRC)\n",
            host->id, opcode, cmd->rsptyp);
    } else if ((status & MSDC_INT_CMDTMO) || (status & MSDC_INT_ACMDTMO)) {
        error = MMC_ERR_TIMEOUT;
        MSG(RSP, "[SD%d] CMD(%d): RSP(%d) ERR(CMDTO) AUTO(%d)\n",
            host->id, opcode, cmd->rsptyp, status & MSDC_INT_ACMDTMO ? 1: 0);
    } else {
        error = MMC_ERR_INVALID;
        MSG(RSP, "[SD%d] CMD(%d): RSP(%d) ERR(INVALID), Status:%x\n",
            host->id, opcode, cmd->rsptyp, status);
    }

end:

    //Light: This seems unnecessary since R1B is implied by RDY
    if (rsptyp == RESP_R1B) {
        while ((MSDC_READ32(MSDC_PS) & 0x10000) != 0x10000);
    }

#if MMC_DEBUG
    if ((error == MMC_ERR_NONE) && (MSG_EVT_MASK & MSG_EVT_RSP)){
        switch(cmd->rsptyp) {
            case RESP_R1:
            case RESP_R1B:
                mmc_dump_card_status(cmd->resp[0]);
                break;
            case RESP_R3:
                mmc_dump_ocr_reg(cmd->resp[0]);
                break;
            case RESP_R5:
                mmc_dump_io_resp(cmd->resp[0]);
                break;
            case RESP_R6:
                mmc_dump_rca_resp(cmd->resp[0]);
                break;
        }
    }
#endif

    cmd->error = error;

    //Light: Comment out 20130203; Moved to mmc_cmd()
    /*
    if (cmd->opcode == MMC_CMD_APP_CMD && error == MMC_ERR_NONE) {
        host->app_cmd = 1;
        host->app_cmd_arg = cmd->arg;
    }
    else
        host->app_cmd = 0;
    */

    return error;
}

static int msdc_get_card_status(struct mmc_host *host, u32 *status)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_STATUS;
    cmd.arg = host->card->rca << 16;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = msdc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE) {
        *status = cmd.resp[0];
    }
    return err;
}

int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    int err;

    err = msdc_send_cmd(host, cmd);
    if (err != MMC_ERR_NONE)
        return err;

    err = msdc_wait_rsp(host, cmd);
    if (err == MMC_ERR_BADCRC) {
        u32 base = host->base;
        u32 tmp = MSDC_READ32(SDC_CMD);

        /* check if data is used by the command or not */
        if (tmp & SDC_CMD_DTYP) {
            msdc_abort_handler(host, 1);
        }

        #if defined(FEATURE_MMC_CM_TUNING)
        //Light: For CMD17/18/24/25, tuning may have been done by
        //       msdc_abort_handler()->msdc_get_card_status()->msdc_cmd() for CMD13->msdc_tune_cmdrsp().
        //       This means that 2nd invocation of msdc_tune_cmdrsp() occurs here!
        //--> To Do: consider if 2nd invocation can be avoid
        if ( host->app_cmd!=2 ) //Light 20121225, to prevent recursive call path: msdc_tune_cmdrsp->msdc_app_cmd->msdc_cmd->msdc_tune_cmdrsp

            err = msdc_tune_cmdrsp(host, cmd);
        #endif
    }
    return err;
}

int msdc_cmd_stop(struct mmc_host *host, struct mmc_command *cmd)
{
    struct mmc_command stop;

    if (mmc_card_mmc(host->card) && (cmd) && (cmd->opcode == 18))
        stop.rsptyp  = RESP_R1;
    else
        stop.rsptyp  = RESP_R1B;
    stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
    stop.arg     = 0;
    stop.retries = CMD_RETRIES;
    stop.timeout = CMD_TIMEOUT;

    return msdc_cmd(host, &stop);
}

#if defined(FEATURE_MMC_SDIO)
int msdc_cmd_io_abort(struct mmc_host *host)
{
    struct mmc_command abort;

    memset(&abort, 0, sizeof(struct mmc_command));
    abort.opcode = SD_IO_RW_DIRECT;
    abort.arg    = 0x80000000;        /* write */
    abort.arg   |= 0 << 28;           /* function 0 */
    abort.arg   |= SDIO_CCCR_ABORT << 9;  /* address */
    abort.arg   |= 0;             /* abort function 0 */
    abort.rsptyp = RESP_R1B;
    abort.retries = CMD_RETRIES;
    abort.timeout = CMD_TIMEOUT;

    return msdc_cmd(host, &abort);
}
#endif

//////////////////////////////////////////////
//
//For re-sending APP cmd when tuning cmd/rsp is performed
//
//////////////////////////////////////////////
static int msdc_app_cmd(struct mmc_host *host)
{
    struct mmc_command appcmd;
    int err = MMC_ERR_NONE;
    int retries = 10; //Why not CMD_RETRIES is used?
    appcmd.opcode  = MMC_CMD_APP_CMD;
    appcmd.arg     = host->app_cmd_arg;
    appcmd.rsptyp  = RESP_R1;
    appcmd.retries = CMD_RETRIES;
    appcmd.timeout = CMD_TIMEOUT;

    //Light 20121225, to prevent recursive call path: msdc_tune_cmdrsp->msdc_app_cmd->msdc_cmd->msdc_tune_cmdrsp
    host->app_cmd=2;

    do {
        err = msdc_cmd(host, &appcmd);
        if (err == MMC_ERR_NONE)
            break;
    } while (retries--);

    //Light 20130125, Recover app_cmd
    host->app_cmd=1;
    return err;
}

void msdc_brk_cmd(struct mmc_host *host)
{
    u32 base = host->base;

    SDC_SEND_CMD(0x000000e8, 0);
}

void msdc_abort(struct mmc_host *host)
{
    u32 base = host->base;

    //printf("[SD%d] Abort: MSDC_FIFOCS=%xh MSDC_PS=%xh SDC_STS=%xh\n",
       // host->id, MSDC_READ32(MSDC_FIFOCS), MSDC_READ32(MSDC_PS), MSDC_READ32(SDC_STS));

    /* reset controller */
    MSDC_RESET();

    /* clear fifo */
    MSDC_CLR_FIFO();

    /* make sure txfifo and rxfifo are empty */
    if (MSDC_TXFIFOCNT() != 0 || MSDC_RXFIFOCNT() != 0) {
        printf("[SD%d] Abort: TXFIFO(%d), RXFIFO(%d) != 0\n",
            host->id, MSDC_TXFIFOCNT(), MSDC_RXFIFOCNT());
    }

    /* clear all interrupts */
    MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
}

void msdc_abort_handler(struct mmc_host *host, int abort_card)
{
    u32 status = 0;
    u32 state = 0;
    u32 err;
    #if 0
    u32 count=0;
    #endif

    while (state != 4) { // until status to "tran"; //20130125 Comment out by Light
    //while ( abort_card ) { //20130125 Light
        msdc_abort(host);
        err=msdc_get_card_status(host, &status);
        //To do: move the following 2 if clause into msdc_get_card_status() or write as a function
        #if 0 //Light: turn if off before I verify it
        //#if defined(MMC_MSDC_DRV_CTP)
        if (err == MMC_ERR_BADCRC) {
            printf("[Err handle][%s:%d]cmd13 crc error\n", __func__, __LINE__);
            msdc_tune_update_cmdrsp(host, count++);

            if (count >= 512)
                count = 0;
        }

        if (err == MMC_ERR_TIMEOUT) {
            printf("[Err handle][%s:%d]cmd13 timeout\n", __func__, __LINE__);
            msdc_tune_update_cmdrsp(host, count++);

            if (count >= 512)
                count = 0;
        }
        #else
        if (err != MMC_ERR_NONE) {
            printf("[Err handle][%s:%d]cmd13 fail\n", __func__, __LINE__);
            goto out;
        }
        #endif

        state = R1_CURRENT_STATE(status);
        #if MMC_DEBUG
        mmc_dump_card_status(status);
        #endif

        printf("check card state<%d>\n", state);
        if (state == 5 || state == 6) {
            if (abort_card) {
                printf("state<%d> need cmd12 to stop\n", state);

                err=msdc_cmd_stop(host, NULL);

                //To do: move the following 2 if clause into msdc_cmd_stop() or write as a function
                #if 0 //Light: turn if off before I verify it
                //#if defined(MMC_MSDC_DRV_CTP)
                if (err == MMC_ERR_BADCRC) {
                    printf("[Err handle][%s:%d]cmd12 crc error\n", __func__, __LINE__);
                    msdc_tune_update_cmdrsp(host, count++);

                    if (count >= 512)
                        count = 0;

                    continue;
                }

                if (err == MMC_ERR_TIMEOUT) {
                    printf("[Err handle][%s:%d]cmd12 timeout\n", __func__, __LINE__);
                    msdc_tune_update_cmdrsp(host, count++);

                    if (count >= 512)
                        count = 0;

                    continue;
                }
                #else
                if (err != MMC_ERR_NONE) {
                    printf("[Err handle][%s:%d]cmd12 fail\n", __func__, __LINE__);
                    goto out;
                }
                #endif
            }
            //break;  //20130125 Light
        } else if (state == 7) {  // busy in programing
            printf("state<%d> card is busy\n", state);
            mdelay(100);
        } else if (state != 4) {
            printf("state<%d> ??? \n", state);
            goto out;
        }
    }

    msdc_abort(host);

    return;

out:
    printf("[SD%d] data abort failed\n",host->id);
}


////////////////////////////////////////////////////////////////////////////////
//
// PIO Data read/write
//
////////////////////////////////////////////////////////////////////////////////
extern int indata;

int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    #if defined(MMC_MSDC_DRV_CTP)
    u8 *ptr8;
    u16 *ptr16;
    #endif
    msdc_priv_t *priv = host->priv;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 100000;
    u32 status;
    u32 totalsz = size;
    u8  done = 0;
    u32 size_per_round;
    u32 dcrc;

    #if defined(MMC_MSDC_DRV_CTP)
    if (priv->pio_bits==32 ) {
        //left_orig=MSDC_FIFO_THD>>2;
        size=((size+3)>>2)<<2;      //Adjust size to multiple of 4 bytes
    } else  if (priv->pio_bits==16 )  {
        //left_orig=MSDC_FIFO_THD>>1;
        ptr16=(u16 *)ptr;
        size=((size+1)>>1)<<1;      //Adjust size to multiple of 2 bytes
    } else {
        //left_orig=MSDC_FIFO_THD;
        ptr8=(u8 *)ptr;
    }
    #else
    size=((size+3)>>2)<<2; //Shall be 32 bit PIO for Preloader/LK
    #endif

    while (1) {
#if defined(MSDC_USE_IRQ)
        //For CTP only
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif

        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n",
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left %d bytes\n",
                host->id, status, size);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left %d bytes, FIFOCS:%xh\n",
                host->id, status, size, MSDC_READ32(MSDC_FIFOCS));
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_ACMDCRCERR) {
            MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
            printf("[SD%d] AUTOCMD CRC error (0x%x), Left %d bytes, FIFOCS:%xh, dcrc:0x%x\n",
                host->id, status, size, MSDC_READ32(MSDC_FIFOCS), dcrc);
            err = MMC_ERR_ACMD_RSPCRC;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            done = 1;
        }

        if (size == 0 && done)
            break;

        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        //if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD))
        if ( size> 0 )
        {
            int left;
            if ( (size >= MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD) )
                left = MSDC_FIFO_THD;
            else if ( (size < MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= size) )
                left = size;
            else
                continue;

            size_per_round = left;

            #if defined(MMC_MSDC_DRV_CTP)
            if (priv->pio_bits==8 )  {
                do {
                    *ptr8++ = MSDC_FIFO_READ8();;
                    left--;
                } while (left);

            } else if (priv->pio_bits==16 )  {
                do {
                    *ptr16++ = MSDC_FIFO_READ16();
                    left-=2;
                } while (left);

            } else
            #endif
            { //if (priv->pio_bits==32 )
                do {
                    *ptr++ = MSDC_FIFO_READ32();
                    /*
                    if(*(--ptr)!=0x0707 && indata){
                        msdc_triger_gpio();
                        //printf("ptr(0x%x),*prt(0x%x)\n",ptr,*ptr);

                        err = MMC_ERR_BADCRC;
                        //return err;
                    }
                    ptr++;*/
                    left-=4;
                } while (left);
            }
            size -= size_per_round;

            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, size_per_round, MSDC_RXFIFOCNT(), size, totalsz);

        }

    }

    if (err != MMC_ERR_NONE) {
        msdc_abort(host); /* reset internal fifo and state machine */
        printf("[SD%d] %d-bit PIO Read Error (%d)\n", host->id,
            priv->pio_bits, err);
    }

    return err;
}

int msdc_pio_write(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u8 *ptr8=(u8 *)ptr;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    u32 status;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    #if defined(MSDC_USE_DMA_MODE)
    u8  stream = (priv->cfg.mode == MSDC_MODE_MMC_STREAM) ? 1 : 0;
    #endif
    u32 size_per_round;
    #if defined(MMC_MSDC_DRV_CTP)
    u32 left_orig;
    #endif

    #if defined(MMC_MSDC_DRV_CTP)
    if ( priv->pio_bits==32 ) {
        size=((size+3)>>2)<<2;      //Adjust size to multiple of 4 bytes
    } else if ( priv->pio_bits==16 ) {
        size=((size+1)>>1)<<1;      //Adjust size to multiple of 4 bytes
    } else { //if ( priv->pio_bits==8 )
        left_orig=MSDC_FIFO_SZ;
    }
    #else
    size=((size+3)>>2)<<2; //Shall be 32 bit PIO for Preloader/LK
    #endif

    while (1) {
#if defined(MSDC_USE_IRQ)
        //For CTP only
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n",
            host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left %d bytes\n",
                host->id, status, size);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left %d bytes, FIFOCS:%xh\n",
                host->id, status, size, MSDC_READ32(MSDC_FIFOCS));
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_ACMDCRCERR) {
            printf("[SD%d] AUTOCMD CRC error (0x%x), Left %d bytes, FIFOCS:%xh, dcrc:0x%x\n",
                host->id, status, size, MSDC_READ32(MSDC_FIFOCS), 0);
            err = MMC_ERR_ACMD_RSPCRC;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            if (size == 0) {
                MSG(OPS_MMC, "[SD%d] all data flushed to card\n", host->id);
                break;
            } else {
                MSG(WRN, "[SD%d]<CHECKME> XFER_COMPL before all data written\n",
                    host->id);
            }
        #if defined(MSDC_USE_DMA_MODE)
        } else if (stream) {
            if (MSDC_READ32(SDC_STS) & SDC_STS_SWR_COMPL)
                break;
            MSG(OPS, "[SD%d] Wait for stream write data flush\n", host->id);
        #endif
        }

        if (size == 0)
            continue;

        //if (size >= size_threshold_large)
        {
            if (MSDC_TXFIFOCNT() == 0) {

                int left;
                #if defined(MMC_MSDC_DRV_CTP)
                if ( priv->pio_bits==32 ) {
                    if ( size >= MSDC_FIFO_THD )
                        left = MSDC_FIFO_THD;
                    else
                        left = size;
                } else
                #endif
                {
                    if ( size >= MSDC_FIFO_SZ )
                        left = MSDC_FIFO_SZ;
                    else
                        left = size;
                }

                size_per_round = left;

                #if 0
                MSDC_WRITE32(0x10008040,0x31);
                MSDC_WRITE32(0x10008044,0x0);
                u32 test_timer1 = MSDC_READ32(0x10008048);
                udelay1(60);//for MT6583 DDR write timeout error verification test case
                u32 test_timer2 = MSDC_READ32(0x10008048);
                test_timer1 = (test_timer2-test_timer1)/6;
                printf("test udelay(%d us)\n",test_timer1);
                #endif

                #if defined(MMC_MSDC_DRV_CTP)
                if ( priv->pio_bits==8 ) {
                    do {
                        MSDC_FIFO_WRITE8(*ptr8); ptr8++;
                        left--;
                    } while (left);
                } else if ( priv->pio_bits==16 ) {
                    do {
                        MSDC_FIFO_WRITE16(*(u16*)ptr8); ptr8+=2;
                        left-=2;
                    } while (left);
                } else
                #endif
                { //if ( write_unit==4 )
                    do {
                        MSDC_FIFO_WRITE32(*(u32*)ptr8); ptr8+=4;
                        left-=4;
                    } while (left);
                }

                size -= size_per_round;
            }
        }
    }


    if (err != MMC_ERR_NONE) {
        msdc_abort(host); /* reset internal fifo and state machine */
        //printf("[SD%d] %d-bit PIO Write Error (%d)\n", host->id,
        //  priv->pio_bits, err);
    }

    return err;
}

#if defined(MMC_MSDC_DRV_PRELOADER)
#if defined(MSDC_USE_DMA_MODE)
int msdc_dma_send_sandisk_fwid(struct mmc_host *host, uchar *buf, u32 opcode, ulong nblks)
{
    int multi;
    struct mmc_command cmd;
    struct mmc_data data;

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Read data %d blks from 0x%x\n", host->id, nblks, buf);

    multi = nblks > 1 ? 1 : 0;

    /* send read command */
    cmd.opcode  = opcode;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    data.blks    = nblks;
    data.buf     = (u8*)buf;
    data.timeout = 100; /* 100ms */

    return msdc_dma_transfer(host, &cmd, &data);
}
#else
int msdc_pio_send_sandisk_fwid(struct mmc_host *host,uchar *buf, u32 opcode)
{
    u8 *read_write;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    struct mmc_command cmd;

    ulong *ptr = (ulong *)buf;

    if ( opcode==MMC_CMD50 ) {
        read_write="Write";
    }
    else if ( opcode==MMC_CMD21 ) {
        read_write="Read";
        msdc_set_timeout(host, 100000000, 0);
    }

    MSG(OPS, "[SD%d] %s data %d bytes to 0x%x\n", read_write, host->id, blksz, buf);

    MSDC_CLR_FIFO();
    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, blksz);

    /* send command */
    cmd.opcode  = opcode;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = 0;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    if ( opcode==MMC_CMD50 ) {
        err = derr = msdc_pio_write(host, (u32*)ptr, 1 * blksz);
    } else {
        err = derr = msdc_pio_read(host, (u32*)ptr, 1 * blksz);
    }

done:
    if (err != MMC_ERR_NONE) {
        if (derr != MMC_ERR_NONE) {
            printf("[SD%d] %s data error (%d)\n", read_write, host->id, derr);
            msdc_abort_handler(host, 1);
        } else {
            printf("[SD%d] %s error (%d)\n", read_write, host->id, err);
        }
    }
    return (derr == MMC_ERR_NONE) ? err : derr;
}
#endif
#endif

#if defined(FEATURE_MMC_SDIO)
#if 0
/*
int msdc_pio_iorw(struct mmc_card *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
*/
#else
int msdc_pio_iorw(struct mmc_card *card, int write,
    u8 *buf, unsigned blocks, unsigned blksz)
#endif
{
    int err;
    struct mmc_host *host = card->host;

    /* Comment out by moving to mmc_io_rw_extended()
    #if 0
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= fn << 28;
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= addr << 9;
    if (blocks == 1 && blksz <= 512) {
        cmd.arg |= (blksz == 512) ? 0 : blksz;    //byte mode
    } else {
        cmd.arg |= 0x08000000 | blocks;           //block mode
    }
    cmd.rsptyp  = RESP_R5;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    msdc_clr_fifo(host);
    msdc_set_blknum(host, blocks);
    msdc_set_blklen(host, blksz);
    err = msdc_cmd(host, &cmd);

    if (err)
        return err;

    if (cmd.resp[0] & R5_ERROR)
        return MMC_ERR_FAILED;
    if (cmd.resp[0] & R5_FUNCTION_NUMBER)
        return MMC_ERR_INVALID;
    if (cmd.resp[0] & R5_OUT_OF_RANGE)
        return MMC_ERR_INVALID;
    #endif
    */

    if (write) {
        err = msdc_pio_write(host, (u32*)buf, blocks * blksz);
    } else {
        err = msdc_pio_read(host, (u32*)buf, blocks * blksz);
    }

    /* SDIO workaround for CMD53 multiple block transfer */
    if (!err && blocks > 1) {
        err=msdc_cmd_io_abort(host);
    }

    return err;
}
#endif

int msdc_pio_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE, cmderr = MMC_ERR_NONE;
    int multi;
    struct mmc_command cmd;
    ulong *ptr = (ulong *)dst;

    MSG(OPS, "[SD%d] Read PIO data %u bytes from 0x%x\n", host->id, (unsigned int)(nblks * blksz), (unsigned int)src);

    multi = nblks > 1 ? 1 : 0;

    MSDC_CLR_FIFO();

    #if 0 /* disable it since tuning algorithm cover this */
    /* Note: DDR50 PIO-8/16 data write timeout issue. Fore write DSMPL=1 to
     * avoid/reduce tuning, which could cause abnormal data latching and
     * trigger data timeout.
     */
    if (mmc_card_ddr(host->card)) {
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, MSDC_SMPL_RISING);
    }
    #endif

    msdc_set_blknum(host, nblks);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    //MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 16, 1);

    err = msdc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        goto done;

    derr = msdc_pio_read(host, (u32*)ptr, nblks * blksz);
    if (derr != MMC_ERR_NONE)
        goto done;
    //MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 16, 0);
    if (multi && (priv->autocmd == 0)) {
        cmderr = msdc_cmd_stop(host, &cmd);
    }

done:

    if (err != MMC_ERR_NONE){
        /* msdc_cmd will do cmd tuning flow, so if enter here, cmd maybe timeout.
         * need reset host */
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
        return err;                    // high level will retry
    }

    if (derr != MMC_ERR_NONE){
        /* crc error find in data transfer. need reset host & send cmd12 */
        /* if autocmd crc occur, will enter here too */
        msdc_abort_handler(host, 1);

        #if defined(MMC_MSDC_DRV_CTP)
        #if 0 //Light: turn if off before I verify it
        if ((sg_autocmd_crc_tuning_blkno == src) && (derr == MMC_ERR_ACMD_RSPCRC)){
            /* update cmd tuning parameter */
            sg_autocmd_crc_tuning_count++;
            msdc_tune_update_cmdrsp(host, sg_autocmd_crc_tuning_count);
        } else if(derr == MMC_ERR_ACMD_RSPCRC) {
            sg_autocmd_crc_tuning_blkno = src;
            sg_autocmd_crc_tuning_count = 0;
        }
        #endif
        #endif

        return derr;
    }

    if (cmderr != MMC_ERR_NONE){
        /* msdc_cmd will do cmd tuning flow, so if enter here, cmd maybe timeout
         * need reset host */
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
        return MMC_ERR_FAILED;  // high level will retry
    }

    return MMC_ERR_NONE;
}

int msdc_pio_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE, cmderr = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    struct mmc_command cmd;
    ulong *ptr = (ulong *)src;

    MSG(OPS, "[SD%d] Write PIO data %u bytes to 0x%x\n", host->id, (unsigned int)(nblks * blksz), (unsigned int)dst);

    multi = nblks > 1 ? 1 : 0;

    MSDC_CLR_FIFO();

    #if 0 /* disable it since tuning algorithm cover this */
    /* Note: DDR50 PIO-8/16 data write timeout issue. Fore write DSMPL=1 to
     * avoid/reduce tuning, which could cause abnormal data latching and
     * trigger data timeout.
     */
    if (mmc_card_ddr(host->card)) {
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, MSDC_SMPL_FALLING);
    }
    #endif

    msdc_set_blknum(host, nblks);
    msdc_set_blklen(host, blksz);

    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    host->cmd = &cmd;
    derr = msdc_pio_write(host, (u32*)ptr, nblks * blksz);

    if (multi && (priv->autocmd == 0)) {
        cmderr = msdc_cmd_stop(host, &cmd);
    }

done:
    if (err != MMC_ERR_NONE){
        /* msdc_cmd will do cmd tuning flow, so if enter here, cmd maybe timeout.
         * need reset host */
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
        return err;                    // high level will retry
    }

    if (derr != MMC_ERR_NONE){
        /* crc error find in data transfer. need reset host & send cmd12 */
        /* if autocmd crc occur, will enter here too */
        msdc_abort_handler(host, 1);

        #if defined(MMC_MSDC_DRV_CTP)
        #if 0 //Light: turn if off before I verify it
        if ((sg_autocmd_crc_tuning_blkno == dst) && (derr == MMC_ERR_ACMD_RSPCRC)){
            /* update cmd tuning parameter */
            sg_autocmd_crc_tuning_count++;
            msdc_tune_update_cmdrsp(host, sg_autocmd_crc_tuning_count);
        } else if(derr == MMC_ERR_ACMD_RSPCRC) {
            sg_autocmd_crc_tuning_blkno = dst;
            sg_autocmd_crc_tuning_count = 0;
        }
        #endif
        #endif

        return derr;
    }

    if (cmderr != MMC_ERR_NONE){
        /* msdc_cmd will do cmd tuning flow, so if enter here, cmd maybe timeout
         * need reset host */
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
        return MMC_ERR_FAILED;  // high level will retry
    }

    return MMC_ERR_NONE;
}

#if defined(FEATURE_MMC_SDIO)
#if 0
/*
int msdc_iorw(struct mmc_card *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
*/
#else
int msdc_iorw(struct mmc_card *card, int write,
    u8 *buf, unsigned blocks, unsigned blksz)
#endif
{
    int err;
    struct mmc_host *host = card->host;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    /*
    MSG(OPS, "[SD%d] IO: wr(%d) fn(%xh) adr(%xh), inc(%d), blks(%d), blksz(%d)\n",
        host->id, write, fn, addr, incr_addr, blocks, blksz);
    */

    /* TODO: refine me */
    #if defined(MSDC_USE_DMA_MODE)
    if (priv->cfg.mode != MSDC_MODE_PIO)
        //err = msdc_pio_iorw(card, write, fn, addr, incr_addr, buf, blocks, blksz);
        err = msdc_dma_iorw(card, write, buf, blocks, blksz);
    else
    #endif
        err = msdc_pio_iorw(card, write, buf, blocks, blksz);


    return err;
}
#endif

#if defined(MSDC_USE_MMC_STREAM)
int msdc_stream_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    struct mmc_card *card = host->card;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    struct mmc_command cmd;
    ulong *ptr = (ulong *)dst;

    if (!(card->csd.cmdclass & CCC_STREAM_READ)) {
        printf("[SD%d]<WARNING>: Card doesn't support stream read\n", host->id);
        return -1;
    }

    MSG(OPS, "[SD%d] Stream read data %d bytes from 0x%x\n", host->id,
        nblks * blksz, src);

    MSDC_CLR_FIFO();

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);

    /* send stream read command */
    cmd.opcode  = MMC_CMD_READ_DAT_UNTIL_STOP;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = msdc_pio_read(host, (u32*)ptr, nblks * blksz);
    if (priv->autocmd == 0) {
        priv->stream_stop = 1;
        err = msdc_cmd_stop(host, &cmd) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
        priv->stream_stop = 0;
    }

done:
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Stream read data error %d\n", host->id, err);
    }

    return err;
}

int msdc_stream_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    struct mmc_card *card = host->card;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    struct mmc_command cmd;
    ulong *ptr = (ulong *)src;

    if (!(card->csd.cmdclass & CCC_STREAM_WRITE)) {
        printf("[SD%d]<WARNING>: Card doesn't support stream write\n", host->id);
        return -1;
    }

    MSG(OPS, "[SD%d] Stream write data %d bytes to 0x%x\n", host->id,
        nblks * blksz, dst);

    MSDC_CLR_FIFO();

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);

    /* send write command */
    cmd.opcode  = MMC_CMD_WRITE_DAT_UNTIL_STOP;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = msdc_pio_write(host, (u32*)ptr, nblks * blksz);
    if (priv->autocmd == 0) {
        priv->stream_stop = 1;
        err = msdc_cmd_stop(host, &cmd) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
        priv->stream_stop = 0;
    }

done:
    if (err != MMC_ERR_NONE) {
       printf("[SD%d] Stream write data error %d\n", host->id, err);
    }

    return err;
}
#endif

#if defined(MMC_MSDC_DRV_CTP)
int msdc_switch_volt(struct mmc_host *host, int volt)
{
    u32 base = host->base;
    int err = MMC_ERR_FAILED;
    u32 timeout = 1000;
    u32 status;
    u32 sclk = host->sclk;

    /* make sure SDC is not busy (TBC) */
    WAIT_COND(!SDC_IS_BUSY(), timeout, timeout);
    if (timeout == 0) {
        err = MMC_ERR_TIMEOUT;
        goto out;
    }

    /* pull up disabled in CMD and DAT[3:0] to allow card drives them to low */

    /* check if CMD/DATA lines both 0 */
    if ((MSDC_READ32(MSDC_PS) & ((1 << 24) | (0xF << 16))) == 0) {

        /* pull up disabled in CMD and DAT[3:0] */
        //Originally set to MSDC_PIN_PULL_NONE. Since MT6572 does not support PULL_NONE, use PULL_DOWN instead
        msdc_config_pin(host, MSDC_PIN_PULL_NONE);

        msdc_set_host_level_pwr(host, 1, 1);

        /* wait at least 5ms for 1.8v signal switching in card */
        mdelay(10);

        /* config clock to 10~12MHz mode for volt switch detection by host. */
        msdc_config_clock(host, 0, 12000000);/*For FPGA 13MHz clock,this not work*/

        /* pull up enabled in CMD and DAT[3:0] */
        msdc_config_pin(host, MSDC_PIN_PULL_UP);
        mdelay(5);

        /* start to detect volt change by providing 1.8v signal to card */
        MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_BV18SDT);

        /* wait at max. 1ms */
        mdelay(1);

        while ((status = MSDC_READ32(MSDC_CFG)) & MSDC_CFG_BV18SDT);

        if (status & MSDC_CFG_BV18PSS)
            err = MMC_ERR_NONE;

        /* config clock back to init clk freq. */
        msdc_config_clock(host, 0, sclk);
    }

out:

    return err;
}
#endif

void msdc_set_tune_forcing(struct mmc_host *host, int enable)
{
    msdc_tune_forcing[host->id] = enable;
}

#if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
int msdc_tune_cal_smaller_trial_clock(struct mmc_host *host, u32 ddr)
{
    int divisor;
    u32 try_this=0, try_this_clock_freq, clock_src_index;

    #if MSDC_USE_CLKSRC_IN_DATCRC
    if ( msdc_clock_tune_seq_index[host->id]+1<ARRAY_SIZE(msdc_clock_tune_seq) ) {
        //Try lower clock source with the same divisor
        divisor=msdc_clock_divisor[host->id];
        msdc_clock_tune_seq_index[host->id]+=1;
    } else {
        Prepare_To_Try_Smaller_Divisor:
        //Roll back to highest clock and try try smaller divisor
        divisor=msdc_clock_divisor[host->id]+1;
        msdc_clock_tune_seq_index[host->id]=0;
    }

    #elif MSDC_USE_CLKDIV_IN_DATCRC
    divisor=msdc_clock_divisor[host->id]+1;

    #endif

    #if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
    {
        #if MSDC_USE_CLKSRC_IN_DATCRC
        clock_src_index=msdc_clock_tune_seq[msdc_clock_tune_seq_index[host->id]];

        #elif MSDC_USE_CLKDIV_IN_DATCRC
        clock_src_index=host->clksrc;

        #endif

        #if MSDC_USE_CLKSRC_IN_DATCRC
        if ( divisor==-1 ) {
            //for mode 0 without division
            if ( msdc_src_clks[clock_src_index] > MSDC_TUNING_CLOCK_MIN_FREQ ) {
                try_this_clock_freq=msdc_src_clks[clock_src_index];
                try_this=1;
            }

        } else
        #endif
        if ( divisor==0 ) {
            if ( msdc_src_clks[clock_src_index]/2 > MSDC_TUNING_CLOCK_MIN_FREQ ) {
                try_this_clock_freq=msdc_src_clks[clock_src_index]/2;
                try_this=1;
            }

        } else if ( divisor>0 ) {
            if ( msdc_src_clks[clock_src_index]/(4*divisor) > MSDC_TUNING_CLOCK_MIN_FREQ ) {
                try_this_clock_freq=msdc_src_clks[clock_src_index]/(4*divisor);
                try_this=1;
            }
        }

        if ( try_this ) {
            printf("[SD%d] <TUNE_BREAD> SRC_CLK: %d,  DIV: %d\n", host->id, msdc_src_clks[clock_src_index], divisor);
            msdc_config_clksrc(host, clock_src_index);
            msdc_config_clock(host, ddr, try_this_clock_freq);
        }
    }
    #endif

    return try_this;

    #if MSDC_USE_CLKDIV_IN_DATCRC
    /*
    //divisor++;
    if ( (host->sclk>>1) > MSDC_TUNING_CLOCK_MIN_FREQ ) { //Divide clock only once
        msdc_config_clksrc(host, clock_src_index);
        msdc_config_clock(host, ddr, host->sclk >> 1);
        goto Repeat_msdc_tune_bread;
    }
    */
    #endif
}
#endif

#if defined(MMC_MSDC_DRV_CTP)
void msdc_tune_update_cmdrsp(struct mmc_host *host, u32 count)
{
    u32 base = host->base;
    u32 sel = 0;
    u32 rsmpl,cur_rsmpl, orig_rsmpl;
    u32 rrdly,cur_rrdly, orig_rrdly;
    u32 cntr,cur_cntr,orig_cmdrtc;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel;
    u32 times = 0;

    printf("sclk = %d\n", host->sclk);
    if (host->sclk > 100000000){
        sel = 1;
    }

    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orig_rrdly);
    MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, orig_cmdrtc);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

    dl_cksel = 0;
    cntr = 0;
    rrdly = 0;
    if (sel == 1){
        if (count >= 8 * 64 && count < 8 * 8 * 64) {
            dl_cksel = count % 8;
            cur_dl_cksel = (orig_dl_cksel + dl_cksel + 1) % 8;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);

            count = count % (8 * 64);
        }

        if (count >= 64 && count < 8 * 64) {
            cntr = count % 8;
            cur_cntr = (orig_cmdrtc + cntr + 1) % 8;
            MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, cur_cntr);

            count = count % 64;
        }
    }

    if (count >= 2 && count < 64) {
        rrdly = count % 32;
        cur_rrdly = (orig_rrdly + rrdly + 1) % 32;
        MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, cur_rrdly);

        count = (count > 32 ? 1 : 0);
    }

    if (count < 2){
        cur_rsmpl = (orig_rsmpl + count) % 2;
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl);
    }
}
#endif


#if defined(FEATURE_MMC_CM_TUNING)
/* optimized for tuning loop */
int msdc_tune_cmdrsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base = host->base;
    u32 ddr = 0, sel = 0;
    u32 rsmpl, cur_rsmpl, orig_rsmpl;
    u32 rrdly, cur_rrdly, orig_rrdly;
    u32 cntr, cur_cntr, orig_cmdrtc;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel;
    u32 times = 0;
    u32 drv = 0;
    int result = MMC_ERR_CMDTUNEFAIL;

    if ( host->sclk > 100000000){
        sel = 1;
        //MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL,0);
    }

    if (host->card)
        ddr = mmc_card_ddr(host->card);

    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orig_rrdly);
    MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, orig_cmdrtc);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    if ( host->id==0 )
        MSDC_GET_FIELD(MSDC0_DRVING_BASE, MSDC0_CK_CM_DAT_DRVING, drv);
    else { //if ( host->id==1 )
        MSDC_GET_FIELD(MSDC1_DRVING_BASE, MSDC1_CK_CM_DAT_DRVING, drv);
    }

Repeat_msdc_tune_cmdrsp:

    dl_cksel = 0;
    do {
        cntr = 0;
        do{
            rrdly = 0;
            do {
                //for (rsmpl = 0; rsmpl < 2; rsmpl++) {
                    //cur_rsmpl = (orig_rsmpl + rsmpl) % 2;
                for (rsmpl = orig_rsmpl; rsmpl < 2; rsmpl++) {    //20130118 Light Add
                    cur_rsmpl = rsmpl % 2;                           //20130118 Light Add
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl);

                    //BEGIN : copied from orginal JB2
                    if(host->sclk <= 400000){ //In sd/emmc init flow, fix rising edge for latching cmd response
                            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
                    }
                    //END : copied from orginal JB2

                    if (!msdc_tune_forcing[host->id] && cmd->opcode != MMC_CMD_STOP_TRANSMISSION) {
                        if(host->app_cmd){
                            //For CMD55 part of an ACMD sent/received successfully
                            result = msdc_app_cmd(host);
                            if(result != MMC_ERR_NONE) {
                                //In this condition it means that whole tuning combination has been tried
                                // in another invocation to msdc_tune_cmdrsp() inside msdc_app_cmd(),
                                // therefore no more tuning is necessary
                                return MMC_ERR_CMDTUNEFAIL;
                            }
                        }

                        //For non-CMD55
                        result = msdc_send_cmd(host, cmd);
                        if (result == MMC_ERR_TIMEOUT) {
                            //rsmpl--; //Comment out Light 20130123.
                                       //According to spec, timeout is recovered by power cycle or reset
                            return result;
                        }
                        /*
                        if ( result != MMC_ERR_NONE && cmd->opcode != MMC_CMD_STOP_TRANSMISSION){
                            if( cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK ||
                                cmd->opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK ||
                                cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK ||
                                cmd->opcode == MMC_CMD_WRITE_BLOCK )
                                msdc_abort_handler(host,1);
                            continue;
                        }
                        */
                        //Added by Light to replace the above commented-out section
                        if ( result == MMC_ERR_NONE )

                            result = msdc_wait_rsp(host, cmd);

                    } else if(cmd->opcode == MMC_CMD_STOP_TRANSMISSION){
                        result = MMC_ERR_NONE;
                        goto done;
                    } else {
                        //msdc_tune_forcing is set: forcing as BAD CRC to toggle all tuning combination
                        result = MMC_ERR_BADCRC;
                    }

                    /* for debugging */
                    #if 1
                    {
                        u32 t_rrdly, t_rsmpl, t_dl_cksel,t_cmdrtc;
                        //u32 t_cksel;

                        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, t_rsmpl);
                        MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, t_rrdly);
                        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, t_cmdrtc);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                        times++;
                        printf("[SD%d] <TUNE_CMD><%d><%s>\n",
                            host->id, times,
                            (result == MMC_ERR_NONE) ? "PASS" : "FAIL");
                        printf("       CMDRRDLY=%d, RSPL=%dh\n"
                               "       MSDC_CMD_RSP_TA_CNTR=%xh\n"
                               "       INT_DAT_LATCH_CK_SEL=%xh\n"
                               "       SCLK=%d, Driving=%d\n",
                               t_rrdly, t_rsmpl,
                               t_cmdrtc,
                               t_dl_cksel,
                               host->sclk, drv);

                    }
                    #endif
                    if (result == MMC_ERR_NONE)
                        goto done;
                    /*if ((result == MMC_ERR_TIMEOUT || result == MMC_ERR_BADCRC)&&  cmd->opcode == MMC_CMD_STOP_TRANSMISSION){
                        printf("[SD%d]TUNE_CMD12--failed ignore\n",host->id);
                        result = MMC_ERR_NONE;
                    goto done;
                    }*/
                    if ( cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK ||
                         cmd->opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK ||
                         cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK ||
                         cmd->opcode == MMC_CMD_WRITE_BLOCK )
                    {
                        msdc_abort_handler(host,1);
                        /*
                        msdc_abort_handler(host,0);
                        //To do: check if msdc_abort_handler(host, 0) shall be used:
                        //       Since command CRC error ocurs, host may not get card status by issue a new command 13.
                        */
                    }
                }
                orig_rsmpl=0;                       //20130118 Light Add
                //cur_rrdly = (orig_rrdly + rrdly + 1) % 32;
                rrdly++;                            //20130118 Light Add
                cur_rrdly = (orig_rrdly + rrdly) % 32;
                MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, cur_rrdly);
            //} while (++rrdly < 32);
            } while (orig_rrdly + rrdly < 32);      //20130118 Light Add

            orig_rrdly = 0;                         //20130118 Light Add

            if(!sel)
                break;
            cntr++;
            cur_cntr = (orig_cmdrtc + cntr) % 8;
            MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, cur_cntr);
        //} while(++cntr < 8);
        } while(orig_cmdrtc + cntr < 8 );

        orig_cmdrtc = 0;                            //20130118 Light Add

        /* no need to update data ck sel */
        if (!sel)
            break;
        dl_cksel++;
        cur_dl_cksel = (orig_dl_cksel +dl_cksel) % 8;
        MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
    //} while(dl_cksel < 8);
    } while(orig_dl_cksel +dl_cksel < 8);

    drv += 2;
    if ( drv<8 ) {
        msdc_set_driving(host, drv, drv, drv);
        goto Repeat_msdc_tune_cmdrsp;
    } else {
        drv=0;
        msdc_set_driving(host, drv, drv, drv);
    }

    #if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
    if ( msdc_tune_cal_smaller_trial_clock(host, ddr) )
        goto Repeat_msdc_tune_cmdrsp;
    #endif

    /* no need to update ck sel */
    if(result != MMC_ERR_NONE)
        result = MMC_ERR_CMDTUNEFAIL;
done:

    return result;
}


#if defined(MMC_MSDC_DRV_CTP) || defined(MMC_MSDC_DRV_LK)
u32 rsmpl_eh = 0;
u32 rrdly_eh = 0;
u32 cntr_eh = 0;
int msdc_tune_enhancedma_cmdrsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base = host->base;
    u32 sel = 0;
    u32 cur_rsmpl, orig_rsmpl;
    u32 cur_rrdly, orig_rrdly;
    u32 cur_cntr,orig_cmdrtc;
    u32 cur_dl_cksel, orig_dl_cksel;
    u32 times = 0;
    int result = MMC_ERR_NONE;

    if (mmc_card_uhs1(host->card) && host->sclk > 100000000){
        sel = 1;
        //MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL,0);
    }

    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orig_rrdly);
    MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, orig_cmdrtc);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

    cur_rsmpl = (orig_rsmpl + 1) % 2;
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl);
    rsmpl_eh++;
    if (rsmpl_eh == 2){
        rsmpl_eh = 0;
        cur_rrdly = (orig_rrdly + 1) % 32;
        MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, cur_rrdly);
        rrdly_eh++;
    }
    if (rrdly_eh == 32 && sel){
        rrdly_eh = 0;
        cur_cntr = (orig_cmdrtc  + 1) % 8;
        MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, cur_cntr);
        cntr_eh++;
    }
    if (cntr_eh == 8 && sel){
        cntr_eh = 0;
        cur_dl_cksel = (orig_dl_cksel +1) % 8;
        MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
    }

    {
        u32 t_rrdly, t_rsmpl, t_dl_cksel,t_cmdrtc;
        //u32 t_cksel;
        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, t_rsmpl);
        MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, t_rrdly);
        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
        MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_CMD_RSP_TA_CNTR, t_cmdrtc);
        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

        times++;
        printf("[SD%d] <TUNE_CMD><Enhance><%s> CMDRRDLY=%d, RSPL=%dh\n",
            host->id, "Change", t_rrdly, t_rsmpl);
        //printf("[SD%d] <TUNE_CMD><Enhance><%s> MSDC_CKGEN_RX_SDCLKO_SEL=%xh\n",
            //host->id, "Change", t_cksel);
        printf("[SD%d] <TUNE_CMD><Enhance><%s> MSDC_CMD_RSP_TA_CNTR=%xh\n",
            host->id, "Change", t_cmdrtc);
        printf("[SD%d] <TUNE_CMD><Enhance><%s> INT_DAT_LATCH_CK_SEL=%xh\n",
            host->id, "Change", t_dl_cksel);

    }

    return result;
}
#endif

#endif

#if defined(FEATURE_MMC_RD_TUNING)
/* optimized for tuning loop */
int msdc_tune_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    u32 base = host->base;
    u32 dcrc = 0, ddr = 0, sel = 0;
    u32 cur_rxdly0, cur_rxdly1;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
    u32 dsel, cur_dsel, orig_dsel;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel;
    u32 rxdly;
    u32 orig_clkmode;
    u32 drv = 0;
    u32 times = 0;
    int result = MMC_ERR_READTUNEFAIL; //Light: the initialized value seems to be overwritten always

    if (host->sclk > 100000000)
        sel = 1;

    if (host->card)
        ddr = mmc_card_ddr(host->card);

    MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD,orig_clkmode);
    //if(orig_clkmode == 1)
    //MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, 0);

    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orig_dsel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);

    /* Tune Method 2. delay each data line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
    if ( host->id==0 )
        MSDC_GET_FIELD(MSDC0_DRVING_BASE, MSDC0_CK_CM_DAT_DRVING, drv);
    else { //if ( host->id==1 )
        MSDC_GET_FIELD(MSDC1_DRVING_BASE, MSDC1_CK_CM_DAT_DRVING, drv);
    }

Repeat_msdc_tune_bread:

    dl_cksel = 0;
    do {
        dsel = 0;
        do{
            rxdly = 0;
            do {
                //for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                    //cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                for (dsmpl = orig_dsmpl; dsmpl < 2; dsmpl++) {    //20130118 Light Add
                    cur_dsmpl = dsmpl % 2;                          //20130118 Light Add
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl);

                    if (!msdc_tune_forcing[host->id]) {
                        result = host->blk_read(host, dst, src, nblks);
                        if (result == MMC_ERR_CMDTUNEFAIL || result == MMC_ERR_CMD_RSPCRC || result == MMC_ERR_ACMD_RSPCRC) {
                            //If PIO is used, this case won't occur since PIO don't tune!
                            //If CMD/RSP fail, it is unnecessary to tune read data
                            result=MMC_ERR_CMDTUNEFAIL;
                            goto done;
                        }

                        //BEGIN Light 20130123
                        else if (result == MMC_ERR_TIMEOUT) {
                            return result;
                        }
                        //END Light 20130123

                        MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
                    } else {
                        //Not STOP CMD and msdc_tune_forcing is set
                        result = MMC_ERR_BADCRC;
                        dcrc = SDC_DCRC_STS_NEG|SDC_DCRC_STS_POS;
                    }

                    if (!ddr) dcrc &= ~SDC_DCRC_STS_NEG;

                    /* for debugging */
                    {
                        u32 t_dspl, t_dl_cksel, t_dsel;
                        //u32 t_cksel;

                        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, t_dspl);
                        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, t_dsel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                        times++;
                        printf("[SD%d] <TUNE_BREAD><%d><%s>\n",
                            host->id, times,
                            (result == MMC_ERR_NONE) ? "PASS" : "FAIL");
                        printf("       DCRC=%xh\n"
                               "       DATRDDLY0=%xh, DATRDDLY1=%xh\n"
                               "       CKGEN_MSDC_DLY_SEL=%xh\n"
                               "       INT_DAT_LATCH_CK_SEL=%xh, DSMPL=%xh\n"
                               "       SCLK=%d, Driving=%d\n",
                               dcrc,
                               MSDC_READ32(MSDC_DAT_RDDLY0), MSDC_READ32(MSDC_DAT_RDDLY1),
                               t_dsel,
                               t_dl_cksel, t_dspl,
                               host->sclk, drv);
                    }

                    /* no cre error in this data line */
                    if (result == MMC_ERR_NONE && dcrc == 0) {
                        goto done;
                    } else {
                        result = MMC_ERR_BADCRC;
                    }
                }

                orig_dsmpl=0;                                   //20130118 Light Add

                cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
                cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

                cur_rxdly0 += (dcrc & ((1 << 0) || (1 << 8)) ) ?  (1<<24) : 0;
                cur_rxdly0 += (dcrc & ((1 << 1) || (1 << 9)) ) ?  (1<<16) : 0;
                cur_rxdly0 += (dcrc & ((1 << 2) || (1 << 10)) ) ?  (1<<8) : 0;
                cur_rxdly0 += (dcrc & ((1 << 3) || (1 << 11)) ) ?  (1<<0) : 0;

                cur_rxdly1 += (dcrc & ((1 << 4) || (1 << 12)) ) ?  (1<<24) : 0;
                cur_rxdly1 += (dcrc & ((1 << 5) || (1 << 13)) ) ?  (1<<16) : 0;
                cur_rxdly1 += (dcrc & ((1 << 6) || (1 << 14)) ) ?  (1<<8) : 0;
                cur_rxdly1 += (dcrc & ((1 << 7) || (1 << 15)) ) ?  (1<<0) : 0;

                MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0&0x1F1F1F1F);
                MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1&0x1F1F1F1F);

                if ( (cur_rxdly0&0x20202020) || (cur_rxdly1&0x20202020) )
                    break;

            } while (++rxdly < 32); //Light: the while condition may be changed as "while(1)" when there are the 3 added lines above

            if(!sel)
                break;

            //Although 32 steps of DAT line delay=8 steps delay of CKGEN delay,
                    //  increse orig_dsel by 8 will has problem if two DAT lines need CKGEN delay in two different 8n window.
            cur_dsel = (orig_dsel + dsel + 1) % 32;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_dsel);
        }while(++dsel < 32);

        /* no need to update data ck sel */
        if (orig_clkmode != 1)
            break;

        cur_dl_cksel = (orig_dl_cksel + dl_cksel + 1)% 8;
        MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
        dl_cksel++;
    } while(dl_cksel < 8);

    //Since we adjust MSDC_INT_DAT_LATCH_CK_SEL  without setting DAT delay as 0,
    // we may have responsibility to clear it before adjusting driving strength and smaller clock frequency
    MSDC_WRITE32(MSDC_DAT_RDDLY0, 0);               //20130118 Light Add
    MSDC_WRITE32(MSDC_DAT_RDDLY1, 0);               //20130118 Light Add
    orig_dl_cksel=0;                                //20130118 Light Add

    drv += 2;
    if ( drv<8 ) {
        msdc_set_driving(host, drv, drv, drv);
        goto Repeat_msdc_tune_bread;
    } else {
        drv=0;
        msdc_set_driving(host, drv, drv, drv);
    }

    #if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
    if ( msdc_tune_cal_smaller_trial_clock(host, ddr) )
        goto Repeat_msdc_tune_bread;
    #endif

done:

    return result;
}

#define READ_TUNING_MAX_HS          (2 * 32)
#define READ_TUNING_MAX_UHS         (2 * 32 * 32)
#define READ_TUNING_MAX_UHS_CLKMOD1 (2 * 32 * 32 *8)

void msdc_reset_tune_counter(struct mmc_host *host)
{
    host->time_read = 0;
}

int msdc_tune_read(struct mmc_host *host)
{
    u32 base = host->base;
    u32 dcrc, ddr = 0, sel = 0;
    u32 cur_rxdly0 = 0 , cur_rxdly1 = 0;
    u32 cur_dsmpl = 0, orig_dsmpl;
    u32 cur_dsel;
    u32 cur_dl_cksel = 0,orig_dl_cksel;
    u32 orig_clkmode;
    int result = MMC_ERR_NONE;

    if (host->sclk > 100000000)
        sel = 1;

    if (host->card)
        ddr = mmc_card_ddr(host->card);

    MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD,orig_clkmode);
    //if(orig_clkmode == 1)
        //MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, 0);

    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_dsel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);

    /* Tune Method 2. delay each data line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

    cur_dsmpl = (orig_dsmpl + 1) ;
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl % 2);

    if(cur_dsmpl >= 2){
        MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
        if (!ddr) dcrc &= ~SDC_DCRC_STS_NEG;

        cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
        cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

        cur_rxdly0 += (dcrc & ((1 << 0) || (1 << 8)) ) ?  (1<<24) : 0;
        cur_rxdly0 += (dcrc & ((1 << 1) || (1 << 9)) ) ?  (1<<16) : 0;
        cur_rxdly0 += (dcrc & ((1 << 2) || (1 << 10)) ) ?  (1<<8) : 0;
        cur_rxdly0 += (dcrc & ((1 << 3) || (1 << 11)) ) ?  (1<<0) : 0;

        cur_rxdly1 += (dcrc & ((1 << 4) || (1 << 12)) ) ?  (1<<24) : 0;
        cur_rxdly1 += (dcrc & ((1 << 5) || (1 << 13)) ) ?  (1<<16) : 0;
        cur_rxdly1 += (dcrc & ((1 << 6) || (1 << 14)) ) ?  (1<<8) : 0;
        cur_rxdly1 += (dcrc & ((1 << 7) || (1 << 15)) ) ?  (1<<0) : 0;

        MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0&0x1F1F1F1F);
        MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1&0x1F1F1F1F);
    }

    if ( (cur_rxdly0&0x20202020) || (cur_rxdly1&0x20202020) ) {
        if(sel){
            //Although 32 steps of DAT line delay=8 steps delay of CKGEN delay,
            //  increse cur_dsel by 8 will has problem if two DAT lines need CKGEN delay in two different 8n window.
            cur_dsel += 1;
            if ( cur_dsel < 32 ) {
                MSDC_WRITE32(MSDC_DAT_RDDLY0, 0);               //20130123 Light Add
                MSDC_WRITE32(MSDC_DAT_RDDLY1, 0);               //20130123 Light Add
                MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_dsel);
            }
        }
    }
    if(cur_dsel >= 32){
        //In this condition, current MSDC_CKGEN_MSDC_DLY_SEL in register is 31

        //Tune when SD bus clock>100MHz
        if(orig_clkmode == 1 && sel){
            cur_dl_cksel = (orig_dl_cksel + 1);
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel % 8);
        }
    }

    MSG(TUNE, "[SD%d] <TUNE_READ_%d> DSMPL=%x, DATRDDLY0=%xh, DATRDDLY1=%xh\n"
              "[SD%d] <TUNE_READ_%d> CKGEN_MSDC_DLY_SEL=%xh, INT_DAT_LATCH_CK_SEL=%xh\n",
        host->id, host->time_read, cur_dsmpl&0x1, cur_rxdly0, cur_rxdly1,
        host->id, host->time_read, cur_dsel, cur_dl_cksel%8);

    //To do:
    //  Consider if necessary to adjust driving strength and/or use smaller clock frequency

    ++(host->time_read);
    if( (sel == 1 && orig_clkmode == 1 && host->time_read == READ_TUNING_MAX_UHS_CLKMOD1)||
        (sel == 1 && orig_clkmode != 1 && host->time_read == READ_TUNING_MAX_UHS)||
        (sel == 0 && orig_clkmode != 1 && host->time_read == READ_TUNING_MAX_HS)){
        result = MMC_ERR_READTUNEFAIL;
    }
    return result;
}
#endif

#if defined(FEATURE_MMC_WR_TUNING)
/* optimized tuning loop */
int msdc_tune_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    u32 base = host->base;
    u32 ddr = 0 , sel = 0;
    //u32 ddrckdly = 0;
    u32 wrrdly, cur_wrrdly, orig_wrrdly;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
    u32 d_cntr,orig_d_cntr,cur_d_cntr;
    u32 rxdly, cur_rxdly0;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3;
    u32 times = 0;
    u32 drv;
    int result = MMC_ERR_WRITETUNEFAIL;

    if (host->sclk > 100000000)
        sel = 1;

    if (host->card)
        ddr = mmc_card_ddr(host->card);

    //if ( ddr ) ddrckdly = 1;

    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orig_wrrdly);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WDSPL, orig_dsmpl);
    //MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, orig_ddrdly);
    //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, orig_d_cntr);
    //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

    /* Tune Method 2. delay data0 line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
    if ( host->id==0 ) {
        MSDC_GET_FIELD(MSDC0_DRVING_BASE, MSDC0_CK_CM_DAT_DRVING, drv);
    } else { //if ( host->id==1 )
        MSDC_GET_FIELD(MSDC1_DRVING_BASE, MSDC1_CK_CM_DAT_DRVING, drv);
    }

    cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);

    orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
    orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
    orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
    orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;

Repeat_msdc_tune_bwrite:

    d_cntr = 0;
    do {
        rxdly = 0;
        do {
            wrrdly = 0;
            do {
                //for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                    //cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                for (dsmpl = orig_dsmpl; dsmpl < 2; dsmpl++) {  //20130118 Light Add
                    cur_dsmpl = dsmpl % 2;                      //20130118 Light Add
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WDSPL, cur_dsmpl);

                    if (!msdc_tune_forcing[host->id]) {
                        result = host->blk_write(host, dst, src, nblks);
                        if(result == MMC_ERR_CMDTUNEFAIL || result == MMC_ERR_CMD_RSPCRC || result == MMC_ERR_ACMD_RSPCRC) {
                            //If CMD/RSP fail, it is unnecessary to tune write data
                            result= MMC_ERR_CMDTUNEFAIL;
                            goto done;
                        }
                        //BEGIN Light 20130123
                        else if (result == MMC_ERR_TIMEOUT) {
                            return result;
                        }
                        //END Light 20130123
                    } else {
                        //Not STOP CMD and msdc_tune_forcing is set
                        result = MMC_ERR_BADCRC;
                    }

                    /* for debugging */
                    {
                        u32 t_dspl, t_wrrdly, t_d_cntr;// t_dl_cksel;
                        //u32 t_ddrdly, t_cksel;

                        MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, t_wrrdly);
                        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WDSPL, t_dspl);
                        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, t_d_cntr);
                        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                        times++;

                        printf("[SD%d] <TUNE_BWRITE><%d><%s>\n",
                            host->id, times,
                            (result == MMC_ERR_NONE) ? "PASS" : "FAIL");
                        printf("       DSPL=%d, DATWRDLY=%d\n"
                               "       MSDC_DAT_RDDLY0=%xh\n"
                               "       MSDC_WRDAT_CRCS_TA_CNTR=%xh\n"
                               "        SCLK=%d, Driving=%d\n",
                               t_dspl, t_wrrdly,
                               MSDC_READ32(MSDC_DAT_RDDLY0),
                               t_d_cntr,
                               //t_dl_cksel, t_dspl,
                               host->sclk, drv);
                    }

                    if (result == MMC_ERR_NONE) {
                        goto done;
                    }
                }
                orig_dsmpl=0;   //20130118 Light Add

                //Designer: tuning of DATWRDLY has little effect (JB2 linux driver for 6589 does not use it)
                cur_wrrdly = ++orig_wrrdly % 32;
                MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, cur_wrrdly);
            } while (++wrrdly < 32);

            orig_wrrdly=0;      //20130118 Light Add

            cur_dat0 = ++orig_dat0 % 32; /* only adjust bit-1 for crc */
            cur_dat1 = orig_dat1;
            cur_dat2 = orig_dat2;
            cur_dat3 = orig_dat3;

            cur_rxdly0 = (cur_dat0 << 24) | (cur_dat1 << 16) |
                (cur_dat2 << 8) | (cur_dat3 << 0);

            MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
        } while (++rxdly < 32);

        orig_dat0=0;   //20130118 Light Add

        /* no need to update data ck sel */
        if (!sel)
            break;

        cur_d_cntr= (orig_d_cntr + d_cntr +1 )% 8;
        MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, cur_d_cntr);
        d_cntr++;
    } while (d_cntr < 8);

    orig_d_cntr=0; //20130118 Light Add

    drv += 2;
    if ( drv<8 ) {
        msdc_set_driving(host, drv, drv, drv);
        goto Repeat_msdc_tune_bwrite;
    } else {
        drv=0;
        msdc_set_driving(host, drv, drv, drv);
    }

    #if (MSDC_USE_CLKSRC_IN_DATCRC) || (MSDC_USE_CLKDIV_IN_DATCRC)
    if ( msdc_tune_cal_smaller_trial_clock(host, ddr) )
        goto Repeat_msdc_tune_bwrite;
    #endif

done:

    return result;
}
#endif




#if defined(MMC_MSDC_DRV_CTP)
int msdc_tune_uhs1(struct mmc_host *host, struct mmc_card *card)
{
    u32 base = host->base;
    u32 status;
    int i;
    int err = MMC_ERR_FAILED;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SEND_TUNING_BLOCK;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = 0xFFFFFFFF;

    msdc_set_timeout(host, 100000000, 0);
    msdc_set_autocmd(host, MSDC_AUTOCMD19, 1);

    for (i = 0; i < 13; i++) {
        /* Note. select a pad to be tuned. msdc only tries 32 times to tune the
         * pad since there is only 32 tuning steps for a pad.
         */
        MSDC_SET_FIELD(SDC_ACMD19_TRG, SDC_ACMD19_TRG_TUNESEL, i);

        /* Note. autocmd19 will only trigger done interrupt and won't trigger
         * autocmd timeout and crc error interrupt. (autocmd19 is a special command
         * and is different from autocmd12 and autocmd23.
         */
        err = msdc_cmd(host, &cmd);
        if (err != MMC_ERR_NONE)
            goto out;

        /* read and check acmd19 sts. bit-1: success, bit-0: fail */
        status = MSDC_READ32(SDC_ACMD19_STS);

        if (!status) {
            printf("[SD%d] ACMD19_TRG(%d), STS(0x%x) Failed\n", host->id, i,
            status);
            err = MMC_ERR_FAILED;
            goto out;
        }
    }
    err = MMC_ERR_NONE;

out:
    msdc_set_autocmd(host, MSDC_AUTOCMD19, 0);
    return err;
}
#endif

int msdc_tune_hs200(struct mmc_host *host, struct mmc_card *card)
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Card Detection, Availability, Protection
//
////////////////////////////////////////////////////////////////////////////////

#ifdef FEATURE_MMC_CARD_DETECT
void msdc_card_detect(struct mmc_host *host, int on)
{
    u32 base = host->base;

    if ((msdc_cap.flags & MSDC_CD_PIN_EN) == 0) {
        MSDC_CARD_DETECTION_OFF();
        return;
    }

    if (on) {
        MSDC_SET_FIELD(MSDC_PS, MSDC_PS_CDDEBOUNCE, DEFAULT_DEBOUNCE);
        MSDC_CARD_DETECTION_ON();
    } else {
        MSDC_CARD_DETECTION_OFF();
        MSDC_SET_FIELD(MSDC_PS, MSDC_PS_CDDEBOUNCE, 0);
    }
}

#else
#define msdc_card_detect(h,on)  do{}while(0)

#endif

#ifdef FEATURE_MMC_CARD_DETECT
int msdc_card_avail(struct mmc_host *host)
{
    u32 base = host->base;
    int sts, avail = 0;

    if ((msdc_cap.flags & MSDC_REMOVABLE) == 0)
        return 1;

    if (msdc_cap.flags & MSDC_CD_PIN_EN) {
        MSDC_GET_FIELD(MSDC_PS, MSDC_PS_CDSTS, sts);
        avail = sts == 0 ? 1 : 0;
    }

    return avail;
}
#endif

#if defined(MMC_MSDC_DRV_CTP)
int msdc_card_protected(struct mmc_host *host)
{
    u32 base = host->base;
    int prot;

    if (msdc_cap.flags & MSDC_WP_PIN_EN) {
        MSDC_GET_FIELD(MSDC_PS, MSDC_PS_WP, prot);
    } else {
        prot = 0;
    }

    return prot;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Reset
//
////////////////////////////////////////////////////////////////////////////////
#if defined(MMC_MSDC_DRV_CTP) || defined(MMC_MSDC_DRV_LK)
void msdc_hard_reset(struct mmc_host *host)
{
    msdc_card_power(host, 0);
    mdelay(10);

    msdc_card_power(host, 1);
    mdelay(10);
}

void msdc_soft_reset(struct mmc_host *host)
{
    u32 base = host->base;
    u32 tmo = 0x0000ffff;

    MSDC_RESET();
    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP, 1);
    WAIT_COND((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS) == 0, 0xFFFF, tmo);

    if (tmo == 0) {
        MSG(DMA, "[SD%d] MSDC_DMA_CFG_STS != inactive\n", host->id);
    }
    MSDC_CLR_FIFO();
}
#endif

void msdc_init_reg(struct mmc_host *host)
{
    u32 base = host->base;

    /* reset tuning parameter */
    #if defined(MSDC_WITH_DEINIT)
    msdc_save_pin_mux(host);
    #endif
    msdc_pad_ctrl_init(host);

    //Write Default Value. TBD: if necessary
    MSDC_WRITE32(MSDC_PAD_TUNE, 0x0000000);
    MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x00000000);
    MSDC_WRITE32(MSDC_DAT_RDDLY1, 0x00000000);
    MSDC_WRITE32(MSDC_IOCON, 0x00000000);

    //TBD, Need Review
    MSDC_WRITE32(MSDC_PATCH_BIT0, 0x003C004F);
    MSDC_WRITE32(MSDC_PATCH_BIT1, 0xFFFF0009);//High 16 bit = 0 mean Power KPI is on
    //MSDC_PATCH_BIT1YD:WRDAT_CRCS_TA_CNTR need fix to 3'001 by default,(<50MHz) (>=50MHz set 3'001 as initial value is OK for tunning)
    //YD:MSDC_CMD_RSP_TA_CNTR need fix to 3'001 by default(<50MHz)(>=50MHz set 3'001as initial value is OK for tunning)

    //Set sampling edge
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, msdc_cap.cmd_edge);
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, msdc_cap.data_edge);

    /* write crc timeout detection */
    MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 30, 1);
}

int msdc_init(int id, struct mmc_host *host, int clksrc, int mode)
{
    u32 baddr[] = {MSDC0_BASE, MSDC1_BASE};
    u32 base = baddr[id];
    msdc_priv_t *priv;
    struct dma_config *cfg;

    clksrc = (clksrc == -1) ? msdc_cap.clk_src : clksrc;
    //printf("MSDC1_BASE = 0x%x\n",baddr[1]);

#if defined(MSDC_USE_SDXC_FPGA)
    {
        /* CHECKME. MSDC_INT.SDIOIRQ is set if this has been set */
        static int sdxc_switch = 0;
        if (sdxc_switch == 0) {
            MSDC_WRITE32(GPO_Base, 0x1);
            sdxc_switch = 1;
        }
    }
#endif

    priv = &msdc_priv[id];
    #if defined(MSDC_USE_DMA_MODE)
    cfg  = &priv->cfg;
    #endif
    memset(priv, 0, sizeof(msdc_priv_t));

#if MSDC_DEBUG
    msdc_reg[id] = (struct msdc_regs*)base;
#endif

    host->id     = id;
    host->base   = base;
    #if defined(MMC_MSDC_DRV_CTP)
    host->f_max  = msdc_src_clks[clksrc];
    #else
    host->f_max  = MSDC_MAX_SCLK;
    #endif
    host->f_min  = MSDC_MIN_SCLK;
    host->blkbits= MMC_BLOCK_BITS;
    host->blklen = 0;
    host->priv   = (void*)priv;

    host->caps   = MMC_CAP_MULTIWRITE;

    if (msdc_cap.flags & MSDC_HIGHSPEED)
        host->caps |= (MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED);
    if (msdc_cap.flags & MSDC_UHS1)
        host->caps |= MMC_CAP_SD_UHS1;
    if (msdc_cap.flags & MSDC_HS200)
        host->caps |= MMC_CAP_EMMC_HS200;
    if (msdc_cap.flags & MSDC_DDR)
        host->caps |= MMC_CAP_DDR;

    if (msdc_cap.data_pins == 4)
        host->caps |= MMC_CAP_4_BIT_DATA;
    else if (msdc_cap.data_pins == 8)
        host->caps |= MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;

    // Light To do: If different MSDC host require different MSDC_CUST_VDD?
    #if 0 //Light To do: confirm which section shall be used
    host->ocr_avail = MSDC_CUST_VDD;
    #else
    host->ocr_avail = MMC_VDD_27_36;
    #endif
    if (host->caps & MMC_CAP_EMMC_HS200)
        host->ocr_avail |= MMC_VDD_165_195;

    host->max_hw_segs   = MAX_DMA_TRAN_SIZE/512;
    host->max_phys_segs = MAX_DMA_TRAN_SIZE/512;
    host->max_seg_size  = MAX_DMA_TRAN_SIZE;
    host->max_blk_size  = 2048;
    host->max_blk_count = 65535;

#if defined(MSDC_USE_DMA_MODE)
    cfg->sg             = &priv->sg[0];
    cfg->burstsz        = MSDC_BRUST_64B;
    cfg->flags          = DMA_FLAG_NONE;
    cfg->mode    = mode;
    cfg->inboot  = 0;

    msdc_init_gpd_bd(host);
    priv->alloc_bd      = 0;
    priv->alloc_gpd     = 0;
    priv->active_head   = NULL;
    priv->active_tail   = NULL;

#endif

    priv->dsmpl         = msdc_cap.data_edge;
    priv->rsmpl         = msdc_cap.cmd_edge;

    //#if defined(MMC_MSDC_DRV_CTP)
    #if 0
    if (host->id == 0)
        enable_clock(MT_CG_MSDC0_SW_CG, "MSDC0");
    else if (host->id == 1)
        enable_clock(MT_CG_MSDC1_SW_CG, "MSDC1");
    #else
    if (host->id == 0)
        MSDC_SET_FIELD(CLK_GATING_ADDR, MSDC0_CLK_GATING_EN, 0);
    else if (host->id == 1)
        MSDC_SET_FIELD(CLK_GATING_ADDR, MSDC1_CLK_GATING_EN, 0);
    #endif

    //Setting Host power and Card power
    msdc_power(host, MMC_POWER_OFF);
    msdc_power(host, MMC_POWER_ON);

    /* set to SD/MMC mode */
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_MODE, MSDC_SDMMC);//SD 3.0 host not support Memory stick mode(YD comment).
                            //Before MSDC_RESET, SW should switch to SD mode first
                            //(FPGA will not be gate by MSDC_CFG[0] = 0,but ASIC will.)

    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_CLR_INT();

    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO);

    /* enable SDIO mode. it's must otherwise sdio command failed */
    MSDC_SET_BIT32(SDC_CFG, SDC_CFG_SDIO);

    msdc_init_reg(host);

    /* enable wake up events */
    MSDC_SET_BIT32(SDC_CFG, SDC_CFG_INSWKUP); //Light To do: check if Preloader/LK need this line

    msdc_config_clksrc(host, clksrc);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_config_clock(host, 0, MSDC_MIN_SCLK);

    msdc_set_dmode(host, mode);
    msdc_set_pio_bits(host, 32);

    /* disable sdio interrupt by default. sdio interrupt enable upon request */
    msdc_intr_unmask(host, 0x0001FF7B);

    msdc_irq_init(host);

    msdc_set_timeout(host, 100000000, 0);
    msdc_card_detect(host, 1);

    return 0;
}

#if defined(MSDC_WITH_DEINIT)
int msdc_deinit(struct mmc_host *host)
{
    u32 base = host->base;

    #ifdef FEATURE_MMC_CARD_DETECT
    msdc_card_detect(host, 0);
    #endif

    msdc_intr_mask(host, 0x0001FFFB);

    msdc_irq_deinit(host);

    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_CLR_INT();
    msdc_power(host, MMC_POWER_OFF);

    msdc_restore_pin_mux(host);

    return 0;
}
#endif

#if defined(MMC_MSDC_DRV_CTP)
int msdc_polling_CD_interrupt(struct mmc_host *host)
{
    u32 base = host->base;
    u32 intsts;
    intsts = MSDC_READ32(MSDC_INT);
    MSDC_WRITE32(MSDC_INT, intsts);
    //printf("SDIO INT(0x%x)\n",intsts);
    if(intsts & MSDC_INT_CDSC)
        return 1;
    else
        return 0;
}
#endif

// 2012-02-25: Apply ett tool result
//Note: Yuchi said that LK/Preloader do not use it. However, 6589's preloader on JB2 calls this function.
#if defined(MMC_MSDC_DRV_CTP)
static int msdc_ett_offline(struct mmc_host *host, struct mmc_card *card)
{
    int ret = 1;
    int size = sizeof(g_mmcTable) / sizeof(mmcdev_info);
    int i, temp;
    u32 base = host->base;
    u8  m_id = card->cid.manfid;
    char * pro_name = card->cid.prod_name;

    printf("msdc_ett_offline_to_pl: size<%d> m_id<0x%x>\n", size, m_id);

    for (i = 0; i < size; i++) {
        printf("msdc <%d> <%s> <%s>\n", i, g_mmcTable[i].pro_name, pro_name);

        if ((g_mmcTable[i].m_id == m_id) && (!strncmp(g_mmcTable[i].pro_name, pro_name, 6))) {
            printf("msdc ett index<%d>: <%d> <%d> <0x%x> <0x%x> <0x%x>\n", i,
            g_mmcTable[i].r_smpl, g_mmcTable[i].d_smpl,
            g_mmcTable[i].cmd_rxdly, g_mmcTable[i].rd_rxdly, g_mmcTable[i].wr_rxdly);

            // set to msdc0
            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, g_mmcTable[i].r_smpl);
            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, g_mmcTable[i].d_smpl);

            MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, g_mmcTable[i].cmd_rxdly);
            MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, g_mmcTable[i].rd_rxdly);
            MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, g_mmcTable[i].wr_rxdly);

            temp = g_mmcTable[i].rd_rxdly; temp &= 0x1F;
            MSDC_WRITE32(MSDC_DAT_RDDLY0, (temp<<0 | temp<<8 | temp<<16 | temp<<24));
            MSDC_WRITE32(MSDC_DAT_RDDLY1, (temp<<0 | temp<<8 | temp<<16 | temp<<24));

            ret = 0;
            break;
        }
    }

    if (ret) printf("msdc failed to find\n");
        return ret;
}
#endif

#if defined(MMC_TEST)
typedef struct {
    /* register offset */
    u32  offset;
    /* r: read only, w: write only, a:readable & writable
     * k: read clear, x: don't care
     * s: readable and write 1 set, c: readable and write 1 clear
     */
    char attr[32];
    /* 0: default is 0
     * 1: default is 1
     * x: don't care
     */
    char reset[32];
} reg_desc_t;

static reg_desc_t msdc_reg_desc[] = {

    {OFFSET_MSDC_CFG      , {"aaraaarraaaaaaaaaaxxxxxxxxxxxxxx"}, {"100110010000000000xxxxxxxxxxxxxx"}}, //bit[2] is rw, but not test. cause this is a reset bit. write to "1" will back to "0" when reset sequence done
    {OFFSET_MSDC_IOCON    , {"aaaaaaxxaaaaaaxxaaaaaaaaxxxxxxxx"}, {"000000xx000000xx0000000000xxxxxx"}},//bit[24][25] can not write(Not in MT8320 MSDC IP)
    //{OFFSET_MSDC_PS       , {"arxxxxxxxxxxaaaarrrrrrrrrxxxxxxr"}, {"01xxxxxxxxxx0000111100001xxxxxx1"}},//bit[1]/bit[16-20]/bit[24]/bit[31] default to 0
    {OFFSET_MSDC_INT      , {"ccxccccccccrcccccxxxxxxxxxxxxxxx"}, {"00x00000000000000xxxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_INTEN    , {"aaaaaaaaaaaaaaaaaxxxxxxxxxxxxxxx"}, {"00000000000000000xxxxxxxxxxxxxxx"}},//bit[2] to RW, default to 0
    {OFFSET_MSDC_FIFOCS   , {"rrrrrrrrxxxxxxxxrrrrrrrrxxxxxxxx"}, {"00000000xxxxxxxx00000000xxxxxxx0"}},//bit[0]~[7] are variable if you use memory tool in Code visor (memory tool must be closed)
    {OFFSET_MSDC_TXDATA   , {"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww"}, {"00000000000000000000000000000000"}},

    /* Should not be touched. */
    //{OFFSET_MSDC_RXDATA   , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},

    {OFFSET_SDC_CFG   , {"aaxxxxxxxxxxxxxxaaxaaaxxaaaaaaaa"}, {"00xxxxxxxxxxxxxx00x010xx00000000"}},
    {OFFSET_SDC_CMD   , {"aaaaaaaaaaxaaaaaaaaaaaaaaaaaaaax"}, {"0000000000x00000000000000000000x"}},
    {OFFSET_SDC_ARG   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_STS   , {"rrxxxxxxxxxxxxxxxxxxxxxxxxxxxxxr"}, {"00xxxxxxxxxxxxxxxxxxxxxxxxxxxxx0"}},
    {OFFSET_SDC_RESP0     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_RESP1     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_RESP2     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_RESP3     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_BLK_NUM   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"10000000000000000000000000000000"}},
    {OFFSET_SDC_CSTS      , {"cccccccccccccccccccccccccccccccc"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_CSTS_EN   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_DCRC_STS  , {"rrrrrrrrrrrrrrrrxxxxxxxxxxxxxxxx"}, {"0000000000000000xxxxxxxxxxxxxxxx"}},//bit[12-15] modify
    {OFFSET_EMMC_CFG0     , {"wwaaxxxxxxxxaaaaxxxxxxxxxxxxxxxx"}, {"0000xxxxxxxx0000xxxxxxxxxxxxxxxx"}},
    {OFFSET_EMMC_CFG1     , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"11000000000000000000010000000000"}},
    {OFFSET_EMMC_STS      , {"ccccrcrxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"00000000xxxxxxxxxxxxxxxxxxxxxxxx"}},////bit[6]=1,0 is wrong,maybe has relationship with OFFSET_MSDC_FIFOCS register bit[0]~[7] which are variable

    {OFFSET_EMMC_IOCON    , {"axxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_SDC_ACMD_RESP , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_ACMD19_TRG, {"aaaaxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"0000xxxxxxxxxxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_SDC_ACMD19_STS, {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_SA   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_CA   , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_CTRL , {"wrwxxxxxaxaxaaaxxxxxxxxxxxxxxxxx"}, {"000xxxxx0x0x011xxxxxxxxxxxxxxxxx"}}, //need new design, bit[1] = AO
    {OFFSET_MSDC_DMA_CFG  , {"raxxxxxxaaxxaaxxaxxxxxxxxxxxxxxx"}, {"00xxxxxx00xx00xx0xxxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_DBG_SEL  , {"aaaaaaaaaaaaaaaaxxxxxxxxxxxxxxxx"}, {"0000000000000000xxxxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_DBG_OUT  , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_LEN  , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}}, //new register in MT6582
    {OFFSET_MSDC_PATCH_BIT0,{"aaaaaaaaaaaaaaaaaaaaaaxxxxaaaaaa"}, {"1111001000000000001111xxxx000010"}},
    {OFFSET_MSDC_PATCH_BIT1,{"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"10010000000000001111111111111111"}},

    /* PAD_CTL0/1/2 deleted in MSDC IP(mt6582) for cost reason */
    //{OFFSET_MSDC_PAD_CTL0,  {"aaaxaaaxaaaaxxxaaaaaaaaaaaaaaaaa"}, {"000x000x0000xxx11001000000000000"}},
    //{OFFSET_MSDC_PAD_CTL1,  {"aaaxaaaxaaaaxxxaaaaaaaaaaaaaaaaa"}, {"000x000x0000xxx00101000000000000"}},
    //{OFFSET_MSDC_PAD_CTL2,  {"aaaxaaaxaxxxxxxaaaaaaaaaaaaaaaaa"}, {"000x000x0000xxx00101000000000000"}},
    {OFFSET_MSDC_PAD_TUNE , {"aaaaaxxxaaaaaxxxaaaaaxaaaaaaaaaa"}, {"00000xxx00000xxx00000x0000000000"}},
    {OFFSET_MSDC_DAT_RDDLY0,{"aaaaaxxxaaaaaxxxaaaaaxxxaaaaaxxx"}, {"00000xxx00000xxx00000xxx00000xxx"}},
    {OFFSET_MSDC_DAT_RDDLY1,{"aaaaaxxxaaaaaxxxaaaaaxxxaaaaaxxx"}, {"00000xxx00000xxx00000xxx00000xxx"}},
    {OFFSET_MSDC_HW_DBG   , {"aaaaaaaaaaaaaaxxaaaaaaaaaaaaaaax"}, {"00000000000000xx000000000000000x"}},
    {OFFSET_MSDC_VERSION  , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00100000111000000100100000000100"}},//20120704
    {OFFSET_MSDC_ECO_VER  , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}}

};

int msdc_reg_test(int id)
{
    u32 baddr[] = {MSDC0_BASE, MSDC1_BASE};
    u32 base = baddr[id];
    u32 i, j, k;
    char v ;
    int error = MMC_ERR_NONE;
    /* check register reset value */
    for (i = 0; i < ARRAY_SIZE(msdc_reg_desc); i++) {
        for (j = 0; j < 32; j++) {
            if (('w' == (msdc_reg_desc[i].attr[j])) ||
                ('x' == (msdc_reg_desc[i].attr[j])))
                continue;
            // printf("read address(0x%x),0xC122003C[0](0x%x)\n",base + msdc_reg_desc[i].offset,MSDC_READ32(base + msdc_reg_desc[10].offset));
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;
            if (v != (msdc_reg_desc[i].reset[j] - '0')) {
                printf("[SD%d] Invalid Reset Value in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v,
                    msdc_reg_desc[i].reset[j] - '0');
                error = MMC_ERR_FAILED;
            }
        }
    }
#if 1
    /* check read only register */
    for (i = 0; i < ARRAY_SIZE(msdc_reg_desc); i++) {
        for (j = 0; j < 32; j++) {
            if ('r' != (msdc_reg_desc[i].attr[j]))
                continue;
            if (0 == (msdc_reg_desc[i].reset[j] - '0'))
                MSDC_SET_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            else
                MSDC_CLR_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            //printf("readonly address(0x%x),0xC122003C[0](0x%x)\n",base + msdc_reg_desc[i].offset,MSDC_READ32(base + msdc_reg_desc[10].offset));
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;
            if (v != (msdc_reg_desc[i].reset[j] - '0')) {
                printf("[SD%d] Read Only Reg Modified in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v,
                    msdc_reg_desc[i].reset[j] - '0');
                error = MMC_ERR_FAILED;
            }
        }
    }

#endif


#if 1
    /* check write register */
    for (i = 0; i < ARRAY_SIZE(msdc_reg_desc); i++) {
        for (j = 0; j < 32; j++) {
            if ('a' != (msdc_reg_desc[i].attr[j]))
                continue;

            MSDC_SET_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;

            if (v != 1) {
                printf("[SD%d] Write 1 Reg Failed in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v,
                    msdc_reg_desc[i].reset[j] - '0');
                error = MMC_ERR_FAILED;
            }

            /* exception rule for clock stable */
            if (((u32)MSDC_CFG == (base + msdc_reg_desc[i].offset)) &&
                (j >= 8 && j < 18)) { /* wait clock stable */
                while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
            }

            /* check other bits are not affected by write 1 */
            for (k = 0; k < 32; k++) {
                if (k == j)
                    continue;
                if (('w' == (msdc_reg_desc[i].attr[k])) ||
                    ('x' == (msdc_reg_desc[i].attr[k])))
                    continue;
                v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> k) & 0x1;
                if (v != (msdc_reg_desc[i].reset[k] - '0')) {
                    printf("[SD%d] Affected by Write 1 to 0x%x[%d] and [%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, k, v,
                    msdc_reg_desc[i].reset[k] - '0');
                     error = MMC_ERR_FAILED;
                }
            }

            /* write 0 to target bit */
            MSDC_CLR_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;

            if (v != 0) {
                printf("[SD%d] Write 0 Reg Failed in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v,
                    msdc_reg_desc[i].reset[j] - '0');
                error = MMC_ERR_FAILED;
            }

            /* exception rule for clock stable */
            if (((u32)MSDC_CFG == (base + msdc_reg_desc[i].offset)) &&
                (j >= 8 && j < 18)) { /* wait clock stable */
                while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
            }

            /* check other bits are not affected by write 1 */
            for (k = 0; k < 32; k++) {
                if (k == j)
                    continue;
                if (('w' == (msdc_reg_desc[i].attr[k])) ||
                    ('x' == (msdc_reg_desc[i].attr[k])))
                    continue;
                v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> k) & 0x1;
                if (v != (msdc_reg_desc[i].reset[k] - '0')) {
                    printf("[SD%d] Affected by Write 0 to 0x%x[%d] and [%d]=%d != %d\n",
                        id, base + msdc_reg_desc[i].offset, j, k, v,
                        msdc_reg_desc[i].reset[k] - '0');
                    error = MMC_ERR_FAILED;
                }
            }

            /* reset to default value */
            if ((msdc_reg_desc[i].reset[j] - '0') == 1)
                MSDC_SET_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            else{
                MSDC_CLR_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            }

        }
    }

#endif
    return error;
}

void msdc_dump_register(struct mmc_host *host)
{
    u32 base = host->base;
    printf("[SD%d] Reg[00] MSDC_CFG     = 0x%x\n", host->id,*(u32*)(base + 0x00));
    printf("[SD%d] Reg[04] MSDC_IOCON   = 0x%x\n", host->id,*(u32*)(base + 0x04));
    printf("[SD%d] Reg[08] MSDC_PS      = 0x%x\n", host->id,*(u32*)(base + 0x08));
    printf("[SD%d] Reg[0C] MSDC_INT     = 0x%x\n", host->id,*(u32*)(base + 0x0C));
    printf("[SD%d] Reg[10] MSDC_INTEN   = 0x%x\n", host->id,*(u32*)(base + 0x10));
    printf("[SD%d] Reg[14] MSDC_FIFOCS  = 0x%x\n", host->id,*(u32*)(base + 0x14));
    printf("[SD%d] Reg[18] MSDC_TXDATA  = not read\n");
    printf("[SD%d] Reg[1C] MSDC_RXDATA  = not read\n");
    printf("[SD%d] Reg[30] SDC_CFG      = 0x%x\n", host->id,*(u32*)(base + 0x30));
    printf("[SD%d] Reg[34] SDC_CMD      = 0x%x\n", host->id,*(u32*)(base + 0x34));
    printf("[SD%d] Reg[38] SDC_ARG      = 0x%x\n", host->id,*(u32*)(base + 0x38));
    printf("[SD%d] Reg[3C] SDC_STS      = 0x%x\n", host->id,*(u32*)(base + 0x3C));
    printf("[SD%d] Reg[40] SDC_RESP0    = 0x%x\n", host->id,*(u32*)(base + 0x40));
    printf("[SD%d] Reg[44] SDC_RESP1    = 0x%x\n", host->id,*(u32*)(base + 0x44));
    printf("[SD%d] Reg[48] SDC_RESP2    = 0x%x\n", host->id,*(u32*)(base + 0x48));
    printf("[SD%d] Reg[4C] SDC_RESP3    = 0x%x\n", host->id,*(u32*)(base + 0x4C));
    printf("[SD%d] Reg[50] SDC_BLK_NUM  = 0x%x\n", host->id,*(u32*)(base + 0x50));
    printf("[SD%d] Reg[58] SDC_CSTS     = 0x%x\n", host->id,*(u32*)(base + 0x58));
    printf("[SD%d] Reg[5C] SDC_CSTS_EN  = 0x%x\n", host->id,*(u32*)(base + 0x5C));
    printf("[SD%d] Reg[60] SDC_DATCRC_STS = 0x%x\n", host->id,*(u32*)(base + 0x60));
    printf("[SD%d] Reg[70] EMMC_CFG0    = 0x%x\n", host->id,*(u32*)(base + 0x70));
    printf("[SD%d] Reg[74] EMMC_CFG1    = 0x%x\n", host->id,*(u32*)(base + 0x74));
    printf("[SD%d] Reg[78] EMMC_STS     = 0x%x\n", host->id,*(u32*)(base + 0x78));
    printf("[SD%d] Reg[7C] EMMC_IOCON   = 0x%x\n", host->id,*(u32*)(base + 0x7C));
    printf("[SD%d] Reg[80] SD_ACMD_RESP = 0x%x\n", host->id,*(u32*)(base + 0x80));
    printf("[SD%d] Reg[84] SD_ACMD19_TRG= 0x%x\n", host->id,*(u32*)(base + 0x84));
    printf("[SD%d] Reg[88] SD_ACMD19_STS= 0x%x\n", host->id,*(u32*)(base + 0x88));
    printf("[SD%d] Reg[90] DMA_SA       = 0x%x\n", host->id,*(u32*)(base + 0x90));
    printf("[SD%d] Reg[94] DMA_CA       = 0x%x\n", host->id,*(u32*)(base + 0x94));
    printf("[SD%d] Reg[98] DMA_CTRL     = 0x%x\n", host->id,*(u32*)(base + 0x98));
    printf("[SD%d] Reg[9C] DMA_CFG      = 0x%x\n", host->id,*(u32*)(base + 0x9C));
    printf("[SD%d] Reg[A0] SW_DBG_SEL   = 0x%x\n", host->id,*(u32*)(base + 0xA0));
    printf("[SD%d] Reg[A4] SW_DBG_OUT   = 0x%x\n", host->id,*(u32*)(base + 0xA4));
    printf("[SD%d] Reg[B0] PATCH_BIT0   = 0x%x\n", host->id,*(u32*)(base + 0xB0));
    printf("[SD%d] Reg[B4] PATCH_BIT1   = 0x%x\n", host->id,*(u32*)(base + 0xB4));
    printf("[SD%d] Reg[E0] PAD_CTL0     = 0x%x\n", host->id,*(u32*)(base + 0xE0));
    printf("[SD%d] Reg[E4] PAD_CTL1     = 0x%x\n", host->id,*(u32*)(base + 0xE4));
    printf("[SD%d] Reg[E8] PAD_CTL2     = 0x%x\n", host->id,*(u32*)(base + 0xE8));
    printf("[SD%d] Reg[EC] PAD_TUNE     = 0x%x\n", host->id,*(u32*)(base + 0xEC));
    printf("[SD%d] Reg[F0] DAT_RD_DLY0  = 0x%x\n", host->id,*(u32*)(base + 0xF0));
    printf("[SD%d] Reg[F4] DAT_RD_DLY1  = 0x%x\n", host->id,*(u32*)(base + 0xF4));
    printf("[SD%d] Reg[F8] HW_DBG_SEL   = 0x%x\n", host->id,*(u32*)(base + 0xF8));
    printf("[SD%d] Reg[100] MAIN_VER    = 0x%x\n", host->id,*(u32*)(base + 0x100));
    printf("[SD%d] Reg[104] ECO_VER     = 0x%x\n", host->id,*(u32*)(base + 0x104));
}
#endif



