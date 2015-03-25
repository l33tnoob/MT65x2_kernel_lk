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

#ifndef _MSDC_H_
#define _MSDC_H_

#include "msdc_cfg.h"

#if defined(MMC_MSDC_DRV_CTP)
#include "reg_base.H"
#include "intrCtrl.h"
#endif

#if defined(MMC_MSDC_DRV_PRELOADER)
#include "platform.h"
#endif

#if defined(MMC_MSDC_DRV_LK)
#include "mt_reg_base.h"
#endif

#include "msdc_cust.h"
#include "msdc_utils.h"
#include "msdc_irq.h" //The inclusion shall be placed after mmc_core.h
#if defined(MSDC_USE_DMA_MODE)
#include "msdc_dma.h"
#endif
#include "mmc_core.h"
#include "mmc_test.h"
#if MMC_DEBUG
#include "mmc_dbg.h"
#endif

#if !defined(IO_CONFIG_BOTTOM_BASE)
#define IO_CONFIG_BOTTOM_BASE   IO_CFG_BOTTOM_BASE
#endif
#if !defined(IO_CONFIG_RIGHT_BASE)
#define IO_CONFIG_RIGHT_BASE    IO_CFG_RIGHT_BASE
#endif

#if defined(MSDC0_base)
#define MSDC0_BASE		(MSDC0_base)
#endif
#if defined(MSDC0_base)
#define MSDC1_BASE		(MSDC1_base)
#endif


/*--------------------------------------------------------------------------*/
/* Common Macro                                 */
/*--------------------------------------------------------------------------*/
#define REG_ADDR(x)         ((volatile uint32*)(base + OFFSET_##x))

/*--------------------------------------------------------------------------*/
/* Common Definition                                */
/*--------------------------------------------------------------------------*/
#define MSDC_FIFO_SZ        (128)
#define MSDC_FIFO_THD       (64)
#define MSDC_MAX_NUM        (2)

#define MSDC_MS             (0)
#define MSDC_SDMMC          (1)

#define MSDC_MODE_UNKNOWN   (0)
#define MSDC_MODE_PIO       (1)
#define MSDC_MODE_DMA_BASIC (2)
#define MSDC_MODE_DMA_DESC  (3)
#define MSDC_MODE_DMA_ENHANCED  (4)
#define MSDC_MODE_MMC_STREAM    (5)

#if defined(MSDC_MODE_DEFAULT_PIO)
#define MSDC_MODE_DEFAULT   MSDC_MODE_PIO
#elif defined(MSDC_MODE_DEFAULT_DMA_BASIC)
#define MSDC_MODE_DEFAULT   MSDC_MODE_DMA_BASIC
#elif defined(MSDC_MODE_DEFAULT_DMA_DESC)
#define MSDC_MODE_DEFAULT   MSDC_MODE_DMA_DESC
#elif defined(MSDC_MODE_DEFAULT_DMA_ENHANCED)
#define MSDC_MODE_DEFAULT   MSDC_MODE_ENHANCED
#elif defined(MSDC_MODE_DEFAULT_DMA_STREAM)
#define MSDC_MODE_DEFAULT   MSDC_MODE_STREAM
#else
#define MSDC_MODE_DEFAULT   MSDC_MODE_UNKNOWN
#endif

#define MSDC_BUS_1BITS      (0)
#define MSDC_BUS_4BITS      (1)
#define MSDC_BUS_8BITS      (2)

#define MSDC_BRUST_8B       (3)
#define MSDC_BRUST_16B      (4)
#define MSDC_BRUST_32B      (5)
#define MSDC_BRUST_64B      (6)

#define MSDC_PIN_PULL_NONE  (0)
#define MSDC_PIN_PULL_DOWN  (1)
#define MSDC_PIN_PULL_UP    (2)
#define MSDC_PIN_KEEP       (3)

#if defined(__FPGA__)
#define MSDC_SRC_CLK        (12000000)
#define MSDC_MAX_SCLK       (MSDC_SRC_CLK>>1)
#else
#define MSDC_MAX_SCLK       (208000000)
#endif
#define MSDC_MIN_SCLK       (260000)
#define MSDC_400K_SCLK      (400000)

#if defined(MMC_MSDC_DRV_CTP)
#define MSDC_TUNING_CLOCK_MIN_FREQ      1
#elif  defined(MMC_MSDC_DRV_PRELOADER)
#define MSDC_TUNING_CLOCK_MIN_FREQ      (MSDC_MAX_SCLK>>1)
#elif  defined(MMC_MSDC_DRV_LK)
#define MSDC_TUNING_CLOCK_MIN_FREQ      (MSDC_MAX_SCLK>>1)
#endif

#define MSDC_AUTOCMD12      (0x0001)
#define MSDC_AUTOCMD23      (0x0002)
#define MSDC_AUTOCMD19      (0x0003)

/*--------------------------------------------------------------------------*/
// PDN register offset definition.
/*--------------------------------------------------------------------------*/
#define TOP_Clk_Ctrl_base       (IO_PHYS + 0x0)
#define MSDC_SW_CG              ((volatile uint32*)(TOP_Clk_Ctrl_base + 0x0024))
#define MSDC_PDN_BIT            (1 << 17)

/*--------------------------------------------------------------------------*/
/* Register Offset                              */
/*--------------------------------------------------------------------------*/
#define OFFSET_MSDC_CFG         (0x0)
#define OFFSET_MSDC_IOCON       (0x04)
#define OFFSET_MSDC_PS          (0x08)
#define OFFSET_MSDC_INT         (0x0c)
#define OFFSET_MSDC_INTEN       (0x10)
#define OFFSET_MSDC_FIFOCS      (0x14)
#define OFFSET_MSDC_TXDATA      (0x18)
#define OFFSET_MSDC_RXDATA      (0x1c)
#define OFFSET_SDC_CFG          (0x30)
#define OFFSET_SDC_CMD          (0x34)
#define OFFSET_SDC_ARG          (0x38)
#define OFFSET_SDC_STS          (0x3c)
#define OFFSET_SDC_RESP0        (0x40)
#define OFFSET_SDC_RESP1        (0x44)
#define OFFSET_SDC_RESP2        (0x48)
#define OFFSET_SDC_RESP3        (0x4c)
#define OFFSET_SDC_BLK_NUM      (0x50)
#define OFFSET_SDC_CSTS         (0x58)
#define OFFSET_SDC_CSTS_EN      (0x5c)
#define OFFSET_SDC_DCRC_STS     (0x60)
#define OFFSET_EMMC_CFG0        (0x70)
#define OFFSET_EMMC_CFG1        (0x74)
#define OFFSET_EMMC_STS         (0x78)
#define OFFSET_EMMC_IOCON       (0x7c)
#define OFFSET_SDC_ACMD_RESP    (0x80)
#define OFFSET_SDC_ACMD19_TRG   (0x84)
#define OFFSET_SDC_ACMD19_STS   (0x88)
#define OFFSET_MSDC_DMA_SA      (0x90)
#define OFFSET_MSDC_DMA_CA      (0x94)
#define OFFSET_MSDC_DMA_CTRL    (0x98)
#define OFFSET_MSDC_DMA_CFG     (0x9c)
#define OFFSET_MSDC_DBG_SEL     (0xa0)
#define OFFSET_MSDC_DBG_OUT     (0xa4)
#define OFFSET_MSDC_DMA_LEN     (0xa8)
#define OFFSET_MSDC_PATCH_BIT0  (0xb0)
#define OFFSET_MSDC_PATCH_BIT1  (0xb4)
#define OFFSET_MSDC_PAD_CTL0    (0xe0)
#define OFFSET_MSDC_PAD_CTL1    (0xe4)
#define OFFSET_MSDC_PAD_CTL2    (0xe8)
#define OFFSET_MSDC_PAD_TUNE    (0xec)
#define OFFSET_MSDC_DAT_RDDLY0  (0xf0)
#define OFFSET_MSDC_DAT_RDDLY1  (0xf4)
#define OFFSET_MSDC_HW_DBG      (0xf8)
#define OFFSET_MSDC_VERSION     (0x100)
#define OFFSET_MSDC_ECO_VER     (0x104)

/*--------------------------------------------------------------------------*/
/* Register Address                             */
/*--------------------------------------------------------------------------*/
#define CLK_MUX_SEL_ADDR        (0x10000000)
#define MSDC0_CLK_MUX_SEL       (7<<11)
#define MSDC1_CLK_MUX_SEL       (7<<20)

#define CLK_GATING_ADDR         (0x10000024)
#define MSDC0_CLK_GATING_EN     (1<<17)
#define MSDC1_CLK_GATING_EN     (1<<18)

/* common register */
#define MSDC_CFG                REG_ADDR(MSDC_CFG)
#define MSDC_IOCON              REG_ADDR(MSDC_IOCON)
#define MSDC_PS                 REG_ADDR(MSDC_PS)
#define MSDC_INT                REG_ADDR(MSDC_INT)
#define MSDC_INTEN              REG_ADDR(MSDC_INTEN)
#define MSDC_FIFOCS             REG_ADDR(MSDC_FIFOCS)
#define MSDC_TXDATA             REG_ADDR(MSDC_TXDATA)
#define MSDC_RXDATA             REG_ADDR(MSDC_RXDATA)

/* sdmmc register */
#define SDC_CFG                 REG_ADDR(SDC_CFG)
#define SDC_CMD                 REG_ADDR(SDC_CMD)
#define SDC_ARG                 REG_ADDR(SDC_ARG)
#define SDC_STS                 REG_ADDR(SDC_STS)
#define SDC_RESP0               REG_ADDR(SDC_RESP0)
#define SDC_RESP1               REG_ADDR(SDC_RESP1)
#define SDC_RESP2               REG_ADDR(SDC_RESP2)
#define SDC_RESP3               REG_ADDR(SDC_RESP3)
#define SDC_BLK_NUM             REG_ADDR(SDC_BLK_NUM)
#define SDC_CSTS                REG_ADDR(SDC_CSTS)
#define SDC_CSTS_EN             REG_ADDR(SDC_CSTS_EN)
#define SDC_DCRC_STS            REG_ADDR(SDC_DCRC_STS)

/* emmc register*/
#define EMMC_CFG0               REG_ADDR(EMMC_CFG0)
#define EMMC_CFG1               REG_ADDR(EMMC_CFG1)
#define EMMC_STS                REG_ADDR(EMMC_STS)
#define EMMC_IOCON              REG_ADDR(EMMC_IOCON)

/* auto command register */
#define SDC_ACMD_RESP           REG_ADDR(SDC_ACMD_RESP)
#define SDC_ACMD19_TRG          REG_ADDR(SDC_ACMD19_TRG)
#define SDC_ACMD19_STS          REG_ADDR(SDC_ACMD19_STS)

/* dma register */
#define MSDC_DMA_SA             REG_ADDR(MSDC_DMA_SA)
#define MSDC_DMA_CA             REG_ADDR(MSDC_DMA_CA)
#define MSDC_DMA_CTRL           REG_ADDR(MSDC_DMA_CTRL)
#define MSDC_DMA_CFG            REG_ADDR(MSDC_DMA_CFG)
#define MSDC_DMA_LEN            REG_ADDR(MSDC_DMA_LEN)

/* pad ctrl register */
#define MSDC_PAD_CTL0           REG_ADDR(MSDC_PAD_CTL0)
#define MSDC_PAD_CTL1           REG_ADDR(MSDC_PAD_CTL1)
#define MSDC_PAD_CTL2           REG_ADDR(MSDC_PAD_CTL2)

/* data read delay */
#define MSDC_DAT_RDDLY0         REG_ADDR(MSDC_DAT_RDDLY0)
#define MSDC_DAT_RDDLY1         REG_ADDR(MSDC_DAT_RDDLY1)

/* debug register */
#define MSDC_DBG_SEL            REG_ADDR(MSDC_DBG_SEL)
#define MSDC_DBG_OUT            REG_ADDR(MSDC_DBG_OUT)

/* misc register */
#define MSDC_PATCH_BIT0         REG_ADDR(MSDC_PATCH_BIT0)
#define MSDC_PATCH_BIT1         REG_ADDR(MSDC_PATCH_BIT1)
#define MSDC_PAD_TUNE           REG_ADDR(MSDC_PAD_TUNE)
#define MSDC_HW_DBG             REG_ADDR(MSDC_HW_DBG)
#define MSDC_VERSION            REG_ADDR(MSDC_VERSION)
#define MSDC_ECO_VER            REG_ADDR(MSDC_ECO_VER)

/*--------------------------------------------------------------------------*/
/* Register Mask                                */
/*--------------------------------------------------------------------------*/

/* MSDC_CFG mask */
#define MSDC_CFG_MODE           (0x1  << 0) /* RW */
#define MSDC_CFG_CKPDN          (0x1  << 1) /* RW */
#define MSDC_CFG_RST            (0x1  << 2) /* RW */
#define MSDC_CFG_PIO            (0x1  << 3) /* RW */
#define MSDC_CFG_CKDRVEN        (0x1  << 4) /* RW */
#define MSDC_CFG_BV18SDT        (0x1  << 5) /* RW */
#define MSDC_CFG_BV18PSS        (0x1  << 6) /* R  */
#define MSDC_CFG_CKSTB          (0x1  << 7) /* R  */
#define MSDC_CFG_CKDIV          (0xff << 8) /* RW */
#define MSDC_CFG_CKMOD          (0x3  << 16)    /* RW */

/* MSDC_IOCON mask */
#define MSDC_IOCON_SDR104CKS    (0x1  << 0) /* RW */
#define MSDC_IOCON_RSPL         (0x1  << 1) /* RW */
#define MSDC_IOCON_DSPL         (0x1  << 2) /* RW */
#define MSDC_IOCON_DDLSEL       (0x1  << 3) /* RW */
#define MSDC_IOCON_DDR50CKD     (0x1  << 4) /* RW */
#define MSDC_IOCON_DSPLSEL      (0x1  << 5) /* RW */
//#define MSDC_IOCON_W_D_SMPL     (0x1  << 8)     /* RW */
#define MSDC_IOCON_WDSPL        (0x1  << 8) /* RW */
#define MSDC_IOCON_WDSPLSEL     (0x1  << 9) /* RW */
#define MSDC_IOCON_WD0SPL       (0x1  << 10)    /* RW */
#define MSDC_IOCON_WD1SPL       (0x1  << 11)    /* RW */
#define MSDC_IOCON_WD2SPL       (0x1  << 12)    /* RW */
#define MSDC_IOCON_WD3SPL       (0x1  << 13)    /* RW */
#define MSDC_IOCON_D0SPL        (0x1  << 16)    /* RW */
#define MSDC_IOCON_D1SPL        (0x1  << 17)    /* RW */
#define MSDC_IOCON_D2SPL        (0x1  << 18)    /* RW */
#define MSDC_IOCON_D3SPL        (0x1  << 19)    /* RW */
#define MSDC_IOCON_D4SPL        (0x1  << 20)    /* RW */
#define MSDC_IOCON_D5SPL        (0x1  << 21)    /* RW */
#define MSDC_IOCON_D6SPL        (0x1  << 22)    /* RW */
#define MSDC_IOCON_D7SPL        (0x1  << 23)    /* RW */
#define MSDC_IOCON_RISCSZ       (0x3  << 24)    /* RW */

/* MSDC_PS mask */
#define MSDC_PS_CDEN            (0x1  << 0) /* RW */
#define MSDC_PS_CDSTS           (0x1  << 1) /* R  */
#define MSDC_PS_CDDEBOUNCE      (0xf  << 12)    /* RW */
#define MSDC_PS_DAT             (0xFF << 16)    /* R  */
#define MSDC_PS_CMD             (0x1  << 24)    /* R  */
#define MSDC_PS_WP              (0x1UL<< 31)    /* R  */

/* MSDC_INT mask */
#define MSDC_INT_MMCIRQ         (0x1  << 0) /* W1C */
#define MSDC_INT_CDSC           (0x1  << 1) /* W1C */
#define MSDC_INT_ACMDRDY        (0x1  << 3) /* W1C */
#define MSDC_INT_ACMDTMO        (0x1  << 4) /* W1C */
#define MSDC_INT_ACMDCRCERR     (0x1  << 5) /* W1C */
#define MSDC_INT_DMAQ_EMPTY     (0x1  << 6) /* W1C */
#define MSDC_INT_SDIOIRQ        (0x1  << 7) /* W1C */
#define MSDC_INT_CMDRDY         (0x1  << 8) /* W1C */
#define MSDC_INT_CMDTMO         (0x1  << 9) /* W1C */
#define MSDC_INT_RSPCRCERR      (0x1  << 10)    /* W1C */
#define MSDC_INT_CSTA           (0x1  << 11)    /* R */
#define MSDC_INT_XFER_COMPL     (0x1  << 12)    /* W1C */
#define MSDC_INT_DXFER_DONE     (0x1  << 13)    /* W1C */
#define MSDC_INT_DATTMO         (0x1  << 14)    /* W1C */
#define MSDC_INT_DATCRCERR      (0x1  << 15)    /* W1C */
#define MSDC_INT_ACMD19_DONE    (0x1  << 16)    /* W1C */

/* MSDC_INTEN mask */
#define MSDC_INTEN_MMCIRQ       (0x1  << 0) /* RW */
#define MSDC_INTEN_CDSC         (0x1  << 1) /* RW */
#define MSDC_INTEN_ACMDRDY      (0x1  << 3) /* RW */
#define MSDC_INTEN_ACMDTMO      (0x1  << 4) /* RW */
#define MSDC_INTEN_ACMDCRCERR   (0x1  << 5) /* RW */
#define MSDC_INTEN_DMAQ_EMPTY   (0x1  << 6) /* RW */
#define MSDC_INTEN_SDIOIRQ      (0x1  << 7) /* RW */
#define MSDC_INTEN_CMDRDY       (0x1  << 8) /* RW */
#define MSDC_INTEN_CMDTMO       (0x1  << 9) /* RW */
#define MSDC_INTEN_RSPCRCERR    (0x1  << 10)    /* RW */
#define MSDC_INTEN_CSTA         (0x1  << 11)    /* RW */
#define MSDC_INTEN_XFER_COMPL   (0x1  << 12)    /* RW */
#define MSDC_INTEN_DXFER_DONE   (0x1  << 13)    /* RW */
#define MSDC_INTEN_DATTMO       (0x1  << 14)    /* RW */
#define MSDC_INTEN_DATCRCERR    (0x1  << 15)    /* RW */
#define MSDC_INTEN_ACMD19_DONE  (0x1  << 16)    /* RW */

/* MSDC_FIFOCS mask */
#define MSDC_FIFOCS_RXCNT       (0xff << 0) /* R */
#define MSDC_FIFOCS_TXCNT       (0xff << 16)    /* R */
#define MSDC_FIFOCS_CLR         (0x1UL<< 31)    /* RW */

/* SDC_CFG mask */
#define SDC_CFG_SDIOINTWKUP     (0x1  << 0) /* RW */
#define SDC_CFG_INSWKUP         (0x1  << 1) /* RW */
#define SDC_CFG_BUSWIDTH        (0x3  << 16)    /* RW */
#define SDC_CFG_SDIO            (0x1  << 19)    /* RW */
#define SDC_CFG_SDIOIDE         (0x1  << 20)    /* RW */
#define SDC_CFG_INTATGAP        (0x1  << 21)    /* RW */
#define SDC_CFG_DTOC            (0xFFUL << 24)  /* RW */

/* SDC_CMD mask */
#define SDC_CMD_OPC             (0x3F << 0) /* RW */
#define SDC_CMD_BRK             (0x1  << 6) /* RW */
#define SDC_CMD_RSPTYP          (0x7  << 7) /* RW */
#define SDC_CMD_DTYP            (0x3  << 11)    /* RW */
#define SDC_CMD_RW              (0x1  << 13)    /* RW */
#define SDC_CMD_STOP            (0x1  << 14)    /* RW */
#define SDC_CMD_GOIRQ           (0x1  << 15)    /* RW */
#define SDC_CMD_BLKLEN          (0xFFF<< 16)    /* RW */
#define SDC_CMD_AUTOCMD         (0x3  << 28)    /* RW */
#define SDC_CMD_VOLSWTH         (0x1  << 30)    /* RW */

/* SDC_STS mask */
#define SDC_STS_SDCBUSY         (0x1  << 0) /* RW */
#define SDC_STS_CMDBUSY         (0x1  << 1) /* RW */
#define SDC_STS_SWR_COMPL       (0x1UL<< 31)    /* RW */

/* SDC_DCRC_STS mask */
#define SDC_DCRC_STS_POS        (0xFF << 0) /* RO */
#define SDC_DCRC_STS_NEG        (0xFF << 8) /* RO */

/* EMMC_CFG0 mask */
#define EMMC_CFG0_BOOTSTART     (0x1  << 0) /* W */
#define EMMC_CFG0_BOOTSTOP      (0x1  << 1) /* W */
#define EMMC_CFG0_BOOTMODE      (0x1  << 2) /* RW */
#define EMMC_CFG0_BOOTACKDIS    (0x1  << 3) /* RW */
#define EMMC_CFG0_BOOTWDLY      (0x7  << 12)    /* RW */
#define EMMC_CFG0_BOOTSUPP      (0x1  << 15)    /* RW */

/* EMMC_CFG1 mask */
#define EMMC_CFG1_BOOTDATTMC    (0xFFFFF << 0)  /* RW */
#define EMMC_CFG1_BOOTACKTMC    (0xFFFUL << 20) /* RW */

/* EMMC_STS mask */
#define EMMC_STS_BOOTCRCERR     (0x1  << 0) /* W1C */
#define EMMC_STS_BOOTACKERR     (0x1  << 1) /* W1C */
#define EMMC_STS_BOOTDATTMO     (0x1  << 2) /* W1C */
#define EMMC_STS_BOOTACKTMO     (0x1  << 3) /* W1C */
#define EMMC_STS_BOOTUPSTATE    (0x1  << 4) /* R */
#define EMMC_STS_BOOTACKRCV     (0x1  << 5) /* W1C */
#define EMMC_STS_BOOTDATRCV     (0x1  << 6) /* R */

/* EMMC_IOCON mask */
#define EMMC_IOCON_BOOTRST      (0x1  << 0) /* RW */

/* SDC_ACMD19_TRG mask */
#define SDC_ACMD19_TRG_TUNESEL  (0xf  << 0) /* RW */

/* MSDC_DMA_CTRL mask */
#define MSDC_DMA_CTRL_START     (0x1  << 0) /* W */
#define MSDC_DMA_CTRL_STOP      (0x1  << 1) /* W */
#define MSDC_DMA_CTRL_RESUME    (0x1  << 2) /* W */
#define MSDC_DMA_CTRL_MODE      (0x1  << 8) /* RW */
#define MSDC_DMA_CTRL_LASTBUF   (0x1  << 10)    /* RW */
#define MSDC_DMA_CTRL_BRUSTSZ   (0x7  << 12)    /* RW */
//#define MSDC_DMA_CTRL_XFERSZ  (0xFFFFUL << 16)/* RW */

/* MSDC_DMA_CFG mask */
#define MSDC_DMA_CFG_STS        (0x1  << 0) /* R */
#define MSDC_DMA_CFG_DECSEN     (0x1  << 1) /* RW */
#define MSDC_DMA_CFG_BDCSERR    (0x1  << 4) /* R */
#define MSDC_DMA_CFG_GPDCSERR   (0x1  << 5) /* R */
#define MSDC_DMA_AHBHPROT2EN    (0x3  << 8) /* RW */
#define MSDC_DMA_ACTIVEEN       (0x3  << 12)    /* RW */

/* MSDC_PATCH_BIT mask */
//#define CKGEN_RX_SDClKO_SEL   (0x1    << 0)     /*This bit removed on MT6589/MT6572*/
#define MSDC_PATCH_BIT_ODDSUPP  (0x1    <<  1)    /* RW */
#define MSDC_PATCH_BIT0_PTCH02  (0x1    <<  2)    /* RW */
#define MSDC_PATCH_BIT0_PTCH4   (0x1    <<  4)    /* RW */ //ACMD53_FAIL_ONE_SHOT
#define MSDC_PATCH_BIT0_PTCH5   (0x1    <<  5)    /* RW */ //MASK_ACMD53_CRC_ERR_INTR
//#define MSDC_PATCH_BIT_CKGEN_CK (0x1  <<  6)    /* This bit removed on MT6589/MT7572 (Only use internel clock) */
#define MSDC_INT_DAT_LATCH_CK_SEL (0x7  <<  7)
#define MSDC_CKGEN_MSDC_DLY_SEL (0x1F   << 10)
#define MSDC_PATCH_BIT0_PTCH15  (0x1    << 15)    /* RW */ //Disable RXFIFO read
#define MSDC_PATCH_BIT_IODSSEL  (0x1    << 16)    /* RW */
#define MSDC_PATCH_BIT_IOINTSEL (0x1    << 17)    /* RW */
#define MSDC_PATCH_BIT_BUSYDLY  (0xF    << 18)    /* RW */
#define MSDC_PATCH_BIT_WDOD     (0xf    << 22)    /* RW */ //Removed on MT6572
#define MSDC_PATCH_BIT_IDRTSEL  (0x1    << 26)    /* RW */
#define MSDC_PATCH_BIT_CMDFSEL  (0x1    << 27)    /* RW */
#define MSDC_PATCH_BIT_INTDLSEL (0x1    << 28)    /* RW */
#define MSDC_PATCH_BIT_SPCPUSH  (0x1    << 29)    /* RW */
#define MSDC_PATCH_BIT_DECRCTMO (0x1    << 30)    /* RW */
#define MSDC_PATCH_BIT0_PTCH31  (0x1    << 31)    /* RW */ //Enable MSDC always drives bus when output wakeup response (BREAK)

/* MSDC_PATCH_BIT1 mask */
#define MSDC_PATCH_BIT1_WRDAT_CRCS  (0x7 << 0)
#define MSDC_CMD_RSP_TA_CNTR    (0x7 << 3)
#define MSDC_PATCH_BIT1_RESV2   (0x3FFFFFF << 6) /* RW */

/* MSDC_PAD_CTL0 mask */
#define MSDC_PAD_CTL0_CLKDRVN   (0x7  << 0) /* RW */
#define MSDC_PAD_CTL0_CLKDRVP   (0x7  << 4) /* RW */
//#define MSDC_PAD_CTL0_CLKSR0    (0x1  << 8)     /* RW */
//#define MSDC_PAD_CTL0_CLKSR1    (0x1  << 9)     /* RW */
//#define MSDC_PAD_CTL0_CLKSR2    (0x1  << 10)    /* RW */
//#define MSDC_PAD_CTL0_CLKSR3    (0x1  << 11)    /* RW */
//#define MSDC_PAD_CTL0_CLKPUPD   (0x1  << 15)    /* RW */
//#define MSDC_PAD_CTL0_CLKR1     (0x1  << 16)    /* RW */
//#define MSDC_PAD_CTL0_CLKR0     (0x1  << 17)    /* RW */
#define MSDC_PAD_CTL0_CLKSR     (0x1  << 8) /* RW */
#define MSDC_PAD_CTL0_CLKPD     (0x1  << 16)    /* RW */
#define MSDC_PAD_CTL0_CLKPU     (0x1  << 17)    /* RW */
#define MSDC_PAD_CTL0_CLKSMT    (0x1  << 18)    /* RW */
#define MSDC_PAD_CTL0_CLKIES    (0x1  << 19)    /* RW */
#define MSDC_PAD_CTL0_CLKTDSEL  (0xF  << 20)    /* RW */
#define MSDC_PAD_CTL0_CLKRDSEL  (0xFFUL<< 24)   /* RW */

/* MSDC_PAD_CTL1 mask */
#define MSDC_PAD_CTL1_CMDDRVN   (0x7  << 0) /* RW */
#define MSDC_PAD_CTL1_CMDDRVP   (0x7  << 4) /* RW */
//#define MSDC_PAD_CTL1_CMDSR0    (0x1  << 8)     /* RW */
//#define MSDC_PAD_CTL1_CMDSR1    (0x1  << 9)     /* RW */
//#define MSDC_PAD_CTL1_CMDSR2    (0x1  << 10)    /* RW */
//#define MSDC_PAD_CTL1_CMDSR3    (0x1  << 11)    /* RW */
//#define MSDC_PAD_CTL1_CMDPUPD   (0x1  << 15)    /* RW */
//#define MSDC_PAD_CTL1_CMDR1     (0x1  << 16)    /* RW */
//#define MSDC_PAD_CTL1_CMDR0     (0x1  << 17)    /* RW */
#define MSDC_PAD_CTL1_CMDSR     (0x1  << 8) /* RW */
#define MSDC_PAD_CTL1_CMDPD     (0x1  << 16)    /* RW */
#define MSDC_PAD_CTL1_CMDPU     (0x1  << 17)    /* RW */
#define MSDC_PAD_CTL1_CMDSMT    (0x1  << 18)    /* RW */
#define MSDC_PAD_CTL1_CMDIES    (0x1  << 19)    /* RW */
#define MSDC_PAD_CTL1_CMDTDSEL  (0xF  << 20)    /* RW */
#define MSDC_PAD_CTL1_CMDRDSEL  (0xFFUL<< 24)   /* RW */

/* MSDC_PAD_CTL2 mask */
#define MSDC_PAD_CTL2_DATDRVN   (0x7  << 0) /* RW */
#define MSDC_PAD_CTL2_DATDRVP   (0x7  << 4) /* RW */
//#define MSDC_PAD_CTL2_DATSR0    (0x1  << 8)     /* RW */
//#define MSDC_PAD_CTL2_DATSR1    (0x1  << 9)     /* RW */
//#define MSDC_PAD_CTL2_DATSR2    (0x1  << 10)    /* RW */
//#define MSDC_PAD_CTL2_DATSR3    (0x1  << 11)    /* RW */
//#define MSDC_PAD_CTL2_DATPUPD   (0x1  << 15)    /* RW */
//#define MSDC_PAD_CTL2_DATR1     (0x1  << 16)    /* RW */
//#define MSDC_PAD_CTL2_DATR0     (0x1  << 17)    /* RW */
#define MSDC_PAD_CTL2_DATSR     (0x1  << 8) /* RW */
#define MSDC_PAD_CTL2_DATPD     (0x1  << 16)    /* RW */
#define MSDC_PAD_CTL2_DATPU     (0x1  << 17)    /* RW */
#define MSDC_PAD_CTL2_DATSMT    (0x1  << 18)    /* RW */
#define MSDC_PAD_CTL2_DATIES    (0x1  << 19)    /* RW */
#define MSDC_PAD_CTL2_DATTDSEL  (0xF  << 20)    /* RW */
#define MSDC_PAD_CTL2_DATRDSEL  (0xFFUL<< 24)   /* RW */

/* MSDC_PAD_TUNE mask */
#define MSDC_PAD_TUNE_DATWRDLY  (0x1F << 0) /* RW */
#define MSDC_PAD_TUNE_DATRRDLY  (0x1F << 8) /* RW */
#define MSDC_PAD_TUNE_CMDRDLY   (0x1F << 16)    /* RW */
#define MSDC_PAD_TUNE_CMDRRDLY  (0x1FUL << 22)  /* RW */
#define MSDC_PAD_TUNE_CLKTXDLY  (0x1FUL << 27)  /* RW */

/* MSDC_DAT_RDDLY0/1 mask */
#define MSDC_DAT_RDDLY0_D3      (0x1F << 0) /* RW */
#define MSDC_DAT_RDDLY0_D2      (0x1F << 8) /* RW */
#define MSDC_DAT_RDDLY0_D1      (0x1F << 16)    /* RW */
#define MSDC_DAT_RDDLY0_D0      (0x1FUL<< 24)   /* RW */

#define MSDC_DAT_RDDLY1_D7      (0x1F << 0) /* RW */
#define MSDC_DAT_RDDLY1_D6      (0x1F << 8) /* RW */
#define MSDC_DAT_RDDLY1_D5      (0x1F << 16)    /* RW */
#define MSDC_DAT_RDDLY1_D4      (0x1FUL << 24)  /* RW */

/* HW_DBG_SEL mask */
#define MSDC_HW_DBG_DBG3SEL     (0xFF << 0) /* RW */
#define MSDC_HW_DBG_DBG2SEL     (0x3F << 8) /* RW */
#define MSDC_HW_DBG_DBG1SEL     (0x3F << 16)    /* RW */
#define MSDC_HW_DBG_DBGWTSEL    (0x3  << 22)    /* RW */
#define MSDC_HW_DBG_DBG0SEL     (0x3F << 24)    /* RW */
#define MSDC_HW_DBG_DBGWSEL     (0x1  << 30)    /* RW */

/********************MSDC0*************************************************/
//TBD Light, Need Review
#if !defined(GPIO_base) && defined(GPIO_BASE)
#define GPIO_base               (GPIO_BASE)
#endif

#define GPO_Base    0x10001E84
#define GPIO_DIR    0x10001E88

#if !defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_IO_CONFIG_BASE    (IO_CONFIG_RIGHT_BASE)  //Use Capital 'BASE' in CTP
#else
#define MSDC0_IO_CONFIG_BASE    (IO_CONFIG_BOTTOM_BASE) //Use Capital 'BASE' in CTP
#endif

#define MSDC1_IO_CONFIG_BASE    (IO_CONFIG_RIGHT_BASE)

#if !defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_CLK_PINMUX_ADDR   (GPIO_base+0x0370)
#define MSDC0_CLK_PINMUX_BITS   (7<<12)
#define MSDC0_CLK_PINMUX_VAL    (1<<12)

#define MSDC0_CMD_PINMUX_ADDR   (GPIO_base+0x0370)
#define MSDC0_CMD_PINMUX_BITS   (7<<16)
#define MSDC0_CMD_PINMUX_VAL    (1<<16)

#define MSDC0_DAT1_PINMUX_ADDR  (GPIO_base+0x0370)
#define MSDC0_DAT1_PINMUX_BITS  (7<<0)
#define MSDC0_DAT1_PINMUX_VAL   (1<<0)

#define MSDC0_DAT2_PINMUX_ADDR  (GPIO_base+0x0370)
#define MSDC0_DAT2_PINMUX_BITS  (7<<4)
#define MSDC0_DAT2_PINMUX_VAL   (1<<4)

#define MSDC0_DAT3_PINMUX_ADDR  (GPIO_base+0x0370)
#define MSDC0_DAT3_PINMUX_BITS  (7<<8)
#define MSDC0_DAT3_PINMUX_VAL   (1<<8)

#define MSDC0_DAT0_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT0_PINMUX_BITS  (7<<28)
#define MSDC0_DAT0_PINMUX_VAL   (1<<28)
#endif

#if defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_DAT1_PINMUX_ADDR  (GPIO_base+0x0350)
#define MSDC0_DAT1_PINMUX_BITS  (7<<28)
#define MSDC0_DAT1_PINMUX_VAL   (1<<28)

#define MSDC0_DAT0_PINMUX_ADDR  (GPIO_base+0x0350)
#define MSDC0_DAT0_PINMUX_BITS  (7<<24)
#define MSDC0_DAT0_PINMUX_VAL   (1<<24)

#define MSDC0_CMD_PINMUX_ADDR  (GPIO_base+0x0350)
#define MSDC0_CMD_PINMUX_BITS  (7<<20)
#define MSDC0_CMD_PINMUX_VAL   (1<<20)

#define MSDC0_CLK_PINMUX_ADDR  (GPIO_base+0x0350)
#define MSDC0_CLK_PINMUX_BITS  (7<<16)
#define MSDC0_CLK_PINMUX_VAL   (1<<16)

#define MSDC0_RST_PINMUX_ADDR   (GPIO_base+0x0360)
#define MSDC0_RST_PINMUX_BITS   (7<<24)
#define MSDC0_RST_PINMUX_VAL    (1<<24)

#define MSDC0_DAT7_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT7_PINMUX_BITS  (7<<20)
#define MSDC0_DAT7_PINMUX_VAL   (1<<20)

#define MSDC0_DAT6_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT6_PINMUX_BITS  (7<<16)
#define MSDC0_DAT6_PINMUX_VAL   (1<<16)

#define MSDC0_DAT5_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT5_PINMUX_BITS  (7<<12)
#define MSDC0_DAT5_PINMUX_VAL   (1<<12)

#define MSDC0_DAT4_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT4_PINMUX_BITS  (7<<8)
#define MSDC0_DAT4_PINMUX_VAL   (1<<8)

#define MSDC0_DAT3_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT3_PINMUX_BITS  (7<<4)
#define MSDC0_DAT3_PINMUX_VAL   (1<<4)

#define MSDC0_DAT2_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC0_DAT2_PINMUX_BITS  (7<<0)
#define MSDC0_DAT2_PINMUX_VAL   (1<<0)
#endif

#define MSDC1_INS1_PINMUX_ADDR  (GPIO_base+0x0330)
#define MSDC1_INS1_PINMUX_BITS  (7<<0)
#define MSDC1_INS1_PINMUX_VAL   (2<<0)

#define MSDC1_INS2_PINMUX_ADDR  (GPIO_base+0x0360)
#define MSDC1_INS2_PINMUX_BITS  (7<<20)
#define MSDC1_INS2_PINMUX_VAL   (2<<20)

#define MSDC1_INS3_PINMUX_ADDR  (GPIO_base+0x0380)
#define MSDC1_INS3_PINMUX_BITS  (7<<12)
#define MSDC1_INS3_PINMUX_VAL   (3<<12)

#define MSDC1_INS4_PINMUX_ADDR  (GPIO_base+0x0390)
#define MSDC1_INS4_PINMUX_BITS  (7<<4)
#define MSDC1_INS4_PINMUX_VAL   (4<<4)

#define MSDC1_INS5_PINMUX_ADDR  (GPIO_base+0x0390)
#define MSDC1_INS5_PINMUX_BITS  (7<<20)
#define MSDC1_INS5_PINMUX_VAL   (3<<20)

#define MSDC1_INS6_PINMUX_ADDR  (GPIO_base+0x03A0)
#define MSDC1_INS6_PINMUX_BITS  (7<<24)
#define MSDC1_INS6_PINMUX_VAL   (4<<24)

#define MSDC1_INS7_PINMUX_ADDR  (GPIO_base+0x03B0)
#define MSDC1_INS7_PINMUX_BITS  (7<<0)
#define MSDC1_INS7_PINMUX_VAL   (2<<0)

#define MSDC1_CLK_PINMUX_ADDR   (GPIO_base+0x03B0)
#define MSDC1_CLK_PINMUX_BITS   (7<<12)
#define MSDC1_CLK_PINMUX_VAL    (1<<12)

#define MSDC1_CMD_PINMUX_ADDR   (GPIO_base+0x03B0)
#define MSDC1_CMD_PINMUX_BITS   (7<<16)
#define MSDC1_CMD_PINMUX_VAL    (1<<16)

#define MSDC1_DAT0_PINMUX_ADDR  (GPIO_base+0x03B0)
#define MSDC1_DAT0_PINMUX_BITS  (7<<20)
#define MSDC1_DAT0_PINMUX_VAL   (1<<20)

#define MSDC1_DAT1_PINMUX_ADDR  (GPIO_base+0x03B0)
#define MSDC1_DAT1_PINMUX_BITS  (7<<24)
#define MSDC1_DAT1_PINMUX_VAL   (1<<24)

#define MSDC1_DAT2_PINMUX_ADDR  (GPIO_base+0x03B0)
#define MSDC1_DAT2_PINMUX_BITS  (7<<28)
#define MSDC1_DAT2_PINMUX_VAL   (1<<28)

#define MSDC1_DAT3_PINMUX_ADDR  (GPIO_base+0x03C0)
#define MSDC1_DAT3_PINMUX_BITS  (7<<0)
#define MSDC1_DAT3_PINMUX_VAL   (1<<0)

#define MSDC0_IES_CFG_BASE      (MSDC0_IO_CONFIG_BASE+0x0000)
#define MSDC0_IES_CFG_SET       (MSDC0_IO_CONFIG_BASE+0x0004)
#define MSDC0_IES_CFG_CLR       (MSDC0_IO_CONFIG_BASE+0x0008)
#define MSDC0_IES_DAT           (0x1 << 0)
#define MSDC0_IES_CK_CM         (0x1 << 1)
#define MSDC0_IES_CK_CM_DAT     (0x1 << 0)  //eMMC booting, Shall not use this for SD booting
#define MSDC0_IES_DAT7_4        (0x1 << 1)  //eMMC booting, Shall not use this for SD booting

#define MSDC0_SR_CFG_BASE       (MSDC0_IO_CONFIG_BASE+0x0010)
#define MSDC0_SR_CFG_SET        (MSDC0_IO_CONFIG_BASE+0x0014)
#define MSDC0_SR_CFG_CLR        (MSDC0_IO_CONFIG_BASE+0x0018)
#define MSDC0_SR_CK_CM_DAT      (0x1 << 0)
#define MSDC0_SR_DAT7_4         (0x1 << 1)  //eMMC booting, Shall not use this for SD booting

#define MSDC0_SMT_CFG_BASE      (MSDC0_IO_CONFIG_BASE+0x0020)
#define MSDC0_SMT_CFG_SET       (MSDC0_IO_CONFIG_BASE+0x0024)
#define MSDC0_SMT_CFG_CLR       (MSDC0_IO_CONFIG_BASE+0x0028)
#define MSDC0_SMT_CK_CM_DAT     (0x1 << 0)
#define MSDC0_SMT_DAT7_4        (0x1 << 1)  //eMMC booting, Shall not use this for SD booting

#define MSDC0_TDSEL_BASE        (MSDC0_IO_CONFIG_BASE+0x0030)
#define MSDC0_RDSEL_BASE        (MSDC0_IO_CONFIG_BASE+0x0034)
#if !defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_TDSEL             (0xF)
#define MSDC0_RDSEL             (0x3F)
#else
#define MSDC0_TDSEL_RST0        (0xF)
#define MSDC0_TDSEL             (0xF  << 4)
#define MSDC0_RDSEL_RST0        (0x3)
#define MSDC0_RDSEL             (0x3F << 2)
#endif

#define MSDC0_PULL_EN_CFG_BASE  (MSDC0_IO_CONFIG_BASE+0x0040)
#define MSDC0_PULL_EN_CFG_SET   (MSDC0_IO_CONFIG_BASE+0x0044)
#define MSDC0_PULL_EN_CFG_CLR   (MSDC0_IO_CONFIG_BASE+0x0048)

#if !defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_DAT0_PULL_EN   (0x1 << 0)
#define MSDC0_DAT1_PULL_EN   (0x1 << 1)
#define MSDC0_DAT2_PULL_EN   (0x1 << 2)
#define MSDC0_DAT3_PULL_EN   (0x1 << 3)
#define MSDC0_CLK_PULL_EN    (0x1 << 4)
#define MSDC0_CMD_PULL_EN    (0x1 << 5)
#else
#define MSDC0_CLK_PULL8K_EN     (0x3 << 0)
#define MSDC0_CLK_PULL50K_EN    (0x1 << 0)
#define MSDC0_CLK_PULL10K_EN    (0x1 << 1)
#define MSDC0_CMD_PULL8K_EN     (0x3 << 2)
#define MSDC0_CMD_PULL50K_EN    (0x1 << 2)
#define MSDC0_CMD_PULL10K_EN    (0x1 << 3)
#define MSDC0_DAT0_PULL8K_EN    (0x3 << 4)
#define MSDC0_DAT0_PULL50K_EN   (0x1 << 4)
#define MSDC0_DAT0_PULL10K_EN   (0x1 << 5)
#define MSDC0_DAT1_PULL8K_EN    (0x3 << 6)
#define MSDC0_DAT1_PULL50K_EN   (0x1 << 6)
#define MSDC0_DAT1_PULL10K_EN   (0x1 << 7)
#define MSDC0_DAT2_PULL8K_EN    (0x3 << 8)
#define MSDC0_DAT2_PULL50K_EN   (0x1 << 8)
#define MSDC0_DAT2_PULL10K_EN   (0x1 << 9)
#define MSDC0_DAT3_PULL8K_EN    (0x3 << 10)
#define MSDC0_DAT3_PULL50K_EN   (0x1 << 10)
#define MSDC0_DAT3_PULL10K_EN   (0x1 << 11)
#define MSDC0_DAT4_PULL8K_EN    (0x3 << 12)
#define MSDC0_DAT4_PULL50K_EN   (0x1 << 12)
#define MSDC0_DAT4_PULL10K_EN   (0x1 << 13)
#define MSDC0_DAT5_PULL8K_EN    (0x3 << 14)
#define MSDC0_DAT5_PULL50K_EN   (0x1 << 14)
#define MSDC0_DAT5_PULL10K_EN   (0x1 << 15)
#define MSDC0_DAT6_PULL8K_EN    (0x3 << 16)
#define MSDC0_DAT6_PULL50K_EN   (0x1 << 16)
#define MSDC0_DAT6_PULL10K_EN   (0x1 << 17)
#define MSDC0_DAT7_PULL8K_EN    (0x3 << 18)
#define MSDC0_DAT7_PULL50K_EN   (0x1 << 18)
#define MSDC0_DAT7_PULL10K_EN   (0x1 << 19)
#define MSDC0_RST_PULL_EN       (0x1 << 20)
#endif

#define MSDC0_PULL_SEL_CFG_BASE (MSDC0_IO_CONFIG_BASE+0x0050)
#define MSDC0_PULL_SEL_CFG_SET  (MSDC0_IO_CONFIG_BASE+0x0054)
#define MSDC0_PULL_SEL_CFG_CLR  (MSDC0_IO_CONFIG_BASE+0x0058)
#if !defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_DAT0_PULL_SEL     (0x1 << 0)
#define MSDC0_DAT1_PULL_SEL     (0x1 << 1)
#define MSDC0_DAT2_PULL_SEL     (0x1 << 2)
#define MSDC0_DAT3_PULL_SEL     (0x1 << 3)
#define MSDC0_CLK_PULL_SEL      (0x1 << 4)
#define MSDC0_CMD_PULL_SEL      (0x1 << 5)
#define MSDC0_PULL_SEL_ALL_MASK  (MSDC0_CLK_PULL_SEL  | \
                                 MSDC0_CMD_PULL_SEL  | \
                                 MSDC0_DAT0_PULL_SEL | \
                                 MSDC0_DAT1_PULL_SEL | \
                                 MSDC0_DAT2_PULL_SEL | \
                                 MSDC0_DAT3_PULL_SEL)
#else
#define MSDC0_CLK_PULL_SEL      (0x1 << 0)
#define MSDC0_CMD_PULL_SEL      (0x1 << 1)
#define MSDC0_DAT0_PULL_SEL     (0x1 << 2)
#define MSDC0_DAT1_PULL_SEL     (0x1 << 3)
#define MSDC0_DAT2_PULL_SEL     (0x1 << 4)
#define MSDC0_DAT3_PULL_SEL     (0x1 << 5)
#define MSDC0_DAT4_PULL_SEL     (0x1 << 6)  //Shall not use this for SD booting
#define MSDC0_DAT5_PULL_SEL     (0x1 << 7)  //Shall not use this for SD booting
#define MSDC0_DAT6_PULL_SEL     (0x1 << 8)  //Shall not use this for SD booting
#define MSDC0_DAT7_PULL_SEL     (0x1 << 9)  //Shall not use this for SD booting
#define MSDC0_RST_PULL_SEL      (0x1 << 10) //Shall not use this for SD booting
#define MSDC0_PULL_SEL_ALL_MASK  (MSDC0_CLK_PULL_SEL  | \
                                 MSDC0_CMD_PULL_SEL  | \
                                 MSDC0_DAT0_PULL_SEL | \
                                 MSDC0_DAT1_PULL_SEL | \
                                 MSDC0_DAT2_PULL_SEL | \
                                 MSDC0_DAT3_PULL_SEL | \
                                 MSDC0_DAT4_PULL_SEL | \
                                 MSDC0_DAT5_PULL_SEL | \
                                 MSDC0_DAT6_PULL_SEL | \
                                 MSDC0_DAT7_PULL_SEL | \
                                 MSDC0_RST_PULL_SEL)
#endif


#define MSDC0_DRVING_BASE   (MSDC0_IO_CONFIG_BASE+0x0060)
#define MSDC0_CK_CM_DAT_DRVING  (0x7 << 0)
#define MSDC0_DAT7_4_DRVING     (0x7 << 3)  //Shall not use this for SD booting
#define MSDC0_RST_DRVING        (0x3 << 6)  //Shall not use this for SD booting

#if defined(FEATURE_MMC_BOOT_MODE) //TBD Light
typedef enum _MSDC0_ODC {
    MSDC0_ODC_2MA = 0,
    MSDC0_ODC_4MA,
    MSDC0_ODC_6MA,
    MSDC0_ODC_8MA,
    MSDC0_ODC_10MA,
    MSDC0_ODC_12MA,
    MSDC0_ODC_14MA,
    MSDC0_ODC_16MA,
    MSDC0_ODC_NUM_MAX
} MSDC0_ODC;

#else
typedef enum _MSDC0_ODC {
    MSDC0_ODC_4MA = 0,
    MSDC0_ODC_8MA,
    MSDC0_ODC_12MA,
    MSDC0_ODC_16MA,
    MSDC0_ODC_20MA,
    MSDC0_ODC_24MA,
    MSDC0_ODC_28MA,
    MSDC0_ODC_32MA,
    MSDC0_ODC_NUM_MAX
} MSDC0_ODC;

#endif

typedef enum _MSDC1_ODC {
    MSDC1_ODC_4MA = 0,
    MSDC1_ODC_8MA,
    MSDC1_ODC_12MA,
    MSDC1_ODC_16MA,
    MSDC1_ODC_20MA,
    MSDC1_ODC_24MA,
    MSDC1_ODC_28MA,
    MSDC1_ODC_32MA,
    MSDC1_ODC_NUM_MAX
} MSDC1_ODC;

#define MSDC0_DINV_DRVING_BASE  (MSDC0_IO_CONFIG_BASE+0x0070)
#define MSDC0_DINV_DRVING_SET   (MSDC0_IO_CONFIG_BASE+0x0074)
#define MSDC0_DINV_DRVING_CLR   (MSDC0_IO_CONFIG_BASE+0x0078)
#if !defined(FEATURE_MMC_BOOT_MODE)
#define MSDC0_DAT0_DINV         (0x1 << 0)
#define MSDC0_DAT1_DINV         (0x1 << 1)
#define MSDC0_DAT2_DINV         (0x1 << 2)
#define MSDC0_DAT3_DINV         (0x1 << 3)
#define MSDC0_CLK_DINV          (0x1 << 4)
#define MSDC0_CMD_DINV          (0x1 << 5)
#else
#define MSDC0_CLK_DINV          (0x1 << 0)
#define MSDC0_CMD_DINV          (0x1 << 1)
#define MSDC0_DAT0_DINV         (0x1 << 2)
#define MSDC0_DAT1_DINV         (0x1 << 3)
#define MSDC0_DAT2_DINV         (0x1 << 4)
#define MSDC0_DAT3_DINV         (0x1 << 5)
#define MSDC0_DAT4_DINV         (0x1 << 6)  //Shall not use this for SD booting
#define MSDC0_DAT5_DINV         (0x1 << 7)  //Shall not use this for SD booting
#define MSDC0_DAT6_DINV         (0x1 << 8)  //Shall not use this for SD booting
#define MSDC0_DAT7_DINV         (0x1 << 9)  //Shall not use this for SD booting
#define MSDC0_RST_DINV          (0x1 << 10) //Shall not use this for SD booting
#endif


/****************************MSDC1*************************************************/
#define MSDC1_IES_CFG_BASE      (MSDC1_IO_CONFIG_BASE+0x0000)
#define MSDC1_IES_CFG_SET       (MSDC1_IO_CONFIG_BASE+0x0004)
#define MSDC1_IES_CFG_CLR       (MSDC1_IO_CONFIG_BASE+0x0008)
#define MSDC1_IES_CK_CM_DAT     (0x1 << 6)

#define MSDC1_SR_CFG_BASE       (MSDC1_IO_CONFIG_BASE+0x0010)
#define MSDC1_SR_CFG_SET        (MSDC1_IO_CONFIG_BASE+0x0014)
#define MSDC1_SR_CFG_CLR        (MSDC1_IO_CONFIG_BASE+0x0018)
#define MSDC1_SR_CK_CM_DAT      (0x1 << 2)

#define MSDC1_SMT_CFG_BASE      (MSDC1_IO_CONFIG_BASE+0x0020)
#define MSDC1_SMT_CFG_SET       (MSDC1_IO_CONFIG_BASE+0x0024)
#define MSDC1_SMT_CFG_CLR       (MSDC1_IO_CONFIG_BASE+0x0028)
#define MSDC1_SMT_CK_CM_DAT     (0x1 << 2)

#define MSDC1_TDSEL_BASE        (MSDC1_IO_CONFIG_BASE+0x0030)
#define MSDC1_RDSEL_BASE        (MSDC1_IO_CONFIG_BASE+0x0034)
#define MSDC1_TDSEL             (0xF  << 8)
#define MSDC1_RDSEL             (0x3F << 10)

#define MSDC1_PULL_EN_CFG_BASE  (MSDC1_IO_CONFIG_BASE+0x0040)
#define MSDC1_PULL_EN_CFG_SET   (MSDC1_IO_CONFIG_BASE+0x0044)
#define MSDC1_PULL_EN_CFG_CLR   (MSDC1_IO_CONFIG_BASE+0x0048)
#define MSDC1_CLK_PULL_EN       (0x1 << 16)
#define MSDC1_CMD_PULL_EN       (0x1 << 17)
#define MSDC1_DAT0_PULL_EN      (0x1 << 18)
#define MSDC1_DAT1_PULL_EN      (0x1 << 19)
#define MSDC1_DAT2_PULL_EN      (0x1 << 20)
#define MSDC1_DAT3_PULL_EN      (0x1 << 21)

#define MSDC1_PULL_SEL_CFG_BASE (MSDC1_IO_CONFIG_BASE+0x0050)
#define MSDC1_PULL_SEL_CFG_SET  (MSDC1_IO_CONFIG_BASE+0x0054)
#define MSDC1_PULL_SEL_CFG_CLR  (MSDC1_IO_CONFIG_BASE+0x0058)
#define MSDC1_CLK_PULL_SEL      (0x1 << 16)
#define MSDC1_CMD_PULL_SEL      (0x1 << 17)
#define MSDC1_DAT0_PULL_SEL     (0x1 << 18)
#define MSDC1_DAT1_PULL_SEL     (0x1 << 19)
#define MSDC1_DAT2_PULL_SEL     (0x1 << 20)
#define MSDC1_DAT3_PULL_SEL     (0x1 << 21)
#define MSDC1_PULL_SEL_ALL_MASK (MSDC1_CLK_PULL_SEL  | \
                                 MSDC1_CMD_PULL_SEL  | \
                                 MSDC1_DAT0_PULL_SEL | \
                                 MSDC1_DAT1_PULL_SEL | \
                                 MSDC1_DAT2_PULL_SEL | \
                                 MSDC1_DAT3_PULL_SEL)

#define MSDC1_DRVING_BASE   (MSDC1_IO_CONFIG_BASE+0x0060)
#define MSDC1_CK_CM_DAT_DRVING  (0x7 << 10)

#define MSDC1_DINV_DRVING_BASE  (MSDC1_IO_CONFIG_BASE+0x0070)
#define MSDC1_DINV_DRVING_SET   (MSDC1_IO_CONFIG_BASE+0x0074)
#define MSDC1_DINV_DRVING_CLR   (MSDC1_IO_CONFIG_BASE+0x0078)
#define MSDC1_CLK_DINV          (0x1 << 16)
#define MSDC1_CMD_DINV          (0x1 << 17)
#define MSDC1_DAT0_DINV         (0x1 << 18)
#define MSDC1_DAT1_DINV         (0x1 << 19)
#define MSDC1_DAT2_DINV         (0x1 << 20)
#define MSDC1_DAT3_DINV         (0x1 << 21)


/****************************Power and Voltage Control*************************************************/
//TBD Light
#define EN18IOKEY_BASE              (GPIO_BASE2+0x920)

#define MSDC_EN18IO_CMP_SEL_BASE    (GPIO_BASE2+0x910)
#define MSDC1_EN18IO_CMP_EN         (0x1 << 3)
#define MSDC1_EN18IO_SEL1           (0x7 << 0)
#define MSDC2_EN18IO_CMP_EN         (0x1 << 7)
#define MSDC2_EN18IO_SEL            (0x7 << 4)

#define MSDC_EN18IO_SW_BASE         (GPIO_BASE+0x900)
#define MSDC1_EN18IO_SW             (0x1 << 19)
#define MSDC2_EN18IO_SW             (0x1 << 25)

typedef enum MSDC_POWER {

    MSDC_VIO18_MC1 = 0,
    MSDC_VIO18_MC2,
    MSDC_VIO28_MC1,
    MSDC_VIO28_MC2,
    MSDC_VMC,
    MSDC_VGP,
} MSDC_POWER_DOMAIN;


/*--------------------------------------------------------------------------*/
/* Register Debugging Structure                         */
/*--------------------------------------------------------------------------*/

typedef struct {
    uint32 msdc:1;
    uint32 ckpwn:1;
    uint32 rst:1;
    uint32 pio:1;
    uint32 ckdrven:1;
    uint32 start18v:1;
    uint32 pass18v:1;
    uint32 ckstb:1;
    uint32 ckdiv:8;
    uint32 ckmod:2;
    uint32 pad:14;
} msdc_cfg_reg;
typedef struct {
    uint32 sdr104cksel:1;
    uint32 rsmpl:1;
    uint32 dsmpl:1;
    uint32 ddlysel:1;
    uint32 ddr50ckd:1;
    uint32 dsplsel:1;
    uint32 pad1:10;
    //Alternative definition for pad1
    //uint32 pad1_1:2;
    //uint32 wdspl:1;
    //uint32 wdsplsel:1;
    //uint32 wd0spl:1;
    //uint32 wd1spl:1;
    //uint32 wd2spl:1;
    //uint32 wd3spl:1;
    //uint32 pad1_2:2;
    uint32 d0spl:1;
    uint32 d1spl:1;
    uint32 d2spl:1;
    uint32 d3spl:1;
    uint32 d4spl:1;
    uint32 d5spl:1;
    uint32 d6spl:1;
    uint32 d7spl:1;
    uint32 riscsz:2;
    uint32 pad2:6;
} msdc_iocon_reg;
typedef struct {
    uint32 cden:1;
    uint32 cdsts:1;
    uint32 pad1:10;
    uint32 cddebounce:4;
    uint32 dat:8;
    uint32 cmd:1;
    uint32 pad2:6;
    uint32 wp:1;
} msdc_ps_reg;
typedef struct {
    uint32 mmcirq:1;
    uint32 cdsc:1;
    uint32 pad1:1;
    uint32 atocmdrdy:1;
    uint32 atocmdtmo:1;
    uint32 atocmdcrc:1;
    uint32 dmaqempty:1;
    uint32 sdioirq:1;
    uint32 cmdrdy:1;
    uint32 cmdtmo:1;
    uint32 rspcrc:1;
    uint32 csta:1;
    uint32 xfercomp:1;
    uint32 dxferdone:1;
    uint32 dattmo:1;
    uint32 datcrc:1;
    uint32 atocmd19done:1;
    uint32 pad2:15;
    //Alternative definiton for pad2
    //uint32 bdcserr:1;
    //uint32 gpdcserr:1;
    //uint32 dmaprotect:1;
    //uint32 gearoutbound:1;
    //uint32 ac53done:1;
    //uint32 ac53fail:1;
    //uint32 pad2:9;
} msdc_int_reg;
typedef struct {
    uint32 mmcirq:1;
    uint32 cdsc:1;
    uint32 pad1:1;
    uint32 atocmdrdy:1;
    uint32 atocmdtmo:1;
    uint32 atocmdcrc:1;
    uint32 dmaqempty:1;
    uint32 sdioirq:1;
    uint32 cmdrdy:1;
    uint32 cmdtmo:1;
    uint32 rspcrc:1;
    uint32 csta:1;
    uint32 xfercomp:1;
    uint32 dxferdone:1;
    uint32 dattmo:1;
    uint32 datcrc:1;
    uint32 atocmd19done:1;
    uint32 pad2:15;
    //Alternative definiton for pad2
    //uint32 bdcserr:1;
    //uint32 gpdcserr:1;
    //uint32 dmaprotect:1;
    //uint32 gearoutbound:1;
    //uint32 ac53done:1;
    //uint32 ac53fail:1;
    //uint32 pad2:9;
} msdc_inten_reg;
typedef struct {
    uint32 rxcnt:8;
    uint32 pad1:8;
    uint32 txcnt:8;
    uint32 pad2:7;
    uint32 clr:1;
} msdc_fifocs_reg;
typedef struct {
    uint32 val;
} msdc_txdat_reg;
typedef struct {
    uint32 val;
} msdc_rxdat_reg;
typedef struct {
    uint32 sdiowkup:1;
    uint32 inswkup:1;
    uint32 pad1:14;
    uint32 buswidth:2;
    uint32 pad2:1;
    uint32 sdio:1;
    uint32 sdioide:1;
    uint32 intblkgap:1;
    uint32 pad4:2;
    uint32 dtoc:8;
} sdc_cfg_reg;
typedef struct {
    uint32 cmd:6;
    uint32 brk:1;
    uint32 rsptyp:3;
    uint32 pad1:1;
    uint32 dtype:2;
    uint32 rw:1;
    uint32 stop:1;
    uint32 goirq:1;
    uint32 blklen:12;
    uint32 atocmd:2;
    uint32 volswth:1;
    uint32 pad2:1;
} sdc_cmd_reg;
typedef struct {
    uint32 arg;
} sdc_arg_reg;
typedef struct {
    uint32 sdcbusy:1;
    uint32 cmdbusy:1;
    uint32 pad:29;
    uint32 swrcmpl:1;
} sdc_sts_reg;
typedef struct {
    uint32 val;
} sdc_resp0_reg;
typedef struct {
    uint32 val;
} sdc_resp1_reg;
typedef struct {
    uint32 val;
} sdc_resp2_reg;
typedef struct {
    uint32 val;
} sdc_resp3_reg;
typedef struct {
    uint32 num;
} sdc_blknum_reg;
typedef struct {
    uint32 sts;
} sdc_csts_reg;
typedef struct {
    uint32 sts;
} sdc_cstsen_reg;
typedef struct {
    uint32 datcrcsts:8;
    uint32 ddrcrcsts:4;
    uint32 pad:20;
} sdc_datcrcsts_reg;
typedef struct {
    uint32 bootstart:1;
    uint32 bootstop:1;
    uint32 bootmode:1;
    uint32 bootachkdis:1;
    uint32 pad1:8;
    uint32 bootwaidly:3;
    uint32 bootsupp:1;
    uint32 pad2:16;
} emmc_cfg0_reg;
typedef struct {
    uint32 bootcrctmc:16;
    uint32 pad:4;
    uint32 bootacktmc:12;
} emmc_cfg1_reg;
typedef struct {
    uint32 bootcrcerr:1;
    uint32 bootackerr:1;
    uint32 bootdattmo:1;
    uint32 bootacktmo:1;
    uint32 bootupstate:1;
    uint32 bootackrcv:1;
    uint32 bootdatrcv:1;
    uint32 pad:25;
} emmc_sts_reg;
typedef struct {
    uint32 bootrst:1;
    uint32 pad:31;
} emmc_iocon_reg;
typedef struct {
    uint32 val;
} msdc_acmd_resp_reg;
typedef struct {
    uint32 tunesel:4;
    uint32 pad:28;
} msdc_acmd19_trg_reg;
typedef struct {
    uint32 val;
} msdc_acmd19_sts_reg;
typedef struct {
    uint32 addr;
} msdc_dma_sa_reg;
typedef struct {
    uint32 addr;
} msdc_dma_ca_reg;
typedef struct {
    uint32 start:1;
    uint32 stop:1;
    uint32 resume:1;
    uint32 pad1:5;
    uint32 mode:1;
    uint32 pad2:1;
    uint32 lastbuf:1;
    uint32 pad3:1;
    uint32 brustsz:3;
    uint32 pad4:1;
    uint32 xfersz:16;
} msdc_dma_ctrl_reg;
typedef struct {
    uint32 xfersz;
} msdc_dma_length;
typedef struct {
    uint32 status:1;
    uint32 decsen:1;
    uint32 pad1:6;
    uint32 ahbhprot2en:2;
    uint32 pad2:2;
    uint32 msdcactiveen:2;
    uint32 pad3:2;
    uint32 dmachksum12b:1;
    uint32 pad4:15;
} msdc_dma_cfg_reg;
typedef struct {
    uint32 sel:16;
    uint32 pad2:16;
} msdc_dbg_sel_reg;
typedef struct {
    uint32 val;
} msdc_dbg_out_reg;
typedef struct {
    //This register is not used in MT6589/MT6572
    uint32 clkdrvn:3;
    uint32 rsv0:1;
    uint32 clkdrvp:3;
    uint32 rsv1:1;
    uint32 clksr:1;
    uint32 rsv2:7;
    uint32 clkpd:1;
    uint32 clkpu:1;
    //Alternative definition for rsv2, clkpd, clkpu
    //uint32 clksr1:1;
    //uint32 clksr2:1;
    //uint32 clksr3:1;
    //uint32 rsv2:3;
    //uint32 clkpupd:1;
    //uint32 clkr1:1;
    //uint32 clkr0:1;
    uint32 clksmt:1;
    uint32 clkies:1;
    uint32 clktdsel:4;
    uint32 clkrdsel:8;
} msdc_pad_ctl0_reg;
typedef struct {
    //This register is not used in MT6589/MT6572
    uint32 cmddrvn:3;
    uint32 rsv0:1;
    uint32 cmddrvp:3;
    uint32 rsv1:1;
    uint32 cmdsr:1;
    uint32 rsv2:7;
    uint32 cmdpd:1;
    uint32 cmdpu:1;
    //Alternative definition for rsv2, clkpd, clkpu
    //uint32 cmdsr1:1;
    //uint32 cmdsr2:1;
    //uint32 cmdsr3:1;
    //uint32 rsv2:3;
    //uint32 cmdpupd:1;
    //uint32 cmdr1:1;
    //uint32 cmdr0:1;
    uint32 cmdsmt:1;
    uint32 cmdies:1;
    uint32 cmdtdsel:4;
    uint32 cmdrdsel:8;
} msdc_pad_ctl1_reg;
typedef struct {
    //This register is not used in MT6589/MT6572
    uint32 datdrvn:3;
    uint32 rsv0:1;
    uint32 datdrvp:3;
    uint32 rsv1:1;
    uint32 datsr:1;
    uint32 rsv2:7;
    uint32 datpd:1;
    uint32 datpu:1;
    //Alternative definition for rsv2, clkpd, clkpu
    //uint32 datsr1:1;
    //uint32 datsr2:1;
    //uint32 datsr3:1;
    //uint32 rsv2:3;
    //uint32 datpupd:1;
    //uint32 datr1:1;
    //uint32 datr0:1;
    uint32 datsmt:1;
    uint32 daties:1;
    uint32 dattdsel:4;
    uint32 datrdsel:8;
} msdc_pad_ctl2_reg;
typedef struct {
    uint32 datwrdly:5;
    uint32 pad1:3;
    uint32 datrddly:5;
    uint32 pad2:3;
    uint32 cmdrxdly:5;
    uint32 pad3:1;
    uint32 cmdrsprxdly:5;
    uint32 clktxdly:5;
} msdc_pad_tune_reg;
typedef struct {
    uint32 dat3:5;
    uint32 rsv3:3;
    uint32 dat2:5;
    uint32 rsv2:3;
    uint32 dat1:5;
    uint32 rsv1:3;
    uint32 dat0:5;
    uint32 rsv0:3;
} msdc_dat_rddly0;
typedef struct {
    uint32 dat7:5;
    uint32 rsv7:3;
    uint32 dat6:5;
    uint32 rsv6:3;
    uint32 dat5:5;
    uint32 rsv5:3;
    uint32 dat4:5;
    uint32 rsv4:3;
} msdc_dat_rddly1;
typedef struct {
    uint32 dbg3sel:8;
    uint32 dbg2sel:6;
    uint32 pad2:2;
    uint32 dbg1sel:6;
    uint32 dbgwtsel:2;
    uint32 dbg0sel:6;
    uint32 dbgwsel:1;
    uint32 pad3:1;
} msdc_hw_dbg_reg;
typedef struct {
    uint32 val;
} msdc_version_reg;
typedef struct {
    uint32 val;
} msdc_eco_ver_reg;

struct msdc_regs {
    msdc_cfg_reg    msdc_cfg;      /* base+0x00h */
    msdc_iocon_reg  msdc_iocon;    /* base+0x04h */
    msdc_ps_reg     msdc_ps;       /* base+0x08h */
    msdc_int_reg    msdc_int;      /* base+0x0ch */
    msdc_inten_reg  msdc_inten;    /* base+0x10h */
    msdc_fifocs_reg msdc_fifocs;   /* base+0x14h */
    msdc_txdat_reg  msdc_txdat;    /* base+0x18h */
    msdc_rxdat_reg  msdc_rxdat;    /* base+0x1ch */
    uint32      rsv1[4];
    sdc_cfg_reg     sdc_cfg;       /* base+0x30h */
    sdc_cmd_reg     sdc_cmd;       /* base+0x34h */
    sdc_arg_reg     sdc_arg;       /* base+0x38h */
    sdc_sts_reg     sdc_sts;       /* base+0x3ch */
    sdc_resp0_reg   sdc_resp0;     /* base+0x40h */
    sdc_resp1_reg   sdc_resp1;     /* base+0x44h */
    sdc_resp2_reg   sdc_resp2;     /* base+0x48h */
    sdc_resp3_reg   sdc_resp3;     /* base+0x4ch */
    sdc_blknum_reg  sdc_blknum;    /* base+0x50h */
    uint32      rsv2[1];
    sdc_csts_reg    sdc_csts;      /* base+0x58h */
    sdc_cstsen_reg  sdc_cstsen;    /* base+0x5ch */
    sdc_datcrcsts_reg   sdc_dcrcsta;   /* base+0x60h */
    uint32      rsv3[3];
    emmc_cfg0_reg   emmc_cfg0;     /* base+0x70h */
    emmc_cfg1_reg   emmc_cfg1;     /* base+0x74h */
    emmc_sts_reg    emmc_sts;      /* base+0x78h */
    emmc_iocon_reg  emmc_iocon;    /* base+0x7ch */
    msdc_acmd_resp_reg  acmd_resp;     /* base+0x80h */
    msdc_acmd19_trg_reg acmd19_trg;    /* base+0x84h */
    msdc_acmd19_sts_reg acmd19_sts;    /* base+0x88h */
    uint32      rsv4[1];
    msdc_dma_sa_reg dma_sa;        /* base+0x90h */
    msdc_dma_ca_reg dma_ca;        /* base+0x94h */
    msdc_dma_ctrl_reg   dma_ctrl;      /* base+0x98h */
    msdc_dma_cfg_reg    dma_cfg;       /* base+0x9ch */
    msdc_dbg_sel_reg    dbg_sel;       /* base+0xa0h */
    msdc_dbg_out_reg    dbg_out;       /* base+0xa4h */
    msdc_dma_length dma_length;    /* base+0xa8h */
    uint32      rsv5[1];
    uint32      patch0;        /* base+0xb0h */
    uint32      patch1;        /* base+0xb4h */
    uint32      rsv6[10];      /* base+0xc0h~0xdch*/
                       /* base+0xc0h~0xcch: ACMD53 tuning for DAT0~3*/
    msdc_pad_ctl0_reg   pad_ctl0;      /* base+0xe0h */
    msdc_pad_ctl1_reg   pad_ctl1;      /* base+0xe4h */
    msdc_pad_ctl2_reg   pad_ctl2;      /* base+0xe8h */
    msdc_pad_tune_reg   pad_tune;      /* base+0xech */
    msdc_dat_rddly0 dat_rddly0;    /* base+0xf0h */
    msdc_dat_rddly1 dat_rddly1;    /* base+0xf4h */
    msdc_hw_dbg_reg hw_dbg;        /* base+0xf8h */
    uint32      rsv7[1];
    msdc_version_reg    version;       /* base+0x100h */
    msdc_eco_ver_reg    eco_ver;       /* base+0x104h */
};

typedef struct {
    int    pio_bits;                            //PIO width: 32, 16, or 8bits
    int    stream_stop;
    int    autocmd;

    #if defined(MSDC_USE_DMA_MODE)
    struct dma_config  cfg;
    struct scatterlist sg[MAX_SG_POOL_SZ];
    int    alloc_gpd;
    int    alloc_bd;
    #endif

    int    dsmpl;
    int    rsmpl;

    #if defined(MSDC_USE_DMA_MODE)
    gpd_t *active_head;
    gpd_t *active_tail;
    gpd_t *gpd_pool;
    bd_t  *bd_pool;
    #endif
} msdc_priv_t;

#define DMA_FLAG_NONE       (0x00000000)
#define DMA_FLAG_EN_CHKSUM  (0x00000001)
#define DMA_FLAG_PAD_BLOCK  (0x00000002)
#define DMA_FLAG_PAD_DWORD  (0x00000004)

#if MSDC_USE_REG_OPS_DUMP
static void reg32_write(volatile uint32 *addr, uint32 data)
{
    *addr = (uint32)data;
    printf("[WR32] %x = %x\n", addr, data);
}

static uint32 reg32_read(volatile uint32 *addr)
{
    uint32 data = *(volatile uint32*)(addr);
    printf("[RD32] %x = %x\n", addr, data);
    return data;
}

static void reg16_write(volatile uint32 *addr, uint16 data)
{
    *(volatile uint16*)(addr) = data;
    printf("[WR16] %x = %x\n", addr, data);
}

static uint16 reg16_read(volatile uint32 *addr)
{
    uint16 data = *(volatile uint16*)addr;
    printf("[RD16] %x = %x\n", addr, data);
    return data;
}

static void reg8_write(volatile uint32 *addr, uint8 data)
{
    *(volatile uint8*)(addr) = data;
    printf("[WR8] %x = %x\n", addr, data);
}

static uint8 reg8_read(volatile uint32 *addr)
{
    uint8 data = *(volatile uint8*)addr;
    printf("[RD8] %x = %x\n", addr, data);
    return data;
}

#define MSDC_WRITE32(addr,data)        reg32_write((volatile uint32*)addr, data)
#define MSDC_READ32(addr)        reg32_read((volatile uint32*)addr)
#define MSDC_WRITE16(addr,data)        reg16_write((volatile uint32*)addr, data)
#define MSDC_READ16(addr)        reg16_read((volatile uint32*)addr)
#define MSDC_WRITE8(addr, data)     reg8_write((volatile uint32*)addr, data)
#define MSDC_READ8(addr)        reg8_read((volatile uint32*)addr)
#define MSDC_SET_BIT32(addr,mask)    \
    do { \
    (*(volatile uint32*)(addr) |= (mask)); \
    printf("[SET32] %x |= %x\n", addr, mask); \
    }while(0)
#define MSDC_CLR_BIT32(addr,mask)    \
    do { \
    (*(volatile uint32*)(addr) &= ~(mask)); \
    printf("[CLR32] %x &= ~%x\n", addr, mask); \
    }while(0)
#define MSDC_SET_BIT16(addr,mask)    \
    do { \
    (*(volatile uint16*)(addr) |= (mask)); \
    printf("[SET16] %x |= %x\n", addr, mask); \
    }while(0)
#define MSDC_CLR_BIT16(addr,mask)    \
    do { \
    (*(volatile uint16*)(addr) &= ~(mask)); \
    printf("[CLR16] %x &= ~%x\n", addr, mask); \
    }while(0)
#else
#define MSDC_WRITE32(addr,data)     (*(volatile uint32*)(addr) = (uint32)(data))
#define MSDC_READ32(addr)           (*(volatile uint32*)(addr))
#define MSDC_WRITE16(addr,data)     (*(volatile uint16*)(addr) = (uint16)(data))
#define MSDC_READ16(addr)           (*(volatile uint16*)(addr))
#define MSDC_WRITE8(addr, data)     (*(volatile uint8*)(addr)  = (uint8)(data))
#define MSDC_READ8(addr)            (*(volatile uint8*)(addr))
#define MSDC_SET_BIT32(addr,mask)   (*(volatile uint32*)(addr) |= (mask))
#define MSDC_CLR_BIT32(addr,mask)   (*(volatile uint32*)(addr) &= ~(mask))
#define MSDC_SET_BIT16(addr,mask)   (*(volatile uint16*)(addr) |= (mask))
#define MSDC_CLR_BIT16(addr,mask)   (*(volatile uint16*)(addr) &= ~(mask))
#endif

#define MSDC_SET_FIELD(reg,field,val) \
    do {    \
    volatile uint32 tv = MSDC_READ32(reg); \
    tv &= ~(field); \
    tv |= ((val) << (uffs(field) - 1)); \
    MSDC_WRITE32(reg,tv); \
    } while(0)

#define MSDC_GET_FIELD(reg,field,val) \
    do {    \
    volatile uint32 tv = MSDC_READ32(reg); \
    val = ((tv & (field)) >> (uffs(field) - 1)); \
    } while(0)

#define MSDC_SET_FIELD_DISCRETE(reg,field,val) \
do {    \
    unsigned int tv = (unsigned int)(*(volatile u32*)(reg)); \
    tv = (val == 1) ? (tv|(field)):(tv & ~(field));\
    (*(volatile u32*)(reg) = (u32)(tv)); \
} while(0)

#define MSDC_GET_FIELD_DISCRETE(reg,field,val) \
do {    \
    unsigned int tv = (unsigned int)(*(volatile u32*)(reg)); \
    val = tv & (field) ; \
    val = (val != 0) ? 1 :0;\
} while(0)

#define MSDC_RETRY(expr,retry,cnt) \
    do { \
    uint32 t = cnt; \
    uint32 r = retry; \
    uint32 c = cnt; \
    while (r) { \
        if (!(expr)) break; \
        if (c-- == 0) { \
        r--; udelay(200); c = t; \
        } \
    } \
    BUG_ON(r == 0); \
    } while(0)

#define MSDC_RESET() \
    do { \
    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_RST); \
    MSDC_RETRY(MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST, 5, 1000); \
    } while(0)

#define MSDC_CLR_INT() \
    do { \
        volatile uint32 val = MSDC_READ32(MSDC_INT); \
        MSDC_WRITE32(MSDC_INT, val); \
        if (MSDC_READ32(MSDC_INT)) { \
            MSG(ERR, "[ASSERT] MSDC_INT is NOT clear\n"); \
        } \
    } while(0)

#define MSDC_CLR_FIFO() \
    do { \
        MSDC_SET_BIT32(MSDC_FIFOCS, MSDC_FIFOCS_CLR); \
        MSDC_RETRY(MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR, 5, 1000); \
    } while(0)

#define MSDC_FIFO_WRITE32(val)  MSDC_WRITE32(MSDC_TXDATA, val)
#define MSDC_FIFO_READ32()  MSDC_READ32(MSDC_RXDATA)
#define MSDC_FIFO_WRITE16(val)  MSDC_WRITE16(MSDC_TXDATA, val)
#define MSDC_FIFO_READ16()  MSDC_READ16(MSDC_RXDATA)
#define MSDC_FIFO_WRITE8(val)   MSDC_WRITE8(MSDC_TXDATA, val)
#define MSDC_FIFO_READ8()   MSDC_READ8(MSDC_RXDATA)

#define MSDC_FIFO_WRITE(val)    MSDC_FIFO_WRITE32(val)
#define MSDC_FIFO_READ()    MSDC_FIFO_READ32()

#define MSDC_TXFIFOCNT() \
    ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#define MSDC_RXFIFOCNT() \
    ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)

#define MSDC_CARD_DETECTION_ON()  MSDC_SET_BIT32(MSDC_PS, MSDC_PS_CDEN)
#define MSDC_CARD_DETECTION_OFF() MSDC_CLR_BIT32(MSDC_PS, MSDC_PS_CDEN)

#define MSDC_DMA_ON()   MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_PIO)
#define MSDC_DMA_OFF()  MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO)

#define SDC_IS_BUSY()        (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)
#define SDC_IS_CMD_BUSY()    (MSDC_READ32(SDC_STS) & SDC_STS_CMDBUSY)

#define SDC_SEND_CMD(cmd,arg) \
    do { \
    MSDC_WRITE32(SDC_ARG, (arg)); \
    MSDC_WRITE32(SDC_CMD, (cmd)); \
    } while(0)

#define MSDC_INIT_GPD_EX(gpd,extlen,cmd,arg,blknum) \
    do { \
    ((gpd_t*)gpd)->extlen = extlen; \
    ((gpd_t*)gpd)->cmd    = cmd; \
    ((gpd_t*)gpd)->arg    = arg; \
    ((gpd_t*)gpd)->blknum = blknum; \
    }while(0)

#define MSDC_INIT_BD(bd, blkpad, dwpad, dptr, dlen) \
    do { \
    BUG_ON(dlen > 0xFFFFUL); \
    ((bd_t*)bd)->blkpad = blkpad; \
    ((bd_t*)bd)->dwpad  = dwpad; \
    ((bd_t*)bd)->ptr    = (void*)dptr; \
    ((bd_t*)bd)->buflen = dlen; \
    }while(0)

#ifdef MMC_PROFILING
static inline void msdc_timer_init(void)
{
    /* clear. CLR[1]=1, EN[0]=0 */
    MSDC_WRITE32(GPT_BASE + 0x30, 0x0);
    MSDC_WRITE32(GPT_BASE + 0x30, 0x2);

    MSDC_WRITE32(GPT_BASE + 0x38, 0);
    MSDC_WRITE32(GPT_BASE + 0x3C, 32768);

    /* 32678HZ RTC free run */
    MSDC_WRITE32(GPT_BASE + 0x34, 0x30);
    MSDC_WRITE32(GPT_BASE + 0x30, 0x32);
}
static inline void msdc_timer_start(void)
{
    *(volatile unsigned int*)(GPT_BASE + 0x30) |= (1 << 0);
}
static inline void msdc_timer_stop(void)
{
    *(volatile unsigned int*)(GPT_BASE + 0x30) &= ~(1 << 0);
}
static inline void msdc_timer_stop_clear(void)
{
    *(volatile unsigned int*)(GPT_BASE + 0x30) &= ~(1 << 0); /* stop  */
    *(volatile unsigned int*)(GPT_BASE + 0x30) |= (1 << 1);  /* clear */
}
static inline unsigned int msdc_timer_get_count(void)
{
    return MSDC_READ32(GPT_BASE + 0x38);
}
#else
#define msdc_timer_init()   do{}while(0)
#define msdc_timer_start()  do{}while(0)
#define msdc_timer_stop()   do{}while(0)
#define msdc_timer_stop_clear() do{}while(0)
#define msdc_timer_get_count()  0
#endif

extern int msdc_reg_test(int id);
extern void msdc_intr_unmask(struct mmc_host *host, u32 bits);
extern void msdc_intr_mask(struct mmc_host *host, u32 bits);
extern int msdc_init(int id, struct mmc_host *host, int clksrc, int mode);
extern int msdc_pio_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks);
extern int msdc_pio_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks);
extern int msdc_dma_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks);
extern int msdc_dma_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks);
#if defined(MSDC_USE_MMC_STREAM)
extern int msdc_stream_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks);
extern int msdc_stream_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks);
#endif
extern int msdc_tune_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks);
extern int msdc_tune_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks);
extern void msdc_tune_update_cmdrsp(struct mmc_host *host, u32 count);
extern int msdc_tune_cmdrsp(struct mmc_host *host, struct mmc_command *cmd);
extern void msdc_reset_tune_counter(struct mmc_host *host);
extern void msdc_abort(struct mmc_host *host);
extern void msdc_abort_handler(struct mmc_host *host, int abort_card);
extern int msdc_tune_read(struct mmc_host *host);
extern void msdc_intr_sdio(struct mmc_host *host, int enable);
extern void msdc_intr_sdio_gap(struct mmc_host * host, int enable);
extern void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz);
extern int msdc_cmd_stop(struct mmc_host *host, struct mmc_command *cmd);
extern int msdc_wait_rsp(struct mmc_host *host, struct mmc_command *cmd);
extern int msdc_send_cmd(struct mmc_host *host, struct mmc_command *cmd);
extern int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd);
extern int msdc_cmd_io_abort(struct mmc_host *host);
extern void msdc_set_blknum(struct mmc_host *host, u32 blknum);
extern void msdc_set_blklen(struct mmc_host *host, u32 blklen);
extern void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks);
extern void msdc_set_autocmd(struct mmc_host *host, int cmd, int on);
extern int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size);
extern void msdc_config_bus(struct mmc_host *, u32);
extern int msdc_deinit(struct mmc_host *);

#endif /* end of _MSDC_H_ */

