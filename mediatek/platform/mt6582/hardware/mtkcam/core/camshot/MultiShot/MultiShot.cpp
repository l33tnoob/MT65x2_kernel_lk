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
#define LOG_TAG "CamShot/MultiShot"
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
MultiShot::
MultiShot(
    EShotMode const eShotMode,
    char const*const szCamShotName
)
    : CamShotImp(eShotMode, szCamShotName)
    , mpISImager(NULL) 
    , mpCamIOPipe(NULL) 
    , mpMemDrv(NULL)
    , mSensorParam()
    , mShotParam()
    , mJpegParam()
    , mRawImgBufInfo()
    , mYuvImgBufInfo()
    , mPostViewImgBufInfo()
    , mJpegImgBufInfo()
    , mThumbImgBufInfo() 
    , mRawMem()
    , mYuvMem()
    , mPostViewMem()
    , mJpegMem()
    , mThumbnailMem()
    // [CS] +
    , mYuvImgBufInfoRead() 		 // used for jpeg_enc to read
    , mYuvImgBufInfoWrite() 	 // used for pass2 to write
    , mYuvImgBufInfoReady() 	 // ready for jpeg compressing
    , mPostViewImgBufInfoRead()  // used for jpeg_enc to read
    , mPostViewImgBufInfoWrite() // used for pass2 to write
    , mPostViewImgBufInfoReady() // ready for thumbnail compressing
    , mJpegImgBufInfoWrite()	 // used for jpeg_enc to write 
    , mJpegImgBufInfoReady() 	 // ready for callback
    , mThumbImgBufInfoYuv()		 // used for Thumbnail yuv data
    , mThumbImgBufInfoWrite()    // used for thumbnail enc to write 
    , mThumbImgBufInfoReady()    // ready for callback 
    , mThumbImgBufInfoTemp()
	, mFocusValRead()	
	, mFocusValWrite()
	, mFocusValReady()
	, mFocusVal()
	, mbSet3ACapMode(MTRUE)
	, mu4ShotCount(0)
	, mu4ShotSpeed(100)
	, mtvLastJpegStart()
	, mu4JpegCount(0)
	, mbCancelShot(MFALSE)
	, mbIsLastShot(MFALSE)
	, mbJpegSemPost(MFALSE)
	, mu4JpegSize(0)
	, mu4ThumbnailSize(0)
    , mYuvReadyBufMtx()
    , mJpegReadyBufMtx()
    , semStartEnd()
    , semJpeg()
    , semThumbnail()
    , mpImageCreateThread(NULL)
    , mpYuvImageCreateThread(NULL)
    , mpThumbnailImageCreateThread(NULL)
    , mpJpegImageCreateThread(NULL)
    // [CS] -
    , mpPipeMgrDrv(NULL)
    , mpResMgrDrv(NULL)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.camera.dump", value, "0"); 
    mu4DumpFlag = ::atoi(value); 
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
init()
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_init);
    //
    MY_LOGD("[init] (ShotMode, ShotName) = (%d, %s)", getShotMode(), getCamShotName()); 
#if 0 
    mpCamIOPipe = ICamIOPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, eScenarioFmt_RAW); 
    CHECK_OBJECT(mpCamIOPipe); 
    if (!mpCamIOPipe->init())
    {
        MY_LOGE("mpCamIOPipe->init() fail ");
        return MFALSE; 
    } 
#endif   
    //
    mpMemDrv = IMemDrv::createInstance(); 
    CHECK_OBJECT(mpMemDrv); 
    //
    mpMemDrv->init();

    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
uninit()
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_uninit);
    //
    freeShotMem(); 

    mpMemDrv->uninit(); 
    mpMemDrv->destroyInstance(); 
    //
#if 0
    CHECK_OBJECT(mpCamIOPipe)
    if (!mpCamIOPipe->uninit())
    {
        MY_LOGE("mpCamIOPipe->uninit() fail ");
        return MFALSE;   
    }
#endif

    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
start(SensorParam const & rSensorParam, MUINT32 u4ShotCount)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_start);
    
    mSensorParam = rSensorParam; 
    //
    dumpSensorParam(mSensorParam); 

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
    
    // (2) prepare buffer
    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "prepare buffer");
    // (2.1) raw buffer
    mRawImgBufInfo = querySensorRawImgBufInfo(); 
    // (2.2) yuv buffer
    mYuvImgBufInfoWrite = queryYuvRawImgBufInfo(); 
    mYuvImgBufInfoReady = queryYuvRawImgBufInfo();
    mYuvImgBufInfoRead = queryYuvRawImgBufInfo();
    // (2.3) PostView buffer
    mPostViewImgBufInfoWrite = queryPostViewImgInfo(); 
    mPostViewImgBufInfoReady = queryPostViewImgInfo(); 
    mPostViewImgBufInfoRead = queryPostViewImgInfo(); 
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
MultiShot::
startOne(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;
    MtkCamUtils::CamProfile profile("[startOne][sensor->mem]", "MultiShot");
    MBOOL ret = MTRUE; 
    mSensorParam = rSensorParam; 
    //
    dumpSensorParam(mSensorParam); 
    EImageFormat eImgFmt = querySensorFmt(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth); 

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
    //
    MY_LOGD("[startOne] enabled msg (nitify, data) = (0x%x, 0x%x)", mi4NotifyMsgSet, mi4DataMsgSet); 
    //
    // Start Of Frame notify
#warning [TODO] this should callback from pipe 
    handleNotifyCallback(ECamShot_NOTIFY_MSG_SOF, 0, 0); 

    if (!isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL))
    {
        MY_LOGE("[startOne] No data msg enable !"); 
        return MFALSE; 
    }

   
    // (1) create raw image 
    // In MT6589, it always need to dump bayer raw image due to 
    // the capture is 3 pass, 
    // 1st pass: Sensor -> TG --> Memory (Raw(bayer),  YUV(yuy2)) 
    // 2nd pass: memory (bayer/yuy2) --> post proc -> mem (yuv, postview) 
    // 3nd pass: memory (yuv) --> jpeg --> mem (bitstream) 
    ImgBufInfo rRawImgBufInfo = querySensorRawImgBufInfo(); 
    ret = ret 
           && createSensorRawImg(mSensorParam, rRawImgBufInfo)
           && handleDataCallback(ECamShot_DATA_MSG_BAYER, 0, 0, reinterpret_cast<MUINT8*>(rRawImgBufInfo.u4BufVA), rRawImgBufInfo.u4BufSize); 

    handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0); 

    // post process 
    if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_POSTVIEW|ECamShot_DATA_MSG_JPEG))
    {
        startOne(rRawImgBufInfo);
    }
   
    CHECK_OBJECT(mpCamIOPipe)
    ret = mpCamIOPipe->uninit(); 
    if (!ret)
    {
        MY_LOGE("mpCamIOPipe->uninit() fail ");
    }
    mpCamIOPipe = NULL; 

    profile.print(); 
    FUNCTION_LOG_END;
    //
    return ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
startOne(ImgBufInfo const & rImgBufInfo)
{
    FUNCTION_LOG_START;
    MtkCamUtils::CamProfile profile("[startOne][mem->Mem]", "MultiShot");
    MBOOL ret = MTRUE; 

    Rect rSrcRect(0, 0, rImgBufInfo.u4ImgWidth, rImgBufInfo.u4ImgHeight); 
    Rect rDstRect(0, 0, mShotParam.u4PictureWidth, mShotParam.u4PictureHeight); 

    // calc the zoom crop ratio 
    Rect rRect = MtkCamUtils::calCrop(rSrcRect, rDstRect, mShotParam.u4ZoomRatio); 
    

    // (2) create yuv image 
    // In MT6589, the postview will be ouput in the 2nd pass 
    // and the yuv image is created in 2nd pass 
    if (isDataMsgEnabled(ECamShot_DATA_MSG_JPEG)) 
    {
        ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
        ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
        ImgBufInfo rJpegImgBufInfo = queryJpegImgBufInfo(); 
        ImgBufInfo rThumbImgBufInfo = queryThumbImgBufInfo(); 

        MUINT32 u4JpegSize = 0; 
        MUINT32 u4ThumbnailSize = 0; 

        ret = ret 
               && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, rYuvImgBufInfo, rPostViewBufInfo) 
               && handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(rPostViewBufInfo.u4BufVA), rPostViewBufInfo.u4BufSize)
               && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(rYuvImgBufInfo.u4BufVA), rYuvImgBufInfo.u4BufSize)
               && createJpegImg(rYuvImgBufInfo, mJpegParam, 0, 0 , rJpegImgBufInfo, u4JpegSize); 
 
        //create thumbnail 
        if (0 != mJpegParam.u4ThumbWidth && 0 != mJpegParam.u4ThumbHeight)
        {
            JpegParam rParam(mJpegParam.u4ThumbQuality, mJpegParam.fgThumbIsSOI);                 
            // postview w/o rotation, hence thumbnail should rotate
            ret = ret && createJpegImg(rPostViewBufInfo,rParam,  mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, rThumbImgBufInfo, u4ThumbnailSize); 
        }
        // Jpeg callback, it contains thumbnail in ext1, ext2. 
        handleDataCallback(ECamShot_DATA_MSG_JPEG, (rThumbImgBufInfo.u4BufVA), u4ThumbnailSize, reinterpret_cast<MUINT8*>(rJpegImgBufInfo.u4BufVA), u4JpegSize); 
    }
    else if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV) && isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW))
    {
        ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
        ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 

        ret = ret 
               && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, rYuvImgBufInfo, rPostViewBufInfo) 
               && handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(rPostViewBufInfo.u4BufVA), rPostViewBufInfo.u4BufSize)
               && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(rYuvImgBufInfo.u4BufVA), rYuvImgBufInfo.u4BufSize);       
    }
    else if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV))
    { 
        ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
        ret = ret 
               && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, rYuvImgBufInfo) 
               && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(rYuvImgBufInfo.u4BufVA), rYuvImgBufInfo.u4BufSize);            
    }
    else if (isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW)) 
    {
        //! should not enter this case 
        ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
        ret = ret 
               && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PostViewRotation, mShotParam.u4PostViewFlip, rPostViewBufInfo) 
               && handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(rPostViewBufInfo.u4BufVA), rPostViewBufInfo.u4BufSize);
    }
    profile.print(); 
    FUNCTION_LOG_END;
    //
    return ret;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
startAsync(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;

    FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
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
    p3AObj->endContinuousShotJobs();
    p3AObj->destroyInstance(); 

    // (5) destroy CamIOPipe
    CPTLogStr(Event_MShot_stop, CPTFlagSeparator, "destroy/uninit CamIOPipe");
    CHECK_OBJECT(mpCamIOPipe)
    MBOOL ret = mpCamIOPipe->uninit(); 
    if (!ret)
    {
        MY_LOGE("mpCamIOPipe->uninit() fail ");
    }
    mpCamIOPipe = NULL; 

    // (6) prepare buffer
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
MultiShot::
setShotParam(ShotParam const & rParam)
{
    FUNCTION_LOG_START;
    mShotParam = rParam; 
    //
    dumpShotParam(mShotParam); 

    FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
setJpegParam(JpegParam const & rParam)
{
    FUNCTION_LOG_START;
    mJpegParam = rParam; 
    //
    dumpJpegParam(mJpegParam); 
    
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
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
MBOOL 
MultiShot::
createSensorRawImg(SensorParam const & rSensorParam, ImgBufInfo const & rRawImgBufInfo)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_createSensorRawImg);
    //MY_LOGD("buf createSensorRawImg, out (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rRawImgBufInfo.u4BufVA, rRawImgBufInfo.u4BufPA, rRawImgBufInfo.u4BufSize, rRawImgBufInfo.i4MemID); 
    MBOOL ret = MTRUE; 
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "lockResZsd");
    if (!lock(RES_MGR_DRV_SCEN_HW_ZSD, PIPE_MGR_DRV_PIPE_MASK_CAM_IO, 3000))
    {
        MY_LOGE("[createSensorRawImg] lock resource fail"); 
        return MFALSE; 
    }
    
    MtkCamUtils::CamProfile profile("createSensorRawImg", "MultiShot");
    EImageFormat eImgFmt = querySensorFmt(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth); 

    // (4). setCallback
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "setCallback");
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL); 
    // (5). Config pipe 
    //
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "config CamIOPipe");
    vector<PortInfo const*> vInPorts;  
    vector<PortInfo const*> vOutPorts; 
    // 
    SensorPortInfo rSensorPort(rSensorParam.u4DeviceID, 
                                           rSensorParam.u4Scenario, 
                                           rSensorParam.u4Bitdepth, 
                                           rSensorParam.fgBypassDelay, 
                                           rSensorParam.fgBypassScenaio, 
                                           rSensorParam.u4RawType
                                          ); 
    vInPorts.push_back(&rSensorPort); 
    //    
    MemoryOutPortInfo rRawPort(ImgInfo(rRawImgBufInfo.eImgFmt, rRawImgBufInfo.u4ImgWidth, rRawImgBufInfo.u4ImgHeight), 
                               rRawImgBufInfo.u4Stride, 0, 0); 
    vOutPorts.push_back(&rRawPort); 
    //
    mpCamIOPipe->configPipe(vInPorts, vOutPorts);  

    // (6). Enqueue, raw buf
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "enqueue raw buf");
    QBufInfo rRawBuf; 
    BufInfo rBufInfo(rRawImgBufInfo.u4BufSize, rRawImgBufInfo.u4BufVA, rRawImgBufInfo.u4BufPA, rRawImgBufInfo.i4MemID);  
    rRawBuf.vBufInfo.push_back(rBufInfo); 
    mpCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rRawBuf); 

    profile.print(); 
    
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "create 3A");
    NS3A::Hal3ABase *p3AObj = Hal3ABase::createInstance(rSensorParam.u4DeviceID);           
    //
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "SetIspProfCap");
    ret = ret 
            && p3AObj->setIspProfile(EIspProfile_NormalCapture); 

	if(mbSet3ACapMode)
	{
		CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "3A CapStart");
    	MY_LOGD("Send ECmd_CaptureStart to 3A"); 
	 	ret = ret 
            	&& p3AObj->sendCommand(ECmd_CaptureStart, 0);  
        p3AObj->onFireCapFlashIfNeeded();
	}
    // (7). start
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "start CamIOPipe");
    mpCamIOPipe->start(); 

    // (8). dequeue 
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "dequeue buffer");
    QTimeStampBufInfo rQRawOutBuf;         
    mpCamIOPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQRawOutBuf); 
    // 
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "stop CamIOPipe");
    mpCamIOPipe->stop(); 
    profile.print(); 
    if (mu4DumpFlag) 
    {
        char fileName[256] = {'\0'}; 
        sprintf(fileName, "/%s/shot_raw%dx%d.raw", MEDIA_PATH, rRawImgBufInfo.u4ImgWidth, rRawImgBufInfo.u4ImgHeight); 
        MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rRawImgBufInfo.u4BufVA), rRawImgBufInfo.u4BufSize);         
    }

#if 0 
    // (10). uninit 
    mpCamIOPipe->uninit(); 
    // (11). destory instance 
    mpCamIOPipe->destroyInstance(); 
    // 
#endif  
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "unlockResZsd");
    unlock(PIPE_MGR_DRV_PIPE_MASK_CAM_IO); 
    //
    
	if( mbSet3ACapMode)
	{
		mbSet3ACapMode = MFALSE;
		CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "3A CapEnd");
    	MY_LOGD("Send ECmd_CaptureEnd to 3A"); 
    	p3AObj->sendCommand(ECmd_CaptureEnd, 0); 
	}
    //
    FeatureParam_T rFeatureParam;
    p3AObj->getSupportedParams(rFeatureParam);
    mFocusValWrite.u4ValH = rFeatureParam.i8BSSVlu >> 32;
    mFocusValWrite.u4ValL = (MUINT32)(rFeatureParam.i8BSSVlu);
    CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "destroy 3A");
    MY_LOGD("Focus value(i8BSSVlu, u4ValH, u4ValL) = (%lld, %d, %d)", rFeatureParam.i8BSSVlu, mFocusValWrite.u4ValH, mFocusValWrite.u4ValL); 
    //
    p3AObj->destroyInstance(); 
	CPTLogStr(Event_MShot_createSensorRawImg, CPTFlagSeparator, "afterDst3A");
    FUNCTION_LOG_END;
    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL 
MultiShot::
createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 const u4Img1Rot, MUINT32 const u4Img1Flip, ImgBufInfo const & rDstImgBufInfo1, ImgBufInfo const &rDstImgBufInfo2 )
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_createYuvRawImg);    
    //MY_LOGD("createYuvRawImg, in (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.i4MemID); 
    //MY_LOGD("createYuvRawImg, out1 (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rDstImgBufInfo1.u4BufVA, rDstImgBufInfo1.u4BufPA, rDstImgBufInfo1.u4BufSize, rDstImgBufInfo1.i4MemID); 
    //MY_LOGD("createYuvRawImg, out2 (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rDstImgBufInfo2.u4BufVA, rDstImgBufInfo2.u4BufPA, rDstImgBufInfo2.u4BufSize, rDstImgBufInfo2.i4MemID); 

    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "lockResIP");
    if (!lock(RES_MGR_DRV_SCEN_HW_IP, PIPE_MGR_DRV_PIPE_MASK_POST_PROC, 3000))
    {
        MY_LOGE("[createYuvRawImg] lock resource fail"); 
        return MFALSE; 
    }

    MtkCamUtils::CamProfile profile("createYuvRawImg", "MultiShot");
    // (1). Create Instance 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "create PostProcPipe");
    IPostProcPipe    *pPostProcPipe = IPostProcPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, static_cast<EScenarioFmt>(mapScenarioType(rSrcImgBufInfo.eImgFmt))); 
    CHECK_OBJECT(pPostProcPipe); 
   
    // (2). Query port property
    // (3). init 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "init PostProcPipe");
    pPostProcPipe->init(); 
    // (4). setCallback
    pPostProcPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "config PostProcPipe");
    // 
    MemoryInPortInfo rMemInPort(ImgInfo(rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight), 
                                0, rSrcImgBufInfo.u4Stride, Rect(rSrcCropRect.x, rSrcCropRect.y, rSrcCropRect.w, rSrcCropRect.h)); 
    //
    MemoryOutPortInfo rVdoPort(ImgInfo(rDstImgBufInfo1.eImgFmt, rDstImgBufInfo1.u4ImgWidth, rDstImgBufInfo1.u4ImgHeight), 
                               rDstImgBufInfo1.u4Stride, u4Img1Rot, u4Img1Flip);   
    rVdoPort.index = 1;   
    //
    vector<PortInfo const*> vInPorts;  
    vector<PortInfo const*> vOutPorts; 
    //
    vInPorts.push_back(&rMemInPort); 
    vOutPorts.push_back(&rVdoPort); 
    //
    //MY_LOGD("[createYuvRawImg] enable postview "); 
    MemoryOutPortInfo rDispPort(ImgInfo(rDstImgBufInfo2.eImgFmt, rDstImgBufInfo2.u4ImgWidth, rDstImgBufInfo2.u4ImgHeight), 
                                   rDstImgBufInfo2.u4Stride, 0, 0); 
    vOutPorts.push_back(&rDispPort); 
    //
    pPostProcPipe->configPipe(vInPorts, vOutPorts); 
    // (6). Enqueue, In buf
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "enqueue in buf");
    // 
    QBufInfo rInQBuf; 
    BufInfo rInBufInfo(rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.i4MemID);  
    rInQBuf.vBufInfo.push_back(rInBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryIn, 0, 0), rInQBuf); 

    // (6.1) Enqueue, postview out buf 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "enqueue postview out buf");
    QBufInfo rDispQBuf; 
    BufInfo rDispBufInfo(rDstImgBufInfo2.u4BufSize, rDstImgBufInfo2.u4BufVA, rDstImgBufInfo2.u4BufPA, rDstImgBufInfo2.i4MemID);  
    rDispQBuf.vBufInfo.push_back(rDispBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rDispQBuf); 

    // (6.2) Enqueue, Yuv out Buf
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "enqueue yuv out buf");
    QBufInfo rVdoQBuf; 
    BufInfo rVdoBufInfo(rDstImgBufInfo1.u4BufSize, rDstImgBufInfo1.u4BufVA, rDstImgBufInfo1.u4BufPA, rDstImgBufInfo1.i4MemID); 
    rVdoQBuf.vBufInfo.push_back(rVdoBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rVdoQBuf); 

    profile.print(); 
    // (7). start
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "start PostProcPipe");
    pPostProcPipe->start(); 

    // (8). YUV Dequeue
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "dequeue yuv buf");
    QTimeStampBufInfo rQVdoOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQVdoOutBuf); 

    // (8.1) postview Dequeue 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "dequeue postview buf");
    QTimeStampBufInfo rQDispOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQDispOutBuf); 
    // (8.2) In buffer dequeue 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "dequeue In buffer");
    QTimeStampBufInfo rQInBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryIn, 0, 0), rQInBuf); 

    // (9). Stop 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "stop PostProcPipe");
    pPostProcPipe->stop();    
    profile.print(); 
    if (mu4DumpFlag) 
    {
        char fileName[256] ={'\0'}; 
        sprintf(fileName, "/%s/shot_yuv%dx%d.yuv", MEDIA_PATH, rDstImgBufInfo1.u4ImgWidth, rDstImgBufInfo1.u4ImgHeight); 
        MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rDstImgBufInfo1.u4BufVA), rDstImgBufInfo1.u4BufSize);         

        ::memset(fileName, '\0', 256); 
        sprintf(fileName,"/%s/shot_pv%d%d.yuv", MEDIA_PATH,  rDstImgBufInfo2.u4ImgWidth, rDstImgBufInfo2.u4ImgHeight); 
        MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rDstImgBufInfo2.u4BufVA), rDstImgBufInfo2.u4BufSize);         
    }
    // (10). uninit 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "uninit PostProcPipe");
    pPostProcPipe->uninit(); 
    // (11). destory instance 
    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "destroy PostProcPipe");
    pPostProcPipe->destroyInstance(); 

    profile.print(""); 

    CPTLogStr(Event_MShot_createYuvRawImg, CPTFlagSeparator, "unlockResIP");
    unlock(PIPE_MGR_DRV_PIPE_MASK_POST_PROC); 
    FUNCTION_LOG_END;

    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL 
MultiShot::
createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 u4Img1Rot, MUINT32 u4Img1Flip, ImgBufInfo const & rDstImgBufInfo)
{
    FUNCTION_LOG_START;
    if (!lock(RES_MGR_DRV_SCEN_HW_IP, PIPE_MGR_DRV_PIPE_MASK_POST_PROC, 3000))
    {
        MY_LOGE("[createYuvRawImg] lock resource fail"); 
        return MFALSE; 
    }
    // (1). Create Instance 
    IPostProcPipe    *pPostProcPipe = IPostProcPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL,  static_cast<EScenarioFmt>(mapScenarioType(rSrcImgBufInfo.eImgFmt))); 
    CHECK_OBJECT(pPostProcPipe); 
   
    // (2). Query port property
    // (3). init 
    pPostProcPipe->init(); 
    // (4). setCallback
    pPostProcPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    // 
    MemoryInPortInfo rMemInPort(ImgInfo(rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight), 
                                0, rSrcImgBufInfo.u4Stride, Rect(rSrcCropRect.x, rSrcCropRect.y, rSrcCropRect.w, rSrcCropRect.h)); 
    //
    MemoryOutPortInfo rVdoPort(ImgInfo(rDstImgBufInfo.eImgFmt, rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight), 
                               rDstImgBufInfo.u4Stride, u4Img1Rot, u4Img1Flip);   
    rVdoPort.index = 1;   
    //
    vector<PortInfo const*> vInPorts;  
    vector<PortInfo const*> vOutPorts; 
    //
    vInPorts.push_back(&rMemInPort); 
    vOutPorts.push_back(&rVdoPort); 
    //
    pPostProcPipe->configPipe(vInPorts, vOutPorts); 
    // (6). Enqueue, In buf
    // 
    QBufInfo rInQBuf; 
    BufInfo rInBufInfo(rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.i4MemID);  
    rInQBuf.vBufInfo.push_back(rInBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryIn, 0, 0), rInQBuf); 

    // (6.1) Enqueue, Yuv out Buf
    QBufInfo rVdoQBuf; 
    BufInfo rVdoBufInfo(rDstImgBufInfo.u4BufSize, rDstImgBufInfo.u4BufVA, rDstImgBufInfo.u4BufPA, rDstImgBufInfo.i4MemID); 
    rVdoQBuf.vBufInfo.push_back(rVdoBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rVdoQBuf); 

    // (7). start
    pPostProcPipe->start(); 
    // (8). YUV Dequeue
    QTimeStampBufInfo rQVdoOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQVdoOutBuf); 
    // (8.1) In buffer dequeue 
    QTimeStampBufInfo rQInBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryIn, 0, 0), rQInBuf); 

    // (9). Stop 
    pPostProcPipe->stop();    

    if (mu4DumpFlag) 
    {
        char fileName[256] = {'\0'}; 
        sprintf(fileName, "/%s/shot_yuv%dx%d.yuv", MEDIA_PATH, rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight); 
        MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rDstImgBufInfo.u4BufVA), rDstImgBufInfo.u4BufSize);         
   }
    // (10). uninit 
    pPostProcPipe->uninit(); 
    // (11). destory instance 
    pPostProcPipe->destroyInstance(); 
    unlock(PIPE_MGR_DRV_PIPE_MASK_POST_PROC); 
    FUNCTION_LOG_END;

    return MTRUE; 
}



/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
createJpegImg(ImgBufInfo const & rSrcImgBufInfo, JpegParam const & rJpgParm, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rJpgImgBufInfo, MUINT32 & u4JpegSize)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_createJpegImg);    
    //MY_LOGD("createJpegImg, in (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.i4MemID); 
    //MY_LOGD("createJpegImg, out (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rJpgImgBufInfo.u4BufVA, rJpgImgBufInfo.u4BufPA, rJpgImgBufInfo.u4BufSize, rJpgImgBufInfo.i4MemID); 
    MtkCamUtils::CamProfile profile("createJpegImg", "MultiShot");
    //
    // (1). Create Instance    
    CPTLogStr(Event_MShot_createJpegImg, CPTFlagSeparator, "create SImager");
    ISImager *pISImager = ISImager::createInstance(rSrcImgBufInfo); 
    CHECK_OBJECT(pISImager); 

    // init setting 
    CPTLogStr(Event_MShot_createJpegImg, CPTFlagSeparator, "init SImager setting");
    BufInfo rBufInfo(rJpgImgBufInfo.u4BufSize, rJpgImgBufInfo.u4BufVA, rJpgImgBufInfo.u4BufPA, rJpgImgBufInfo.i4MemID); 
    //
    pISImager->setTargetBufInfo(rBufInfo); 
    //
    pISImager->setStrideAlign(rSrcImgBufInfo.u4Stride);
    //
    pISImager->setFormat(eImgFmt_JPEG); 
    //
    pISImager->setRotation(u4Rot); 
    //
    pISImager->setFlip(u4Flip); 
    // 
    pISImager->setResize(rJpgImgBufInfo.u4ImgWidth, rJpgImgBufInfo.u4ImgHeight); 
    //
    pISImager->setEncodeParam(rJpgParm.fgIsSOI, rJpgParm.u4Quality); 
    //
    pISImager->setROI(Rect(0, 0, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight)); 
    //
    CPTLogStr(Event_MShot_createJpegImg, CPTFlagSeparator, "execute");
    pISImager->execute(); 
    //
    CPTLogStr(Event_MShot_createJpegImg, CPTFlagSeparator, "get jpeg size");
    u4JpegSize = pISImager->getJpegSize(); 
    
    CPTLogStr(Event_MShot_createJpegImg, CPTFlagSeparator, "destroy SImager");
    pISImager->destroyInstance(); 

    profile.print();
    FUNCTION_LOG_END;
    return MTRUE; 
}
/*******************************************************************************
* 
********************************************************************************/
MBOOL    
MultiShot::
lock(MUINT32 u4HWScenario, MUINT32 u4PipeType,  MUINT32 const u4TimeOutInMs)
{
    //
    mpPipeMgrDrv = PipeMgrDrv::CreateInstance();
    CHECK_OBJECT(mpPipeMgrDrv); 
    mpPipeMgrDrv->Init();    
    // 
    mpResMgrDrv = ResMgrDrv::CreateInstance();
    CHECK_OBJECT(mpResMgrDrv); 
    mpResMgrDrv->Init();
    //
    RES_MGR_DRV_MODE_STRUCT rResMgrMode; 
    rResMgrMode.Dev = RES_MGR_DRV_DEV_CAM; 
    rResMgrMode.ScenSw = RES_MGR_DRV_SCEN_SW_CAM_CAP; 
    rResMgrMode.ScenHw = static_cast<RES_MGR_DRV_SCEN_HW_ENUM>(u4HWScenario); 
    if (!mpResMgrDrv->SetMode(&rResMgrMode))
    {
        MY_LOGE("fail to set resource mode"); 
        return MFALSE; 
    }
    //
    PIPE_MGR_DRV_LOCK_STRUCT rPipeMgrMode; 
    rPipeMgrMode.Timeout = u4TimeOutInMs; 
    rPipeMgrMode.PipeMask = u4PipeType; 
    if (!mpPipeMgrDrv->Lock(&rPipeMgrMode))
    {
        MY_LOGE("fail to lock pipe"); 
        return MFALSE; 
    }

    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
unlock(MUINT32 u4PipeType)
{
    CHECK_OBJECT(mpPipeMgrDrv); 
    CHECK_OBJECT(mpResMgrDrv); 
    //
    PIPE_MGR_DRV_UNLOCK_STRUCT rPipeMgrMode; 
    rPipeMgrMode.PipeMask = u4PipeType; 
    //    
    if (!mpPipeMgrDrv->Unlock(&rPipeMgrMode))
    {
        MY_LOGE("fail to unlock pipe"); 
        return MFALSE;      
    }
    //
    mpPipeMgrDrv->Uninit(); 
    mpPipeMgrDrv->DestroyInstance(); 
    mpPipeMgrDrv = NULL; 
    //
    mpResMgrDrv->Uninit(); 
    mpResMgrDrv->DestroyInstance(); 
    mpResMgrDrv = NULL; 
    return MTRUE; 
}
// [CS]+
/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
onCreateImage()
{
    AutoCPTLog cptlog(Event_MShot_onCreateImage);
    mpYuvImageCreateThread->postCommand(Command(Command::eID_WAKEUP));
    MUINT32 u4ShotCount = 0;
    //MBOOL bCShotEndCB = false;

    // (3) loop, handle jpeg buffer
    while(u4ShotCount<mu4ShotCount)
    {
        
        CPTLogStr(Event_MShot_onCreateImage, CPTFlagSeparator, "wait jpeg done");
        ::sem_wait(&semJpeg);

        CPTLogStr(Event_MShot_onCreateImage, CPTFlagSeparator, "handle callback");
        if(mbIsLastShot || u4ShotCount==mu4ShotCount-1)  // last frame
        {
            MY_LOGD("notify last shot will callback");
            handleNotifyCallback(ECamShot_NOTIFY_MSG_CSHOT_END, 0, 0); 
            handleDataCallback(ECamShot_DATA_MSG_JPEG, (mThumbImgBufInfoReady.u4BufVA), mu4ThumbnailSize, reinterpret_cast<MUINT8*>(mJpegImgBufInfoReady.u4BufVA), mu4JpegSize); 
            break;
        }
        
        handleDataCallback(ECamShot_DATA_MSG_JPEG, (mThumbImgBufInfoReady.u4BufVA), mu4ThumbnailSize, reinterpret_cast<MUINT8*>(mJpegImgBufInfoReady.u4BufVA), mu4JpegSize); 
        u4ShotCount++;
    }
    
    // (7) start end
    CPTLogStr(Event_MShot_start, CPTFlagSeparator, "post start end sem");
    ::sem_post(&semStartEnd);

    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
onCreateYuvImage()
{ 
    //FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_onCreateYuvImage);
    MtkCamUtils::CamProfile profile("startOne", "MultiShot");
    
    MBOOL ret = MTRUE; 

    if(mbIsLastShot)
    {
        MY_LOGD("last jpeg has been done, no need create yuv image"); 
        return MFALSE;
    }
    
    CPTLogStr(Event_MShot_onCreateYuvImage, CPTFlagSeparator, "notify sof");
    // Start Of Frame notify
#warning [TODO] this should callback from pipe 
    handleNotifyCallback(ECamShot_NOTIFY_MSG_SOF, 0, 0); 

    
    // (1) create raw image 
    // In MT6589, it always need to dump bayer raw image due to 
    // the capture is 3 pass, 
    // 1st pass: Sensor -> TG --> Memory (Raw(bayer),  YUV(yuy2)) 
    // 2nd pass: memory (bayer/yuy2) --> post proc -> mem (yuv, postview) 
    // 3nd pass: memory (yuv) --> jpeg --> mem (bitstream) 
    CPTLogStr(Event_MShot_onCreateYuvImage, CPTFlagSeparator, "create raw image");
    ret = ret 
           && createSensorRawImg(mSensorParam, mRawImgBufInfo)
           && handleDataCallback(ECamShot_DATA_MSG_BAYER, 0, 0, reinterpret_cast<MUINT8*>(mRawImgBufInfo.u4BufVA), mRawImgBufInfo.u4BufSize); 


    mSensorParam.fgBypassDelay = MTRUE;
    mSensorParam.fgBypassScenaio = MTRUE;
    CPTLogStr(Event_MShot_onCreateYuvImage, CPTFlagSeparator, "notify eof");
    handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0); 
    
    Rect rSrcRect(0, 0, mRawImgBufInfo.u4ImgWidth, mRawImgBufInfo.u4ImgHeight); 
    Rect rDstRect(0, 0, mShotParam.u4PictureWidth, mShotParam.u4PictureHeight); 

    // calc the zoom crop ratio 
    Rect rRect = MtkCamUtils::calCrop(rSrcRect, rDstRect, mShotParam.u4ZoomRatio); 
 
    if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_POSTVIEW|ECamShot_DATA_MSG_JPEG))
    {        
        // (2) create yuv image 
        // In MT6589, the postview will be ouput in the 2nd pass 
        // and the yuv image is created in 2nd pass 
        if (isDataMsgEnabled(ECamShot_DATA_MSG_JPEG)) 
        {

            //(2.1) create YUV image and callback quickview
            CPTLogStr(Event_MShot_onCreateYuvImage, CPTFlagSeparator, "create yuv image");
            ret = ret 
                   && createYuvRawImg(mRawImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, mYuvImgBufInfoWrite, mPostViewImgBufInfoWrite);
                   
            CPTLogStr(Event_MShot_onCreateYuvImage, CPTFlagSeparator, "postview callback");
            ret = ret 
                   &&  handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(mPostViewImgBufInfoWrite.u4BufVA), mPostViewImgBufInfoWrite.u4BufSize);
            
            // (2.2) update new ready buffer and trigger the JpegCreateThread
            CPTLogStr(Event_MShot_onCreateYuvImage, CPTFlagSeparator, "update ready buffer");
            updateReadyBuf();
        }
        else if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV) && isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW))
        {
            //ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
            //ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
        
            ret = ret 
                   && createYuvRawImg(mRawImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, mYuvImgBufInfoWrite, mPostViewImgBufInfoWrite) 
                   && handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(mPostViewImgBufInfoWrite.u4BufVA), mPostViewImgBufInfoWrite.u4BufSize)
                   && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(mYuvImgBufInfoWrite.u4BufVA), mYuvImgBufInfoWrite.u4BufSize);       
        }
        else if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV))
        { 
            //ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
            ret = ret 
                   && createYuvRawImg(mRawImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, mYuvImgBufInfoWrite) 
                   && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(mYuvImgBufInfoWrite.u4BufVA), mYuvImgBufInfoWrite.u4BufSize);            
        }
        else if (isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW)) 
        {
            //! should not enter this case 
            //ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
            ret = ret 
                   && createYuvRawImg(mRawImgBufInfo, rRect, mShotParam.u4PostViewRotation, mShotParam.u4PostViewFlip, mYuvImgBufInfoWrite) 
                   && handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(mPostViewImgBufInfoWrite.u4BufVA), mPostViewImgBufInfoWrite.u4BufSize);
        }
    }
    
    profile.print(); 
    //FUNCTION_LOG_END;
    return MTRUE; 
}
/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
onCreateThumbnailImage() 
{ 
    //FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_onCreateThumbImage);

    MUINT32 u4ThumbnailSize = 0; 
    MBOOL ret = MTRUE; 

    JpegParam rParam(mJpegParam.u4ThumbQuality, mJpegParam.fgThumbIsSOI);                 
    // postview w/o rotation, hence thumbnail should rotate
    ret = createJpegImgSW(mPostViewImgBufInfoRead, mThumbImgBufInfoWrite, u4ThumbnailSize);

    //if (0 != mJpegParam.u4ThumbWidth && 0 != mJpegParam.u4ThumbHeight)
    //{
    //    JpegParam rParam(mJpegParam.u4ThumbQuality, mJpegParam.fgThumbIsSOI);                 
        // postview w/o rotation, hence thumbnail should rotate
    //    ret = ret && createJpegImg(mPostViewImgBufInfoRead,rParam,  mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, mThumbImgBufInfoWrite, u4ThumbnailSize); 
    //}

    // Fix me use DDP and SW Jpeg Enc

    //(3) thumbnail done
	mu4ThumbnailSize = u4ThumbnailSize; 
    ::sem_post(&semThumbnail);
    //FUNCTION_LOG_END;

    return MTRUE; 
}
/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
onCreateJpegImage() 
{ 
    //FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_MShot_onCreateJpegImage);

    MUINT32 u4JpegSize = 0; 
    MBOOL ret = MTRUE; 

    // (1) directly return if the last shot has been done.
    if(mbIsLastShot)
    {
        MY_LOGD("last jpeg has been done, no need create jpeg image"); 
        return MFALSE;
    }

    // (2) control shot speed
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "control shot speed");
    timeval tv;
    ::gettimeofday(&tv, NULL);
    
    MUINT32 usLast = mtvLastJpegStart.tv_sec * 1000000 + mtvLastJpegStart.tv_usec;
    MUINT32 usCurr = tv.tv_sec * 1000000 + tv.tv_usec;
    MUINT32 usDiff = (usCurr-usLast);
    MUINT32 usMinShotToShotTime = 1000000/mu4ShotSpeed; // us
    
    //  Make sure that each capture takes the specified time at least before starting the next capture.
    if  ( usDiff < usMinShotToShotTime )
    {
        MUINT32 const usSleep = usMinShotToShotTime - usDiff;
        MY_LOGD("[onCreateJpegImage] (mu4JpegCount, usSleep, usMinShotToShotTime)=(%d, %d, %d)", mu4JpegCount, usSleep, usMinShotToShotTime);
        ::usleep( usSleep );
    }

    ::gettimeofday(&mtvLastJpegStart, NULL);
    
    // (3) get read buffer for Jpeg and Thumbnail
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "get read buffer");
    getReadBuf();
    /*
    MUINT32 u4ThumbnailSize = 0; 
    
    //create thumbnail 
    if (0 != mJpegParam.u4ThumbWidth && 0 != mJpegParam.u4ThumbHeight)
    {
        JpegParam rParam(mJpegParam.u4ThumbQuality, mJpegParam.fgThumbIsSOI);                 
        // postview w/o rotation, hence thumbnail should rotate
        //ret = createJpegImg(mPostViewImgBufInfoRead,rParam,  mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, mThumbImgBufInfoWrite, u4ThumbnailSize); 
    
        ret = createJpegImgSW(mPostViewImgBufInfoRead, mThumbImgBufInfoWrite, u4ThumbnailSize);
        mu4ThumbnailSize = u4ThumbnailSize;
    }
*/
    // (4) trigger the ThumbnailCreateThread
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "trigger thumbnailb thread");
    if (0 != mJpegParam.u4ThumbWidth && 0 != mJpegParam.u4ThumbHeight)
    {
        mpThumbnailImageCreateThread->postCommand(Command(Command::eID_POSTVIEW_BUF));
    }
    
    // (5) callback YUV buffer if needed
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "callback YUV buffer");
    handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(mYuvImgBufInfoRead.u4BufVA), mYuvImgBufInfoRead.u4BufSize);

    // (6) create Jpeg
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "create Jpeg");
    createJpegImg(mYuvImgBufInfoRead, mJpegParam, 0, 0 , mJpegImgBufInfoWrite, u4JpegSize);

    // (7) wait thumbnail done
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "wait thumbnail done");
    if (0 != mJpegParam.u4ThumbWidth && 0 != mJpegParam.u4ThumbHeight)
    {
        ::sem_wait(&semThumbnail);
    }

    // (8) callback Jpeg
    CPTLogStr(Event_MShot_onCreateJpegImage, CPTFlagSeparator, "callback Jpeg");
    returnJpegBuf();
    mu4JpegSize = u4JpegSize; 
    mu4JpegCount++;
    if(mbCancelShot || mu4JpegCount==mu4ShotCount)
    {
        mbIsLastShot = MTRUE;
        ret = MFALSE;
        MY_LOGD("Last Jpeg done");
    }
    ::sem_post(&semJpeg);
    //FUNCTION_LOG_END;

    return ret; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
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
MBOOL
MultiShot::
uninitImageCreateThread()
{
//#if 0
    if  ( mpImageCreateThread != 0 )
    {
        MY_LOGD(
            "ImageCreateThread: (tid, getStrongCount, mpImageCreateThread)=(%d, %d, %p)", 
            mpImageCreateThread->getTid(), mpImageCreateThread->getStrongCount(), mpImageCreateThread.get()
        );
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        mpImageCreateThread->requestExit();
        status_t status = OK;
        if  ( OK != (status = mpImageCreateThread->join()) )
        {
            MY_LOGW("Not to wait ImageCreateThread(tid:%d), status[%s(%d)]", mpImageCreateThread->getTid(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        mpImageCreateThread = NULL;
    }

    if  ( mpYuvImageCreateThread != 0 )
    {
        MY_LOGD(
            "YuvImageCreateThread: (tid, getStrongCount, mpYuvImageCreateThread)=(%d, %d, %p)", 
            mpYuvImageCreateThread->getTid(), mpYuvImageCreateThread->getStrongCount(), mpYuvImageCreateThread.get()
        );
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        mpYuvImageCreateThread->requestExit();
        status_t status = OK;
        if  ( OK != (status = mpYuvImageCreateThread->join()) )
        {
            MY_LOGW("Not to wait YuvImageCreateThread(tid:%d), status[%s(%d)]", mpYuvImageCreateThread->getTid(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        mpYuvImageCreateThread = NULL;
    }
//#endif

    if  ( mpThumbnailImageCreateThread != 0 )
    {
        MY_LOGD(
            "ThumbnailImageCreateThread: (tid, getStrongCount, mpThumbnailImageCreateThread)=(%d, %d, %p)", 
            mpThumbnailImageCreateThread->getTid(), mpThumbnailImageCreateThread->getStrongCount(), mpThumbnailImageCreateThread.get()
        );
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        mpThumbnailImageCreateThread->requestExit();
        status_t status = OK;
        if  ( OK != (status = mpThumbnailImageCreateThread->join()) )
        {
            MY_LOGW("Not to wait ThumbnailImageCreateThread(tid:%d), status[%s(%d)]", mpThumbnailImageCreateThread->getTid(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        mpThumbnailImageCreateThread = NULL;
    }
//#if 0

    
    if  ( mpJpegImageCreateThread != 0 )
    {
        MY_LOGD(
            "JpegImageCreateThread: (tid, getStrongCount, mpJpegImageCreateThread)=(%d, %d, %p)", 
            mpJpegImageCreateThread->getTid(), mpJpegImageCreateThread->getStrongCount(), mpJpegImageCreateThread.get()
        );
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        mpJpegImageCreateThread->requestExit();
        status_t status = OK;
        if  ( OK != (status = mpJpegImageCreateThread->join()) )
        {
            MY_LOGW("Not to wait JpegImageCreateThread(tid:%d), status[%s(%d)]", mpJpegImageCreateThread->getTid(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        mpJpegImageCreateThread = NULL;
    }
    
//#endif

    return MTRUE;  
}

/*******************************************************************************
* 
********************************************************************************/
MVOID
MultiShot::
updateReadyBuf()
{
    //FUNCTION_LOG_START;

    Mutex::Autolock lock(mYuvReadyBufMtx);

    ImgBufInfo rYuvImgBufInfo = mYuvImgBufInfoWrite; 
    mYuvImgBufInfoWrite = mYuvImgBufInfoReady;
    mYuvImgBufInfoReady = rYuvImgBufInfo;
    
    ImgBufInfo rPostViewBufInfo = mPostViewImgBufInfoWrite; 
    mPostViewImgBufInfoWrite = mPostViewImgBufInfoReady;
    mPostViewImgBufInfoReady = rPostViewBufInfo;

    mFocusValReady = mFocusValWrite;
    
    if(!mbJpegSemPost)
    {
        MY_LOGD("trigger JpegThread");
        mbJpegSemPost = MTRUE;
        mpJpegImageCreateThread->postCommand(Command(Command::eID_YUV_BUF));  // must to trigger here, not all YUV cpmpressed to Jpeg
    }

    //FUNCTION_LOG_END;
}
/*******************************************************************************
* 
********************************************************************************/
MVOID
MultiShot::
getReadBuf()
{
    //FUNCTION_LOG_START;

    Mutex::Autolock lock(mYuvReadyBufMtx);
    
    ImgBufInfo rYuvImgBufInfo = mYuvImgBufInfoRead; 
    mYuvImgBufInfoRead = mYuvImgBufInfoReady;
    mYuvImgBufInfoReady = rYuvImgBufInfo;

    
    ImgBufInfo rPostViewBufInfo = mPostViewImgBufInfoRead; 
    mPostViewImgBufInfoRead = mPostViewImgBufInfoReady;
    mPostViewImgBufInfoReady = rPostViewBufInfo;

    mFocusValRead = mFocusValReady;
    
    mbJpegSemPost = MFALSE;  // means new Jpeg compress begin
    
    //FUNCTION_LOG_END;
}
/*******************************************************************************
* 
********************************************************************************/
MVOID
MultiShot::
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
    
    //FUNCTION_LOG_END;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
convertImage(ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, int rot)
{
    //return MFALSE;
    AutoCPTLog cptlog(Event_MShot_convertImage);

    MY_LOGD("convertImage, src (VA, PA, Size, ID)(W, H) = (0x%x, 0x%x, %d, %d)(%d, %d)", rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.i4MemID, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight); 
    MY_LOGD("convertImage, dst (VA, PA, Size, ID)(W, H) = (0x%x, 0x%x, %d, %d)(%d, %d)(rot:%d)", rDstImgBufInfo.u4BufVA, rDstImgBufInfo.u4BufPA, rDstImgBufInfo.u4BufSize, rDstImgBufInfo.i4MemID, rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight, rot); 

    MtkCamUtils::CamProfile profile("convertImage", "MultiShot");

    DpBlitStream thumbnailStream;
    DpColorFormat dp_in_fmt;
    DpInterlaceFormat dp_interlace_fmt = eInterlace_None;
    
    switch (mPostViewImgBufInfoRead.eImgFmt) {
        case eImgFmt_YV12:
            dp_in_fmt = eYUV_420_3P;
            break;
        default:
            return MFALSE;
            break;
    }

    unsigned char *src_yp = (unsigned char *)(rSrcImgBufInfo.u4BufVA); //
    int src_ysize = rSrcImgBufInfo.u4ImgWidth * rSrcImgBufInfo.u4ImgHeight; //
    int src_usize, src_vsize;  //
    src_usize = src_vsize = src_ysize / 4;  //
    
    unsigned int src_addr_list[3];//
    unsigned int src_size_list[3];//
    int plane_num;//
    // set & register src buffer
    switch(rSrcImgBufInfo.eImgFmt) {
        case eImgFmt_YV12:  
            src_addr_list[0] = (unsigned int)src_yp;
            src_addr_list[1] = (unsigned int)(src_yp + src_ysize * 5 / 4);
            src_addr_list[2] = (unsigned int)(src_yp + src_ysize);
    
            src_size_list[0] = src_ysize;
            src_size_list[1] = src_vsize;
            src_size_list[2] = src_usize;

            plane_num = 3;
            break;
        default:
            return MFALSE;
            break;
    }

    unsigned char *dst_yp = (unsigned char *)(rDstImgBufInfo.u4BufVA);
    //unsigned char *ori_yp;
    //ori_yp  = (unsigned char *)(mPostViewImgBufInfoRead.u4BufVA);
   
    int dst_ysize = rDstImgBufInfo.u4ImgWidth*rDstImgBufInfo.u4ImgHeight;  
    int dst_usize, dst_vsize;
    dst_usize = dst_vsize = dst_ysize / 4;
    unsigned int dst_addr_list[3];
    unsigned int dst_size_list[3];
    int plane_num_out;//
    switch(rSrcImgBufInfo.eImgFmt) {
        case eImgFmt_YV12:  
            dst_addr_list[0] = (unsigned int)dst_yp;
            dst_addr_list[1] = (unsigned int)(dst_yp + dst_ysize * 5 / 4);
            dst_addr_list[2] = (unsigned int)(dst_yp + dst_ysize);
    
            dst_size_list[0] = dst_ysize;
            dst_size_list[1] = dst_vsize;
            dst_size_list[2] = dst_usize;

            plane_num_out = 3;
            break;
        default:
            return MFALSE;
            break;
    }

    
    //MY_LOGD("src addr (0, 1, 2) = (%p, %p, %p)", src_addr_list[0], src_addr_list[1], src_addr_list[2]); 
    //MY_LOGD("dst addr (0, 1, 2) = (%p, %p, %p)", dst_addr_list[0], dst_addr_list[1], dst_addr_list[2]); 

    CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set src buffer");

    // set src buffer
    if (rSrcImgBufInfo.i4MemID > 0) {
        thumbnailStream.setSrcBuffer(rSrcImgBufInfo.i4MemID, src_size_list, plane_num);
    } else {
        thumbnailStream.setSrcBuffer((void**)src_addr_list, src_size_list, plane_num);
    }
    
    CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set src config");
    thumbnailStream.setSrcConfig(rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight, dp_in_fmt, dp_interlace_fmt);

    CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set dst buffer");        
    if (rDstImgBufInfo.i4MemID > 0) 
    {
        thumbnailStream.setDstBuffer(rDstImgBufInfo.i4MemID, dst_size_list, plane_num_out);
    }
    else
    {
        thumbnailStream.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    }
    
    CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set dst config");        
    if (rot == 90 || rot == 270)
	{
    	thumbnailStream.setDstConfig(rDstImgBufInfo.u4ImgHeight, rDstImgBufInfo.u4ImgWidth, eYUV_420_3P);
	}
	else
	{
    	thumbnailStream.setDstConfig(rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight, eYUV_420_3P);
	}
    thumbnailStream.setRotate(rot);

    //*****************************************************************************//
    CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "invalidate");        
    DP_STATUS_ENUM ret = thumbnailStream.invalidate();  //trigger HW
    if ( ret < 0 )
    {
          MY_LOGW("thumbnailStream invalidate failed %d", ret);
          return MFALSE;
    }
    CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "invalidate end");        
    profile.print(); 
    if (mu4DumpFlag) 
    { 
        char fileNames[256] = {'\0'}; 
        sprintf(fileNames, "/%s/convert_yuv_%d_%dx%d_src.yuv", MEDIA_PATH, mu4JpegCount, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight); 
        MtkCamUtils::saveBufToFile(fileNames, reinterpret_cast<MUINT8*>( rSrcImgBufInfo.u4BufVA), rSrcImgBufInfo.u4BufSize);   

            
        char fileNamed[256] = {'\0'}; 

		
		if (rot == 90 || rot == 270)
		{
        	sprintf(fileNamed, "/%s/convert_yuv_%d_%dx%d_dst.yuv", MEDIA_PATH, mu4JpegCount, rDstImgBufInfo.u4ImgHeight, rDstImgBufInfo.u4ImgWidth); 
		}
		else
		{
        sprintf(fileNamed, "/%s/convert_yuv_%d_%dx%d_dst.yuv", MEDIA_PATH, mu4JpegCount, rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight); 
		}

        MtkCamUtils::saveBufToFile(fileNamed, reinterpret_cast<MUINT8*>( rDstImgBufInfo.u4BufVA), rDstImgBufInfo.u4BufSize);         
    }    
    return MTRUE;  
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
MultiShot::
createJpegImgSW(ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, MUINT32 & u4JpegSize)
{
    AutoCPTLog cptlog(Event_MShot_createJpegImgSW);
    if(rSrcImgBufInfo.eImgFmt != eImgFmt_YV12)
    {
        MY_LOGW("createJpegImgSW failed, src fmt is %d", rSrcImgBufInfo.eImgFmt);
        return MFALSE;
    }

	MBOOL bNeedTwiceResize = MFALSE;

    if (rDstImgBufInfo.u4ImgWidth*6 < rSrcImgBufInfo.u4ImgWidth ||
         rDstImgBufInfo.u4ImgHeight*6 < rSrcImgBufInfo.u4ImgHeight)
    {
        MY_LOGD("Need twice resize, Resize >6x src =(%d,%d), dst=(%d,%d), rot=(%d)",
                 rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight,
                 rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight, mShotParam.u4PictureRotation);
        bNeedTwiceResize  = MTRUE;
    }

	if(bNeedTwiceResize) 
	{
		convertImage(rSrcImgBufInfo, mThumbImgBufInfoTemp, 0);
		
		convertImage(mThumbImgBufInfoTemp, mThumbImgBufInfoYuv, mShotParam.u4PictureRotation);
	}
	else
	{
		convertImage(rSrcImgBufInfo, mThumbImgBufInfoYuv, mShotParam.u4PictureRotation);
	}
    

    YV12ToJpeg(mThumbImgBufInfoYuv, rDstImgBufInfo, u4JpegSize, mShotParam.u4PictureRotation);
        
    return MTRUE;  
}
MBOOL
MultiShot::
YV12ToJpeg(ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, MUINT32 & u4JpegSize, int rot)
{
    AutoCPTLog cptlog(Event_MShot_YV12ToJpeg);

    MY_LOGD("YV12ToJpeg, src (VA, PA, Size, ID)(w,h) = (0x%x, 0x%x, %d, %d)(%d, %d)", rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.i4MemID,  rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight); 
    MY_LOGD("YV12ToJpeg, dst (VA, PA, Size, ID)(w,h) = (0x%x, 0x%x, %d, %d)(%d, %d)", rDstImgBufInfo.u4BufVA, rDstImgBufInfo.u4BufPA, rDstImgBufInfo.u4BufSize, rDstImgBufInfo.i4MemID,  rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight); 

    unsigned char* Y = (unsigned char *)(rSrcImgBufInfo.u4BufVA);
    unsigned char* U = Y + rSrcImgBufInfo.u4ImgWidth * rSrcImgBufInfo.u4ImgHeight*5/4;
    unsigned char* V  =  Y + rSrcImgBufInfo.u4ImgWidth * rSrcImgBufInfo.u4ImgHeight; 

	int width = 0;
	int height = 0;
	
    if (rot == 90 || rot == 270)
	{
	
	    width = rSrcImgBufInfo.u4ImgHeight;
	    height = rSrcImgBufInfo.u4ImgWidth;
	}
	else
	{
	
		width = rSrcImgBufInfo.u4ImgWidth;
		height = rSrcImgBufInfo.u4ImgHeight;
	}
	
    int quality = mJpegParam.u4Quality;
    unsigned char* dst = (unsigned char *)(rDstImgBufInfo.u4BufVA); 
    long unsigned int jpegSize = rDstImgBufInfo.u4BufSize;

    //if (width %8 != 0 || height % 8 != 0) return -1;
    int i,j;
    JSAMPROW y[16],cb[16],cr[16]; 
    // y[2][5] = color sample of row 2 and pixel column 5; (one plane)
    JSAMPARRAY data[3]; 
    // t[0][2][5] = color sample 0 of row 2 and column 5
    data[0] = y;
    data[1] = cb;
    data[2] = cr;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    // errors get written to stderr
    cinfo.err = jpeg_std_error(&jerr);  
    
    CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "create compress");
    jpeg_create_compress (&cinfo);

    CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "set defaults");
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    jpeg_set_defaults (&cinfo);
    //jpeg_set_colorspace(&cinfo, JCS_YCbCr);
    cinfo.raw_data_in = true; 

    // supply downsampled data
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 2;
    cinfo.comp_info[1].h_samp_factor = 1;
    cinfo.comp_info[1].v_samp_factor = 1;
    cinfo.comp_info[2].h_samp_factor = 1;
    cinfo.comp_info[2].v_samp_factor = 1;
    
    CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "set quality");
    jpeg_set_quality (&cinfo, quality, true);
    //cinfo.dct_method = JDCT_FLOAT;
    cinfo.dct_method = JDCT_IFAST;

    CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "mem dest");
    jpeg_mem_dest (&cinfo, &dst, &jpegSize);     
//    #warning "[FIXME] jpeg_mem_dest not ready"
    // data written to file
    CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "start compress");
    jpeg_start_compress (&cinfo, TRUE);

    /*
    for (j=0;j < HEIGHT;j+=16) 
    {
        for (i=0;i < 16;i++) 
        {
            y[i] = image + WIDTH*(i+j);
            if (i%2 == 0) 
            {
                cb[i/2] = image + WIDTH * HEIGHT + WIDTH/2*((i+j)/2);
                cr[i/2] = image + WIDTH * HEIGHT + WIDTH * HEIGHT/4 + WIDTH/2*((i+j)/2);
            }
        }
        jpeg_write_raw_data (&cinfo, data, 16);
    }
    */
   CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "write raw data");
    for (j = 0; j < height; j += 16) 
    {
        for (i = 0; i < 16; i++) 
        {
            y[i] = Y + i * width;
            if (i%2 == 0) 
            { 
                cb[i/2] = U + (i/2) * ( width / 2 );
                cr[i/2] = V + (i/2) * ( width / 2 );
            }
       }
        
        jpeg_write_raw_data (&cinfo, data, 16);
        Y = Y + 16 * width;
        U = U + 8 * (width / 2);
        V = V + 8 * (width / 2);
    }
    
    CPTLogStr(Event_MShot_YV12ToJpeg, CPTFlagSeparator, "finish/destroy compress");
    jpeg_finish_compress (&cinfo);
    jpeg_destroy_compress (&cinfo);

    u4JpegSize = jpegSize;

    if (mu4DumpFlag) 
    { 
        char fileNames[256] = {'\0'}; 
        sprintf(fileNames, "/%s/thumb_yuv_thumb_%d_%dx%d.yuv", MEDIA_PATH, mu4JpegCount, width, height); 
        MtkCamUtils::saveBufToFile(fileNames, reinterpret_cast<MUINT8*>( rSrcImgBufInfo.u4BufVA), rSrcImgBufInfo.u4BufSize);   

        char fileNamed[256] = {'\0'}; 
        sprintf(fileNamed, "/%s/thumb_jpeg_%d_%d.jpg", MEDIA_PATH, mu4JpegCount, u4JpegSize); 
        MtkCamUtils::saveBufToFile(fileNamed, reinterpret_cast<MUINT8*>( rDstImgBufInfo.u4BufVA), u4JpegSize);         
    }

    return MTRUE;  
}

// [CS]-

/*******************************************************************************
* 
********************************************************************************/
MBOOL   
MultiShot::
registerImgBufInfo(ECamShotImgBufType const eBufType, ImgBufInfo const &rImgBuf)
{
     FUNCTION_LOG_START;
     MY_LOGD("[registerImgBufInfo] type = %d", eBufType); 
     MY_LOGD("[registerImgBufInfo] (width, height, format) = (%d, %d, 0x%x)", rImgBuf.u4ImgWidth, rImgBuf.u4ImgHeight, rImgBuf.eImgFmt); 
     MY_LOGD("[registerImgBufInfo] (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rImgBuf.u4BufVA, rImgBuf.u4BufPA, rImgBuf.u4BufSize, rImgBuf.i4MemID); 
     if (ECamShot_BUF_TYPE_BAYER == eBufType) 
     {
         mRawImgBufInfo = rImgBuf; 
     }
     else if (ECamShot_BUF_TYPE_YUV == eBufType)  
     {
         mYuvImgBufInfo = rImgBuf; 
     }
     else if (ECamShot_BUF_TYPE_POSTVIEW == eBufType)
     {
         mPostViewImgBufInfo = rImgBuf;
     }
     else if (ECamShot_BUF_TYPE_JPEG == eBufType)
     {
         mJpegImgBufInfo = rImgBuf;
     }
     else if (ECamShot_BUF_TYPE_THUMBNAIL == eBufType)
     {
         mThumbImgBufInfo = rImgBuf;
     }
     FUNCTION_LOG_END;
     //
     return MTRUE;
}

/******************************************************************************
* 
*******************************************************************************/
MBOOL
MultiShot::
allocMem(IMEM_BUF_INFO & rMemBuf) 
{
    // 
    if (mpMemDrv->allocVirtBuf(&rMemBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
        return MFALSE;              
    }
    //::memset((void*)rMemBuf.virtAddr, 0 , rMemBuf.size);
#if 1
    if (mpMemDrv->mapPhyAddr(&rMemBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
        return MFALSE;        
    }
#endif 
    return MTRUE; 
}

/******************************************************************************
* 
*******************************************************************************/
MBOOL
MultiShot::
deallocMem(IMEM_BUF_INFO & rMemBuf)
{
    //
#if 1
    if (mpMemDrv->unmapPhyAddr(&rMemBuf)) 
    {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;              
    }
#endif
    //
    if (mpMemDrv->freeVirtBuf(&rMemBuf)) 
    {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;        
    }        
    rMemBuf.size = 0; 

    return MTRUE; 
}

/******************************************************************************
* 
*******************************************************************************/
MBOOL 
MultiShot::
reallocMem(IMEM_BUF_INFO & rMemBuf, MUINT32 const u4Size )
{   
    MBOOL ret = MTRUE;  
    //
    ret = deallocMem(rMemBuf); 
    rMemBuf.size = u4Size; 
    //
    ret = allocMem(rMemBuf);     
    return ret; 
}

/******************************************************************************
* 
*******************************************************************************/
MBOOL     
MultiShot::
allocImgMem(char const* const pszName, EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, IMEM_BUF_INFO & rMem)
{
    //
    MtkCamUtils::CamProfile profile("allocImgMem", "MultiShot");
    MY_LOGD("[allocImgMem] %s, (format, width, height) = (0x%x, %d, %d)", pszName, eFmt, u4Width, u4Height); 
    MUINT32 u4BufSize = queryImgBufSize(eFmt, u4Width, u4Height); 
    //
    if (0 == rMem.size) 
    {        
        rMem.size = (u4BufSize  + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
        allocMem(rMem); 
        MY_LOGD("[allocImgMem] (va, pa, size) = (0x%x, 0x%x, %d)",  rMem.virtAddr, rMem.phyAddr, rMem.size);  
    }
    else 
    {
        if (rMem.size < u4BufSize) 
        {          
            reallocMem(rMem, u4BufSize); 
            MY_LOGD("[allocImgMem] re-allocate (va, pa, size) = (0x%x, 0x%x, %d)", rMem.virtAddr, rMem.phyAddr, rMem.size);  
        }
    }  
    profile.print(); 
    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MVOID     
MultiShot::
setImageBuf(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height,ImgBufInfo & rBuf, IMEM_BUF_INFO & rMem)
{    
    rBuf.u4ImgWidth = u4Width;
    rBuf.u4ImgHeight = u4Height; 
    rBuf.eImgFmt = eFmt; 
    rBuf.u4Stride[0] = queryImgStride(eFmt, u4Width, 0); 
    rBuf.u4Stride[1] = queryImgStride(eFmt, u4Width, 1); 
    rBuf.u4Stride[2] = queryImgStride(eFmt, u4Width, 2); 
    rBuf.u4BufSize = rMem.size; 
    rBuf.u4BufVA = rMem.virtAddr;
    rBuf.u4BufPA = rMem.phyAddr;
    rBuf.i4MemID = rMem.memID; 
}


/*******************************************************************************
* 
********************************************************************************/
MVOID     
MultiShot::
freeShotMem()
{
    MtkCamUtils::CamProfile profile("freeShotMem", "MultiShot");
    // Raw 
    //if (0 != mRawMem.size) 
    //{
    //    deallocMem(mRawMem); 
    //}
    
    FUNCTION_LOG_START;
    for(MINT32 i=0; i<mRawMem.size(); i++)
    {
        if (0 != mRawMem[i].size) 
        {
            deallocMem(mRawMem[i]); 
        }
    }
    mRawMem.clear();
    // Yuv 
    //if (0 != mYuvMem.size) 
    //{
    //    deallocMem(mYuvMem); 
    //}
    
    for(MINT32 i=0; i<mYuvMem.size(); i++)
    {
        if (0 != mYuvMem[i].size) 
        {
            deallocMem(mYuvMem[i]); 
        }
    }
    mYuvMem.clear();
    // Postview 
    //if (0 != mPostViewMem.size)
    //{
    //    deallocMem(mPostViewMem); 
    //} 
    
    for(MINT32 i=0; i<mPostViewMem.size(); i++)
    {
        if (0 != mPostViewMem[i].size) 
        {
            deallocMem(mPostViewMem[i]); 
        }
    }
    mPostViewMem.clear();
    // Jpeg 
    //if (0 != mJpegMem.size)
    //{
    //    deallocMem(mJpegMem); 
    //}
    
    for(MINT32 i=0; i<mJpegMem.size(); i++)
    {
        if (0 != mJpegMem[i].size) 
        {
            deallocMem(mJpegMem[i]); 
        }
    }
    mJpegMem.clear();
    // Thumbnail 
    //if (0 != mThumbnailMem.size)
    //{
    //    deallocMem(mThumbnailMem); 
    //}
    
    for(MINT32 i=0; i<mThumbnailMem.size(); i++)
    {
        if (0 != mThumbnailMem[i].size) 
        {
            deallocMem(mThumbnailMem[i]); 
        }
    }
    mThumbnailMem.clear();
    FUNCTION_LOG_END;
    // [CS]-
    profile.print(); 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
querySensorRawImgBufInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mRawImgBufInfo.u4BufSize) 
    {
        return mRawImgBufInfo; 
    }

    // Raw Buffer 
    MUINT32 u4SensorWidth = 0, u4SensorHeight = 0; 
    EImageFormat eImgFmt = querySensorFmt(mSensorParam.u4DeviceID, mSensorParam.u4Scenario, mSensorParam.u4Bitdepth);        
    querySensorResolution(mSensorParam.u4DeviceID, mSensorParam.u4Scenario, u4SensorWidth, u4SensorHeight); 

    MY_LOGD("[querySensorRawImgBufInfo] Sensor (fmt, width, height) = (0x%x, %d, %d)", eImgFmt, u4SensorWidth, u4SensorHeight); 
    // [CS]+
	IMEM_BUF_INFO rawMem;
    memset(&rawMem, 0, sizeof(IMEM_BUF_INFO));
    allocImgMem("SensorRaw", eImgFmt, u4SensorWidth, u4SensorHeight, rawMem); 
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt, u4SensorWidth, u4SensorHeight, rImgBufInfo, rawMem); 
    mRawMem.push_back(rawMem);
    // {CS]-
    
    FUNCTION_LOG_END;
    return rImgBufInfo;    
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
queryYuvRawImgBufInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mYuvImgBufInfo.u4BufSize)
    {
        return mYuvImgBufInfo; 
    }

    //
    EImageFormat eImgFmt = mShotParam.ePictureFmt; 
    // YUV format not set, use YUY2 as default
    if (eImgFmt_UNKNOWN == eImgFmt || !isDataMsgEnabled(ECamShot_DATA_MSG_YUV))
    {
        eImgFmt = eImgFmt_YUY2; 
    } 
    MUINT32 u4Width = 0, u4Height = 0; 
    getPictureDimension(u4Width, u4Height);
    //
    // [CS]+
	IMEM_BUF_INFO yuvMem; 
    memset(&yuvMem, 0, sizeof(IMEM_BUF_INFO));
    allocImgMem("Yuv", eImgFmt, u4Width, u4Height, yuvMem); 
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt, u4Width, u4Height,  rImgBufInfo, yuvMem);         
    mYuvMem.push_back(yuvMem);
    // {CS]-
    
    FUNCTION_LOG_END;
    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
queryJpegImgBufInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mJpegImgBufInfo.u4BufSize) 
    {
        return mJpegImgBufInfo; 
    }
    
    // the Raw Mem is allocated in MultiShot, re-use raw mem
    MUINT32 u4Width = 0, u4Height = 0; 
    getPictureDimension(u4Width, u4Height);

    ImgBufInfo rImgBufInfo; 
    if (0 != 0 )//mRawMem.size) // [CS]+ not use Raw Mem
    {
        setImageBuf(eImgFmt_JPEG, u4Width, u4Height, rImgBufInfo, mRawMem[0]); 
    }
    else 
    {
        // [CS]+
    	IMEM_BUF_INFO jpegMem; 
        memset(&jpegMem, 0, sizeof(IMEM_BUF_INFO));
        allocImgMem("Jpeg", eImgFmt_JPEG, u4Width, u4Height, jpegMem); 
        setImageBuf(eImgFmt_JPEG, u4Width, u4Height,  rImgBufInfo, jpegMem);          
        mJpegMem.push_back(jpegMem);
    }
    
    FUNCTION_LOG_END;
    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
queryPostViewImgInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mPostViewImgBufInfo.u4BufSize)
    {
        return mPostViewImgBufInfo; 
    }
    // no postview format, use YUY2 as default for jpeg encode
    if (eImgFmt_UNKNOWN == mShotParam.ePostViewFmt) 
    {
        mShotParam.ePostViewFmt = eImgFmt_YV12;   // [CS]+
    }
    MUINT32 u4BufSize  = queryImgBufSize(mShotParam.ePostViewFmt, mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight); 
    // [CS]+
	IMEM_BUF_INFO postviewMem; 
    memset(&postviewMem, 0, sizeof(IMEM_BUF_INFO));
    allocImgMem("PostView", mShotParam.ePostViewFmt, mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight, postviewMem); 
    ImgBufInfo rImgBufInfo; 
    //
    setImageBuf(mShotParam.ePostViewFmt, mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight,  rImgBufInfo, postviewMem);
    mPostViewMem.push_back(postviewMem);
    // {CS]-
    FUNCTION_LOG_END;

    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
queryThumbImgBufInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mThumbImgBufInfo.u4BufSize) 
    {
        return mThumbImgBufInfo; 
    }
    //
    // [CS]+
    /*
    if (mThumbnailMem.size == 0)
    {
        mThumbnailMem.size = 64 * 1024; 
        MY_LOGD("allocate thumbnail mem, size = %d", mThumbnailMem.size); 
        allocMem(mThumbnailMem); 
    }
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt_JPEG, mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight, rImgBufInfo, mThumbnailMem); 
    */
	IMEM_BUF_INFO thumbnailMem; 
    
    memset(&thumbnailMem, 0, sizeof(IMEM_BUF_INFO));
    thumbnailMem.size = 128 * 1024; 
    MY_LOGD("allocate thumbnail mem, size = %d", thumbnailMem.size); 
    allocMem(thumbnailMem); 
    
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt_JPEG, mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight, rImgBufInfo, thumbnailMem); 
    mThumbnailMem.push_back(thumbnailMem);
    // {CS]-
    FUNCTION_LOG_END;

    return rImgBufInfo; 
}
/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
queryThumbYuvImgBufInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mThumbImgBufInfoYuv.u4BufSize) 
    {
        return mThumbImgBufInfoYuv; 
    }
    //
    
    //MUINT32 u4BufSize  = queryImgBufSize(mShotParam.ePostViewFmt,mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight); 
	IMEM_BUF_INFO thumbnailMem; 
    memset(&thumbnailMem, 0, sizeof(IMEM_BUF_INFO));
    allocImgMem("ThumbYuv", eImgFmt_YV12, mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight, thumbnailMem); 
    ImgBufInfo rImgBufInfo; 
    //
    setImageBuf(eImgFmt_YV12, mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight,  rImgBufInfo, thumbnailMem);
    mThumbnailMem.push_back(thumbnailMem);
    FUNCTION_LOG_END;

    return rImgBufInfo; 
}
/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
MultiShot::
queryThumbTempImgBufInfo()
{
    FUNCTION_LOG_START;

    // is upper layer register buffer 
    if (0 != mThumbImgBufInfoTemp.u4BufSize) 
    {
        return mThumbImgBufInfoTemp; 
    }
    //
    
	IMEM_BUF_INFO thumbnailMem; 
    memset(&thumbnailMem, 0, sizeof(IMEM_BUF_INFO));
    allocImgMem("ThumbYuv", eImgFmt_YV12, mJpegParam.u4ThumbWidth*5, mJpegParam.u4ThumbHeight*5, thumbnailMem); 
    ImgBufInfo rImgBufInfo; 
    //
    setImageBuf(eImgFmt_YV12,  mJpegParam.u4ThumbWidth*5, mJpegParam.u4ThumbHeight*5,  rImgBufInfo, thumbnailMem);
    mThumbnailMem.push_back(thumbnailMem);
    FUNCTION_LOG_END;

    return rImgBufInfo; 
}


/*******************************************************************************
* 
********************************************************************************/
MVOID     
MultiShot::getPictureDimension(MUINT32 & u4Width,  MUINT32 & u4Height)
{   
    u4Width =  mShotParam.u4PictureWidth; 
    u4Height = mShotParam.u4PictureHeight; 
    if (90 == mShotParam.u4PictureRotation || 270 == mShotParam.u4PictureRotation) 
    {
        u4Width = mShotParam.u4PictureHeight; 
        u4Height = mShotParam.u4PictureWidth; 
    }
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32   // [CC] 
MultiShot::mapScenarioType(EImageFormat const eFmt)
{
    switch (eFmt)
    {
        case eImgFmt_VYUY:
        case eImgFmt_YVYU:
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
            return eScenarioFmt_YUV; 
        break; 
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER12:
        default:
            return eScenarioFmt_RAW; 
        break; 
    }


}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

