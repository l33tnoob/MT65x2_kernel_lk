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

#define LOG_TAG "MtkCam/Shot"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
//
#include <mtkcam/common/camutils/CamFormat.h>
#include <mtkcam/v1/camutils/CamInfo.h>
//
#include <mtkcam/hal/sensor_hal.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
//
#include <Shot/IShot.h>
//
#include <ImageUtils.h>
#include "ImpShot.h"
#include "EngParam.h"
#include "EngShot.h"
//
#include "mtkcam/hal/aaa_hal_base.h" // mediatek\hardware\camera\inc\Featureio
#include "isp_tuning.h"
#include "camera_custom_nvram.h"
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "dbg_isp_param.h"
#include "dbg_aaa_param.h"
#include "flash_param.h"
#include "isp_tuning_mgr.h"
#include "ae_mgr.h" // CCTOPAESetCaptureMode // mediatek\platform\mt6589\hardware\camera\core\featureio\pipe\aaa\Ae_mgr

using namespace android;
using namespace NSShot;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
sp<IShot>
createInstance_EngShot(
    char const*const    pszShotName, 
    uint32_t const      u4ShotMode, 
    int32_t const       i4OpenId
)
{
    CAM_LOGD("createInstance_EngShot");
    sp<IShot>       pShot = NULL;
    sp<EngShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new EngShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new EngShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
EngShot::
onCreate()
{
#warning "[TODO] EngShot::onCreate()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
EngShot::
onDestroy()
{
#warning "[TODO] EngShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
EngShot::
EngShot(
    char const*const pszShotName, 
    uint32_t const u4ShotMode, 
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
{
}


/******************************************************************************
 *
 ******************************************************************************/
EngShot::
~EngShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
sendCommand(
    uint32_t const  cmd, 
    uint32_t const  arg1, 
    uint32_t const  arg2
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset, 
    //  performing a new capture should work well, no matter whether previous 
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    case eCmd_setShotParam:
        ret = setEngParam(reinterpret_cast<void const*>(arg1), arg2);
        ret &= ImpShot::sendCommand(cmd, arg1, sizeof(ShotParam));
        break;
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
onCmd_reset()
{
#warning "[TODO] EngShot::onCmd_reset()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
onCmd_capture()
{ 
    MBOOL ret = MTRUE; 

    NSIspTuning::IspTuningMgr::getInstance().setOperMode(NSIspTuning::EOperMode_EM);
    if (EngParam::ENG_SENSOR_MODE_PREVIEW == mEngParam->mi4EngSensorMode)
    {
        MINT32 err1;
        err1 = NS3A::AeMgr::getInstance().CCTOPAESetCaptureMode(0);
        CAM_LOGD("CCTOPAESetCaptureMode(0): %d", err1);

        MBOOL err2;
        err2 = NSIspTuning::IspTuningMgr::getInstance().setSensorMode(NSIspTuning::ESensorMode_Preview);
        CAM_LOGD("setSensorMode(NSIspTuning::ESensorMode_Preview): %d", err2);
    }
    else if (EngParam::ENG_SENSOR_MODE_CAPTURE == mEngParam->mi4EngSensorMode)
    {
        MINT32 err1;
        err1 = NS3A::AeMgr::getInstance().CCTOPAESetCaptureMode(1);
        CAM_LOGD("CCTOPAESetCaptureMode(3): %d", err1);

        MBOOL err2;
        err2 = NSIspTuning::IspTuningMgr::getInstance().setSensorMode(NSIspTuning::ESensorMode_Capture);
        CAM_LOGD("setSensorMode(NSIspTuning::ESensorMode_Capture): %d", err2);
    }
    else if (EngParam::ENG_SENSOR_MODE_VIDEO_PREVIEW == mEngParam->mi4EngSensorMode)
    {
        MINT32 err1;
        err1 = NS3A::AeMgr::getInstance().CCTOPAESetCaptureMode(2);
        CAM_LOGD("CCTOPAESetCaptureMode(3): %d", err1);

        MBOOL err2;
        err2 = NSIspTuning::IspTuningMgr::getInstance().setSensorMode(NSIspTuning::ESensorMode_Video);
        CAM_LOGD("setSensorMode(NSIspTuning::ESensorMode_Video): %d", err2);
    }
    
    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "EngShot"); 
    // 
    pSingleShot->init(); 

    // 
    pSingleShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_SOF); 
    //
    EImageFormat ePostViewFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat)); 

    MINT32 i4msg = 0;
    if (mEngParam->mi4EngRawSaveEn)
    {
        i4msg |= NSCamShot::ECamShot_DATA_MSG_BAYER;
    }
    pSingleShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_JPEG | i4msg
                               | ((ePostViewFmt != eImgFmt_UNKNOWN) ? NSCamShot::ECamShot_DATA_MSG_POSTVIEW : NSCamShot::ECamShot_DATA_MSG_NONE)
                               ); 

    // shot param 
    NSCamShot::ShotParam rShotParam(eImgFmt_YUY2,         //yuv format 
                         mShotParam.mi4PictureWidth,      //picutre width 
                         mShotParam.mi4PictureHeight,     //picture height
                         mShotParam.mi4Rotation,          //picture rotation 
                         0,                               //picture flip 
                         ePostViewFmt,                    // postview format 
                         mShotParam.mi4PostviewWidth,      //postview width 
                         mShotParam.mi4PostviewHeight,     //postview height 
                         0,                               //postview rotation 
                         0,                               //postview flip 
                         mShotParam.mu4ZoomRatio           //zoom   
                        );                                  
 
    // jpeg param 
    NSCamShot::JpegParam rJpegParam(NSCamShot::ThumbnailParam(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight, 
                                                                mJpegParam.mu4JpegThumbQuality, MTRUE),
                                                        mJpegParam.mu4JpegQuality,       //Quality 
                                                        MFALSE                            //isSOI 
                         ); 
 
                                                                     
    // sensor param 
    MUINT32 u4Scenario;
    if (1 == mEngParam->mi4EngRawSaveEn)
    {
            u4Scenario = mEngParam->mi4EngSensorMode;
    }
    else
    {
#warning [FIXME] workaround for Alta phone capture mode cant work 
            u4Scenario = ACDK_SCENARIO_ID_CAMERA_PREVIEW; // u4Scenario = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
    }
    NSCamShot::SensorParam rSensorParam(static_cast<MUINT32>(MtkCamUtils::DevMetaInfo::queryHalSensorDev(getOpenId())),                             //Device ID 

#warning [FIXME] workaround for Alta phone capture mode cant work 
                             //ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio 
                             u4Scenario,         //Scenaio 
                             10,                                       //bit depth 
                             MFALSE,                                   //bypass delay 
                             MFALSE,                                   //bypass scenario 
                             mEngParam->mi4EngIspMode); 
    //
    mEngParam->u4Bitdepth = rSensorParam.u4Bitdepth;
    NSCamShot::querySensorInfo(rSensorParam.u4DeviceID, rSensorParam.u4Scenario, rSensorParam.u4Bitdepth,
                                mEngParam->eImgFmt,
                                mEngParam->u4SensorWidth,
                                mEngParam->u4SensorHeight,
                                mEngParam->u4RawPixelID /* SENSOR_OUTPUT_FORMAT_RAW_B=0, SENSOR_OUTPUT_FORMAT_RAW_Gb, SENSOR_OUTPUT_FORMAT_RAW_Gr, SENSOR_OUTPUT_FORMAT_RAW_R */ );

    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this); 
    //     
    ret = pSingleShot->setShotParam(rShotParam); 
    
    //
    ret = pSingleShot->setJpegParam(rJpegParam); 

    // 
    ret = pSingleShot->startOne(rSensorParam); 
   
    //
    ret = pSingleShot->uninit(); 

    //
    pSingleShot->destroyInstance(); 


    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
EngShot::
onCmd_cancel()
{
#warning "[TODO] EngShot::onCmd_cancel()"
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL 
EngShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    EngShot *pEngShot = reinterpret_cast <EngShot *>(user); 
    if (NULL != pEngShot) 
    {
        if (NSCamShot::ECamShot_NOTIFY_MSG_SOF == msg.msgType) 
        {
            pEngShot->mpShotCallback->onCB_Shutter(true, 
                                                      0
                                                     ); 
        }
    }

    return MTRUE; 
}




/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EngShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    EngShot *pEngShot = reinterpret_cast<EngShot *>(user); 
    if (NULL != pEngShot) 
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType) 
        {
            pEngShot->handlePostViewData( msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pEngShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_BAYER == msg.msgType)
        {
            pEngShot->mpShotCallback->onCB_RawImage(0, msg.u4Size, reinterpret_cast<uint8_t const*>(msg.puData));
        }
    }

    return MTRUE; 
}

bool
EngShot::
setEngParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to EngParam");
        return  false;
    }
    //
    if  ( size != sizeof(EngParam) )
    {
        MY_LOGE("size[%d] != sizeof(EngParam)[%d]; please fully build source codes", size, sizeof(EngParam));
        return  false;
    }
    //
    mEngParam = const_cast<EngParam *>(reinterpret_cast<EngParam const*>(pParam));

    return  true;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
EngShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGD("[handlePostViewData] + (puBuf, size) = (%p, %d)", puBuf, u4Size); 
    mpShotCallback->onCB_PostviewDisplay(0, 
                                         u4Size, 
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        ); 

    MY_LOGD("[handlePostViewData] -"); 
    return  MTRUE;
    }

/******************************************************************************
*
*******************************************************************************/
MBOOL
EngShot::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize)
{
    MY_LOGD("[handleJpegData] + (puJpgBuf, jpgSize, puThumbBuf, thumbSize) = (%p, %d, %p, %d)", puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize); 

    MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024]; 
    MUINT32 u4ExifHeaderSize = 0; 

    makeExifHeader(eAppMode_EngMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
    MY_LOGD("[handleJpegData] (thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)", 
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
    // Jpeg callback 
    mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize, 
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,                       //header size 
                                         puExifHeaderBuf,                    //header buf
                                         0,                       //callback index 
                                         true                     //final image 
                                         ); 
    MY_LOGD("[handleJpegData] -"); 

    delete [] puExifHeaderBuf; 

    return MTRUE; 

}


