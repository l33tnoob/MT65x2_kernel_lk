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
#ifndef _MTK_STEREO_KERNEL_H
#define _MTK_STEREO_KERNEL_H

#include "MTKUtilType.h"
#include "MTKStereoKernelErrCode.h"


typedef enum
{
    STEREO_KERNEL_FEATURE_BEGIN = 0,
    STEREO_KERNEL_FEATURE_GET_RESULT,
    STEREO_KERNEL_FEATURE_GET_STATUS,
    STEREO_KERNEL_FEATURE_SAVE_LOG,

    STEREO_KERNEL_FEATURE_SET_PROC_INFO,
    
    STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO,
    STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO,

    STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING, 
    

    STEREO_KERNEL_FEATURE_MAX
}
STEREO_KERNEL_FEATURE_ENUM;

typedef enum
{
    INPUT_FORMAT_YV12 = 0,
    INPUT_FORMAT_RGBA,
}
INPUT_FORMAT_ENUM ;
/*
typedef enum
{
    INPUT_TYPE_SBS = 0, // Side-By-Side
    INPUT_TYPE_IND,        // frame INDpendent
}
INPUT_TYPE_ENUM ;
*/
typedef enum
{
    DISP_SCEREEN_LAYOUT_HORIZONTAL = 0,
    DISP_SCEREEN_LAYOUT_VERTICAL
}
DISP_SCREEN_LAYOUT_ENUM ;

typedef enum
{
    STEREO_KERNEL_SCENARIO_IMAGE_PREVIEW,
    STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE,
    STEREO_KERNEL_SCENARIO_IMAGE_PLAYBACK,
    STEREO_KERNEL_SCENARIO_VIDEO_RECORD,
    STEREO_KERNEL_SCENARIO_VIDEO_PLAYBACK,
    STEREO_KERNEL_SCENARIO_IMAGE_ZOOM,
}
STEREO_KERNEL_SCENARIO_ENUM; 

/////////////////////////////
//    For Tuning
/////////////////////////////
typedef struct
{
    MUINT8 conv_effect_img     ;    // 0: normal, 1: strong
    MUINT8 conv_effect_vdo     ;    // 0: normal, 1: strong
    MUINT8 conv_speed_img      ;    // 0: slow, 1: normal, 2: fast
    MUINT8 conv_speed_vdo      ;    // 0: slow, 1: normal, 2: fast
    MUINT8 conv_sensing_img    ;    // 0: slow, 1: normal, 2: fast
    MUINT8 conv_sensing_vdo    ;    // 0: slow, 1: normal, 2: fast

    MUINT8 conv_min_deg_h      ;    // 1-5, default 2
    MUINT8 conv_max_deg_h      ;    // 1-5, default 2
    MUINT8 conv_min_deg_v      ;    // 1-5, default 1
    MUINT8 conv_max_deg_v      ;    // 1-5, default 1

    MINT8  conv_def_position   ;    // default 0

    MUINT8 conv_behavior       ;    // 0: bound, 1: peak
    MUINT8 conv_behavior_ctrl  ;    // 0: off, 1: on

    MUINT8 moving_gap_x        ;    // define the gap for x moving, default 5
    MUINT8 moving_gap_y        ;    // define the gap for y moving, default 3

    MUINT8 alg_color           ;    // 0:slightly, 1:normal, 2: strong

    MUINT8 cc_thr              ;    // 0-100, default 85
    MUINT8 cc_gap_vdo          ;    // >=0, default 5
    MUINT8 cc_gap_img          ;    // >=0, default 0
    MUINT8 cc_thr_change       ;    // >=0, default 10
    MUINT8 cc_sim_sample_rate  ;    // > 0, default 4
    MUINT8 cc_forget_factor    ;    // 0-100, default 80
}
STEREO_TUNING_PARA_STRUCT, *P_STEREO_TUNING_PARA_STRUCT ;

typedef struct
{
    STEREO_TUNING_PARA_STRUCT customer_tuning_info ;
}
STEREO_KERNEL_TUNING_PARA_STRUCT, *P_STEREO_KERNEL_TUNING_PARA_STRUCT ;


/////////////////////////////

typedef struct
{
    STEREO_KERNEL_SCENARIO_ENUM scenario ; 
    MUINT32 working_buffer_size ;

    MUINT8 screen_layout ; // N3D: default 0, S3D: hori-0, vert-1

    MUINT16 source_image_width  ;
    MUINT16 source_image_height ;
    MUINT16 crop_image_width    ;
    MUINT16 crop_image_height   ;

    MUINT16 rgba_image_width    ;
    MUINT16 rgba_image_height   ;

    MUINT16 rgba_image_stride   ;
    MUINT16 fefm_image_width    ;
    MUINT16 fefm_image_height   ;
    MUINT16 fefm_image_stride   ;
    INPUT_FORMAT_ENUM fefm_image_format ;

    MUINT8  isUseHWFE           ;    // 0: SW FE, 1: HW FE
    MUINT8  hw_block_size       ;

    MINT32  *learning_data      ;    // N3D only

    MUINT32 cl_correction       ;    
    MUINT32 enable_gpu          ;
    MUINT32 mtk3Dtag            ;    // 1 for N3D image

    MUINT8  pic_idx_for_warp    ;    // 0: LEFT, 1: RIGHT

    MUINT16 left_ratio_crop_start_x ;
    MUINT16 left_ratio_crop_start_y ;

    MUINT16 right_ratio_crop_start_x ;
    MUINT16 right_ratio_crop_start_y ;

    P_STEREO_KERNEL_TUNING_PARA_STRUCT ptuning_para ;
}
STEREO_KERNEL_SET_ENV_INFO_STRUCT ;

typedef struct
{
    MUINT32 ext_mem_size;
    MUINT32 ext_mem_start_addr; //working buffer start address
}
STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT, *P_STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT ;

typedef struct
{
    //MUINT32 source_image_left_addr ;    // No use (image with smaller view angle)
    MUINT32 source_image_right_addr ;    // for Warping (image with large view angle)
    MUINT32 fefm_image_left_addr    ;    // for Geometric Correction & Convergence
    MUINT32 fefm_image_right_addr   ;    // for Geometric Correction & Convergence
    MUINT32 rgba_image_left_addr    ;    // for Photometric Correction
    MUINT32 rgba_image_right_addr   ;    // for Photometric Correction

    MUINT32 warped_image_addr       ;    //    warping result image

    //MUINT32 warped_image_addr       ;    // for warped image
    //P_STEREO_KERNEL_RESULT_FOR_WARP_STRUCT warp_info;

    //FOR HW FE (N3D)
    MINT16* hwfe_data_left          ;    // Data array for Hardware Feature Extraction, Left  Image
    MINT16* hwfe_data_right         ;    // Data array for Hardware Feature Extraction, Right Image
}
STEREO_KERNEL_SET_PROC_INFO_STRUCT, *P_STEREO_KERNEL_SET_PROC_INFO_STRUCT;

typedef struct
{
    MINT32 left_offset_x ;
    MINT32 left_offset_y ;
    MINT32 right_offset_x ;
    MINT32 right_offset_y ;

    MINT32 cropping_offsetX_L[9] ; //for manual adjustment offsetX1
    MINT32 cropping_offsetX_R[9] ; //for manual adjustment offsetX2
    MINT32 cropping_interval_default ; // index for default position

    MINT32 cropping_size_width[9] ;
    MINT32 cropping_size_height[9] ;
    MINT32 cropping_offsetY_L ; //for manual adjustment offsetY1
    MINT32 cropping_offsetY_R ; //for manual adjustment offsetY2

    MBOOL isBounded ;

    MINT32 *pt_info_NVRAM ; //N3D
}
STEREO_KERNEL_RESULT_STRUCT, *P_STEREO_KERNEL_RESULT_STRUCT ;


/*
        CLASS
*/
class MTKStereoKernel {
public:
    static MTKStereoKernel* createInstance();
    virtual void destroyInstance() = 0;
       
    virtual ~MTKStereoKernel(){};
    // Process Control
    virtual MRESULT StereoKernelInit(void* InitInData);
    virtual MRESULT StereoKernelMain();    // START
    virtual MRESULT StereoKernelReset();   //Reset
            
    // Feature Control        
    virtual MRESULT StereoKernelFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};


#endif