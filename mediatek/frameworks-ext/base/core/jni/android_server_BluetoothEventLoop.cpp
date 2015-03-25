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

#define LOG_TAG "BluetoothEventLoop.cpp"

#include "android_bluetooth_common.h"
#include "android_runtime/AndroidRuntime.h"
#include "cutils/sockets.h"
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"
#ifndef MTK_BSP_PACKAGE
#include "custom_prop.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <cutils/properties.h>

#ifdef __BTMTK__
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bt_mmi.h"
#include "bt_message.h"
#include "bluetooth_struct.h"
#include "bluetooth_hdp_struct.h"
#endif

#ifdef HAVE_BLUETOOTH
#include <dbus/dbus.h>
#endif

namespace android {

#define CREATE_DEVICE_ALREADY_EXISTS 1
#define CREATE_DEVICE_SUCCESS 0
#define CREATE_DEVICE_FAILED -1

//#ifdef HAVE_BLUETOOTH
static jfieldID field_mNativeData;

static jmethodID method_onPropertyChanged;
static jmethodID method_onDevicePropertyChanged;
static jmethodID method_onDeviceFound;
static jmethodID method_onDeviceDisappeared;
static jmethodID method_onDeviceCreated;
static jmethodID method_onDeviceRemoved;
static jmethodID method_onDeviceDisconnectRequested;
static jmethodID method_onNetworkDeviceDisconnected;
static jmethodID method_onNetworkDeviceConnected;

static jmethodID method_onCreatePairedDeviceResult;
static jmethodID method_onCreateDeviceResult;
static jmethodID method_onDiscoverServicesResult;
static jmethodID method_onGetDeviceServiceChannelResult;

static jmethodID method_onRequestPinCode;
static jmethodID method_onRequestPasskey;
static jmethodID method_onRequestPasskeyConfirmation;
static jmethodID method_onRequestPairingConsent;
static jmethodID method_onDisplayPasskey;
static jmethodID method_onRequestOobData;
static jmethodID method_onAgentOutOfBandDataAvailable;
static jmethodID method_onAgentAuthorize;
static jmethodID method_onAgentCancel;

static jmethodID method_onInputDevicePropertyChanged;
static jmethodID method_onInputDeviceConnectionResult;
static jmethodID method_onPanDevicePropertyChanged;
static jmethodID method_onPanDeviceConnectionResult;
static jmethodID method_onHealthDevicePropertyChanged;
static jmethodID method_onHealthDeviceChannelChanged;
static jmethodID method_onHealthDeviceConnectionResult;

typedef event_loop_native_data_t native_data_t;
#if __BTMTK__
void btmtk_host_cache_init(native_data_t *nat);
void btmtk_paired_dev_cache_init(native_data_t *nat);
void btmtk_paired_dev_cache_write(native_data_t *nat);
extern bool btmtk_gap_service_search_request(bt_native_data_t *nat, bt_addr_struct *addr);
extern btmtk_hdp_fd_struct *btmtk_hdp_util_find_hdp_fd(bt_native_data_t *nat, bt_addr_struct *addr, unsigned short mdlid);
extern void btmtk_hdp_add_fd(bt_native_data_t *nat, bt_addr_struct *addr, unsigned short mdlid, int fd);
extern void btmtk_hdp_remove_fd(bt_native_data_t *nat, bt_addr_struct *addr, unsigned short mdlid);
extern int btmtk_hdp_create_socket(U8 l2capchannelId);


unsigned char SDP_INSECURE[16] = 
	{0x30, 0x1c, 0x21, 0x4f, 0x91, 0xa2, 0x43, 0xbf, 0xa7, 0x95, 0x09, 0xd1, 0x19, 0x8a, 0x81, 0xa7};
unsigned char SDP_SECURE[16] = 
	{0x85, 0x91, 0xD7, 0x57, 0x18, 0xEE, 0x45, 0xE1, 0x9B, 0x12, 0x92, 0x87, 0x5D, 0x06, 0xBA, 0x23};
extern bool btmtk_gap_security_oob_data(bt_native_data_t *nat, bt_addr_struct *addr, bool accept, char *hash, char *rand);
bool btmtk_hdp_compose_channel(bt_addr_struct *addr, U16 mdlId, char *channelPath);
#endif

#define EVENT_LOOP_REFS 10

static inline native_data_t * get_native_data(JNIEnv *env, jobject object) {
    return (native_data_t *)(env->GetIntField(object,
                                                 field_mNativeData));
}

native_data_t *get_EventLoop_native_data(JNIEnv *env, jobject object) {
    return get_native_data(env, object);
}

static void classInitNative(JNIEnv* env, jclass clazz) {
    ALOGV("%s", __FUNCTION__);

//#ifdef HAVE_BLUETOOTH
    method_onPropertyChanged = env->GetMethodID(clazz, "onPropertyChanged",
                                                "([Ljava/lang/String;)V");
    method_onDevicePropertyChanged = env->GetMethodID(clazz,
                                                      "onDevicePropertyChanged",
                                                      "(Ljava/lang/String;[Ljava/lang/String;)V");
    method_onDeviceFound = env->GetMethodID(clazz, "onDeviceFound",
                                            "(Ljava/lang/String;[Ljava/lang/String;)V");
    method_onDeviceDisappeared = env->GetMethodID(clazz, "onDeviceDisappeared",
                                                  "(Ljava/lang/String;)V");
    method_onDeviceCreated = env->GetMethodID(clazz, "onDeviceCreated", "(Ljava/lang/String;)V");
    method_onDeviceRemoved = env->GetMethodID(clazz, "onDeviceRemoved", "(Ljava/lang/String;)V");
    method_onDeviceDisconnectRequested = env->GetMethodID(clazz, "onDeviceDisconnectRequested",
                                                        "(Ljava/lang/String;)V");
    method_onNetworkDeviceConnected = env->GetMethodID(clazz, "onNetworkDeviceConnected",
                                                     "(Ljava/lang/String;Ljava/lang/String;I)V");
    method_onNetworkDeviceDisconnected = env->GetMethodID(clazz, "onNetworkDeviceDisconnected",
                                                              "(Ljava/lang/String;)V");

    method_onCreatePairedDeviceResult = env->GetMethodID(clazz, "onCreatePairedDeviceResult",
                                                         "(Ljava/lang/String;I)V");
    method_onCreateDeviceResult = env->GetMethodID(clazz, "onCreateDeviceResult",
                                                         "(Ljava/lang/String;I)V");
    method_onDiscoverServicesResult = env->GetMethodID(clazz, "onDiscoverServicesResult",
                                                         "(Ljava/lang/String;Z)V");

    method_onAgentAuthorize = env->GetMethodID(clazz, "onAgentAuthorize",
                                               "(Ljava/lang/String;Ljava/lang/String;I)V");
    method_onAgentOutOfBandDataAvailable = env->GetMethodID(clazz, "onAgentOutOfBandDataAvailable",
                                               "(Ljava/lang/String;)Z");
    method_onAgentCancel = env->GetMethodID(clazz, "onAgentCancel", "()V");
    method_onRequestPinCode = env->GetMethodID(clazz, "onRequestPinCode",
                                               "(Ljava/lang/String;I)V");
    method_onRequestPasskey = env->GetMethodID(clazz, "onRequestPasskey",
                                               "(Ljava/lang/String;I)V");
    method_onRequestPasskeyConfirmation = env->GetMethodID(clazz, "onRequestPasskeyConfirmation",
                                               "(Ljava/lang/String;II)V");
    method_onRequestPairingConsent = env->GetMethodID(clazz, "onRequestPairingConsent",
                                               "(Ljava/lang/String;I)V");
    method_onDisplayPasskey = env->GetMethodID(clazz, "onDisplayPasskey",
                                               "(Ljava/lang/String;II)V");
    method_onInputDevicePropertyChanged = env->GetMethodID(clazz, "onInputDevicePropertyChanged",
                                               "(Ljava/lang/String;[Ljava/lang/String;)V");
    method_onInputDeviceConnectionResult = env->GetMethodID(clazz, "onInputDeviceConnectionResult",
                                               "(Ljava/lang/String;I)V");
    method_onPanDevicePropertyChanged = env->GetMethodID(clazz, "onPanDevicePropertyChanged",
                                               "(Ljava/lang/String;[Ljava/lang/String;)V");
    method_onPanDeviceConnectionResult = env->GetMethodID(clazz, "onPanDeviceConnectionResult",
                                               "(Ljava/lang/String;I)V");
    method_onHealthDeviceConnectionResult = env->GetMethodID(clazz,
                                                             "onHealthDeviceConnectionResult",
                                                             "(II)V");
    method_onHealthDevicePropertyChanged = env->GetMethodID(clazz, "onHealthDevicePropertyChanged",
                                               "(Ljava/lang/String;[Ljava/lang/String;)V");
    method_onHealthDeviceChannelChanged = env->GetMethodID(clazz, "onHealthDeviceChannelChanged",
                                               "(Ljava/lang/String;Ljava/lang/String;Z)V");
    method_onRequestOobData = env->GetMethodID(clazz, "onRequestOobData",
                                               "(Ljava/lang/String;I)V");

    field_mNativeData = env->GetFieldID(clazz, "mNativeData", "I");
//#endif
}

static void initializeNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    if (NULL == nat) {
        ALOGE("%s: out of memory!", __FUNCTION__);
        return;
    }
    memset(nat, 0, sizeof(native_data_t));

    pthread_mutex_init(&(nat->thread_mutex), NULL);

    env->SetIntField(object, field_mNativeData, (jint)nat);

    {
        DBusError err;
        dbus_error_init(&err);
        dbus_threads_init_default();
        nat->conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
        if (dbus_error_is_set(&err)) {
            ALOGE("%s: Could not get onto the system bus!", __FUNCTION__);
            dbus_error_free(&err);
        }
        dbus_connection_set_exit_on_disconnect(nat->conn, FALSE);
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    int i;
    struct stat buf;

    ALOGI("android_server_BluetoothEventLoop.cpp: initializeNativeDataNative");
    if (NULL == nat) {
        ALOGE("%s: out of memory!", __FUNCTION__);
        return;
    }
    memset(nat, 0, sizeof(native_data_t));
    pthread_mutex_init(&(nat->thread_mutex), NULL);
    env->SetIntField(object, field_mNativeData, (jint)nat);

    nat->gapSock = -1;
    nat->activity = BTMTK_GAP_ACT_NONE;
    btmtk_host_cache_init(nat);
    btmtk_paired_dev_cache_init(nat);
#endif
}

static void cleanupNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat =
            (native_data_t *)env->GetIntField(object, field_mNativeData);

    pthread_mutex_destroy(&(nat->thread_mutex));

    if (nat) {
        free(nat);
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat =
            (native_data_t *)env->GetIntField(object, field_mNativeData);

    pthread_mutex_destroy(&(nat->thread_mutex));
	
    if (nat) {
        free(nat);
    }
#endif
}

#ifdef __BTMTK__
static void addWatch(native_data_t *nat, int newFD, short events) {
    for (int y = 0; y<nat->pollMemberCount; y++) {
        if ((nat->pollData[y].fd == newFD) &&
                (nat->pollData[y].events == events)) {
            ALOGV("[JNI] duplicate add");
            return;
        }
    }
    if (nat->pollMemberCount == nat->pollDataSize) {
        ALOGV("Bluetooth EventLoop poll struct growing");
        struct pollfd *temp = (struct pollfd *)malloc(
                sizeof(struct pollfd) * (nat->pollMemberCount+1));
        if (!temp) {
            return;
        }
        memcpy(temp, nat->pollData, sizeof(struct pollfd) *
                nat->pollMemberCount);
        free(nat->pollData);
        nat->pollData = temp;
        nat->pollDataSize++;
    }
    nat->pollData[nat->pollMemberCount].fd = newFD;
    nat->pollData[nat->pollMemberCount].revents = 0;
    nat->pollData[nat->pollMemberCount].events = events;
    /* nat->watchData[nat->pollMemberCount] = watch; */
    nat->pollMemberCount++;
}

static jboolean setUpEventLoop(native_data_t *nat) {
    ALOGV("[JNI][API] setUpEventLoop");

    ALOGI("[JNI][API] setUpEventLoop(0x%X) : gapSock=%d", nat, nat->gapSock);
    if (nat != NULL && nat->gapSock < 0) {

        nat->gapSock = socket_local_server(BT_SOCK_NAME_EXT_ADP, 
                                                            ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
                                                            SOCK_DGRAM);
        if (nat->gapSock < 0)
        {
            ALOGE("[JNI][ERR] create gap socket failed : %s, errno=%d", strerror(errno), errno);
            return JNI_FALSE;
        }
        /* Add into polling array */
        addWatch(nat, nat->gapSock, POLLIN);
        /* For BlueZ, the return BT address is AdapterPrefix/dev_XX_XX_XX_XX_XX_XX. */
        /* So we use "MTKBT" as our adapter prefix */
        nat->adapter = BTMTK_ANDROID_ADAPTER_PREFIX;
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

static void tearDownEventLoop(native_data_t *nat) {
    ALOGV(__FUNCTION__);
    if (nat != NULL && nat->gapSock >= 0) 
    {
        close(nat->gapSock);
        nat->gapSock = -1;
	 //unlink(BT_SOCK_NAME_EXT_ADP);
    }
}

#define EVENT_LOOP_EXIT 1
#define EVENT_LOOP_ADD  2
#define EVENT_LOOP_REMOVE 3
int btmtk_util_convert_pair_result(btbm_gap_result result)
{
    switch(result)
    {
    case BTBM_ADP_SUCCESS:
        return BOND_RESULT_SUCCESS;
    case BTBM_ADP_FAIL_TIMEOUT:
        return BOND_RESULT_AUTH_TIMEOUT;
    case BTBM_ADP_FAIL_REMOTE_REJECT:
        return BOND_RESULT_AUTH_REJECTED;
    case BTBM_ADP_FAIL_LINK_KEY_DISMISSED:
        return BOND_RESULT_REMOVED;
    default:
        return BOND_RESULT_AUTH_FAILED;
    }
}

btmtk_sdp_req_struct *btmtk_util_find_sdp_request(native_data_t *nat, bt_addr_struct *addr)
{
    btmtk_sdp_req_struct *ptr = nat->requests;
    ALOGI("[GAP] btmtk_util_find_sdp_request search 0x%X:0x%X:0x%X", addr->lap, addr->uap, addr->nap);
    while (ptr)
    {
        ALOGI("[GAP] btmtk_util_find_sdp_request found 0x%X:0x%X:0x%X", ptr->addr.lap, ptr->addr.uap, ptr->addr.nap);
        if (btmtk_util_equal_bdaddr(&ptr->addr, addr))
        {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

void btmtk_util_update_adapter_property_discovering(JNIEnv *env, native_data_t *nat, bool is_discovering)
{
    jclass stringClass = NULL;
    jobjectArray str_array = NULL;
    Properties prop;
    property_value value;
    int array_index = 0;

    ALOGI("[GAP] btmtk_util_update_adapter_property_discovering: is_discovering = %d", is_discovering);

    if (is_discovering)
    {
        nat->activity = (btmtk_gap_activity_enum)(nat->activity | BTMTK_GAP_ACT_INQUIRY);
    }
    else
    {
        nat->activity = (btmtk_gap_activity_enum)(nat->activity & ~BTMTK_GAP_ACT_INQUIRY);
    }
    
    stringClass = env->FindClass("java/lang/String");
    str_array = env->NewObjectArray(2, stringClass, NULL);
    if (str_array != NULL) 
    {
        prop.type = DBUS_TYPE_BOOLEAN;
        strcpy( prop.name, "Discovering");
        value.int_val = (is_discovering) ? 1 : 0;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, method_onPropertyChanged, str_array);
    }
    else
    {
        ALOGE("NewObjectArray failed");
    }
}

void btmtk_util_update_adapter_property_device(JNIEnv *env, native_data_t *nat)
{
    jclass stringClass = NULL;
    jobjectArray str_array = NULL;
    Properties prop;
    property_value value;
    int array_index = 0, i;

    stringClass = env->FindClass("java/lang/String");
    str_array = env->NewObjectArray(2 + nat->paired_cache_no, stringClass, NULL);
    if (str_array != NULL) 
    {
        prop.type = DBUS_TYPE_ARRAY;
        strcpy(prop.name, "Devices");

        if (nat->paired_cache_no)
        {
            value.array_val = (char **)calloc(4, nat->paired_cache_no);
            for (i = 0; i < nat->paired_cache_no; i++)
            {
                value.array_val[i] = (char*)malloc(BTMTK_MAX_OBJECT_PATH_SIZE);
                if(value.array_val[i])
                {
                    btmtk_util_convert_bdaddr2objpath(value.array_val[i], &nat->paired_dev_cache[i].addr);
                    ALOGI("[GAP] Set Devices: %s", value.array_val[i]);  
                }
            }
        }
        create_prop_array(env, str_array, &prop, &value, nat->paired_cache_no, &array_index);
        if (nat->paired_cache_no)
        {
            for (i = 0; i < nat->paired_cache_no; i++)
            {
                free(value.array_val[i]);
            }
            free(value.array_val);
        }

        env->CallVoidMethod(nat->me, method_onPropertyChanged, str_array);
    }
}

void btmtk_util_update_device_property_trusted(JNIEnv *env, native_data_t *nat, bt_addr_struct *addr, bool trusted)
{
    jclass stringClass = NULL;
    jobjectArray str_array = NULL;
    Properties prop;
    property_value value;
    int array_index = 0;
    btmtk_device_entry_struct *i_entry, *p_entry;

    ALOGI("[GAP] btmtk_util_update_device_property_trusted: trusted = %d", trusted);
    if ((p_entry = btmtk_paired_dev_cache_find(nat, addr)) != NULL)
    {
        p_entry->trusted = trusted;
        btmtk_paired_dev_cache_write(nat);
    }
    if ((i_entry = btmtk_inquired_dev_cache_find(nat, addr)) != NULL)
    {
        i_entry->trusted = trusted;
    }

    stringClass = env->FindClass("java/lang/String");
    str_array = env->NewObjectArray(2, stringClass, NULL);
    if (str_array != NULL) 
    {
        char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
        btmtk_util_convert_bdaddr2objpath(object_path, addr);
        prop.type = DBUS_TYPE_BOOLEAN;
        strcpy(prop.name, "Trusted");
        value.int_val = (trusted) ? 1 : 0;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, 
                            method_onDevicePropertyChanged, 
                            env->NewStringUTF(object_path),
                            str_array);
    }
    else
    {
        ALOGE("NewObjectArray failed");
    }
}

void btmtk_util_update_device_property_paired(JNIEnv *env, native_data_t *nat, bt_addr_struct *addr, btmtk_bond_state paired)
{
    jclass stringClass = NULL;
    jobjectArray str_array = NULL;
    Properties prop;
    property_value value;
    int array_index = 0;
    btmtk_device_entry_struct *i_entry, *p_entry;

    ALOGI("[GAP] btmtk_util_update_device_property_paired: paired = %d", paired);
    if ((p_entry = btmtk_paired_dev_cache_find(nat, addr)) != NULL)
    {
        p_entry->paired = paired;
        btmtk_paired_dev_cache_write(nat);
    }
    if ((i_entry = btmtk_inquired_dev_cache_find(nat, addr)) != NULL)
    {
        i_entry->paired = paired;
    }

    stringClass = env->FindClass("java/lang/String");
    str_array = env->NewObjectArray(2, stringClass, NULL);
    if (str_array != NULL && (paired == BTMTK_BOND_STATE_UNBOND || paired == BTMTK_BOND_STATE_BONDED) ) 
    {
        char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
        btmtk_util_convert_bdaddr2objpath(object_path, addr);
        prop.type = DBUS_TYPE_BOOLEAN;
        strcpy(prop.name, "Paired");
        value.int_val = (paired == BTMTK_BOND_STATE_BONDED) ? 1 : 0;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, 
                            method_onDevicePropertyChanged, 
                            env->NewStringUTF(object_path),
                            str_array);
		//Set alies name as NULL
		memset(&prop, 0x0, sizeof(Properties));
		prop.type = DBUS_TYPE_STRING;
		strcpy(prop.name, "Alias");
		value.str_val = NULL;
		array_index = 0;
		create_prop_array(env, str_array, &prop, &value, 0, &array_index);
		env->CallVoidMethod(nat->me, 
                            method_onDevicePropertyChanged, 
                            env->NewStringUTF(object_path),
                            str_array);
    }
    else
    {
        ALOGE("NewObjectArray failed");
    }
}

void btmtk_util_update_device_property_name(JNIEnv *env, native_data_t *nat, bt_addr_struct *addr, char *devicename, int namelen)
{
    jclass stringClass = NULL;
    jobjectArray str_array = NULL;
    Properties prop;
    property_value value;
    int array_index = 0;
    btmtk_device_entry_struct *entry[2] = {NULL, NULL};
	char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
	int len = (namelen >= BTBM_ADP_MAX_NAME_LEN) ? BTBM_ADP_MAX_NAME_LEN : namelen;
	char name[BTBM_ADP_MAX_NAME_LEN] = {'\0'};
	int index = 0;
	BOOL shouldUpdate = FALSE;
	int endPos = 0;

    ALOGI("[GAP] btmtk_util_update_device_property_name: name = %s", devicename);
	if (devicename == NULL || namelen == 0)
	{
		return;
	}
	strncpy(name, devicename, len);
	if(len == BTBM_ADP_MAX_NAME_LEN)
	{
		endPos = btmtk_util_utf8_endpos(&name[BTBM_ADP_MAX_NAME_LEN-7], 7);
		name[BTBM_ADP_MAX_NAME_LEN-endPos] = {'\0'};
		ALOGI("[GAP] need cut string pos:[%d]\n", endPos);
	}
	
	entry[0] = btmtk_inquired_dev_cache_find(nat, addr);
	entry[1] = btmtk_paired_dev_cache_find(nat, addr);

	for (index = 0; index < 2; index ++)
	{
		if (entry[index] == NULL || (strncmp(entry[index]->name, name, BTBM_ADP_MAX_NAME_LEN) == 0))
		{
			continue;
		}
		shouldUpdate = TRUE;
		strcpy(entry[index]->name, name);

		if (entry[index]->paired == BTMTK_BOND_STATE_BONDED) 
		{
			btmtk_paired_dev_cache_write(nat);
		}
		
	}
	if (!shouldUpdate) 
	{
		return;
	}
	
	stringClass = env->FindClass("java/lang/String");
	str_array = env->NewObjectArray(2, stringClass, NULL); 
	prop.type = 's';
	strcpy(prop.name, "Name");
	value.str_val = name;
	create_prop_array(env, str_array, &prop, &value, 0, &array_index);
	btmtk_util_convert_bdaddr2objpath(object_path, addr);
	env->CallVoidMethod(nat->me, 
                        method_onDevicePropertyChanged, 
                        env->NewStringUTF(object_path), 
                        str_array);
}


btmtk_device_entry_struct *btmtk_inquired_dev_cache_add(native_data_t *nat, btmtk_device_entry_struct *entry)
{
    int i;
    btmtk_device_entry_struct *ptr = NULL;
	if (entry == NULL)
	{
		ALOGE("[GAP] btmtk_inquired_dev_cache_add: entry is null ");
		return NULL;
	}

    ALOGI("[GAP] btmtk_inquired_dev_cache_add: addr=0x%X:0x%X:0x%X", entry->addr.lap, entry->addr.uap, entry->addr.nap);

    for (i = 0; i < nat->inquired_cache_no; i++)
    {
        if (btmtk_util_equal_bdaddr(&nat->inquired_dev_cache[i].addr, &entry->addr))
        {
            ALOGI("[GAP] btmtk_inquired_dev_cache_add: already paired");
            ptr = &nat->inquired_dev_cache[i];
            break;
        }
    }

    if (!ptr)
    {
        if (nat->inquired_cache_no == BTBM_ADP_MAX_INQUIRY_NO)
        {
            ALOGE("[GAP] btmtk_inquired_dev_cache_add: exceed %d entries", BTBM_ADP_MAX_INQUIRY_NO);
            return NULL;
        }
        ptr = &nat->inquired_dev_cache[nat->inquired_cache_no++];
    }
    ALOGI("[GAP] btmtk_inquired_dev_cache_add: %d device inquired", nat->inquired_cache_no);
    memcpy(ptr, entry, sizeof(btmtk_device_entry_struct));
	return ptr;
}
void btmtk_inquired_dev_cache_reset(native_data_t *nat)
{
    int i;
    btmtk_device_entry_struct inquired_dev[BTBM_ADP_MAX_INQUIRY_NO];
	int count = nat->inquired_cache_no;
	memcpy(inquired_dev, nat->inquired_dev_cache, sizeof(btmtk_device_entry_struct) *BTBM_ADP_MAX_INQUIRY_NO);	

    ALOGI("[GAP] btmtk_inquired_dev_cache_reset");

	//reset orignal cache
	memset(nat->inquired_dev_cache, 0x0, sizeof(btmtk_device_entry_struct) *BTBM_ADP_MAX_INQUIRY_NO);		
	nat->inquired_cache_no = 0;
	
    for (i = 0; i < count; i++)
    {
        if (btmtk_util_find_sdp_request(nat, &inquired_dev[i].addr) != NULL)
        {
            ALOGI("[GAP] the device is in bonding");
            btmtk_inquired_dev_cache_add(nat, &inquired_dev[i]);
        }
    }  
}


void btmtk_host_cache_write(native_data_t *nat)
{
    int fd, len;
    int size = sizeof(btmtk_host_cache_struct);

    ALOGI("[GAP] btmtk_host_cache_write");
    if ((fd = open(BTMTK_HOST_CACHE_PATH, O_WRONLY | O_CREAT, 0666)) < 0)
    {
        ALOGE("[GAP] btmtk_host_cache_write open error %d: %s", errno, strerror(errno));
        return;
    }
	len = write(fd, &nat->host_cache, size);
	if (len < 0 || len != size) 
    {
        ALOGE("[GAP] btmtk_host_cache_write %d byte error %d: %s", len, errno, strerror(errno));
	}
    close(fd);
}


/*****************************************************************************
 * FUNCTION
 *  btmtk_host_cache_init
 * DESCRIPTION
 *  Read all paired device cache data from file
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void btmtk_host_cache_init(native_data_t *nat)
{
    int fd;
    struct stat sts;
    int size = sizeof(btmtk_host_cache_struct);
    
    ALOGI("[GAP] btmtk_host_cache_init");
    memset(&nat->host_cache, 0x0, size);
    if (stat(BTMTK_HOST_CACHE_PATH, &sts) == -1 || 
        sts.st_size != size ||
        (fd = open(BTMTK_HOST_CACHE_PATH, O_RDONLY)) < 0)
    {
#ifdef MTK_BSP_PACKAGE
        unlink(BTMTK_HOST_CACHE_PATH);
        strncpy(nat->host_cache.name, BTMTK_ANDROID_DEFAULT_LOCAL_NAME, BTBM_ADP_MAX_NAME_LEN);
#else
        char value[MAX_VALUE_LEN];
        int result = custom_get_string(MODULE_BLUETOOTH, HOST_NAME, value, BTMTK_ANDROID_DEFAULT_LOCAL_NAME);
        unlink(BTMTK_HOST_CACHE_PATH);
        strncpy(nat->host_cache.name, value, BTBM_ADP_MAX_NAME_LEN);
#endif
        nat->host_cache.scan_mode = BTBM_ADP_P_ON_I_OFF;
        nat->host_cache.scan_mode_timeout = 120;
        btmtk_host_cache_write(nat);
        ALOGE("[GAP] btmtk_host_cache_init open error %d: %s", errno, strerror(errno));
    }
    else
    {
        int read_byte = TEMP_FAILURE_RETRY(read(fd, &nat->host_cache, size)), i;
        if (read_byte < 0) 
        {
            ALOGE("[GAP] btmtk_host_cache_init read error %d: %s", errno, strerror(errno));
        } 
        else if (read_byte != size) 
        {
            ALOGE("[GAP] btmtk_host_cache_init read error(%d bytes read) %d: %s", read_byte, errno, strerror(errno));
        }
        close(fd);
    }
}

void btmtk_paired_dev_cache_write(native_data_t *nat)
{
    int fd, len;
    int size = BTBM_ADP_MAX_PAIRED_LIST_NO * sizeof(btmtk_device_entry_struct);

    ALOGI("[GAP] btmtk_paired_dev_cache_write");
    if ((fd = open(BTMTK_DEV_CACHE_PATH, O_WRONLY | O_CREAT, 0666)) < 0)
    {
        ALOGE("[GAP] btmtk_paired_dev_cache_write open error %d: %s", errno, strerror(errno));
        return;
    }
	len = write(fd, &nat->paired_dev_cache, size);
	if (len < 0 || len != size) 
    {
        ALOGE("[GAP] btmtk_paired_dev_cache_write %d byte error %d: %s", len, errno, strerror(errno));
	}
    close(fd);
}

void btmtk_paired_dev_cache_del(native_data_t *nat, bt_addr_struct *addr)
{
    int i, idx = -1;

    ALOGI("[GAP] btmtk_paired_dev_cache_del: addr=0x%X:0x%X:0x%X", addr->lap, addr->uap, addr->nap);
    for (i = 0; i < nat->paired_cache_no; i++)
    {
        if (btmtk_util_equal_bdaddr(&nat->paired_dev_cache[i].addr, addr))
        {
            ALOGI("[GAP] btmtk_paired_dev_cache_del: found");
            idx = i;
            break;
        }
    }

    /* pack */
    if (idx != -1)
    {
        memset(&nat->paired_dev_cache[idx], 0x0, sizeof(btmtk_device_entry_struct));
        for (i = idx + 1; i < nat->paired_cache_no; i++)
        {
            memcpy(&nat->paired_dev_cache[i - 1], &nat->paired_dev_cache[i], sizeof(btmtk_device_entry_struct));
        }
        memset(&nat->paired_dev_cache[nat->paired_cache_no - 1], 0x0, sizeof(btmtk_device_entry_struct));
        nat->paired_cache_no--;
    }
    ALOGI("[GAP] btmtk_paired_dev_cache_del: %d device paired", nat->paired_cache_no);
    btmtk_paired_dev_cache_write(nat);
}

void btmtk_paired_dev_cache_add(native_data_t *nat, btmtk_device_entry_struct *entry)
{
    int i;
    btmtk_device_entry_struct *ptr = NULL;

    ALOGI("[GAP] btmtk_paired_dev_cache_add: addr=0x%X:0x%X:0x%X", entry->addr.lap, entry->addr.uap, entry->addr.nap);
    entry->paired = BTMTK_BOND_STATE_BONDED;
    for (i = 0; i < nat->paired_cache_no; i++)
    {
        if (btmtk_util_equal_bdaddr(&nat->paired_dev_cache[i].addr, &entry->addr))
        {
            ALOGI("[GAP] btmtk_paired_dev_cache_add: already paired");
            ptr = &nat->paired_dev_cache[i];
            break;
        }
    }
    if (!ptr)
    {
        if (nat->paired_cache_no == BTBM_ADP_MAX_PAIRED_LIST_NO)
        {
            ALOGE("[GAP] btmtk_paired_dev_cache_add: exceed %d entries", BTBM_ADP_MAX_PAIRED_LIST_NO);
            return;
        }
        ptr = &nat->paired_dev_cache[nat->paired_cache_no++];
    }
    ALOGI("[GAP] btmtk_paired_dev_cache_add: %d device paired", nat->paired_cache_no);
    memcpy(ptr, entry, sizeof(btmtk_device_entry_struct));
    btmtk_paired_dev_cache_write(nat);
}

/*****************************************************************************
 * FUNCTION
 *  btmtk_paired_dev_cache_init
 * DESCRIPTION
 *  Read all paired device cache data from file
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void btmtk_paired_dev_cache_init(native_data_t *nat)
{
    int fd;
    struct stat sts;
    int size = BTBM_ADP_MAX_PAIRED_LIST_NO * sizeof(btmtk_device_entry_struct);
    
    ALOGI("[GAP] btmtk_paired_dev_cache_init");
    memset(nat->paired_dev_cache, 0x0, size);
    if (stat(BTMTK_DEV_CACHE_PATH, &sts) == -1 || 
        sts.st_size != size ||
        (fd = open(BTMTK_DEV_CACHE_PATH, O_RDONLY)) < 0)
    {
        unlink(BTMTK_DEV_CACHE_PATH);
        ALOGE("[GAP] btmtk_paired_dev_cache_init open error %d: %s", errno, strerror(errno));
    }
    else
    {
        int read_byte = TEMP_FAILURE_RETRY(read(fd, nat->paired_dev_cache, size)), i;
        if (read_byte < 0) 
        {
            ALOGE("[GAP] btmtk_paired_dev_cache_init read error %d: %s", errno, strerror(errno));
        } 
        else if (read_byte != size) 
        {
            ALOGE("[GAP] btmtk_paired_dev_cache_init read error(%d bytes read) %d: %s", read_byte, errno, strerror(errno));
        }
        close(fd);

        for (i = 0; i < BTBM_ADP_MAX_PAIRED_LIST_NO; i++)
        {
            if (nat->paired_dev_cache[i].addr.lap == 0 && 
                nat->paired_dev_cache[i].addr.uap == 0 && 
                nat->paired_dev_cache[i].addr.nap == 0)
            {
                break;
            }
        }
        nat->paired_cache_no = i;
    }
}

static btmtk_device_entry_struct * btmtk_gap_handle_pair_ind(native_data_t *nat, JNIEnv *env, bt_addr_struct *bd_addr, kal_uint32 cod, kal_uint8 name_len, char *name)
{
    jobjectArray str_array = NULL;
    jclass stringClass = NULL;
    Properties prop;
    property_value value;
    int array_index = 0;
    char buf[32] = {'\0'};
    btmtk_device_entry_struct inquired_entry;
	btmtk_device_entry_struct * entry = NULL;
    int length;
	int endPos =0;

    ALOGI("[GAP] remote initiate pairing");

    /* Add to inquired cache so that following handling consistent with bonding */
    memset(&inquired_entry, 0x0, sizeof(btmtk_device_entry_struct));
    memcpy(&inquired_entry.addr, bd_addr, sizeof(bt_addr_struct));
    if (name_len > 0 && name != NULL)
    {
    	length = (strlen(name) < BTBM_ADP_MAX_NAME_LEN ) ? strlen(name) : BTBM_ADP_MAX_NAME_LEN;	
    	strncpy(inquired_entry.name, name, length);    	
		if(length == BTBM_ADP_MAX_NAME_LEN)
		{
			endPos = btmtk_util_utf8_endpos(&inquired_entry.name[BTBM_ADP_MAX_NAME_LEN-7], 7);
			inquired_entry.name[BTBM_ADP_MAX_NAME_LEN-endPos] = {'\0'};
			ALOGI("[GAP] need cut string pos:[%d]\n", endPos);
		}
    }
    inquired_entry.cod = cod;
    inquired_entry.legacy_pairing = true;
    entry = btmtk_inquired_dev_cache_add(nat, &inquired_entry);

    /* onDeviceFound handler */
    stringClass = env->FindClass("java/lang/String");
    str_array = env->NewObjectArray(4, stringClass, NULL); /* only need Class and Name */
    /* Class */
    prop.type = 'u';
    strcpy( prop.name, "Class");
    value.int_val = cod;
    create_prop_array(env, str_array, &prop, &value, 0, &array_index);
    /* Name */
    prop.type = 's';
    strcpy( prop.name, "Name");
    value.str_val = inquired_entry.name;
    create_prop_array(env, str_array, &prop, &value, 0, &array_index);
    btmtk_util_convert_bdaddr2string(buf, bd_addr);

    if (str_array != NULL) {
        env->CallVoidMethod(nat->me,
                            method_onDeviceFound,
                            env->NewStringUTF(buf),
                            str_array);
    }
	return entry;
}

static void btmtk_gap_handle_link_state_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    jobjectArray str_array = NULL;
    jclass stringClass = NULL;

    Properties prop;
    property_value value;
    int array_index = 0;
    char remote_device_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_link_state_ind_struct *ind_p = (bt_bm_link_state_ind_struct*)ilm->ilm_data;;

    btmtk_util_convert_bdaddr2objpath(remote_device_path, &ind_p->bd_addr);
    ALOGI("[GAP] MSG_ID_BT_BM_LINK_STATE_IND linkno=%d addr=%s", ind_p->current_number, remote_device_path);
    if (nat->conn_no != ind_p->current_number)
    {
        btmtk_device_entry_struct *p_entry = btmtk_paired_dev_cache_find(nat, &ind_p->bd_addr);
        btmtk_device_entry_struct *i_entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr);

        stringClass = env->FindClass("java/lang/String");
        str_array = env->NewObjectArray(2, stringClass, NULL);
        prop.type = 'b';
        strcpy(prop.name, "Connected");
        if (nat->conn_no > ind_p->current_number)
        {
            value.int_val = 0;
            nat->conn_no--;
        }
        else
        {
            value.int_val = 1;
            nat->conn_no++;
        }

        if (p_entry)
        {
            p_entry->connected = value.int_val;
        }
        if (i_entry)
        {
            i_entry->connected = value.int_val;
        }
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, 
                            method_onDevicePropertyChanged, 
                            env->NewStringUTF(remote_device_path), 
                            str_array);
    }
}

static void btmtk_gap_handle_discovery_result_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    jobjectArray str_array = NULL;
    jclass stringClass = NULL;
    int str_array_len = 0;

    Properties prop;
    property_value value;
    char buf[32] = {'\0'};
    int array_index = 0;
    char *name = NULL;
    bt_bm_discovery_result_ind_struct *ind_p = (bt_bm_discovery_result_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct entry, *paired_entry, *inquiry_entry  = NULL;
        
    ALOGI("[GAP] MSG_ID_BT_BM_DISCOVERY_RESULT_IND addr=0x%X:0x%X:0x%X", ind_p->bd_addr.lap, ind_p->bd_addr.uap, ind_p->bd_addr.nap);

    /* Write to cache */
	memset(&value, 0x0, sizeof(property_value));
    memset(&entry, 0x0, sizeof(btmtk_device_entry_struct));
    memcpy(&entry.addr, &ind_p->bd_addr, sizeof(bt_addr_struct));
    if (ind_p->name_len != 0)
    {       
		
        name = (char *)ind_p->name;
   		strncpy(entry.name, name, BTBM_ADP_MAX_NAME_LEN);
    	if (strlen(name) >= BTBM_ADP_MAX_NAME_LEN)
    	{
			entry.name[BTBM_ADP_MAX_NAME_LEN-1]='\0';
    	}
    }
    entry.cod = ind_p->cod;
    if (ind_p->supported_servlist)
    {
        int i, idx = 0;
        for (i = 0; i < BTBM_ADP_MAX_SDAP_UUID_NO && idx < 32 * 6; idx++)
        {
            if (idx < 32)
            {
                if ((ind_p->service_list1 >> idx) & 0x1)
                {
                    entry.sdp.uuid[i] = 0x1100 | idx;
                    ALOGI("[GAP] entry.uuid[%d] = 0x%x", i, entry.sdp.uuid[i]);
                    i++;
                }
            }
            else if (idx < 64)
            {
                int bit = idx - 32;
                if ((ind_p->service_list2 >> bit) & 0x1)
                {
                    entry.sdp.uuid[i] = 0x1120 | bit;
                    ALOGI("[GAP] entry.uuid[%d] = 0x%x", i, entry.sdp.uuid[i]);
                    i++;
                }
            }
            else if (idx < 96)
            {
                int bit = idx - 64;
                if ((ind_p->service_list3 >> bit) & 0x1)
                {
                    entry.sdp.uuid[i] = 0x1200 | bit;
                    ALOGI("[GAP] entry.uuid[%d] = 0x%x", i, entry.sdp.uuid[i]);
                    i++;
                }
            }
            else if (idx < 128)
            {
                int bit = idx - 96;
                if ((ind_p->service_list4 >> bit) & 0x1)
                {
                    entry.sdp.uuid[i] = 0x1300 | bit;
                    ALOGI("[GAP] entry.uuid[%d] = 0x%x", i, entry.sdp.uuid[i]);
                    i++;
                }
            }
            else if (idx < 160)
            {
                int bit = idx - 128;
                if ((ind_p->service_list5 >> bit) & 0x1)
                {
                    entry.sdp.uuid[i] = 0x1400 | bit;
                    ALOGI("[GAP] entry.uuid[%d] = 0x%x", i, entry.sdp.uuid[i]);
                    i++;
                }
            }
            else if (idx < 192)
            {
                int bit = idx - 160;
                if ((ind_p->service_list6 >> bit) & 0x1)
                {
                    entry.sdp.uuid[i] = 0x1800 | bit;
                    ALOGI("[GAP] entry.uuid[%d] = 0x%x", i, entry.sdp.uuid[i]);
                    i++;
                }
            }
        }
        entry.sdp.uuid_no = i;
    }
    if ((paired_entry = btmtk_paired_dev_cache_find(nat, &ind_p->bd_addr)) != NULL)
    {
        entry.paired = BTMTK_BOND_STATE_BONDED;
        entry.trusted = paired_entry->trusted;
    }
    entry.legacy_pairing = (ind_p->supported_eir == false);
    entry.rssi = ind_p->rssi;
    entry.device_type = ind_p->device_type;
    btmtk_inquired_dev_cache_add(nat, &entry);
    btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, name, ind_p->name_len);

    /* onDeviceFound handler */
    stringClass = env->FindClass("java/lang/String");
    str_array_len = 4; //RSSI + Class
    if (NULL != name)
    {
	str_array_len += 2;
    }
#ifndef MTK_BSP_PACKAGE
    if (entry.sdp.uuid_no > 0)
    {
	str_array_len += 2 + entry.sdp.uuid_no;
    }	
#endif
    str_array = env->NewObjectArray(str_array_len, stringClass, NULL); 
    
    /* RSSI */
    prop.type = 'n';
    strcpy( prop.name, "RSSI");
    value.int_val = ind_p->rssi;
    create_prop_array(env, str_array, &prop, &value, 0, &array_index);
    /* Class */
    prop.type = 'u';
    strcpy( prop.name, "Class");
    value.int_val = ind_p->cod;
    create_prop_array(env, str_array, &prop, &value, 0, &array_index);
    /* Name */
    if (NULL != name)
    {
        prop.type = 's';
        strcpy( prop.name, "Name");
	/*if nickname exist, no need to update remote device name.*/
	//if ((paired_entry != NULL) && strlen(paired_entry->nickname) > 0)
	//{
	//	value.str_val = (char*)paired_entry->nickname;
//	} else {
        value.str_val = name;
//	}
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
   }
	value.array_val = NULL;
#ifndef MTK_BSP_PACKAGE
	/*UUIDs*/
	if (entry.sdp.uuid_no)
	{	
		prop.type = (int) 'a';
		strcpy(prop.name, "UUIDs");
		value.array_val = (char **)calloc(4, entry.sdp.uuid_no);
    	for (int i = 0; i < entry.sdp.uuid_no; i++)
    	{
    		value.array_val[i] = (char*)calloc(1, BTMTK_MAX_UUID_STR_SIZE);
			if(value.array_val[i])
			{
				int uuid128[16/4];
				btmtk_util_convert_uuid16_2_uuid128((char*)uuid128, entry.sdp.uuid[i]);
				btmtk_util_convert_uuid128_2_string(value.array_val[i], (char*)uuid128);
				ALOGI("[GAP] btmtk_gap_handle_discovery_result_ind UUID: %s", value.array_val[i]);  
        	}
     	}
    	create_prop_array(env, str_array, &prop, &value, (int)entry.sdp.uuid_no, &array_index);
	}
#endif

    btmtk_util_convert_bdaddr2string(buf, &ind_p->bd_addr);

    if (str_array != NULL) 
	{
        env->CallVoidMethod(nat->me,
                            method_onDeviceFound,
                            env->NewStringUTF(buf),
                            str_array);
    }
	
	if (value.array_val != NULL)
	{
	 	for (int i = 0; i < entry.sdp.uuid_no; i++)
        {
            free(value.array_val[i]);
        }
        free(value.array_val);
	}
}

static void btmtk_gap_handle_discovery_update_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    bt_bm_discovery_update_ind_struct *ind_p = (bt_bm_discovery_update_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct *entry = NULL;
        
    ALOGI("[GAP] MSG_ID_BT_BM_DISCOVERY_UPDATE_IND addr=0x%X:0x%X:0x%X", ind_p->bd_addr.lap, ind_p->bd_addr.uap, ind_p->bd_addr.nap);

	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);

/*    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) != NULL &&
        ind_p->name_len > 0 &&
        strcmp(entry->name, (const char *)ind_p->name) != 0)
    {
        jobjectArray str_array = NULL;
        jclass stringClass = NULL;
        Properties prop;
        property_value value;
        char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
        int array_index = 0;

		strncpy(entry->name, (const char *)ind_p->name, (BTBM_ADP_MAX_NAME_LEN-1));
		entry->name[BTBM_ADP_MAX_NAME_LEN-1] = '\0';
		stringClass = env->FindClass("java/lang/String");
		str_array = env->NewObjectArray(2, stringClass, NULL); 
		prop.type = 's';
		strcpy(prop.name, "Name");
		value.str_val = entry->name;
		create_prop_array(env, str_array, &prop, &value, 0, &array_index);
		btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);
		env->CallVoidMethod(nat->me, 
                        method_onDevicePropertyChanged, 
                        env->NewStringUTF(object_path), 
                        str_array);
    }*/
}



static void btmtk_gap_handle_discovery_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    ALOGI("[GAP] MSG_ID_BT_BM_DISCOVERY_CNF");
    btmtk_util_update_adapter_property_discovering(env, nat, false);
}

static void btmtk_gap_handle_remote_device_name_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
	
	bt_bm_remote_name_ind_struct *ind_p = (bt_bm_remote_name_ind_struct*)ilm->ilm_data;
		   
	ALOGI("[GAP] btmtk_gap_handle_remote_device_name_ind addr=0x%X:0x%X:0x%X", ind_p->bd_addr.lap, ind_p->bd_addr.uap, ind_p->bd_addr.nap);
	
	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);
}


static void btmtk_gap_handle_delete_trust_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_delete_trust_cnf_struct *cnf_p = (bt_bm_delete_trust_cnf_struct*)ilm->ilm_data;

    btmtk_util_convert_bdaddr2objpath(object_path, &cnf_p->bd_addr);
    ALOGI("[GAP] MSG_ID_BT_BM_DELETE_TRUST_CNF result=%d, addr=%s", cnf_p->result, object_path);
    if(cnf_p->result == BTBM_ADP_SUCCESS)
  	{
  	    /* Update cache */
  	    btmtk_device_entry_struct *inquired_entry = btmtk_inquired_dev_cache_find(nat, &cnf_p->bd_addr);
	btmtk_device_entry_struct *paired_entry = btmtk_paired_dev_cache_find(nat, &cnf_p->bd_addr);;
		
        if (inquired_entry)
        {
            inquired_entry->paired = BTMTK_BOND_STATE_UNBOND;
            inquired_entry->trusted = false;
        }
	else if (paired_entry) 
	{
	    paired_entry->paired = BTMTK_BOND_STATE_UNBOND;
            paired_entry->trusted = false;
			btmtk_inquired_dev_cache_add(nat, paired_entry);
	}
		
  	btmtk_paired_dev_cache_del(nat, &cnf_p->bd_addr);

        /* Update adapter property */
        btmtk_util_update_adapter_property_device(env, nat);
        /* Update device property */
        btmtk_util_update_device_property_paired(env, nat, &cnf_p->bd_addr, BTMTK_BOND_STATE_UNBOND);
        /* Handler */
        env->CallVoidMethod(nat->me,
                            method_onDeviceRemoved,
                            env->NewStringUTF(object_path));
    }
}

static void btmtk_gap_handle_pin_code_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_pin_code_ind_struct *ind_p = (bt_bm_pin_code_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct *entry;

    btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);
    ALOGI("[GAP] MSG_ID_BT_BM_PIN_CODE_IND addr=%s", object_path);

    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) == NULL)
    {
        btmtk_gap_handle_pair_ind(nat, env, &ind_p->bd_addr, ind_p->cod, ind_p->name_len, (char *)ind_p->name);
    }
	
    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) != NULL)
    {
		entry->paired = BTMTK_BOND_STATE_BONDING;
	}

	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);

    env->CallVoidMethod(nat->me, 
                        method_onRequestPinCode, 
                        env->NewStringUTF(object_path), 
                        BTMTK_PAIR_TYPE_PINCODE);
}

static void btmtk_gap_handle_bonding_result_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char address[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_bonding_result_ind_struct *ind_p = (bt_bm_bonding_result_ind_struct*)ilm->ilm_data;
    btmtk_sdp_req_struct *req = btmtk_util_find_sdp_request(nat, &ind_p->bd_addr);
    
    ALOGI("[GAP] MSG_ID_BT_BM_BONDING_RESULT_IND result=%d", ind_p->result);
    btmtk_util_convert_bdaddr2string(address, &ind_p->bd_addr);
    if (req!= NULL && req->type == BTMTK_SDP_CREATE_PAIRED_DEVICE)
    {
        /* Bonding */
		if(ind_p->result == BTBM_ADP_SUCCESS)
    	{
            btmtk_device_entry_struct entry, *inquired_entry;
            memset(&entry, 0x0, sizeof(btmtk_device_entry_struct));
            
            inquired_entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr);
            if (inquired_entry == NULL)
            {
        	memcpy(&entry.addr, &ind_p->bd_addr, sizeof(bt_addr_struct));				
    	    }
	    else
            {
	        inquired_entry->paired = BTMTK_BOND_STATE_BONDED;			
                memcpy(&entry, inquired_entry, sizeof(btmtk_device_entry_struct));
            }
            entry.cod = ind_p->cod;
	    entry.device_type = ind_p->device_type;
            btmtk_paired_dev_cache_add(nat, &entry);

            /* wait for SDP result */
            ALOGI("[GAP] btmtk_gap_handle_bonding_result_ind wait for SDP result");
            entry.sdp.uuid_no = 0;
            memset(entry.sdp.uuid, 0x0, BTBM_ADP_MAX_SDAP_UUID_NO * sizeof(unsigned short));
            return;
        }
	else
	{
	    btmtk_device_entry_struct *entry = NULL;
            
            entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr);
	    if (entry != NULL)
	    {
		entry->paired = BTMTK_BOND_STATE_UNBOND;
	    }
	}
    }
    else
    {
        /* Pairing */
        if(ind_p->result == BTBM_ADP_SUCCESS)
        {
            btmtk_device_entry_struct entry, *inquired_entry;
            btmtk_sdp_req_struct *req;
            
            inquired_entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr);
            memset(&entry, 0x0, sizeof(btmtk_device_entry_struct));
            if (inquired_entry)
            {
            	inquired_entry->paired = BTMTK_BOND_STATE_BONDED;
                memcpy(&entry, inquired_entry, sizeof(btmtk_device_entry_struct));
                inquired_entry->device_type = ind_p->device_type;
            }
            else
            {
                ALOGE("[GAP] cannot find addr=0x%X:0x%X:0x%X in inquired cache", ind_p->bd_addr.lap, ind_p->bd_addr.uap, ind_p->bd_addr.nap);
				memcpy(&entry.addr, &ind_p->bd_addr, sizeof(bt_addr_struct));
		inquired_entry = btmtk_gap_handle_pair_ind(nat, env, &ind_p->bd_addr, ind_p->cod, 0, NULL);
		if (inquired_entry)
		{
		    inquired_entry->device_type = ind_p->device_type;
		    inquired_entry->paired = BTMTK_BOND_STATE_BONDED;
		}
	    }
            entry.device_type = ind_p->device_type;
            entry.cod = ind_p->cod;
            btmtk_paired_dev_cache_add(nat, &entry);

            if (entry.device_type != BTBM_DEVICE_TYPE_LE)
            {
                req = (btmtk_sdp_req_struct *) calloc(1, sizeof(btmtk_sdp_req_struct));
                req->type = BTMTK_SDP_CREATE_PAIRED_DEVICE;
                memcpy(&req->addr, &ind_p->bd_addr, sizeof(bt_addr_struct));
                btmtk_util_list_append((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)req);
                btmtk_gap_service_search_request(nat->service_nat, &ind_p->bd_addr);
            }
        //    else 
            {
                char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
                btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);
                env->CallVoidMethod(nat->me,
                                    method_onDeviceCreated,
                                    env->NewStringUTF(object_path));
                env->CallVoidMethod(nat->me,
                                    method_onCreatePairedDeviceResult,
                                    env->NewStringUTF(address),
                                    BOND_RESULT_SUCCESS);
            }
        }
        else
        {
            char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
	    btmtk_device_entry_struct *inquired_entry = NULL; 
            btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);
	    btmtk_device_entry_struct *paired_entry = btmtk_paired_dev_cache_find(nat, &ind_p->bd_addr);
		
            if (paired_entry) 
	    {
		paired_entry->paired = BTMTK_BOND_STATE_UNBOND;
            	paired_entry->trusted = false;
		btmtk_inquired_dev_cache_add(nat, paired_entry);
	    }
	    inquired_entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr);
	    if (inquired_entry)
        	{
          		inquired_entry->paired = BTMTK_BOND_STATE_UNBOND;
        	}
			
            btmtk_paired_dev_cache_del(nat, &ind_p->bd_addr);
            
            env->CallVoidMethod(nat->me,
                                method_onCreatePairedDeviceResult,
                                env->NewStringUTF(address),
                                btmtk_util_convert_pair_result((btbm_gap_result)ind_p->result));

            env->CallVoidMethod(nat->me,
                                method_onDeviceRemoved,
                                env->NewStringUTF(object_path));
        }
    }
}

static void btmtk_gap_handle_bonding_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char address[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_bonding_cnf_struct *cnf_p = (bt_bm_bonding_cnf_struct*)ilm->ilm_data;

    ALOGI("[GAP] MSG_ID_BT_BM_BONDING_CNF result=%d", cnf_p->result);
    btmtk_util_convert_bdaddr2string(address, &cnf_p->bd_addr);
    if(cnf_p->result == BTBM_ADP_SUCCESS)
    {
    }
    else
    {
        btmtk_sdp_req_struct *req = btmtk_util_find_sdp_request(nat, &cnf_p->bd_addr);
        
        ALOGI("[GAP] Bonding failed");
        if (req!= NULL && req->type == BTMTK_SDP_CREATE_PAIRED_DEVICE)
        {
            btmtk_util_list_remove((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)req);
        }
    }

    env->CallVoidMethod(nat->me,
                        method_onCreatePairedDeviceResult,
                        env->NewStringUTF(address),
                        btmtk_util_convert_pair_result((btbm_gap_result)cnf_p->result));

    if (cnf_p->result != BTBM_ADP_SUCCESS)
    {
        char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
        btmtk_util_convert_bdaddr2objpath(object_path, &cnf_p->bd_addr);
        /* Handler */
        env->CallVoidMethod(nat->me,
                            method_onDeviceRemoved,
                            env->NewStringUTF(object_path));
    }
}

static void btmtk_gap_handle_security_user_confirm_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    uint32_t number;
    bt_bm_security_user_confirm_ind_struct *ind_p = (bt_bm_security_user_confirm_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct *entry;
	kal_uint8 numeric[7];

	memset(numeric, 0x0, 7);
	memcpy(numeric, ind_p->numeric, 6);
    number = atoi((const char*)numeric);
    ALOGI("[GAP] MSG_ID_BT_BM_SECURITY_USER_CONFIRM_IND : number=%d, display=%d", number, ind_p->display_numeric);
    btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);

    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) == NULL)
    {
        btmtk_gap_handle_pair_ind(nat, env, &ind_p->bd_addr, ind_p->cod, ind_p->name_len, (char *)ind_p->name);
    }

    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) != NULL)
    {
		entry->paired = BTMTK_BOND_STATE_BONDING;
	}	

	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);

    if (ind_p->display_numeric)
    {
        env->CallVoidMethod(nat->me, method_onRequestPasskeyConfirmation,
                                       env->NewStringUTF(object_path),
                                       number,
                                       BTMTK_PAIR_TYPE_USER_CONFIRM);
    }
    else
    {
        env->CallVoidMethod(nat->me, method_onRequestPairingConsent,
                                       env->NewStringUTF(object_path),
                                       BTMTK_PAIR_TYPE_JUST_WORK);
    }
}

static void btmtk_gap_handle_security_passkey_entry_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_security_passkey_entry_ind_struct *ind_p = (bt_bm_security_passkey_entry_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct *entry;

    ALOGI("[GAP] MSG_ID_BT_BM_SECURITY_PASSKEY_ENTRY_IND");
    btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);

    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) == NULL)
    {
        btmtk_gap_handle_pair_ind(nat, env, &ind_p->bd_addr, 0, ind_p->name_len, (char *)ind_p->name);
    }

	if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) != NULL)
    {
		entry->paired = BTMTK_BOND_STATE_BONDING;
	}

	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);

    env->CallVoidMethod(nat->me, method_onRequestPasskey,
                                   env->NewStringUTF(object_path),
                                   BTMTK_PAIR_TYPE_PASSKEY_INPUT);
}

static void btmtk_gap_handle_security_passkey_notify_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_security_passkey_notify_ind_struct *ind_p = (bt_bm_security_passkey_notify_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct *entry;

    ALOGI("[GAP] MSG_ID_BT_BM_SECURITY_PASSKEY_NOTIFY_IND : passkey=%d", ind_p->passkey);
    btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->bd_addr);

    if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) == NULL)
    {
        btmtk_gap_handle_pair_ind(nat, env, &ind_p->bd_addr, ind_p->cod, ind_p->name_len, (char *)ind_p->name);
    }

	if ((entry = btmtk_inquired_dev_cache_find(nat, &ind_p->bd_addr)) != NULL)
    {
		entry->paired = BTMTK_BOND_STATE_BONDING;
	}

	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);

    env->CallVoidMethod(nat->me, method_onDisplayPasskey,
                                   env->NewStringUTF(object_path),
                                   ind_p->passkey,
                                   BTMTK_PAIR_TYPE_PASSKEY_DISPLAY);
}

static void btmtk_gap_handle_security_oob_data_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bt_bm_security_oob_data_ind_struct *ind_p = (bt_bm_security_oob_data_ind_struct*)ilm->ilm_data;
    btmtk_device_entry_struct *entry;
//    char hash[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x0, 0x9, 0x8, 0x7};

    ALOGI("[GAP] MSG_ID_BT_BM_SECURITY_OOB_DATA_IND");

	btmtk_util_update_device_property_name(env, nat, &ind_p->bd_addr, (char *)ind_p->name, ind_p->name_len);

//    btmtk_gap_security_oob_data(nat->service_nat, &ind_p->bd_addr, TRUE, hash, NULL);
    btmtk_gap_security_oob_data(nat->service_nat, &ind_p->bd_addr, TRUE, NULL, NULL);
}

void btmtk_gap_handle_service_search_result_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    bt_bm_service_search_result_ind_struct *ind_p = (bt_bm_service_search_result_ind_struct *)ilm->ilm_data;
    btmtk_sdp_req_struct *req;
    
    ALOGI("[GAP] MSG_ID_BT_BM_SERVICE_SEARCH_RESULT_IND : uuid=0x%X", ind_p->uuid);
    req = btmtk_util_find_sdp_request(nat, &ind_p->bd_addr);
    if (req && req->uuid_no < BTBM_ADP_MAX_SDAP_UUID_NO)
    {
        req->uuid[req->uuid_no++] = (unsigned short)ind_p->uuid;
        ALOGI("[GAP] MSG_ID_BT_BM_SERVICE_SEARCH_RESULT_IND : uuid_no=%d", req->uuid_no);
    }
}

void btmtk_gap_handle_service_search_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    bt_bm_service_search_cnf_struct *cnf_p = (bt_bm_service_search_cnf_struct *)ilm->ilm_data;
    btmtk_sdp_req_struct *req;
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
	char address_buf[32] = {'\0'};
    int i;

    ALOGI("[GAP] MSG_ID_BT_BM_SERVICE_SEARCH_CNF: addr=0x%X:0x%X:0x%X, result=%d, searched=%d", 
         cnf_p->bd_addr.lap, cnf_p->bd_addr.uap, cnf_p->bd_addr.nap, cnf_p->result, cnf_p->searched_number);

    btmtk_util_convert_bdaddr2objpath(object_path, &cnf_p->bd_addr);
 	btmtk_util_convert_bdaddr2string(address_buf, &cnf_p->bd_addr);
    req = btmtk_util_find_sdp_request(nat, &cnf_p->bd_addr);

    if (req == NULL)
    {
        ALOGE("[GAP] btmtk_gap_handle_service_search_cnf no sdp request found");
        return;
    }
    ALOGI("[GAP] btmtk_gap_handle_service_search_cnf sdp req type=%d, uuid_no=%d", req->type, req->uuid_no);
    
    /* write to FS */
    if (cnf_p->result == BTBM_ADP_SUCCESS)
    {
        btmtk_device_entry_struct *p_entry = btmtk_paired_dev_cache_find(nat, &cnf_p->bd_addr);
        btmtk_device_entry_struct *i_entry = btmtk_inquired_dev_cache_find(nat, &cnf_p->bd_addr);
        if (p_entry)
        {
            p_entry->sdp.uuid_no = req->uuid_no;
            memcpy(p_entry->sdp.uuid, req->uuid, BTBM_ADP_MAX_SDAP_UUID_NO * sizeof(unsigned short));
            btmtk_paired_dev_cache_write(nat);
        }
        if (i_entry)
        {
            i_entry->sdp.uuid_no = req->uuid_no;
            memcpy(i_entry->sdp.uuid, req->uuid, BTBM_ADP_MAX_SDAP_UUID_NO * sizeof(unsigned short));
        }
    }

    /* Send device property changed */
    if (req->uuid_no)
    {
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray str_array = env->NewObjectArray(2 + req->uuid_no, stringClass, NULL);
        Properties prop;
        property_value value;
        int array_index = 0;
        
        strcpy(prop.name, "UUIDs");
        prop.type = (int) 'a';
        value.array_val = (char **)calloc(4, req->uuid_no);
        for (i = 0; i < req->uuid_no; i++)
        {
            value.array_val[i] = (char*)calloc(1, BTMTK_MAX_UUID_STR_SIZE);
            if(value.array_val[i])
            {
                int uuid128[16/4];
		if (req->uuid[i] == 0x2001) 
		{
		    memcpy((char*)uuid128, SDP_INSECURE, 16);
		} 
		else if (req->uuid[i] == 0x2002)
		{
		    memcpy((char*)uuid128, SDP_SECURE, 16);
		}  else {
                btmtk_util_convert_uuid16_2_uuid128((char*)uuid128, req->uuid[i]);
		}
                btmtk_util_convert_uuid128_2_string(value.array_val[i], (char*)uuid128);
                ALOGI("[GAP] btmtk_gap_handle_service_search_cnf UUID: %s", value.array_val[i]);  
            }
        }
        create_prop_array(env, str_array, &prop, &value, (int)req->uuid_no, &array_index);
        env->CallVoidMethod(nat->me, 
                            method_onDevicePropertyChanged, 
                            env->NewStringUTF(object_path), 
                            str_array);
        for (i = 0; i < req->uuid_no; i++)
        {
            free(value.array_val[i]);
        }
        free(value.array_val);
    }

    /* Corresponding SDP result handler */
    switch (req->type)
    {
    case BTMTK_SDP_CREATE_DEVICE:
    {
        jint result;

        ALOGI("[GAP] btmtk_gap_handle_service_search_cnf: BTMTK_SDP_CREATE_DEVICE");
        result = (cnf_p->result == BTBM_ADP_SUCCESS) ? CREATE_DEVICE_SUCCESS : CREATE_DEVICE_FAILED;
        env->CallVoidMethod(nat->me,
                            method_onCreateDeviceResult,
                            env->NewStringUTF(address_buf),
                            result);
        break;
    }
    case BTMTK_SDP_CREATE_PAIRED_DEVICE:
    {
        jint result;

        ALOGI("[GAP] btmtk_gap_handle_service_search_cnf: BTMTK_SDP_CREATE_PAIRED_DEVICE");
        /* Update adapter property */
        btmtk_util_update_adapter_property_device(env, nat);
        
        /* Create device handler */
        result = (cnf_p->result == BTBM_ADP_SUCCESS) ? CREATE_DEVICE_SUCCESS : CREATE_DEVICE_FAILED;
        env->CallVoidMethod(nat->me,
                            method_onDeviceCreated,
                            env->NewStringUTF(object_path));
#if 0
        env->CallVoidMethod(nat->me,
                            method_onCreateDeviceResult,
                            env->NewStringUTF(object_path),
                            result);
#endif
		btmtk_device_entry_struct *inquired_entry = btmtk_inquired_dev_cache_find(nat, &cnf_p->bd_addr);
		if (inquired_entry && inquired_entry->paired == BTMTK_BOND_STATE_BONDED)
        {
        	//In CTS case, some reference phone will request pairng twice.
        	//so when user confirmation(or other else) is on-going,
        	// bond state event will not sent to upper layer until bonding is complete
			
        	env->CallVoidMethod(nat->me,
                            method_onCreatePairedDeviceResult,
                            env->NewStringUTF(address_buf),
                            BOND_RESULT_SUCCESS);
		}
        break;
    }
    case BTMTK_SDP_DISCOVERY:
    {
        bool result;
        ALOGI("[GAP] btmtk_gap_handle_service_search_cnf: BTMTK_SDP_DISCOVERY");
        result = (cnf_p->result == BTBM_ADP_SUCCESS) ? JNI_TRUE : JNI_FALSE;
        env->CallVoidMethod(nat->me,
                            method_onDiscoverServicesResult,
                            env->NewStringUTF(object_path),
                            result);
        break;
    }
    default:
        ALOGE("[GAP] btmtk_gap_handle_service_search_result_ind() req->type=%d", req->type);
        break;
    }
    btmtk_util_list_remove((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)req);
}

void btmtk_gap_handle_service_search_raw_result(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    bt_bm_search_raw_result_struct *cnf_p = (bt_bm_search_raw_result_struct *)ilm->ilm_data;
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bool result;
    btmtk_sdp_req_struct *req;

    ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_RAW_RESULT : addr=0x%X:0x%X:0x%X, len=%d", 
        cnf_p->bd_addr.lap, cnf_p->bd_addr.uap, cnf_p->bd_addr.nap, cnf_p->len);

    req = btmtk_util_find_sdp_request(nat, &cnf_p->bd_addr);
    btmtk_util_convert_bdaddr2objpath(object_path, &cnf_p->bd_addr);
    if (req->type == BTMTK_SDP_DISCOVERY && req->app_uuid_no && cnf_p->len == 4)
    {
        btmtk_device_entry_struct *p_entry = btmtk_paired_dev_cache_find(nat, &cnf_p->bd_addr);
        btmtk_device_entry_struct *i_entry = btmtk_inquired_dev_cache_find(nat, &cnf_p->bd_addr);
        ALOGI("[GAP] Service record handle = 0x%2x %2x %2x %2x", cnf_p->data[0], cnf_p->data[1], cnf_p->data[2], cnf_p->data[3]);
        if (p_entry)
        {
            int i;
            for (i = 0; i < p_entry->sdp.app_uuid_no; i++)
            {
                if (memcmp(p_entry->sdp.app_uuid[i], req->app_uuid, BTMTK_SDP_UUID_128_BIT_SIZE) == 0)
                {
                    break;
                }
            }
            if (i >= p_entry->sdp.app_uuid_no)
            {
                ALOGI("[GAP] Add %dth app_uuid to paired cache: result=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 
                    p_entry->sdp.app_uuid_no + 1, req->app_uuid[0], req->app_uuid[1], req->app_uuid[2], req->app_uuid[3],
                    req->app_uuid[4], req->app_uuid[5], req->app_uuid[6], req->app_uuid[7]);

                memcpy(p_entry->sdp.app_uuid[p_entry->sdp.app_uuid_no], req->app_uuid, BTMTK_SDP_UUID_128_BIT_SIZE);
                p_entry->sdp.app_uuid_no++;
                btmtk_paired_dev_cache_write(nat);
            }
        }
        if (i_entry)
        {
            int i;
            for (i = 0; i < i_entry->sdp.app_uuid_no; i++)
            {
                if (memcmp(i_entry->sdp.app_uuid[i], req->app_uuid, BTMTK_SDP_UUID_128_BIT_SIZE) == 0)
                {
                    break;
                }
            }
            if (i >= i_entry->sdp.app_uuid_no)
            {
                ALOGI("[GAP] Add app_uuid to inquiry cache");
                memcpy(i_entry->sdp.app_uuid[i_entry->sdp.app_uuid_no], req->app_uuid, BTMTK_SDP_UUID_128_BIT_SIZE);
                i_entry->sdp.app_uuid_no++;
            }
        }
		if (NULL == i_entry && NULL == p_entry)
		{
			btmtk_device_entry_struct entry;
			memset(&entry, 0x0, sizeof(btmtk_device_entry_struct));
			memcpy(&entry.addr, &cnf_p->bd_addr, sizeof(bt_addr_struct));
			memcpy(entry.sdp.app_uuid[entry.sdp.app_uuid_no], req->app_uuid, BTMTK_SDP_UUID_128_BIT_SIZE);
            entry.sdp.app_uuid_no++;
			entry.device_type = BTBM_DEVICE_TYPE_BR_EDR;
			btmtk_inquired_dev_cache_add(nat, &entry);
		}
    }
}


void btmtk_gap_handle_service_search_raw_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
    bt_bm_search_raw_cnf_struct *cnf_p = (bt_bm_search_raw_cnf_struct *)ilm->ilm_data;
    char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    bool result;
    btmtk_sdp_req_struct *req;

    ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_RAW_CNF : result=%d", cnf_p->result);

    req = btmtk_util_find_sdp_request(nat, &cnf_p->bd_addr);
    btmtk_util_convert_bdaddr2objpath(object_path, &cnf_p->bd_addr);
    result = (cnf_p->result == BTBM_ADP_SUCCESS) ? JNI_TRUE : JNI_FALSE;
    env->CallVoidMethod(nat->me,
                        method_onDiscoverServicesResult,
                        env->NewStringUTF(object_path),
                        result);
    btmtk_util_list_remove((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)req);
}

void btmtk_hdp_handle_connect_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
	bt_hdp_connect_cnf_struct *cnf = (bt_hdp_connect_cnf_struct *)ilm->ilm_data;
    char device_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    char channel_path[BTMTK_MAX_HDP_CHANNEL_PATH_SIZE];
    bool result;

    ALOGI("[GAP] btmtk_hdp_handle_connect_cnf : result=%d", cnf->result);
	btmtk_util_convert_bdaddr2objpath(channel_path, &cnf->bdaddr);

	env->CallVoidMethod(nat->me,
                        method_onHealthDeviceConnectionResult,
                        cnf->index,
                        btmtk_hdp_util_convert_result(cnf->result));
}

void btmtk_hdp_handle_disconnect_cnf(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
	bt_hdp_disconnect_cnf_struct *cnf = (bt_hdp_disconnect_cnf_struct *)ilm->ilm_data;
    char device_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    char channel_path[BTMTK_MAX_HDP_CHANNEL_PATH_SIZE];
    bool result;

    ALOGI("[GAP] btmtk_hdp_handle_disconnect_cnf : result=%d", cnf->result);
	btmtk_util_convert_bdaddr2objpath(device_path, &cnf->bdaddr);

	env->CallVoidMethod(nat->me,
                        method_onHealthDeviceConnectionResult,
                        cnf->index,
                        btmtk_hdp_util_convert_result(cnf->result));
	if (BTBM_ADP_SUCCESS == cnf->result) 
	{
	
    	btmtk_util_convert_bdaddr2objpath(device_path, &cnf->bdaddr);
		if (btmtk_hdp_compose_channel(&cnf->bdaddr, cnf->mdlId, channel_path))
		{

			env->CallVoidMethod(nat->me,
                        method_onHealthDeviceChannelChanged,
                        env->NewStringUTF(device_path),
                        env->NewStringUTF(channel_path),
                        JNI_FALSE);
		}
	}
}

void btmtk_hdp_handle_channel_opened_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
	bt_hdp_channel_opened_ind_struct *ind = (bt_hdp_channel_opened_ind_struct *)ilm->ilm_data;
    char device_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    char channel_path[BTMTK_MAX_HDP_CHANNEL_PATH_SIZE];
	btmtk_hdp_fd_struct * hdp_fd = NULL;
	int socketFd = -1;
    bool result;

    ALOGI("[GAP] btmtk_hdp_handle_channel_opened_ind ");

	btmtk_util_convert_bdaddr2objpath(device_path, &ind->bdaddr);
	result = btmtk_hdp_compose_channel(&ind->bdaddr, ind->mdlId, channel_path);
	if (!result)
	{
		return;
	}
	/*Create socket FD when channale is created, and add the fd to list*/
	hdp_fd = btmtk_hdp_util_find_hdp_fd(nat->service_nat, &ind->bdaddr, ind->mdlId);
	if (NULL == hdp_fd || -1 == hdp_fd->fd) 
	{		
		socketFd = btmtk_hdp_create_socket(ind->l2capId);
		if (socketFd >= 0)
		{
			if (NULL == hdp_fd) 
			{
				btmtk_hdp_add_fd(nat->service_nat, &ind->bdaddr, ind->mdlId, socketFd);    
			}
			else 
			{
				hdp_fd->fd = socketFd;
			}
		}
	} 
	env->CallVoidMethod(nat->me,
                       method_onHealthDeviceChannelChanged,
                       env->NewStringUTF(device_path),
                       env->NewStringUTF(channel_path),
                       JNI_TRUE);
	if (ind->mainChannel == TRUE)
	{
		jobjectArray str_array = NULL;
    	Properties prop;
   		property_value value;
   		int array_index = 0;
   		jclass stringClass = env->FindClass("java/lang/String");
   		str_array = env->NewObjectArray(2, stringClass, NULL);
		if (str_array != NULL) 
		{
			prop.type = DBUS_TYPE_STRING;
   			strcpy( prop.name, "MainChannel");
   			value.str_val = channel_path;
			create_prop_array(env, str_array, &prop, &value, 0, &array_index);
			
			env->CallVoidMethod(nat->me,
                        method_onHealthDevicePropertyChanged,
                        env->NewStringUTF(device_path),
	                      str_array);
    	}
	}
}

void btmtk_hdp_handle_channel_closed_ind(native_data_t *nat, JNIEnv *env, ilm_struct *ilm)
{
	bt_hdp_channel_closed_ind_struct *ind = (bt_hdp_channel_closed_ind_struct *)ilm->ilm_data;
    char device_path[BTMTK_MAX_OBJECT_PATH_SIZE];
    char channel_path[BTMTK_MAX_HDP_CHANNEL_PATH_SIZE];
    bool result;

    ALOGI("[GAP] btmtk_hdp_handle_disconnect_ind");
	btmtk_util_convert_bdaddr2objpath(device_path, &ind->bdaddr);
	if (btmtk_hdp_compose_channel(&ind->bdaddr, ind->mdlId, channel_path))
	{
		env->CallVoidMethod(nat->me,
                        method_onHealthDeviceChannelChanged,
                        env->NewStringUTF(device_path),
                        env->NewStringUTF(channel_path),
                        JNI_FALSE);
	//	pthread_mutex_lock(&(nat->thread_mutex));
		btmtk_hdp_remove_fd(nat->service_nat, &ind->bdaddr, ind->mdlId);
	//	pthread_mutex_unlock(&(nat->thread_mutex));
	}
	else
	{
		ALOGI("[GAP] btmtk_hdp_handle_disconnect_ind: invalid parms");
	}
}



void btmtk_handle_reset_ind(native_data_t *nat, JNIEnv *env, ilm_struct *rec_ilm)
{
    bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_handle_reset_ind");

    jobjectArray str_array = NULL;
    jclass stringClass = NULL;
    Properties prop;
    property_value value;
    int array_index = 0;  
	
    stringClass = env->FindClass("java/lang/String");
    str_array = env->NewObjectArray(2, stringClass, NULL); 

    if (str_array != NULL) 
    {
        prop.type = DBUS_TYPE_BOOLEAN; 
        strcpy(prop.name, "Powered");
        value.int_val = 1;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, method_onPropertyChanged, str_array);
     } 
     else 
     {
	ALOGE("NewObjectArray fail");
     }
		
/*    nat->service_nat->state = BTMTK_POWER_STATE_RESET;

    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_POWEROFF_REQ;
    if (bt_sendmsg(nat->gapSock, (void*)&ilm, 0) &&
        wait_response(nat->gapSock, MSG_ID_BT_POWEROFF_CNF, &ilm, 5000) > 0)
    {
        bt_poweroff_cnf_struct *cnf_p;
        cnf_p = (bt_poweroff_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_POWEROFF_CNF: result=%d", cnf_p->result);
        ret = cnf_p->result;
    }

    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_POWERON_REQ;
    if (bt_sendmsg(nat->gapSock, (void*)&ilm, 0) &&
        wait_response(nat->gapSock, MSG_ID_BT_POWERON_CNF, &ilm, 5000) > 0)
    {
        bt_poweron_cnf_struct *cnf_p;
        cnf_p = (bt_poweron_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_POWERON_CNF: result=%d", cnf_p->result);
        ret = cnf_p->result;
    }

    if (ret)
    {
        const char *name = nat->host_cache.name;
        bt_bm_write_local_name_req_struct *msg_p1;
        bt_bm_write_scanenable_mode_req_struct *msg_p2;
*/
        /* Set name */
  /*      memset(&ilm, 0x0, sizeof(ilm_struct));
        msg_p1 = (bt_bm_write_local_name_req_struct*)ilm.ilm_data;
        ilm.msg_id = MSG_ID_BT_BM_WRITE_LOCAL_NAME_REQ;
        msg_p1->name_len = (strlen(name) > BTBM_ADP_MAX_NAME_LEN) ? BTBM_ADP_MAX_NAME_LEN : strlen(name);
        strncpy((char *)msg_p1->name, name, msg_p1->name_len);
	if (strlen(name) >= BTBM_ADP_MAX_NAME_LEN)
	{
	    msg_p1->name[BTBM_ADP_MAX_NAME_LEN-1] = '\0';
	}
        ALOGI("[GAP] btmtk_gap_set_local_name : name=%s, len=%d", name, msg_p1->name_len);
        if(bt_sendmsg(nat->gapSock, (void*)&ilm, sizeof(bt_bm_write_local_name_req_struct)) &&
           wait_response(nat->gapSock, MSG_ID_BT_BM_WRITE_LOCAL_NAME_CNF, &ilm, 2000) > 0)
        {
            bt_bm_write_local_name_cnf_struct *cnf_p = (bt_bm_write_local_name_cnf_struct*)ilm.ilm_data;
            ALOGI("[GAP] MSG_ID_BT_BM_WRITE_LOCAL_NAME_CNF: result=%d", cnf_p->result);
        }
*/
        /* Set scanenable */
    /*    ALOGI("[GAP] btmtk_gap_set_scanable_mode: mode=0x%X", nat->host_cache.scan_mode);
        memset(&ilm, 0x0, sizeof(ilm_struct));
        msg_p2 = (bt_bm_write_scanenable_mode_req_struct*)ilm.ilm_data;
        ilm.msg_id = MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_REQ;
        msg_p2->modenotconnected = nat->host_cache.scan_mode;
        if(bt_sendmsg(nat->gapSock, (void*)&ilm, sizeof(bt_bm_write_scanenable_mode_req_struct)) && 
           wait_response(nat->gapSock, MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_CNF, &ilm, 2000) > 0)
        {
            bt_bm_write_scanenable_mode_cnf_struct *cnf_p = (bt_bm_write_scanenable_mode_cnf_struct*)ilm.ilm_data;
            ALOGI("[GAP] MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_CNF: result=%d", cnf_p->result);
        }

        nat->service_nat->state = BTMTK_POWER_STATE_ON;
    }
    else
    {
        nat->service_nat->state = BTMTK_POWER_STATE_OFF;
    }*/
}

static bool msg_handler(native_data_t *nat, ilm_struct *ilm) {
    JNIEnv *env;

    ALOGI("[GAP] receive message=%d", ilm->msg_id);
    nat->vm->GetEnv((void**)&env, nat->envVer);
    env->PushLocalFrame(EVENT_LOOP_REFS);
    switch (ilm->msg_id)
    {
    case MSG_ID_BT_RESET_REQ_IND:
    {
        btmtk_handle_reset_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_DISCOVERY_RESULT_IND:
    {
        btmtk_gap_handle_discovery_result_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_DISCOVERY_UPDATE_IND:
    {
        btmtk_gap_handle_discovery_update_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_DISCOVERY_CNF:
    {
        btmtk_gap_handle_discovery_cnf(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_DELETE_TRUST_CNF:
    {
        btmtk_gap_handle_delete_trust_cnf(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_BONDING_CNF:
    {
        btmtk_gap_handle_bonding_cnf(nat, env, ilm);
		break;
    }
    case MSG_ID_BT_BM_PIN_CODE_IND:
    {
        btmtk_gap_handle_pin_code_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_BONDING_RESULT_IND:
    {
        btmtk_gap_handle_bonding_result_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_SECURITY_USER_CONFIRM_IND:
    {
        btmtk_gap_handle_security_user_confirm_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_SECURITY_PASSKEY_ENTRY_IND:
    {
        btmtk_gap_handle_security_passkey_entry_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_SECURITY_PASSKEY_NOTIFY_IND:
    {
        btmtk_gap_handle_security_passkey_notify_ind(nat, env, ilm);
        break;
    }
	case MSG_ID_BT_BM_SECURITY_OOB_DATA_IND:
    {
        btmtk_gap_handle_security_oob_data_ind(nat, env, ilm);
        break;
    }
    case MSG_ID_BT_BM_SECURITY_KEYPRESS_NOTIFY_IND:
        ALOGW("MSG_ID_BT_BM_SECURITY_KEYPRESS_NOTIFY_IND handle not implemented yet");
        break;
    case MSG_ID_BT_POWERON_CNF:
		ALOGI("[GAP] MSG_ID_BT_POWERON_CNF");
		break;
    case MSG_ID_BT_POWEROFF_CNF:
		ALOGI("[GAP] MSG_ID_BT_POWEROFF_CNF");
		break;
    case MSG_ID_BT_BM_DISCOVERY_CANCEL_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_DISCOVERY_CANCEL_CNF");
		break;
    case MSG_ID_BT_BM_WRITE_LOCAL_NAME_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_WRITE_LOCAL_NAME_CNF");
		break;
    case MSG_ID_BT_BM_READ_LOCAL_NAME_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_NAME_CNF");
		break;
    case MSG_ID_BT_BM_READ_REMOTE_NAME_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_REMOTE_NAME_CNF");
		break;
    case MSG_ID_BT_BM_READ_REMOTE_NAME_CANCEL_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_REMOTE_NAME_CANCEL_CNF");
		break;
	case MSG_ID_BT_BM_REMOTE_NAME_IND:
    {
		btmtk_gap_handle_remote_device_name_ind(nat, env, ilm);
		break;
	}
    case MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_CNF");
		break;
    case MSG_ID_BT_BM_READ_SCANENABLE_MODE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_SCANENABLE_MODE_CNF");
		break;
    case MSG_ID_BT_BM_READ_LOCAL_ADDR_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_ADDR_CNF");
		break;
    case MSG_ID_BT_BM_READ_LOCAL_COD_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_COD_CNF");
		break;
    case MSG_ID_BT_BM_WRITE_LOCAL_COD_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_WRITE_LOCAL_COD_CNF");
		break;
    case MSG_ID_BT_BM_READ_PROPERTY_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_READ_PROPERTY_CNF");
		break;
    case MSG_ID_BT_BM_WRITE_AUTHENTICATION_MODE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_WRITE_AUTHENTICATION_MODE_CNF");
		break;
    case MSG_ID_BT_BM_BLOCK_ACTIVE_LINK_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_BLOCK_ACTIVE_LINK_CNF");
		break;
    case MSG_ID_BT_BM_BLOCK_LIST_UPDATE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_BLOCK_LIST_UPDATE_CNF");
		break;
    case MSG_ID_BT_BM_DELETE_TRUST_ALL_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_DELETE_TRUST_ALL_CNF");
		break;
    case MSG_ID_BT_BM_BONDING_CANCEL_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_BONDING_CANCEL_CNF");
		break;
    case MSG_ID_BT_BM_PAIRING_CANCEL_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_PAIRING_CANCEL_CNF");
		break;
    case MSG_ID_BT_BM_SERVICE_SEARCH_RESULT_IND:
        btmtk_gap_handle_service_search_result_ind(nat, env, ilm);
		break;
    case MSG_ID_BT_BM_SERVICE_SEARCH_CNF:
        btmtk_gap_handle_service_search_cnf(nat, env, ilm);
		break;
    case MSG_ID_BT_BM_SERVICE_SEARCH_CANCEL_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_SERVICE_SEARCH_CANCEL_CNF");
		break;
    case MSG_ID_BT_BM_SEARCH_ATTRIBUTE_RESULT_IND:
		ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_ATTRIBUTE_RESULT_IND");
		break;
    case MSG_ID_BT_BM_SEARCH_ATTRIBUTE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_ATTRIBUTE_CNF");
		break;
    case MSG_ID_BT_BM_LINK_STATE_IND:
        btmtk_gap_handle_link_state_ind(nat, env, ilm);
		break;
    case MSG_ID_BT_BM_LINK_ALLOW_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_LINK_ALLOW_CNF");
		break;
    case MSG_ID_BT_BM_LINK_DISALLOW_CNF:    
		ALOGI("[GAP] MSG_ID_BT_BM_LINK_DISALLOW_CNF");
		break;
    case MSG_ID_BT_BM_LINK_CONNECT_ACCEPT_IND:
		ALOGI("[GAP] MSG_ID_BT_BM_LINK_CONNECT_ACCEPT_IND");
		break;
    case MSG_ID_BT_BM_GET_LINK_STATE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_GET_LINK_STATE_CNF");
		break;
    case MSG_ID_BT_BM_GET_SCATTERNET_STATE_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_GET_SCATTERNET_STATE_CNF");
		break;
    case MSG_ID_BT_BM_SEARCH_RAW_RESULT:
		ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_RAW_RESULT");
        btmtk_gap_handle_service_search_raw_result(nat, env, ilm);
		break;
    case MSG_ID_BT_BM_SEARCH_RAW_CNF:
		ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_RAW_CNF");
        btmtk_gap_handle_service_search_raw_cnf(nat, env, ilm);
		break;
	case MSG_ID_BT_HDP_CONNECT_CNF:
		btmtk_hdp_handle_connect_cnf(nat, env, ilm);
		break;
	case MSG_ID_BT_HDP_DISCONNECT_CNF:
		btmtk_hdp_handle_disconnect_cnf(nat, env, ilm);
		break;
		
	case MSG_ID_BT_HDP_CHANNEL_OPENED_IND:
		btmtk_hdp_handle_channel_opened_ind(nat, env, ilm);
		break;
		
	case MSG_ID_BT_HDP_CHANNEL_CLOSED_IND:
		btmtk_hdp_handle_channel_closed_ind(nat, env, ilm);
		break;
		
    }
    env->PopLocalFrame(NULL);
    return true;
}

/* true : handled, 0 : not handled */
static bool event_handler(native_data_t *nat, ev_struct *event) {
    JNIEnv *env;

    nat->vm->GetEnv((void**)&env, nat->envVer);
    ALOGI("[GAP] receive event=%d", event->msg_id);
    env->PushLocalFrame(EVENT_LOOP_REFS);
    switch (event->msg_id)
    {
    case ANDROID_EV_GAP_DISCOVERY_START:
	//nat->inquired_cache_no = 0;
        //memset(nat->inquired_dev_cache, 0x0, sizeof(btmtk_device_entry_struct) * BTBM_ADP_MAX_INQUIRY_NO);       
        btmtk_inquired_dev_cache_reset(nat);
        btmtk_util_update_adapter_property_discovering(env, nat, true);
        break;
    case ANDROID_EV_GAP_DISCOVERY_STOP:
        btmtk_util_update_adapter_property_discovering(env, nat, false);
        break;
    case ANDROID_EV_GAP_LOCAL_NAME_CHANGE:
    {
        jobjectArray str_array = NULL;
        jclass stringClass = NULL;
        Properties prop;
        jstring obj;
        property_value value;
        int array_index = 0;
        
        btmtk_android_gap_name_event_struct *ind_p;
        ind_p = (btmtk_android_gap_name_event_struct *)event->ilm_data;
        
        ALOGI("ANDROID_EV_GAP_LOCAL_NAME_CHANGE : name=%s , len = %d", ind_p->name, ind_p->name_len);
        memcpy(nat->host_cache.name, ind_p->name, BTBM_ADP_MAX_NAME_LEN);
        btmtk_host_cache_write(nat);
        
        stringClass = env->FindClass("java/lang/String");
        str_array = env->NewObjectArray(2, stringClass, NULL);        
        prop.type = 's';
        strcpy( prop.name, "Name");
        value.str_val = (char*)ind_p->name; // true
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, method_onPropertyChanged, str_array);
        break;
    }
    case ANDROID_EV_GAP_SCAN_MODE_CHANGE:
    {
        jobjectArray str_array = NULL;
        jclass stringClass = NULL;
        Properties prop;
        jstring obj;
        property_value value;
        int array_index = 0;
        
        btbm_scan_enable_type mode = (btbm_scan_enable_type)event->ilm_data[0];
        
        ALOGI("ANDROID_EV_GAP_SCAN_MODE_CHANGE : mode = 0x%x", mode);
        nat->host_cache.scan_mode = mode;
        btmtk_host_cache_write(nat);
        
        stringClass = env->FindClass("java/lang/String");
        str_array = env->NewObjectArray(4, stringClass, NULL);        

        prop.type = 'b';
        strcpy( prop.name, "Discoverable");
        value.int_val = (mode & BTBM_ADP_P_OFF_I_ON) ? 1 : 0;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        prop.type = (int) 'b'; 
        strcpy(prop.name, "Pairable");
        value.int_val = (mode & BTBM_ADP_P_ON_I_OFF) ? 1 : 0;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        env->CallVoidMethod(nat->me, method_onPropertyChanged, str_array);
        break;
    }
    case ANDROID_EV_GAP_SCAN_MODE_TIMEOUT_CHANGE:
    {
   
        btbm_scan_enable_type mode = (btbm_scan_enable_type)event->ilm_data[0];
        
        ALOGI("ANDROID_EV_GAP_SCAN_MODE_TIMEOUT_CHANGE : timeout = %d", *(int *)event->ilm_data);
        nat->host_cache.scan_mode_timeout = *(unsigned int *)event->ilm_data;
        btmtk_host_cache_write(nat); 
        break;
    }
	case ANDROID_EV_GAP_PAIRED_DEVICE_RENAME:
    {
		jobjectArray str_array = NULL;
        jclass stringClass = NULL;
        Properties prop;
        jstring obj;
        property_value value;
        int array_index = 0;  
		char object_path[BTMTK_MAX_OBJECT_PATH_SIZE];
		btmtk_device_entry_struct *ptr = NULL; 
		btmtk_android_gap_remote_name_event_struct *ind_p;
		ind_p = (btmtk_android_gap_remote_name_event_struct *)event->ilm_data;
		ALOGI("[GAP] receive event ANDROID_EV_GAP_PAIRED_DEVICE_RENAME : name=%s , len = %d",
					ind_p->name, ind_p->name_len);		

		if ((ptr = btmtk_paired_dev_cache_find(nat, &(ind_p->addr))) != NULL)
        {   
        	memset(ptr->nickname, 0x0, BTBM_ADP_MAX_NAME_LEN);
        	if (ind_p->name_len >= BTBM_ADP_MAX_NAME_LEN)    			
        	{
        		strncpy(ptr->nickname, ind_p->name, BTBM_ADP_MAX_NAME_LEN-1);
				ptr->nickname[BTBM_ADP_MAX_NAME_LEN-1]='\0';
			} else if (ind_p->name_len > 0) {
				strncpy(ptr->nickname, ind_p->name, ind_p->name_len);
			} else {
				ptr->nickname[0]='\0';
			}
			btmtk_paired_dev_cache_write(nat);

			stringClass = env->FindClass("java/lang/String");
			str_array = env->NewObjectArray(2, stringClass, NULL); 
			prop.type = 's';
			strcpy( prop.name, "Alias");
			value.str_val = (char*)ind_p->name; // true
			create_prop_array(env, str_array, &prop, &value, 0, &array_index);
		//	env->CallVoidMethod(nat->me, method_onDevicePropertyChanged, str_array);
			btmtk_util_convert_bdaddr2objpath(object_path, &ind_p->addr);
			env->CallVoidMethod(nat->me, 
                            method_onDevicePropertyChanged, 
                            env->NewStringUTF(object_path), 
                            str_array);
		}
		break;
    }
    case ANDROID_EV_SDP_DEVICE_CREATE:
    {
        btmtk_sdp_req_struct *entry = 
            (btmtk_sdp_req_struct *) calloc(1, sizeof(btmtk_sdp_req_struct));
        bt_addr_struct *addr = (bt_addr_struct *)event->ilm_data;

        ALOGI("[GAP] receive event ANDROID_EV_SDP_DEVICE_CREATE 0x%X:0x%X:0x%X", addr->lap, addr->uap, addr->nap);
        entry->type = BTMTK_SDP_CREATE_DEVICE;
        memcpy(&entry->addr, addr, sizeof(bt_addr_struct));
        btmtk_util_list_append((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)entry);
        break;
    }
    case ANDROID_EV_SDP_PAIRED_DEVICE_CREATE:
    {
        btmtk_sdp_req_struct *entry = 
            (btmtk_sdp_req_struct *) calloc(1, sizeof(btmtk_sdp_req_struct));
        bt_addr_struct *addr = (bt_addr_struct *)event->ilm_data;

        ALOGI("[GAP] receive event ANDROID_EV_SDP_PAIRED_DEVICE_CREATE 0x%X:0x%X:0x%X", addr->lap, addr->uap, addr->nap);
        entry->type = BTMTK_SDP_CREATE_PAIRED_DEVICE;
        memcpy(&entry->addr, addr, sizeof(bt_addr_struct));
        btmtk_util_list_append((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)entry);
        break;
    }
	case ANDROID_EV_SDP_PAIRED_DEVICE_REMOVED:
	{
		bt_addr_struct *addr = (bt_addr_struct *)event->ilm_data;
		btmtk_sdp_req_struct *req = btmtk_util_find_sdp_request(nat, addr);
        
        if (req!= NULL && req->type == BTMTK_SDP_CREATE_PAIRED_DEVICE)
        {
            btmtk_util_list_remove((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)req);
        }
		break;
	}
    case ANDROID_EV_DEVICE_TRUSTED:
    {
		btmtk_android_gap_device_trusted_struct *ind_p;
		ind_p = (btmtk_android_gap_device_trusted_struct *)event->ilm_data;
        btmtk_util_update_device_property_trusted(env, nat, &ind_p->addr, ind_p->trusted);
        break;
    }
    case ANDROID_EV_SDP_DISCOVER:
    {
        btmtk_sdp_req_struct *entry = 
            (btmtk_sdp_req_struct *) calloc(1, sizeof(btmtk_sdp_req_struct));
        btmtk_android_sdp_service_search_event_struct *ptr = (btmtk_android_sdp_service_search_event_struct *)event->ilm_data;

        ALOGI("[GAP] receive event ANDROID_EV_SDP_DEVICE_CREATE 0x%X:0x%X:0x%X", ptr->addr.lap, ptr->addr.uap, ptr->addr.nap);
        ALOGI("[GAP] pattern (%d) %2x:%2x:%2x:%2x", 
            ptr->pattern_len, ptr->pattern[0], ptr->pattern[1], ptr->pattern[2], ptr->pattern[3]);
        entry->type = BTMTK_SDP_DISCOVERY;
        memcpy(&entry->addr, &ptr->addr, sizeof(bt_addr_struct));
        entry->app_uuid_no = 1;
        memcpy(entry->app_uuid, ptr->pattern, ptr->pattern_len);
        btmtk_util_list_append((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)entry);
        break;
    }
    case ANDROID_EV_DUN_PPPD_START:
    {
        int ret;
        ret = property_set("ctl.start", "pppd");	
        if (ret < 0)
        {
            ALOGE("[GAP] failed to start ppp daemon process: %d", ret);
        }
        break;
    }
    case ANDROID_EV_DUN_PPPD_STOP:
    {
        property_set("ctl.stop", "pppd");
        break;
    }
	case ANDROID_EV_GAP_POWER_STATE_CHANGE:
	{
		btmtk_power_state_enum state = (btmtk_power_state_enum)event->ilm_data[0];
		ALOGI("ANDROID_EV_GAP_POWER_STATE_CHANGE : %d", state);
		if (state == BTMTK_POWER_STATE_OFF)
		{
			nat->activity = BTMTK_GAP_ACT_NONE;

    		while (nat->requests)
    		{
       			btmtk_util_list_remove((btmtk_list_header_struct **)&nat->requests, (btmtk_list_header_struct *)nat->requests);
    		}
    		nat->requests = NULL;
		}
		if(nat->service_nat->state == BTMTK_POWER_STATE_TURNING_OFF)
			nat->service_nat->state = BTMTK_POWER_STATE_OFF;
		break;
	}
    }
    env->PopLocalFrame(NULL);
    return true;
}



static void *eventLoopMain(void *ptr) {
    native_data_t *nat = (native_data_t *)ptr;
    JNIEnv *env;

    JavaVMAttachArgs args;
    char name[] = "BT EventLoop";
    args.version = nat->envVer;
    args.name = name;
    args.group = NULL;

    nat->vm->AttachCurrentThread(&env, &args);
    /*
    dbus_connection_set_watch_functions(nat->conn, dbusAddWatch,
            dbusRemoveWatch, dbusToggleWatch, ptr, NULL);
    */
    while (1) {
        ALOGI("[MSG] Start retrieve data");
        for (int i = 0; i < nat->pollMemberCount; i++) {
            if (!nat->pollData[i].revents) {
                continue;
            }
            ALOGI("[MSG] fd %d data ready", i);
            if (nat->pollData[i].fd == nat->controlFdR) {
                ALOGI("[MSG] nat->controlFdR data ready");
                char data;
                while (recv(nat->controlFdR, &data, sizeof(char), MSG_DONTWAIT)
                        != -1) {
                    ALOGI("[MSG] nat->controlFdR receive : %d", data);
                    switch (data) {
                    case EVENT_LOOP_EXIT:
                    {
			   /*
                        dbus_connection_set_watch_functions(nat->conn,
                                NULL, NULL, NULL, NULL, NULL);
                       */
                        tearDownEventLoop(nat);
                        nat->vm->DetachCurrentThread();
                        shutdown(nat->controlFdR,SHUT_RDWR);
                        return NULL;
                    }
                    case EVENT_LOOP_ADD:
                    {
                        /* handleWatchAdd(nat);*/
                        break;
                    }
                    case EVENT_LOOP_REMOVE:
                    {
                        /* handleWatchRemove(nat); */
                        break;
                    }
                    }
                }
            }
            else 
            {
                ilm_struct ilm;
                int ret;
                
                ALOGI("[MSG] nat->pollData[i].fd data ready : revents = 0x%X", nat->pollData[i].revents);
                while ((ret = recv(nat->pollData[i].fd, (void*)&ilm, sizeof(ilm_struct), MSG_DONTWAIT)) != -1) 
                {
                    ALOGI("[MSG] msg %d received : size=%d", ilm.msg_id, ret);
		    if (nat->service_nat->state == BTMTK_POWER_STATE_OFF)
		    {
                        ALOGI("[MSG] Wrong state %d: discard msg", nat->service_nat->state);
		    }
                    else if (ilm.msg_id >= MSG_ID_BT_CUSTOM_MSG_ID_BEGIN)
                    {
                        pthread_mutex_lock(&(nat->thread_mutex));
                        event_handler(nat, &ilm);
						pthread_mutex_unlock(&(nat->thread_mutex));
                    }
                    else
                    {
                        pthread_mutex_lock(&(nat->thread_mutex));
                        msg_handler(nat, &ilm);
						pthread_mutex_unlock(&(nat->thread_mutex));
                    }
					
		    if (env->ExceptionCheck())
	            {
			env->ExceptionDescribe();
			env->ExceptionClear();
		    }
                }
                /*
                short events = nat->pollData[i].revents;
                unsigned int flags = unix_events_to_dbus_flags(events);
                dbus_watch_handle(nat->watchData[i], flags);
                nat->pollData[i].revents = 0;
                */
                // can only do one - it may have caused a 'remove'
                break;
            }
        }
	 /*
        while (dbus_connection_dispatch(nat->conn) == 
                DBUS_DISPATCH_DATA_REMAINS) {
        }
        */
        ALOGI("[MSG] Start polling");
        poll(nat->pollData, nat->pollMemberCount, -1);
        ALOGI("[MSG] Polling returned");
    }
}
#endif

#ifdef HAVE_BLUETOOTH
static DBusHandlerResult event_filter(DBusConnection *conn, DBusMessage *msg,
                                      void *data);
DBusHandlerResult agent_event_filter(DBusConnection *conn,
                                     DBusMessage *msg,
                                     void *data);
static int register_agent(native_data_t *nat,
                          const char *agent_path, const char *capabilities);

static const DBusObjectPathVTable agent_vtable = {
    NULL, agent_event_filter, NULL, NULL, NULL, NULL
};

static unsigned int unix_events_to_dbus_flags(short events) {
    return (events & DBUS_WATCH_READABLE ? POLLIN : 0) |
           (events & DBUS_WATCH_WRITABLE ? POLLOUT : 0) |
           (events & DBUS_WATCH_ERROR ? POLLERR : 0) |
           (events & DBUS_WATCH_HANGUP ? POLLHUP : 0);
}

static short dbus_flags_to_unix_events(unsigned int flags) {
    return (flags & POLLIN ? DBUS_WATCH_READABLE : 0) |
           (flags & POLLOUT ? DBUS_WATCH_WRITABLE : 0) |
           (flags & POLLERR ? DBUS_WATCH_ERROR : 0) |
           (flags & POLLHUP ? DBUS_WATCH_HANGUP : 0);
}

static jboolean setUpEventLoop(native_data_t *nat) {
    ALOGV("%s", __FUNCTION__);

    if (nat != NULL && nat->conn != NULL) {
        dbus_threads_init_default();
        DBusError err;
        dbus_error_init(&err);

        const char *agent_path = "/android/bluetooth/agent";
        const char *capabilities = "DisplayYesNo";
        if (register_agent(nat, agent_path, capabilities) < 0) {
            dbus_connection_unregister_object_path (nat->conn, agent_path);
            return JNI_FALSE;
        }

        // Add a filter for all incoming messages
        if (!dbus_connection_add_filter(nat->conn, event_filter, nat, NULL)){
            return JNI_FALSE;
        }

        // Set which messages will be processed by this dbus connection
        dbus_bus_add_match(nat->conn,
                "type='signal',interface='org.freedesktop.DBus'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }
        dbus_bus_add_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Adapter'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }
        dbus_bus_add_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Device'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }
        dbus_bus_add_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Input'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }
        dbus_bus_add_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Network'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }
        dbus_bus_add_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".NetworkServer'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }

        dbus_bus_add_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".HealthDevice'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }

        dbus_bus_add_match(nat->conn,
                "type='signal',interface='org.bluez.AudioSink'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
            return JNI_FALSE;
        }

        return JNI_TRUE;
    }
    return JNI_FALSE;
}


const char * get_adapter_path(DBusConnection *conn) {
    DBusMessage *msg = NULL, *reply = NULL;
    DBusError err;
    const char *device_path = NULL;
    int attempt = 0;

    for (attempt = 0; attempt < 1000 && reply == NULL; attempt ++) {
        msg = dbus_message_new_method_call("org.bluez", "/",
              "org.bluez.Manager", "DefaultAdapter");
        if (!msg) {
            ALOGE("%s: Can't allocate new method call for get_adapter_path!",
                  __FUNCTION__);
            return NULL;
        }
        dbus_message_append_args(msg, DBUS_TYPE_INVALID);
        dbus_error_init(&err);
        reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                if (dbus_error_has_name(&err,
                    "org.freedesktop.DBus.Error.ServiceUnknown")) {
                    // bluetoothd is still down, retry
                    LOG_AND_FREE_DBUS_ERROR(&err);
                    usleep(10000);  // 10 ms
                    continue;
                } else {
                    // Some other error we weren't expecting
                    LOG_AND_FREE_DBUS_ERROR(&err);
                }
            }
            goto failed;
        }
    }
    if (attempt == 1000) {
        ALOGE("Time out while trying to get Adapter path, is bluetoothd up ?");
        goto failed;
    }

    if (!dbus_message_get_args(reply, &err, DBUS_TYPE_OBJECT_PATH,
                               &device_path, DBUS_TYPE_INVALID)
                               || !device_path){
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        goto failed;
    }
    dbus_message_unref(msg);
    return device_path;

failed:
    dbus_message_unref(msg);
    return NULL;
}

static int register_agent(native_data_t *nat,
                          const char * agent_path, const char * capabilities)
{
    DBusMessage *msg, *reply;
    DBusError err;
    dbus_bool_t oob = TRUE;

    if (!dbus_connection_register_object_path(nat->conn, agent_path,
            &agent_vtable, nat)) {
        ALOGE("%s: Can't register object path %s for agent!",
              __FUNCTION__, agent_path);
        return -1;
    }

    nat->adapter = get_adapter_path(nat->conn);
    if (nat->adapter == NULL) {
        return -1;
    }
    msg = dbus_message_new_method_call("org.bluez", nat->adapter,
          "org.bluez.Adapter", "RegisterAgent");
    if (!msg) {
        ALOGE("%s: Can't allocate new method call for agent!",
              __FUNCTION__);
        return -1;
    }
    dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &agent_path,
                             DBUS_TYPE_STRING, &capabilities,
                             DBUS_TYPE_INVALID);

    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block(nat->conn, msg, -1, &err);
    dbus_message_unref(msg);

    if (!reply) {
        ALOGE("%s: Can't register agent!", __FUNCTION__);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        return -1;
    }

    dbus_message_unref(reply);
    dbus_connection_flush(nat->conn);

    return 0;
}

static void tearDownEventLoop(native_data_t *nat) {
    ALOGV("%s", __FUNCTION__);
    if (nat != NULL && nat->conn != NULL) {

        DBusMessage *msg, *reply;
        DBusError err;
        dbus_error_init(&err);
        const char * agent_path = "/android/bluetooth/agent";

        msg = dbus_message_new_method_call("org.bluez",
                                           nat->adapter,
                                           "org.bluez.Adapter",
                                           "UnregisterAgent");
        if (msg != NULL) {
            dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &agent_path,
                                     DBUS_TYPE_INVALID);
            reply = dbus_connection_send_with_reply_and_block(nat->conn,
                                                              msg, -1, &err);

            if (!reply) {
                if (dbus_error_is_set(&err)) {
                    LOG_AND_FREE_DBUS_ERROR(&err);
                    dbus_error_free(&err);
                }
            } else {
                dbus_message_unref(reply);
            }
            dbus_message_unref(msg);
        } else {
             ALOGE("%s: Can't create new method call!", __FUNCTION__);
        }

        dbus_connection_flush(nat->conn);
        dbus_connection_unregister_object_path(nat->conn, agent_path);

        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".AudioSink'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Device'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Input'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Network'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".NetworkServer'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".HealthDevice'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='org.bluez.audio.Manager'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='"BLUEZ_DBUS_BASE_IFC".Adapter'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }
        dbus_bus_remove_match(nat->conn,
                "type='signal',interface='org.freedesktop.DBus'",
                &err);
        if (dbus_error_is_set(&err)) {
            LOG_AND_FREE_DBUS_ERROR(&err);
        }

        dbus_connection_remove_filter(nat->conn, event_filter, nat);
    }
}


#define EVENT_LOOP_EXIT 1
#define EVENT_LOOP_ADD  2
#define EVENT_LOOP_REMOVE 3
#define EVENT_LOOP_WAKEUP 4

dbus_bool_t dbusAddWatch(DBusWatch *watch, void *data) {
    native_data_t *nat = (native_data_t *)data;

    if (dbus_watch_get_enabled(watch)) {
        // note that we can't just send the watch and inspect it later
        // because we may get a removeWatch call before this data is reacted
        // to by our eventloop and remove this watch..  reading the add first
        // and then inspecting the recently deceased watch would be bad.
        char control = EVENT_LOOP_ADD;
        write(nat->controlFdW, &control, sizeof(char));

        int fd = dbus_watch_get_fd(watch);
        write(nat->controlFdW, &fd, sizeof(int));

        unsigned int flags = dbus_watch_get_flags(watch);
        write(nat->controlFdW, &flags, sizeof(unsigned int));

        write(nat->controlFdW, &watch, sizeof(DBusWatch*));
    }
    return true;
}

void dbusRemoveWatch(DBusWatch *watch, void *data) {
    native_data_t *nat = (native_data_t *)data;

    char control = EVENT_LOOP_REMOVE;
    write(nat->controlFdW, &control, sizeof(char));

    int fd = dbus_watch_get_fd(watch);
    write(nat->controlFdW, &fd, sizeof(int));

    unsigned int flags = dbus_watch_get_flags(watch);
    write(nat->controlFdW, &flags, sizeof(unsigned int));
}

void dbusToggleWatch(DBusWatch *watch, void *data) {
    if (dbus_watch_get_enabled(watch)) {
        dbusAddWatch(watch, data);
    } else {
        dbusRemoveWatch(watch, data);
    }
}

void dbusWakeup(void *data) {
    native_data_t *nat = (native_data_t *)data;

    char control = EVENT_LOOP_WAKEUP;
    write(nat->controlFdW, &control, sizeof(char));
}

static void handleWatchAdd(native_data_t *nat) {
    DBusWatch *watch;
    int newFD;
    unsigned int flags;

    read(nat->controlFdR, &newFD, sizeof(int));
    read(nat->controlFdR, &flags, sizeof(unsigned int));
    read(nat->controlFdR, &watch, sizeof(DBusWatch *));
    short events = dbus_flags_to_unix_events(flags);

    for (int y = 0; y<nat->pollMemberCount; y++) {
        if ((nat->pollData[y].fd == newFD) &&
                (nat->pollData[y].events == events)) {
            ALOGV("DBusWatch duplicate add");
            return;
        }
    }
    if (nat->pollMemberCount == nat->pollDataSize) {
        ALOGV("Bluetooth EventLoop poll struct growing");
        struct pollfd *temp = (struct pollfd *)malloc(
                sizeof(struct pollfd) * (nat->pollMemberCount+1));
        if (!temp) {
            return;
        }
        memcpy(temp, nat->pollData, sizeof(struct pollfd) *
                nat->pollMemberCount);
        free(nat->pollData);
        nat->pollData = temp;
        DBusWatch **temp2 = (DBusWatch **)malloc(sizeof(DBusWatch *) *
                (nat->pollMemberCount+1));
        if (!temp2) {
            return;
        }
        memcpy(temp2, nat->watchData, sizeof(DBusWatch *) *
                nat->pollMemberCount);
        free(nat->watchData);
        nat->watchData = temp2;
        nat->pollDataSize++;
    }
    nat->pollData[nat->pollMemberCount].fd = newFD;
    nat->pollData[nat->pollMemberCount].revents = 0;
    nat->pollData[nat->pollMemberCount].events = events;
    nat->watchData[nat->pollMemberCount] = watch;
    nat->pollMemberCount++;
}

static void handleWatchRemove(native_data_t *nat) {
    int removeFD;
    unsigned int flags;

    read(nat->controlFdR, &removeFD, sizeof(int));
    read(nat->controlFdR, &flags, sizeof(unsigned int));
    short events = dbus_flags_to_unix_events(flags);

    for (int y = 0; y < nat->pollMemberCount; y++) {
        if ((nat->pollData[y].fd == removeFD) &&
                (nat->pollData[y].events == events)) {
            int newCount = --nat->pollMemberCount;
            // copy the last live member over this one
            nat->pollData[y].fd = nat->pollData[newCount].fd;
            nat->pollData[y].events = nat->pollData[newCount].events;
            nat->pollData[y].revents = nat->pollData[newCount].revents;
            nat->watchData[y] = nat->watchData[newCount];
            return;
        }
    }
    ALOGW("WatchRemove given with unknown watch");
}

static void *eventLoopMain(void *ptr) {
    native_data_t *nat = (native_data_t *)ptr;
    JNIEnv *env;

    JavaVMAttachArgs args;
    char name[] = "BT EventLoop";
    args.version = nat->envVer;
    args.name = name;
    args.group = NULL;

    nat->vm->AttachCurrentThread(&env, &args);

    dbus_connection_set_watch_functions(nat->conn, dbusAddWatch,
            dbusRemoveWatch, dbusToggleWatch, ptr, NULL);
    dbus_connection_set_wakeup_main_function(nat->conn, dbusWakeup, ptr, NULL);

    nat->running = true;

    while (1) {
        for (int i = 0; i < nat->pollMemberCount; i++) {
            if (!nat->pollData[i].revents) {
                continue;
            }
            if (nat->pollData[i].fd == nat->controlFdR) {
                char data;
                while (recv(nat->controlFdR, &data, sizeof(char), MSG_DONTWAIT)
                        != -1) {
                    switch (data) {
                    case EVENT_LOOP_EXIT:
                    {
                        dbus_connection_set_watch_functions(nat->conn,
                                NULL, NULL, NULL, NULL, NULL);
                        tearDownEventLoop(nat);
                        nat->vm->DetachCurrentThread();

                        int fd = nat->controlFdR;
                        nat->controlFdR = 0;
                        close(fd);
                        return NULL;
                    }
                    case EVENT_LOOP_ADD:
                    {
                        handleWatchAdd(nat);
                        break;
                    }
                    case EVENT_LOOP_REMOVE:
                    {
                        handleWatchRemove(nat);
                        break;
                    }
                    case EVENT_LOOP_WAKEUP:
                    {
                        // noop
                        break;
                    }
                    }
                }
            } else {
                short events = nat->pollData[i].revents;
                unsigned int flags = unix_events_to_dbus_flags(events);
                dbus_watch_handle(nat->watchData[i], flags);
                nat->pollData[i].revents = 0;
                // can only do one - it may have caused a 'remove'
                break;
            }
        }
        while (dbus_connection_dispatch(nat->conn) ==
                DBUS_DISPATCH_DATA_REMAINS) {
        }

        poll(nat->pollData, nat->pollMemberCount, -1);
    }
}
#endif // HAVE_BLUETOOTH

static jboolean startEventLoopNative(JNIEnv *env, jobject object) {
    jboolean result = JNI_FALSE;
#ifdef HAVE_BLUETOOTH
    event_loop_native_data_t *nat = get_native_data(env, object);

    pthread_mutex_lock(&(nat->thread_mutex));

    nat->running = false;

    if (nat->pollData) {
        ALOGW("trying to start EventLoop a second time!");
        pthread_mutex_unlock( &(nat->thread_mutex) );
        return JNI_FALSE;
    }

    nat->pollData = (struct pollfd *)calloc(
            DEFAULT_INITIAL_POLLFD_COUNT, sizeof(struct pollfd));
    if (!nat->pollData) {
        ALOGE("out of memory error starting EventLoop!");
        goto done;
    }

    nat->watchData = (DBusWatch **)calloc(
            DEFAULT_INITIAL_POLLFD_COUNT, sizeof(DBusWatch *));
    if (!nat->watchData) {
        ALOGE("out of memory error starting EventLoop!");
        goto done;
    }

    nat->pollDataSize = DEFAULT_INITIAL_POLLFD_COUNT;
    nat->pollMemberCount = 1;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, &(nat->controlFdR))) {
        ALOGE("Error getting BT control socket");
        goto done;
    }
    nat->pollData[0].fd = nat->controlFdR;
    nat->pollData[0].events = POLLIN;

    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();

    nat->me = env->NewGlobalRef(object);

    if (setUpEventLoop(nat) != JNI_TRUE) {
        ALOGE("failure setting up Event Loop!");
        goto done;
    }

    pthread_create(&(nat->thread), NULL, eventLoopMain, nat);
    result = JNI_TRUE;

done:
    if (JNI_FALSE == result) {
        if (nat->controlFdW) {
            close(nat->controlFdW);
            nat->controlFdW = 0;
        }
        if (nat->controlFdR) {
            close(nat->controlFdR);
            nat->controlFdR = 0;
        }
        if (nat->me) env->DeleteGlobalRef(nat->me);
        nat->me = NULL;
        if (nat->pollData) free(nat->pollData);
        nat->pollData = NULL;
        if (nat->watchData) free(nat->watchData);
        nat->watchData = NULL;
        nat->pollDataSize = 0;
        nat->pollMemberCount = 0;
    }

    pthread_mutex_unlock(&(nat->thread_mutex));
#endif // HAVE_BLUETOOTH

#ifdef __BTMTK__
    ALOGV("[JNI][API] startEventLoopNative");
    event_loop_native_data_t *nat = get_native_data(env, object);

    pthread_mutex_lock(&(nat->thread_mutex));

    if (nat->pollData) {
        ALOGW("trying to start EventLoop a second time!");
        pthread_mutex_unlock( &(nat->thread_mutex) );
        return JNI_FALSE;
    }

    nat->pollData = (struct pollfd *)calloc(
            DEFAULT_INITIAL_POLLFD_COUNT, sizeof(struct pollfd));
    if (!nat->pollData) {
        ALOGE("out of memory error starting EventLoop!");
        goto done;
    }

    /*
    nat->watchData = (DBusWatch **)calloc(
            DEFAULT_INITIAL_POLLFD_COUNT, sizeof(DBusWatch *));
    if (!nat->watchData) {
        ALOGE("out of memory error starting EventLoop!");
        goto done;
    }
    */

    nat->pollDataSize = DEFAULT_INITIAL_POLLFD_COUNT;
    nat->pollMemberCount = 1;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, &(nat->controlFdR))) {
        ALOGE("Error getting BT control socket");
        goto done;
    }
    nat->pollData[0].fd = nat->controlFdR;
    nat->pollData[0].events = POLLIN;

    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();

    nat->me = env->NewGlobalRef(object);

    if (setUpEventLoop(nat) != JNI_TRUE) {
        ALOGE("failure setting up Event Loop!");
        goto done;
    }

    pthread_create(&(nat->thread), NULL, eventLoopMain, nat);
    result = JNI_TRUE;

done:
    if (JNI_FALSE == result) {
        if (nat->controlFdW || nat->controlFdR) {
            shutdown(nat->controlFdW, SHUT_RDWR);
            nat->controlFdW = 0;
            nat->controlFdR = 0;
        }
        if (nat->me) env->DeleteGlobalRef(nat->me);
        nat->me = NULL;
        if (nat->pollData) free(nat->pollData);
        nat->pollData = NULL;
        nat->pollDataSize = 0;
        nat->pollMemberCount = 0;
    }

    pthread_mutex_unlock(&(nat->thread_mutex));
#endif

    return result;
}

static void stopEventLoopNative(JNIEnv *env, jobject object) {
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);

    pthread_mutex_lock(&(nat->thread_mutex));
    if (nat->pollData) {
        char data = EVENT_LOOP_EXIT;
        ssize_t t = write(nat->controlFdW, &data, sizeof(char));
        void *ret;
        pthread_join(nat->thread, &ret);

        env->DeleteGlobalRef(nat->me);
        nat->me = NULL;
        free(nat->pollData);
        nat->pollData = NULL;
        free(nat->watchData);
        nat->watchData = NULL;
        nat->pollDataSize = 0;
        nat->pollMemberCount = 0;

        int fd = nat->controlFdW;
        nat->controlFdW = 0;
        close(fd);
    }
    nat->running = false;
    pthread_mutex_unlock(&(nat->thread_mutex));
#endif // HAVE_BLUETOOTH
}

static jboolean isEventLoopRunningNative(JNIEnv *env, jobject object) {
    jboolean result = JNI_FALSE;
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);

    pthread_mutex_lock(&(nat->thread_mutex));
    if (nat->running) {
        result = JNI_TRUE;
    }
    pthread_mutex_unlock(&(nat->thread_mutex));

#endif // HAVE_BLUETOOTH

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    //pthread_mutex_lock(&(nat->thread_mutex));
    if (nat->pollData) {
        result = JNI_TRUE;
    }
    //pthread_mutex_unlock(&(nat->thread_mutex));

#endif // __BTMTK__

    return result;
}

#ifdef HAVE_BLUETOOTH
extern DBusHandlerResult a2dp_event_filter(DBusMessage *msg, JNIEnv *env);

// Called by dbus during WaitForAndDispatchEventNative()
static DBusHandlerResult event_filter(DBusConnection *conn, DBusMessage *msg,
                                      void *data) {
    native_data_t *nat;
    JNIEnv *env;
    DBusError err;
    DBusHandlerResult ret;

    dbus_error_init(&err);

    nat = (native_data_t *)data;
    nat->vm->GetEnv((void**)&env, nat->envVer);
    if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL) {
        ALOGV("%s: not interested (not a signal).", __FUNCTION__);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    ALOGV("%s: Received signal %s:%s from %s", __FUNCTION__,
        dbus_message_get_interface(msg), dbus_message_get_member(msg),
        dbus_message_get_path(msg));

    env->PushLocalFrame(EVENT_LOOP_REFS);
    if (dbus_message_is_signal(msg,
                               "org.bluez.Adapter",
                               "DeviceFound")) {
        char *c_address;
        DBusMessageIter iter;
        jobjectArray str_array = NULL;
        if (dbus_message_iter_init(msg, &iter)) {
            dbus_message_iter_get_basic(&iter, &c_address);
            if (dbus_message_iter_next(&iter))
                str_array =
                    parse_remote_device_properties(env, &iter);
        }
        if (str_array != NULL) {
            env->CallVoidMethod(nat->me,
                                method_onDeviceFound,
                                env->NewStringUTF(c_address),
                                str_array);
        } else
            LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.Adapter",
                                     "DeviceDisappeared")) {
        char *c_address;
        if (dbus_message_get_args(msg, &err,
                                  DBUS_TYPE_STRING, &c_address,
                                  DBUS_TYPE_INVALID)) {
            ALOGV("... address = %s", c_address);
            env->CallVoidMethod(nat->me, method_onDeviceDisappeared,
                                env->NewStringUTF(c_address));
        } else LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.Adapter",
                                     "DeviceCreated")) {
        char *c_object_path;
        if (dbus_message_get_args(msg, &err,
                                  DBUS_TYPE_OBJECT_PATH, &c_object_path,
                                  DBUS_TYPE_INVALID)) {
            ALOGV("... address = %s", c_object_path);
            env->CallVoidMethod(nat->me,
                                method_onDeviceCreated,
                                env->NewStringUTF(c_object_path));
        } else LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.Adapter",
                                     "DeviceRemoved")) {
        char *c_object_path;
        if (dbus_message_get_args(msg, &err,
                                 DBUS_TYPE_OBJECT_PATH, &c_object_path,
                                 DBUS_TYPE_INVALID)) {
           ALOGV("... Object Path = %s", c_object_path);
           env->CallVoidMethod(nat->me,
                               method_onDeviceRemoved,
                               env->NewStringUTF(c_object_path));
        } else LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto success;
    } else if (dbus_message_is_signal(msg,
                                      "org.bluez.Adapter",
                                      "PropertyChanged")) {
        jobjectArray str_array = parse_adapter_property_change(env, msg);
        if (str_array != NULL) {
            /* Check if bluetoothd has (re)started, if so update the path. */
            jstring property =(jstring) env->GetObjectArrayElement(str_array, 0);
            const char *c_property = env->GetStringUTFChars(property, NULL);
            if (!strncmp(c_property, "Powered", strlen("Powered"))) {
                jstring value =
                    (jstring) env->GetObjectArrayElement(str_array, 1);
                const char *c_value = env->GetStringUTFChars(value, NULL);
                if (!strncmp(c_value, "true", strlen("true")))
                    nat->adapter = get_adapter_path(nat->conn);
                env->ReleaseStringUTFChars(value, c_value);
            }
            env->ReleaseStringUTFChars(property, c_property);

            env->CallVoidMethod(nat->me,
                              method_onPropertyChanged,
                              str_array);
        } else LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto success;
    } else if (dbus_message_is_signal(msg,
                                      "org.bluez.Device",
                                      "PropertyChanged")) {
        jobjectArray str_array = parse_remote_device_property_change(env, msg);
        if (str_array != NULL) {
            const char *remote_device_path = dbus_message_get_path(msg);
            env->CallVoidMethod(nat->me,
                            method_onDevicePropertyChanged,
                            env->NewStringUTF(remote_device_path),
                            str_array);
        } else LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto success;
    } else if (dbus_message_is_signal(msg,
                                      "org.bluez.Device",
                                      "DisconnectRequested")) {
        const char *remote_device_path = dbus_message_get_path(msg);
        env->CallVoidMethod(nat->me,
                            method_onDeviceDisconnectRequested,
                            env->NewStringUTF(remote_device_path));
        goto success;
    } else if (dbus_message_is_signal(msg,
                                      "org.bluez.Input",
                                      "PropertyChanged")) {

        jobjectArray str_array =
                    parse_input_property_change(env, msg);
        if (str_array != NULL) {
            const char *c_path = dbus_message_get_path(msg);
            env->CallVoidMethod(nat->me,
                                method_onInputDevicePropertyChanged,
                                env->NewStringUTF(c_path),
                                str_array);
        } else {
            LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        }
        goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.Network",
                                     "PropertyChanged")) {

       jobjectArray str_array =
                   parse_pan_property_change(env, msg);
       if (str_array != NULL) {
           const char *c_path = dbus_message_get_path(msg);
           env->CallVoidMethod(nat->me,
                               method_onPanDevicePropertyChanged,
                               env->NewStringUTF(c_path),
                               str_array);
       } else {
           LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
       }
       goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.NetworkServer",
                                     "DeviceDisconnected")) {
       char *c_address;
       if (dbus_message_get_args(msg, &err,
                                  DBUS_TYPE_STRING, &c_address,
                                  DBUS_TYPE_INVALID)) {
           env->CallVoidMethod(nat->me,
                               method_onNetworkDeviceDisconnected,
                               env->NewStringUTF(c_address));
       } else {
           LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
       }
       goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.NetworkServer",
                                     "DeviceConnected")) {
       char *c_address;
       char *c_iface;
       uint16_t uuid;

       if (dbus_message_get_args(msg, &err,
                                  DBUS_TYPE_STRING, &c_address,
                                  DBUS_TYPE_STRING, &c_iface,
                                  DBUS_TYPE_UINT16, &uuid,
                                  DBUS_TYPE_INVALID)) {
           env->CallVoidMethod(nat->me,
                               method_onNetworkDeviceConnected,
                               env->NewStringUTF(c_address),
                               env->NewStringUTF(c_iface),
                               uuid);
       } else {
           LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
       }
       goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.HealthDevice",
                                     "ChannelConnected")) {
       const char *c_path = dbus_message_get_path(msg);
       const char *c_channel_path;
       jboolean exists = JNI_TRUE;
       if (dbus_message_get_args(msg, &err,
                                  DBUS_TYPE_OBJECT_PATH, &c_channel_path,
                                  DBUS_TYPE_INVALID)) {
           env->CallVoidMethod(nat->me,
                               method_onHealthDeviceChannelChanged,
                               env->NewStringUTF(c_path),
                               env->NewStringUTF(c_channel_path),
                               exists);
       } else {
           LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
       }
       goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.HealthDevice",
                                     "ChannelDeleted")) {

       const char *c_path = dbus_message_get_path(msg);
       const char *c_channel_path;
       jboolean exists = JNI_FALSE;
       if (dbus_message_get_args(msg, &err,
                                  DBUS_TYPE_OBJECT_PATH, &c_channel_path,
                                  DBUS_TYPE_INVALID)) {
           env->CallVoidMethod(nat->me,
                               method_onHealthDeviceChannelChanged,
                               env->NewStringUTF(c_path),
                               env->NewStringUTF(c_channel_path),
                               exists);
       } else {
           LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
       }
       goto success;
    } else if (dbus_message_is_signal(msg,
                                     "org.bluez.HealthDevice",
                                     "PropertyChanged")) {
        jobjectArray str_array =
                    parse_health_device_property_change(env, msg);
        if (str_array != NULL) {
            const char *c_path = dbus_message_get_path(msg);
            env->CallVoidMethod(nat->me,
                                method_onHealthDevicePropertyChanged,
                                env->NewStringUTF(c_path),
                                str_array);
       } else {
           LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
       }
       goto success;
    }

    ret = a2dp_event_filter(msg, env);
    env->PopLocalFrame(NULL);
    return ret;

success:
    env->PopLocalFrame(NULL);
    return DBUS_HANDLER_RESULT_HANDLED;
}

// Called by dbus during WaitForAndDispatchEventNative()
DBusHandlerResult agent_event_filter(DBusConnection *conn,
                                     DBusMessage *msg, void *data) {
    native_data_t *nat = (native_data_t *)data;
    JNIEnv *env;
    if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
        ALOGV("%s: not interested (not a method call).", __FUNCTION__);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    ALOGI("%s: Received method %s:%s", __FUNCTION__,
         dbus_message_get_interface(msg), dbus_message_get_member(msg));

    if (nat == NULL) return DBUS_HANDLER_RESULT_HANDLED;

    nat->vm->GetEnv((void**)&env, nat->envVer);
    env->PushLocalFrame(EVENT_LOOP_REFS);

    if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "Cancel")) {
        env->CallVoidMethod(nat->me, method_onAgentCancel);
        // reply
        DBusMessage *reply = dbus_message_new_method_return(msg);
        if (!reply) {
            ALOGE("%s: Cannot create message reply\n", __FUNCTION__);
            goto failure;
        }
        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(reply);
        goto success;

    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "Authorize")) {
        char *object_path;
        const char *uuid;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_STRING, &uuid,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for Authorize() method", __FUNCTION__);
            goto failure;
        }

        ALOGV("... object_path = %s", object_path);
        ALOGV("... uuid = %s", uuid);

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onAgentAuthorize,
                env->NewStringUTF(object_path), env->NewStringUTF(uuid),
                int(msg));

        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "OutOfBandAvailable")) {
        char *object_path;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for OutOfBandData available() method", __FUNCTION__);
            goto failure;
        }

        ALOGV("... object_path = %s", object_path);

        bool available =
            env->CallBooleanMethod(nat->me, method_onAgentOutOfBandDataAvailable,
                env->NewStringUTF(object_path));


        // reply
        if (available) {
            DBusMessage *reply = dbus_message_new_method_return(msg);
            if (!reply) {
                ALOGE("%s: Cannot create message reply\n", __FUNCTION__);
                goto failure;
            }
            dbus_connection_send(nat->conn, reply, NULL);
            dbus_message_unref(reply);
        } else {
            DBusMessage *reply = dbus_message_new_error(msg,
                    "org.bluez.Error.DoesNotExist", "OutofBand data not available");
            if (!reply) {
                ALOGE("%s: Cannot create message reply\n", __FUNCTION__);
                goto failure;
            }
            dbus_connection_send(nat->conn, reply, NULL);
            dbus_message_unref(reply);
        }
        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "RequestPinCode")) {
        char *object_path;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for RequestPinCode() method", __FUNCTION__);
            goto failure;
        }

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onRequestPinCode,
                                       env->NewStringUTF(object_path),
                                       int(msg));
        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "RequestPasskey")) {
        char *object_path;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for RequestPasskey() method", __FUNCTION__);
            goto failure;
        }

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onRequestPasskey,
                                       env->NewStringUTF(object_path),
                                       int(msg));
        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "RequestOobData")) {
        char *object_path;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for RequestOobData() method", __FUNCTION__);
            goto failure;
        }

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onRequestOobData,
                                       env->NewStringUTF(object_path),
                                       int(msg));
        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "DisplayPasskey")) {
        char *object_path;
        uint32_t passkey;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_UINT32, &passkey,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for RequestPasskey() method", __FUNCTION__);
            goto failure;
        }

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onDisplayPasskey,
                                       env->NewStringUTF(object_path),
                                       passkey,
                                       int(msg));
        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "RequestConfirmation")) {
        char *object_path;
        uint32_t passkey;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_UINT32, &passkey,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for RequestConfirmation() method", __FUNCTION__);
            goto failure;
        }

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onRequestPasskeyConfirmation,
                                       env->NewStringUTF(object_path),
                                       passkey,
                                       int(msg));
        goto success;
    } else if (dbus_message_is_method_call(msg,
            "org.bluez.Agent", "RequestPairingConsent")) {
        char *object_path;
        if (!dbus_message_get_args(msg, NULL,
                                   DBUS_TYPE_OBJECT_PATH, &object_path,
                                   DBUS_TYPE_INVALID)) {
            ALOGE("%s: Invalid arguments for RequestPairingConsent() method", __FUNCTION__);
            goto failure;
        }

        dbus_message_ref(msg);  // increment refcount because we pass to java
        env->CallVoidMethod(nat->me, method_onRequestPairingConsent,
                                       env->NewStringUTF(object_path),
                                       int(msg));
        goto success;
    } else if (dbus_message_is_method_call(msg,
                  "org.bluez.Agent", "Release")) {
        // reply
        DBusMessage *reply = dbus_message_new_method_return(msg);
        if (!reply) {
            ALOGE("%s: Cannot create message reply\n", __FUNCTION__);
            goto failure;
        }
        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(reply);
        goto success;
    } else {
        ALOGV("%s:%s is ignored", dbus_message_get_interface(msg), dbus_message_get_member(msg));
    }

failure:
    env->PopLocalFrame(NULL);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

success:
    env->PopLocalFrame(NULL);
    return DBUS_HANDLER_RESULT_HANDLED;

}
#endif


#ifdef HAVE_BLUETOOTH

void onCreatePairedDeviceResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    const char *address = (const char *)user;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    jstring addr;

    nat->vm->GetEnv((void**)&env, nat->envVer);

    ALOGV("... address = %s", address);

    jint result = BOND_RESULT_SUCCESS;
    if (dbus_set_error_from_message(&err, msg)) {
        if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.AuthenticationFailed")) {
            // Pins did not match, or remote device did not respond to pin
            // request in time
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_AUTH_FAILED;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.AuthenticationRejected")) {
            // We rejected pairing, or the remote side rejected pairing. This
            // happens if either side presses 'cancel' at the pairing dialog.
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_AUTH_REJECTED;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.AuthenticationCanceled")) {
            // Not sure if this happens
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_AUTH_CANCELED;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.ConnectionAttemptFailed")) {
            // Other device is not responding at all
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_REMOTE_DEVICE_DOWN;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.AlreadyExists")) {
            // already bonded
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_SUCCESS;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.InProgress") &&
                   !strcmp(err.message, "Bonding in progress")) {
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            goto done;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.InProgress") &&
                   !strcmp(err.message, "Discover in progress")) {
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_DISCOVERY_IN_PROGRESS;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.RepeatedAttempts")) {
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_REPEATED_ATTEMPTS;
        } else if (!strcmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.AuthenticationTimeout")) {
            ALOGV("... error = %s (%s)\n", err.name, err.message);
            result = BOND_RESULT_AUTH_TIMEOUT;
        } else {
            ALOGE("%s: D-Bus error: %s (%s)\n", __FUNCTION__, err.name, err.message);
            result = BOND_RESULT_ERROR;
        }
    }

    addr = env->NewStringUTF(address);
    env->CallVoidMethod(nat->me,
                        method_onCreatePairedDeviceResult,
                        addr,
                        result);
    env->DeleteLocalRef(addr);
done:
    dbus_error_free(&err);
    free(user);
}

void onCreateDeviceResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    const char *address= (const char *)user;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);

    ALOGV("... Address = %s", address);

    jint result = CREATE_DEVICE_SUCCESS;
    if (dbus_set_error_from_message(&err, msg)) {
        if (dbus_error_has_name(&err, "org.bluez.Error.AlreadyExists")) {
            result = CREATE_DEVICE_ALREADY_EXISTS;
        } else {
            result = CREATE_DEVICE_FAILED;
        }
        LOG_AND_FREE_DBUS_ERROR(&err);
    }
    jstring addr = env->NewStringUTF(address);
    env->CallVoidMethod(nat->me,
                        method_onCreateDeviceResult,
                        addr,
                        result);
    env->DeleteLocalRef(addr);
    free(user);
}

void onDiscoverServicesResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    const char *path = (const char *)user;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);

    ALOGV("... Device Path = %s", path);

    bool result = JNI_TRUE;
    if (dbus_set_error_from_message(&err, msg)) {
        LOG_AND_FREE_DBUS_ERROR(&err);
        result = JNI_FALSE;
    }
    jstring jPath = env->NewStringUTF(path);
    env->CallVoidMethod(nat->me,
                        method_onDiscoverServicesResult,
                        jPath,
                        result);
    env->DeleteLocalRef(jPath);
    free(user);
}

void onGetDeviceServiceChannelResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    const char *address = (const char *) user;
    native_data_t *nat = (native_data_t *) n;

    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);

    jint channel = -2;

    ALOGV("... address = %s", address);

    if (dbus_set_error_from_message(&err, msg) ||
        !dbus_message_get_args(msg, &err,
                               DBUS_TYPE_INT32, &channel,
                               DBUS_TYPE_INVALID)) {
        ALOGE("%s: D-Bus error: %s (%s)\n", __FUNCTION__, err.name, err.message);
        dbus_error_free(&err);
    }

done:
    jstring addr = env->NewStringUTF(address);
    env->CallVoidMethod(nat->me,
                        method_onGetDeviceServiceChannelResult,
                        addr,
                        channel);
    env->DeleteLocalRef(addr);
    free(user);
}

void onInputDeviceConnectionResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    const char *path = (const char *)user;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);

    jint result = INPUT_OPERATION_SUCCESS;
    if (dbus_set_error_from_message(&err, msg)) {
        if (!strcmp(err.name, BLUEZ_ERROR_IFC ".ConnectionAttemptFailed")) {
            result = INPUT_CONNECT_FAILED_ATTEMPT_FAILED;
        } else if (!strcmp(err.name, BLUEZ_ERROR_IFC ".AlreadyConnected")) {
            result = INPUT_CONNECT_FAILED_ALREADY_CONNECTED;
        } else if (!strcmp(err.name, BLUEZ_ERROR_IFC ".Failed")) {
            // TODO():This is flaky, need to change Bluez to add new error codes
            if (!strcmp(err.message, "Transport endpoint is not connected")) {
              result = INPUT_DISCONNECT_FAILED_NOT_CONNECTED;
            } else {
              result = INPUT_OPERATION_GENERIC_FAILURE;
            }
        } else {
            result = INPUT_OPERATION_GENERIC_FAILURE;
        }
        LOG_AND_FREE_DBUS_ERROR(&err);
    }

    ALOGV("... Device Path = %s, result = %d", path, result);
    jstring jPath = env->NewStringUTF(path);
    env->CallVoidMethod(nat->me,
                        method_onInputDeviceConnectionResult,
                        jPath,
                        result);
    env->DeleteLocalRef(jPath);
    free(user);
}

void onPanDeviceConnectionResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    const char *path = (const char *)user;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);

    jint result = PAN_OPERATION_SUCCESS;
    if (dbus_set_error_from_message(&err, msg)) {
        if (!strcmp(err.name, BLUEZ_ERROR_IFC ".ConnectionAttemptFailed")) {
            result = PAN_CONNECT_FAILED_ATTEMPT_FAILED;
        } else if (!strcmp(err.name, BLUEZ_ERROR_IFC ".Failed")) {
            // TODO():This is flaky, need to change Bluez to add new error codes
            if (!strcmp(err.message, "Device already connected")) {
                result = PAN_CONNECT_FAILED_ALREADY_CONNECTED;
            } else if (!strcmp(err.message, "Device not connected")) {
                result = PAN_DISCONNECT_FAILED_NOT_CONNECTED;
            } else {
                result = PAN_OPERATION_GENERIC_FAILURE;
            }
        } else {
            result = PAN_OPERATION_GENERIC_FAILURE;
        }
        LOG_AND_FREE_DBUS_ERROR(&err);
    }

    ALOGV("... Pan Device Path = %s, result = %d", path, result);
    jstring jPath = env->NewStringUTF(path);
    env->CallVoidMethod(nat->me,
                        method_onPanDeviceConnectionResult,
                        jPath,
                        result);
    env->DeleteLocalRef(jPath);
    free(user);
}

void onHealthDeviceConnectionResult(DBusMessage *msg, void *user, void *n) {
    ALOGV("%s", __FUNCTION__);

    native_data_t *nat = (native_data_t *)n;
    DBusError err;
    dbus_error_init(&err);
    JNIEnv *env;
    nat->vm->GetEnv((void**)&env, nat->envVer);

    jint result = HEALTH_OPERATION_SUCCESS;
    if (dbus_set_error_from_message(&err, msg)) {
        if (!strcmp(err.name, BLUEZ_ERROR_IFC ".InvalidArgs")) {
            result = HEALTH_OPERATION_INVALID_ARGS;
        } else if (!strcmp(err.name, BLUEZ_ERROR_IFC ".HealthError")) {
            result = HEALTH_OPERATION_ERROR;
        } else if (!strcmp(err.name, BLUEZ_ERROR_IFC ".NotFound")) {
            result = HEALTH_OPERATION_NOT_FOUND;
        } else if (!strcmp(err.name, BLUEZ_ERROR_IFC ".NotAllowed")) {
            result = HEALTH_OPERATION_NOT_ALLOWED;
        } else {
            result = HEALTH_OPERATION_GENERIC_FAILURE;
        }
        LOG_AND_FREE_DBUS_ERROR(&err);
    }

    jint code = *(int *) user;
    ALOGV("... Health Device Code = %d, result = %d", code, result);
    env->CallVoidMethod(nat->me,
                        method_onHealthDeviceConnectionResult,
                        code,
                        result);
    free(user);
}
#endif

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"classInitNative", "()V", (void *)classInitNative},
    {"initializeNativeDataNative", "()V", (void *)initializeNativeDataNative},
    {"cleanupNativeDataNative", "()V", (void *)cleanupNativeDataNative},
    {"startEventLoopNative", "()V", (void *)startEventLoopNative},
    {"stopEventLoopNative", "()V", (void *)stopEventLoopNative},
    {"isEventLoopRunningNative", "()Z", (void *)isEventLoopRunningNative}
};

int register_android_server_BluetoothEventLoop(JNIEnv *env) {
    return AndroidRuntime::registerNativeMethods(env,
            "android/server/BluetoothEventLoop", sMethods, NELEM(sMethods));
}

} /* namespace android */
