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
#define LOG_TAG "CamShot/SingleShot"
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
#include <pthread.h>
#include <sys/prctl.h>
//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
// 
#include <mtkcam/v1/config/PriorityDefs.h>
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
#include <mtkcam/campipe/_params.h>
//
#include <mtkcam/drv/res_mgr_drv.h>
#include <mtkcam/campipe/pipe_mgr_drv.h>
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>
//
#include <mtkcam/camshot/ISImager.h> 
#include "../inc/ImageUtils.h"
//
#include "../inc/CamShotImp.h"
#include "../inc/SingleShot.h"
//
//fake orientation
#include <mtkcam/hal/sensor_hal.h>

using namespace android; 
using namespace NSCamPipe; 
using namespace NS3A; 
#define USE_HAL3A 1

class ResMgrDrv; 
class PipeMgrDrv; 

#define MEDIA_PATH  "/sdcard/"

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

#define ANGLE_SUB(x,y)   ( ( (x) + 360 - (y) ) % 360 )

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
SingleShot::
SingleShot(
    EShotMode const eShotMode,
    char const*const szCamShotName
)
    : CamShotImp(eShotMode, szCamShotName)
    , mPreAllocMemThreadHandle(0)
    , mpISImager(NULL) 
    , mpCamIOPipe(NULL) 
    , mpMemDrv(NULL)
    , mu4DiffOri(0)
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
    , mPrePostViewMem()
    , mJpegMem()
    , mThumbnailMem()
    , mpPipeMgrDrv(NULL)
    , mpResMgrDrv(NULL)
    , mu4FlashCountDown(-1)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.camera.dump", value, "0"); 
    mu4DumpFlag = ::atoi(value); 
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
SingleShot::
init()
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_init);
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
SingleShot::
uninit()
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_uninit);
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
SingleShot::
start(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;
    mSensorParam = rSensorParam; 
    //
    dumpSensorParam(mSensorParam); 
#warning [TODO] for continouous shot 
 
    FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SingleShot::
startOne(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_startOneSensor);
    MtkCamUtils::CamProfile profile("[startOne][sensor->mem]", "SingleShot");
    MBOOL ret = MTRUE; 
    mSensorParam = rSensorParam; 
    //fake orientation
    SensorHal* sensor_hal = SensorHal::createInstance();
    sensor_hal->init();

    MUINT32 fake_orientation = 0;
    sensor_hal->sendCommand( static_cast<halSensorDev_e>(rSensorParam.u4DeviceID), 
                             SENSOR_CMD_GET_FAKE_ORIENTATION,
                             (MUINT32)&fake_orientation, 0, 0 );

    if( fake_orientation )
    {
        MUINT32 realOri, facing;

        sensor_hal->sendCommand( static_cast<halSensorDev_e>(rSensorParam.u4DeviceID),
                                 SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE,
                                 (MUINT32)&realOri, 0, 0 );
        sensor_hal->sendCommand( static_cast<halSensorDev_e>(rSensorParam.u4DeviceID),
                                 SENSOR_CMD_GET_SENSOR_FACING_DIRECTION,
                                 (MUINT32)&facing, 0, 0 );
        MY_LOGD("realOri %d, facing %d", realOri, facing );
        if( facing == 1)
        {
            mu4DiffOri = ANGLE_SUB(270, realOri);
        }
        else
        {
            mu4DiffOri = ANGLE_SUB(90, realOri);
        }
    }
    else
    {
        mu4DiffOri = 0;
    }

    sensor_hal->uninit();
    sensor_hal->destroyInstance();

    MY_LOGD("DeviceID(%d), fake_orientation(%d), DiffOri(%d)", \
            rSensorParam.u4DeviceID, fake_orientation, mu4DiffOri);
    //
    dumpSensorParam(mSensorParam); 

    EImageFormat eImgFmt = querySensorFmt(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth); 

    // (1). Create Instance 
    CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "camIOInit");
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
    MY_LOGD("[startOne] enabled msg (notify, data) = (0x%x, 0x%x)", mi4NotifyMsgSet, mi4DataMsgSet); 
    //
    // Start Of Frame notify
//#warning [TODO] this should callback from pipe 
    //handleNotifyCallback(ECamShot_NOTIFY_MSG_SOF, 0, 0); 

    if (!isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL))
    {
        MY_LOGE("[startOne] No data msg enable !"); 
        return MFALSE; 
    }

   
    // (1) create raw image 
    // it always need to dump bayer raw image due to 
    // the capture is 3 pass, 
    // 1st pass: Sensor -> TG --> Memory (Raw(bayer),  YUV(yuy2)) 
    // 2nd pass: memory (bayer/yuy2) --> post proc -> mem (yuv, postview) 
    // 3nd pass: memory (yuv) --> jpeg --> mem (bitstream) 
    CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "querySensorRawImgBufInfo");
    ImgBufInfo rRawImgBufInfo = querySensorRawImgBufInfo(); 
    CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "createSensorRawImg");
    
    if (isDataMsgEnabled(ECamShot_DATA_MSG_JPEG))
    {
        preAllocMem(); 
    }
    ret = ret 
           && createSensorRawImg(mSensorParam, rRawImgBufInfo)
           && handleDataCallback(ECamShot_DATA_MSG_BAYER, 0, 0, reinterpret_cast<MUINT8*>(rRawImgBufInfo.u4BufVA), rRawImgBufInfo.u4BufSize); 

    //handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0); 

    // post process 
    if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_POSTVIEW|ECamShot_DATA_MSG_JPEG))
    {
        CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "startOneMem");   
        waitPreAllocMemDone(); 
        startOne(rRawImgBufInfo);
    }
    // 
    CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "camIOUninit");   
    CHECK_OBJECT(mpCamIOPipe)
    ret = mpCamIOPipe->uninit(); 
    if (!ret)
    {
        MY_LOGE("mpCamIOPipe->uninit() fail ");
    }
    mpCamIOPipe->destroyInstance();
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
SingleShot::
startOne(ImgBufInfo const & rImgBufInfo)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_startOneMem);
    MtkCamUtils::CamProfile profile("[startOne][mem->Mem]", "SingleShot");
    MBOOL ret = MTRUE; 

    Rect rSrcRect(0, 0, rImgBufInfo.u4ImgWidth, rImgBufInfo.u4ImgHeight); 
    Rect rDstRect(0, 0, mShotParam.u4PictureWidth, mShotParam.u4PictureHeight); 

    //fake orientation:
    //  If sensor is set in a wrong orientation, another run is needed to rotate
    //  Quickview.
    if( mu4DiffOri == 90 || mu4DiffOri == 270 )
    {
        rDstRect.w = mShotParam.u4PictureHeight;
        rDstRect.h = mShotParam.u4PictureWidth;
    }

    // calc the zoom crop ratio 
    Rect rRect = MtkCamUtils::calCrop(rSrcRect, rDstRect, mShotParam.u4ZoomRatio); 

    // (2) create yuv image 
    // the postview will be ouput in the 2nd pass 
    // and the yuv image is created in 2nd pass 
    if (isDataMsgEnabled(ECamShot_DATA_MSG_JPEG)) 
    {
        CPTLogStr(Event_SShot_startOneMem, CPTFlagSeparator, "queryYuvRawImgBufInfo");
        ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
        CPTLogStr(Event_SShot_startOneMem, CPTFlagSeparator, "queryPostViewImgInfo");   
        ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
        CPTLogStr(Event_SShot_startOneMem, CPTFlagSeparator, "queryJpegImgBufInfo");  
        ImgBufInfo rJpegImgBufInfo = queryJpegImgBufInfo(); 
        CPTLogStr(Event_SShot_startOneMem, CPTFlagSeparator, "queryThumbImgBufInfo");   
        ImgBufInfo rThumbImgBufInfo = queryThumbImgBufInfo(); 

        MUINT32 u4JpegSize = 0; 
        MUINT32 u4ThumbnailSize = 0; 
        CPTLogStr(Event_SShot_startOneMem, CPTFlagSeparator, "createImg");   
        if( mu4DiffOri )
        {
            //fake orientation: need another run for postview
            ImgBufInfo rPrePostViewBufInfo = queryPrePostViewImgInfo(); 
            ret = ret 
                   && createYuvRawImg(rImgBufInfo, rRect, ANGLE_SUB(mShotParam.u4PictureRotation, mu4DiffOri) , mShotParam.u4PictureFlip, rYuvImgBufInfo, rPrePostViewBufInfo) 
                   && rotatePostview( rPrePostViewBufInfo, ANGLE_SUB(360, mu4DiffOri), 0, rPostViewBufInfo );
        }
        else
        {
            ret = ret 
                   && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, rYuvImgBufInfo, rPostViewBufInfo);
        } 
        ret = ret
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

        if( mu4DiffOri )
        {
            ImgBufInfo rPrePostViewBufInfo = queryPrePostViewImgInfo(); 
            ret = ret 
                   && createYuvRawImg(rImgBufInfo, rRect, ANGLE_SUB(mShotParam.u4PictureRotation, mu4DiffOri), mShotParam.u4PictureFlip, rYuvImgBufInfo, rPrePostViewBufInfo) 
                   && rotatePostview( rPrePostViewBufInfo, ANGLE_SUB(360, mu4DiffOri), 0, rPostViewBufInfo );
        }
        else
        {
            ret = ret 
                   && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PictureRotation, mShotParam.u4PictureFlip, rYuvImgBufInfo, rPostViewBufInfo); 
        }
        ret = ret
               && handleDataCallback(ECamShot_DATA_MSG_POSTVIEW, 0, 0, reinterpret_cast<MUINT8*>(rPostViewBufInfo.u4BufVA), rPostViewBufInfo.u4BufSize)
               && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(rYuvImgBufInfo.u4BufVA), rYuvImgBufInfo.u4BufSize);       
    }
    else if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV))
    { 
        ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
        ret = ret 
               && createYuvRawImg(rImgBufInfo, rRect, ANGLE_SUB(mShotParam.u4PictureRotation, mu4DiffOri), mShotParam.u4PictureFlip, rYuvImgBufInfo) 
               && handleDataCallback(ECamShot_DATA_MSG_YUV, 0 , 0 , reinterpret_cast<MUINT8*>(rYuvImgBufInfo.u4BufVA), rYuvImgBufInfo.u4BufSize);            
    }
    else if (isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW)) 
    {
        //! should not enter this case 
        ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
        if( mu4DiffOri )
        {
            ImgBufInfo rPrePostViewBufInfo = queryPrePostViewImgInfo(); 
            ret = ret 
                   && createYuvRawImg(rImgBufInfo, rRect, ANGLE_SUB(mShotParam.u4PostViewRotation, mu4DiffOri), mShotParam.u4PostViewFlip, rPrePostViewBufInfo) 
                   && rotatePostview( rPrePostViewBufInfo, ANGLE_SUB(360, mu4DiffOri), 0, rPostViewBufInfo );
        }
        else
        {
            ret = ret 
                   && createYuvRawImg(rImgBufInfo, rRect, mShotParam.u4PostViewRotation, mShotParam.u4PostViewFlip, rPostViewBufInfo);
        }
        ret = ret
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
SingleShot::
startOne(SensorParam const & rSensorParam, ImgBufInfo const & rImgBufInfo)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_startOneSensor);
    MtkCamUtils::CamProfile profile("[startOne][sensor->mem]", "SingleShot");
    MBOOL ret = MTRUE;
    mSensorParam = rSensorParam;
    //fake orientation
    SensorHal* sensor_hal = SensorHal::createInstance();
    sensor_hal->init();

    MUINT32 fake_orientation = 0;
    sensor_hal->sendCommand( static_cast<halSensorDev_e>(rSensorParam.u4DeviceID),
                             SENSOR_CMD_GET_FAKE_ORIENTATION,
                             (MUINT32)&fake_orientation, 0, 0 );

    if( fake_orientation )
    {
        MUINT32 realOri, facing;

        sensor_hal->sendCommand( static_cast<halSensorDev_e>(rSensorParam.u4DeviceID),
                                 SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE,
                                 (MUINT32)&realOri, 0, 0 );
        sensor_hal->sendCommand( static_cast<halSensorDev_e>(rSensorParam.u4DeviceID),
                                 SENSOR_CMD_GET_SENSOR_FACING_DIRECTION,
                                 (MUINT32)&facing, 0, 0 );
        MY_LOGD("realOri %d, facing %d", realOri, facing );
        if( facing == 1)
        {
            mu4DiffOri = ANGLE_SUB(270, realOri);
        }
        else
        {
            mu4DiffOri = ANGLE_SUB(90, realOri);
        }
    }
    else
    {
        mu4DiffOri = 0;
    }

    sensor_hal->uninit();
    sensor_hal->destroyInstance();

    MY_LOGD("DeviceID(%d), fake_orientation(%d), DiffOri(%d)", \
            rSensorParam.u4DeviceID, fake_orientation, mu4DiffOri);
    //
    dumpSensorParam(mSensorParam);
    EImageFormat eImgFmt = querySensorFmt(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth);

    // workaroud: without init CameraIO, postproc will KE when start
    // (1). Create Instance
    CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "camIOInit");
    #if 1
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
    //
    MY_LOGD("[startOne] enabled msg (notify, data) = (0x%x, 0x%x)", mi4NotifyMsgSet, mi4DataMsgSet);
    //
    // Start Of Frame notify
#warning [TODO] this should callback from pipe
    handleNotifyCallback(ECamShot_NOTIFY_MSG_SOF, 0, 0);

    if (!isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && !isNotifyMsgEnabled(ECamShot_NOTIFY_MSG_ALL))
    {
        MY_LOGE("[startOne] No data msg enable !");
        return MFALSE;
    }

    ret = ret
           && handleDataCallback(ECamShot_DATA_MSG_BAYER, 0, 0, reinterpret_cast<MUINT8*>(rImgBufInfo.u4BufVA), rImgBufInfo.u4BufSize);

    handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);

#if USE_HAL3A
    NS3A::Hal3ABase *p3AObj = Hal3ABase::createInstance(rSensorParam.u4DeviceID);
    //
    ret = ret
            && p3AObj->setIspProfile(EIspProfile_NormalCapture);
//            && p3AObj->sendCommand(ECmd_ZsdCaptureStart, 0);
#endif

    // post process
    if (isDataMsgEnabled(ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_POSTVIEW|ECamShot_DATA_MSG_JPEG))
    {
        CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "startOneMem");
        //startOne(rRawImgBufInfo);
        startOne(rImgBufInfo);
    }

#if USE_HAL3A
    //p3AObj->sendCommand(ECmd_ZsdCaptureEnd, 0);
    p3AObj->destroyInstance();
#endif
    CPTLogStr(Event_SShot_startOneSensor, CPTFlagSeparator, "camIOUninit");
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
SingleShot::
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
SingleShot::
stop()
{
    FUNCTION_LOG_START;
#warning [TODO] for continouous shot     

    FUNCTION_LOG_END;
    //
    return MTRUE;
}



/*******************************************************************************
* 
********************************************************************************/
MBOOL
SingleShot::
setShotParam(ShotParam const & rParam)
{
    //FUNCTION_LOG_START;
    mShotParam = rParam; 
    //
    dumpShotParam(mShotParam); 

    //FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SingleShot::
setJpegParam(JpegParam const & rParam)
{
    //FUNCTION_LOG_START;
    mJpegParam = rParam; 
    //
    dumpJpegParam(mJpegParam); 
    
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
SingleShot::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
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
SingleShot::
createSensorRawImg(SensorParam const & rSensorParam, ImgBufInfo const & rRawImgBufInfo)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_createSensorRawImg);
    MBOOL ret = MTRUE; 
    if (!lock(RES_MGR_DRV_SCEN_HW_ZSD, PIPE_MGR_DRV_PIPE_MASK_CAM_IO, 3000))
    {
        MY_LOGE("[createSensorRawImg] lock resource fail"); 
        return MFALSE; 
    }   
    MtkCamUtils::CamProfile profile("createSensorRawImg", "SingleShot");

    EImageFormat eImgFmt = querySensorFmt(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth); 

    // (4). setCallback
    mpCamIOPipe->setCallbacks(fgPipeNotifyCb, NULL, this); 
    mpCamIOPipe->enableNotifyMsg(ECamPipe_NOTIFY_MSG_SOF  |
                                 ECamPipe_NOTIFY_MSG_EOF  |
                                 ECamPipe_NOTIFY_MSG_DROPFRAME);
    //
#if USE_HAL3A
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "3A Create");
    NS3A::Hal3ABase *p3AObj = Hal3ABase::createInstance(rSensorParam.u4DeviceID);           
    mu4FlashCountDown = p3AObj->getFlashFrameNumBeforeCapFrame();
#endif

    // (5). Config pipe 
    //
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
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "configPipe");
    mpCamIOPipe->configPipe(vInPorts, vOutPorts);  

    // (6). Enqueue, raw buf
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "enqueBuf");
    QBufInfo rRawBuf; 
    BufInfo rBufInfo(rRawImgBufInfo.u4BufSize, rRawImgBufInfo.u4BufVA, rRawImgBufInfo.u4BufPA, rRawImgBufInfo.i4MemID);  
    rRawBuf.vBufInfo.push_back(rBufInfo); 
    mpCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rRawBuf); 
    profile.print(); 
    //
#if USE_HAL3A
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "SetIspProfCap");
    ret = ret && p3AObj->setIspProfile(EIspProfile_NormalCapture);
	CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "3A CapStart");
    ret = ret && p3AObj->sendCommand(ECmd_CaptureStart, 0);  
#endif 

    // (7). start
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "camIOStart");
    mpCamIOPipe->start(); 

    // (8). dequeue 
    QTimeStampBufInfo rQRawOutBuf;         
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "dequeBuf");
    mpCamIOPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQRawOutBuf); 
    // 
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
    unlock(PIPE_MGR_DRV_PIPE_MASK_CAM_IO); 
    //
#if USE_HAL3A
    CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "3A CapEnd");
    p3AObj->sendCommand(ECmd_CaptureEnd, 0); 
	CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "destroy 3A");
    p3AObj->destroyInstance(); 
	CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "afterDst3A");
#endif 

    FUNCTION_LOG_END;
    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL 
SingleShot::
createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 const u4Img1Rot, MUINT32 const u4Img1Flip, ImgBufInfo const & rDstImgBufInfo1, ImgBufInfo const &rDstImgBufInfo2 )
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_createYuvRawImg);
    if (!lock(RES_MGR_DRV_SCEN_HW_IP, PIPE_MGR_DRV_PIPE_MASK_POST_PROC, 3000))
    {
        MY_LOGE("[createYuvRawImg] lock resource fail"); 
        return MFALSE; 
    }

    MtkCamUtils::CamProfile profile("createYuvRawImg", "SingleShot");
    // (1). Create Instance 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "init");
    IPostProcPipe    *pPostProcPipe = IPostProcPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, static_cast<EScenarioFmt>(mapScenarioType(rSrcImgBufInfo.eImgFmt))); 
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
    MY_LOGD("[createYuvRawImg] enable postview "); 
    MemoryOutPortInfo rDispPort(ImgInfo(rDstImgBufInfo2.eImgFmt, rDstImgBufInfo2.u4ImgWidth, rDstImgBufInfo2.u4ImgHeight), 
                                   rDstImgBufInfo2.u4Stride, 0, 0); 
    vOutPorts.push_back(&rDispPort); 
    //
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "configPipe");
    pPostProcPipe->configPipe(vInPorts, vOutPorts); 
    // (6). Enqueue, In buf
    // 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "enqueBuf");
    QBufInfo rInQBuf; 
    BufInfo rInBufInfo(rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.i4MemID);  
    rInQBuf.vBufInfo.push_back(rInBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryIn, 0, 0), rInQBuf); 

    // (6.1) Enqueue, postview out buf 
    QBufInfo rDispQBuf; 
    BufInfo rDispBufInfo(rDstImgBufInfo2.u4BufSize, rDstImgBufInfo2.u4BufVA, rDstImgBufInfo2.u4BufPA, rDstImgBufInfo2.i4MemID);  
    rDispQBuf.vBufInfo.push_back(rDispBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rDispQBuf); 

    // (6.2) Enqueue, Yuv out Buf
    QBufInfo rVdoQBuf; 
    BufInfo rVdoBufInfo(rDstImgBufInfo1.u4BufSize, rDstImgBufInfo1.u4BufVA, rDstImgBufInfo1.u4BufPA, rDstImgBufInfo1.i4MemID); 
    rVdoQBuf.vBufInfo.push_back(rVdoBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rVdoQBuf); 

    profile.print(); 
    // (7). start
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "start");
    pPostProcPipe->start(); 

    // (8). YUV Dequeue
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "dequeBuf");
    QTimeStampBufInfo rQVdoOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQVdoOutBuf); 

    // (8.1) postview Dequeue 
    QTimeStampBufInfo rQDispOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQDispOutBuf); 
    // (8.2) In buffer dequeue 
    QTimeStampBufInfo rQInBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryIn, 0, 0), rQInBuf); 

    // (9). Stop 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "stop");
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
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "uninit");
    pPostProcPipe->uninit(); 
    // (11). destory instance 
    pPostProcPipe->destroyInstance(); 

    profile.print(""); 
    unlock(PIPE_MGR_DRV_PIPE_MASK_POST_PROC); 
    FUNCTION_LOG_END;

    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL 
SingleShot::
createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 u4Img1Rot, MUINT32 u4Img1Flip, 
                ImgBufInfo const & rDstImgBufInfo)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_createYuvRawImg);
    if (!lock(RES_MGR_DRV_SCEN_HW_IP, PIPE_MGR_DRV_PIPE_MASK_POST_PROC, 3000))
    {
        MY_LOGE("[createYuvRawImg] lock resource fail"); 
        return MFALSE; 
    }
    // (1). Create Instance 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "init");
    IPostProcPipe *pPostProcPipe = IPostProcPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL,  static_cast<EScenarioFmt>(mapScenarioType(rSrcImgBufInfo.eImgFmt))); 
    CHECK_OBJECT(pPostProcPipe); 
   
    // (2). Query port property
    // (3). init 

    pPostProcPipe->init(); 
    // (4). setCallback
    pPostProcPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    // 
    MemoryInPortInfo rMemInPort(ImgInfo(rSrcImgBufInfo.eImgFmt, 
                                        rSrcImgBufInfo.u4ImgWidth, 
                                        rSrcImgBufInfo.u4ImgHeight), 
                                0, 
                                rSrcImgBufInfo.u4Stride, 
                                Rect(rSrcCropRect.x, 
                                     rSrcCropRect.y, 
                                     rSrcCropRect.w, 
                                     rSrcCropRect.h)
                               ); 
    //
    MemoryOutPortInfo rVdoPort(ImgInfo(rDstImgBufInfo.eImgFmt, 
                                       rDstImgBufInfo.u4ImgWidth,
                                       rDstImgBufInfo.u4ImgHeight), 
                               rDstImgBufInfo.u4Stride,
                               u4Img1Rot,
                               u4Img1Flip);   
    rVdoPort.index = 1;   
    //
    vector<PortInfo const*> vInPorts;  
    vector<PortInfo const*> vOutPorts; 
    //
    vInPorts.push_back(&rMemInPort); 
    vOutPorts.push_back(&rVdoPort); 
    //
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "configPipe");
    pPostProcPipe->configPipe(vInPorts, vOutPorts); 
    // (6). Enqueue, In buf
    // 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "enqueBuf");
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
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "start");
    pPostProcPipe->start(); 
    // (8). YUV Dequeue
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "dequeBuf");
    QTimeStampBufInfo rQVdoOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQVdoOutBuf); 
    // (8.1) In buffer dequeue 
    QTimeStampBufInfo rQInBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryIn, 0, 0), rQInBuf); 

    // (9). Stop 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "stop");
    pPostProcPipe->stop();    

    if (mu4DumpFlag) 
    {
        char fileName[256] = {'\0'}; 

        sprintf(fileName, "/%s/shot_yuv%dx%d.yuv", MEDIA_PATH, rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight); 
        MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rDstImgBufInfo.u4BufVA), rDstImgBufInfo.u4BufSize);         
   }
    // (10). uninit 
    CPTLogStr(Event_SShot_createYuvRawImg, CPTFlagSeparator, "uninit");
    pPostProcPipe->uninit(); 
    // (11). destory instance 
    pPostProcPipe->destroyInstance(); 

    unlock(PIPE_MGR_DRV_PIPE_MASK_POST_PROC); 
    FUNCTION_LOG_END;

    return MTRUE; 
}


MBOOL
SingleShot::
rotatePostview(ImgBufInfo const & rSrcImgBufInfo, MINT32 const u4Rot, MINT32 const u4Flip, ImgBufInfo const & rDstImgBufInfo )
{
    FUNCTION_LOG_START;
    //AutoCPTLog cptlog(Event_SShot_createJpegImg);
    //MtkCamUtils::CamProfile profile("createJpegImg", "SingleShot");
    //
    // (1). Create Instance 
    //CPTLogStr(Event_SShot_createJpegImg, CPTFlagSeparator, "init");
    ISImager *pISImager = ISImager::createInstance(rSrcImgBufInfo); 
    CHECK_OBJECT(pISImager); 

    // init setting     
    BufInfo rBufInfo(rDstImgBufInfo.u4BufSize, rDstImgBufInfo.u4BufVA, rDstImgBufInfo.u4BufPA, rDstImgBufInfo.i4MemID); 
    //
    pISImager->setTargetBufInfo(rBufInfo); 
    //
    pISImager->setFormat( rSrcImgBufInfo.eImgFmt); 
    //
    pISImager->setRotation(u4Rot); 
    //
    pISImager->setFlip(u4Flip); 
    // 
    if( u4Rot == 90 || u4Rot == 270 )
    {
        pISImager->setResize(rDstImgBufInfo.u4ImgHeight, rDstImgBufInfo.u4ImgWidth); 
    }
    else
    {
        pISImager->setResize(rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight); 
    }
    //
    //pISImager->setEncodeParam(rDstParm.fgIsSOI, rDstParm.u4Quality); 
    //
    pISImager->setROI(Rect(0, 0, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight)); 
    //
    //CPTLogStr(Event_SShot_createJpegImg, CPTFlagSeparator, "execute");
    pISImager->execute(); 
    //
    //u4JpegSize = pISImager->getJpegSize(); 

    pISImager->destroyInstance(); 

    //profile.print();
    FUNCTION_LOG_END;
    return MTRUE; 

}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SingleShot::
createJpegImg(ImgBufInfo const & rSrcImgBufInfo, JpegParam const & rJpgParm, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rJpgImgBufInfo, MUINT32 & u4JpegSize)
{
    FUNCTION_LOG_START;
    AutoCPTLog cptlog(Event_SShot_createJpegImg);
    MtkCamUtils::CamProfile profile("createJpegImg", "SingleShot");
    //
    // (1). Create Instance 
    CPTLogStr(Event_SShot_createJpegImg, CPTFlagSeparator, "init");
    ISImager *pISImager = ISImager::createInstance(rSrcImgBufInfo); 
    CHECK_OBJECT(pISImager); 

    // init setting     
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
    CPTLogStr(Event_SShot_createJpegImg, CPTFlagSeparator, "execute");
    pISImager->execute(); 
    //
    u4JpegSize = pISImager->getJpegSize(); 

    pISImager->destroyInstance(); 

    profile.print();
    FUNCTION_LOG_END;
    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL    
SingleShot::
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
SingleShot::
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



/*******************************************************************************
* 
********************************************************************************/
MBOOL   
SingleShot::
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
SingleShot::
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
SingleShot::
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
SingleShot::
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
SingleShot::
allocImgMem(char const* const pszName, EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, IMEM_BUF_INFO & rMem)
{
    //
    MtkCamUtils::CamProfile profile("allocImgMem", "SingleShot");
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
inline MVOID     
SingleShot::
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
inline    MVOID     
SingleShot::
freeShotMem()
{
    MtkCamUtils::CamProfile profile("freeShotMem", "SingleShot");
    // Raw 
    if (0 != mRawMem.size) 
    {
        deallocMem(mRawMem); 
    }
    // Yuv 
    if (0 != mYuvMem.size) 
    {
        deallocMem(mYuvMem); 
    }
    // Postview 
    if (0 != mPostViewMem.size)
    {
        deallocMem(mPostViewMem); 
    } 
    // PrePostview 
    if (0 != mPrePostViewMem.size)
    {
        deallocMem(mPrePostViewMem); 
    } 
    // Jpeg 
    if (0 != mJpegMem.size)
    {
        deallocMem(mJpegMem); 
    }
    // Thumbnail 
    if (0 != mThumbnailMem.size)
    {
        deallocMem(mThumbnailMem); 
    }
    profile.print(); 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
SingleShot::
querySensorRawImgBufInfo()
{
    // is upper layer register buffer 
    if (0 != mRawImgBufInfo.u4BufSize) 
    {
        return mRawImgBufInfo; 
    }

    // Raw Buffer 
    MUINT32 u4SensorWidth = 0, u4SensorHeight = 0; 
    EImageFormat eImgFmt = querySensorFmt(mSensorParam.u4DeviceID, mSensorParam.u4Scenario, mSensorParam.u4Bitdepth);        
    querySensorResolution(mSensorParam.u4DeviceID, mSensorParam.u4Scenario, u4SensorWidth, u4SensorHeight); 

    //MY_LOGD("[querySensorRawImgBufInfo] Sensor (fmt, width, height) = (0x%x, %d, %d)", eImgFmt, u4SensorWidth, u4SensorHeight); 
    // 
    allocImgMem("SensorRaw", eImgFmt, u4SensorWidth, u4SensorHeight, mRawMem); 
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt, u4SensorWidth, u4SensorHeight, rImgBufInfo, mRawMem); 
    return rImgBufInfo;    
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
SingleShot::
queryYuvRawImgBufInfo()
{
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
    allocImgMem("Yuv", eImgFmt, u4Width, u4Height, mYuvMem); 
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt, u4Width, u4Height,  rImgBufInfo, mYuvMem);         

    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
SingleShot::
queryJpegImgBufInfo()
{
    // is upper layer register buffer 
    if (0 != mJpegImgBufInfo.u4BufSize) 
    {
        return mJpegImgBufInfo; 
    }
    
    // the Raw Mem is allocated in singleShot, re-use raw mem
    MUINT32 u4Width = 0, u4Height = 0; 
    getPictureDimension(u4Width, u4Height);

    ImgBufInfo rImgBufInfo; 
    if (0 != mRawMem.size)
    {
        setImageBuf(eImgFmt_JPEG, u4Width, u4Height, rImgBufInfo, mRawMem); 
        // optimize the jpeg buffer size to increase the performance 
        MUINT32 u4JpegSize = queryImgBufSize(eImgFmt_JPEG, u4Width, u4Height); 
        rImgBufInfo.u4BufSize = (rImgBufInfo.u4BufSize > u4JpegSize)  ? (u4JpegSize) : (rImgBufInfo.u4BufSize);        
    }
    else 
    {
        allocImgMem("Jpeg", eImgFmt_JPEG, u4Width, u4Height, mJpegMem); 
        setImageBuf(eImgFmt_JPEG, u4Width, u4Height,  rImgBufInfo, mJpegMem);          
    }
    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
SingleShot::
queryPostViewImgInfo()
{
    // is upper layer register buffer 
    if (0 != mPostViewImgBufInfo.u4BufSize)
    {
        return mPostViewImgBufInfo; 
    }
    // no postview format, use YUY2 as default for jpeg encode
    if (eImgFmt_UNKNOWN == mShotParam.ePostViewFmt) 
    {
        mShotParam.ePostViewFmt = eImgFmt_YUY2; 
    }

    allocImgMem("PostView", mShotParam.ePostViewFmt, mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight, mPostViewMem); 
    ImgBufInfo rImgBufInfo; 
    //
    setImageBuf(mShotParam.ePostViewFmt, mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight,  rImgBufInfo, mPostViewMem); 
    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
SingleShot::
queryPrePostViewImgInfo()
{
    // never be registered by upper layer

    // no postview format, use YUY2 as default for jpeg encode
    if (eImgFmt_UNKNOWN == mShotParam.ePostViewFmt) 
    {
        mShotParam.ePostViewFmt = eImgFmt_YUY2; 
    }

    MUINT32 u4Width = 0, u4Height = 0; 
    getPrePostviewDimension(u4Width, u4Height);
    //
    allocImgMem("PrePostView", mShotParam.ePostViewFmt, u4Width, u4Height, mPrePostViewMem); 
    ImgBufInfo rImgBufInfo; 
    //
    setImageBuf(mShotParam.ePostViewFmt, u4Width, u4Height, rImgBufInfo, mPrePostViewMem); 
    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
ImgBufInfo 
SingleShot::
queryThumbImgBufInfo()
{
    // is upper layer register buffer 
    if (0 != mThumbImgBufInfo.u4BufSize) 
    {
        return mThumbImgBufInfo; 
    }
    //
    if (mThumbnailMem.size == 0)
    {
        mThumbnailMem.size = 128 * 1024; 
        MY_LOGD("allocate thumbnail mem, size = %d", mThumbnailMem.size); 
        allocMem(mThumbnailMem); 
    }
    ImgBufInfo rImgBufInfo; 
    setImageBuf(eImgFmt_JPEG, mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight, rImgBufInfo, mThumbnailMem); 
    return rImgBufInfo; 
}

/*******************************************************************************
* 
********************************************************************************/
inline    MVOID     
SingleShot::getPictureDimension(MUINT32 & u4Width,  MUINT32 & u4Height)
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
inline    MVOID     
SingleShot::getPrePostviewDimension(MUINT32 & u4Width,  MUINT32 & u4Height)
{   
    u4Width =  mShotParam.u4PostViewWidth;
    u4Height = mShotParam.u4PostViewHeight;
    if (90 == mu4DiffOri || 270 == mu4DiffOri ) 
    {
        u4Height = mShotParam.u4PostViewWidth;  
        u4Width  = mShotParam.u4PostViewHeight; 
    }
    MY_LOGD("prepost: %d, %d", u4Width, u4Height);
}
/*******************************************************************************
* 
********************************************************************************/
inline    MUINT32    
SingleShot::mapScenarioType(EImageFormat const eFmt)
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


/*******************************************************************************
*
********************************************************************************/
void* 
SingleShot::
_preAllocMemThread(void* arg)
{
    MY_LOGD(" + tid(%d)", ::gettid());
    ::prctl(PR_SET_NAME,"SingleShot@AllocMem",0,0,0);
    //

    //
    SingleShot*const pSingleShot = reinterpret_cast<SingleShot*>(arg);
    if  ( ! pSingleShot )
    {
        MY_LOGE("NULL arg");
        return  NULL;
    }

    pSingleShot->_preAllocMem(); 
    MY_LOGD(" - tid(%d)", ::gettid());
    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
preAllocMem()
{
    FUNCTION_LOG_START;
    //
    pthread_attr_t const attr = {0, NULL, 1024*1024, 4096, SCHED_RR, PRIO_RT_CAMERA_CAPTURE - 1};

#if MTKCAM_HAVE_RR_PRIORITY
    MINT32 err = ::pthread_create(&mPreAllocMemThreadHandle, &attr, _preAllocMemThread, this);
#else 
    MINT32 err = ::pthread_create(&mPreAllocMemThreadHandle, NULL, _preAllocMemThread, this); 
#endif 
    if  (err)
    {
        MY_LOGE("pthread_create return err(%d)", err);
    }
    //
    FUNCTION_LOG_END;
    return (0 == err); 
}

/*******************************************************************************
*
********************************************************************************/
MBOOL    
SingleShot::
waitPreAllocMemDone()
{
    FUNCTION_LOG_START;
    if (0 != mPreAllocMemThreadHandle) 
    {
       pthread_join(mPreAllocMemThreadHandle, NULL); 
       mPreAllocMemThreadHandle = 0; 

}
    FUNCTION_LOG_END;
    return MTRUE; 
}


/*******************************************************************************
*
********************************************************************************/
MBOOL    
SingleShot::
_preAllocMem()
{
    FUNCTION_LOG_START;
    if (isDataMsgEnabled(ECamShot_DATA_MSG_JPEG)) 
    {
        ImgBufInfo rYuvImgBufInfo = queryYuvRawImgBufInfo(); 
        if( mu4DiffOri )
        {
            //fake orientation: another buf for rotation
            ImgBufInfo rPrePostViewBufInfo = queryPrePostViewImgInfo();
        }
        ImgBufInfo rPostViewBufInfo = queryPostViewImgInfo(); 
        ImgBufInfo rJpegImgBufInfo = queryJpegImgBufInfo(); 
        ImgBufInfo rThumbImgBufInfo = queryThumbImgBufInfo(); 
    }

    FUNCTION_LOG_END;
    return MTRUE; 
}

MBOOL
SingleShot::
fgPipeNotifyCb(MVOID* user, NSCamPipe::PipeNotifyInfo const msg)
{
    SingleShot *pSingleShot = reinterpret_cast <SingleShot *>(user); 
    if (NULL != pSingleShot) 
    {
        switch( msg.msgType )
        {
            case NSCamPipe::ECamPipe_NOTIFY_MSG_SOF:
                pSingleShot->handleNotifyCallback(ECamShot_NOTIFY_MSG_SOF, 0, 0); 
                break;
            case NSCamPipe::ECamPipe_NOTIFY_MSG_EOF:
                pSingleShot->handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0); 
                break;
            case NSCamPipe::ECamPipe_NOTIFY_MSG_DROPFRAME:
                MY_LOGD("dropframe %d", msg.ext1);
                pSingleShot->checkIfFireFlash(msg.ext1); 
                break;
            default:
                break;
        }
    }

    return MTRUE; 
}

MBOOL
SingleShot::
checkIfFireFlash(MUINT32 countdown)
{
    if( mu4FlashCountDown > 0 &&  //not fire yet
        ( mu4FlashCountDown == (countdown+1)  || //count down
          mu4FlashCountDown > countdown )        //fire immediately
        )
    {
        MY_LOGD("fire flash");
        NS3A::Hal3ABase *p3AObj = Hal3ABase::createInstance(mSensorParam.u4DeviceID);           
        p3AObj->onFireCapFlashIfNeeded();
        p3AObj->destroyInstance();
        mu4FlashCountDown = -1;
    }
    return MTRUE;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

