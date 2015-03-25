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
/*
** $Log: fd_hal_base.h $
 *
*/

#ifndef _AUTORAMA_HAL_BASE_H_
#define _AUTORAMA_HAL_BASE_H_

/*******************************************************************************
*
********************************************************************************/

#define AUTORAMA_MAX_WIDTH                    (30720)                      // max output image width
#define MOTION_MAX_IN_WIDTH                   (320)                       // max input image width
#define MOTION_MAX_IN_HEIGHT                  (240)                       // max input image height
#define MOTION_PIPE_WORKING_BUFFER_SIZE       (MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 3)
#define OVERLAP_RATIO                         (32) 

typedef enum HalAUTORAMAObject_s {
    HAL_AUTORAMA_OBJ_NONE = 0,
    HAL_AUTORAMA_OBJ_AUTO,
    HAL_AUTORAMA_OBJ_UNKNOWN = 0xFF,
} HalAUTORAMAObject_e;
//
#define HAL_AUTORAMA_RESET_PARAM    0x00
#define HAL_AUTORAMA_SET_DST_DIR    0x10   // Direction (direction)
#define HAL_AUTORAMA_SET_DST_BUF    0x11   // Buffer (buffer address, size)
#define HAL_AUTORAMA_SET_SRC_DIM    0x20   // Dimension (width, height)
#define HAL_AUTORAMA_SET_SRC_BUF    0x21   // Buffer (index, buffer address)
#define HAL_AUTORAMA_GET_RESULT     0x40   // result (buffer address, width, height)

typedef enum
{   
    MTKPIPEAUTORAMA_DIR_RIGHT=0,                // panorama direction is right
    MTKPIPEAUTORAMA_DIR_LEFT,                   // panorama direction is left
    MTKPIPEAUTORAMA_DIR_UP,                     // panorama direction is up
    MTKPIPEAUTORAMA_DIR_DOWN,                   // panorama direction is down
    MTKPIPEAUTORAMA_DIR_NO                      // panorama direction is not decided
} MTKPIPEAUTORAMA_DIRECTION_ENUM;

struct MTKPipeAutoramaEnvInfo
{
    MUINT16  SrcImgWidth;                      // input image width
    MUINT16  SrcImgHeight;                     // input image height
    MUINT16  MaxPanoImgWidth;                  // final output image max width
    MUINT8   MaxSnapshotNumber;                // max capture image num
    MUINT32  WorkingBufAddr;                   // working buffer
    MUINT32  WorkingBufSize;                   // working buffer size
    MBOOL    FixAE;                            // fixAe or not, if false, system should provide the ev information
    MUINT32  FocalLength;                      // lens focal length depends on lens, normally, this value can be set 750
    MBOOL    GPUWarp;                          // enable gpu cylindrical projection or not
};

struct MTKPipeMotionTuningPara
{
    MINT32  MaxMV;                      // maximum motion vertor        (default: 8)
    MINT32  StepLB;                     // upper bound of frame step    (default: 3)
    MINT32  StepUB;                     // lower bound of frame step    (default: 8)
    MINT32  ToleranceX;                 // horizontal tolerance of motion vector deviation
    MINT32  ToleranceY;                 // vertical tolerance of motion vector deviation
    MINT32  OverlapRatio;               // overlap ratio between captured images
};

struct MTKPipeMotionEnvInfo
{
    MUINT32 SrcImgWidth;                // source image width
    MUINT32 SrcImgHeight;               // source image height
    MUINT32 WorkingBuffAddr;            // working buffer address
    MUINT32 WorkingBuffSize;            // working buffer size
    MTKPipeMotionTuningPara *pTuningPara;     // tuning parameters
};

struct MTKPipeAutoramaResultInfo
{
    MUINT32  ImgBufferAddr;    // output image address, currently, only output YUV420
    MUINT16  ImgWidth;         // output image width
    MUINT16  ImgHeight;        // output image height
};

struct AutoramaMotionResult
{
    /* motion tracking results */
    MINT16      MV_X;                   // horizontal accumulated motion
    MINT16      MV_Y;                   // vertical accumulated motion
    MBOOL       ReadyToShot;            // ready to shot flag (0/1 = not ready/ready)
    MINT32      RetCode;                // returned code of state machine
    MUINT8      ErrPattern;             // returned error/warning bit pattern 
                                        // bit 0: low trust value (set if warning)
                                        // bit 1: large motion vector (set if warning)
                                        // bit 2: vertical shake (set if warning)
    MTKPIPEAUTORAMA_DIRECTION_ENUM Direction;   // panorama direction enum
};
/*******************************************************************************
*
********************************************************************************/
class halAUTORAMABase {
public:
    //
    MINT32 gImgEV[9];
    static halAUTORAMABase* createInstance(HalAUTORAMAObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halAUTORAMABase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaInit () -
    //! \brief init autorama 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalAutoramaInit(MTKPipeAutoramaEnvInfo AutoramaInitInData, MTKPipeMotionEnvInfo MotionInitInfo) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaUninit () -
    //! \brief Autorama uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalAutoramaUninit() {return 0;}
       
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaGetParam () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalAutoramaGetResult(MTKPipeAutoramaResultInfo* ResultInfo) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaCalcStitch () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalAutoramaCalcStitch(void* SrcImg,MINT32 gEv,MTKPIPEAUTORAMA_DIRECTION_ENUM DIRECTION) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaDoStitch () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalAutoramaDoStitch() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaDoMotion () -
    //! \brief check motion and capture 
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalAutoramaDoMotion(MUINT32* ImgSrc,MUINT32* MotionResult) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAutoramaGetWokSize () -
    //! \brief get working buffer size 
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalAutoramaGetWokSize(int SrcWidth, int SrcHeight, int ShotNum, int &WorkingSize) {return 0;}
};

class halAUTORAMATmp : public halAUTORAMABase {
public:
    //
    static halAUTORAMABase* getInstance();
    virtual void destroyInstance();
    //
    halAUTORAMATmp() {}; 
    virtual ~halAUTORAMATmp() {};
};

#endif

