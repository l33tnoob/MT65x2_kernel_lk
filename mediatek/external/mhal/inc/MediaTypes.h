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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MEDIA_TYPES_H
#define _MEDIA_TYPES_H

/*******************************************************************************
*
********************************************************************************/
//
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
//
typedef void            MHAL_VOID;
typedef char            MHAL_BOOL;
typedef char            MHAL_CHAR;
typedef signed char     MHAL_INT8;
typedef signed short    MHAL_INT16;
typedef signed int      MHAL_INT32;
typedef unsigned char   MHAL_UCHAR;
typedef unsigned char   MHAL_UINT8;
typedef unsigned short  MHAL_UINT16;
typedef unsigned int    MHAL_UINT32;
typedef unsigned long long  MHAL_UINT64;

//
typedef MHAL_VOID       MVOID;
typedef MHAL_UINT8      MUINT8;
typedef MHAL_UINT16     MUINT16;
typedef MHAL_UINT32     MUINT32;
typedef MHAL_UINT64     MUINT64;
typedef MHAL_INT32      MINT32;

/*******************************************************************************
*
********************************************************************************/
#define READ32(addr)        *(MUINT32 *) (addr)
#define WRITE32(addr, val)  *(MUINT32 *) (addr) = (val)

#define MHAL_TRUE     1
#define MHAL_FALSE    0

/*******************************************************************************
*
********************************************************************************/
//typedef signed char         CHAR;
//typedef char                UCHAR;
#define CHAR                signed char
#define UCHAR               char
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef signed short        INT16;
//typedef signed int          BOOL;
#define BOOL                signed int
//typedef signed int          INT32;
#define INT32               signed int
typedef unsigned int        UINT32;
typedef long long           INT64;

#if !defined(UINT64)

#ifndef VCODEC_INC
typedef unsigned long long  UINT64;
#define VCODEC_INC
#endif

#endif

typedef float               FLOAT;
typedef double              DOUBLE;
typedef void                VOID;

typedef INT32 MRESULT;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif

/*******************************************************************************
*
********************************************************************************/

/**
 * @par Enumeration
 *   MHAL_ERROR_ENUM
 * @par Description
 *   This is the return status of each MHAL function
 */
typedef enum
{
    MHAL_NO_ERROR = 0,                  ///< The function work successfully
    MHAL_INVALID_DRIVER,                ///< Error due to invalid driver
    MHAL_INVALID_CTRL_CODE,             ///< Error due to invalid control code
    MHAL_INVALID_PARA,                  ///< Error due to invalid parameter
    MHAL_INVALID_MEMORY,                ///< Error due to invalid memory
    MHAL_INVALID_FORMAT,                ///< Error due to invalid file format
    MHAL_INVALID_RESOURCE,              ///< Error due to invalid resource, like IDP

    MHAL_UNKNOWN_ERROR = 0x80000000,    ///< Unknown error
    MHAL_ALL = 0xFFFFFFFF
} MHAL_ERROR_ENUM;


#endif // _MEDIA_TYPES_H


