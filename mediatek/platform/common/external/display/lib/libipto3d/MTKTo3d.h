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
 

#ifndef _MTK_TO3D_H
#define _MTK_TO3D_H

#include "MTKTo3dType.h"
#include "MTKTo3dErrCode.h"
#include <GLES2/gl2.h> //20130325
//2012-0605
//#include <ui/GraphicBuffer.h>

//#define SIM_MAIN
//#define DEBUG

//#define OUT_RESULT

typedef enum
{
	TO3D_STATE_IDLE=0,		
	TO3D_STATE_STANDBY,		// After Create Obj or Reset
	TO3D_STATE_INIT,			// After Called To3DInit
	TO3D_STATE_PROC,			// After Called To3DMain
	To3D_STATE_READY,         // After Algo. process done
} TO3D_STATE_ENUM;

typedef enum
{    
	TO3D_IMAGE_FORMAT_LUMA = 0,
    TO3D_IMAGE_FORMAT_RGB565,
    TO3D_IMAGE_FORMAT_RGB888,
    TO3D_IMAGE_FORMAT_RGBA8888,                              
    TO3D_IMAGE_FORMAT_YUV444,
    TO3D_IMAGE_FORMAT_YUV422,
    TO3D_IMAGE_FORMAT_YUV420 = 6,

    TO3D_IMAGE_FORMAT_MAX
} TO3D_IMAGE_FORMAT_ENUM;

typedef enum
{
    TO3D_STILL_IMAGE_PLAYBACK,
    TO3D_VIDEO_PLAYBACK
} TO3D_SCENARIO_ENUM;

typedef enum
{
    TO3D_FEATURE_BEGIN = 0,

    TO3D_FEATURE_GET_RESULT,
    TO3D_FEATURE_GET_STATUS,
    TO3D_FEATURE_SET_PROC_INFO,
    TO3D_FEATURE_SAVE_LOG,
    
    TO3D_FEATURE_MAX
}	TO3D_FEATURE_ENUM;

typedef struct
{
    MFLOAT global_thr_ratio;			//threshold for recongnize H, default 0.65, range[0.5,1.0]
    MFLOAT baseline;					//eye distance, default 1.6
    MUINT32 global_weighting;			//global weighting (default global:local = 8:24), range[0,16]
    MUINT32 scene_change_thr;			//scence change threshold, default 80, range[30,100]
}TO3D_TUNING_PARA_STRUCT;

typedef struct
{
    MUINT16 large_image_width;
    MUINT16 large_image_height;
		MUINT16 small_image_width;
		MUINT16 small_image_height;

    TO3D_IMAGE_FORMAT_ENUM large_image_format;
    TO3D_IMAGE_FORMAT_ENUM small_image_format;
    TO3D_SCENARIO_ENUM to3d_scenario; 
    TO3D_TUNING_PARA_STRUCT to3d_tuning_data;
} TO3D_SET_ENV_INFO_STRUCT, *P_TO3D_SET_ENV_INFO_STRUCT;

typedef struct
{
    MUINT16 output_image_width;
    MUINT16 output_image_height;
    MUINT32 large_image_addr; // input image address
    MUINT32 small_image_addr;
    MUINT32 output_image_addr; //output side-by-side image 
		MUINT32 angle;
		
		//20130325
		GLuint large_image_texID;
		GLuint output_image_texID;
		GLuint output_image_fboID;
		
} TO3D_SET_PROC_INFO_STRUCT, *P_TO3D_SET_PROC_INFO_STRUCT;


typedef struct
{
    TO3D_IMAGE_FORMAT_ENUM image_format;
}TO3D_RESULT_STRUCT;


class MTKTo3d {
public:
    static MTKTo3d* createInstance();
    virtual void   destroyInstance() = 0;
       
	virtual ~MTKTo3d(){}
    // Process Control
    virtual MRESULT To3dInit(void* InitInData, void* InitOutData); // Env. setting
    virtual MRESULT To3dMain();	// START
    virtual MRESULT To3dReset();   //Reset
            
	// Feature Control        
	virtual MRESULT To3dFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};


#endif
