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

//! \file  mdp_mgr_drv.h

#ifndef _MDP_MGR_IMP_H_
#define _MDP_MGR_IMP_H_

#include "mdp_mgr.h"

#define DPSTREAM_NUM ((NSImageio::NSIspio::eScenarioID_CONFIG_FMT) + 1)

/**  
*@enum MDPMGR_PLANE_ENUM
*/
typedef enum
{
    MDPMGR_PLANE_1,
    MDPMGR_PLANE_2,
    MDPMGR_PLANE_3
} MDPMGR_PLANE_ENUM;


/**
 *@class MdpMgrImp
 *@brief Implementation of MdpMgr
*/
class MdpMgrImp : public MdpMgr
{
    public :

        /**                       
              *@brief MdpMgrImp constructor
             */
        MdpMgrImp();

        /**                       
              *@brief MdpMgrImp destructor
             */
        virtual ~MdpMgrImp();
       
        /**                       
               *@brief Destory MdpMgrImp Object
             */
        virtual MVOID destroyInstance();

        /**                       
               *@brief Initialize function
               *@note Must call this function after createInstance and before other functions
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 init (MDPMGR_CFG_STRUCT cfgData);

        /**                       
               *@brief Uninitialize function               
               *@return
               *-MTRUE indicates success, otherwise indicates fail
            */
        virtual MINT32 uninit();

        /**                       
               *@brief Configure and trigger MDP 
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 startMdp(MDPMGR_CFG_STRUCT cfgData);

        /**                       
               *@brief Stop MDP 
               *
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 stopMdp(MDPMGR_CFG_STRUCT cfgData);

        /**                       
               *@brief Dequeue input or output buffer
               *
               *@param[in] deqIndex : indicate input or output
               *@param[in] cfgData : configure data
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 dequeueBuf(MDPMGR_DEQUEUE_INDEX deqIndex, MDPMGR_CFG_STRUCT cfgData);        
        
    private :        
        
        /**                       
               *@brief Covert ISP image format to DpFrameWork image format
               *
               *@param[in] leImgFmt : ISP image format
               *@param[in] lDpColorFormat :DpFrameWork image format
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 DpColorFmtConvert(EImageFormat ispImgFmt,DpColorFormat *dpColorFormat);

        /**                       
               *@brief Covert CDP DMA output format to DpFrameWork output format
               *
               *@param[in] dma_out : CdpRotDMACfg data
               *@param[in] lDpColorFormat : DpFrameWork image format
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32  DpDmaOutColorFmtConvert(CdpRotDMACfg dma_out,DpColorFormat *dpColorFormat);

        /**                       
               *@brief Chose correct DpIspStream object according to scenario ID and cmdQ index
               *
               *@param[in] sceID : scenario ID of ISP pipe
               *@param[in] cqIdx : index of pass2 cmdQ
               *
               *@return
               *-NULL indicates fail, otherwise indicates success
             */
        virtual DpIspStream *selectDpStream(MUINT32 sceID, MUINT32 cqIdx);

        /**                       
               *@brief Dump ISP_TPIPE_CONFIG_STRUCT info
               *
               *@param[in] a_info : ISP_TPIPE_CONFIG_STRUCT info
             */
        virtual MVOID dumpIspTPipeInfo(ISP_TPIPE_CONFIG_STRUCT a_info);

        /**************************************************************************************/

        mutable Mutex mLock;
        volatile int mUser;        
        
        DpIspStream *m_pCq1DpStream[DPSTREAM_NUM];
        DpIspStream *m_pCq2DpStream[DPSTREAM_NUM];
};

#endif

