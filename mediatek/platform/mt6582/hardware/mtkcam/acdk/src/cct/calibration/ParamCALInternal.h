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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// ParamCALInternal.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  ParamCALInternal.h
//! \brief
#define FLOAT_VERSION
//#define READ_GAIN_TABLE

#ifndef INTERNAL_PARAM
#define INTERNAL_PARAM


#define CAL_ASSERT_OPT    0
/*******************************************************************************
 *
 ********************************************************************************/
#if (CAL_ASSERT_OPT == 1)
#define CAL_ASSERT(x, str); \
    if (x) {printf("[Assert %s, %d]: %s", __FILE__, __LINE__, str); LOGE("[Assert %s, %d]: %s", __FILE__, __LINE__, str); while(1);} \
    else {}
#else
#define CAL_ASSERT(x, str);
#endif


/****************************************
 *	Trivial define, not used in embedded sys.
 ****************************************/
#define LSC_Debug_Table_Output 0
#define Shading_Table_Size 4096

#ifdef FLOAT_VERSION
	#define AVG_PIXEL_SIZE (33)  //  (32) //(16)
#else
#define AVG_PIXEL_SIZE (16)
#endif
#define MAX_PIXEL_GAIN (4)

/****************************************/
/*	LSC_CalHwTbl.cpp                                    */
/****************************************/
/****************************************/
/*	MACROS                                                  */
/****************************************/
#define MAX2(a,b) (a>b?a:b)
#define MIN2(a,b) (a>b?b:a)

#define GN_BIT 13
#define COEF_BASE_BIT 15 /* it dangerous when (COEF_BASE_BIT+3)+(COEF_DIV_BIT) close to 32 bit */
#define COEF_DIV_BIT 10
#define COEF_DIV6_VAL ((1<<COEF_DIV_BIT)/6.0+0.5)
#define COEF_DIV3_VAL ((1<<COEF_DIV_BIT)/3.0+0.5)
#define COEF_FXX_BIT (COEF_BASE_BIT-GN_BIT)
#define COEF_BIT0 9
#define COEF_BIT1 8
#define COEF_BIT2 7

/****************************************
 *	isp_lsc_core.h
 ****************************************/
#define RATIO_POLY_BIT 8
#define SQRT_NORMALIZE_BIT 12
/* new added */
typedef struct {
	int grid_num; // grid number
	int di;	// d[i]=x[i]-x[i-1]
	int dn;	// d[n]=x[n]-x[n-1], n=GRID_N or GRID_M
	int di2;	// d[i]*d[i]
	int dn2;	// d[n]*d[n]
	float dir;	// 1/d[i]
	float dnr;	// 1/d[n]
	float di2r; // 1/(d[i]*d[i])
	float ddir;	// 1/(x[i+1]-x[i-1])
	float ddnr;	// 1/(x[n+1]-x[n-1]), n=GRID_N-1 or GRID_M-1
} DIM_INFO_T;
typedef struct {
	int dx2;
	int dy2;
	int f00;
	int f01;
	int f10;
	int f11;
	float f00x2;
	float f01x2;
	float f10x2;
	float f11x2;
	float f00y2;
	float f01y2;
	float f10y2;
	float f11y2;
} BLK_INFO_T;

 typedef struct
{
	float coef_a;
	float coef_b;
	float coef_c;
	float coef_d;
	float coef_e;
	float coef_f;
	int ratio_poly_flag;
}RATIO_POLY_T;
typedef struct
{
	int raw_wd;
	int raw_ht;
	int plane_wd;
	int plane_ht;
	int bayer_order;
	int crop_ini_x;
	int crop_end_x;
	int crop_ini_y;
	int crop_end_y;
	int block_wd;
	int block_ht;
	int block_wd_last;
	int block_ht_last;
	int avg_pixel_size;
	int avg_pixel_size_bit;
	int x_grid_num;
	int y_grid_num;
	int pxl_gain_max;
	RATIO_POLY_T poly_coef;
}LSC_PARAM_T;
typedef struct
{
#ifdef FLOAT_VERSION
	float max_val[4];
#else
	int max_val[4];
#endif
	int x_max_pos[4];
	int y_max_pos[4];
}LSC_RESULT_T;
enum
{
	BB_PLANE = 0,
	GB_PLANE,
	GR_PLANE,
	RR_PLANE,
};
typedef struct
{
       int i4GridXNUM;
       int i4GridYNUM;
       int i4XINIBorder;
       int i4XENDBorder;
       int i4YINIBorder;
       int i4YENDBorder;
	   int i4AvgWinSize;
       unsigned int u4ImgWidth;
       unsigned int u4ImgHeight;
       unsigned short u2BayerStart;
	RATIO_POLY_T poly_coef;
}LSC_CAL_INI_PARAM_T;


/****************************************
 *	 SVDtool.cpp
 ****************************************/
#define Capture_BPC_Table_NAME "//data//Capture_BPC_Table_NAME.txt"
#define Preview_BPC_Table_NAME "//data//Preview_BPC_Table_NAME.txt"

#define Preview_SVD_MATRIX_NAME "//data//Preview_SVD_Matrix.h"
#define Capture_SVD_MATRIX_NAME "//data//Capture_SVD_Matrix.h"
#define EIGEN_SEQUENCE_MODIFICATION 1

#define NR_END 1
//#define FREE_ARG char*
static float maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

static float sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

typedef struct{
	int min_val;
	unsigned char len_frac;
}SVD_UV_INFO_T;


/****************************************
 *	badpixel calibration
 ****************************************/
#define  BPC_Debug_Table_Output 0
#define  CAL_Table_Size 24576 //5M sensor BPC ratio 0.1% = 5000 pixel == 20K bytes

typedef struct
{
	unsigned int u4CalibrateMode;
       unsigned int u4ImgWidth;
       unsigned int u4ImgHeight;
	float fBrightPixelLevel;
	float fDarkPixelLevel;
	unsigned char *raw_img_addr;
	unsigned char *mask_buffer;
}BPC_CAL_INI_PARAM_T;

typedef struct
{
	int **valid_cnt;
	int **MAX_cnt;
	long **ACC;
}Block_Info;
#endif
