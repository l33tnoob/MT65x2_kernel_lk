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

#define LOG_TAG "EMDFO"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <cutils/xlog.h>
#include "ModuleDfo.h"
#include "RPCClient.h"

extern "C"{
#include "meta_dfo.h"
#include "meta_dfo_para.h"
#include "mtkfb.h"
#include "mtkfb_info.h"
}

#define FB_DEV_NODE "/dev/graphics/fb0"

/*
// TODO: remove unit test code
const unsigned char META_SUCCESS = '0';
int width = 0;
int height = 0;
typedef struct{ int count;} Dfo_read_count_cnf;
typedef struct{ int index; } Dfo_read_req;
typedef struct{ char name[32]; int value; int partition; } Dfo_read_cnf;
typedef struct{ char name[32]; int value; int partition; int save; } Dfo_write_req;
bool META_Dfo_Init(void) {return true;}
void META_Dfo_Deinit(void) {}
unsigned char Dfo_ReadCount_OP(Dfo_read_count_cnf* cnf) {cnf->count = 3; return META_SUCCESS;}
unsigned char Dfo_ReadValue_OP(Dfo_read_req* req, Dfo_read_cnf* cnf) {
	if (req->index == 1) {
		strcpy(cnf->name, "LCM_WIDTH");
		cnf->value = width;
		cnf->partition = 1;
	} else if (req->index == 2) {
		strcpy(cnf->name, "LCM_HEIGHT");
		cnf->value = height;
		cnf->partition = 1;
	}
	return META_SUCCESS;
}
unsigned char Dfo_WriteValue_OP(Dfo_write_req* req) {
	if (strcmp(req->name, "LCM_WIDTH") == 0) {
		width = req->value;
	} else if (strcmp(req->name, "LCM_HEIGHT") == 0) {
		height = req->value;
	}
	return META_SUCCESS;
}
// TODO: remove unit test code
*/


static FT_DFO_CNF dfoCnf;

static void dfoCallback(void *buf, unsigned int size) {
	memcpy(&dfoCnf, buf, sizeof(FT_DFO_CNF));
};

int ModuleDfo::init(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	XLOGD("Enter META_Dfo_Init()\n");
	bool ret = META_Dfo_Init();
	XLOGD("META_Dfo_Init() returned %d\n", ret);

	if (ret) {
		msgSender->PostMsg((char*) SUCCESS);
	} else {
		msgSender->PostMsg((char*) ERROR);
	}
	return 0;
}

int ModuleDfo::readCount(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	// Init request
	FT_DFO_REQ req;
	memset(&req, 0, sizeof(req));
	req.op = DFO_OP_READ_COUNT;

	// Get DFO count
	XLOGD("Enter Dfo_ReadCount_OP()\n");
	META_Dfo_SetCallback(dfoCallback);
	META_Dfo_OP(&req);
	Dfo_read_count_cnf *cnf = &(dfoCnf.result.read_count_cnf);
	XLOGD("Dfo_ReadCount_OP() returned %d, count = %d\n", dfoCnf.status,
			cnf->count);

	// Return result
	if (dfoCnf.status == META_SUCCESS) {
		char result[RESULT_SIZE] = { 0 };
		snprintf(result, RESULT_SIZE, "%d", cnf->count);
		msgSender->PostMsg((char*) SUCCESS);
		msgSender->PostMsg(result);
	} else {
		msgSender->PostMsg((char*) ERROR);
	}
	return 0;
}

int ModuleDfo::read(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 1) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	int index = msgSender->ReadInt();

	// Init request
	FT_DFO_REQ req;
	memset(&req, 0, sizeof(req));
	req.op = DFO_OP_READ;
	req.cmd.read_req.index = index;

	// Get DFO value
	XLOGD("Enter Dfo_ReadValue_OP(), index = %d\n", index);
	META_Dfo_SetCallback(dfoCallback);
	META_Dfo_OP(&req);
	Dfo_read_cnf *cnf = &(dfoCnf.result.read_cnf);
	XLOGD("Dfo_ReadValue_OP() returned %d, value = %d, partition = %d\n",
			dfoCnf.status, cnf->value, cnf->partition);

	// Return result
	if (dfoCnf.status == META_SUCCESS) {
		msgSender->PostMsg((char*) SUCCESS);
		char result[RESULT_SIZE] = { 0 };
		snprintf(result, sizeof(cnf->name), "%s", cnf->name);
		XLOGD("name = %s\n", result);
		msgSender->PostMsg(result);
		snprintf(result, RESULT_SIZE, "%d", cnf->value);
		msgSender->PostMsg(result);
		snprintf(result, RESULT_SIZE, "%d", cnf->partition);
		msgSender->PostMsg(result);
	} else {
		msgSender->PostMsg((char*) ERROR);
	}
	return 0;
}

int ModuleDfo::write(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 4) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_STRING) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	int L = msgSender->ReadInt();
	char *buf = new char[L + 1];
	memset(buf, 0, L + 1);
	msgSender->Receive(buf, L);

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	int value = msgSender->ReadInt();

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	int partition = msgSender->ReadInt();

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	int save = msgSender->ReadInt();

	// Init request
	FT_DFO_REQ req;
	memset(&req, 0, sizeof(req));
	req.op = DFO_OP_WRITE;
	strncpy(req.cmd.write_req.name, buf, sizeof(req.cmd.write_req.name) - 1);
	req.cmd.write_req.value = value;
	req.cmd.write_req.partition = partition;
	req.cmd.write_req.save = save;

	// Write DFO value
	XLOGD("Enter Dfo_WriteValue_OP(), name = %s, value = %d, partition = %d, save = %d\n",
			req.cmd.write_req.name, req.cmd.write_req.value, req.cmd.write_req.partition, req.cmd.write_req.save);
	META_Dfo_SetCallback(dfoCallback);
	META_Dfo_OP(&req);
	Dfo_write_cnf *cnf = &(dfoCnf.result.write_cnf);
	XLOGD("Dfo_WriteValue_OP() returned %d\n", dfoCnf.status);

	// Return result
	if (dfoCnf.status == META_SUCCESS) {
		msgSender->PostMsg((char*) SUCCESS);
	} else {
		msgSender->PostMsg((char*) ERROR);
	}
	return 0;
}

int ModuleDfo::deinit(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	XLOGD("Enter META_Dfo_Deinit()\n");
	META_Dfo_Deinit();
	XLOGD("META_Dfo_Deinit() returned\n");

	msgSender->PostMsg((char*) SUCCESS);
	return 0;
}

int ModuleDfo::propertySet(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 2) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	int height = msgSender->ReadInt();

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	int width = msgSender->ReadInt();

	char value[RESULT_SIZE] = { 0 };
	snprintf(value, RESULT_SIZE, "%dx%d", height, width);
	XLOGD("Enter property_set(), value = %s\n", value);
	int ret = property_set("persist.sys.current.screensize", value);
	XLOGD("property_set() returned %d\n", ret);

	if (ret >= 0) {
		msgSender->PostMsg((char*) SUCCESS);
	} else {
		msgSender->PostMsg((char*) ERROR);
	}
	return 0;
}

int ModuleDfo::getDefaultSize(RPCClient* msgSender) {
	if (msgSender->ReadInt() != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	mtk_dispif_info info;
	memset(&info, 0, sizeof(info));

	XLOGD("Enter MTKFB_GET_DISPLAY_IF_INFORMATION\n");
	int fd = open(FB_DEV_NODE, O_RDONLY);
	ioctl(fd, MTKFB_GET_DISPLAY_IF_INFORMATION, &info);
	::close(fd);
	XLOGD("MTKFB_GET_DISPLAY_IF_INFORMATION returned %d, %d, %d\n", info.displayMode, info.lcmOriginalWidth, info.lcmOriginalHeight);

	// If driver not support, return -1
	if (info.displayMode == DISPIF_MODE_VIDEO) {
		info.lcmOriginalWidth = -1;
		info.lcmOriginalHeight = -1;
	}

	msgSender->PostMsg((char*) SUCCESS);
	char value[RESULT_SIZE] = { 0 };
	snprintf(value, RESULT_SIZE, "%d", info.lcmOriginalWidth);
	msgSender->PostMsg(value);
	snprintf(value, RESULT_SIZE, "%d", info.lcmOriginalHeight);
	msgSender->PostMsg(value);
	return 0;
}

ModuleDfo::ModuleDfo(void) {
}

ModuleDfo::~ModuleDfo(void) {
}

