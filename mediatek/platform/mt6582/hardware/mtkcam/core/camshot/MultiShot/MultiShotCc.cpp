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
#define LOG_TAG "CamShot/MultiShotCc"
//
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV(fmt, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD(fmt, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI(fmt, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE(fmt, ##arg)
#define FUNCTION_LOG_START      MY_LOGD("[%s] +", __FUNCTION__);
#define FUNCTION_LOG_END        MY_LOGD("[%s] -", __FUNCTION__);
//
#include <cutils/properties.h>
//
#include <linux/cache.h>
//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
// 
#include <mtkcam/v1/camutils/CamMisc.h>
#include <mtkcam/v1/camutils/CamProfile.h>
//
#include <mtkcam/drv/imem_drv.h>
//
#include <mtkcam/hal/aaa_hal_base.h>
//
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/ICamIOPipe.h>
#include <mtkcam/campipe/IPostProcPipe.h>
//
#include <mtkcam/drv/res_mgr_drv.h>
#include <mtkcam/campipe/pipe_mgr_drv.h>
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>

#include <mtkcam/camshot/ISImager.h> 
#include "../inc/ImageUtils.h"

#include <DpBlitStream.h>   //[CS]+
extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

#include <mtkcam/common/camutils/CamFormat.h>

//
#include "../inc/CamShotImp.h"
#include "../inc/MultiShot.h"
//
using namespace android; 
using namespace NSCamPipe; 
using namespace NS3A; 

class ResMgrDrv; 
class PipeMgrDrv; 

#define MEDIA_PATH  "/sdcard/"

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
MultiShotCc::
MultiShotCc(
    EShotMode const eShotMode,
    char const*const szCamShotName
)
    : MultiShot(eShotMode, szCamShotName)
    , mrNode()
    , mpCaptureBufMgr(NULL)
{
}

/*******************************************************************************
* 
********************************************************************************/
MultiShotCc::
~MultiShotCc(
)
{
    if ( mpCaptureBufMgr != NULL )
        mpCaptureBufMgr = NULL;
}
/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShotCc::
start(SensorParam const & rSensorParam, MUINT32 u4ShotCount)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_start);
    
    mSensorParam = rSensorParam; 
    //
    //dumpSensorParam(mSensorParam); 

    MY_LOGD("[start] enabled msg (nitify, data) = (0x%x, 0x%x)", mi4NotifyMsgSet, mi4DataMsgSet); 
    //
    if (!isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL))
    {
        MY_LOGE("[start] No data msg enable !"); 
        return MFALSE; 
    }

    mbCancelShot = MFALSE;
    mbIsLastShot = MFALSE;
    mu4JpegCount = 0;
    mu4ShotCount = u4ShotCount;
	mbJpegSemPost = MFALSE; 
    ::sem_init(&semJpeg, 0, 0);
    ::sem_init(&semThumbnail, 0, 0);
    ::sem_init(&semStartEnd, 0, 0);
    
    MY_LOGD("mu4ShotCount = %d", mu4ShotCount);

#if 0    
    EImageFormat eImgFmt = querySensorFmt(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth); 

    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "create/init CamIOPipe");
    // (1). Create Instance 
    if (NULL == mpCamIOPipe) 
    {
        mpCamIOPipe = ICamIOPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, static_cast<EScenarioFmt>(mapScenarioType(eImgFmt))); 
        CHECK_OBJECT(mpCamIOPipe); 
        // (2). Query port property
    #warning [TODO] Query port property
        // (3). init 
        mpCamIOPipe->init(); 
     }
#endif   
    // (2) prepare buffer
    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "prepare buffer");
    // (2.1) raw buffer
    //mRawImgBufInfo = querySensorRawImgBufInfo(); 
    // (2.2) yuv buffer
    //mYuvImgBufInfoWrite = queryYuvRawImgBufInfo(); 
    //mYuvImgBufInfoReady = queryYuvRawImgBufInfo();
    //mYuvImgBufInfoRead = queryYuvRawImgBufInfo();
    // (2.3) PostView buffer
    //mPostViewImgBufInfoWrite = queryPostViewImgInfo(); 
    //mPostViewImgBufInfoReady = queryPostViewImgInfo(); 
    //mPostViewImgBufInfoRead = queryPostViewImgInfo(); 
    // (2.4) jpeg buffer
    mJpegImgBufInfoWrite = queryJpegImgBufInfo(); 
    mJpegImgBufInfoReady = queryJpegImgBufInfo(); 

    // (2.5) Thumb buffer
    mThumbImgBufInfoYuv = queryThumbYuvImgBufInfo(); 
    mThumbImgBufInfoWrite = queryThumbImgBufInfo(); 
    mThumbImgBufInfoReady = queryThumbImgBufInfo();
    mThumbImgBufInfoTemp = queryThumbTempImgBufInfo(); 
    
    // (3) init thread
    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "init image create thread");
    initImageCreateThread();
    
    // (4) start c-shot loop
    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "wakeup create thread");
    mpImageCreateThread->postCommand(Command(Command::eID_WAKEUP));
    //onCreateImage();
    
    
    FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShotCc::
stop()
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_stop);
#warning [TODO] for continouous shot     
    // [CS]+

    // (1) 
    mbCancelShot = MTRUE;
     
    // (2) wait start end
    CPTLogStr(Event_MShot_stop, CPTFlagSeparator, "wait start end");
    ::sem_wait(&semStartEnd);  // must call before thread stop, to sure the lastimage notify callback do.

    // (3) uninit thread
    CPTLogStr(Event_MShot_stop, CPTFlagSeparator, "uninit image create thread");
    uninitImageCreateThread();

    // (4) end continuous shot jobs in 3A
    NS3A::Hal3ABase *p3AObj = Hal3ABase::createInstance(mSensorParam.u4DeviceID);   
    //p3AObj->endContinuousShotJobs();
    p3AObj->destroyInstance(); 

#if 0    
    // (4) destroy CamIOPipe
    CPTLogStr(Event_MShot_stop, CPTFlagSeparator, "destroy/uninit CamIOPipe");
    CHECK_OBJECT(mpCamIOPipe)
    MBOOL ret = mpCamIOPipe->uninit(); 
    if (!ret)
    {
        MY_LOGE("mpCamIOPipe->uninit() fail ");
    }
    mpCamIOPipe = NULL; 
#endif
    // (5) prepare buffer
    freeShotMem();

    // [CS]-

    FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShotCc::
initImageCreateThread()
{
    FUNCTION_LOG_START;

    // (0) create display thread
    status_t status = OK;

    
    mpImageCreateThread = IImageCreateThread::createInstance(IMAGE_CREATE, this);
    if  (
            mpImageCreateThread == 0
        ||  OK != (status = mpImageCreateThread->run())
        )
    {
        MY_LOGE(
            "Fail to run ImageCreateThread - mpImageCreateThread.get(%p), status[%s(%d)]", 
            mpImageCreateThread.get(), ::strerror(-status), -status
        );
        return MFALSE;  
    }
#if 0    
    mpYuvImageCreateThread = IImageCreateThread::createInstance(YUV_IMAGE_CREATE, this);
    if  (
            mpYuvImageCreateThread == 0
        ||  OK != (status = mpYuvImageCreateThread->run())
        )
    {
        MY_LOGE(
            "Fail to run YuvImageCreateThread - mpYuvImageCreateThread.get(%p), status[%s(%d)]", 
            mpYuvImageCreateThread.get(), ::strerror(-status), -status
        );
        return MFALSE;  
    }
#endif
    mpThumbnailImageCreateThread = IImageCreateThread::createInstance(THUMBNAIL_IMAGE_CREATE, this);
    if  (
            mpThumbnailImageCreateThread == 0
        ||  OK != (status = mpThumbnailImageCreateThread->run())
        )
    {
        MY_LOGE(
            "Fail to run ThumbnailImageCreateThread - mpThumbnailImageCreateThread.get(%p), status[%s(%d)]", 
            mpThumbnailImageCreateThread.get(), ::strerror(-status), -status
        );
        return MFALSE;  
    }

    mpJpegImageCreateThread = IImageCreateThread::createInstance(JPEG_IMAGE_CREATE, this);
    if  (
            mpJpegImageCreateThread == 0
        ||  OK != (status = mpJpegImageCreateThread->run())
        )
    {
        MY_LOGE(
            "Fail to run JpegImageCreateThread - mpJpegImageCreateThread.get(%p), status[%s(%d)]", 
            mpJpegImageCreateThread.get(), ::strerror(-status), -status
        );
        return MFALSE;  
    }
    FUNCTION_LOG_END;

    return MTRUE;  
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShotCc::
onCreateImage()
{
    //FUNCTION_LOG_START;

    AutoCPTLog cptlog(Event_MShot_onCreateImage);
    MUINT32 u4ShotCount = 0;
    //
    mpJpegImageCreateThread->postCommand(Command(Command::eID_YUV_BUF));
    
    // move in jpeg create
    CPTLogStr(Event_MShot_onCreateImage, CPTFlagSeparator, "callback EOF");
    handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);

    // loop, trigger jpeg create
    while(u4ShotCount<mu4ShotCount)
    {   
        CPTLogStr(Event_MShot_onCreateImage, CPTFlagSeparator, "wait jpeg done");
        ::sem_wait(&semJpeg);

        CPTLogStr(Event_MShot_onCreateImage, CPTFlagSeparator, "handle callback");
        if(mbIsLastShot || u4ShotCount==mu4ShotCount-1)  // last frame
        {
            MY_LOGD("notify last shot will callback");
            handleNotifyCallback(ECamShot_NOTIFY_MSG_CSHOT_END, 0, 0); 
            handleNotifyCallback(ECamShot_NOTIFY_MSG_FOCUS_VALUE, mFocusVal.u4ValH, mFocusVal.u4ValL);
            handleDataCallback(ECamShot_DATA_MSG_JPEG, (mThumbImgBufInfoReady.u4BufVA), mu4ThumbnailSize, reinterpret_cast<MUINT8*>(mJpegImgBufInfoReady.u4BufVA), mu4JpegSize); 
            break;
        }
        else  // create next jpeg
        {
            // trigger next jpeg create
            //
            mpJpegImageCreateThread->postCommand(Command(Command::eID_YUV_BUF)); 
            handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);
            handleNotifyCallback(ECamShot_NOTIFY_MSG_FOCUS_VALUE, mFocusVal.u4ValH, mFocusVal.u4ValL);
            handleDataCallback(ECamShot_DATA_MSG_JPEG, (mThumbImgBufInfoReady.u4BufVA), mu4ThumbnailSize, reinterpret_cast<MUINT8*>(mJpegImgBufInfoReady.u4BufVA), mu4JpegSize); 
            u4ShotCount++;
        }
        
    }
    
    // (7) start end
    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "post start end sem");
    ::sem_post(&semStartEnd);
    
    //FUNCTION_LOG_END;
    return MTRUE;
}
/*******************************************************************************
* 
********************************************************************************/
MVOID
MultiShotCc::
getReadBuf()
{
    //FUNCTION_LOG_START;

    mpCaptureBufMgr->dequeProcessor(mrNode, 0);

    Mutex::Autolock lock(mYuvReadyBufMtx);  // Fix me need this??
    
    mapNodeToImageBuf(mrNode.mainImgNode, mYuvImgBufInfoRead);
    mapNodeToImageBuf(mrNode.subImgNode, mPostViewImgBufInfoRead);
    
    mFocusValRead.u4ValH = mrNode.u4FocusValH;
    mFocusValRead.u4ValL = mrNode.u4FocusValL;
    
    mbJpegSemPost = MFALSE;  // Fix me need this??
    //FUNCTION_LOG_END;
}
/*******************************************************************************
* 
********************************************************************************/
MVOID
MultiShotCc::
returnJpegBuf()
{
    //FUNCTION_LOG_START;

    Mutex::Autolock lock(mJpegReadyBufMtx);

    ImgBufInfo rJpegImgBufInfo = mJpegImgBufInfoWrite; 
    mJpegImgBufInfoWrite = mJpegImgBufInfoReady;
    mJpegImgBufInfoReady = rJpegImgBufInfo;

    ImgBufInfo rThumbImgBufInfo = mThumbImgBufInfoWrite; 
    mThumbImgBufInfoWrite = mThumbImgBufInfoReady;
    mThumbImgBufInfoReady = rThumbImgBufInfo;
    
    mFocusVal = mFocusValRead;

    mpCaptureBufMgr->enqueProcessor(mrNode);
    
    //FUNCTION_LOG_START;
   
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShotCc::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;
    //
    switch  (cmd)
    {
    case ECamShot_CMD_SET_CSHOT_SPEED:
        if(arg1 > 0)
        {
            mu4ShotSpeed = arg1;
            ret = MTRUE;
        }
        else
        {
            MY_LOGD("set invalid shot speed: %d", arg1); 
            ret = MFALSE;
        }
        break;
    case ECamShot_CMD_SET_CAPBUF_MGR:
        {
            ICaptureBufMgrHandler* pCaptureBufMgr =  reinterpret_cast<ICaptureBufMgrHandler*>(arg1);
            mpCaptureBufMgr = pCaptureBufMgr;
        }
        break;
    default:
        break;
    }
    //

    FUNCTION_LOG_END;
    //
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MultiShotCc::
mapNodeToImageBuf(ImgBufQueNode & rNode, ImgBufInfo & rImgBuf)
{
    //FUNCTION_LOG_START;

    rImgBuf.u4ImgWidth = rNode->getImgWidth();
    rImgBuf.u4ImgHeight = rNode->getImgHeight();
    rImgBuf.eImgFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(rNode->getImgFormat()));
    rImgBuf.u4Stride[0] = rNode->getImgWidthStride(0);
    rImgBuf.u4Stride[1] = rNode->getImgWidthStride(1);
    rImgBuf.u4Stride[2] = rNode->getImgWidthStride(2);
    rImgBuf.u4BufSize = rNode->getBufSize();
    rImgBuf.u4BufVA = (MUINT32)rNode->getVirAddr();
    rImgBuf.u4BufPA = (MUINT32)rNode->getPhyAddr();
    rImgBuf.i4MemID = rNode->getIonFd();

    
    MY_LOGD("[registerImgBufInfo] (width, height, format) = (%d, %d, 0x%x)", rImgBuf.u4ImgWidth, rImgBuf.u4ImgHeight, rImgBuf.eImgFmt); 
    MY_LOGD("[registerImgBufInfo] (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rImgBuf.u4BufVA, rImgBuf.u4BufPA, rImgBuf.u4BufSize, rImgBuf.i4MemID); 

    
    //FUNCTION_LOG_END;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

