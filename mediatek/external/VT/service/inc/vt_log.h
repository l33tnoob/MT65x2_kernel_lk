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

#ifndef _VT_LOG_H_
#define _VT_LOG_H_
#include <mtk_vt_log.h>
#ifdef DEBUG_ENABLE_LOG

#define _V(...) do{if(VT_LOG_IS_ENABLE(VT_LOG_VERBOSE)){__android_log_print(ANDROID_LOG_VERBOSE, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);mtk_vt_debug_printf_v(VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);}}while(0)
#define _D(...) do{if(VT_LOG_IS_ENABLE(VT_LOG_DEBUG)){__android_log_print(ANDROID_LOG_DEBUG, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);mtk_vt_debug_printf_d(VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);}}while(0)
#define _I(...) do{if(VT_LOG_IS_ENABLE(VT_LOG_INFO)){__android_log_print(ANDROID_LOG_INFO, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);mtk_vt_debug_printf_i(VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);}}while(0)
#define _W(...) do{if(VT_LOG_IS_ENABLE(VT_LOG_WARN)){__android_log_print(ANDROID_LOG_WARN, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);mtk_vt_debug_printf_w(VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);}}while(0)
#define _E(...) do{if(VT_LOG_IS_ENABLE(VT_LOG_ERROR)){__android_log_print(ANDROID_LOG_ERROR, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);mtk_vt_debug_printf_e(VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);}}while(0)
#define _G(group, ...) do{if(VT_LOG_IS_ENABLE(group)){__android_log_print(ANDROID_LOG_INFO, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);mtk_vt_debug_printf(group, VT_TAGS[LOG_TAG_IND].tag, __VA_ARGS__);}}while(0) 
#else

#define _V(...) do{}while(0)
#define _D(...)	do{}while(0)
#define _I(...)	do{}while(0)
#define _W(...)	do{}while(0)
#define _E(...)	do{}while(0)
#define _G(group, ...) do{}while(0)

#endif

#endif 