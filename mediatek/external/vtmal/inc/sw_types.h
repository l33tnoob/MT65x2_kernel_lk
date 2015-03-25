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

#ifndef __SW_TYPES_H
#define __SW_TYPES_H


#if !defined(__MTK_TARGET__) 
/*
 * general definitions
 */

typedef signed char    int8;
typedef signed short   int16;
typedef signed long    int32;
//typedef __int64       int64;
typedef signed int     intx;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
//typedef unsigned __int64       uint64;
typedef unsigned int   uintx;
//typedef unsigned char  bool;

typedef unsigned char   BYTE,UINT8,*PUINT8,*PBYTE;
typedef unsigned short  UINT16,*PUINT16;
typedef unsigned long   UINT32,*PUINT32;

typedef signed char				kal_int8;
typedef signed short			   kal_int16;

//#undef  kal_int32
//typedef signed long				kal_int32; // comment by mtk80691 for corruption with decoder

//typedef __int64              kal_int64;
typedef signed int				kal_intx;
typedef unsigned char			kal_uint8;
typedef unsigned short			kal_uint16;


//#undef  kal_uint32
//typedef unsigned long			kal_uint32; // comment by mtk80691 for corruption with decoder

//typedef unsigned __int64       kal_uint64;
typedef unsigned int			kal_uintx;
typedef char					kal_bool;
typedef char					kal_char;
#endif

/***** comment by mtk80691 for corruption with decoder
#ifndef KAL_FALSE
#define KAL_TRUE 1
#define KAL_FALSE 0
#endif
************/

/*
 * Definitions for BOOLEAN
 */

#define FALSE          0
#define TRUE           1

/*
 * Definitions for NULL
 */
#ifndef NULL
#define NULL           0
#endif

//enum boolean{false, true};
enum {RX, TX, NONE};

typedef struct 
{
   kal_uint8              vos_data[100];
   kal_uint32             size;
}MP4VIDEO_VOS_STRUCT; 

#endif


