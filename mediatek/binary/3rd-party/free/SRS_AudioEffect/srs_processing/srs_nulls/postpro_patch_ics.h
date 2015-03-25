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

/********************************************************************************
 *	SRS Labs CONFIDENTIAL
 *	@Copyright 2012 by SRS Labs.
 *	All rights reserved.
 *
 *  Delta-removing patch for Ice-Cream Sandwich's AudioFlinger.cpp
 ********************************************************************************/

#ifndef ANDROID_POSTPRO_NULL_PATCH
#define ANDROID_POSTPRO_NULL_PATCH

#define POSTPRO_PATCH_ICS_PARAMS_SET(a) ((void)0)
#define POSTPRO_PATCH_ICS_PARAMS_GET(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_MIX_INIT(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_MIX_SAMPLES(a, fmt, buf, bsize, rate, count) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_MIX_EXIT(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_MIX_ROUTE(a, para, val) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_DIRECT_INIT(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_DIRECT_SAMPLES(a, fmt, buf, bsize, rate, count) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_DIRECT_EXIT(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_DUPE_INIT(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_DUPE_SAMPLES(a, fmt, buf, bsize, rate, count) ((void)0)
#define POSTPRO_PATCH_ICS_OUTPROC_DUPE_EXIT(a, b) ((void)0)
#define POSTPRO_PATCH_ICS_INPROC_INIT(a, b, fmt) ((void)0)
#define POSTPRO_PATCH_ICS_INPROC_SAMPLES(a, fmt, buf, bsize, rate, count) ((void)0)
#define POSTPRO_PATCH_ICS_INPROC_EXIT(a, b, fmt) ((void)0)
#define POSTPRO_PATCH_ICS_INPROC_ROUTE(a, para, val) ((void)0)

#endif // ANDROID_POSTPRO_NULL_PATCH
