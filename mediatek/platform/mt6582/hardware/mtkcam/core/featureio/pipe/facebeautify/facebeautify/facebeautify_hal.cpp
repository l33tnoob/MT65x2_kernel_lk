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

//#include <mhal/inc/MediaLog.h>
//#include <mhal/inc/MediaAssert.h>
//#include <mhal/inc/MediaHal.h>
#include <cutils/xlog.h>

#include <mtkcam/algorithm/libfb/AppFaceBeauty.h>
#include "facebeautify_hal.h"
#include <mtkcam/featureio/facebeautify_hal_base.h>
#include <mhal/inc/camera.h>

#define MHAL_LOG(fmt, arg...)    XLOGD(fmt,##arg) 

#define MHAL_ASSERT(x, str); \
    if (x) {} \
    else {printf("[Assert %s, %d]: %s", __FILE__, __LINE__, str); XLOGE("[Assert %s, %d]: %s", __FILE__, __LINE__, str); while(1); } 

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
    MHAL_LOG("[halFACEBEAUTIFY] getInstance \n");
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
    
    MHAL_LOG("[mHalFacebeautifyInit] \n");
    
   
    if (m_pMTKFacebeautifyObj) {
        Retcode = E_FACEBEAUTY_ERR;
        MHAL_LOG("[mHalFacebeautifyInit] Err, Init has been called \n");
    }    

    /*  Create MTKFacebeautify Interface  */
    m_pMTKFacebeautifyObj = MTKFaceBeauty::createInstance(DRV_FACEBEAUTY_OBJ_SW);
    MHAL_ASSERT(m_pMTKFacebeautifyObj != NULL, "Err");


    Retcode = m_pMTKFacebeautifyObj->FaceBeautyInit(FaceBeautyEnvInfo, 0);
    /*  Set Pano Driver Environment Parameters */

    
      
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFACEBEAUTIFY::mHalFacebeautifyUninit(
)
{
    MHAL_LOG("[mHalFacebeautifyUninit] \n");
    
    if (m_pMTKFacebeautifyObj) {
        m_pMTKFacebeautifyObj->FaceBeautyExit();
        m_pMTKFacebeautifyObj->destroyInstance();
    }
    m_pMTKFacebeautifyObj = NULL;

    return S_FACEBEAUTY_OK;
}

MINT32 
halFACEBEAUTIFY::mHalCreateAlphaMap(void* ImgSrcAddr,void* ImgDsAddr,void* ImgBlurAddr,void* FDResult,void* FDInfo,void* FaceBeautyResultInfo
)
{
    MHAL_LOG("[mHalCreateAlphaMap] \n");    
    camera_face_metadata_mtk* FDReslutInfo=(camera_face_metadata_mtk*)FDResult;
    mtk_face_info* MTKFDInfo=(mtk_face_info*)FDInfo;
    MRESULT Retcode;  
    
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKPIPEFACEBEAUTY_CTRL_ALPHA_MAP;
    FaceBeautyProcInfo.SrcImgDsAddr = (MUINT8*)ImgDsAddr;
    FaceBeautyProcInfo.SrcImgAddr = (MUINT8*)ImgSrcAddr;
    FaceBeautyProcInfo.SrcImgBlurAddr = (MUINT8*)ImgBlurAddr;
    MHAL_LOG("[mHalCreateAlphaMap] SrcImgDsAddr 0x%x SrcImgAddr 0x%x SrcImgBlurAddr 0x%x\n",(MUINT32)FaceBeautyProcInfo.SrcImgDsAddr,(MUINT32)FaceBeautyProcInfo.SrcImgAddr,(MUINT32)FaceBeautyProcInfo.SrcImgBlurAddr);
    MHAL_LOG("[mHalCreateAlphaMap] number_of_faces %d \n",FDReslutInfo->number_of_faces);
    for(int i=0;i<FDReslutInfo->number_of_faces;i++)
    {
        FaceBeautyProcInfo.FDLeftTopPointX1[i] = FDReslutInfo->faces[i].rect[0];
        FaceBeautyProcInfo.FDLeftTopPointY1[i] = FDReslutInfo->faces[i].rect[1];
        FaceBeautyProcInfo.FDBoxSize[i] = (FDReslutInfo->faces[i].rect[2]-FDReslutInfo->faces[i].rect[0]);
        FaceBeautyProcInfo.FDPose[i] = MTKFDInfo[i].rip_dir;
        MHAL_LOG("[mHalCreateAlphaMap] x0 %d y0 %d w %d pose %d \n",FaceBeautyProcInfo.FDLeftTopPointX1[i],FaceBeautyProcInfo.FDLeftTopPointY1[i],FaceBeautyProcInfo.FDBoxSize[i],FaceBeautyProcInfo.FDPose[i]);
    }
    FaceBeautyProcInfo.FaceCount = FDReslutInfo->number_of_faces;
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    return Retcode;
}

MINT32 
halFACEBEAUTIFY::mHalBLENDTEXTURE(void* BlendResultAdr,void* BlendYSResultAdr,void* AplhaMapBuffer,void* FaceBeautyResultInfo
)
{
	  MRESULT Retcode;
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKPIPEFACEBEAUTY_CTRL_BLEND_TEXTURE_IMG;
    FaceBeautyProcInfo.AlphaMap = (MUINT8*)AplhaMapBuffer;
    FaceBeautyProcInfo.TexBlendResultAddr = (MUINT8*)BlendResultAdr;
    FaceBeautyProcInfo.TexBlendAndYSResultAddr = (MUINT8*)BlendYSResultAdr;
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);  
    
    return Retcode;
}

MINT32 
halFACEBEAUTIFY::mHalBLENDCOLOR(void* FinalAdr,void* AplhaMapColorBuffer,void* PCABuffer,void* FaceBeautyResultInfo
)
{
	  MTKPipeFaceBeautyResultInfo* FaceBeautifyResultInfo = (MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
	  MRESULT Retcode;
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKPIPEFACEBEAUTY_CTRL_BLEND_COLOR_IMG;
    FaceBeautyProcInfo.PCAImgAddr = (MUINT8*)PCABuffer;
    FaceBeautyProcInfo.AlphaMapColor = (MUINT8*)AplhaMapColorBuffer;
    FaceBeautyProcInfo.ColorBlendResultAddr = (MUINT8*)FinalAdr;
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    return Retcode;
}

MINT32 
halFACEBEAUTIFY::mHalWARPING(void* FinalAdr,void* BlendResultAdr,void* FaceBeautyResultInfo
)
{
	  MTKPipeFaceBeautyResultInfo* FaceBeautifyResultInfo = (MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
	  MRESULT Retcode;
    FaceBeautyProcInfo.FaceBeautyCtrlEnum = MTKFACEBEAUTY_CTRL_WARPING;
    FaceBeautyProcInfo.WarpingTempAddr = (MUINT8*)BlendResultAdr;
    FaceBeautyProcInfo.WarpingResultAddr = (MUINT8*)FinalAdr;
    
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_SET_PROC_INFO, &FaceBeautyProcInfo, 0);
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyMain();
    Retcode = m_pMTKFacebeautifyObj->FaceBeautyFeatureCtrl(MTKFACEBEAUTY_FEATURE_GET_RESULT, 0, FaceBeautyResultInfo);
    
    return Retcode;
}
