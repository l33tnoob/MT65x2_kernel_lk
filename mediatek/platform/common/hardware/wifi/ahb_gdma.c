/******************************************************************************
*[File]             ahb_gdma.c
*[Version]          v1.0
*[Revision Date]    2013-01-16
*[Author]
*[Description]
*    The program provides AHB GDMA driver
*[Copyright]
*    Copyright (C) 2013 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/



/*
** $Log: ahb_gdma.c $
 *
 * 01 16 2013 vend_samp.lin
 * Add AHB GDMA support
 * 1) Initial version
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#define MODULE_AHB_DMA

#include <linux/version.h>      /* constant of kernel version */

#include <linux/kernel.h>       /* bitops.h */

#include <linux/timer.h>        /* struct timer_list */
#include <linux/jiffies.h>      /* jiffies */
#include <linux/delay.h>        /* udelay and mdelay macro */

#if CONFIG_ANDROID
#include <linux/wakelock.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12)
#include <linux/irq.h>          /* IRQT_FALLING */
#endif

#include <linux/netdevice.h>    /* struct net_device, struct net_device_stats */
#include <linux/etherdevice.h>  /* for eth_type_trans() function */
#include <linux/wireless.h>     /* struct iw_statistics */
#include <linux/if_arp.h>
#include <linux/inetdevice.h>   /* struct in_device */

#include <linux/ip.h>           /* struct iphdr */

#include <linux/string.h>       /* for memcpy()/memset() function */
#include <linux/stddef.h>       /* for offsetof() macro */

#include <linux/proc_fs.h>      /* The proc filesystem constants/structures */

#include <linux/rtnetlink.h>    /* for rtnl_lock() and rtnl_unlock() */
#include <linux/kthread.h>      /* kthread_should_stop(), kthread_run() */
#include <asm/uaccess.h>        /* for copy_from_user() */
#include <linux/fs.h>           /* for firmware download */
#include <linux/vmalloc.h>

#include <linux/kfifo.h>        /* for kfifo interface */
#include <linux/cdev.h>         /* for cdev interface */

#include <linux/firmware.h>     /* for firmware download */

#include <linux/random.h>


#include <asm/io.h>             /* readw and writew */

#include <linux/module.h>

#include "../../../../../../platform/mt6572/kernel/core/include/mach/mt_clkmgr.h"

#include "hif.h"
#include "hif_gdma.h"

#if (CONF_MTK_AHB_DMA == 1)

//#define GDMA_DEBUG_SUP

#ifdef GDMA_DEBUG_SUP
#define GDMA_DBG(msg)   printk msg
#else
#define GDMA_DBG(msg)
#endif /* GDMA_DEBUG_SUP */


/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/


/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static VOID
HifGdmaConfig (
    IN void                     *HifInfoSrc,
    IN void                     *Conf
    );

static VOID
HifGdmaStart(
    IN void                     *HifInfoSrc
    );

static VOID
HifGdmaStop(
    IN void                     *HifInfoSrc
    );

static MTK_WCN_BOOL
HifGdmaPollStart(
    IN void                     *HifInfoSrc
    );

static MTK_WCN_BOOL
HifGdmaPollIntr(
    IN void                     *HifInfoSrc
    );

static VOID
HifGdmaAckIntr(
    IN void                     *HifInfoSrc
    );

static VOID
HifGdmaClockCtrl(
    IN UINT32                   FlgIsEnabled
    );

static VOID
HifGdmaRegDump(
    IN void                     *HifInfoSrc
    );


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/


/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
GL_HIF_DMA_OPS_T HifGdmaOps = {
    .DmaConfig = HifGdmaConfig,
    .DmaStart = HifGdmaStart,
    .DmaStop = HifGdmaStop,
    .DmaPollStart = HifGdmaPollStart,
    .DmaPollIntr = HifGdmaPollIntr,
    .DmaAckIntr = HifGdmaAckIntr,
    .DmaClockCtrl = HifGdmaClockCtrl,
    .DmaRegDump = HifGdmaRegDump
};


/*******************************************************************************
*                        P U B L I C   F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* \brief Config GDMA TX/RX.
*
* \param[in] DmaRegBaseAddr     Pointer to the IO register base.
* \param[in] Conf               Pointer to the DMA operator.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
VOID
HifGdmaInit (
    GL_HIF_INFO_T               *HifInfo
    )
{
    /* IO remap GDMA register memory */
    HifInfo->DmaRegBaseAddr = ioremap(AP_DMA_HIF_BASE, AP_DMA_HIF_0_LENGTH);

    /* assign GDMA operators */
    HifInfo->DmaOps = &HifGdmaOps;

    /* enable GDMA mode */
    HifInfo->fgDmaEnable = TRUE;

    GDMA_DBG(("GDMA> HifGdmaInit ok!\n"));
}


/*******************************************************************************
*                       P R I V A T E   F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* \brief Config GDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
* \param[in] Param              Pointer to the settings.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifGdmaConfig (
    IN void                     *HifInfoSrc,
    IN void                     *Param
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    MTK_WCN_HIF_DMA_CONF *Conf = (MTK_WCN_HIF_DMA_CONF *)Param;
    UINT32 RegVal;
    

    /* Assign fixed value */
    Conf->Ratio = HIF_GDMA_RATIO_1;
    Conf->Connect = HIF_GDMA_CONNECT_SET1;
    Conf->Wsize = HIF_GDMA_WRITE_2;
    Conf->Burst = HIF_GDMA_BURST_4_8;
    Conf->Fix_en = TRUE;


    /* AP_P_DMA_G_DMA_2_CON */
    GDMA_DBG(("GDMA> Conf->Dir = %d\n", Conf->Dir));

    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_CON);
    RegVal &= ~(ADH_CR_FLAG_FINISH | ADH_CR_RSIZE | ADH_CR_WSIZE | \
            ADH_CR_BURST_LEN | ADH_CR_WADDR_FIX_EN | ADH_CR_RADDR_FIX_EN);
    if (Conf->Dir == HIF_DMA_DIR_TX)
    {
        RegVal |= (((Conf->Wsize<<ADH_CR_WSIZE_OFFSET)&ADH_CR_WSIZE) | \
                ((Conf->Burst<<ADH_CR_BURST_LEN_OFFSET)&ADH_CR_BURST_LEN) | \
                ((Conf->Fix_en<<ADH_CR_WADDR_FIX_EN_OFFSET)&ADH_CR_WADDR_FIX_EN));
    }
    else
    {
        RegVal |= (((Conf->Wsize<<ADH_CR_RSIZE_OFFSET)&ADH_CR_RSIZE) | \
                ((Conf->Burst<<ADH_CR_BURST_LEN_OFFSET)&ADH_CR_BURST_LEN) | \
                ((Conf->Fix_en<<ADH_CR_RADDR_FIX_EN_OFFSET)&ADH_CR_RADDR_FIX_EN));
    }
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_CON, RegVal);
    GDMA_DBG(("GDMA> AP_P_DMA_G_DMA_2_CON = 0x%08x\n", RegVal));

    /* AP_P_DMA_G_DMA_2_CONNECT */
    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_CONNECT);
    RegVal &= ~(ADH_CR_RATIO | ADH_CR_DIR | ADH_CR_CONNECT);
    RegVal |= (((Conf->Ratio<<ADH_CR_RATIO_OFFSET)&ADH_CR_RATIO) | \
            ((Conf->Dir<<ADH_CR_DIR_OFFSET)&ADH_CR_DIR) | \
            (Conf->Connect&ADH_CR_CONNECT));
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_CONNECT, RegVal);
    GDMA_DBG(("GDMA> AP_P_DMA_G_DMA_2_CONNECT = 0x%08x\n", RegVal));

    /* AP_DMA_HIF_0_SRC_ADDR */
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_SRC_ADDR, Conf->Src);
    GDMA_DBG(("GDMA> AP_P_DMA_G_DMA_2_SRC_ADDR = 0x%08x\n",  Conf->Src));

    /* AP_DMA_HIF_0_DST_ADDR */
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_DST_ADDR, Conf->Dst);
    GDMA_DBG(("GDMA> AP_P_DMA_G_DMA_2_DST_ADDR = 0x%08x\n",  Conf->Dst));

    /* AP_P_DMA_G_DMA_2_LEN1 */
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_LEN1, (Conf->Count & ADH_CR_LEN));
    GDMA_DBG(("GDMA> AP_P_DMA_G_DMA_2_LEN1 = %ld\n",  (Conf->Count & ADH_CR_LEN)));

}/* End of HifGdmaConfig */


/*----------------------------------------------------------------------------*/
/*!
* \brief Start GDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifGdmaStart(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 RegVal;


    /* Enable interrupt */
    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_INT_EN);
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_INT_EN, (RegVal | ADH_CR_INTEN_FLAG_0));


    /* Start DMA */
    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_EN);
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_EN, (RegVal | ADH_CR_EN | ADH_CR_CONN_BUR_EN));

    GDMA_DBG(("GDMA> HifGdmaStart...\n"));

} /* End of HifGdmaStart */


/*----------------------------------------------------------------------------*/
/*!
* \brief Stop GDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifGdmaStop(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 RegVal;
//    UINT32 pollcnt;


    /* Disable interrupt */
    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_INT_EN);
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_INT_EN, (RegVal & ~(ADH_CR_INTEN_FLAG_0)));


#if 0 /* DE says we donot need to do it */
    /* Stop DMA */
    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_STOP);
    HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_STOP, (RegVal | ADH_CR_STOP));


    /* Polling START bit turn to 0 */
    pollcnt = 0;
    do {
        RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_EN);
        if (pollcnt++ > 100000) {
            /* TODO: warm reset GDMA */
        }
    } while(RegVal&ADH_CR_EN);
#endif

} /* End of HifGdmaStop */


/*----------------------------------------------------------------------------*/
/*!
* \brief Enable GDMA TX/RX.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static MTK_WCN_BOOL
HifGdmaPollStart(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
	UINT32 RegVal;


    RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_EN);
	return (((RegVal & ADH_CR_EN) != 0) ? TRUE : FALSE);

} /* End of HifGdmaPollStart */


/*----------------------------------------------------------------------------*/
/*!
* \brief Poll GDMA TX/RX done.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static MTK_WCN_BOOL
HifGdmaPollIntr(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
	UINT32 RegVal;


	RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_INT_FLAG);
	return (((RegVal & ADH_CR_FLAG_0) != 0) ? TRUE : FALSE);

} /* End of HifGdmaPollIntr */


/*----------------------------------------------------------------------------*/
/*!
* \brief Acknowledge GDMA TX/RX done.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifGdmaAckIntr(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
	UINT32 RegVal;


	/* Write 0 to clear interrupt */
	RegVal = HIF_DMAR_READL(HifInfo, AP_P_DMA_G_DMA_2_INT_FLAG);
	HIF_DMAR_WRITEL(HifInfo, AP_P_DMA_G_DMA_2_INT_FLAG, (RegVal & ~ADH_CR_FLAG_0));

} /* End of HifGdmaAckIntr */


/*----------------------------------------------------------------------------*/
/*!
* \brief Acknowledge GDMA TX/RX done.
*
* \param[in] FlgIsEnabled       TRUE: enable; FALSE: disable
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifGdmaClockCtrl(
    IN UINT32                   FlgIsEnabled
    )
{
#ifdef MT_CG_APDMA_SW_CG
    if (FlgIsEnabled == TRUE)
        enable_clock(MT_CG_APDMA_SW_CG, "WLAN");
    else
        disable_clock(MT_CG_APDMA_SW_CG, "WLAN");
#endif /* MT_CG_APDMA_SW_CG */
}


/*----------------------------------------------------------------------------*/
/*!
* \brief Dump GDMA related registers.
*
* \param[in] HifInfo            Pointer to the GL_HIF_INFO_T structure.
*
* \retval NONE
*/
/*----------------------------------------------------------------------------*/
static VOID
HifGdmaRegDump(
    IN void                     *HifInfoSrc
    )
{
    GL_HIF_INFO_T *HifInfo = (GL_HIF_INFO_T *)HifInfoSrc;
    UINT32 RegId, RegVal;
    UINT32 RegNum = 0;


    printk("PDMA> Register content 0x%x=\n\t", AP_DMA_HIF_BASE);
    for(RegId=0; RegId<AP_DMA_HIF_0_LENGTH; RegId+=4)
    {
        RegVal = HIF_DMAR_READL(HifInfo, RegId);
        printk("0x%08x ", RegVal);

        if (RegNum++ >= 3)
        {
            printk("\n");
            printk("PDMA> Register content 0x%x=\n\t", AP_DMA_HIF_BASE+RegId+4);
            RegNum = 0;
        }
    }

    printk("\nGDMA> clock status = 0x%x\n\n", *(volatile unsigned int *)0xF0000024);
}

#endif /* CONF_MTK_AHB_DMA */

/* End of ahb_gdma.c */
