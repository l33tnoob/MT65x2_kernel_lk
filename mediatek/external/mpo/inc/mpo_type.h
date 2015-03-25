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

#ifndef MPO_TYPE_H
#define MPO_TYPE_H

#define TYPE_Undefined   0x000000
#define TYPE_Baseline    0x030000
#define TYPE_LargeThumb1 0x010001
#define TYPE_LargeThumb2 0x010002
#define TYPE_Panorama    0x020001
#define TYPE_Disparity   0x020002
#define TYPE_MultiAngle  0x020003

//mpo types that is used solely by encoder and decoder of Mediatek
//this types are hidden in MPO index ifd, that other decoder will
//ignor it by skipping it.
#define MTK_TYPE_NONE    TYPE_Undefined
#define MTK_TYPE_MAV     TYPE_MultiAngle
#define MTK_TYPE_Stereo  TYPE_Disparity
#define MTK_TYPE_3DPan   TYPE_Panorama

//Params types for MPO encoder
#define SOURCE_TYPE_FILE  1
#define SOURCE_TYPE_BUF   2

struct MPImageInfo {
    int MPIndividualNum;
//    bool dependImage;
//    bool dependImageNum;
//    bool representativeImage;
    int PanOrientation;
    int PanOverlap_H[2];
    int PanOverlap_V[2];
    int BaseViewpointNum;
    int ConvergenceAngel[2];
    int BaselineLength[2];
    int VerticalDivergence[2];
    int AxisDistance_X[2];
    int AxisDistance_Y[2];
    int AxisDistance_Z[2];
    int YawAngel[2];
    int PitchAngel[2];
    int RollAngle[2];
    int sourceType;
    char * filename;
    char * imageBuf;
    int imageSize;
    int type;//no need to handle
};

#endif
