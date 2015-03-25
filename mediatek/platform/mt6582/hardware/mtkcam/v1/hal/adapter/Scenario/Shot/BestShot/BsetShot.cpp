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
#include <fcntl.h>
#include <sys/stat.h>
//
#include <cutils/properties.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
//
#include <mtkcam/common/camutils/CamFormat.h>
#include <mtkcam/v1/camutils/CamInfo.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
//
#include <mtkcam/hal/sensor_hal.h>
//
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "BestShot.h"
//
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
createInstance_BestShot(
    char const*const    pszShotName, 
    uint32_t const      u4ShotMode, 
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<BestShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new BestShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new Best Shot", __FUNCTION__);
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
BestShot::
onCreate()
{
#warning "[TODO] BestShot::onCreate()"
    bool ret = true;

    mu4CurrentCount = 0;
    mu4TotalCount = 0; 
    mu4BestShotValue = 0;      
    mu4BestShotIndex = 0;  

    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
BestShot::
onDestroy()
{
#warning "[TODO] BestShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
BestShot::
BestShot(
    char const*const pszShotName, 
    uint32_t const u4ShotMode, 
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mvTmpBuf_postview()
    , mvTmpBuf_jpeg()
    , mvTmpBuf_exif()
    , mu4CurrentCount(0)
    , mu4TotalCount(0)
    , mu4BestShotValue(0)
    , mu4BestShotIndex(0)
    , mfgIsCanceled(MFALSE)
{
}


/******************************************************************************
 *
 ******************************************************************************/
BestShot::
~BestShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
BestShot::
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
BestShot::
onCmd_reset()
{
#warning "[TODO] BestShot::onCmd_reset()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
BestShot::
onCmd_capture()
{ 
    MBOOL ret = MTRUE; 
    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "BestShot"); 
    // 
    pSingleShot->init(); 

    // 
    pSingleShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_SOF); 
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
                         ePostViewFmt,                    //postview format 
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
 
    // 
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
    mu4TotalCount = BEST_SHOT_COUNT; 
    for (MUINT32 i = 0; i < mu4TotalCount; i++) 
    {
        // increase the shot speed, bypass to set the sensor again 
        if (0 != i) 
        {
            rSensorParam.fgBypassDelay = MTRUE; 
            rSensorParam.fgBypassScenaio = MTRUE; 
        }
        //
        ret = pSingleShot->startOne(rSensorParam); 
        // 
        mu4CurrentCount++;       
        if (MTRUE == mfgIsCanceled)
        { 
            break; 
        }        
    }
    handleBestShotProcess();  
    handleCaptureDone(); 
   
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
BestShot::
onCmd_cancel()
{
    mfgIsCanceled = MTRUE; 
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL 
BestShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    BestShot *pBestShot = reinterpret_cast <BestShot *>(user); 
    if (NULL != pBestShot) 
    {
        // In best shot, the shutter callback only do at last image
        if (NSCamShot::ECamShot_NOTIFY_MSG_SOF == msg.msgType) 
        {         
            pBestShot->handleShutter();
        }
    }

    return MTRUE; 
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BestShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    BestShot *pBestShot = reinterpret_cast<BestShot *>(user); 
    if (NULL != pBestShot) 
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType) 
        {
            pBestShot->handlePostViewData(msg.puData, msg.u4Size); 
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pBestShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2); 
        }
    }

    return MTRUE; 
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGI("Current index: %d", mu4CurrentCount);
    if (mu4CurrentCount < mu4TotalCount)
    {   
        //  Save Postview 
        saveToTmpBuf(mvTmpBuf_postview[mu4CurrentCount], puBuf, u4Size);        
    }
    //
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize)
{
    MY_LOGI("Current index: %d", mu4CurrentCount);
    if (mu4CurrentCount < mu4TotalCount)
    {
        //
        MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024]; 
        MUINT32 u4ExifHeaderSize = 0; 
        makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
        MY_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)", 
                          puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
        //  Save  Jpeg and exif         
        saveToTmpBuf(mvTmpBuf_jpeg[mu4CurrentCount], puJpegBuf, u4JpegSize);
        saveToTmpBuf(mvTmpBuf_exif[mu4CurrentCount], puExifHeaderBuf, u4ExifHeaderSize);
        //
    }
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleShutter()
{
    if (mu4TotalCount-1 == mu4CurrentCount)
    {
        mpShotCallback->onCB_Shutter(true, 0); 
    }
    return MTRUE; 
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleCaptureDone()
{
    MBOOL   ret = MTRUE;
    //
    MY_LOGI("the last best-shot: %d is selected", mu4BestShotIndex);
    //  [1] restore the buffers of best index.
    MUINT32 u4PostviewSize = 0;
    MUINT32 u4JpgPictureSize = 0;
    MUINT32 u4ExifSize = 0; 

    // [2] invoke callbacks.
    // postView callback 
    mpShotCallback->onCB_PostviewDisplay(0, 
                                         mvTmpBuf_postview[mu4BestShotIndex].size(), 
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_postview[mu4BestShotIndex].begin())
                                        ); 

    // Jpeg callback 
    mpShotCallback->onCB_CompressedImage(0,
                                         mvTmpBuf_jpeg[mu4BestShotIndex].size(), 
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_jpeg[mu4BestShotIndex].begin()),
                                         mvTmpBuf_exif[mu4BestShotIndex].size(),                       //header size 
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_exif[mu4BestShotIndex].begin()),                    //header buf
                                         0,                       //callback index 
                                         true                     //final image 
                                         ); 
    //
    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
saveToTmpBuf(TmpBuf_t& rvTmpBuf, MUINT8 const*const pData, MUINT32 const u4Size)
{
    MY_LOGI("+ (data,size)=(%p,%d)", pData, u4Size);
    //
    if  ( ! pData || 0 == u4Size )
    {
        MY_LOGW("bad arguments - (pData,u4Size)=(%p,%d)", pData, u4Size);
        return  MFALSE;
    }
    //
    rvTmpBuf.resize(u4Size);
    ::memcpy(rvTmpBuf.begin(), pData, u4Size);
    //
    MY_LOGI("- TmpBuf:(size,capacity)=(%d,%d)", rvTmpBuf.size(), rvTmpBuf.capacity());
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
loadFromTmpBuf(TmpBuf_t const& rvTmpBuf, MUINT8*const pData, MUINT32& ru4Size)
{
    MY_LOGI("+ data(%p); TmpBuf:(size,capacity)=(%d,%d)", pData, rvTmpBuf.size(), rvTmpBuf.capacity());
    //
    if  ( ! pData )
    {
        MY_LOGW("bad arguments - pData(%p)", pData);
        return  MFALSE;
    }
    //
    ru4Size = rvTmpBuf.size();
    ::memcpy(pData, rvTmpBuf.begin(), ru4Size);
    //
    MY_LOGI("-");
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleBestShotProcess()
{
    MBOOL ret = MTRUE; 
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.bestshot.dump", value, "0"); 
    MUINT32 u4DumpFlag = ::atoi(value); 

    // compare the jpeg size 
    MUINT32 u4MaxSize = mvTmpBuf_jpeg[0].size(); 
    for (MUINT32 i = 1; i < BEST_SHOT_COUNT; i++) 
    {
        if (mvTmpBuf_jpeg[i].size() > u4MaxSize)
        {
            mu4BestShotIndex = i; 
            u4MaxSize = mvTmpBuf_jpeg[i].size(); 
        }
    }

    if (u4DumpFlag != 0) 
    {
        char fileName[256] = {'\0'};
        for (MUINT32 i = 0; i < BEST_SHOT_COUNT; i++) 
        {
            ::memset(fileName, '\0', 256); 
            ::sprintf(fileName, "//sdcard//%d_best_shot_%d.jpg", mu4BestShotIndex,  i );             
            MY_LOGD("opening file [%s]", fileName);
            int fd = ::open(fileName, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR|S_IRUSR);
            if (fd < 0) {
                MY_LOGE("failed to create file [%s]: %s", fileName, ::strerror(errno));
                return false;
            }

            ::write(fd,
                      reinterpret_cast<void const*>(mvTmpBuf_exif[i].begin()), 
                      mvTmpBuf_exif[i].size()
                     ); 
            ::write(fd,  
                      reinterpret_cast<void const*>(mvTmpBuf_jpeg[i].begin()),
                      mvTmpBuf_jpeg[i].size()
                     ); 
            ::close(fd);
        }
    }

    return ret;     
}


