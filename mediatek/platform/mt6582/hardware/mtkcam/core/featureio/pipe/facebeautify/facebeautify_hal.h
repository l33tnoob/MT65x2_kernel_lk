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
/*
** $Log: facebeautify_hal.h $
 *
*/

#ifndef _FACEBEAUTIFY_HAL_H_
#define _FACEBEAUTIFY_HAL_H_

#include <mtkcam/featureio/facebeautify_hal_base.h>
#include <mtkcam/algorithm/libfb/MTKFaceBeauty.h>

class MTKFacebeautify;

/*******************************************************************************
*
********************************************************************************/
class halFACEBEAUTIFY: public halFACEBEAUTIFYBase 
{
public:
    //
    static halFACEBEAUTIFYBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////                       
    halFACEBEAUTIFY(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halFACEBEAUTIFY();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFACEBEAUTIFYInit () -
    //! \brief init facebeautify
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFacebeautifyInit(void* FaceBeautyEnvInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFacebeautifyUninit () -
    //! \brief facebeautify uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFacebeautifyUninit();
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP2 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP2(void* ImgSrcAddr, void* FaceMetadata, void* FaceBeautyResultInfo);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP3 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP3(void* ImgSrcAddr, void* FaceBeautyResultInfo);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP1 () -
    //! 
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalSTEP1(void* ImgSrcAddr, void* FaceBeautyResultInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP4 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP4(void* ImgSrcAddr,void* BlurResultAdr,void* AplhaMapBuffer,void* FaceBeautyResultInfo);
   
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP5 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP5(void* ImgSrcAddr,void* AplhaMapColorBuffer,void* FaceBeautyResultInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP6 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP6(void* ImgSrcAddr,void* WarpWorkBufAdr,void* FaceBeautyResultInfo);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // getWorkingBuffSize () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////         
    virtual MINT32 getWorkingBuffSize(int SrcImgWidth, int SrcImgHeight, int Step2SrcImgWidth, int Step2SrcImgHeight, int Step1SrcImgWidth, int Step1SrcImgHeight);
       
protected:


private:
    MTKFaceBeauty* m_pMTKFacebeautifyObj;
    MTKFaceBeautyProcInfo FaceBeautyProcInfo;
};

#endif

