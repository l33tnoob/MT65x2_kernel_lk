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
#include <stdio.h>
#include <string.h>

#include <target/cust_msdc.h>
#include <platform/msdc.h>
#include <platform/msdc_utils.h>
#include <platform/mmc_core.h>
#include <kernel/event.h>
#include <platform/mt_irq.h>
#include <platform/mt_gpt.h>

#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100) /* 100ms */

#define PERI_MSDC_SRCSEL   (0xc100000c)

/* Tuning Parameter */
#define DEFAULT_DEBOUNCE   (8)	/* 8 cycles */
#define DEFAULT_DTOC       (40)	/* data timeout counter. 65536x40 sclk. */
#define DEFAULT_WDOD       (0)	/* write data output delay. no delay. */
#define DEFAULT_BSYDLY     (8)	/* card busy delay. 8 extend sclk */

#define MAX_DMA_CNT        (32768)
#define MAX_GPD_POOL_SZ    (2)  /* include null gpd */
#define MAX_BD_POOL_SZ     (16)
#define MAX_SG_POOL_SZ     (MAX_BD_POOL_SZ)
#define MAX_SG_BUF_SZ      (MAX_DMA_CNT)
#define MAX_BD_PER_GPD     (MAX_BD_POOL_SZ/(MAX_GPD_POOL_SZ-1)) /* except null gpd */

#define MAX_DMA_TRAN_SIZE  (MAX_SG_POOL_SZ*MAX_SG_BUF_SZ)

#if MAX_SG_BUF_SZ > MAX_DMA_CNT
#error "incorrect max sg buffer size"
#endif

 
typedef struct {
    int    autocmd;
    struct dma_config  cfg;
    struct scatterlist sg[MAX_SG_POOL_SZ];
    int    alloc_gpd;
    int    alloc_bd;
    int    dsmpl;
    int    rsmpl;
    gpd_t *active_head;
    gpd_t *active_tail;
    gpd_t *gpd_pool;
    bd_t  *bd_pool;
} msdc_priv_t;

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

static msdc_priv_t msdc_priv[MSDC_MAX_NUM];
static gpd_t msdc_gpd_pool[MSDC_MAX_NUM][MAX_GPD_POOL_SZ] __attribute__ ((aligned(64))); // FOR mt6582 EMI not cross 2K boundary
static bd_t  msdc_bd_pool[MSDC_MAX_NUM][MAX_BD_POOL_SZ] __attribute__ ((aligned(64)));
static event_t msdc_int_event;
static u32 g_int_status = 0;
void msdc_dump_card_status(u32 card_status)
{
#if MSDC_DEBUG
    static char *state[] = {
        "Idle",			/* 0 */
        "Ready",		/* 1 */
        "Ident",		/* 2 */
        "Stby",			/* 3 */
        "Tran",			/* 4 */
        "Data",			/* 5 */
        "Rcv",			/* 6 */
        "Prg",			/* 7 */
        "Dis",			/* 8 */
        "Reserved",		/* 9 */
        "Reserved",		/* 10 */
        "Reserved",		/* 11 */
        "Reserved",		/* 12 */
        "Reserved",		/* 13 */
        "Reserved",		/* 14 */
        "I/O mode",		/* 15 */
    };
    if (card_status & STA_OUT_OF_RANGE)
        printf("\t[CARD_STATUS] Out of Range\n");
    if (card_status & STA_ADDRESS_ERROR)
        printf("\t[CARD_STATUS] Address Error\n");
    if (card_status & STA_BLOCK_LEN_ERROR)
        printf("\t[CARD_STATUS] Block Len Error\n");
    if (card_status & STA_ERASE_SEQ_ERROR)
        printf("\t[CARD_STATUS] Erase Seq Error\n");
    if (card_status & STA_ERASE_PARAM)
        printf("\t[CARD_STATUS] Erase Param\n");
    if (card_status & STA_WP_VIOLATION)
        printf("\t[CARD_STATUS] WP Violation\n");
    if (card_status & STA_CARD_IS_LOCKED)
        printf("\t[CARD_STATUS] Card is Locked\n");
    if (card_status & STA_LOCK_UNLOCK_FAILED)
        printf("\t[CARD_STATUS] Lock/Unlock Failed\n");
    if (card_status & STA_COM_CRC_ERROR)
        printf("\t[CARD_STATUS] Command CRC Error\n");
    if (card_status & STA_ILLEGAL_COMMAND)
        printf("\t[CARD_STATUS] Illegal Command\n");
    if (card_status & STA_CARD_ECC_FAILED)
        printf("\t[CARD_STATUS] Card ECC Failed\n");
    if (card_status & STA_CC_ERROR)
        printf("\t[CARD_STATUS] CC Error\n");
    if (card_status & STA_ERROR)
        printf("\t[CARD_STATUS] Error\n");
    if (card_status & STA_UNDERRUN)
        printf("\t[CARD_STATUS] Underrun\n");
    if (card_status & STA_OVERRUN)
        printf("\t[CARD_STATUS] Overrun\n");
    if (card_status & STA_CID_CSD_OVERWRITE)
        printf("\t[CARD_STATUS] CID/CSD Overwrite\n");
    if (card_status & STA_WP_ERASE_SKIP)
        printf("\t[CARD_STATUS] WP Eraser Skip\n");
    if (card_status & STA_CARD_ECC_DISABLED)
        printf("\t[CARD_STATUS] Card ECC Disabled\n");
    if (card_status & STA_ERASE_RESET)
        printf("\t[CARD_STATUS] Erase Reset\n");
    if (card_status & STA_READY_FOR_DATA)
        printf("\t[CARD_STATUS] Ready for Data\n");
    if (card_status & STA_SWITCH_ERROR)
        printf("\t[CARD_STATUS] Switch error\n");
    if (card_status & STA_URGENT_BKOPS)
        printf("\t[CARD_STATUS] Urgent background operations\n");
    if (card_status & STA_APP_CMD)
        printf("\t[CARD_STATUS] App Command\n");

    printf("\t[CARD_STATUS] '%s' State\n", 
    state[STA_CURRENT_STATE(card_status)]);
#endif    
}

void msdc_dump_ocr_reg(u32 resp)
{
#if MSDC_DEBUG
    if (resp & (1 << 7))
        printf("\t[OCR] Low Voltage Range\n");
    if (resp & (1 << 15))
        printf("\t[OCR] 2.7-2.8 volt\n");
    if (resp & (1 << 16))
        printf("\t[OCR] 2.8-2.9 volt\n");
    if (resp & (1 << 17))
        printf("\t[OCR] 2.9-3.0 volt\n");
    if (resp & (1 << 18))
        printf("\t[OCR] 3.0-3.1 volt\n");
    if (resp & (1 << 19))
        printf("\t[OCR] 3.1-3.2 volt\n");
    if (resp & (1 << 20))
        printf("\t[OCR] 3.2-3.3 volt\n");
    if (resp & (1 << 21))
        printf("\t[OCR] 3.3-3.4 volt\n");
    if (resp & (1 << 22))
        printf("\t[OCR] 3.4-3.5 volt\n");
    if (resp & (1 << 23))
        printf("\t[OCR] 3.5-3.6 volt\n");
    if (resp & (1 << 24))
        printf("\t[OCR] Switching to 1.8V Accepted (S18A)\n");
    if (resp & (1 << 30))
        printf("\t[OCR] Card Capacity Status (CCS)\n");
    if (resp & (1UL << 31))
        printf("\t[OCR] Card Power Up Status (Idle)\n");
    else
        printf("\t[OCR] Card Power Up Status (Busy)\n");
#endif
}

void msdc_dump_io_resp(u32 resp)
{
#if MSDC_DEBUG
    u32 flags = (resp >> 8) & 0xFF;
    char *state[] = {"DIS", "CMD", "TRN", "RFU"};
    
    if (flags & (1 << 7))
        printf("\t[IO] COM_CRC_ERR\n");
    if (flags & (1 << 6))
        printf("\t[IO] Illgal command\n");   
    if (flags & (1 << 3))
        printf("\t[IO] Error\n");
    if (flags & (1 << 2))
        printf("\t[IO] RFU\n");
    if (flags & (1 << 1))
        printf("\t[IO] Function number error\n");
    if (flags & (1 << 0))
        printf("\t[IO] Out of range\n");

    printf("[IO] State: %s, Data:0x%x\n", state[(resp >> 12) & 0x3], resp & 0xFF);
#endif
}

void msdc_dump_rca_resp(u32 resp)
{
#if MSDC_DEBUG
    u32 card_status = (((resp >> 15) & 0x1) << 23) |
                      (((resp >> 14) & 0x1) << 22) |
                      (((resp >> 13) & 0x1) << 19) |
                        (resp & 0x1fff);

    printf("\t[RCA] 0x%x\n", resp >> 16);
    msdc_dump_card_status(card_status);
#endif
}

#if MSDC_DEBUG
static void msdc_dump_dma_desc(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    int i;
    u32 *ptr;

    if (MSG_EVT_MASK & MSG_EVT_DMA) {
        for (i = 0; i < priv->alloc_gpd; i++) {
            ptr = (u32*)&priv->gpd_pool[i];
            printf("[SD%d] GD[%d]: %xh %xh %xh %xh %xh %xh %xh\n", 
                host->id, i, *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), 
                *(ptr+5), *(ptr+6));
        }

        for (i = 0; i < priv->alloc_bd; i++) {
            ptr = (u32*)&priv->bd_pool[i];
            printf("[SD%d] BD[%d]: %xh %xh %xh %xh\n", 
                host->id, i, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
        }
    }
}
#endif

/* add function for MSDC_PAD_CTL handle */
#ifndef FPGA_PLATFORM

/* pull up means that host driver the line to HIGH
 * pull down means that host driver the line to LOW */
static void msdc_pin_pud(struct mmc_host *host, int mode)
{
    /* driver CLK/DAT pin */
	switch(host->id){
		case 0:
            switch (mode) {
                case MSDC0_PULL_NONE:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 0);
                    //MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 0);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 0);
                    //MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 0);
                    break;
                case MSDC0_PU_10K:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 0);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 0);

                    /* clock pull down with 50k during card init */
                    MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_PUPD_MASK, 1);
                    break;
                case MSDC0_PU_50K:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 0);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 0);
                    break;
                case MSDC0_PU_8K:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 0);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 0);
                    break;
                case MSDC0_PD_10K:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 1);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 1);
                    break;
                case MSDC0_PD_50K:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 1);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 0);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 1);
                    break;
                case MSDC0_PD_8K:
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_PUPD_MASK, 1);
                    
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R0_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_R1_MASK, 1);
                    MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_PUPD_MASK, 1);
                    break;
                default:
                    /* error mode! */
                    printf("\t[msdc_pin_pud] host0 mode error(%d)!\n", mode);
                    break;
            }
			break;
		case 1:
            if (MSDC1_PU_50K == mode){
                MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 0);
            } else if (MSDC1_PD_50K == mode){
                MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PD_MASK, 1);
                
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 1);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 1);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 1);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 1);
            } else if (MSDC1_PULL_NONE == mode){
                MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 0);
            } else {
                /* error mode! */
                printf("\t[msdc_pin_pud] host1 mode error(%d)!\n", mode);
            }
			break;
		case 2:
            if (MSDC2_PU_50K == mode){
                MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PD_MASK, 0);
             
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 0);

                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 1);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 0);
            } else if (MSDC2_PD_50K == mode){
                MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PD_MASK, 1);
                
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 1);

                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 1);
                
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 1);
                
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 1);
            } else if (MSDC2_PULL_NONE == mode){
                MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PD_MASK, 0);
         
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 0);
                
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 0);
                MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 0);
            } else {
                /* error mode! */
                printf("\t[msdc_pin_pud] host2 mode error(%d)!\n", mode);
            }
			break;
		default:
			break;
		}
}

static void msdc_pin_pnul(struct mmc_host *host, int mode)
{
	switch(host->id){
		case 1:
            MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 0);
            break;
        case 2:
            MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT0_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT1_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT2_PD_MASK, 0);

            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PU_MASK, 0);
            MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_DAT3_PD_MASK, 0);
            break;
        default:
            break;
    }
}

static void msdc_set_smt(struct mmc_host *host,int set_smt)
{
	switch(host->id){
		case 0:
			MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_SMT_MASK, set_smt);
			MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_SMT_MASK, set_smt);
			MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_SMT_MASK, set_smt); 
			break;
		case 1:
			MSDC_SET_FIELD(MSDC1_GPIO_CLK_BASE, GPIO_SMT_MASK, set_smt);
			MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_SMT_MASK, set_smt);
			MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_SMT_MASK, set_smt); 
			break;
		case 2:
			MSDC_SET_FIELD(MSDC2_GPIO_CLK_BASE, GPIO_SMT_MASK, set_smt);
			MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_SMT_MASK, set_smt);
			MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_SMT_MASK, set_smt); 
			break;
		default:
			break;
	}
}

/* host can modify from 0-7 */
static void msdc_set_driving(struct mmc_host *host,u8 clkdrv, u8 cmddrv, u8 datdrv)
{
	switch(host->id){
		case 0:
			MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_MSDC0_DRVN, clkdrv);
			MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_MSDC0_DRVN, cmddrv);
			MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_MSDC0_DRVN, datdrv);
			break;
		case 1:
			MSDC_SET_FIELD(MSDC1_GPIO_CLK_BASE, GPIO_MSDC1_MSDC2_DRVN, clkdrv);
			MSDC_SET_FIELD(MSDC1_GPIO_CMD_BASE, GPIO_MSDC1_MSDC2_DRVN, cmddrv);
			MSDC_SET_FIELD(MSDC1_GPIO_DAT_BASE, GPIO_MSDC1_MSDC2_DRVN, datdrv);
			break;
		case 2:
			MSDC_SET_FIELD(MSDC2_GPIO_CLK_BASE, GPIO_MSDC1_MSDC2_DRVN, clkdrv);
			MSDC_SET_FIELD(MSDC2_GPIO_CMD_BASE, GPIO_MSDC1_MSDC2_DRVN, cmddrv);
			MSDC_SET_FIELD(MSDC2_GPIO_DAT_BASE, GPIO_MSDC1_MSDC2_DRVN, datdrv);
			break;
		default:
			break;
		}
}

#else

void msdc_set_driving(struct mmc_host *host, u8 clkdrv, u8 cmddrv, u8 datdrv)
{
    u32 base = host->base;
    MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVN, clkdrv);
    MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVP, clkdrv);
    MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVN, cmddrv);
    MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVP, cmddrv);
    MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVN, datdrv);
    MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVP, datdrv);
}
#endif /* end of FPGA_PLATFORM */

#if MSDC_USE_DMA_MODE
void msdc_flush_membuf(void *buf, u32 len)
{
    arch_clean_invalidate_cache_range((addr_t)buf, len);
    //apmcu_clean_invalidate_dcache();
    //apmcu_clean_invalidate_dcache_ARM11();
}

u8 msdc_cal_checksum(u8 *buf, u32 len)
{
    u32 i, sum = 0;
    for (i = 0; i < len; i++) {
        sum += buf[i];
    }
    return 0xFF - (u8)sum;
}

/* allocate gpd link-list from gpd_pool */
gpd_t *msdc_alloc_gpd(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    gpd_t *gpd, *ptr, *prev;

    if (priv->alloc_gpd + num + 1 > MAX_GPD_POOL_SZ || num == 0)
        return NULL;

    gpd = priv->gpd_pool + priv->alloc_gpd;
    priv->alloc_gpd += (num + 1); /* include null gpd */

    memset(gpd, 0, sizeof(gpd_t) * (num + 1));

    ptr = gpd + num - 1;
    ptr->next = (void*)(gpd + num); /* pointer to null gpd */
    
    /* create link-list */
    if (ptr != gpd) {
        do {
            prev = ptr - 1;
            prev->next = ptr;
            ptr = prev;
        } while (ptr != gpd);
    }

    return gpd;
}

/* allocate bd link-list from bd_pool */
bd_t *msdc_alloc_bd(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    bd_t *bd, *ptr, *prev;

    if (priv->alloc_bd + num > MAX_BD_POOL_SZ || num == 0)
        return NULL;
    
    bd = priv->bd_pool + priv->alloc_bd;
    priv->alloc_bd += num;

    memset(bd, 0, sizeof(bd_t) * num);

    ptr = bd + num - 1;
    ptr->eol  = 1;
    ptr->next = 0;

    /* create link-list */
    if (ptr != bd) {
        do {
            prev = ptr - 1;
            prev->next = ptr;
            prev->eol  = 0;
            ptr = prev;
        } while (ptr != bd);
    }

    return bd;
}

bd_t *msdc_alloc_bd_simp(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    bd_t *bd, *ptr, *prev;

    if (priv->alloc_bd + num > MAX_BD_POOL_SZ || num == 0)
        return NULL;
    
    bd = priv->bd_pool + priv->alloc_bd;
    priv->alloc_bd += num;

    //memset(bd, 0, sizeof(bd_t) * num);

    ptr = bd + num - 1;
    ptr->eol  = 1;
    ptr->next = 0;

    /* create link-list */
    if (ptr != bd) {
        do {
            prev = ptr - 1;
            prev->next = ptr;
            prev->eol  = 0;
            ptr = prev;
        } while (ptr != bd);
    }

    return bd;
}

/* queue bd link-list to one gpd */
void msdc_queue_bd(struct mmc_host *host, gpd_t *gpd, bd_t *bd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    BUG_ON(gpd->ptr);

    gpd->hwo = 1;
    gpd->bdp = 1;
    gpd->ptr = (void*)bd;

    if ((priv->cfg.flags & DMA_FLAG_EN_CHKSUM) == 0) 
        return;

    /* calculate and fill bd checksum */
    while (bd) {
        bd->chksum = msdc_cal_checksum((u8*)bd, 16);
        bd = bd->next;
    }
}

/* queue data buf to one gpd */
void msdc_queue_buf(struct mmc_host *host, gpd_t *gpd, u8 *buf)
{
    BUG_ON(gpd->ptr);

    gpd->hwo = 1;
    gpd->bdp = 0;
    gpd->ptr = (void*)buf;
}

/* add gpd link-list to active list */
void msdc_add_gpd(struct mmc_host *host, gpd_t *gpd, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    if (num > 0) {
        if (!priv->active_head) {
            priv->active_head = gpd;
        } else {
            priv->active_tail->next = gpd;
        }
        priv->active_tail = gpd + num - 1;

        if ((priv->cfg.flags & DMA_FLAG_EN_CHKSUM) == 0)
            return;

        /* calculate and fill gpd checksum */
        while (gpd) {
            gpd->chksum = msdc_cal_checksum((u8 *)gpd, 16);
            gpd = gpd->next;
        }
    }
}

void msdc_reset_gpd(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    priv->alloc_bd  = 0;
    priv->alloc_gpd = 0;
    priv->active_head = NULL;
    priv->active_tail = NULL;
}
#endif
#ifdef FPGA_PLATFORM
#define msdc_set_host_level_pwr(level) do{}while(0)
#define msdc_set_host_pwr(on) do{}while(0)
#define PWR_GPIO            (0x10001E84)
#define PWR_GPIO_EO         (0x10001E88)
#define PWR_MASK_VOL_33     (0x1 << 10)
#define PWR_MASK_EN         (0x1 << 8)

void msdc_set_card_pwr(int on)
{
    /* set which bits are available */
    MSDC_WRITE32(PWR_GPIO_EO, (PWR_MASK_VOL_33 | PWR_MASK_EN));

    if (on) {
        //printf("[%s]: start set gpio, on\n", __func__);
        MSDC_WRITE32(PWR_GPIO, (PWR_MASK_VOL_33 | PWR_MASK_EN));
    } else {
        //printf("[%s]: start set gpio, off\n", __func__);
        MSDC_WRITE32(PWR_GPIO, 0x0);
    }
    //printf("[%s]: before delay(3)\n", __func__);
    mdelay(3);
    printf("[%s]: finish, on=%d\n", __func__, on);
}

#else
#if 0 /* don't power on/off device and use power-on default volt */
void msdc_set_host_level_pwr(int level)
{
    unsigned int ret;
    
    if (level) {
        ret = pmic_config_interface(0xA7,0x2,0x7,4); /* VMC=1.8V */
    } else {
        ret = pmic_config_interface(0xA7,0x7,0x7,4); /* VMC=3.3V */
    }
    if (ret != 0) {
        MSG(ERR, "PMIC: Set MSDC Vol Level Fail\n");
    }
    //mdelay(100);
}
void msdc_set_card_pwr(int on)
{
    unsigned int ret;

    ret = pmic_config_interface(0xAB,0x7,0x7,4); /* VMCH=3.3V */

    if (ret == 0) {
        if(on) {            
            ret = pmic_config_interface(0xAB,0x1,0x1,0); /* VMCH_EN=1 */
        } else {
            ret = pmic_config_interface(0xAB,0x0,0x1,0); /* VMCH_EN=0 */
        }
    }
    if (ret != 0) {
        MSG(ERR, "PMIC: Set MSDC Card Power Fail\n");
    }
    mdelay(50);
}
void msdc_set_host_pwr(int on)
{
    unsigned int ret;

    ret = pmic_config_interface(0xA7,0x7,0x7,4); /* VMC=3.3V */

    if (ret == 0) {
        if(on) {
            ret = pmic_config_interface(0xA7,0x1,0x1,0); /* VMC_EN=1 */
        } else {
            ret = pmic_config_interface(0xA7,0x0,0x1,0); /* VMC_EN=0 */
        }
    }
    
    if (ret != 0) {
        MSG(ERR, "PMIC: Set MSDC Host Power Fail\n");
    }
    
    mdelay(50);
}
#else
#define msdc_set_host_level_pwr(level)  do{}while(0)
#define msdc_set_host_pwr(on)           do{}while(0)
#define msdc_set_card_pwr(on)           do{}while(0)
#endif
#endif

void msdc_set_smpl(struct mmc_host *host, u8 dsmpl, u8 rsmpl)
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

static u32 msdc_cal_timeout(struct mmc_host *host, u32 ns, u32 clks, u32 clkunit)
{
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout / clkunit;
    return timeout;
}

void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks)
{
    u32 base = host->base;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout >> 20; /* in 1048576 sclk cycle unit (MT6589 MSDC IP)*/
    timeout = timeout > 1 ? timeout - 1 : 0;
    timeout = timeout > 255 ? 255 : timeout;

    MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, timeout);

    MSG(OPS, "[SD%d] Set read data timeout: %dns %dclks -> %d x 65536 cycles\n",
        host->id, ns, clks, timeout + 1);
}

void msdc_set_blklen(struct mmc_host *host, u32 blklen)
{
    u32 base = host->base;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    host->blklen     = blklen;
    priv->cfg.blklen = blklen;
    MSDC_CLR_FIFO();
}

void msdc_set_blknum(struct mmc_host *host, u32 blknum)
{
    u32 base = host->base;

    MSDC_WRITE32(SDC_BLK_NUM, blknum);
}

void msdc_set_dma(struct mmc_host *host, u8 burstsz, u32 flags)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;

    cfg->burstsz = burstsz;
    cfg->flags   = flags;
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

void msdc_clr_fifo(struct mmc_host *host)
{
    u32 base = host->base;
    MSDC_CLR_FIFO();
}

void msdc_abort(struct mmc_host *host)
{
    u32 base = host->base;

    /* reset controller */
    MSDC_RESET();

    /* clear fifo */
    MSDC_CLR_FIFO();

    /* make sure txfifo and rxfifo are empty */
    if (MSDC_TXFIFOCNT() != 0 || MSDC_RXFIFOCNT() != 0) {
        MSG(INF, "[SD%d] Abort: TXFIFO(%d), RXFIFO(%d) != 0\n",
            host->id, MSDC_TXFIFOCNT(), MSDC_RXFIFOCNT());
    }

    /* clear all interrupts */
    MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
}
int msdc_send_cmd(struct mmc_host *host, struct mmc_command *cmd);
int msdc_wait_rsp(struct mmc_host *host, struct mmc_command *cmd);
int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd);

static int msdc_get_card_status(struct mmc_host *host, u32 *status)
{
	int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_STATUS;
    cmd.arg     = host->card->rca << 16;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = msdc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE) {
        *status = cmd.resp[0];
    }
    return err;
}
int msdc_abort_handler(struct mmc_host *host, int abort_card)
{
    struct mmc_command stop;
	u32 status = 0;    
	u32 state = 0; 
    
	while (state != 4) { // until status to "tran"
			msdc_abort(host);
			if (msdc_get_card_status(host, &status)) {
				printf("Get card status failed\n");	
				return 1;
			}	
			state = STA_CURRENT_STATE(status);
			printf("check card state<%d>\n", state);
			if (state == 5 || state == 6) {
				printf("state<%d> need cmd12 to stop\n", state); 
				if (abort_card) {
        			stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
       			 	stop.rsptyp  = RESP_R1B;
        			stop.arg     = 0;
        			stop.retries = CMD_RETRIES;
        			stop.timeout = CMD_TIMEOUT;
        			msdc_send_cmd(host, &stop);
        			msdc_wait_rsp(host, &stop); // don't tuning
			} else if (state == 7) {  // busy in programing 		
				printf("state<%d> card is busy\n", state);	
				mdelay(100);
			} else if (state != 4) {
				printf("state<%d> ??? \n", state);
				return 1;		
			}
		}
	}
	msdc_abort(host);
	return 0;
}

u32 msdc_intr_wait(struct mmc_host *host, u32 intrs)
{
    u32 base = host->base;
    u32 sts;

	MSG(INT, "[msdc_intr_wait]s\n");
    /* warning that interrupts are not enabled */
    WARN_ON((MSDC_READ32(MSDC_INTEN) & intrs) != intrs);
	event_wait(&msdc_int_event);
	MSG(INT, "[msdc_intr_wait]get event\n");
   // while (((sts = MSDC_READ32(MSDC_INT)) & intrs) == 0);
    sts = g_int_status;
   g_int_status = 0;
   // sts = MSDC_READ32(MSDC_INT);
    MSG(INT, "[SD%d] INT(0x%x)\n", host->id, sts);
   // MSDC_WRITE32(MSDC_INT, (sts & intrs));
    if (~intrs & sts) {
        MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
            host->id, ~intrs & sts);
    }
    return sts;
}

u32 msdc_intr_cmd_wait(struct mmc_host *host, u32 intrs)
{
    u32 base = host->base;
    u32 sts;
    /* warning that interrupts are not enabled */
 //   WARN_ON((MSDC_READ32(MSDC_INTEN) & intrs) != intrs);
    while (((sts = MSDC_READ32(MSDC_INT)) & intrs) == 0);
    MSG(INT, "[SD%d] INT(0x%x)\n", host->id, sts);
    MSDC_WRITE32(MSDC_INT, (sts & intrs));
    if (~intrs & sts) {
        MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
            host->id, ~intrs & sts);
    }
    return sts;
}

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

void lk_msdc_irq_handler(unsigned int irq)
{
	u32 intrs,sts;
	struct mmc_host *host = mmc_get_host(irq-MT_MSDC0_IRQ_ID);
	u32 base = host->base;
	
	mt_irq_ack(irq);
	intrs = MSDC_READ32(MSDC_INTEN);
	msdc_intr_mask(host,intrs);
	MSG(INT, "[lk_msdc_irq_handler]irq %x enable:%x %x\n",irq,intrs,MSDC_READ32(MSDC_INT));
	if((sts = MSDC_READ32(MSDC_INT))& intrs){	
		g_int_status = sts;
		MSG(INT, "[lk_msdc_irq_handler]send event,%x\n",g_int_status);
		MSDC_WRITE32(MSDC_INT, sts);
		event_signal(&msdc_int_event,0);
	}
	return;
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
    } else if (opcode == MMC_CMD_WRITE_BLOCK) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK || 
               opcode == SD_ACMD_SEND_SCR ||
               opcode == SD_CMD_SWITCH ||
               opcode == MMC_CMD_SEND_EXT_CSD) {
        rawcmd |= (1 << 11);    
    } else if (opcode == MMC_CMD_STOP_TRANSMISSION) {
        rawcmd |= (1 << 14);
        rawcmd &= ~(0x0FFF << 16);
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
    } else if (opcode == SD_CMD_VOL_SWITCH) {
        rawcmd |= (1 << 30);
    } else if (opcode == SD_CMD_SEND_TUNING_BLOCK) {
        rawcmd |= (1 << 11); /* CHECKME */
        if (priv->autocmd & MSDC_AUTOCMD19)
            rawcmd |= (3 << 28);
    } else if (opcode == MMC_CMD_GO_IRQ_STATE) {
        rawcmd |= (1 << 15);
    } else if (opcode == MMC_CMD_WRITE_DAT_UNTIL_STOP) {
        rawcmd |= ((1<< 13) | (3 << 11));
    } else if (opcode == MMC_CMD_READ_DAT_UNTIL_STOP) {
        rawcmd |= (3 << 11);
    }
    
    MSG(CMD, "[SD%d] CMD(%d): ARG(0x%x), RAW(0x%x), RSP(%d)\n", 
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), cmd->arg, rawcmd, rsptyp);

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
    
    SDC_SEND_CMD(rawcmd, cmd->arg);

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

    status = msdc_intr_cmd_wait(host, wints);

    if (status == 0) {
        error = MMC_ERR_TIMEOUT;
        goto end;
    }
    
    if ((status & MSDC_INT_CMDRDY) || (status & MSDC_INT_ACMDRDY) ||
        (status & MSDC_INT_ACMD19_DONE)) {
        switch (rsptyp) {
        case RESP_NONE:
            MSG(RSP, "[SD%d] CMD(%d): RSP(%d)\n", host->id, opcode, rsptyp);
            break;
        case RESP_R2:
        {
            u32 *resp = &cmd->resp[0];
            *resp++ = MSDC_READ32(SDC_RESP3);
            *resp++ = MSDC_READ32(SDC_RESP2);
            *resp++ = MSDC_READ32(SDC_RESP1);
            *resp++ = MSDC_READ32(SDC_RESP0);
            MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x 0x%x 0x%x 0x%x\n", 
                host->id, opcode, cmd->rsptyp, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);         
            break;
        }
        default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
            if ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE))
                cmd->resp[0] = MSDC_READ32(SDC_ACMD_RESP);
            else
                cmd->resp[0] = MSDC_READ32(SDC_RESP0);
            MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x AUTO(%d)\n", host->id, opcode, 
                cmd->rsptyp, cmd->resp[0], 
                ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE)) ? 1 : 0);
            break;
        }
    } else if ((status & MSDC_INT_RSPCRCERR) || (status & MSDC_INT_ACMDCRCERR)) {
        error = MMC_ERR_BADCRC;
        printf("[SD%d] CMD(%d): RSP(%d) ERR(BADCRC)\n", 
            host->id, opcode, cmd->rsptyp);
    } else if ((status & MSDC_INT_CMDTMO) || (status & MSDC_INT_ACMDTMO)) {
        error = MMC_ERR_TIMEOUT;
        MSG(RSP, "[SD%d] CMD(%d): RSP(%d) ERR(CMDTO) AUTO(%d)\n", 
            host->id, opcode, cmd->rsptyp, status & MSDC_INT_ACMDTMO ? 1: 0);
    } else {
        error = MMC_ERR_INVALID;
        printf("[SD%d] CMD(%d): RSP(%d) ERR(INVALID), Status:%x\n", 
            host->id, opcode, cmd->rsptyp, status);        
    }

end:

    if (rsptyp == RESP_R1B) {
        while ((MSDC_READ32(MSDC_PS) & 0x10000) != 0x10000);
    }

#if MSDC_DEBUG    
    if ((error == MMC_ERR_NONE) && (MSG_EVT_MASK & MSG_EVT_RSP)){
        switch(cmd->rsptyp) {
        case RESP_R1:
        case RESP_R1B:
            msdc_dump_card_status(cmd->resp[0]);
            break;
        case RESP_R3:
            msdc_dump_ocr_reg(cmd->resp[0]);
            break;
        case RESP_R5:
            msdc_dump_io_resp(cmd->resp[0]);
            break;
        case RESP_R6:
            msdc_dump_rca_resp(cmd->resp[0]);
            break;
        }
    }
#endif

    cmd->error = error;
	 if(cmd->opcode == MMC_CMD_APP_CMD && error == MMC_ERR_NONE){
			 host->app_cmd = 1;
			 host->app_cmd_arg = cmd->arg;
			 }
	 else 
	 	host->app_cmd = 0;
	 		
    return error;
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
        if (tmp & 0x1800) {
            if(msdc_abort_handler(host, 1))
				printf("[SD%d] abort failed\n",host->id);
        }
        #ifdef FEATURE_MMC_CM_TUNING
        err = msdc_tune_cmdrsp(host, cmd);
        #endif
    }
    return err;
}
static int msdc_app_cmd(struct mmc_host *host)
{
	struct mmc_command appcmd;
	int err = MMC_ERR_NONE;
	int retries = 10;
	appcmd.opcode = MMC_CMD_APP_CMD;
	appcmd.arg = host->app_cmd_arg;
	appcmd.rsptyp  = RESP_R1;
    appcmd.retries = CMD_RETRIES;
    appcmd.timeout = CMD_TIMEOUT;
	
	do {
        err = msdc_cmd(host, &appcmd);
        if (err == MMC_ERR_NONE)
            break;
    } while (retries--);
	return err;
}
#if MSDC_USE_DMA_MODE
int msdc_sg_init(struct scatterlist *sg, void *buf, u32 buflen)
{
    int i = MAX_SG_POOL_SZ;
    char *ptr = (char *)buf;

    BUG_ON(buflen > MAX_SG_POOL_SZ * MAX_SG_BUF_SZ);
    msdc_flush_membuf(buf, buflen);
    while (i > 0) {
        if (buflen > MAX_SG_BUF_SZ) {
            sg->addr = (u32)ptr;
            sg->len  = MAX_SG_BUF_SZ;
            buflen  -= MAX_SG_BUF_SZ;
            ptr     += MAX_SG_BUF_SZ;
            sg++; i--; 
        } else {
            sg->addr = (u32)ptr;
            sg->len  = buflen;
            i--;
            break;
        }
    }

    

    return MAX_SG_POOL_SZ - i;
}

void msdc_dma_init(struct mmc_host *host, struct dma_config *cfg, void *buf, u32 buflen)
{
    u32 base = host->base;
    
    cfg->xfersz = buflen;
    
    if (cfg->mode == MSDC_MODE_DMA_BASIC) {
        cfg->sglen = 1;
        cfg->sg[0].addr = (u32)buf;
        cfg->sg[0].len = buflen;
        msdc_flush_membuf(buf, buflen);
    } else {
        cfg->sglen = msdc_sg_init(cfg->sg, buf, buflen);
    }

    MSDC_CLR_FIFO();
    MSDC_DMA_ON();    
}

int msdc_dma_cmd(struct mmc_host *host, struct dma_config *cfg, struct mmc_command *cmd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 opcode = cmd->opcode;
    u32 rsptyp = cmd->rsptyp;    
    u32 rawcmd;

    rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) | 
        rsptyp << 7 | host->blklen << 16;

    if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
        rawcmd |= ((2 << 11) | (1 << 13));
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_WRITE_BLOCK) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK) {
        rawcmd |= (1 << 11);
    } else {
        return -1;
    }

    MSG(DMA, "[SD%d] DMA CMD(%d), AUTOCMD12(%d), AUTOCMD23(%d)\n",
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), 
        (priv->autocmd & MSDC_AUTOCMD12) ? 1 : 0,
        (priv->autocmd & MSDC_AUTOCMD23) ? 1 : 0);
        
    cfg->cmd = rawcmd;
    cfg->arg = cmd->arg;

    return 0;
}

#ifdef MSDC_DMA_BOUNDARY_LIMITAION

#define MSDC_DMA_BOUNDARY_SIZE                       (2048)
#define MSDC_DMA_START_ADDRESS_ALIGNMENT             (0)
#define MSDC_DMA_NOT_EXCEED_BOUNDARY                 (0)
#define MSDC_DMA_ACROSS_BOUNDARY                     (1)

u32 msdc_dma_address_check(u32 start, u32 len)
{
    /* mask bit0-1 */
    start &= (~0x3);

    /* two type of violation 2k boundary will be checked */
    if (0 == start % 64)
        return MSDC_DMA_START_ADDRESS_ALIGNMENT;
    
    if (len > (MSDC_DMA_BOUNDARY_SIZE - (start & 0x7FF))){
        /* cross boundary */
        return MSDC_DMA_ACROSS_BOUNDARY;
    }

    return MSDC_DMA_NOT_EXCEED_BOUNDARY;
}
#endif

int msdc_dma_config(struct mmc_host *host, struct dma_config *cfg)
{
    u32 base = host->base;
    u32 sglen = cfg->sglen;
    u32 i, j, num, bdlen, arg, xfersz;
    u8  blkpad, dwpad, chksum;
    struct scatterlist *sg = cfg->sg;
    gpd_t *gpd;
    bd_t *bd;
#ifdef MSDC_DMA_BOUNDARY_LIMITAION
    u32 ret;
    u32 dma_address, dma_len;
    u32 dma_address_tmp, dma_len_tmp;
#endif

    switch (cfg->mode) {
    case MSDC_MODE_DMA_BASIC:
        BUG_ON(cfg->xfersz > 65535);
        BUG_ON(cfg->sglen != 1);
        MSDC_WRITE32(MSDC_DMA_SA, sg->addr);
        
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_LASTBUF, 1);
        MSDC_WRITE32(MSDC_DMA_LEN, sg->len);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 0);
        break;
    case MSDC_MODE_DMA_DESC:
        blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
        dwpad  = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
        chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;

#if 0 /* YD: current design doesn't support multiple GPD in descriptor dma mode */
        /* calculate the required number of gpd */
        num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;        
        gpd = msdc_alloc_gpd(host, num);
        for (i = 0; i < num; i++) {
            gpd[i].intr = 0;
            if (sglen > MAX_BD_PER_GPD) {
                bdlen  = MAX_BD_PER_GPD;
                sglen -= MAX_BD_PER_GPD;
            } else {
                bdlen = sglen;
                sglen = 0;
            }
            bd = msdc_alloc_bd(host, bdlen);
            for (j = 0; j < bdlen; j++) {
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                sg++;
            }
            msdc_queue_bd(host, &gpd[i], bd);
            msdc_flush_membuf(bd, bdlen * sizeof(bd_t));
        }     
        msdc_add_gpd(host, gpd, num);
        #if MSDC_DEBUG
        msdc_dump_dma_desc(host);
        #endif
        msdc_flush_membuf(gpd, num * sizeof(gpd_t));
        MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
        MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);
#else
        /* calculate the required number of gpd */
        BUG_ON(sglen > MAX_BD_POOL_SZ);
        
        gpd = msdc_alloc_gpd(host, 1);
        gpd->intr = 0;

#ifdef MSDC_DMA_BOUNDARY_LIMITAION
        bd = ((msdc_priv_t*)host->priv)->bd_pool + ((msdc_priv_t*)host->priv)->alloc_bd;;
        for (j = 0; j < sglen; j++) {
            /* just for MT6582 */
            dma_address = sg->addr;
            dma_len = sg->len;
            ret =  msdc_dma_address_check(dma_address, dma_len);
            if (ret == MSDC_DMA_ACROSS_BOUNDARY){
                /* add debug info */
                MSG(DMA, "across 2k boundary; start = 0x%x, len = %d\n", dma_address, dma_len);

                /* need spilt the BD to BD_1, BD_2, 
                 * rule: make sure the start address of BD_2 is 2K boundary */
                dma_len_tmp = MSDC_DMA_BOUNDARY_SIZE - (dma_address & 0x7FF);
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address, dma_len_tmp);           

                MSG(DMA, "BD_1: start = 0x%x, len = %d\n", dma_address, dma_len_tmp);

                bd[j].eol = 0; 

                j += 1;
                sglen += 1;

                dma_address_tmp = dma_address + dma_len_tmp;
                dma_len_tmp = dma_len - dma_len_tmp;
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address_tmp, dma_len_tmp);            

                MSG(DMA, "BD_2: start = 0x%x, len = %d\n", dma_address_tmp, dma_len_tmp);

                if(j == sglen - 1) {
                    bd[j].eol = 1;     	/* the last bd */
                } else {
                    bd[j].eol = 0; 	
                }
            } else {
                /* if no need change, take the old way */
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, dma_address, dma_len);            

                if(j == sglen - 1) {
                    bd[j].eol = 1;     	/* the last bd */
                } else {
                    bd[j].eol = 0; 	
                }
            }
            sg++;
        }

        //MSG(DMA, "j = %d, sglen = %d\n", j, sglen);
        bd = msdc_alloc_bd_simp(host, sglen);
#else
        bd = msdc_alloc_bd(host, sglen);
        for (j = 0; j < sglen; j++) {
            MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
            sg++;
        }
#endif
        msdc_queue_bd(host, &gpd[0], bd);
        msdc_flush_membuf(bd, sglen * sizeof(bd_t));

        msdc_add_gpd(host, gpd, 1);
#if MSDC_DEBUG
        msdc_dump_dma_desc(host);
#endif
        msdc_flush_membuf(gpd, (1 + 1) * sizeof(gpd_t));
        MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
        MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);

#endif        
        break;
    case MSDC_MODE_DMA_ENHANCED:
        arg = cfg->arg;
        blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
        dwpad  = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
        chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;

        /* calculate the required number of gpd */
        num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;        
        gpd = msdc_alloc_gpd(host, num);
        for (i = 0; i < num; i++) {            
            xfersz = 0;
            if (sglen > MAX_BD_PER_GPD) {
                bdlen  = MAX_BD_PER_GPD;
                sglen -= MAX_BD_PER_GPD;
            } else {
                bdlen = sglen;
                sglen = 0;
            }
            bd = msdc_alloc_bd(host, bdlen);
            for (j = 0; j < bdlen; j++) {
                xfersz += sg->len;
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                sg++;
            }
            /* YD: 1 XFER_COMP interrupt will be triggerred by each GPD when it
             * is done. For multiple GPDs, multiple XFER_COMP interrupts will be 
             * triggerred. In such situation, it's not easy to know which 
                 * interrupt indicates the transaction is done. So, we use the 
                 * latest one GPD's INT as the transaction done interrupt.
                 */
            //gpd[i].intr = cfg->intr;
            gpd[i].intr = (i == num - 1) ? 0 : 1;
            gpd[i].cmd  = cfg->cmd;
            gpd[i].blknum = xfersz / cfg->blklen;
            gpd[i].arg  = arg;
            gpd[i].extlen = 0xC;

            arg += xfersz;

            msdc_queue_bd(host, &gpd[i], bd);
            msdc_flush_membuf(bd, bdlen * sizeof(bd_t));
        }     
        msdc_add_gpd(host, gpd, num);
        #if MSDC_DEBUG
        msdc_dump_dma_desc(host);
        #endif
        msdc_flush_membuf(gpd, num * sizeof(gpd_t));
        MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
        MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);
        break;
    default:
        break;
    }
    MSG(DMA, "[SD%d] DMA_SA   = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_SA));
    MSG(DMA, "[SD%d] DMA_CA   = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CA));
    MSG(DMA, "[SD%d] DMA_CTRL = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CTRL));
    MSG(DMA, "[SD%d] DMA_CFG  = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CFG));

    return 0;
}

void msdc_dma_resume(struct mmc_host *host)
{
    u32 base = host->base;

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_RESUME, 1);

    MSG(DMA, "[SD%d] DMA resume\n", host->id);
}

void msdc_dma_start(struct mmc_host *host)
{
    u32 base = host->base;
	u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR;
	msdc_intr_unmask(host,wints);

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_START, 1);

    MSG(DMA, "[SD%d] DMA start\n", host->id);
}

void msdc_dma_stop(struct mmc_host *host)
{
    u32 base = host->base;
	u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR;

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP, 1);
    while ((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS) != 0);
    MSDC_DMA_OFF();

    MSG(DMA, "[SD%d] DMA Stopped\n", host->id);
    msdc_intr_mask(host,wints);
    msdc_reset_gpd(host);
}

int msdc_dma_wait_done(struct mmc_host *host, u32 timeout)
{
    u32 base = host->base;
    //u32 tmo = timeout;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    u32 status;
    u32 error = MMC_ERR_NONE;
    u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR |
        MSDC_INT_DXFER_DONE | MSDC_INT_DMAQ_EMPTY | 
        MSDC_INT_ACMDRDY | MSDC_INT_ACMDTMO | MSDC_INT_ACMDCRCERR | 
        MSDC_INT_CMDRDY | MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR;

    do {
        MSG(DMA, "[SD%d] DMA Curr Addr: 0x%x, Active: %d\n", host->id, 
            MSDC_READ32(MSDC_DMA_CA), MSDC_READ32(MSDC_DMA_CFG) & 0x1);    

        status = msdc_intr_wait(host, wints);

        if (status == 0 || status & MSDC_INT_DATTMO) {
            MSG(DMA, "[SD%d] DMA DAT timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;
        } else if (status & MSDC_INT_DATCRCERR) {
            MSG(DMA, "[SD%d] DMA DAT CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;
        } else if (status & MSDC_INT_CMDTMO) {
            MSG(DMA, "[SD%d] DMA CMD timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;        
        } else if (status & MSDC_INT_RSPCRCERR) {
            MSG(DMA, "[SD%d] DMA CMD CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;    
        } else if (status & MSDC_INT_ACMDTMO) {
            MSG(DMA, "[SD%d] DMA ACMD timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;        
        } else if (status & MSDC_INT_ACMDCRCERR) {
            MSG(DMA, "[SD%d] DMA ACMD CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;
        }

        if ((cfg->mode == MSDC_MODE_DMA_ENHANCED) && (status & MSDC_INT_CMDRDY)) {
            cfg->rsp = MSDC_READ32(SDC_RESP0);
            MSG(DMA, "[SD%d] DMA ENH CMD Rdy, Resp(%xh)\n", host->id, cfg->rsp);
            #if MSDC_DEBUG 
            msdc_dump_card_status(cfg->rsp);
            #endif
        }
        if (status & MSDC_INT_ACMDRDY) {
            cfg->autorsp = MSDC_READ32(SDC_ACMD_RESP);
            MSG(DMA, "[SD%d] DMA AUTO CMD Rdy, Resp(%xh)\n", host->id, cfg->autorsp);
            #if MSDC_DEBUG 
            msdc_dump_card_status(cfg->autorsp);
            #endif
        }
        if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
            /* YD: 1 XFER_COMP interrupt will be triggerred by each GPD when it
             * is done. For multiple GPDs, multiple XFER_COMP interrupts will be 
             * triggerred. In such situation, it's not easy to know which 
             * interrupt indicates the transaction is done. So, we use the 
             * latest one GPD's INT as the transaction done interrupt.
             */        
            if (status & MSDC_INT_DXFER_DONE)
                break;
        } else {
            if (status & MSDC_INT_XFER_COMPL)
                break;
        }
    } while (1);

    /* check dma status */
    do {
        status = MSDC_READ32(MSDC_DMA_CFG);
        if (status & MSDC_DMA_CFG_GPDCSERR) {
            MSG(DMA, "[SD%d] GPD checksum error\n", host->id);
            error = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_DMA_CFG_BDCSERR) {
            MSG(DMA, "[SD%d] BD checksum error\n", host->id);
            error = MMC_ERR_BADCRC;
            break;
        } else if ((status & MSDC_DMA_CFG_STS) == 0) {
            break;
        }
    } while (1);
end:
	if(error)
		MSDC_RESET();
    return error;
}

int msdc_dma_transfer(struct mmc_host *host, struct mmc_command *cmd, u8 *buf, u32 nblks)
{
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    struct mmc_command stop;

    BUG_ON(nblks * blksz > MAX_DMA_TRAN_SIZE);

    multi = nblks > 1 ? 1 : 0;

    if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
        if (multi && (priv->autocmd == 0))
            msdc_set_autocmd(host, MSDC_AUTOCMD12, 1);
        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);
        msdc_dma_cmd(host, cfg, cmd);
        msdc_dma_init(host, cfg, (void*)buf, nblks * blksz);
        msdc_dma_config(host, cfg);
        msdc_dma_start(host);
        err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
        msdc_dma_stop(host);
		msdc_flush_membuf(buf,nblks * blksz);
        if (multi && (priv->autocmd == 0))
            msdc_set_autocmd(host, MSDC_AUTOCMD12, 0);
    } else {
        u32 left_sz, xfer_sz;

        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);

        left_sz = nblks * blksz;

        if (cfg->mode == MSDC_MODE_DMA_BASIC) {
            xfer_sz = left_sz > MAX_DMA_CNT ? MAX_DMA_CNT : left_sz;
            nblks   = xfer_sz / blksz;
        } else {
            xfer_sz = left_sz;
        }

        while (left_sz) {
#if 1
            msdc_set_blknum(host, nblks);
            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);

            err = msdc_send_cmd(host, cmd);
            msdc_dma_config(host, cfg);
           	
            if (err != MMC_ERR_NONE) {
                msdc_reset_gpd(host);
                goto done;
            }

            err = msdc_wait_rsp(host, cmd);
            
            if (err == MMC_ERR_BADCRC) {
                u32 base = host->base;
                u32 tmp = MSDC_READ32(SDC_CMD);
            
                /* check if data is used by the command or not */
                if (tmp & 0x1800) {
                    if(msdc_abort_handler(host, 1))
						printf("[SD%d] abort failed\n",host->id);
                }
                #ifdef FEATURE_MMC_CM_TUNING
                err = msdc_tune_cmdrsp(host, cmd);
                #endif
				if(err != MMC_ERR_NONE){
					msdc_reset_gpd(host);
					goto done;
				}
            }
			msdc_dma_start(host);
            err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);
			msdc_flush_membuf(buf,nblks * blksz);
#else
            msdc_set_blknum(host, nblks);
            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);
            msdc_dma_config(host, cfg);

            err = msdc_cmd(host, cmd);
            if (err != MMC_ERR_NONE) {
                msdc_reset_gpd(host);
                goto done;
            }

            msdc_dma_start(host);
            err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);
#endif

            if (multi && (priv->autocmd == 0)) {
                stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
                stop.rsptyp  = RESP_R1B;
                stop.arg     = 0;
                stop.retries = CMD_RETRIES;
                stop.timeout = CMD_TIMEOUT;
                err = msdc_cmd(host, &stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
            }
            if (err != MMC_ERR_NONE)
                goto done;
            buf     += xfer_sz;
            left_sz -= xfer_sz;

            /* left_sz > 0 only when in basic dma mode */
            if (left_sz) {
                cmd->arg += nblks; /* update to next start address */
                xfer_sz  = (xfer_sz > left_sz) ? left_sz : xfer_sz;
                nblks    = (left_sz > xfer_sz) ? nblks : left_sz / blksz;
            }
        }
    }
done:
    if (derr != MMC_ERR_NONE) {
        printf("[SD%d] <CMD%d> DMA data error (%d)\n", host->id, cmd->opcode, derr);
		if(msdc_abort_handler(host, 1))
			printf("[SD%d] abort failed\n",host->id);
    }

    return err;
}

int msdc_dma_bread(struct mmc_host *host, u8 *dst, u32 src, u32 nblks)
{
    int multi;
    struct mmc_command cmd;    

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Read data %d blks from 0x%x\n", host->id, nblks, src);

    multi = nblks > 1 ? 1 : 0;

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    return msdc_dma_transfer(host, &cmd, dst, nblks);
}

int msdc_dma_bwrite(struct mmc_host *host, u32 dst, u8 *src, u32 nblks)
{
    int multi;
    struct mmc_command cmd;

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Write data %d blks to 0x%x\n", host->id, nblks, dst);

    multi = nblks > 1 ? 1 : 0;

    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    return msdc_dma_transfer(host, &cmd, src, nblks);
}
#endif

int msdc_pio_read_word(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 100000;
    u32 status;
    u32 totalsz = size;
    u8  done = 0;
	u8* u8ptr;
    while (1) {
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left:%d/%d bytes, RXFIFO:%d\n", 
                host->id, status, size, totalsz, MSDC_RXFIFOCNT());
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left: %d/%d bytes, RXFIFO:%d\n", 
                host->id, status, size, totalsz, MSDC_RXFIFOCNT());
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            done = 1;
        }

        if (size == 0 && done)
            break;

        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD)) {
            int left = MSDC_FIFO_THD >> 2;
            do {
                *ptr++ = MSDC_FIFO_READ32();
            } while (--left);
            size -= MSDC_FIFO_THD;
            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(), size, totalsz);
        } else if ((size < MSDC_FIFO_THD) && MSDC_RXFIFOCNT() >= size) {
            while (size) {
                if (size > 3) {
                    *ptr++ = MSDC_FIFO_READ32();
                    size -= 4;
                } else {
                    u8ptr = (u8*)ptr;
					while(size --)
                    	*u8ptr++ = MSDC_FIFO_READ8();
                }
            }
            MSG(FIO, "[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
                host->id, MSDC_RXFIFOCNT(), size, totalsz);
        }
    }

    return err;
}

int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = msdc_pio_read_word(host, (u32*)ptr, size); 

    if (err != MMC_ERR_NONE) {
        msdc_abort(host);
        MSG(OPS, "[SD%d] PIO Read Error (%d)\n", host->id, err);
    }
    
    return err;
}

int msdc_pio_write_word(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 250000;
    u32 status;
	u8* u8ptr;
    //msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    while (1) {
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            if (size == 0) {
                MSG(OPS, "[SD%d] all data flushed to card\n", host->id); 
                break;
            } else {
                MSG(WRN, "[SD%d]<CHECKME> XFER_COMPL before all data written\n",
                    host->id);               
            }
        }

        if (size == 0)
            continue;

        if (size >= MSDC_FIFO_SZ) {
            if (MSDC_TXFIFOCNT() == 0) {
                int left = MSDC_FIFO_SZ >> 2;
                do {
                    MSDC_FIFO_WRITE32(*ptr); ptr++;
                } while (--left);
                size -= MSDC_FIFO_SZ;
            }
        } else if (size < MSDC_FIFO_SZ && MSDC_TXFIFOCNT() == 0) {            
            while (size ) {
                if (size > 3) {
                    MSDC_FIFO_WRITE32(*ptr); ptr++;
                    size -= 4;
                } else {
                     u8ptr = (u8*)ptr;
					 while(size --){
                    	 MSDC_FIFO_WRITE8(*u8ptr);
						 u8ptr++;
					 	}
                }
            }
        }
    }

    return err;
}

int msdc_pio_write(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = msdc_pio_write_word(host, (u32*)ptr, size);

    if (err != MMC_ERR_NONE) {
        msdc_abort(host);
        MSG(OPS, "[SD%d] PIO Write Error (%d)\n", host->id, err);
    }

    return err;
}

#if !MSDC_USE_DMA_MODE
int msdc_pio_bread(struct mmc_host *host, u8 *dst, u32 src, u32 nblks)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    struct mmc_command cmd;
    struct mmc_command stop;
    u32 *ptr = (u32 *)dst;

    MSG(OPS, "[SD%d] Read data %d bytes from 0x%x\n", host->id, nblks * blksz, src);

    multi = nblks > 1 ? 1 : 0;

    MSDC_CLR_FIFO();

    msdc_set_blknum(host, nblks);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = derr = msdc_pio_read(host, (u32*)ptr, nblks * blksz);
    
    if (multi && (priv->autocmd == 0)) {
        stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
        stop.rsptyp  = RESP_R1B;
        stop.arg     = 0;
        stop.retries = CMD_RETRIES;
        stop.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
    }

done:
    if (err != MMC_ERR_NONE) {
        if (derr != MMC_ERR_NONE) {
            printf("[SD%d] Read data error (%d)\n", host->id, derr);
            if(msdc_abort_handler(host, 1))
				printf("[SD%d] abort failed\n",host->id);
        } else {
            printf("[SD%d] Read error (%d)\n", host->id, err);
        }
    }
    return (derr == MMC_ERR_NONE) ? err : derr;
}

int msdc_pio_bwrite(struct mmc_host *host, u32 dst, u8 *src, u32 nblks)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    struct mmc_command cmd;
    struct mmc_command stop;
    u32 *ptr = (u32 *)src;

    MSG(OPS, "[SD%d] Write data %d bytes to 0x%x\n", host->id, nblks * blksz, dst);

    multi = nblks > 1 ? 1 : 0;	

    MSDC_CLR_FIFO();

    msdc_set_blknum(host, nblks);
    msdc_set_blklen(host, blksz);
    
    /* No need since MSDC always waits 8 cycles for write data timeout */
    
    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = derr = msdc_pio_write(host, (u32*)ptr, nblks * blksz);

    if (multi && (priv->autocmd == 0)) {
        stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
        stop.rsptyp  = RESP_R1B;
        stop.arg     = 0;
        stop.retries = CMD_RETRIES;
        stop.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
    }

done:
    if (err != MMC_ERR_NONE) {
        if (derr != MMC_ERR_NONE) {
           	printf("[SD%d] Write data error (%d)\n", host->id, derr);
            if(msdc_abort_handler(host, 1))
				printf("[SD%d] abort failed\n",host->id);
        } else {
            printf("[SD%d] Write error (%d)\n", host->id, err);
        }
    }
    return (derr == MMC_ERR_NONE) ? err : derr;
}
#endif

void msdc_config_clksrc(struct mmc_host *host, clk_source_t clksrc)
{
    u32 clks[] = {200000000};
	#if 0 // PLL manager will set MT6588 default clock source . CHECK ME
    u32 tmp;

    MSDC_SET_BIT32(0xc1000010, 1 << (15 + host->id)); /* Enable PDN MSDC */

    /* 2 msdc clock cycles is required before gating. Use 1ms delay to avoid
     * glitch in switching clock source
     */
    mdelay(1);

    tmp = MSDC_READ32(PERI_MSDC_SRCSEL);
    tmp &= ~0x3;
    tmp |= clksrc;
    MSDC_WRITE32(PERI_MSDC_SRCSEL, tmp);
    #endif
    host->clksrc = clksrc;
#ifdef FPGA_PLATFORM
    host->clk = 12000000; 
#else 
    host->clk  = clks[clksrc];
#endif
#if 0 // PLL manager will set MT6588 default clock source . CHECK ME
    MSDC_CLR_BIT32(0xc1000010, 1 << (15 + host->id)); /* Disable PDN MSDC */
#endif
}

void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz)
{
    u32 base = host->base;
    u32 mode;
    u32 div;
    u32 sclk;
    u32 orig_clksrc = host->clksrc;

    if (ddr) {
        mode = 0x2; /* ddr mode and use divisor */
        if (hz >= (host->clk >> 2)) {
            div  = 0;			   /* mean div = 1/2 */
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
            div  = 0;			   /* mean div = 1/2 */
            sclk = host->clk >> 1; /* sclk = clk / 2 */
        } else {
            div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
            sclk = (host->clk >> 2) / div;
        }
    }
    host->sclk = sclk;

#if 0
    if (hz > 100000000) {
        MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVN, msdc_cap.clk_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVP, msdc_cap.clk_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVN, msdc_cap.cmd_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVP, msdc_cap.cmd_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVN, msdc_cap.dat_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVP, msdc_cap.dat_18v_drv);
        /* don't enable cksel for ddr mode */
        if (host->card && mmc_card_ddr(host->card) == 0)
            MSDC_SET_FIELD(MSDC_PATCH_BIT0,MSDC_CKGEN_MSDC_CK_SEL, 1);
    }
#endif

    //msdc_config_clksrc(host, 3);

    /* set clock mode and divisor */
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD|MSDC_CFG_CKDIV, (mode << 8)|div);
    //MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, div);
	//int temp = 0;
	//MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD|MSDC_CFG_CKDIV,temp);
	 //printf("MSDC_CFG_CKMOD|MSDC_CFG_CKDIV<0x%x>\n",temp);
    msdc_config_clksrc(host, orig_clksrc);

    /* wait clock stable */
    while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));

    printf("[SD%d] SET_CLK(%dkHz): SCLK(%dkHz) MODE(%d) DDR(%d) DIV(%d) DS(%d) RS(%d)\n",
        host->id, hz/1000, sclk/1000, mode, ddr > 0 ? 1 : 0, div,  
        msdc_cap.data_edge, msdc_cap.cmd_edge);
}

void msdc_config_bus(struct mmc_host *host, u32 width)
{
    u32 base = host->base;
    u32 val  = MSDC_READ32(SDC_CFG);

    val &= ~SDC_CFG_BUSWIDTH;

    switch (width) {
    default:
        width = HOST_BUS_WIDTH_1;
    case HOST_BUS_WIDTH_1:
        val |= (MSDC_BUS_1BITS << 16);
        break;
    case HOST_BUS_WIDTH_4:
        val |= (MSDC_BUS_4BITS << 16);
        break;
    case HOST_BUS_WIDTH_8:
        val |= (MSDC_BUS_8BITS << 16);
        break;
    }
    MSDC_WRITE32(SDC_CFG, val);

    printf("[SD%d] Bus Width: %d\n", host->id, width);
}


void msdc_config_pin(struct mmc_host *host, int mode)
{
#ifdef FPGA_PLATFORM
    u32 base = host->base;
#endif

    MSG(CFG, "[SD%d] Pins mode(%d), none(0), down(1), up(2), keep(3)\n", 
        host->id, mode);

    switch (mode) {
    case MSDC_PIN_PULL_UP:
		#ifdef FPGA_PLATFORM
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPU, 1);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPD, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPU, 1);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPD, 0);
		#else
		msdc_pin_pud(host,1);
		#endif
        break;
    case MSDC_PIN_PULL_DOWN:
		#ifdef FPGA_PLATFORM
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPD, 1);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPD, 1);
		#else
		msdc_pin_pud(host,0);
		#endif
        break;
    case MSDC_PIN_PULL_NONE:
    default:
		#ifdef FPGA_PLATFORM
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPD, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPD, 0);
		#else
		msdc_pin_pnul(host,0);
		#endif
        break;
    }
}

#ifdef FEATURE_MMC_UHS1
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

    /* check if CMD/DATA lines both 0 */
    if ((MSDC_READ32(MSDC_PS) & ((1 << 24) | (0xF << 16))) == 0) {

        /* pull up disabled in CMD and DAT[3:0] */
        msdc_config_pin(host, MSDC_PIN_PULL_NONE);

        /* change signal from 3.3v to 1.8v */
        msdc_set_host_level_pwr(1);

        /* wait at least 5ms for 1.8v signal switching in card */
        mdelay(10);

        /* config clock to 10~12MHz mode for volt switch detection by host */
        msdc_config_clock(host, 0, 10000000);

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

void msdc_clock(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s clock \n", host->id, on ? "on" : "off", "host");
}

void msdc_host_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "host");

    if (on) {
        msdc_config_pin(host, MSDC_PIN_PULL_UP);      
        msdc_set_host_pwr(1);
        msdc_clock(host, 1);
    } else {
        msdc_clock(host, 0);
        msdc_set_host_pwr(0);
        msdc_config_pin(host, MSDC_PIN_PULL_DOWN);
    }
}

void msdc_card_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "card");

    if (on) {
        msdc_set_card_pwr(1);
    } else {
        msdc_set_card_pwr(0);
    }
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
void msdc_reset_tune_counter(struct mmc_host *host)
{
	host->time_read = 0;
}
#ifdef FEATURE_MMC_CM_TUNING
int msdc_tune_cmdrsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base = host->base;
	u32 sel = 0;
    u32 rsmpl,cur_rsmpl, orig_rsmpl;
	u32 rrdly,cur_rrdly, orig_rrdly;
	u32 cntr,cur_cntr,orig_cmdrtc;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel;
    u32 times = 0;
    int result = MMC_ERR_CMDTUNEFAIL;

    if (host->sclk > 100000000){
		sel = 1;
		//MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL,0);
		}
		
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, orig_rrdly);
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, orig_cmdrtc);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

	 dl_cksel = 0;
        do {
			cntr = 0;
			do{
				rrdly = 0;
             do {
                for (rsmpl = 0; rsmpl < 2; rsmpl++) {
                    cur_rsmpl = (orig_rsmpl + rsmpl) % 2;
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl);
					if(host->sclk <= 400000){ //In sd/emmc init flow, fix rising edge for latching cmd response
							MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
						}
                    if (cmd->opcode != MMC_CMD_STOP_TRANSMISSION) {
						if(host->app_cmd){
							result = msdc_app_cmd(host);
							if(result != MMC_ERR_NONE)
								return MMC_ERR_CMDTUNEFAIL;
							}
                        result = msdc_send_cmd(host, cmd);
						if(result == MMC_ERR_TIMEOUT)
							rsmpl--;
                        if (result != MMC_ERR_NONE && cmd->opcode != MMC_CMD_STOP_TRANSMISSION){
							if(cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK || cmd->opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK || cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK ||cmd->opcode == MMC_CMD_WRITE_BLOCK)
								msdc_abort_handler(host,1);
                            continue;
							}
                        result = msdc_wait_rsp(host, cmd);                        
                    } else if(cmd->opcode == MMC_CMD_STOP_TRANSMISSION){
							result = MMC_ERR_NONE;
							goto done;
                    	}
					  else
                       		 result = MMC_ERR_BADCRC;
                    #if MSDC_TUNE_LOG
                    /* for debugging */
                    {
                        u32 t_rrdly, t_rsmpl, t_dl_cksel,t_cmdrtc;
						MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, t_rsmpl);
						MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, t_rrdly);
                        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
						MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, t_cmdrtc);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);
                        
                        times++;
                        printf("[SD%d] <TUNE_CMD><%d><%s> CMDRRDLY=%d, RSPL=%dh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", t_rrdly, t_rsmpl);
                        //printf("[SD%d] <TUNE_CMD><%d><%s> MSDC_CKGEN_RX_SDCLKO_SEL=%xh\n",
                            //host->id, times, (result == MMC_ERR_NONE) ?
                            //"PASS" : "FAIL", t_cksel);
						printf("[SD%d] <TUNE_CMD><%d><%s> CMD_RSP_TA_CNTR=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", t_cmdrtc);
                        printf("[SD%d] <TUNE_CMD><%d><%s> INT_DAT_LATCH_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", t_dl_cksel);
						
                    }
					#endif
                    if (result == MMC_ERR_NONE)                        
                        goto done;
					/*if ((result == MMC_ERR_TIMEOUT || result == MMC_ERR_BADCRC)&&  cmd->opcode == MMC_CMD_STOP_TRANSMISSION){
						printf("[SD%d]TUNE_CMD12--failed ignore\n",host->id);
						result = MMC_ERR_NONE;
						goto done;
						}*/
					if(cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK || cmd->opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK || cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK ||cmd->opcode == MMC_CMD_WRITE_BLOCK)
						msdc_abort_handler(host,1);
                }    
                cur_rrdly = (orig_rrdly + rrdly + 1) % 32;
                MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, cur_rrdly);
            	} while (++rrdly < 32);
			 if(!sel)
			 	break;			 	
			 	cur_cntr = (orig_cmdrtc + cntr + 1) % 8;
                MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, cur_cntr);
        	}while(++cntr < 8);
            /* no need to update data ck sel */
            if (!sel)
                break;        
            cur_dl_cksel = (orig_dl_cksel +dl_cksel+1) % 8;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
            dl_cksel++;                
        } while(dl_cksel < 8);
        /* no need to update ck sel */
		if(result != MMC_ERR_NONE)
			result = MMC_ERR_CMDTUNEFAIL;
done:    
    return result;
}
#endif

#ifdef FEATURE_MMC_RD_TUNING
int msdc_tune_bread(struct mmc_host *host, u8 *dst, u32 src, u32 nblks)
{
    u32 base = host->base;
    u32 dcrc, ddr = 0, sel = 0;
    u32 cur_rxdly0, cur_rxdly1;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
	u32 dsel,cur_dsel,orig_dsel;
	u32 dl_cksel,cur_dl_cksel,orig_dl_cksel;
	u32 rxdly;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3, cur_dat4, cur_dat5, 
        cur_dat6, cur_dat7;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3, orig_dat4, orig_dat5, 
        orig_dat6, orig_dat7;
    u32 orig_clkmode;
    u32 times = 0;
    int result = MMC_ERR_READTUNEFAIL;

    if (host->sclk > 100000000)
        sel = 1;
	if(host->card)
    	ddr = mmc_card_ddr(host->card);
	MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD,orig_clkmode);
	//if(orig_clkmode == 1)
		//MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, 0);
	
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orig_dsel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);
	
    /* Tune Method 2. delay each data line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
 
        dl_cksel = 0;
        do {
			dsel = 0;
			do{
            rxdly = 0;
            do {
                for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                    cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl);                    
                    result = host->blk_read(host, dst, src, nblks);
					if(result == MMC_ERR_CMDTUNEFAIL || result == MMC_ERR_CMD_RSPCRC || result == MMC_ERR_ACMD_RSPCRC)
						goto done;
                    MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
					
                    if (!ddr) dcrc &= ~SDC_DCRC_STS_NEG;

                    /* for debugging */
					 #if MSDC_TUNE_LOG
                    {
                        u32 t_dspl, t_dl_cksel;

                        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, t_dspl);
                        //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                        times++;
                        printf("[SD%d] <TUNE_BREAD_%d><%s> DCRC=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", dcrc);
                        printf("[SD%d] <TUNE_BREAD_%d><%s> DATRDDLY0=%xh, DATRDDLY1=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", MSDC_READ32(MSDC_DAT_RDDLY0), MSDC_READ32(MSDC_DAT_RDDLY1));
                        //printf("[SD%d] <TUNE_BREAD_%d><%s> MSDC_CKGEN_RX_SDCLKO_SEL=%xh\n",
                            //host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            //"PASS" : "FAIL", t_cksel);
                        printf("[SD%d] <TUNE_BREAD_%d><%s> INT_DAT_LATCH_CK_SEL=%xh, DSMPL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", t_dl_cksel, t_dspl);
                    }
                    #endif
                    /* no cre error in this data line */
                    if (result == MMC_ERR_NONE && dcrc == 0) {
                        goto done;
                    } else {
                        result = MMC_ERR_BADCRC;
                    }      
                }
                cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
                cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

                
                orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
                orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
                orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
                orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
                orig_dat4 = (cur_rxdly1 >> 24) & 0x1F;
                orig_dat5 = (cur_rxdly1 >> 16) & 0x1F;
                orig_dat6 = (cur_rxdly1 >> 8) & 0x1F;
                orig_dat7 = (cur_rxdly1 >> 0) & 0x1F;
                
                if (ddr) {
                    cur_dat0 = (dcrc & (1 << 0) || dcrc & (1 <<  8)) ? ((orig_dat0 + 1) % 32) : orig_dat0;
                    cur_dat1 = (dcrc & (1 << 1) || dcrc & (1 <<  9)) ? ((orig_dat1 + 1) % 32) : orig_dat1;
                    cur_dat2 = (dcrc & (1 << 2) || dcrc & (1 << 10)) ? ((orig_dat2 + 1) % 32) : orig_dat2;
                    cur_dat3 = (dcrc & (1 << 3) || dcrc & (1 << 11)) ? ((orig_dat3 + 1) % 32) : orig_dat3;
					cur_dat4 = (dcrc & (1 << 4) || dcrc & (1 << 12)) ? ((orig_dat4 + 1) % 32) : orig_dat4;
        		    cur_dat5 = (dcrc & (1 << 5) || dcrc & (1 << 13)) ? ((orig_dat5 + 1) % 32) : orig_dat5;
            		cur_dat6 = (dcrc & (1 << 6) || dcrc & (1 << 14)) ? ((orig_dat6 + 1) % 32) : orig_dat6;
            		cur_dat7 = (dcrc & (1 << 7) || dcrc & (1 << 15)) ? ((orig_dat7 + 1) % 32) : orig_dat7;
                } else {
                    cur_dat0 = (dcrc & (1 << 0)) ? ((orig_dat0 + 1) % 32) : orig_dat0;
                    cur_dat1 = (dcrc & (1 << 1)) ? ((orig_dat1 + 1) % 32) : orig_dat1;
                    cur_dat2 = (dcrc & (1 << 2)) ? ((orig_dat2 + 1) % 32) : orig_dat2;
                    cur_dat3 = (dcrc & (1 << 3)) ? ((orig_dat3 + 1) % 32) : orig_dat3;
					cur_dat4 = (dcrc & (1 << 4)) ? ((orig_dat4 + 1) % 32) : orig_dat4;
                	cur_dat5 = (dcrc & (1 << 5)) ? ((orig_dat5 + 1) % 32) : orig_dat5;
                	cur_dat6 = (dcrc & (1 << 6)) ? ((orig_dat6 + 1) % 32) : orig_dat6;
                	cur_dat7 = (dcrc & (1 << 7)) ? ((orig_dat7 + 1) % 32) : orig_dat7;
                }
              
                cur_rxdly0 = ((cur_dat0 & 0x1F) << 24) | ((cur_dat1 & 0x1F) << 16) |
                    ((cur_dat2 & 0x1F)<< 8) | ((cur_dat3 & 0x1F) << 0);
                cur_rxdly1 = ((cur_dat4 & 0x1F) << 24) | ((cur_dat5 & 0x1F) << 16) |
                    ((cur_dat6 & 0x1F) << 8) | ((cur_dat7 & 0x1F) << 0);

                MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
                MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);            
            } while (++rxdly < 32);
			if(!sel)
				break;
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
done:

    return result;
}
#define READ_TUNING_MAX_HS (2 * 32)
#define READ_TUNING_MAX_UHS (2 * 32 * 32)
#define READ_TUNING_MAX_UHS_CLKMOD1 (2 * 32 * 32 *8)

int msdc_tune_read(struct mmc_host *host)
{
    u32 base = host->base;
    u32 dcrc, ddr = 0, sel = 0;
    u32 cur_rxdly0 = 0 , cur_rxdly1 = 0;
    u32 cur_dsmpl = 0, orig_dsmpl;
	u32 cur_dsel = 0,orig_dsel;
	u32 cur_dl_cksel = 0,orig_dl_cksel;
    u32 cur_dat0 = 0, cur_dat1 = 0, cur_dat2 = 0, cur_dat3 = 0, cur_dat4 = 0, cur_dat5 = 0, 
        cur_dat6 = 0, cur_dat7 = 0;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3, orig_dat4, orig_dat5, 
        orig_dat6, orig_dat7;
    u32 orig_clkmode;
    //u32 times = 0;
    int result = MMC_ERR_NONE;

    if (host->sclk > 100000000)
        sel = 1;
	if(host->card)
	    ddr = mmc_card_ddr(host->card);
	MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD,orig_clkmode);
	//if(orig_clkmode == 1)
		//MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, 0);
	
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orig_dsel);
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

                
                orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
                orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
                orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
                orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
                orig_dat4 = (cur_rxdly1 >> 24) & 0x1F;
                orig_dat5 = (cur_rxdly1 >> 16) & 0x1F;
                orig_dat6 = (cur_rxdly1 >> 8) & 0x1F;
                orig_dat7 = (cur_rxdly1 >> 0) & 0x1F;
                
                if (ddr) {
                    cur_dat0 = (dcrc & (1 << 0) || dcrc & (1 <<  8)) ? (orig_dat0 + 1) : orig_dat0;
                    cur_dat1 = (dcrc & (1 << 1) || dcrc & (1 <<  9)) ? (orig_dat1 + 1) : orig_dat1;
                    cur_dat2 = (dcrc & (1 << 2) || dcrc & (1 << 10)) ? (orig_dat2 + 1) : orig_dat2;
                    cur_dat3 = (dcrc & (1 << 3) || dcrc & (1 << 11)) ? (orig_dat3 + 1) : orig_dat3;					
					cur_dat4 = (dcrc & (1 << 4) || dcrc & (1 << 12)) ? (orig_dat4 + 1) : orig_dat4;
        		    cur_dat5 = (dcrc & (1 << 5) || dcrc & (1 << 13)) ? (orig_dat5 + 1) : orig_dat5;
            		cur_dat6 = (dcrc & (1 << 6) || dcrc & (1 << 14)) ? (orig_dat6 + 1) : orig_dat6;
            		cur_dat7 = (dcrc & (1 << 7) || dcrc & (1 << 15)) ? (orig_dat7 + 1) : orig_dat7;
                } else {
                    cur_dat0 = (dcrc & (1 << 0)) ? (orig_dat0 + 1) : orig_dat0;
                    cur_dat1 = (dcrc & (1 << 1)) ? (orig_dat1 + 1) : orig_dat1;
                    cur_dat2 = (dcrc & (1 << 2)) ? (orig_dat2 + 1) : orig_dat2;
                    cur_dat3 = (dcrc & (1 << 3)) ? (orig_dat3 + 1) : orig_dat3;
					cur_dat4 = (dcrc & (1 << 4)) ? (orig_dat4 + 1) : orig_dat4;
                	cur_dat5 = (dcrc & (1 << 5)) ? (orig_dat5 + 1) : orig_dat5;
                	cur_dat6 = (dcrc & (1 << 6)) ? (orig_dat6 + 1) : orig_dat6;
                	cur_dat7 = (dcrc & (1 << 7)) ? (orig_dat7 + 1) : orig_dat7;
                }
                

                cur_rxdly0 = ((cur_dat0 & 0x1F) << 24) | ((cur_dat1 & 0x1F) << 16) |
                    ((cur_dat2 & 0x1F) << 8) | ((cur_dat3 & 0x1F) << 0);
                cur_rxdly1 = ((cur_dat4 & 0x1F) << 24) | ((cur_dat5 & 0x1F)<< 16) |
                    ((cur_dat6 & 0x1F) << 8) | ((cur_dat7 & 0x1F) << 0);

                MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
                MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1); 
				
            }
	  	if(cur_dat0 >= 32 || cur_dat1 >= 32 || cur_dat2 >= 32 || cur_dat3 >= 32 ||
		   cur_dat4 >= 32 || cur_dat5 >= 32 || cur_dat6 >= 32 || cur_dat7 >= 32){
			if(sel){
				cur_dsel = (orig_dsel + 1);
           		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, cur_dsel % 32);

				}			
		}
		if(cur_dsel >= 32){
			if(orig_clkmode == 1 && sel){	
				cur_dl_cksel = (orig_dl_cksel + 1);
            	MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel % 8);
			}               
		}
		++(host->time_read);
		if((sel == 1 && orig_clkmode == 1 && host->time_read == READ_TUNING_MAX_UHS_CLKMOD1)||
           (sel == 1 && orig_clkmode != 1 && host->time_read == READ_TUNING_MAX_UHS)||
           (sel == 0 && orig_clkmode != 1 && host->time_read == READ_TUNING_MAX_HS)){
			result = MMC_ERR_READTUNEFAIL;
			}
    return result;
}
#endif

#ifdef FEATURE_MMC_WR_TUNING
int msdc_tune_bwrite(struct mmc_host *host, u32 dst, u8 *src, u32 nblks)
{
    u32 base = host->base;
    u32 orig_clkmode;
    u32 sel = 0;
    u32 wrrdly, cur_wrrdly, orig_wrrdly;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
	u32 d_cntr,orig_d_cntr,cur_d_cntr;
    u32 rxdly, cur_rxdly0;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3;
	
    u32 times = 0;
    //u32 status;
    int result = MMC_ERR_WRITETUNEFAIL;  
	

    if (host->sclk > 100000000)
        sel = 1;
		
	MSDC_GET_FIELD(MSDC_CFG,MSDC_CFG_CKMOD,orig_clkmode);
		//if(orig_clkmode == 1)
			//MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, 0);
		

    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orig_wrrdly);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_D_SMPL, orig_dsmpl);
    //MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, orig_ddrdly);
    //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, orig_cksel);
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, orig_d_cntr);
    //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

    /* Tune Method 2. delay data0 line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

    cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);


    orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
    orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
    orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
    orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;

            d_cntr = 0;
            do {
                rxdly = 0;
                do {
                    wrrdly = 0;
                    do {
                        for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                            cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_D_SMPL, cur_dsmpl);
							result = host->blk_write(host, dst, src, nblks);
							if(result == MMC_ERR_CMDTUNEFAIL || result == MMC_ERR_CMD_RSPCRC || result == MMC_ERR_ACMD_RSPCRC)
									goto done;

                            /* for debugging */
							 #if MSDC_TUNE_LOG
                            {
                                u32 t_dspl, t_wrrdly, t_d_cntr;// t_dl_cksel;

                                MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, t_wrrdly);
                                MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_D_SMPL, t_dspl);
                                //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_RX_SDCLKO_SEL, t_cksel);
								MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, t_d_cntr);
                                //MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                                times++;

                                printf("[SD%d] <TUNE_BWRITE_%d><%s> DSPL=%d, DATWRDLY=%d\n",
                                    host->id, times, result == MMC_ERR_NONE ? "PASS" : "FAIL", 
                                    t_dspl, t_wrrdly);
								printf("[SD%d] <TUNE_BWRITE_%d><%s> MSDC_DAT_RDDLY0=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    MSDC_READ32(MSDC_DAT_RDDLY0));
                               /* printf("[SD%d] <TUNE_BWRITE_%d><%s> DDR50CKD=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    t_ddrdly);*/
                                printf("[SD%d] <TUNE_BWRITE_%d><%s> MSDC_PATCH_BIT1_WRDAT_CRCS=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    t_d_cntr);
							    //printf("[SD%d] <TUNE_BWRITE_%d><%s> MSDC_CKGEN_RX_SDCLKO_SEL=%xh\n",
                                    //host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                   // t_cksel);
                                
                            }
							 #endif
                            if (result == MMC_ERR_NONE) {
                                goto done;
                            }
                        }
                        cur_wrrdly = ++orig_wrrdly % 32;
                        MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, cur_wrrdly);
                    } while (++wrrdly < 32);
                    
                    cur_dat0 = ++orig_dat0 % 32; /* only adjust bit-1 for crc */
                    cur_dat1 = orig_dat1;
                    cur_dat2 = orig_dat2;
                    cur_dat3 = orig_dat3;

                    cur_rxdly0 = (cur_dat0 << 24) | (cur_dat1 << 16) |
                        (cur_dat2 << 8) | (cur_dat3 << 0);
                    
                    MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
                } while (++rxdly < 32);

                /* no need to update data ck sel */
                if (!sel)
                    break;

                cur_d_cntr= (orig_d_cntr + d_cntr +1 )% 8;
                MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, cur_d_cntr);
                d_cntr++;                
            } while (d_cntr < 8);
done:

    return result;
}
#endif

#ifdef FEATURE_MMC_UHS1
int msdc_tune_uhs1(struct mmc_host *host, struct mmc_card *card)
{
    u32 base = host->base;
    u32 status;
    int i;
    int err = MMC_ERR_FAILED;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SEND_TUNING_BLOCK;
    cmd.arg     = 0;
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
            MSG(ERR, "[SD%d] ACMD19_TRG(%d), STS(0x%x) Failed\n", host->id, i,
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

int msdc_card_avail(struct mmc_host *host)
{
    u32 base = host->base;
    unsigned int sts, avail = 0;

    if ((msdc_cap.flags & MSDC_REMOVABLE) == 0)
        return 1;

    if (msdc_cap.flags & MSDC_CD_PIN_EN) {
        MSDC_GET_FIELD(MSDC_PS, MSDC_PS_CDSTS, sts);
        avail = sts == 0 ? 1 : 0;        
    }

    return avail;
}

#if 1
int msdc_card_protected(struct mmc_host *host)
{
    u32 base = host->base;
    unsigned int prot;

    if (msdc_cap.flags & MSDC_WP_PIN_EN) {
        MSDC_GET_FIELD(MSDC_PS, MSDC_PS_WP, prot);
    } else {
        prot = 0;
    }

    return prot;
}

void msdc_hard_reset(void)
{
    msdc_set_card_pwr(0);
    msdc_set_card_pwr(1);
}
#endif

#ifdef FEATURE_MMC_BOOT_MODE
int msdc_emmc_boot_start(struct mmc_host *host, u32 hz, int ddr, int mode, int ackdis)
{
    int err = MMC_ERR_NONE;
    u32 sts;
    u32 base = host->base;
    u32 tmo = 0xFFFFFFFF;
    u32 acktmo, dattmo;

    /* configure msdc and clock frequency */    
    MSDC_RESET();
    MSDC_CLR_FIFO();    
    msdc_set_blklen(host, 512);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_config_clock(host, ddr, hz);

    /* requires 74 clocks/1ms before CMD0 */
    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);
    mdelay(2);
    MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);

    /* configure boot timeout value */
    WAIT_COND(SDC_IS_BUSY() == 0, tmo, tmo);

    acktmo = msdc_cal_timeout(host, 50 * 1000 * 1000, 0, 256);   /* 50ms */
    dattmo = msdc_cal_timeout(host, 1000 * 1000 * 1000, 0, 256); /* 1sec */

    acktmo = acktmo > 0xFFF ? 0xFFF : acktmo;
    dattmo = dattmo > 0xFFFFF ? 0xFFFFF : dattmo;

    printf("[SD%d] EMMC BOOT ACK timeout: %d ms (clkcnt: %d)\n", host->id, 
        (acktmo * 256) / (host->sclk / 1000), acktmo);
    printf("[SD%d] EMMC BOOT DAT timeout: %d ms (clkcnt: %d)\n", host->id,
        (dattmo * 256) / (host->sclk / 1000), dattmo);
    
    MSDC_SET_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSUPP);
    MSDC_SET_FIELD(EMMC_CFG0, EMMC_CFG0_BOOTACKDIS, ackdis);
    MSDC_SET_FIELD(EMMC_CFG0, EMMC_CFG0_BOOTMODE, mode);
    MSDC_SET_FIELD(EMMC_CFG1, EMMC_CFG1_BOOTACKTMC, acktmo);
    MSDC_SET_FIELD(EMMC_CFG1, EMMC_CFG1_BOOTDATTMC, dattmo);

    if (mode == EMMC_BOOT_RST_CMD_MODE) {
        MSDC_WRITE32(SDC_ARG, 0xFFFFFFFA);
    } else {
        MSDC_WRITE32(SDC_ARG, 0);
    }
    MSDC_WRITE32(SDC_CMD, 0x02001000); /* multiple block read */
    MSDC_SET_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSTART);

    WAIT_COND((MSDC_READ32(EMMC_STS) & EMMC_STS_BOOTUPSTATE) == 
        EMMC_STS_BOOTUPSTATE, tmo, tmo);

    if (!ackdis) {
        do {
            sts = MSDC_READ32(EMMC_STS);
            if (sts == 0)
                continue;
            MSDC_WRITE32(EMMC_STS, sts);    /* write 1 to clear */
            if (sts & EMMC_STS_BOOTACKRCV) {
                printf("[SD%d] EMMC_STS(%x): boot ack received\n", host->id, sts);
                break;
            } else if (sts & EMMC_STS_BOOTACKERR) {
                printf("[SD%d] EMMC_STS(%x): boot up ack error\n", host->id, sts);
                err = MMC_ERR_BADCRC;
                goto out;
            } else if (sts & EMMC_STS_BOOTACKTMO) {
                printf("[SD%d] EMMC_STS(%x): boot up ack timeout\n", host->id, sts);
                err = MMC_ERR_TIMEOUT;
                goto out;
            } else if (sts & EMMC_STS_BOOTUPSTATE) {
                printf("[SD%d] EMMC_STS(%x): boot up mode state\n", host->id, sts);
            } else {
                printf("[SD%d] EMMC_STS(%x): boot up unexpected\n", host->id, sts);
            }
        } while (1);
    }

    /* check if data received */
    do {
        sts = MSDC_READ32(EMMC_STS);
        if (sts == 0)
            continue;
        if (sts & EMMC_STS_BOOTDATRCV) {
            printf("[SD%d] EMMC_STS(%x): boot dat received\n", host->id, sts);
            break;
        }
        if (sts & EMMC_STS_BOOTCRCERR) {
            err = MMC_ERR_BADCRC;
            goto out;
        } else if (sts & EMMC_STS_BOOTDATTMO) {
            err = MMC_ERR_TIMEOUT;
            goto out;
        }
    } while(1);
out:
    return err;    
}

void msdc_emmc_boot_stop(struct mmc_host *host, int mode)
{
    u32 base = host->base;
    u32 tmo = 0xFFFFFFFF;

    /* Step5. stop the boot mode */
    MSDC_WRITE32(SDC_ARG, 0x00000000);
    MSDC_WRITE32(SDC_CMD, 0x00001000);

    MSDC_SET_FIELD(EMMC_CFG0, EMMC_CFG0_BOOTWDLY, 2);
    MSDC_SET_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSTOP);
    WAIT_COND((MSDC_READ32(EMMC_STS) & EMMC_STS_BOOTUPSTATE) == 0, tmo, tmo);
    
    /* Step6. */
    MSDC_CLR_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSUPP);

    /* Step7. clear EMMC_STS bits */
    MSDC_WRITE32(EMMC_STS, MSDC_READ32(EMMC_STS));
}

int msdc_emmc_boot_read(struct mmc_host *host, u32 size, u32 *to)
{
    int err = MMC_ERR_NONE;
    u32 sts;
    u32 totalsz = size;
    u32 base = host->base;
  
    while (size) {
        sts = MSDC_READ32(EMMC_STS);
        if (sts & EMMC_STS_BOOTCRCERR) {
            err = MMC_ERR_BADCRC;
            goto out;
        } else if (sts & EMMC_STS_BOOTDATTMO) {
            err = MMC_ERR_TIMEOUT;
            goto out;
        }
        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD)) {
            int left = MSDC_FIFO_THD >> 2;
            do {
                *to++ = MSDC_FIFO_READ32();
            } while (--left);
            size -= MSDC_FIFO_THD;
            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(), size, totalsz);
        } else if ((size < MSDC_FIFO_THD) && MSDC_RXFIFOCNT() >= size) {
            while (size) {
                if (size > 3) {
                    *to++ = MSDC_FIFO_READ32();
                    size -= 4;
                } else {
                    u32 val = MSDC_FIFO_READ32();
                    memcpy(to, &val, size);
                    size = 0;
                }
            }
            MSG(FIO, "[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
                host->id, MSDC_RXFIFOCNT(), size, totalsz);
        }        
    }

out:
    if (err) {
        printf("[SD%d] EMMC_BOOT: read boot code fail(%d), FIFOCNT=%d\n", 
            host->id, err, MSDC_RXFIFOCNT());
    }
    return err;
}

void msdc_emmc_boot_reset(struct mmc_host *host, int reset)
{
    u32 base = host->base;

    switch (reset) {
    case EMMC_BOOT_PWR_RESET:
        msdc_hard_reset();
        break;
    case EMMC_BOOT_RST_N_SIG:
        /* set n_reset pin to low */
        MSDC_SET_BIT32(EMMC_IOCON, EMMC_IOCON_BOOTRST);

        /* tRSTW (RST_n pulse width) at least 1us */
        mdelay(1);

        /* set n_reset pin to high */
        MSDC_CLR_BIT32(EMMC_IOCON, EMMC_IOCON_BOOTRST);
        MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);
        /* tRSCA (RST_n to command time) at least 200us, 
           tRSTH (RST_n high period) at least 1us */
        mdelay(1);
        MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);

        break;
    case EMMC_BOOT_PRE_IDLE_CMD:
        /* bring emmc to pre-idle mode by software reset command. (MMCv4.41)*/
        SDC_SEND_CMD(0x0, 0xF0F0F0F0);   
        break;
    }
}
#endif

int msdc_init(struct mmc_host *host, int id)
{
    u32 baddr[] = {MSDC0_BASE, MSDC1_BASE, MSDC2_BASE, MSDC3_BASE};
    u32 base = baddr[id];
    gpd_t *gpd;
    bd_t *bd;
    msdc_priv_t *priv;
    struct dma_config *cfg;
    clk_source_t clksrc;

    printf("[%s]: start\n", __func__);
    clksrc = msdc_cap.clk_src;

#if MSDC_USE_DMA_MODE
    gpd  = &msdc_gpd_pool[id][0];
    bd   = &msdc_bd_pool[id][0];
#endif    
    priv = &msdc_priv[id];
    cfg  = &priv->cfg;

#if MSDC_DEBUG
    msdc_reg[id] = (struct msdc_regs*)base;
#endif

#if MSDC_USE_DMA_MODE
    memset(gpd, 0, sizeof(gpd_t) * MAX_GPD_POOL_SZ);
    memset(bd, 0, sizeof(bd_t) * MAX_BD_POOL_SZ);
#endif
    memset(priv, 0, sizeof(msdc_priv_t));

    host->id     = id;
    host->base   = base;
    host->f_max  = MSDC_MAX_SCLK;
    host->f_min  = MSDC_MIN_SCLK;
    host->blkbits= MMC_BLOCK_BITS;
    host->blklen = 0;
    host->priv   = (void*)priv;
        
    host->caps   = MMC_CAP_MULTIWRITE;

    if (msdc_cap.flags & MSDC_HIGHSPEED)
        host->caps |= (MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED);
#ifdef FEATURE_MMC_UHS1
    if (msdc_cap.flags & MSDC_UHS1)
        host->caps |= MMC_CAP_SD_UHS1;
#endif
    if (msdc_cap.flags & MSDC_DDR)
        host->caps |= MMC_CAP_DDR;
    if (msdc_cap.data_pins == 4)
        host->caps |= MMC_CAP_4_BIT_DATA;
    if (msdc_cap.data_pins == 8)
        host->caps |= MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;

    host->ocr_avail = MSDC_VDD_32_33;  /* TODO: To be customized */

    host->max_hw_segs   = MAX_DMA_TRAN_SIZE/512;
    host->max_phys_segs = MAX_DMA_TRAN_SIZE/512;
    host->max_seg_size  = MAX_DMA_TRAN_SIZE;
    host->max_blk_size  = 2048;
    host->max_blk_count = 65535;
	host->app_cmd = 0;
	host->app_cmd_arg = 0;
#if MSDC_USE_DMA_MODE
    host->blk_read  = msdc_dma_bread;
    host->blk_write = msdc_dma_bwrite;
    cfg->mode       = MSDC_MODE_DMA_DESC;
#else
    host->blk_read  = msdc_pio_bread;
    host->blk_write = msdc_pio_bwrite;
    cfg->mode       = MSDC_MODE_PIO;
#endif

#if MSDC_USE_DMA_MODE
    priv->alloc_bd    = 0;
    priv->alloc_gpd   = 0;
    priv->bd_pool     = bd;
    priv->gpd_pool    = gpd;
    priv->active_head = NULL;
    priv->active_tail = NULL;
#endif 
    priv->dsmpl       = msdc_cap.data_edge;
    priv->rsmpl       = msdc_cap.cmd_edge;

#if MSDC_USE_DMA_MODE
    cfg->sg      = &priv->sg[0];
    cfg->burstsz = MSDC_BRUST_64B;
    cfg->flags   = DMA_FLAG_NONE;
#endif
#ifdef FPGA_PLATFORM
	MSDC_WRITE32(PWR_GPIO_EO,0xF00); //setup GPIO mode (GPO or GPI)
	printf("set up GPIO for MSDC\n");
#endif
    msdc_power(host, MMC_POWER_OFF);
    msdc_power(host, MMC_POWER_ON);

    /* set to SD/MMC mode */
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_MODE, MSDC_SDMMC);
    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO);

    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_CLR_INT();

    /* reset tuning parameter */
    //MSDC_WRITE32(MSDC_PAD_CTL0, 0x0098000);
    //MSDC_WRITE32(MSDC_PAD_CTL1, 0x00A0000);
    //MSDC_WRITE32(MSDC_PAD_CTL2, 0x00A0000);
    MSDC_WRITE32(MSDC_PAD_TUNE, 0x0000000);
    MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x00000000);
    MSDC_WRITE32(MSDC_DAT_RDDLY1, 0x00000000);
    MSDC_WRITE32(MSDC_IOCON, 0x00000000);
    //MSDC_WRITE32(MSDC_PATCH_BIT0, 0x003C004F);
	MSDC_WRITE32(MSDC_PATCH_BIT1, 0xFFFF00C9);//High 16 bit = 0 mean Power KPI is on, enable msdc ECO for write timeout issue.
					//MSDC_PATCH_BIT1YD:WRDAT_CRCS_TA_CNTR need fix to 3'001 by default,(<50MHz) (>=50MHz set 3'001 as initial value is OK for tunning)
					//YD:CMD_RSP_TA_CNTR need fix to 3'001 by default(<50MHz)(>=50MHz set 3'001as initial value is OK for tunning)					

    /* 2012-01-07 using internal clock instead of feedback clock */
    //MSDC_SET_BIT32(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL);

    /* enable SDIO mode. it's must otherwise sdio command failed */
    MSDC_SET_BIT32(SDC_CFG, SDC_CFG_SDIO);

    /* enable wake up events */
    //MSDC_SET_BIT32(SDC_CFG, SDC_CFG_INSWKUP);

    /* eneable SMT for glitch filter */
	#ifdef FPGA_PLATFORM
    MSDC_SET_BIT32(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKSMT);
    MSDC_SET_BIT32(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDSMT);
    MSDC_SET_BIT32(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATSMT);
	#else
	msdc_set_smt(host,1);
	#endif
    /* set clk, cmd, dat pad driving */
    msdc_set_driving(host, msdc_cap.clk_drv, msdc_cap.cmd_drv, msdc_cap.dat_drv);

    /* set sampling edge */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, msdc_cap.cmd_edge);
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, msdc_cap.data_edge);

    /* write crc timeout detection */
    MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 30, 1);

    msdc_config_clksrc(host, clksrc);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_config_clock(host, 0, MSDC_MIN_SCLK);
    /* disable sdio interrupt by default. sdio interrupt enable upon request */
   // msdc_intr_unmask(host, 0x0001FF7B);
  	msdc_intr_mask(host,0xffffffff);
	mt_irq_set_sens(MT_MSDC0_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
	mt_irq_set_polarity(MT_MSDC0_IRQ_ID, MT65xx_POLARITY_LOW);
	event_init(&msdc_int_event,false,EVENT_FLAG_AUTOUNSIGNAL);
	mt_irq_unmask(MT_MSDC0_IRQ_ID);
    msdc_set_timeout(host, 100000000, 0);
    msdc_card_detect(host, 1);

    printf("[%s]: finish succesfully\n", __func__);
    return 0;
}

int msdc_deinit(struct mmc_host *host)
{
    u32 base = host->base;

    msdc_card_detect(host, 0);
    msdc_intr_mask(host, 0x0001FFFB);
    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_CLR_INT();
    msdc_power(host, MMC_POWER_OFF);

    return 0;
}

