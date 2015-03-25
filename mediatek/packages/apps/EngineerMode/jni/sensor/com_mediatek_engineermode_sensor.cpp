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

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#undef LOG_NDEBUG 
#undef NDEBUG

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "SENSOR-JNI"
#endif

#include <cutils/xlog.h>
extern "C"{
#include "libhwm.h"
}

using namespace android;

static jint doGsensorCalibration(JNIEnv *env, jclass clazz, jint tolerance) {
	XLOGD("Enter doGsensorCalibration(), tolerance = %d\n", tolerance);
	int ret = do_gsensor_calibration(tolerance);
	XLOGD("doGsensorCalibration() returned %d\n", ret);

	return ret;
}

static jint getGsensorCalibration(JNIEnv *env, jclass clazz, jfloatArray result) {
	float values[3] = {0};
	XLOGD("Enter getGsensorCalibration()\n");
	int ret = get_gsensor_calibration(&values[0], &values[1], &values[2]);
	XLOGD("getGsensorCalibration() returned %d, %f, %f, %f\n", ret, values[0], values[1], values[2]);

    env->SetFloatArrayRegion(result, 0, 3, (jfloat*) values);
    return ret;
}

static jint clearGsensorCalibration(JNIEnv *env, jclass clazz) {
	XLOGD("Enter clearGsensorCalibration()\n");
	int ret = clear_gsensor_calibration();
	XLOGD("clearGsensorCalibration() returned %d\n", ret);

	return ret;
}

static jint doGyroscopeCalibration(JNIEnv *env, jclass clazz, jint tolerance) {
	XLOGD("Enter doGyroscopeCalibration(), tolerance = %d\n", tolerance);
	int ret = do_gyroscope_calibration(tolerance);
	XLOGD("doGyroscopeCalibration() returned %d\n", ret);

	return ret;
}

static jint getGyroscopeCalibration(JNIEnv *env, jclass clazz, jfloatArray result) {
	float values[3] = {0};
	XLOGD("Enter getGyroscopeCalibration()\n");
	int ret = get_gyroscope_calibration(&values[0], &values[1], &values[2]);
	XLOGD("getGyroscopeCalibration() returned %d, %f, %f, %f\n", ret, values[0], values[1], values[2]);

	env->SetFloatArrayRegion(result, 0, 3, (jfloat*) values);
	return ret;
}

static jint clearGyroscopeCalibration(JNIEnv *env, jclass clazz) {
	XLOGD("Enter clearGyroscopeCalibration()\n");
	int ret = clear_gyroscope_calibration();
	XLOGD("clearGyroscopeCalibration() returned %d\n", ret);

	return ret;
}

static jint getPsensorData(JNIEnv *env, jclass clazz) {
	XLOGD("Enter getPsensorData()\n");
	int ret = get_psensor_data();
	XLOGD("getPsensorData() returned %d\n", ret);

	return ret;
}

static jint getPsensorThreshold(JNIEnv *env, jclass clazz, jintArray result) {
	int values[2] = {0};
	XLOGD("Enter getPsensorThreshold()\n");
	values[0] = get_psensor_threshold(0);
	values[1] = get_psensor_threshold(1);
	XLOGD("getPsensorThreshold() returned %d, %d\n", values[0], values[1]);

    env->SetIntArrayRegion(result, 0, 2, (jint*) values);
	return 1;
}

static jint setPsensorThreshold(JNIEnv *env, jclass clazz, jintArray result) {
	int values[2] = {0};
	XLOGD("Enter getPsensorThreshold()\n");
    env->GetIntArrayRegion(result, 0, 2, (jint*) values);
	int ret = set_psensor_threshold(values[0], values[1]);
	XLOGD("getPsensorThreshold() returned %d\n", ret);

	return ret;
}

static jint calculatePsensorMinValue(JNIEnv *env, jclass clazz) {
	XLOGD("Enter calculatePsensorMinValue()\n");
	int ret = calculate_psensor_min_value();
	XLOGD("calculatePsensorMinValue() returned %d\n", ret);

	return ret;
}

static jint getPsensorMinValue(JNIEnv *env, jclass clazz) {
	XLOGD("Enter getPsensorMinValue()\n");
	int ret = get_psensor_min_value();
	XLOGD("getPsensorMinValue() returned %d\n", ret);

	return ret;
}

static jint calculatePsensorMaxValue(JNIEnv *env, jclass clazz) {
	XLOGD("Enter calculatePsensorMaxValue()\n");
	int ret = calculate_psensor_max_value();
	XLOGD("calculatePsensorMaxValue() returned %d\n", ret);

	return ret;
}

static jint getPsensorMaxValue(JNIEnv *env, jclass clazz) {
	XLOGD("Enter getPsensorMaxValue()\n");
	int ret = get_psensor_max_value();
	XLOGD("getPsensorMaxValue() returned %d\n", ret);

	return ret;
}

static jint doPsensorCalibration(JNIEnv *env, jclass clazz, jint min, jint max) {
	XLOGD("Enter doPsensorCalibration()\n");
	int ret = do_calibration(min, max);
	XLOGD("doPsensorCalibration() returned %d\n", ret);

	return ret;
}

static JNINativeMethod mehods[] = {
//	{ "doGsensorCalibration", "(I)I",(void *) doGsensorCalibration },
//	{ "getGsensorCalibration", "([F)I",(void *) getGsensorCalibration },
//	{ "clearGsensorCalibration", "()I", (void *) clearGsensorCalibration },
//	{ "doGyroscopeCalibration", "(I)I",(void *) doGyroscopeCalibration },
//	{ "getGyroscopeCalibration", "([F)I",(void *) getGyroscopeCalibration },
//	{ "clearGyroscopeCalibration", "()I",(void *) clearGyroscopeCalibration },
	{ "getPsensorData", "()I",(void *) getPsensorData },
	{ "getPsensorThreshold", "([I)I",(void *) getPsensorThreshold },
//	{ "setPsensorThreshold", "([I)I",(void *) setPsensorThreshold },
	{ "calculatePsensorMinValue", "()I",(void *) calculatePsensorMinValue },
	{ "getPsensorMinValue", "()I",(void *) getPsensorMinValue },
	{ "calculatePsensorMaxValue", "()I",(void *) calculatePsensorMaxValue },
	{ "getPsensorMaxValue", "()I",(void *) getPsensorMaxValue },
//	{ "doPsensorCalibration", "(II)I",(void *) doPsensorCalibration },
};

// This function only registers the native methods
static int register_com_mediatek_sensor(JNIEnv *env) {
	XLOGE("Register: register_com_mediatek_sensor()...\n");
	return AndroidRuntime::registerNativeMethods(env,
			"com/mediatek/engineermode/sensor/EmSensor", mehods, NELEM(mehods));
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

	if (register_com_mediatek_sensor(env) < 0) {
		XLOGE("ERROR: Sensor native registration failed\n");
		goto bail;
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

	XLOGD("Leave JNI_OnLoad()...\n");
	bail: return result;
}

