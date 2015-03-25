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
#define LOG_TAG "CamShot/SImager"
//
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");

//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>

//
#include <mtkcam/camshot/_callbacks.h>
//
#include "../inc/ImageUtils.h"
//
#include "../inc/IJpegCodec.h"
#include "../inc/IImageTransform.h"
//
#include "../inc/SImager.h"

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
SImager::
SImager(
    ImgBufInfo const & rImgBufInfo
)
    : mi4ErrorCode(0)
    , mpCbUser(NULL)
    , mNotifyCb(NULL)
    , mu4JpegSize(0)
    , mSrcImgBufInfo(rImgBufInfo)
    , mTargetImgInfo()
    , mTargetBufInfo()
{
    mTargetImgInfo.eImgFmt = rImgBufInfo.eImgFmt; 
    mTargetImgInfo.u4ImgWidth = rImgBufInfo.u4ImgWidth; 
    mTargetImgInfo.u4ImgHeight = rImgBufInfo.u4ImgHeight; 
    mTargetImgInfo.u4Rotation = 0; 
    mTargetImgInfo.u4Flip = 0; 
    mTargetImgInfo.rROI.x = 0;   
    mTargetImgInfo.rROI.y = 0; 
    mTargetImgInfo.rROI.w = rImgBufInfo.u4ImgWidth; 
    mTargetImgInfo.rROI.h = rImgBufInfo.u4ImgHeight; 

    MY_LOGD("Src (fmt, width, height) = (0x%x, %d, %d)", rImgBufInfo.eImgFmt, rImgBufInfo.u4ImgWidth, rImgBufInfo.u4ImgHeight); 
    MY_LOGD("src (VA, PA, Size, memId) = (0x%x, 0x%x, %d, %d)", rImgBufInfo.u4BufVA, rImgBufInfo.u4BufPA, rImgBufInfo.u4BufSize, rImgBufInfo.i4MemID); 
 

    mu4StrideAlign[0] = mu4StrideAlign[1] = mu4StrideAlign[2] = 1; 
}

/*******************************************************************************
* 
********************************************************************************/
MVOID
SImager::
setCallback(SImagerNotifyCallback_t notify_cb, MVOID* user)
{
    MY_LOGV("(notify_cb, user)=(%p, %p, %p)", notify_cb, user);
    mpCbUser = user;
    mNotifyCb = notify_cb;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
execute(MUINT32 const u4TimeoutMs)
{
    FUNCTION_LOG_START;
    //
    if (!isSupportedSrcFormat(mSrcImgBufInfo.eImgFmt)) 
    {
        MY_LOGE("Unsupport source format:0x%x", mSrcImgBufInfo.eImgFmt); 
        return MFALSE; 
    }   

    MUINT32 u4Width = mTargetImgInfo.u4ImgWidth; 
    MUINT32 u4Height = mTargetImgInfo.u4ImgHeight; 
    // rotation 
    if(90 == mTargetImgInfo.u4Rotation || 270 == mTargetImgInfo.u4Rotation)
    {
        u4Width = mTargetImgInfo.u4ImgHeight; 
        u4Height = mTargetImgInfo.u4ImgWidth; 
    }

    // stride 
    mTargetImgInfo.u4Stride[0] = (~(mu4StrideAlign[0]-1)) & ((mu4StrideAlign[0]-1) + NSCamShot::queryImgStride(mTargetImgInfo.eImgFmt, u4Width, 0)); 
    mTargetImgInfo.u4Stride[1] = (~(mu4StrideAlign[1]-1)) & ((mu4StrideAlign[1]-1) + NSCamShot::queryImgStride(mTargetImgInfo.eImgFmt, u4Width, 1)); 
    mTargetImgInfo.u4Stride[2] = (~(mu4StrideAlign[2]-1)) & ((mu4StrideAlign[2]-1) + NSCamShot::queryImgStride(mTargetImgInfo.eImgFmt, u4Width, 2)); 

    MY_LOGD("fmt = 0x%x, stride = (%d, %d, %d)", mTargetImgInfo.eImgFmt, mTargetImgInfo.u4Stride[0], mTargetImgInfo.u4Stride[1], mTargetImgInfo.u4Stride[2]); 
    MY_LOGD("(width, height) = (%d, %d)", u4Width, u4Height); 

    ImgBufInfo rTargetImgBuf(ImgInfo(mTargetImgInfo.eImgFmt, u4Width, u4Height), 
                        BufInfo(mTargetBufInfo), mTargetImgInfo.u4Stride); 

    if (mTargetImgInfo.eImgFmt == eImgFmt_JPEG)
    {
        encode(mSrcImgBufInfo, rTargetImgBuf, mTargetImgInfo.rROI, mTargetImgInfo.u4Rotation, mTargetImgInfo.u4Flip, mTargetImgInfo.u4Quality, mTargetImgInfo.u4IsSOI, mu4JpegSize);
    }
    else 
    {
        imgTransform(mSrcImgBufInfo, rTargetImgBuf, mTargetImgInfo.rROI, mTargetImgInfo.u4Rotation, mTargetImgInfo.u4Flip, u4TimeoutMs);    
    }

    FUNCTION_LOG_END;
     //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
executeAsync()
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
SImager::
cancel()
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
SImager::
setTargetBufInfo(BufInfo const &rBufInfo)
{
    //FUNCTION_LOG_START;

    mTargetBufInfo = rBufInfo; 

    MY_LOGD(" Target Buf Info (size, VA, PA, id) = (%d, %x, %x, %d)", 
                 mTargetBufInfo.u4BufSize, mTargetBufInfo.u4BufVA, mTargetBufInfo.u4BufPA, mTargetBufInfo.i4MemID); 

    //FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
setFormat(EImageFormat const eFormat)
{
    //FUNCTION_LOG_START;
    MY_LOGI("format = 0x%x", eFormat); 
    mTargetImgInfo.eImgFmt = eFormat;    
        
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
setFlip(MUINT32 const u4Flip)
{
    //FUNCTION_LOG_START;
    MY_LOGI("u4Flip = 0x%x", u4Flip); 
    mTargetImgInfo.u4Flip = u4Flip; 
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
setROI(Rect const rROI)
{
    //FUNCTION_LOG_START;
    MY_LOGI("roi (x, y, w, h) = (%d, %d, %d, %d)", rROI.x, rROI.y, rROI.w, rROI.h); 
    mTargetImgInfo.rROI = rROI; 
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
setResize(MUINT32 const u4Width, MUINT32 const u4Height)
{
    //FUNCTION_LOG_START;
    MY_LOGI("size (w, h) = (%d, %d)", u4Width, u4Height); 
    mTargetImgInfo.u4ImgWidth = u4Width; 
    mTargetImgInfo.u4ImgHeight = u4Height; 
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
setRotation(MUINT32 const u4Ratation)
{
    //FUNCTION_LOG_START;
    MY_LOGI("rotation = %d", u4Ratation); 
    mTargetImgInfo.u4Rotation = u4Ratation;  

    //FUNCTION_LOG_END;
    //
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
setEncodeParam(MUINT32 const & u4IsSOI, MUINT32 const & u4Quality)
{
    //FUNCTION_LOG_START;
    MY_LOGI("enc param (SOI, Quality) = (%d, %d)", u4IsSOI, u4Quality); 
    mTargetImgInfo.u4IsSOI = u4IsSOI; 
    mTargetImgInfo.u4Quality = u4Quality>95?95:u4Quality; 
    MY_LOGI("enc param (SOI, Quality) = (%d, %d)", u4IsSOI, u4Quality); 
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
encode(
    ImgBufInfo const rSrcBufInfo, 
    ImgBufInfo const rDstBufInfo, 
    Rect const rROI, 
    MUINT32 const u4Rotation, 
    MUINT32 const u4Flip, 
    MUINT32 const u4Quality, 
    MUINT32 const u4IsSOI, 
    MUINT32 &u4EncSize
)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE; 
    //
    IJpegCodec *pJpegCodec = IJpegCodec::createInstance(); 
    ret = pJpegCodec->encode(rSrcBufInfo, rDstBufInfo, rROI, u4Rotation, u4Flip,  u4Quality, u4IsSOI, u4EncSize);     
    pJpegCodec->destroyInstance(); 
    //
    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
SImager::
imgTransform(
    ImgBufInfo const rSrcBufInfo, 
    ImgBufInfo const rDstBufInfo,  
    Rect const rROI, 
    MUINT32 const u4Rotation, 
    MUINT32 const u4Flip,
    MUINT32 const u4TimeoutInMs
)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE; 
    //
    IImageTransform *pImgTransform = IImageTransform::createInstance(); 
    ret =  pImgTransform->execute(rSrcBufInfo, rDstBufInfo, rROI, u4Rotation, u4Flip, u4TimeoutInMs); 
    pImgTransform->destroyInstance();
    //
    FUNCTION_LOG_END;
    return ret ; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL   
SImager::
setStrideAlign(MUINT32 const u4StrideAlign[3])
{
    FUNCTION_LOG_START;
    MY_LOGI("stride align = (%d, %d, %d)", u4StrideAlign[0], u4StrideAlign[1], u4StrideAlign[2]); 
    mu4StrideAlign[0] = u4StrideAlign[0]; 
    mu4StrideAlign[1] = u4StrideAlign[1]; 
    mu4StrideAlign[2] = u4StrideAlign[2]; 

    return MTRUE; 
    FUNCTION_LOG_END;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL  
SImager::  
isSupportedSrcFormat(EImageFormat const eFmt)
{
    EImageFormat eUnSupportFmtList[] = {eImgFmt_BAYER8, eImgFmt_BAYER10, eImgFmt_BAYER12,
                                       eImgFmt_NV21_BLK, eImgFmt_NV12_BLK, eImgFmt_JPEG, 
                                       eImgFmt_RGB565, eImgFmt_RGB888, eImgFmt_ARGB888}; 
    //
    for (MUINT32 i = 0; i < sizeof(eUnSupportFmtList)/sizeof(EImageFormat) ; i++)
    {
        if (eFmt == eUnSupportFmtList[i])
        {
            return MFALSE; 
        }    
    }

    return MTRUE; 
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

