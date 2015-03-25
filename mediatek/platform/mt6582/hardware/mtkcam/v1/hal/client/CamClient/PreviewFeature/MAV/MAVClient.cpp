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
#define LOG_TAG "MtkCam/CamClient/MAVClient"

#include "MAVClient.h"
//
using namespace NSCamClient;

//
/******************************************************************************
*
*******************************************************************************/
MAVClient*  MAVClientObj;
sem_t       MAVAddImgDone;
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
savedataToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
MAVClient::
MAVClient(int ShotNum)
    : MAVnum(ShotNum)    
{
    MY_LOGD("+ this(%p) num %d", this,MAVnum);
    MAVClientObj = this;
    // create algorithm object
    mpMAVObj = hal3DFBase::createInstance(HAL_MAV_OBJ_NORMAL);
    if ( ! mpMAVObj )
    {
        MY_LOGE("[init] mpMAVObj==NULL \n");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MAVClient::
~MAVClient()
{
    MY_LOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
MAVClient::
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
MAVClient::
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
MAVClient::
init(int bufwidth,int bufheight)
{
    bool ret = MTRUE;   // MTRUE: no error. MFALSE: error.
    MINT32 err = NO_ERROR;
    //
    MY_LOGD("+");
    
    mMAVFrameWidth  = bufwidth;
    mMAVFrameHeight = bufheight;
    mMAVFrameSize   = (mMAVFrameWidth * mMAVFrameHeight * 3 / 2);
    mCancel = MTRUE;
    mStop = MFALSE;
    //
    // (1) Create frame buffer buffer  
    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }
    MY_LOGD("mMAVFrameWidth %d mMAVFrameHeight %d mMAVFrameSize %d MAVnum %d",mMAVFrameWidth,mMAVFrameHeight,mMAVFrameSize,MAVnum);                                                                                
    for(int i=0;i<MAVnum;i++)
    {
        mpframeBuffer[i].size =  mMAVFrameSize;    
        if(!(allocMem(mpframeBuffer[i])))
        {
            mpframeBuffer[i].size = 0;
            MY_LOGE("[init] mpframeBuffer alloc fail");
            ret = MFALSE;
            goto lb_Abnormal_Exit;
        }
        MY_LOGD("[init] mpframeBuffer alloc index %d adr 0x%x",i,mpframeBuffer[i].virtAddr);
    }               
    // (2) Create working buffer buffer
    mpMotionBuffer.size = MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 3;
    if(!(allocMem(mpMotionBuffer)))
    {
        mpMotionBuffer.size = 0;
        MY_LOGE("[init] mpMotionBuffer alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }  
    
    mpMAVMotionBuffer.size = MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 3;
    if(!(allocMem(mpMAVMotionBuffer)))
    {
        mpMAVMotionBuffer.size = 0;
        MY_LOGE("[init] mpMAVMotionBuffer alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }
   
    mpWarpBuffer.size = mMAVFrameWidth * mMAVFrameHeight * 2 + 2048;    
    if(!(allocMem(mpWarpBuffer)))
    {
        mpWarpBuffer.size = 0;
        MY_LOGE("[init] mpWarpBuffer alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }        
    
    // (3) initial algorithm
    err = mpMAVObj->mHal3dfInit((void*)mpMAVWorkingBuf.virtAddr, (void*)mpMotionBuffer.virtAddr, (void*)mpWarpBuffer.virtAddr, (void*)mpMAVMotionBuffer.virtAddr);
    if ( err < 0 ) {
        MY_LOGE("mpMAVObj->mHalMavinit() Err");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }
    
    mpMAVObj->mHal3dfGetWokSize(mMAVFrameWidth,mMAVFrameHeight,mpMAVWorkingBuf.size);
    MY_LOGD("[init] working buffer size %d",mpMAVWorkingBuf.size);
    if(mpMAVWorkingBuf.size==0)
        mpMAVWorkingBuf.size = mMAVFrameWidth * mMAVFrameHeight * 4 * 10;    
        
    if(!(allocMem(mpMAVWorkingBuf)))
    {
        mpMAVWorkingBuf.size = 0;
        MY_LOGE("[init] mpMAVWorkingBuf alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    } 
        
    err = mpMAVObj->mHal3dfSetWokBuff((void*)mpMAVWorkingBuf.virtAddr);
    if ( err < 0 ) {
        MY_LOGE("mpMAVObj->mHal3dfSetWokBuff() Err");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }    
    // (4) reset member parameter
    mMAVaddImgIdx = 0;
    mMAVFrameIdx = 0;
    
    // (5) thread create
    sem_init(&MAVSemThread, 0, 0);
    sem_init(&MAVmergeDone, 0, 0); 
    sem_init(&MAVAddImgDone, 0, 0);    
    pthread_create(&MAVFuncThread, NULL, MAVthreadFunc, this);  
    //


lb_Abnormal_Exit:

    MY_LOGD("-. ret: %d.", ret);
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MAVClient::
uninit()
{
    Mutex::Autolock lock(mLockUninit);

    bool ret = MTRUE;   // MTRUE: no error. MFALSE: error.
    MY_LOGD("+");

    for(int i=0;i<MAVnum;i++)
    {
        MY_LOGD("mpframeBuffer free %d adr 0x%x",i,mpframeBuffer[i].virtAddr);
        if(!(deallocMem(mpframeBuffer[i])))
        {
            mpframeBuffer[i].size = 0;
            MY_LOGE("[uninit] mpframeBuffer alloc fail");
            ret = MFALSE;
            goto lb_Abnormal_Exit;
        }
    }
    MY_LOGD("mpframeBuffer free done");
    if(!(deallocMem(mpMAVWorkingBuf)))
    {
        mpMAVWorkingBuf.size = 0;
        MY_LOGE("[uninit] mpMAVWorkingBuf alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }
    MY_LOGD("mpMAVWorkingBuf free done");
    if(!(deallocMem(mpMAVMotionBuffer)))
    {
        mpMAVMotionBuffer.size = 0;
        MY_LOGE("[uninit] mpMAVMotionBuffer alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }
    MY_LOGD("mpMAVMotionBuffer free done");
    if(!(deallocMem(mpMotionBuffer)))
    {
        mpMotionBuffer.size = 0;
        MY_LOGE("[uninit] mpMotionBuffer alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }        
    MY_LOGD("mpMotionBuffer free done");
    if(!(deallocMem(mpWarpBuffer)))
    {
        mpWarpBuffer.size = 0;
        MY_LOGE("[uninit] mpWarpBuffer alloc fail");
        ret = MFALSE;
        goto lb_Abnormal_Exit;
    }
     
    if (mpMAVObj) {
        mpMAVObj->mHal3dfUninit();
        mpMAVObj->destroyInstance();
        mpMAVObj = NULL;
    }

lb_Abnormal_Exit:

    MY_LOGD("-. ret: %d.", ret);
    return  ret;

}

/******************************************************************************
 *
 ******************************************************************************/
MVOID  
MAVClient:: 
setImgCallback(ImgDataCallback_t data_cb)
{
    MY_LOGD("(notify_cb)=(%p)", data_cb);
    mDataCb = data_cb;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MAVClient::
stopFeature(int cancel)
{
    bool ret = MTRUE;   // MTRUE: no error. MFALSE: error.
    int err;    // NO_ERROR (0): no error. Others: error code.
    MY_LOGD("+");


    MY_LOGD("CAM_CMD_STOP_MAV, do merge %d mMAVaddImgIdx %d MAVnum %d", cancel, mMAVaddImgIdx, MAVnum);
    mCancel = cancel;
    mStop = MTRUE;
    sem_post(&MAVSemThread);
    pthread_join(MAVFuncThread, NULL);   
    
    if (mMAVaddImgIdx == MAVnum)
    {
        err = mHalCamFeatureMerge();
        sem_post(&MAVmergeDone);
        if (err != NO_ERROR)
        {
            ret = MFALSE;
            goto lb_Abnormal_Exit;
        }

    }
    
lb_Abnormal_Exit:
    MY_LOGD("-. ret: %d.", ret);
    return  ret;

}

/*******************************************************************************
*
********************************************************************************/
MINT32
MAVClient::
mHalCamFeatureAddImg()
{
    MINT32 err = NO_ERROR;
    
    if (mMAVaddImgIdx >= MAVnum){
        return err;
    }
    
    if(!mCancel)
    {
        MY_LOGD("mHalCamPanoramaAddImg exit mCancel %d", mCancel);
        return err;
    }
    
    MY_LOGD("mHalCamMAVAddImg(): %d", mMAVaddImgIdx);

    MavPipeImageInfo ImageInfo;
    ImageInfo.ImgAddr = mpframeBuffer[mMAVaddImgIdx].virtAddr;
    ImageInfo.Width = mMAVFrameWidth;
    ImageInfo.Height = mMAVFrameHeight;
    ImageInfo.ControlFlow = 0;
    
    ImageInfo.MotionValue[0] = mpMAVResult.ImageInfo[mMAVaddImgIdx].MotionValue[0];
    ImageInfo.MotionValue[1] = mpMAVResult.ImageInfo[mMAVaddImgIdx].MotionValue[1];

    mpMAVResult.ImageInfo[mMAVaddImgIdx].Width = mMAVFrameWidth;
    mpMAVResult.ImageInfo[mMAVaddImgIdx].Height = mMAVFrameHeight;
    mpMAVResult.ImageInfo[mMAVaddImgIdx].ImgAddr = ImageInfo.ImgAddr;

    MY_LOGD("ImgAddr 0x%x, Width %d, Height %d, Motion: %d %d", 
             ImageInfo.ImgAddr, ImageInfo.Width, ImageInfo.Height,
             ImageInfo.MotionValue[0], ImageInfo.MotionValue[1]);

    err = mpMAVObj->mHal3dfAddImg((MavPipeImageInfo*)&ImageInfo);
    if (err != NO_ERROR) {
        MY_LOGE("mHal3dfAddImg Err");
        return err;
    }
     
    mMAVaddImgIdx++;
    MY_LOGD("mHalCamMAVAddImg mMAVaddImgIdx %d MAVnum %d",mMAVaddImgIdx,MAVnum);
    // Do merge
    
    MY_LOGD("mHalCamMAVAddImg X");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
MAVClient::
mHalCamFeatureMerge()
{
    MY_LOGD("mHalCamMAVdoMerge");
    
    MINT32 err = NO_ERROR;
    sem_wait(&MAVAddImgDone);
    MY_LOGD("mHalMavMerge");
    err = mpMAVObj->mHal3dfMerge((MUINT32*)&mpMAVResult);
    if (err != NO_ERROR) {
        MY_LOGE("mHal3dfMerge Err");
        return err;
    }
      
    MavPipeImageInfo ImageInfo;

    ImageInfo.ImgAddr = mpframeBuffer[0].virtAddr;
    ImageInfo.Width = mpMAVResult.ImageInfo[0].Width;
    ImageInfo.Height = mpMAVResult.ImageInfo[0].Height;
    ImageInfo.ClipX = mpMAVResult.ImageInfo[0].ClipX;
    ImageInfo.ClipY = mpMAVResult.ImageInfo[0].ClipY;
    MY_LOGD("[mHalCamMAVMakeMPO] 0x%x w %d h %d cx %d cy %d", ImageInfo.ImgAddr,ImageInfo.Width ,ImageInfo.Height, ImageInfo.ClipX, ImageInfo.ClipY);   
           
    err = mpMAVObj->mHal3dfWarp(&ImageInfo,(MUINT32*)&mpMAVResult, MAVnum);   
    if (err != NO_ERROR) {
        MY_LOGE("mHal3dfWarp Err");
        return err;
    }
       
    MUINT32 result;
    MUINT32 width;
    MUINT32 height;
    err = mpMAVObj->mHal3dfGetResult(result, width , height);
    if ( err < 0 ) 
        return err;
    mpMAVResult.ClipWidth = (MINT16)width;
    mpMAVResult.ClipHeight = (MINT16)height;
    MY_LOGD("mHalMavGetResult result x %d y %d",width , height);
    
 #ifdef debug
    char sourceFiles[80];
    for (MUINT8 i = 0; i < MAVnum; i++)
    {
        sprintf(sourceFiles, "%s%d.raw", "/sdcard/afterwarp", i);
        savedataToFile((char *) sourceFiles, (MUINT8 *)(mpframeBuffer[i].virtAddr), mMAVFrameSize);
    }
#endif
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
MAVClient::
mHalCamFeatureCompress()
{
    MY_LOGD("[mHalCamFeatureCompress]");

    MINT32 err = NO_ERROR;
    
    // (1) confirm merge is done; so mutex is not necessary
    sem_wait(&MAVmergeDone);  
    MY_LOGD("get MAVmergeDone semaphore");
    MY_LOGD("mHalCamFeatureCompress 0x%x 0x%x",mpframeBuffer[0].virtAddr,mpframeBuffer[0]);
    MY_LOGD("mHalCamFeatureCompress 0x%x 0x%x",mpframeBuffer,&mpframeBuffer);
    mDataCb((MVOID*)mpframeBuffer,mpMAVResult.ClipWidth , mpMAVResult.ClipHeight);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID* 
MAVClient::
MAVthreadFunc(void *arg)
{
    MY_LOGD("[MAVthreadFunc] +");

    ::prctl(PR_SET_NAME,"MavTHREAD", 0, 0, 0);

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
    
    int SemValue = 0;
    // loop for thread until access uninit state
    while(((!MAVClientObj->mStop)||(SemValue)))
    {           
        MY_LOGD("Semaphone value: %d", SemValue);
        sem_wait(&MAVClientObj->MAVSemThread);
        MINT32 err = MAVClientObj->mHalCamFeatureAddImg();       
        if (err != NO_ERROR) {
            MY_LOGE("[mHalCamFeatureAddImg] fail");
        } 
        sem_getvalue(&MAVClientObj->MAVSemThread, &SemValue);   
        MY_LOGD("[MAV][MAVthreadFunc]: after do merge SemValue %d",SemValue);
    }
    sem_post(&MAVAddImgDone);
    MY_LOGD("[MAVthreadFunc] -");
    return NULL;
}
