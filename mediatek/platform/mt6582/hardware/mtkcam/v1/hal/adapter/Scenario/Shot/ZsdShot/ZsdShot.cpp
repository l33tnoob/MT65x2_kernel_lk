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

#define LOG_TAG "MtkCam/ZSDShot"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
//
#include <mtkcam/common/camutils/CamFormat.h>
#include <mtkcam/v1/camutils/CamInfo.h>
//
#include <mtkcam/v1/camutils/CamMisc.h>
#include <mtkcam/v1/camutils/CamProfile.h>

//
#include <mtkcam/hal/sensor_hal.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include <cutils/properties.h>

//
#include <mtkcam/camshot/ISImager.h>
using namespace NSCamShot;


#include <list>
using namespace std;

#include <mtkcam/v1/camutils/IBuffer.h>
#include <mtkcam/v1/camutils/IImgBufQueue.h>
using namespace android;
using namespace MtkCamUtils;
#include <ICaptureBufHandler.h>

#include <DpBlitStream.h>
#include <mtkcam/drv/imem_drv.h>

//
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;


#include "ZsdShot.h"

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

#define FUNCTION_LOG_START      MY_LOGD("[%s] +", __FUNCTION__);
#define FUNCTION_LOG_END        MY_LOGD("[%s] -", __FUNCTION__);

#define ZSD_DUMP_PATH "/sdcard/zsd/"
/******************************************************************************
 *
 ******************************************************************************/
extern "C"
sp<IShot>
createInstance_ZsdShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<ZsdShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new ZsdShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new ZsdShot", __FUNCTION__);
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
ZsdShot::
onCreate()
{
#warning "[TODO] ZsdShot::onCreate()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
ZsdShot::
onDestroy()
{
#warning "[TODO] ZsdShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
ZsdShot::
ZsdShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mJpegMem()
    , mYuvMem()
    , mThumbnailMem()
    , mpMemDrv(NULL)
    , mShotMode(0)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.camera.zsddump", value, "0");
    mu4DumpFlag = ::atoi(value);
    if (mu4DumpFlag) MtkCamUtils::makePath(ZSD_DUMP_PATH, 0660);
}


/******************************************************************************
 *
 ******************************************************************************/
ZsdShot::
~ZsdShot()
{
    if (mpCaptureBufHandler !=0 )
        mpCaptureBufHandler = 0;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdShot::
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
    case eCmd_setCaptureBufHandler:
        onCmd_setCaptureBufHandler(arg1, arg2);
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
ZsdShot::
onCmd_reset()
{
#warning "[TODO] ZsdShot::onCmd_reset()"
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdShot::
onCmd_setCaptureBufHandler(
    uint32_t const  arg1,
    uint32_t const  arg2)
{
    bool ret = true;
    ICaptureBufMgrHandler* pHandler =  reinterpret_cast<ICaptureBufMgrHandler*>(arg1);
    mpCaptureBufHandler = pHandler;
    mShotMode = arg2;
    MY_LOGD("mpCaptureBufMgr %d 0x%x, mShotMode(%d)",  mpCaptureBufHandler->getStrongCount(), mpCaptureBufHandler.get(), mShotMode);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdShot::
onYuv_capture(CapBufQueNode & rNode)
{
    FUNCTION_LOG_START;
    ImgBufInfo rMainImgBufInfo;
    ImgBufInfo rSubImgBufInfo;

    ImgBufInfo rJpgImgBufInfo;
    ImgBufInfo rYuvImgBufInfo;
    ImgBufInfo rThumbImgBufInfo;

    MUINT32 u4JpegSize = 0;
    MUINT32 u4ThumbnailSize = 0;

    bool ret = true;

    //Since Raw has been generated, call back shutter
    mpShotCallback->onCB_Shutter(true, 0);

    mapNodeToImageBuf(rNode.mainImgNode, rMainImgBufInfo);
    mapNodeToImageBuf(rNode.subImgNode, rSubImgBufInfo);
    mapNodeToImageBuf(rNode.subImgNode, rYuvImgBufInfo);

    MY_LOGD_IF(mShotParam.mi4Rotation != rNode.mainImgNode.getRotation(), "Shot Param/Buffer Info rotation not sync");
    mShotParam.mi4Rotation = rNode.mainImgNode.getRotation();

    CPTLogStr(Event_ZsdShot_capture, CPTFlagSeparator, "onYuv_alloc");

    ret = ret &&
          onYuv_alloc(rJpgImgBufInfo, rYuvImgBufInfo, rThumbImgBufInfo);

    if (checkIfNeedImgTransform(rMainImgBufInfo, rJpgImgBufInfo))
    {
        MY_LOGE("Main image: can not do image transform ");
        return false;
    }

    CPTLogStr(Event_ZsdShot_capture, CPTFlagSeparator, "createJpegImg:Main");
    //Main Image
    ret = ret &&
        createJpegImg(rMainImgBufInfo, mJpegParam.mu4JpegQuality, 0, rJpgImgBufInfo, u4JpegSize);


    CPTLogStr(Event_ZsdShot_capture, CPTFlagSeparator, "imageTransform");

    // use sub buffer to genarate Thumbnail YUV.
    if (checkIfNeedImgTransform(rSubImgBufInfo, rYuvImgBufInfo))
    {
        // DDP image transform downscale only support 6X. Max src may be 1920*1080?
        if (checkIfImgTransformSupport(rSubImgBufInfo, rYuvImgBufInfo, mShotParam.mi4Rotation))
        {
            ret = ret &&
                imageTransform(rSubImgBufInfo, rYuvImgBufInfo, mShotParam.mi4Rotation);
        }
        else
        {
            rMainImgBufInfo.u4ImgWidth = (rMainImgBufInfo.u4ImgWidth < rSubImgBufInfo.u4ImgWidth/2)
                                        ? rMainImgBufInfo.u4ImgWidth
                                        : rSubImgBufInfo.u4ImgWidth/2;
            rMainImgBufInfo.u4ImgHeight = (rMainImgBufInfo.u4ImgHeight < rSubImgBufInfo.u4ImgHeight/2)
                                        ? rMainImgBufInfo.u4ImgHeight
                                        : rSubImgBufInfo.u4ImgHeight/2;
            rMainImgBufInfo.eImgFmt = eImgFmt_YV12;

            ret = ret
                && imageTransform(rSubImgBufInfo, rMainImgBufInfo, 0)
                && imageTransform(rMainImgBufInfo, rYuvImgBufInfo, mShotParam.mi4Rotation)
                ;
        }


    }
    CPTLogStr(Event_ZsdShot_capture, CPTFlagSeparator, "createJpegImg:Thumb");

    //Thumb nail
    //create thumbnail
    if (0 != mJpegParam.mi4JpegThumbWidth && 0 != mJpegParam.mi4JpegThumbHeight)
    {
        ret = ret &&
            createJpegImg(rYuvImgBufInfo, mJpegParam.mu4JpegThumbQuality, 1, rThumbImgBufInfo, u4ThumbnailSize);
    }

    handleJpegData(reinterpret_cast<MUINT8*>(rJpgImgBufInfo.u4BufVA), u4JpegSize,
                   reinterpret_cast<MUINT8*>(rThumbImgBufInfo.u4BufVA), u4ThumbnailSize);

    onYuv_free();
    FUNCTION_LOG_END;

    return ret;
}

bool
ZsdShot::
onRaw_capture(CapBufQueNode & rNode)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "ZsdShot");
    //
    pSingleShot->init();

    //
    pSingleShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_EOF);
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


    ImgBufInfo rImgBufInfo;
    mapNodeToImageBuf(rNode.mainImgNode, rImgBufInfo);

    //
#if 0
    ret = pSingleShot->startOne(rImgBufInfo);
#else
    //workaround
    ret = pSingleShot->startOne(rSensorParam ,rImgBufInfo);
#endif

    //
    ret = pSingleShot->uninit();

    //
    pSingleShot->destroyInstance();

    FUNCTION_LOG_END;

    return ret;

}


/******************************************************************************
 *
 ******************************************************************************/
bool
ZsdShot::
onCmd_capture()
{
    MBOOL ret = MTRUE;
    //
    CPTLog(Event_ZsdShot_capture, CPTFlagStart);

    CapBufQueNode rNode;

    mpCaptureBufHandler->dequeProcessor(rNode, mpCaptureBufHandler->getStoredBufferCnt()-1);
    CPTLogStrEx(Event_ZsdShot_capture, CPTFlagSeparator, (MUINT32)rNode.mainImgNode->getVirAddr(), 0, "dequeProcessor");

    EImageFormat eSrcFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(rNode.mainImgNode->getImgFormat()));

    if ( mShotMode == 1 )
    {
        ret = onRaw_capture(rNode);
    }
    else
    {
        switch (eSrcFmt) {
            case eImgFmt_BAYER8:
            case eImgFmt_BAYER10:
            case eImgFmt_BAYER12:
                ret = onRaw_capture(rNode);
                break;
            case eImgFmt_YV12:
            case eImgFmt_I420:
            case eImgFmt_YVYU:
            case eImgFmt_VYUY:
            case eImgFmt_YUY2:
                ret = onYuv_capture(rNode);
                break;
            default:
                MY_LOGE("cant not handle this format %d", eSrcFmt);
                break;
        }
    }

    if (mu4DumpFlag)
    {
        //usleep(1000*1000*5);

        //
        if (rNode.mainImgNode.getImgBuf() != 0) {
            char fileName[256] = {'\0'};
            sprintf(fileName, ZSD_DUMP_PATH "zsd_cap_main_%dx%d.bin",  rNode.mainImgNode->getImgWidth(), rNode.mainImgNode->getImgHeight());
            MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rNode.mainImgNode->getVirAddr()), rNode.mainImgNode->getBufSize());
        }
        //
        if (rNode.subImgNode.getImgBuf() != 0) {
            char fileName[256] = {'\0'};
            sprintf(fileName, ZSD_DUMP_PATH "zsd_cap_sub_%dx%d.bin",  rNode.subImgNode->getImgWidth(), rNode.subImgNode->getImgHeight());
            MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rNode.subImgNode->getVirAddr()), rNode.subImgNode->getBufSize());
        }

    }

    mpCaptureBufHandler->enqueProcessor(rNode);

    CPTLog(Event_ZsdShot_capture, CPTFlagEnd);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ZsdShot::
onCmd_cancel()
{
#warning "[TODO] ZsdShot::onCmd_cancel()"
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ZsdShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    ZsdShot *pZsdShot = reinterpret_cast <ZsdShot *>(user);
    if (NULL != pZsdShot)
    {
        if (NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            pZsdShot->mpShotCallback->onCB_Shutter(true,
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
ZsdShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    ZsdShot *pZsdShot = reinterpret_cast<ZsdShot *>(user);
    if (NULL != pZsdShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            pZsdShot->handlePostViewData( msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pZsdShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2);
        }
        }

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
ZsdShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
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
ZsdShot::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize)
{
    AutoCPTLog cptlog(Event_ZsdShot_handleJpegData);
    MY_LOGD("+ (puJpgBuf, jpgSize, puThumbBuf, thumbSize) = (%p, %d, %p, %d)", puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize);

    MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024];
    MUINT32 u4ExifHeaderSize = 0;

    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);
    MY_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)",
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);
    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
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
/*******************************************************************************
*
********************************************************************************/
bool
ZsdShot::
checkIfNeedImgTransform(ImgBufInfo const & rSrcBufInfo, ImgBufInfo const & rDstBufInfo)
{
    if (rDstBufInfo.u4ImgWidth != rSrcBufInfo.u4ImgWidth ||
         rDstBufInfo.u4ImgHeight != rSrcBufInfo.u4ImgHeight)
    {
        MY_LOGD("Resize src =(%d,%d), dst=(%d,%d)",
                 rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                 rDstBufInfo.u4ImgWidth, rDstBufInfo.u4ImgHeight);
        return true;
    }
    //
    MY_LOGD("No need to do image transform");

    return false;
}

/*******************************************************************************
*
********************************************************************************/
bool
ZsdShot::
checkIfImgTransformSupport(ImgBufInfo const & rSrcBufInfo, ImgBufInfo const & rDstBufInfo, int rot)
{
    if (rot == 90 || rot == 270)
    {
        if (rDstBufInfo.u4ImgWidth*6 < rSrcBufInfo.u4ImgHeight ||
             rDstBufInfo.u4ImgHeight*6 < rSrcBufInfo.u4ImgWidth)
        {
            MY_LOGD("Not support Resize >6x src =(%d,%d), dst=(%d,%d), rot=(%d)",
                     rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                     rDstBufInfo.u4ImgWidth, rDstBufInfo.u4ImgHeight, rot);
            return false;
        }
    }
    else
    {
        if (rDstBufInfo.u4ImgWidth*6 < rSrcBufInfo.u4ImgWidth ||
             rDstBufInfo.u4ImgHeight*6 < rSrcBufInfo.u4ImgHeight)
        {
            MY_LOGD("Not support Resize >6x src =(%d,%d), dst=(%d,%d), rot=(%d)",
                     rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                     rDstBufInfo.u4ImgWidth, rDstBufInfo.u4ImgHeight, rot);
            return false;
        }

    }

    return true;
}

/*******************************************************************************
*
********************************************************************************/
bool
ZsdShot::
createJpegImg(ImgBufInfo const & rSrcImgBufInfo,
                    MUINT32 const u4u4Quality,
                    MUINT32 const u4fgIsSOI,
                    ImgBufInfo const & rJpgImgBufInfo,
                    MUINT32 & u4JpegSize)
{
    //FUNCTION_LOG_START;
    //MtkCamUtils::CamProfile profile("createJpegImg", "SingleShot");
    //
    // (1). Create Instance
    ISImager *pISImager = ISImager::createInstance(rSrcImgBufInfo);
    //CHECK_OBJECT(pISImager);

    // init setting
    BufInfo rBufInfo(rJpgImgBufInfo.u4BufSize, rJpgImgBufInfo.u4BufVA, rJpgImgBufInfo.u4BufPA, rJpgImgBufInfo.i4MemID);
    //
    pISImager->setTargetBufInfo(rBufInfo);
    //
    pISImager->setFormat(eImgFmt_JPEG);
    //
    // When ZSDCC, can not do rotation or flip in this stage, only create JPGE.
    pISImager->setRotation(0);
    //
    pISImager->setFlip(0);
    //
    pISImager->setResize(rJpgImgBufInfo.u4ImgWidth, rJpgImgBufInfo.u4ImgHeight);
    //
    pISImager->setEncodeParam(u4fgIsSOI, u4u4Quality);
    //
    pISImager->setROI(Rect(0, 0, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight));
    //
    pISImager->execute();
    //
    u4JpegSize = pISImager->getJpegSize();

    pISImager->destroyInstance();

    //profile.print();
    //FUNCTION_LOG_END;
    return true;
}

/*******************************************************************************
*
********************************************************************************/
bool
ZsdShot::
imageTransform  (ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, int const & rot)
{
    FUNCTION_LOG_START;
    //return MFALSE;
    //AutoCPTLog cptlog(Event_MShot_convertImage);

    //MY_LOGD("convertImage, src (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.i4MemID);
    //MY_LOGD("convertImage, dst (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rDstImgBufInfo.u4BufVA, rDstImgBufInfo.u4BufPA, rDstImgBufInfo.u4BufSize, rDstImgBufInfo.i4MemID);
    MY_LOGD("%d@%dx%d->%d@%dx%d(rot:%d)",
        rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight,
        rDstImgBufInfo.eImgFmt, rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight, rot);

    //MtkCamUtils::CamProfile profile("convertImage", "MultiShot");

    DpBlitStream thumbnailStream;
    DpColorFormat dp_in_fmt;
    DpColorFormat dp_out_fmt;
    DpInterlaceFormat dp_interlace_fmt = eInterlace_None;

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
            src_addr_list[1] = (unsigned int)(src_yp + src_ysize);
            src_addr_list[2] = (unsigned int)(src_yp + src_ysize * 5 / 4);

            src_size_list[0] = src_ysize;
            src_size_list[1] = src_vsize;
            src_size_list[2] = src_usize;

            plane_num = 3;
            dp_in_fmt = DP_COLOR_YV12;
            thumbnailStream.setSrcBuffer((void**)src_addr_list, src_size_list, plane_num);
            break;
        case eImgFmt_YUY2:
            src_addr_list[0] = (unsigned int)src_yp;

            src_size_list[0] = src_ysize*2;

            plane_num = 1;
            dp_in_fmt = DP_COLOR_YUYV;
            thumbnailStream.setSrcBuffer((void*)src_yp, src_ysize*2);
            break;
        default:
            MY_LOGE("not support src format %d", rSrcImgBufInfo.eImgFmt);
            return false;
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
    switch(rDstImgBufInfo.eImgFmt) {
        case eImgFmt_YV12:
            dst_addr_list[0] = (unsigned int)dst_yp;
            dst_addr_list[1] = (unsigned int)(dst_yp + dst_ysize);
            dst_addr_list[2] = (unsigned int)(dst_yp + dst_ysize * 5 / 4);

            dst_size_list[0] = dst_ysize;
            dst_size_list[1] = dst_vsize;
            dst_size_list[2] = dst_usize;

            plane_num_out = 3;
            dp_out_fmt = DP_COLOR_YV12;
            thumbnailStream.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num_out);
            break;
         case eImgFmt_YUY2:
            dst_addr_list[0] = (unsigned int)dst_yp;

            dst_size_list[0] = dst_ysize*2;
            plane_num_out = 1;
            dp_out_fmt = DP_COLOR_YUYV;
            thumbnailStream.setDstBuffer((void*)dst_yp, dst_ysize*2);
            break;
        default:
            return false;
            break;
    }


    //MY_LOGD("src addr (0, 1, 2) = (%p, %p, %p)", src_addr_list[0], src_addr_list[1], src_addr_list[2]);
    //MY_LOGD("dst addr (0, 1, 2) = (%p, %p, %p)", dst_addr_list[0], dst_addr_list[1], dst_addr_list[2]);

    //CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set src buffer");

    // set src buffer
    //thumbnailStream.setSrcBuffer((void**)src_addr_list, src_size_list, plane_num);

    //CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set src config");
    thumbnailStream.setSrcConfig(rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight, dp_in_fmt, dp_interlace_fmt);

    //CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set dst buffer");
    //thumbnailStream.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num_out);

   //CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "set dst config");
    thumbnailStream.setDstConfig(rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight, dp_out_fmt);

    thumbnailStream.setRotate(rot);
    //*****************************************************************************//
    //CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "invalidate");
    DP_STATUS_ENUM ret = thumbnailStream.invalidate();  //trigger HW
    if ( ret < 0 )
    {
          MY_LOGW("thumbnailStream invalidate failed %d", ret);
          return false;
    }
    //CPTLogStr(Event_MShot_convertImage, CPTFlagSeparator, "invalidate end");
    //profile.print();
    if (mu4DumpFlag)
    {
        //char fileNames[256] = {'\0'};
        //sprintf(fileNames, "/sdcard/zsd_sub_src_%dx%d.bin", rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight);
        //android::MtkCamUtils::saveBufToFile(fileNames, reinterpret_cast<MUINT8*>( rSrcImgBufInfo.u4BufVA), rSrcImgBufInfo.u4BufSize);
        /*
        char fileName[256] = {'\0'};
        sprintf(fileName, ZSD_DUMP_PATH "zsd_cap_thumb_%dx%d.bin",  rDstImgBufInfo.u4ImgWidth, rDstImgBufInfo.u4ImgHeight);
        MtkCamUtils::saveBufToFile(fileName, reinterpret_cast<MUINT8*>( rDstImgBufInfo.u4BufVA), rDstImgBufInfo.u4BufSize);
        */
        usleep(1000*1000);
    }

    FUNCTION_LOG_END;

    return true;
}

/*******************************************************************************
*
********************************************************************************/
inline void
ZsdShot::
setImageBuf(EImageFormat eFmt, MUINT32 const u4Width, MUINT32 const u4Height, ImgBufInfo & rBuf, IMEM_BUF_INFO & rMem)
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
inline void
ZsdShot::
mapNodeToImageBuf(ImgBufQueNode & rNode, ImgBufInfo & rBuf)
{
    rBuf.u4ImgWidth = rNode->getImgWidth();
    rBuf.u4ImgHeight = rNode->getImgHeight();
    rBuf.eImgFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(rNode->getImgFormat()));
    rBuf.u4Stride[0] = rNode->getImgWidthStride(0);
    rBuf.u4Stride[1] = rNode->getImgWidthStride(1);
    rBuf.u4Stride[2] = rNode->getImgWidthStride(2);
    rBuf.u4BufSize = rNode->getBufSize();
    rBuf.u4BufVA = (MUINT32)rNode->getVirAddr();
    rBuf.u4BufPA = (MUINT32)rNode->getPhyAddr();
    rBuf.i4MemID = rNode->getIonFd();
}


/******************************************************************************
*
*******************************************************************************/
bool
ZsdShot::
allocMem(IMEM_BUF_INFO & rMemBuf)
{
    //
    if (mpMemDrv->allocVirtBuf(&rMemBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
        return false;
    }
    //::memset((void*)rMemBuf.virtAddr, 0 , rMemBuf.size);
#if 1
    if (mpMemDrv->mapPhyAddr(&rMemBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
        return false;
    }
#endif
    return true;
}

/******************************************************************************
*
*******************************************************************************/
bool
ZsdShot::
deallocMem(IMEM_BUF_INFO & rMemBuf)
{
    //
#if 1
    if (mpMemDrv->unmapPhyAddr(&rMemBuf))
    {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        return false;
    }
#endif
    //
    if (mpMemDrv->freeVirtBuf(&rMemBuf))
    {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return false;
    }
    rMemBuf.size = 0;

    return true;
}

/*******************************************************************************
*
********************************************************************************/
bool
ZsdShot::
onYuv_alloc(ImgBufInfo   & rJpgImgBufInfo, ImgBufInfo & rYuvImgBufInfo, ImgBufInfo & rThumbImgBufInfo)
{
    FUNCTION_LOG_START;
    // init
    mpMemDrv = IMemDrv::createInstance();
    mpMemDrv->init();

    // JPEG
    MUINT32 u4Width = 0, u4Height = 0;
    MUINT32 u4TWidth = 0, u4THeight = 0;
    u4Width =  mShotParam.mi4PictureWidth;
    u4Height = mShotParam.mi4PictureHeight;
    u4TWidth =  mJpegParam.mi4JpegThumbWidth;
    u4THeight = mJpegParam.mi4JpegThumbHeight;
    if (90 == mShotParam.mi4Rotation || 270 == mShotParam.mi4Rotation)
    {
        u4Width = mShotParam.mi4PictureHeight;
        u4Height = mShotParam.mi4PictureWidth;
        u4TWidth =  mJpegParam.mi4JpegThumbHeight;
        u4THeight = mJpegParam.mi4JpegThumbWidth;
    }

    // Main JPEG
    if ( mJpegMem.size == 0)
    {
        mJpegMem.size = u4Width * u4Height ;    //? assume the JPEG ratio is 1/2
        MY_LOGD("allocate JPEG mem,(%dx%d) size = %d", u4Width, u4Height, mJpegMem.size);
        allocMem(mJpegMem);
        setImageBuf(eImgFmt_JPEG, u4Width, u4Height,  rJpgImgBufInfo, mJpegMem);
    }

    // ThumbNail YUV
    if (mYuvMem.size == 0)
    {
        //if (u4TWidth != rYuvImgBufInfo.u4ImgWidth || u4THeight != rYuvImgBufInfo.u4ImgHeight)
        {
            rYuvImgBufInfo.eImgFmt = eImgFmt_YUY2;
            mYuvMem.size = queryImgBufSize(rYuvImgBufInfo.eImgFmt ,u4TWidth, u4THeight);
            MY_LOGD("allocate YUV mem,(%dx%d) size = %d", u4TWidth, u4THeight,mYuvMem.size);
            allocMem(mYuvMem);
            setImageBuf(rYuvImgBufInfo.eImgFmt, u4TWidth, u4THeight, rYuvImgBufInfo, mYuvMem);
        }
    }

    // ThumbNail JPEG
    if (mThumbnailMem.size == 0)
    {
        mThumbnailMem.size = 128 * 1024;
        MY_LOGD("allocate thumbnail mem,(%dx%d) size = %d", mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight, mThumbnailMem.size);
        allocMem(mThumbnailMem);
        setImageBuf(eImgFmt_JPEG, u4TWidth, u4THeight, rThumbImgBufInfo, mThumbnailMem);
    }
    FUNCTION_LOG_END;

    return true;

}




/*******************************************************************************
*
********************************************************************************/
bool
ZsdShot::
onYuv_free  ()
{
    FUNCTION_LOG_START;
    // Yuv
    if (0 != mYuvMem.size)
    {
        deallocMem(mYuvMem);
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

    mpMemDrv->uninit();
    mpMemDrv->destroyInstance();
    FUNCTION_LOG_END;
    return true;

}

/*******************************************************************************
*
********************************************************************************/
MUINT32
ZsdShot::
queryImgStride(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4PlaneIndex)
{
#define ALIGN_16(x) ( (~15) & ((15) + (x)) )
    MUINT32 u4Stride = 0;
    //
    switch(eFmt)
    {
        // YUV 420 format 
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK: 
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
            u4Stride = (u4PlaneIndex == 2) ? (0) : u4Width ; 
            break; 
        case eImgFmt_YV12:
            u4Stride = (u4PlaneIndex == 0) ? ALIGN_16( u4Width ) : ALIGN_16( u4Width>>1 );
            break; 
        case eImgFmt_I420:
            u4Stride = (u4PlaneIndex == 0) ? ALIGN_16( u4Width ) : ALIGN_16( u4Width>>1 );
            break; 
        // YUV 422 format , RGB565
        case eImgFmt_YUY2: 
        case eImgFmt_UYVY:
        case eImgFmt_VYUY:
        case eImgFmt_YVYU:
        case eImgFmt_RGB565:
            u4Stride = (u4PlaneIndex == 0) ? (u4Width) : 0; 
            break; 
        case eImgFmt_YV16:
        case eImgFmt_I422:
        case eImgFmt_NV16:
        case eImgFmt_NV61:        
            u4Stride = (u4PlaneIndex == 0) ? (u4Width) : (u4Width >> 1); 
            break;         
        case eImgFmt_RGB888:         
            u4Stride = u4Width; 
            break; 
        case eImgFmt_ARGB888:
            u4Stride = u4Width; 
            break; 
        case eImgFmt_JPEG:
            u4Stride = u4Width ; 
            break; 
        case eImgFmt_Y800:
            u4Stride = (u4PlaneIndex == 0) ? (u4Width) : (0); 
            break; 
        default:
            u4Stride = u4Width; 
            break; 
    }
    return u4Stride;
#undef ALIGN_16(x)
}
/*******************************************************************************
*
********************************************************************************/
MUINT32
ZsdShot::
queryImgBufSize(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height)
{
    MUINT32 u4BufSize = 0;
    //
    switch(eFmt)
    {
        // YUV 420 format 
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK: 
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
            u4BufSize = u4Width * u4Height * 3 / 2; 
            break; 
        case eImgFmt_YV12:
        case eImgFmt_I420:
            u4BufSize = queryImgStride(eFmt, u4Width, 0) * u4Height +
                        queryImgStride(eFmt, u4Width, 1) * u4Height;
            break;
        // YUV 422 format , RGB565
        case eImgFmt_YUY2: 
        case eImgFmt_UYVY:
        case eImgFmt_VYUY:
        case eImgFmt_YVYU: 
        case eImgFmt_YV16:
        case eImgFmt_I422:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
        case eImgFmt_RGB565:
            u4BufSize = u4Width * u4Height * 2; 
            break;         
        case eImgFmt_RGB888:         
            u4BufSize = u4Width * u4Height * 3; 
            break; 
        case eImgFmt_ARGB888:
            u4BufSize = u4Width * u4Height * 4; 
            break; 
        case eImgFmt_JPEG:
            u4BufSize = u4Width * u4Height;    //? assume the JPEG ratio is 1/2 
            break; 
        case eImgFmt_Y800:
            u4BufSize = u4Width * u4Height; 
            break; 
        default:
            u4BufSize = 0;
            break;
    }
    return u4BufSize;
}
