/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __MTK_VT_EM_H__
#define __MTK_VT_EM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h> // For 'O_RDWR' & 'O_EXCL'
#include <android/log.h>
#include <semaphore.h>
#include <pthread.h>

typedef enum VTSMEDIATYPE{
	VTSEM_VIDEO = 0,
	VTSEM_AUDIO
}VTSMEDIATYPE;

typedef struct VTSEmDTFContext{
	char * fileName;
	FILE * fp;
	int isValid;
}VTSEmDTFContext;

typedef struct VTSEmLBContext{
	int isVidValid;	// 1 for loopback
	int isAudValid;
}VTSEmLBContext;

enum DTFPATHID{
	UNKNOWN_DTF = -1,
	AUDIO_UL_DTF_STK = 0,
	VIDEO_UL_DTF_STK,
	AUDIO_DL_DTF_STK,
	VIDEO_DL_DTF_STK,
	AUDIO_UL_DTF_MAL,
	VIDEO_UL_DTF_MAL,
	AUDIO_DL_DTF_MAL,
	VIDEO_DL_DTF_MAL,
	DTF_NR
};

typedef struct VTSEmContext{
	VTSEmLBContext networkLBContext;
	VTSEmLBContext mediaLBContext;
	VTSEmDTFContext vtsEmDTFContext[DTF_NR];
}VTSEmContext;

typedef struct VTSEmContextFunc{
	void (*configNetworkLB)(int, int);
	int (*isNetworkLB)(int);
	void (*configMediaLB)(int, int);
	int (*isMediaLB)(int);
	int (*configDTFEnable)(int);
	void (*configDTFDisable)(int);
	int (*dumpDataToFile)(int, unsigned char * , int);
	int (*isDTFEnable)(int);
}VTSEmDTFFunc;
void initEm();

extern struct VTSEmContextFunc gVTSEmContextFunc;

#ifdef __cplusplus
}
#endif

#endif /* __MTK_VT_DEFS_H__ */

