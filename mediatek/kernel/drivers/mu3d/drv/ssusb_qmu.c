#ifdef USE_SSUSB_QMU
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>

#include "musb_core.h"
#include <linux/mu3d/drv/SSUSB_DEV_c_header.h>
#include <linux/mu3d/hal/mu3d_hal_osal.h>
#include <linux/mu3d/hal/mu3d_hal_qmu_drv.h>
#include "ssusb_qmu.h"

#ifdef NEVER
/*
 * 1. Configure the HW register (starting address)  with gpd heads (starting address)  for each Queue
 * 2. Enable Queue with each EP and interrupts
*/
void usb_initialize_qmu() {
	mu3d_hal_init_qmu();
}

/*
 * txstate_qmu() & rxstate_qmu() should not be called.
 * musb_g_tx() wont call txstate_qmu(), because this flow is PIO mode.
 * and there is no function calling rxstate_qmu().
 */

/*
  put usb_request buffer into GPD/BD data structures, and ask QMU to start TX.

  caller: musb_g_tx

*/

void txstate_qmu(struct musb *musb, struct musb_request *req)
{
	u8			epnum = req->epnum;
	struct musb_ep		*musb_ep;
	struct usb_request	*request;
	u32 txcsr0 = 0;

	musb_ep = &musb->endpoints[epnum].ep_in;
	request = &req->request;

	txcsr0 = os_readl(musb->endpoints[epnum].addr_txcsr0);

	if (txcsr0 & TX_SENDSTALL) {
		os_printk(K_DEBUG, "%s stalling, txcsr %03x\n",
				musb_ep->end_point.name, txcsr0);
		return;
	}
//	request->actual = request->length;
//	mu3d_hal_insert_transfer_gpd(epnum, USB_TX, (DEV_UINT8 *)request->dma, request->length, true,true,false,(musb_ep->type==USB_ENDPOINT_XFER_ISOC?0:1), musb_ep->packet_sz);
//	mu3d_hal_resume_qmu(epnum, USB_TX);
	os_printk(K_DEBUG,"tx start...length : %d\r\n",request->length);

}


/* the caller ensures that req is not NULL. */

void rxstate_qmu(struct musb *musb, struct musb_request *req)
{
	u8			epnum = req->epnum;
	struct musb_ep		*musb_ep = &musb->endpoints[epnum].ep_out;
	struct usb_request	*request;
	u32 rxcsr0 = os_readl(musb->endpoints[epnum].addr_rxcsr0);

	request = &req->request;

	if(rxcsr0 & RX_SENDSTALL) {
		os_printk(K_DEBUG, "%s stalling, RXCSR %04x\n",
			musb_ep->end_point.name, rxcsr0);
		return;
	}

	os_printk(K_DEBUG, "rxstate_qmu 0\n");
	mu3d_hal_insert_transfer_gpd(epnum, USB_RX, (DEV_UINT8*)request->dma, request->length, true,true,false,(musb_ep->type==USB_ENDPOINT_XFER_ISOC?0:1), musb_ep->packet_sz);
	mu3d_hal_resume_qmu(epnum, USB_RX);
	os_printk(K_DEBUG,"rx start\r\n");

}
#endif /* NEVER */

/*
    1. Find the last gpd HW has executed and update Tx_gpd_last[]
    2. Set the flag for txstate to know that TX has been completed

    ported from proc_qmu_tx() from test driver.

    caller:qmu_interrupt after getting QMU done interrupt and TX is raised

*/

void qmu_tx_interrupt(struct musb *musb, u8 ep_num)
{
	TGPD* gpd = Tx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(os_readl(USB_QMU_TQCPR(ep_num)));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_in;
	struct usb_request	*request = NULL;
	struct musb_request	*req;
	unsigned long flags = 0;

	//trying to give_back the request to gadget driver.
	req = next_request(musb_ep);
	if (!req) {
		os_printk(K_DEBUG, "%s [ERROR] Cannot get next request of %d, \
			but QMU has done.\n", __func__, ep_num);
		return;
	} else {
		request = &req->request;
	}

	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = gpd_phys_to_virt(gpd_current, USB_TX, ep_num);

	/*
                      gpd or Last       gdp_current
                           |                  |
            |->  GPD1 --> GPD2 --> GPD3 --> GPD4 --> GPD5 -|
            |----------------------------------------------|
	*/

	os_printk(K_DEBUG, "%s ep_num=%d, Tx_gpd_last=%p, gpd_current=%p, Tx_gpd_end=%p\n",
		__func__, ep_num, gpd, gpd_current, Tx_gpd_end[ep_num]);

	/*gpd_current should at least point to the next GPD to the previous last one.*/
	if (gpd == gpd_current) {
		os_printk(K_DEBUG, "%s [ERROR] gpd(%p) == gpd_current(%p)\n", __func__, gpd, \
			gpd_current);
		return;
	}

	spin_unlock_irqrestore(&musb->lock, flags);

	//flush data from device to CPU
	dma_sync_single_for_cpu(musb->controller, \
				mu3d_hal_gpd_virt_to_phys(gpd, USB_TX, ep_num), \
				sizeof(TGPD), DMA_BIDIRECTIONAL);

	spin_lock_irqsave(&musb->lock, flags);

	while (gpd!=gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {
		os_printk(K_DEBUG, "Tx gpd=%p ->HWO=%d, BPD=%d, Next_GPD=%x, DataBuffer=%x, \
			BufferLen=%d, Endpoint=%d\n", gpd, (DEV_UINT32)TGPD_GET_FLAG(gpd), \
			(DEV_UINT32)TGPD_GET_FORMAT(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd), \
			(DEV_UINT32)TGPD_GET_DATA(gpd), (DEV_UINT32)TGPD_GET_BUF_LEN(gpd), \
			(DEV_UINT32)TGPD_GET_EPaddr(gpd));

		gpd = TGPD_GET_NEXT(gpd);

		spin_unlock_irqrestore(&musb->lock, flags);

		/*flush data from device to CPU*/
		dma_sync_single_for_cpu(musb->controller, \
				(dma_addr_t)gpd, sizeof(TGPD), DMA_BIDIRECTIONAL);

		spin_lock_irqsave(&musb->lock, flags);

		gpd = gpd_phys_to_virt(gpd,USB_TX,ep_num);

		Tx_gpd_last[ep_num] = gpd;
		musb_g_giveback(musb_ep, request, 0);
		req = next_request(musb_ep);
		request = &req->request;
	}

	os_printk(K_DEBUG,"%s Tx-EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Tx_gpd_last[ep_num], Tx_gpd_end[ep_num]);
}

/*
   When receiving RXQ done interrupt, qmu_interrupt calls this function.

   1. Traverse GPD/BD data structures to count actual transferred length.
   2. Set the done flag to notify rxstate_qmu() to report status to upper gadget driver.

    ported from proc_qmu_rx() from test driver.

    caller:qmu_interrupt after getting QMU done interrupt and TX is raised

*/
void qmu_rx_interrupt(struct musb *musb, u8 ep_num)
{
	DEV_UINT32 buf_len = 0, received_len = 0;
	TGPD* gpd = (TGPD*)Rx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(os_readl(USB_QMU_RQCPR(ep_num)));
	TBD* bd;
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_out;
	struct usb_request	*request = NULL;
	struct musb_request	*req;
	unsigned long flags = 0;

	//trying to give_back the request to gadget driver.
	req = next_request(musb_ep);
	if (!req) {
		os_printk(K_DEBUG, "%s [ERROR] Cannot get next request of %d, \
			but QMU has done.\n", __func__, ep_num);
		return;
	} else {
		request = &req->request;
	}

	os_printk(K_INFO, "phys_to_virt=%x-->%x\n", gpd_current, phys_to_virt(gpd_current));

	gpd_current = gpd_phys_to_virt(gpd_current, USB_RX, ep_num);

	os_printk(K_INFO,"%s ep_num=%d, Rx_gpd_last=%p, gpd_current=%p, Rx_gpd_end=%p\n",
		__func__, ep_num, gpd, gpd_current, Rx_gpd_end[ep_num]);

	if (gpd==gpd_current) {
		os_printk(K_DEBUG, "%s [ERROR] gpd(%p) == gpd_current(%p)\n", __func__, gpd, \
			gpd_current);
		return;
	}

	spin_unlock_irqrestore(&musb->lock, flags);

	//invalidate cache in CPU
	dma_sync_single_for_cpu(musb->controller, \
				mu3d_hal_gpd_virt_to_phys(gpd, USB_RX, ep_num), \
				sizeof(TGPD), DMA_BIDIRECTIONAL);

	spin_lock_irqsave(&musb->lock, flags);

	if(!gpd || !gpd_current) {
		os_printk(K_ERR, "%s [ERROR] ep_num:%d, gpd=%p, gpd_current=%p, ishwo=%d, \
			rx_gpd_last=%p, RQCPR=0x%x\n", __func__,
			ep_num, gpd, gpd_current, ((gpd==NULL) ? 999 : TGPD_IS_FLAGS_HWO(gpd)),
			Rx_gpd_last[ep_num], os_readl(USB_QMU_RQCPR(ep_num)));
		return;
	}

	while(gpd != gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {

		if(TGPD_IS_FORMAT_BDP(gpd)) {

			bd = (TBD *)TGPD_GET_DATA(gpd);

			spin_unlock_irqrestore(&musb->lock, flags);

			//flush data from device to CPU
			dma_sync_single_for_cpu(musb->controller, \
					(dma_addr_t)bd, sizeof(TBD), DMA_BIDIRECTIONAL);

			spin_lock_irqsave(&musb->lock, flags);

			bd = (TBD *)bd_phys_to_virt(bd,USB_RX,ep_num);

			while(1) {
				os_printk(K_INFO,"%s BD=%p, len=%x\n", __func__, bd, (DEV_UINT32)TBD_GET_BUF_LEN(bd));

				request->actual += TBD_GET_BUF_LEN(bd);

				if(TBD_IS_FLAGS_EOL(bd)) {
					os_printk(K_INFO,"%s Total Len=0x%x\n", __func__, request->actual);
					break;
				}
				bd = TBD_GET_NEXT(bd);

				spin_unlock_irqrestore(&musb->lock, flags);

				//flush data from device to CPU
				dma_sync_single_for_cpu(musb->controller, \
						(dma_addr_t)bd, sizeof(TBD), DMA_BIDIRECTIONAL);

				spin_lock_irqsave(&musb->lock, flags);

				bd = bd_phys_to_virt(bd, USB_RX, ep_num);
			}
		}	else {
			received_len = (DEV_UINT32)TGPD_GET_BUF_LEN(gpd);
			buf_len  = (DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd);
			if(received_len > buf_len)
				os_printk(K_ERR, "%s [ERROR] received(%d) > buf(%d) AUK!?\n", __func__, received_len, buf_len);

			request->actual += received_len;
		}

		//os_printk(K_INFO,"Rx gpd info { HWO %d, Next_GPD %x ,DataBufferLength %d, DataBuffer %x, Recived Len %d, Endpoint %d, TGL %d, ZLP %d}\r\n",
		//		(DEV_UINT32)TGPD_GET_FLAG(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd),
		//		(DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd), (DEV_UINT32)TGPD_GET_DATA(gpd),
		//		(DEV_UINT32)TGPD_GET_BUF_LEN(gpd), (DEV_UINT32)TGPD_GET_EPaddr(gpd),
		//		(DEV_UINT32)TGPD_GET_TGL(gpd), (DEV_UINT32)TGPD_GET_ZLP(gpd));


		if(!TGPD_GET_NEXT(gpd) || !TGPD_GET_DATA(gpd)) {
			os_printk(K_ERR, "%s [ERROR] ep_num:%d ,gpd=%p\n", __func__, ep_num, gpd);
			BUG_ON(1);
		}

		gpd = TGPD_GET_NEXT(gpd);

		spin_unlock_irqrestore(&musb->lock, flags);

		//invalidate next GPD data in CPU
		dma_sync_single_for_cpu(musb->controller, (dma_addr_t)gpd, sizeof(TGPD), DMA_BIDIRECTIONAL);

		spin_lock_irqsave(&musb->lock, flags);

		gpd = gpd_phys_to_virt(gpd, USB_RX, ep_num);

		if(!gpd) {
			os_printk(K_ERR, "%s [ERROR] ep_num:%d ,gpd=%p\n", __func__, ep_num, gpd);
			BUG_ON(1);
		}

		//spin_lock_irqsave(&musb->lock, flags);

		Rx_gpd_last[ep_num]=gpd;
		musb_g_giveback(musb_ep, request, 0);
		req = next_request(musb_ep);
		request = &req->request;

		//spin_unlock_irqrestore(&musb->lock, flags);

	}

	os_printk(K_DEBUG,"%s Rx-EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Rx_gpd_last[ep_num], Rx_gpd_end[ep_num]);
}

//void qmu_done_interrupt(unsigned long pparam)
void qmu_done_interrupt(struct musb *musb, DEV_UINT32 wQmuVal)
{
	//struct musb *musb;
	//unsigned int wQmuVal;
	unsigned int i;
	//unsigned long flags;

	//musb = ((QMU_DONE_ISR_DATA *)pparam)->musb;
	//wQmuVal = ((QMU_DONE_ISR_DATA *)pparam)->wQmuVal;

	os_printk(K_DEBUG, "%s %x\n", __func__, wQmuVal);

	for(i=1; i<=MAX_QMU_EP; i++) {
		if(wQmuVal & QMU_RX_DONE(i)) {
			qmu_rx_interrupt(musb, i);
		}
		if(wQmuVal & QMU_TX_DONE(i)) {
			qmu_tx_interrupt(musb, i);
		}
	}

	//spin_lock_irqsave(&musb->lock, flags);
	//((QMU_DONE_ISR_DATA *)pparam)->wQmuVal = 0;
	//os_printk(K_DEBUG, "%s clear QMU Done interrupt %x\n", __func__, ((QMU_DONE_ISR_DATA *)pparam)->wQmuVal);
	//spin_unlock_irqrestore(&musb->lock, flags);
}

void qmu_exception_interrupt(struct musb *musb, DEV_UINT32 wQmuVal)
{
    u32 wErrVal;
	int i = (int)wQmuVal;

	if(wQmuVal & RXQ_CSERR_INT)
		os_printk(K_ERR, "==Rx %d checksum error==\n", i);

	if(wQmuVal & RXQ_LENERR_INT)
		os_printk(K_ERR, "==Rx %d length error==\n", i);

	if(wQmuVal & TXQ_CSERR_INT)
		os_printk(K_ERR, "==Tx %d checksum error==\n", i);

	if(wQmuVal & TXQ_LENERR_INT)
		os_printk(K_ERR, "==Tx %d length error==\n", i);

    if((wQmuVal & RXQ_CSERR_INT)||(wQmuVal & RXQ_LENERR_INT)) {
     	wErrVal=os_readl(U3D_RQERRIR0);
     	os_printk(K_DEBUG, "Rx Queue error in QMU mode![0x%x]\r\n", (unsigned int)wErrVal);
     	for(i=1; i<=MAX_QMU_EP; i++) {
     		if(wErrVal &QMU_RX_CS_ERR(i))
     			os_printk(K_DEBUG, "Rx %d length error!\r\n", i);

     		if(wErrVal &QMU_RX_LEN_ERR(i))
     			os_printk(K_DEBUG, "Rx %d recieve length error!\r\n", i);
     	}
     	os_writel(U3D_RQERRIR0, wErrVal);
    }

    if(wQmuVal & RXQ_ZLPERR_INT) {
		wErrVal=os_readl(U3D_RQERRIR1);
		os_printk(K_DEBUG, "Rx Queue error in QMU mode![0x%x]\r\n", (unsigned int)wErrVal);

     	for(i=1; i<=MAX_QMU_EP; i++) {
     		if(wErrVal &QMU_RX_ZLP_ERR(i)) {
     			os_printk(K_DEBUG, "Rx %d recieve an zlp packet!\r\n", i);
     		}
     	}
     	os_writel(U3D_RQERRIR1, wErrVal);
    }

    if((wQmuVal & TXQ_CSERR_INT)||(wQmuVal & TXQ_LENERR_INT)) {

 		wErrVal=os_readl(U3D_TQERRIR0);
 		os_printk(K_DEBUG, "Tx Queue error in QMU mode![0x%x]\r\n", (unsigned int)wErrVal);

		for(i=1; i<=MAX_QMU_EP; i++) {
 			if(wErrVal &QMU_TX_CS_ERR(i))
 				os_printk(K_DEBUG, "Tx %d checksum error!\r\n", i);

 			if(wErrVal &QMU_TX_LEN_ERR(i))
 				os_printk(K_DEBUG, "Tx %d buffer length error!\r\n", i);
 		}
 		os_writel(U3D_TQERRIR0, wErrVal);
    }

	if((wQmuVal & RXQ_EMPTY_INT)||(wQmuVal & TXQ_EMPTY_INT)) {
 		DEV_UINT32 wEmptyVal = os_readl(U3D_QEMIR);
 		os_printk(K_DEBUG, "Rx Empty in QMU mode![0x%x]\r\n", wEmptyVal);
 		os_writel(U3D_QEMIR, wEmptyVal);
	}

}

#endif //USE_SSUSB_QMU
