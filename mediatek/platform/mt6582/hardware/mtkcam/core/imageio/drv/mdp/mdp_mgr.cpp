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

//! \file  mdp_mgr.cpp
 
#define LOG_TAG "MdpMgr"

#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>  // For property_get().

#include "imageio_types.h"  // For type definitions.
#include "isp_datatypes.h"
#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EScenarioID.
#include <mtkcam/drv/isp_drv.h>
#include "mdp_mgr_imp.h"

#include "DpDataType.h" // For DP_STATUS_ENUM

/*************************************************************************************
* Log Utility
*************************************************************************************/

#undef   DBG_LOG_TAG    // Decide a Log TAG for current file.
#define  DBG_LOG_TAG    "{MdpMgr} "

#include "imageio_log.h"    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(mdpmgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (mdpmgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

/**************************************************************************
* 
**************************************************************************/
MdpMgrImp::MdpMgrImp()
            :MdpMgr()
            ,mUser(0)            
{
    LOG_INF("+");
    
    for(MINT32 i = 0; i < DPSTREAM_NUM; i++)
    {
        m_pCq1DpStream[i] = NULL;
        m_pCq2DpStream[i] = NULL;
    }

    LOG_INF("-");
}

/**************************************************************************
* 
**************************************************************************/
MdpMgrImp::~MdpMgrImp()        
{
    LOG_INF("+"); 

    Mutex::Autolock lock(mLock);

    for(MINT32 i = 0; i < DPSTREAM_NUM; i++)
    {
        if(m_pCq1DpStream[i] != NULL)
        {
            delete m_pCq1DpStream[i];
            m_pCq1DpStream[i] = NULL;
        }
        
        if(m_pCq2DpStream[i] != NULL)
        {
            delete m_pCq2DpStream[i];
            m_pCq2DpStream[i] = NULL;
        }
    }

    LOG_INF("-");
}

/**************************************************************************
* 
**************************************************************************/
MdpMgr *MdpMgr::createInstance()
{
    LOG_INF("+");

    DBG_LOG_CONFIG(imageio, mdpmgr);
    static MdpMgrImp singleton;

    LOG_INF("-");    
    return &singleton;
}

/**************************************************************************
* 
**************************************************************************/
MVOID MdpMgrImp::destroyInstance()
{    
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::init(MDPMGR_CFG_STRUCT cfgData)
{
    LOG_INF("+,cqIndex(%u),sceID(%u)",cfgData.cqIdx,cfgData.sceID);
    
    Mutex::Autolock lock(mLock);

    MBOOL err = MDPMGR_NO_ERROR;  

    if(cfgData.cqIdx == ISP_DRV_CQ01)
    {
        if(m_pCq1DpStream[cfgData.sceID] != NULL)
        {
            LOG_INF("cq1 dpstream for scenario(%u) already exist",cfgData.sceID);
            return MDPMGR_NO_ERROR;
        } 
        else
        {
            switch(cfgData.sceID)
            {
                case NSImageio::NSIspio::eScenarioID_IC :
                    m_pCq1DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_IC_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_VR :
                    m_pCq1DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_VR_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_ZSD :
                    m_pCq1DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_IP:
                    m_pCq1DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_IP_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_VSS :
                    m_pCq1DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
                    break;
                default : LOG_ERR("cq1 DpStream not support this scenario(%u)",cfgData.sceID);  
                           err = MDPMGR_WRONG_PARAM;
            } 
        }
    }
    else if(cfgData.cqIdx == ISP_DRV_CQ02)
    {
        if(m_pCq2DpStream[cfgData.sceID] != NULL)
        {
            LOG_INF("cq2 dpstream for scenario(%u) already exist",cfgData.sceID);
            return MDPMGR_NO_ERROR;
        }
        else
        {
            switch(cfgData.sceID)
            {
                case NSImageio::NSIspio::eScenarioID_IC :
                    m_pCq2DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_IC_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_VR :
                    m_pCq2DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_VR_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_ZSD :
                    m_pCq2DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_IP:
                    m_pCq2DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_IP_STREAM);
                    break;
                case NSImageio::NSIspio::eScenarioID_VSS :
                    m_pCq2DpStream[cfgData.sceID] = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
                    break;
                default : LOG_ERR("cq2 DpStream not support this scenario(%u)",cfgData.sceID);
                           err = MDPMGR_WRONG_PARAM;
            } 
        }
    }    

    return err;    
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::uninit()
{
    LOG_DBG("+");

    return MDPMGR_NO_ERROR;
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::startMdp(MDPMGR_CFG_STRUCT cfgData)
{
    MINT32 err = MDPMGR_NO_ERROR;
    MUINT32 sceID = cfgData.sceID;
    DpIspStream *pDpStream = NULL;

    LOG_INF("+,cqIndx(%u),sceID(%u)",cfgData.cqIdx,sceID);
    
    //====== Select Correct DpIspStream ======

    pDpStream = selectDpStream(cfgData.sceID,cfgData.cqIdx);
    if(pDpStream == NULL)
    {
        LOG_ERR("pDpStream is NULL");
        return MDPMGR_NULL_OBJECT;
    } 

    //====== Configure Source ======

    DP_COLOR_ENUM srcFmt;

    // format convert
    err = DpColorFmtConvert(cfgData.srcFmt, &srcFmt);
    if(err != MDPMGR_NO_ERROR)
    {
        LOG_ERR("DpColorFmtConvert fail");
        return MDPMGR_API_FAIL;
    }

    // source image configure
    LOG_DBG("src,W(%u),H(%u),stride(%u)",cfgData.srcW,cfgData.srcH,cfgData.srcStride);
    
    err = pDpStream->setSrcConfig(srcFmt,cfgData.srcW ,cfgData.srcH,cfgData.srcStride);
    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setSrcConfig fail(%d)",err);
        return MDPMGR_API_FAIL;
    } 

    // source image memory configure
    LOG_DBG("src,VA(0x%8x),PA(0x%8x),size(0x%8x)",cfgData.srcVirAddr,cfgData.srcPhyAddr,cfgData.srcBufSize);

    if(cfgData.srcVirAddr == 0 || cfgData.srcPhyAddr == 0)
    {
        LOG_ERR("src memAddr is 0,VA(0x%8x),PA(0x%8x),size(0x%8x)",cfgData.srcVirAddr,cfgData.srcPhyAddr,cfgData.srcBufSize);
        return MDPMGR_NULL_OBJECT;
    }

    err = pDpStream->queueSrcBuffer((MVOID *)cfgData.srcVirAddr,cfgData.srcPhyAddr,cfgData.srcBufSize);
    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("queueSrcBuffer fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    // source image crop info
    LOG_DBG("dst_crop,X(%u),FloatX(%u),Y(%u),FloatY(%u),W(%u),H(%u)",cfgData.dstCropX,
                                                                      cfgData.dstCropFloatX,
                                                                      cfgData.dstCropY,
                                                                      cfgData.dstCropFloatY,
                                                                      cfgData.dstCropW,
                                                                      cfgData.dstCropH);
    
    err = pDpStream->setSrcCrop(cfgData.dstCropX,
                                cfgData.dstCropFloatX,
                                cfgData.dstCropY,
                                cfgData.dstCropFloatY,
                                cfgData.dstCropW,
                                cfgData.dstCropH);

    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setSrcCrop fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    //====== Configure Output DMA ======

    DP_COLOR_ENUM dstFmt;
    MVOID *dstVirList[3];
    MUINT32 dstSizeList[3];
    MUINT32 dstPhyList[3];
    
    for(MINT32 index = 0; index < ISP_MDP_DL_NUM; index++)
    {
        if(cfgData.dstPortCfg[index] == 1)
        {
            LOG_DBG("DMA index(%d)",index);

            // format convert
            err = DpDmaOutColorFmtConvert(cfgData.dstDma[index],&dstFmt);
            if(err != MDPMGR_NO_ERROR)
            {
                LOG_ERR("DpDmaOutColorFmtConvert fail");
                return MDPMGR_API_FAIL;
            }

            LOG_DBG("fmt(%d),rot(%d),flip(%d)",dstFmt, cfgData.dstDma[index].Rotation, cfgData.dstDma[index].Flip);

            LOG_DBG("W(%u),H(%u),W_c(%u),H_c(%u),W_v(%u),H_v(%u)",cfgData.dstDma[index].size.w,
                                                                   cfgData.dstDma[index].size.h,
                                                                   cfgData.dstDma[index].size_c.w,
                                                                   cfgData.dstDma[index].size_c.h,
                                                                   cfgData.dstDma[index].size_v.w,
                                                                   cfgData.dstDma[index].size_v.h);           
            
            LOG_DBG("pixel_byte(%d),stride(%u),stride_c(%u),stride_v(%u)",cfgData.dstDma[index].pixel_byte,
                                                                           cfgData.dstDma[index].size.stride   * cfgData.dstDma[index].pixel_byte,
                                                                           cfgData.dstDma[index].size_c.stride * cfgData.dstDma[index].pixel_byte,
                                                                           cfgData.dstDma[index].size_v.stride * cfgData.dstDma[index].pixel_byte);
            
            
            // image info configure
        #if 0
            err = pDpStream->setDstConfig(index,
                                          dstFmt,
                                          cfgData.dstDma[index].size.w,
                                          cfgData.dstDma[index].size.h,
                                          cfgData.dstDma[index].size.stride * cfgData.dstDma[index].pixel_byte);

        #else   

            err = pDpStream->setDstConfig(index,                                          
                                          cfgData.dstDma[index].size.w,
                                          cfgData.dstDma[index].size.h,                                                                      
                                          cfgData.dstDma[index].size.stride * cfgData.dstDma[index].pixel_byte,
                                          cfgData.dstDma[index].size_c.stride * cfgData.dstDma[index].pixel_byte,
                                          dstFmt);
        #endif            
            if(err != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("setDstConfig fail(%d)",err);
                return MDPMGR_API_FAIL;
            } 

            // rotation
            err = pDpStream->setRotation(index, cfgData.dstDma[index].Rotation * 90);
            if(err != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("setRotation fail(%d)",err);
                return MDPMGR_API_FAIL;
            } 

            // flip
            err = pDpStream->setFlipStatus(index, cfgData.dstDma[index].Flip);
            if(err != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("setFlipStatus fail(%d)",err);
                return MDPMGR_API_FAIL;
            }

            // memory
            if(cfgData.dstVirAddr[index] == 0 || cfgData.dstPhyAddr[index] == 0)
            {
                LOG_ERR("index(%d)",index);
                LOG_ERR("dst memAddr is 0,VA(0x%8x),PA(0x%8x)",cfgData.dstVirAddr[index],cfgData.dstPhyAddr[index]);
                LOG_ERR("dst, W(%u),H(%u),W_c(%u),H_c(%u),W_v(%u),H_v(%u)",cfgData.dstDma[index].size.w,
                                                                            cfgData.dstDma[index].size.h,
                                                                            cfgData.dstDma[index].size_c.w,
                                                                            cfgData.dstDma[index].size_c.h,
                                                                            cfgData.dstDma[index].size_v.w,
                                                                            cfgData.dstDma[index].size_v.h);

                LOG_ERR("pixel_byte(%d),stride(%u),stride_c(%u),stride_v(%u)",cfgData.dstDma[index].pixel_byte,
                                                                               cfgData.dstDma[index].size.stride   * cfgData.dstDma[index].pixel_byte,
                                                                               cfgData.dstDma[index].size_c.stride * cfgData.dstDma[index].pixel_byte,
                                                                               cfgData.dstDma[index].size_v.stride * cfgData.dstDma[index].pixel_byte);
                return MDPMGR_NULL_OBJECT;
            }
            
            dstSizeList[0] = cfgData.dstDma[index].size.h   * (cfgData.dstDma[index].size.stride   * cfgData.dstDma[index].pixel_byte);
            dstSizeList[1] = cfgData.dstDma[index].size_c.h * (cfgData.dstDma[index].size_c.stride * cfgData.dstDma[index].pixel_byte);
            dstSizeList[2] = cfgData.dstDma[index].size_v.h * (cfgData.dstDma[index].size_v.stride * cfgData.dstDma[index].pixel_byte);
            LOG_DBG("dstSizeList,(0,1,2)=(0x%8x,0x%8x,0x%8x)\n",dstSizeList[0],dstSizeList[1],dstSizeList[2]);

            dstVirList[0] = (MVOID *)cfgData.dstVirAddr[index];
            dstVirList[1] = (MUINT8 *)((MUINT32)dstSizeList[0] + dstVirList[0]);
            dstVirList[2] = (MUINT8 *)((MUINT32)dstSizeList[1] + dstVirList[1]);             
            LOG_DBG("dstVirList,(0,1,2)=(0x%8x,0x%8x,0x%8x)\n",dstVirList[0],dstVirList[1],dstVirList[2]);
      
            dstPhyList[0] = cfgData.dstPhyAddr[index];
            dstPhyList[1] = (dstPhyList[0] + dstSizeList[0]);
            dstPhyList[2] = (dstPhyList[1] + dstSizeList[1]);  
            LOG_DBG("dstPhyList,(0,1,2)=(0x%8x,0x%8x,0x%8x)\n",dstPhyList[0],dstPhyList[1],dstPhyList[2]);

            LOG_DBG("plane num(%d)",cfgData.dstDma[index].Plane);
            err = pDpStream->queueDstBuffer(index, &dstVirList[0],&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
            if(err != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("queueDstBuffer fail(%d)",err);
                return MDPMGR_API_FAIL;
            }
        }
    }

    //====== Configure TPIPE ======

    if(cfgData.cqIdx == ISP_DRV_CQ01)
    {
        cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqNum = TPIPE_PASS2_CMDQ_1;
        cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqPriority = TPIPE_PASS2_CMDQ_PRIOR_HIGH;
    }
    else if(cfgData.cqIdx == ISP_DRV_CQ02)
    {
        cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqNum = TPIPE_PASS2_CMDQ_2;
        cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqPriority = TPIPE_PASS2_CMDQ_PRIOR_LOW;
    }
    else
    {
        LOG_ERR("wrong cq index for TPIPE(%d)",cfgData.cqIdx);
        return MDPMGR_WRONG_PARAM;
    }

    dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo);    

    err = pDpStream->setParameter(cfgData.ispTpipeCfgInfo);
    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setParameter fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    //====== Start DpIspStream ======

    err = pDpStream->startStream();
    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("startStream fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    return MDPMGR_NO_ERROR;          
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::stopMdp(MDPMGR_CFG_STRUCT cfgData)
{
    MINT32 err = MDPMGR_NO_ERROR;
    MUINT32 sceID = cfgData.sceID;
    DpIspStream *pDpStream = NULL;

    LOG_INF("+,cqIndx(%u),sceID(%u)",cfgData.cqIdx,sceID);
    
    //====== Select Correct DpIspStream ======

    pDpStream = selectDpStream(cfgData.sceID,cfgData.cqIdx);
    if(pDpStream == NULL)
    {
        LOG_ERR("pDpStream is NULL");
        return MDPMGR_NULL_OBJECT;
    } 

    //====== Stop DpIspStream ======
    
    err = pDpStream->stopStream();
    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("stopStream fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    return MDPMGR_NO_ERROR;
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::dequeueBuf(MDPMGR_DEQUEUE_INDEX deqIndex, MDPMGR_CFG_STRUCT cfgData)
{
    MINT32 err = MDPMGR_NO_ERROR;
    MUINT32 sceID = cfgData.sceID;
    DpIspStream *pDpStream = NULL;

    MVOID *deqBuf[3];

    LOG_INF("+,cqIndx(%u),sceID(%u),deqIndex(%d)",cfgData.cqIdx,sceID,deqIndex);

    //====== Select Correct DpIspStream ======
  
    pDpStream = selectDpStream(cfgData.sceID,cfgData.cqIdx);
    if(pDpStream == NULL)
    {
        LOG_ERR("pDpStream is NULL");
        return MDPMGR_NULL_OBJECT;        
    } 

    //====== Dequeue Buffer ======
    
    if(deqIndex == MDPMGR_DEQ_SRC)
    {
        err = pDpStream->dequeueSrcBuffer();
        if(err != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("dequeueSrcBuffer fail(%d)",err);
            return MDPMGR_API_FAIL;
        }
    }
    else if(deqIndex == MDPMGR_DEQ_DISPO)
    {
        err = pDpStream->dequeueDstBuffer((MINT32)ISP_MDP_DL_DISPO, &deqBuf[0], (cfgData.cqIdx == ISP_DRV_CQ01) ? true : false);

        if(cfgData.cqIdx == ISP_DRV_CQ01)
        {
            if(err != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("dequeueDstBuffer for dispo fail(%d)",err);
                return MDPMGR_API_FAIL;
            }
        }
        else
        {
            if(err != DP_STATUS_RETURN_SUCCESS && err != DP_STATUS_BUFFER_EMPTY)
            {
                LOG_ERR("dequeueDstBuffer for dispo fail(%d)",err);
                return MDPMGR_API_FAIL;
            }
            else if(err == DP_STATUS_BUFFER_EMPTY)
            {
                return MDPMGR_VSS_DEQ_FALSE_FAIL;
            }
        }        
    }
    else if(deqIndex == MDPMGR_DEQ_VIDO)
    {
        err = pDpStream->dequeueDstBuffer((MINT32)ISP_MDP_DL_VIDO, &deqBuf[0], (cfgData.cqIdx == ISP_DRV_CQ01) ? true : false);
      
        if(cfgData.cqIdx == ISP_DRV_CQ01)
        {
            if(err != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("dequeueDstBuffer for vido fail(%d)",err);
                return MDPMGR_API_FAIL;
            }
        }
        else
        {
            if(err != DP_STATUS_RETURN_SUCCESS && err != DP_STATUS_BUFFER_EMPTY)
            {
                LOG_ERR("dequeueDstBuffer for vido fail(%d)",err);
                return MDPMGR_API_FAIL;
            }
            else if(err == DP_STATUS_BUFFER_EMPTY)
            {
                return MDPMGR_VSS_DEQ_FALSE_FAIL;
            }
        }
    }

    return MDPMGR_NO_ERROR;
}

/**************************************************************************
* 
**************************************************************************/
DpIspStream *MdpMgrImp::selectDpStream(MUINT32 sceID, MUINT32 cqIdx)
{ 
    LOG_DBG("+,cqIndx(%u),sceID(%u)",cqIdx,sceID);

    //====== Select Correct DpIspStream ======

    switch(cqIdx)
    {
        case ISP_DRV_CQ01 :
            return m_pCq1DpStream[sceID];
            break;
        case ISP_DRV_CQ02 :
            return m_pCq2DpStream[sceID];
            break;
        default :
            LOG_ERR("wrong cmdQ index(%d)",cqIdx);
            return NULL;
    }
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::DpColorFmtConvert(EImageFormat ispImgFmt,DpColorFormat *dpColorFormat)
{
    MINT32 ret = MDPMGR_NO_ERROR;
    DpColorFormat localDpColorFormat = DP_COLOR_BAYER8;

    LOG_DBG("+,ispImgFmt(%d)",ispImgFmt); 
    
    switch(ispImgFmt) 
    {
        case eImgFmt_Y800:  //  0x040000,   /*!< Y plane only  */ 
            localDpColorFormat = DP_COLOR_GREY;
            break;        
        case eImgFmt_NV16:  // 0x8000,  422 format, 2 plane
            localDpColorFormat = DP_COLOR_NV16;
            break;
        case eImgFmt_NV21:  // 0x0010, 420 format, 2 plane (VU)
            localDpColorFormat = DP_COLOR_NV21;
            break;
        case eImgFmt_NV12:  // 0x0040, 420 format, 2 plane (UV)
            localDpColorFormat = DP_COLOR_NV12;
            break;
        case eImgFmt_YV12:  // 0x00008, 420 format, 3 plane (YVU)
            localDpColorFormat = DP_COLOR_YV12;
            break;
        case eImgFmt_I420:  // 0x20000, 420 format, 3 plane(YUV)
            localDpColorFormat = DP_COLOR_I420;
            break;
        case eImgFmt_YV16:  // 0x4000, 422 format, 3 plane
            localDpColorFormat = DP_COLOR_YV16;
            break;
        case eImgFmt_I422:  // 0x4000, 422 format, 3 plane
            localDpColorFormat = DP_COLOR_I422;
            break;            
        case eImgFmt_YUY2:  // 0x0100,  422 format, 1 plane (YUYV)
            localDpColorFormat = DP_COLOR_YUYV;            
            break;
        case eImgFmt_UYVY:  // 0x0200, 422 format, 1 plane 
            localDpColorFormat = DP_COLOR_UYVY;
            break;
        case eImgFmt_VYUY:  // 0x100000, 422 format, 1 plane 
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_YVYU:  // 0x080000, 422 format, 1 plane 
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_RGB565:    // 0x0400, RGB 565 (16-bit), 1 plane
            localDpColorFormat = DP_COLOR_RGB565;
            break;
        case eImgFmt_RGB888:    // 0x0800, RGB 888 (24-bit), 1 plane
            localDpColorFormat = DP_COLOR_RGB888;
            break;
        case eImgFmt_ARGB888:   // 0x1000, ARGB (32-bit), 1 plane
            localDpColorFormat = DP_COLOR_ARGB8888;
            break;
        case eImgFmt_BAYER8:    // 0x0001,  Bayer format, 8-bit
            localDpColorFormat = DP_COLOR_BAYER8;
            break;
        case eImgFmt_BAYER10:   // 0x0002,  Bayer format, 10-bit
            localDpColorFormat = DP_COLOR_BAYER10;
            break;
        case eImgFmt_BAYER12:   // 0x0004,  Bayer format, 12-bit
            localDpColorFormat = DP_COLOR_BAYER12;
            break;
        case eImgFmt_NV21_BLK:  // 0x0020, 420 format block mode, 2 plane (UV)
            localDpColorFormat = DP_COLOR_420_BLKP;    
            break;
        case eImgFmt_NV12_BLK:  // 0x0080,  420 format block mode, 2 plane (VU)
            localDpColorFormat = DP_COLOR_420_BLKI;
            break;
        default:
            LOG_ERR("wrong format(%d)",ispImgFmt); 
         	ret = MDPMGR_WRONG_PARAM;
         break;
    }
    
    *dpColorFormat = localDpColorFormat;

    LOG_DBG("-,dpColorFormat(0x%x)",*dpColorFormat); 
    return ret;
}

/**************************************************************************
* 
**************************************************************************/
MINT32 MdpMgrImp::DpDmaOutColorFmtConvert(CdpRotDMACfg dma_out,DpColorFormat *dpColorFormat)
{
    MBOOL ret = MDPMGR_NO_ERROR;
    DpColorFormat localDpColorFormat = DP_COLOR_YUYV;

    LOG_DBG("+,dma,Fmt(%d),Plane(%d),uv_plane_swap(%d),Sequence(%d)",dma_out.Format,
                                                                      dma_out.Plane,
                                                                      dma_out.uv_plane_swap,
                                                                      dma_out.Sequence);
    switch(dma_out.Format)
    {
        case CDP_DRV_FORMAT_YUV422:
            if(dma_out.Plane == CDP_DRV_PLANE_3)
        	{
        	    if(dma_out.uv_plane_swap == 1)
        	    {
        	        localDpColorFormat = DP_COLOR_YV16; // 422,3P
        	    }
        	    else
        	    {
        	        localDpColorFormat = DP_COLOR_I422; // 422,3P
        	    }
        	}
        	else if(dma_out.Plane == CDP_DRV_PLANE_2)
            {         
                if(dma_out.uv_plane_swap == 1)
                {
                    localDpColorFormat = DP_COLOR_NV61; //422,2P
                }
                else
                {
                    localDpColorFormat = DP_COLOR_NV16;
                }
        	}
        	else
        	{
                if(dma_out.Sequence == CDP_DRV_SEQUENCE_YVYU) //MSN->LSB
                {
                    localDpColorFormat = DP_COLOR_UYVY;
                }
                else if(dma_out.Sequence == CDP_DRV_SEQUENCE_YUYV)
                {
                    localDpColorFormat = DP_COLOR_VYUY;
                }
                else if(dma_out.Sequence == CDP_DRV_SEQUENCE_VYUY)
                {
                    localDpColorFormat = DP_COLOR_YUYV;
                }       
                else if(dma_out.Sequence == CDP_DRV_SEQUENCE_UYVY)
                {
                    localDpColorFormat = DP_COLOR_YVYU;
                }
        	}
            break;        
        case CDP_DRV_FORMAT_YUV420:
        	if(dma_out.Plane == CDP_DRV_PLANE_3)
        	{                
                if(dma_out.uv_plane_swap == 1)
                {
                    localDpColorFormat = DP_COLOR_YV12;
                }
                else
                { 
                    localDpColorFormat = DP_COLOR_I420;    
                }
            }
        	else if(dma_out.Plane == CDP_DRV_PLANE_2)
        	{
                if(dma_out.Sequence == CDP_DRV_SEQUENCE_UVUV) //MSB->LSB
                {
                    localDpColorFormat= DP_COLOR_NV21;  //420_2P_YVYU;
                }
                else
                {
                    localDpColorFormat= DP_COLOR_NV12;  //_420_2P_YUYV
                }
        	}
            break;     
        case  CDP_DRV_FORMAT_Y:
        	localDpColorFormat = DP_COLOR_GREY;
            break;
        case CDP_DRV_FORMAT_RGB888:
        	localDpColorFormat = DP_COLOR_RGB888;
            break;
        case CDP_DRV_FORMAT_RGB565:
        	localDpColorFormat = DP_COLOR_RGB565;
            break;        
        case CDP_DRV_FORMAT_XRGB8888:
        	localDpColorFormat = DP_COLOR_XRGB8888;
            break;
        default:
            LOG_ERR("wrong format(%d)",dma_out.Format); 
            localDpColorFormat = DP_COLOR_YUYV;
            ret = MDPMGR_WRONG_PARAM;
        break;
    }
    
    *dpColorFormat = localDpColorFormat;

    LOG_DBG("-,dpColorFormat(0x%x)",*dpColorFormat); 
    return ret;    
}


/**************************************************************************
* 
**************************************************************************/
MVOID MdpMgrImp::dumpIspTPipeInfo(ISP_TPIPE_CONFIG_STRUCT a_info)
{
    LOG_DBG("TpipeCfgInfo");
    LOG_DBG("top.cam_in_fmt(%d)",a_info.top.cam_in_fmt);
    LOG_DBG("top.mode(%d)",a_info.top.mode);       
    LOG_DBG("top.scenario(%d)",a_info.top.scenario);       
    LOG_DBG("top.pixel_id(%d)",a_info.top.pixel_id);       
    LOG_DBG("top.cam_in_fmt(%d)",a_info.top.cam_in_fmt); 
    LOG_DBG("top.nbc_en(%d)",a_info.top.nbc_en);
    LOG_DBG("top.seee_en(%d)",a_info.top.seee_en);
    LOG_DBG("img2o.img2o_mux_en(%d)",a_info.img2o.img2o_mux_en);       
    LOG_DBG("img2o.img2o_mux(%d)",a_info.img2o.img2o_mux);

    //top
    LOG_DBG("top.scenario(%d)", a_info.top.scenario);
    LOG_DBG("top.mode(%d)", a_info.top.mode);
    LOG_DBG("top.debug_sel(%d)", a_info.top.debug_sel);
    LOG_DBG("top.pixel_id(%d)", a_info.top.pixel_id);
    LOG_DBG("top.cam_in_fmt(%d)", a_info.top.cam_in_fmt);
    LOG_DBG("top.tcm_load_en(%d)", a_info.top.tcm_load_en);
    LOG_DBG("top.ctl_extension_en(%d)", a_info.top.ctl_extension_en);
    LOG_DBG("top.rsp_en(%d)", a_info.top.rsp_en);
    LOG_DBG("top.mdp_crop_en(%d)", a_info.top.mdp_crop_en);
    LOG_DBG("top.imgi_en(%d)", a_info.top.imgi_en);
    LOG_DBG("top.lce_en(%d)", a_info.top.lce_en);
    LOG_DBG("top.lcei_en(%d)", a_info.top.lcei_en);
    LOG_DBG("top.lsci_en(%d)", a_info.top.lsci_en);
    LOG_DBG("top.unp_en(%d)", a_info.top.unp_en);
    LOG_DBG("top.bnr_en(%d)", a_info.top.bnr_en);
    LOG_DBG("top.lsc_en(%d)", a_info.top.lsc_en);
    LOG_DBG("top.sl2_en(%d)", a_info.top.sl2_en);
    LOG_DBG("top.c24_en(%d)", a_info.top.c24_en);
    LOG_DBG("top.cfa_en(%d)", a_info.top.cfa_en);
    LOG_DBG("top.c42_en(%d)", a_info.top.c42_en);
    LOG_DBG("top.nbc_en(%d)", a_info.top.nbc_en);
    LOG_DBG("top.seee_en(%d)", a_info.top.seee_en);
    LOG_DBG("top.imgo_en(%d)", a_info.top.imgo_en);
    LOG_DBG("top.img2o_en(%d)", a_info.top.img2o_en   );
    LOG_DBG("top.cdrz_en(%d)", a_info.top.cdrz_en);
    LOG_DBG("top.mdp_sel(%d)", a_info.top.mdp_sel);
    LOG_DBG("top.interlace_mode(%d)", a_info.top.interlace_mode);
    
    //sw
    LOG_DBG("sw.log_en(%d)", a_info.sw.log_en);
    LOG_DBG("sw.src_width(%d)", a_info.sw.src_width);
    LOG_DBG("sw.src_height(%d)", a_info.sw.src_height);
    LOG_DBG("sw.tpipe_irq_mode(%d)", a_info.sw.tpipe_irq_mode);
    LOG_DBG("sw.tpipe_width(%d)", a_info.sw.tpipe_width);
    LOG_DBG("sw.tpipe_height(%d)", a_info.sw.tpipe_height);
    
    //imgi
    LOG_DBG("imgi.imgi_stride(%d)", a_info.imgi.imgi_stride);
    
    //lcei
    LOG_DBG("lcei.lcei_stride(%d)", a_info.lcei.lcei_stride);
    
    //lsci
    LOG_DBG("lsci.lsci_stride(%d)", a_info.lsci.lsci_stride);
    
    //bnr
    LOG_DBG("bnr.bpc_en(%d)", a_info.bnr.bpc_en);
    
    //lsc
    LOG_DBG("lsc.sdblk_width(%d)", a_info.lsc.sdblk_width);
    LOG_DBG("lsc.sdblk_xnum(%d)", a_info.lsc.sdblk_xnum);
    LOG_DBG("lsc.sdblk_last_width(%d)", a_info.lsc.sdblk_last_width);
    LOG_DBG("lsc.sdblk_height(%d)", a_info.lsc.sdblk_height);
    LOG_DBG("lsc.sdblk_ynum(%d)", a_info.lsc.sdblk_ynum);
    LOG_DBG("lsc.sdblk_last_height(%d)", a_info.lsc.sdblk_last_height);
    
    //lce
    LOG_DBG("lce.lce_bc_mag_kubnx(%d)", a_info.lce.lce_bc_mag_kubnx);
    LOG_DBG("lce.lce_slm_width(%d)", a_info.lce.lce_slm_width);
    LOG_DBG("lce.lce_bc_mag_kubny(%d)", a_info.lce.lce_bc_mag_kubny);
    LOG_DBG("lce.lce_slm_height(%d)", a_info.lce.lce_slm_height);
    
    //nbc
    LOG_DBG("nbc.anr_eny(%d)", a_info.nbc.anr_eny);
    LOG_DBG("nbc.anr_enc(%d)", a_info.nbc.anr_enc);
    LOG_DBG("nbc.anr_iir_mode(%d)", a_info.nbc.anr_iir_mode);
    LOG_DBG("nbc.anr_scale_mode(%d)", a_info.nbc.anr_scale_mode);
    
    //seee
    LOG_DBG("seee.se_edge(%d)", a_info.seee.se_edge);
    
    //imgo
    LOG_DBG("imgo.imgo_stride(%d)", a_info.imgo.imgo_stride);
    LOG_DBG("imgo.imgo_crop_en(%d)", a_info.imgo.imgo_crop_en);
    LOG_DBG("imgo.imgo_xoffset(0x%08x)", a_info.imgo.imgo_xoffset);
    LOG_DBG("imgo.imgo_yoffset(0x%08x)", a_info.imgo.imgo_yoffset);
    LOG_DBG("imgo.imgo_xsize(0x%08x)", a_info.imgo.imgo_xsize);
    LOG_DBG("imgo.imgo_ysize(0x%08x)", a_info.imgo.imgo_ysize);
    LOG_DBG("imgo.imgo_mux_en(%d)", a_info.imgo.imgo_mux_en);
    LOG_DBG("imgo.imgo_mux(%d)", a_info.imgo.imgo_mux);
    
    //cdrz
    LOG_DBG("cdrz.cdrz_input_crop_width(%d)", a_info.cdrz.cdrz_input_crop_width);
    LOG_DBG("cdrz.cdrz_input_crop_height(%d)", a_info.cdrz.cdrz_input_crop_height);
    LOG_DBG("cdrz.cdrz_output_width(%d)", a_info.cdrz.cdrz_output_width);
    LOG_DBG("cdrz.cdrz_output_height(%d)", a_info.cdrz.cdrz_output_height);
    LOG_DBG("cdrz.cdrz_luma_horizontal_integer_offset(%d)", a_info.cdrz.cdrz_luma_horizontal_integer_offset);
    LOG_DBG("cdrz.cdrz_luma_horizontal_subpixel_offset(%d)", a_info.cdrz.cdrz_luma_horizontal_subpixel_offset);
    LOG_DBG("cdrz.cdrz_luma_vertical_integer_offset(%d)", a_info.cdrz.cdrz_luma_vertical_integer_offset);
    LOG_DBG("cdrz.cdrz_luma_vertical_subpixel_offset(%d)", a_info.cdrz.cdrz_luma_vertical_subpixel_offset);
    LOG_DBG("cdrz.cdrz_horizontal_luma_algorithm(%d)", a_info.cdrz.cdrz_horizontal_luma_algorithm);
    LOG_DBG("cdrz.cdrz_vertical_luma_algorithm(%d)", a_info.cdrz.cdrz_vertical_luma_algorithm);
    LOG_DBG("cdrz.cdrz_horizontal_coeff_step(%d)", a_info.cdrz.cdrz_horizontal_coeff_step);
    LOG_DBG("cdrz.cdrz_vertical_coeff_step(%d)", a_info.cdrz.cdrz_vertical_coeff_step);
    
    //img2o
    LOG_DBG("img2o.img2o_stride(%d)", a_info.img2o.img2o_stride);
    LOG_DBG("img2o.img2o_crop_en(%d)", a_info.img2o.img2o_crop_en);
    LOG_DBG("img2o.img2o_xoffset(0x%08x)", a_info.img2o.img2o_xoffset);
    LOG_DBG("img2o.img2o_yoffset(0x%08x)", a_info.img2o.img2o_yoffset);
    LOG_DBG("img2o.img2o_xsize(0x%08x)", a_info.img2o.img2o_xsize);
    LOG_DBG("img2o.img2o_ysize(0x%08x)", a_info.img2o.img2o_ysize);
    LOG_DBG("img2o.img2o_mux_en(%d)", a_info.img2o.img2o_mux_en);
    LOG_DBG("img2o.img2o_mux(%d)", a_info.img2o.img2o_mux);
    
    //cfa
    LOG_DBG("cfa.bayer_bypass(%d)", a_info.cfa.bayer_bypass);
    
    //sl2
    LOG_DBG("sl2.sl2_hrz_comp(%d)", a_info.sl2.sl2_hrz_comp);
    
    //pass2
    LOG_DBG("pass2.Pass2CmdqNum(%d)", a_info.pass2.Pass2CmdqNum);
	LOG_DBG("pass2.Pass2CmdqPriority(%d)", a_info.pass2.Pass2CmdqPriority);
}


