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

#ifndef __EMI_HW_H__
#define __EMI_HW_H__

#include "mt6572.h"

#define EMI_base	 EMI_BASE
#define UINT32P unsigned int *

/* EMI DFS shadow registers */
#define EMI_DFSA                 ((volatile UINT32P)(EMI_base+0x0000))
#define EMI_DFSB                 ((volatile UINT32P)(EMI_base+0x0008))
#define EMI_DFSC                 ((volatile UINT32P)(EMI_base+0x0010))
#define EMI_DFSD                 ((volatile UINT32P)(EMI_base+0x0018))
#define EMI_DFSE                 ((volatile UINT32P)(EMI_base+0x0020))
#define EMI_DFSF                ((volatile UINT32P)(EMI_base+0x0028))

/*EMI DRAM control registers */
#define EMI_CONI                 ((volatile UINT32P)(EMI_base+0x0040))  
#define EMI_CONJ                 ((volatile UINT32P)(EMI_base+0x0048)) 
#define EMI_CONK                 ((volatile UINT32P)(EMI_base+0x0050))  
#define EMI_CONL                 ((volatile UINT32P)(EMI_base+0x0058))  
#define EMI_CONM                 ((volatile UINT32P)(EMI_base+0x0060))
#define EMI_CONN                 ((volatile UINT32P)(EMI_base+0x0068))

#define EMI_GENA                 ((volatile UINT32P)(EMI_base+0x0070))
#define EMI_DRCT                 ((volatile UINT32P)(EMI_base+0x0078))
#define EMI_DDRV                 ((volatile UINT32P)(EMI_base+0x0080))
#define EMI_GEND                 ((volatile UINT32P)(EMI_base+0x0088))
#define EMI_PPCT                 ((volatile UINT32P)(EMI_base+0x0090))

#define EMI_DLLV                 ((volatile UINT32P)(EMI_base+0x00A0))
#define EMI_DLLI                 ((volatile UINT32P)(EMI_base+0x00A8))

#define EMI_DFTC                 ((volatile UINT32P)(EMI_base+0x00F0))
#define EMI_DFTD                 ((volatile UINT32P)(EMI_base+0x00F8))

#define EMI_ARBA                 ((volatile UINT32P)(EMI_base+0x0100))
#define EMI_ARBB                 ((volatile UINT32P)(EMI_base+0x0108))
#define EMI_ARBC                 ((volatile UINT32P)(EMI_base+0x0110))
#define EMI_ARBD                 ((volatile UINT32P)(EMI_base+0x0118))
#define EMI_ARBE                 ((volatile UINT32P)(EMI_base+0x0120))
#define EMI_ARBF                 ((volatile UINT32P)(EMI_base+0x0128))
#define EMI_ARBG                 ((volatile UINT32P)(EMI_base+0x0130))

#define EMI_SLCT                 ((volatile UINT32P)(EMI_base+0x0150))
#define EMI_ABCT	             ((volatile UINT32P)(EMI_base+0x0158))

#define EMI_IDLA                 ((volatile UINT32P)(EMI_base+0x0200))
#define EMI_IDLB                 ((volatile UINT32P)(EMI_base+0x0208))
#define EMI_IDLC                 ((volatile UINT32P)(EMI_base+0x0210))
#define EMI_IDLD                 ((volatile UINT32P)(EMI_base+0x0218))
#define EMI_IDLE                 ((volatile UINT32P)(EMI_base+0x0220))
#define EMI_IDLF                 ((volatile UINT32P)(EMI_base+0x0228))
#define EMI_IDLG                 ((volatile UINT32P)(EMI_base+0x0230))
#define EMI_IDLH                 ((volatile UINT32P)(EMI_base+0x0238))
#define EMI_IDLI                 ((volatile UINT32P)(EMI_base+0x0240))
#define EMI_IDLJ                 ((volatile UINT32P)(EMI_base+0x0248))

#define EMI_ODLA	             ((volatile UINT32P)(EMI_base+0x0258))
#define EMI_ODLB	             ((volatile UINT32P)(EMI_base+0x0260))
#define EMI_ODLC	             ((volatile UINT32P)(EMI_base+0x0268))
#define EMI_ODLD	             ((volatile UINT32P)(EMI_base+0x0270))
#define EMI_ODLE	             ((volatile UINT32P)(EMI_base+0x0278))
#define EMI_ODLF	             ((volatile UINT32P)(EMI_base+0x0280))
#define EMI_ODLG	             ((volatile UINT32P)(EMI_base+0x0288))
#define EMI_ODLH	             ((volatile UINT32P)(EMI_base+0x0290))
#define EMI_ODLI	                    ((volatile UINT32P)(EMI_base+0x0298))
#define EMI_ODLJ	                   ((volatile UINT32P)(EMI_base+0x02A0))
#define EMI_ODLK	             ((volatile UINT32P)(EMI_base+0x02A8))
#define EMI_ODLL	             ((volatile UINT32P)(EMI_base+0x02B0))
#define EMI_ODLM	             ((volatile UINT32P)(EMI_base+0x02B8))
#define EMI_ODLN	             ((volatile UINT32P)(EMI_base+0x02C0))

#define EMI_DUTA	             ((volatile UINT32P)(EMI_base+0x0300))
#define EMI_DUTB                 ((volatile UINT32P)(EMI_base+0x0308))
#define EMI_DUTC	             ((volatile UINT32P)(EMI_base+0x0310))
#define EMI_DRVA                 ((volatile UINT32P)(EMI_base+0x0318))    
#define EMI_DRVB                 ((volatile UINT32P)(EMI_base+0x0320))    
#define EMI_IOCL                 ((volatile UINT32P)(EMI_base+0x0328))   
//#define EMI_IOCM                 ((volatile UINT32P)(EMI_base+0x0330))   

#define EMI_ODTA                ((volatile UINT32P)(EMI_base+0x0360))   
#define EMI_ODTB                 ((volatile UINT32P)(EMI_base+0x0368))   

#define EMI_BMEN                 ((volatile UINT32P)(EMI_base+0x0400))
#define EMI_BCNT                 ((volatile UINT32P)(EMI_base+0x0408))
#define EMI_TACT                 ((volatile UINT32P)(EMI_base+0x0410))
#define EMI_TSCT                 ((volatile UINT32P)(EMI_base+0x0418))
#define EMI_WACT                 ((volatile UINT32P)(EMI_base+0x0420))
#define EMI_WSCT                 ((volatile UINT32P)(EMI_base+0x0428))
#define EMI_BACT                 ((volatile UINT32P)(EMI_base+0x0430))
#define EMI_BSCT                 ((volatile UINT32P)(EMI_base+0x0438))
#define EMI_MSEL                 ((volatile UINT32P)(EMI_base+0x0440))
#define EMI_TSCT2                ((volatile UINT32P)(EMI_base+0x0448))
#define EMI_TSCT3                ((volatile UINT32P)(EMI_base+0x0450))
#define EMI_WSCT2                ((volatile UINT32P)(EMI_base+0x0458))
#define EMI_WSCT3                ((volatile UINT32P)(EMI_base+0x0460))
#define EMI_MSEL2                ((volatile UINT32P)(EMI_base+0x0468))
#define EMI_MSEL3                ((volatile UINT32P)(EMI_base+0x0470))
#define EMI_MSEL4                ((volatile UINT32P)(EMI_base+0x0478))
#define EMI_MSEL5                ((volatile UINT32P)(EMI_base+0x0480))
#define EMI_MSEL6                ((volatile UINT32P)(EMI_base+0x0488))
#define EMI_MSEL7                ((volatile UINT32P)(EMI_base+0x0490))
#define EMI_MSEL8                ((volatile UINT32P)(EMI_base+0x0498))
#define EMI_MSEL9                ((volatile UINT32P)(EMI_base+0x04A0))
#define EMI_MSEL10                ((volatile UINT32P)(EMI_base+0x04A8))
#define EMI_BMID0                ((volatile UINT32P)(EMI_base+0x04B0))
#define EMI_BMID1                ((volatile UINT32P)(EMI_base+0x04B8))
#define EMI_BMID2                ((volatile UINT32P)(EMI_base+0x04C0))
#define EMI_BMID3                ((volatile UINT32P)(EMI_base+0x04C8))
#define EMI_BMID4                ((volatile UINT32P)(EMI_base+0x04D0))
#define EMI_BMID5                ((volatile UINT32P)(EMI_base+0x04D8))

#define EMI_TTYPE1               ((volatile UINT32P)(EMI_base+0x0500))
#define EMI_TTYPE2               ((volatile UINT32P)(EMI_base+0x0508))
#define EMI_TTYPE3               ((volatile UINT32P)(EMI_base+0x0510))
#define EMI_TTYPE4               ((volatile UINT32P)(EMI_base+0x0518))
#define EMI_TTYPE5               ((volatile UINT32P)(EMI_base+0x0520))
#define EMI_TTYPE6               ((volatile UINT32P)(EMI_base+0x0528))
#define EMI_TTYPE7               ((volatile UINT32P)(EMI_base+0x0530))
#define EMI_TTYPE8               ((volatile UINT32P)(EMI_base+0x0538))
#define EMI_TTYPE9               ((volatile UINT32P)(EMI_base+0x0540))
#define EMI_TTYPE10              ((volatile UINT32P)(EMI_base+0x0548))
#define EMI_TTYPE11              ((volatile UINT32P)(EMI_base+0x0550))
#define EMI_TTYPE12              ((volatile UINT32P)(EMI_base+0x0558))
#define EMI_TTYPE13              ((volatile UINT32P)(EMI_base+0x0560))
#define EMI_TTYPE14              ((volatile UINT32P)(EMI_base+0x0568))
#define EMI_TTYPE15              ((volatile UINT32P)(EMI_base+0x0570))
#define EMI_TTYPE16              ((volatile UINT32P)(EMI_base+0x0578))
#define EMI_TTYPE17              ((volatile UINT32P)(EMI_base+0x0580))
#define EMI_TTYPE18              ((volatile UINT32P)(EMI_base+0x0588))
#define EMI_TTYPE19              ((volatile UINT32P)(EMI_base+0x0590))
#define EMI_TTYPE20              ((volatile UINT32P)(EMI_base+0x0598))
#define EMI_TTYPE21              ((volatile UINT32P)(EMI_base+0x05A0))
#define EMI_BSCT2                ((volatile UINT32P)(EMI_base+0x05A8)) 
#define EMI_BSCT3                ((volatile UINT32P)(EMI_base+0x05B0)) 

#define EMI_RBSELA                ((volatile UINT32P)(EMI_base+0x05F0)) 

#define EMI_MBISTA               ((volatile UINT32P)(EMI_base+0x0600))
#define EMI_MBISTB               ((volatile UINT32P)(EMI_base+0x0608))
#define EMI_MBISTC               ((volatile UINT32P)(EMI_base+0x0610))
#define EMI_MBISTD               ((volatile UINT32P)(EMI_base+0x0618))
#define EMI_MBISTE               ((volatile UINT32P)(EMI_base+0x0620))

#define EMI_RFCA                 ((volatile UINT32P)(EMI_base+0x0630))
#define EMI_RFCB                 ((volatile UINT32P)(EMI_base+0x0638))

#define EMI_DQSA                 ((volatile UINT32P)(EMI_base+0x0700))
#define EMI_DQSB                 ((volatile UINT32P)(EMI_base+0x0708))
#define EMI_DQSC                 ((volatile UINT32P)(EMI_base+0x0710))
#define EMI_DQSD                 ((volatile UINT32P)(EMI_base+0x0718))
#define EMI_DQSI                 ((volatile UINT32P)(EMI_base+0x0740))
#define EMI_DQSU                 ((volatile UINT32P)(EMI_base+0x0748))
#define EMI_DQSV                 ((volatile UINT32P)(EMI_base+0x0750))

#define EMI_CALA                 ((volatile UINT32P)(EMI_base+0x0758))
#define EMI_CALB                 ((volatile UINT32P)(EMI_base+0x0760))

#define EMI_CALE                 ((volatile UINT32P)(EMI_base+0x0778))
#define EMI_CALF                 ((volatile UINT32P)(EMI_base+0x0780))

#define EMI_CALI                 ((volatile UINT32P)(EMI_base+0x0798))
#define EMI_CALJ                 ((volatile UINT32P)(EMI_base+0x07A0))
#define EMI_CALM                 ((volatile UINT32P)(EMI_base+0x07B8))
#define EMI_CALN                 ((volatile UINT32P)(EMI_base+0x07C0))
#define EMI_CALO                 ((volatile UINT32P)(EMI_base+0x07C8))
#define EMI_CALP                 ((volatile UINT32P)(EMI_base+0x07D0))

#define EMI_DUCA                 ((volatile UINT32P)(EMI_base+0x07D8))
#define EMI_DUCB                 ((volatile UINT32P)(EMI_base+0x07E0))
#define EMI_DUCC                 ((volatile UINT32P)(EMI_base+0x07E8))
#define EMI_DUCD                 ((volatile UINT32P)(EMI_base+0x07F0))
#define EMI_DUCE                 ((volatile UINT32P)(EMI_base+0x07F8))

#define EMI_ADSA                 ((volatile UINT32P)(EMI_base+0x0800))
#define EMI_ADSB                 ((volatile UINT32P)(EMI_base+0x0808))
#define EMI_ADSC                 ((volatile UINT32P)(EMI_base+0x0810))
#define EMI_ADSD                 ((volatile UINT32P)(EMI_base+0x0818))
#define EMI_ADSE                 ((volatile UINT32P)(EMI_base+0x0820))
#define EMI_ADSF                 ((volatile UINT32P)(EMI_base+0x0828))
#define EMI_ADSG                 ((volatile UINT32P)(EMI_base+0x0830))
#define EMI_ADSH                ((volatile UINT32P)(EMI_base+0x0838))
#define EMI_ADSI                 ((volatile UINT32P)(EMI_base+0x0840))
#define EMI_ADSJ                 ((volatile UINT32P)(EMI_base+0x0848))

#define EMI_REMAP                ((volatile UINT32P)(EMI_base+0x0070))  /* EMI Re-map Control Register */

#define EMI_EFUSE           ((volatile UINT32P)0x801C0104)  
#define DRAM_TYPE_MASK		0x00070000
#define DRAM_TYPE_SDR		0x00000000
#define DRAM_TYPE_DDR		0x00020000
#define MD_MCU_CON0              ((volatile UINT32P)(CONFIG_base+0x0020)) 

/* SRAM repair */
#define MFG_RP_CON                ((volatile UINT32P)(0x13000100))
#define MFG_MBIST_MODE        ((volatile UINT32P)(0x13000060))
#define MFG_MBIST_DONE_0    ((volatile UINT32P)(0x13000090))
#define MFG_MBIST_DONE_1    ((volatile UINT32P)(0x13000094))
#define MFG_MBIST_DONE_2    ((volatile UINT32P)(0x13000098))
#define MFG_MBIST_DONE_3    ((volatile UINT32P)(0x1300009c))
#define MFG_MBIST_FAIL_0     ((volatile UINT32P)(0x130000a8))
#define MFG_MBIST_FAIL_1     ((volatile UINT32P)(0x130000ac))
#define MFG_MBIST_FAIL_2     ((volatile UINT32P)(0x130000b0))
#define MFG_MBIST_FAIL_3     ((volatile UINT32P)(0x130000b4))
#define MFG_RP_MON_0           ((volatile UINT32P)(0x13000104))

#define MDP_WROT_MBISR_RESET ((volatile UINT32P)(0x14000850))
#define MDP_WROT_MBIST_FAIL    ((volatile UINT32P)(0x14000854))
#define MDP_WROT_MBIST_OK       ((volatile UINT32P)(0x14000858))

#define MMSYS_MBIST_CON        ((volatile UINT32P)(0x14000820))
#define MMSYS_MBIST_MODE     ((volatile UINT32P)(0x14000810))
#define MMSYS_MBIST_DONE     ((volatile UINT32P)(0x14000800))
#define MMSYS_MBIST_FAIL0    ((volatile UINT32P)(0x14000804))    
#define MMSYS_MBIST_FAIL1    ((volatile UINT32P)(0x14000808))
                                                        
#define MEM_REPAIR     ((volatile UINT32P)(0x10001804))
                                                        
/* EMI_MPU */
#define EMI_MPUA ((volatile UINT32P)(EMI_BASE+0x0160))
#define EMI_MPUB ((volatile UINT32P)(EMI_BASE+0x0168))
#define EMI_MPUC ((volatile UINT32P)(EMI_BASE+0x0170))
#define EMI_MPUD ((volatile UINT32P)(EMI_BASE+0x0178))
#define EMI_MPUE ((volatile UINT32P)(EMI_BASE+0x0180))
#define EMI_MPUF ((volatile UINT32P)(EMI_BASE+0x0188))
#define EMI_MPUG ((volatile UINT32P)(EMI_BASE+0x0190))
#define EMI_MPUH ((volatile UINT32P)(EMI_BASE+0x0198))
#define EMI_MPUI ((volatile UINT32P)(EMI_BASE+0x01A0))
#define EMI_MPUJ ((volatile UINT32P)(EMI_BASE+0x01A8))
#define EMI_MPUK ((volatile UINT32P)(EMI_BASE+0x01B0))
#define EMI_MPUL ((volatile UINT32P)(EMI_BASE+0x01B8))
#define EMI_MPUM ((volatile UINT32P)(EMI_BASE+0x01C0))
#define EMI_MPUN ((volatile UINT32P)(EMI_BASE+0x01C8))
#define EMI_MPUP ((volatile UINT32P)(EMI_BASE+0x01D8))
#define EMI_MPUQ ((volatile UINT32P)(EMI_BASE+0x01E0))
#define EMI_MPUS ((volatile UINT32P)(EMI_BASE+0x01F0))
#define EMI_MPUT ((volatile UINT32P)(EMI_BASE+0x01F8))
#define EMI_MKEY ((volatile UINT32P)(EMI_BASE+0x0650))
#define EMI_MPSW ((volatile UINT32P)(EMI_BASE+0x0658))
#define EMI_MLST ((volatile UINT32P)(EMI_BASE+0x0660))


#endif	// __EMI_HW_H__
