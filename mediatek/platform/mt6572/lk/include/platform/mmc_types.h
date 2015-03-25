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

#ifndef _MMC_TYPES_H_
#define _MMC_TYPES_H_

#include "msdc_cfg.h"

#if defined(MMC_MSDC_DRV_CTP)
#include "typedefs.h"
#endif

#if defined(MMC_MSDC_DRV_LK)
#include <sys/types.h>
#include "mt_typedefs.h"
#endif

#if defined(MMC_MSDC_DRV_PRELOADER)
#include "typedefs.h"
#endif

#ifndef s8
typedef signed char         s8;
#endif
#ifndef u8
typedef unsigned char       u8;
#endif

#ifndef s16
typedef signed short        s16;
#endif
#ifndef u16
typedef unsigned short      u16;
#endif

#ifndef s32
typedef signed int          s32;
#endif
#ifndef u32
typedef unsigned int        u32;
#endif

#ifndef s64
typedef signed long long    s64;
#endif
#ifndef u64
typedef unsigned long long  u64;
#endif

/* bsd */
#ifndef u_char
typedef unsigned char		u_char;
#endif
#ifndef u_short
typedef unsigned short		u_short;
#endif
#ifndef u_int
typedef unsigned int		u_int;
#endif
#ifndef u_long
typedef unsigned long		u_long;
#endif

/* sysv */
#ifndef unchar
typedef unsigned char		unchar;
#endif
#ifndef uchar
typedef unsigned char		uchar;
#endif
#ifndef ushort
typedef unsigned short		ushort;
#endif
#ifndef uint
typedef unsigned int		uint;
#endif
#ifndef ulong
typedef unsigned long		ulong;
#endif

#ifndef uint8
typedef unsigned char       uint8;
#endif
#ifndef uint16
typedef unsigned short      uint16;
#endif
#ifndef uint32
#if !defined(MMC_MSDC_DRV_PRELOADER)
typedef unsigned int        uint32;
#endif
#endif
#ifndef int8
typedef signed char         int8;
#endif
#ifndef int16
typedef signed short        int16;
#endif
#ifndef int32
#if !defined(MMC_MSDC_DRV_PRELOADER)
typedef signed int          int32;
#endif
#endif

#ifndef bool
//typedef enum {
#define FALSE               0
#define TRUE                1
//} bool;
#endif

#ifndef NULL
#define NULL                0
#endif

#endif /* _MMC_TYPES_H_ */

