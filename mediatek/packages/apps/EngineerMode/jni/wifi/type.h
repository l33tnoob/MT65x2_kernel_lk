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
** $Id: type.h,v 1.1 2008/05/26 14:04:37 MTK01385 Exp $
*/

/*******************************************************************************
** Copyright (c) 2005 - 2007 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

/*
** $Log: type.h,v $
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.1  2008/05/26 14:04:37  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.2  2007/10/16 09:34:08  MTK01086
** Add the WPDNIC lib
**
** Revision 1.1  2007/10/16 06:57:37  MTK01267
** Inital version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/

#ifndef _TYPE_H
#define _TYPE_H

#include <string.h>
//#include <tchar.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

//#include <utils/string8.h>

#include "definition.h"
#include "dbg.h"
//#include "wireless_copy.h"
#include "linux/wireless.h"


namespace android{

/*! \brief Type definition for signed integers */
typedef signed char             INT_8;
typedef signed short            INT_16;
typedef signed long             INT_32;
typedef signed long long        INT_64;

/*! \brief Type definition for unsigned integers */
typedef unsigned char           UINT_8;
typedef unsigned short          UINT_16;
typedef unsigned long           UINT_32;
typedef unsigned long long      UINT_64;

typedef unsigned int 			BOOL;
typedef	int						HANDLE;

typedef char			CHAR;
typedef unsigned char 	UCHAR;

#define	TRUE 	1
#define	FALSE	0

#define NAMESIZE	30
#define IFNAMSIZ	16

//for Android
#undef UNICODE
//for windows:
#ifdef UNICODE
typedef wchar_t TCHAR;
//#define	_T(x)			 L##x
#else
typedef char	TCHAR;
//#define	_T(x)			 x
#endif

//#define string 	string8
#define wstring	string


//for linux:
//typedef char            TCHAR;
//typedef void            VOID;
//typedef bool            BOOL;

#define IN
#define OUT

#ifdef _DLL_EXPORTS
#define DLL_FUNC    extern "C" __declspec(dllexport)
#define DllExport   __declspec(dllexport)
#else
#define DLL_FUNC
#define DllExport
#endif


#define BIT(n)                          ((UINT_32) 1 << (n))

/* bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m,n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))

#ifndef RX_ANT_
#define RX_ANT_
typedef enum {
    AGC_RX_ANT_SEL,
    MPDU_RX_ANT_SEL,
    FIXED_0,
    FIXED_1
} RX_ANT_SEL;
#endif

}


#endif
