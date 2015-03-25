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
#define LOG_TAG "MtkCam/CamClient/PanoramaClient"

#include <DpBlitStream.h>
#include "PanoramaClient.h"
//
using namespace NSCamClient;

/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
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
//#define debug
#ifdef debug
#include <sys/stat.h>
#include <fcntl.h>
int Mcount=0;

bool
dumpBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    CAM_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    CAM_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        CAM_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    CAM_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            CAM_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    CAM_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}
#endif


/******************************************************************************
 *
 ******************************************************************************/
MINT32
PanoramaClient::
CreateMotionSrc(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr)
{
    bool ret = true;
    MY_LOGD("[CreateMotionSrc] +");
    DpBlitStream Motionstream;

    unsigned int src_addr_list[3];
    unsigned int src_size_list[3];
    unsigned int dst_addr_list[3];
    unsigned int dst_size_list[3];

    unsigned char *src_yp;
    unsigned char *dst_rgb;

    int src_ysize = ImgWidth * ImgHeight;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;
    MY_LOGD("[CreateMotionSrc] src_ysize %d adr 0x%x w %d h %d",src_ysize,srcbufadr,ImgWidth,ImgHeight);
    int plane_num = 3;
    src_yp = (unsigned char *)srcbufadr;
    src_addr_list[0] = (unsigned int)src_yp;
    src_addr_list[1] = (unsigned int)(src_yp + src_ysize);
    src_addr_list[2] = (unsigned int)(src_yp + src_ysize + src_usize);

    src_size_list[0] = src_ysize;
    src_size_list[1] = src_vsize;
    src_size_list[2] = src_usize;
    //*****************************************************************************//
    Motionstream.setSrcBuffer((void**)src_addr_list, src_size_list, plane_num);
    Motionstream.setSrcConfig(ImgWidth,ImgHeight, DP_COLOR_YV12);

     //***************************dst RGB565********************************//
    dst_rgb = (unsigned char *)dstbufadr;
    Motionstream.setDstBuffer((void *)dst_rgb, MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT*2);
    Motionstream.setDstConfig(MOTION_MAX_IN_WIDTH, MOTION_MAX_IN_HEIGHT, DP_COLOR_RGB565);
    Motionstream.setRotate(0);

    //*****************************************************************************//

     MY_LOGD("DDP_Performance_RGB565 Start");

    // set & add pipe to stream
    if (!Motionstream.invalidate())  //trigger HW
    {
          MY_LOGD("FDstream invalidate failed");
          return false;
    }
    #ifdef debug
    char sourceFiles[80];
    sprintf(sourceFiles, "%s%d_%dx%d.rgb565", "/sdcard/motion", Mcount,MOTION_MAX_IN_WIDTH,MOTION_MAX_IN_HEIGHT);
    dumpBufToFile((char *) sourceFiles, (MUINT8 *)dst_rgb , (MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 2));
    Mcount++;
    #endif
    MY_LOGD("DDP_Performance_RGB565 End");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
PanoramaClient::
ISShot(MVOID * bufadr, MVOID *arg1, MBOOL &shot)
{
    bool ret = false;

    MY_LOGD("[ISShot] +");
    MINT32 err = NO_ERROR;

    err = mpPanoramaObj->mHalAutoramaDoMotion((MUINT32 *)bufadr, (MUINT32 *) arg1);
    if ( err != NO_ERROR ) {
        MY_LOGE("[ISShot] mHalAutoramaDoMotion fail");
        return true;
    }

    shot = ((AutoramaMotionResult*)arg1)->ReadyToShot > 0 ? true : false;
    MY_LOGD("[ISShot] +");

    return ret;

}


MINT32
PanoramaClient::
mHalCamFeatureProc(MVOID * bufadr, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot)
{
    Mutex::Autolock lock(mLockUninit);

    MY_LOGD("[mHalCamPanoramaProc]");
    MINT32 err = NO_ERROR;
    isShot = false;


    AutoramaMotionResult PanoramaResult;
    memset((void*)&PanoramaResult,0,sizeof(AutoramaMotionResult));
    CreateMotionSrc(bufadr, mPanoramaFrameWidth, mPanoramaFrameHeight, (MVOID*)mpMotionBuffer.virtAddr);
    err = ISShot((MVOID*)mpMotionBuffer.virtAddr, &PanoramaResult, isShot);

    mStitchDir = (MTKPIPEAUTORAMA_DIRECTION_ENUM)PanoramaResult.Direction;
    MINT32 MV[3] = {PanoramaResult.MV_X, PanoramaResult.MV_Y, mStitchDir};
    if (mPanoramaFrameIdx == 0)
    {
       isShot = true;
       PanoramaResult.MV_X = 0;
       PanoramaResult.MV_Y = 0;
       mStitchDir = MTKPIPEAUTORAMA_DIR_NO;
       //mpMhalCam->mHalCam3ACtrl(1, 0, 1);
       //mpHal3AObj->setAutoExposureLock(0);
    }

    MY_LOGD("PanoramaResult.MV X: %d Y: %d dir %d isShot %d ", PanoramaResult.MV_X, PanoramaResult.MV_Y, mStitchDir, isShot);
    if (isShot)
    {
        mpHal3A->enableAELimiterControl(true);
        if (mPanoramaFrameIdx < PanoramaNum)
        {
            MY_LOGD("[Panorama]Num %d total %d", mPanoramaFrameIdx,PanoramaNum);

            MUINT8 *pbufIn = (MUINT8 *) bufadr;
            MUINT8 *pbufOut = (MUINT8 *) (mpframeBuffer.virtAddr + ((mPanoramaFrameWidth * mPanoramaFrameHeight * 3 / 2) * mPanoramaFrameIdx));
            memcpy(pbufOut, pbufIn, (mPanoramaFrameWidth * mPanoramaFrameHeight * 3 / 2) );

            #ifdef debug
            char sourceFiles[80];
            sprintf(sourceFiles, "%s%d.raw", "/sdcard/getpreview", mPanoramaFrameIdx);
            dumpBufToFile((char *) sourceFiles, pbufOut , (mPanoramaFrameWidth * mPanoramaFrameHeight * 3 / 2));
            #endif

            mPanoramaFrameIdx++;
            sem_post(&PanoramaSemThread);
        }
        else
        {
            MY_LOGD("[Panorama]Num %d", mPanoramaFrameIdx);
            mPanoramaFrameIdx++;
        }
    }
    mvX = PanoramaResult.MV_X;
    mvY = PanoramaResult.MV_Y;
    dir = mStitchDir;

    if(err)
        return false;
    else
        return true;
}

