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
#define LOG_TAG "mHalFacebeautify"

#include <mtkcam/Log.h>
#include <mtkcam/common/faces.h>
#include <mtkcam/algorithm/libfb/AppFaceBeauty.h>
#include "facebeautify_hal.h"

#define MY_LOGD(fmt, arg...)    CAM_LOGD(fmt,##arg) 
#define MY_LOGE(fmt, arg...)    CAM_LOGE(fmt,##arg) 

/*******************************************************************************
*
********************************************************************************/
static halFACEBEAUTIFYBase *pHalFACEBEAUTIFY = NULL;

/*******************************************************************************
*
********************************************************************************/
halFACEBEAUTIFYBase*
halFACEBEAUTIFY::
getInstance()
{
    MY_LOGD("[halFACEBEAUTIFY] getInstance \n");
    if (pHalFACEBEAUTIFY == NULL) {
        pHalFACEBEAUTIFY = new halFACEBEAUTIFY();
    }
    return pHalFACEBEAUTIFY;
}

/*******************************************************************************
*
********************************************************************************/
void   
halFACEBEAUTIFY::
destroyInstance() 
{
    if (pHalFACEBEAUTIFY) {
        delete pHalFACEBEAUTIFY;
    }
    pHalFACEBEAUTIFY = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halFACEBEAUTIFY::halFACEBEAUTIFY()
{
    m_pMTKFacebeautifyObj = NULL; 
     
    if (m_pMTKFacebeautifyObj) {
        MY_LOGD("[mHalFacebeautifyInit] Err, Init has been called \n");
    }    

    /*  Create MTKFacebeautify Interface  */
    m_pMTKFacebeautifyObj = MTKFaceBeauty::createInstance(DRV_FACEBEAUTY_OBJ_SW);
    if(!m_pMTKFacebeautifyObj)
    {
        MY_LOGD("[mHalFacebeautifyInit] createInstance fail\n");
    }
    else
        MY_LOGD("[mHalFacebeautifyInit] createInstance done\n");
}

/*******************************************************************************
*                                            
********************************************************************************/
halFACEBEAUTIFY::~halFACEBEAUTIFY()
{    
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFACEBEAUTIFY::mHalFacebeautifyInit(void* FaceBeautyEnvInfo
)
{	  
    MRESULT Retcode = S_FACEBEAUTY_OK;
    
    MY_LOGD("[mHalFacebeautifyInit] + \n");
    
    if(sizeof(MTKPipeFaceBeautyResultInfo)!=sizeof(MTKFaceBeautyResultInfo))
    {
        MY_LOGD("[mHalFacebeautifyInit] MTKPipeFaceBeautyResultInfo size not match \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }
        
    if(sizeof(MTKPipeFaceBeautyEnvInfo)!=sizeof(MTKFaceBeautyEnvInfo))
    {
        MY_LOGD("[mHalFacebeautifyInit] MTKPipeFaceBeautyEnvInfo size not match \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }
    
    if(sizeof(MTKPipeFaceBeautyTuningPara)!=sizeof(MTKFaceBeautyTuningPara))
    {
        MY_LOGD("[mHalFacebeautifyInit] MTKPipeFaceBeautyTuningPara size not match \n");
        Retcode = E_FACEBEAUTY_ERR;
        return Retcode;
    }    
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyInit(FaceBeautyEnvInfo, 0);        

    MY_LOGD("[mHalFacebeautifyInit] - RC %d \n",Retcode);
    
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFACEBEAUTIFY::mHalFacebeautifyUninit(
)
{
    MY_LOGD("[mHalFacebeautifyUninit] \n");
    
    if (m_pMTKFacebeautifyObj) {
        m_pMTKFacebeautifyObj->FaceBeautyExit();
        m_pMTKFacebeautifyObj->destroyInstance();
    }
    m_pMTKFacebeautifyObj = NULL;

    return S_FACEBEAUTY_OK;
}

MINT32 
halFACEBEAUTIFY::getWorkingBuffSize(int SrcImgWidth, int SrcImgHeight,
                                        int Step2SrcImgWidth, int Step2SrcImgHeight,
                                        int Step1SrcImgWidth, int Step1SrcImgHeight)
{
    MTKFaceBeautyGetProcInfo FaceBeautyGetProcInfo;
    MRESULT Retcode = S_FACEBEAUTY_OK;
    MY_LOGD("[getWorkingBuffSize] SrcImgWidth %d SrcImgHeight %d Step2SrcImgWidth %d Step2SrcImgHeight %d Step1SrcImgWidth %d Step1SrcImgHeight %d\n",SrcImgWidth,SrcImgHeight,Step2SrcImgWidth,Step2SrcImgHeight,Step1SrcImgWidth,Step1SrcImgHeight);
    FaceBeautyProcInfo.Step1SrcImgWidth = Step1SrcImgWidth;
    FaceBeautyProcInfo.Step1SrcImgHeight = Step1SrcImgHeight;
    FaceBeautyProcInfo.Step2SrcImgWidth = Step2SrcImgWidth;
    FaceBeautyProcInfo.Step2SrcImgHeight = Step2SrcImgHeight;
    FaceBeautyProcInfo.SrcImgWidth = SrcImgWidth;
    FaceBeautyProcInfo.SrcImgHeight = SrcImgHeight;
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_PROC_INFO, &FaceBeautyProcInfo, &FaceBeautyGetProcInfo);
    
    return FaceBeautyGetProcInfo.WorkingBufferSize;
}
    

/*******************************************************************************
*
********************************************************************************/
MINT32
halFACEBEAUTIFY::mHalSTEP2(void* ImgSrcAddr, void* FaceMetadata, void* FaceBeautyResultInfo
)
{	  
    MRESULT Retcode = S_FACEBEAUTY_OK;
    MY_LOGD("[mHalSTEP2] + \n");    
       
    //Set fd info
    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP2] cancel \n");
        Retcode = E_FACEBEAUTY_ERR; 
        return Retcode;
    }
    MtkCameraFaceMetadata *mFaceMetadata = (MtkCameraFaceMetadata*)FaceMetadata;
    MY_LOGD("[mHalSTEP2] number_of_faces %d ImgSrcAddr %x",mFaceMetadata->number_of_faces,(MUINT8*)ImgSrcAddr);
    MtkCameraFace* FDReslutInfo=(MtkCameraFace*)mFaceMetadata->faces;
    MtkFaceInfo* MTKFDInfo=(MtkFaceInfo*)mFaceMetadata->posInfo;    
        
    FaceBeautyProcInfo.Step2SrcImgAddr = (MUINT8*)ImgSrcAddr;   
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP2;
    FaceBeautyProcInfo.FaceCount = mFaceMetadata->number_of_faces;
       
    for(int i=0;i<mFaceMetadata->number_of_faces;i++)
    {
        FaceBeautyProcInfo.FDLeftTopPointX1[i] = mFaceMetadata->faces[i].rect[0];
        FaceBeautyProcInfo.FDLeftTopPointY1[i] = mFaceMetadata->faces[i].rect[1];
        FaceBeautyProcInfo.FDBoxSize[i] = (mFaceMetadata->faces[i].rect[2]-mFaceMetadata->faces[i].rect[0]);
        FaceBeautyProcInfo.FDPose[i] = mFaceMetadata->posInfo[i].rip_dir;
        MY_LOGD("[mHalSTEP2] x0 %d y0 %d w %d pose %d",FaceBeautyProcInfo.FDLeftTopPointX1[i],FaceBeautyProcInfo.FDLeftTopPointY1[i],FaceBeautyProcInfo.FDBoxSize[i],FaceBeautyProcInfo.FDPose[i]);
    }  
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);

    MY_LOGD("[mHalSTEP2] - Retcode %d",Retcode); 
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halFACEBEAUTIFY::mHalSTEP1(void* ImgSrcAddr, void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP1] + \n");            
    MRESULT Retcode = S_FACEBEAUTY_OK;  
    
    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP1] cancel \n");
        Retcode = E_FACEBEAUTY_ERR; 
        return Retcode;
    }
    
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP1;
    FaceBeautyProcInfo.Step1SrcImgAddr = (MUINT8*)ImgSrcAddr;
    MY_LOGD("[mHalSTEP1] SrcImgAddr 0x%x \n",(MUINT32)ImgSrcAddr);

    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    MY_LOGD("[mHalSTEP1] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halFACEBEAUTIFY::mHalSTEP3(void* ImgSrcAddr, void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP3] + \n");            
    MRESULT Retcode = S_FACEBEAUTY_OK;  
    
    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP3] cancel \n");
        Retcode = E_FACEBEAUTY_ERR; 
        return Retcode;
    }
    
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP3;
    FaceBeautyProcInfo.Step2SrcImgAddr = (MUINT8*)ImgSrcAddr;
    MY_LOGD("[mHalSTEP3] SrcImgAddr 0x%x \n",(MUINT32)ImgSrcAddr);

    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    MY_LOGD("[mHalSTEP3] - ret %d\n",Retcode);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    MY_LOGD("[mHalSTEP3] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halFACEBEAUTIFY::mHalSTEP4(void* ImgSrcAddr,void* BlurResultAdr,void* AplhaMapBuffer,void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP4] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;
	  
	  if(CANCEL)
    {
        MY_LOGD("[mHalSTEP4] cancel \n");
        Retcode = E_FACEBEAUTY_ERR; 
        return Retcode;
    }
    
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP4;       
    FaceBeautyProcInfo.SrcImgAddr = (MUINT8*)ImgSrcAddr;
    FaceBeautyProcInfo.Step4SrcImgAddr_1 = (MUINT8*)BlurResultAdr;
    FaceBeautyProcInfo.Step4SrcImgAddr_2 = (MUINT8*)AplhaMapBuffer;
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);  
    
    MY_LOGD("[mHalSTEP4] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halFACEBEAUTIFY::mHalSTEP5(void* ImgSrcAddr,void* AplhaMapColorBuffer,void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP5] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;
    
    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP5] cancel \n");
        Retcode = E_FACEBEAUTY_ERR; 
        return Retcode;
    }
    
	  MTKPipeFaceBeautyResultInfo* FaceBeautifyResultInfo = (MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;	  
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP5;
    FaceBeautyProcInfo.Step5SrcImgAddr = (MUINT8*)AplhaMapColorBuffer;
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    MY_LOGD("[mHalSTEP5] - ret %d\n",Retcode);
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halFACEBEAUTIFY::mHalSTEP6(void* ImgSrcAddr,void* WarpWorkBufAdr,void* FaceBeautyResultInfo
)
{
    MY_LOGD("[mHalSTEP6] + \n");
    MRESULT Retcode = S_FACEBEAUTY_OK;
    
    if(CANCEL)
    {
        MY_LOGD("[mHalSTEP6] cancel \n");
        Retcode = E_FACEBEAUTY_ERR; 
        return Retcode;
    }
    
	  MTKPipeFaceBeautyResultInfo* FaceBeautifyResultInfo = (MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;	  
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_STEP6;
    FaceBeautyProcInfo.Step6TempAddr = (MUINT8*)WarpWorkBufAdr;
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    MY_LOGD("[mHalSTEP6] - ret %d\n",Retcode);
    return Retcode;
}
