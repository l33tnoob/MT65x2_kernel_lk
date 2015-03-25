/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudDrv_Kernelc
 *
 * Project:
 * --------
 *   MT6583  Audio Driver Kernel Function
 *
 * Description:
 * ------------
 *   Audio register
 *
 * Author:
 * -------
 * Chipeng Chang
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 * 09 11 2013 kh.hung
 * [ALPS00996254] MR2 migration for MT6572
 * Support mSBC.
 *
 *
 *******************************************************************************/


/*****************************************************************************
 *                     C O M P I L E R   F L A G S
 *****************************************************************************/


/*****************************************************************************
 *                E X T E R N A L   R E F E R E N C E S
 *****************************************************************************/
#include "AudDrv_BTCVSD.h"
#include "AudDrv_BTCVSD_ioctl.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/xlog.h>
#include <mach/irqs.h>
#include <mach/mt_irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/mt_reg_base.h>
#include <asm/div64.h>
#include <linux/aee.h>

/*****************************************************************************
*           DEFINE AND CONSTANT
******************************************************************************
*/

#define AUDIO_BTSYS_PKV_PHYSICAL_BASE  (0x18000000)
#define AUDIO_BTSYS_SRAM_BANK2_PHYSICAL_BASE  (0x18080000)
#define INFRA_MISC (0xF0001F00)

#define AUDDRV_BTCVSD_NAME   "MediaTek Audio BTCVSD Driver"
#define AUDDRV_AUTHOR "MediaTek WCX"

#define MASK_ALL		  (0xFFFFFFFF)

/*****************************************************************************
*           V A R I A B L E     D E L A R A T I O N
*******************************************************************************/

static char       auddrv_btcvsd_name[]       = "AudioMTKBTCVSD";
static u64        AudDrv_btcvsd_dmamask      = 0xffffffffUL;

static kal_uint32 disableBTirq = 0;

static const kal_uint32 btsco_PacketValidMask[6][6] = {{0x1   , 0x1<<1, 0x1<<2, 0x1<<3, 0x1<<4 , 0x1<<5 },  //30
                                                       {0x1   , 0x1   , 0x2   , 0x2   , 0x4    , 0x4    },  //60
                                                       {0x1   , 0x1   , 0x1   , 0x2   , 0x2    , 0x2    },  //90
                                                       {0x1   , 0x1   , 0x1   , 0x1   , 0      , 0      },  //120
                                                       {0x7   , 0x7<<3, 0x7<<6, 0x7<<9, 0x7<<12, 0x7<<15},  //10
                                                       {0x3   , 0x3<<1, 0x3<<3, 0x3<<4, 0x3<<6 , 0x3<<7 }}; //20

static const kal_uint8 btsco_PacketInfo[6][6] = {{ 30, 6, BT_SCO_PACKET_180/SCO_TX_ENCODE_SIZE, BT_SCO_PACKET_180/SCO_RX_PLC_SIZE},  //30
                                                 { 60, 3, BT_SCO_PACKET_180/SCO_TX_ENCODE_SIZE, BT_SCO_PACKET_180/SCO_RX_PLC_SIZE},  //60
                                                 { 90, 2, BT_SCO_PACKET_180/SCO_TX_ENCODE_SIZE, BT_SCO_PACKET_180/SCO_RX_PLC_SIZE},  //90
                                                 {120, 1, BT_SCO_PACKET_120/SCO_TX_ENCODE_SIZE, BT_SCO_PACKET_120/SCO_RX_PLC_SIZE},  //120
                                                 { 10,18, BT_SCO_PACKET_180/SCO_TX_ENCODE_SIZE, BT_SCO_PACKET_180/SCO_RX_PLC_SIZE},  //10
                                                 { 20, 9, BT_SCO_PACKET_180/SCO_TX_ENCODE_SIZE, BT_SCO_PACKET_180/SCO_RX_PLC_SIZE}}; //20


static struct{
   BT_SCO_TX_T *pTX;
   BT_SCO_RX_T *pRX;
   kal_uint8 *pStructMemory;
   kal_uint8 *pWorkingMemory;
   kal_uint16 uAudId;
   CVSD_STATE uTXState;
   CVSD_STATE uRXState;
   kal_bool  fIsStructMemoryOnMED;
   kal_bool  fWideBand;
}btsco;

static volatile kal_uint32 *bt_hw_REG_PACKET_W, *bt_hw_REG_PACKET_R, *bt_hw_REG_CONTROL;

static kal_uint32 btcvsd_InterruptTime =0;

static DEFINE_SPINLOCK(auddrv_BTCVSDTX_lock); 
static DEFINE_SPINLOCK(auddrv_BTCVSDRX_lock);

static kal_uint32 BTCVSD_write_wait_queue_flag  = 0;
static kal_uint32 BTCVSD_read_wait_queue_flag = 0;


DECLARE_WAIT_QUEUE_HEAD(BTCVSD_Write_Wait_Queue);
DECLARE_WAIT_QUEUE_HEAD(BTCVSD_Read_Wait_Queue);

static void Disable_CVSD_Wakeup()
{
   volatile kal_uint32 *INFRA_MISC_REGISTER = (volatile kal_uint32*)(INFRA_MISC);
   *INFRA_MISC_REGISTER |= 0x00000002;
}

static void Enable_CVSD_Wakeup()
{
   volatile kal_uint32 *INFRA_MISC_REGISTER = (volatile kal_uint32*)(INFRA_MISC);
   *INFRA_MISC_REGISTER &= ~(0x00000002);
}
	
static int AudDrv_btcvsd_Allocate_Buffer(struct file *fp, kal_uint8 isRX)
{
	printk("AudDrv_btcvsd_Allocate_Buffer(+) isRX=%d\n",isRX);
	
	if(isRX==1)
	{
		BT_CVSD_Mem.u4RXBufferSize = sizeof(BT_SCO_RX_T);
		
		 if( (BT_CVSD_Mem.pucRXVirtBufAddr == NULL) && (BT_CVSD_Mem.pucRXPhysBufAddr == 0) ){
				BT_CVSD_Mem.pucRXVirtBufAddr = dma_alloc_coherent(0, BT_CVSD_Mem.u4RXBufferSize,&BT_CVSD_Mem.pucRXPhysBufAddr,GFP_KERNEL);
				if((0 == BT_CVSD_Mem.pucRXPhysBufAddr)||(NULL == BT_CVSD_Mem.pucRXVirtBufAddr))
				{
					 printk("AudDrv_btcvsd_Allocate_Buffer dma_alloc_coherent RX fail \n");
					 return -1;
				}
			  
				memset((void*)BT_CVSD_Mem.pucRXVirtBufAddr,0,BT_CVSD_Mem.u4RXBufferSize);
	
				PRINTK_AUDDRV("BT_CVSD_Mem.pucRXVirtBufAddr = %p BT_CVSD_Mem.pucRXPhysBufAddr = 0x%x\n" ,
					 BT_CVSD_Mem.pucRXVirtBufAddr, BT_CVSD_Mem.pucRXPhysBufAddr);

				btsco.pRX =  BT_CVSD_Mem.pucRXVirtBufAddr;
				btsco.pRX->u4BufferSize = SCO_RX_PACKER_BUF_NUM * (SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE);
		 }
	}
	else
	{
			BT_CVSD_Mem.u4TXBufferSize = sizeof(BT_SCO_TX_T);
				
			if( (BT_CVSD_Mem.pucTXVirtBufAddr == NULL) && (BT_CVSD_Mem.pucTXPhysBufAddr == 0) )
			{
				BT_CVSD_Mem.pucTXVirtBufAddr = dma_alloc_coherent(0, BT_CVSD_Mem.u4TXBufferSize,&BT_CVSD_Mem.pucTXPhysBufAddr,GFP_KERNEL);
				if((0 == BT_CVSD_Mem.pucTXPhysBufAddr)||(NULL == BT_CVSD_Mem.pucTXVirtBufAddr))
				{
					 printk("AudDrv_btcvsd_Allocate_Buffer dma_alloc_coherent TX fail \n");
					 return -1;
				}
				
				memset((void*)BT_CVSD_Mem.pucTXVirtBufAddr,0,BT_CVSD_Mem.u4TXBufferSize);

				PRINTK_AUDDRV("BT_CVSD_Mem.pucTXVirtBufAddr = %p BT_CVSD_Mem.pucTXPhysBufAddr = 0x%x\n" ,
				BT_CVSD_Mem.pucTXVirtBufAddr, BT_CVSD_Mem.pucTXPhysBufAddr);

				btsco.pTX =  BT_CVSD_Mem.pucTXVirtBufAddr;
				btsco.pTX->u4BufferSize = SCO_TX_PACKER_BUF_NUM * SCO_TX_ENCODE_SIZE;
			}
	}
	printk("AudDrv_btcvsd_Allocate_Buffer(-) \n");
	return 0;
}

static int AudDrv_btcvsd_Free_Buffer(struct file *fp, kal_uint8 isRX)
{
	printk("AudDrv_btcvsd_Free_Buffer(+) isRX=%d\n",isRX);

	if(isRX==1)
	{
	   if( (BT_CVSD_Mem.pucRXVirtBufAddr != NULL) && (BT_CVSD_Mem.pucRXPhysBufAddr != 0) )
	   {
	       PRINTK_AUDDRV("AudDrv_btcvsd_Free_Buffer dma_free_coherent pucRXVirtBufAddr = %p pucRXPhysBufAddr = %x",BT_CVSD_Mem.pucRXVirtBufAddr,BT_CVSD_Mem.pucRXPhysBufAddr);
   			 btsco.pRX =  NULL;
	       dma_free_coherent(0,BT_CVSD_Mem.u4RXBufferSize,BT_CVSD_Mem.pucRXVirtBufAddr,BT_CVSD_Mem.pucRXPhysBufAddr);
			 BT_CVSD_Mem.u4RXBufferSize = 0;
			 BT_CVSD_Mem.pucRXVirtBufAddr = NULL;
			 BT_CVSD_Mem.pucRXPhysBufAddr = 0;

	   }
	   else
	   {
	       PRINTK_AUDDRV("AudDrv_btcvsd_Free_Buffer cannot dma_free_coherent pucRXVirtBufAddr = %p pucRXPhysBufAddr = %x",BT_CVSD_Mem.pucRXVirtBufAddr,BT_CVSD_Mem.pucRXPhysBufAddr);
			 return -1;
	   }
	}
	else
	{
		if( (BT_CVSD_Mem.pucTXVirtBufAddr != NULL) && (BT_CVSD_Mem.pucTXPhysBufAddr != 0) )
	   {
	       PRINTK_AUDDRV("AudDrv_btcvsd_Free_Buffer dma_free_coherent pucTXVirtBufAddr = %p pucTXPhysBufAddr = %x",BT_CVSD_Mem.pucTXVirtBufAddr,BT_CVSD_Mem.pucTXPhysBufAddr);
  			 btsco.pTX =  NULL;
	       dma_free_coherent(0,BT_CVSD_Mem.u4TXBufferSize,BT_CVSD_Mem.pucTXVirtBufAddr,BT_CVSD_Mem.pucTXPhysBufAddr);
	 		 BT_CVSD_Mem.u4TXBufferSize = 0;
			 BT_CVSD_Mem.pucTXVirtBufAddr = NULL;
			 BT_CVSD_Mem.pucTXPhysBufAddr = 0;

	   }
	   else
	   {
	       PRINTK_AUDDRV("AudDrv_btcvsd_Free_Buffer cannot dma_free_coherent pucTXVirtBufAddr = %p pucTXPhysBufAddr = %x",BT_CVSD_Mem.pucTXVirtBufAddr,BT_CVSD_Mem.pucTXPhysBufAddr);
			 return -1;
	   }
	}
	printk("AudDrv_btcvsd_Free_Buffer(-) \n");
	return 0;
}


/*****************************************************************************
 * FILE OPERATION FUNCTION
 *  AudDrv_btcvsd_ioctl
 *
 * DESCRIPTION
 *  IOCTL Msg handle
 *
 *****************************************************************************
 */
static long AudDrv_btcvsd_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    int  ret = 0;
	
	printk("AudDrv_btcvsd_ioctl cmd = 0x%x arg = %lu\n",cmd,arg);

    switch(cmd)
    {
		  
			case ALLOCATE_FREE_BTCVSD_BUF:
			{
				// 0: allocate TX buf
				// 1: free TX buf
				// 2: allocate RX buf
				// 3: free TX buf
				if(arg == 0) 
				{
         			ret =  AudDrv_btcvsd_Allocate_Buffer(fp,0);
				}
				else if(arg == 1) 
				{
         			ret =  AudDrv_btcvsd_Free_Buffer(fp,0);
				}
				else if(arg == 2) 
				{
         			ret =  AudDrv_btcvsd_Allocate_Buffer(fp,1);
				}
				else if(arg == 3) 
				{
         			ret =  AudDrv_btcvsd_Free_Buffer(fp,1);
				}
				break;
		  	}
			
		case SET_BTCVSD_STATE:
		{
		   kal_uint8 BT_CVSD_State;
           printk("AudDrv SET_BTCVSD_STATE \n");
			if(arg == BT_SCO_TXSTATE_DIRECT_LOOPBACK)
			{
				btsco.uTXState = arg;
				btsco.uRXState = arg;
			}
			else if((arg&0x10)==0) //TX state
			{
				btsco.uTXState = arg;
				printk("SET_BTCVSD_STATE set btsco.uTXState to %d \n",arg);
			}
			else //RX state
			{
				btsco.uRXState = arg;
				printk("SET_BTCVSD_STATE set btsco.uRXState to %d \n",arg);
			}
			if( btsco.uTXState == BT_SCO_TXSTATE_IDLE && btsco.uRXState == BT_SCO_RXSTATE_IDLE )
			{
				printk("SET_BTCVSD_STATE disable BT IRQ disableBTirq = %d",disableBTirq);
				if(disableBTirq==0)
				{
					disable_irq(MT_BT_CVSD_IRQ_ID);
					Disable_CVSD_Wakeup();
					disableBTirq = 1;
				}
			}
			else
			{
				printk("SET_BTCVSD_STATE enable BT IRQ disableBTirq = %d",disableBTirq);
				if(disableBTirq==1)
				{
					enable_irq(MT_BT_CVSD_IRQ_ID);
					Enable_CVSD_Wakeup();
					disableBTirq = 0;
				}
            }
            break;
        }
        case GET_BTCVSD_STATE:
        {

            break;
        }
        default:
		  {
            printk("AudDrv_btcvsd_ioctl Fail command: %x \n",cmd);
            ret = -1;
            break;
        }
   }
	return ret;
}

//=============================================================================================
//    BT SCO Internal Function
//=============================================================================================

static void AudDrv_BTCVSD_DataTransfer(BT_SCO_DIRECT uDir, kal_uint8 *pSrc, kal_uint8 *pDst, kal_uint32 uBlockSize, kal_uint32 uBlockNum, CVSD_STATE uState)
{
   kal_int32 i, j;

   if(uBlockSize == 60 || uBlockSize == 120 || uBlockSize == 20)
   {
      kal_uint32 *pSrc32 = (kal_uint32*)pSrc;
      kal_uint32 *pDst32 = (kal_uint32*)pDst;

      for(i=0 ; i<(uBlockSize*uBlockNum/4) ; i++)
      {
         *pDst32++ = *pSrc32++;
      }
   }
   else
   {
      kal_uint16 *pSrc16 = (kal_uint16*)pSrc;
      kal_uint16 *pDst16 = (kal_uint16*)pDst;
      for(j=0 ; j< uBlockNum ; j++)
      {
         for(i=0 ; i<(uBlockSize/2) ; i++)
         {
            *pDst16++ = *pSrc16++;
         }
         if(uDir == BT_SCO_DIRECT_BT2ARM)
         {
            pSrc16++;
         }
         else
         {
            pDst16++;
         }
      }
   }
}

static void AudDrv_BTCVSD_ReadFromBT(BT_SCO_PACKET_LEN uLen, kal_uint32 uPacketLength, kal_uint32 uPacketNumber, kal_uint32 uBlockSize, kal_uint32 uControl, kal_uint32 ap_addr_rx)
{
	kal_int32 i;
	kal_uint16 pv;
	kal_uint8 *pSrc;
	kal_uint8 *pPacketBuf;
	unsigned long flags;
	//printk("AudDrv_BTCVSD_ReadFromBT(+) btsco.pRX->iPacket_w=%d\n",btsco.pRX->iPacket_w);
	pSrc = (kal_uint8 *)ap_addr_rx;

	//printk("AudDrv_BTCVSD_ReadFromBT()uPacketLength=%d,uPacketNumber=%d, btsco.uRXState=%d\n",uPacketLength, uPacketNumber,btsco.uRXState);
	AudDrv_BTCVSD_DataTransfer(BT_SCO_DIRECT_BT2ARM, pSrc, btsco.pRX->TempPacketBuf, uPacketLength, uPacketNumber, btsco.uRXState);
	//printk("AudDrv_BTCVSD_ReadFromBT()AudDrv_BTCVSD_DataTransfer DONE!!!,uControl=0x%x,uLen=%d \n",uControl,uLen);
	
	if(btsco.uRXState ==  BT_SCO_RXSTATE_ENDING)
  {
#if defined(__MSBC_CODEC_SUPPORT__)
     if(btsco.fWideBand)
     {
        memset((void *)(btsco.pRX->TempPacketBuf),    0, uPacketLength*uPacketNumber*sizeof(kal_uint8));
     }
     else
#endif
     {
        memset((void *)(btsco.pRX->TempPacketBuf), 0x55, uPacketLength*uPacketNumber*sizeof(kal_uint8));
     }
   }
	
	spin_lock_irqsave(&auddrv_BTCVSDRX_lock, flags);
	for(i=0;i<uBlockSize;i++)
	{
		memcpy(btsco.pRX->PacketBuf[btsco.pRX->iPacket_w & SCO_RX_PACKET_MASK], btsco.pRX->TempPacketBuf+(SCO_RX_PLC_SIZE*i), SCO_RX_PLC_SIZE);
		if( (uControl & btsco_PacketValidMask[uLen][i]) == btsco_PacketValidMask[uLen][i])
		{
			pv = 1;
		}
		else
		{
			pv = 0;
		}
	
		pPacketBuf = (kal_uint8 *)btsco.pRX->PacketBuf + (btsco.pRX->iPacket_w & SCO_RX_PACKET_MASK)*(SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE) + SCO_RX_PLC_SIZE;
		memcpy((void *)pPacketBuf, (void *)&pv , BTSCO_CVSD_PACKET_VALID_SIZE);
		btsco.pRX->iPacket_w++;
	}
	spin_unlock_irqrestore(&auddrv_BTCVSDRX_lock, flags);
	//printk("AudDrv_BTCVSD_ReadFromBT(-) btsco.pRX->iPacket_w=%d\n",btsco.pRX->iPacket_w);
}

static void AudDrv_BTCVSD_WriteToBT(BT_SCO_PACKET_LEN uLen, kal_uint32 uPacketLength, kal_uint32 uPacketNumber, kal_uint32 uBlockSize, kal_uint32 ap_addr_tx)
{
	kal_int32 i;
	unsigned long flags;
	kal_uint8 *pDst; 
	//printk("AudDrv_BTCVSD_WriteToBT(+) btsco.pTX->iPacket_r=%d \n",btsco.pTX->iPacket_r);
	spin_lock_irqsave(&auddrv_BTCVSDTX_lock, flags);
	if(btsco.pTX!=NULL)
	{
		if(btsco.pTX->fUnderflow)
		{
#if defined(__MSBC_CODEC_SUPPORT__)
			if(btsco.fWideBand)
			{
				memset((void *)(btsco.pTX->TempPacketBuf),    0, uPacketLength*uPacketNumber*sizeof(kal_uint8));
			}
			else
#endif
			{
				memset((void *)(btsco.pTX->TempPacketBuf), 0x55, uPacketLength*uPacketNumber*sizeof(kal_uint8));
			}
		}
		else
		{
			for(i=0;i<uBlockSize;i++)
			{
				//printk("PacketBuf: %d %d %d \n",btsco.pTX->PacketBuf[btsco.pTX->iPacket_r & SCO_TX_PACKET_MASK][0],btsco.pTX->PacketBuf[btsco.pTX->iPacket_r & SCO_TX_PACKET_MASK][1],btsco.pTX->PacketBuf[btsco.pTX->iPacket_r & SCO_TX_PACKET_MASK][2]);
				memcpy((void *)(btsco.pTX->TempPacketBuf+(SCO_TX_ENCODE_SIZE*i)), (void *)(btsco.pTX->PacketBuf[btsco.pTX->iPacket_r & SCO_TX_PACKET_MASK]), SCO_TX_ENCODE_SIZE);
				//printk("TempPacketBuf: %d %d %d \n",*((kal_uint8 *)(btsco.pTX->TempPacketBuf)+(SCO_TX_ENCODE_SIZE*i)+0),*((kal_uint8 *)(btsco.pTX->TempPacketBuf)+(SCO_TX_ENCODE_SIZE*i)+1),*((kal_uint8 *)(btsco.pTX->TempPacketBuf)+(SCO_TX_ENCODE_SIZE*i)+2));
				btsco.pTX->iPacket_r++;
			}
		}
	}
	spin_unlock_irqrestore(&auddrv_BTCVSDTX_lock, flags);
	//printk("AudDrv_BTCVSD_WriteToBT(2) \n");
	//printk("AudDrv_BTCVSD_WriteToBT connsys_addr_tx=0x%x,ap_addr_tx=0x%x \n",connsys_addr_tx,ap_addr_tx);
	pDst = (kal_uint8 *)ap_addr_tx;
	if(btsco.pTX!=NULL)
	{
		AudDrv_BTCVSD_DataTransfer(BT_SCO_DIRECT_ARM2BT, btsco.pTX->TempPacketBuf, pDst, uPacketLength, uPacketNumber, btsco.uTXState);
	}
	//printk("AudDrv_BTCVSD_WriteToBT(-),btsco.pTX->iPacket_r=%d \n",btsco.pTX->iPacket_r);
}

static int AudDrv_BTCVSD_IRQ_handler(void)
{
	kal_uint32 uPacketType, uPacketNumber, uPacketLength, uBufferCount_TX, uBufferCount_RX, uControl;
	kal_uint32 uIsBtsysOff;
	kal_uint16 i,j;
	kal_uint32 connsys_addr_tx, ap_addr_tx, connsys_addr_rx, ap_addr_rx;

	if( (btsco.uRXState != BT_SCO_RXSTATE_RUNNING && btsco.uRXState != BT_SCO_RXSTATE_ENDING)
		&&(btsco.uTXState != BT_SCO_TXSTATE_RUNNING && btsco.uTXState != BT_SCO_TXSTATE_ENDING)
		&&(btsco.uTXState != BT_SCO_TXSTATE_DIRECT_LOOPBACK))
	{
		printk("AudDrv_BTCVSD_IRQ_handler: btsco.uRXState: %d, btsco.uTXState: %d, disableBTirq: %d\n (idle)", btsco.uRXState, btsco.uTXState, disableBTirq);
		*bt_hw_REG_CONTROL &= ~BT_CVSD_CLEAR;
		goto AudDrv_BTCVSD_IRQ_handler_exit;
	}
	uControl = *bt_hw_REG_CONTROL;
	connsys_addr_rx = *bt_hw_REG_PACKET_R;
	connsys_addr_tx = *bt_hw_REG_PACKET_W;
	ap_addr_rx = BTSYS_SRAM_BANK2_BASE_ADDRESS + (connsys_addr_rx & 0xFFFF);
	ap_addr_tx = BTSYS_SRAM_BANK2_BASE_ADDRESS + (connsys_addr_tx & 0xFFFF);

	uPacketType = (uControl >> 18) & 0x7;
	printk("AudDrv_BTCVSD_IRQ_handler: btsco.uRXState: %d, btsco.uTXState: %d, uControl =0x%x, ap_addr_rx =0x%x, ap_addr_tx =0x%x, disableBTirq: %d\n",
		btsco.uRXState, btsco.uTXState, uControl, ap_addr_rx, ap_addr_tx, disableBTirq);
	if( ((uControl>>31) & 1) == 0)
	{
		*bt_hw_REG_CONTROL &= ~BT_CVSD_CLEAR;
		goto AudDrv_BTCVSD_IRQ_handler_exit;
	}
	ASSERT(uPacketType < BT_SCO_CVSD_MAX);
	uPacketLength	= (kal_uint32)btsco_PacketInfo[uPacketType][0];
	uPacketNumber	= (kal_uint32)btsco_PacketInfo[uPacketType][1];
	uBufferCount_TX = (kal_uint32)btsco_PacketInfo[uPacketType][2];
	uBufferCount_RX = (kal_uint32)btsco_PacketInfo[uPacketType][3];
	if(btsco.pTX && btsco.uTXState == BT_SCO_TXSTATE_DIRECT_LOOPBACK)
	{
		kal_uint8 *pSrc, *pDst;
		pSrc = (kal_uint8 *)ap_addr_rx;
		pDst = (kal_uint8 *)ap_addr_tx;
		AudDrv_BTCVSD_DataTransfer(BT_SCO_DIRECT_BT2ARM, pSrc, btsco.pTX->TempPacketBuf, uPacketLength, uPacketNumber, BT_SCO_RXSTATE_RUNNING);
		AudDrv_BTCVSD_DataTransfer(BT_SCO_DIRECT_ARM2BT, btsco.pTX->TempPacketBuf, pDst, uPacketLength, uPacketNumber, BT_SCO_TXSTATE_RUNNING);
	}
	else
	{
		if(btsco.pRX)
		{
			if(btsco.uRXState == BT_SCO_RXSTATE_RUNNING || btsco.uRXState == BT_SCO_RXSTATE_ENDING)
			{
				//printk("AudDrv_BTCVSD_IRQ_handler pRX->fOverflow=%d, pRX->iPacket_w=%d, pRX->iPacket_r=%d, uBufferCount_RX=%d \n",btsco.pRX->fOverflow, btsco.pRX->iPacket_w, btsco.pRX->iPacket_r, uBufferCount_RX);
				if(btsco.pRX->fOverflow)
				{
					if(btsco.pRX->iPacket_w - btsco.pRX->iPacket_r <= SCO_RX_PACKER_BUF_NUM - 2*uBufferCount_RX) 
					{
						//free space is larger then twice interrupt rx data size
						btsco.pRX->fOverflow = KAL_FALSE;
						//printk("AudDrv_BTCVSD_IRQ_handler pRX->fOverflow FALSE!!! \n");
					}  
				}
				if(!btsco.pRX->fOverflow && (btsco.pRX->iPacket_w - btsco.pRX->iPacket_r <= SCO_RX_PACKER_BUF_NUM - uBufferCount_RX))
				{
					AudDrv_BTCVSD_ReadFromBT(uPacketType, uPacketLength, uPacketNumber, uBufferCount_RX, uControl, ap_addr_rx);
				}
				else
				{
					btsco.pRX->fOverflow = KAL_TRUE;
					printk("AudDrv_BTCVSD_IRQ_handler pRX->fOverflow TRUE!!! \n");
				}
			}
		}
		if(btsco.pTX)
		{
			if(btsco.uTXState == BT_SCO_TXSTATE_RUNNING || btsco.uTXState == BT_SCO_TXSTATE_ENDING)
			{  	
				//printk("AudDrv_BTCVSD_IRQ_handler pTX->fUnderflow=%d, pTX->iPacket_w=%d, pTX->iPacket_r=%d, uBufferCount_TX=%d \n", btsco.pTX->fUnderflow, btsco.pTX->iPacket_w, btsco.pTX->iPacket_r, uBufferCount_TX);
				if(btsco.pTX->fUnderflow)
				{
					//prepared data is larger then twice interrupt tx data size
					if(btsco.pTX->iPacket_w - btsco.pTX->iPacket_r >= 2*uBufferCount_TX)
					{
						btsco.pTX->fUnderflow = KAL_FALSE;
						//printk("AudDrv_BTCVSD_IRQ_handler pTX->fUnderflow FALSE!!! \n");
					}			
				}
				if(btsco.pTX->iPacket_w - btsco.pTX->iPacket_r < uBufferCount_TX)
				{
				   btsco.pTX->fUnderflow = KAL_TRUE;
				   printk("AudDrv_BTCVSD_IRQ_handler pTX->fUnderflow TRUE!!! \n");
				}
				AudDrv_BTCVSD_WriteToBT(uPacketType, uPacketLength, uPacketNumber, uBufferCount_TX, ap_addr_tx);
			}
		}
	}	    
 	*bt_hw_REG_CONTROL &= ~BT_CVSD_CLEAR;
	BTCVSD_read_wait_queue_flag =1;
	wake_up_interruptible(&BTCVSD_Read_Wait_Queue);
	BTCVSD_write_wait_queue_flag =1;
	wake_up_interruptible(&BTCVSD_Write_Wait_Queue);
	AudDrv_BTCVSD_IRQ_handler_exit:
	return IRQ_HANDLED;
}

static int AudDrv_btcvsd_probe(struct platform_device *dev)
{
   int ret = 0;
   //xlog_printk(ANDROID_LOG_INFO, "Sound","AudDrv_btcvsd_probe \n");
   printk("AudDrv_btcvsd_probe \n");

   //printk("TODO: +request_irq \n");
	
   printk("AudDrv_btcvsd_probe disable BT IRQ disableBTirq = %d",disableBTirq);
    ret = request_irq(MT_BT_CVSD_IRQ_ID, AudDrv_BTCVSD_IRQ_handler,IRQF_TRIGGER_LOW/*IRQF_TRIGGER_FALLING*/, "BTCVSD_ISR_Handle", dev);
    if(ret < 0 ){
       printk("AudDrv_btcvsd_probe request_irq MT_BT_CVSD_IRQ_ID Fail \n");
    }
	if(disableBTirq==0)
	{
		disable_irq(MT_BT_CVSD_IRQ_ID);
		Disable_CVSD_Wakeup();
		disableBTirq = 1;
	}
   
	// init
	memset((void*)&BT_CVSD_Mem,0,sizeof(CVSD_MEMBLOCK_T));

  	memset((void*)&btsco,0,sizeof(btsco));
	btsco.uTXState = BT_SCO_TXSTATE_IDLE;
	btsco.uRXState = BT_SCO_RXSTATE_IDLE;
	
	// ioremap to BT HW register base address
	BTSYS_PKV_BASE_ADDRESS = ioremap_nocache(AUDIO_BTSYS_PKV_PHYSICAL_BASE,0x10000); 
	BTSYS_SRAM_BANK2_BASE_ADDRESS =ioremap_nocache(AUDIO_BTSYS_SRAM_BANK2_PHYSICAL_BASE,0x10000);
 	printk("BTSYS_PKV_BASE_ADDRESS = %p BTSYS_SRAM_BANK2_BASE_ADDRESS = %p\n",BTSYS_PKV_BASE_ADDRESS,BTSYS_SRAM_BANK2_BASE_ADDRESS);
	bt_hw_REG_PACKET_W = (volatile kal_uint32*)(BTSYS_PKV_BASE_ADDRESS + 0x0FD4);
	bt_hw_REG_PACKET_R = (volatile kal_uint32*)(BTSYS_PKV_BASE_ADDRESS + 0x0FD0);
	bt_hw_REG_CONTROL = (volatile kal_uint32*)(BTSYS_PKV_BASE_ADDRESS + 0x0FD8);
	
   printk("-AudDrv_btcvsd_probe \n");
   return 0;
}


static int AudDrv_btcvsd_open(struct inode *inode, struct file *fp)
{
	 xlog_printk(ANDROID_LOG_INFO, "Sound","AudDrv_btcvsd_open do nothing inode:%p, file:%p \n",inode,fp);
    return 0;
}

static ssize_t AudDrv_btcvsd_write(struct file *fp, const char __user *data, size_t count, loff_t *offset)
{
	 int written_size = count ,ret =0, copy_size = 0, BTSCOTX_WriteIdx;
	 unsigned long flags;
	 char *data_w_ptr = (char*)data;
	 kal_uint64 write_timeout_limit; 
	
	 if( (btsco.pTX == NULL) || (btsco.pTX->PacketBuf == NULL) || (btsco.pTX->u4BufferSize == 0)){
		  printk("AudDrv_btcvsd_write btsco.pTX == NULL || btsco.pTX->PacketBuf == NULL || (btsco.pTX->u4BufferSize == 0 !!! \n");
		  msleep(60);
		  return written_size;
	 }
	 
	write_timeout_limit = ((kal_uint64)SCO_TX_PACKER_BUF_NUM * SCO_TX_ENCODE_SIZE * 16 * 1000000000)/2/2/64000; //ns
	
	while(count)
	{
		//printk("AudDrv_btcvsd_write btsco.pTX->iPacket_w=%d, btsco.pTX->iPacket_r=%d \n",btsco.pTX->iPacket_w, btsco.pTX->iPacket_r);
		
		 spin_lock_irqsave(&auddrv_BTCVSDTX_lock, flags);
		 copy_size = btsco.pTX->u4BufferSize - (btsco.pTX->iPacket_w - btsco.pTX->iPacket_r)*SCO_TX_ENCODE_SIZE;  //  free space of TX packet buffer
		 spin_unlock_irqrestore(&auddrv_BTCVSDTX_lock, flags);
		 if(count <= (kal_uint32) copy_size)
		 {
			  copy_size = count;
		 }
		 //printk("AudDrv_btcvsd_write count=%d, copy_size=%d \n",count, copy_size);
	
		 ASSERT(copy_size%SCO_TX_ENCODE_SIZE==0); //copysize must be multiple of SCO_TX_ENCODE_SIZE
	
		 if(copy_size != 0)
		 {
			  spin_lock_irqsave(&auddrv_BTCVSDTX_lock, flags);
			  BTSCOTX_WriteIdx = (btsco.pTX->iPacket_w & SCO_TX_PACKET_MASK) * SCO_TX_ENCODE_SIZE;
			  spin_unlock_irqrestore(&auddrv_BTCVSDTX_lock, flags);
	
			  if(BTSCOTX_WriteIdx + copy_size < btsco.pTX->u4BufferSize ) // copy once
			  {
					if(!access_ok(VERIFY_READ,data_w_ptr,copy_size))
					{
						 printk("AudDrv_btcvsd_write 0ptr invalid data_w_ptr=%x, size=%d \n",(kal_uint32)data_w_ptr,copy_size);
						 printk("AudDrv_btcvsd_write u4BufferSize=%d, BTSCOTX_WriteIdx=%d \n",btsco.pTX->u4BufferSize, BTSCOTX_WriteIdx);
					}
					else
					{
						 //PRINTK_AUDDRV("mcmcpy btsco.pTX->PacketBuf+BTSCOTX_WriteIdx= %x data_w_ptr = %p copy_size = %x\n", btsco.pTX->PacketBuf+BTSCOTX_WriteIdx,data_w_ptr,copy_size);
						if(copy_from_user((void *)((kal_uint8 *)btsco.pTX->PacketBuf+BTSCOTX_WriteIdx),(const void __user *)data_w_ptr,copy_size))
						{
							printk( "AudDrv_btcvsd_write Fail copy_from_user \n");
							return -1;
						}
					}
	
					 spin_lock_irqsave(&auddrv_BTCVSDTX_lock, flags);
					 btsco.pTX->iPacket_w += copy_size/SCO_TX_ENCODE_SIZE;
					 spin_unlock_irqrestore(&auddrv_BTCVSDTX_lock, flags);
					 data_w_ptr += copy_size;
					 count -= copy_size;
					
					 //printk("AudDrv_btcvsd_write finish1, copy_size:%d, pTX->iPacket_w:%d, pTX->iPacket_r=%d, count=%d \r\n",  copy_size,btsco.pTX->iPacket_w,btsco.pTX->iPacket_r,count);
			  }
			  else	// copy twice
			  {
					kal_int32 size_1 = 0,size_2 = 0;
					size_1 = btsco.pTX->u4BufferSize - BTSCOTX_WriteIdx;
					size_2 = copy_size - size_1;
					//printk("AudDrv_btcvsd_write size_1=%d, size_2=%d \n",size_1,size_2);
					ASSERT(size_1%SCO_TX_ENCODE_SIZE==0);
					ASSERT(size_2%SCO_TX_ENCODE_SIZE==0);
					if(!access_ok (VERIFY_READ,data_w_ptr,size_1))
					{
						 printk("AudDrv_btcvsd_write 1ptr invalid data_w_ptr=%x, size_1=%d \n",(kal_uint32)data_w_ptr,size_1);
						 printk("AudDrv_btcvsd_write u4BufferSize=%d, BTSCOTX_WriteIdx=%d \n",btsco.pTX->u4BufferSize, BTSCOTX_WriteIdx);
					}
					else
					{
						 //PRINTK_AUDDRV("mcmcpy btsco.pTX->PacketBuf+BTSCOTX_WriteIdx= %x data_w_ptr = %p size_1 = %x\n",  btsco.pTX->PacketBuf+BTSCOTX_WriteIdx,data_w_ptr,size_1);
						 if ((copy_from_user( (void *)((kal_uint8 *)btsco.pTX->PacketBuf+BTSCOTX_WriteIdx), (const void __user *)data_w_ptr, size_1)) )
						 {
							  printk("AudDrv_write Fail 1 copy_from_user \n");
							  return -1;
						 }
					}
					spin_lock_irqsave(&auddrv_BTCVSDTX_lock, flags);
					btsco.pTX->iPacket_w += size_1/SCO_TX_ENCODE_SIZE;
					spin_unlock_irqrestore(&auddrv_BTCVSDTX_lock, flags);
	
					if(!access_ok (VERIFY_READ,data_w_ptr+size_1, size_2))
					{
						 printk("AudDrv_btcvsd_write 2ptr invalid data_w_ptr=%x, size_1=%d, size_2=%d \n",(kal_uint32)data_w_ptr,size_1,size_2);
						 printk("AudDrv_btcvsd_write u4BufferSize=%d, pTX->iPacket_w=%d \n",btsco.pTX->u4BufferSize, btsco.pTX->iPacket_w);
					}
					else
					{
						 //PRINTK_AUDDRV("mcmcpy btsco.pTX->PacketBuf+BTSCOTX_WriteIdx+size_1= %x data_w_ptr+size_1 = %p size_2 = %x\n", btsco.pTX->PacketBuf+BTSCOTX_WriteIdx+size_1,data_w_ptr+size_1,size_2);
						 if ((copy_from_user((void *)((kal_uint8 *)btsco.pTX->PacketBuf),(const void __user *)(data_w_ptr+size_1), size_2)))
						 {
							  printk("AudDrv_btcvsd_write Fail 2 copy_from_user \n");
							  return -1;
						 }
					}
	
					spin_lock_irqsave(&auddrv_BTCVSDTX_lock, flags);\
					btsco.pTX->iPacket_w += size_2/SCO_TX_ENCODE_SIZE;
					spin_unlock_irqrestore(&auddrv_BTCVSDTX_lock,flags);
					count -= copy_size;
					data_w_ptr += copy_size;
					//printk("AudDrv_btcvsd_write finish2, copy size:%d, pTX->iPacket_w=%d,pTX->iPacket_r=%d, count:%d \r\n", copy_size,btsco.pTX->iPacket_w,btsco.pTX->iPacket_r,count );
			  }
		 }
		 else
		 {
			  //PRINTK_AUDDRV("AudDrv_btcvsd_write copy_size =0,	pTX->iPacket_w:%x,pTX->iPacket_r=%x count:%x \r\n",	btsco.pTX->iPacket_w,btsco.pTX->iPacket_r,count );
		 }
	
		 if(count != 0)
		 {
			  kal_uint64 t1,t2;
			  //printk("AudDrv_btcvsd_write WAITING...btsco.pTX->iPacket_w=%d, count=%d \n",btsco.pTX->iPacket_w,count);
			  t1 = sched_clock();
			  BTCVSD_write_wait_queue_flag=0;
			  ret = wait_event_interruptible_timeout(BTCVSD_Write_Wait_Queue, BTCVSD_write_wait_queue_flag,write_timeout_limit/1000000/10);
			  t2 = sched_clock();
			  //printk("AudDrv_btcvsd_write WAKEUP...count=%d \n",count);
			  t2 = t2 -t1; // in ns (10^9)
			  if(t2 > write_timeout_limit)
			  {
					printk("AudDrv_btcvsd_write timeout, [Warning](%llu)ns, write_timeout_limit(%llu)\n",t2, write_timeout_limit);
					return written_size;
			  }
		 }
		 // here need to wait for interrupt handler
	}
	printk("AudDrv_btcvsd_write written_size = %d, write_timeout_limit=%llu \n",written_size,write_timeout_limit);
	return written_size;
}


static ssize_t AudDrv_btcvsd_read(struct file *fp,  char __user *data, size_t count,loff_t *offset)
{
	 char *Read_Data_Ptr = (char*)data;
	 ssize_t ret ,DMA_Read_Ptr =0 , read_size = 0,read_count = 0, BTSCORX_ReadIdx_tmp;
	 kal_uint32 u4DataRemained;
	 unsigned long flags;
	 kal_uint64 read_timeout_limit;
	
	 if( (btsco.pRX == NULL) || (btsco.pRX->PacketBuf == NULL) || (btsco.pRX->u4BufferSize == 0)){
		  printk("AudDrv_btcvsd_read btsco.pRX == NULL || btsco.pRX->PacketBuf == NULL || btsco.pRX->u4BufferSize == 0!!! \n");
		  msleep(60);
		  return -1;
	 }
	 read_timeout_limit = ((kal_uint64)SCO_RX_PACKER_BUF_NUM * SCO_RX_PLC_SIZE * 16 * 1000000000)/2/2/64000;
	
	 while(count)
	 {
		 //printk("AudDrv_btcvsd_read btsco.pRX->iPacket_w=%d, btsco.pRX->iPacket_r=%d,count=%d \n",btsco.pRX->iPacket_w, btsco.pRX->iPacket_r,count);
		 
		  spin_lock_irqsave(&auddrv_BTCVSDRX_lock,flags);
		  u4DataRemained = (btsco.pRX->iPacket_w - btsco.pRX->iPacket_r)*(SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE);  //  available data in RX packet buffer
		  if(count > u4DataRemained){
				read_size = u4DataRemained;
		  }
		  else{
				read_size = count;
		  }
		  BTSCORX_ReadIdx_tmp = (btsco.pRX->iPacket_r& SCO_RX_PACKET_MASK) * (SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE);
		  spin_unlock_irqrestore(&auddrv_BTCVSDRX_lock, flags);
	
		  ASSERT(read_size%(SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE)==0 );
	
		  //printk("AudDrv_btcvsd_read read_size=%d, BTSCORX_ReadIdx_tmp=%d \n",read_size, BTSCORX_ReadIdx_tmp);
		  // PRINTK_AUDDRV("AudDrv_btcvsd_read finish0, read_count:%x, read_size:%x, u4DataRemained:%x, pRX->iPacket_r:0x%x, pRX->iPacket_w:%x \r\n", read_count,read_size,u4DataRemained,btsco.pRX->iPacket_r,btsco.pRX->iPacket_w);
		  if(BTSCORX_ReadIdx_tmp + read_size < btsco.pRX->u4BufferSize) //copy once
		  {
			//printk("AudDrv_btcvsd_read 1 copy_to_user target=0x%x, source=0x%x, read_size=%d \n",Read_Data_Ptr,((kal_uint8 *)btsco.pRX->PacketBuf+BTSCORX_ReadIdx_tmp),read_size);
				if(copy_to_user((void __user *)Read_Data_Ptr,(void *)((kal_uint8 *)btsco.pRX->PacketBuf+BTSCORX_ReadIdx_tmp),read_size))
				{
					 printk("AudDrv_btcvsd_read Fail 1 copy to user Read_Data_Ptr:%p, pRX->PacketBuf:%p, BTSCORX_ReadIdx_tmp:%d, read_size:%d",
					 Read_Data_Ptr,(kal_uint8 *)btsco.pRX->PacketBuf, BTSCORX_ReadIdx_tmp, read_size);
					 if(read_count == 0){
							return -1;
					 }
					 else {
							return read_count;
					  }
				 }
				 read_count += read_size;
				 spin_lock_irqsave(&auddrv_BTCVSDRX_lock,flags);
				 btsco.pRX->iPacket_r += read_size/(SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE); // 2 byte is packetvalid info
				 spin_unlock_irqrestore(&auddrv_BTCVSDRX_lock, flags);
	
				 Read_Data_Ptr += read_size;
				 count -= read_size;
				 //PRINTK_AUDDRV("AudDrv_btcvsd_read finish1, read_sizesize:%x, pRX->iPacket_r:0x%x, pRX->iPacket_w:%x, count:%x \r\n",	 read_size,btsco.pRX->iPacket_r,btsco.pRX->iPacket_w,count );
		  }
		  else //copy twice
		  {
				kal_uint32 size_1 = btsco.pRX->u4BufferSize - BTSCORX_ReadIdx_tmp;
				kal_uint32 size_2 = read_size - size_1;
				//printk("AudDrv_btcvsd_read 2-2 copy_to_user target=0x%x, source=0x%x, size_1=%d\n",Read_Data_Ptr,((kal_uint8 *)btsco.pRX->PacketBuf+BTSCORX_ReadIdx_tmp),size_1);
				if (copy_to_user( (void __user *)Read_Data_Ptr,(void *)((kal_uint8 *)btsco.pRX->PacketBuf+BTSCORX_ReadIdx_tmp),size_1))
				{
					 //PRINTK_AUDDRV("AudDrv_btcvsd_read Fail 2 copy to user Read_Data_Ptr:%p, pRX->PacketBuf:%p, BTSCORX_ReadIdx_tmp:0x%x, read_size:%x", Read_Data_Ptr,btsco.pRX->PacketBuf, BTSCORX_ReadIdx_tmp, read_size);
					 if(read_count == 0){
						  return -1;
					 }
					 else {
						  return read_count;
					 }
				}
				read_count += size_1;
				spin_lock_irqsave(&auddrv_BTCVSDRX_lock,flags);
				btsco.pRX->iPacket_r += size_1/(SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE); // 2 byte is packetvalid info
				spin_unlock_irqrestore(&auddrv_BTCVSDRX_lock,flags);
	
				//printk("AudDrv_btcvsd_read 2-2 copy_to_user target=0x%x, source=0x%x,size_2=%d\n",(Read_Data_Ptr+size_1),((kal_uint8 *)btsco.pRX->PacketBuf+BTSCORX_ReadIdx_tmp+size_1),size_2);
				if(copy_to_user((void __user *)(Read_Data_Ptr+size_1),(void *)((kal_uint8 *)btsco.pRX->PacketBuf), size_2))
				{
					 xlog_printk(ANDROID_LOG_ERROR, "Sound","AudDrv_btcvsd_read Fail 3 copy to user Read_Data_Ptr:%p, pRX->PacketBuf:%p, BTSCORX_ReadIdx_tmp:0x%x, read_size:%x", Read_Data_Ptr, btsco.pRX->PacketBuf, BTSCORX_ReadIdx_tmp, read_size);
					 if(read_count == 0){
						  return -1;
					 }
					 else {
						  return read_count;
					 }
				}
				read_count += size_2;
				spin_lock_irqsave(&auddrv_BTCVSDRX_lock,flags);
 				btsco.pRX->iPacket_r += size_2/(SCO_RX_PLC_SIZE+BTSCO_CVSD_PACKET_VALID_SIZE); // 2 byte is packetvalid info
 				spin_unlock_irqrestore(&auddrv_BTCVSDRX_lock, flags);
	
				count -= read_size;
				Read_Data_Ptr += read_size;
				//PRINTK_AUDDRV("AudDrv_btcvsd_read finish3, copy size_2:%x, pRX->iPacket_r:0x%x, pRX->iPacket_w:0x%x u4DataRemained:%x \r\n",	 size_2,btsco.pRX->iPacket_r,btsco.pRX->iPacket_w);
		 }
	
		 if(count != 0){
			kal_uint64 t1,t2;
	
			//printk("AudDrv_btcvsd_read WAITING... pRX->iPacket_r=%d, count=%d \n",btsco.pRX->iPacket_r,count);
			t1 = sched_clock();
			BTCVSD_read_wait_queue_flag =0;
			ret = wait_event_interruptible_timeout(BTCVSD_Read_Wait_Queue,BTCVSD_read_wait_queue_flag, read_timeout_limit/1000000/10);
			t2 = sched_clock(); 
			//printk("AudDrv_btcvsd_read WAKEUP...count=%d \n",count);
			t2 = t2 -t1; // in ns (10^9)
			if(t2 > read_timeout_limit)
			{
				printk("AudDrv_btcvsd_read timeout, [Warning](%llu)ns, read_timeout_limit(%llu)\n",t2, read_timeout_limit);
				return read_count;
			}											
		 }
	}
	printk("AudDrv_btcvsd_read read_count = %d,read_timeout_limit=%llu \n",read_count,read_timeout_limit);
	return read_count;
}


/**************************************************************************
 * STRUCT
 *  File Operations and misc device
 *
 **************************************************************************/

static struct file_operations AudDrv_btcvsd_fops = {
   .owner   = THIS_MODULE,
   .open    = AudDrv_btcvsd_open,
   //.release = AudDrv_btcvsd_release,
   .unlocked_ioctl   = AudDrv_btcvsd_ioctl,
   .write   = AudDrv_btcvsd_write,
   .read   	= AudDrv_btcvsd_read,
   //.flush   = AudDrv_btcvsd_flush,
   //.fasync	= AudDrv_btcvsd_fasync,
   //.mmap    = AudDrv_btcvsd_remap_mmap
};

static struct miscdevice AudDrv_btcvsd_device = {
   .minor = MISC_DYNAMIC_MINOR,
   .name = "ebc",
   .fops = &AudDrv_btcvsd_fops,
};

/***************************************************************************
 * FUNCTION
 *  AudDrv_btcvsd_mod_init / AudDrv_btcvsd_mod_exit
 *
 * DESCRIPTION
 *  Module init and de-init (only be called when system boot up)
 *
 **************************************************************************/

static struct platform_driver AudDrv_btcvsd = {
   .probe	 = AudDrv_btcvsd_probe,
   //.remove	 = AudDrv_btcvsd_remove,
   //.shutdown = AudDrv_btcvsd_shutdown,
   //.suspend	 = AudDrv_btcvsd_suspend,
   //.resume	 = AudDrv_btcvsd_resume,
   .driver   = {
       .name = auddrv_btcvsd_name,
       },
};

static struct platform_device AudDrv_device2 = {
    .name  = auddrv_btcvsd_name,
    .id    = 0,
    .dev   = {
        .dma_mask = &AudDrv_btcvsd_dmamask,
        .coherent_dma_mask =  0xffffffffUL
    }
};

static int AudDrv_btcvsd_mod_init(void)
{
    int ret = 0;
    printk("+AudDrv_btcvsd_mod_init \n");


    // Register platform DRIVER
    ret = platform_driver_register(&AudDrv_btcvsd);
    if(ret)
    {
        printk("AudDrv Fail:%d - Register DRIVER \n",ret);
        return ret;
    }
	 
    // register MISC device
    if((ret = misc_register(&AudDrv_btcvsd_device)))
    {
        printk("AudDrv_btcvsd_mod_init misc_register Fail:%d \n", ret);
        return ret;
    }

    printk("AudDrv_btcvsd_mod_init: Init Audio WakeLock\n");
    return 0;
}

static void  AudDrv_btcvsd_mod_exit(void)
{
    PRINTK_AUDDRV("+AudDrv_btcvsd_mod_exit \n");
    /*
    remove_proc_entry("audio", NULL);
    platform_driver_unregister(&AudDrv_btcvsd);
    */
    PRINTK_AUDDRV("-AudDrv_btcvsd_mod_exit \n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(AUDDRV_BTCVSD_NAME);
MODULE_AUTHOR(AUDDRV_AUTHOR);

module_init(AudDrv_btcvsd_mod_init);
module_exit(AudDrv_btcvsd_mod_exit);

