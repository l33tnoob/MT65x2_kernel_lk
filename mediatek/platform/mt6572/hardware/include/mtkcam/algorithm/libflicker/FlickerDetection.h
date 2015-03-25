/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Name:   FlickerDetection.h
 * Author: Wei-Kai Liao
 * Date:   2009/02/19
 * Note:   Header file of AutoFocus.cpp, EIS.cpp, and FlickerDetection.cpp
 *         The header file for automatic flicker detection algorithm
 */
//#pragma once
#ifndef _FLICKERDETECTION_H_
#define _FLICKERDETECTION_H_

/*************************************************************************************************************
 *
 *                                              constants definition
 *
 *************************************************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef signed long long  MINT64;
typedef void            MVOID;


/* WCT's setting: 3x3 - 3 windows per row and 3 rows per frame */
static const MUINT32 nWin_w = 3;
static const MUINT32 nWin_h = 3;

/* define the precision for flicker classification */
static const MINT32 PRECISION_CLASSIFY1 = 10;
static const MINT32 PRECISION_CLASSIFY2 = 32;
static const MINT32 OUTPUT_MODEL_CLIST_DIFF = 0;

static const MINT32 W_WIN[2] = {3, 1};

/* thresholds of flicker detection */
static MINT32 THRE_C[2] = {-1, -1};
static MINT32 THRE_A[2] = {-1, -1};
static MINT32 THRE_F[3] = {-1, -1, -1};
static MUINT32 MOTION_WIN_NUM = 16;                                                 // number of motion windows from EIS

/*************************************************************************************************************
 *
 *                                              enumerate type
 *
 *************************************************************************************************************/

typedef enum STAT_TYPE_t {AD_T, SD_T} STAT_TYPE;                                            /* absolute difference and square difference */
typedef enum LMV_STATUS_t {LARGE_MOTION, SMALL_MOTION} LMV_STATUS;                          /* status from local motion vectors: large inter-frame motion & small inter-frame motion */
typedef enum FLICKER_STATUS_t {INCONCLUSIVE, FK000, FK100, FK120} FLICKER_STATUS;           /* inconclusive, no flicker, 100 Hz, 120 Hz */


/*************************************************************************************************************
 *
 *                                              function declaration
 *
 *************************************************************************************************************/

/*********************************************** Flicker Detection ***********************************************/

/* software job of flicker detection */
FLICKER_STATUS detectFlicker_SW(const MUINT32 w, const MUINT32 h, const MINT32 *vx, const MINT32 *vy, const MINT32 LMV_th, LMV_STATUS *lmvFlag, MINT64 *AF_stat, const MINT32 *C_list0, const MINT32 *C_list1, const MUINT32 pck, const MUINT32 fll, MINT32 vAMDF[8], const FLICKER_STATUS oldStatus, MINT32 *freq_frame, MINT32 *f000, MINT32 *f100, MINT32 *f120);

/* build final representative column vector and compute AMDF */
MVOID calcFlickerStat(const MUINT32 w, const MUINT32 h, const MINT64 *AF_stat, const int *C_list0, const int *C_list1, const MUINT32 pck, const MUINT32 fll, int vAMDF[8]);

/* classify based on two frames' result */
MINT32 classifyFlicker(const MINT32 *v, const MUINT32 size);

/* accumulate frame-by-frame classification confidence over time to make decision */
FLICKER_STATUS accTemporalConfidence(const MINT32 flag, MINT32 *f000, MINT32 *f100, MINT32 *f120);

/* compute AMDF */
MINT32 AMDF(const MINT32 *C, const MINT32 N, const MINT32 m);

/* use local motion vectors to reject or accept this image pair for flicker detection */
LMV_STATUS measureMotion(const MINT32 *vx, const MINT32 *vy, const MUINT32 num, const MINT32 threshold);

/* set thresholds */
MVOID setThreshold(const MINT32 threc[2], const MINT32 threa[2], const MINT32 thref[3]);

/* set LMV count number */
MVOID setLMVcnt(MUINT32 u4LMVNum);

#endif//_FLICKERDETECTION_H_
