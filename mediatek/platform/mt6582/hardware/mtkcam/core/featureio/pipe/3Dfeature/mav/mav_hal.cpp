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
#define LOG_TAG "mHalMav"

#include <mtkcam/common.h>
#include <mtkcam/Log.h>
#include "mav_hal.h"


/*******************************************************************************
*
********************************************************************************/

static hal3DFBase *pHalMAV = NULL;
static MBOOL MAV_OPTIMIZE = 0;

/*******************************************************************************
*
********************************************************************************/
hal3DFBase*
halMAV::
getInstance()
{
    CAM_LOGD("[halMAV] getInstance \n");
    if (pHalMAV == NULL) {
        pHalMAV = new halMAV();
    }
    return pHalMAV;
}

/*******************************************************************************
*
********************************************************************************/
void   
halMAV::
destroyInstance() 
{
    CAM_LOGD("[halMAV] destroyInstance \n");
    if (pHalMAV) {
        delete pHalMAV;
    }
    pHalMAV = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halMAV::halMAV()
{
    CAM_LOGD("[halMAV consturtor] \n");
    m_pMTKMavObj = NULL;   
    m_pMTKMotionObj = NULL; 
    m_pMTKWarpObj = NULL; 
    m_pMTKPanoMotionObj = NULL; 
    memset(&MAVPreMotionResult,0,sizeof(MAVMotionResultInfo));
    FrameCunt=0;               
}


halMAV::~halMAV()
{    
    CAM_LOGD("[halMAV desturtor] \n");
} 

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfInit(void* MavInitInData,void* MotionInitInData,void* WarpInitInData,void* Pano3DInitInData
)
{
    MINT32 err = S_MAV_OK;
    MavInitInfo myInfo;
    MTKMotionEnvInfo MyMotionEnvInfo;
    MTKMotionTuningPara MyMotionTuningPara;
    CAM_LOGD("[mHalMavInit] \n");
    
    if (m_pMTKMavObj) 
        CAM_LOGD("[mHalMavInit] m_pMTKMavObj Init has been called \n");    
    else    	
        m_pMTKMavObj = MTKMav::createInstance(DRV_MAV_OBJ_SW);
    //Not use initial function to set working buffer        
    myInfo.WorkingBuffAddr=(MUINT32)MavInitInData;
    myInfo.pTuningInfo = NULL;
    m_pMTKMavObj->MavInit((void*)&myInfo, NULL);
    
    if (m_pMTKMotionObj) 
        CAM_LOGD("[mHalMavInit] m_pMTKMotionObj Init has been called \n");    
    else
        m_pMTKMotionObj = MTKMotion::createInstance(DRV_MOTION_OBJ_MAV);
    MyMotionEnvInfo.WorkingBuffAddr = (MUINT32)Pano3DInitInData;
    MyMotionEnvInfo.pTuningPara = NULL;
    m_pMTKMotionObj->MotionInit(&MyMotionEnvInfo, NULL);
       
    if (m_pMTKPanoMotionObj) 
        CAM_LOGD("[mHalMavInit] m_pMTKPanoMotionObj Init has been called \n");    
    else
        m_pMTKPanoMotionObj = MTKMotion::createInstance(DRV_MOTION_OBJ_PANO);
    MyMotionEnvInfo.WorkingBuffAddr = (MUINT32)MotionInitInData;
    MyMotionEnvInfo.pTuningPara = &MyMotionTuningPara;
    MyMotionEnvInfo.SrcImgWidth = MOTION_IM_WIDTH;
    MyMotionEnvInfo.SrcImgHeight = MOTION_IM_HEIGHT;
    MyMotionEnvInfo.WorkingBuffSize = MOTION_WORKING_BUFFER_SIZE;
    MyMotionEnvInfo.pTuningPara->OverlapRatio = OVERLAP_RATIO;
    m_pMTKPanoMotionObj->MotionInit(&MyMotionEnvInfo, NULL);
    
    if (m_pMTKWarpObj) 
        CAM_LOGD("[mHalMavInit] m_pMTKWarpObj Init has been called \n");    
    else
        m_pMTKWarpObj = MTKWarp::createInstance(DRV_WARP_OBJ_MAV);
    m_pMTKWarpObj->WarpInit((MUINT32*)WarpInitInData,NULL);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfUninit(
)
{
    CAM_LOGD("[mHalMavUninit] \n");

    if (m_pMTKMavObj) {
        m_pMTKMavObj->MavReset();
        m_pMTKMavObj->destroyInstance();
    }
    m_pMTKMavObj = NULL;
    
    if (m_pMTKPanoMotionObj) {
        m_pMTKPanoMotionObj->MotionExit();
        m_pMTKPanoMotionObj->destroyInstance();
    }
    m_pMTKPanoMotionObj = NULL;
    
    if (m_pMTKMotionObj) {
        m_pMTKMotionObj->MotionExit();
        m_pMTKMotionObj->destroyInstance();
    }
    m_pMTKMotionObj = NULL;
    
    if (m_pMTKWarpObj) {
        m_pMTKWarpObj->WarpReset();
        m_pMTKWarpObj->destroyInstance();
    }
    m_pMTKWarpObj = NULL;
    
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHalMavMain(
)
{
	  MINT32 err = S_MAV_OK;
    CAM_LOGD("[mHalMavMain] \n");	
    if (!m_pMTKMavObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalMavMain] Err, Init has been called \n");
    }
    m_pMTKMavObj->MavMain();
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfAddImg(MavPipeImageInfo* pParaIn
)
{
	  MINT32 err = S_MAV_OK;
    CAM_LOGD("[mHalMavAddImg] \n");	
    if (!m_pMTKMavObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalMavAddImg] Err, Init has been called \n");
    }
    MAV_OPTIMIZE=pParaIn->ControlFlow;
    m_pMTKMavObj->MavFeatureCtrl(MAV_FEATURE_ADD_IMAGE,pParaIn,NULL);
    mHalMavMain();
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halMAV::mHal3dfGetMavResult(void* pParaOut
)
{
    MINT32 err = S_MAV_OK;
    CAM_LOGD("[mHal3dfGetMavResult] \n");	
    if (!m_pMTKMavObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHal3dfGetMavResult] Err, object not exist \n");
    }
    m_pMTKMavObj->MavFeatureCtrl(MAV_FEATURE_GET_RESULT, 0, pParaOut);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfMerge(MUINT32 *MavResult
)
{
	  MINT32 err = S_MAV_OK;
	  MavResultInfo	MyMavResultInfo;
	  MavPipeResultInfo* MyMAVResult=(MavPipeResultInfo*)MavResult;
	  //MFLOAT* myMavResult=(MFLOAT*)MavResult;
    CAM_LOGD("[mHalMavMerge] \n");	
    if (!m_pMTKMavObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalMavMerge] Err, Init has been called \n");
    }
    m_pMTKMavObj->MavMerge((MUINT32 *)&MyMavResultInfo);
    MyMAVResult->ViewIdx=MyMavResultInfo.ViewIdx;
    MyMAVResult->ClipWidth=MyMavResultInfo.ClipWidth;
    MyMAVResult->ClipHeight=MyMavResultInfo.ClipHeight;
    MyMAVResult->RetCode=MyMavResultInfo.RetCode;
    MyMAVResult->ErrPattern=MyMavResultInfo.ErrPattern;
    memcpy(MyMAVResult->ImageHmtx,(void*)&MyMavResultInfo.ImageHmtx,sizeof(MFLOAT)*MAV_MAX_IMAGE_NUM*RANK*RANK);
    
    for(int i=0;i<MAV_MAX_IMAGE_NUM;i++)
    {
    	   MyMAVResult->ImageInfo[i].ClipX = MyMavResultInfo.ImageInfo[i].ClipX;
    	   MyMAVResult->ImageInfo[i].ClipY = MyMavResultInfo.ImageInfo[i].ClipY;
           //LOGD("[mHalMavMerge] MyMavResultInfo %f %f %f , %f %f %f , %f %f %f \n",(MFLOAT)MyMavResultInfo.ImageHmtx[i][0][0],(MFLOAT)MyMavResultInfo.ImageHmtx[i][0][1],(MFLOAT)MyMavResultInfo.ImageHmtx[i][0][2],(MFLOAT)MyMavResultInfo.ImageHmtx[i][1][0],(MFLOAT)MyMavResultInfo.ImageHmtx[i][1][1],(MFLOAT)MyMavResultInfo.ImageHmtx[i][1][2],(MFLOAT)MyMavResultInfo.ImageHmtx[i][2][0],(MFLOAT)MyMavResultInfo.ImageHmtx[i][2][1],(MFLOAT)MyMavResultInfo.ImageHmtx[i][2][2]);
           //LOGD("[mHalMavMerge] MyMAVResult Width %d  Height %d ",MyMAVResult->ImageInfo[i].Width,MyMAVResult->ImageInfo[i].Height);       
           //LOGD("[mHalMavMerge] MavResult %f %f %f , %f %f %f , %f %f %f \n",(MFLOAT)*(myMavResult+(i*9)),(MFLOAT)*(myMavResult+(i*9+1)),(MFLOAT)*(myMavResult+(i*9+2)),(MFLOAT)*(myMavResult+(i*9+3)),(MFLOAT)*(myMavResult+(i*9+4)),(MFLOAT)*(myMavResult+(i*9+5)),(MFLOAT)*(myMavResult+(i*9+6)),(MFLOAT)*(myMavResult+(i*9+7)),(MFLOAT)*(myMavResult+(i*9+8)));
    } 
  
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfDoMotion(void* InputData,MUINT32* MotionResult, MUINT32 u4SrcImgWidth, MUINT32 u4SrcImgHeight)
{
	  MINT32 err = S_MAV_OK;
	  MTKMotionProcInfo MotionInfo;
	  MAVMotionResultInfo*  MAVMotionResult = (MAVMotionResultInfo*)MotionResult;
    //eis_stat_t* EISResult=(eis_stat_t*) InputData;
    CAM_LOGD("[mHalMavDoMotion] FrameCunt %d. u4SrcImgWidth: %d. u4SrcImgHeight: %d. \n", FrameCunt, u4SrcImgWidth, u4SrcImgHeight);	
    if(FrameCunt<3)
    {
       MAVMotionResult->MV_X=0;
       MAVMotionResult->MV_Y=0; 
       MAVMotionResult->ReadyToShot=0;
    	 FrameCunt++;
    	 return err;
    }	  
    else
    	 FrameCunt=3;
    if (!m_pMTKPanoMotionObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalMavDoMotion] Err, m_pMTKPanoMotionObj Init has been called \n");
    }
       
    MotionInfo.ImgAddr = (MUINT32)InputData;
    CAM_LOGD("[mHalMAVDoMotion] ImgAddr 0x%x\n",MotionInfo.ImgAddr);
    m_pMTKPanoMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_SET_PROC_INFO, &MotionInfo, NULL);
    m_pMTKPanoMotionObj->MotionMain();    
    m_pMTKPanoMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_GET_RESULT, NULL, MotionResult);
 
    if (!m_pMTKMotionObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalMavDoMotion] Err, m_pMTKMotionObj Init has been called \n");
    }
    
    MFLOAT fScaleW = (-1) * float(u4SrcImgWidth) / 320.0f;
    MFLOAT fScaleH = (-1) * float(u4SrcImgHeight) / 240.0f;
    CAM_LOGD("[mHalMavDoMotion] MVX/Y: (%f, %f). PreMVX/Y: (%f, %f). fScaleW/H: (%f, %f).\n",(MFLOAT)MAVMotionResult->MV_X,(MFLOAT)MAVMotionResult->MV_Y,(MFLOAT)MAVPreMotionResult.MV_X,(MFLOAT)MAVPreMotionResult.MV_Y, fScaleW, fScaleH);
    for(int i=0;i<MOTION_TOTAL_BN;i++)
    {
        //MotionInfo.MotionValueXY[i*2]=(MFLOAT)EISResult->i4LMV_X[i]/256;
        //MotionInfo.MotionValueXY[(i*2)+1]=(MFLOAT)EISResult->i4LMV_Y[i]/256;
        //MotionInfo.TrustValueXY[i*2]=(MFLOAT)EISResult->i4Trust_X[i];
        //MotionInfo.TrustValueXY[(i*2)+1]=(MFLOAT)EISResult->i4Trust_Y[i];
        //CAM_LOGD("[mHalMavDoMotion] MVX %f MVY %f TVX %f TVY %f\n",(MFLOAT)EISResult.i4LMV_X[i],(MFLOAT)EISResult.i4LMV_Y[i],(MFLOAT)EISResult.i4Trust_X[i],(MFLOAT)EISResult.i4Trust_Y[i]);
        
        MotionInfo.MotionValueXY[i*2]=(MFLOAT)(MAVMotionResult->MV_X-MAVPreMotionResult.MV_X);
        MotionInfo.MotionValueXY[(i*2)+1]=(MFLOAT)(MAVMotionResult->MV_Y-MAVPreMotionResult.MV_Y);
//        MotionInfo.MotionValueXY[i*2]*= -2;
//        MotionInfo.MotionValueXY[(i*2)+1]*= -2;
        MotionInfo.MotionValueXY[i*2]*= fScaleW;
        MotionInfo.MotionValueXY[(i*2)+1]*= fScaleH;
        MotionInfo.TrustValueXY[i*2]= 50;
        MotionInfo.TrustValueXY[(i*2)+1]= 50;
    }
    MAVPreMotionResult.MV_X=MAVMotionResult->MV_X;
    MAVPreMotionResult.MV_Y=MAVMotionResult->MV_Y;
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_SET_PROC_INFO, &MotionInfo, NULL);
    m_pMTKMotionObj->MotionMain();    
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_GET_RESULT, NULL, MotionResult);
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfWarp(MavPipeImageInfo* pParaIn, MUINT32 *MavResult,MUINT8 ImgNum
)
{
	  MINT32 err = S_MAV_OK;
	  WarpImageInfo	MyImageInfo;
	  MavPipeResultInfo MyMavPipeResultInfo;
    CAM_LOGD("[mHalMavWarp] \n");	
    if (!m_pMTKWarpObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalMavWarp] Err, Init has been called \n");
    }
    memcpy((void*)&MyMavPipeResultInfo, MavResult, sizeof(MavPipeResultInfo));
    if(MAV_OPTIMIZE)
    {  
    	  CAM_LOGD("[mHal3dfWarp] Optimize Ture \n");
        MyImageInfo.ImgAddr[0] = (MUINT32)pParaIn->ImgAddr;
        MyImageInfo.ImgNum = 1;
        MyImageInfo.ImgFmt = WARP_IMAGE_NV21;
        MyImageInfo.Width = pParaIn->Width;
        MyImageInfo.Height = pParaIn->Height;
        memcpy(MyImageInfo.Hmtx[0], MyMavPipeResultInfo.ImageHmtx[ImgNum], 9*sizeof(float));
        MyImageInfo.ClipWidth = pParaIn->Width;
        MyImageInfo.ClipHeight = pParaIn->Height;
        MyImageInfo.ClipX[0] = 0;
        MyImageInfo.ClipY[0] = 0;        
    }
    else        
    {
    	  CAM_LOGD("[mHal3dfWarp] Optimize false \n");
        
        MyImageInfo.ImgNum = ImgNum;
        MyImageInfo.ImgFmt = WARP_IMAGE_NV21;
        MyImageInfo.Width = pParaIn->Width;
        MyImageInfo.Height = pParaIn->Height;
        MyImageInfo.ClipWidth = MyMavPipeResultInfo.ClipWidth;
        MyImageInfo.ClipHeight = MyMavPipeResultInfo.ClipHeight;
        memcpy(MyImageInfo.Hmtx, MyMavPipeResultInfo.ImageHmtx, sizeof(MFLOAT)*ImgNum*RANK*RANK);

        for(int i=0;i<ImgNum;i++)
        {
            MyImageInfo.ImgAddr[i] = (MUINT32)MyMavPipeResultInfo.ImageInfo[i].ImgAddr;
            MyImageInfo.ClipX[i] = MyMavPipeResultInfo.ImageInfo[i].ClipX;
            MyImageInfo.ClipY[i] = MyMavPipeResultInfo.ImageInfo[i].ClipY;
            CAM_LOGD("[mHalMavWarp] ClipX %d ClipY %d time %d",MyImageInfo.ClipX[i],MyImageInfo.ClipY[i],i); 
            CAM_LOGD("[mHalMavWarp] ImgAddr 0x%x ImgNum %d Width %d Height %d ClipWidth %d ClipHeight %d",MyImageInfo.ImgAddr[i],MyImageInfo.ImgNum,MyImageInfo.Width,MyImageInfo.Height,MyImageInfo.ClipWidth,MyImageInfo.ClipHeight);    
            CAM_LOGD("[mHalMavWarp] Hmtx %f %f %f , %f %f %f , %f %f %f",(MFLOAT)MyImageInfo.Hmtx[i][0],(MFLOAT)MyImageInfo.Hmtx[i][1],(MFLOAT)MyImageInfo.Hmtx[i][2],(MFLOAT)MyImageInfo.Hmtx[i][3],(MFLOAT)MyImageInfo.Hmtx[i][4],(MFLOAT)MyImageInfo.Hmtx[i][5],(MFLOAT)MyImageInfo.Hmtx[i][6],(MFLOAT)MyImageInfo.Hmtx[i][7],(MFLOAT)MyImageInfo.Hmtx[i][8]);    
      
        } 
        if(MyMavPipeResultInfo.RetCode!=S_MAV_OK)
        {
            CAM_LOGD("[mHal3dfWarp] Rectify error martix reset \n");
            for(int i=0;i<MAV_MAX_IMAGE_NUM;i++)
            {
                MyImageInfo.Hmtx[i][0]=1.f;
                MyImageInfo.Hmtx[i][1]=0.f;
                MyImageInfo.Hmtx[i][2]=0.f;
                MyImageInfo.Hmtx[i][3]=0.f;
                MyImageInfo.Hmtx[i][4]=1.f;
                MyImageInfo.Hmtx[i][5]=0.f;
                MyImageInfo.Hmtx[i][6]=0.f;
                MyImageInfo.Hmtx[i][7]=0.f;
                MyImageInfo.Hmtx[i][8]=1.f;
            }
        }    
    }    
    
            
    m_pMTKWarpObj->WarpFeatureCtrl(WARP_FEATURE_ADD_IMAGE, &MyImageInfo, NULL);

    // warping
    m_pMTKWarpObj->WarpMain();
        
    return err;	 
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfCrop(MUINT32 *MavResult,MUINT8 ImgNum
)
{
    MINT32 err = S_MAV_OK;
    
    WarpImageInfo	MyImageInfo;
    MavPipeResultInfo MyMavPipeResultInfo;
    memcpy((void*)&MyMavPipeResultInfo, MavResult, sizeof(MavPipeResultInfo));
    MFLOAT Imtx[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	  
    MyImageInfo.ImgAddr[0] = MyMavPipeResultInfo.ImageInfo[0].ImgAddr;
    MyImageInfo.ImgNum = ImgNum;
    MyImageInfo.ImgFmt = WARP_IMAGE_YV12;
    MyImageInfo.Width = MyMavPipeResultInfo.ImageInfo[0].Width;
    MyImageInfo.Height = MyMavPipeResultInfo.ImageInfo[0].Height;
    MyImageInfo.ClipWidth = MyMavPipeResultInfo.ClipWidth;
    MyImageInfo.ClipHeight = MyMavPipeResultInfo.ClipHeight;
    
    CAM_LOGD("[mHalMavWarp] ImgAddr 0x%x ClipWidth %d ClipHeight %d ImgNum %d\n",MyImageInfo.ImgAddr,MyMavPipeResultInfo.ClipWidth,MyMavPipeResultInfo.ClipHeight,ImgNum); 
    for(int i=0;i<MAV_MAX_IMAGE_NUM;i++)
    { 
        memcpy(MyImageInfo.Hmtx[i], Imtx, sizeof(MFLOAT)*RANK*RANK);
        MyImageInfo.ClipX[i] = MyMavPipeResultInfo.ImageInfo[i].ClipX;
        MyImageInfo.ClipY[i] = MyMavPipeResultInfo.ImageInfo[i].ClipY;
        //CAM_LOGD("[mHalMavWarp] ClipX %d ClipY %d time %d\n",MyImageInfo.ClipX[i],MyImageInfo.ClipY[i],i);        
    }   
    m_pMTKWarpObj->WarpFeatureCtrl(WARP_FEATURE_ADD_IMAGE, &MyImageInfo, NULL);
    // warping
    m_pMTKWarpObj->WarpMain();
    m_pMTKMavObj->MavReset();
    return err;
}
/*******************************************************************************
*
********************************************************************************/
MINT32
halMAV::mHal3dfGetResult(MUINT32& MavResult,MUINT32& ClipWidth, MUINT32& ClipHeight
)
{
	 MINT32 err = S_MAV_OK;
	 WarpResultInfo MyResultInfo;
	 m_pMTKWarpObj->WarpFeatureCtrl(WARP_FEATURE_GET_RESULT, NULL, &MyResultInfo);
	 if(MyResultInfo.RetCode!=1)
	 {	
	 	  CAM_LOGD("[mHalMavGetResult] Warp fail %d\n",MyResultInfo.RetCode);	
	 	  MavResult=0;
	 	  ClipWidth = 0; 
	 	  ClipHeight = 0;
	 	  err = E_MAV_ERR;
	 }
	 else
	 {
	 	  CAM_LOGD("[mHalMavGetResult] Warp success\n");	
	 	  ClipWidth = MyResultInfo.Width; 
	 	  ClipHeight = MyResultInfo.Height;
	 	  MavResult=1; 	 	   
	 }	
	 return err;	
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halMAV::mHal3dfGetWokSize(int SrcWidth, int SrcHeight, MUINT32 &WorkingSize)
{
    MINT32 err = S_MAV_OK;
	  MUINT32 mav_buf_size = 0;
	  MavImageInfo MyMavInfo;
    MyMavInfo.Width = SrcWidth;
    MyMavInfo.Height = SrcHeight;
    CAM_LOGD("[mHal3dfGetWokSize] 0x%x 0x%x ",&MyMavInfo,&mav_buf_size);
    m_pMTKMavObj->MavFeatureCtrl(MAV_FEATURE_GET_WORKBUF_SIZE, &MyMavInfo, &mav_buf_size);
    WorkingSize = mav_buf_size;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halMAV::mHal3dfSetWokBuff(void* WorkingBuff)
{
    MINT32 err = S_MAV_OK;
    CAM_LOGD("[mHal3dfSetWokBuff] 0x%x",(MUINT32)WorkingBuff);
    m_pMTKMavObj->MavFeatureCtrl(MAV_FEATURE_SET_WORKBUF_ADDR, &WorkingBuff, NULL);
    return err;
}
