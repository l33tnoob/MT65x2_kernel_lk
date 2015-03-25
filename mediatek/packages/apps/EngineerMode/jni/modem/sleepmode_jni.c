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

#define LOG_TAG "SleepMode"
//#define LOG_NDEBUG 1

#include "jni.h"
#include "JNIHelp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cutils/xlog.h>
//#include <cutils/log.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "JNIHelp.h"

#include <errno.h>
#include <sys/ioctl.h>

#define CCCI_UEM_TX 19

typedef enum {
	UEM_CCCI_EM_REQ_GET_SLEEP_MODE = 0,
	UEM_CCCI_EM_REQ_SET_SLEEP_MODE,
	UEM_CCCI_EM_REQ_GET_DCM_MODE,
	UEM_CCCI_EM_REQ_SET_DCM_MODE,

} MD_EM_EVENT_TYPE;

JNIEXPORT jboolean Java_com_mediatek_engineermode_modem_SleepMode_setSleepMode(
		JNIEnv* env, jobject clazz, jint dwReserved) {
	XLOGW("%d", dwReserved);

	unsigned int Message[4] = { 0xFFFFFFFF, UEM_CCCI_EM_REQ_SET_SLEEP_MODE,
			CCCI_UEM_TX, dwReserved };

	int file = -1;
	file = open("/dev/ccci_uem_tx", O_WRONLY);
	if (-1 == file) {
		return 0;
	}

	XLOGW("open file successful");
	write(file, Message, sizeof(Message));
	XLOGW("write file successful");
	XLOGW("After set SleepMode: %d %d %d %d", Message[0], Message[1],
			Message[2], Message[3]);
	close(file);

	int fileRead = -1;
	fileRead = open("/dev/ccci_uem_rx", O_RDONLY);
	if (-1 == fileRead) {
		XLOGW("open ccci_uem_rx failed");
		return -1;
	}
	XLOGW("open ccci_uem_rx successful");
	XLOGW("Source: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	read(fileRead, Message, sizeof(Message));
	XLOGW("read file successful");
	XLOGW("Recv: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	close(fileRead);

	return 1;
}

JNIEXPORT jint Java_com_mediatek_engineermode_modem_SleepMode_getSleepMode(
		JNIEnv* env, jobject clazz) {
	XLOGW("getSleepMode");
	unsigned int Message[4] = { 0xFFFFFFFF, UEM_CCCI_EM_REQ_GET_SLEEP_MODE,
			CCCI_UEM_TX, 0 };
	// set read cmd
	int file = -1;
	file = open("/dev/ccci_uem_tx", O_WRONLY);
	if (-1 == file) {
		XLOGW("open ccci_uem_tx failed");
		return -1;
	}

	XLOGW("open file successful");
	write(file, Message, sizeof(Message));
	XLOGW("write file successful");
	close(file);

	int fileRead = -1;
	fileRead = open("/dev/ccci_uem_rx", O_RDONLY);
	if (-1 == fileRead) {
		XLOGW("open ccci_uem_rx failed");
		return -1;
	}
	XLOGW("open ccci_uem_rx successful");
	XLOGW("Source: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	read(fileRead, Message, sizeof(Message));
	XLOGW("read file successful");
	XLOGW("Recv: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	close(fileRead);

	return  Message[3];
}

JNIEXPORT jboolean Java_com_mediatek_engineermode_modem_SleepMode_setDCM(
		JNIEnv* env, jobject clazz, jint dwReserved) {
	XLOGW("%d", dwReserved);

	unsigned int Message[4] = { 0xFFFFFFFF, UEM_CCCI_EM_REQ_SET_DCM_MODE,
			CCCI_UEM_TX, dwReserved };

	int file = -1;
	file = open("/dev/ccci_uem_tx", O_WRONLY);
	if (-1 == file) {
		return 0;
	}

	XLOGW("open file successful");
	write(file, Message, 16);
	XLOGW("write file successful");
	XLOGW("After set DCM: %d %d %d %d", Message[0], Message[1], Message[2],
			Message[3]);
	close(file);

	int fileRead = -1;
	fileRead = open("/dev/ccci_uem_rx", O_RDONLY);
	if (-1 == fileRead) {
		XLOGW("open ccci_uem_rx failed");
		return -1;
	}
	XLOGW("open ccci_uem_rx successful");
	XLOGW("Source: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	read(fileRead, Message, sizeof(Message));
	XLOGW("read file successful");
	XLOGW("Recv: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	close(fileRead);

	return 1;
}

JNIEXPORT jint Java_com_mediatek_engineermode_modem_SleepMode_getDCM(
		JNIEnv* env, jobject clazz) {
	XLOGW("getDCM");
	unsigned int Message[4] = { 0xFFFFFFFF, UEM_CCCI_EM_REQ_GET_DCM_MODE,
			CCCI_UEM_TX, 0 };
	// set read cmd
	int file = -1;
	file = open("/dev/ccci_uem_tx", O_WRONLY);
	if (-1 == file) {
		XLOGW("open ccci_uem_tx failed");
		return -1;
	}

	XLOGW("open file successful");
	write(file, Message, sizeof(Message));
	XLOGW("write file successful");
	close(file);

	int fileRead = -1;
	fileRead = open("/dev/ccci_uem_rx", O_RDONLY);
	if (-1 == fileRead) {
		XLOGW("open ccci_uem_rx failed");
		return -1;
	}
	XLOGW("open ccci_uem_rx successful");
	XLOGW("Source: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	read(fileRead, Message, sizeof(Message));
	XLOGW("read file successful");
	XLOGW("Recv: %d %d %d %d", Message[0], Message[1], Message[2], Message[3]);
	close(fileRead);

	return Message[3];
}

