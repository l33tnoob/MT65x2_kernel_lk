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
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_SIMAGER_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_SIMAGER_H_
//
//
#include <mtkcam/common.h>

//
using namespace NSCamHW; 
//

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////



/*******************************************************************************
*
********************************************************************************/
class SImager
{
protected:
    struct TargetImgConfig : public ImgInfo
    {
        MUINT32 u4Flip; 
        Rect rROI; 
        MUINT32 u4Rotation; 
        MUINT32 u4IsSOI; 
        MUINT32 u4Quality; 
        MUINT32 u4Stride[3]; 

        TargetImgConfig(
        )
            : u4Flip(0)
            , rROI()
            , u4Rotation(0)
            , u4IsSOI(0)
            , u4Quality(0)
        {
            u4Stride[0] = u4Stride[1] = u4Stride[2] = 0; 
        }

        TargetImgConfig(
            MUINT32 _u4Flip, 
            Rect    _rROI,
            MUINT32 _u4Rotation, 
            MUINT32 _u4Quality, 
            MUINT32 _u4Stride[3]
        )
            : u4Flip(_u4Flip) 
            , rROI(_rROI.x, _rROI.y, _rROI.w, _rROI.h)
            , u4Rotation(_u4Rotation) 
            , u4Quality(_u4Quality)
        {
            u4Stride[0] = _u4Stride[0]; 
            u4Stride[1] = _u4Stride[1]; 
            u4Stride[2] = _u4Stride[2]; 
        }
    
    }; 

public:     ////    Constructor/Destructor.
    SImager(ImgBufInfo const & rImgBufInfo );
    virtual ~SImager() {}; 

public:     ////    Instantiation.

public:      //// attributes
    virtual MUINT32  getJpegSize() {return mu4JpegSize; }; 
public:     ////    Callback. 
    virtual MVOID    setCallback(SImagerNotifyCallback_t notify_cb, MVOID* user); 

public:     ////    Operations.
    virtual MBOOL    execute(MUINT32 u4TimeoutMs = 0xFFFFFFFF); 
    virtual MBOOL    executeAsync(); 
    virtual MBOOL    cancel(); 

public:     ////    Settings.
    virtual MBOOL   setTargetBufInfo(BufInfo const &rBufInfo); 
    virtual MBOOL   setFormat(EImageFormat const eFormat); 
    virtual MBOOL   setFlip(MUINT32 const u4Flip); 
    virtual MBOOL   setROI(Rect const rROI); 
    virtual MBOOL   setResize(MUINT32 const u4Width, MUINT32 const u4Height); 
    virtual MBOOL   setRotation(MUINT32 const u4Ratation); 
    virtual MBOOL   setEncodeParam(MUINT32 const &u4IsSOI, MUINT32 const &u4Quality); 
    virtual MBOOL   setStrideAlign(MUINT32 const u4StrideAlign[3]); 

public:     ////    Attributes 
    virtual MINT32      getLastErrorCode() const    { return mi4ErrorCode; }

private:   ////
    MINT32              mi4ErrorCode;
    MVOID*              mpCbUser;           //  Callback user.
    SImagerNotifyCallback_t    mNotifyCb;      //  notify callback function pointer.    
private:   //// 
    MUINT32     mu4JpegSize; 
    ImgBufInfo mSrcImgBufInfo; 
    TargetImgConfig mTargetImgInfo; 
    BufInfo mTargetBufInfo; 
    MUINT32     mu4StrideAlign[3]; 

private:     ////
    MBOOL    isSupportedSrcFormat(EImageFormat const eFmt); 
    //
    MBOOL    encode(
                  ImgBufInfo const rSrcBufInfo, 
                  ImgBufInfo const rDstBufInfo, 
                  Rect const rROI, 
                  MUINT32 const u4Rotation, 
                  MUINT32 const u4Flip, 
                  MUINT32 const u4Quality, 
                  MUINT32 const u4IsSOI, 
                  MUINT32 &u4EncSize
              ); 
    // 
    MBOOL    imgTransform(
                 ImgBufInfo const rSrcBufInfo, 
                 ImgBufInfo const rDstBufInfo, 
                 Rect const rROI, 
                 MUINT32 const u4Rotation, 
                 MUINT32 u4Flip,
                 MUINT32 const u4TimeoutInMs
             ); 
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_SIMAGER_H_

