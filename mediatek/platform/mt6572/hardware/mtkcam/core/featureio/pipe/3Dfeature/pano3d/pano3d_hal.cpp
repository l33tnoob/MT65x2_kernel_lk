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
#define LOG_TAG "mHalPANO3D"

#include <mtkcam/common.h>
#include <mtkcam/Log.h>

#include "pano3d_hal.h"


/*******************************************************************************
*
********************************************************************************/

static hal3DFBase *pHalPANO3D = NULL;
MUINT8 frameskipcnt=0;
/*******************************************************************************
*
********************************************************************************/
hal3DFBase*
halPANO3D::
getInstance()
{
    CAM_LOGD("[halPANO3D] getInstance \n");
    if (pHalPANO3D == NULL) {
        pHalPANO3D = new halPANO3D();
    }
    return pHalPANO3D;
}

/*******************************************************************************
*
********************************************************************************/
void   
halPANO3D::
destroyInstance() 
{
    if (pHalPANO3D) {
        delete pHalPANO3D;
    }
    pHalPANO3D = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halPANO3D::halPANO3D()
{
    m_pMTKMavObj = NULL;   
    m_pMTKMotionObj = NULL; 
    m_pMTKWarpObj = NULL; 
    m_pMTKPano3dObj = NULL;
}


halPANO3D::~halPANO3D()
{    
} 

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHal3dfInit(void* MavInitInData,void* MotionInitInData,void* WarpInitInData,void* Pano3DInitInData
)
{
    MINT32 err = S_MAV_OK;
    MavInitInfo myInfo;
    MTKMotionEnvInfo MyMotionEnvInfo;
    Pano3DImageInfo MyPano3DIamgeInfo;
    MTKMotionTuningPara MyMotionTuningPara;
    
    CAM_LOGD("[mHal3dfInit] \n");
    CAM_LOGD("[mHal3dfInit] MavInitInData 0x%x MotionInitInData 0x%x WarpInitInData 0x%x Pano3DInitInData 0x%x\n",(MUINT32)MavInitInData,(MUINT32)MotionInitInData,(MUINT32)WarpInitInData,(MUINT32)Pano3DInitInData);
    if (m_pMTKMavObj) 
        CAM_LOGD("[mHal3dfInit] m_pMTKMavObj Init has been called \n");    
    else    	
        m_pMTKMavObj = MTKMav::createInstance(DRV_MAV_OBJ_PANO3D);
    myInfo.WorkingBuffAddr=(MUINT32)MavInitInData;
    myInfo.pTuningInfo = NULL;
    m_pMTKMavObj->MavInit((void*)&myInfo, NULL);
    
    if (m_pMTKMotionObj) 
        CAM_LOGD("[mHal3dfInit] m_pMTKMotionObj Init has been called \n");    
    else
        m_pMTKMotionObj = MTKMotion::createInstance(DRV_MOTION_OBJ_3D_PANO);
    MyMotionEnvInfo.WorkingBuffAddr = (MUINT32)MotionInitInData;
    MyMotionEnvInfo.pTuningPara = &MyMotionTuningPara;
    MyMotionEnvInfo.SrcImgWidth = MOTION_IM_WIDTH;
    MyMotionEnvInfo.SrcImgHeight = MOTION_IM_HEIGHT;
    MyMotionEnvInfo.WorkingBuffSize = MOTION_WORKING_BUFFER_SIZE;
    MyMotionEnvInfo.pTuningPara->OverlapRatio = PANO3DOVERLAP_RATIO;
    m_pMTKMotionObj->MotionInit(&MyMotionEnvInfo, NULL);
        
    if (m_pMTKWarpObj) 
        CAM_LOGD("[mHalMavInit] m_pMTKWarpObj Init has been called \n");    
    else
        m_pMTKWarpObj = MTKWarp::createInstance(DRV_WARP_OBJ_MAV);
    m_pMTKWarpObj->WarpInit((MUINT32*)WarpInitInData,NULL);
    
    if (m_pMTKPano3dObj) 
        CAM_LOGD("[mHalMavInit] m_pMTKWarpObj Init has been called \n");    
    else
        m_pMTKPano3dObj = MTKPano3D::createInstance(DRV_PANO3D_OBJ_SW);
    MyPano3DIamgeInfo.WorkingBuffAddr = (MUINT32)Pano3DInitInData;
    m_pMTKPano3dObj->Pano3DInit((void*)&MyPano3DIamgeInfo,NULL);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHal3dfUninit(
)
{
    CAM_LOGD("[mHalPANO3DUninit] \n");

    if (m_pMTKMavObj) {
        m_pMTKMavObj->MavReset();
        m_pMTKMavObj->destroyInstance();
    }
    m_pMTKMavObj = NULL;
    
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

    if (m_pMTKPano3dObj) {
        m_pMTKPano3dObj->Pano3DReset();
        m_pMTKPano3dObj->destroyInstance();
    }
    m_pMTKPano3dObj = NULL;
        
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHalPANO3DMain(
)
{
	  MINT32 err = S_MAV_OK;
    CAM_LOGD("[mHalPANO3DMain] \n");	
    if (!m_pMTKPano3dObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalPANO3DMain] Err, Init has been called \n");
    }
    m_pMTKPano3dObj->Pano3DMain();
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHal3dfAddImg(MavPipeImageInfo* pParaIn
)
{
	  MINT32 err = S_MAV_OK;
    CAM_LOGD("[mHalPANO3DAddImg] \n");	
    if (!m_pMTKMavObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalPANO3DAddImg] Err, Init has been called \n");
    }
    m_pMTKMavObj->MavFeatureCtrl(MAV_FEATURE_ADD_IMAGE,pParaIn,NULL);
    m_pMTKMavObj->MavMain();
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHal3dfMerge(MUINT32 *MavResult
)
{
	  MINT32 err = S_MAV_OK;
	  MavResultInfo	MyMavResultInfo;
	  MavPipeResultInfo* MyMAVResult=(MavPipeResultInfo*)MavResult;
	  //MFLOAT* myMavResult=(MFLOAT*)MavResult;
    CAM_LOGD("[mHalPANO3DMerge] \n");	
    if (!m_pMTKMavObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalPANO3DMerge] Err, Init has been called \n");
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
    	   MyMAVResult->ImageInfo[i].GridX = MyMavResultInfo.ImageInfo[i].GridX;
         MyMAVResult->ImageInfo[i].MinX = MyMavResultInfo.ImageInfo[i].MinX;
         MyMAVResult->ImageInfo[i].MaxX = MyMavResultInfo.ImageInfo[i].MaxX;
    	   
         //LOGD("[mHalMavMerge] MyMavResultInfo %f %f %f , %f %f %f , %f %f %f \n",(MFLOAT)MyMavResultInfo.ImageHmtx[i][0][0],(MFLOAT)MyMavResultInfo.ImageHmtx[i][0][1],(MFLOAT)MyMavResultInfo.ImageHmtx[i][0][2],(MFLOAT)MyMavResultInfo.ImageHmtx[i][1][0],(MFLOAT)MyMavResultInfo.ImageHmtx[i][1][1],(MFLOAT)MyMavResultInfo.ImageHmtx[i][1][2],(MFLOAT)MyMavResultInfo.ImageHmtx[i][2][0],(MFLOAT)MyMavResultInfo.ImageHmtx[i][2][1],(MFLOAT)MyMavResultInfo.ImageHmtx[i][2][2]);
         //LOGD("[mHalMavMerge] MyMAVResult Width %d  Height %d  ClipX %d ClipY %d ",MyMAVResult->ImageInfo[i].Width,MyMAVResult->ImageInfo[i].Height,MyMavResultInfo.ImageInfo[i].ClipX,MyMavResultInfo.ImageInfo[i].ClipY);       
         //LOGD("[mHalMavMerge] MyMAVResult GridX %d  MinX %d  MaxX %d ",MyMAVResult->ImageInfo[i].GridX,MyMAVResult->ImageInfo[i].MinX,MyMAVResult->ImageInfo[i].MaxX);       
    } 
  
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHal3dfDoMotion(void* InputData,MUINT32* MotionResult, MUINT32 u4SrcImgWidth, MUINT32 u4SrcImgHeight)
{
	  MINT32 err = S_MAV_OK;
	  MTKMotionProcInfo MotionInfo;
	  frameskipcnt++;
	  if(frameskipcnt<3)
	  {
	  	 //CAM_LOGD("[mHalAutoramaDoMotion] return skipcnt %d \n",skipcnt);	
	  	 return err;
	  }
	  else 
	  	 frameskipcnt=3;
	  
    //CAM_LOGD("[mHalAutoramaDoMotion] skipcnt %d \n",skipcnt);	
    if (!m_pMTKMotionObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalAutoramaDoMotion] Err, Init has been called \n");
        return err;
    }
    MotionInfo.ImgAddr = (MUINT32)InputData;
    //CAM_LOGD("[mHalAutoramaDoMotion] ImgAddr 0x%x\n",MotionInfo.ImgAddr);
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_SET_PROC_INFO, &MotionInfo, NULL);
    m_pMTKMotionObj->MotionMain();    
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_GET_RESULT, NULL, MotionResult);
    return S_MAV_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halPANO3D::mHal3dfWarp(MavPipeImageInfo* pParaIn, MUINT32 *MavResult,MUINT8 ImgNum
)
{
	  MINT32 err = S_MAV_OK;
	  WarpImageInfo	MyImageInfo;
	  MavPipeResultInfo MyMavPipeResultInfo;
    CAM_LOGD("[mHalPANO3DWarp] ImgNum %d\n",ImgNum);	
    if (!m_pMTKWarpObj) {
        err = E_MAV_ERR;
        CAM_LOGD("[mHalPANO3DWarp] Err, Init has been called \n");
    }
    memcpy((void*)&MyMavPipeResultInfo, MavResult, sizeof(MavPipeResultInfo));
    
    MyImageInfo.ImgNum = ImgNum;
    MyImageInfo.ImgFmt = WARP_IMAGE_YV12;
    MyImageInfo.Width = MyMavPipeResultInfo.ImageInfo[0].Width;
    MyImageInfo.Height = MyMavPipeResultInfo.ImageInfo[0].Height;
    MyImageInfo.ClipWidth = MyMavPipeResultInfo.ImageInfo[0].Width;
    MyImageInfo.ClipHeight = MyMavPipeResultInfo.ImageInfo[0].Height;
    memcpy(MyImageInfo.Hmtx, MyMavPipeResultInfo.ImageHmtx, sizeof(MFLOAT)*MAV_MAX_IMAGE_NUM*RANK*RANK);
    
    for(int i=0;i<MAV_MAX_IMAGE_NUM;i++)
    {
        MyImageInfo.ImgAddr[i] = MyMavPipeResultInfo.ImageInfo[i].ImgAddr;
        MyImageInfo.ClipX[i] = 0;
        MyImageInfo.ClipY[i] = 0;
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
halPANO3D::mHal3dfStitch(MUINT32 *MavResult,MUINT8 ImgNum
)
{
	  MINT32 err = S_MAV_OK;
	  
    Pano3DImageInfo MyPano3DIamgeInfo;
    MavPipeResultInfo MyMavPipeResultInfo;
    CAM_LOGD("[mHal3dfStitch] IN %d \n",ImgNum);
    memcpy((void*)&MyMavPipeResultInfo, MavResult, sizeof(MavPipeResultInfo));    
    MyPano3DIamgeInfo.ImgAddr = MyMavPipeResultInfo.ImageInfo[0].ImgAddr;
    MyPano3DIamgeInfo.ImgNum = ImgNum;
    MyPano3DIamgeInfo.ImgWidth = MyMavPipeResultInfo.ImageInfo[0].Width;;
    MyPano3DIamgeInfo.ImgHeight = MyMavPipeResultInfo.ImageInfo[0].Height;
    MyPano3DIamgeInfo.ClipY = MyMavPipeResultInfo.ImageInfo[0].ClipY;
    MyPano3DIamgeInfo.ClipHeight = MyMavPipeResultInfo.ClipHeight;
    //CAM_LOGD("[mHal3dfStitch] ImgAddr 0x%x ImgWidth %d ImgHeight %d ClipY %d ClipHeight %d\n",MyPano3DIamgeInfo.ImgAddr,MyPano3DIamgeInfo.ImgWidth,MyPano3DIamgeInfo.ImgHeight,MyPano3DIamgeInfo.ClipY,MyPano3DIamgeInfo.ClipHeight);
    for(int i=0; i<ImgNum; i++)
    {
       MyPano3DIamgeInfo.GridX[i] = MyMavPipeResultInfo.ImageInfo[i].GridX;
       MyPano3DIamgeInfo.MinX[i] = MyMavPipeResultInfo.ImageInfo[i].MinX;
       MyPano3DIamgeInfo.MaxX[i] = MyMavPipeResultInfo.ImageInfo[i].MaxX;
       //CAM_LOGD("[mHal3dfStitch] GridX %d MinX %d MaxX %d \n",MyPano3DIamgeInfo.GridX[i],MyPano3DIamgeInfo.MinX[i],MyPano3DIamgeInfo.MaxX[i]);    
    }
    m_pMTKPano3dObj->Pano3DFeatureCtrl(PANO3D_FEATURE_SET_ENV_INFO, (void *)(&MyPano3DIamgeInfo), 0);
    
    for(int i=0; i<ImgNum; i++)
    {
        m_pMTKPano3dObj->Pano3DMain();
    }
    return err;	
}

/*******************************************************************************
* Get Stitch result
********************************************************************************/
MINT32
halPANO3D::mHal3dfGetStitchResult(void* Pano3dResult
)
{
	 MINT32 err = S_MAV_OK;
	 Pano3DResultInfo MyPano3DResultInfo;
	 m_pMTKPano3dObj->Pano3DFeatureCtrl(PANO3D_FEATURE_GET_RESULT, 0, (void *)(&MyPano3DResultInfo));
	 if(MyPano3DResultInfo.RetCode)
	 {	
	 	  CAM_LOGD("[mHalPANO3DGetResult] Stitch fail\n");	
	 	  err = E_MAV_ERR;
	 }
	 else
	 {
	 	  memcpy(Pano3dResult,(void *)&MyPano3DResultInfo,sizeof(Pano3DResultInfo));
	 	  CAM_LOGD("[mHalPANO3DGetResult] Stitch success\n");		 	   
	 }	
	 return err;	
}


/*******************************************************************************
* Get Warp result
********************************************************************************/
MINT32
halPANO3D::mHal3dfGetResult(MUINT32& MavResult,MUINT32& ClipWidth, MUINT32& ClipHeight
)
{
	 MINT32 err = S_MAV_OK;
	 WarpResultInfo MyResultInfo;
	 m_pMTKWarpObj->WarpFeatureCtrl(WARP_FEATURE_GET_RESULT, NULL, &MyResultInfo);
	 if(MyResultInfo.RetCode)
	 {	
	 	  CAM_LOGD("[mHalPANO3DGetResult] Warp fail\n");	
	 	  MavResult=0;
	 	  err = E_MAV_ERR;
	 }
	 else
	 {
	 	  CAM_LOGD("[mHalPANO3DGetResult] Warp success\n");	
	 	  MavResult=1; 	 	   
	 }	
	 return err;	
}
