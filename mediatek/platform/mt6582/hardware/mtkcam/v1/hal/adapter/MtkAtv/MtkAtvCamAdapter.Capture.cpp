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

#define LOG_TAG "MtkATV/Capture"
//
#include <camera/MtkCamera.h>
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>

// [ATV]+
#include <mtkcam/camshot/ISImager.h>
#include <core/camshot/inc/ImageUtils.h>

using namespace NSCamShot;
// [ATV]-

//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <mtkcam/v1/hwscenario/HwBuffHandler.h>
#include "inc/MtkAtvCamAdapter.h"

#include <stdio.h>
extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
    #include <setjmp.h>
}



using namespace NSMtkAtvCamAdapter;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

// [ATV] +
#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
// [ATV] -

/******************************************************************************
*   Function Prototype.
*******************************************************************************/
bool
createShotInstance(
    sp<IShot>&          rpShot, 
    uint32_t const      u4ShotMode, 
    int32_t const       i4OpenId, 
    sp<IParamsManager>  pParamsMgr
);


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
updateShotInstance()
{
    String8 const s8ShotMode = getParamsManager()->getShotModeStr();
    uint32_t const u4ShotMode = getParamsManager()->getShotMode();
    MY_LOGI("<shot mode> %#x(%s)", u4ShotMode, s8ShotMode.string());
    //
    return  createShotInstance(mpShot, u4ShotMode, getOpenId(), getParamsManager());
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isTakingPicture() const
{
    bool ret =  mpStateManager->isState(IState::eState_CapturePreview)  // [ATV]
            ||  mpStateManager->isState(IState::eState_PreCapture)
                ;
    if  ( ret )
    {
        MY_LOGD("isTakingPicture(1):%s", mpStateManager->getCurrentState()->getName());
    }
    //
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
takePicture()
{
    MY_LOGD("+");
    status_t status = OK;
    //
    status = mpStateManager->getCurrentState()->onPreCapture(this);
    if  ( OK != status ) {
        MY_LOGD("onPreCapture fail");
        goto lbExit;
    }
    //[ATV]+
    /*
    status = mpStateManager->getCurrentState()->onStopPreview(this);
    if  ( OK != status ) {
        goto lbExit;
    }
    */
    //[ATV]-
    status = mpStateManager->getCurrentState()->onCapture(this);
    if  ( OK != status ) {
        MY_LOGD("onCapture fail");
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD("-");

    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
cancelPicture()
{
    return  mpStateManager->getCurrentState()->onCancelCapture(this);
}

/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
setCShotSpeed(int32_t i4CShotSpeeed)
{
    if(i4CShotSpeeed <= 0)
    {
        MY_LOGE("can not set continuous shot speed as %d fps)", i4CShotSpeeed);
        return BAD_VALUE;
    }
    
    sp<IShot> pShot = mpShot;
    if  ( pShot != 0 )
    {
        pShot->sendCommand(eCmd_setCShotSpeed);
    }
    
    return OK;
}



/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() -> 
*   IStateHandler::onHandleCapture() -> CamAdapter::onHandleCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCapture()
{
    MY_LOGD("+");
    status_t status = DEAD_OBJECT;
    //
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
    MY_LOGD("-");
    return  status;
}


/******************************************************************************
*   
*******************************************************************************/
status_t
CamAdapter::
onHandleCaptureDone()
{
    MY_LOGD("+");
#if 0
    //  Message may disable before shutter/image callback if: DONE --> Image CB
    mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE, 0, mCallbackCookie);
#endif
    // [ATV]+
    /*
    mpStateManager->transitState(IState::eState_Idle);
    */
    mpStateManager->transitState(IState::eState_Preview);
    // [ATV]-
    MY_LOGD("-");
    return  OK;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() -> 
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelCapture()
{
    sp<IShot> pShot = mpShot;
    if  ( pShot != 0 )
    {
        pShot->sendCommand(eCmd_cancel);
    }
    //
    
    // [ATV]+ sam need or not, will run onHandleCaptureDone ???
    mpStateManager->transitState(IState::eState_Preview); 
    // [ATV]-
    return  OK;
}


bool
CamAdapter::
onCaptureThreadLoop()
{
    MY_LOGD("+");
    bool ret = false;
    //
    //  [1] transit to "Capture" state.
    mpStateManager->transitState(IState::eState_CapturePreview);  // [ATV]    
    
    // [2.1]. Create ISImager Instance 
    ImgBufInfo rSrcImgBufInfo;
    uint32_t u4JpegSize = 0;

    rSrcImgBufInfo.u4ImgWidth = mpCurPrvBuf->getImgWidth();
    rSrcImgBufInfo.u4ImgHeight = mpCurPrvBuf->getImgHeight();
    rSrcImgBufInfo.eImgFmt = (EImageFormat)(android::MtkCamUtils::FmtUtils::queryImageioFormat(mpCurPrvBuf->getImgFormat()));    
    rSrcImgBufInfo.u4Stride[0] = queryImgStride(rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4ImgWidth, 0); 
    rSrcImgBufInfo.u4Stride[1] = queryImgStride(rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4ImgWidth, 1); 
    rSrcImgBufInfo.u4Stride[2] = queryImgStride(rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4ImgWidth, 2); 
    rSrcImgBufInfo.u4BufSize = mpCurPrvBuf->getBufSize(); 
    rSrcImgBufInfo.u4BufVA = reinterpret_cast<MUINT32 const>(mpCurPrvBuf->getVirAddr());
    rSrcImgBufInfo.u4BufPA = reinterpret_cast<MUINT32 const>(mpCurPrvBuf->getPhyAddr());
    rSrcImgBufInfo.i4MemID = mpCurPrvBuf->getIonFd();


    MY_LOGD("rSrcImgBufInfo w %d, h %d, fmt %d, Va %p size %d", rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight,
                rSrcImgBufInfo.eImgFmt, rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufSize);

    unsigned char *rDstImgBufInfo = new unsigned char[mpCurPrvBuf->getBufSize()];

    {

        ALOGD("YV12ToJpeg, src (VA, PA, Size, ID) = (0x%x, 0x%x, %d, %d)", rSrcImgBufInfo.u4BufVA, rSrcImgBufInfo.u4BufPA, rSrcImgBufInfo.u4BufSize, rSrcImgBufInfo.i4MemID); 
        
        unsigned char* Y = (unsigned char *)(rSrcImgBufInfo.u4BufVA);
        unsigned char* U = Y + rSrcImgBufInfo.u4ImgWidth * rSrcImgBufInfo.u4ImgHeight*5/4;
        unsigned char* V  =  Y + rSrcImgBufInfo.u4ImgWidth * rSrcImgBufInfo.u4ImgHeight; 
        int width = rSrcImgBufInfo.u4ImgWidth;
        int height = rSrcImgBufInfo.u4ImgHeight;
        int quality = 75;
        unsigned char* dst = (unsigned char *)(rDstImgBufInfo); 
        long unsigned int jpegSize = mpCurPrvBuf->getBufSize();

        //if (width %8 != 0 || height % 8 != 0) return -1;
        int i =0,j=0;
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
        
        jpeg_create_compress (&cinfo);

        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_YCbCr;
        jpeg_set_defaults (&cinfo);
        //jpeg_set_colorspace(&cinfo, JCS_YCbCr);
        cinfo.raw_data_in = TRUE; 

        // supply downsampled data
        cinfo.comp_info[0].h_samp_factor = 2;
        cinfo.comp_info[0].v_samp_factor = 2;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
        
        jpeg_set_quality (&cinfo, quality, true);
        cinfo.dct_method = JDCT_IFAST;
        jpeg_mem_dest (&cinfo, &dst, &jpegSize);    
//        #warning "[FIXME] jpeg_mem_dest not ready"
        // data written to file
        jpeg_start_compress (&cinfo, TRUE);

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
        
        jpeg_finish_compress (&cinfo);
        jpeg_destroy_compress (&cinfo);

        u4JpegSize = jpegSize;
        MY_LOGD("YV12ToJpeg done size %d %x, %x %x", u4JpegSize, *rDstImgBufInfo, *(rDstImgBufInfo+1), *(rDstImgBufInfo+2));

    ///}
    }
    
    // call back jpeg data
    onCB_CompressedImage(0,
                         u4JpegSize, 
                         (unsigned char *)(rDstImgBufInfo),
                         0,        // FIX this, no header ???
                         NULL,  
                         0, 
                         true
                         ); 
    
    //
    //  [5.2] notify capture done.
    mpStateManager->getCurrentState()->onCaptureDone(this);
    //
    //
    ///dstimage.clear();
    if(rDstImgBufInfo != NULL)
        delete [] rDstImgBufInfo;
    
    MY_LOGD("-");
    return  true;
}


