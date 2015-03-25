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

#ifndef VIDEO_TELEPHONE_UTILS_H
#define VIDEO_TELEPHONE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mtk_vt_utils.h"
#include "mtk_vt_defs.h"
#include "mtk_vt_tasks.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <semaphore.h>

	#define VTSString 		String8
	#define VTSCamParam 	String8
    /* MTK80976 */
	#define VTSSurface 		Surface
	#define VTSBuffer 		IMemory
	#define VTSThread_t 	pthread_t
	#define VTSMAObject 	IVTMultiMediaAdaptor
	#define VTSMAMediaObj 	IMediaObject
	#define VTSMALStub 		VTSMALDroidStub
	#define VTSMALMBuffer	IMediaBuffer
	#define VTSChannelInfo	media_vcall_channel_status_ind_struct

	#define VtStk_Status 	int
	
	#define LIKELY (exp) (__builtin_expect((exp)!=0, true))
	#define UNLIKELY (exp) (__builtin_expect((exp)!=0, false))


	#define VTS_DATA_PATH_NR		4

	typedef struct VTSQueueHeader{
		struct VTSQueueNode* next; 
	}VTSQueueHeader, * PtrVTSQueueHeader;
	
	typedef enum VTSDATAPATH{
		PEER_AUDIO,
		PEER_VIDEO,
		LOCAL_AUDIO,
		LOCAL_VIDEO,
		VTS_PATH_NR
	}VTSDATAPATH;

	typedef struct VideoConfig{
		int width;
		int height;
	}VideoConfig, *PtrVideoConfig;

	typedef struct AudioConfig{
		int frameBufNr;
		int sampleSize;
		int channelNr;
		int sampleRate;
	}AudioConfig, *PtrAudioConfig;
	
	typedef struct ChannelConfig{
		int type;
		union {
			VideoConfig vidConfig;
			AudioConfig audConfig;
		};
	}ChannelConfig,* PtrChannelConfig;
	
	typedef struct vpInfo_t{
			unsigned int addr;
			unsigned int size;
	}vpInfo_t;
	
	typedef struct VTSVFrameInfo{
		char * mimeType;
		int width;
		int height;
		int realSize;
		int packetNr;
		int infoLen;
		struct vpInfo_t * vpInfo;
	}VTSVFrameInfo, *PtrVTSVFrameInfo;

	typedef struct VTSAFrameInfo{
		char * mimeType;
		int realSize;
	}VTSAFrameInfo, *PtrVTSAFrameInfo;

#define STK_HANDLE_NR (VTMSG_ID_STK_HANDLE_BY_SVC_END - VTMSG_ID_STK_HANDLE_BY_SVC_BEGIN -1)
#define VT_SVC_HANDL_NR 1

#define QCIF_W 176
#define QCIF_H 144
#define SQCIF_W 128
#define SQCIF_H 96

#ifdef __cplusplus
}
#endif

#endif
		
