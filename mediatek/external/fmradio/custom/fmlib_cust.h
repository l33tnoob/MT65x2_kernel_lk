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

#ifndef __FMLIB_CUST_H__
#define __FMLIB_CUST_H__

struct fm_fake_channel
{
     int freq;
     int rssi_th;
     int reserve;
};

struct fm_fake_channel_t
{
    int size;
    struct fm_fake_channel *chan;
};

struct CUST_cfg_ds 
{
     int16_t chip;
     int32_t band;
     int32_t low_band;
     int32_t high_band;
     int32_t seek_space;
     int32_t max_scan_num;
     int32_t seek_lev;
     int32_t scan_sort;
     int32_t short_ana_sup;
     int32_t rssi_th_l2;
     struct fm_fake_channel_t *fake_chan;
};
 
#ifdef MTK_FM_50KHZ_SUPPORT
#define MT6620_FM_FAKE_CHANNEL 	\
     {                           \
         {10400, -40, -1},         \
         {9100, -40, -1},         \
         {9600, -40, -1},          \
		 {9220, -80, -1}		   \
     }
#define MT6627_FM_FAKE_CHANNEL \
{ \
    {9600,-107,-1},\
    {10400,-107,-1},\
    {10750,-224,-1}\
}
#else
#define MT6620_FM_FAKE_CHANNEL 	\
     {                           \
         {1040, -40, -1},         \
         {910, -40, -1},         \
         {960, -40, -1},           \
		 {922, -80, -1}		   \
     }
#define MT6627_FM_FAKE_CHANNEL \
{ \
    {960,-107,-1},\
    {1040,-107,-1},\
    {1075,-224,-1}\
}
#endif //MTK_FM_50KHZ_SUPPORT

#define FM_JNI_SCAN_SPACE_50K 5
#define FM_JNI_SCAN_SPACE_100K 1
#define FM_JNI_SCAN_SPACE_200K 2
#if (defined(MT6620_FM) || defined(MT6628_FM)||defined(MT6627_FM)||defined(MT6630_FM))
/*implement fm scan by soft mute tune 
 change to 0 will scan by orginal way*/
#define FMR_SOFT_MUTE_TUEN_SCAN 1
#define FMR_NOISE_FLOORT_DETECT 1
#define RSSI_TH -296
#define FM_SEVERE_RSSI_TH -107//67dBuV
#define FM_NOISE_FLOOR_OFFSET 10
#else/*if new chip support, need to modify following parameters*/
#define FMR_SOFT_MUTE_TUEN_SCAN 0
#define FMR_NOISE_FLOORT_DETECT 0
#endif
#endif //__FMLIB_CUST_H__
