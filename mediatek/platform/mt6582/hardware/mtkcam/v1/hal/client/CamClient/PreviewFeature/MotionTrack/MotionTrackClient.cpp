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
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>

//
using namespace NSCamClient;

//
/******************************************************************************
*
*******************************************************************************/

MotionTrackClient*   MotionTrackClientObj; 
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
//#define MOTIONTRACK_DEBUG 
#ifdef MOTIONTRACK_DEBUG
#include <fcntl.h>
#include <sys/stat.h>
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

/******************************************************************************
 *
 ******************************************************************************/
MotionTrackClient::
MotionTrackClient(int ShotNum)
    : MotionTrackNum(ShotNum)
{
    MY_LOGD("+ this(%p) num %d", this,MotionTrackNum);
    MotionTrackClientObj = this;
}


/******************************************************************************
 *
 ******************************************************************************/
MotionTrackClient::
~MotionTrackClient()
{
    MY_LOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
MotionTrackClient::
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
MotionTrackClient::
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
MotionTrackClient::
init(int bufwidth,int bufheight)
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    
    mMotionTrackFrameWidth  = bufwidth;
    mMotionTrackFrameHeight = bufheight;
    mMotionTrackFrameSize   =(mMotionTrackFrameWidth * mMotionTrackFrameHeight * 3 / 2);
    mMotionTrackThumbSize   =((mMotionTrackFrameWidth/8) * (mMotionTrackFrameHeight/8) * 3 / 2);
    mPreviewFrameCount = 0;
	mNumBlendImages = 0;
    mCancel = MFALSE;
    //
    MINT32 const i4SensorDevId = 1;

    // (1) Sensor
    mpSensor = SensorHal::createInstance();    
    if (mpSensor == NULL)
    {
        MY_LOGE("Init sensor fail!!");
        return false;
    }
    mpSensor->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_DEV);
    mpSensor->init();
    uint32_t  u4TgInW = 0; 
    uint32_t  u4TgInH = 0;
    mpSensor->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_PRV_RANGE, (int32_t)&u4TgInW, (uint32_t)&u4TgInH);
    if(u4TgInW == 0 || u4TgInH == 0)
    {
        MY_LOGE("Wrong sensor preview range!!");
        return false;
    }
    MY_LOGD("Sensor RAW width %d height %d", u4TgInW, u4TgInH);

    // (2) EIS
    mpEisHal = EisHalBase::createInstance("mtkdefaultAdapter");
    if(mpEisHal != NULL)
    {
        eisHal_config_t eisHalConfig;
        eisHalConfig.imageWidth = u4TgInW;
        eisHalConfig.imageHeight = u4TgInH;
        mpEisHal->configEIS(
                    NSHwScenario::eHW_VSS,
                    eisHalConfig);
        mSensorRawWidth = u4TgInW;
        mSensorRawHeight = u4TgInH;
    }
    else
    {
        MY_LOGE("Create EisHal fail!!");
        return false;
    }
        
    NSCamHW::Rect rHWSrc(0, 0, mSensorRawWidth, mSensorRawHeight); 
    NSCamHW::Rect rHWDst(0, 0, mMotionTrackFrameWidth, mMotionTrackFrameHeight); 
    NSCamHW::Rect rHWCrop = MtkCamUtils::calCrop(rHWSrc, rHWDst, 100); 
    mPreviewCropWidth = rHWCrop.w;
    mPreviewCropHeight = rHWCrop.h;
    MY_LOGD("Preview crop width %d height %d", mPreviewCropWidth, mPreviewCropHeight);
        
    // (1) Create frame buffer buffer  
    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        return false;
    }
    MY_LOGD("mMotionTrackFrameWidth %d mMotionTrackFrameHeight %d mMotionTrackFrameSize %d mMotionTrackThumbSize %d MotionTrackNum %d",mMotionTrackFrameWidth,mMotionTrackFrameHeight,mMotionTrackFrameSize,mMotionTrackThumbSize,MotionTrackNum);                                      
    mpFrameBuffer.size =  mMotionTrackFrameSize * MotionTrackNum;    
    if(!(allocMem(mpFrameBuffer)))
    {
        mpFrameBuffer.size = 0;
        MY_LOGE("[init] mpFrameBuffer alloc fail");
        return false;
    }                     
    
    // (2) create algorithm object
    mpMotionTrackObj = NULL;
    mpMotionTrackObj = halMOTIONTRACKBase::createInstance();
    if ( ! mpMotionTrackObj )
    {
        MY_LOGE("[init] mpMotionTrackObj==NULL \n");
        return false;
    }       
    
    // (3) Initial algorithm
    MTKPipeMotionTrackEnvInfo mMotionTrackEnvInfo; 
    mMotionTrackEnvInfo.SrcImgWidth = mMotionTrackFrameWidth ;
    mMotionTrackEnvInfo.SrcImgHeight = mMotionTrackFrameHeight;
    
    ret = mpMotionTrackObj->mHalMotionTrackInit(mMotionTrackEnvInfo);
    if (!ret) {
        MY_LOGE("mHalMotionTrackInit Err \n");
        return false;
    }
    
    // (4) Create working buffer buffer    
    mpThumbBuffer.size = mMotionTrackThumbSize;
    if(!(allocMem(mpThumbBuffer)))
    {
        mpThumbBuffer.size = 0;
        MY_LOGE("[init] mpThumbBuffer alloc fail");
        return false;
    }
    MINT32 initBufSize = 0;
    
    MTKPipeMotionTrackWorkBufInfo MotionTrackWorkBufInfo;
    ret = mpMotionTrackObj->mHalMotionTrackGetWorkSize(&MotionTrackWorkBufInfo);
    if (!ret) {
        MY_LOGE("mHalMotionTrackGetWorkSize Err \n");
        return false;
    }
    MY_LOGD("[init] algorithm working buffer size %d", MotionTrackWorkBufInfo.WorkBufSize);
    mpMotionTrackWorkingBuf.size =  MotionTrackWorkBufInfo.WorkBufSize;
    if(!(allocMem(mpMotionTrackWorkingBuf)))
    {
        mpMotionTrackWorkingBuf.size = 0;
        MY_LOGE("[init] mpMotionTrackWorkingBuf alloc fail");
        return false;
    }
    MotionTrackWorkBufInfo.WorkBufAddr = (MUINT8*) mpMotionTrackWorkingBuf.virtAddr;
    ret = mpMotionTrackObj->mHalMotionTrackSetWorkBuf(MotionTrackWorkBufInfo);
    if (!ret) {
        MY_LOGE("mHalMotionTrackSetWorkBuf Err \n");
        return false;
    }
    
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.motiontrack.eismethod", value, "1");
    MUINT16 method = atoi(value);
    updateEISMethod(method);
    
    // (5) reset member parameter
    mMotionTrackaddImgIdx = 0;
    mMotionTrackFrameIdx = 0;

    // (6) thread create
    sem_init(&MotionTrackBlendDone, 0, 0);  
    sem_init(&MotionTrackAddImgDone, 0, 0);   
    
    //pthread_create(&MotionTrackFuncThread, NULL, MotionTrackthreadFunc, this);  

    //
    ret = true;
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
MotionTrackClient::
uninit()
{
    Mutex::Autolock lock(mLockUninit);

    MY_LOGD("+");        
    
    if(!(deallocMem(mpFrameBuffer)))
    {
        mpFrameBuffer.size = 0;
        MY_LOGE("[uninit] mpFrameBuffer dealloc fail");
        return  MFALSE;
    }
    if(!(deallocMem(mpMotionTrackWorkingBuf)))
    {
        mpMotionTrackWorkingBuf.size = 0;
        MY_LOGE("[uninit] mpMotionTrackWorkingBuf dealloc fail");
        return  MFALSE;
    }       
    if(!(deallocMem(mpThumbBuffer)))
    {
        mpThumbBuffer.size = 0;
        MY_LOGE("[uninit] mpThumbBuffer dealloc fail");
        return  MFALSE;
    }            

    //(4) stop eis
    if(mpEisHal != NULL)
    {
        // mpEisHal->stopAccumulationMode();
        mpEisHal->destroyInstance("mtkdefaultAdapter");
        mpEisHal = NULL;
    }

    if(mpSensor)
    {
        mpSensor->uninit();
        mpSensor->destroyInstance();
    }
    
    if (mpMotionTrackObj) {
        mpMotionTrackObj->mHalMotionTrackUninit();
        mpMotionTrackObj->destroyInstance();
        mpMotionTrackObj = NULL;
    }

    MY_LOGD("-");
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID  
MotionTrackClient:: 
setImgCallback(ImgDataCallback_t data_cb)
{
    MY_LOGD("(notify_cb)=(%p)", data_cb);
    mDataCb = data_cb;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MotionTrackClient::
stopFeature(int cancel)
{
    MY_LOGD("+");
    bool ret = false;
	  int err;
    MY_LOGD("CAM_CMD_STOP_MOTIONTRACK%s, mMotionTrackaddImgIdx %d MotionTrackNum %d", 
                    (cancel == true)? " (cancel)": "",mMotionTrackaddImgIdx, MotionTrackNum);
    mCancel = MTRUE;
    
    /*
    sem_post(&MotionTrackSemThread);
    pthread_join(MotionTrackFuncThread, NULL);
    */
    if(mpMotionTrackObj)
    {
        if ((cancel == true) || (mMotionTrackaddImgIdx > 1)) 
        {
            // Do merge 
            MY_LOGD("  CAM_CMD_STOP_MOTIONTRACK: Merge Accidently ");	
            if (mHalCamFeatureBlend() != true)
            {   
                MY_LOGD("  mHalCamFeatureBlend fail");             
            }         
        }         
        else 
        {
            MY_LOGD("  CAM_CMD_STOP_MOTIONTRACK: Cancel");	
        }
    }
    else
    {
       MY_LOGE("CAM_CMD_STOP_MOTIONTRACK fail: mpMotionTrackObj is NULL");            
    }
    sem_post(&MotionTrackBlendDone);

    MY_LOGD("-");
    return  true;    
}

static MINT32
ConvertYV12toNV21(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr)
{
    bool ret = true;
    MY_LOGD("[ConvertYV12toNV21] +");
    DpBlitStream Motionstream;

    //***************************src YV12********************************//
    int src_ysize = ImgWidth * ImgHeight;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;
    unsigned int src_addr_list[3];
    unsigned int src_size_list[3];
    int plane_num = 3;
    src_addr_list[0] = (unsigned int)srcbufadr;
    src_addr_list[1] = (unsigned int)(srcbufadr + src_ysize);
    src_addr_list[2] = (unsigned int)(srcbufadr + src_ysize + src_usize);

    src_size_list[0] = src_ysize;
    src_size_list[1] = src_vsize;
    src_size_list[2] = src_usize;
    Motionstream.setSrcBuffer((void **)src_addr_list, src_size_list, plane_num);
    Motionstream.setSrcConfig(ImgWidth, ImgHeight, DP_COLOR_YV12);

    //***************************dst NV21********************************//
    int dst_ysize = ImgWidth * ImgHeight;
    int dst_uvsize;
    dst_uvsize = dst_ysize / 2;
    unsigned int dst_addr_list[2];
    unsigned int dst_size_list[2];
    plane_num = 2;
    dst_addr_list[0] = (unsigned int)dstbufadr;
    dst_addr_list[1] = (unsigned int)(dstbufadr + dst_ysize);

    dst_size_list[0] = dst_ysize;
    dst_size_list[1] = dst_uvsize;
    //*****************************************************************************//
    Motionstream.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    Motionstream.setDstConfig(ImgWidth,ImgHeight, DP_COLOR_NV21);
    Motionstream.setRotate(0);

    // set & add pipe to stream
    if (Motionstream.invalidate())  //trigger HW
    {
          MY_LOGD("[ConvertYV12toNV21] FDstream invalidate failed");
          return false;
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
MotionTrackClient::
mHalCamFeatureBlend()
{
    //sem_wait(&MotionTrackAddImgDone);
    MY_LOGD("mHalCamFeatureBlend");

    /* Get selected image information */
    MTKPipeMotionTrackSelectImageInfo MotionTrackSelectImageInfo;
    if ((mpMotionTrackObj->mHalMotionTrackSelectImage(&MotionTrackSelectImageInfo) != true) ||
        (MotionTrackSelectImageInfo.NumCandidateImages > MOTIONTRACK_MAX_NUM_OF_BLENDING_IMAGES) ||
        (MotionTrackSelectImageInfo.NumCandidateImages == 0))
    {
        MY_LOGE("mHalCamFeatureBlend: mHalMotionTrackSelectImage failed");            
        return false;
    }
    for (int i = 0; i < MotionTrackSelectImageInfo.NumCandidateImages; i++)
    {
        if (MotionTrackSelectImageInfo.CandidateImageIndex[i] > mMotionTrackaddImgIdx) 
        {
            MY_LOGE("mHalCamFeatureBlend: mHalMotionTrackSelectImage failed");            
            return false;
        }
    }

    /* Get intermediate data */
    MTKPipeMotionTrackIntermediateData MotionTrackIntermediateData;
    if (mpMotionTrackObj->mHalMotionTrackGetIntermediateDataSize(&MotionTrackIntermediateData) != true)
    {
        MY_LOGE("mHalCamFeatureBlend: mHalMotionTrackGetIntermediateDataSize failed");            
        return false;
    }
    IMEM_BUF_INFO mpIntermediateDataBuffer;
    mpIntermediateDataBuffer.size = MotionTrackIntermediateData.DataSize;    
    if(!(allocMem(mpIntermediateDataBuffer)))
    {
        MY_LOGE("mHalCamFeatureBlend: mpIntermediateDataBuffer alloc fail");
        return false;
    }                     
    MotionTrackIntermediateData.DataAddr = (void*) mpIntermediateDataBuffer.virtAddr;
    if (mpMotionTrackObj->mHalMotionTrackGetIntermediateData(MotionTrackIntermediateData) != true)
    {
        MY_LOGE("mHalCamFeatureBlend: mHalMotionTrackGetIntermediateData failed");            
        deallocMem(mpIntermediateDataBuffer);
        return false;
    }
    MUINT32 cbdata[3];
    cbdata[0] = 3;  /* Callback on Intermediate Data */
    cbdata[1] = mpIntermediateDataBuffer.size;
    cbdata[2] = (MUINT32) mpIntermediateDataBuffer.virtAddr;
    mDataCb((MVOID*) cbdata, 0, 0);
    if(!(deallocMem(mpIntermediateDataBuffer)))
    {
        MY_LOGE("mHalCamFeatureBlend: mpIntermediateDataBuffer dealloc failed");            
    }

    /* Start to blend */
    MTKPipeMotionTrackBlendImageInfo MotionTrackBlendImageInfo;
	MotionTrackBlendImageInfo.NumBlendImages = MotionTrackSelectImageInfo.NumCandidateImages;
    mpBlendBuffer.size =  mMotionTrackFrameSize * (MotionTrackBlendImageInfo.NumBlendImages + 1); /* one more buffer for format conversion */
    if(!(allocMem(mpBlendBuffer)))
    {
        mpBlendBuffer.size = 0;
        MY_LOGE("mHalCamFeatureBlend: mpBlendBuffer alloc fail");
        return false;
    }                     
    for (int i = 0; i < MotionTrackBlendImageInfo.NumBlendImages; i++)
    {
        MotionTrackBlendImageInfo.BlendImageIndex[i] = MotionTrackSelectImageInfo.CandidateImageIndex[i];
        MotionTrackBlendImageInfo.SrcImageAddr[i] = (MUINT8*) (mpFrameBuffer.virtAddr + (mMotionTrackFrameSize * MotionTrackSelectImageInfo.CandidateImageIndex[i]));
        MotionTrackBlendImageInfo.ResultImageAddr[i] = (MUINT8*) (mpBlendBuffer.virtAddr + (mMotionTrackFrameSize * (i + 1)));

        #if 0//def MOTIONTRACK_DEBUG
        char sourceFiles[80];
        sprintf(sourceFiles, "%s_%dx%d_%02d.yuv", "/sdcard/slctd", mMotionTrackFrameWidth, mMotionTrackFrameHeight, i);
        dumpBufToFile((char *) sourceFiles, MotionTrackBlendImageInfo.SrcImageAddr[i], mMotionTrackFrameSize);
        #endif
    }
    MTKPipeMotionTrackResultImageInfo MotionTrackResultImageInfo;
    if (mpMotionTrackObj->mHalMotionTrackBlendImage(MotionTrackBlendImageInfo, &MotionTrackResultImageInfo) != true)
    {
        MY_LOGE("mHalCamFeatureBlend: mHalMotionTrackBlendImage failed");            
        deallocMem(mpBlendBuffer);
        return false;
    }	
    mNumBlendImages = MotionTrackBlendImageInfo.NumBlendImages;
    mMotionTrackOutputWidth = MotionTrackResultImageInfo.OutputImgWidth;
    mMotionTrackOutputHeight = MotionTrackResultImageInfo.OutputImgHeight;
    MY_LOGD("mHalCamFeatureBlend: Output width %d, height %d", mMotionTrackOutputWidth, mMotionTrackOutputHeight);
    
    #ifdef MOTIONTRACK_DEBUG
    char sourceFiles[80];
    for (int i = 0; i < mNumBlendImages; i++)
    {
        sprintf(sourceFiles, "%s_%dx%d_%d.yuv", "/sdcard/Blend", mMotionTrackOutputWidth, mMotionTrackOutputHeight, i);
        dumpBufToFile((char *) sourceFiles, (MUINT8*) (mpBlendBuffer.virtAddr + (mMotionTrackFrameSize * (i + 1))), mMotionTrackFrameSize);
    }
    #endif

    /* Convert from YV12 to NV21 */
    for (int i = 0; i < mNumBlendImages; i++)
    {
        ConvertYV12toNV21((MVOID*) (mpBlendBuffer.virtAddr + (mMotionTrackFrameSize * (i + 1))), 
                          mMotionTrackOutputWidth, 
                          mMotionTrackOutputHeight, 
                          (MVOID*) (mpBlendBuffer.virtAddr + (mMotionTrackFrameSize * i)));
    }
    
    return true;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
MotionTrackClient::
mHalCamFeatureCompress()
{
    MY_LOGD("[mHalCamFeatureCompress]");

    MINT32 err = NO_ERROR;
    
    // (1) confirm merge is done; so mutex is not necessary

    sem_wait(&MotionTrackBlendDone);  
    MY_LOGD("get MotionTrackBlendDone semaphore");

    if (mNumBlendImages)
    {
        MUINT32 cbdata[5];
        cbdata[0] = 2;  /* Callback on Blended Images */
        cbdata[2] = mNumBlendImages;
        for (int i = 0; i < mNumBlendImages; i++)
        {
            cbdata[1] = i;
            cbdata[4] = (MUINT32) (mpBlendBuffer.virtAddr + (mMotionTrackFrameSize * i));
            mDataCb((MVOID*) cbdata, mMotionTrackOutputWidth, mMotionTrackOutputHeight);
        }

        if(!(deallocMem(mpBlendBuffer)))
        {
            mpBlendBuffer.size = 0;
            MY_LOGE("mpBlendBuffer dealloc fail");
        }
    }
    else
    {
        /* No blended images */
        MUINT32 cbdata[3];
        cbdata[0] = 2;  /* Callback on Blended Images */
        cbdata[1] = 0;
        cbdata[2] = 0;
        mDataCb((MVOID*) cbdata, mMotionTrackFrameWidth, mMotionTrackFrameHeight);
    }
    
    return err;
}

#if 0
/*******************************************************************************
*
********************************************************************************/
MVOID* 
MotionTrackClient::
MotionTrackthreadFunc(void *arg)
{
    MY_LOGD("[MotionTrackthreadFunc] +");

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
    while(!MotionTrackClientObj->mCancel)
    {           
        MY_LOGD("[MotionTrack][MotionTrackthreadFunc]: wait thread");
        int SemValue;
        sem_getvalue(&MotionTrackClientObj->MotionTrackSemThread, &SemValue);
        MY_LOGD("Semaphone value: %d", SemValue);
        sem_wait(&MotionTrackClientObj->MotionTrackSemThread);
        MY_LOGD("get MotionTrackSemThread Semaphone");
        MINT32 err = MotionTrackClientObj->mHalCamFeatureAddImg();       
        if (err != NO_ERROR) {
             MY_LOGD("[mHalCamFeatureAddImg] fail");
        }   
        MY_LOGD("[MotionTrack][MotionTrackthreadFunc]: after do merge");
    }
    sem_post(&MotionTrackAddImgDone);
    MY_LOGD("[MotionTrackthreadFunc] -");
    return NULL;
}
#endif
