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
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MULTISHOT_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MULTISHOT_H_
//
// [CS] +
#include <semaphore.h>
#include "ImageCreateThread.h"

//
#include <mtkcam/v1/camutils/CamInfo.h>
#include <list>
using namespace std;

#include <mtkcam/v1/camutils/IBuffer.h>
#include <mtkcam/v1/camutils/IImgBufQueue.h>
using namespace android;
using namespace MtkCamUtils;
#include <ICaptureBufHandler.h>

// [CS] -
namespace NSCamPipe
{
    class ICamIOPipe;
    class IPostProcPipe; 
};

class ISImager; 

class ResMgrDrv; 
class PipeMgrDrv; 

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

struct FocusValue
{
public: 
	MUINT32 volatile u4ValH;
	MUINT32 volatile u4ValL;

public:
	FocusValue(
		MUINT32 _u4ValH = 0, 
		MUINT32 _u4ValL = 0
	)
		:u4ValH(_u4ValH)
		,u4ValL(_u4ValL)
	{}
};


/*******************************************************************************
*
********************************************************************************/

class CamShotImp;


/*******************************************************************************
*
********************************************************************************/
class MultiShot : public CamShotImp
					, public IImageCreateThreadHandler  // [CS]+
{
public:     ////    Constructor/Destructor.
                    MultiShot(
                        EShotMode const eShotMode,
                        char const*const szCamShotName
                    );

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam, MUINT32 u4ShotCount=0xFFFFFFFF);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam);    
    virtual MBOOL   startOne(SensorParam const  & rSensorParam); 
    virtual MBOOL   startOne(ImgBufInfo const  & rImgBufInfo); 
    virtual MBOOL   stop();

public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam); 
    virtual MBOOL   setJpegParam(JpegParam const & rParam); 

public:     ////    buffer setting. 
    virtual MBOOL   registerImgBufInfo(ECamShotImgBufType, ImgBufInfo const &rImgBuf); 

public:     ////    Info.

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

// [CS]+
public: 	////	image thread handle.
	virtual MBOOL		 onCreateImage();
	virtual MBOOL        onCreateYuvImage();
	virtual MBOOL        onCreateThumbnailImage();
	virtual MBOOL        onCreateJpegImage();
// [CS]-	  


protected: 
    MBOOL     createSensorRawImg(SensorParam const & rSensorParam, ImgBufInfo const & rRawImgBufInfo); 
    MBOOL     createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 const u4Img1Rot, MUINT32 const u4Img1Flip, ImgBufInfo const & rDstImgBufInfo1,  ImgBufInfo const &rDstImgBufInfo2); 
    MBOOL     createYuvRawImg(ImgBufInfo const & rSrcImgBufInfo, Rect const rSrcCropRect, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rDstImgBufInfo);     
    MBOOL     createJpegImg(ImgBufInfo const & rSrcImgBufInfo, JpegParam const & rJpgParm, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rJpgImgBufInfo, MUINT32 & u4JpegSize); 
// [CS]+
	virtual MBOOL		 initImageCreateThread();
	virtual MBOOL		 uninitImageCreateThread();
	virtual MVOID		 updateReadyBuf();
	virtual MVOID		 getReadBuf();
	virtual MVOID		 returnJpegBuf();
	virtual MBOOL		 convertImage(ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, int  rot);
	virtual MBOOL		 createJpegImgSW(ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, MUINT32 & u4JpegSize);
	virtual MBOOL		 YV12ToJpeg(ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, MUINT32 & u4JpegSize, int  rot); 
	
// [CS]-	  

protected:    //// helper function 
    MBOOL     allocMem(IMEM_BUF_INFO & rMemBuf); 
    MBOOL     deallocMem(IMEM_BUF_INFO & rMemBuf); 
    MBOOL     reallocMem(IMEM_BUF_INFO & rMemBuf, MUINT32 const u4Size); 
    MBOOL     allocImgMem(char const* const pszName, EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, IMEM_BUF_INFO & rMem);
    // 
    MVOID     setImageBuf(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, ImgBufInfo & rBuf, IMEM_BUF_INFO & rMem); 
    MVOID     freeShotMem(); 
    //
    ImgBufInfo querySensorRawImgBufInfo(); 
    ImgBufInfo queryYuvRawImgBufInfo(); 
    ImgBufInfo queryJpegImgBufInfo(); 
    ImgBufInfo queryPostViewImgInfo(); 
    ImgBufInfo queryThumbImgBufInfo(); 
    ImgBufInfo queryThumbYuvImgBufInfo(); 
    ImgBufInfo queryThumbTempImgBufInfo(); 
    MVOID     getPictureDimension(MUINT32 & u4Width,  MUINT32 & u4Height); 
    MUINT32    mapScenarioType(EImageFormat const eFmt);     	  
    //
    MBOOL        lock(MUINT32 u4HWScenario, MUINT32 u4PipeType,  MUINT32 const u4TimeOutInMs);
    MBOOL        unlock(MUINT32 u4PipeType);
    

protected:      //// data members
    ISImager *mpISImager; 
    ICamIOPipe    *mpCamIOPipe;
    IMemDrv *mpMemDrv; 
    // 
    MUINT32   mu4DumpFlag; 

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
	//[CS]+
	vector<IMEM_BUF_INFO> 	mRawMem; 
	vector<IMEM_BUF_INFO> 	mYuvMem; 
    vector<IMEM_BUF_INFO> 	mPostViewMem;
    vector<IMEM_BUF_INFO> 	mJpegMem; 
    vector<IMEM_BUF_INFO> 	mThumbnailMem; 
    
    ImgBufInfo mYuvImgBufInfoRead; 		 // used for jpeg_enc to read
    ImgBufInfo mYuvImgBufInfoWrite; 	 // used for pass2 to write
    ImgBufInfo mYuvImgBufInfoReady; 	 // ready for jpeg compressing
    ImgBufInfo mPostViewImgBufInfoRead;  // used for jpeg_enc to read
    ImgBufInfo mPostViewImgBufInfoWrite; // used for pass2 to write
    ImgBufInfo mPostViewImgBufInfoReady; // ready for thumbnail compressing
    ImgBufInfo mJpegImgBufInfoWrite;	 // used for jpeg_enc to write 
    ImgBufInfo mJpegImgBufInfoReady; 	 // ready for callback
    ImgBufInfo mThumbImgBufInfoYuv;		 // used for Thumbnail yuv data
    ImgBufInfo mThumbImgBufInfoWrite;    // used for thumbnail enc to write 
    ImgBufInfo mThumbImgBufInfoReady;    // ready for callback 
    ImgBufInfo mThumbImgBufInfoTemp;

	FocusValue  	mFocusValRead;  	// sync with mYuvImgBufInfoRead and mJpegImgBufInfoWrite
	FocusValue  	mFocusValWrite;	    // sync with mYuvImgBufInfoWrite
	FocusValue 		mFocusValReady;     // sync with mYuvImgBufInfoReady
	FocusValue 		mFocusVal;  		// sync with mJpegImgBufInfoReady

	MBOOL  			   		mbSet3ACapMode;
	MUINT32 volatile		mu4ShotCount;
	MUINT32 volatile		mu4ShotSpeed;
    timeval         		mtvLastJpegStart;    //  the timeval of capture start of last time
	MUINT32 volatile		mu4JpegCount;
	MBOOL volatile	   		mbCancelShot;
	MBOOL volatile	   		mbIsLastShot;

	MBOOL volatile	   		mbJpegSemPost;
	MUINT32 volatile	   	mu4JpegSize; 
	MUINT32 volatile	   	mu4ThumbnailSize; 
	Mutex               	mYuvReadyBufMtx;
    Mutex               	mJpegReadyBufMtx;
    sem_t 					semStartEnd;
    sem_t 					semJpeg;
    sem_t 					semThumbnail;
	
	
	// Thread
    sp<IImageCreateThread>  mpImageCreateThread;
    sp<IImageCreateThread>  mpYuvImageCreateThread;
    sp<IImageCreateThread>  mpThumbnailImageCreateThread;
    sp<IImageCreateThread>  mpJpegImageCreateThread;
	//[CS]-
    PipeMgrDrv   *mpPipeMgrDrv; 
    ResMgrDrv    *mpResMgrDrv; 

};

class MultiShotNcc: public MultiShot
{
public: 
	  ////    Constructor/Destructor.
	                    MultiShotNcc(
	                        EShotMode const eShotMode,
	                        char const*const szCamShotName
	                    );
	 virtual 			~MultiShotNcc();
	 
public: ////    Operations.
    virtual MBOOL   	start(SensorParam const & rSensorParam, MUINT32 u4ShotCount=0xFFFFFFFF);
    virtual MBOOL   	stop();
	virtual MBOOL		initImageCreateThread();
	virtual MBOOL		onCreateImage();
	virtual MVOID		getReadBuf();
	virtual MVOID		returnJpegBuf();
    virtual MBOOL   	sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

};

class MultiShotCc: public MultiShot
{
public: 
	  ////    Constructor/Destructor.
	                    MultiShotCc(
	                        EShotMode const eShotMode,
	                        char const*const szCamShotName
	                    );
	 virtual 			~MultiShotCc();
	 
public: ////    Operations.
    virtual MBOOL   	start(SensorParam const & rSensorParam, MUINT32 u4ShotCount=0xFFFFFFFF);
    virtual MBOOL   	stop();
	virtual MBOOL		initImageCreateThread();
	virtual MBOOL		onCreateImage();
	virtual MVOID		getReadBuf();
	virtual MVOID		returnJpegBuf();
    virtual MBOOL   	sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);
    virtual MVOID   	mapNodeToImageBuf(ImgBufQueNode & rNode, ImgBufInfo & rImgBuf);

public: ////    data members .
	sp<ICaptureBufMgrHandler>			mpCaptureBufMgr;
	CapBufQueNode 						mrNode;

};



////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MULTISHOT_H_

