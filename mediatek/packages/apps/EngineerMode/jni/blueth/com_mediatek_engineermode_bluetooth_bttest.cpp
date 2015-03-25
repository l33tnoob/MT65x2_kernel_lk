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

/* //device/libs/android_runtime/android_media_MediaPlayer.cpp
 **
 ** Copyright 2007, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

//#define LOG_NDEBUG 0
//#define LOG_TAG "emBtTest-JNI"
//#include "utils/Log.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <utils/threads.h>
#include <dlfcn.h>
#include <string.h>
#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "utils/Errors.h"  // for status_t
#undef LOG_NDEBUG 
#undef NDEBUG

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "emBtTest-JNI"
#endif

#include "bt_em.h"

#include <cutils/xlog.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
//typedef int BOOL;


extern "C" void RELAYER_exit(void);
 extern "C" BOOL RELAYER_start(int serial_port, int serial_speed);
extern "C" int setFWDump(unsigned int fwdump);
extern "C" BOOL bt_set_ssp_debug_mode(BOOL on);

/*
extern "C" void EM_BT_deinit(void);
extern "C" BOOL EM_BT_init(void);

extern "C" BOOL EM_BT_read(unsigned char *peer_buf, int peer_len,int* piResultLen);

extern "C" BOOL EM_BT_write(unsigned char *peer_buf, int peer_len);


 typedef BOOL (*bt_init)(void);
 typedef void (*bt_deinit)(void);
 typedef BOOL (*bt_read)(
 unsigned char *peer_buf,
 int  peer_len,
 int* piResultLen);
 typedef BOOL (*bt_write)(
 unsigned char *peer_buf,
 int peer_len);

 static bt_init EM_BT_init = NULL;
 static bt_deinit EM_BT_deinit = NULL;
 static bt_read EM_BT_read = NULL;
 static bt_write EM_BT_write = NULL;
 */

// ----------------------------------------------------------------------------

using namespace android;

// ----------------------------------------------------------------------------

struct fields_t {
	jfieldID patter;
	jfieldID channels;
	jfieldID pocketType;
	jfieldID pocketTypeLen;
	jfieldID freq;
	jfieldID power;
};
static fields_t fields;

static unsigned char Pattern_Map[] = { 0x01, 0x02, 0x03, 0x04, 0x09 };
static unsigned char PocketType_Map[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E, 0x0F, 0x17, 0x1C, 0x1D, 0x24,
		0x28, 0x2A, 0x2B, 0x2E, 0x2F, 0x36, 0x37, 0x3C, 0x3D };
static unsigned char Channels_Map[] = { 0x00, 0x01 };

//
//

//
static bool DoBTReset() {
	XLOGE("Enter DoBTReset()...\n");
	//	unsigned char peer_write_buf[256] = { 0 };
	unsigned char ucEvent[512];
	int iResultLen = 0;
	unsigned char HCI_RESET[] = { 0x01, 0x03, 0x0c, 0x0 };

	if (false == EM_BT_write(HCI_RESET, sizeof(HCI_RESET))) {
		XLOGE("Leave DoBTReset() due to EM_BT_write() failure...\n");
		EM_BT_deinit();
		return false;
	}

	XLOGD("EM_BT_write");
	for (int i = 0; i < sizeof(HCI_RESET); i++) {
		XLOGD("%1$02x	", HCI_RESET[i]);
	}

	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave DoBTReset() due to EM_BT_read() failure...\n");
		EM_BT_deinit();
		return false;
	}

	XLOGD("EM_BT_read, length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}

	XLOGE("Leave DoBTReset()...\n");
	return true;
}

//
//
//
static bool process_init_param(JNIEnv *env, jobject thiz) {
	XLOGE("Enter process_init_param()...\n");
	jclass clazz = env->GetObjectClass(thiz);
	if (clazz == NULL) {
		XLOGE("Can't find com/mediatek/engineermode/BtTest");
		XLOGE(
				"Leave process_init_param() due to Can't find com/mediatek/engineermode/BtTest...\n");
		jniThrowException(env, "java/lang/Exception", NULL);
		return false;
	}

	fields.patter = env->GetFieldID(clazz, "mPatter", "I");
	if (NULL == fields.patter) {
		XLOGE(
				"Leave process_init_param() due to Can't find Bluetooth.iPatter...\n");
		jniThrowException(env, "java/lang/RuntimeException",
				"Can't find Bluetooth.mPatter");
		return false;
	}

	fields.channels = env->GetFieldID(clazz, "mChannels", "I");

	if (NULL == fields.channels) {
		XLOGE(
				"Leave process_init_param() due to Can't find Bluetooth.iChannels...\n");
		jniThrowException(env, "java/lang/RuntimeException",
				"Can't find Bluetooth.mChannels");
		return false;
	}

	fields.pocketType = env->GetFieldID(clazz, "mPocketType", "I");
	if (NULL == fields.pocketType) {
		XLOGE(
				"Leave process_init_param() due to Can't find Bluetooth.iPocketType...\n");
		jniThrowException(env, "java/lang/RuntimeException",
				"Can't find Bluetooth.mPocketType");
		return false;
	}

	fields.pocketTypeLen = env->GetFieldID(clazz, "mPocketTypeLen", "I");
	if (NULL == fields.pocketTypeLen) {
		XLOGE(
				"Leave process_init_param() due to Can't find Bluetooth.iPocketTypeLen...\n");
		jniThrowException(env, "java/lang/RuntimeException",
				"Can't find Bluetooth.mPocketTypeLen");
		return false;
	}

	fields.freq = env->GetFieldID(clazz, "mFreq", "I");
	if (NULL == fields.freq) {
		XLOGE(
				"Leave process_init_param() due to Can't find Bluetooth.iFreq...\n");
		jniThrowException(env, "java/lang/RuntimeException",
				"Can't find Bluetooth.mFreq");
		return false;
	}
	fields.power = env->GetFieldID(clazz, "mPower", "I");
	if (NULL == fields.power) {
		XLOGE(
				"Leave process_init_param() due to Can't find Bluetooth.iPower...\n");
		jniThrowException(env, "java/lang/RuntimeException",
				"Can't find Bluetooth.mPower");
		return false;
	}
	XLOGE("Leave process_init_param()...\n");
	return true;
}
static int post_process_txtest(JNIEnv *env, jobject thiz) {
	XLOGE("Enter post_process_txtest()...\n");
	EM_BT_deinit();
	XLOGE("Leave post_process_txtest()...\n");	
	return 0;
}
//
//
//
static int process_txtest(JNIEnv *env, jobject thiz) {
	XLOGE("Enter process_txtest()...\n");
	if (!process_init_param(env, thiz)) {
		XLOGE("Leave process_txtest() due to process_init_param() failure...\n");
		return -1;
	}

	//	jclass clazz = env->GetObjectClass(env, thiz);
	//	if (clazz == NULL) {
	//		XLOGE("Can't find com/mediatek/engineermode/BtTest");
	//		XLOGE(
	//				"Leave process_txtest() due to can't find com/mediatek/engineermode/BtTes...\n");
	//		jniThrowException(env, "Ljava/lang/Exception", NULL);
	//		return -1;
	//	}

	int nPatter = (int) env->GetIntField(thiz, fields.patter);
	int nChannels = (int) env->GetIntField(thiz, fields.channels);
	int nPocketType = (int) env->GetIntField(thiz, fields.pocketType);
	int nPocketTypeLen = (int) env->GetIntField(thiz, fields.pocketTypeLen);
	int nFreq = (int) env->GetIntField(thiz, fields.freq);

//	XLOGI("nPatter = %d", nPatter);
//	XLOGI("nChannels = %d", nChannels);
//	XLOGI("nPocketType = %d", nPocketType);
//	XLOGI("nPocketTypeLen = %d", nPocketTypeLen);
//	XLOGI("nFreq = %d", nFreq);

	if (EM_BT_init() == false) {
		XLOGE("Leave process_txtest() due to EM_BT_init() failure...\n");
		//jniThrowException(env, "BT init failed", NULL);
		return -1;
	}

	unsigned char ucEvent[512];
	int iResultLen = 0;
/*	
	unsigned char peer_write_buf3[] = { 0x01, 0x1A, 0x0C, 0x01, 0x03 };

	if (false == EM_BT_write(peer_write_buf3, sizeof(peer_write_buf3))) {
		XLOGE("Leave process_modetest() due to EM_BT_write() failure... (3)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf3); i++) {
		XLOGD("%1$02x	", peer_write_buf3[i]);
	}

	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave process_modetest() due to EM_BT_read() failure. (3)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGE("EM_BT_read:length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}

	*////LXO comment, because Posh Sun said so.
	//unsigned char HCI_TX_TEST ={0x};

/*	if (false == DoBTReset()) {
		XLOGE("Leave process_txtest() due to DoBTReset() failure...\n");
		return -1;
	}
*/
	unsigned char peer_write_buf[] = { 0x01, 0x0D, 0xfc, 0x17, 0x00, 0x00,
			Pattern_Map[nPatter], Channels_Map[nChannels], nFreq, 0x00, 0x00, 0x01,
			PocketType_Map[nPocketType], ((unsigned char) (nPocketTypeLen
					&0xff)), ((unsigned char) ((0xff00 & nPocketTypeLen)
					>> 8)), 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00 };

	if (false == EM_BT_write(peer_write_buf, sizeof(peer_write_buf))) {
		XLOGE("Leave process_txtest() due to EM_BT_write() failure...\n");
		EM_BT_deinit();
		return -1;
	}

//	jstring result;
	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf); i++) {
		XLOGD("%1$02x	", peer_write_buf[i]);
//		result += "	"+peer_write_buf[i];
	}
//	XLOGD(result);

	memset(ucEvent, 0, sizeof(ucEvent));
	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave process_txtest() due to EM_BT_read() failure...\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGE("EM_BT_read:length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}
	//this means there are more than one event data in this message, if iResultLen > 3 + ucEvent[2]
	//if there are 2 piece of event data in this message, we donot need to read event again
	if(!(iResultLen > 3 + ucEvent[2]))
	{
		
		memset(ucEvent, 0, sizeof(ucEvent));
        if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
                XLOGE("Leave process_txtest() due to EM_BT_read() failure...\n");
                EM_BT_deinit();
                return -1;
        }

        XLOGE("EM_BT_read:length = %d", iResultLen);
        for (int i = 0; i < iResultLen; i++) {
                XLOGE("%1$02x    ", ucEvent[i]);
        }
	}

// LXO comment for low power.
	//EM_BT_deinit();
	XLOGE("Leave process_txtest()...\n");

	return 0;
}

static int post_process_modetest(JNIEnv *env, jobject thiz) {
	XLOGE("Enter post_process_modetest()...\n");
	EM_BT_deinit();
	XLOGE("Leave post_process_modetest()...\n");
	return 0;
}

//
//
//
static int process_modetest(JNIEnv *env, jobject thiz) {
	XLOGE("Enter process_modetest()...\n");

	unsigned char ucEvent[512];
	int iResultLen = 0;
	if (!process_init_param(env, thiz)) {
		XLOGE("Leave process_modetest() due to process_init_param() failure...\n");
		return -1;
	}
	if (EM_BT_init() == false) {
		XLOGE("Leave process_modetest() due to EM_BT_init() failure...\n");
		//jniThrowException(env, "BT init failed", NULL);
		return -1;
	}

	//==============set power==============
	int nPower = (int) env->GetIntField(thiz, fields.power);
	unsigned char power_write[] = {0x01, 0x79, 0xfc, 0x06, 0x07, 0x80, 0x00, 0x06, 0x05, 0x07};
	if(nPower<=7 && nPower>=0)
	{
		power_write[4] = (unsigned char)nPower;
		power_write[9] = (unsigned char)nPower;
	}
	XLOGE("Power val %1$02x:", power_write[4]);
		
	if (false == EM_BT_write(power_write, sizeof(power_write))) {
		XLOGE("Leave process_modetest() due to power_write EM_BT_write() failure... (3)\n");
		EM_BT_deinit();
		return -1;
	}
	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave process_modetest() due to power_write EM_BT_read() failure. (3)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGE("Power EM_BT_read:");
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}
	//==============end set power============
	//	unsigned char peer_write_buf[] = { 0x01, 0x7a, 0xfc, 0x07, 0x00, 0x40,
	//			0x1f, 0x40, 0x1f, 0x00, 0x04 };

	/*	if (false == EM_BT_write(peer_write_buf, sizeof(peer_write_buf))) {
	 XLOGE("Leave process_modetest(): due to EM_BT_write() failure. (1)\n");
	 EM_BT_deinit();
	 return -1;
	 }

	 if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
	 XLOGE("Leave process_modetest() due to EM_BT_read() failure... (1)\n");
	 EM_BT_deinit();
	 return -1;
	 }
	 
	if (false == DoBTReset()) {
		XLOGE("Leave process_modetest() due to DoBTReset() failure... \n");
		return -1;
	}
*/
	////////
	unsigned char peer_write_buf2[] = { 0x01, 0x03, 0x18, 0x00 };

	if (false == EM_BT_write(peer_write_buf2, sizeof(peer_write_buf2))) {
		XLOGE("Leave process_modetest() due to EM_BT_write() failure... (2)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf2); i++) {
		XLOGD("%1$02x	", peer_write_buf2[i]);
	}

	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave process_modetest() due to EM_BT_read() failure... (2)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGE("EM_BT_read:length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}

	///////
	unsigned char peer_write_buf3[] = { 0x01, 0x1A, 0x0C, 0x01, 0x03 };

	if (false == EM_BT_write(peer_write_buf3, sizeof(peer_write_buf3))) {
		XLOGE("Leave process_modetest() due to EM_BT_write() failure... (3)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf3); i++) {
		XLOGD("%1$02x	", peer_write_buf3[i]);
	}

	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave process_modetest() due to EM_BT_read() failure. (3)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGE("EM_BT_read:length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}

	//////
	unsigned char peer_write_buf4[] = { 0x01, 0x05, 0x0C, 0x03, 0x02, 0x00, 0x02 };

	if (false == EM_BT_write(peer_write_buf4, sizeof(peer_write_buf4))) {
		XLOGE("Leave process_modetest() due to EM_BT_write() failure...  (4) \n");
		EM_BT_deinit();
		return -1;
	}

	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf4); i++) {
		XLOGD("%1$02x	", peer_write_buf4[i]);
	}

	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave process_modetest() due to EM_BT_read() fail...  (4)\n");
		EM_BT_deinit();
		return -1;
	}

	XLOGE("EM_BT_read:length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}

	//EM_BT_deinit();
	XLOGE("Leave process_modetest()...\n");
	return 0;
}

/*
	-1: means error occurs
	0: BLE feature is not support
	-1:BLE feature is support
*/
static  jint BtTest_isBLESupport(JNIEnv *env, jobject thiz)
{
	unsigned char ucEvent[512] = {0};
	int iResultLen = 0;
	int result = -1;
	/*TODO: judge if BLE feature is supported first by chipID
	&&&
	&&&
	&&&
	&&&
	&&&
	&&&	
	*/
	if (EM_BT_init() == false) {
		XLOGE("Leave isBLESupport() due to EM_BT_init() failure...\n");
		return -1;
	}
	unsigned char peer_write_buf4[] = { 0x01, 0x03, 0x10, 0x00 };
	XLOGD("Enter isBLESupport()...\n");
	if (false == EM_BT_write(peer_write_buf4, sizeof(peer_write_buf4))) {
		XLOGE("Leave isBLESupport() due to EM_BT_write() failure...  (4) \n");
		EM_BT_deinit();
		return -1;
	}

	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf4); i++) {
		XLOGD("%1$02x	", peer_write_buf4[i]);
	}

	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave isBLESupport() due to EM_BT_read() fail...  (4)\n");
		EM_BT_deinit();
		return -1;
	}
	XLOGE("EM_BT_read: event length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}
	if(iResultLen > 11)
	{
		result = (ucEvent[11] & (1 << 6)) == 0 ? 0 : 1;
	}
	else
	{
		result = 0;
	}
	EM_BT_deinit();
	XLOGD("Leave isBLESupport()...\n");
	return result;
}

static jcharArray BtTest_HCIReadEvent(JNIEnv *env, jobject thiz) {
	int i = 0;
	int iResultLen = 0;
	unsigned char ucResultArray[512] = {0};
	unsigned short usResultArray[512] = {0};
	jcharArray resultArray = NULL;
	XLOGD("Enter BtTest_HCIReadEvent()...\n");
	
	if (false == EM_BT_read(ucResultArray, sizeof(ucResultArray), &iResultLen)) {
		XLOGE("EM_BT_read call failed, Leave BtTest_HCIReadEvent()...\n");
		return NULL;
	}
	XLOGD("EM_BT_read:  event length = %d", iResultLen);
	for (i = 0; i < iResultLen; i++) {
		XLOGD("%1$02x", ucResultArray[i]);
		usResultArray[i] = ucResultArray[i];
	}
	resultArray = env->NewCharArray(iResultLen);
	jsize start = 0;
	jsize end = iResultLen;
	if(NULL != resultArray)
	{
    		env->SetCharArrayRegion(resultArray, start, end, (const jchar *)usResultArray);
	}
	XLOGD("Leave BtTest_HCIReadEvent()...\n");
	return resultArray;
}

static jcharArray BtTest_HCICommandRun(JNIEnv *env, jobject thiz, jcharArray HCICmdBuffer, jint HCICmdLength) {
	//call EM_BT_write to send HCI command
	int iResultLen = 0;
	unsigned char ucResultArray[512] = {0};
	unsigned short usResultArray[512] = {0};
	jchar* jniParameter = NULL;
	jcharArray resultArray = NULL;
	unsigned char *ucHCICmdBuffer = new unsigned char[HCICmdLength];
	XLOGD("Enter BtTest_HCICommandRun()...\n");
	//parameter validation check
	if (HCICmdBuffer) {
		jniParameter = (jchar *) env->GetCharArrayElements(HCICmdBuffer, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return NULL; // out of memory or no data to load                                                                  
		}
	} else {
		XLOGE("NULL java array of readEEPRom16");
		return NULL;
	}
	XLOGD("EM_BT_write:");
	for (int i = 0; i < HCICmdLength; i++) {
		ucHCICmdBuffer[i] = (unsigned char)*(jniParameter + i);
		/*
		XLOGD("addr(ucHCICmdBuffer + i) = %1$08x", ucHCICmdBuffer + i);
		XLOGD("value(ucHCICmdBuffer + i) = %1$02x",ucHCICmdBuffer[i]);
		*/
		XLOGD("%1$02x",ucHCICmdBuffer[i]);
	}
	if (!EM_BT_write(ucHCICmdBuffer, HCICmdLength))
	{
		XLOGE("EM_BT_write call failed, Leave BtTest_HCICommandRun()...\n");
		return NULL;
	}

	if (false == EM_BT_read(ucResultArray, sizeof(ucResultArray), &iResultLen)) {
		XLOGE("EM_BT_read call failed, Leave BtTest_HCICommandRun()...\n");
		return NULL;
	}
	XLOGD("EM_BT_read:  event length = %d", iResultLen);
	for (int i = 0; i < iResultLen; i++) {
		/*
		XLOGD("addr(ucResultArray + i) = %1$08x", ucResultArray + i);
		XLOGD("value(ucResultArray + i) = %1$02x", ucResultArray[i]);
		*/
		XLOGD("%1$02x", ucResultArray[i]);
		usResultArray[i] = ucResultArray[i];
	}
	resultArray = env->NewCharArray(iResultLen);
	jsize start = 0;
	jsize end = iResultLen;
	if(NULL != resultArray)
	{
    	env->SetCharArrayRegion(resultArray, start, end, (const jchar *)usResultArray);
		XLOGE("Leave BtTest_HCICommandRun()...\n");
	}
	else
	{
		XLOGE("Leave BtTest_HCICommandRun()...\n");
	}
	env->ReleaseCharArrayElements(HCICmdBuffer, jniParameter, JNI_ABORT);
	
	return resultArray;
}
static int BtTest_getChipId(JNIEnv *env, jobject thiz) {
	int chipId = 0x0;
#ifdef MT6620
	XLOGE("MTK_BT_CHIP = MT6620\n");
	chipId = 0x6620;
#endif
#ifdef MT6616
	XLOGE("MTK_BT_CHIP = MT6616\n");
	chipId = 0x6616;	
#endif

#ifdef MT6612
	XLOGE("MTK_BT_CHIP = MT6612\n");
	chipId = 0x6612;	
#endif
#ifdef MTK_MT6611
	XLOGE("MTK_BT_CHIP = MTK_MT6611\n");
	chipId = 0x6611;
#endif

#ifdef MTK_MT6612
	XLOGE("MTK_BT_CHIP = MTK_MT6612\n");
	chipId = 0x6612;
#endif

#ifdef MTK_MT6616
	XLOGE("MTK_BT_CHIP = MTK_MT6616\n");
	chipId = 0x6616;
#endif

#ifdef MTK_MT6620
	XLOGE("MTK_BT_CHIP = MTK_MT6620\n");
	chipId = 0x6620;
#endif

#ifdef MTK_MT6622
	XLOGE("MTK_BT_CHIP = MTK_MT6622\n");
	chipId = 0x6622;
#endif

#ifdef MTK_MT6626
	XLOGE("MTK_BT_CHIP = MTK_MT6626\n");
	chipId = 0x6626;
#endif

#ifdef MTK_MT6628
	XLOGE("MTK_BT_CHIP = MTK_MT6628\n");
	chipId = 0x6628;
#endif
	return chipId;
}
static jint BtTest_isComboSupport(JNIEnv *env, jobject thiz){

#ifdef MTK_COMBO_SUPPORT
	return 1;
#else
	return 0;
#endif
}

static int BtTest_Init(JNIEnv *env, jobject thiz) {

	return EM_BT_init() ? 0 : -1;
}

static int BtTest_UnInit(JNIEnv *env, jobject thiz) {
	EM_BT_deinit();
	return 0;
}

static int BtTest_doBtTest(JNIEnv *env, jobject thiz, jint kind) {
	XLOGE("Enter BtTest_doBtTest(kind=%x)... \n", kind);


	if (0 == kind) {
		return process_txtest(env, thiz);
	} else if(1 == kind){
		return process_modetest(env, thiz);
	}
	else if(2 == kind)
	{
		return post_process_modetest(env, thiz);
	}
	else if(3 == kind)	
	{
		return post_process_txtest(env, thiz);
	}
			
	XLOGE("Leave BtTest_doBtTest()...\n");
	return 0;
}

static unsigned char NoSigRxBBMap[]= {0x01, 0x02, 0x03, 0x04, 0x09};
static unsigned char NoSigRxRRMap[]= {0x03, 0x04, 0x0A, 0x0B, 0x0E, 0x0F,
										0x24, 0x28, 0x2A, 0x2B, 0x2E, 0x2F};
static jintArray BtTest_EndNoSigRxTest(JNIEnv *env, jobject thiz)
{
XLOGE("Enter BtTest_EndNoSigRxTest.");

	unsigned char ucEvent[512];
	int iResultLen = 0;
//=============================GET result===
	unsigned char peer_write_buf2[] = { 0x01, 0x0D, 0xFC, 0x17, 0x00, 0x00, 
	0xFF, 0x00,0x00,0x00, 0x00,0x01, 0x00,0x00,0x00,0x02,0x00,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	if (false == EM_BT_write(peer_write_buf2, sizeof(peer_write_buf2))) {
		XLOGE("Leave BtTest_NoSigRxTest() due to EM_BT_write(2) failure...\n");
		EM_BT_deinit();
		return 0;
	}


	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf2); i++) {
		XLOGD("%1$02x	", peer_write_buf2[i]);
	}


	memset(ucEvent, 0, sizeof(ucEvent));
	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave BtTest_NoSigRxTest() due to EM_BT_read() failure...\n");
		EM_BT_deinit();
		return 0;
	}

	XLOGE("EM_BT_read:");
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}

	
	int iRes[4];
	iRes[0] = *(int*)(&ucEvent[7]);
	iRes[1] = *(int*)(&ucEvent[11])/10000; // 1.24 -> 124 
	iRes[2] = *(int*)(&ucEvent[15]);
	iRes[3] = *(int*)(&ucEvent[19])/10000;
	jintArray result = env->NewIntArray(4);
	env->SetIntArrayRegion(result, 0, 4, iRes);	
	
	EM_BT_deinit();
	XLOGE("Leave BtTest_EndNoSigRxTest.");
	return result;
}
static jboolean BtTest_StartNoSigRxTest(JNIEnv *env, jobject thiz, 
             jint nPatternIdx, jint nPocketTypeIdx, jint nFreq, jint nAddress) {
	XLOGE("Enter BtTest_NoSigRxTest.");

	unsigned char ucEvent[512];
	int iResultLen = 0;

	if (EM_BT_init() == false) {
		XLOGE("Leave BtTest_NoSigRxTest() due to EM_BT_init() failure\n");		
		return 0;
	}
	
	unsigned char peer_write_buf[] = { 0x01, 0x0D, 0xFC, 0x17, 0x00, 
			NoSigRxBBMap[nPatternIdx], 
			0x0B, 0x00, 0x00, 
			(char)nFreq, 
			0x00, 0x01,
			NoSigRxRRMap[nPocketTypeIdx], 
			0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00,
			((unsigned char) ((0xff000000 & nAddress)>> 24)),
			((unsigned char) ((0xff0000 & nAddress)>> 16)),
			((unsigned char) ((0xff00 & nAddress)>> 8)),
			 ((unsigned char) (nAddress&0xff)), 			 
			0x00, 0x00 };

	if (false == EM_BT_write(peer_write_buf, sizeof(peer_write_buf))) {
		XLOGE("Leave BtTest_NoSigRxTest() due to EM_BT_write() failure...\n");
		EM_BT_deinit();
		return 0;
	}


	XLOGD("EM_BT_write:");
	for (int i = 0; i < sizeof(peer_write_buf); i++) {
		XLOGD("%1$02x	", peer_write_buf[i]);
	}


	memset(ucEvent, 0, sizeof(ucEvent));
	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave BtTest_NoSigRxTest() due to EM_BT_read() failure...\n");
		EM_BT_deinit();
		return 0;
	}

	XLOGE("EM_BT_read:");
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}
	
	if(iResultLen == 14) //receive it again
		{
				memset(ucEvent, 0, sizeof(ucEvent));
	if (false == EM_BT_read(ucEvent, sizeof(ucEvent), &iResultLen)) {
		XLOGE("Leave BtTest_NoSigRxTest() due to EM_BT_read() failure...\n");
		EM_BT_deinit();
		return 0;
	}

	XLOGE("EM_BT_read:");
	for (int i = 0; i < iResultLen; i++) {
		XLOGE("%1$02x	", ucEvent[i]);
	}
		}
	
	return 1;
}

// chip info
static BT_CHIP_ID chip_id;
static BT_HW_ECO eco_num;
static jlong back_len ;
static bool getInfoFlag = false;
//
static void getBtChipInfo(JNIEnv *env, jobject thiz) {
	char patch_id[30];
	unsigned long patch_len = 30;
	EM_BT_getChipInfo(&chip_id, &eco_num);
	XLOGI("chip_id=%d \n	", chip_id);
	XLOGI("eco_num=%d \n	", eco_num);
	EM_BT_getPatchInfo(patch_id, &patch_len);
	XLOGI("patch_len=%d \n", patch_len);
	back_len = (long) patch_len;
	XLOGI("back_len=%d \n", back_len);
	getInfoFlag = true;
}

static int BtTest_GetChipIdInt(JNIEnv *env, jobject thiz) {
	getBtChipInfo(env, thiz);
	XLOGI("return chip_id\n");
	return chip_id;
}
static int BtTest_GetChipEcoNum(JNIEnv *env, jobject thiz) {
	if (!getInfoFlag) {
		getBtChipInfo(env, thiz);
	}
	XLOGI("return eco_num\n");
	return eco_num;
}

static jcharArray BtTest_GetPatchId(JNIEnv *env, jobject thiz) {
	if (!getInfoFlag) {
		EM_BT_getChipInfo(&chip_id, &eco_num);
	}
	char patch_id[30]={'a'};
	unsigned short usResultArray[30] = {0};
	unsigned long patch_len = 30;
	EM_BT_getPatchInfo(patch_id, &patch_len);

	jsize start = 0;
	jsize end = strlen(patch_id);
	jcharArray resultArray = env->NewCharArray(end);

	XLOGI("strlen =%d \n	", end);
	for (int i = 0; i < end; i++) {
//		XLOGI("i=%d --%c	\n", i, patch_id[i]);
		usResultArray[i] = patch_id[i];
		XLOGI("i=%d --%c	\n", i, usResultArray[i]);
	}
	if (NULL != resultArray) {
		env->SetCharArrayRegion(resultArray, start, end,
				(const jchar *) usResultArray);
		XLOGD("Leave BtTest_GetPatchId()...\n");
	}
	XLOGI("return resultArray\n");
	return resultArray;
}
static jlong BtTest_GetPatchLen(JNIEnv *env, jobject thiz) {
	if (!getInfoFlag) {
		getBtChipInfo(env, thiz);
	}
	XLOGI("return back_len=%d \n", back_len);
	return back_len;
}

static int BtTest_relayerStart(JNIEnv *env, jobject thiz , jint portNumber, jint serialSpeed) {
	XLOGI("enter RELAYER_start serialSpeed =%d portNumber =%d\n", serialSpeed, portNumber);
	return RELAYER_start(portNumber, serialSpeed) ? 0 : -1;
}

static int BtTest_relayerExit(JNIEnv *env, jobject thiz) {
	XLOGI("enter RELAYER_exit\n");
	RELAYER_exit();
	return 0;
}

static int BtTest_setFWDump(JNIEnv *env, jobject thiz, jlong value) {
     XLOGI("enter setFWDump, value=%u \n", value);
     return setFWDump((unsigned int)value);
}

static int BtTest_pollingStop(JNIEnv *env, jobject thiz) {
     XLOGI("enter pollingStop\n"); 
     EM_BT_polling_stop();
     return 1;
}

static int BtTest_pollingStart(JNIEnv *env, jobject thiz) {
     XLOGI("enter pollingStart\n"); 
     EM_BT_polling_start();
     return 1;
}
static jboolean BtTest_setSSPDebugMode(JNIEnv *env, jobject thiz, 
             jboolean value) {
	XLOGE("Enter BtTest_setSSPDebugMode. value:%d",value);
	return (jboolean)bt_set_ssp_debug_mode(value);

}
static JNINativeMethod mehods[] = {
		{ "getChipId", "()I",(void *) BtTest_getChipId },
		{ "isBLESupport", "()I",(void *) BtTest_isBLESupport },
		{ "init", "()I", (void *) BtTest_Init },
		{ "doBtTest", "(I)I",(void *) BtTest_doBtTest },
		{ "unInit", "()I",(void *) BtTest_UnInit },
		{ "hciCommandRun", "([CI)[C",(void *) BtTest_HCICommandRun },
		{ "hciReadEvent", "()[C",(void *) BtTest_HCIReadEvent },

		{ "noSigRxTestStart", "(IIII)Z", (void *) BtTest_StartNoSigRxTest },
		{ "noSigRxTestResult", "()[I", (void *) BtTest_EndNoSigRxTest },

		{"getChipIdInt", "()I", (void *) BtTest_GetChipIdInt },
		{"getChipEcoNum", "()I", (void *) BtTest_GetChipEcoNum },
		{"getPatchId", "()[C", (void *) BtTest_GetPatchId },
		{"getPatchLen", "()J", (void *) BtTest_GetPatchLen },

		{"relayerStart", "(II)I", (void *) BtTest_relayerStart },
		{"relayerExit", "()I", (void *) BtTest_relayerExit },
		{"setFWDump", "(J)I", (void *)BtTest_setFWDump },
		{"isComboSupport","()I", (void *)BtTest_isComboSupport },
		{"pollingStart","()I", (void*)BtTest_pollingStart },
		{"pollingStop", "()I", (void*)BtTest_pollingStop },
		{"setSSPDebugMode", "(Z)Z", (void*)BtTest_setSSPDebugMode},};
// This function only registers the native methods
static int register_com_mediatek_bluetooth_bttest(JNIEnv *env) {
	XLOGE("Register: register_com_mediatek_bluetooth_bttest()...\n");
	/*
	 JNINativeMethod nm;
	 nm.name = "doBtTest";
	 nm.signature = "(I)I";
	 nm.fnPtr = (void *)BtTest_doBtTest;
	 */
	XLOGE("register_com_mediatek_bluetooth_bttest");
	//  return AndroidRuntime::registerNativeMethods(env, "com/mediatek/engineermode/bluetooth/BtTest", &nm, 1);
	return AndroidRuntime::registerNativeMethods(env,
			"com/mediatek/engineermode/bluetooth/BtTest", mehods, NELEM(mehods));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	XLOGD("Enter JNI_OnLoad()...\n");
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		XLOGE("ERROR: GetEnv failed\n");
		goto bail;
	}
	assert(env != NULL);

	//if(registerNatives(env) != JNI_TRUE)
	if (register_com_mediatek_bluetooth_bttest(env) < 0) {
		XLOGE("ERROR: Bluetooth native registration failed\n");
		goto bail;
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

	XLOGD("Leave JNI_OnLoad()...\n");
	bail: return result;
}
