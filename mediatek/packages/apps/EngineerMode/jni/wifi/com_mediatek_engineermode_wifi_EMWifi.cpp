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

#undef LOG_NDEBUG 
#undef NDEBUG
#define LOG_TAG "EM-Wifi-JNI"
#include <cutils/xlog.h>

#include <stdio.h>
#include <unistd.h>

#include "jni.h"
#include "JNIHelp.h"
#include "type.h"
#include "android_runtime/AndroidRuntime.h"
#include "WiFi_EM_API.h"

using namespace android;

#define UNIMPLEMENT  	-2
#define STATUS_ERROR  -1
#define STATUS_OK     0

static int check_EMWifi_status(status_t status) {
	if (status == NO_ERROR) {
		return STATUS_OK;
	} else {
		return STATUS_ERROR;
	}
}

static int com_mediatek_engineermode_wifi_EMWifi_setTestMode(JNIEnv *env,
		jobject thiz) {
	XLOGE("JNI, Enter setTestMode succeed");
	INT_32 index = -1;
	index = CWrapper::setTestMode();
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setNormalMode(JNIEnv *env,
		jobject thiz) {
	XLOGE("JNI, Enter setNormalMode succeed");
	INT_32 index = -1;
	index = CWrapper::setNormalMode();
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setStandBy(JNIEnv *env,
		jobject thiz) {
	XLOGE("JNI, Enter setStandBy succeed");
	INT_32 index = -1;
	index = CWrapper::setStandBy();
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setEEPRomSize(JNIEnv *env,
		jobject thiz, jlong i4EepromSz) {
	INT_32 index = -1;
	XLOGE("JNI Set the eep_rom size is size = (%d)", ((int) i4EepromSz));
	index = CWrapper::setEEPRomSize(i4EepromSz);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setEEPRomFromFile(JNIEnv *env,
		jobject thiz, jstring atcFileName) {
	INT_32 index = -1;

	if (atcFileName == NULL) {
		return STATUS_ERROR;
	}

	INT_32 len = env->GetStringUTFLength(atcFileName);
	const CHAR *file_name = env->GetStringUTFChars(atcFileName, NULL);

	CHAR *name = new CHAR[len + 1];
	if (name) {
		memcpy(name, file_name, len);
		name[len] = '\0';

		XLOGV("setEEPRomFromFile, file name = (%s)", name);

		index = CWrapper::setEEPRomFromFile(name);

		delete[] name;
	} else {
		XLOGV("Error when to alloc memeory at setEEPRomFromFile");
	}
	env->ReleaseStringUTFChars(atcFileName, file_name);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_readTxPowerFromEEPromEx(
		JNIEnv *env, jobject thiz, jlong i4ChnFreg, jlong i4Rate,
		jlongArray PowerStatus, jint arrayLength) {

	jlong *jniParameter = NULL;
	INT_32 pi4TxPwrGain = -1;
	INT_32 pi4Eerp = -1;
	INT_32 pi4ThermoVal = -1;
	INT_32 index = -1;

	if (PowerStatus) {
		jniParameter = (jlong *) env->GetLongArrayElements(PowerStatus, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}
	} else {
		XLOGE("NULL java array of TxPower from EE Prom Ex");
		return -2;
	}

	if (arrayLength != 3) {
		env->ReleaseLongArrayElements(PowerStatus, jniParameter, JNI_ABORT);
		XLOGE("Error length pass to the array");
		return STATUS_ERROR;
	} else {
		index = CWrapper::readTxPowerFromEEPromEx(i4ChnFreg, i4Rate,
				&pi4TxPwrGain, &pi4Eerp, &pi4ThermoVal);
		if (index == 0) {
			jniParameter[0] = pi4TxPwrGain;
			jniParameter[1] = pi4Eerp;
			jniParameter[2] = pi4ThermoVal;
		} else {
			XLOGE("Error to call readTxPowerFromEEPromEx in native");
		}
	}
	XLOGE(
			"get the readTxPowerFromEEPromEx value, pi4TxPwrGain = (%d), pi4Eerp = (%d), pi4ThermoVal = (%d)",
			((int) pi4TxPwrGain), ((int) pi4Eerp), ((int) pi4ThermoVal));
	env->ReleaseLongArrayElements(PowerStatus, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getPacketTxStatusEx(
		JNIEnv *env, jobject thiz, jlongArray PktStatus, jint arraylen) {

	jlong *jniParameter = NULL;
	INT_32 index = -1;

	INT_32 pi4SentCount, pi4AckCount, pi4Alc, pi4CckGainControl,
			pi4OfdmGainControl;

	if (PktStatus) {
		jniParameter = (jlong *) env->GetLongArrayElements(PktStatus, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}
	} else {
		XLOGE("NULL java array of Packet Tx Status Ex");
		return -2;
	}

	if (arraylen != 5) {
		env->ReleaseLongArrayElements(PktStatus, jniParameter, JNI_ABORT);
		XLOGE("Error length pass to the array");
		return STATUS_ERROR;
	} else {
		index = CWrapper::getPacketTxStatusEx(&pi4SentCount, &pi4AckCount,
				&pi4Alc, &pi4CckGainControl, &pi4OfdmGainControl);
		if (index == 0) {
			jniParameter[0] = pi4SentCount;
			jniParameter[1] = pi4AckCount;
			jniParameter[2] = pi4Alc;
			jniParameter[3] = pi4CckGainControl;
			jniParameter[4] = pi4OfdmGainControl;
		}
	}
	ABORT: env->ReleaseLongArrayElements(PktStatus, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setEEPromCKSUpdated(
		JNIEnv *env, jobject thiz) {
	XLOGE("JNI, Enter setEEPromCKSUpdated succeed");
	INT_32 index = -1;
	index = CWrapper::setEEPromCKSUpdated();
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getPacketRxStatus(JNIEnv *env,
		jobject thiz, jlongArray i4Init, jint arraylen) {

	XLOGE("JNI, Enter getPacketRxStatus succeed");
	INT_32 index = -1;
	jlong *jniParameter = NULL;
	INT_32 pi4RxOk, pi4RxCrcErr;

	if (arraylen != 2) {
		XLOGE("Wrong array length for getPacketRxStatus");
		return STATUS_ERROR;
	}

	if (i4Init) {
		jniParameter = (jlong *) env->GetLongArrayElements(i4Init, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}
	} else {
		XLOGE("NULL java array of getPacketRxStatus");
		return -2;
	}

	index = CWrapper::getPacketRxStatus(&pi4RxOk, &pi4RxCrcErr);
	if (index == 0) {
		jniParameter[0] = pi4RxOk;
		jniParameter[1] = pi4RxCrcErr;
		XLOGE("JNI, getPacketRxStatus value pi4RxOk = (%d), pi4RxCrcErr = (%d)",
				(int) pi4RxOk, (int) pi4RxCrcErr);
	} else {
		XLOGE("Native, get getPacketRxStatus failed");
	}
	env->ReleaseLongArrayElements(i4Init, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setOutputPower(JNIEnv *env,
		jobject thiz, jlong i4Rate, jlong i4TxPwrGain, jlong i4TxAnt) {
	XLOGE(
			"JNI, Enter setOutputPower succeed, i4Rate = %d, i4TxPwrGain = %d, i4TxAnt = %d",
			(int) i4Rate, (int) i4TxPwrGain, (int) i4TxAnt);
	INT_32 index = -1;
	index = CWrapper::setOutputPower(i4Rate, i4TxPwrGain, i4TxAnt);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setLocalFrequecy(JNIEnv *env,
		jobject thiz, jlong i4TxPwrGain, jlong i4TxAnt) {
	XLOGE("JNI, Enter setLocalFrequecy succeed, i4TxPwrGain = %d, i4TxAnt = %d",
			(int) i4TxPwrGain, (int) i4TxAnt);
	INT_32 index = -1;
	index = CWrapper::setLocalFrequecy(i4TxPwrGain, i4TxAnt);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setCarrierSuppression(
		JNIEnv *env, jobject thiz, jlong i4Modulation, jlong i4TxPwrGain,
		jlong i4TxAnt) {
	XLOGE(
			"JNI, Enter setCarrierSuppression succeed, i4Modulation = %d, i4TxPwrGain = %d, i4TxAnt = %d",
			(int) i4Modulation, (int) i4TxPwrGain, (int) i4TxAnt);
	INT_32 index = -1;
	index = CWrapper::setCarrierSuppression(i4Modulation, i4TxPwrGain, i4TxAnt);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setOperatingCountry(
		JNIEnv *env, jobject thiz, jstring acChregDomain) {
	INT_32 index = -1;
	const CHAR *file_name = env->GetStringUTFChars(acChregDomain, NULL);
	INT_32 len = env->GetStringLength(acChregDomain);
	CHAR *name = new char[len + 1];

	if (!name) {
		env->ReleaseStringUTFChars(acChregDomain, file_name);
		return STATUS_ERROR;
	}

	memcpy(name, file_name, len);
	name[len] = '\0';

	XLOGE("JNI, Enter setOperatingCountry succeed, country name = %s", name);
	index = CWrapper::setOperatingCountry(name);
	delete[] name;
	env->ReleaseStringUTFChars(acChregDomain, file_name);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setChannel(JNIEnv *env,
		jobject thiz, jlong i4ChFreqKHz) {
	XLOGE("JNI, Enter setChannel succeed, country i4ChFreqKHz = %d",
			(int) i4ChFreqKHz);
	INT_32 index = -1;
	index = CWrapper::setChannel(i4ChFreqKHz);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getSupportedRates(JNIEnv *env,
		jobject thiz, jintArray pu2RateBuf, jlong i4MaxNum) {
	INT_32 index = -1;
	jint *jniParameter = NULL;
	UINT_16 rate = 0;

	if (pu2RateBuf) {
		jniParameter = (jint *) env->GetIntArrayElements(pu2RateBuf, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}
	} else {
		XLOGE("NULL java array of getSupportedRates");
		return -2;
	}
	index = CWrapper::getSupportedRates(&rate, i4MaxNum);
	if (index == 0) {
		jniParameter[0] = rate;
	} else {
		XLOGE("Native, methods call failed");
	}
	XLOGE(
			"JNI, Enter getSupportedRates succeed, pu2RateBuf = %d, i4MaxNum = %d",
			(int) rate, (int) i4MaxNum);
	env->ReleaseIntArrayElements(pu2RateBuf, jniParameter, JNI_ABORT);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setOutputPin(JNIEnv *env,
		jobject thiz, jlong i4PinIndex, jlong i4OutputLevel) {
	INT_32 index = -1;

	XLOGE(
			"JNI, Enter setOutputPin succeed, i4PinIndex = %d, i4OutputLevel = %d",
			(int) i4PinIndex, (int) i4OutputLevel);
	index = CWrapper::setOutputPin(i4PinIndex, i4OutputLevel);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_readEEPRom16(JNIEnv *env,
		jobject thiz, jlong u4Offset, jlongArray pu4Value) {
	jlong *jniParameter = NULL;
	UINT_32 value = 0;
	INT_32 index = -1;

	if (pu4Value) {
		jniParameter = (jlong *) env->GetLongArrayElements(pu4Value, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load                                                                  
		}
	} else {
		XLOGE("NULL java array of readEEPRom16");
		return -2;
	}

	index = CWrapper::readEEPRom16(u4Offset, &value);
	jniParameter[0] = value;

	XLOGE("JNI, Enter readEEPRom16 succeed, u4Offset = %d, pu4Value = %d",
			(int) u4Offset, (int) value);
	env->ReleaseLongArrayElements(pu4Value, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_readSpecEEPRom16(JNIEnv *env,
		jobject thiz, jlong u4Offset, jlongArray pu4Value) {
	jlong *jniParameter = NULL;
	UINT_32 value = 0;
	INT_32 index = -1;

	if (pu4Value) {
		jniParameter = (jlong *) env->GetLongArrayElements(pu4Value, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load                                                                  
		}
	} else {
		XLOGE("NULL java array of readSpecEEPRom16");
		return -2;
	}

	index = CWrapper::readSpecEEPRom16(u4Offset, &value);
	jniParameter[0] = value;

	XLOGE("JNI, Enter readSpecEEPRom16 succeed, u4Offset = %d, pu4Value = %d",
			(int) u4Offset, (int) value);
	env->ReleaseLongArrayElements(pu4Value, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_writeEEPRom16(JNIEnv *env,
		jobject thiz, jlong u4Offset, jlong u4Value) {
	INT_32 index = -1;

	XLOGE("JNI, Enter writeEEPRom16 succeed, u4Offset = %d, u4Value = %d",
			(int) u4Offset, (int) u4Value);
	index = CWrapper::writeEEPRom16(u4Offset, u4Value);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_eepromReadByteStr(JNIEnv *env,
		jobject thiz, jlong u4Addr, jlong u4Length, jbyteArray paucStr) {
	jbyte *jniParameter = NULL;
	INT_32 index = -1;

	if (paucStr) {
		jniParameter = (jbyte *) env->GetByteArrayElements(paucStr, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}
	} else {
		XLOGE("NULL java array of eepromReadByteStr");
		return -2;
	}

	index = CWrapper::eepromReadByteStr(u4Addr, u4Length, jniParameter);

	//	env -> SetByteArrayRegion(jniParameter, 0, 512, (jbyte*)pStr);

	if (index == 0) {
		//		memcpy(jniParameter, pStr, u4Length*2);
		XLOGE("JNI, Enter eepromReadByteStr succeed, paucStr = %s", jniParameter);
	} else {
		XLOGE("Native, eepromReadByteStr call failed");
	}

	env->ReleaseByteArrayElements(paucStr, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_eepromWriteByteStr(JNIEnv *env,
		jobject thiz, jlong u4Addr, jlong u4Length, jstring paucStr) {
	INT_32 index = -1;
	INT_32 len = env->GetStringUTFLength(paucStr);
	const CHAR *str = env->GetStringUTFChars(paucStr, NULL);

	INT_8 *name = new INT_8[len + 1];
	if (name) {
		memcpy(name, str, len);
		name[len] = '\0';

		index = CWrapper::eepromWriteByteStr(u4Addr, u4Length, name);

		XLOGE("JNI, Enter eepromWriteByteStr succeed, name = %s", name);

		delete[] name;
	}

	XLOGE(
			"JNI, Enter eepromWriteByteStr succeed, u4Addr = %d, u4Length = %d, str = %s",
			(int) u4Addr, (int) u4Length, str);
	env->ReleaseStringUTFChars(paucStr, str);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setATParam(JNIEnv *env,
		jobject thiz, jlong u4FuncIndex, jlong pu4FuncData) {
	INT_32 index = -1;
	XLOGE("JNI, Enter setATParam succeed, u4FuncIndex = %d, pu4FuncData = %d",
			(unsigned int) u4FuncIndex, (unsigned int) pu4FuncData);
	index = CWrapper::setATParam(u4FuncIndex, pu4FuncData);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getATParam(JNIEnv *env,
		jobject thiz, jlong u4FuncIndex, jlongArray pu4FuncData) {
	jlong *jniParameter = NULL;
	INT_32 index = -1;
	UINT_32 value = 0;

	if (pu4FuncData) {
		jniParameter = (jlong *) env->GetLongArrayElements(pu4FuncData, NULL);

		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}

	} else {
		XLOGE("NULL java array of getATParam");
		return -2;
	}

	index = CWrapper::getATParam(u4FuncIndex, &value);
	jniParameter[0] = value;

	XLOGE("JNI, Enter getATParam succeed, u4FuncIndex = %d, pu4FuncData = %d",
			(unsigned int) u4FuncIndex, (unsigned int) value);
	env->ReleaseLongArrayElements(pu4FuncData, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setXtalTrimToCr(JNIEnv *env,
		jobject thiz, jlong u4Value) {
	//	XLOGD("com_mediatek_engineermode_wifi_EMWifi_setXtalTrimToCr_u4Value = %l", u4Value);
	int index;
	XLOGE("JNI, Enter getATParam succeed, u4Value = %d", (int) u4Value);
	index = CWrapper::setXtalTrimToCr(u4Value);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_queryThermoInfo(JNIEnv *env,
		jobject thiz, jlongArray pi4Enable, jint len) {
	INT_32 value = 0;
	INT_32 index = -1;
	UINT_32 pu4Value = 0;
	jlong *jniParameter = NULL;

	if (len != 2) {
		XLOGE("Wrong length of queryThermoInfo array");
		return -2;
	}

	if (pi4Enable) {
		jniParameter = (jlong *) env->GetLongArrayElements(pi4Enable, NULL);

		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}

	} else {
		XLOGE("NULL java array of queryThermoInfo");
		return -2;
	}

	index = CWrapper::queryThermoInfo(&value, &pu4Value);
	if (index == 0) {
		jniParameter[0] = value;
		jniParameter[1] = pu4Value;
	} else {
		XLOGE("Native, Error to call queryThermoInfo");
	}

	XLOGE(
			"JNI, Enter queryThermoInfo succeed, pi4Enable1 = %d, pi4Enable2 = %d",
			(int) value, (int) pu4Value);

	env->ReleaseLongArrayElements(pi4Enable, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setThermoEn(JNIEnv *env,
		jobject thiz, jlong i4Enable) {
	INT_32 index = -1;
	XLOGE("JNI, Enter setThermoEn succeed, i4Enable = %d", (int) i4Enable);
	index = CWrapper::setThermoEn(i4Enable);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getSpecEEPRomSize(JNIEnv *env,
		jobject thiz, jlongArray pi4EepromSz) {
	jlong *jniParameter = NULL;
	INT_32 value;
	INT_32 index;
	if (pi4EepromSz) {
		jniParameter = (jlong *) env->GetLongArrayElements(pi4EepromSz, NULL);

		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}

	} else {
		XLOGE("NULL java array of getSpecEEPRomSize");
		return -2;
	}
	index = CWrapper::getSpecEEPRomSize(&value);
	XLOGE("JNI, Enter getSpecEEPRomSize succeed, value = %d", (int) value);
	jniParameter[0] = value;
	env->ReleaseLongArrayElements(pi4EepromSz, jniParameter, JNI_ABORT);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getEEPRomSize(JNIEnv *env,
		jobject thiz, jlongArray pi4EepromSz) {
	jlong *jniParameter = NULL;
	INT_32 value;
	INT_32 index;
	if (pi4EepromSz) {
		jniParameter = (jlong *) env->GetLongArrayElements(pi4EepromSz, NULL);

		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}

	} else {
		XLOGE("NULL java array of getEEPRomSize");
		return -2;
	}
	index = CWrapper::getEEPRomSize(&value);
	XLOGE("JNI, Enter getEEPRomSize succeed, value = %d", (int) value);
	jniParameter[0] = value;
	env->ReleaseLongArrayElements(pi4EepromSz, jniParameter, JNI_ABORT);
	return index;
}

//static long com_mediatek_engineermode_wifi_EMWifi_getEEPRomSize(JNIEnv *env,
//		jobject thiz) {
//
//	UINT_32 result = 0;
//	INT_32 index = -1;
//
//	index = CWrapper::getXtalTrimToCr(&result);
//
//	XLOGE("step into ");
//
//	if (index != 0) {
//		return index;
//	}
//	return result;
//}

static int com_mediatek_engineermode_wifi_EMWifi_setPnpPower(JNIEnv *env,
		jobject thiz, jlong i4PowerMode) {
	INT_32 index = -1;
	XLOGE("JNI, Enter setPnpPower succeed, i4PowerMode = %d", (int) i4PowerMode);
	index = CWrapper::setPnpPower(i4PowerMode);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setAnritsu8860bTestSupportEn(
		JNIEnv *env, jobject thiz, jlong i4Enable) {
	INT_32 index = -1;
	XLOGE("JNI, Enter setAnritsu8860bTestSupportEn succeed, i4Enable = %d",
			(int) i4Enable);
	index = CWrapper::setAnritsu8860bTestSupportEn(i4Enable);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_writeMCR32(JNIEnv *env,
		jobject thiz, jlong offset, jlong value) {
	INT_32 index = -1;
	XLOGE("JNI, Enter writeMCR32 succeed, offset = %d, value = %d",
			(int) offset, (int) value);
	index = CWrapper::writeMCR32(offset, value);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_readMCR32(JNIEnv *env,
		jobject thiz, jlong offset, jlongArray value) {
	jlong *jniParameter = NULL;
	UINT_32 val = 0;
	INT_32 index = -1;

	if (value) {
		jniParameter = (jlong *) env->GetLongArrayElements(value, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2;
		}
	} else {
		XLOGE("NULL java array of readMCR32");
		return -2;
	}
	index = CWrapper::readMCR32(offset, &val);
	jniParameter[0] = val;
	XLOGE("JNI, Enter writeMCR32 succeed, offset = %d, value = %d",
			(int) offset, (int) val);
	env->ReleaseLongArrayElements(value, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_initial(JNIEnv *env,
		jobject thiz/*, jlong chipID*/) {
	const CHAR* wrapperID = "local";
	INT_32 index = -1;
/*	
	if(chipID == 0x5921)
	{
		wrapperID = "MT5921";
	}
	else
	{
		wrapperID = "MT6620";
	}
	*/
	index = CWrapper::Initialize(wrapperID);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_unInitial(JNIEnv *env,
		jobject thiz) {
	INT_32 index = -1;
	XLOGE("JNI, Entener Uninitialize");
	index = CWrapper::Uninitialize();
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getXtalTrimToCr(JNIEnv *env,
		jobject thiz, jlongArray value) {
	jlong *jniParameter = NULL;
	INT_32 index = -1;
	UINT_32 result = 0;

	if (value) {
		jniParameter = (jlong *) env->GetLongArrayElements(value, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2;
		}
	} else {
		XLOGE("NULL java array of getXtalTrimToCr, can't get value");
		return -2;
	}

	index = CWrapper::getXtalTrimToCr(&result);

	jniParameter[0] = result;
	XLOGE("step into getXtalTrimToCr and got the value result = (%d)",
			(int) result);

	env->ReleaseLongArrayElements(value, jniParameter, JNI_ABORT);

	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_getDPDLength(JNIEnv *env,
		jobject thiz, jlongArray pi4DPDLength) {
	jlong *jniParameter = NULL;
	INT_32 value;
	INT_32 index;
	if (pi4DPDLength) {
		jniParameter = (jlong *) env->GetLongArrayElements(pi4DPDLength, NULL);

		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}

	} else {
		XLOGE("NULL java array of getDPDLength");
		return -2;
	}
	index = CWrapper::getDPDLength(&value);
	XLOGE("JNI, Enter getDPDLength succeed");
	jniParameter[0] = value;
	env->ReleaseLongArrayElements(pi4DPDLength, jniParameter, JNI_ABORT);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_writeDPD32(JNIEnv *env,
		jobject thiz, jlong offset, jlong value) {
	INT_32 index = -1;
	XLOGE("JNI, Enter writeDPD32 succeed, offset = %d, value = %d",
			(int) offset, (int) value);
	index = CWrapper::writeDPD32(offset, value);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_readDPD32(JNIEnv *env,
		jobject thiz, jlong offset, jlongArray value) {
	jlong *jniParameter = NULL;
	UINT_32 val = 0;
	INT_32 index = -1;

	if (value) {
		jniParameter = (jlong *) env->GetLongArrayElements(value, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2;
		}
	} else {
		XLOGE("NULL java array of readDPD32");
		return -2;
	}
	index = CWrapper::readDPD32(offset, &val);
	jniParameter[0] = val;
	XLOGE("JNI, Enter readDPD32 succeed, offset = %d, value = %d",
			(int) offset, (int) val);
	env->ReleaseLongArrayElements(value, jniParameter, JNI_ABORT);

	return index;
}




static int com_mediatek_engineermode_wifi_EMWifi_setDPDFromFile(JNIEnv *env,
		jobject thiz, jstring atcFileName) {
	INT_32 index = -1;

	if (atcFileName == NULL) {
		return STATUS_ERROR;
	}

	INT_32 len = env->GetStringUTFLength(atcFileName);
	const CHAR *file_name = env->GetStringUTFChars(atcFileName, NULL);

	CHAR *name = new CHAR[len + 1];
	if (name) {
		memcpy(name, file_name, len);
		name[len] = '\0';

		XLOGV("setDPDFromFile, file name = (%s)", name);

		index = CWrapper::setDPDFromFile(name);

		delete[] name;
	} else {
		XLOGV("Error when to alloc memeory at setDPDFromFile");
	}
	env->ReleaseStringUTFChars(atcFileName, file_name);

	return index;
}

// Added by mtk54046 @ 2012-01-05 for get support channel list
static int com_mediatek_engineermode_wifi_EMWifi_getSupportChannelList(JNIEnv *env,
		jobject thiz, jlongArray pau4Channel) {
	jlong *jniParameter = NULL;
	UINT_32 value[65];
	INT_32 index;
	if (pau4Channel) {
		jniParameter = (jlong *) env->GetLongArrayElements(pau4Channel, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM paramters");
			return -2; // out of memory or no data to load
		}
	} else {
		XLOGE("NULL java array of getSupportChannelList");
		return -2;
	}
	index = CWrapper::getSupportChannelList(value);
	if (!index) {
		XLOGE("JNI, Enter getSupportChannelList succeed, channel list length is %d. value[1] is %d, value[2] is %d", (int)value[0], (int)value[1], (int)value[2]);
//		memcpy(jniParameter, value, 65);
		jniParameter[0] = value[0];
		for (int i=1; i<= value[0]; i++) {
			jniParameter[i] = value[i];
		}
	}
	env->ReleaseLongArrayElements(pau4Channel, jniParameter, JNI_ABORT);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_doCTIATestSet(JNIEnv *env,
		jobject thiz, jlong u4Id, jlong u4Value) {
	INT_32 index;
	XLOGE("JNI Set ID is %u, value is %u", u4Id, u4Value);
	index = CWrapper::doCTIATestSet(u4Id, u4Value);
	if (!index) {
		XLOGE("doCTIATestSet succeed, ID is %u", u4Id);
	}
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_doCTIATestGet(JNIEnv *env,
		jobject thiz, jlong u4Id, jlongArray pu4Value) {
	INT_32 index;
	jlong *jniParameter = NULL;
	UINT_32 value = 0;
	if (pu4Value) {
		jniParameter = (jlong *) env->GetLongArrayElements(pu4Value, NULL);
		if (jniParameter == NULL) {
			XLOGE("Error retrieving source of EM parameters");
			return -2;
		}
	} else {
		XLOGE("NULL java array of doCTIATestGet");
		return -2;
	}
	XLOGE("JNI Get ID is %u, value is %u", u4Id, value);
	index = CWrapper::doCTIATestGet(u4Id, &value);
	if (!index) {
		XLOGE("doCTIATestGet succeed, ID is %u, value is %u", u4Id, value);
	}
	jniParameter[0] = value;
	env->ReleaseLongArrayElements(pu4Value, jniParameter, JNI_ABORT);
	return index;
}

//method register to vm
static JNINativeMethod
		gMethods[] = {
				{ "initial", "()I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_initial },
				{ "unInitial", "()I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_unInitial },
				{
						"getXtalTrimToCr",
						"([J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getXtalTrimToCr },
				{
						"setTestMode",
						"()I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setTestMode },
				{
						"setNormalMode",
						"()I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setNormalMode },
				{ "setStandBy", "()I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setStandBy },
				{
						"setEEPRomSize",
						"(J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setEEPRomSize },
				{
						"setEEPRomFromFile",
						"(Ljava/lang/String;)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setEEPRomFromFile },
				{
						"readTxPowerFromEEPromEx",
						"(JJ[JI)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_readTxPowerFromEEPromEx },
				{
						"setEEPromCKSUpdated",
						"()I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setEEPromCKSUpdated },
				{
						"getPacketTxStatusEx",
						"([JI)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getPacketTxStatusEx },
				{
						"getPacketRxStatus",
						"([JI)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getPacketRxStatus },
				{
						"setOutputPower",
						"(JJI)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setOutputPower },
				{
						"setLocalFrequecy",
						"(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setLocalFrequecy },
				{
						"setCarrierSuppression",
						"(JJJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setCarrierSuppression },
				{
						"setOperatingCountry",
						"(Ljava/lang/String;)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setOperatingCountry },
				{ "setChannel", "(J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setChannel },
				{
						"getSupportedRates",
						"([IJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getSupportedRates },
				{
						"setOutputPin",
						"(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setOutputPin },
				{
						"readEEPRom16",
						"(J[J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_readEEPRom16 },
				{
						"readSpecEEPRom16",
						"(J[J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_readSpecEEPRom16 },
						
				{
						"writeEEPRom16",
						"(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_writeEEPRom16 },
				{
						"eepromReadByteStr",
						"(JJ[B)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_eepromReadByteStr },
				{
						"eepromWriteByteStr",
						"(JJLjava/lang/String;)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_eepromWriteByteStr },
				{ "setATParam", "(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setATParam },
				{ "getATParam", "(J[J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getATParam },
				{
						"setXtalTrimToCr",
						"(J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setXtalTrimToCr },
				{
						"queryThermoInfo",
						"([JI)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_queryThermoInfo },
				{
						"setThermoEn",
						"(J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setThermoEn },
				{
						"getEEPRomSize",
						"([J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getEEPRomSize },
				{
						"getSpecEEPRomSize",
						"([J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getSpecEEPRomSize },
						
				{
						"setPnpPower",
						"(J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setPnpPower },
				{
						"setAnritsu8860bTestSupportEn",
						"(J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setAnritsu8860bTestSupportEn },
				{ "writeMCR32", "(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_writeMCR32 },
				{ "readMCR32", "(J[J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_readMCR32 }, 
				{
						"getDPDLength",
						"([J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getDPDLength },
				{ "writeDPD32", "(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_writeDPD32 },
				{ "readDPD32", "(J[J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_readDPD32 }, 
				{
						"setDPDFromFile",
						"(Ljava/lang/String;)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_setDPDFromFile },
				// Added by mtk54046 @ 2012-01-05 for get support channel list
				{ "getSupportChannelList", "([J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_getSupportChannelList },
				{ "doCTIATestSet", "(JJ)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_doCTIATestSet },
				{ "doCTIATestGet", "(J[J)I",
						(void*) com_mediatek_engineermode_wifi_EMWifi_doCTIATestGet },
			};

static const char* const kClassPathName = "com/mediatek/engineermode/wifi";

int register_com_mediatek_engineermode_wifi_EMWifi(JNIEnv *env) {

	//	jclass EMWifi = env->FindClass(kClassPathName);
	//	LOG_FATAL_IF(EMWifi == NULL, "Unable to find class ");

	return AndroidRuntime::registerNativeMethods(env,
			"com/mediatek/engineermode/wifi/EMWifi", gMethods, NELEM(gMethods));
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	JNIEnv *env = NULL;
	jint result = -1;

	if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
		XLOGE("ERROR: GetEnv failed");
		goto bail;
	}

	assert(env != NULL);

	if (register_com_mediatek_engineermode_wifi_EMWifi(env) < 0) {
		XLOGE("ERROR: Wi-Fi native for engineermode registration failed\n");
		goto bail;
	}

	result = JNI_VERSION_1_4;

	bail: return result;
}

