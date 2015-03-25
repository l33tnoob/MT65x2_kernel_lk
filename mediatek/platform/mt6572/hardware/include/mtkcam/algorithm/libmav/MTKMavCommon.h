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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_MAV_COMMON_H
#define _MTK_MAV_COMMON_H

#include "MTKMavType.h"
#include "MTKMav.h"

//========================================================================================
/* optimization option */
//========================================================================================
/* parameters in ExtractMatchedFeaturePairs */
#define RZ_SCALE                (0.2f)                     
#define MAV_RI_DATA_SIZE (MAX_MATCH_NO*4+9)
#define denom                   (41943)                    

/* parameters in ExtractHarrisCorner function */
#define PARTIAL_DER_STEP        (1)
#define KAPPA                   (3)                        
#define TH_RC                   (80)                       
#define FE_HARRIS_KAPPA_BITS    (5)
#define HARRIS_AVG_BITS         (20)

/* parameters in imresize function */
#define P_HEIGHT                (13)                       

/* parameters in ExtractMatchedFeaturePairs */
#define MAX_FEATURE_NO          (3072)                     
#define MAV_MAX_FEATURE_NO     (1024)
#define MAX_MATCH_NO            (512)                      

/* perfomance parameter options in RectifyImage function */
#define PARA_NUM                (4)                        
#define BLOCK_SIZE              (32)                        
#define BLOCK_SIZE_SQUARE    (BLOCK_SIZE)*(BLOCK_SIZE)
#define LM_INFO_SZ              (10)

/* buffer number for multi-core */
#define MAV_MAX_CORE_NO     (2)
#define EXTRA_BUFFER_NO     (7)
#define BUFFER_NO_OF_BLUR   (MAV_MAX_CORE_NO + EXTRA_BUFFER_NO)
#define BUFFER_NO_OF_GRAD   (MAV_MAX_CORE_NO)
#define BUFFER_NO_OF_RC     (MAV_MAX_CORE_NO)

#define max(a,b)  ((a) < (b) ? (b) : (a))
#define min(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))

//store position in 2-bytes (for NEON_OPT loading data)
typedef struct mav_point2D_struct
{
  MINT16  x;
  MINT16  y;
} mav_point2D_struct;

typedef struct mav_point2Df_struct
{
  MFLOAT  x;  
  MFLOAT  y;  
  MFLOAT  m_v[2];
}mav_point2Df_struct;

typedef struct mav_feature_point_struct
{	
    MINT32 feature_no;
    mav_point2D_struct * feature_pt;
} mav_feature_point_struct ;

typedef struct mav_TPerspective_struct
{
    MFLOAT  match_no;
    MBOOL  m_IsRectified;
    MFLOAT theta[RANK];          // 3x1 vector
    MFLOAT flen;                 // focal length
    MFLOAT Hmtx[RANK][RANK];     // 3x3 rectification matrix
    MFLOAT Ko[RANK][RANK];       // 3x3 Origin Calib matrix
    MFLOAT Kn[RANK][RANK];       // 3x3 New Calib matrix
    MFLOAT Rmtx[RANK][RANK];     // 3x3 Rot matrix
}mav_TPerspective_struct;

typedef struct MavMatchPointfStruct
{
    mav_point2Df_struct p1;
    mav_point2Df_struct p2;
    MFLOAT   similarity;
    MFLOAT   m_v[4];
} MavMatchPointfStruct;

typedef struct MavMatchImagePairStruct
{
    MINT32 match_no;
    MINT32 m_Image[2];
   MavMatchPointfStruct* m_MatchPt;
}MavMatchImagePairStruct;

typedef struct mav_rec_par_struct
{
    mav_TPerspective_struct* m_Img;
    /* intermediate data */
    MavMatchImagePairStruct* m_Match;
    MavMatchImagePairStruct* m_RectifMatch;
    MINT32 imWidth;
    MINT32 imHeight;

    // Driver object enum
    DrvMavObject_e MavDrvObjectEnum;
}mav_rec_par_struct;

typedef MFLOAT		ResizeArrayF[P_HEIGHT];
typedef MINT32		ResizeArrayI[P_HEIGHT];

typedef struct mav_cal_struct
{ 
    MUINT32 SrcBufAddr[MAV_MAX_IMAGE_NUM];
    MUINT8* BlurImage[BUFFER_NO_OF_BLUR];
    MINT16 ImageWidth;
    MINT16 ImageHeight;  
    MINT16 ImageIdx;  
    MUINT32 SrcBufAddrOffset;
    DrvMavObject_e MavDrvObjectEnum;

    // working buffer in harris
    MINT8 *grdx[BUFFER_NO_OF_GRAD];
    MINT8 *grdy[BUFFER_NO_OF_GRAD];
    MINT32 *rc[BUFFER_NO_OF_RC];
    MINT32 *tmp_rc[BUFFER_NO_OF_RC];

    // tuning parameters
    MINT32 rc_win_bound;    // window bound for selecting points
    MINT32 search_range;    // search range for matching
    MINT32 match_rate;      // match rate of minimum error and second minimum error
    MBOOL  sw_eis_enable;
    MUINT32 core_no;        // cpu core number

    // working buffer index
    MUINT32 buffer_no_of_blur;
    MUINT32 buffer_no_of_grad_rc;

    // working buffer in imresize (used if SW EIS enable)
    MUINT8 *sIg1;
    MUINT8 *sIg2;
    MFLOAT *rz_u;
    MINT32 *rz_left;
    MFLOAT *rz_sum;
    ResizeArrayI* rz_indices;
    ResizeArrayF* rz_weights;
    MFLOAT* rz_tmp_out;
}mav_cal_struct;

typedef struct RECTIFY_IMAGE_CAL_STRUCT
{
    MUINT32 ProcBufAddr;
    MINT32 num_para;           
    MINT32 match_no;           
    MFLOAT p[PARA_NUM];        
    MFLOAT x[MAX_MATCH_NO];    
    MFLOAT lb[PARA_NUM];       
    MFLOAT ub[PARA_NUM] ;      
    MFLOAT opts[5];    
    MFLOAT info[LM_INFO_SZ];            
    MINT32 ffdif; 
    MINT32 nfev;
    MFLOAT delta;                      
    MFLOAT *hx1;
    MFLOAT *hx2;  
    MFLOAT *hxx;
    MFLOAT *adata; /* pointer to possibly additional data, passed uninterpreted to func.*/
                /* Set to NULL if not needed */

    // tuning parameters
    MFLOAT rect_th_err;
    MFLOAT rect_max_angle;
    MINT32 para_max_iter;         
}RECTIFY_IMAGE_CAL_STRUCT;

#endif
