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

#define LOG_TAG "EMSENSOR"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/xlog.h>
#include "ModuleSensor.h"
#include "RPCClient.h"
extern "C"{
#include "libhwm.h"
}

int ModuleSensor::doCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int min = 0, max = 0;
	if (paraNum != 2) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	min = msgSender->ReadInt();

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	max = msgSender->ReadInt();

	XLOGD("Enter do_calibration() min %d, max %d\n", min, max);
	int ret = do_calibration(min, max);
	XLOGD("do_calibration() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::clearCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	if (paraNum != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	XLOGD("Enter clear_psensor_calibration()\n");
	int ret = clear_psensor_calibration();
	XLOGD("clear_psensor_calibration() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::setThreshold(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int high = 0, low = 0;
	if (paraNum != 2) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	high = msgSender->ReadInt();

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	low = msgSender->ReadInt();

	XLOGD("Enter set_psensor_threshold() min %d, max %d\n", high, low);
	int ret = set_psensor_threshold(high, low);
	XLOGD("set_psensor_threshold() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::doGsensorCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int tolerance = 0;
	if (paraNum != 1) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	tolerance = msgSender->ReadInt();

	XLOGD("Enter do_gsensor_calibration() tolerance %d\n", tolerance);
	int ret = do_gsensor_calibration(tolerance);
	XLOGD("do_gsensor_calibration() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::getGsensorCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int tolerance = 0;
	if (paraNum != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	float x = 0.0f, y = 0.0f, z = 0.0f;
	XLOGD("Enter get_gsensor_calibration()\n");
	int ret = get_gsensor_calibration(&x, &y, &z);
	XLOGD("get_gsensor_calibration() returned %d, %f, %f, %f\n", ret, x, y, z);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	sprintf(result, "%f", x);
	msgSender->PostMsg(result);
	sprintf(result, "%f", y);
	msgSender->PostMsg(result);
	sprintf(result, "%f", z);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::clearGsensorCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int tolerance = 0;
	if (paraNum != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	XLOGD("Enter clear_gsensor_calibration()\n");
	int ret = clear_gsensor_calibration();
	XLOGD("clear_gsensor_calibration() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::doGyroscopeCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int tolerance = 0;
	if (paraNum != 1) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	if (msgSender->ReadInt() != PARAM_TYPE_INT) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}
	msgSender->ReadInt();
	tolerance = msgSender->ReadInt();

	XLOGD("Enter do_gyroscope_calibration() tolerance %d\n", tolerance);
	int ret = do_gyroscope_calibration(tolerance);
	XLOGD("do_gyroscope_calibration() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::getGyroscopeCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int tolerance = 0;
	if (paraNum != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	float x = 0.0f, y = 0.0f, z = 0.0f;
	XLOGD("Enter get_gyroscope_calibration()\n");
	int ret = get_gyroscope_calibration(&x, &y, &z);
	XLOGD("get_gyroscope_calibration() returned %d, %f, %f, %f\n", ret, x, y, z);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	sprintf(result, "%f", x);
	msgSender->PostMsg(result);
	sprintf(result, "%f", y);
	msgSender->PostMsg(result);
	sprintf(result, "%f", z);
	msgSender->PostMsg(result);
	return 0;
}

int ModuleSensor::clearGyroscopeCalibration(RPCClient* msgSender) {
	int paraNum = msgSender->ReadInt();
	int tolerance = 0;
	if (paraNum != 0) {
		msgSender->PostMsg((char*) ERROR);
		return -1;
	}

	XLOGD("Enter clear_gyroscope_calibration()\n");
	int ret = clear_gyroscope_calibration();
	XLOGD("clear_gyroscope_calibration() returned %d\n", ret);

	char result[RESULT_SIZE] = { 0 };
	sprintf(result, "%d", ret);
	msgSender->PostMsg(result);
	return 0;
}

ModuleSensor::ModuleSensor(void) {
}

ModuleSensor::~ModuleSensor(void) {
}

