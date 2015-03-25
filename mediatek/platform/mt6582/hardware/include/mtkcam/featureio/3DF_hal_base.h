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

#ifndef _3DF_HAL_BASE_H_
#define _3DF_HAL_BASE_H_

/*******************************************************************************
*
********************************************************************************/

#define	MAV_PIPE_MAX_IMAGE_NUM	(25)	// maximum image number
#define Pipe_RANK        (3)
#define MOTION_MAX_IN_WIDTH         (320)                       // max input image width
#define MOTION_MAX_IN_HEIGHT        (240)                       // max input image height
#define OVERLAP_RATIO               (32)
#define PANO3DOVERLAP_RATIO         (58) 
#define PIPEPANO3D_MAX_IMG_NUM          (25)

typedef enum Hal3DFObject_s {
    HAL_3DF_OBJ_NONE = 0,
    HAL_MAV_OBJ_NORMAL,
    HAL_PANO3D_OBJ_NORMAL,
    HAL_3DF_OBJ_UNKNOWN = 0xFF,
} Hal3DFObject_e;

typedef enum
{   
    MTKMAV_DIR_RIGHT=0,
    MTKMAV_DIR_LEFT,
    MTKMAV_DIR_UP,
    MTKMAV_DIR_DOWN,
    MTKMAV_DIR_NO
} MTKMAV_DIRECTION_ENUM;

struct MAVMotionResultInfo
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
    MTKMAV_DIRECTION_ENUM Direction;   // panorama direction enum
};

struct MavPipeImageInfo
{
    MUINT32 				ImgAddr;
    MUINT16 				Width;					// input image width
    MUINT16 				Height;					// input image height
    MUINT32					AngleValueX;
    MUINT32					AngleValueY;  
    MFLOAT 					AngleValueZ;		// AngleValue  
    MINT32 					ClipX;				// Image Global Offset X
    MINT32	 				ClipY;				// Image Global Offset Y
    MINT32					MotionValue[2];    
    
    // for 3D Panorama
    MUINT16                 GridX;              // vertical offset in panorama space
    MUINT16                 MinX;               // x_start in panorama space
    MUINT16                 MaxX;               // x_end in panorama space
    MBOOL           ControlFlow;        // rectify once or not
};

struct MavPipeResultInfo
{
	  MINT32					RetCode;		// return warning
    MINT16 			  	ClipWidth;		// Image Result Width
    MINT16 			  	ClipHeight;		// Image Result Height
    MUINT16 				ViewIdx;		// Image Start View Index
    MUINT8                  ErrPattern;         // Returned error/Warning bit pattern
                                                // bit 0: lack of match points (set if error)
                                                // bit 1: small clip region (set if error)
                                                // bit 2: reverse order(set if warning) -> 0/1 = Left-To-Right/Right-To-Left
    MavPipeImageInfo		ImageInfo[MAV_PIPE_MAX_IMAGE_NUM];
    MFLOAT					      ImageHmtx[MAV_PIPE_MAX_IMAGE_NUM][Pipe_RANK][Pipe_RANK];   // 3x3 rectification matrix
};

struct PipePano3DResultInfo
{
    MUINT16                 PanoWidth;
    MUINT16                 PanoHeight;
    MUINT32                 LeftPanoImageAddr;
    MUINT32                 RightPanoImageAddr;
    MUINT32                 RetCode;
    MUINT8                  ErrPattern;         // Returned error/Warning bit pattern
                                                // bit 0: small overlap width (set if error)
                                                // bit 1: large seam change (set if error)
    MUINT32                 ClipX;              // horizontal offset of ROI
    MUINT32                 ClipY;              // vertical offset of ROI
    MUINT32                 ClipWidth;          // width of ROI
    MUINT32                 ClipHeight;         // height of ROI

    // Optimal Seam - debug purpose
    MINT32                 OptimalSeamLeft[PIPEPANO3D_MAX_IMG_NUM][600];
};

/*******************************************************************************
*
********************************************************************************/
class hal3DFBase {
public:
    static hal3DFBase* createInstance(Hal3DFObject_e eobject);
    virtual MVOID      destroyInstance() = 0;
    virtual ~hal3DFBase() {};
protected:
    

public:     
    virtual MINT32 mHal3dfInit(void* MavInitInData,void* MotionInitInData,void* WarpInitInData,void* Pano3DInitInData) {return 0;} 
    virtual MINT32 mHal3dfUninit() {return 0;}  
    virtual MINT32 mHal3dfMain(void) {return 0;}    
    virtual MINT32 mHal3dfAddImg(MavPipeImageInfo* pParaIn);   
    virtual MINT32 mHal3dfGetMavResult(void* pParaOut);  
    virtual MINT32 mHal3dfMerge(MUINT32 *MavResult);
    virtual MINT32 mHal3dfDoMotion(void* InputData,MUINT32* MotionResult, MUINT32 u4SrcImgWidth = 0, MUINT32 u4SrcImgHeight = 0);
    virtual MINT32 mHal3dfWarp(MavPipeImageInfo* pParaIn,MUINT32 *MavResult,MUINT8 ImgNum); 
    virtual MINT32 mHal3dfCrop(MUINT32 *MavResult,MUINT8 ImgNum);
    virtual MINT32 mHal3dfGetResult(MUINT32& MavResult,MUINT32& ClipWidth, MUINT32& ClipHeight);  
    virtual MINT32 mHal3dfStitch(MUINT32 *MavResult,MUINT8 ImgNum);    
    virtual MINT32 mHal3dfGetStitchResult(void* Pano3dResult);   
    virtual MINT32 mHal3dfGetWokSize(int SrcWidth, int SrcHeight, MUINT32 &WorkingSize);     
    virtual MINT32 mHal3dfSetWokBuff(void* WorkingBuff);
};  
    
class hal3DFTmp : public hal3DFBase {
public:
    //
    static hal3DFBase* getInstance();
    virtual void destroyInstance();
    //
    hal3DFTmp() {}; 
    virtual ~hal3DFTmp() {};
};

#endif

