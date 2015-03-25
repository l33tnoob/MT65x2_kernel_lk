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

#ifndef _KAL_NON_SPECIFIC_GENERAL_TYPES_H
#define _KAL_NON_SPECIFIC_GENERAL_TYPES_H

/*******************************************************************************
 * Type Definitions
 *******************************************************************************/
typedef unsigned char           kal_uint8;
typedef signed char             kal_int8;
typedef char                    kal_char;
typedef unsigned short          kal_wchar;

typedef unsigned short int      kal_uint16;
typedef signed short int        kal_int16;

/*
#undef  kal_uint32
typedef unsigned int            kal_uint32; 
  //conflicting with Sw_types.h... so comment by mtk80691

#undef  kal_int32
typedef signed int              kal_int32;  //conflicting with Sw_types.h... so comment by mtk80691

*/

//#if !defined(GEN_FOR_PC) && !defined(__MTK_TARGET__)
#if 0
   //typedef ULONG64              kal_uint64;
   //typedef LONG64               kal_int64;
   typedef unsigned __int64      kal_uint64;
   typedef __int64               kal_int64;
#else
   typedef unsigned long long   kal_uint64;
   typedef signed long long     kal_int64;
#endif

#if !defined(__MTK_TARGET__)
   typedef int kal_jmpbuf[64];
#elif defined(__RVCT__)   
   typedef long long kal_jmpbuf[48];
#else
   typedef int kal_jmpbuf[32];
#endif

/*//conflicting with Sw_types.h... so comment by mtk80691
#ifndef KAL_FALSE
typedef enum 
{
  KAL_FALSE,
  KAL_TRUE
} kal_bool;
#endif
*/
typedef void (*kal_func_ptr)(void);

/*******************************************************************************
 * Constant definition
 *******************************************************************************/
#ifndef NULL
#define NULL               0
#endif

#if defined(KAL_ON_NUCLEUS)

#define KAL_AND               NU_AND
#define KAL_CONSUME           NU_OR_CONSUME
#define KAL_AND_CONSUME       NU_AND_CONSUME
#define KAL_NO_SUSPEND        NU_NO_SUSPEND
#define KAL_OR                NU_OR
#define KAL_OR_CONSUME        NU_OR_CONSUME
#define KAL_SUSPEND           NU_SUSPEND

#elif defined (KAL_ON_OSCAR)    /* KAL_ON_NUCLEUS */

#define KAL_AND               OSC_ACTION_FULL_SET
#define KAL_CONSUME           OSC_ACTION_CLS
#define KAL_AND_CONSUME       OSC_ACTION_FULL_SET | OSC_ACTION_CLS
#define KAL_NO_SUSPEND        OSC_TIMEOUT_NONE
#define KAL_OR                OSC_ACTION_PART_SET
#define KAL_OR_CONSUME        OSC_ACTION_PART_SET | OSC_ACTION_CLS
#define KAL_SUSPEND           OSC_TIMEOUT_FOREVER

#elif defined(KAL_ON_THREADX)   /* KAL_ON_NUCLEUS */

#define KAL_AND               TX_AND
#define KAL_CONSUME           TX_OR_CLEAR
#define KAL_AND_CONSUME       TX_AND_CLEAR
#define KAL_NO_SUSPEND        TX_NO_WAIT
#define KAL_OR                TX_OR
#define KAL_OR_CONSUME        TX_OR_CLEAR
#define KAL_SUSPEND           TX_WAIT_FOREVER

#endif  /* KAL_ON_NUCLEUS */

#endif  /* _KAL_NON_SPECIFIC_GENERAL_TYPES_H */


