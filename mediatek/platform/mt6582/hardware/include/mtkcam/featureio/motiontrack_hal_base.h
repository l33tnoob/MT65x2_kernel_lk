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
/*
** $Log: fd_hal_base.h $
 *
*/

#ifndef _MOTIONTRACK_HAL_BASE_H_
#define _MOTIONTRACK_HAL_BASE_H_

/*******************************************************************************
*
********************************************************************************/

//
//
#define MOTIONTRACK_MAX_NUM_OF_BLENDING_IMAGES 8
struct MTKPipeMotionTrackEnvInfo
{
    MUINT16  SrcImgWidth;                      // input image width
    MUINT16  SrcImgHeight;                     // input image height
};
struct MTKPipeMotionTrackWorkBufInfo
{
    MUINT8*  WorkBufAddr;
    MUINT32  WorkBufSize;
};
struct MTKPipeMotionTrackAddImageInfo
{
    MUINT8   ImageIndex;
    MUINT8*  ThumbImageAddr;
    MUINT32  ThumbImageWidth;
    MUINT32  ThumbImageHeight;
    MUINT32  ThumbImageStrideY;
    MUINT32  ThumbImageStrideUV;
    MINT32   MvX;
    MINT32   MvY;
};
struct MTKPipeMotionTrackIntermediateData
{
    MUINT32  DataSize;
    void*    DataAddr;
};
struct MTKPipeMotionTrackSelectImageInfo
{
    MUINT32  NumCandidateImages;                //candidate number returned;
    MUINT8   CandidateImageIndex[MOTIONTRACK_MAX_NUM_OF_BLENDING_IMAGES];
};
struct MTKPipeMotionTrackBlendImageInfo
{
	MUINT32  NumBlendImages;
    MUINT8   BlendImageIndex[MOTIONTRACK_MAX_NUM_OF_BLENDING_IMAGES];
    MUINT8*  SrcImageAddr[MOTIONTRACK_MAX_NUM_OF_BLENDING_IMAGES];
    MUINT8*  ResultImageAddr[MOTIONTRACK_MAX_NUM_OF_BLENDING_IMAGES];
};
struct MTKPipeMotionTrackResultImageInfo
{
    MUINT16  OutputImgWidth;
    MUINT16  OutputImgHeight;
};
/*******************************************************************************
*
********************************************************************************/
class halMOTIONTRACKBase {
public:
    //
    static halMOTIONTRACKBase* createInstance();
    virtual void      destroyInstance() = 0;
    virtual ~halMOTIONTRACKBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackInit () -
    //! \brief init MotionTrack 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackInit(MTKPipeMotionTrackEnvInfo MotionTrackEnvInfo) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackUninit () -
    //! \brief MotionTrack uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackUninit() = 0;
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackGetWorkSize () -
    //! \brief get working buffer size 
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalMotionTrackGetWorkSize(MTKPipeMotionTrackWorkBufInfo *MotionTrackWorkBufInfo) = 0;
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackSetWorkBuf () -
    //! \brief set working buffer
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalMotionTrackSetWorkBuf(MTKPipeMotionTrackWorkBufInfo MotionTrackWorkBufInfo) = 0;
       
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackAddImage () -
    //! \brief add image
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackAddImage(MTKPipeMotionTrackAddImageInfo MotionTrackAddImageInfo) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackGetIntermediateDataSize () -
    //! \brief get size of intermediate data
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackGetIntermediateDataSize(MTKPipeMotionTrackIntermediateData *MotionTrackIntermediateData) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackSelectImage () -
    //! \brief get intermediate data
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackGetIntermediateData(MTKPipeMotionTrackIntermediateData MotionTrackIntermediateData) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackSelectImage () -
    //! \brief select candidate images
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackSelectImage(MTKPipeMotionTrackSelectImageInfo *MotionTrackSelectImageInfo) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackBlendImage () -
    //! \brief blend images
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackBlendImage(MTKPipeMotionTrackBlendImageInfo MotionTrackBlendImageInfo, MTKPipeMotionTrackResultImageInfo *MotionTrackResultImageInfo) = 0;
};

#endif

