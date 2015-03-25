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
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
//
#include <mtkcam/hal/sensor_hal.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "NormalShot.h"
//
using namespace android;
using namespace NSShot;

#define SHUTTER_TIMING (NSCamShot::ECamShot_NOTIFY_MSG_EOF)
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
createInstance_NormalShot(
    char const*const    pszShotName, 
    uint32_t const      u4ShotMode, 
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<NormalShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new NormalShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new NormalShot", __FUNCTION__);
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
NormalShot::
onCreate()
{
#warning "[TODO] NormalShot::onCreate()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
NormalShot::
onDestroy()
{
#warning "[TODO] NormalShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
NormalShot::
NormalShot(
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
NormalShot::
~NormalShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
NormalShot::
sendCommand(
    uint32_t const  cmd, 
    uint32_t const  arg1, 
    uint32_t const  arg2
)
{
    AutoCPTLog cptlog(Event_Shot_sendCmd, cmd, arg1);
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
NormalShot::
onCmd_reset()
{
#warning "[TODO] NormalShot::onCmd_reset()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
NormalShot::
onCmd_capture()
{ 
    AutoCPTLog cptlog(Event_Shot_capture);
    MBOOL ret = MTRUE; 
    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "NormalShot"); 
    // 
    pSingleShot->init(); 

    // 
    pSingleShot->enableNotifyMsg( SHUTTER_TIMING ); 
    //
    EImageFormat ePostViewFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat)); 

    pSingleShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_JPEG
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
    NSCamShot::SensorParam rSensorParam(static_cast<MUINT32>(MtkCamUtils::DevMetaInfo::queryHalSensorDev(getOpenId())),                             //Device ID 
                             ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio 
                             10,                                       //bit depth 
                             MFALSE,                                   //bypass delay 
                             MFALSE                                   //bypass scenario 
                            );  
    //
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
NormalShot::
onCmd_cancel()
{
    AutoCPTLog cptlog(Event_Shot_cancel);
#warning "[TODO] NormalShot::onCmd_cancel()"
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL 
NormalShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    AutoCPTLog cptlog(Event_Shot_handleNotifyCb);
    NormalShot *pNormalShot = reinterpret_cast <NormalShot *>(user); 
    if (NULL != pNormalShot) 
    {
        if ( SHUTTER_TIMING == msg.msgType) 
        {
            pNormalShot->mpShotCallback->onCB_Shutter(true, 
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
NormalShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    NormalShot *pNormalShot = reinterpret_cast<NormalShot *>(user); 
    if (NULL != pNormalShot) 
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType) 
        {
            pNormalShot->handlePostViewData( msg.puData, msg.u4Size);  
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pNormalShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2);
        }
        }

    return MTRUE; 
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    AutoCPTLog cptlog(Event_Shot_handlePVData);
    MY_LOGD("+ (puBuf, size) = (%p, %d)", puBuf, u4Size); 
    mpShotCallback->onCB_PostviewDisplay(0, 
                                         u4Size, 
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        ); 

    MY_LOGD("-"); 
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShot::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize)
{
    AutoCPTLog cptlog(Event_Shot_handleJpegData);
    MY_LOGD("+ (puJpgBuf, jpgSize, puThumbBuf, thumbSize) = (%p, %d, %p, %d)", puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize); 

    MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024]; 
    MUINT32 u4ExifHeaderSize = 0; 

    CPTLogStr(Event_Shot_handleJpegData, CPTFlagSeparator, "makeExifHeader");
    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
    MY_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)", 
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 

    // dummy raw callback 
    mpShotCallback->onCB_RawImage(0, 0, NULL);   

    // Jpeg callback 
    CPTLogStr(Event_Shot_handleJpegData, CPTFlagSeparator, "onCB_CompressedImage");
    mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize, 
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,                       //header size 
                                         puExifHeaderBuf,                    //header buf
                                         0,                       //callback index 
                                         true                     //final image 
                                         ); 
    MY_LOGD("-"); 

    delete [] puExifHeaderBuf; 

    return MTRUE; 

}


