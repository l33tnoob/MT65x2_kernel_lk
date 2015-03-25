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

#include "PanoramaClient.h"

//
using namespace NSCamClient;

//
/******************************************************************************
*
*******************************************************************************/

PanoramaClient*   PanoramaClientObj;
sem_t      PanoramaAddImgDone;
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
#include <fcntl.h>
#include <sys/stat.h>
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
PanoramaClient::
PanoramaClient(int ShotNum)
    : PanoramaNum(ShotNum)
{
    MY_LOGD("+ this(%p) num %d", this,PanoramaNum);
    PanoramaClientObj = this;
}


/******************************************************************************
 *
 ******************************************************************************/
PanoramaClient::
~PanoramaClient()
{
    MY_LOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
PanoramaClient::
allocMem(IMEM_BUF_INFO &memBuf)
{
    if (mpIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error \n");
        return MFALSE;
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error \n");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
PanoramaClient::
deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
PanoramaClient::
init(int bufwidth,int bufheight)
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");

    mPanoramaFrameWidth  = bufwidth;
    mPanoramaFrameHeight = bufheight;

    mPanoramaFrameSize   =(mPanoramaFrameWidth * mPanoramaFrameHeight * 3 / 2);
    mCancel = MFALSE;
    //
    MINT32 const i4SensorDevId = 1;
    mpHal3A = Hal3ABase::createInstance(i4SensorDevId);

    // (1) Create frame buffer buffer
    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        return false;
    }
    MY_LOGD("mPanoramaFrameWidth %d mPanoramaFrameHeight %d mPanoramaFrameSize %d PanoramaNum %d",mPanoramaFrameWidth,mPanoramaFrameHeight,mPanoramaFrameSize,PanoramaNum);
    mpframeBuffer.size =  mPanoramaFrameSize * PanoramaNum;
    if(!(allocMem(mpframeBuffer)))
    {
        mpframeBuffer.size = 0;
        MY_LOGE("[init] mpframeBuffer alloc fail");
        return false;
    }

    // (2) create algorithm object
    mpPanoramaObj = NULL;
    mpPanoramaObj = halAUTORAMABase::createInstance(HAL_AUTORAMA_OBJ_AUTO);
    if ( ! mpPanoramaObj )
    {
        MY_LOGE("[init] mpPanoramaObj==NULL \n");
        return false;
    }

    // (3) Create working buffer buffer
    mpMotionBuffer.size = MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 3;
    if(!(allocMem(mpMotionBuffer)))
    {
        mpMotionBuffer.size = 0;
        MY_LOGE("[init] mpMotionBuffer alloc fail");
        return false;
    }
    MINT32 initBufSize = 0;

    mpPanoramaObj->mHalAutoramaGetWokSize( mPanoramaFrameWidth, mPanoramaFrameHeight, PanoramaNum, initBufSize);
    MY_LOGD("[init] autorama working buffer size %d",initBufSize);
    mpPanoramaWorkingBuf.size = initBufSize;
    if(!(allocMem(mpPanoramaWorkingBuf)))
    {
        mpPanoramaWorkingBuf.size = 0;
        MY_LOGE("[init] mpPanoramaWorkingBuf alloc fail");
        return false;
    }

    // (4) Initial algorithm
    SensorHal* sensor_hal = SensorHal::createInstance();
    int iFOV_horizontal = 50;
    int iFOV_vertical = 50;
    if(sensor_hal) {
        sensor_hal->init();
        sensor_hal->sendCommand(static_cast<halSensorDev_e>(i4SensorDevId)
                                , static_cast<int>(SENSOR_CMD_GET_SENSOR_VIEWANGLE)
                                , (int)&iFOV_horizontal
                                , (int)&iFOV_vertical
                                );
        sensor_hal->uninit();
        sensor_hal->destroyInstance();
    }

    MUINT32 focalLengthInPixel = mPanoramaFrameWidth
                                / (2.0 * tan(iFOV_horizontal/2.0/180.0*M_PI));
    // for debug
    {
    	char value[PROPERTY_VALUE_MAX] = {'\0'};
    	property_get("mediatek.panorama.focal", value, "0");
    	MUINT32 focal = atoi(value);
        if(focal) {
            focalLengthInPixel = focal;
            MY_LOGD("force focal length %d", focalLengthInPixel);
        }
    }

    MY_LOGD("viewnalge (h,v)=(%d,%d) focalLengthInPixel=%d"
            , iFOV_horizontal
            , iFOV_vertical
            , focalLengthInPixel);

    MTKPipeAutoramaEnvInfo mAutoramaInitInData;
    mAutoramaInitInData.SrcImgWidth = mPanoramaFrameWidth ;
    mAutoramaInitInData.SrcImgHeight = mPanoramaFrameHeight;
    mAutoramaInitInData.MaxPanoImgWidth = AUTORAMA_MAX_WIDTH;
    mAutoramaInitInData.WorkingBufAddr = (MUINT32)mpPanoramaWorkingBuf.virtAddr;
    mAutoramaInitInData.WorkingBufSize = initBufSize;
    mAutoramaInitInData.MaxSnapshotNumber = PanoramaNum;
    mAutoramaInitInData.FixAE = 0;
    mAutoramaInitInData.FocalLength = focalLengthInPixel;
    mAutoramaInitInData.GPUWarp = 0;

    MTKPipeMotionEnvInfo mMotionInitInfo;
    MTKPipeMotionTuningPara mMotionTuningPara;
    mMotionInitInfo.WorkingBuffAddr = (MUINT32)mpMotionBuffer.virtAddr;
    mMotionInitInfo.pTuningPara = &mMotionTuningPara;
    mMotionInitInfo.SrcImgWidth = MOTION_MAX_IN_WIDTH;
    mMotionInitInfo.SrcImgHeight = MOTION_MAX_IN_HEIGHT;
    mMotionInitInfo.WorkingBuffSize = MOTION_PIPE_WORKING_BUFFER_SIZE;
    mMotionInitInfo.pTuningPara->OverlapRatio = OVERLAP_RATIO;

    ret = mpPanoramaObj->mHalAutoramaInit(mAutoramaInitInData,mMotionInitInfo);
    if ( ret < 0) {
        MY_LOGE("mHalAutoramaInit Err \n");
        return false;
    }

    // (5) reset member parameter
    mPanoramaaddImgIdx = 0;
    mPanoramaFrameIdx = 0;
    mStitchDir = MTKPIPEAUTORAMA_DIR_NO;

    // (6) thread create
    sem_init(&PanoramaSemThread, 0, 0);
    sem_init(&PanoramamergeDone, 0, 0);
    sem_init(&PanoramaAddImgDone, 0, 0);

    pthread_create(&PanoramaFuncThread, NULL, PanoramathreadFunc, this);

    //
    ret = true;
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PanoramaClient::
uninit()
{
    Mutex::Autolock lock(mLockUninit);

    MY_LOGD("+");

    mpHal3A->destroyInstance();

    if(!(deallocMem(mpframeBuffer)))
    {
        mpframeBuffer.size = 0;
        MY_LOGE("[uninit] mpframeBuffer alloc fail");
        return  MFALSE;
    }
    if(!(deallocMem(mpPanoramaWorkingBuf)))
    {
        mpPanoramaWorkingBuf.size = 0;
        MY_LOGE("[uninit] mpPanoramaWorkingBuf alloc fail");
        return  MFALSE;
    }
    if(!(deallocMem(mpMotionBuffer)))
    {
        mpMotionBuffer.size = 0;
        MY_LOGE("[uninit] mpMotionBuffer alloc fail");
        return  MFALSE;
    }

    if (mpPanoramaObj) {
        mpPanoramaObj->mHalAutoramaUninit();
        mpPanoramaObj->destroyInstance();
        mpPanoramaObj = NULL;
    }

    MY_LOGD("-");
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PanoramaClient::
setImgCallback(ImgDataCallback_t data_cb)
{
    MY_LOGD("(notify_cb)=(%p)", data_cb);
    mDataCb = data_cb;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
PanoramaClient::
stopFeature(int cancel)
{
    MY_LOGD("+");
    bool ret = false;
	  int err;
    MY_LOGD("CAM_CMD_STOP_AUTORAMA, do merge %d mPanoramaaddImgIdx %d PanoramaNum %d", cancel,mPanoramaaddImgIdx,PanoramaNum);
    mCancel = MTRUE;

    sem_post(&PanoramaSemThread);
    pthread_join(PanoramaFuncThread, NULL);
    mpHal3A->enableAELimiterControl(false);
    if(mpPanoramaObj)
    {
        if ((cancel == 1) || (mPanoramaaddImgIdx == PanoramaNum))
        {
            // Do merge

            MY_LOGD("  CAM_CMD_STOP_AUTORAMA: Merge Accidently ");
            err = mHalCamFeatureMerge();
            sem_post(&PanoramamergeDone);
            if (err != NO_ERROR)
            {
                MY_LOGD("  mHalCamFeatureMerge fail");
                return false;
            }
        }
        else
        {
            MY_LOGD("  CAM_CMD_STOP_AUTORAMA: Cancel");
        }
    }
    else
    {
       MY_LOGE("AUTORAMA fail: mhal3DObj is NULL");
    }
    MY_LOGD("-");
    return  true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
PanoramaClient::
mHalCamFeatureAddImg()
{
    MINT32 err = NO_ERROR;
    Mutex::Autolock lock(mLock);
    if (mPanoramaaddImgIdx >= PanoramaNum){
        MY_LOGD("mHalCamPanoramaAddImg mPanoramaaddImgIdx %d PanoramaNum %d", mPanoramaaddImgIdx, PanoramaNum);
        return err;
    }
    if(mCancel)
    {
        MY_LOGD("mHalCamPanoramaAddImg exit mCancel %d", mCancel);
        return err;
    }
    FrameOutputParam_T OutputParam;
    mpHal3A->getRTParams(OutputParam);
    MY_LOGD("Panorama EV %d",OutputParam.i4ExposureValue_x10);
    mpPanoramaObj->gImgEV[mPanoramaaddImgIdx] = OutputParam.i4ExposureValue_x10;
    MY_LOGD("mHalCamPanoramaAddImg(): %d LV %d", mPanoramaaddImgIdx,mpPanoramaObj->gImgEV[mPanoramaaddImgIdx]);

    err = mpPanoramaObj->mHalAutoramaCalcStitch(
                            (void*)(mpframeBuffer.virtAddr +
                            (mPanoramaFrameSize * mPanoramaaddImgIdx)),
                            mpPanoramaObj->gImgEV[mPanoramaaddImgIdx],
                            mStitchDir);

    if ( err != NO_ERROR) {

         MY_LOGD("mHalAutoramaCalcStitch(): ret %d", err);
         return err;
     }

    mPanoramaaddImgIdx++;
    MY_LOGD("mHalCamPanoramaAddImg X");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
PanoramaClient::
mHalCamFeatureMerge()
{
    MY_LOGD("mHalPanoramaMerge");

    MINT32 err = NO_ERROR;
    sem_wait(&PanoramaAddImgDone);
    MY_LOGD(" mHalAutoramaDoStitch");
    err = mpPanoramaObj->mHalAutoramaDoStitch();
    if ( err != NO_ERROR) {
        return err;
    }

    MY_LOGD(" mHalAutoramaGetResult");
    memset((void*)&mpPanoramaResult,0,sizeof(MTKPipeAutoramaResultInfo));
    err = mpPanoramaObj->mHalAutoramaGetResult(&mpPanoramaResult);
    if ( err != NO_ERROR) {
        return err;
    }
    MY_LOGD(" ImgWidth %d ImgHeight %d ImgBufferAddr 0x%x", mpPanoramaResult.ImgWidth, mpPanoramaResult.ImgHeight, mpPanoramaResult.ImgBufferAddr);

    #ifdef debug
    char sourceFiles[80];
    sprintf(sourceFiles, "%s_%dx%d.yuv", "/sdcard/Final", mpPanoramaResult.ImgWidth, mpPanoramaResult.ImgHeight);
    dumpBufToFile((char *) sourceFiles, (MUINT8 *)mpPanoramaResult.ImgBufferAddr , (mpPanoramaResult.ImgWidth * mpPanoramaResult.ImgHeight * 2));
    #endif

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
PanoramaClient::
mHalCamFeatureCompress()
{
    MY_LOGD("[mHalCamFeatureCompress]");

    MINT32 err = NO_ERROR;

    // (1) confirm merge is done; so mutex is not necessary

    sem_wait(&PanoramamergeDone);
    MY_LOGD("get PanoramamergeDone semaphore");

    mDataCb((MVOID*)mpPanoramaResult.ImgBufferAddr,mpPanoramaResult.ImgWidth , mpPanoramaResult.ImgHeight);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID*
PanoramaClient::
PanoramathreadFunc(void *arg)
{
    MY_LOGD("[PanoramathreadFunc] +");

    ::prctl(PR_SET_NAME,"PanoTHREAD", 0, 0, 0);

    // (1) set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );

    // loop for thread until access uninit state
    while(!PanoramaClientObj->mCancel)
    {
        MY_LOGD("[Panorama][PanoramathreadFunc]: wait thread");
        int SemValue;
        sem_getvalue(&PanoramaClientObj->PanoramaSemThread, &SemValue);
        MY_LOGD("Semaphone value: %d", SemValue);
        sem_wait(&PanoramaClientObj->PanoramaSemThread);
        MY_LOGD("get PanoramaSemThread Semaphone");
        MINT32 err = PanoramaClientObj->mHalCamFeatureAddImg();
        if (err != NO_ERROR) {
             MY_LOGD("[mHalCamFeatureAddImg] fail");
        }
        MY_LOGD("[Panorama][PanoramathreadFunc]: after do merge");
    }
    sem_post(&PanoramaAddImgDone);
    MY_LOGD("[PanoramathreadFunc] -");
    return NULL;
}
