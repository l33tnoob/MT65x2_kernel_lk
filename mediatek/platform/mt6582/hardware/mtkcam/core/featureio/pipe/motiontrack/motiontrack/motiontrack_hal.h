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
** $Log: motiontrack_hal.h $
 *
*/

#ifndef _MOTIONTRACK_HAL_H_
#define _MOTIONTRACK_HAL_H_

#include <mtkcam/featureio/motiontrack_hal_base.h>

/*******************************************************************************
*
********************************************************************************/
class halMOTIONTRACK: public halMOTIONTRACKBase 
{
public:
    //
    static halMOTIONTRACKBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////                       
    halMOTIONTRACK(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halMOTIONTRACK();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackInit () -
    //! \brief init MotionTrack 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackInit(MTKPipeMotionTrackEnvInfo MotionTrackInitEnvInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackUninit () -
    //! \brief MotionTrack uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackUninit();
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackGetWorkSize () -
    //! \brief get working buffer size 
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalMotionTrackGetWorkSize(MTKPipeMotionTrackWorkBufInfo *MotionTrackWorkBufInfo);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackSetWorkBuf () -
    //! \brief set working buffer
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalMotionTrackSetWorkBuf(MTKPipeMotionTrackWorkBufInfo MotionTrackWorkBufInfo);
       
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackAddImage () -
    //! \brief add image
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackAddImage(MTKPipeMotionTrackAddImageInfo MotionTrackAddImageInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackGetIntermediateDataSize () -
    //! \brief get size of intermediate data
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackGetIntermediateDataSize(MTKPipeMotionTrackIntermediateData *MotionTrackIntermediateData);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackGetIntermediateData () -
    //! \brief get intermediate data
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackGetIntermediateData(MTKPipeMotionTrackIntermediateData MotionTrackIntermediateData);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackSelectImage () -
    //! \brief select candidate images
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackSelectImage(MTKPipeMotionTrackSelectImageInfo *MotionTrackSelectImageInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMotionTrackBlendImage () -
    //! \brief blend images
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMotionTrackBlendImage(MTKPipeMotionTrackBlendImageInfo MotionTrackBlendImageInfo, MTKPipeMotionTrackResultImageInfo *MotionTrackResultImageInfo);
    
protected:


private:
    MTKMfbmm* mpMTKMotionTrackObj;
};

#endif

