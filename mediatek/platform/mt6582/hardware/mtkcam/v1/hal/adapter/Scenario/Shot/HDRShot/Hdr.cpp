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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "MyHdr.h"
#include <utils/threads.h>
#include <sys/prctl.h>  // For prctl()/PR_SET_NAME.

#define LOG_TAG "MtkCam/HDRShot"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "Hdr.h"
//
using namespace android;
using namespace NSShot;

#if 1	//kidd for ImpShot
extern "C"
sp<IShot>
createInstance_HdrShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<HdrShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new HdrShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new HdrShot", __FUNCTION__);
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
HdrShot::
onCreate()
{
	FUNCTION_LOG_START;
    bool ret = true;

	FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
HdrShot::
onDestroy()
{
	FUNCTION_LOG_START;
	uninit();
}


/*******************************************************************************
*
*******************************************************************************/
HdrShot::
HdrShot(char const*const pszShotName, uint32_t const u4ShotMode, int32_t const i4OpenId)
  : ImpShot(pszShotName, u4ShotMode, i4OpenId)
  //
  , mu4W_yuv(0)
  , mu4H_yuv(0)
  , mu4W_first(1600)
  , mu4H_first(1200)
  , mu4W_small(0)
  , mu4H_small(0)
  , mu4W_se(0)
  , mu4H_se(0)
  , mu4W_dsmap(0)
  , mu4H_dsmap(0)
  //
  , mpIMemDrv(NULL)
  //
  , mpHdrHal(NULL)
  //
  , OriWeight(NULL)
  , BlurredWeight(NULL)
  //
  , mu4OutputFrameNum(0)
  , mu4TargetTone(0)
  //, mu4RunningNumber(0)
  , mfgIsForceBreak(MFALSE)
  , mHdrState(HDR_STATE_INIT)
  , mHdrRound(1)
  , mHdrRoundTotal(2)
  , mTestMode(0)
  , mTotalBufferSize(0)
  , mTotalKernelBufferSize(0)
  , mTotalUserBufferSize(0)

  , mShutterCBDone(0)
  , mRawCBDone(0)
  , mJpegCBDone(0)
  , mCaptueIndex(0)

  , mNormalJpegThread(NULL)
  , mCaptureIMemThread(NULL)
  , mProcessIMemThread(NULL)
{
	mu4FinalGainDiff[0] = 0;
	mu4FinalGainDiff[1] = 0;

	mpCamExif[0] = NULL;
	mpCamExif[1] = NULL;
	mpCamExif[2] = NULL;

#if 1	//setShotParam() default values
	ShotParam param;
#if HDR_DEBUG_FORCE_SINGLE_RUN
	param.mi4PictureWidth = 1600;
	param.mi4PictureHeight = 1200;
#else
	//param.mi4PictureWidth = 4000;
	//param.mi4PictureHeight = 3000;
	param.mi4PictureWidth = 3264;
	param.mi4PictureHeight = 2448;
#endif
#if HDR_DEBUG_FORCE_ROTATE
	param.mi4Rotation = 90;
#endif
	param.mi4PostviewWidth = 800;
	param.mi4PostviewHeight = 600;
	setShotParam(&param, sizeof(ShotParam));

	mPostviewFormat = eImgFmt_YV12;

	#if 0
	mu4W_yuv = 1280;
	mu4H_yuv = 960;
	#endif
	#if 0	//from EV Bracket
	mu4W_yuv = 4000;
	mu4H_yuv = 3000;
	#endif
	#if 0	//from EV Bracket
	mu4W_yuv = 2048;	//2592;
	mu4H_yuv = 1536;	//1944;
	#endif
	mu4SourceSize = mu4W_yuv * mu4H_yuv * 3/2;	//eImgFmt_I420
	mu4FirstRunSourceSize = mu4W_first * mu4H_first * 3/2;	//eImgFmt_I420

	mPostviewWidth = 800;
	mPostviewHeight = 600;
	mPostviewFormat = eImgFmt_YV12;
#endif

#if 1
	mu4OutputFrameNum = 3;
	#if 1	//ev bracket
	mu4FinalGainDiff[0]	= 2048;
	mu4FinalGainDiff[1]	= 512;
	mu4TargetTone		= 150;
	#endif
#endif

	for(MUINT32 i=0; i<eMaxOutputFrameNum; i++) {
		mpSourceImgBuf[i].virtAddr = NULL;
		mpFirstRunSourceImgBuf[i].virtAddr = NULL;
		mpSmallImgBuf[i].virtAddr = NULL;
		mpSEImgBuf[i].virtAddr = NULL;
		mWeightingBuf[i].virtAddr = NULL;
		mpBlurredWeightMapBuf[i].virtAddr = NULL;
		mpDownSizedWeightMapBuf[i].virtAddr = NULL;
	}
	mpPostviewImgBuf.virtAddr = NULL;
	mpResultImgBuf.virtAddr = NULL;
	mpHdrWorkingBuf.virtAddr = NULL;
	mpMavWorkingBuf.virtAddr = NULL;
	mRawBuf.virtAddr = NULL;
	mNormalJpegBuf.virtAddr = NULL;
	mNormalThumbnailJpegBuf.virtAddr = NULL;
	mHdrJpegBuf.virtAddr = NULL;
	mHdrThumbnailJpegBuf.virtAddr = NULL;
	mBlendingBuf.virtAddr = NULL;

    mYUVBufInfoPort1 = NULL;
    mYUVBufInfoPort2 = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
HdrShot::
~HdrShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::
sendCommand(
    uint32_t const  cmd,
    uint32_t const  arg1,
    uint32_t const  arg2
)
{
	FUNCTION_LOG_START;
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
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2);
    }

    //
	FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::
onCmd_reset()
{
	FUNCTION_LOG_START;
    bool ret = true;

	FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::
onCmd_capture()
{
	FUNCTION_LOG_START;
	bool ret = true;
	CPTLog(Event_HdrShot, CPTFlagStart);

#if (HDR_PROFILE_CAPTURE2)
	MyDbgTimer DbgTmr("capture");
#endif

	//
	sem_init(&mSaveNormalJpegDone, 0, 0);
	sem_init(&mEncodeHdrThumbnailJpegDone, 0, 0);
	//
	updateInfo();

	if(mHdrRoundTotal == 1) {
		ret = ret
			//single run
			&& configureForSingleRun()
			&& EVBracketCapture()
			&& ImageRegistratoin()
			&& WeightingMapGeneration()
			&& Blending()
			;
	} else {
		ret = ret
			//first run
			&& configureForFirstRun()
			&& EVBracketCapture()
			&& ImageRegistratoin()
			&& WeightingMapGeneration()
			&& Blending()
			//second run
			&& configureForSecondRun()
			&& WeightingMapGeneration()
			&& Blending()
			;
	}

#if	HDR_SPEEDUP_JPEG
	//for force break
	if(mNormalJpegThread) {
		pthread_join(mNormalJpegThread, NULL);
	}
#endif

#if	HDR_SPEEDUP_MALLOC
	if(mCaptureIMemThread) {
		pthread_join(mCaptureIMemThread, NULL);
	}
	if(mProcessIMemThread) {
		pthread_join(mProcessIMemThread, NULL);
	}
#endif

	//error handler
	if(!mTestMode)
	{

		if(!mShutterCBDone) {
			MY_ERR("send fake onCB_Shutter");
			mpShotCallback->onCB_Shutter(true,0);
		}

		if(!mRawCBDone) {
			MY_ERR("send fake onCB_RawImage");
			MUINT32	u4ExifHeaderSize = 512;
			MUINT32	u4JpegSize = 512;
			MUINT8	puImageBuffer[1024];

		    mpShotCallback->onCB_RawImage(0
		    							, u4ExifHeaderSize+u4JpegSize
		                                , reinterpret_cast<uint8_t const*>(puImageBuffer)
		                                );

		}

		if(!mJpegCBDone) {
			MY_ERR("send fake onCB_CompressedImage");
			MUINT32	u4ExifHeaderSize = 512;
			MUINT8	puExifHeaderBuf[512];
			MUINT32	u4JpegSize = 512;
			MUINT8 	puJpegBuf[512];
			MUINT32	u4Index = 0;
			MBOOL	bFinal = MTRUE;

		    mpShotCallback->onCB_CompressedImage(0,
	                                     u4JpegSize,
	                                     reinterpret_cast<uint8_t const*>(puJpegBuf),
	                                     u4ExifHeaderSize,	//header size
	                                     puExifHeaderBuf,	//header buf
	                                     u4Index,			//callback index
	                                     bFinal				//final image
	                                     );
		}
	}


	//@TODO list
	//#cancel
	//#multi-thread
	//#fail
	//#document
	//#full frame
	//#replace IMEM_Info by ImgBufInfo
	//#speed up do_DownScaleWeightMap()

lbExit:
	//  ()  HDR finished, clear HDR setting.
    do_HdrSettingClear();
    // Don't know exact time of lbExit in HDR flow, so release all again
    // (there is protection in each release function).
    releaseSourceImgBuf();
    releaseFirstRunSourceImgBuf();
    releaseSmallImgBuf();
    releaseSEImgBuf();
    releaseHdrWorkingBuf();
    releaseOriWeightMapBuf();
    releaseDownSizedWeightMapBuf();
    releaseBlurredWeightMapBuf();
    releasePostviewImgBuf();

    releaseNormalJpegBuf();
    releaseNormalThumbnailJpegBuf();
    releaseHdrJpegBuf();
    releaseHdrThumbnailJpegBuf();

	releaseBlendingBuf();

    #if (HDR_PROFILE_CAPTURE2)
	DbgTmr.print("HdrProfiling2:: HDRFinish Time");
    #endif

	CPTLog(Event_HdrShot, CPTFlagEnd);

	FUNCTION_LOG_END;
	return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
HdrShot::
onCmd_cancel()
{
	FUNCTION_LOG_START;
	mfgIsForceBreak = MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
bool
HdrShot::
setShotParam(void const* pParam, size_t const size)
{
	FUNCTION_LOG_START;
	bool ret = true;

	if(!ImpShot::setShotParam(pParam, size)) {
		MY_ERR("[HDR] HdrShot->setShotParam() fail.");
		ret = false;
	}

	FUNCTION_LOG_END;
	return ret;
}
#endif


#if HDR_USE_THREAD

/*******************************************************************************
*
********************************************************************************/
HdrState_e
HdrShot::GetHdrState(void)
{
	FUNCTION_LOG_START;
    return mHdrState;
}


/*******************************************************************************
*
********************************************************************************/
void
HdrShot::SetHdrState(HdrState_e eHdrState)
{
	FUNCTION_LOG_START;
    mHdrState = eHdrState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID *mHalCamHdrThread(MVOID *arg)
{
	FUNCTION_LOG_START;
	MBOOL   ret = MTRUE;

    ::prctl(PR_SET_NAME,"mHalCamHdrThread", 0, 0, 0);   // Give this thread a name.
    ::pthread_detach(::pthread_self()); // Make this thread releases all its resources when it's finish.

    //
    MINT32  err = 0;     // 0: No error.
    HdrState_e eHdrState;

    //
    MY_DBG("[mHalCamHdrThread] tid: %d.", gettid());
    eHdrState = pHdrObj->GetHdrState();
    while (eHdrState != HDR_STATE_UNINIT)
    {
        ::sem_wait(&semHdrThread);
        eHdrState = pHdrObj->GetHdrState();
        MY_DBG("[mHalCamHdrThread] Got semHdrThread. eHdrState: %d.", eHdrState);

        pHdrObj->mHalCamHdrProc(eHdrState);
        ::sem_post(&semHdrThreadBack);

    }

    ::sem_post(&semHdrThreadEnd);

	MY_DBG("[mHalCamHdrThread] - X. err: %d.", err);
    return NULL;
}


#if 1
/*******************************************************************************
*
********************************************************************************/
MINT32
HdrShot::mHalCamHdrProc(HdrState_e eHdrState)
{
	FUNCTION_LOG_START;
	MBOOL ret = MTRUE;
    MY_DBG("[mHalCamHdrProc] - E. eHdrState: %d.", eHdrState);

    #if (HDR_PROFILE_CAPTURE2)
	MyDbgTimer DbgTmr("mHalCamHdrProc");
    #endif


    //
//    if (!mfdObj)
//    {
//        break;
//    }
//    ///
//    mfdObj->lock();


    switch (eHdrState)
    {
        case HDR_STATE_INIT:
        MY_DBG("[mHalCamHdrProc] HDR_STATE_INIT.");
        break;

        case HDR_STATE_NORMALIZATION:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_NORMALIZATION.");
          ret =
              //  ()  Normalize small images, and put them back to SmallImg[].
              do_Normalization()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_Normalization Time")
                                                    #endif
              ;
        }
        break;

        case HDR_STATE_FEATURE_EXTRACITON:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_FEATURE_EXTRACITON.");
          ret =
              //  ()  Do Feature Extraciton.
              do_FeatureExtraction()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_FeatureExtraction Time")
                                                    #endif
              ;
        }
        break;

        case HDR_STATE_ALIGNMENT:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_ALIGNMENT.");
            ret =
                //  ()  Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
                do_Alignment()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_Alignment Time")
                                                    #endif
                ;
        }
        break;

        case HDR_STATE_BLEND:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_BLEND.");
          ret =
              //  ()  Do Fusion.
              do_Fusion()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_Fusion Time")
                                                    #endif
              ;
        }
        break;

        case HDR_STATE_UNINIT:
        MY_DBG("[mHalCamHdrProc] HDR_STATE_UNINIT.");
        // Do nothing. Later will leave while() and post semHdrThreadEnd to indicate that mHalCamHdrThread is end and is safe to uninit.
        break;

        default:
            MY_DBG("[mHalCamHdrProc] undefined HDR_STATE, do nothing.");

    }


    //
//    if (mfdObj)
//    {
//        mfdObj->unlock();
//    }

    //

    #if (HDR_PROFILE_CAPTURE2)
	DbgTmr.print("HdrProfiling:: mHalCamHdrProc Finish.");
    #endif

	FUNCTION_LOG_END;
    return ret;

}
#endif

#endif  // HDR_USE_THREAD

