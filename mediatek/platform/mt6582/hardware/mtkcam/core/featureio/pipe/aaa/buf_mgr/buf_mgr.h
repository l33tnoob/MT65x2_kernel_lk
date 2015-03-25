/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
 * @file buf_mgr.h
 * @brief buffer manager
 */

#ifndef _BUF_MGR_H_
#define _BUF_MGR_H_

#include <utils/threads.h>
#include <list>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/imem_drv.h>

using namespace std;
using namespace android;

class IMemDrv;
class IspDrv;

namespace NS3A
{

/**  
 * @brief enum for 3A statistics DMA
 */

typedef enum
{
    ECamDMA_AAO  = 0, ///< AE and AWB statistics
    ECamDMA_AFO,      ///< AF tatistics    
    ECamDMA_NUM       ///< total number
} ECamDMA_T;

template<ECamDMA_T eCamDma> struct camdma2type { enum {v=eCamDma}; };

typedef IMEM_BUF_INFO BufInfo_T;

typedef list<BufInfo_T> BufInfoList_T;

/*******************************************************************************
*  AAO buffer
*******************************************************************************/
#define AE_STAT_SIZE (((AWB_WINDOW_NUM_X + 3) / 4 ) * 4 * AWB_WINDOW_NUM_Y)
#define AE_HIST_SIZE (4 * 256)
#define AWB_STAT_SIZE (AWB_WINDOW_NUM_X * AWB_WINDOW_NUM_Y * 4)
#define AAO_BUFFER_SIZE (AE_STAT_SIZE + AE_HIST_SIZE + AWB_STAT_SIZE)
#define AAO_OFFSET_ADDR (0)
#define AAO_XSIZE (AAO_BUFFER_SIZE - 1)
#define AAO_YSIZE (0)
#define AAO_STRIDE_BUS_SIZE (3)
//#define AAO_STRIDE_STRIDE (AAO_BUFFER_SIZE)

#define MAX_AAO_BUFFER_CNT (2)

/*******************************************************************************
*  AFO buffer
*******************************************************************************/
#define AF_HW_WIN 37
#define AF_WIN_DATA 8
#define AFO_BUFFER_SIZE (AF_WIN_DATA*AF_HW_WIN)
#define AFO_XSIZE (AFO_BUFFER_SIZE - 1)
#define MAX_AFO_BUFFER_CNT (1)

/**  
 * @brief 3A statistics buffer manager 
 */
class BufMgr
{

private:
    static BufInfoList_T m_rHwBufList[ECamDMA_NUM];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    BufMgr(BufMgr const&);
    //  Copy-assignment operator is disallowed.
    BufMgr& operator=(BufMgr const&);

public:  ////
    BufMgr();
    ~BufMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**  
     * @brief get instance
     */
    static BufMgr& getInstance();

    /**  
     * @brief init function
     */
    MRESULT init();

    /**  
     * @brief uninit function
     */    
    MRESULT uninit();

    /**  
     * @brief debug message print
     * @param [in] i4DmaChannel DMA channel 
     */ 
    MRESULT debugPrint(MINT32 i4DmaChannel);

    /**  
     * @brief enqueue HW buffer
     * @param [in] i4DmaChannel DMA channel 
     * @param [in] rBufInfo buffer information
     */
    MRESULT enqueueHwBuf(MINT32 i4DmaChannel, BufInfo_T& rBufInfo);

    /**  
     * @brief dequeue HW buffer
     * @param [in] i4DmaChannel DMA channel 
     * @param [in] rBufInfo buffer information
     */
    MRESULT dequeueHwBuf(MINT32 i4DmaChannel, BufInfo_T& rBufInfo);

    /**  
     * @brief get current HW buffer
     * @param [in] i4DmaChannel DMA channel 
     * @return physical memory address of current HW buffer
     */     
    MUINT32 getCurrHwBuf(MINT32 i4DmaChannel);

    /**  
     * @brief get next HW buffer
     * @param [in] i4DmaChannel DMA channel 
     * @return physical memory address of next HW buffer
     */ 
    MUINT32 getNextHwBuf(MINT32 i4DmaChannel);

    /**  
     * @brief allocate HW buffer
     * @param [in] rBufInfo buffer information 
     * @param [in] u4BufSize buffer size
     */
    MRESULT allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize);

    /**  
     * @brief free HW buffer
     * @param [in] rBufInfo buffer information 
     */
    MRESULT freeBuf(BufInfo_T &rBufInfo);

    /**  
     * @brief DMA config function
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA 
     */ 
    MRESULT DMAConfig(camdma2type<ECamDMA_AAO>);

    /**  
     * @brief update DMA base address
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA
     * @param [in] u4BaseAddr base address
     */
    MRESULT updateDMABaseAddr(camdma2type<ECamDMA_AAO>, MUINT32 u4BaseAddr);

    /**  
     * @brief DMA init function
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA 
     */
    MRESULT DMAInit(camdma2type<ECamDMA_AAO>);

    /**  
     * @brief DMA uninit function
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA 
     */
    MRESULT DMAUninit(camdma2type<ECamDMA_AAO>);

    /**  
     * @brief AA statistics enable function
     * @param [in] En TRUE: enable, FALSE: disable 
     */
    MRESULT AAStatEnable(MBOOL En);

    /**  
     * @brief DMA config function
     * @param [in] camdma2type<ECamDMA_AFO> AFO DMA 
     */
    MRESULT DMAConfig(camdma2type<ECamDMA_AFO>);

    /**  
     * @brief update DMA base address
     * @param [in] camdma2type<ECamDMA_AFO> AFO DMA
     * @param [in] u4BaseAddr base address
     */
    MRESULT updateDMABaseAddr(camdma2type<ECamDMA_AFO>, MUINT32 u4BaseAddr);

    /**  
     * @brief DMA uninit function
     * @param [in] camdma2type<ECamDMA_AFO> AFO DMA 
     */
    MRESULT DMAInit(camdma2type<ECamDMA_AFO>);

    /**  
     * @brief DMA uninit function
     * @param [in] camdma2type<ECamDMA_AFO> AFO DMA 
     */
    MRESULT DMAUninit(camdma2type<ECamDMA_AFO>);

    /**  
     * @brief AF statistics enable function
     * @param [in] En TRUE: enable, FALSE: disable 
     */
    MRESULT AFStatEnable(MBOOL En);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IMemDrv *m_pIMemDrv;
    IspDrv*                m_pIspDrv;
    isp_reg_t*             m_pIspReg;
    IspDrv*                m_pVirtIspDrvCQ0;
    isp_reg_t*             m_pVirtIspRegCQ0;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL                  m_bDebugEnable;

    BufInfo_T m_rAAOBufInfo[MAX_AAO_BUFFER_CNT];
    BufInfo_T m_rAFOBufInfo[MAX_AFO_BUFFER_CNT];
    
};

};  //  namespace NS3A
#endif // _BUF_MGR_H_

