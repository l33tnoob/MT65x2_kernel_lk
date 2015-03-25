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

#if defined(MMC_MSDC_DRV_LK)
#include <kernel/event.h>
#include <platform/mt_irq.h>

static event_t msdc_int_event;
static u32 g_int_status = 0;

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
#endif

#if defined(MSDC_USE_IRQ)
#include "isrentry.h"
//For CTP only
typedef void (*irq_handler_t)(void);
static const u32 msdc_irq_line[MSDC_MAX_NUM] = {MSDC0_IRQ_ID, MSDC1_IRQ_ID};
static void *msdc_irq_data[MSDC_MAX_NUM];
volatile u32 msdc_irq_sts[MSDC_MAX_NUM];

#define DECLARE_MSDC_IRQ_HANDLER(x) \
static void msdc_irq_handler_##x(void) \
{ \
    msdc_irq_handler(msdc_irq_data[(x)]); \
}

#define REGISTER_MSDC_IRQ_HANDLER(id, hndl, data) \
do { \
    msdc_irq_data[(id)] = (void*)(data); \
    IRQSensitivity(msdc_irq_line[(id)], LEVEL_SENSITIVE); \
    IRQPolarity(msdc_irq_line[(id)], LOW_LEVEL_TRIGGER); \
    IRQ_Register_LISR(msdc_irq_line[(id)], hndl, NULL); \
    IRQUnmask(msdc_irq_line[(id)]); \
} while(0)

#define UNREGISTER_MSDC_IRQ_HANDLER(id) \
do { \
    IRQMask(msdc_irq_line[(id)]); \
    msdc_irq_data[(id)] = NULL; \
    IRQ_Register_LISR(msdc_irq_line[(id)], IRQ_Default_LISR, NULL); \
} while(0)

#if defined(FEATURE_MMC_SDIO)
static hw_irq_handler_t sdio_irq_handler[MSDC_MAX_NUM];

void msdc_register_hwirq(struct mmc_host *host, hw_irq_handler_t handler)
{
    sdio_irq_handler[host->id] = handler;
}
#endif

int in_iqr = 0;
static void msdc_irq_handler(void *data)
{
    struct mmc_host *host = (struct mmc_host*)data;
    u32 id = host->id;
    u32 base = host->base;
    u32 intsts;
    msdc_int_reg *int_reg = (msdc_int_reg*)&intsts;

    BUG_ON(base == 0);

    //IRQMask(msdc_irq_line[id]);
    DisableIRQ();

    intsts = MSDC_READ32(MSDC_INT);

    msdc_irq_sts[id] |= intsts;

    if (intsts & MSDC_INT_CDSC) {
    	/* card detection */
    	in_iqr++;
    	printf("in_irq(%d) intsts(0x%x)",in_iqr,intsts);
    	printf("\n[SD%d] Card %s\n", id, msdc_card_avail(host) ? "Inserted" : "Removed");
    }

    if (intsts & MSDC_INT_SDIOIRQ) {
    	/* sdio bus interrupt */
    	MSG(INT, "[SD%d] SDIO interrupt <===\n", id);
    	/* Note. msdc detects logical-low of dat1 to trigger sdio interrupt.
    	 * It will be triggered and should be _IGNORED_ since DAT1 of
    	 * SDIO/MMC/SD/SDXC card is pulled low when it's inserted into the slot.
    	 */
#if defined(FEATURE_MMC_SDIO)
    	if (sdio_irq_handler[id]) {
    	    sdio_irq_handler[id]();
    
    	    /* clear it since it's already handled */
    	    msdc_irq_sts[id] &= ~MSDC_INT_SDIOIRQ;
    	}
#endif
    }

    if (intsts & (MSDC_INT_CMDRDY|MSDC_INT_CMDTMO|MSDC_INT_RSPCRCERR)) {
	    /* command done */
    }

    if (intsts & (MSDC_INT_ACMDRDY|MSDC_INT_ACMDTMO|MSDC_INT_ACMDCRCERR)) {
	    /* auto command done */
    }

    if (intsts & MSDC_INT_ACMD19_DONE) {
	    /* auto command 10 done */
    }

    if (intsts & MSDC_INT_XFER_COMPL) {
	    /* data transfer done */
    }

    if (intsts & (MSDC_INT_DATCRCERR|MSDC_INT_DATTMO)) {
	    /* data error */
    }

    if (intsts & MSDC_INT_DXFER_DONE) {
	    /* dma transfer done */
    }

    if (intsts & MSDC_INT_DMAQ_EMPTY) {
	    /* DMA queue empty */
    }

    if (intsts & MSDC_INT_CSTA) {
	    /* CSTA available */
    }

    if (intsts & MSDC_INT_MMCIRQ) {
	    /* MMCIRQ available */
    }

    MSG(INT, "[SD%d] IRQ_EVT(0x%x): MMCIRQ(%d) CDSC(%d), ACRDY(%d), ACTMO(%d), ACCRE(%d) AC19DN(%d)\n",
    	id,
    	intsts,
    	int_reg->mmcirq,
    	int_reg->cdsc,
    	int_reg->atocmdrdy,
    	int_reg->atocmdtmo,
    	int_reg->atocmdcrc,
    	int_reg->atocmd19done);

    MSG(INT, "[SD%d] IRQ_EVT(0x%x): SDIO(%d) CMDRDY(%d), CMDTMO(%d), RSPCRC(%d), CSTA(%d)\n",
    	id,
    	intsts,
    	int_reg->sdioirq,
    	int_reg->cmdrdy,
    	int_reg->cmdtmo,
    	int_reg->rspcrc,
    	int_reg->csta);

    MSG(INT, "[SD%d] IRQ_EVT(0x%x): XFCMP(%d) DXDONE(%d), DATTMO(%d), DATCRC(%d), DMAEMP(%d)\n",
    	id,
    	intsts,
    	int_reg->xfercomp,
    	int_reg->dxferdone,
    	int_reg->dattmo,
    	int_reg->datcrc,
    	int_reg->dmaqempty);

    MSDC_WRITE32(MSDC_INT, intsts); /* clear interrupts */

    //IRQUnmask(msdc_irq_line[id]);
    EnableIRQ();
}

DECLARE_MSDC_IRQ_HANDLER(0);
DECLARE_MSDC_IRQ_HANDLER(1);

static irq_handler_t isr[] = {
        msdc_irq_handler_0,
        msdc_irq_handler_1
    };
#endif

void msdc_irq_init(struct mmc_host *host)
{
#if defined(MMC_MSDC_DRV_LK)
    
    msdc_intr_mask(host,0xffffffff);

    if ( host->id==0 ) {
        mt_irq_set_sens(MT_MSDC0_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
        mt_irq_set_polarity(MT_MSDC0_IRQ_ID, MT65xx_POLARITY_LOW);
    }
    #if defined(FEATURE_MMC_MEM_PRESERVE_MODE)
    if ( host->id==1 ) {
	    mt_irq_set_sens(MT_MSDC1_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
        mt_irq_set_polarity(MT_MSDC1_IRQ_ID, MT65xx_POLARITY_LOW);
    }
    #endif

	event_init(&msdc_int_event,false,EVENT_FLAG_AUTOUNSIGNAL);

    if ( host->id==0 )
	    mt_irq_unmask(MT_MSDC0_IRQ_ID);
    #if defined(FEATURE_MMC_MEM_PRESERVE_MODE)
    if ( host->id==1 )
        mt_irq_unmask(MT_MSDC1_IRQ_ID);
    #endif
#endif

#if defined(MSDC_USE_IRQ)
    //For CTP only
	msdc_irq_sts[host->id]  = 0;
	#if defined(FEATURE_MMC_SDIO)
    sdio_irq_handler[id] = NULL;
    #endif
	REGISTER_MSDC_IRQ_HANDLER(host->id, isr[host->id], host);
#endif
}

void msdc_irq_deinit(struct mmc_host *host)
{
    #if defined(MSDC_USE_IRQ)
    //UNREGISTER_MSDC_IRQ_HANDLER(host->id); //Light: Comment it out to solve build error in CTP
    #endif
}

#if defined(MMC_MSDC_DRV_LK)
u32 msdc_lk_intr_wait(struct mmc_host *host, u32 intrs)
{
    u32 base = host->base;
    u32 sts;

	MSG(INT, "[msdc_intr_wait]\n");
    /* warning that interrupts are not enabled */
    WARN_ON((MSDC_READ32(MSDC_INTEN) & intrs) != intrs);
	event_wait(&msdc_int_event);
	MSG(INT, "[msdc_intr_wait]get event\n");
    sts = g_int_status;
    g_int_status = 0;
    MSG(INT, "[SD%d] INT(0x%x)\n", host->id, sts);
    if (~intrs & sts) {
        MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
            host->id, ~intrs & sts);
    }
    return sts;
}
#endif

u32 msdc_intr_wait(struct mmc_host *host, u32 intrs)
{
    u32 base = host->base;
    u32 sts;

    /* warning that interrupts are not enabled */
    WARN_ON((MSDC_READ32(MSDC_INTEN) & intrs) != intrs);

#if defined(MSDC_USE_IRQ)
    //For CTP only
    while (1) {
        DisableIRQ();
        if (msdc_irq_sts[host->id] & intrs) {
            sts = msdc_irq_sts[host->id];
            msdc_irq_sts[host->id] &= ~intrs;
            EnableIRQ();
            break;
        }
        EnableIRQ();
    }
#else
    {
        //Light: WAIT_COND is added after E1 ECO workaround.
        //       I combine WAIT_COND with infinite wait
        u32 tmo = 500000; //set tmo=0 to choose infinite wait
        WAIT_COND( ((sts = MSDC_READ32(MSDC_INT)) & intrs), tmo, tmo);

        if (tmo == 0) {
            //msdc_dump_register(host);
            //printf(" timeout cmd is %d\n", cmd->opcode);
            //for (i = 0; i <= 25; i++){
            //    *(u32*)(base + 0xa0) = i;
            //    printf("[SD%d] Reg[a0] SW_DBG_SEL      = 0x%x\n", host->id,*(u32*)(base + 0xa0));
            //    printf("[SD%d] Reg[a4] SW_DBG_OUT      = 0x%x\n", host->id,*(u32*)(base + 0xa4));
            //}

            //while(1);
            printf("[SD%d] ECO WARNNING ==> Wait INT timeout\n", host->id);
            MSDC_RESET();
        }
    }

    MSDC_WRITE32(MSDC_INT, (sts & intrs));
#endif

    MSG(INT, "[SD%d] INT(0x%x)\n", host->id, sts);

    if (~intrs & sts) {
        MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n",
            host->id, ~intrs & sts);
    }

    return sts;
}

