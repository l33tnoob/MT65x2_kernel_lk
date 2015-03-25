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

/*
	SRS Labs CONFIDENTIAL
	@Copyright 2009 by SRS Labs.
	All rights reserved.

	(RCS keywords below, do not edit)
	$Id: //srstech/srs_common/std_fxp/include/srs_typedefs.h#9 $
  $Author: weiguoy $
  $Date: 2010/10/26 $
	
	Defines various basic data types
	
	This file should not contain anything but #defines and no // comments.
	This is because it is used to preprocess many kinds of files.
*/

#ifndef __SRS_TYPEDEFS_H__
#define __SRS_TYPEDEFS_H__

#include "srs_platdefs.h"

/******************************************************************************
 *Important Note:
 *   Read on if you are porting SRS IPs to your platform
 * Carefully check the integer type definitions below to see if they hold for
 * your platform. If not, redefine them for your platform
 *****************************************************************************/


#define srs_int8	char
#define srs_int16	short
#define srs_int32   int

typedef unsigned char 		srs_uint8;
typedef unsigned short 		srs_uint16;
typedef unsigned int		srs_uint32;

#if SRS_TIC6X	/*long is 40-bit in TI compiler CCS for C6000 DSP's*/
#define srs_int40	long
typedef unsigned long srs_uint40;
#endif

typedef long long srs_int64;
typedef unsigned long long srs_uint64;

/*Stereo channel*/
typedef struct
{
	srs_int32 *Left;
	srs_int32 *Right;
} SRSStereoCh;

typedef struct
{
	srs_int32 *Left;
	srs_int32 *Right;
	srs_int32 *Center;
	srs_int32 *Sub;
	srs_int32 *SLeft;
	srs_int32 *SRight;
	srs_int32 *SCenter;
} SRS6_1Ch;

typedef struct
{
	srs_int32 *Left;
	srs_int32 *Right;
	srs_int32 *Center;
	srs_int32 *Sub;
	srs_int32 *SLeft;
	srs_int32 *SRight;
} SRS5_1Ch;

typedef enum
{
	SRS_NO_ERROR,
	SRS_INVALID_PARAMETER,
	SRS_NOT_SUPPORTED,
	SRS_INCOMPATIBLE
} SRSResult;


typedef enum
{
	SRSFilter16,		//PEQ filters coefficients are in 16-bit fixed-point
	SRSFilter32			//PEQ filters coefficients are in 32-bit fixed-point
} SRSFilterWl;	//filter word length


//SRSIOMode: descripting the I/O channels of SRS modules:
#define SRSIOLeftChBit					(1)
#define SRSIORightChBit					(1 << 1)
#define SRSIOCenterChBit				(1 << 2)
#define SRSIOSubChBit					(1 << 3)
#define SRSIOSLeftChBit					(1 << 4)
#define SRSIOSRightChBit				(1 << 5)
#define SRSIOSCenterChBit				(1 << 6)
#define SRSIOLtRtBit					(1 << 8)
#define SRSIOHeadphoneBit				(1 << 9)
#define SRSIOBSDigitalBit				(1 << 10)
#define SRSIOPL2MusicBit				(1 << 11)
//#define SRSIOCSIIBit					(1 << 12)

typedef enum
{
	SRS_IO_1_0_0 = SRSIOLeftChBit,	//a.k.a SRS_IO_MONO, mono input/output
	SRS_IO_2_0_0 = SRSIOLeftChBit | SRSIORightChBit,	//a.k.a SRS_IO_STEREO, stereo input/output
	SRS_IO_3_0_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit,
	SRS_IO_2_0_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSubChBit,
	SRS_IO_3_0_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSubChBit,
	SRS_IO_2_2_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSLeftChBit | SRSIOSRightChBit,
	SRS_IO_3_2_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSLeftChBit | SRSIOSRightChBit,
	SRS_IO_2_2_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSubChBit | SRSIOSLeftChBit | SRSIOSRightChBit,
	SRS_IO_3_2_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSubChBit | SRSIOSLeftChBit | SRSIOSRightChBit,
	SRS_IO_2_1_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSCenterChBit,
	SRS_IO_3_1_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSCenterChBit,
	SRS_IO_2_1_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSubChBit | SRSIOSCenterChBit,
	SRS_IO_3_1_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSubChBit | SRSIOSCenterChBit,
	SRS_IO_2_3_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOSCenterChBit,
	SRS_IO_3_3_0 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOSCenterChBit,
	SRS_IO_2_3_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOSubChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOSCenterChBit,
	SRS_IO_3_3_1 = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSubChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOSCenterChBit,
	SRS_IO_LtRt = SRSIOLeftChBit | SRSIORightChBit | SRSIOLtRtBit,
	SRS_IO_Headphone = SRSIOLeftChBit | SRSIORightChBit | SRSIOHeadphoneBit,
	SRS_IO_BSDigital = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSubChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOBSDigitalBit,	//woth subwoofer or not?
	SRS_IO_PL2Music =  SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSubChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOPL2MusicBit	//with subwoofer or not?
	//	SRS_IO_CSII = SRSIOLeftChBit | SRSIORightChBit | SRSIOCenterChBit | SRSIOSLeftChBit | SRSIOSRightChBit | SRSIOCSIIBit,	//with new cs decoder IP, the subwoofer output is mixed into SLeft and SRight
} SRSIOMode;

#define SRS_IO_MONO		SRS_IO_1_0_0
#define SRS_IO_STEREO	SRS_IO_2_0_0

/*
 *	Technology Version Number
 *
 *	kSrsVersionMajor
 *
 *	The major version number of a technology will change if
 *	backward compatibility with previous versions is broken.
 *
 *	kSrsVersionMinor
 *
 *	The minor version is incremented upon each new release of the
 *	software that includes new features, but maintains backward
 *	compatibility with the previous release.
 *
 *	kSrsVersionRevision
 *
 *	The revision is incremented upon each new release that includes
 *	minor modifications or fixes, but no additional new features.
 *	Revision increments also maintain backward compatibility
 *	with the previous release.
 *
 *	kSrsVersionRelease
 *
 *	The release number is either zero or 255 for standard releases along
 *	the main development path.  The value 255 should be considered to be
 *	equivalent to the value zero.  Other number ranges for the release have
 *	the following meanings:
 *
 *		  1 -  99	Reserved for internal use by SRS Labs, Inc.
 *		100 - 199	Alpha release
 *		200 - 254	Beta release
 *
 *	Depending on the purpose of the release, nonzero release numbers may or
 *	may not maintain compatibility with the corresponding standard release.
 *
 */
typedef enum
{
	SRS_VERSION_MAJOR,
    SRS_VERSION_MINOR,
    SRS_VERSION_REVISION,
    SRS_VERSION_RELEASE
} SRSVersion;

#endif /*__SRS_TYPEDEFS_H__*/
