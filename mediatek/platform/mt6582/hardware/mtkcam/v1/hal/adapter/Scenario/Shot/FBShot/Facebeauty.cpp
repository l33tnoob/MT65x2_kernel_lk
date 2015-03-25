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

#define LOG_TAG "MtkCam/FBShot"
//
#include "Facebeauty.h"
#include "../NormalShot/NormalShot.h"
#include <mtkcam/common/camutils/CamFormat.h>
#include <mtkcam/featureio/fd_hal_base.h>
#include <sys/stat.h>
#include <camera/MtkCamera.h>

#define SmallFaceWidthThreshold 40
#define BigFaceWidthThreshold 60
Mhal_facebeauty*     mpFbObj;

//#define Debug_Mode

//Thread
static MVOID* FBCapture(void *arg);
static MVOID* FBUtility(void *arg);
static pthread_t threadFB;
static sem_t semFBthread;
static pthread_t threadUtility;
static sem_t semMemoryDone;
static sem_t semUtilitythread;
static int UtilityStatus = 1;
static sem_t semJPGDone;

MBOOL SaveJpg();

#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/*******************************************************************************
*
*******************************************************************************/
sp<IShot>
createNormalShot(char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId)
{
    sp<IShot>       pShot = NULL;
    sp<NormalShot>  pImpShot = NULL;
    CAM_LOGD("new NormalShot");
    pImpShot = new NormalShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new NormalShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] NormalShot onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] NormalShot new IShot", __FUNCTION__);
        goto lbExit;
    }
lbExit:
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }

    return  pShot;
}

sp<IShot>
createFBShot(char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId,
    MtkCameraFaceMetadata* FaceInfo,
    int32_t const       iSmoothLevel,
    int32_t const       iSkinColor,
    int32_t const       iSharp
)
{
    sp<IShot>       pShot = NULL;
    sp<Mhal_facebeauty>  pImpShot = NULL;
    CAM_LOGD("new FBShot");
    pImpShot = new Mhal_facebeauty(pszShotName,u4ShotMode,i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new FBShot \n", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate(FaceInfo) ) {
        CAM_LOGE("[%s] FBShot onCreate() \n", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] FBShot new IShot \nt", __FUNCTION__);
        goto lbExit;
    }


lbExit:
    //
    int LS = iSmoothLevel;
    int LC = iSkinColor;
    int LW = iSharp;
    //(3)  tuning parameter
    LS+=5;
    LC+=5;
    LW+=5;

    pImpShot->mSmoothLevel = LS;
    pImpShot->mBrightLevel = LC;
    pImpShot->mRuddyLevel = 10-LC;
    pImpShot->mWarpLevel = LW;
    pImpShot->mContrastLevel = 5;

    if(pImpShot->mRuddyLevel<3)
        pImpShot->mRuddyLevel = 3;

    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}
extern "C"
sp<IShot>
createInstance_FaceBeautyShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    int32_t const       iSmoothLevel,
    int32_t const       iSkinColor,
    int32_t const       iSharp
)
{
    MtkCameraFaceMetadata FaceInfo;
    MtkCameraFace FBFaceInfo[15];
    MtkFaceInfo MTKPoseInfo[15];
    FaceInfo.faces=(MtkCameraFace *)FBFaceInfo;
    FaceInfo.posInfo=(MtkFaceInfo *)MTKPoseInfo;
    CPTLog(Event_FcaeBeautyShot, CPTFlagStart);
    // Get FD info
    //Create FD object and get FD result info
    //************************
    halFDBase* fdobj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    fdobj->halFDGetFaceInfo(&FaceInfo);
    fdobj->destroyInstance();
    //FaceInfo.number_of_faces = FdResult_Num;
    CAM_LOGD("[createInstance_FaceBeautyShot] number_of_faces %d ",FaceInfo.number_of_faces);
    CAM_LOGD("[createInstance_FaceBeautyShot] smooth-level=%d, skin-color=%d, sharp=%d", iSmoothLevel, iSkinColor, iSharp);


    //for(int i=0;i< 15;i++)
    //{
    //     FaceInfo.faces[i].rect[0] = FdResult[i].rect[0];
    //     FaceInfo.faces[i].rect[1] = FdResult[i].rect[1];
    //     FaceInfo.faces[i].rect[2] = FdResult[i].rect[2];
    //     FaceInfo.faces[i].rect[3] = FdResult[i].rect[3];
    //     FaceInfo.faces[i].score = FdResult[i].score;
    //     FaceInfo.posInfo[i].rop_dir = FdResult[i].rop_dir;
    //     FaceInfo.posInfo[i].rip_dir = FdResult[i].rip_dir;
    //     CAM_LOGD("[createInstance_FaceBeautyShot] FBFaceInfo index %d left %d top %d right %d button %d pose %d \n",i,FaceInfo.faces[i].rect[0],FaceInfo.faces[i].rect[1],FaceInfo.faces[i].rect[2],FaceInfo.faces[i].rect[3],FaceInfo.posInfo[i].rip_dir);
    //}

    //************************

    #ifdef Debug_Mode
    FaceInfo.faces[0].rect[0] = -350;
    FaceInfo.faces[0].rect[1] = 58;
    FaceInfo.faces[0].rect[2] = 225;
    FaceInfo.faces[0].rect[3] = 825;
    FaceInfo.posInfo[0].rop_dir = 0;
    FaceInfo.posInfo[0].rip_dir = 0;

    FaceInfo.faces[1].rect[0] = -112;
    FaceInfo.faces[1].rect[1] = -708;
    FaceInfo.faces[1].rect[2] = 262;
    FaceInfo.faces[1].rect[3] = -208;
    FaceInfo.posInfo[1].rop_dir = 0;
    FaceInfo.posInfo[1].rip_dir = 0;

    FaceInfo.faces[2].rect[0] = 393;
    FaceInfo.faces[2].rect[1] = -716;
    FaceInfo.faces[2].rect[2] = 875;
    FaceInfo.faces[2].rect[3] = -75;
    FaceInfo.posInfo[2].rop_dir = 0;
    FaceInfo.posInfo[2].rip_dir = 0;

    FaceInfo.faces[3].rect[0] = -825;
    FaceInfo.faces[3].rect[1] = 216;
    FaceInfo.faces[3].rect[2] = -587;
    FaceInfo.faces[3].rect[3] = 533;
    FaceInfo.posInfo[3].rop_dir = 0;
    FaceInfo.posInfo[3].rip_dir = 0;

    FaceInfo.faces[4].rect[0] = 400;
    FaceInfo.faces[4].rect[1] = 150;
    FaceInfo.faces[4].rect[2] = 781;
    FaceInfo.faces[4].rect[3] = 658;
    FaceInfo.posInfo[4].rop_dir = 0;
    FaceInfo.posInfo[4].rip_dir = 3;

    FaceInfo.faces[5].rect[0] = -856;
    FaceInfo.faces[5].rect[1] = -708;
    FaceInfo.faces[5].rect[2] = -518;
    FaceInfo.faces[5].rect[3] = -175;
    FaceInfo.posInfo[5].rop_dir = 0;
    FaceInfo.posInfo[5].rip_dir = 11;

    FaceInfo.number_of_faces = 6;
    #endif
    //
    //  (1.1) new Implementator.
    if(FaceInfo.number_of_faces==0)
    {
        return createNormalShot(pszShotName,u4ShotMode,i4OpenId);
    }
    else
    {
        return createFBShot(pszShotName,u4ShotMode,i4OpenId,&FaceInfo,iSmoothLevel,iSkinColor,iSharp);
    }
}
/*******************************************************************************
*
*******************************************************************************/
Mhal_facebeauty::
Mhal_facebeauty(char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
{
    MY_LOGD("[Mhal_facebeauty] Mhal_facebeauty +");
    halSensorDev_e          meSensorDev;
    if (i4OpenId == 0) {
            meSensorDev = SENSOR_DEV_MAIN;
    }
    else if (i4OpenId == 1) {
            meSensorDev == SENSOR_DEV_SUB;
    }
    else {
            meSensorDev == SENSOR_DEV_NONE;
    }
    SensorHal* sensor = SensorHal::createInstance();
    if(sensor)
        sensor->sendCommand(meSensorDev, SENSOR_CMD_GET_SENSOR_TYPE, (int)&meSensorType);
    else
        MY_LOGE("[Mhal_facebeauty] Can not get sensor object \n");
    MY_LOGD("[Mhal_facebeauty] Mhal_facebeauty meSensorType %d \n",meSensorType);
    sensor->destroyInstance();

    MY_LOGD("[Mhal_facebeauty] Mhal_facebeauty -");
}

/*******************************************************************************
*
*******************************************************************************/
bool
Mhal_facebeauty::
onCreate(MtkCameraFaceMetadata* FaceInfo)
{
    MBOOL   ret = MFALSE;
    MINT32  ec = 0;
    MINT32  g_BufWidth;
    MINT32  g_BufHeight;
    MY_LOGD("[facebeauty init] FBFaceInfo adr 0x%x FBFaceInfo num %d \n",(MUINT32)FaceInfo->faces,FaceInfo->number_of_faces);
    UtilityStatus = 1;
    FBmetadata.faces=(MtkCameraFace *)FBFaceInfo;
    FBmetadata.posInfo=(MtkFaceInfo *)MTKPoseInfo;
    FBmetadata.number_of_faces = FaceInfo->number_of_faces;

    for(int i=0;i<FaceInfo->number_of_faces;i++)
    {
        FBmetadata.faces[i].rect[0] = FaceInfo->faces[i].rect[0];
        FBmetadata.faces[i].rect[1] = FaceInfo->faces[i].rect[1];
        FBmetadata.faces[i].rect[2] = FaceInfo->faces[i].rect[2];
        FBmetadata.faces[i].rect[3] = FaceInfo->faces[i].rect[3];
        FBmetadata.posInfo[i].rop_dir = FaceInfo->posInfo[i].rop_dir;
        FBmetadata.posInfo[i].rip_dir = FaceInfo->posInfo[i].rip_dir;
    }

    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        return 0;
    }

    mpFb = halFACEBEAUTIFYBase::createInstance(HAL_FACEBEAUTY_OBJ_SW);

    if  ( ! mpFb )
    {
        MY_LOGE("[init] NULL mpFb \n");
        goto lbExit;
    }
    mpFbObj = this;
    mpFb->CANCEL = MFALSE;

    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        onDestroy();
    }
    MY_LOGD("[init] rc(%d) \n", ret);
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
void
Mhal_facebeauty::
onDestroy()
{
    MY_LOGD("[uninit] in");

    if  (mpFb)
    {
        mpFb->mHalFacebeautifyUninit();
        mpFb->destroyInstance();
        mpFb = NULL;
    }
    if  (mpIMemDrv)
    {
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }
    mu4W_yuv = 0;
    mu4H_yuv = 0;

    MY_LOGD("[uninit] out");
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
onCmd_capture()
{
    MBOOL   ret = MFALSE;

    MINT32  g_BufWidth;
    MINT32  g_BufHeight;
    if((mShotParam.mi4PostviewWidth*3) == (mShotParam.mi4PostviewHeight*4))
    {
        g_BufWidth = 320;
        g_BufHeight = 240;
    }
    else if((mShotParam.mi4PostviewWidth*9) == (mShotParam.mi4PostviewHeight*16))
    {
        g_BufWidth = 320;
        g_BufHeight = 180;
    }
    else if((mShotParam.mi4PostviewWidth*3) == (mShotParam.mi4PostviewHeight*5))
    {
        g_BufWidth = 320;
        g_BufHeight = 192;
    }
    else
    {
        g_BufWidth = 320;
        if(mShotParam.mi4PostviewWidth != 0)
          g_BufHeight = 320 * mShotParam.mi4PostviewHeight/mShotParam.mi4PostviewWidth;
        else
          g_BufHeight = 180;
    }

    MY_LOGD("[onCmd_capture] Postview %dx%d -> Buf %dx%d\n",mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight, g_BufWidth, g_BufHeight);

    for(int i=0;i<FBmetadata.number_of_faces;i++)
    {
        FBmetadata.faces[i].rect[0] = ((FBmetadata.faces[i].rect[0] + 1000) * g_BufWidth) / 2000;
        FBmetadata.faces[i].rect[1] = ((FBmetadata.faces[i].rect[1] + 1000) * g_BufHeight) / 2000;
        FBmetadata.faces[i].rect[2] = ((FBmetadata.faces[i].rect[2] + 1000) * g_BufWidth) / 2000;
        FBmetadata.faces[i].rect[3] = ((FBmetadata.faces[i].rect[3] + 1000) * g_BufHeight) / 2000;

        int face_size = FBmetadata.faces[i].rect[2] - FBmetadata.faces[i].rect[0];
        if(face_size >= 30)
        {
        	  int zoom_size;
            zoom_size = face_size/15;

            if( (FBmetadata.faces[i].rect[0] - zoom_size >= 0) &&
            	  (FBmetadata.faces[i].rect[1] - zoom_size >= 0) &&
            	  (FBmetadata.faces[i].rect[2] + zoom_size <= g_BufWidth -1) &&
            	  (FBmetadata.faces[i].rect[3] + zoom_size <= g_BufHeight-1))
            {
                 zoom_size = face_size/12;
            		 if( (FBmetadata.faces[i].rect[0] - zoom_size >= 0) &&
            	       (FBmetadata.faces[i].rect[1] - zoom_size >= 0) &&
            	       (FBmetadata.faces[i].rect[2] + zoom_size <= g_BufWidth -1) &&
            	       (FBmetadata.faces[i].rect[3] + zoom_size <= g_BufHeight-1))
            	   {
            	   	    zoom_size = face_size/10;
            		      if( (FBmetadata.faces[i].rect[0] - zoom_size >= 0) &&
            	            (FBmetadata.faces[i].rect[1] - zoom_size >= 0) &&
            	            (FBmetadata.faces[i].rect[2] + zoom_size <= g_BufWidth -1) &&
            	            (FBmetadata.faces[i].rect[3] + zoom_size <= g_BufHeight-1))
            	       {
            	       	    zoom_size = face_size/8;
            	    	      if( (FBmetadata.faces[i].rect[0] - zoom_size >= 0) &&
            	                (FBmetadata.faces[i].rect[1] - zoom_size >= 0) &&
            	                (FBmetadata.faces[i].rect[2] + zoom_size <= g_BufWidth -1) &&
            	                (FBmetadata.faces[i].rect[3] + zoom_size <= g_BufHeight-1))
            	            {
            	    	           zoom_size = face_size/7;
            	    	           if( (FBmetadata.faces[i].rect[0] - zoom_size >= 0) &&
            	                     (FBmetadata.faces[i].rect[1] - zoom_size >= 0) &&
            	                     (FBmetadata.faces[i].rect[2] + zoom_size <= g_BufWidth -1) &&
            	                     (FBmetadata.faces[i].rect[3] + zoom_size <= g_BufHeight-1))
            	                 {
            	    	               ;
            	                 }
            	                 else
            	                 {
            	            		     zoom_size = face_size/8;
            	                 }
            	            }
            	            else
            	            {
            	            		zoom_size = face_size/10;
            	            }
            	       }
            	       else
            	       {
            	       	  zoom_size = face_size/12;
            	       }
            	   }
            	   else
            	   {
            	   	   zoom_size = face_size/15;
            	   }
                 FBmetadata.faces[i].rect[0] -= zoom_size;
                 FBmetadata.faces[i].rect[1] -= zoom_size;
                 FBmetadata.faces[i].rect[2] += zoom_size;
                 FBmetadata.faces[i].rect[3] += zoom_size;
            }



        }
        MY_LOGI("[onCmd_capture] After FBFaceInfo num %d left %d top %d right %d button %d pose %d \n",i,FBmetadata.faces[i].rect[0],FBmetadata.faces[i].rect[1],FBmetadata.faces[i].rect[2],FBmetadata.faces[i].rect[3],MTKPoseInfo[i].rip_dir);
    }

    sem_init(&semMemoryDone, 0, 0);
    sem_init(&semFBthread, 0, 0);
    pthread_create(&threadFB, NULL, FBCapture, NULL);

    sem_init(&semUtilitythread, 0, 0);
    pthread_create(&threadUtility, NULL, FBUtility, NULL);

    sem_wait(&semFBthread);

    ret = createFBJpegImg(mpSource,mu4W_yuv,mu4H_yuv,0);
    if  ( ! ret )
    {
        goto lbExit;
    }

    //------------------ Sava test ----------------//
    #ifdef BanchMark
    char szFileName[100];
    MUINT32 FDInfo[100]={0};
    MUINT32* htable=(MUINT32*)msFaceBeautyResultInfo.PCAHTable;
    int i=0;
    for(i=0;i<FBmetadata.number_of_faces;i++)
    {
       FDInfo[i*4]   = FBmetadata.faces[i].rect[0];
       FDInfo[i*4+1] = FBmetadata.faces[i].rect[1];
       FDInfo[i*4+2] = FBmetadata.faces[i].rect[2]-FBmetadata.faces[i].rect[0];
       FDInfo[i*4+3] = MTKPoseInfo[i].rip_dir;
       MY_LOGI("[FACEINFO] x %d y %d w %d",FBmetadata.faces[i].rect[0],FBmetadata.faces[i].rect[1],FBmetadata.faces[i].rect[2]);
    }
    ::sprintf(szFileName, "/sdcard/DCIM/Camera/%s_H_%d_%d.txt", "FDinfo", *htable,capturecount);
    saveBufToFile(szFileName, (MUINT8*)&FDInfo, 100 * 4);
    MY_LOGI("[FACEINFO] Save File done");
    #endif
    //------------------ Sava test ----------------//

    //  Force to handle done even if there is any error before.
    //to do handleCaptureDone();

    ret = MTRUE;
lbExit:
    releaseBufs();
    pthread_join(threadFB, NULL);
    UtilityStatus = 0;
    sem_post(&semUtilitythread);
    pthread_join(threadUtility, NULL);
    CPTLog(Event_FcaeBeautyShot, CPTFlagEnd);
    CPTLog(Event_FBShot_Utility, CPTFlagEnd);
#if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling:: Done");
#endif
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
onCmd_reset()
{
    MY_LOGD("[onCmd_reset] in");
    MBOOL   ret = MTRUE;
    mpFb->CANCEL = MFALSE;
    //ret = releaseBufs();
    MY_LOGD("[onCmd_reset] out");
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
onCmd_cancel()
{
    MBOOL   ret = MFALSE;
    mpFb->CANCEL = MTRUE;
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
Mhal_facebeauty::
sendCommand(
    uint32_t const  cmd,
    uint32_t const  arg1,
    uint32_t const  arg2
)
{
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
    return ret;
}


/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
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
#ifdef Debug_Mode
/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    CAM_LOGD("opening file [%s] adr 0x%x\n", fname,buf);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        CAM_LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    CAM_LOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            CAM_LOGE("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        readCnt += nr;
        cnt++;
    }
    CAM_LOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}
#endif

/******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
fgCamShotNotifyCb(MVOID* user, CamShotNotifyInfo const msg)
{
    CAM_LOGD("[fgCamShotNotifyCb] + ");
    Mhal_facebeauty *pFBlShot = reinterpret_cast <Mhal_facebeauty *>(user);
    if (NULL != pFBlShot)
    {
        CAM_LOGD("[fgCamShotNotifyCb] call back type %d",msg.msgType);
        if (NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            pFBlShot->mpShotCallback->onCB_Shutter(true, 0);
            CAM_LOGD("[fgCamShotNotifyCb] call back done");
        }
    }
    CAM_LOGD("[fgCamShotNotifyCb] -");
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
handleYuvDataCallback(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGD("[handleYuvDataCallback] + (puBuf, size) = (%p, %d)", puBuf, u4Size);

    #ifdef Debug_Mode
    saveBufToFile("/sdcard/yuv.yuv", puBuf, u4Size);
    #endif

    return 0;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGD("[handlePostViewData] + (puBuf, size) = (%p, %d)", puBuf, u4Size);
    mpShotCallback->onCB_PostviewDisplay(0,
                                         u4Size,
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        );

    MY_LOGD("[handlePostViewData] -");
    return  MTRUE;
    }

/******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT32 const Mode)
{
    MY_LOGD("[handleJpegData] + (puJpgBuf, jpgSize, puThumbBuf, thumbSize, mode ) = (%p, %d, %p, %d, %d)", puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize, Mode);

    MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024];
    MUINT32 u4ExifHeaderSize = 0;
    mpIMemDrv->cacheFlushAll();
    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);
    MY_LOGD("[handleJpegData] (thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)",
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);
    // Jpeg callback
    if(Mode)
    {
        mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize,
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,          //header size
                                         puExifHeaderBuf,           //header buf
                                         0,                         //callback index
                                         false,                     //final image
                                         MTK_CAMERA_MSG_EXT_DATA_FACEBEAUTY
                                         );
    }
    else
    {
        mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize,
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,                       //header size
                                         puExifHeaderBuf,                    //header buf
                                         0,                       //callback index
                                         true                     //final image
                                         );
    }
    MY_LOGD("[handleJpegData] -");

    delete [] puExifHeaderBuf;

    return MTRUE;

}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
fgCamShotDataCb(MVOID* user, CamShotDataInfo const msg)
{
    Mhal_facebeauty *pFBlShot = reinterpret_cast<Mhal_facebeauty *>(user);
    CAM_LOGD("[fgCamShotDataCb] type %d +" ,msg.msgType);
    if (NULL != pFBlShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            pFBlShot->handlePostViewData( msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pFBlShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2,1);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_YUV == msg.msgType)
        {
            pFBlShot->handleYuvDataCallback(msg.puData, msg.u4Size);
        }
    }
    CAM_LOGD("[fgCamShotDataCb] -" );
    return MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createFullFrame(IMEM_BUF_INFO Srcbufinfo)
{
    MBOOL  ret = MTRUE;
    MINT32 err = 0;
    CPTLog(Event_FBShot_createFullFrame, CPTFlagStart);
    MY_LOGD("[createFullFrame] + \n");
    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(eShotMode_FaceBeautyShot, "FaceBeautyshot");
    //
    pSingleShot->init();
    EImageFormat ePostViewFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));

    ImgBufInfo rSrcImgInfo;

    rSrcImgInfo.u4ImgWidth = mu4W_yuv;
    rSrcImgInfo.u4ImgHeight = mu4H_yuv;
    rSrcImgInfo.eImgFmt = eImgFmt_I422;
    rSrcImgInfo.u4Stride[0] = rSrcImgInfo.u4ImgWidth;
    rSrcImgInfo.u4Stride[1] = rSrcImgInfo.u4ImgWidth >> 1;
    rSrcImgInfo.u4Stride[2] = rSrcImgInfo.u4ImgWidth >> 1;
    rSrcImgInfo.u4BufSize = Srcbufinfo.size;
    rSrcImgInfo.u4BufVA = Srcbufinfo.virtAddr;
    rSrcImgInfo.u4BufPA = Srcbufinfo.phyAddr;
    rSrcImgInfo.i4MemID = Srcbufinfo.memID;
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_YUV, rSrcImgInfo);

    ImgBufInfo rPostImgInfo;
    rPostImgInfo.u4ImgWidth = mShotParam.mi4PostviewWidth;
    rPostImgInfo.u4ImgHeight = mShotParam.mi4PostviewHeight;
    rPostImgInfo.eImgFmt = ePostViewFmt;
    rPostImgInfo.u4Stride[0] = rPostImgInfo.u4ImgWidth;
    rPostImgInfo.u4Stride[1] = rPostImgInfo.u4ImgWidth >> 1;
    rPostImgInfo.u4Stride[2] = rPostImgInfo.u4ImgWidth >> 1;
    // using blurimg buffer for reduce buffer size
    rPostImgInfo.u4BufSize = mpBlurImg.size;
    rPostImgInfo.u4BufVA = mpBlurImg.virtAddr;
    rPostImgInfo.u4BufPA = mpBlurImg.phyAddr;
    rPostImgInfo.i4MemID = mpBlurImg.memID;

    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_POSTVIEW, rPostImgInfo);

    //

    pSingleShot->enableDataMsg(ECamShot_DATA_MSG_YUV
                                 //| ECamShot_DATA_MSG_JPEG
                                 );
    pSingleShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_EOF);
    // shot param
    NSCamShot::ShotParam rShotParam(eImgFmt_I422,         //yuv format
                         mShotParam.mi4PictureWidth,      //picutre width
                         mShotParam.mi4PictureHeight,     //picture height
                         0,                               //picture rotation in jpg
                         0,                               //picture flip
                         ePostViewFmt,
                         mShotParam.mi4PostviewWidth,      //postview width
                         mShotParam.mi4PostviewHeight,     //postview height
                         0,                                //postview rotation
                         0,                                //postview flip
                         mShotParam.mu4ZoomRatio           //zoom
                        );

    // jpeg param
    NSCamShot::JpegParam rJpegParam(mJpegParam.mu4JpegQuality,       //Quality
                         MTRUE                            //isSOI
                        );


    // sensor param
        NSCamShot::SensorParam rSensorParam(static_cast<MUINT32>(MtkCamUtils::DevMetaInfo::queryHalSensorDev(getOpenId())),                             //Device ID

#warning [FIXME] workaround for Alta phone capture mode can not work
                             ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio
                             //ACDK_SCENARIO_ID_CAMERA_PREVIEW,         //Scenaio
                             10,                                       //bit depth
                             MFALSE,                                   //bypass delay
                             MFALSE                                   //bypass scenario
                            );
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    pSingleShot->setShotParam(rShotParam);

    //
    pSingleShot->setJpegParam(rJpegParam);

    //
    pSingleShot->startOne(rSensorParam);

    //
    pSingleShot->uninit();

    //
    pSingleShot->destroyInstance();

    if(mShotParam.ms8ShotFileName.string()!=NULL) {
        ret = createFBJpegImg(Srcbufinfo,mu4W_yuv,mu4H_yuv,1);
        if  ( ! ret )
        {
            goto lbExit;
        }
    }

    sem_wait(&semMemoryDone);
    #ifdef Debug_Mode
    loadFileToBuf("/data/FBSOURCE.yuv",(uint8_t*)Srcbufinfo.virtAddr,Srcbufinfo.size);
    saveBufToFile("/sdcard/img.yuv", (uint8_t*)Srcbufinfo.virtAddr, Srcbufinfo.size);
    #endif
    CPTLog(Event_FBShot_createFullFrame, CPTFlagEnd);
    MY_LOGD("[createFullFrame] - \n");
lbExit:
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createJpegImg(NSCamHW::ImgBufInfo const & rSrcImgBufInfo
      , NSCamShot::JpegParam const & rJpgParm
      , MUINT32 const u4Rot
      , MUINT32 const u4Flip
      , NSCamHW::ImgBufInfo const & rJpgImgBufInfo
      , MUINT32 & u4JpegSize)
{
    MBOOL ret = MTRUE;
    // (0). debug
    MY_LOGD("[createJpegImg] - E.");
    MY_LOGD("[createJpegImg] - rSrcImgBufInfo.eImgFmt=%d", rSrcImgBufInfo.eImgFmt);
    MY_LOGD("[createJpegImg] - u4Rot=%d", u4Rot);
    MY_LOGD("[createJpegImg] - u4Flip=%d", u4Flip);
    CPTLog(Event_FBShot_JpegEncodeImg, CPTFlagStart);
    //
    // (1). Create Instance
    NSCamShot::ISImager *pISImager = NSCamShot::ISImager::createInstance(rSrcImgBufInfo);
    if(!pISImager) {
    MY_LOGE("HdrShot::createJpegImg can't get ISImager instance.");
    return MFALSE;
    }

    // init setting
    NSCamHW::BufInfo rBufInfo(rJpgImgBufInfo.u4BufSize, rJpgImgBufInfo.u4BufVA, rJpgImgBufInfo.u4BufPA, rJpgImgBufInfo.i4MemID);
    //
    pISImager->setTargetBufInfo(rBufInfo);
    //
    pISImager->setFormat(eImgFmt_JPEG);
    //
    pISImager->setRotation(u4Rot);
    //
    pISImager->setFlip(u4Flip);
    //
    pISImager->setResize(rJpgImgBufInfo.u4ImgWidth, rJpgImgBufInfo.u4ImgHeight);
    //
    pISImager->setEncodeParam(rJpgParm.fgIsSOI, rJpgParm.u4Quality);
    //
    pISImager->setROI(Rect(0, 0, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight));
    //
    pISImager->execute();
    //
    u4JpegSize = pISImager->getJpegSize();

    pISImager->destroyInstance();
    CPTLog(Event_FBShot_JpegEncodeImg, CPTFlagEnd);

    MY_LOGD("[init] - X. ret: %d.", ret);
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createJpegImgWithThumbnail(NSCamHW::ImgBufInfo const &rYuvImgBufInfo, NSCamHW::ImgBufInfo const &rPostViewBufInfo, MUINT32 const Mode)
{
    MBOOL ret = MTRUE;
    MUINT32 stride[3];
    MY_LOGD("[createJpegImgWithThumbnail] in");
    //rJpegImgBufInfo
    IMEM_BUF_INFO jpegBuf;
    jpegBuf.size = mu4W_yuv * mu4H_yuv;
    mpIMemDrv->allocVirtBuf(&jpegBuf);
    NSCamHW::ImgInfo    rJpegImgInfo(eImgFmt_JPEG, mu4W_yuv, mu4H_yuv);
    NSCamHW::BufInfo    rJpegBufInfo(jpegBuf.size, jpegBuf.virtAddr, jpegBuf.phyAddr, jpegBuf.memID);
    NSCamHW::ImgBufInfo   rJpegImgBufInfo(rJpegImgInfo, rJpegBufInfo, stride);

    //rThumbImgBufInfo
    IMEM_BUF_INFO thumbBuf;
    thumbBuf.size = mJpegParam.mi4JpegThumbWidth * mJpegParam.mi4JpegThumbHeight * 2;
    mpIMemDrv->allocVirtBuf(&thumbBuf);
    NSCamHW::ImgInfo    rThumbImgInfo(eImgFmt_JPEG, mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);
    NSCamHW::BufInfo    rThumbBufInfo(thumbBuf.size, thumbBuf.virtAddr, thumbBuf.phyAddr, thumbBuf.memID);
    NSCamHW::ImgBufInfo   rThumbImgBufInfo(rThumbImgInfo, rThumbBufInfo, stride);


    MUINT32 u4JpegSize = 0;
    MUINT32 u4ThumbSize = 0;

    NSCamShot::JpegParam yuvJpegParam(mJpegParam.mu4JpegQuality, MFALSE);
    ret = ret && createJpegImg(rYuvImgBufInfo, yuvJpegParam, mShotParam.mi4Rotation, 0 , rJpegImgBufInfo, u4JpegSize);

    // (3.1) create thumbnail
    // If postview is enable, use postview buffer,
    // else use yuv buffer to do thumbnail
    if (0 != mJpegParam.mi4JpegThumbWidth && 0 != mJpegParam.mi4JpegThumbHeight)
    {
        NSCamShot::JpegParam rParam(mJpegParam.mu4JpegThumbQuality, MTRUE);
        ret = ret && createJpegImg(rPostViewBufInfo, rParam, mShotParam.mi4Rotation, 0, rThumbImgBufInfo, u4ThumbSize);
    }

    #ifdef Debug_Mode // Save Img for debug.
    {
        char szFileName[100];

        saveBufToFile("/sdcard/Result.jpg", (uint8_t*)jpegBuf.virtAddr, u4JpegSize);
        MY_LOGD("[createJpegImgWithThumbnail] Save %s done.", szFileName);

        saveBufToFile("/sdcard/ThumbImg.jpg", (uint8_t*)thumbBuf.virtAddr, u4ThumbSize);
        MY_LOGD("[createJpegImgWithThumbnail] Save %s done.", szFileName);
    }
    #endif  // Debug_Mode


    // Jpeg callback, it contains thumbnail in ext1, ext2.
    handleJpegData((MUINT8*)rJpegImgBufInfo.u4BufVA, u4JpegSize, (MUINT8*)rThumbImgBufInfo.u4BufVA, u4ThumbSize, Mode);


    mpIMemDrv->freeVirtBuf(&jpegBuf);
    mpIMemDrv->freeVirtBuf(&thumbBuf);
    MY_LOGD("[createJpegImgWithThumbnail] out");
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createFBJpegImg(IMEM_BUF_INFO Srcbufinfo, int u4SrcWidth, int u4SrcHeight, MUINT32 const Mode)
{
    MY_LOGD("[createFBJpegImg] in");
    MBOOL ret = MTRUE;
    CPTLog(Event_FBShot_createFBJpegImg, CPTFlagStart);
    MUINT32     u4Stride[3];
    u4Stride[0] = mu4W_yuv;
    u4Stride[1] = mu4W_yuv >> 1;
    u4Stride[2] = mu4W_yuv >> 1;
    //mrHdrCroppedResult as rYuvImgBufInfo
    MUINT32         u4ResultSize = Srcbufinfo.size;
    NSCamHW::ImgInfo    rYuvImgInfo(eImgFmt_I422, u4SrcWidth , u4SrcHeight);
    NSCamHW::BufInfo    rYuvBufInfo(u4ResultSize, (MUINT32)Srcbufinfo.virtAddr, 0, Srcbufinfo.memID);
    NSCamHW::ImgBufInfo   rYuvImgBufInfo(rYuvImgInfo, rYuvBufInfo, u4Stride);

    mPostviewWidth = mShotParam.mi4PostviewWidth;
    mPostviewHeight = mShotParam.mi4PostviewHeight;
    IMEM_BUF_INFO tmpPostView;
    tmpPostView.size = android::MtkCamUtils::FmtUtils::queryImgBufferSize(mShotParam.ms8PostviewDisplayFormat, mPostviewWidth, mPostviewHeight);
    if(!(allocMem(tmpPostView)))
    {
        tmpPostView.size = 0;
        MY_LOGE("[STEP1] tmpPostView alloc fail");
        ret = MFALSE;
        return ret;
    }

    EImageFormat mPostviewFormat = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));
    mpPostviewImgBuf.size = android::MtkCamUtils::FmtUtils::queryImgBufferSize(mShotParam.ms8PostviewDisplayFormat, mPostviewWidth, mPostviewHeight);
    mpPostviewImgBuf.virtAddr = tmpPostView.virtAddr; //using original buffer for reduce memory
    mpPostviewImgBuf.memID = -1;
    ImgProcess(Srcbufinfo, u4SrcWidth, u4SrcHeight, eImgFmt_I422, mpPostviewImgBuf, mPostviewWidth, mPostviewHeight, mPostviewFormat);

    MUINT32     u4PosStride[3];
    u4PosStride[0] = mPostviewWidth;
    u4PosStride[1] = mPostviewWidth >> 1;
    u4PosStride[2] = mPostviewWidth >> 1;

    NSCamHW::ImgInfo    rPostViewImgInfo(mPostviewFormat, mPostviewWidth, mPostviewHeight);
    NSCamHW::BufInfo    rPostViewBufInfo(mpPostviewImgBuf.size, (MUINT32)mpPostviewImgBuf.virtAddr, 0, mpPostviewImgBuf.memID);
    NSCamHW::ImgBufInfo   rPostViewImgBufInfo(rPostViewImgInfo, rPostViewBufInfo, u4PosStride);

    if(!Mode)
        handlePostViewData((MUINT8*)mpPostviewImgBuf.virtAddr, mpPostviewImgBuf.size);

    ret = createJpegImgWithThumbnail(rYuvImgBufInfo, rPostViewImgBufInfo, Mode);

    if(!(deallocMem(tmpPostView)))
    {
        tmpPostView.size = 0;
        MY_LOGE("tmpPostView dealloc fail");
        ret = MFALSE;
        return ret;
    }

    CPTLog(Event_FBShot_createFBJpegImg, CPTFlagEnd);
    MY_LOGD("[createFBJpegImg] out");
    return ret;
}

MBOOL
Mhal_facebeauty::
doCapture()
{
    MBOOL ret = MFALSE;
    CPTLog(Event_FBShot_Utility, CPTFlagStart);
    //MINT8 TargetColor = NSCamCustom::get_FB_ColorTarget();
    //MINT8 BlurLevel = NSCamCustom::get_FB_BlurLevel();
    MINT8 TargetColor =0;
    MINT8 BlurLevel =4;

    ret =
        //  ()  Request Buffers.
        requestBufs()
        &&  createFullFrame(mpSource)
        &&  InitialAlgorithm(mu4W_yuv, mu4H_yuv, BlurLevel, TargetColor)
        &&  STEP1(mpSource, mu4W_yuv, mu4H_yuv, mpBlurImg, mpAmap, (void*) &msFaceBeautyResultInfo)
        //&&  SaveJpg()
        &&  STEP2(mpSource, mu4W_yuv, mu4H_yuv,mpAmap, &FBmetadata,(void*) &msFaceBeautyResultInfo)
        &&  STEP3(mpAmap,(void*) &msFaceBeautyResultInfo)
        //&&  WaitSaveDone()
        &&  STEP4(mpSource, mu4W_yuv, mu4H_yuv, mpBlurImg, mpAmap, (void*) &msFaceBeautyResultInfo)
        &&  STEP5(mpSource, mu4W_yuv, mu4H_yuv, mpAmap, (void*) &msFaceBeautyResultInfo)
        &&  STEP6(mpSource, mu4W_yuv, mu4H_yuv, mpBlurImg, (void*) &msFaceBeautyResultInfo)
        ;
    if  ( ! ret )
    {
        MY_LOGI("[FBCapture] Capture fail \n");
    }
    sem_post(&semFBthread);

    return ret;
}

MVOID* FBCapture(void *arg)
{
    mpFbObj->doCapture();
    return NULL;
}

MVOID* FBUtility(void *arg)
{
    MBOOL ret = MFALSE;
    while(UtilityStatus)
    {
        CAM_LOGD("[FBUtility] Wait in UtilityStatus %d",UtilityStatus);
        sem_wait(&semUtilitythread);
        CAM_LOGD("[FBUtility] get command UtilityStatus %d",UtilityStatus);
        switch(UtilityStatus)
        {
            case 3: // memory allocate
                CAM_LOGD("[FBUtility] memory allocate");
                mpFbObj->FBWorkingBufferSize = mpFbObj->mpFb->getWorkingBuffSize(mpFbObj->mu4W_yuv,mpFbObj->mu4H_yuv,mpFbObj->mDSWidth,mpFbObj->mDSHeight,((mpFbObj->mu4W_yuv >> 1) & 0xFFFFFFF0),((mpFbObj->mu4H_yuv >> 1) & 0xFFFFFFF0));
                CAM_LOGD("[requestBufs] FBWorkingBufferSize %d",mpFbObj->FBWorkingBufferSize);
                mpFbObj->mpWorkingBuferr.size = mpFbObj->FBWorkingBufferSize;
                if(!(mpFbObj->allocMem(mpFbObj->mpWorkingBuferr)))
                {
                    mpFbObj->mpWorkingBuferr.size = 0;
                    CAM_LOGE("[requestBufs] mpWorkingBuferr alloc fail");
                }

                mpFbObj->mpAmap.size = mpFbObj->mu4SourceSize;
                if(!(mpFbObj->allocMem(mpFbObj->mpAmap)))
                {
                    mpFbObj->mpAmap.size = 0;
                    CAM_LOGE("[requestBufs] mpAmap alloc fail");
                }

                mpFbObj->mpBlurImg.size = mpFbObj->mu4SourceSize;
                if(!(mpFbObj->allocMem(mpFbObj->mpBlurImg)))
                {
                    mpFbObj->mpBlurImg.size = 0;
                    CAM_LOGE("[requestBufs] mpBlurImg alloc fail");
                }

                sem_post(&semMemoryDone);
                break;
            case 2: // jpg encode
                CAM_LOGD("[FBUtility] jpg encode ");
                ret = mpFbObj->createFBJpegImg(mpFbObj->mpSource,mpFbObj->mu4W_yuv,mpFbObj->mu4H_yuv,1);
                if  ( ! ret )
                {
                    CAM_LOGD("[FBUtility] jpg encode fail");
                }
                sem_post(&semJPGDone);
                break;
            case 0:
            default:
                break;
        }
    }
    CAM_LOGD("[FBUtility] out");
    return NULL;
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
allocMem(IMEM_BUF_INFO &memBuf)
{
    //memBuf.memID=-1;
    //memBuf.virtAddr = (MUINT32)malloc(memBuf.size);
    //if(!memBuf.virtAddr) {
        //MY_LOGE("g_pIMemDrv->allocVirtBuf() error \n");
        //return MFALSE;
    //}

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
Mhal_facebeauty::
deallocMem(IMEM_BUF_INFO &memBuf)
{
    //if (mpIMemDrv->unmapPhyAddr(&memBuf)) {
        //MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        //return MFALSE;
    //}
    //free((void*)memBuf.virtAddr);
    if (mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
requestBufs()
{
    MBOOL   fgRet = MFALSE;
    mu4W_yuv = mShotParam.mi4PictureWidth;
    mu4H_yuv = mShotParam.mi4PictureHeight;
    #ifdef Debug_Mode
    mu4W_yuv = 640;
    mu4H_yuv = 480;
    #endif
    if((mu4W_yuv*3) == (mu4H_yuv*4))
    {
        mDSWidth = 640;
        mDSHeight = 480;
    }
    else if((mu4W_yuv*9) == (mu4H_yuv*16))
    {
        mDSWidth = 640;
        mDSHeight = 360;
    }
    else if((mu4W_yuv*3) == (mu4H_yuv*5))
    {
        mDSWidth = 640;
        mDSHeight = 384;
    }
    else
    {
        mDSWidth = 640;

        if(mu4W_yuv != 0)
          mDSHeight = 640 * mu4H_yuv/mu4W_yuv;
        else
          mDSHeight = 480;
    }

    CPTLog(Event_FBShot_requestBufs, CPTFlagStart);
    MY_LOGD("[requestBufs] mu4W_yuv %d mu4H_yuv %d",mu4W_yuv,mu4H_yuv);
    //  (1)

    mu4SourceSize=mu4W_yuv*mu4H_yuv*2;
    if(mu4SourceSize< (mShotParam.mi4PostviewWidth * mShotParam.mi4PostviewHeight * 2) )
        mu4SourceSize = (mShotParam.mi4PostviewWidth * mShotParam.mi4PostviewHeight * 2);
    mpSource.size = mu4SourceSize;
    if(!(allocMem(mpSource)))
    {
        mpSource.size = 0;
        MY_LOGE("[requestBufs] mpSource alloc fail");
        goto lbExit;
    }

    UtilityStatus = 3;
    sem_post(&semUtilitythread);

    CPTLog(Event_FBShot_requestBufs, CPTFlagEnd);
    fgRet = MTRUE;
lbExit:
    if  ( ! fgRet )
    {
        releaseBufs();
    }
    return  fgRet;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
releaseBufs()
{
    if(!(deallocMem(mpSource)))
        return  MFALSE;
    if(!(deallocMem(mpBlurImg)))
        return  MFALSE;
    if(!(deallocMem(mpAmap)))
        return  MFALSE;
    if(!(deallocMem(mpWorkingBuferr)))
        return  MFALSE;

    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
SaveJpg()
{
    //optimize jpg save in step2
    CAM_LOGD(" Save JPG %s",mShotParam.ms8ShotFileName.string());
    if(mShotParam.ms8ShotFileName.string()!=NULL)
    {
        CAM_LOGD("Save JPG");
        sem_init(&semJPGDone, 0, 0);
        UtilityStatus = 2;
        sem_post(&semUtilitythread);
    }
    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
WaitSaveDone()
{
    //optimize jpg save in step2
    CAM_LOGD(" WaitSaveDone %s",mShotParam.ms8ShotFileName.string());
    if(mShotParam.ms8ShotFileName.string()!=NULL)
    {
        sem_wait(&semJPGDone);
    }
    return  MTRUE;
}

