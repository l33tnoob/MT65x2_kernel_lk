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
** $Log: fd_hal_base.h $
 *
*/

#ifndef _FD_HAL_BASE_H_
#define _FD_HAL_BASE_H_

//#include <system/camera.h>
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>
#include <mtkcam/common/faces.h>
/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef unsigned char MUINT8;
typedef signed int    MBOOL;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif


/*******************************************************************************
*
********************************************************************************/
enum HalFDObject_e {
    HAL_FD_OBJ_NONE = 0,
    HAL_FD_OBJ_SW,
    HAL_FD_OBJ_HW,
    HAL_FD_OBJ_FDFT_SW,
    HAL_FD_OBJ_UNKNOWN = 0xFF
} ;


struct FD_RESULT {

    MINT32 rect[4];
    MINT32 score;
    MINT32 rop_dir;
    MINT32 rip_dir;
};

/*******************************************************************************
*
********************************************************************************/
class halFDBase {
public:
    //
    static halFDBase* createInstance(HalFDObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halFDBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDInit () -
    //! \brief init face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH, MUINT32 WorkingBuffer, MUINT32 WorkingBufferSize, MBOOL   SWResizerEnable) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDDo () -
    //! \brief process face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDDo(MUINT32 ScaleImages, MUINT32 ImageBuffer1, MUINT32 ImageBuffer2, MBOOL SDEnable, MINT32 rRotation_Info) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDUninit () -
    //! \brief fd uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDDrawFaceRect () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDDrawFaceRect(MUINT8 *pbuf) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSDDrawFaceRect () -
    //! \brief draw  smile detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halSDDrawFaceRect(MUINT8 *pbuf) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDGetFaceInfo () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual  MINT32 halFDGetFaceInfo(MtkCameraFaceMetadata *fd_info_result) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDGetFaceResult () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDGetFaceResult(MtkCameraFaceMetadata * fd_result) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDSetDispInfo () -
    //! \brief set display info
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFDSetDispInfo(MUINT32 x, MUINT32 y, MUINT32 w, MUINT32 h, MUINT32 rotate, MUINT32 sensor_rotate, MINT32 CameraId) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSetDetectPara () -
    //! \brief set detection parameter 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halSetDetectPara(MUINT8 Para) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSDGetSmileResult () -
    //! \brief get smile detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halSDGetSmileResult( ) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDM4URegister () -
    //! \brief register buffer 
    //
    /////////////////////////////////////////////////////////////////////////   
    virtual MINT32 halFDM4URegister(MUINT8 *pbuf,MUINT32 BufSize,MUINT8 BufCunt)  {return 0;}
    
   /////////////////////////////////////////////////////////////////////////
    //
    // halFDBufferCreate () -
    //! \brief create face buffer
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDBufferCreate(MUINT8 *dstAddr, MUINT8 *srcAddr, MUINT8  ucBufferGroup) {return 0;}    
    
};

class halFDTmp : public halFDBase {
public:
    //
    static halFDBase* getInstance();
    virtual void destroyInstance();
    //
    halFDTmp() {}; 
    virtual ~halFDTmp() {};
};

#endif

