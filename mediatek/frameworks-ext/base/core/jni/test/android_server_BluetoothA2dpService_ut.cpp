/*
** Copyright 2008, The Android Open Source Project
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
**
** This file is only for unit test
*/

#define LOG_TAG "BluetoothA2dpService.cpp"

#include "../android_bluetooth_common.h"
#include "android_runtime/AndroidRuntime.h"
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bt_simulator.h"

#define A2DP_EVENT_LOOP_REFS 20

#define A2DP_UT_DEBUG

#ifdef A2DP_UT_DEBUG
#define A2DP_LOG(fmt, ...) XLOGI("[BT][A2DP][UT]%s:" fmt, __FUNCTION__, ## __VA_ARGS__)
#else
#define A2DP_LOG(fmt, ...)
#endif

void test()
{
	::BTUTLog_initReceiver((char*)("Bluetooth.BTSimulatorReceiver.A2DP"));
}

namespace android {

static jmethodID method_onSinkPropertyChanged;

typedef struct {
    JavaVM *vm;
    int envVer;
    jobject me;  // for callbacks to java

	int isRunning;  // is ut simulator receiver  running

    pthread_mutex_t thread_mutex;
    pthread_t thread;
} native_data_t;

static native_data_t *nat = NULL;  // global native data

static jboolean startEventLoop(JNIEnv* env, jobject object);
static void *a2dp_eventLoopMain(void *ptr);

/* Returns true on success (even if adapter is present but disabled).
 * Return false if dbus is down, or another serious error (out of memory)
*/
static bool initNative(JNIEnv* env, jobject object)
{

    ALOGI("[A2DP][JNI] initNative");

    nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    if (NULL == nat) {
        ALOGE("[A2DP][ERR] %s: out of memory!", __FUNCTION__);
        return false;
    }

    memset(nat,0,sizeof(native_data_t));
    nat->isRunning = FALSE;
    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();
    nat->me = env->NewGlobalRef(object);
    return true;

}

static void cleanupNative(JNIEnv* env, jobject object)
{
    ALOGI("[A2DP][JNI] cleanupNative");
    if (nat) 
    {
        free(nat);
    }
}


static void startNative(JNIEnv* env, jobject object)
{
    ALOGI("[A2DPUT][JNI] startNative");

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return;
    }

	if (nat->isRunning == FALSE)
	{
		startEventLoop(env, object);
		::BTUTLog_initReceiver("Bluetooth.BTSimulatorReceiver.A2DP");
		nat->isRunning = TRUE;
	}
}

static void stopNative(JNIEnv* env, jobject object) 
{
    ALOGI("[A2DP][JNI] stopNative");
	//btmtk_a2dp_deactive_req();
}

static jobjectArray getSinkPropertiesNative(JNIEnv *env, jobject object, jstring path)
{
    jobjectArray strArray = NULL;
    int array_index = 0;
    jclass stringClass = NULL;
    /* Get properties */
    Properties prop;
    property_value value;
    //A2dpStatus iA2dpState;

    ALOGI("[A2DP][JNI] getSinkPropertiesNative");

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }
	return NULL;
}

static jboolean connectSinkNative(JNIEnv *env, jobject object, jstring path) 
{
    const char *c_path = env->GetStringUTFChars(path, NULL);
    bt_addr_struct addr;

    ALOGI("[A2DP][JNI] connectSinkNative: addr=%s", c_path);

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }
    return JNI_TRUE;
}

static jboolean disconnectSinkNative(JNIEnv *env, jobject object, jstring path)
{
    const char *c_path = env->GetStringUTFChars(path, NULL);
    bt_addr_struct addr;
    int adapterlen;
    int i;
    ALOGI("[A2DP][JNI] disconnectSinkNative: addr=%s", c_path);

    char *ptr;

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }
    return JNI_TRUE;
}

static jboolean suspendSinkNative(JNIEnv *env, jobject object, jstring path) 
{
    ALOGI("[A2DP][JNI] suspendSinkNative");
    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }

    return JNI_TRUE;
}

static jboolean resumeSinkNative(JNIEnv *env, jobject object, jstring path) 
{
    ALOGI("[A2DP][JNI] resumeSinkNative");
    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }

    return JNI_TRUE;
}

static jboolean avrcpVolumeUpNative(JNIEnv *env, jobject object, jstring path)
{

    ALOGE("[avrcp] avrcpVolumeUpNative");
    return JNI_TRUE;

}

static jboolean avrcpVolumeDownNative(JNIEnv *env, jobject object, jstring path)
{
    ALOGE("[avrcp] avrcpVolumeDownNative");
    return JNI_TRUE;
}

static jint fmSendStartReqNative(JNIEnv* env, jobject object)
{
#ifdef MTK_BT_FM_OVER_BT_VIA_CONTROLLER
	ALOGI("[A2DP][JNI] fmSendStartReqNative");

#endif
	return 1;
}

static void fmSendStopReqNative(JNIEnv* env, jobject object)
{
#ifdef MTK_BT_FM_OVER_BT_VIA_CONTROLLER
	ALOGI("[A2DP][JNI] fmSendStopReqNative");
	//btmtk_a2dp_fm_send_stop_req();
#endif 
}

static void decA2dpThroughput4WifiOnNative(JNIEnv* env, jobject object){ 
	ALOGI("[A2DP][JNI] decA2dpThroughput4WifiOnNative");
	//btmtk_a2dp_send_wifi_connect_req(); 
}

static void incA2dpThroughput4WifiOffNative(JNIEnv* env, jobject object)
{
 	ALOGI("[A2DP][JNI] incA2dpThroughput4WifiOffNative");
	//btmtk_a2dp_send_wifi_disconnect_req();
 
}

static JNINativeMethod sMethods[] = {
    {"initNative", "()Z", (void *)initNative},
    {"cleanupNative", "()V", (void *)cleanupNative},
    {"startNative", "()V", (void *)startNative},
    {"stopNative", "()V", (void *)stopNative},

    {"fmSendStartReqNative", "()I", (void *)fmSendStartReqNative},
    {"fmSendStopReqNative", "()V", (void *)fmSendStopReqNative},

    {"decA2dpThroughput4WifiOnNative", "()V", (void *)decA2dpThroughput4WifiOnNative},
    {"incA2dpThroughput4WifiOffNative", "()V", (void *)incA2dpThroughput4WifiOffNative},

    /* Bluez audio 4.47 API */
    {"connectSinkNative", "(Ljava/lang/String;)Z", (void *)connectSinkNative},
    {"disconnectSinkNative", "(Ljava/lang/String;)Z", (void *)disconnectSinkNative},
    {"suspendSinkNative", "(Ljava/lang/String;)Z", (void*)suspendSinkNative},
    {"resumeSinkNative", "(Ljava/lang/String;)Z", (void*)resumeSinkNative},
    {"getSinkPropertiesNative", "(Ljava/lang/String;)[Ljava/lang/Object;",
                                    (void *)getSinkPropertiesNative},
    {"avrcpVolumeUpNative", "(Ljava/lang/String;)Z", (void*)avrcpVolumeUpNative},
    {"avrcpVolumeDownNative", "(Ljava/lang/String;)Z", (void*)avrcpVolumeDownNative},
};
//extern int register_android_server_BluetoothFrameworkUT(JNIEnv *env);
int register_android_server_BluetoothA2dpService(JNIEnv *env)
{
    jclass clazz = env->FindClass("android/server/BluetoothA2dpService");
    if (clazz == NULL) {
        ALOGE("[A2DP][ERR] Can't find android/server/BluetoothA2dpService");
        return -1;
    }

    method_onSinkPropertyChanged = env->GetMethodID(clazz, "onSinkPropertyChanged",
                                          "(Ljava/lang/String;[Ljava/lang/String;)V");

	int res = AndroidRuntime::registerNativeMethods(env,
                "android/server/BluetoothA2dpService", sMethods, NELEM(sMethods));

	//register_android_server_BluetoothFrameworkUT(env);
	
    return res;
}

static jboolean startEventLoop(JNIEnv* env, jobject object)
{
    ALOGI("[A2DP][JNI] startEventLoop");

    pthread_mutex_lock(&(nat->thread_mutex));

    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();
    nat->me = env->NewGlobalRef(object);

    pthread_create(&(nat->thread), NULL, a2dp_eventLoopMain, nat);

    pthread_mutex_unlock(&(nat->thread_mutex));
    return JNI_TRUE;
}
#define BTUT_MAX_OBJECT_PATH_SIZE 32

typedef enum {
	A2DPUT_STATUS_NO_CHANGE = 0,
	A2DPUT_STATUS_CHANGE_DISCONNECTED,
	A2DPUT_STATUS_CHANGE_CONNECTING,
	A2DPUT_STATUS_CHANGE_CONNECTED,
	A2DPUT_STATUS_CHANGE_PLAYING,
	A2DPUT_STATUS_FM_PLAYING_FAILED,
};

static void *a2dp_eventLoopMain(void *ptr)
{
	JNIEnv *env;
	int res = 0;
	ilm_struct ilm;
	JavaVMAttachArgs args;
	char name[] = "BT A2DP EventLoop";
	char* cbFuncs[] = {"onSinkPropertyChanged",						
						};
	int  cbFuncNum = 1;	

	args.version = nat->envVer;
     args.name = name;
     args.group = NULL;

     nat->vm->AttachCurrentThread(&env, &args);

	while (1)
	{
	    char object_path[BTUT_MAX_OBJECT_PATH_SIZE];
	    Properties prop;
	    property_value value;
	    jobjectArray strArray = NULL;
	    jclass stringClass = NULL;
	    int array_index = 0;
	    int curCbFunc = -1;

	    strcpy(prop.name, "State");
	    prop.type = (int) 's';

	    memset(object_path,0,BTUT_MAX_OBJECT_PATH_SIZE*sizeof(char));

	    curCbFunc = BTUTLog_listenToCbFunc(cbFuncs,cbFuncNum);
         if (curCbFunc == 0)
	    {
             UTJniLog* curLog=NULL;
		   int opt = 0;

		   curLog = BTUTLog_getInfo(UT_PROFILE_A2DP);
		   if (curLog == NULL)
		   {
                 //BTUTLog_next(UT_PROFILE_A2DP);
                 sleep(1);
			  continue;
		   }

		   stringClass = env->FindClass("java/lang/String");
             strArray = env->NewObjectArray(2, stringClass, NULL);

		   strcat(object_path, curLog->params[0]);
		   opt = atoi(curLog->params[1]);

		   ALOGI("[A2DPUT][eventLoopMain] create_prop_array  playing obj_path %s opt %d ",
						object_path, opt);
		   switch (opt)
		   {
		       case A2DPUT_STATUS_CHANGE_DISCONNECTED:
					{
						//"Cb;1;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;1;"
						jstring str;
						value.str_val = (char *)"disconnected";

						create_prop_array(env, strArray, &prop, &value, 0, &array_index);

						str = env->NewStringUTF(object_path);
						env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
						env->DeleteLocalRef(str);

						BTUTLog_next(UT_PROFILE_A2DP);
					}
					break;
				case A2DPUT_STATUS_CHANGE_CONNECTING:
					{
						//"Cb;1;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;2;"
						jstring str;
						value.str_val = (char *)"connecting";

						create_prop_array(env, strArray, &prop, &value, 0, &array_index);

						str = env->NewStringUTF(object_path);
						env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
						env->DeleteLocalRef(str);

						BTUTLog_next(UT_PROFILE_A2DP);
					}
					break;
				case A2DPUT_STATUS_CHANGE_CONNECTED:
					{
						//"Cb;1;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;3;"
						jstring str;
						value.str_val = (char *)"connecting";

						create_prop_array(env, strArray, &prop, &value, 0, &array_index);

						str = env->NewStringUTF(object_path);
						env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
						env->DeleteLocalRef(str);

						BTUTLog_next(UT_PROFILE_A2DP);
					}
					break;
				case A2DPUT_STATUS_CHANGE_PLAYING:
					{
						//"Cb;1;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;4;"
						jstring str;
						value.str_val = (char *)"playing";

						create_prop_array(env, strArray, &prop, &value, 0, &array_index);

						str = env->NewStringUTF(object_path);
						env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
						env->DeleteLocalRef(str);

						BTUTLog_next(UT_PROFILE_A2DP);
					}
					break;
				case A2DPUT_STATUS_FM_PLAYING_FAILED:
					{
						//"Cb;1;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;3;"
						jstring str;
						value.str_val = (char *)"fmstartfailed";

						create_prop_array(env, strArray, &prop, &value, 0, &array_index);

						str = env->NewStringUTF(object_path);
						env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
						env->DeleteLocalRef(str);

						BTUTLog_next(UT_PROFILE_A2DP);
					}
					break;
				default:
					break;					
			}
			env->DeleteLocalRef(strArray);
		}
		sleep(1);
	}
	return NULL;
}
 

} /* namespace android */
