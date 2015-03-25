
#include <linux/mu3d/hal/mu3d_hal_osal.h>
#define _MTK_QMU_DRV_EXT_
#include <linux/mu3d/hal/mu3d_hal_qmu_drv.h>
#undef _MTK_QMU_DRV_EXT_
#include <linux/mu3d/hal/mu3d_hal_usb_drv.h>
#include <linux/mu3d/hal/mu3d_hal_hw.h>



/**
 * get_bd - get a null bd
 * @args - arg1: dir, arg2: ep number
 */
PBD get_bd(USB_DIR dir,DEV_UINT32 num)
{
	PBD ptr;

	if (dir == USB_RX) {
		ptr = (PBD)Rx_bd_List[num].pNext;

		os_printk(K_DEBUG,"%s Rx_bd_List[%d].pNext=%p\n", __func__, num, (Rx_bd_List[num].pNext));

		if( (Rx_bd_List[num].pNext+1) < Rx_bd_List[num].pEnd)
			Rx_bd_List[num].pNext++;
		else
			Rx_bd_List[num].pNext = Rx_bd_List[num].pStart;

	} else {
		ptr = (PBD)Tx_bd_List[num].pNext;

		os_printk(K_DEBUG,"%s Tx_gpd_List[%d].pNext=%p\n", __func__, num, (Tx_bd_List[num].pNext));

		Tx_bd_List[num].pNext++;
		Tx_bd_List[num].pNext = Tx_bd_List[num].pNext + AT_BD_EXT_LEN;

		if( Tx_bd_List[num].pNext >= Tx_bd_List[num].pEnd) {
			Tx_bd_List[num].pNext = Tx_bd_List[num].pStart;
		}
	}
	return ptr;
}

/**
 * get_bd - get a null gpd
 * @args - arg1: dir, arg2: ep number
 */
PGPD get_gpd(USB_DIR dir,DEV_UINT32 num)
{
	PGPD ptr;

	if( dir == USB_RX) {
		ptr = Rx_gpd_List[num].pNext;
		//os_printk(K_DEBUG,"(Rx_gpd_List[%d].pNext : 0x%08X\n", num,(DEV_UINT32)(Rx_gpd_List[num].pNext));
		if ( (Rx_gpd_List[num].pNext+1) < Rx_gpd_List[num].pEnd ) {
			Rx_gpd_List[num].pNext++;
		} else {
			Rx_gpd_List[num].pNext = Rx_gpd_List[num].pStart;
		}
	} else {
		ptr = Tx_gpd_List[num].pNext;
		//os_printk(K_DEBUG,"(Tx_gpd_List[%d].pNext : 0x%08X\n", num,(DEV_UINT32)(Tx_gpd_List[num].pNext));
		Tx_gpd_List[num].pNext++;
		Tx_gpd_List[num].pNext = Tx_gpd_List[num].pNext + AT_GPD_EXT_LEN;

		if ( Tx_gpd_List[num].pNext >= Tx_gpd_List[num].pEnd ) {
			Tx_gpd_List[num].pNext = Tx_gpd_List[num].pStart;
		}
	}
	return ptr;
}

/**
 * get_bd - align gpd ptr to target ptr
 * @args - arg1: dir, arg2: ep number, arg3: target ptr
 */
void gpd_ptr_align(USB_DIR dir,DEV_UINT32 num,PGPD ptr)
{
 	DEV_UINT32 run_next;
	run_next =true;

	//os_printk(K_DEBUG,"gpd_ptr_align %d, num=0x%x, ptr=%p\n", dir, num, ptr);

	while(run_next)
	{
	 	if(ptr==get_gpd(dir,num)){
			run_next=false;
	 	}
	}
}

/**
 * bd_virt_to_phys - map bd virtual address to physical address
 * @args - arg1: virtual address, arg2: dir, arg3: ep number
 * @return - physical address
 */
dma_addr_t bd_virt_to_phys(void *vaddr,USB_DIR dir,DEV_UINT32 num)
{
	DEV_UINT32 *ptr;

	if ( dir == USB_RX ) {
		ptr = (DEV_UINT32 *)((DEV_UINT32)vaddr - Rx_bd_Offset[num]);
	} else {
		ptr = (DEV_UINT32 *)((DEV_UINT32)vaddr - Tx_bd_Offset[num]);
	}
	/*os_printk(K_DEBUG,"%s %s[%d]phys=%p<->virt=%p\n", __func__, \
		((dir==USB_RX)?"RX":"TX"), num , ptr, vaddr);*/

	return (dma_addr_t)ptr;
}

/**
 * bd_phys_to_virt - map bd physical address to virtual address
 * @args - arg1: physical address, arg2: dir, arg3: ep number
 * @return - virtual address
 */
void *bd_phys_to_virt(void *paddr,USB_DIR dir,DEV_UINT32 num)
{
	DEV_UINT32 * ptr;

	os_printk(K_DEBUG,"bd_phys_to_virt paddr=%p, num=%d\n", paddr, num);

	if (dir == USB_RX) {
		ptr = (DEV_UINT32 *)((DEV_UINT32)paddr + Rx_bd_Offset[num]);
	} else {
		ptr = (DEV_UINT32 *)((DEV_UINT32)paddr + Tx_bd_Offset[num]);
	}
	/*os_printk(K_DEBUG,"%s %s[%d]phys=%p<->virt=%p\n", __func__, \
		((dir==USB_RX)?"RX":"TX"), num , paddr, ptr);*/

	return ptr;
}

/**
 * mu3d_hal_gpd_virt_to_phys - map gpd virtual address to physical address
 * @args - arg1: virtual address, arg2: dir, arg3: ep number
 * @return - physical address
 */
dma_addr_t mu3d_hal_gpd_virt_to_phys(void *vaddr,USB_DIR dir,DEV_UINT32 num)
{
	DEV_UINT32 *ptr;

	if (dir == USB_RX) {
		ptr = (DEV_UINT32 *)((DEV_UINT32)vaddr-Rx_gpd_Offset[num]);
	} else {
		ptr = (DEV_UINT32 *)((DEV_UINT32)vaddr-Tx_gpd_Offset[num]);
	}
	/*os_printk(K_DEBUG,"%s %s[%d]phys=%p<->virt=%p\n", __func__, \
		((dir==USB_RX)?"RX":"TX"), num , ptr, vaddr);*/

	return (dma_addr_t)ptr;
}

/**
 * gpd_phys_to_virt - map gpd physical address to virtual address
 * @args - arg1: physical address, arg2: dir, arg3: ep number
 * @return - virtual address
 */
void *gpd_phys_to_virt(void *paddr, USB_DIR dir, DEV_UINT32 num)
{
	DEV_UINT32 *ptr;

	os_printk(K_DEBUG,"%s paddr=%p, num=%d\n", __func__, paddr, num);

	if (dir == USB_RX) {
		os_printk(K_DEBUG, "%s Rx_gpd_Offset[%d]=0x%08X\n", __func__, num, \
							Rx_gpd_Offset[num]);
		ptr = (DEV_UINT32 *)((DEV_UINT32)paddr+Rx_gpd_Offset[num]);
	} else {
		os_printk(K_DEBUG,"%s Tx_gpd_Offset[%d]=0x%08X\n", __func__, num, \
							Tx_gpd_Offset[num]);
		ptr = (DEV_UINT32 *)((DEV_UINT32)paddr+Tx_gpd_Offset[num]);
	}
	/*os_printk(K_DEBUG,"%s %s[%d]phys=%p<->virt=%p\n", __func__, \
		((dir==USB_RX)?"RX":"TX"), num , paddr, ptr);*/

	return ptr;
}

/**
 * init_bd_list - initialize bd management list
 * @args - arg1: dir, arg2: ep number, arg3: bd virtual addr, arg4: bd ioremap addr, arg5: bd number
 */
void init_bd_list(USB_DIR dir, int num, PBD ptr, PBD io_ptr, DEV_UINT32 size)
{
	if (dir == USB_RX) {
		Rx_bd_List[num].pStart = ptr;
		Rx_bd_List[num].pEnd = (PBD)(ptr + size);
		Rx_bd_Offset[num]=(DEV_UINT32)ptr - (DEV_UINT32)io_ptr; //os_virt_to_phys(ptr);
		ptr++;
		Rx_bd_List[num].pNext = ptr;
		os_printk(K_DEBUG,"Rx_bd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n", \
			num, Rx_bd_List[num].pStart, Rx_bd_List[num].pNext, Rx_bd_List[num].pEnd);
		os_printk(K_DEBUG,"Rx_bd_Offset[%d]=0x%08X\n", num, Rx_bd_Offset[num]);
		os_printk(K_DEBUG,"virtual addr=%p\n", ptr);
		os_printk(K_DEBUG,"DMA addr ptr=%p, end=%p\n", io_ptr, io_ptr+size);
	} else {
		Tx_bd_List[num].pStart = ptr;
	 	Tx_bd_List[num].pEnd = (PBD)((DEV_UINT8*)(ptr + size) + AT_BD_EXT_LEN*size);
		Tx_bd_Offset[num] = (DEV_UINT32)ptr - (DEV_UINT32)io_ptr; //os_virt_to_phys(ptr);
		ptr++;
	 	Tx_bd_List[num].pNext = (PBD)((DEV_UINT8*)ptr + AT_BD_EXT_LEN);
		os_printk(K_DEBUG,"Tx_bd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n", \
			num,Tx_bd_List[num].pStart, Tx_bd_List[num].pNext, Tx_bd_List[num].pEnd);
		os_printk(K_DEBUG,"Tx_bd_Offset[%d]=0x%08X\n", num, Tx_bd_Offset[num]);
		os_printk(K_DEBUG,"virtual addr=%p\n", ptr);
		os_printk(K_DEBUG,"DMA addr ptr=%p, end=%p\n", io_ptr, io_ptr+size);
	}
}


/**
 * init_gpd_list - initialize gpd management list
 * @args - arg1: dir, arg2: ep number, arg3: gpd virtual addr, arg4: gpd ioremap addr, arg5: gpd number
 */
void init_gpd_list(USB_DIR dir,int num,PGPD ptr,PGPD io_ptr,DEV_UINT32 size)
{
	if (dir == USB_RX) {
		Rx_gpd_List[num].pStart = ptr;
		Rx_gpd_List[num].pEnd = (PGPD)(ptr + size);
		Rx_gpd_Offset[num]=(DEV_UINT32)ptr - (DEV_UINT32)io_ptr; //(DEV_UINT32)os_virt_to_phys(ptr);
		ptr++;
		Rx_gpd_List[num].pNext = ptr;
		os_printk(K_DEBUG,"Rx_gpd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n", \
			num, Rx_gpd_List[num].pStart, Rx_gpd_List[num].pNext, Rx_gpd_List[num].pEnd);
		os_printk(K_DEBUG,"Rx_gpd_Offset[%d]=0x%08X\n", num, Rx_gpd_Offset[num]);
		os_printk(K_DEBUG,"virtual start=%p, end=%p\n", ptr, ptr+size);
		os_printk(K_DEBUG,"dma addr start=%p, end=%p\n", io_ptr, io_ptr+size);
	} else {
		Tx_gpd_List[num].pStart = ptr;
	 	Tx_gpd_List[num].pEnd = (PGPD)((DEV_UINT8*)(ptr + size) + AT_GPD_EXT_LEN*size);
		Tx_gpd_Offset[num] = (DEV_UINT32)ptr - (DEV_UINT32)io_ptr; //(DEV_UINT32)os_virt_to_phys(ptr);
		ptr++;
	 	Tx_gpd_List[num].pNext = (PGPD)((DEV_UINT8*)ptr + AT_GPD_EXT_LEN);
		os_printk(K_DEBUG,"Tx_gpd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n",
			num, Tx_gpd_List[num].pStart, Tx_gpd_List[num].pNext, Tx_gpd_List[num].pEnd);
		os_printk(K_DEBUG,"Tx_gpd_Offset[%d]=0x%08X\n", num, Tx_gpd_Offset[num]);
		os_printk(K_DEBUG,"virtual start=%p, end=%p\n", ptr, ptr+size);
		os_printk(K_DEBUG,"dma addr start=%p, end=%p\n", io_ptr, io_ptr+size);
	}
}

/**
 * free_gpd - free gpd management list
 * @args - arg1: dir, arg2: ep number
 */
void free_gpd(USB_DIR dir,int num)
{
	if (dir == USB_RX) {
		os_memset(Rx_gpd_List[num].pStart, 0, MAX_GPD_NUM*sizeof(TGPD));
	} else {
		os_memset(Tx_gpd_List[num].pStart, 0, MAX_GPD_NUM*sizeof(TGPD));
	}
}

/**
 * mu3d_hal_alloc_qmu_mem - allocate gpd and bd memory for all ep
 *
 */
void mu3d_hal_alloc_qmu_mem(void){
	DEV_UINT32 i, size;
	TGPD *ptr,*io_ptr;
	TBD *bptr,*io_bptr;

	for(i=1; i<=MAX_QMU_EP; i++){
		/* Allocate Tx GPD */
		size = sizeof(TGPD);
		size *=MAX_GPD_NUM;
		ptr = (TGPD*)os_mem_alloc(size);
		os_memset(ptr, 0 , size);

		io_ptr = (TGPD *)dma_map_single(NULL, ptr, size, DMA_TO_DEVICE);
		//io_ptr = (TGPD*)os_ioremap(os_virt_to_phys(ptr),size);
		//io_ptr = (TGPD*)(os_virt_to_phys(ptr));
		//init_gpd_list(USB_RX,i,ptr,io_ptr,MAX_GPD_NUM);
		init_gpd_list( USB_RX, i, ptr, io_ptr, MAX_GPD_NUM);
		Rx_gpd_end[i]= ptr;
		os_printk(K_DEBUG,"ALLOC RX GPD End [%d] Virtual Mem=%p, DMA addr=%p\n", i, Rx_gpd_end[i], io_ptr);
		//os_memset(Rx_gpd_end[i], 0 , sizeof(TGPD));
		TGPD_CLR_FLAGS_HWO(Rx_gpd_end[i]);
		Rx_gpd_head[i]=Rx_gpd_last[i]=Rx_gpd_end[i];
		os_printk(K_DEBUG,"RQSAR[%d]=%p\n", i, (void *)mu3d_hal_gpd_virt_to_phys(Rx_gpd_end[i],USB_RX,i));

		/* Allocate Rx GPD */
		size = sizeof(TGPD);
		size += AT_GPD_EXT_LEN;
		size *=MAX_GPD_NUM;
		ptr = (TGPD *)os_mem_alloc(size);
		os_memset(ptr, 0 , size);

		io_ptr = (TGPD *)dma_map_single(NULL, ptr, size, DMA_TO_DEVICE);
		//io_ptr = (TGPD*)os_ioremap(os_virt_to_phys(ptr),size);
		init_gpd_list( USB_TX, i, ptr, io_ptr, MAX_GPD_NUM);
		Tx_gpd_end[i]= ptr;
		os_printk(K_DEBUG,"ALLOC TX GPD End [%d] Virtual Mem=%p, DMA addr=%p\n", i, Tx_gpd_end[i], io_ptr);
		//os_memset(Tx_gpd_end[i], 0 , sizeof(TGPD)+AT_GPD_EXT_LEN);
		TGPD_CLR_FLAGS_HWO(Tx_gpd_end[i]);
		Tx_gpd_head[i]=Tx_gpd_last[i]=Tx_gpd_end[i];
		os_printk(K_DEBUG,"TQSAR[%d]=%p\n", i, (void *)mu3d_hal_gpd_virt_to_phys(Tx_gpd_end[i],USB_TX,i));

		/* Allocate Tx BD */
		size = (sizeof(TBD));
		size *= MAX_BD_NUM;
		bptr = (TBD *)os_mem_alloc(size);
		os_memset(bptr, 0 , size);
		io_bptr = (TBD *)dma_map_single(NULL, bptr, size, DMA_TO_DEVICE);
		//io_bptr = (TBD*)os_ioremap(os_virt_to_phys(bptr),size);
		init_bd_list(USB_RX, i, bptr, io_bptr, MAX_BD_NUM);

		/* Allocate Rx BD */
		size = (sizeof(TBD));
		size += AT_BD_EXT_LEN;
		size *= MAX_BD_NUM;
		bptr = (TBD *)os_mem_alloc(size);
		os_memset(bptr, 0 , size);
		io_bptr = (TBD *)dma_map_single(NULL, bptr, size, DMA_TO_DEVICE);
		//io_bptr = (TBD*)os_ioremap(os_virt_to_phys(bptr),size);
		init_bd_list(USB_TX, i, bptr, io_bptr, MAX_BD_NUM);
    }
}

/**
 * mu3d_hal_init_qmu - initialize qmu
 *
 */
void mu3d_hal_init_qmu(void)
{
	DEV_UINT32 i;
    DEV_UINT32 QCR = 0;

	/* Initialize QMU Tx/Rx start address. */
	for(i=1; i<=MAX_QMU_EP; i++){
		os_printk(K_DEBUG, "==EP[%d]==Start addr RXQ=0x%08x, TXQ=0x%08x\n", i, \
			mu3d_hal_gpd_virt_to_phys(Rx_gpd_head[i], USB_RX, i), \
			mu3d_hal_gpd_virt_to_phys(Tx_gpd_head[i], USB_TX, i));
		QCR|=QMU_RX_EN(i);
		QCR|=QMU_TX_EN(i);
		os_writel(USB_QMU_RQSAR(i), mu3d_hal_gpd_virt_to_phys(Rx_gpd_head[i],USB_RX,i));
		os_writel(USB_QMU_TQSAR(i), mu3d_hal_gpd_virt_to_phys(Tx_gpd_head[i],USB_TX,i));
		Tx_gpd_end[i] = Tx_gpd_last[i] = Tx_gpd_head[i];
		Rx_gpd_end[i] = Rx_gpd_last[i] = Rx_gpd_head[i];
		gpd_ptr_align(USB_TX,i,Tx_gpd_end[i]);
		gpd_ptr_align(USB_RX,i,Rx_gpd_end[i]);
	}
	/* Enable QMU Tx/Rx. */
	os_writel(U3D_QGCSR, QCR);
	os_writel(U3D_QIESR0, QCR);
	/* Enable QMU interrupt. */
	os_writel(U3D_QIESR1, TXQ_EMPTY_IESR|TXQ_CSERR_IESR|TXQ_LENERR_IESR|RXQ_EMPTY_IESR|RXQ_CSERR_IESR|RXQ_LENERR_IESR|RXQ_ZLPERR_IESR);
	os_writel(U3D_EPIESR, EP0ISR);
}

/**
 * mu3d_hal_cal_checksum - calculate check sum
 * @args - arg1: data buffer, arg2: data length
 */
DEV_UINT8 mu3d_hal_cal_checksum(DEV_UINT8 *data, DEV_INT32 len)
{
	DEV_UINT8 *uDataPtr, ckSum;
	DEV_INT32 i;

	*(data + 1) = 0x0;
	uDataPtr = data;
	ckSum = 0;
	for (i = 0; i < len; i++){
		ckSum += *(uDataPtr + i);
	}
  	return 0xFF - ckSum;
}

/**
 * mu3d_hal_resume_qmu - resume qmu function
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_resume_qmu(DEV_INT32 q_num,  USB_DIR dir){

    #if defined(USB_RISC_CACHE_ENABLED)
    os_flushinvalidateDcache();
    #endif

    if (dir == USB_TX) {
 		os_writel(USB_QMU_TQCSR(q_num), QMU_Q_RESUME);
        if(!os_readl( USB_QMU_TQCSR(q_num))) {
			os_printk(K_DEBUG, "%s QMU_TQCSR[%d]=%x\n", __func__, q_num, os_readl( USB_QMU_TQCSR(q_num)));
			os_writel( USB_QMU_TQCSR(q_num), QMU_Q_RESUME);
        }
    } else if(dir == USB_RX) {
       	os_writel(USB_QMU_RQCSR(q_num), QMU_Q_RESUME);
		if(!os_readl( USB_QMU_RQCSR(q_num))) {
			os_printk(K_DEBUG, "%s QMU_RQCSR[%d]=%x\n", __func__, q_num, os_readl( USB_QMU_RQCSR(q_num)));
			os_writel( USB_QMU_RQCSR(q_num), QMU_Q_RESUME);
		}
    } else {
    	os_printk(K_INFO, "%s wrong direction!!!\n", __func__);
		BUG_ON(1);
    }
}

/**
 * mu3d_hal_prepare_tx_gpd - prepare tx gpd/bd
 * @args - arg1: gpd address, arg2: data buffer address, arg3: data length, arg4: ep number, arg5: with bd or not, arg6: write hwo bit or not,  arg7: write ioc bit or not
 */
TGPD* mu3d_hal_prepare_tx_gpd(TGPD* gpd, dma_addr_t pBuf, DEV_UINT32 data_len,
					DEV_UINT8 ep_num, DEV_UINT8 _is_bdp, DEV_UINT8 isHWO,
					DEV_UINT8 ioc, DEV_UINT8 bps,DEV_UINT8 zlp)
{
	DEV_UINT32 offset;
	DEV_INT32 i;
	DEV_INT32 bd_num;
	DEV_UINT32 length;

	TBD *bd_next;
	TBD *bd_head;
	TBD *bd;
	DEV_UINT8 *pBuffer;

	/*If data length is less than the GPD buffer size, just use GPD*/
	//if (data_len <= GPD_BUF_SIZE) {
	//	_is_bdp = 0;
	//}

	os_printk(K_INFO, "%s gpd=%p, epnum=%d, len=%d, _is_bdp=%d\n", __func__, \
		gpd, ep_num, data_len, _is_bdp);

	if (!_is_bdp) {
		/*Set actual data point to "DATA Buffer"*/
		TGPD_SET_DATA(gpd, pBuf);
		/*Clear "BDP(Buffer Descriptor Present)" flag*/
		TGPD_CLR_FORMAT_BDP(gpd);
	} else {
		/*Get the first BD*/
		bd_head = (TBD*)get_bd(USB_TX, ep_num);
		os_printk(K_INFO,"bd_head=x%p\n", bd_head);

		bd = bd_head;
		os_memset(bd, 0, sizeof(TBD));

		/*Date length for transfer*/
		length = data_len;

		/*Point of data buffer*/
		pBuffer = (DEV_UINT8*)(pBuf);

		/*The size of BD buffer*/
		offset = BD_BUF_SIZE;

		/*Count how many BD this transfer need.*/
		bd_num = (!(length%offset)) ? (length/offset) : ((length/offset)+1);

		os_printk(K_INFO,"bd_num=%d\n", bd_num);

		/*If the size of BD buffer is bigger than the length of actual transfer, use the actual length*/
		if (offset > length)
			offset = length;

		/*Insert data into each BD*/
		for( i=0; i<bd_num; i++) {
			os_printk(K_INFO,"bd[%d]=%p\n", i, bd);
			if (i == (bd_num-1)) { /*The last BD*/
				TBD_SET_EXT_LEN(bd, 0); /*"BD Extension Length" = 0. Does not use BD EXT!!*/
				TBD_SET_BUF_LEN(bd, length); /*"Data Buffer Length" = the rest of data length*/
				TBD_SET_DATA(bd, pBuffer); /*Store the data pointer to "Data Buffer"*/

				TBD_SET_FLAGS_EOL(bd); /*Set "EOL"*/
				TBD_SET_NEXT(bd, 0); /*Set "Next BD pointer" = 0*/
				TBD_SET_CHKSUM(bd, CHECKSUM_LENGTH); /*Set "BD Checksum"*/

				/*Flush the data of BD stuct to device*/
				dma_sync_single_for_device(NULL, bd_virt_to_phys(bd,USB_RX,ep_num), \
								sizeof(TBD), DMA_BIDIRECTIONAL);

				/*There is no data left to be transfered by GPD*/
				//data_len=length;
				data_len = 0;

				/*There is no data left to insert BD*/
				length = 0;
			} else {
				TBD_SET_EXT_LEN(bd, 0); /*"BD Extension length" = 0. Does not use BD EXT!!*/
				TBD_SET_BUF_LEN(bd, offset); /*"Data Buffer Length" = the MAX BD transfer size*/
				TBD_SET_DATA(bd, pBuffer); /*Store the data pointer to "Data Buffer"*/

				TBD_CLR_FLAGS_EOL(bd); /*Clear "EOL"*/
				/*Get the next BD*/
				bd_next = (TBD*)get_bd(USB_TX,ep_num);
				os_memset(bd_next, 0, sizeof(TBD));

				/*Set "Next BD pointer" as the next BD*/
				TBD_SET_NEXT(bd, bd_virt_to_phys(bd_next,USB_TX,ep_num));
				TBD_SET_CHKSUM(bd, CHECKSUM_LENGTH); /*Set BD Checksum*/

				/*Flush the data of BD stuct to device*/
				dma_sync_single_for_device(NULL, bd_virt_to_phys(bd,USB_RX,ep_num), \
								sizeof(TBD), DMA_BIDIRECTIONAL);

				/*Calculate the left data length*/
				length -= offset;

				/*Move to pointer of buffer*/
				pBuffer += offset;

				/*Move to next BD*/
				bd = bd_next;
			}
		}

		/*Set the BD pointer into "BD Pointer" at GPD*/
		TGPD_SET_DATA(gpd, bd_virt_to_phys(bd_head,USB_TX,ep_num));

		/*Set "BDP(Buffer Descriptor Present)" flag*/
		TGPD_SET_FORMAT_BDP(gpd);
	}

	os_printk(K_INFO,"%s GPD data_length=%d\n", __func__, data_len);

	/*
	 * "Data Buffer Length" =
	 * 0        (If data length > GPD buffer length, use BDs),
	 * data_len (If data length < GPD buffer length, only use GPD)
	 */
	TGPD_SET_BUF_LEN(gpd, data_len);

	/*"GPD extension length" = 0. Does not use GPD EXT!!*/
	TGPD_SET_EXT_LEN(gpd, 0);

	/*Default: zlp=false, except type=ISOC*/
	if(zlp)
		TGPD_SET_FORMAT_ZLP(gpd);
	else
		TGPD_CLR_FORMAT_ZLP(gpd);

	/*Default: bps=false*/
	if(bps)
		TGPD_SET_FORMAT_BPS(gpd);
	else
		TGPD_CLR_FORMAT_BPS(gpd);

	/*Default: ioc=true*/
	if(ioc)
		TGPD_SET_FORMAT_IOC(gpd);
	else
		TGPD_CLR_FORMAT_IOC(gpd);

	/*Get the next GPD*/
	Tx_gpd_end[ep_num] = get_gpd(USB_TX ,ep_num);
	os_printk(K_INFO,"Tx_gpd_end[%d]=%p\n", ep_num, Tx_gpd_end[ep_num]);

	/*Initialize the new GPD*/
	os_memset(Tx_gpd_end[ep_num], 0 , sizeof(TGPD));

	/*Clear "HWO(Hardware Own)" flag*/
	TGPD_CLR_FLAGS_HWO(Tx_gpd_end[ep_num]);

	/*Set "Next GDP pointer" as the next GPD*/
	TGPD_SET_NEXT(gpd, mu3d_hal_gpd_virt_to_phys(Tx_gpd_end[ep_num], USB_TX, ep_num));

	/*Default: isHWO=true*/
	if (isHWO) {
		TGPD_SET_CHKSUM(gpd, CHECKSUM_LENGTH); /*Set GPD Checksum*/
		TGPD_SET_FLAGS_HWO(gpd); /*Set HWO flag*/
	} else {
		TGPD_CLR_FLAGS_HWO(gpd);
		TGPD_SET_CHKSUM_HWO(gpd, CHECKSUM_LENGTH);
	}

	/*Flush the data of GPD stuct to device*/
	dma_sync_single_for_device(NULL, mu3d_hal_gpd_virt_to_phys(gpd,USB_TX,ep_num), \
					sizeof(TGPD), DMA_BIDIRECTIONAL);

#if defined(USB_RISC_CACHE_ENABLED)
	os_flushinvalidateDcache();
#endif

	return gpd;
}

/**
 * mu3d_hal_prepare_rx_gpd - prepare rx gpd/bd
 * @args - arg1: gpd address, arg2: data buffer address, arg3: data length, arg4: ep number, arg5: with bd or not, arg6: write hwo bit or not,  arg7: write ioc bit or not
 */
TGPD* mu3d_hal_prepare_rx_gpd(TGPD*gpd, dma_addr_t pBuf, DEV_UINT32 data_len,
				DEV_UINT8 ep_num, DEV_UINT8 _is_bdp, DEV_UINT8 isHWO,
				DEV_UINT8 ioc, DEV_UINT8 bps, DEV_UINT32 cMaxPacketSize)
{
	DEV_UINT32 offset;
	DEV_INT32 i;
	DEV_INT32 bd_num;
	DEV_UINT32 length;

	TBD *bd_next;
	TBD *bd_head;
	TBD *bd;
	DEV_UINT8* pBuffer;

	/*If data length is less than the GPD buffer size, just use GPD*/
	if (data_len < GPD_BUF_SIZE) {
		_is_bdp = 0;
	}

	os_printk(K_INFO, "%s gpd=%p, epnum=%d, len=%d, _is_bdp=%d, maxp=%d\n", __func__, \
		gpd, ep_num, data_len, _is_bdp, cMaxPacketSize);

	if (!_is_bdp) {
		/*Set actual data point to "DATA Buffer"*/
		TGPD_SET_DATA(gpd, pBuf);
		/*Clear "BDP(Buffer Descriptor Present)" flag*/
		TGPD_CLR_FORMAT_BDP(gpd);
	} else {
		/*Get the first BD*/
		bd_head = (TBD*)get_bd(USB_RX,ep_num);
		os_printk(K_INFO,"bd_head=x%p\n", bd_head);

		bd = bd_head;
		os_memset(bd, 0, sizeof(TBD));

		/*Date length for transfer*/
		length = data_len;

		/*Point of data buffer*/
		pBuffer = (DEV_UINT8*)(pBuf);

		/*The size of BD buffer*/
		offset = BD_BUF_SIZE;

		/*Count how many BD this transfer need.*/
		bd_num = (!(length%offset)) ? (length/offset) : ((length/offset)+1);

		os_printk(K_INFO,"%s bd_num=%d\n", __func__, bd_num);

		/*Insert data into each BD*/
		for (i=0; i<bd_num; i++) {
			os_printk(K_INFO,"%s bd[%d]=%p\n", __func__, i, bd);
			if(i==(bd_num-1)){
				TBD_SET_BUF_LEN(bd, 0); /*Set "Transferred Data Length" = 0*/

				/*The last one's data buffer lengnth must be precise, or the GPD will never done unless ZLP or short packet.*/
				/*"Allow Data Buffer Length" = the rest of data length**/
				length = (!(length%cMaxPacketSize)) ? (length) : ((length/cMaxPacketSize)+1)*cMaxPacketSize;
				TBD_SET_DataBUF_LEN(bd, length);

				TBD_SET_DATA(bd, pBuffer); /*Store the data pointer to "Data Buffer"*/

				TBD_SET_FLAGS_EOL(bd); /*Set "EOL"*/
				TBD_SET_NEXT(bd, 0); /*Set "Next BD pointer" = 0*/
				TBD_SET_CHKSUM(bd, CHECKSUM_LENGTH); /*Set "BD Checksum"*/

				/*Flush the data of BD stuct to device*/
				dma_sync_single_for_device(NULL, bd_virt_to_phys(bd,USB_RX,ep_num), sizeof(TBD), DMA_BIDIRECTIONAL);

				break;
			} else {
				TBD_SET_BUF_LEN(bd, 0); /*Set "Transferred Data Length" = 0*/

				/*"Allow Data Buffer Length" = the MAX BD transfer size*/
				TBD_SET_DataBUF_LEN(bd, offset);

				TBD_SET_DATA(bd, pBuffer); /*Store the data pointer to "Data Buffer"*/

				TBD_CLR_FLAGS_EOL(bd); /*Clear "EOL"*/
				/*Get the next BD*/
				bd_next = (TBD*)get_bd(USB_RX,ep_num);
				os_memset(bd_next, 0, sizeof(TBD));

				/*Set "Next BD pointer" as the next BD*/
				TBD_SET_NEXT(bd, bd_virt_to_phys(bd_next,USB_RX,ep_num));
				TBD_SET_CHKSUM(bd, CHECKSUM_LENGTH); /*Set BD Checksum*/

				/*Flush the data of BD stuct to device*/
				dma_sync_single_for_device(NULL, bd_virt_to_phys(bd,USB_RX,ep_num), sizeof(TBD), DMA_BIDIRECTIONAL);

				/*Calculate the left data length*/
				length -= offset;

				/*Move to pointer of buffer*/
				pBuffer += offset;

				/*Move to next BD*/
				bd = bd_next;
			}
		}

		/*Set the BD pointer into "BD Pointer" at GPD*/
		TGPD_SET_DATA(gpd, bd_virt_to_phys(bd_head,USB_RX,ep_num));

		/*Set "BDP(Buffer Descriptor Present)" flag*/
		TGPD_SET_FORMAT_BDP(gpd);
	}

	os_printk(K_INFO,"%s GPD data_length=%d\n", __func__, data_len);

	/*
	 * Set "Allow Data Buffer Length" =
	 * 0        (If data length > GPD buffer length, use BDs),
	 * data_len (If data length < GPD buffer length, only use GPD)
	 */
	TGPD_SET_DataBUF_LEN(gpd, data_len);
	//TGPD_SET_DataBUF_LEN(gpd, gpd_buf_size);

	/*Set "Transferred Data Length" = 0*/
	TGPD_SET_BUF_LEN(gpd, 0);

	/*Default: bps=false*/
	if(bps)
		TGPD_SET_FORMAT_BPS(gpd);
	else
		TGPD_CLR_FORMAT_BPS(gpd);

	/*Default: ioc=true*/
	if(ioc)
		TGPD_SET_FORMAT_IOC(gpd);
	else
	  	TGPD_CLR_FORMAT_IOC(gpd);

	/*Get the next GPD*/
	Rx_gpd_end[ep_num] = get_gpd(USB_RX ,ep_num);
	os_printk(K_INFO,"%s Rx_gpd_end[%d]=%p\n", __func__, ep_num, Tx_gpd_end[ep_num]);

	/*Initialize the new GPD*/
	os_memset(Rx_gpd_end[ep_num], 0 , sizeof(TGPD));

	/*Clear "HWO(Hardware Own)" flag*/
	TGPD_CLR_FLAGS_HWO(Rx_gpd_end[ep_num]);

	/*Set Next GDP pointer to the next GPD*/
	TGPD_SET_NEXT(gpd, mu3d_hal_gpd_virt_to_phys(Rx_gpd_end[ep_num],USB_RX,ep_num));

	/*Default: isHWO=true*/
	if(isHWO) {
		TGPD_SET_CHKSUM(gpd, CHECKSUM_LENGTH); /*Set GPD Checksum*/
		TGPD_SET_FLAGS_HWO(gpd); /*Set HWO flag*/
	} else {
		TGPD_CLR_FLAGS_HWO(gpd);
		TGPD_SET_CHKSUM_HWO(gpd, CHECKSUM_LENGTH);
	}

	//os_printk(K_DEBUG,"Rx gpd info { HWO %d, Next_GPD %x ,DataBufferLength %d, DataBuffer %x, Recived Len %d, Endpoint %d, TGL %d, ZLP %d}\n",
	//				(DEV_UINT32)TGPD_GET_FLAG(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd),
	//				(DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd), (DEV_UINT32)TGPD_GET_DATA(gpd),
	//				(DEV_UINT32)TGPD_GET_BUF_LEN(gpd), (DEV_UINT32)TGPD_GET_EPaddr(gpd),
	//				(DEV_UINT32)TGPD_GET_TGL(gpd), (DEV_UINT32)TGPD_GET_ZLP(gpd));

	/*Flush the data of GPD stuct to device*/
	dma_sync_single_for_device(NULL, mu3d_hal_gpd_virt_to_phys(gpd,USB_RX,ep_num), sizeof(TGPD), DMA_BIDIRECTIONAL);

	return gpd;
}

/**
 * mu3d_hal_insert_transfer_gpd - insert new gpd/bd
 * @args - arg1: ep number, arg2: dir, arg3: data buffer, arg4: data length,  arg5: write hwo bit or not,  arg6: write ioc bit or not
 */
void mu3d_hal_insert_transfer_gpd(DEV_INT32 ep_num,USB_DIR dir, dma_addr_t buf,
					DEV_UINT32 count, DEV_UINT8 isHWO, DEV_UINT8 ioc,
					DEV_UINT8 bps, DEV_UINT8 zlp, DEV_UINT32 maxp)
{
 	TGPD* gpd;
	os_printk(K_INFO,"[%s] %s-EP%d, buf=%p, cnt=%d, zlp=%d, maxp=%d\n", \
		__func__, ((dir==USB_TX)?"TX":"RX"), ep_num, buf, count, zlp, maxp);

 	if(dir == USB_TX) {
		gpd = Tx_gpd_end[ep_num];
		//os_printk(K_INFO,"TX gpd :%x\n", (unsigned int)gpd);
		mu3d_hal_prepare_tx_gpd(gpd, buf, count, ep_num, IS_BDP, isHWO, ioc, bps, zlp);
	} else if(dir == USB_RX) {
		gpd = Rx_gpd_end[ep_num];
		//os_printk(K_INFO,"RX gpd :%x\n",(unsigned int)gpd);
	 	mu3d_hal_prepare_rx_gpd(gpd, buf, count, ep_num, IS_BDP, isHWO, ioc, bps, maxp);
	}

}

/**
 * mu3d_hal_start_qmu - start qmu function (QMU flow : mu3d_hal_init_qmu ->mu3d_hal_start_qmu -> mu3d_hal_insert_transfer_gpd -> mu3d_hal_resume_qmu)
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_start_qmu(DEV_INT32 Q_num,  USB_DIR dir){

    DEV_UINT32 QCR;
    DEV_UINT32 Temp;
    DEV_UINT32 txcsr;

    if(dir == USB_TX){

		txcsr = USB_ReadCsr32(U3D_TX1CSR0, Q_num) & 0xFFFEFFFF;
		USB_WriteCsr32(U3D_TX1CSR0, Q_num, txcsr | TX_DMAREQEN);
		QCR = os_readl(U3D_QCR0);
		os_writel(U3D_QCR0, QCR|QMU_TX_CS_EN(Q_num));
#if (TXZLP==HW_MODE)
		QCR = os_readl(U3D_QCR1);
	 	os_writel(U3D_QCR1, QCR &~ QMU_TX_ZLP(Q_num));
		QCR = os_readl(U3D_QCR2);
		os_writel(U3D_QCR2, QCR|QMU_TX_ZLP(Q_num));
#elif (TXZLP==GPD_MODE)
		QCR = os_readl(U3D_QCR1);
		os_writel(U3D_QCR1, QCR|QMU_TX_ZLP(Q_num));
#endif
		os_writel(U3D_QEMIESR, os_readl(U3D_QEMIESR) | QMU_TX_EMPTY(Q_num));
		os_writel(U3D_TQERRIESR0, QMU_TX_LEN_ERR(Q_num)|QMU_TX_CS_ERR(Q_num));
      	Temp = os_readl(USB_QMU_TQCSR(Q_num));
       	os_printk(K_DEBUG,"USB_QMU_TQCSR:0x%08X\n", Temp);

 		if(os_readl(USB_QMU_TQCSR(Q_num))&QMU_Q_ACTIVE){
		  	os_printk(K_DEBUG,"Tx %d Active Now!\n", Q_num);
		  	return;
		}
       	#if defined(USB_RISC_CACHE_ENABLED)
       	os_flushinvalidateDcache();
       	#endif
		os_writel(USB_QMU_TQCSR(Q_num), QMU_Q_START);
    }
    else if(dir == USB_RX){

		USB_WriteCsr32(U3D_RX1CSR0, Q_num, USB_ReadCsr32(U3D_RX1CSR0, Q_num) |(RX_DMAREQEN));
      	QCR = os_readl(U3D_QCR0);
		os_writel(U3D_QCR0, QCR|QMU_RX_CS_EN(Q_num));

		#ifdef CFG_RX_ZLP_EN
			QCR = os_readl(U3D_QCR3);
			os_writel(U3D_QCR3, QCR|QMU_RX_ZLP(Q_num));
		#else
			QCR = os_readl(U3D_QCR3);
			os_writel(U3D_QCR3, QCR&~(QMU_RX_ZLP(Q_num)));
		#endif

		#ifdef CFG_RX_COZ_EN
			QCR = os_readl(U3D_QCR3);
			os_writel(U3D_QCR3, QCR|QMU_RX_COZ(Q_num));
		#else
			QCR = os_readl(U3D_QCR3);
			os_writel(U3D_QCR3, QCR&~(QMU_RX_COZ(Q_num)));
		#endif

		os_writel(U3D_QEMIESR, os_readl(U3D_QEMIESR) | QMU_RX_EMPTY(Q_num));
		os_writel(U3D_RQERRIESR0, QMU_RX_LEN_ERR(Q_num)|QMU_RX_CS_ERR(Q_num));
		os_writel(U3D_RQERRIESR1, QMU_RX_EP_ERR(Q_num)|QMU_RX_ZLP_ERR(Q_num));

		if(os_readl(USB_QMU_RQCSR(Q_num))&QMU_Q_ACTIVE){
		  	os_printk(K_DEBUG,"Rx %d Active Now!\n", Q_num);
		  	return;
		}

     	#if defined(USB_RISC_CACHE_ENABLED)
      	os_flushinvalidateDcache();
       	#endif
		os_writel(USB_QMU_RQCSR(Q_num), QMU_Q_START);
    }

#if (CHECKSUM_TYPE==CS_16B)
	os_writel(U3D_QCR0, os_readl(U3D_QCR0)|CS16B_EN);
#else
	os_writel(U3D_QCR0, os_readl(U3D_QCR0)&~CS16B_EN);
#endif


}

/**
 * mu3d_hal_stop_qmu - stop qmu function (after qmu stop, fifo should be flushed)
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_stop_qmu(DEV_INT32 q_num,  USB_DIR dir)
{
    if (dir == USB_TX) {
  		if(!(os_readl(USB_QMU_TQCSR(q_num)) & (QMU_Q_ACTIVE))){
 			os_printk(K_DEBUG,"Tx%d inActive Now!\n", q_num);
		  	return;
		}
		os_writel(USB_QMU_TQCSR(q_num), QMU_Q_STOP);
		while((os_readl(USB_QMU_TQCSR(q_num)) & (QMU_Q_ACTIVE)));
		os_printk(K_CRIT,"Tx%d stop Now!\n", q_num);
    } else if(dir == USB_RX) {
		if(!(os_readl(USB_QMU_RQCSR(q_num)) & QMU_Q_ACTIVE)){
			os_printk(K_DEBUG,"Rx%d inActive Now!\n", q_num);
		  	return;
		}
		os_writel(USB_QMU_RQCSR(q_num), QMU_Q_STOP);
		while((os_readl(USB_QMU_RQCSR(q_num)) & (QMU_Q_ACTIVE)));
		os_printk(K_CRIT,"Rx%d stop now!\n", q_num);
    }
}

/**
 * mu3d_hal_send_stall - send stall
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_send_stall(DEV_INT32 q_num,  USB_DIR dir)
{
	if (dir == USB_TX) {
		USB_WriteCsr32(U3D_TX1CSR0, q_num, USB_ReadCsr32(U3D_TX1CSR0, q_num) | TX_SENDSTALL);
		while(!(USB_ReadCsr32(U3D_TX1CSR0, q_num) & TX_SENTSTALL));
		USB_WriteCsr32(U3D_TX1CSR0, q_num, USB_ReadCsr32(U3D_TX1CSR0, q_num) | TX_SENTSTALL);
		USB_WriteCsr32(U3D_TX1CSR0, q_num, USB_ReadCsr32(U3D_TX1CSR0, q_num) &~ TX_SENDSTALL);
	} else if(dir == USB_RX) {
		USB_WriteCsr32(U3D_RX1CSR0, q_num, USB_ReadCsr32(U3D_RX1CSR0, q_num) | RX_SENDSTALL);
		while(!(USB_ReadCsr32(U3D_RX1CSR0, q_num) & RX_SENTSTALL));
		USB_WriteCsr32(U3D_RX1CSR0, q_num, USB_ReadCsr32(U3D_RX1CSR0, q_num) | RX_SENTSTALL);
		USB_WriteCsr32(U3D_RX1CSR0, q_num, USB_ReadCsr32(U3D_RX1CSR0, q_num) &~ RX_SENDSTALL);
	}

	os_printk(K_CRIT,"%s %s-EP[%d] sent stall\n", __func__, ((dir==USB_TX)?"TX":"RX"), q_num);
}


/**
 * mu3d_hal_restart_qmu - clear toggle(or sequence) number and start qmu
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_restart_qmu(DEV_INT32 q_num,  USB_DIR dir)
{
	DEV_UINT32 ep_rst;

	os_printk(K_CRIT,"%s : Rest %s-EP[%d]\n", __func__, ((dir==USB_TX)?"TX":"RX"), q_num);

	if (dir == USB_TX) {
		ep_rst = BIT16<<q_num;
		os_writel(U3D_EP_RST, ep_rst);
		os_ms_delay(1);
		os_writel(U3D_EP_RST, 0);
	} else {
		ep_rst = 1<<q_num;
		os_writel(U3D_EP_RST, ep_rst);
		os_ms_delay(1);
		os_writel(U3D_EP_RST, 0);
	}
	mu3d_hal_start_qmu(q_num, dir);
}

/**
 * flush_qmu - stop qmu and align qmu start ptr t0 current ptr
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_flush_qmu(DEV_INT32 Q_num,  USB_DIR dir)
{
	TGPD* gpd_current;

	struct USB_REQ *req = mu3d_hal_get_req(Q_num, dir);

	os_printk(K_CRIT,"%s flush QMU %s\n", __func__, ((dir==USB_TX)?"TX":"RX"));

	if (dir == USB_TX) {
		/*Stop QMU*/
		mu3d_hal_stop_qmu(Q_num, USB_TX);

		/*Get TX Queue Current Pointer Register*/
		gpd_current = (TGPD*)(os_readl(USB_QMU_TQCPR(Q_num)));

		/*If gpd_current = 0, it means QMU has not yet to execute GPD in QMU.*/
		if(!gpd_current){
			/*Get TX Queue Starting Address Register*/
			gpd_current = (TGPD*)(os_readl(USB_QMU_TQSAR(Q_num)));
		}

		/*Switch physical to virtual address*/
		os_printk(K_CRIT,"gpd_current(P) %p\n", gpd_current);
		gpd_current = gpd_phys_to_virt(gpd_current, USB_TX, Q_num);
		os_printk(K_CRIT,"gpd_current(V) %p\n", gpd_current);

		/*Reset the TX GPD list state*/
		Tx_gpd_end[Q_num] = Tx_gpd_last[Q_num] = gpd_current;
		gpd_ptr_align(dir,Q_num,Tx_gpd_end[Q_num]);
		free_gpd(dir,Q_num);

		/*FIXME: Do not know why...*/
		os_writel(USB_QMU_TQSAR(Q_num), mu3d_hal_gpd_virt_to_phys(Tx_gpd_last[Q_num], USB_TX, Q_num));
		os_printk(K_ERR,"USB_QMU_TQSAR %x\n", os_readl(USB_QMU_TQSAR(Q_num)));
		req->complete=true;
		//os_printk(K_ERR,"TxQ %d Flush Now!\n", Q_num);
	} else if(dir == USB_RX) {
		/*Stop QMU*/
		mu3d_hal_stop_qmu(Q_num, USB_RX);

		/*Get RX Queue Current Pointer Register*/
		gpd_current = (TGPD*)(os_readl(USB_QMU_RQCPR(Q_num)));
		if(!gpd_current){
			/*Get RX Queue Starting Address Register*/
			gpd_current = (TGPD*)(os_readl(USB_QMU_RQSAR(Q_num)));
		}

		/*Switch physical to virtual address*/
		os_printk(K_CRIT,"gpd_current(P) %p\n", gpd_current);
		gpd_current = gpd_phys_to_virt(gpd_current,USB_RX,Q_num);
		os_printk(K_CRIT,"gpd_current(V) %p\n", gpd_current);

		/*Reset the RX GPD list state*/
		Rx_gpd_end[Q_num] = Rx_gpd_last[Q_num] = gpd_current;
		gpd_ptr_align(dir,Q_num,Rx_gpd_end[Q_num]);
		free_gpd(dir,Q_num);

		/*FIXME: Do not know why...*/
		os_writel(USB_QMU_RQSAR(Q_num), mu3d_hal_gpd_virt_to_phys(Rx_gpd_end[Q_num], USB_RX, Q_num));
		os_printk(K_ERR,"USB_QMU_RQSAR %x\n", os_readl(USB_QMU_RQSAR(Q_num)));
		req->complete=true;
		//os_printk(K_ERR,"RxQ %d Flush Now!\n", Q_num);
	}
}

