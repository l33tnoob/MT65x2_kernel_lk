
/*******************************************************************************
*
*   bt_framework.cpp
*   This file is used to completed the BTSimulatorReceiver call.
*
*******************************************************************************/

#define LOG_TAG "bt_framework.cpp"

#include "../android_bluetooth_common.h"
#include "android_runtime/AndroidRuntime.h"
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include <sys/socket.h>
#include <sys/time.h>

#include "cutils/sockets.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bt_simulator.h"

#define BTFW_UT_DEBUG

#ifdef BTFW_UT_DEBUG
#define BTFW_LOG(fmt, ...) ALOGI("[BT][FRAMEWORK][UT]%s:" fmt, __FUNCTION__, ## __VA_ARGS__)
#else
#define BTFW_LOG(fmt, ...)
#endif

namespace android {

static jmethodID method_onCreate;
static jmethodID method_onDestroy;
static jmethodID method_onStop;
static jmethodID method_onDumpCoverage;
static jmethodID method_onCallPrivate;

typedef struct {
    JavaVM *vm;
    int envVer;
    jobject me;  // for callbacks to java
} native_data_t;

static native_data_t *nat = NULL;  // global native data

static JNIEnv* g_jni_env = NULL;

static jclass g_BtFwTester_clazz = NULL;

/*
*   Use call back to call BluetoothFrameworkTester callbackDumpCoverage
*/
void BtFw_dumpEmmaReport()
{
	g_jni_env->CallStaticVoidMethod(g_BtFwTester_clazz,
            method_onDumpCoverage);
}

/*
*   Use call back to call BluetoothFrameworkTester callbackCallPrivateMethod
*/
void BtFw_callPrivateMethod(int classId, int id)
{
	
	BTFW_LOG("[API:BtFw_callbackPrivate] class id is %d", classId);
	BTFW_LOG("[API:BtFw_callbackPrivate] method id is : %d", id);
	g_jni_env->CallStaticVoidMethod(g_BtFwTester_clazz, method_onCallPrivate, classId, id);
}

static jboolean initServiceNative(JNIEnv* env, jobject object) 
{
	return JNI_TRUE;
}

/*
*   initBtUtEnv, used to register dump coverage report and call private method
*/
static void initBtUtEnv()
{
	BTFW_LOG(" BTUTLog_setOnDumpEmmaReport");
	BTUTLog_setOnDumpEmmaReport(BtFw_dumpEmmaReport);
	BTUTLog_setCallbackPrivate(BtFw_callPrivateMethod);
}

/*
*   native methods table
*/
static JNINativeMethod sMethods[] = {
	{"initServiceNative", "()Z", (void*) initServiceNative}
};

/*
*   This method is called in AndroidRuntime.cpp, which will be called when the phone is opened
*   call initBtUtEnv, register native method, get BluetoothFrameworkTester class
*   register callback methods
*/
int register_android_server_BluetoothFrameworkUT(JNIEnv *env)
{
	if (g_jni_env == NULL)
	{
		g_jni_env = env;
		initBtUtEnv();
	}
    g_BtFwTester_clazz = env->FindClass("android/server/BluetoothFrameworkTester");
    if (g_BtFwTester_clazz == NULL) {
        ALOGE("[BT][FRAMEWORK][ERR] Can't find android/server/BluetoothFrameworkTester");
        return -1;
    }

	BTFW_LOG("clazz 0x%x",g_BtFwTester_clazz);
	method_onCreate = env->GetStaticMethodID(g_BtFwTester_clazz, "callbackCreateBTFrameworkTester", "(Z)V");
	method_onDumpCoverage = env->GetStaticMethodID(g_BtFwTester_clazz, "callbackDumpCoverage", "()V");
	method_onCallPrivate = env->GetStaticMethodID(g_BtFwTester_clazz, "callbackCallPrivateMethod", "(II)V");

    return AndroidRuntime::registerNativeMethods(env,
                "android/server/BluetoothFrameworkTester", sMethods, NELEM(sMethods));
}


} /* namespace android */
