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
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_
//
//

namespace NSCamPipe
{
    class ICamIOPipe;
    class IPostProcPipe; 
    struct PipeNotifyInfo;
};

class ISImager; 

class ResMgrDrv; 
class PipeMgrDrv; 

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/

class CamShotImp;


/*******************************************************************************
*
********************************************************************************/
class SingleShot : public CamShotImp
{
public:     ////    Constructor/Destructor.
                    SingleShot(
                        EShotMode const eShotMode,
                        char const*const szCamShotName
                    );

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam);    
    virtual MBOOL   startOne(SensorParam const  & rSensorParam); 
    virtual MBOOL   startOne(ImgBufInfo const  & rImgBufInfo); 
    virtual MBOOL   startOne(SensorParam const & rSensorParam, ImgBufInfo const & rImgBufInfo);
    virtual MBOOL   stop();

public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam); 
    virtual MBOOL   setJpegParam(JpegParam const & rParam); 

public:     ////    buffer setting. 
    virtual MBOOL   registerImgBufInfo(ECamShotImgBufType, ImgBufInfo const &rImgBuf); 

public:     ////    Info.

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);
protected:  ////    callback
    static  MBOOL   fgPipeNotifyCb(MVOID* user, NSCamPipe::PipeNotifyInfo const msg);

protected: 
    MBOOL     createSensorRawImg(SensorParam const & rSensorParam, ImgBufInfo const & rRawImgBufInfo); 
    MBOOL     createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 const u4Img1Rot, MUINT32 const u4Img1Flip, ImgBufInfo const & rDstImgBufInfo1,  ImgBufInfo const &rDstImgBufInfo2); 
    MBOOL     createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rDstImgBufInfo);     
    MBOOL     createJpegImg(ImgBufInfo const & rSrcImgBufInfo, JpegParam const & rJpgParm, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rJpgImgBufInfo, MUINT32 & u4JpegSize); 
    MBOOL     rotatePostview(ImgBufInfo const & rSrcImgBufInfo, MINT32 const u4Rot, MINT32 const u4Flip, ImgBufInfo const & rDstImgBufInfo );
    MBOOL     checkIfFireFlash(MUINT32 countdown);

protected:    //// helper function 
    MBOOL     allocMem(IMEM_BUF_INFO & rMemBuf); 
    MBOOL     deallocMem(IMEM_BUF_INFO & rMemBuf); 
    MBOOL     reallocMem(IMEM_BUF_INFO & rMemBuf, MUINT32 const u4Size); 
    MBOOL     allocImgMem(char const* const pszName, EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, IMEM_BUF_INFO & rMem);
    // 
    inline    MVOID     setImageBuf(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, ImgBufInfo & rBuf, IMEM_BUF_INFO & rMem); 
    inline    MVOID     freeShotMem(); 
    //
    ImgBufInfo querySensorRawImgBufInfo(); 
    ImgBufInfo queryYuvRawImgBufInfo(); 
    ImgBufInfo queryJpegImgBufInfo(); 
    ImgBufInfo queryPostViewImgInfo(); 
    ImgBufInfo queryPrePostViewImgInfo();//fake orientation 
    ImgBufInfo queryThumbImgBufInfo(); 
    inline    MVOID     getPictureDimension(MUINT32 & u4Width,  MUINT32 & u4Height); 
    //for fake orientation
    inline    MVOID     getPrePostviewDimension(MUINT32 & u4Width,  MUINT32 & u4Height); 
    inline    MUINT32    mapScenarioType(EImageFormat const eFmt);     
    //
    MBOOL        lock(MUINT32 u4HWScenario, MUINT32 u4PipeType,  MUINT32 const u4TimeOutInMs);
    MBOOL        unlock(MUINT32 u4PipeType);
    
private:
    pthread_t mPreAllocMemThreadHandle;

private:
    static void*    _preAllocMemThread(void* arg);	
    MBOOL    preAllocMem(); 
    MBOOL    waitPreAllocMemDone(); 
    MBOOL    _preAllocMem(); 

private:      //// data members
    ISImager *mpISImager; 
    ICamIOPipe    *mpCamIOPipe;
    IMemDrv *mpMemDrv; 
    // 
    MUINT32   mu4DumpFlag; 
    // for fake orientation
    MUINT32   mu4DiffOri;
    ////      parameters
    SensorParam mSensorParam; 
    ShotParam   mShotParam; 
    JpegParam   mJpegParam;     
    ////      buffer 
    ImgBufInfo mRawImgBufInfo; 
    ImgBufInfo mYuvImgBufInfo; 
    ImgBufInfo mPostViewImgBufInfo; 
    ImgBufInfo mJpegImgBufInfo; 
    ImgBufInfo mThumbImgBufInfo; 
    ////
    IMEM_BUF_INFO mRawMem; 
    IMEM_BUF_INFO mYuvMem; 
    IMEM_BUF_INFO mPostViewMem; 
    IMEM_BUF_INFO mPrePostViewMem;//fake orientation 
    IMEM_BUF_INFO mJpegMem; 
    IMEM_BUF_INFO mThumbnailMem; 

    PipeMgrDrv   *mpPipeMgrDrv; 
    ResMgrDrv    *mpResMgrDrv; 

    MINT32       mu4FlashCountDown;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_SINGLESHOT_H_

