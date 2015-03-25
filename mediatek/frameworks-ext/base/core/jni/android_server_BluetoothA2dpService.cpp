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
*/

#define LOG_TAG "BluetoothA2dpService.cpp"

#include "android_bluetooth_common.h"
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

#ifdef HAVE_BLUETOOTH
#include <dbus/dbus.h>
#endif

#ifdef __BTMTK__
extern "C"
{
#include "bt_a2dp_api.h"
#include "bt_a2dp_hdl.h"
}

#define A2DP_EVENT_LOOP_REFS 20

#endif

namespace android {

#ifdef HAVE_BLUETOOTH
static jmethodID method_onSinkPropertyChanged;
static jmethodID method_onConnectSinkResult;

typedef struct {
    JavaVM *vm;
    int envVer;
    DBusConnection *conn;
    jobject me;  // for callbacks to java
} native_data_t;

static native_data_t *nat = NULL;  // global native data
static void onConnectSinkResult(DBusMessage *msg, void *user, void *n);

static Properties sink_properties[] = {
        {"State", DBUS_TYPE_STRING},
        {"Connected", DBUS_TYPE_BOOLEAN},
        {"Playing", DBUS_TYPE_BOOLEAN},
      };
#endif

#ifdef __BTMTK__

static jmethodID method_onSinkPropertyChanged;

typedef struct {
    JavaVM *vm;
    int envVer;
    jobject me;  // for callbacks to java

    pthread_mutex_t thread_mutex;
    pthread_t thread;

    int a2dpsrvcsock;    // for receiving indication
    int a2dpsock;           // for send request and receive response
} native_data_t;

static native_data_t *nat = NULL;  // global native data
#endif

#ifdef __BTMTK__
static jboolean startEventLoop(JNIEnv* env, jobject object);
static void *a2dp_eventLoopMain(void *ptr);
static jboolean a2dp_registerSocket(JNIEnv *env, jobject object);
#endif

/* Returns true on success (even if adapter is present but disabled).
 * Return false if dbus is down, or another serious error (out of memory)
*/
static bool initNative(JNIEnv* env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    if (NULL == nat) {
        ALOGE("%s: out of memory!", __FUNCTION__);
        return false;
    }
    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();
    nat->me = env->NewGlobalRef(object);

    DBusError err;
    dbus_error_init(&err);
    dbus_threads_init_default();
    nat->conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        ALOGE("Could not get onto the system bus: %s", err.message);
        dbus_error_free(&err);
        return false;
    }
    dbus_connection_set_exit_on_disconnect(nat->conn, FALSE);
    return true;
#endif  /*HAVE_BLUETOOTH*/
#ifdef __BTMTK__
    ALOGI("[A2DP][JNI] initNative");

    nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    if (NULL == nat) {
        ALOGE("[A2DP][ERR] %s: out of memory!", __FUNCTION__);
        return false;
    }

    memset(nat,0,sizeof(native_data_t));
    //init socket handler value
    nat->a2dpsrvcsock = -1;
    nat->a2dpsock = -1;
    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();
    nat->me = env->NewGlobalRef(object);
    return true;
#endif
    return true;
}

static void cleanupNative(JNIEnv* env, jobject object) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        dbus_connection_close(nat->conn);
        env->DeleteGlobalRef(nat->me);
        free(nat);
        nat = NULL;
    }
#endif
#ifdef __BTMTK__
    ALOGI("[A2DP][JNI] cleanupNative");
    if (nat) 
    {
        free(nat);
    }
#endif
}

static void startNative(JNIEnv* env, jobject object) {
#ifdef __BTMTK__
    ALOGI("[A2DP][JNI] startNative");

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return;
    }

    if(nat->a2dpsrvcsock == -1)
    {
        jboolean ret;
        ret = a2dp_registerSocket(env, object);
        if(!ret)
        {
            ALOGW("[A2DP] a2dp_registerSocket fail");
            return;
        }
    }
    else
    {
        //-----------------------------------------------------------------
        //           activate A2DP
        //-----------------------------------------------------------------
        btmtk_a2dp_send_activate_req(nat->a2dpsrvcsock, nat->a2dpsock);
    }
#endif
}

static void stopNative(JNIEnv* env, jobject object) 
{
#ifdef __BTMTK__

    ALOGI("[A2DP][JNI] stopNative");
	btmtk_a2dp_deactive_req();
	
#endif
}

static jobjectArray getSinkPropertiesNative(JNIEnv *env, jobject object,
                                            jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        DBusMessage *msg, *reply;
        DBusError err;
        dbus_error_init(&err);

        const char *c_path = env->GetStringUTFChars(path, NULL);
        reply = dbus_func_args_timeout(env,
                                   nat->conn, -1, c_path,
                                   "org.bluez.AudioSink", "GetProperties",
                                   DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);
        if (!reply && dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, reply);
            return NULL;
        } else if (!reply) {
            ALOGE("DBus reply is NULL in function %s", __FUNCTION__);
            return NULL;
        }
        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter))
            return parse_properties(env, &iter, (Properties *)&sink_properties,
                                 sizeof(sink_properties) / sizeof(Properties));
    }
#endif
#ifdef __BTMTK__
    jobjectArray strArray = NULL;
    int array_index = 0;
    jclass stringClass = NULL;
    /* Get properties */
    Properties prop;
    property_value value;
    A2dpStatus iA2dpState;

    ALOGI("[A2DP][JNI] getSinkPropertiesNative");

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }

    if(nat->a2dpsrvcsock == -1)
    {
        jboolean ret;
        ret = a2dp_registerSocket(env, object);
        if(!ret)
        {
            ALOGW("[A2DP] a2dp_registerSocket fail");
            return NULL;
        }
    }

    stringClass = env->FindClass("java/lang/String");
    //strArray = env->NewObjectArray(BTMTK_A2DP_PROP_TOTAL * 2, stringClass, NULL);
    strArray = env->NewObjectArray(6, stringClass, NULL);
    
    /* Get A2dp State */
    ALOGI("[A2DP] Get State (disc:1, connecting:2, onnected:3, or playing:4)");	
    
    /* Add property: "State", DBUS_TYPE_STRING */
    iA2dpState = btmtk_a2dp_get_ap_state();
    ALOGI("[A2DP] Get State result %d", iA2dpState);	
    strcpy(prop.name, "State");
    prop.type = (int) 's';
    
    if(iA2dpState == A2DP_STATUS_CHANGE_DISCONNECTED)
        value.str_val = (char *)"disconnected";
    else if (iA2dpState == A2DP_STATUS_CHANGE_CONNECTING)
        value.str_val = (char *)"connecting";
   else if (iA2dpState ==A2DP_STATUS_CHANGE_CONNECTED)
        value.str_val = (char *)"connected";
   else
        value.str_val = (char *)"playing";
    
    create_prop_array(env, strArray, &prop, &value, 0, &array_index);
    /* Add property: "Connected", DBUS_TYPE_BOOLEAN */
    strcpy(prop.name, "Connected");
    prop.type = (int) 'b';
    value.int_val = (iA2dpState == A2DP_STATUS_CHANGE_CONNECTED) ? 1 : 0;
   
	create_prop_array(env, strArray, &prop, &value, 0, &array_index);

    /* Add property: "Playing", DBUS_TYPE_BOOLEAN */
    strcpy(prop.name, "Playing");
    prop.type = (int) 'b';
    value.int_val = (iA2dpState == A2DP_STATUS_CHANGE_PLAYING) ? 1 : 0;
    
	create_prop_array(env, strArray, &prop, &value, 0, &array_index);
   
    return strArray;
#endif
    return NULL;
}

static jboolean connectSinkNative(JNIEnv *env, jobject object, jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1, onConnectSinkResult, context_path,
                                    nat, c_path, "org.bluez.AudioSink", "Connect",
                                    DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
    const char *c_path = env->GetStringUTFChars(path, NULL);
    bt_addr_struct addr;

    ALOGI("[A2DP][JNI] connectSinkNative: addr=%s", c_path);

    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }

    if(nat->a2dpsrvcsock == -1)
    {
        jboolean ret;
        ret = a2dp_registerSocket(env, object);
        if(!ret)
        {
            ALOGW("[A2DP] a2dp_registerSocket fail");
            return NULL;
        }
    }
    
    btmtk_util_convert_objpath2bdaddr(c_path, &addr);
                
    btmtk_a2dp_send_stream_open_req(MOD_BT, &addr, BT_A2DP_SINK);
    return JNI_TRUE;
#endif
    return JNI_FALSE;
}

static jboolean disconnectSinkNative(JNIEnv *env, jobject object,
                                     jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);

        bool ret = dbus_func_args_async(env, nat->conn, -1, NULL, NULL, nat,
                                    c_path, "org.bluez.AudioSink", "Disconnect",
                                    DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
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

    if(nat->a2dpsrvcsock == -1)
    {
        jboolean ret;
        ret = a2dp_registerSocket(env, object);
        if(!ret)
        {
            ALOGW("[A2DP] a2dp_registerSocket fail");
            return NULL;
        }
    }

    adapterlen = strlen(BTMTK_ANDROID_ADAPTER_PREFIX);
    
    /* pattern ($adapter)/dev_XX_XX_XX_XX_XX_XX */
    //for(i = 0, ptr= (char*)c_path + adapterlen + 5; i < 6; i++, ptr++)
    //{
    //    addr[i] = strtoul(ptr, &ptr, 16);
    //}
    //btmtk_A2dp_send_appi_bt_disconnect_request(addr);
    ALOGE("[A2DP] a2dp_close_disconnect() function is running");
    btmtk_util_convert_objpath2bdaddr(c_path, &addr);
    ALOGE("[A2DP] close request addr:%x:%x:%x", addr.lap, addr.nap, addr.uap);
    btmtk_a2dp_close_device(&addr);

    return JNI_TRUE;
#endif
    return JNI_FALSE;
}

static jboolean suspendSinkNative(JNIEnv *env, jobject object,
                                     jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        bool ret = dbus_func_args_async(env, nat->conn, -1, NULL, NULL, nat,
                           c_path, "org.bluez.audio.Sink", "Suspend",
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
    ALOGI("[A2DP][JNI] suspendSinkNative");
    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }

    if(nat->a2dpsrvcsock == -1)
    {
        jboolean ret;
        ret = a2dp_registerSocket(env, object);
        if(!ret)
        {
            ALOGW("[A2DP] a2dp_registerSocket fail");
            return NULL;
        }
    }
    btmtk_A2dp_send_appi_bt_stop_request();
    return JNI_TRUE;
#endif
    return JNI_FALSE;
}

static jboolean resumeSinkNative(JNIEnv *env, jobject object,
                                     jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        bool ret = dbus_func_args_async(env, nat->conn, -1, NULL, NULL, nat,
                           c_path, "org.bluez.audio.Sink", "Resume",
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
    ALOGI("[A2DP][JNI] resumeSinkNative");
    if(!nat)
    {
        ALOGW("[A2DP] nat is NULL");
        return NULL;
    }

    if(nat->a2dpsrvcsock == -1)
    {
        jboolean ret;
        ret = a2dp_registerSocket(env, object);
        if(!ret)
        {
            ALOGW("[A2DP] a2dp_registerSocket fail");
            return NULL;
        }
    }
    btmtk_A2dp_send_appi_bt_start_request();
    return JNI_TRUE;
#endif
    return JNI_FALSE;
}

static jboolean avrcpVolumeUpNative(JNIEnv *env, jobject object,
                                     jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        bool ret = dbus_func_args_async(env, nat->conn, -1, NULL, NULL, nat,
                           c_path, "org.bluez.Control", "VolumeUp",
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

//XH, add avrcp_cl function
#ifdef __BTMTK__
    ALOGE("[avrcp] avrcpVolumeUpNative");
    btmtk_a2dp_avrcp_send_cmd_key_req(0x41); //AVRCP_POP_VOLUME_UP         0x41
	return JNI_TRUE;
#endif
    return JNI_FALSE;
}

static jboolean avrcpVolumeDownNative(JNIEnv *env, jobject object,
                                     jstring path) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        bool ret = dbus_func_args_async(env, nat->conn, -1, NULL, NULL, nat,
                           c_path, "org.bluez.Control", "VolumeDown",
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

//XH, add avrcp_cl function
#ifdef __BTMTK__
    ALOGE("[avrcp] avrcpVolumeDownNative");
    btmtk_a2dp_avrcp_send_cmd_key_req(0x42); //AVRCP_POP_VOLUME_DOWN         0x42
	return JNI_TRUE;
#endif
    return JNI_FALSE;
}

static jint fmSendStartReqNative(JNIEnv* env, jobject object){
#ifdef __BTMTK__
#ifdef MTK_BT_FM_OVER_BT_VIA_CONTROLLER
	ALOGI("[A2DP][JNI] fmSendStartReqNative");
	if(btmtk_a2dp_fm_send_start_req() != 2){
        ALOGE("[A2DP]Reconfig fail, because no match sbc parameters with remote device failure");
		return 0;
	}else{
		return 1;
	}
#endif
#endif
		return 0;
}

static void fmSendStopReqNative(JNIEnv* env, jobject object){
#ifdef __BTMTK__
#ifdef MTK_BT_FM_OVER_BT_VIA_CONTROLLER
	ALOGI("[A2DP][JNI] fmSendStopReqNative");
	btmtk_a2dp_fm_send_stop_req();
#endif
#endif
}

static void decA2dpThroughput4WifiOnNative(JNIEnv* env, jobject object){
#ifdef __BTMTK__
	ALOGI("[A2DP][JNI] decA2dpThroughput4WifiOnNative");
	btmtk_a2dp_send_wifi_connect_req();
#endif
}

static void incA2dpThroughput4WifiOffNative(JNIEnv* env, jobject object){
#ifdef __BTMTK__
	ALOGI("[A2DP][JNI] incA2dpThroughput4WifiOffNative");
	btmtk_a2dp_send_wifi_disconnect_req();
#endif
}


#ifdef HAVE_BLUETOOTH
DBusHandlerResult a2dp_event_filter(DBusMessage *msg, JNIEnv *env) {
    DBusError err;

    if (!nat) {
        ALOGV("... skipping %s\n", __FUNCTION__);
        ALOGV("... ignored\n");
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    dbus_error_init(&err);

    if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (dbus_message_is_signal(msg, "org.bluez.AudioSink",
                                      "PropertyChanged")) {
        jobjectArray str_array =
                    parse_property_change(env, msg, (Properties *)&sink_properties,
                                sizeof(sink_properties) / sizeof(Properties));
        const char *c_path = dbus_message_get_path(msg);
        jstring path = env->NewStringUTF(c_path);
        env->CallVoidMethod(nat->me,
                            method_onSinkPropertyChanged,
                            path,
                            str_array);
        env->DeleteLocalRef(path);
        result = DBUS_HANDLER_RESULT_HANDLED;
        return result;
    } else {
        ALOGV("... ignored");
    }
    if (env->ExceptionCheck()) {
        ALOGE("VM Exception occurred while handling %s.%s (%s) in %s,"
             " leaving for VM",
             dbus_message_get_interface(msg), dbus_message_get_member(msg),
             dbus_message_get_path(msg), __FUNCTION__);
    }

    return result;
}

void onConnectSinkResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    const char *path = (const char *)user;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);


    bool result = JNI_TRUE;
    if (dbus_set_error_from_message(&err, msg)) {
        LOG_AND_FREE_DBUS_ERROR(&err);
        result = JNI_FALSE;
    }
    ALOGV("... Device Path = %s, result = %d", path, result);

    jstring jPath = env->NewStringUTF(path);
    env->CallVoidMethod(nat->me,
                        method_onConnectSinkResult,
                        jPath,
                        result);
    env->DeleteLocalRef(jPath);
    free(user);
}


#endif


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

int register_android_server_BluetoothA2dpService(JNIEnv *env) {
    jclass clazz = env->FindClass("android/server/BluetoothA2dpService");
    if (clazz == NULL) {
        ALOGE("[A2DP][ERR] Can't find android/server/BluetoothA2dpService");
        return -1;
    }

#ifdef HAVE_BLUETOOTH
    method_onSinkPropertyChanged = env->GetMethodID(clazz, "onSinkPropertyChanged",
                                          "(Ljava/lang/String;[Ljava/lang/String;)V");
    method_onConnectSinkResult = env->GetMethodID(clazz, "onConnectSinkResult",
                                                         "(Ljava/lang/String;Z)V");
#endif

#ifdef __BTMTK__
    method_onSinkPropertyChanged = env->GetMethodID(clazz, "onSinkPropertyChanged",
                                          "(Ljava/lang/String;[Ljava/lang/String;)V");
#endif
    return AndroidRuntime::registerNativeMethods(env,
                "android/server/BluetoothA2dpService", sMethods, NELEM(sMethods));
}

#ifdef __BTMTK__
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
static void *a2dp_eventLoopMain(void *ptr)
{
    //native_data_t *nat_event_loop = (native_data_t *)ptr;
    JNIEnv *env;
    int res = 0;
    fd_set readfs;
    int sockfd;
    ilm_struct ilm;

    JavaVMAttachArgs args;
    char name[] = "BT A2DP EventLoop";

    args.version = nat->envVer;
    args.name = name;
    args.group = NULL;
    
    nat->vm->AttachCurrentThread(&env, &args);
    
    ALOGI("[A2DP][JNI] a2dp_eventLoopMain");
    
    while (1)
    {
#if A2DP_STREAM_DEBUG
        ALOGI("[A2DP][EventLoop] A2dp Service Start retrieve data");
#endif
        env->PushLocalFrame(A2DP_EVENT_LOOP_REFS);

        sockfd = nat->a2dpsrvcsock;
        FD_ZERO(&readfs);
        if(sockfd>=0)
        {
            FD_SET(sockfd, &readfs);
        }
        else
        {
            ALOGE("[A2DP][EventLoop][ERR] nat->a2dpsrvcsock == 0. exit");
        }
#if A2DP_STREAM_DEBUG
        //ALOGI("[A2DP][EventLoop] Start select : sockfd=%d", sockfd);
#endif
        res = select(sockfd+1, &readfs, NULL, NULL, NULL);
#if A2DP_STREAM_DEBUG
        //ALOGI("[A2DP][EventLoop] Return from select : soresckfd=%d", res);
#endif
        if(res > 0)
        {
            res = recvfrom(sockfd, (void*)&ilm, sizeof(ilm_struct), 0, NULL, NULL);
            ALOGI("[A2DP][EventLoop] Recv A2DP CNF/IND : %lu", ilm.msg_id);
            if(res < 0)
            {
                ALOGE("[A2DP][EventLoop][ERR] recvfrom failed : %s, %d", strerror(errno), errno);
            }
            else if(res == sizeof(int))
            {
#if 0            
                int evt = *(int*)&ilm;
                ALOGI("recv event : %d", evt);
                env->CallVoidMethod(object,
                    method_onEvent,
                    evt);
#endif
            }
            else
            {
                A2dpStatus iResult = A2DP_STATUS_NO_CHANGE;
                char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
                bt_addr_struct  addr;
                jobjectArray strArray = NULL;
                int array_index = 0;
                jclass stringClass = NULL;
                /* Get properties */
                Properties prop;
                property_value value;

                stringClass = env->FindClass("java/lang/String");
                strArray = env->NewObjectArray(2, stringClass, NULL);

                strcpy(prop.name, "State");
                prop.type = (int) 's';
                               
                iResult = btmtk_a2dp_handle_message(&ilm);
#if A2DP_STREAM_DEBUG
                ALOGI("[A2DP][EventLoop] btmtk_a2dp_handle_message result =%d", iResult);
#endif
                nat->vm->GetEnv((void**)&env, nat->envVer);
                switch(iResult)
                {
                    case A2DP_STATUS_CHANGE_DISCONNECTED:
                        {
                            jstring str;
                            btmtk_a2dp_get_bdaddr(&addr);
						
                            btmtk_util_convert_bdaddr2objpath(object_path, &(addr));
                            value.str_val = (char *)"disconnected";
                           
							create_prop_array(env, strArray, &prop, &value, 0, &array_index);
                            ALOGI("[A2DP][EventLoop] create_prop_array  disconnected");
                            str = env->NewStringUTF(object_path);
                            env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
                            env->DeleteLocalRef(str);
                   	   }
                        break;
                    case A2DP_STATUS_CHANGE_CONNECTING:
                        {
                            jstring str;
                            btmtk_a2dp_get_bdaddr(&addr);
						
                            btmtk_util_convert_bdaddr2objpath(object_path, &(addr));
                            value.str_val = (char *)"connecting";
                            
			    create_prop_array(env, strArray, &prop, &value, 0, &array_index);
                            ALOGI("[A2DP][EventLoop] create_prop_array  connecting");
                            str = env->NewStringUTF(object_path);
                            env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
                            env->DeleteLocalRef(str);
                   	   }
                        break;
                    case A2DP_STATUS_CHANGE_CONNECTED:
                        {
                            jstring str;
                            btmtk_a2dp_get_bdaddr(&addr);
							
                            btmtk_util_convert_bdaddr2objpath(object_path, &(addr));
                            value.str_val = (char *)"connected";
                         
			    create_prop_array(env, strArray, &prop, &value, 0, &array_index);
                            ALOGI("[A2DP][EventLoop] create_prop_array  connected");
                            str = env->NewStringUTF(object_path);
                            env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
                            env->DeleteLocalRef(str);
                   	   }
                        break;
                    case A2DP_STATUS_CHANGE_PLAYING:
                        {
                            jstring str;
                            btmtk_a2dp_get_bdaddr(&addr);
						
                            btmtk_util_convert_bdaddr2objpath(object_path, &(addr));
                            value.str_val = (char *)"playing";
                           
			    create_prop_array(env, strArray, &prop, &value, 0, &array_index);
                            ALOGI("[A2DP][EventLoop] create_prop_array  playing");
                            str = env->NewStringUTF(object_path);
                            env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
                            env->DeleteLocalRef(str);
                   	   }
                        break;
					#ifdef __BTMTK__
					#ifdef MTK_BT_FM_OVER_BT_VIA_CONTROLLER
					case A2DP_STATUS_FM_PLAYING_FAILED:
						{
							jstring str;
                            btmtk_a2dp_get_bdaddr(&addr);
						
                            btmtk_util_convert_bdaddr2objpath(object_path, &(addr));
                            value.str_val = (char *)"fmstartfailed";
                           
			    create_prop_array(env, strArray, &prop, &value, 0, &array_index);
                            ALOGI("[A2DP][EventLoop] fmstartfailed");
                            str = env->NewStringUTF(object_path);
                            env->CallVoidMethod(nat->me, method_onSinkPropertyChanged, str, strArray);
                            env->DeleteLocalRef(str);
						}
						break;
					#endif
					#endif
                }
                env->DeleteLocalRef(strArray);
            }
        }
        else if(res == 0)
        {
            ALOGW("[A2DP][EventLoop][ERR] timeout waiting indication");
            break;
        }
        else
        {
            if ( errno != EINTR ) {
                ALOGE("[A2DP][EventLoop][ERR] select failed : %s, %d", strerror(errno), errno);
                break;
            }
        }
        env->PopLocalFrame(NULL);
    }
    if(res <= 0)
    {
        nat->vm->DetachCurrentThread();
    }
    return NULL;
}
static jboolean a2dp_registerSocket(JNIEnv *env, jobject object)
{
    sockaddr_un a2dpextname;
    socklen_t   a2dpextnamelen;
    struct sockaddr_un a2dpintname;
    socklen_t   a2dpintnamelen;    

    ALOGI("[A2DP][JNI] a2dp_registerSocket");

    //-----------------------------------------------------------------
    //           start setup socket
    //-----------------------------------------------------------------
    // Setup bt server address
    a2dpintname.sun_family = AF_UNIX;
    strcpy (a2dpintname.sun_path, /*BT_SERV_SOCK_ADDR*/BT_SOCK_NAME_INT_ADP);
    a2dpintnamelen = (offsetof (struct sockaddr_un, sun_path) + strlen (a2dpintname.sun_path) + 1);    
    // Setup a2dp service socket
    nat->a2dpsrvcsock = socket_local_server(BT_SOCK_NAME_EXT_ADP_A2DP, 
                                                                      ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
                                                                      SOCK_DGRAM);
    if(nat->a2dpsrvcsock < 0)
    {
        ALOGE("[A2DP][ERR] create a2dp server socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }
    else
    {//nat->a2dpsrvcsock >= 0
        a2dpextnamelen = sizeof(a2dpextname.sun_path);
        a2dpextname.sun_path[0] = '\0';
	 ALOGI("[A2DP] create a2dp server socket success");
        if (getsockname(nat->a2dpsrvcsock, (sockaddr*)&a2dpextname, &a2dpextnamelen) < 0)
        {
            ALOGI("[A2DP] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[A2DP] Auto bind A2DP server : len=%d, addr=%s", a2dpextnamelen, &a2dpextname.sun_path[1]);
        }
    }
    btmtk_a2dp_setSockAddress(&a2dpextname, a2dpextnamelen);
    // Setup a2dp api socket
    a2dpextnamelen = sizeof(short);
    nat->a2dpsock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    ALOGI("[A2DP] initNative nat->a2dpsock==%d", nat->a2dpsock);
    if (nat->a2dpsock < 0)
    {
        ALOGE("[A2DP][ERR] create a2dp api socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }

	//for block issue[ALPS00134835]
	struct timeval timeout;
	timeout.tv_sec = 30; //30s timeout
        timeout.tv_usec = 0;
	if(setsockopt(nat->a2dpsock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)))
	{
	    ALOGE("[A2DP] set a2dpsock timeout fail");	

	}else{
		
            ALOGI("[A2DP] set a2dpsock timeout success");
	}

	
    if (bind (nat->a2dpsock, (struct sockaddr *) &a2dpextname, a2dpextnamelen) < 0)
    {
        ALOGE("[A2DP][ERR] bind a2dp api socket failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }
    else
    {
        a2dpextnamelen = sizeof(a2dpextname.sun_path);
        a2dpextname.sun_path[0] = '\0';
        if (getsockname(nat->a2dpsock, (sockaddr*)&a2dpextname, &a2dpextnamelen) < 0)
        {
            ALOGE("[A2DP][ERR] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[A2DP] Auto bind A2DP api socket : len=%d, addr=%s", a2dpextnamelen, &a2dpextname.sun_path[1]);
        }
    }
    if ( connect(nat->a2dpsock, (const struct sockaddr*)&a2dpintname, a2dpintnamelen) < 0)
    {
        ALOGE("[A2DP][ERR] connect to /data/btserv failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }

    //-----------------------------------------------------------------
    //           start receiving indication
    //-----------------------------------------------------------------
    startEventLoop(env, object);

    //-----------------------------------------------------------------
    //           activate A2DP
    //-----------------------------------------------------------------
    btmtk_a2dp_send_activate_req(nat->a2dpsrvcsock, nat->a2dpsock);

    return JNI_TRUE;

exit:
    if (nat->a2dpsrvcsock>=0)
    {
        close(nat->a2dpsrvcsock);
        nat->a2dpsrvcsock = -1;
    }
    if (nat->a2dpsock>=0)
    {
        close(nat->a2dpsock);
        nat->a2dpsock = -1;
    }
    return JNI_FALSE;
}

#endif

} /* namespace android */
