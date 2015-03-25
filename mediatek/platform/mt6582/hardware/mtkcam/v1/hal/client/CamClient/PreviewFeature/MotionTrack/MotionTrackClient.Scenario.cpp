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
#define LOG_TAG "MtkCam/CamClient/MotionTrackClient"

#include "DpBlitStream.h"
#include "MotionTrackClient.h"
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
static MUINT16      mEISMethod = 1;

//#define MOTIONTRACK_DEBUG
#ifdef MOTIONTRACK_DEBUG
#include <sys/stat.h>
#include <fcntl.h>
int Mcount=0;

bool
static dumpBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
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

static MINT32
ConvertNV21toYV12(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr)
{
    bool ret = true;
    MY_LOGD("[ConvertNV21toYV12] +");
    DpBlitStream Motionstream;

    unsigned int src_addr_list[2];
    unsigned int src_size_list[2];

    unsigned char *src_yp;

    int src_ysize = ImgWidth * ImgHeight;
    int src_uvsize;
    src_uvsize = src_ysize / 2;
    MY_LOGD("[ConvertNV21toYV12] src_ysize %d adr 0x%x w %d h %d",src_ysize,srcbufadr,ImgWidth,ImgHeight);
    int plane_num = 2;
    src_yp = (unsigned char *)srcbufadr;
    src_addr_list[0] = (unsigned int)src_yp;
    src_addr_list[1] = (unsigned int)(src_yp + src_ysize);

    src_size_list[0] = src_ysize;
    src_size_list[1] = src_uvsize;
    //*****************************************************************************//
    Motionstream.setSrcBuffer((void**)src_addr_list, src_size_list, plane_num);
    Motionstream.setSrcConfig(ImgWidth,ImgHeight, DP_COLOR_NV21);

    //***************************dst YV12********************************//
    int dst_ysize = ImgWidth * ImgHeight;
    int dst_usize, dst_vsize;
    dst_usize = dst_vsize = dst_ysize / 4;
    unsigned int dst_addr_list[3];
    unsigned int dst_size_list[3];
    plane_num = 3;
    dst_addr_list[0] = (unsigned int)dstbufadr;
    dst_addr_list[1] = (unsigned int)(dstbufadr + dst_ysize);
    dst_addr_list[2] = (unsigned int)(dstbufadr + dst_ysize + dst_usize);

    dst_size_list[0] = dst_ysize;
    dst_size_list[1] = dst_vsize;
    dst_size_list[2] = dst_usize;
    Motionstream.setDstBuffer((void **)dst_addr_list, dst_size_list, plane_num);
    Motionstream.setDstConfig(ImgWidth, ImgHeight, DP_COLOR_YV12);
    Motionstream.setRotate(0);

    // set & add pipe to stream
    if (Motionstream.invalidate())  //trigger HW
    {
          MY_LOGD("[ConvertNV21toYV12] FDstream invalidate failed");
          return false;
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
MotionTrackClient::
CreateThumbImage(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr)
{
    bool ret = true;
    MY_LOGD("[CreateThumbImage] +");
    DpBlitStream Motionstream;

    unsigned int src_addr_list[3];
    unsigned int src_size_list[3];

    unsigned char *src_yp;

    int src_ysize = ImgWidth * ImgHeight;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;
    MY_LOGD("[CreateThumbImage] src_ysize %d adr 0x%x w %d h %d",src_ysize,srcbufadr,ImgWidth,ImgHeight);
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
    int dst_ysize = (ImgWidth/8) * (ImgHeight/8);
    int dst_usize, dst_vsize;
    dst_usize = dst_vsize = dst_ysize / 4;
    unsigned int dst_addr_list[3];
    unsigned int dst_size_list[3];
    dst_addr_list[0] = (unsigned int)dstbufadr;
    dst_addr_list[1] = (unsigned int)(dstbufadr + dst_ysize);
    dst_addr_list[2] = (unsigned int)(dstbufadr + dst_ysize + dst_usize);

    dst_size_list[0] = dst_ysize;
    dst_size_list[1] = dst_vsize;
    dst_size_list[2] = dst_usize;
    Motionstream.setDstBuffer((void **)dst_addr_list, dst_size_list, plane_num);
    Motionstream.setDstConfig(ImgWidth/8, ImgHeight/8, DP_COLOR_YV12);
    Motionstream.setRotate(0);

    // set & add pipe to stream
    if (Motionstream.invalidate())  //trigger HW
    {
          MY_LOGD("[CreateThumbImage] FDstream invalidate failed");
          return false;
    }
    return ret;
}

MVOID
MotionTrackClient::
updateEISMethod(MUINT16 method)
{
    if (mEISMethod > 2)
        mEISMethod = 1;
    mEISMethod = method;
    MY_LOGD("EIS method = %d", mEISMethod);
}

MINT32
MotionTrackClient::
mHalCamFeatureProc(MVOID * bufadr, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot)
{
    Mutex::Autolock lock(mLockUninit);

    MY_LOGD("[mHalCamMotionTrackProc]");

    if (mPreviewFrameCount == 0)
    {
       isShot = true;
       mvX = mMvX = 0;
       mvY = mMvY = 0;
       mTimelapse = 0;
    }
    else
    {
        mpEisHal->getEISGmv((MUINT32&)mvX, (MUINT32&)mvY);
        mMvX -= mvX * mMotionTrackFrameWidth / mPreviewCropWidth / 256;
        mMvY -= mvY * mMotionTrackFrameHeight / mPreviewCropHeight / 256;
        mvX = mMvX;
        mvY = mMvY;
        MY_LOGD("[mHalCamMotionTrackProc] EIS mvx %d, mvy %d, %d ms", mvX, mvY, dir);

        isShot = false;
        if (mTimestamp <= dir)
        {
            if ((mTimelapse += (dir - mTimestamp)) > (200))
            {
                isShot = true;
                mTimelapse = 0;
            }
        }
    }
    mTimestamp = dir;
    mPreviewFrameCount++;

    if (isShot)
    {
        if (mMotionTrackFrameIdx < MotionTrackNum)
        {
            MY_LOGD("[mHalCamMotionTrackProc] Shot %d", mMotionTrackFrameIdx);

            /* Convert from NV21 to YV12 */
            MVOID * yv12bufadr = (MVOID *) (mpFrameBuffer.virtAddr + (mMotionTrackFrameIdx * mMotionTrackFrameSize));
            ConvertNV21toYV12(bufadr, mMotionTrackFrameWidth, mMotionTrackFrameHeight, yv12bufadr);

            /* Add small images */
            CreateThumbImage(yv12bufadr, mMotionTrackFrameWidth, mMotionTrackFrameHeight, (MVOID*)mpThumbBuffer.virtAddr);
            #ifdef MOTIONTRACK_DEBUG
            char sourceFiles[80];
            sprintf(sourceFiles, "%s_%dx%d_%02d.yuv", "/sdcard/frame", mMotionTrackFrameWidth, mMotionTrackFrameHeight, mMotionTrackFrameIdx);
            dumpBufToFile((char *) sourceFiles, (MUINT8*) yv12bufadr, mMotionTrackFrameSize);
            sprintf(sourceFiles, "%s_%dx%d_%02d.yuv", "/sdcard/thumb", mMotionTrackFrameWidth/8, mMotionTrackFrameHeight/8, mMotionTrackFrameIdx);
            dumpBufToFile((char *) sourceFiles, (MUINT8 *)mpThumbBuffer.virtAddr, mMotionTrackThumbSize);
            Mcount++;
            #endif

            MTKPipeMotionTrackAddImageInfo MotionTrackAddImageInfo;
            MotionTrackAddImageInfo.ImageIndex = mMotionTrackFrameIdx;
            MotionTrackAddImageInfo.ThumbImageAddr = (MUINT8*)mpThumbBuffer.virtAddr;
            MotionTrackAddImageInfo.ThumbImageWidth = mMotionTrackFrameWidth / 8;
            MotionTrackAddImageInfo.ThumbImageHeight = mMotionTrackFrameHeight / 8;
            MotionTrackAddImageInfo.ThumbImageStrideY = MotionTrackAddImageInfo.ThumbImageWidth;
            MotionTrackAddImageInfo.ThumbImageStrideUV = MotionTrackAddImageInfo.ThumbImageStrideY / 2;
            /* Get EIS accumulated GMV */
            // MFLOAT floatMvX, floatMvY;
            // if (mMotionTrackFrameIdx == 0)
            // {
                // mpEisHal->startAccumulationMode(50);
            // }
            // mpEisHal->getAccumulatedGMV(floatMvX, floatMvY);
            // MotionTrackAddImageInfo.MvX = (MINT32) (((floatMvX * mMotionTrackFrameWidth) + (((floatMvX > 0)? ((MFLOAT) 1): ((MFLOAT) -1)) * mPreviewCropWidth / 2)) / mPreviewCropWidth);
            // MotionTrackAddImageInfo.MvY = (MINT32) (((floatMvY * mMotionTrackFrameHeight) + (((floatMvY > 0)? ((MFLOAT) 1): ((MFLOAT) -1)) * mPreviewCropHeight / 2)) / mPreviewCropHeight);
// #if 1//Remove later
            // if (mEISMethod == 1)
            // {
            MotionTrackAddImageInfo.MvX = -mvX;
            MotionTrackAddImageInfo.MvY = -mvY;
            // }
// #endif
            MY_LOGD("[mHalCamMotionTrackProc] EIS accumulated mvx %d, mvy %d", MotionTrackAddImageInfo.MvX, MotionTrackAddImageInfo.MvY);

            int err = NO_ERROR;
            err = mpMotionTrackObj->mHalMotionTrackAddImage(MotionTrackAddImageInfo);
            if (!err)
            {
                MY_LOGE("mHalCamMotionTrackProc: mHalMotionTrackAddImage failed");            
                MY_LOGE("mHalCamMotionTrackProc: Skip this index");            
                return true;
            }	

            MUINT32 cbdata[5];
            cbdata[0] = 1;  /* Callback on Captured Images */
            cbdata[1] = mMotionTrackFrameIdx;
            cbdata[4] = (MUINT32) bufadr;
            mDataCb((MVOID*) cbdata, mMotionTrackFrameWidth, mMotionTrackFrameHeight);

            mMotionTrackaddImgIdx = mMotionTrackFrameIdx;
        }
        else
        {
            MY_LOGD("[MotionTrack]Num %d", mMotionTrackFrameIdx);
            sem_post(&MotionTrackAddImgDone);
        }
        ++mMotionTrackFrameIdx;
    }

    return true;
}

