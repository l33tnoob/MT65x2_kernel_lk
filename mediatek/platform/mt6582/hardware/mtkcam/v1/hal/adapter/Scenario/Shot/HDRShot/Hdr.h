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
#ifndef _HDR_H_
#define _HDR_H_

#include <mtkcam/common.h>    //workaround for hwstddef.h
#include <mtkcam/common/hw/hwstddef.h> //workaround for _params.h
#include <mtkcam/camshot/_params.h>
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/drv/imem_drv.h>
#include <IShot.h>
#include <ImpShot.h>

#include <mtkcam/featureio/hdr_hal_base.h>

#include <pthread.h>
#include <semaphore.h>


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define JPG_SAVING_OPTIMIZE     1   // Save JPEG while HDR thread are doing things.

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum {
    HDR_STATE_INIT					= 0x0000,
    HDR_STATE_NORMALIZATION			= 0x0001,
    HDR_STATE_FEATURE_EXTRACITON	= 0x0002,
    HDR_STATE_ALIGNMENT				= 0x0003,
    HDR_STATE_BLEND				= 0x0004,
    HDR_STATE_UNINIT				= 0x0800,
} HdrState_e;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class CamExif;

namespace android {
namespace NSShot {
class HdrShot : public ImpShot //kidd? : public ShotBase
{
protected:  ////    Multi-Frame
    enum    { eMaxOutputFrameNum = 3 };

protected:  ////    Resolutions.
    MUINT32         mu4W_yuv;		//  YUV Width	// Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32         mu4H_yuv;		//  YUV Height	// Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32         mu4W_first;		//  YUV Width for first run     // always be 1600
    MUINT32         mu4H_first;		//  YUV Height for first run	// always be 1200
    MUINT32         mu4W_small;		//  Small Image Width	// Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32         mu4H_small;		//  Small Image Height	// Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32         mu4W_se;		//  SW EIS Image Width	// Obtained in requestOtherBufs()\QuerySEImgResolution().
    MUINT32         mu4H_se;		//  SW EIS Image Height	// Obtained in requestOtherBufs()\QuerySEImgResolution().
    MUINT32         mu4W_dsmap;		//  Down-sized Weighting Map Width	// Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_width.
    MUINT32         mu4H_dsmap;		//  Down-sized Weighting Map Height	// Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_height.
    MUINT32         mPostviewWidth;
    MUINT32         mPostviewHeight;
    EImageFormat    mPostviewFormat;

protected:  ////    Thread
    pthread_t       mNormalJpegThread;
    sem_t           mSaveNormalJpegDone;
    sem_t           mEncodeHdrThumbnailJpegDone;

    pthread_t       mSaveJpegThread;
    pthread_t       mCaptureIMemThread;
    pthread_t       mProcessIMemThread;

protected:  ////    Pipes.
    HdrHalBase      *mpHdrHal;
    CamExif         *mpCamExif[3];

protected:  ////    Buffers.
    IMemDrv *mpIMemDrv;
    MUINT32         mTotalBufferSize;
    MUINT32         mTotalKernelBufferSize;
    MUINT32         mTotalUserBufferSize;

	//@TODO use ImgBufInfo to replace IMEMINFO
	IMEM_BUF_INFO   mpSourceImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SourceSize;	// Source Image Size.

	IMEM_BUF_INFO   mpFirstRunSourceImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4FirstRunSourceSize;	// First Run Source Image Size.

	IMEM_BUF_INFO   mpSmallImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SmallImgSize;	// Small Image Size.

	IMEM_BUF_INFO   mpSEImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SEImgSize;	// SW EIS Image Size.

    IMEM_BUF_INFO   mWeightingBuf[eMaxOutputFrameNum];
    MUINT32         mWeightingBufSize;

    IMEM_BUF_INFO   mpBlurredWeightMapBuf[eMaxOutputFrameNum];
    MUINT32         muBlurredWeightMapSize;	// Blurred Weighting Map Size.

	IMEM_BUF_INFO   mpDownSizedWeightMapBuf[eMaxOutputFrameNum];
    MUINT32         mu4DownSizedWeightMapSize;	// Down-sized Weighting Map Size.

	IMEM_BUF_INFO   mpPostviewImgBuf;
    MUINT32         mu4PostviewImgSize;	// First Run HDR Result Image Size.

	IMEM_BUF_INFO   mpResultImgBuf;
    MUINT32         mu4ResultImgSize;	// HDR Result Image Size.

	IMEM_BUF_INFO   mpHdrWorkingBuf;
    MUINT32         mu4HdrWorkingBufSize;	// HDR Working Buf Size.

	IMEM_BUF_INFO   mpMavWorkingBuf;
    MUINT32         mu4MavWorkingBufSize;	// MAV Working Buf Size.

    IMEM_BUF_INFO   mRawBuf;
    MUINT32         mu4RawBufSize;	// Raw Image Size.

    IMEM_BUF_INFO   mNormalJpegBuf;
    MUINT32         mNormalJpegBufSize;

    IMEM_BUF_INFO   mNormalThumbnailJpegBuf;
    MUINT32         mNormalThumbnailJpegBufSize;

    IMEM_BUF_INFO   mHdrJpegBuf;
    MUINT32         mHdrJpegBufSize;

    IMEM_BUF_INFO   mHdrThumbnailJpegBuf;
    MUINT32         mHdrThumbnailJpegBufSize;

    IMEM_BUF_INFO   mBlendingBuf;
    MUINT32         mBlendingBufSize;

	HDR_PIPE_SET_BMAP_INFO mHdrSetBmapInfo;
    HDR_PIPE_WEIGHT_TBL_INFO** OriWeight;
	HDR_PIPE_WEIGHT_TBL_INFO** BlurredWeight;

    ImgBufInfo*     mYUVBufInfoPort1;
    ImgBufInfo*     mYUVBufInfoPort2;

protected:  ////    Parameters.
	static MUINT32  mu4RunningNumber;		// A serial number for file saving. For debug.

	MUINT32			mu4OutputFrameNum;		// Output frame number (2 or 3).	// Do not use mu4OutputFrameNum in code directly, use OutputFrameNumGet() instead.

	MUINT32			mu4FinalGainDiff[2];
	MUINT32			mu4TargetTone;


	HDR_PIPE_HDR_RESULT_STRUCT mrHdrCroppedResult;

	volatile MUINT32    mfgIsForceBreak;		// A flag to indicate whether a cancel capture signal is sent.

	HdrState_e		mHdrState;

    MUINT32         mHdrRound;
    MUINT32         mHdrRoundTotal;

    MBOOL           mShutterCBDone;
    MBOOL           mRawCBDone;
    MBOOL           mJpegCBDone;

    int             mCapturePolicy;
    int             mCapturePriority;

    MUINT32         mCaptueIndex;

public:     ////    for development.
    MUINT32         mTestMode;
    MUINT32         mDebugMode;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//protected:  ////    JPEG
//    virtual MUINT32 getJpgEncInAddr() const { return mu4RawDecAddr; }
//    virtual MUINT32 getJpgEncInSize() const { return mu4RawDecSize; }

public:     ////    Attributes.
    inline MUINT32	OutputFrameNumGet() const { return /*eMaxOutputFrameNum*/ mu4OutputFrameNum ;}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//kidd: remove this block
public:     ////    Interfaces.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////                    Instantiation.
    virtual         ~HdrShot();
                    HdrShot(char const*const pszShotName
                            , uint32_t const u4ShotMode
                            , int32_t const i4OpenId
                            );

public:     ////                    Operations.

    //  This function is invoked when this object is firstly created.
    //  All resources can be allocated here.
    virtual bool    onCreate();

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void    onDestroy();

    virtual bool    sendCommand(uint32_t const  cmd
                                , uint32_t const  arg1
                                , uint32_t const  arg2
                                );
    virtual bool    setShotParam(void const* pParam, size_t const size);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool    onCmd_reset();
    virtual bool    onCmd_capture();
    virtual void    onCmd_cancel();

protected:  ////                    callbacks
    static MBOOL    fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    static MBOOL    fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg);

protected:
    //virtual MBOOL   handleBayerData(MUINT8* const puBuf, MUINT32 const u4Size);
    virtual MBOOL   handleYuvData(MUINT8* const puBuf, MUINT32 const u4Size);
    virtual MBOOL   handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size);
    virtual MBOOL   handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT32 const u4Index, MBOOL bFinal);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Buffers.
    static  MVOID*  allocateCaptureMemoryTask_First(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_Others(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_All(MVOID* arg);
    static  MVOID*  allocateProcessMemoryTask(MVOID* arg);

    virtual MBOOL   requestSourceImgBuf(void);
    virtual MBOOL   releaseSourceImgBuf(void);
    virtual MBOOL   requestFirstRunSourceImgBuf(void);
    virtual MBOOL   releaseFirstRunSourceImgBuf(void);
    virtual MBOOL   requestSmallImgBuf(void);
    virtual MBOOL   releaseSmallImgBuf(void);
    virtual MBOOL   requestSEImgBuf(void);
    virtual MBOOL   releaseSEImgBuf(void);
    virtual MBOOL   requestHdrWorkingBuf(void);
    virtual MBOOL   releaseHdrWorkingBuf(void);
    virtual MBOOL   requestOriWeightMapBuf(void);
    virtual MBOOL   releaseOriWeightMapBuf(void);
    virtual MBOOL   requestBlurredWeightMapBuf(void);
    virtual MBOOL   releaseBlurredWeightMapBuf(void);
    virtual MBOOL   requestDownSizedWeightMapBuf(void);
    virtual MBOOL   releaseDownSizedWeightMapBuf(void);
    virtual MBOOL   requestPostviewImgBuf(void);
    virtual MBOOL   releasePostviewImgBuf(void);

    virtual MBOOL   requestNormalJpegBuf(void);
    virtual MBOOL   releaseNormalJpegBuf(void);
    virtual MBOOL   requestNormalThumbnailJpegBuf(void);
    virtual MBOOL   releaseNormalThumbnailJpegBuf(void);
    virtual MBOOL   requestHdrJpegBuf(void);
    virtual MBOOL   releaseHdrJpegBuf(void);
    virtual MBOOL   requestHdrThumbnailJpegBuf(void);
    virtual MBOOL   releaseHdrThumbnailJpegBuf(void);

    virtual MBOOL   requestBlendingBuf(void);
    virtual MBOOL   releaseBlendingBuf(void);

    virtual MBOOL   updateYUVBufferAddress(void);

//protected:  ////    CDP.
public:  ////    CDP.
    virtual MBOOL   update3AExif(CamExif *pCamExif);
    virtual MBOOL   updateThumbnailExif(CamExif *pCamExif, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT8* puExifBuf, MUINT32 &u4FinalExifSize);

    virtual MBOOL   CDPResize(IMEM_BUF_INFO* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srcFormat, IMEM_BUF_INFO* desAdr, MUINT32 desWidth, MUINT32 desHeight, EImageFormat dstFormat, MUINT32 rotate);
    static  MBOOL   CDPResize_simple(IMEM_BUF_INFO* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srcFormat, IMEM_BUF_INFO* desAdr, MUINT32 desWidth, MUINT32 desHeight, EImageFormat dstFormat, MUINT32 rotate);
    static  MBOOL   GetStride(MUINT32 srcWidth, EImageFormat srcFormat, MUINT32 *pStride);
    static  MUINT32 getAlignedSize(MUINT32 const u4Size);

protected:  ////    Save.
    virtual MBOOL   touchVirtualMemory(MUINT8* vm, MUINT32 size);
    static unsigned int    dumpToFile(char const *fname, unsigned char *pbuf, unsigned int size);
    virtual MUINT32 allocMem(IMEM_BUF_INFO *memBuf);
    virtual MUINT32 allocMem_User(IMEM_BUF_INFO *memBuf, MBOOL touch, MBOOL mapping);
    virtual MUINT32 allocMem_Kernel(IMEM_BUF_INFO *memBuf);
    virtual MBOOL   deallocMem(IMEM_BUF_INFO *memBuf);

protected:  ////    Misc.
    virtual MBOOL   updateInfo();
    virtual MBOOL   decideCaptureMode();

    //flow
    virtual MBOOL	configureForSingleRun(void);
    virtual MBOOL	configureForFirstRun(void);
    virtual MBOOL	configureForSecondRun(void);

    virtual MBOOL	EVBracketCapture(void);
    virtual MBOOL	ImageRegistratoin(void);
    virtual MBOOL	WeightingMapGeneration(void);
    virtual MBOOL	Blending(void);

    //virtual MBOOL	createFullFrame(void);
    //
    virtual MBOOL	createSourceAndSmallImg(void);
    virtual MBOOL	createFirstRunSourceImg(void);
    //
    virtual MBOOL	createSourceAndFirstRunSourceImg(void);
    virtual MBOOL	createSmallImg(void);

    //virtual MBOOL	saveSmallImgForDebug(void);
    virtual MBOOL	createSEImg(void);

    virtual MBOOL   createJpegImg(ImgBufInfo const & rSrcImgBufInfo, NSCamShot::JpegParam const & rJpgParm, MUINT32 const u4Rot, MUINT32 const u4Flip, ImgBufInfo const & rJpgImgBufInfo, MUINT32 & u4JpegSize);

    virtual NSCamHW::ImgBufInfo
                    imem2ImgBuf(IMEM_BUF_INFO imembufinfo
                        , EImageFormat format
						, MUINT32 widht, MUINT32 height);

    //virtual MVOID*  createNormalJpegImgTask(MVOID* arg);
    static  MVOID*  createNormalJpegImgTask(MVOID* arg);
    virtual MVOID*  encodeNormalJpeg(MVOID *arg);
    virtual MVOID*  encodeNormalThumbnailJpeg(MVOID *arg);
    virtual MVOID*  saveNormalJpeg(MVOID *arg);

    //virtual MVOID*  createHdrJpegImgMain(MVOID* arg);
    static  MVOID*  saveFileTask(MVOID* arg);
    static  MVOID*  createHdrJpegImgTask(MVOID* arg);
    virtual MVOID*  encodeHdrJpeg(MVOID *arg);
    virtual MVOID*  encodeHdrThumbnailJpeg(MVOID *arg);
    virtual MVOID*  saveHdrJpeg(MVOID *arg);

    virtual MBOOL	do_Normalization(void);
    virtual MBOOL	do_SE(void);
    virtual MBOOL	do_FeatureExtraction(void);
    virtual MBOOL	do_Alignment(void);
    virtual MBOOL	do_OriWeightMapGet(void);
    virtual MBOOL   do_SetBmapBuffer(void);

    virtual MBOOL	do_DownScaleWeightMap(void);
    virtual MBOOL	do_UpScaleWeightMap(void);
    virtual MBOOL	do_Fusion(void);
    virtual MBOOL	do_HdrCroppedResultGet(void);
    virtual MBOOL	do_CroppedPostviewResize(void);
    virtual MBOOL	do_CroppedResultResize(void);   //@deprecated

    virtual MBOOL	do_HdrSettingClear(void);
    virtual MBOOL	saveSourceJpg(void);
    virtual MBOOL   do_SecondRound(void);

public:		////    Thread.
    virtual HdrState_e	GetHdrState(void);
    virtual void	SetHdrState(HdrState_e eHdrState);
    virtual MINT32	mHalCamHdrProc(HdrState_e eHdrState);
    static  MBOOL   SetThreadProp(int policy, int priority);
    static  MBOOL   GetThreadProp(int *policy, int *priority);
};


}; // namespace NSShot
}; // namespace android
#endif  //  _HDR_H_

