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
#ifndef _MTK_CAMERA_INC_CAMPIPE_PORTS_H_
#define _MTK_CAMERA_INC_CAMPIPE_PORTS_H_
//
#include <mtkcam/common/hw/hwstddef.h>

using namespace NSCamHW; 
/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {
////////////////////////////////////////////////////////////////////////////////

/**  
 * @enum EPipeSignal
 * @brief Pipe Signal event
 */
enum EPipeSignal 
{
    EPipeSignal_NONE    = 0x0000,           /*!< signal None */  
    EPipeSignal_SOF     = 0x0001,           /*!< signal Start Of Frame */ 
    EPipeSignal_EOF     = 0x0002,           /*!< signal End of Frame */  
};

/*******************************************************************************
* 
********************************************************************************/

/**  
 * @enum EPortType
 * @brief Pipe Port Type.
 */
enum EPortType
{
    EPortType_Sensor,                        /*!< Sensor Port Type */  
    EPortType_MemoryIn,                   /*!< Memory In Port Type */   
    EPortType_MemoryOut,                /*!< Memory Out Port Type */    
};


/**  
 * @struct PortID
 * @brief Pipe Port ID (Descriptor).
 *
 */
struct PortID
{
public:     //// fields.
    /**
      * @var type 
      * EPortType
      */ 
    EPortType     type    :   8;      //  
    /**
      * @var index 
      *  port index
      */     
    MUINT32       index   :   8;      // 
    /**
      * @var inout 
      * 0:in/1:out
      */     
    MUINT32       inout   :   1;      //  
    /**
      * @var reserved 
      * reserved for future use. 
      */     
    MUINT32       reserved:   15;
    //
public:     //// constructors.
    PortID(
        EPortType const _eType  = EPortType_MemoryOut, 
        MUINT32 const _index    = 0, 
        MUINT32 const _inout    = 0
    )
    {
        type    = _eType;
        index   = _index;
        inout   = _inout;
        reserved= 0;
    }
    //
public:     //// operations.
    MUINT32 operator()() const
    {
        return  *reinterpret_cast<MUINT32 const*>(this);
    }
};


/**  
 * @struct PortInfo
 * @brief Pipe Port Info.
 *
 */
struct PortInfo : public PortID
{
public:     //// constructors.
    PortInfo()
        : PortID()
    {
    }
    //
    //
    PortInfo(PortID const& _PortID)
        : PortID(_PortID)
    {
    }
    //
};

/**  
 * @struct SensorPortInfo
 * @brief Sensor port configuration 
 *
 */
struct SensorPortInfo : public PortInfo
{
public:
    /**
      * @var u4DeviceID 
      * Sensor device id 
      */ 	
    MUINT32     u4DeviceID;     
    /**
      * @var u4Scenario 
      * Sensor scenario, preview/capture/video ...etc 
      */     
    MUINT32     u4Scenario; 
    /**
      * @var u4Bitdepth 
      * The sensor raw bitdepth, the value is 8, 10 
      */     
    MUINT32     u4Bitdepth; 
    /**
      * @var fgBypassDelay 
      * Bypass sensor delay if ture, useful when sensor chnage mode 
      * such as from preview mode to capture mode 
      */         
    MBOOL       fgBypassDelay;    //  
    /**
      * @var fgBypassScenaio 
      * Bypass to set the scenario, if the user don't want to set 
      * sensor setting again can set it to true. 
      */     
    MBOOL       fgBypassScenaio;
    /**
      * @var u4RawType 
      * The raw type, 0: pure raw, 1: pre-process raw 
      */     
    MUINT32    u4RawType; 
    

public:     //// constructors.
    SensorPortInfo()
        : PortInfo(PortID(EPortType_Sensor, 0, 0)) 
        , u4DeviceID(0)
        , u4Scenario(0)
        , u4Bitdepth(8)
        , fgBypassDelay(MFALSE)
        , fgBypassScenaio(MFALSE)
        , u4RawType(0)
    {
    }

    //
    SensorPortInfo(
        MUINT32 const _u4DeviceID, 
        MUINT32 const _u4Scenario, 
        MUINT32 const _u4Bitdepth, 
        MBOOL const _fgBypassDelay, 
        MBOOL const _fgBypassScenaio, 
        MUINT32 const _u4RawType = 0        
        )
        : PortInfo(PortID(EPortType_Sensor, 0, 0))
        , u4DeviceID(_u4DeviceID)
        , u4Scenario(_u4Scenario)
        , u4Bitdepth(_u4Bitdepth)
        , fgBypassDelay(_fgBypassDelay)
        , fgBypassScenaio(_fgBypassScenaio)
        , u4RawType(_u4RawType)
    {
    }
};


/**  
 * @struct MemoryInPortInfo
 * @brief Memory In Port Info.
 *
 */
struct MemoryInPortInfo : public PortInfo, public ImgInfo
{
public:
    /**
      * @var u4Offset 
      * The offset of the memory 
      */ 		
    MUINT32     u4Offset; 
    /**
      * @var u4Stride 
      * The stride of the input image width stride of each plane 
      */ 	    
    MUINT32     u4Stride[3]; 
    /**
      * @var rCrop 
      * The crop information of the input image. 
      */ 	    
    Rect        rCrop; 

public:     //// constructors.
    MemoryInPortInfo()
        : PortInfo(PortID(EPortType_MemoryIn, 0, 0)) 
        , ImgInfo()
        , u4Offset(0)
        , rCrop()
    {
        u4Stride[0] = u4Stride[1] = u4Stride[2] = 0; 
    }
    //
    MemoryInPortInfo(MUINT32 const _u4Offset, MUINT32 const _u4Stride[3], Rect const _rCrop)
        : PortInfo(PortID(EPortType_MemoryIn, 0, 0))
        , ImgInfo()
        , u4Offset(_u4Offset)
        , rCrop(_rCrop.x, _rCrop.y, _rCrop.w, _rCrop.h)
    {
        u4Stride[0] = _u4Stride[0]; 
        u4Stride[1] = _u4Stride[1]; 
        u4Stride[2] = _u4Stride[2]; 
    }
    //
    MemoryInPortInfo(ImgInfo const _rImgInfo, MUINT32 const _u4Offset, MUINT32 const _u4Stride[3], Rect const _rCrop)
        : PortInfo(PortID(EPortType_MemoryIn, 0, 0))
        , ImgInfo(_rImgInfo.eImgFmt, _rImgInfo.u4ImgWidth, _rImgInfo.u4ImgHeight)
        , u4Offset(_u4Offset)
        , rCrop(_rCrop.x, _rCrop.y, _rCrop.w, _rCrop.h)
    {        
        u4Stride[0] = _u4Stride[0]; 
        u4Stride[1] = _u4Stride[1]; 
        u4Stride[2] = _u4Stride[2]; 
    }
};

/**  
 * @struct MemoryOutPortInfo
 * @brief Memory Out Port Info.
 *
 */
struct MemoryOutPortInfo : public PortInfo, public ImgInfo
{
public:
    /**
      * @var u4Stride 
      * The stride of the output image width stride of each plane 
      */ 	  	
    MUINT32     u4Stride[3]; 
    /**
      * @var u4Rotation 
      * The rotation operation of the output image 
      */ 	    
    MUINT32     u4Rotation; 
    /**
      * @var u4Flip 
      * The flip operation of the output image 
      */ 	    
    MUINT32     u4Flip; 

public:     //// constructors.
    MemoryOutPortInfo()
        : PortInfo(PortID(EPortType_MemoryOut, 0, 1)) 
        , ImgInfo()
        , u4Rotation(0)
        , u4Flip(0) 
    {
        u4Stride[0] = u4Stride[1] = u4Stride[2] = 0; 
    }
    //
    MemoryOutPortInfo(MUINT32 const _u4Stride[3], MUINT32 const _u4Rotation, MUINT32 const _u4Flip)        
        : PortInfo(PortID(EPortType_MemoryOut, 0, 1))
        , ImgInfo()
        , u4Rotation(_u4Rotation)
        , u4Flip(_u4Flip)
    {
        u4Stride[0] = _u4Stride[0]; 
        u4Stride[1] = _u4Stride[1]; 
        u4Stride[2] = _u4Stride[2]; 
    }

    //
    MemoryOutPortInfo(ImgInfo const _rImgInfo, MUINT32 const _u4Stride[3], MUINT32 const _u4Rotation, MUINT32 const _u4Flip)
        : PortInfo(PortID(EPortType_MemoryOut, 0, 0))
        , ImgInfo(_rImgInfo.eImgFmt, _rImgInfo.u4ImgWidth, _rImgInfo.u4ImgHeight)
        , u4Rotation(_u4Rotation)
        , u4Flip(_u4Flip)
    {        
        u4Stride[0] = _u4Stride[0]; 
        u4Stride[1] = _u4Stride[1]; 
        u4Stride[2] = _u4Stride[2]; 
    }
};


/**  
 * @struct PortProperty
 * @brief Pipe Port Property 
 *
 */
struct PortProperty: public PortID 
{
public: 
    /**
      * @var u4SupportFmt 
      * The port support format 
      */ 	
    MUINT32 u4SupportFmt; 
    /**
      * @var fgIsSupportRotate 
      * Is the port support rotation operation. 
      */     
    MBOOL fgIsSupportRotate; 
    /**
      * @var fgIsSupportFlip 
      * Is the port support flip operation. 
      */     
    MBOOL fgIsSupportFlip; 

public: 
    PortProperty(MUINT32 const _u4SupportFmt = eImgFmt_UNKNOWN, 
                 MBOOL const _fgIsSupportRotate = MFALSE,
                 MBOOL const _fgIsSupportFlip = MFALSE
    ) 
        : PortID()
        , u4SupportFmt(_u4SupportFmt)
        , fgIsSupportRotate(_fgIsSupportRotate)
        , fgIsSupportFlip(_fgIsSupportFlip)
    {
    }

    PortProperty(PortID const & _PortID, 
                 MUINT32 const _u4SupportFmt = eImgFmt_UNKNOWN, 
                 MBOOL const _fgIsSupportRotate = MFALSE, 
                 MBOOL const _fgIsSupportFlip = MFALSE
    )
        : PortID(_PortID)
        , u4SupportFmt(_u4SupportFmt) 
        , fgIsSupportRotate(_fgIsSupportRotate)
        , fgIsSupportFlip(_fgIsSupportFlip)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamPipe
#endif  //  _MTK_CAMERA_INC_CAMPIPE_PORTS_H_

