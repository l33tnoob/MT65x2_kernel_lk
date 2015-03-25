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

#ifndef ANDROID_SRS_SETUP
#define ANDROID_SRS_SETUP

// Path Stringizers

#define SRS_STR(s) DOSRS_STR(s)		// Trick to allow values of defines to become strings properly...
#define DOSRS_STR(s) #s

#ifdef _SRSCFG_ARCH_ARM
	#define SRSLIBINC(file) DOSRS_STR( srs_include_arm/file )
	#include SRSLIBINC(srs_fxp.h)
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
	#define SRSLIBINC(file) DOSRS_STR( srs_include_x86/file )
	#define SRS_FXP16(val,iwl) val
	#define SRS_FXP32(val,iwl) val
#endif	// _SRSCFG_ARCH_X86

// Didn't allow Logging?
#ifndef SRS_VERBOSE

#ifdef SRS_LOG		// Mute Android's own LOG macro (add other macros to mute as needed)
	#undef SRS_LOG
#endif

#define SRS_LOG(...)   ((void)0)

#else	// SRS_VERBOSE

#ifdef SRS_LOG		// Mute Android's own LOG macro (add other macros to mute as needed)
	#undef SRS_LOG
#endif

//#if (SRS_AND_PLAT_INDEX < 8)
#if (SRS_AND_PLAT_INDEX < 10)
#define SRS_LOG(...)   ((void)LOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#endif	// SRS_AND_PLAT_INDEX < 8

//#if (SRS_AND_PLAT_INDEX >= 8)
#if (SRS_AND_PLAT_INDEX >= 10)
#define SRS_LOG(...)   ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#endif 	// SRS_AND_PLAT_INDEX >= 8

#endif	// SRS_VERBOSE


#endif	// ANDROID_SRS_SETUP

