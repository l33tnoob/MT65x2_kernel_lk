/*
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, eseither express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define DBUS_ADAPTER_IFACE BLUEZ_DBUS_BASE_IFC ".Adapter"
#define DBUS_DEVICE_IFACE BLUEZ_DBUS_BASE_IFC ".Device"
#define DBUS_INPUT_IFACE BLUEZ_DBUS_BASE_IFC ".Input"
#define DBUS_NETWORK_IFACE BLUEZ_DBUS_BASE_IFC ".Network"
#define DBUS_NETWORKSERVER_IFACE BLUEZ_DBUS_BASE_IFC ".NetworkServer"
#define DBUS_HEALTH_MANAGER_PATH "/org/bluez"
#define DBUS_HEALTH_MANAGER_IFACE BLUEZ_DBUS_BASE_IFC ".HealthManager"
#define DBUS_HEALTH_DEVICE_IFACE BLUEZ_DBUS_BASE_IFC ".HealthDevice"
#define DBUS_HEALTH_CHANNEL_IFACE BLUEZ_DBUS_BASE_IFC ".HealthChannel"

#define LOG_TAG "BluetoothService.cpp"

#include "android_bluetooth_common.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_util_Binder.h"
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#ifdef __BTMTK__
#include "cutils/sockets.h"

#include <sys/un.h>
#include "bt_mmi.h"
#include "bt_message.h"
#include "bluetooth_struct.h"
#include "bluetooth_gap_struct.h"
#include "bluetooth_sdp_struct.h"
#include "bluetooth_hdp_struct.h"
#endif
#ifdef HAVE_BLUETOOTH
#include <dbus/dbus.h>
#include <bluedroid/bluetooth.h>
#endif

#include <cutils/properties.h>

namespace android {

#define BLUETOOTH_CLASS_ERROR 0xFF000000
#define PROPERTIES_NREFS 10
#define HDP_CHANNEL_DELIMS  "\MTKDEV"

// We initialize these variables when we load class
// android.server.BluetoothService
static jfieldID field_mNativeData;
static jfieldID field_mEventLoop;

extern "C" int socket_make_sockaddr_un(const char *name, int namespaceId, 
        struct sockaddr_un *p_addr, socklen_t *alen);

extern event_loop_native_data_t *get_EventLoop_native_data(JNIEnv *,
                                                           jobject);
#ifdef HAVE_BLUETOOTH 
extern DBusHandlerResult agent_event_filter(DBusConnection *conn,
                                            DBusMessage *msg,
                                            void *data);
void onCreatePairedDeviceResult(DBusMessage *msg, void *user, void *nat);
void onDiscoverServicesResult(DBusMessage *msg, void *user, void *nat);
void onCreateDeviceResult(DBusMessage *msg, void *user, void *nat);
void onInputDeviceConnectionResult(DBusMessage *msg, void *user, void *nat);
void onPanDeviceConnectionResult(DBusMessage *msg, void *user, void *nat);
void onHealthDeviceConnectionResult(DBusMessage *msg, void *user, void *nat);
#endif
typedef bt_native_data_t native_data_t;

/** Get native data stored in the opaque (Java code maintained) pointer mNativeData
 *  Perform quick sanity check, if there are any problems return NULL
 */
static inline native_data_t * get_native_data(JNIEnv *env, jobject object) {
    native_data_t *nat =
            (native_data_t *)(env->GetIntField(object, field_mNativeData));
#if defined(__BTMTK__)
    if (nat == NULL || nat->servsock < 0)
#elif defined(HAVE_BLUETOOTH)
    if (nat == NULL || nat->conn == NULL)
#endif
    {
        ALOGE("Uninitialized native data\n");
        return NULL;
    }
    return nat;
}

#ifdef __BTMTK__
const unsigned short g_sdp_uuid[] = {
BTMTK_SDP_UUID_HEADSET,
BTMTK_SDP_UUID_HEADSET_AUDIO_GATEWAY,
BTMTK_SDP_UUID_HF_HANDSFREE,
BTMTK_SDP_UUID_AG_HANDSFREE,
BTMTK_SDP_UUID_AUDIO_SOURCE,
BTMTK_SDP_UUID_AUDIO_SINK,
BTMTK_SDP_UUID_AV_REMOTE_CONTROL_TARGET,
BTMTK_SDP_UUID_ADV_AUDIO_DISTRIBUTION,
BTMTK_SDP_UUID_AV_REMOTE_CONTROL,
BTMTK_SDP_UUID_OBEX_OBJECT_PUSH,
BTMTK_SDP_UUID_SERIAL_PORT,
BTMTK_SDP_UUID_DIALUP_NETWORKING,
BTMTK_SDP_UUID_OBEX_FILE_TRANSFER,
BTMTK_SDP_UUID_PANU,
BTMTK_SDP_UUID_PAN_NAP,
BTMTK_SDP_UUID_PAN_GN,
BTMTK_SDP_UUID_DIRECT_PRINTING,
BTMTK_SDP_UUID_IMAGING_RESPONDER,
BTMTK_SDP_UUID_BASIC_PRINTING,
BTMTK_SDP_UUID_HUMAN_INTERFACE,
BTMTK_SDP_UUID_SAP,
BTMTK_SDP_UUID_PBAP_PCE,
BTMTK_SDP_UUID_PBAP_PSE,
BTMTK_SDP_UUID_GENERIC_AUDIO,
BTMTK_SDP_UUID_MAP,
BTMTK_SDP_UUID_INSECURE,
BTMTK_SDP_UUID_SECURE,
BTMTK_SDP_UUID_FAX,
BTMTK_SDP_UUID_IRMC_SYNC,
0};

const unsigned short g_sdp_uuid_le[] = {
BTMTK_SDP_UUID_PRX,
0};

typedef enum
{
    BTMTK_SDP_ELEM_UNSIGNED_INT,
    BTMTK_SDP_ELEM_SIGNED_INT,
    BTMTK_SDP_ELEM_UUID,
    BTMTK_SDP_ELEM_TEXT,
    BTMTK_SDP_ELEM_BOOL,
    BTMTK_SDP_ELEM_SEQUENCE,
    BTMTK_SDP_ELEM_ALTERNATIVE,
    BTMTK_SDP_ELEM_URL
} btmtk_sdp_element_type;

#define BTMTK_SDP_DESC_UNSIGNED_INT 0x08  /* = 1 << 3 */
#define BTMTK_SDP_DESC_SIGNED_INT 0x10    /* = 2 << 3 */
#define BTMTK_SDP_DESC_UUID 0x18          /* = 3 << 3 */
#define BTMTK_SDP_DESC_TEXT 0x20          /* = 4 << 3 */
#define BTMTK_SDP_DESC_BOOL 0x28          /* = 5 << 3 */
#define BTMTK_SDP_DESC_SEQUENCE 0x30      /* = 6 << 3 */
#define BTMTK_SDP_DESC_ALTERNATIVE 0x38   /* = 7 << 3 */
#define BTMTK_SDP_DESC_URL 0x40           /* = 8 << 3 */ 

#define BTMTK_SDP_DESC_SIZE_1_B 0
#define BTMTK_SDP_DESC_SIZE_2_B 1
#define BTMTK_SDP_DESC_SIZE_4_B 2
#define BTMTK_SDP_DESC_SIZE_8_B 3
#define BTMTK_SDP_DESC_SIZE_16_B 4
#define BTMTK_SDP_DESC_SIZE_IN_NEXT_B 5
#define BTMTK_SDP_DESC_SIZE_IN_NEXT_2B 6
#define BTMTK_SDP_DESC_SIZE_IN_NEXT_4B 7

#define SDP_WRITE_8BIT(buf, idx, value)  {buf[idx++] = value;}
#define SDP_WRITE_16BIT(buf, idx, value) {buf[idx++] = (U8)((value & 0xff00) >> 8);  /* Bits[15:8] of size */ \
                                          buf[idx++] = (U8)(value & 0x00ff);         /* Bits[7:0] of size */}
#define SDP_WRITE_32BIT(buf, idx, value) {buf[idx++] = (U8)((value & 0xff000000) >> 24);  /* Bits[32:24] of size */\
                                          buf[idx++] = (U8)((value & 0x00ff0000) >> 16);  /* Bits[23:16] of size */\
                                          buf[idx++] = (U8)((value & 0x0000ff00) >> 8);   /* Bits[15:8] of size */\
                                          buf[idx++] = (U8)(value & 0x000000ff);          /* Bits[7:0] of size */}

#define SDP_ATTR_SERVICE_CLASS_ID_LIST 0x0001
#define SDP_ATTR_PROTOCOL_DESC_LIST    0x0004
#define SDP_ATTR_SERVICE_NAME          (0x0000+0x0100)

#define SDP_PROT_L2CAP  0x0100
#define SDP_PROT_RFCOMM 0x0003


//static jfieldID field_mIsDiscovering;

extern void create_prop_array(JNIEnv *env, jobjectArray strArray, Properties *property,
                       property_value *value, int len, int *array_index );
/** Get native data stored in the opaque (Java code maintained) pointer mNativeData
 *  Perform quick sanity check, if there are any problems return NULL
 */
bool btmtk_gap_set_local_name(native_data_t *nat, char *name, int namelen);
bool btmtk_gap_set_scanable_mode(native_data_t *nat, btbm_scan_enable_type scanablemode);
extern btmtk_sdp_req_struct *btmtk_util_find_sdp_request(event_loop_native_data_t *nat, bt_addr_struct *addr);

/* Sometimes we need to trigger indication here to event loop */
static bool bt_sendind(native_data_t *nat, void* ptr, int size)
{
    int ret;
    ilm_struct *ilm;
    struct sockaddr_un name;
    socklen_t namelen;

    
    /*
    name.sun_family = AF_UNIX;
    strcpy (name.sun_path, BT_SOCK_NAME_EXT_ADP);
    namelen = (offsetof (struct sockaddr_un, sun_path) + strlen (name.sun_path) + 1);
    */
    socket_make_sockaddr_un(BT_SOCK_NAME_EXT_ADP, ANDROID_SOCKET_NAMESPACE_ABSTRACT, (struct sockaddr_un*)&name, &namelen);
    
    size = sizeof(ilm_struct) - MAX_ILM_BUFFER_SIZE + size;
    ALOGI("[JNI] bt_sendind(ptr=0x%X, len=%d)", ptr, size);

    ret = -1;
    ilm = (ilm_struct*)ptr;
    ilm->src_mod_id = MOD_MMI;
    ilm->dest_mod_id = MOD_BT;
    ALOGI("[JNI] send ind=%d", ilm->msg_id);
    if(nat->servsock >= 0)
    {
        ret = sendto(nat->servsock, ptr, size, 0, (const sockaddr*)&name, namelen);
        if(ret < 0)
        {
            ALOGE("[JNI] send ind fail : %s, %d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[JNI] send ind success : %d", ret);
        }
    }
    else
    {
        ALOGE("[JNI] server socket uninitialized");
    }
    return (ret >= 0)?true:false;
}

/* GAP function */
static void btmtk_gap_send_name_change_event(native_data_t *nat, char *name, int namelen)
{
    ev_struct event;
    btmtk_android_gap_name_event_struct *ptr;
    
    ALOGI("[GAP] btmtk_gap_send_name_change_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_GAP_LOCAL_NAME_CHANGE;
    ptr = (btmtk_android_gap_name_event_struct *) event.ilm_data;
    ptr->name_len = namelen;
    strncpy(ptr->name, name, namelen);
    if (bt_sendind(nat, &event, sizeof(btmtk_android_gap_name_event_struct)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_GAP_LOCAL_NAME_CHANGE failed");
    }
}

static void btmtk_gap_send_scan_mode_change_event(native_data_t *nat, unsigned char mode)
{
    ev_struct event;
    
    ALOGI("[GAP] btmtk_gap_send_scan_mode_change_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_GAP_SCAN_MODE_CHANGE;
    event.ilm_data[0] = mode;
    if (bt_sendind(nat, &event, sizeof(unsigned char)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_GAP_SCAN_MODE_CHANGE failed");
    }
}

static void btmtk_gap_send_scan_mode_timeout_change_event(native_data_t *nat, unsigned int timeout)
{
    ev_struct event;
    
    ALOGI("[GAP] btmtk_gap_send_scan_mode_timeout_change_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_GAP_SCAN_MODE_TIMEOUT_CHANGE;
    *(unsigned int *)event.ilm_data = timeout;
    if (bt_sendind(nat, &event, sizeof(unsigned int)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_GAP_SCAN_MODE_TIMOUET_CHANGE failed");
    }
}


static void btmtk_gap_send_discovery_start_event(native_data_t *nat)
{
    ev_struct event;

    ALOGI("[GAP] btmtk_gap_send_discovery_start_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_GAP_DISCOVERY_START;
    if(bt_sendind(nat, (void*)&event, 0) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_GAP_DISCOVERY_START failed");
    }
}

static void btmtk_gap_send_discovery_stop_event(native_data_t *nat)
{
    ev_struct event;

    ALOGI("[GAP] btmtk_gap_send_discovery_stop_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_GAP_DISCOVERY_STOP;
    if (bt_sendind(nat, (void*)&event, 0) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_GAP_DISCOVERY_STOP failed");
    }
}

static void btmtk_gap_send_sdp_device_create_event(native_data_t *nat, bt_addr_struct *addr)
{
    ev_struct event;

    ALOGI("[GAP] btmtk_gap_send_device_create_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_SDP_DEVICE_CREATE;
    memcpy(event.ilm_data, addr, sizeof(bt_addr_struct));
    if (bt_sendind(nat, (void*)&event, sizeof(bt_addr_struct)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_GAP_DEVICE_CREATE failed");
    }
}

static void btmtk_gap_send_sdp_paired_device_create_event(native_data_t *nat, bt_addr_struct *addr)
{
    ev_struct event;

    ALOGI("[GAP] btmtk_gap_send_sdp_paired_device_create_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_SDP_PAIRED_DEVICE_CREATE;
    memcpy(event.ilm_data, addr, sizeof(bt_addr_struct));
    if (bt_sendind(nat, (void*)&event, sizeof(bt_addr_struct)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_SDP_PAIRED_DEVICE_CREATE failed");
    }
}

static void btmtk_gap_send_sdp_paired_device_remove_event(native_data_t *nat, bt_addr_struct *addr)
{
    ev_struct event;

    ALOGI("[GAP] btmtk_gap_send_sdp_paired_device_remove_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_SDP_PAIRED_DEVICE_REMOVED;
    memcpy(event.ilm_data, addr, sizeof(bt_addr_struct));
    if (bt_sendind(nat, (void*)&event, sizeof(bt_addr_struct)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_SDP_PAIRED_DEVICE_REMOVED failed");
    }
}


static void btmtk_gap_send_device_trusted_event(native_data_t *nat, bt_addr_struct *addr, bool trusted)
{
    ev_struct event;
    btmtk_android_gap_device_trusted_struct *ptr;

    ALOGI("[GAP] btmtk_gap_send_device_trusted_event: trusted=%d", trusted);
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_DEVICE_TRUSTED;
    ptr = (btmtk_android_gap_device_trusted_struct *) event.ilm_data;
	ptr->addr= *addr;
    ptr->trusted = trusted;
    if (bt_sendind(nat, (void*)&event, sizeof(btmtk_android_gap_device_trusted_struct)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_DEVICE_TRUSTED failed");
    }
}

static void btmtk_gap_send_paired_device_rename_event(native_data_t *nat, bt_addr_struct *addr, char *name)
{
	ev_struct event;
    btmtk_android_gap_remote_name_event_struct *ptr;

    ALOGI("[GAP] btmtk_gap_send_paired_device_rename_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id =ANDROID_EV_GAP_PAIRED_DEVICE_RENAME;	
    ptr = (btmtk_android_gap_remote_name_event_struct *) event.ilm_data;
	ptr->addr= *addr;
	ptr->name_len = (strlen(name) > BTBM_ADP_MAX_NAME_LEN) ? BTBM_ADP_MAX_NAME_LEN : strlen(name);
    strncpy(ptr->name, name, ptr->name_len);

	ALOGI("name is %s,len is %d",ptr->name,ptr->name_len);

	
    if (bt_sendind(nat, (void*)&event, sizeof(btmtk_android_gap_remote_name_event_struct)) == false)
    {
        ALOGE("[GAP] Send btmtk_gap_send_paired_device_rename_event failed");
    }
}


static void btmtk_gap_send_sdp_discover_event(native_data_t *nat, bt_addr_struct *addr, const char *pattern, int pattern_len)
{
    ev_struct event;
    btmtk_android_sdp_service_search_event_struct *ptr;

    ALOGI("[GAP] btmtk_gap_send_sdp_discover_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_SDP_DISCOVER;
    ptr = (btmtk_android_sdp_service_search_event_struct *)event.ilm_data;
    
    memcpy(&ptr->addr, addr, sizeof(bt_addr_struct));
    memcpy(ptr->pattern, pattern,  pattern_len);
    ptr->pattern_len = pattern_len;
    if (bt_sendind(nat, (void*)&event, sizeof(btmtk_android_sdp_service_search_event_struct)) == false)
    {
        ALOGE("[GAP] Send ANDROID_EV_SDP_DISCOVER failed");
    }
}

static void btmtk_gap_send_power_state_change_event(native_data_t *nat, btmtk_power_state_enum state)
{
    ev_struct event;

    ALOGI("[GAP] btmtk_gap_send_power_state_change_event");
    memset(&event, 0x0, sizeof(ev_struct));
    event.msg_id = ANDROID_EV_GAP_POWER_STATE_CHANGE;
    
    *(unsigned int *)event.ilm_data = state;
    if (bt_sendind(nat, &event, sizeof(unsigned int)) == false)
    {
        ALOGE("[GAP] Send btmtk_gap_send_power_state_change_event failed");
    }
}


static void btmtk_gap_init(JNIEnv *env, jobject object)
{
    native_data_t *nat = get_native_data(env, object);
    event_loop_native_data_t *event_nat =
        get_EventLoop_native_data(env, env->GetObjectField(object, field_mEventLoop));
  //  const char *name;
	char name[BTBM_ADP_MAX_NAME_LEN] = {'\0'};
	btbm_scan_enable_type type;

    nat->event_nat = event_nat;
	pthread_mutex_lock(&(nat->event_nat->thread_mutex));
    event_nat->service_nat = nat;
    event_nat->activity = BTMTK_GAP_ACT_NONE;
    event_nat->requests = NULL;
  //  event_nat->host_cache.scan_mode = BTBM_ADP_P_ON_I_OFF;
  //  name = event_nat->host_cache.name;
  	strncpy(name, event_nat->host_cache.name, BTBM_ADP_MAX_NAME_LEN-1);
  	type = (btbm_scan_enable_type)event_nat->host_cache.scan_mode;
	pthread_mutex_unlock(&(nat->event_nat->thread_mutex));
    btmtk_gap_set_local_name(nat, (char *)name, strlen(name));
    btmtk_gap_set_scanable_mode(nat, type);
}

static void btmtk_gap_deinit(JNIEnv *env, jobject object)
{
    native_data_t *nat = get_native_data(env, object);

/*	pthread_mutex_lock(&(nat->event_nat->thread_mutex));
    nat->event_nat->activity = BTMTK_GAP_ACT_NONE;

    while (nat->event_nat->requests)
    {
        btmtk_util_list_remove((btmtk_list_header_struct **)&nat->event_nat->requests, (btmtk_list_header_struct *)nat->event_nat->requests);
    }
    nat->event_nat->requests = NULL;
	pthread_mutex_unlock(&(nat->event_nat->thread_mutex));
	*/

	btmtk_gap_send_power_state_change_event(nat, BTMTK_POWER_STATE_OFF);

	
}

bool btmtk_gap_power_on(native_data_t *nat)
{
    bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_power_on");

    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_POWERON_REQ;
	
	pthread_mutex_lock(&(nat->thread_mutex));
    if (bt_sendmsg(nat->servsock, (void*)&ilm, 0) &&
        wait_response(nat->servsock, MSG_ID_BT_POWERON_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_poweron_cnf_struct *cnf_p;
        cnf_p = (bt_poweron_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_POWERON_CNF: result=%d", cnf_p->result);
        ret = cnf_p->result;
    }
	
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_power_on failed");
    }
    return ret;
}

bool btmtk_gap_power_off(native_data_t *nat)
{
    bool ret= false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_power_off");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_POWEROFF_REQ;
	
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) &&
       wait_response(nat->servsock, MSG_ID_BT_POWEROFF_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_poweroff_cnf_struct *cnf_p;
        cnf_p = (bt_poweroff_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_POWEROFF_CNF: result=%d", cnf_p->result);
        ret = cnf_p->result;
    }	
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_power_off failed");
    }
    return ret;
}

bool btmtk_gap_get_scanable_mode(native_data_t *nat, unsigned char *scanablemode)
{
    bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_get_scanable_mode");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_READ_SCANENABLE_MODE_REQ;
	
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) &&
       wait_response(nat->servsock, MSG_ID_BT_BM_READ_SCANENABLE_MODE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_read_scanenable_mode_cnf_struct *cnf_p = (bt_bm_read_scanenable_mode_cnf_struct*)ilm.ilm_data;

        ALOGI("[GAP] MSG_ID_BT_BM_READ_SCANENABLE_MODE_CNF: result=%d, mode=%d", 
              cnf_p->result, cnf_p->modeconnected);
        if(cnf_p->result == BTBM_ADP_SUCCESS)
        {
            *scanablemode = cnf_p->modeconnected;
            ret = true;
        }
    }	
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_get_scanable_mode failed");
    }
    return ret;
}

bool btmtk_gap_set_scanable_mode(native_data_t *nat, btbm_scan_enable_type scanablemode)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_write_scanenable_mode_req_struct *msg_p;
        
    ALOGI("[GAP] btmtk_gap_set_scanable_mode: mode=0x%X", scanablemode);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    msg_p = (bt_bm_write_scanenable_mode_req_struct*)ilm.ilm_data;
    ilm.msg_id = MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_REQ;
    msg_p->mode = scanablemode;
    #ifdef MTK_BT_40_SUPPORT
    unsigned short scan_mode = (unsigned short)scanablemode;
    scan_mode &= 0xFF;
    scan_mode |= (scan_mode<<8);
    msg_p->mode = (btbm_scan_enable_type)scan_mode;
    #endif
	
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_write_scanenable_mode_req_struct)) && 
       wait_response(nat->servsock, MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_write_scanenable_mode_cnf_struct *cnf_p = (bt_bm_write_scanenable_mode_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_BM_WRITE_SCANENABLE_MODE_CNF: result=%d", cnf_p->result);
        if (cnf_p->result == BTBM_ADP_SUCCESS)
        {
            if (nat->state != BTMTK_POWER_STATE_TURNING_ON)
            {
                btmtk_gap_send_scan_mode_change_event(nat, scanablemode);
            }
            ret = true;
        }
    }	
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_set_scanable_mode failed");
    }
    return ret;
}

bool btmtk_gap_get_local_cod(native_data_t *nat, unsigned long *cod)
{
    bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_get_local_cod");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_READ_LOCAL_COD_REQ;
	
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) &&
       wait_response(nat->servsock, MSG_ID_BT_BM_READ_LOCAL_COD_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_read_local_cod_cnf_struct *cnf_p = (bt_bm_read_local_cod_cnf_struct*)ilm.ilm_data;
        
        ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_COD_CNF is received : result=%d, CoD=0x%X", cnf_p->result, cnf_p->cod);
        if(cnf_p->result == BTBM_ADP_SUCCESS)
        {
            *cod = cnf_p->cod;
            ret = true;
        }
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_get_local_cod failed");
    }
    return ret;
}

bool btmtk_gap_set_local_cod(native_data_t *nat, unsigned long cod)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_write_local_cod_req_struct *msg_p;
        
    ALOGI("[GAP] btmtk_gap_set_local_cod: cod = 0x%X", cod);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    msg_p = (bt_bm_write_local_cod_req_struct*)ilm.ilm_data;
    ilm.msg_id = MSG_ID_BT_BM_WRITE_LOCAL_COD_REQ;
    msg_p->cod = cod;
    msg_p->write_type = BTBM_WRITE_COD_NEW;
    
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_write_local_cod_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_BM_WRITE_LOCAL_COD_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT)  > 0)
    {
        bt_bm_write_local_cod_cnf_struct *cnf_p = (bt_bm_write_local_cod_cnf_struct*)ilm.ilm_data;;

        ALOGI("[GAP] MSG_ID_BT_BM_WRITE_LOCAL_COD_CNF: result=%d", cnf_p->result);
        ret = (cnf_p->result == BTBM_ADP_SUCCESS);
    }
	pthread_mutex_unlock(&(nat->thread_mutex));
    
    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_set_local_cod failed");
    }
    return ret;
}

/* namelen will return length of the local name */
bool btmtk_gap_get_local_name(native_data_t *nat, char *name, int *namelen)
{
    bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_get_local_name");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_READ_LOCAL_NAME_REQ;
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) && 
       wait_response(nat->servsock, MSG_ID_BT_BM_READ_LOCAL_NAME_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_read_local_name_cnf_struct *cnf_p = (bt_bm_read_local_name_cnf_struct*)ilm.ilm_data;
        
        ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_NAME_CNF: name=%s, namelen=%d", cnf_p->name, cnf_p->name_len);
        if (cnf_p->name_len)
        {
            strncpy(name, (char *)cnf_p->name, *namelen);
        }
        *namelen = cnf_p->name_len;
        ret = true;
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_get_local_name failed");
    }
    return ret;
}

bool btmtk_gap_set_local_name(native_data_t *nat, char *name, int namelen)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_write_local_name_req_struct *msg_p;
        
    ALOGI("[GAP] btmtk_gap_set_local_name : name=%s, len=%d", name, namelen);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    namelen = (namelen > BTBM_ADP_MAX_NAME_LEN) ? BTBM_ADP_MAX_NAME_LEN : namelen;
    msg_p = (bt_bm_write_local_name_req_struct*)ilm.ilm_data;
    ilm.msg_id = MSG_ID_BT_BM_WRITE_LOCAL_NAME_REQ;
    strncpy((char *)msg_p->name, name, namelen);
    msg_p->name_len = namelen;

	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_write_local_name_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_BM_WRITE_LOCAL_NAME_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_write_local_name_cnf_struct *cnf_p = (bt_bm_write_local_name_cnf_struct*)ilm.ilm_data;
        
        ALOGI("[GAP] MSG_ID_BT_BM_WRITE_LOCAL_NAME_CNF: result=%d", cnf_p->result);
        if(cnf_p->result == BTBM_ADP_SUCCESS)
        {
            if (nat->state != BTMTK_POWER_STATE_TURNING_ON)
            {
                btmtk_gap_send_name_change_event(nat, name, namelen);
            }
            ret = true;
        }
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_set_local_name failed");
    }

    return ret;
}

bool btmtk_gap_get_local_addr(native_data_t *nat, char *buf)
{
    bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_get_local_addr");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_READ_LOCAL_ADDR_REQ;
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) && 
       wait_response(nat->servsock, MSG_ID_BT_BM_READ_LOCAL_ADDR_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_read_local_addr_cnf_struct *cnf_p = (bt_bm_read_local_addr_cnf_struct*)ilm.ilm_data;
        btmtk_util_convert_bdaddr2string(buf, &cnf_p->bd_addr);
        ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_ADDR_CNF: result=%d, address=%s", cnf_p->result, buf);
        ret = (cnf_p->result == BTBM_ADP_SUCCESS);
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_get_local_name failed");
    }
    return ret;
}

bool btmtk_gap_local_uuid(native_data_t *nat, unsigned short buf[], int * len)
{
	bool ret = false;
    ilm_struct ilm;

    ALOGI("[GAP] btmtk_gap_local_uuid");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_READ_LOCAL_UUID_REQ;
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) && 
       wait_response(nat->servsock, MSG_ID_BT_BM_READ_LOCAL_UUID_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
    	bt_bm_read_local_uuid_cnf_struct *cnf_p = (bt_bm_read_local_uuid_cnf_struct*)ilm.ilm_data;

		*len = btmtk_util_convert_uuidlist_2_uuid16(cnf_p->service_list1,
											 cnf_p->service_list2,
											 cnf_p->service_list3,
											 cnf_p->service_list4,
											 cnf_p->service_list5,
											 cnf_p->service_list6,
											 buf);
        ALOGI("[GAP] MSG_ID_BT_BM_READ_LOCAL_UUID_CNF: result=%d", cnf_p->result);
        ret = (cnf_p->result == BTBM_ADP_SUCCESS);
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_get_local_uuid failed");
    }
    return ret;
	
}



bool btmtk_gap_get_remote_name_cod(native_data_t *nat, btbm_bd_addr_t *bd_addr, char *name, unsigned long *namelen,  unsigned long *cod, unsigned long timeout)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_read_remote_name_req_struct* msg_p;
        
    ALOGI("[GAP] btmtk_gap_get_remote_name_cod: addr=%X:%X:%X", bd_addr->lap, bd_addr->uap, bd_addr->nap);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_READ_REMOTE_NAME_REQ;
    msg_p = (bt_bm_read_remote_name_req_struct*)ilm.ilm_data;    
    msg_p->bd_addr = *bd_addr;

	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_read_remote_name_req_struct)) && 
       wait_response(nat->servsock, MSG_ID_BT_BM_READ_REMOTE_NAME_CNF, &ilm, timeout) > 0)
    {
        bt_bm_read_remote_name_cnf_struct *cnf_p = (bt_bm_read_remote_name_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_BM_READ_REMOTE_NAME_CNF: result=%d, name=%s, cod=0x%X", cnf_p->result, cnf_p->name, cnf_p->cod);
        if(cnf_p->result == BTBM_ADP_SUCCESS)
        {
            strncpy(name, (char *)cnf_p->name, *namelen);
            *namelen = (cnf_p->name_len > *namelen) ? *namelen: cnf_p->name_len;
            *cod = cnf_p->cod;
            ret = true;
        }
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret) 
    {
        ALOGE("[GAP] btmtk_gap_get_remote_name_cod failed");
    }
    return ret;
}

void btmtk_gap_discovery_cancel(native_data_t *nat)
{
    ilm_struct ilm;
	
    ALOGI("[GAP] btmtk_gap_discovery_cancel");
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_DISCOVERY_CANCEL_REQ;
	pthread_mutex_lock(&(nat->thread_mutex));
    if((nat->event_nat->activity & BTMTK_GAP_ACT_INQUIRY) == 0)
    {
        ALOGI("[GAP] btmtk_gap_discovery_cancel already cancelled");
        btmtk_gap_send_discovery_stop_event(nat);
    }
    else if(bt_sendmsg(nat->servsock, (void*)&ilm, 0) &&
       wait_response(nat->servsock, MSG_ID_BT_BM_DISCOVERY_CANCEL_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_bm_discovery_cancel_cnf_struct *msg = (bt_bm_discovery_cancel_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_BM_DISCOVERY_CANCEL_CNF: result=%d, total=%d", msg->result, msg->total_number);
        btmtk_gap_send_discovery_stop_event(nat);
    }
	pthread_mutex_unlock(&(nat->thread_mutex));
}

bool btmtk_gap_discovery(native_data_t *nat, btbm_discovery_type mode)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_discovery_req_struct *req_p;

    ALOGI("[GAP] btmtk_gap_discovery");

	if(nat->event_nat->activity & BTMTK_GAP_ACT_INQUIRY)
	{
		return true;
	}
	
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_DISCOVERY_REQ;
    req_p = (bt_bm_discovery_req_struct*)ilm.ilm_data;
    req_p->cod = 0xFFFFFFFF;
    req_p->inquiry_length = 0x30;
    req_p->inquiry_number = 25;//0xFF;
    req_p->discovery_mode = BTBM_DISCOVERY_WITH_NAME;
    req_p->access_mode = BTBM_GENERAL_INQUIRY;
    req_p->discovery_type = mode;
    btmtk_gap_send_discovery_start_event(nat);
    ret = true;
    if (!bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_discovery_req_struct)))
    {
        btmtk_gap_send_discovery_stop_event(nat);
        ret = false;
    }
    return ret;
}

bool btmtk_gap_bond(native_data_t *nat, bt_addr_struct *addr, bool do_sdp)
{
    ilm_struct ilm;
    bt_bm_bonding_req_struct *req_p;
    int idx = 0, idx_le;

    ALOGI("[GAP] btmtk_gap_bond: addr=%X:%X:%X, do_sdp=%d", addr->lap, addr->uap, addr->nap, do_sdp);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_BONDING_REQ;
    req_p = (bt_bm_bonding_req_struct*)ilm.ilm_data;
    memcpy(&req_p->bd_addr, addr, sizeof(bt_addr_struct));
    if (do_sdp)
    {
   //     btmtk_device_entry_struct *inquired_entry = btmtk_inquired_dev_cache_find(nat->event_nat, addr);
   //     if (inquired_entry != NULL && inquired_entry->device_type != BTBM_DEVICE_TYPE_LE)
        {
            for (idx = 0; idx < BTBM_ADP_MAX_SDAP_UUID_NO && g_sdp_uuid[idx] != 0; idx++)
            {
                req_p->sdap_uuid[idx] = (kal_uint32)g_sdp_uuid[idx];
            }
            for (idx_le = 0; idx < BTBM_ADP_MAX_SDAP_UUID_NO && g_sdp_uuid_le[idx_le] != 0; idx_le++, idx++)
            {
                req_p->sdap_uuid[idx] = (kal_uint32)g_sdp_uuid_le[idx_le];
            }
        }
        req_p->sdap_len = idx;
    }
    else
    {
        req_p->sdap_len = 0;
    }
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_bonding_req_struct));
}

bool btmtk_gap_bond_cancel(native_data_t *nat, bt_addr_struct *addr)
{
    ilm_struct ilm;
    bool ret = false;
    bt_bm_bonding_cancel_req_struct *req_p;

    ALOGI("[GAP] btmtk_gap_bond_cancel: addr=%X:%X:%X", addr->lap, addr->uap, addr->nap);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_BONDING_CANCEL_REQ;
    req_p = (bt_bm_bonding_cancel_req_struct*)ilm.ilm_data;
    memcpy(&req_p->bd_addr, addr, sizeof(bt_addr_struct));
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_bonding_cancel_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_BM_BONDING_CANCEL_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        ret = true;
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if (!ret)
    {
        ALOGI("[GAP] btmtk_gap_bond_cancel failed");
    }
    return ret ? JNI_TRUE : JNI_FALSE;
}

bool btmtk_gap_unpair(native_data_t *nat, bt_addr_struct *addr)
{
    ilm_struct ilm;
    bt_bm_delete_trust_req_struct *req_p;

    ALOGI("[GAP] btmtk_gap_unpair: addr=%X:%X:%X", addr->lap, addr->uap, addr->nap);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_DELETE_TRUST_REQ;
    req_p = (bt_bm_delete_trust_req_struct *)ilm.ilm_data;
    memcpy(&req_p->bd_addr, addr, sizeof(bt_addr_struct));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_delete_trust_req_struct));
}

bool btmtk_gap_pin_res(native_data_t *nat, bt_addr_struct *addr, const char *pin)
{
    ilm_struct ilm;
    bt_bm_pin_code_rsp_struct *ind_p;
    
    ALOGI("[GAP] btmtk_gap_pin_res: addr=%X:%X:%X, pin=%s", addr->lap, addr->uap, addr->nap, pin);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_PIN_CODE_RSP;
    ind_p = (bt_bm_pin_code_rsp_struct*)ilm.ilm_data;
    if (pin == NULL)
    {
        ind_p->pin_len = 0;
    }
    else
    {
        ind_p->pin_len = strlen(pin);
        ind_p->pin_len = (ind_p->pin_len > 16) ? 16 : ind_p->pin_len;
        strncpy((char *)ind_p->pin_code, pin, ind_p->pin_len);
    }    
    memcpy(&ind_p->bd_addr, addr, sizeof(bt_addr_struct));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_pin_code_rsp_struct));
}

bool btmtk_gap_security_user_confirm(native_data_t *nat, bt_addr_struct *addr, bool accept)
{
    ilm_struct ilm;
    bt_bm_security_user_confirm_rsp_struct *rsp_p;
    
    ALOGI("[GAP] btmtk_gap_security_user_confirm: addr=%X:%X:%X", addr->lap, addr->uap, addr->nap);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SECURITY_USER_CONFIRM_RSP;
    rsp_p = (bt_bm_security_user_confirm_rsp_struct*)ilm.ilm_data;
    rsp_p->accept = accept;
    memcpy(&rsp_p->bd_addr, addr, sizeof(bt_addr_struct));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_security_user_confirm_rsp_struct));
}

bool btmtk_gap_security_passkey_entry(native_data_t *nat, bt_addr_struct *addr, bool accept, int passkey)
{
    ilm_struct ilm;
    bt_bm_security_passkey_entry_rsp_struct *rsp_p;
    
    ALOGI("[GAP] btmtk_gap_security_passkey_entry: addr=%X:%X:%X, accept=%d, passkey=%d", addr->lap, addr->uap, addr->nap, accept, passkey);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SECURITY_PASSKEY_ENTRY_RSP;
    rsp_p = (bt_bm_security_passkey_entry_rsp_struct*)ilm.ilm_data;
    rsp_p->accept = accept;
	rsp_p->passkey = passkey;
    memcpy(&rsp_p->bd_addr, addr, sizeof(bt_addr_struct));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_security_passkey_entry_rsp_struct));
}

bool btmtk_gap_security_passkey_notify(native_data_t *nat, bt_addr_struct *addr, bool accept)
{
    ilm_struct ilm;
    bt_bm_security_passkey_notify_rsp_struct *rsp_p;
    
    ALOGI("[GAP] btmtk_gap_security_passkey_notify: addr=%X:%X:%X, accept=%d", addr->lap, addr->uap, addr->nap, accept);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SECURITY_PASSKEY_NOTIFY_RSP;
    rsp_p = (bt_bm_security_passkey_notify_rsp_struct *)ilm.ilm_data;
    rsp_p->accept = accept;
    memcpy(&rsp_p->bd_addr, addr, sizeof(bt_addr_struct));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_security_passkey_notify_rsp_struct));
}

bool btmtk_gap_security_oob_data(native_data_t *nat, bt_addr_struct *addr, bool accept, char *hash, char *rand)
{
    ilm_struct ilm;
    bt_bm_security_oob_data_rsp_struct *rsp_p;
    
    ALOGI("[GAP] btmtk_gap_security_oob_data: addr=%X:%X:%X, accept=%d", addr->lap, addr->uap, addr->nap, accept);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SECURITY_OOB_DATA_RSP;
    rsp_p = (bt_bm_security_oob_data_rsp_struct*)ilm.ilm_data;
    rsp_p->accept = accept;
    if (hash)
    {
        memcpy(rsp_p->c, hash, 16);
    }
    if (rand)
    {
        memcpy(rsp_p->r, rand, 16);
    }
    memcpy(&rsp_p->bd_addr, addr, sizeof(bt_addr_struct));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_security_oob_data_rsp_struct));
}

bool btmtk_gap_service_search_request(native_data_t *nat, bt_addr_struct *addr)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_service_search_req_struct *req_p;
    btmtk_device_entry_struct *entry = NULL;
    int idx = 0, idx_le;

    ALOGI("[GAP] btmtk_gap_service_search_request addr=%X:%X:%X", addr->lap, addr->uap, addr->nap);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SERVICE_SEARCH_REQ;
    req_p = (bt_bm_service_search_req_struct *)ilm.ilm_data;
    memcpy(&req_p->bd_addr, addr, sizeof(bt_addr_struct));

	entry = btmtk_inquired_dev_cache_find(nat->event_nat, addr);
	if (entry == NULL) 
	{
		entry = btmtk_paired_dev_cache_find(nat->event_nat, addr);
		if (entry == NULL) 
		{
			ALOGE("The device is not in cache list");
			return FALSE;
		}
	}
	if (entry != NULL && entry->device_type != BTBM_DEVICE_TYPE_LE)
    {
        for (idx = 0; idx < BTBM_ADP_MAX_SDAP_UUID_NO && g_sdp_uuid[idx] != 0; idx++)
        {
            req_p->sdap_uuid[idx] = (kal_uint32)g_sdp_uuid[idx];
        }
        for (idx_le = 0; idx < BTBM_ADP_MAX_SDAP_UUID_NO && g_sdp_uuid_le[idx_le] != 0; idx_le++, idx++)
        {
            req_p->sdap_uuid[idx] = (kal_uint32)g_sdp_uuid_le[idx_le];
        }
    }
    req_p->sdap_len = idx;
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_service_search_req_struct));
}

bool btmtk_gap_service_attrib_search_request(native_data_t *nat, bt_addr_struct *addr, btbm_uuid_type uuid_type, unsigned short uuid16, char *uuid128, unsigned short attrib, void *value)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_search_attribute_req_struct *req_p;

    ALOGI("[GAP] btmtk_gap_service_attrib_search_request addr=%X:%X:%X, uuid_type=%d, uuid=0x%X, attrib=0x%X", addr->lap, addr->uap, addr->nap, uuid_type, uuid16, attrib);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SEARCH_ATTRIBUTE_REQ;
    req_p = (bt_bm_search_attribute_req_struct *)ilm.ilm_data;
    memcpy(&req_p->bd_addr, addr, sizeof(bt_addr_struct));
    req_p->type = uuid_type;
    req_p->sdap_uuid_16 = uuid16;
    memcpy(req_p->sdap_uuid_128, uuid128, BTMTK_SDP_UUID_128_BIT_SIZE);
    req_p->attribute_id = attrib;

	
	pthread_mutex_lock(&(nat->thread_mutex));
	/*the operation costs too much time in some cases, so turn to twin socket for TX/RX*/
    if(bt_sendmsg(nat->twinservsock, (void*)&ilm, sizeof(bt_bm_search_attribute_req_struct)) &&
       wait_response(nat->twinservsock, MSG_ID_BT_BM_SEARCH_ATTRIBUTE_CNF, &ilm, 30000) > 0)
    {
        bt_bm_search_attribute_cnf_struct *cnf_p = (bt_bm_search_attribute_cnf_struct *)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_ATTRIBUTE_CNF: result=%d", cnf_p->result);
        if (cnf_p->result == BTBM_ADP_SUCCESS)
        {
            ALOGI("[GAP] MSG_ID_BT_BM_SEARCH_ATTRIBUTE_RESULT_IND: len=%d", cnf_p->len);
            ALOGI("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", cnf_p->data[0], cnf_p->data[1], cnf_p->data[2]
                 , cnf_p->data[3], cnf_p->data[4], cnf_p->data[5], cnf_p->data[6], cnf_p->data[7], cnf_p->data[8], cnf_p->data[9]);

            switch (attrib)
            {
            case BTMTK_SDP_ATTRIB_PROTOCOL_DESC_LIST:
                if (cnf_p->len == 1)
                {
                    int i;
                    btmtk_device_entry_struct *entry = btmtk_paired_dev_cache_find(nat->event_nat, addr);
		    if (entry == NULL) 
		    {
		        entry = btmtk_inquired_dev_cache_find(nat->event_nat, addr);
		    }
                    if (entry != NULL)
		    		{
						

                    if (uuid_type == BTBM_UUID_16)
                    {
                        for (i = 0; i < entry->sdp.uuid_no; i++)
                        {
                            if (entry->sdp.uuid[i] == uuid16)
                            {
                                entry->sdp.channel[i] = cnf_p->data[0];
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (i = 0; i < entry->sdp.app_uuid_no; i++)
                        {
                            if (memcmp(entry->sdp.app_uuid[i], uuid128, BTMTK_SDP_UUID_128_BIT_SIZE))
                            {
                                entry->sdp.app_channel[i] = cnf_p->data[0];
                                break;
                            }
                        }
                    }
		    }
					
                    *(int *)value = cnf_p->data[0];
                    ret = true;
                }
                break;
            default:
                break;
            }
        }
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_service_attrib_search_request failed");
    }
    return ret;
}

bool btmtk_gap_service_search_raw_request(native_data_t *nat, bt_addr_struct *addr, const char *pattern, int pattern_size)
{
    bool ret = false;
    ilm_struct ilm;
    bt_bm_search_raw_req_struct *req_p;

    ALOGI("[GAP] btmtk_gap_service_search_raw_request addr=%X:%X:%X, size=%d", addr->lap, addr->uap, addr->nap, pattern_size);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_BM_SEARCH_RAW_REQ;
    req_p = (bt_bm_search_raw_req_struct *)ilm.ilm_data;
    memcpy(&req_p->bd_addr, addr, sizeof(bt_addr_struct));
    req_p->search_type = BT_JSR82_SERVICE_SEARCH_REQ;
    req_p->search_pattern_size = pattern_size;
    memcpy(req_p->search_pattern, pattern, MIN(BT_MAX_SDAP_RAW_SIZE, pattern_size));
    return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_bm_search_raw_req_struct));
}

int btmtk_jsr82_int_get_data_element_header(btmtk_sdp_element_type type, int size)
{
    int size_desc;
    switch (type)
    {
    case BTMTK_SDP_ELEM_BOOL:
        return (BTMTK_SDP_DESC_BOOL | BTMTK_SDP_DESC_SIZE_1_B);
    case BTMTK_SDP_ELEM_SIGNED_INT:
        switch (size)
        {
        case 1:
            return (BTMTK_SDP_DESC_SIGNED_INT | BTMTK_SDP_DESC_SIZE_1_B);
        case 2:
            return (BTMTK_SDP_DESC_SIGNED_INT | BTMTK_SDP_DESC_SIZE_2_B);
        case 4:
            return (BTMTK_SDP_DESC_SIGNED_INT | BTMTK_SDP_DESC_SIZE_4_B);
        case 8:
            return (BTMTK_SDP_DESC_SIGNED_INT | BTMTK_SDP_DESC_SIZE_8_B);
        case 16:
            return (BTMTK_SDP_DESC_SIGNED_INT | BTMTK_SDP_DESC_SIZE_16_B);
        default:
            return 0;
        }
    case BTMTK_SDP_ELEM_UNSIGNED_INT:
        switch (size)
        {
        case 1:
            return (BTMTK_SDP_DESC_UNSIGNED_INT | BTMTK_SDP_DESC_SIZE_1_B);
        case 2:
            return (BTMTK_SDP_DESC_UNSIGNED_INT | BTMTK_SDP_DESC_SIZE_2_B);
        case 4:
            return (BTMTK_SDP_DESC_UNSIGNED_INT | BTMTK_SDP_DESC_SIZE_4_B);
        case 8:
            return (BTMTK_SDP_DESC_UNSIGNED_INT | BTMTK_SDP_DESC_SIZE_8_B);
        case 16:
            return (BTMTK_SDP_DESC_UNSIGNED_INT | BTMTK_SDP_DESC_SIZE_16_B);
        default:
            return 0;
        }
    case BTMTK_SDP_ELEM_UUID:
        switch (size)
        {
        case 2:
            return (BTMTK_SDP_DESC_UUID | BTMTK_SDP_DESC_SIZE_2_B);
        case 4:
            return (BTMTK_SDP_DESC_UUID | BTMTK_SDP_DESC_SIZE_4_B);
        case 16:
            return (BTMTK_SDP_DESC_UUID | BTMTK_SDP_DESC_SIZE_16_B);
        default:
            return 0;
        }
    default:
        if (size < 0xFF)
        {
            size_desc = BTMTK_SDP_DESC_SIZE_IN_NEXT_B;
        }
        else if (size < 0xFFFF)
        {
            size_desc = BTMTK_SDP_DESC_SIZE_IN_NEXT_2B;
        }
        else
        {
            size_desc = BTMTK_SDP_DESC_SIZE_IN_NEXT_4B;
        }
    }

    switch (type)
    {
    case BTMTK_SDP_ELEM_TEXT:
        return (BTMTK_SDP_DESC_TEXT | size_desc);
    case BTMTK_SDP_ELEM_URL:
        return (BTMTK_SDP_DESC_URL | size_desc);
    case BTMTK_SDP_ELEM_SEQUENCE:
        return (BTMTK_SDP_DESC_SEQUENCE | size_desc);
    case BTMTK_SDP_ELEM_ALTERNATIVE:
        return (BTMTK_SDP_DESC_ALTERNATIVE | size_desc);
    default:
            return 0;
    }
    return 0;
}

int btmtk_jsr82_int_write_size_bytes(char *buf, int size)
{
    int idx = 0;
    if (size < 0xFF)
    {
        SDP_WRITE_8BIT(buf, idx, size);
    }
    else if (size < 0xFFFF)
    {
        SDP_WRITE_16BIT(buf, idx, size);
    }
    else
    {
        SDP_WRITE_32BIT(buf, idx, size);
    }
    return idx;
}

/*****************************************************************************
 * FUNCTION
 *  btmtk_jsr82_int_compose_record
 * DESCRIPTION
 *  Compose record from CreateInfo
 * PARAMETERS
 *  rec_buf           [OUT]        
 *  CreateInfo        [IN]        
 *  rec_buf_size      [IN]        
 * RETURNS
 *  Size in bytes
 *****************************************************************************/
int btmtk_jsr82_int_compose_record(char **buf, const char *name, char *uuid, short channel)
{
    int idx = 0, size = 0, name_len = 0, proto_list_size = (2 + 3) + (2 + 3 + 2);
    char *record;

    /* Name attr size */
    if (name && (name_len = strlen(name)) > 0)
    {
        size = strlen(name) + 1 + 3;
    }
    /* UUID attr size */
    size += (2 + 2 + 1 + BTMTK_SDP_UUID_128_BIT_SIZE);
    /* Protocol description attr size */
    size += proto_list_size;
    if ((*buf = (char*)malloc(size + 50)) == NULL)
    {
        return NULL;
    }
    ALOGI("btmtk_jsr82_int_compose_record: calculated size=%d ", size);
    
	memset(*buf, 0x0, (size + 50));
    
    record = *buf;
    /* UUID */
    /* 2 */ SDP_WRITE_16BIT(record, idx, SDP_ATTR_SERVICE_CLASS_ID_LIST);
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_SEQUENCE, BTMTK_SDP_UUID_128_BIT_SIZE +1); 
    /* 1 */ idx += btmtk_jsr82_int_write_size_bytes(record + idx, BTMTK_SDP_UUID_128_BIT_SIZE + 1);
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_UUID, BTMTK_SDP_UUID_128_BIT_SIZE);
    /* 16 */ memcpy(record + idx, uuid, BTMTK_SDP_UUID_128_BIT_SIZE);
    idx += BTMTK_SDP_UUID_128_BIT_SIZE;

    /* PROTOCOL DESCRIPTION LIST*/
    /* 2 */ SDP_WRITE_16BIT(record, idx, SDP_ATTR_PROTOCOL_DESC_LIST);
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_SEQUENCE, proto_list_size); 
    /* 1 */ idx += btmtk_jsr82_int_write_size_bytes(record + idx, proto_list_size);

    /* L2CAP */
    proto_list_size = 3;
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_SEQUENCE, proto_list_size); 
    /* 1 */ idx += btmtk_jsr82_int_write_size_bytes(record + idx, proto_list_size);
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_UUID, BTMTK_SDP_UUID_16_BIT_SIZE);
    /* 2 */ SDP_WRITE_16BIT(record, idx, SDP_PROT_L2CAP);
    /* RFCOMM + channel */
    proto_list_size = 3 + 2;
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_SEQUENCE, proto_list_size); 
    /* 1 */ idx += btmtk_jsr82_int_write_size_bytes(record + idx, proto_list_size);
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_UUID, BTMTK_SDP_UUID_16_BIT_SIZE);
    /* 2 */ SDP_WRITE_16BIT(record, idx, SDP_PROT_RFCOMM);
    /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_UNSIGNED_INT, 1);
    /* 1 */ SDP_WRITE_8BIT(record, idx, channel & 0xFF);

    /* NAME */
    if (name_len)
    {
        /* 2 */ SDP_WRITE_16BIT(record, idx, SDP_ATTR_SERVICE_NAME);
        /* 1 */ record[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_TEXT, name_len + 1);
        /* 1 */ idx += btmtk_jsr82_int_write_size_bytes(record + idx, name_len + 1);
        /* namelen */ strncpy(record + idx, name, name_len);
        idx += (name_len + 1);
    }

    if (idx >= size + 50)
    {
        ALOGE("error!!!! btmtk_jsr82_int_compose_record: size=%d >= malloc size=%d", idx, size + 50);
    }
    ALOGI("btmtk_jsr82_int_compose_record: size=%d ", idx);
    return idx;
}

int btmtk_jsr82_create_record(native_data_t *nat, char **rec, int rec_len)
{
    ilm_struct ilm;
	int handle = -1;

    ALOGI("[GAP] btmtk_jsr82_create_record rec_len=%d", rec_len);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    ilm.msg_id = MSG_ID_BT_APP_SDPDB_GET_HANDLE_REQ;
	pthread_mutex_lock(&(nat->thread_mutex));
    if (bt_sendmsg(nat->servsock, (void*)&ilm, 0) &&
        wait_response(nat->servsock, MSG_ID_BT_APP_SDPDB_GET_HANDLE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_app_sdpdb_get_handle_cnf_struct *cnf_p;
        cnf_p = (bt_app_sdpdb_get_handle_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_APP_SDPDB_GET_HANDLE_CNF: result=%d, handle=%d", cnf_p->result, cnf_p->handle);
        if (cnf_p->result == BTSDPDBAPP_SUCCESS)
        {
            ilm_struct ilm2;
            bt_app_sdpdb_register_req_struct *req_p;

            memset(&ilm2, 0x0, sizeof(ilm_struct));
            ilm2.msg_id = MSG_ID_BT_APP_SDPDB_REGISTER_REQ;
            req_p = (bt_app_sdpdb_register_req_struct *)ilm2.ilm_data;
            req_p->handle = cnf_p->handle;
            req_p->type = BT_APP_REGISTER_RECORD;
            if (rec_len > BT_SDPDB_MAX_SERVICE_RECORD_SIZE)
            {
                ALOGE("[GAP] rec_len=%d > BT_SDPDB_MAX_SERVICE_RECORD_SIZE", rec_len);
          /*      free(*rec);
                *rec = NULL;
                return -1;
                */
                goto exit;
            }
            memcpy(req_p->record_raw, *rec, MIN(BT_SDPDB_MAX_SERVICE_RECORD_SIZE, rec_len));
            req_p->record_raw_length = rec_len;
            if (bt_sendmsg(nat->servsock, (void*)&ilm2, sizeof(bt_app_sdpdb_register_req_struct)) &&
                wait_response(nat->servsock, MSG_ID_BT_APP_SDPDB_REGISTER_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
            {
                bt_app_sdpdb_register_cnf_struct *cnf2_p;
                cnf2_p = (bt_app_sdpdb_register_cnf_struct*)ilm.ilm_data;
                ALOGI("[GAP] MSG_ID_BT_APP_SDPDB_REGISTER_CNF: result=%d, handle=%d", cnf2_p->result, cnf2_p->handle);
                if (cnf2_p->result == BTSDPDBAPP_SUCCESS)
                {
               /*     free(*rec);
                    *rec = NULL;
                    return cnf2_p->handle;
                    */
					handle = cnf2_p->handle;
                }
            }
        }
    }
exit:
	pthread_mutex_unlock(&(nat->thread_mutex));
    free(*rec);
    *rec = NULL;
    return handle;
}

bool btmtk_jsr82_remove_record(native_data_t *nat, int handle)
{
    bool ret = false;
    ilm_struct ilm;
    bt_app_sdpdb_deregister_req_struct *req_p;

    ALOGI("[GAP] btmtk_jsr82_remove_record, handle=%d", handle);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    req_p = (bt_app_sdpdb_deregister_req_struct*)ilm.ilm_data;
    req_p->handle = handle;
    ilm.msg_id = MSG_ID_BT_APP_SDPDB_DEREGISTER_REQ;
	pthread_mutex_lock(&(nat->thread_mutex));
    if (bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_app_sdpdb_deregister_req_struct)) &&
        wait_response(nat->servsock, MSG_ID_BT_APP_SDPDB_DEREGISTER_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
    {
        bt_app_sdpdb_deregister_cnf_struct *cnf_p;
        cnf_p = (bt_app_sdpdb_deregister_cnf_struct*)ilm.ilm_data;
        ALOGI("[GAP] MSG_ID_BT_APP_SDPDB_DEREGISTER_CNF: result=%d, handle=%d", cnf_p->result, cnf_p->handle);
        ret = (cnf_p->result == BTSDPDBAPP_SUCCESS) ? true : false;
    }
	pthread_mutex_unlock(&(nat->thread_mutex));

    if(!ret)
    {
        ALOGE("[GAP] btmtk_jsr82_remove_record failed");
    }
    return ret;
}

bool btmtk_gap_set_ssp_debug_mode(native_data_t *nat, bool on)
{
    bool ret = false;
    ilm_struct ilm;
    bt_ssp_debug_mode_req_struct *msg_p;
        
    ALOGI("[GAP] btmtk_gap_set_ssp_debug_mode: on = 0x%X", on);
    memset(&ilm, 0x0, sizeof(ilm_struct));
    msg_p = (bt_ssp_debug_mode_req_struct*)ilm.ilm_data;
    ilm.msg_id = MSG_ID_BT_SSP_DEBUG_MODE_REQ;
    msg_p->on = on;
    
	pthread_mutex_lock(&(nat->thread_mutex));
    if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_ssp_debug_mode_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_SSP_DEBUG_MODE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT)  > 0)
    {
        bt_ssp_debug_mode_cnf_struct *cnf_p = (bt_ssp_debug_mode_cnf_struct*)ilm.ilm_data;;

        ALOGI("[GAP] MSG_ID_BT_SSP_DEBUG_MODE_CNF: result=%d", cnf_p->result);
        ret = cnf_p->result;
    }
	pthread_mutex_unlock(&(nat->thread_mutex));
    
    if(!ret)
    {
        ALOGE("[GAP] btmtk_gap_set_ssp_debug_mode failed");
    }
    return ret;
}


U8 btmtk_hdp_int_string2channel(const char* type)
{
   bt_hdp_channel_type hdpType = BT_HDP_CHANNEL_TYPE_NO_PREFERENCE;
   if (NULL != type)
   { 
	   if (!strcmp("Reliable", type))
	   {
		   hdpType = BT_HDP_CHANNEL_TYPE_RELIABLE;
	   }
	   else if (!strcmp("Streaming", type))
	   {
		   hdpType = BT_HDP_CHANNEL_TYPE_STREAMING;
	   }
   
   }
   return hdpType;
}

U8 btmtk_hdp_int_string2role(const char* role)
{
	bt_hdp_role hdpRole = BT_HDP_ROLE_INVALID;
	if (NULL != role)
	{ 
		if (!strcmp("Sink", role))
		{
			hdpRole = BT_HDP_ROLE_SINK;
		}
		else if (!strcmp("Source", role))
		{
			hdpRole = BT_HDP_ROLE_SOURCE;
		}
	
	}
	return hdpRole;
}

void btmtk_hdp_init(JNIEnv *env, jobject object)
{
	ALOGI("[HDP] btmtk_hdp_init");
	native_data_t *nat = get_native_data(env, object);
	if (NULL != nat)
	{
		pthread_mutex_lock(&(nat->event_nat->thread_mutex));
		nat->hdp_fds = NULL;		
		pthread_mutex_unlock(&(nat->event_nat->thread_mutex));
	}
	else
	{
		ALOGE("[HDP] nat is NULL");
	}
}
void btmtk_hdp_deinit(JNIEnv *env, jobject object)
{
    ALOGI("[HDP] btmtk_hdp_deinit");
    native_data_t *nat = get_native_data(env, object);
	while (NULL != nat && nat->hdp_fds)
    {
        btmtk_util_list_remove((btmtk_list_header_struct **)&nat->hdp_fds, (btmtk_list_header_struct *)nat->hdp_fds);
    }
    nat->hdp_fds = NULL;
}

int btmtk_hdp_create_socket(U8 l2capchannelId)
{
    sockaddr_un hdpname;
    socklen_t   hdpnamelen;
    struct sockaddr_un btname;
    socklen_t   btnamelen; 
    char addr[UNIX_PATH_MAX];
    int socketFd;
	unsigned value = 1;

	//internal address
	btname.sun_family = AF_UNIX;
    strcpy (btname.sun_path, BT_SOCK_NAME_INT_ADP);
    btnamelen = (offsetof (struct sockaddr_un, sun_path) + strlen (btname.sun_path) + 1); 
	

	sprintf(addr, "%s%d", BT_SOCK_NAME_EXT_ADP_HDP_DATA_PREFIX, l2capchannelId);
	socketFd = socket_local_server(addr, 
										ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
										SOCK_DGRAM);

	
	ALOGI("btmtk_hdp_create_socket : len=%d, addr=%s", hdpnamelen, hdpname.sun_path);

	if (socketFd < 0)
    {
        ALOGE("[HDP]create HDP socket failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }

	setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    
    hdpnamelen = sizeof(hdpname.sun_path);
    hdpname.sun_path[0] = '\0';
    if (getsockname(socketFd, (sockaddr*)&hdpname, &hdpnamelen) < 0)
    {
    	ALOGE("[HDP] getsockname failed : %s, errno=%d", strerror(errno), errno);
		goto exit;
    }
    else
	{
    	ALOGI("[HDP] getsockname success : len=%d, addr=%s", hdpnamelen, &hdpname.sun_path[1]);
    }
    if ( connect(socketFd, (const struct sockaddr*)&btname, btnamelen) < 0)
    {
        ALOGE("[HDP]] connect HDP socket failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }
	else
	{
		return socketFd;
	}
exit:
	if(socketFd >= 0) 
	{
		close(socketFd);
		socketFd = -1;
	}
	return socketFd;
}

bool btmtk_hdp_destroy_socket(int socketFd)
{
	sockaddr_un hdpname;
	socklen_t   hdpnamelen;
	bool ret = FALSE;
	ALOGE("[HDP] btmtk_hdp_destroy_socket: %d", socketFd);

	if (socketFd >= 0)
	{
		hdpnamelen = sizeof(hdpname.sun_path);
        hdpname.sun_path[0] = '\0';
		if (getsockname(socketFd, (sockaddr*)&hdpname, &hdpnamelen) < 0)
		{
			ALOGE("[HDP] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[HDP]getsockname : len=%d, addr=%s", hdpnamelen, &hdpname.sun_path[1]);
        }
		if (shutdown(socketFd, 2) != 0)
		{
			ALOGI("[HDP]shutdown socket : %s, errno=%d", strerror(errno), errno);
		}
		close(socketFd);
		ret = TRUE;
	}
	return ret;
}

btmtk_hdp_fd_struct *btmtk_hdp_util_find_hdp_fd(native_data_t *nat, bt_addr_struct *addr, unsigned short mdlid)
{
    btmtk_hdp_fd_struct *ptr = nat->hdp_fds;
    ALOGI("[HDP] btmtk_util_find_hdp_fd search 0x%lX:0x%X:0x%X: mdl id=0x%X", addr->lap, addr->uap, addr->nap, mdlid);
    while (ptr)
    {
     //   ALOGI("[HDP] btmtk_util_find_hdp_fd found 0x%lX:0x%X:0x%X", ptr->addr.lap, ptr->addr.uap, ptr->addr.nap);
        if (btmtk_util_equal_bdaddr(&ptr->addr, addr) && 
            ptr->mdl_id == mdlid)
        {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}


void btmtk_hdp_add_fd(native_data_t *nat, bt_addr_struct *addr, unsigned short mdlid, int fd)
{
	btmtk_hdp_fd_struct *hdp_fd = (btmtk_hdp_fd_struct *) calloc(1, sizeof(btmtk_hdp_fd_struct));
	ALOGI("btmtk_hdp_add_fd:header addr=%d", &nat->hdp_fds);
	ALOGI("btmtk_hdp_add_fd:addr=%X:%X:%X, mdlid=%d, fd=%d",addr->lap, addr->uap, addr->nap, mdlid, fd);
	if (NULL != hdp_fd)
	{
		hdp_fd->fd = fd;
		hdp_fd->mdl_id = mdlid;
		memcpy(&hdp_fd->addr, addr, sizeof(bt_addr_struct));
		btmtk_util_list_append((btmtk_list_header_struct **)&nat->hdp_fds, (btmtk_list_header_struct *)hdp_fd);    
	}

	//TEST CODE
//	btmtk_hdp_remove_fd(header, addr, mdlid);
}
void btmtk_hdp_remove_fd(native_data_t *nat, bt_addr_struct *addr, unsigned short mdlid)
{

	btmtk_hdp_fd_struct *hdp_fd = btmtk_hdp_util_find_hdp_fd(nat, addr, mdlid);
	ALOGI("btmtk_hdp_remove_fd:header addr=%d", &nat->hdp_fds);
	ALOGI("btmtk_hdp_remove_fd:addr=%X:%X:%X, mdlid=%d",addr->lap, addr->uap, addr->nap, mdlid);
	if (NULL != hdp_fd)
	{
		btmtk_hdp_destroy_socket(hdp_fd->fd);
		btmtk_util_list_remove((btmtk_list_header_struct **)&nat->hdp_fds, (btmtk_list_header_struct *)hdp_fd);
	}  
}



bool btmtk_hdp_parse_channel(const char *channelPath, bt_addr_struct *addr, U16 *mdlId)
{
	ALOGI("[HDP]btmtk_hdp_parse_channel channel path is :%s", channelPath);
	if (NULL == channelPath || NULL == addr || NULL == mdlId)
	{
		return false;
	}
	char *channelDup = strdup(channelPath);
	char *p;
	p = strtok(channelDup, HDP_CHANNEL_DELIMS);	
	*mdlId = atoi(p);

	ALOGI("[HDP]first string :%s", p);
	
//	p = strtok(NULL, HDP_CHANNEL_DELIMS); 
	ALOGI("[HDP]sec string :%s", p);

	p = strstr(channelPath, HDP_CHANNEL_DELIMS);
	if (NULL != p)
	{
		btmtk_util_convert_string2bdaddr(p+strlen(HDP_CHANNEL_DELIMS), addr);
	}
	free(channelDup);
	return TRUE;	
}
bool btmtk_hdp_compose_channel(bt_addr_struct *addr, U16 mdlId, char *channelPath)
{
	if (NULL == channelPath || NULL == addr)
	{
		return false;
	}
	char buf[32] = {'\0'};
	btmtk_util_convert_bdaddr2string(buf, addr);
	sprintf(channelPath, "%d%s%s", mdlId, HDP_CHANNEL_DELIMS, buf);
	ALOGI("[HDP]btmtk_hdp_compose_channel channel path is :%s", channelPath);
	return TRUE;	
	
}

bool btmtk_hdp_register_instance(native_data_t *nat, 
										U8 role, 
										U8 channelType, 
										U16 dataType,
										const char *description,
										U8 *mdepId)
{
	bt_hdp_register_instance_req_struct *req;	
    ilm_struct ilm;
	bool ret = FALSE;

	ALOGI("[HDP] btmtk_hdp_register_instance: on = 0x%X");
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_REGISTER_INSTANCE_REQ;
	req = (bt_hdp_register_instance_req_struct *)ilm.ilm_data;
	req->role = role;
	req->channelType = channelType;
	req->dataType = dataType;
	if (NULL != description)
	{
		strncpy(req->description, description, strlen(description));
	}

	pthread_mutex_lock(&(nat->thread_mutex));
	if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_register_instance_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_HDP_REGISTER_INSTANCE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
	{
		bt_hdp_register_instance_cnf_struct *cnf = (bt_hdp_register_instance_cnf_struct*)ilm.ilm_data;
		
        if(cnf->result == BT_HDP_SUCCESS)
        {
        	*mdepId = cnf->mdepId;
			ret = TRUE;
			
			ALOGI("[HDP] btmtk_hdp_register_instance success: mdep id = 0x%X", cnf->result);
        }
		else
		{
			ALOGI("[HDP] btmtk_hdp_register_instance failed: result = 0x%X", cnf->result);
		}
	} 
	else
	{
		ALOGE("[HDP] btmtk_hdp_register_instance timeout");

	}
	pthread_mutex_unlock(&(nat->thread_mutex));
	return ret;
}

bool btmtk_hdp_deregister_instance(native_data_t *nat, U8 id)
{
	bt_hdp_deregister_instance_req_struct *req;	
    ilm_struct ilm;
	bool ret = FALSE;

	ALOGI("[HDP] btmtk_hdp_deregister_instance: 0x%X", id);
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_DEREGISTER_INSTANCE_REQ;
	req = (bt_hdp_deregister_instance_req_struct *)ilm.ilm_data;
	req->mdepId = id;

	pthread_mutex_lock(&(nat->thread_mutex));
	if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_deregister_instance_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_HDP_DEREGISTER_INSTANCE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
	{
		bt_hdp_deregister_instance_cnf_struct *cnf = (bt_hdp_deregister_instance_cnf_struct*)ilm.ilm_data;

		if (cnf->mdepId != id)
		{
			ALOGE("[GAP] btmtk_hdp_deregister_instance failed: mdep id mismatch 0x%X", cnf->mdepId);
		}
		else
		{
			ALOGI("[GAP] btmtk_hdp_deregister_instance result:  0x%X", cnf->result);
			ret = cnf->result == BT_HDP_SUCCESS;
		}
	} 
	else
	{
		ALOGE("[HDP] btmtk_hdp_register_instance timeout");

	}
	pthread_mutex_unlock(&(nat->thread_mutex));
	return ret;
}

bool btmtk_hdp_connect(native_data_t *nat, bt_addr_struct *addr, U8 mdepId, U8 config, U32 index)
{
	bt_hdp_connect_req_struct *req;
	
    ilm_struct ilm;

	ALOGI("[HDP] btmtk_hdp_connect: %d", mdepId);
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_CONNECT_REQ;
	req = (bt_hdp_connect_req_struct *)ilm.ilm_data;
	req->mdepId = mdepId;
	memcpy(&req->bdaddr, addr, sizeof(bt_addr_struct));
	req->config = config;
	req->index = index;
	
	return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_connect_req_struct));
}

bool btmtk_hdp_disconnect(native_data_t *nat, bt_addr_struct *addr, U16 mdlId, U32 index)
{
	bt_hdp_disconnect_req_struct *req;	
    ilm_struct ilm;
	bool ret;

	ALOGI("[HDP] btmtk_hdp_disconnect");
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_DISCONNECT_REQ;
	req = (bt_hdp_disconnect_req_struct *)ilm.ilm_data;
	req->mdlId= mdlId;
	memcpy(&req->bdaddr, addr, sizeof(bt_addr_struct));	
	req->index = index;
	return bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_disconnect_req_struct));	
}

bool btmtk_hdp_get_main_channel(native_data_t *nat, bt_addr_struct *addr, U8 *mldId)
{
	bt_hdp_get_main_channel_req_struct *req;	
    ilm_struct ilm;
	bool ret;

	ALOGI("[HDP] btmtk_hdp_get_main_channel");
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_GET_MAIN_CHANNEL_REQ;
	req = (bt_hdp_get_main_channel_req_struct *)ilm.ilm_data;
	memcpy(&req->bdaddr, addr, sizeof(bt_addr_struct));	

	pthread_mutex_lock(&(nat->thread_mutex));
	if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_get_main_channel_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_HDP_GET_MAIN_CHANNEL_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
	{
		bt_hdp_get_main_channel_cnf_struct *cnf = (bt_hdp_get_main_channel_cnf_struct*)ilm.ilm_data;
		
        if(cnf->result == BT_HDP_SUCCESS)
        {
        	*mldId = cnf->mdlId;
			ALOGI("[HDP] btmtk_hdp_get_main_channel success: channel id = 0x%X", cnf->mdlId);
        }
		else
		{
			ALOGI("[HDP] btmtk_hdp_get_main_channel failed: result = 0x%X", cnf->result);
		}
		ret = cnf->result == BT_HDP_SUCCESS;
	} 
	else
	{
		ALOGE("[HDP] btmtk_hdp_get_main_channel timeout");
		ret = false;
	}
	pthread_mutex_unlock(&(nat->thread_mutex));
	return ret;

}

bool btmtk_hdp_get_instance(native_data_t *nat,  bt_addr_struct *addr, U16 mdlId, U8 *mdepId)
{
	bt_hdp_get_instance_req_struct *req;	
    ilm_struct ilm;
	bool ret;

	ALOGI("[HDP] btmtk_hdp_get_instance: 0x%X", mdlId);
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_GET_INSTANCE_REQ;
	req = (bt_hdp_get_instance_req_struct *)ilm.ilm_data;
	memcpy(&req->bdaddr, addr, sizeof(bt_addr_struct));
	req->mdlId = mdlId;

	pthread_mutex_lock(&(nat->thread_mutex));
	if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_get_instance_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_HDP_GET_INSTANCE_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
	{
		bt_hdp_get_instance_cnf_struct *cnf = (bt_hdp_get_instance_cnf_struct*)ilm.ilm_data;
		
        if(cnf->result == BT_HDP_SUCCESS)
        {
        	*mdepId = cnf->mdepId;
			ALOGI("[HDP] btmtk_hdp_get_instance success: mdep id = 0x%X", cnf->mdepId);
        }
		else
		{
			ALOGI("[HDP] btmtk_hdp_get_instance failed: result = 0x%X", cnf->result);
		}
		ret = cnf->result == BT_HDP_SUCCESS;
	} 
	else
	{
		ALOGE("[HDP] btmtk_hdp_get_instance timeout");
		ret = false;
	}
	pthread_mutex_unlock(&(nat->thread_mutex));
	return ret;
}

/*Notes: get l2cap channel based on address and mdl id*/
bool btmtk_hdp_get_l2cap_channel(native_data_t *nat, bt_addr_struct *addr, U16 mdlId, U16 *l2capChnId)
{
	bt_hdp_get_l2cap_channel_req_struct *req;	
    ilm_struct ilm;
	bool ret;

	ALOGI("[HDP] btmtk_hdp_get_l2cap_channel");
	
	memset(&ilm, 0x0, sizeof(ilm_struct));	
    ilm.msg_id = MSG_ID_BT_HDP_GET_L2CAP_CHANNEL_REQ;
	req = (bt_hdp_get_l2cap_channel_req_struct *)ilm.ilm_data;
	memcpy(&req->bdaddr, addr, sizeof(bt_addr_struct));	
	req->mdlId = mdlId;

	pthread_mutex_lock(&(nat->thread_mutex));
	if(bt_sendmsg(nat->servsock, (void*)&ilm, sizeof(bt_hdp_get_l2cap_channel_req_struct)) &&
       wait_response(nat->servsock, MSG_ID_BT_HDP_GET_L2CAP_CHANNEL_CNF, &ilm, BTMTK_MAX_STACK_TIMEOUT) > 0)
	{
		bt_hdp_get_l2cap_channel_cnf_struct *cnf = (bt_hdp_get_l2cap_channel_cnf_struct*)ilm.ilm_data;
		
        if(cnf->result == BT_HDP_SUCCESS &&
			btmtk_util_equal_bdaddr(addr, &cnf->bdaddr) && 
			cnf->mdlId == mdlId)
        {
        	*l2capChnId = cnf->l2capId;
			ALOGI("[HDP] btmtk_hdp_get_main_channel success: mdl id = 0x%X, l2cap id is %d", cnf->mdlId, cnf->l2capId);
        }
		else
		{
			ALOGI("[HDP] btmtk_hdp_get_main_channel failed: result = 0x%X", cnf->result);
		}
		ret = cnf->result == BT_HDP_SUCCESS;
	} 
	else
	{
		ALOGE("[HDP] btmtk_hdp_get_main_channel timeout");
		ret = false;
	}
	pthread_mutex_unlock(&(nat->thread_mutex));
	return ret;

}





#endif

static void classInitNative(JNIEnv* env, jclass clazz) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    field_mNativeData = get_field(env, clazz, "mNativeData", "I");
    field_mEventLoop = get_field(env, clazz, "mEventLoop",
            "Landroid/server/BluetoothEventLoop;");
#endif

#ifdef __BTMTK__
    field_mNativeData = get_field(env, clazz, "mNativeData", "I");
    field_mEventLoop = get_field(env, clazz, "mEventLoop",
            "Landroid/server/BluetoothEventLoop;");
#endif
}

/* Returns true on success (even if adapter is present but disabled).
 * Return false if dbus is down, or another serious error (out of memory)
*/
static bool initializeNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    if (NULL == nat) {
        ALOGE("%s: out of memory!", __FUNCTION__);
        return false;
    }
    nat->env = env;

    env->SetIntField(object, field_mNativeData, (jint)nat);
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
#endif  /*HAVE_BLUETOOTH*/

#ifdef __BTMTK__
    native_data_t *nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    ALOGI("android_server_BluetoothService.cpp: initializeNativeDataNative");
    if (NULL == nat) {
        ALOGE("%s: out of memory!", __FUNCTION__);
        return false;
    }
    nat->env = env;

    env->SetIntField(object, field_mNativeData, (jint)nat);
    /* Initialize server socket */
    nat->servsock = socket_local_server(BT_SOCK_NAME_APP_GAP, 
                                                            ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
                                                            SOCK_DGRAM);
    if (nat->servsock < 0)
    {
        ALOGE("[GAP][ERR] create server socket failed");
        return false;
    }
	nat->twinservsock = socket_local_server(BT_SOCK_NAME_APP_GAP_TWIN, 
                                                            ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
                                                            SOCK_DGRAM);
	if (nat->twinservsock < 0)
    {
        ALOGE("[GAP][ERR] create server socket failed");
        return false;
    }
	
    pthread_mutex_init(&(nat->thread_mutex), NULL);

#endif

    return true;
}

static const char *get_adapter_path(JNIEnv* env, jobject object) {
#ifdef HAVE_BLUETOOTH
    event_loop_native_data_t *event_nat =
        get_EventLoop_native_data(env, env->GetObjectField(object,
                                                           field_mEventLoop));
    if (event_nat == NULL)
        return NULL;
    return event_nat->adapter;
#endif

#ifdef __BTMTK__
    event_loop_native_data_t *event_nat =
        get_EventLoop_native_data(env, env->GetObjectField(object,
                                                           field_mEventLoop));
    ALOGI("[GAP][API] get_adapter_path : set object path to \"%s\"", event_nat ? event_nat->adapter : "");
    if (event_nat == NULL)
    return NULL;
    return event_nat->adapter;
#endif

return NULL;
}

// This function is called when the adapter is enabled.
static jboolean setupNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat =
        (native_data_t *)env->GetIntField(object, field_mNativeData);
    event_loop_native_data_t *event_nat =
        get_EventLoop_native_data(env, env->GetObjectField(object,
                                                           field_mEventLoop));
    // Register agent for remote devices.
    const char *device_agent_path = "/android/bluetooth/remote_device_agent";
    static const DBusObjectPathVTable agent_vtable = {
                 NULL, agent_event_filter, NULL, NULL, NULL, NULL };

    if (!dbus_connection_register_object_path(nat->conn, device_agent_path,
                                              &agent_vtable, event_nat)) {
        ALOGE("%s: Can't register object path %s for remote device agent!",
                               __FUNCTION__, device_agent_path);
        return JNI_FALSE;
    }
#endif /*HAVE_BLUETOOTH*/
    return JNI_TRUE;
}

static jboolean tearDownNativeDataNative(JNIEnv *env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat =
               (native_data_t *)env->GetIntField(object, field_mNativeData);
    if (nat != NULL) {
        const char *device_agent_path =
            "/android/bluetooth/remote_device_agent";
        dbus_connection_unregister_object_path (nat->conn, device_agent_path);
    }
#endif /*HAVE_BLUETOOTH*/
    return JNI_TRUE;
}

static void cleanupNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat =
        (native_data_t *)env->GetIntField(object, field_mNativeData);
    if (nat) {
        free(nat);
        nat = NULL;
    }
#endif
#ifdef __BTMTK__
    native_data_t *nat =
        (native_data_t *)env->GetIntField(object, field_mNativeData);
    if (nat) {
        free(nat);
        nat = NULL;
    }
#endif
}

static jstring getAdapterPathNative(JNIEnv *env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        return (env->NewStringUTF(get_adapter_path(env, object)));
    }
#endif

#ifdef __BTMTK__
    ALOGI("[GAP][API] getAdapterPathNative : set object path to \"MTKBT\"");
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        return (env->NewStringUTF(get_adapter_path(env, object)));
    }    
#endif

    return NULL;
}



static jboolean startDiscoveryNative(JNIEnv *env, jobject object, int mode) {
    ALOGV(__FUNCTION__);

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[GAP][API] startDiscoveryNative");

    if (nat && nat->state == BTMTK_POWER_STATE_ON)
    {
        btbm_discovery_type type;
        switch (mode)
        {
        case 0:
            type = BTBM_DISCOVERY_BR_EDR_ONLY;
            break;
        case 1:
            type = BTBM_DISCOVERY_LE_ONLY;
            break;
        case 2:
            type = BTBM_DISCOVERY_DUAL;
            break;
        default:
    return JNI_FALSE;
        }
        return btmtk_gap_discovery(nat, type);
    }
#endif
    return JNI_FALSE;
}

static jboolean stopDiscoveryNative(JNIEnv *env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError err;
    const char *name;
    native_data_t *nat;
    jboolean ret = JNI_FALSE;

    dbus_error_init(&err);

    nat = get_native_data(env, object);
    if (nat == NULL) {
        goto done;
    }

    /* Compose the command */
    msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC,
                                       get_adapter_path(env, object),
                                       DBUS_ADAPTER_IFACE, "StopDiscovery");
    if (msg == NULL) {
        if (dbus_error_is_set(&err))
            LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        goto done;
    }

    /* Send the command. */
    reply = dbus_connection_send_with_reply_and_block(nat->conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        if(strncmp(err.name, BLUEZ_DBUS_BASE_IFC ".Error.NotAuthorized",
                   strlen(BLUEZ_DBUS_BASE_IFC ".Error.NotAuthorized")) == 0) {
            // hcid sends this if there is no active discovery to cancel
            ALOGV("%s: There was no active discovery to cancel", __FUNCTION__);
            dbus_error_free(&err);
        } else {
            LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, msg);
        }
        goto done;
    }

    ret = JNI_TRUE;
done:
    if (msg) dbus_message_unref(msg);
    if (reply) dbus_message_unref(reply);
    return ret;
#elif defined __BTMTK__
  native_data_t *nat = get_native_data(env, object);

    ALOGI("[GAP][API] stopDiscoveryNative");
    if (nat->state == BTMTK_POWER_STATE_ON)
    {
        btmtk_gap_discovery_cancel(nat);
    }
    else
    {
        ALOGI("[GAP] btmtk_gap_discovery_cancel already cancelled");
        btmtk_gap_send_discovery_stop_event(nat);
    }
    return JNI_TRUE;
#else 
    return JNI_FALSE;
#endif
}

static jbyteArray readAdapterOutOfBandDataNative(JNIEnv *env, jobject object) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    DBusError err;
    jbyte *hash, *randomizer;
    jbyteArray byteArray = NULL;
    int hash_len, r_len;
    if (nat) {
       DBusMessage *reply = dbus_func_args(env, nat->conn,
                           get_adapter_path(env, object),
                           DBUS_ADAPTER_IFACE, "ReadLocalOutOfBandData",
                           DBUS_TYPE_INVALID);
       if (!reply) return NULL;

       dbus_error_init(&err);
       if (dbus_message_get_args(reply, &err,
                                DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &hash, &hash_len,
                                DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &randomizer, &r_len,
                                DBUS_TYPE_INVALID)) {
          if (hash_len == 16 && r_len == 16) {
               byteArray = env->NewByteArray(32);
               if (byteArray) {
                   env->SetByteArrayRegion(byteArray, 0, 16, hash);
                   env->SetByteArrayRegion(byteArray, 16, 16, randomizer);
               }
           } else {
               ALOGE("readAdapterOutOfBandDataNative: Hash len = %d, R len = %d",
                                                                  hash_len, r_len);
           }
       } else {
          LOG_AND_FREE_DBUS_ERROR(&err);
       }
       dbus_message_unref(reply);
       return byteArray;
    }
#endif
    return NULL;
}

static jboolean createPairedDeviceNative(JNIEnv *env, jobject object,
                                         jstring address, jint timeout_ms) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        ALOGV("... address = %s", c_address);
        char *context_address = (char *)calloc(BTADDR_SIZE, sizeof(char));
        const char *capabilities = "DisplayYesNo";
        const char *agent_path = "/android/bluetooth/remote_device_agent";

        strlcpy(context_address, c_address, BTADDR_SIZE);  // for callback
        bool ret = dbus_func_args_async(env, nat->conn, (int)timeout_ms,
                                        onCreatePairedDeviceResult, // callback
                                        context_address,
                                        eventLoopNat,
                                        get_adapter_path(env, object),
                                        DBUS_ADAPTER_IFACE,
                                        "CreatePairedDevice",
                                        DBUS_TYPE_STRING, &c_address,
                                        DBUS_TYPE_OBJECT_PATH, &agent_path,
                                        DBUS_TYPE_STRING, &capabilities,
                                        DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(address, c_address);
        return ret ? JNI_TRUE : JNI_FALSE;

    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    if (nat && nat->event_nat && nat->state == BTMTK_POWER_STATE_ON)
    {
        const char *c_address = env->GetStringUTFChars(address, NULL);	
        bt_addr_struct addr;
        bool ret;
        int i;

        ALOGI("[GAP][API] createPairedDeviceNative: address=%s, eventloop native data=0x%X", c_address, nat->event_nat);
        btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
		btmtk_gap_send_sdp_paired_device_create_event(nat, &addr);
        if((ret = btmtk_gap_bond(nat, &addr, true)) == false)
        {
            btmtk_gap_send_sdp_paired_device_remove_event(nat, &addr);
        }
        env->ReleaseStringUTFChars(address, c_address);
        ALOGI("[GAP] createPairedDeviceNative sdp return %d", ret);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

    return JNI_FALSE;
}

static jboolean createPairedDeviceOutOfBandNative(JNIEnv *env, jobject object,
                                                jstring address, jint timeout_ms) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        ALOGV("... address = %s", c_address);
        char *context_address = (char *)calloc(BTADDR_SIZE, sizeof(char));
        const char *capabilities = "DisplayYesNo";
        const char *agent_path = "/android/bluetooth/remote_device_agent";

        strlcpy(context_address, c_address, BTADDR_SIZE);  // for callback
        bool ret = dbus_func_args_async(env, nat->conn, (int)timeout_ms,
                                        onCreatePairedDeviceResult, // callback
                                        context_address,
                                        eventLoopNat,
                                        get_adapter_path(env, object),
                                        DBUS_ADAPTER_IFACE,
                                        "CreatePairedDeviceOutOfBand",
                                        DBUS_TYPE_STRING, &c_address,
                                        DBUS_TYPE_OBJECT_PATH, &agent_path,
                                        DBUS_TYPE_STRING, &capabilities,
                                        DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(address, c_address);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jint getDeviceServiceChannelNative(JNIEnv *env, jobject object,
                                          jstring path,
                                          jstring pattern, jint attr_id) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);
    if (nat && eventLoopNat) {
        const char *c_pattern = env->GetStringUTFChars(pattern, NULL);
        const char *c_path = env->GetStringUTFChars(path, NULL);
        ALOGV("... pattern = %s", c_pattern);
        ALOGV("... attr_id = %#X", attr_id);
        DBusMessage *reply =
            dbus_func_args(env, nat->conn, c_path,
                           DBUS_DEVICE_IFACE, "GetServiceAttributeValue",
                           DBUS_TYPE_STRING, &c_pattern,
                           DBUS_TYPE_UINT16, &attr_id,
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(pattern, c_pattern);
        env->ReleaseStringUTFChars(path, c_path);
        return reply ? dbus_returns_int32(env, reply) : -1;
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    int channel = 0;

    if (nat && nat->event_nat && nat->state == BTMTK_POWER_STATE_ON)
    {
        const char *c_pattern = env->GetStringUTFChars(pattern, NULL);
        const char *c_object_path = env->GetStringUTFChars(path, NULL);
        bt_addr_struct addr;
        btbm_uuid_type uuid_type = BTBM_UUID_128;
        btmtk_sdp_req_struct *req;
        unsigned short uuid16 = 0;
        char uuid128[BTMTK_SDP_UUID_128_BIT_SIZE];
        btmtk_device_entry_struct *entry;
        int i;

        ALOGI("[GAP][API] getDeviceServiceChannelNative addr=%s, pattern:%s, attr=0x%X ", c_object_path, c_pattern, attr_id);
        btmtk_util_convert_objpath2bdaddr((char*)c_object_path, &addr);
        btmtk_util_convert_string_2_uuid128(uuid128, c_pattern);
        if (btmtk_util_is_assigned_uuid(uuid128))
        {
            uuid16 = btmtk_util_convert_uuid128_2_uuid16(uuid128);
            uuid_type = BTBM_UUID_16;
        }
        env->ReleaseStringUTFChars(pattern, c_pattern);
        env->ReleaseStringUTFChars(path, c_object_path);
        
        entry = btmtk_paired_dev_cache_find(nat->event_nat, &addr);

        if(entry == NULL)
        {
        	entry = btmtk_inquired_dev_cache_find(nat->event_nat, &addr);
		if (entry == NULL) 
            return -1;
        }
        for (i = 0; i < entry->sdp.uuid_no; i++)
        {
            if (entry->sdp.uuid[i] == uuid16 && entry->sdp.channel[i] != 0)
            {
                ALOGI("[GAP][API] getDeviceServiceChannelNative cached channel=%d", entry->sdp.channel[i]);
                return (jint) entry->sdp.channel[i];
            }
        }

        if (btmtk_gap_service_attrib_search_request(nat, &addr, uuid_type, uuid16, uuid128, (unsigned short)attr_id, &channel))
        {
            return (jint)channel;
        }
    }
#endif

    return -1;
}

static jboolean cancelDeviceCreationNative(JNIEnv *env, jobject object,
                                           jstring address) {
    ALOGV("%s", __FUNCTION__);
    jboolean result = JNI_FALSE;
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        DBusError err;
        dbus_error_init(&err);
        ALOGV("... address = %s", c_address);
        DBusMessage *reply =
            dbus_func_args_timeout(env, nat->conn, -1,
                                   get_adapter_path(env, object),
                                   DBUS_ADAPTER_IFACE, "CancelDeviceCreation",
                                   DBUS_TYPE_STRING, &c_address,
                                   DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(address, c_address);
        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            } else
                ALOGE("DBus reply is NULL in function %s", __FUNCTION__);
            return JNI_FALSE;
        } else {
            result = JNI_TRUE;
        }
        dbus_message_unref(reply);
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    if (nat && nat->state == BTMTK_POWER_STATE_ON)
    {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        bt_addr_struct addr;
		btmtk_device_entry_struct *entry = NULL;
        bool ret = FALSE;
		btmtk_sdp_req_struct *req = NULL;	
        ALOGI("[GAP][API] cancelDeviceCreationNative: addr=%s", c_address);
        btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
		entry = btmtk_paired_dev_cache_find(nat->event_nat, &addr);
		if (NULL == entry)
		{
			ret = btmtk_gap_bond_cancel(nat, &addr);
		}
		else
		{
			ret = btmtk_gap_unpair(nat, &addr);
		}

		req = btmtk_util_find_sdp_request(nat->event_nat, &addr);		 
		if (req != NULL && req->type == BTMTK_SDP_CREATE_PAIRED_DEVICE)
		{			
			btmtk_gap_send_sdp_paired_device_remove_event(nat, &addr);
		} 
        env->ReleaseStringUTFChars(address, c_address);
        ALOGI("[GAP] btmtk_gap_bond_cancel return %d", ret);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

    return JNI_FALSE;
}

static jboolean removeDeviceNative(JNIEnv *env, jobject object, jstring object_path) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_object_path = env->GetStringUTFChars(object_path, NULL);
        bool ret = dbus_func_args_async(env, nat->conn, -1,
                                        NULL,
                                        NULL,
                                        NULL,
                                        get_adapter_path(env, object),
                                        DBUS_ADAPTER_IFACE,
                                        "RemoveDevice",
                                        DBUS_TYPE_OBJECT_PATH, &c_object_path,
                                        DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(object_path, c_object_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

#ifdef __BTMTK__
    /* Looks like asynchronous call. There might be a indication after the remove is complete */
    /* BlueZ : "org.bluez.Adapter","DeviceRemoved" - indication after device is removed */
    native_data_t *nat = get_native_data(env, object);
    if (nat && nat->state == BTMTK_POWER_STATE_ON) 
    {
        bool ret;
        bt_addr_struct addr;
        const char *c_object_path = env->GetStringUTFChars(object_path, NULL);

        ALOGI("[GAP][API] removeDeviceNative: addr=%s", c_object_path);
        btmtk_util_convert_objpath2bdaddr((char*)c_object_path, &addr);
        ret = btmtk_gap_unpair(nat, &addr);
        env->ReleaseStringUTFChars(object_path, c_object_path);
        ALOGI("[GAP] btmtk_gap_unpair return %d", ret);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

    return JNI_FALSE;
}

static jint enableNative(JNIEnv *env, jobject object) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    return bt_enable();
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[GAP][API] enableNative");
    if(nat)
    {
        nat->state = BTMTK_POWER_STATE_TURNING_ON;
        if (btmtk_gap_power_on(nat))
        {
            btmtk_gap_init(env, object);
			btmtk_hdp_init(env, object);
            nat->state = BTMTK_POWER_STATE_ON;
            return 0;
        }
        else
        {
            nat->state = BTMTK_POWER_STATE_OFF;
    return -1;
}
    }
#endif
    return -1;
}

static jint disableNative(JNIEnv *env, jobject object) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    return bt_disable();
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    ALOGI("[GAP][API] disableNative");
    if(nat)
    {
        nat->state = BTMTK_POWER_STATE_TURNING_OFF;
#ifdef __ANDROID_EMULATOR__
        if (btmtk_gap_set_scanable_mode(nat, BTBM_ADP_P_OFF_I_OFF))
        {
            nat->state = BTMTK_POWER_STATE_OFF;
        }
#else
        if (btmtk_gap_power_off(nat))
        {
            btmtk_gap_deinit(env, object);
			btmtk_hdp_deinit(env, object);
			/*the state has to be changed in event loop thread in order to clear event loop context*/
        //   nat->state = BTMTK_POWER_STATE_OFF;
        }
#endif
    }
    
    return ((nat->state == BTMTK_POWER_STATE_OFF) ? 0 : -1);
#endif
    return -1;
}

static jint isEnabledNative(JNIEnv *env, jobject object) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    return bt_is_enabled();
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    if(nat)
    {
        return (nat->state == BTMTK_POWER_STATE_ON) ? 1 : -1;
    }
    else
    {
    return -1;
}
#endif
    return -1;
}

static jboolean setPairingConfirmationNative(JNIEnv *env, jobject object,
                                             jstring address, bool confirm,
                                             int nativeData) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg = (DBusMessage *)nativeData;
        DBusMessage *reply;
        if (confirm) {
            reply = dbus_message_new_method_return(msg);
        } else {
            reply = dbus_message_new_error(msg,
                "org.bluez.Error.Rejected", "User rejected confirmation");
        }

        if (!reply) {
            ALOGE("%s: Cannot create message reply to RequestPasskeyConfirmation or"
                  "RequestPairingConsent to D-Bus\n", __FUNCTION__);
            dbus_message_unref(msg);
            return JNI_FALSE;
        }

        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(msg);
        dbus_message_unref(reply);
        return JNI_TRUE;
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    
    if (nat && nat->state == BTMTK_POWER_STATE_ON) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        bt_addr_struct addr;
        
        ALOGI("[GAP][API] setPairingConfirmationNative : addr=%s, confirm=%d, nativeData=%d", c_address, confirm, nativeData);
        btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
        env->ReleaseStringUTFChars(address, c_address);

        switch(nativeData)
        {
        case BTMTK_PAIR_TYPE_USER_CONFIRM: /* onRequestPasskeyConfirmation */
        case BTMTK_PAIR_TYPE_JUST_WORK: /* onRequestPairingConsent */
            return (btmtk_gap_security_user_confirm(nat, &addr, confirm) ? JNI_TRUE : JNI_FALSE);
        case BTMTK_PAIR_TYPE_PASSKEY_DISPLAY: /* onDisplayPasskey */
            return btmtk_gap_security_passkey_notify(nat, &addr, true);
            break;
        case BTMTK_PAIR_TYPE_PINCODE: /* onRequestPinCode */
        case BTMTK_PAIR_TYPE_PASSKEY_INPUT: /* onRequestPasskey */
        default:
            ALOGE("Unexpected type of pairing type=%d", nativeData);
            break;
        }

    }
#endif

    return JNI_FALSE;
}

static jboolean setPasskeyNative(JNIEnv *env, jobject object, jstring address,
                         int passkey, int nativeData) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg = (DBusMessage *)nativeData;
        DBusMessage *reply = dbus_message_new_method_return(msg);
        if (!reply) {
            ALOGE("%s: Cannot create message reply to return Passkey code to "
                 "D-Bus\n", __FUNCTION__);
            dbus_message_unref(msg);
            return JNI_FALSE;
        }

        dbus_message_append_args(reply, DBUS_TYPE_UINT32, (uint32_t *)&passkey,
                                 DBUS_TYPE_INVALID);

        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(msg);
        dbus_message_unref(reply);
        return JNI_TRUE;
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    
    if (nat && nat->state == BTMTK_POWER_STATE_ON) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        bt_addr_struct addr;
        
        ALOGI("[GAP][API] setPasskeyNative : addr=%s, passkey=%u", c_address, passkey);
        btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
        env->ReleaseStringUTFChars(address, c_address);
        return (btmtk_gap_security_passkey_entry(nat, &addr, TRUE, passkey) ? JNI_TRUE : JNI_FALSE);
    }
#endif

    return JNI_FALSE;
}

static jboolean setRemoteOutOfBandDataNative(JNIEnv *env, jobject object, jstring address,
                         jbyteArray hash, jbyteArray randomizer, int nativeData) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg = (DBusMessage *)nativeData;
        DBusMessage *reply = dbus_message_new_method_return(msg);
        jbyte *h_ptr = env->GetByteArrayElements(hash, NULL);
        jbyte *r_ptr = env->GetByteArrayElements(randomizer, NULL);
        if (!reply) {
            ALOGE("%s: Cannot create message reply to return remote OOB data to "
                 "D-Bus\n", __FUNCTION__);
            dbus_message_unref(msg);
            return JNI_FALSE;
        }

        dbus_message_append_args(reply,
                                DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &h_ptr, 16,
                                DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &r_ptr, 16,
                                DBUS_TYPE_INVALID);

        env->ReleaseByteArrayElements(hash, h_ptr, 0);
        env->ReleaseByteArrayElements(randomizer, r_ptr, 0);

        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(msg);
        dbus_message_unref(reply);
        return JNI_TRUE;
    }
#endif

#ifdef __BTMTK__
        native_data_t *nat = get_native_data(env, object);
        
        if (nat && nat->state == BTMTK_POWER_STATE_ON) {
            const char *c_address = env->GetStringUTFChars(address, NULL);
            bt_addr_struct addr;
            char hash_array[16];
            char rand_array[16];
            
            ALOGI("[GAP][API] setRemoteOutOfBandDataNative : addr=%s", c_address);
            btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
            env->ReleaseStringUTFChars(address, c_address);
            env->GetByteArrayRegion(hash, 0, 16, (jbyte *)hash_array);
            env->GetByteArrayRegion(randomizer, 0, 16, (jbyte *)rand_array);
            return (btmtk_gap_security_oob_data(nat, &addr, TRUE, hash_array, rand_array) ? JNI_TRUE : JNI_FALSE);
        }
#endif

    return JNI_FALSE;
}

static jboolean setAuthorizationNative(JNIEnv *env, jobject object, jstring address,
                         jboolean val, int nativeData) {
#ifdef HAVE_BLUETOOTH
  ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg = (DBusMessage *)nativeData;
        DBusMessage *reply;
        if (val) {
            reply = dbus_message_new_method_return(msg);
        } else {
            reply = dbus_message_new_error(msg,
                    "org.bluez.Error.Rejected", "Authorization rejected");
        }
        if (!reply) {
            ALOGE("%s: Cannot create message reply D-Bus\n", __FUNCTION__);
            dbus_message_unref(msg);
            return JNI_FALSE;
        }

        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(msg);
        dbus_message_unref(reply);
        return JNI_TRUE;
    }
#endif
    return JNI_FALSE;
}

static jboolean setPinNative(JNIEnv *env, jobject object, jstring address,
                         jstring pin, int nativeData) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg = (DBusMessage *)nativeData;
        DBusMessage *reply = dbus_message_new_method_return(msg);
        if (!reply) {
            ALOGE("%s: Cannot create message reply to return PIN code to "
                 "D-Bus\n", __FUNCTION__);
            dbus_message_unref(msg);
            return JNI_FALSE;
        }

        const char *c_pin = env->GetStringUTFChars(pin, NULL);

        dbus_message_append_args(reply, DBUS_TYPE_STRING, &c_pin,
                                 DBUS_TYPE_INVALID);

        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(msg);
        dbus_message_unref(reply);
        env->ReleaseStringUTFChars(pin, c_pin);
        return JNI_TRUE;
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    bt_addr_struct addr;
    bool ret = false;
    
    if (nat && nat->state == BTMTK_POWER_STATE_ON) {
        const char *c_pin = env->GetStringUTFChars(pin, NULL);
        const char *c_address = env->GetStringUTFChars(address, NULL);
        ALOGI("[GAP][API] setPinNative : addr=%s, pin=%s", c_address, c_pin);
        if (c_address)
        {
            btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
            if (c_pin)
            {
                ret = btmtk_gap_pin_res(nat, &addr, c_pin);
                env->ReleaseStringUTFChars(pin, c_pin);
            }
            env->ReleaseStringUTFChars(address, c_address);
        }
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

    return JNI_FALSE;
}

static jboolean cancelPairingUserInputNative(JNIEnv *env, jobject object,
                                            jstring address, int nativeData) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg = (DBusMessage *)nativeData;
        DBusMessage *reply = dbus_message_new_error(msg,
                "org.bluez.Error.Canceled", "Pairing User Input was canceled");
        if (!reply) {
            ALOGE("%s: Cannot create message reply to return cancelUserInput to"
                 "D-BUS\n", __FUNCTION__);
            dbus_message_unref(msg);
            return JNI_FALSE;
        }

        dbus_connection_send(nat->conn, reply, NULL);
        dbus_message_unref(msg);
        dbus_message_unref(reply);
        return JNI_TRUE;
    }
#endif

#ifdef __BTMTK__
    int res = 0;
    bool ret = false;
    native_data_t *nat = get_native_data(env, object);
    const char *c_address = env->GetStringUTFChars(address, NULL);

    ALOGI("[GAP][API] cancelPairingUserInputNative: addr=%s, nativeData=%d", c_address, nativeData);
    if(nat && c_address)
    {
        ilm_struct ilm;
        bt_addr_struct addr;
        btmtk_util_convert_string2bdaddr(c_address, &addr);

        switch(nativeData)
        {
        case BTMTK_PAIR_TYPE_PINCODE: /* onRequestPinCode */
        {
            ret = btmtk_gap_pin_res(nat, &addr, NULL);
            break;
        }
        case BTMTK_PAIR_TYPE_USER_CONFIRM: /* onRequestPasskeyConfirmation */
        {
            ret = btmtk_gap_security_user_confirm(nat, &addr, false);
            break;
        }
        case BTMTK_PAIR_TYPE_PASSKEY_INPUT: /* onRequestPasskey */
            ret = btmtk_gap_security_passkey_entry(nat, &addr, false, 0);
            break;
        case BTMTK_PAIR_TYPE_JUST_WORK: /* onRequestPairingConsent */
        {
            ret = btmtk_gap_security_user_confirm(nat, &addr, false);
            break;
        }
        case BTMTK_PAIR_TYPE_PASSKEY_DISPLAY: /* onDisplayPasskey */
        default:
            ALOGE("Unexpected type of pairing type=%d", nativeData);
            break;
        }
    }
    env->ReleaseStringUTFChars(address, c_address);
    return ret ? JNI_TRUE : JNI_FALSE;
#endif

    return JNI_FALSE;
}

static jobjectArray getDevicePropertiesNative(JNIEnv *env, jobject object,
                                                    jstring path)
{
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg, *reply;
        DBusError err;
        dbus_error_init(&err);

        const char *c_path = env->GetStringUTFChars(path, NULL);
        reply = dbus_func_args_timeout(env,
                                   nat->conn, -1, c_path,
                                   DBUS_DEVICE_IFACE, "GetProperties",
                                   DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            } else
                ALOGE("DBus reply is NULL in function %s", __FUNCTION__);
            return NULL;
        }
        env->PushLocalFrame(PROPERTIES_NREFS);

        DBusMessageIter iter;
        jobjectArray str_array = NULL;
        if (dbus_message_iter_init(reply, &iter))
           str_array =  parse_remote_device_properties(env, &iter);
        dbus_message_unref(reply);

        return (jobjectArray) env->PopLocalFrame(str_array);
    }
#endif
#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    const char *c_path = env->GetStringUTFChars(path, NULL);
    jobjectArray str_array = NULL;
    btmtk_device_entry_struct *entry;
    bt_addr_struct addr;

    ALOGI("[GAP][API] getDevicePropertiesNative: addr=%s", c_path);
    btmtk_util_convert_objpath2bdaddr(c_path, &addr);
            
    entry = btmtk_paired_dev_cache_find(nat->event_nat, &addr);
    if (entry == NULL)
    {
        entry = btmtk_inquired_dev_cache_find(nat->event_nat, &addr);
    }

    if (nat && nat->event_nat && c_path && entry) 
    {
        jclass stringClass = env->FindClass("java/lang/String");
        Properties prop;
        property_value value;
        int array_index = 0;
        int i;
        char addr_str[BTMTK_MAX_ADDR_STR_SIZE];

	entry->sdp.uuid_no = (entry->sdp.uuid_no < 0) ? 0 : entry->sdp.uuid_no;
	entry->sdp.app_uuid_no = (entry->sdp.app_uuid_no < 0) ? 0 : entry->sdp.app_uuid_no;
		
        str_array = env->NewObjectArray(BTMTK_DEVICE_PROP_TOTAL * 2 + entry->sdp.uuid_no + entry->sdp.app_uuid_no, stringClass, NULL);

        /* Property Address: DBUS_TYPE_STRING */
        btmtk_util_convert_bdaddr2string(addr_str, &addr);
        strcpy(prop.name, "Address");
        prop.type = DBUS_TYPE_STRING;
        value.str_val = addr_str;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        
        /* Property Name: DBUS_TYPE_STRING */
        strcpy(prop.name, "Name");
        prop.type = DBUS_TYPE_STRING;
        value.str_val = ((strlen(entry->name)) ? entry->name : (char *)"None");
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);

		/* Property Nickname: DBUS_TYPE_STRING */
        strcpy(prop.name, "Alias");
        prop.type = DBUS_TYPE_STRING;
        value.str_val = ((strlen(entry->nickname)) ? entry->nickname : (char *)'\0');
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
		
		ALOGI("[GAP] create_prop_array: Nickname is %s ",entry->nickname);

        /* Property Icon: DBUS_TYPE_STRING */

        /* Property Class: DBUS_TYPE_STRING */
        strcpy(prop.name, "Class");
        prop.type = DBUS_TYPE_UINT32;
        value.int_val = entry->cod;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);

        /* Property UUIDs: DBUS_TYPE_ARRAY */
        strcpy(prop.name, "UUIDs");
        prop.type = DBUS_TYPE_ARRAY;
        if (entry->sdp.uuid_no + entry->sdp.app_uuid_no)
        {
            value.array_val = (char **)calloc(4, entry->sdp.uuid_no + entry->sdp.app_uuid_no);
            for (i = 0; i < entry->sdp.uuid_no; i++)
            {
                value.array_val[i] = (char*)calloc(1, BTMTK_MAX_UUID_STR_SIZE);
                if(value.array_val[i])
                {
                    char uuid128[BTMTK_SDP_UUID_128_BIT_SIZE];
                    btmtk_util_convert_uuid16_2_uuid128(uuid128, entry->sdp.uuid[i]);
                    btmtk_util_convert_uuid128_2_string(value.array_val[i], uuid128);
                    ALOGI("[GAP] UUID: %s", value.array_val[i]);  
                }
            }
            for (i = entry->sdp.uuid_no; i < entry->sdp.uuid_no + entry->sdp.app_uuid_no; i++)
            {
                value.array_val[i] = (char*)calloc(1, BTMTK_MAX_UUID_STR_SIZE);
                if(value.array_val[i])
                {
                    char uuid128[BTMTK_SDP_UUID_128_BIT_SIZE];
                    btmtk_util_convert_uuid128_2_string(value.array_val[i], (char *)&entry->sdp.app_uuid[i - entry->sdp.uuid_no][0]);
                    ALOGI("[GAP] APP UUID[%d-%d]: %s", i ,entry->sdp.uuid_no, value.array_val[i]);  
                }
            }
        }
        create_prop_array(env, str_array, &prop, &value, entry->sdp.uuid_no + entry->sdp.app_uuid_no, &array_index);
        ALOGI("[GAP] create_prop_array done");
        if (entry->sdp.uuid_no + entry->sdp.app_uuid_no)
        {
            for (i = 0; i < entry->sdp.uuid_no; i++)
            {
                free(value.array_val[i]);
            }
            for (i = entry->sdp.uuid_no; i < entry->sdp.uuid_no + entry->sdp.app_uuid_no; i++)
            {
                free(value.array_val[i]);
            }
            free(value.array_val);
        }

        /* Property Paired: DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "Paired");
        prop.type = DBUS_TYPE_BOOLEAN;
        value.int_val = (entry->paired == BTMTK_BOND_STATE_BONDED);
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);

        /* Property Connected: DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "Connected");
        prop.type = DBUS_TYPE_BOOLEAN;
        value.int_val = entry->connected;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);

        /* Property Trusted: DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "Trusted");
        prop.type = DBUS_TYPE_BOOLEAN;
        value.int_val = entry->trusted;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);

        /* Property Alias: DBUS_TYPE_STRING */
        /* Property Nodes: DBUS_TYPE_ARRAY */
        /* Property Adapter: DBUS_TYPE_OBJECT_PATH */
        /* Property LegacyPairing: DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "LegacyPairing");
        prop.type = DBUS_TYPE_BOOLEAN;
        value.int_val = entry->legacy_pairing;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);

        /* Property LegacyPairing: DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "RSSI");
        prop.type = DBUS_TYPE_INT16;
        value.int_val = entry->rssi;
        create_prop_array(env, str_array, &prop, &value, 0, &array_index);
        /* Property TX: DBUS_TYPE_UINT32 */
    }
    env->ReleaseStringUTFChars(path, c_path);
    return str_array;
#endif
    return NULL;
}

int addDefaultProfile(unsigned short *uuid)
{
		int num = 0;
#ifdef __BTMTK__		
		uuid[num] = BTMTK_SDP_UUID_HEADSET_AUDIO_GATEWAY;
		num ++;
		
		uuid[num] = BTMTK_SDP_UUID_AG_HANDSFREE;
		num ++;
		
		uuid[num] = BTMTK_SDP_UUID_AUDIO_SOURCE;
		num ++;
#endif		
		return num;
}

static jobjectArray getAdapterPropertiesNative(JNIEnv *env, jobject object) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg, *reply;
        DBusError err;
        dbus_error_init(&err);

        reply = dbus_func_args_timeout(env,
                                   nat->conn, -1, get_adapter_path(env, object),
                                   DBUS_ADAPTER_IFACE, "GetProperties",
                                   DBUS_TYPE_INVALID);
        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            } else
                ALOGE("DBus reply is NULL in function %s", __FUNCTION__);
            return NULL;
        }
        env->PushLocalFrame(PROPERTIES_NREFS);

        DBusMessageIter iter;
        jobjectArray str_array = NULL;
        if (dbus_message_iter_init(reply, &iter))
            str_array = parse_adapter_properties(env, &iter);
        dbus_message_unref(reply);

        return (jobjectArray) env->PopLocalFrame(str_array);
    }
#endif
#ifdef __BTMTK__
    jobjectArray strArray = NULL;
    int i, array_index = 0;
    jclass stringClass = NULL;
    /* Get properties */
    int res;
    bool ret;
    ilm_struct ilm;
    Properties prop;
    property_value value;
    char buf[32] = {'\0'};
    jstring obj;
    
	unsigned char scanenable;
    unsigned long cod;
    char name[BTMTK_MAX_DEVICE_NAME_SIZE];
    int name_len = BTMTK_MAX_DEVICE_NAME_SIZE;
    unsigned short uuid[BTBM_ADP_MAX_SDAP_UUID_NO];
    int uuid_len = 0;
	int device_count = 0; // adam
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[GAP][API] getAdapterPropertiesNative");

    if (0 == env->PushLocalFrame(PROPERTIES_NREFS) )
        ALOGI("[GAP]PushLocalFrame(%d) success", PROPERTIES_NREFS);
    else
        ALOGI("[GAP]PushLocalFrame(%d) failed", PROPERTIES_NREFS);

    stringClass = env->FindClass("java/lang/String");

	if(nat && nat->event_nat){
		device_count = nat->event_nat->paired_cache_no;
	}
  
    if(btmtk_gap_local_uuid(nat, uuid, &uuid_len))
    {
		if(uuid_len == 0)
		{
			uuid_len = addDefaultProfile(uuid);
		}
        strArray = env->NewObjectArray(BTMTK_ADAPTER_PROP_TOTAL * 2 + device_count + uuid_len, 
							stringClass, NULL);
        ALOGI("[GAP] uuid_len:%d",uuid_len);
    }	
    else 
    {
        strArray = env->NewObjectArray(BTMTK_ADAPTER_PROP_TOTAL * 2 + device_count, stringClass, NULL);
    }

    /* Get address */
    ALOGI("[GAP] Get local address");	
    if(btmtk_gap_get_local_addr(nat, buf))
    {
        /* Add property: adapter_properties[0]: DBUS_TYPE_STRING */
        strcpy(prop.name, "Address");
        prop.type = (int) 's';
        value.str_val = buf;
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
    }

    if( nat && nat->event_nat ){
        /* Get Name */
        ALOGI("[GAP] Get local name");
        name_len = strlen(nat->event_nat->host_cache.name);
        /* Add property: adapter_properties[1]: DBUS_TYPE_STRING */
        strcpy(prop.name, "Name");
        prop.type = (int) 's';
        if(name_len)
        {
            value.str_val = (char*)nat->event_nat->host_cache.name;
        }
        else
        {
            value.str_val = (char*)"None";
        }
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
    }
	
    /* Get Class (CoD) */
    ALOGI("[GAP] Get Class(CoD)");
    if(btmtk_gap_get_local_cod(nat, &cod))
    {
        /* Add property: adapter_properties[2]: DBUS_TYPE_UINT32 */
        strcpy(prop.name, "Class");
        prop.type = (int) 'u';
        value.int_val = cod;
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
    }

    /* Get powered: adapter_properties[3]: DBUS_TYPE_BOOLEAN */
    strcpy(prop.name, "Powered");
    prop.type = (int) 'b';
    value.int_val = (nat->state == BTMTK_POWER_STATE_ON) ? 1 : 0;
    create_prop_array(env, strArray, &prop, &value, 0, &array_index);

    if( nat && nat->event_nat ){
        /* Get Discoverable & pairable */
        ALOGI("[GAP] Get Discoverable & pairable");	
        /* Add discoverable property: adapter_properties[4]: DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "Discoverable");
        prop.type = (int) 'b';
        value.int_val = (nat->event_nat->host_cache.scan_mode & BTBM_ADP_P_OFF_I_ON) ? 1 : 0;
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
        /* Add discoverable property: adapter_properties[6]:  DBUS_TYPE_BOOLEAN */
        strcpy(prop.name, "Pairable");
        prop.type = (int) 'b'; 
        value.int_val = (nat->event_nat->host_cache.scan_mode & BTBM_ADP_P_ON_I_OFF) ? 1 : 0;
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
    
    	/* Get Discoverable timeout */
        strcpy(prop.name, "DiscoverableTimeout");
        prop.type = (int) 'u';
        value.int_val = nat->event_nat->host_cache.scan_mode_timeout;
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
    
        /* TODO : DiscoverableTimeout & PairableTimeout */
        /* Discovering */
        ALOGI("[GAP] Get Discovering");
        strcpy(prop.name, "Discovering");
        prop.type = (int) 'b';
        value.int_val = (nat->event_nat->activity & BTMTK_GAP_ACT_INQUIRY) ? 1 : 0;
        create_prop_array(env, strArray, &prop, &value, 0, &array_index);
        
        /* Devices: adapter_properties[9]: DBUS_TYPE_ARRAY */
        ALOGI("[GAP] Get Devices");
        value.array_val = NULL; 
        strcpy(prop.name, "Devices");
        prop.type = (int) 'a'; 
        if (nat->event_nat->paired_cache_no)
        {
            value.array_val = (char **)calloc(4, nat->event_nat->paired_cache_no);
            for (i = 0; i < nat->event_nat->paired_cache_no; i++)
            {
                value.array_val[i] = (char*)malloc(BTMTK_MAX_OBJECT_PATH_SIZE);
                if(value.array_val[i])
                {
                    btmtk_util_convert_bdaddr2objpath(value.array_val[i], &nat->event_nat->paired_dev_cache[i].addr);
                    ALOGI("[GAP] Get Devices: %s", value.array_val[i]);  
                }
            }
        }
        create_prop_array(env, strArray, &prop, &value, nat->event_nat->paired_cache_no, &array_index);
        if (nat->event_nat->paired_cache_no)
        {
            for (i = 0; i < nat->event_nat->paired_cache_no; i++)
            {
                free(value.array_val[i]);
            }
            free(value.array_val);
        }
    }

    /* Uuid */
    
    if (uuid_len > 0)
    {
    	ALOGI("[GAP] Get UUID");
    	value.array_val = NULL; 
    	strcpy(prop.name, "UUIDs");
    	prop.type = (int) 'a';
	value.array_val = (char **)calloc(4, uuid_len);
	for (i = 0; i < uuid_len; i++)
	{
            value.array_val[i] = (char*)calloc(1, BTMTK_MAX_UUID_STR_SIZE);
	    if(value.array_val[i])
	    {
		int uuid128[16/4];
		btmtk_util_convert_uuid16_2_uuid128((char*)uuid128, uuid[i]);
		btmtk_util_convert_uuid128_2_string(value.array_val[i], (char*)uuid128);
		ALOGI("[GAP] Get UUID: %s", value.array_val[i]); 
		}
	}
    	create_prop_array(env, strArray, &prop, &value, uuid_len, &array_index);
        if (value.array_val)
        {
            for (int i = 0; i < uuid_len; i++)
            {
                free(value.array_val[i]);
            }
            free(value.array_val);
        }
    }
	
    
    //env->PopLocalFrame(NULL);
    //return strArray;
    return (jobjectArray) env->PopLocalFrame(strArray);
#endif

    return NULL;
}

static jboolean setAdapterPropertyNative(JNIEnv *env, jobject object, jstring key,
                                         void *value, jint type) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg;
        DBusMessageIter iter;
        dbus_bool_t reply = JNI_FALSE;
        const char *c_key = env->GetStringUTFChars(key, NULL);

        msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC,
                                           get_adapter_path(env, object),
                                           DBUS_ADAPTER_IFACE, "SetProperty");
        if (!msg) {
            ALOGE("%s: Can't allocate new method call for GetProperties!",
                  __FUNCTION__);
            env->ReleaseStringUTFChars(key, c_key);
            return JNI_FALSE;
        }

        dbus_message_append_args(msg, DBUS_TYPE_STRING, &c_key, DBUS_TYPE_INVALID);
        dbus_message_iter_init_append(msg, &iter);
        append_variant(&iter, type, value);

        // Asynchronous call - the callbacks come via propertyChange
        reply = dbus_connection_send_with_reply(nat->conn, msg, NULL, -1);
        dbus_message_unref(msg);

        env->ReleaseStringUTFChars(key, c_key);
        return reply ? JNI_TRUE : JNI_FALSE;

    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[GAP][API] setAdapterPropertyNative");
    if (nat) 
    {
        bool ret = false;
        const char *c_key = env->GetStringUTFChars(key, NULL);
        ALOGI("key = %s", c_key);
        if(!strcmp(c_key, "Name"))
        {
            ret = btmtk_gap_set_local_name(nat, *(char**)value, strlen(*(char**)value));
        }
        else if(!strcmp(c_key, "Class"))
        {
            ret = btmtk_gap_set_local_cod(nat, *(unsigned long*)value);
        }
        else if(!strcmp(c_key, "Discoverable") || !strcmp(c_key, "Pairable"))
        {
            unsigned char scanablemode = nat->event_nat->host_cache.scan_mode;
            if(!strcmp(c_key, "Pairable"))
            {
                if (*(bool*)value != false)
                {   /* Pairable off */
                    scanablemode |= BTBM_ADP_P_ON_I_OFF;
                }
                else
                {
                    scanablemode &= ~BTBM_ADP_P_ON_I_OFF;
                }
            }
            else
            {
                if (*(bool*)value != false)
                {   /* Discoverable on */
                    scanablemode |= BTBM_ADP_P_OFF_I_ON;
                }
                else
                {  /* Discoverable off */
                    scanablemode &= ~BTBM_ADP_P_OFF_I_ON;                    
                }
            }

            if (scanablemode == nat->event_nat->host_cache.scan_mode)
            {
                /* No need to modify */
                ret = true;
            }
            else
            {
                ret = btmtk_gap_set_scanable_mode(nat, (btbm_scan_enable_type)scanablemode);
            }
        }
	else if(!strcmp(c_key, "DiscoverableTimeout"))
        {
            //The attribute has not been supported by BT stack, so just keep the attribute in host cache
		btmtk_gap_send_scan_mode_timeout_change_event(nat, *(unsigned int*)value);
	}
        else
        {
            ALOGW("[WARN] Unsupported property %s", c_key);
        }
        env->ReleaseStringUTFChars(key, c_key);

        return ret ? JNI_TRUE : JNI_TRUE;
    }
#endif
    return JNI_FALSE;
}

static jboolean setAdapterPropertyStringNative(JNIEnv *env, jobject object, jstring key,
                                               jstring value) {
#ifdef HAVE_BLUETOOTH
    const char *c_value = env->GetStringUTFChars(value, NULL);
    jboolean ret =  setAdapterPropertyNative(env, object, key, (void *)&c_value, DBUS_TYPE_STRING);
    env->ReleaseStringUTFChars(value, (char *)c_value);
    return ret;
#endif

#ifdef __BTMTK__
    const char *c_value = env->GetStringUTFChars(value, NULL);
    ALOGI("[GAP][API] setAdapterPropertyStringNative : %s", c_value);
    jboolean ret =  setAdapterPropertyNative(env, object, key, (void *)&c_value, DBUS_TYPE_STRING);
    env->ReleaseStringUTFChars(value, (char *)c_value);
    return ret;
#endif

    return JNI_FALSE;
}

static jboolean setAdapterPropertyIntegerNative(JNIEnv *env, jobject object, jstring key,
                                               jint value) {
#ifdef HAVE_BLUETOOTH
    return setAdapterPropertyNative(env, object, key, (void *)&value, DBUS_TYPE_UINT32);
#elif defined __BTMTK__
    ALOGI("[GAP][API] setAdapterPropertyIntegerNative");
    return setAdapterPropertyNative(env, object, key, (void *)&value, DBUS_TYPE_UINT32);
#else
    return JNI_FALSE;
#endif
}

static jboolean setAdapterPropertyBooleanNative(JNIEnv *env, jobject object, jstring key,
                                               jint value) {
#ifdef HAVE_BLUETOOTH
    return setAdapterPropertyNative(env, object, key, (void *)&value, DBUS_TYPE_BOOLEAN);
#elif defined __BTMTK__
    ALOGI("[GAP][API] setAdapterPropertyBooleanNative");
    return setAdapterPropertyNative(env, object, key, (void *)&value, DBUS_TYPE_BOOLEAN);
#else
    return JNI_FALSE;
#endif
}

static jboolean setDevicePropertyNative(JNIEnv *env, jobject object, jstring path,
                                               jstring key, void *value, jint type) {
#ifdef HAVE_BLUETOOTH
    ALOGV("%s", __FUNCTION__);
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *msg;
        DBusMessageIter iter;
        dbus_bool_t reply = JNI_FALSE;

        const char *c_key = env->GetStringUTFChars(key, NULL);
        const char *c_path = env->GetStringUTFChars(path, NULL);

        msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC,
                                          c_path, DBUS_DEVICE_IFACE, "SetProperty");
        if (!msg) {
            ALOGE("%s: Can't allocate new method call for device SetProperty!", __FUNCTION__);
            env->ReleaseStringUTFChars(key, c_key);
            env->ReleaseStringUTFChars(path, c_path);
            return JNI_FALSE;
        }

        dbus_message_append_args(msg, DBUS_TYPE_STRING, &c_key, DBUS_TYPE_INVALID);
        dbus_message_iter_init_append(msg, &iter);
        append_variant(&iter, type, value);

        // Asynchronous call - the callbacks come via Device propertyChange
        reply = dbus_connection_send_with_reply(nat->conn, msg, NULL, -1);
        dbus_message_unref(msg);

        env->ReleaseStringUTFChars(path, c_path);
        env->ReleaseStringUTFChars(key, c_key);

        return reply ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
    ALOGI("[GAP][API] setDevicePropertyNative");
    native_data_t *nat = get_native_data(env, object);
   if (nat && nat->event_nat) 
    {
        bool ret = true;
        const char *c_key = env->GetStringUTFChars(key, NULL);
        const char *c_path = env->GetStringUTFChars(path, NULL);
        bt_addr_struct addr;

        ALOGI("key=%s, path=%s", c_key, c_path);
        btmtk_util_convert_objpath2bdaddr(c_path, &addr);

        if(!strcmp(c_key, "Trusted"))
        {
            btmtk_gap_send_device_trusted_event(nat, &addr, *(bool*)value);
        }
        else if(!strcmp(c_key, "Alias")) {
			ALOGI("the string is %s",*(char**)value);
			btmtk_gap_send_paired_device_rename_event(nat, &addr, *(char**)value);
        }
        else
        {
            ALOGW("key %s is not supported", c_key);
            ret = false;
        }
        env->ReleaseStringUTFChars(key, c_key);
        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean setDevicePropertyBooleanNative(JNIEnv *env, jobject object,
                                                     jstring path, jstring key, jint value) {
#ifdef HAVE_BLUETOOTH
    return setDevicePropertyNative(env, object, path, key,
                                        (void *)&value, DBUS_TYPE_BOOLEAN);
#elif defined __BTMTK__
    ALOGI("[GAP][API] setDevicePropertyBooleanNative");
    return setDevicePropertyNative(env, object, path, key,
                                        (void *)&value, DBUS_TYPE_BOOLEAN);
#else
    return JNI_FALSE;
#endif
}

static jboolean setDevicePropertyStringNative(JNIEnv *env, jobject object,
                                              jstring path, jstring key, jstring value) {
#ifdef HAVE_BLUETOOTH
    const char *c_value = env->GetStringUTFChars(value, NULL);
    jboolean ret = setDevicePropertyNative(env, object, path, key,
                                           (void *)&c_value, DBUS_TYPE_STRING);
    env->ReleaseStringUTFChars(value, (char *)c_value);
    return ret;
#elif defined __BTMTK__
	const char *c_value = env->GetStringUTFChars(value, NULL);
	ALOGI("[GAP][API] setDevicePropertyStringNative : %s", c_value);
	jboolean ret = setDevicePropertyNative(env, object, path, key,
                                        (void *)&c_value, DBUS_TYPE_STRING);
	env->ReleaseStringUTFChars(value, (char *)c_value);
	return ret;
#else
    return JNI_FALSE;
#endif
}

static jboolean createDeviceNative(JNIEnv *env, jobject object,
                                                jstring address) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        ALOGV("... address = %s", c_address);
        char *context_address = (char *)calloc(BTADDR_SIZE, sizeof(char));
        strlcpy(context_address, c_address, BTADDR_SIZE);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1,
                                        onCreateDeviceResult,
                                        context_address,
                                        eventLoopNat,
                                        get_adapter_path(env, object),
                                        DBUS_ADAPTER_IFACE,
                                        "CreateDevice",
                                        DBUS_TYPE_STRING, &c_address,
                                        DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(address, c_address);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    
    if (nat)
    {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        bt_addr_struct addr;
        bool ret = false;

        ALOGI("[GAP][API] createDeviceNative : addr=%s", c_address);
        btmtk_util_convert_string2bdaddr((char*)c_address, &addr);
        if((ret = btmtk_gap_service_search_request(nat, &addr)) == true)
        {
            btmtk_gap_send_sdp_device_create_event(nat, &addr);
        }
        env->ReleaseStringUTFChars(address, c_address);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

    return JNI_FALSE;
}

static jboolean discoverServicesNative(JNIEnv *env, jobject object,
                                               jstring path, jstring pattern) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        const char *c_pattern = env->GetStringUTFChars(pattern, NULL);
        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        ALOGV("... Object Path = %s", c_path);
        ALOGV("... Pattern = %s, strlen = %d", c_pattern, strlen(c_pattern));

        bool ret = dbus_func_args_async(env, nat->conn, -1,
                                        onDiscoverServicesResult,
                                        context_path,
                                        eventLoopNat,
                                        c_path,
                                        DBUS_DEVICE_IFACE,
                                        "DiscoverServices",
                                        DBUS_TYPE_STRING, &c_pattern,
                                        DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(path, c_path);
        env->ReleaseStringUTFChars(pattern, c_pattern);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    const char *c_path = env->GetStringUTFChars(path, NULL);
    const char *c_pattern = env->GetStringUTFChars(pattern, NULL);
    bt_addr_struct addr;
    bool ret = false;
    
    ALOGI("[GAP][API] discoverServicesNative : addr=%s, pattern=%s", c_path, c_pattern);
    if (nat && c_path)
    {
        btmtk_util_convert_objpath2bdaddr((char*)c_path, &addr);
        if (c_pattern && strlen(c_pattern))
        {
            char uuid128[BTMTK_SDP_UUID_128_BIT_SIZE], buf[BT_MAX_SDAP_RAW_SIZE];
            int idx = 0;

            btmtk_util_convert_string_2_uuid128(uuid128, c_pattern);
            buf[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_SEQUENCE, BTMTK_SDP_UUID_128_BIT_SIZE +1); 
            idx += btmtk_jsr82_int_write_size_bytes(buf + idx, BTMTK_SDP_UUID_128_BIT_SIZE + 1);
            buf[idx++] = btmtk_jsr82_int_get_data_element_header(BTMTK_SDP_ELEM_UUID, BTMTK_SDP_UUID_128_BIT_SIZE);
            memcpy(buf + idx, uuid128, BTMTK_SDP_UUID_128_BIT_SIZE);
            idx += BTMTK_SDP_UUID_128_BIT_SIZE;
            if (idx < BT_MAX_SDAP_RAW_SIZE)
            {
                if((ret = btmtk_gap_service_search_raw_request(nat, &addr, buf, idx)) == true)
                {
                    btmtk_gap_send_sdp_discover_event(nat, &addr, uuid128, 16);
                }
            }
            else
            {
                ALOGE("[GAP][API] discoverServicesNative : idx=%d exceed max %d", idx, BT_MAX_SDAP_RAW_SIZE);
            }
        }
        else
        {
            if((ret = btmtk_gap_service_search_request(nat, &addr)) == true)
            {
                btmtk_gap_send_sdp_discover_event(nat, &addr, c_pattern, 0);
            }
        }
    }
    env->ReleaseStringUTFChars(path, c_path);
    env->ReleaseStringUTFChars(pattern, c_pattern);
    return ret ? JNI_TRUE : JNI_FALSE;
#endif

    return JNI_FALSE;
}

#ifdef HAVE_BLUETOOTH
static jintArray extract_handles(JNIEnv *env, DBusMessage *reply) {
    jint *handles;
    jintArray handleArray = NULL;
    int len;

    DBusError err;
    dbus_error_init(&err);

    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &handles, &len,
                              DBUS_TYPE_INVALID)) {
        handleArray = env->NewIntArray(len);
        if (handleArray) {
            env->SetIntArrayRegion(handleArray, 0, len, handles);
        } else {
            ALOGE("Null array in extract_handles");
        }
    } else {
        LOG_AND_FREE_DBUS_ERROR(&err);
    }
    return handleArray;
}
#endif

static jintArray addReservedServiceRecordsNative(JNIEnv *env, jobject object,
                                                jintArray uuids) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    DBusMessage *reply = NULL;

    native_data_t *nat = get_native_data(env, object);

    jint* svc_classes = env->GetIntArrayElements(uuids, NULL);
    if (!svc_classes) return NULL;

    int len = env->GetArrayLength(uuids);
    reply = dbus_func_args(env, nat->conn,
                            get_adapter_path(env, object),
                            DBUS_ADAPTER_IFACE, "AddReservedServiceRecords",
                            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32,
                            &svc_classes, len, DBUS_TYPE_INVALID);
    env->ReleaseIntArrayElements(uuids, svc_classes, 0);
    return reply ? extract_handles(env, reply) : NULL;

#endif
    return NULL;
}

static jboolean removeReservedServiceRecordsNative(JNIEnv *env, jobject object,
                                                   jintArray handles) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jint *values = env->GetIntArrayElements(handles, NULL);
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    if (values == NULL) return JNI_FALSE;

    jsize len = env->GetArrayLength(handles);

    reply = dbus_func_args(env, nat->conn,
                            get_adapter_path(env, object),
                            DBUS_ADAPTER_IFACE, "RemoveReservedServiceRecords",
                            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32,
                            &values, len, DBUS_TYPE_INVALID);
    env->ReleaseIntArrayElements(handles, values, 0);
    return reply ? JNI_TRUE : JNI_FALSE;
#endif
    return JNI_FALSE;
}

static jint addRfcommServiceRecordNative(JNIEnv *env, jobject object,
        jstring name, jlong uuidMsb, jlong uuidLsb, jshort channel) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_name = env->GetStringUTFChars(name, NULL);
        ALOGV("... name = %s", c_name);
        ALOGV("... uuid1 = %llX", uuidMsb);
        ALOGV("... uuid2 = %llX", uuidLsb);
        ALOGV("... channel = %d", channel);
        DBusMessage *reply = dbus_func_args(env, nat->conn,
                           get_adapter_path(env, object),
                           DBUS_ADAPTER_IFACE, "AddRfcommServiceRecord",
                           DBUS_TYPE_STRING, &c_name,
                           DBUS_TYPE_UINT64, &uuidMsb,
                           DBUS_TYPE_UINT64, &uuidLsb,
                           DBUS_TYPE_UINT16, &channel,
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(name, c_name);
        return reply ? dbus_returns_uint32(env, reply) : -1;
    }
#endif
#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    if (nat) 
    {
        const char *c_name = env->GetStringUTFChars(name, NULL);
        char uuid[BTMTK_SDP_UUID_128_BIT_SIZE];
        char *rec;
        int rec_len;

        ALOGI("[GAP][API] addRfcommServiceRecordNative : name=%s, uuidMsb=%llX, uuidLsb=%llX, channel=%d", c_name, uuidMsb, uuidLsb, channel);
        btmtk_util_convert_juuid_2_uuid128(uuid, uuidMsb, uuidLsb);
        rec_len = btmtk_jsr82_int_compose_record(&rec, c_name, uuid, channel);
        env->ReleaseStringUTFChars(name, c_name);
        return btmtk_jsr82_create_record(nat, &rec, rec_len);
    }
#endif
    return -1;
}

static jboolean removeServiceRecordNative(JNIEnv *env, jobject object, jint handle) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        ALOGV("... handle = %X", handle);
        DBusMessage *reply = dbus_func_args(env, nat->conn,
                           get_adapter_path(env, object),
                           DBUS_ADAPTER_IFACE, "RemoveServiceRecord",
                           DBUS_TYPE_UINT32, &handle,
                           DBUS_TYPE_INVALID);
        return reply ? JNI_TRUE : JNI_FALSE;
    }
#endif
#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);
    bool ret = false;
    if (nat) 
    {
        ALOGI("[GAP][API] removeServiceRecordNative : handle=%d", handle);
        ret = btmtk_jsr82_remove_record(nat, handle);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean setLinkTimeoutNative(JNIEnv *env, jobject object, jstring object_path,
                                     jint num_slots) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_object_path = env->GetStringUTFChars(object_path, NULL);
        DBusMessage *reply = dbus_func_args(env, nat->conn,
                           get_adapter_path(env, object),
                           DBUS_ADAPTER_IFACE, "SetLinkTimeout",
                           DBUS_TYPE_OBJECT_PATH, &c_object_path,
                           DBUS_TYPE_UINT32, &num_slots,
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(object_path, c_object_path);
        return reply ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean setSSPDebugModeNative(JNIEnv *env, jobject object, jboolean on) {
    ALOGV(__FUNCTION__);
#ifdef __BTMTK__
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[GAP][API] setSSPDebugModeNative");
    if (nat)
    {
        return btmtk_gap_set_ssp_debug_mode(nat, on);
    }
#endif
    return JNI_FALSE;
}

static jboolean connectInputDeviceNative(JNIEnv *env, jobject object, jstring path) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);

        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1, onInputDeviceConnectionResult,
                                        context_path, eventLoopNat, c_path, DBUS_INPUT_IFACE,
                                        "Connect",
                                        DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean disconnectInputDeviceNative(JNIEnv *env, jobject object,
                                     jstring path) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);

        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1, onInputDeviceConnectionResult,
                                        context_path, eventLoopNat, c_path, DBUS_INPUT_IFACE,
                                        "Disconnect",
                                        DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean setBluetoothTetheringNative(JNIEnv *env, jobject object, jboolean value,
                                            jstring src_role, jstring bridge) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        DBusMessage *reply;
        const char *c_role = env->GetStringUTFChars(src_role, NULL);
        const char *c_bridge = env->GetStringUTFChars(bridge, NULL);
        if (value) {
            ALOGE("setBluetoothTetheringNative true");
            reply = dbus_func_args(env, nat->conn,
                                  get_adapter_path(env, object),
                                  DBUS_NETWORKSERVER_IFACE,
                                  "Register",
                                  DBUS_TYPE_STRING, &c_role,
                                  DBUS_TYPE_STRING, &c_bridge,
                                  DBUS_TYPE_INVALID);
        } else {
            ALOGE("setBluetoothTetheringNative false");
            reply = dbus_func_args(env, nat->conn,
                                  get_adapter_path(env, object),
                                  DBUS_NETWORKSERVER_IFACE,
                                  "Unregister",
                                  DBUS_TYPE_STRING, &c_role,
                                  DBUS_TYPE_INVALID);
        }
        env->ReleaseStringUTFChars(src_role, c_role);
        env->ReleaseStringUTFChars(bridge, c_bridge);
        return reply ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean connectPanDeviceNative(JNIEnv *env, jobject object, jstring path,
                                       jstring dstRole) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    ALOGE("connectPanDeviceNative");
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        const char *dst = env->GetStringUTFChars(dstRole, NULL);

        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1,onPanDeviceConnectionResult,
                                    context_path, eventLoopNat, c_path,
                                    DBUS_NETWORK_IFACE, "Connect",
                                    DBUS_TYPE_STRING, &dst,
                                    DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);
        env->ReleaseStringUTFChars(dstRole, dst);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean disconnectPanDeviceNative(JNIEnv *env, jobject object,
                                     jstring path) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    ALOGE("disconnectPanDeviceNative");
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);

        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1,onPanDeviceConnectionResult,
                                        context_path, eventLoopNat, c_path,
                                        DBUS_NETWORK_IFACE, "Disconnect",
                                        DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jboolean disconnectPanServerDeviceNative(JNIEnv *env, jobject object,
                                                jstring path, jstring address,
                                                jstring iface) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    ALOGE("disconnectPanServerDeviceNative");
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_address = env->GetStringUTFChars(address, NULL);
        const char *c_path = env->GetStringUTFChars(path, NULL);
        const char *c_iface = env->GetStringUTFChars(iface, NULL);

        int len = env->GetStringLength(path) + 1;
        char *context_path = (char *)calloc(len, sizeof(char));
        strlcpy(context_path, c_path, len);  // for callback

        bool ret = dbus_func_args_async(env, nat->conn, -1,
                                        onPanDeviceConnectionResult,
                                        context_path, eventLoopNat,
                                        get_adapter_path(env, object),
                                        DBUS_NETWORKSERVER_IFACE,
                                        "DisconnectDevice",
                                        DBUS_TYPE_STRING, &c_address,
                                        DBUS_TYPE_STRING, &c_iface,
                                        DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(address, c_address);
        env->ReleaseStringUTFChars(iface, c_iface);
        env->ReleaseStringUTFChars(path, c_path);
        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif
    return JNI_FALSE;
}

static jstring registerHealthApplicationNative(JNIEnv *env, jobject object,
                                           jint dataType, jstring role,
                                           jstring name, jstring channelType) {
    ALOGV("%s", __FUNCTION__);
    jstring path = NULL;
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_role = env->GetStringUTFChars(role, NULL);
        const char *c_name = env->GetStringUTFChars(name, NULL);
        const char *c_channel_type = env->GetStringUTFChars(channelType, NULL);
        char *c_path;
        DBusMessage *msg, *reply;
        DBusError err;
        dbus_error_init(&err);

        msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC,
                                            DBUS_HEALTH_MANAGER_PATH,
                                            DBUS_HEALTH_MANAGER_IFACE,
                                            "CreateApplication");

        if (msg == NULL) {
            ALOGE("Could not allocate D-Bus message object!");
            return NULL;
        }

        /* append arguments */
        append_dict_args(msg,
                         "DataType", DBUS_TYPE_UINT16, &dataType,
                         "Role", DBUS_TYPE_STRING, &c_role,
                         "Description", DBUS_TYPE_STRING, &c_name,
                         "ChannelType", DBUS_TYPE_STRING, &c_channel_type,
                         DBUS_TYPE_INVALID);


        /* Make the call. */
        reply = dbus_connection_send_with_reply_and_block(nat->conn, msg, -1, &err);

        env->ReleaseStringUTFChars(role, c_role);
        env->ReleaseStringUTFChars(name, c_name);
        env->ReleaseStringUTFChars(channelType, c_channel_type);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            }
        } else {
            if (!dbus_message_get_args(reply, &err,
                                      DBUS_TYPE_OBJECT_PATH, &c_path,
                                      DBUS_TYPE_INVALID)) {
                if (dbus_error_is_set(&err)) {
                    LOG_AND_FREE_DBUS_ERROR(&err);
                }
            } else {
               path = env->NewStringUTF(c_path);
            }
            dbus_message_unref(reply);
        }
    }
#endif

#ifdef __BTMTK__
	native_data_t *nat = get_native_data(env, object);
	const char *c_role = env->GetStringUTFChars(role, NULL);
	const char *c_name = env->GetStringUTFChars(name, NULL);
	const char *c_channel_type = env->GetStringUTFChars(channelType, NULL);

	ALOGI("[HDP]registerHealthApplicationNative: role-> %s, channel type->%s, name->%s",
				c_role, c_channel_type, c_name);
    if (nat) {

		U8 id = BT_HDP_INSTANCE_INVALID_ID;
		bool ret = FALSE;

		U8 i_role = btmtk_hdp_int_string2role(c_role);
		U8 i_type = btmtk_hdp_int_string2channel(c_channel_type);

		if (BT_HDP_ROLE_INVALID != i_role)
		{
			ret = btmtk_hdp_register_instance(nat, i_role, i_type, (U16)dataType, c_name, &id);
			if (id != BT_HDP_INSTANCE_INVALID_ID)
			{
				char c_mdepId[4];
				sprintf(c_mdepId, "%d", id);
				path = env->NewStringUTF(c_mdepId);
			}
			else
			{
				ALOGE("[HDP]fail to register APP: result %d, id %d", ret, id);
			}
		} 
                else
		{
			ALOGE("[HDP] invalid parms: role %d, type %d", i_role, i_type);
		}
    }
	env->ReleaseStringUTFChars(role, c_role);
    env->ReleaseStringUTFChars(name, c_name);
    env->ReleaseStringUTFChars(channelType, c_channel_type);
#endif
    return path;
}

static jstring registerSinkHealthApplicationNative(JNIEnv *env, jobject object,
                                           jint dataType, jstring role,
                                           jstring name) {
    ALOGV("%s", __FUNCTION__);
    jstring path = NULL;
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_role = env->GetStringUTFChars(role, NULL);
        const char *c_name = env->GetStringUTFChars(name, NULL);
        char *c_path;

        DBusMessage *msg, *reply;
        DBusError err;
        dbus_error_init(&err);

        msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC,
                                            DBUS_HEALTH_MANAGER_PATH,
                                            DBUS_HEALTH_MANAGER_IFACE,
                                            "CreateApplication");

        if (msg == NULL) {
            ALOGE("Could not allocate D-Bus message object!");
            return NULL;
        }

        /* append arguments */
        append_dict_args(msg,
                         "DataType", DBUS_TYPE_UINT16, &dataType,
                         "Role", DBUS_TYPE_STRING, &c_role,
                         "Description", DBUS_TYPE_STRING, &c_name,
                         DBUS_TYPE_INVALID);


        /* Make the call. */
        reply = dbus_connection_send_with_reply_and_block(nat->conn, msg, -1, &err);

        env->ReleaseStringUTFChars(role, c_role);
        env->ReleaseStringUTFChars(name, c_name);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            }
        } else {
            if (!dbus_message_get_args(reply, &err,
                                      DBUS_TYPE_OBJECT_PATH, &c_path,
                                      DBUS_TYPE_INVALID)) {
                if (dbus_error_is_set(&err)) {
                    LOG_AND_FREE_DBUS_ERROR(&err);
                }
            } else {
                path = env->NewStringUTF(c_path);
            }
            dbus_message_unref(reply);
        }
    }
#endif
#ifdef __BTMTK__
	native_data_t *nat = get_native_data(env, object);
	ALOGI("[HDP]registerSinkHealthApplicationNative");
	if (nat) {
		const char *c_role = env->GetStringUTFChars(role, NULL);
		const char *c_name = env->GetStringUTFChars(name, NULL);
		U8 id = BT_HDP_INSTANCE_INVALID_ID;
		bool ret = FALSE;
		U8 i_role = btmtk_hdp_int_string2role(c_role);
		U8 i_type = btmtk_hdp_int_string2channel(NULL);
	
		if (BT_HDP_ROLE_INVALID != i_role)
		{
			ret = btmtk_hdp_register_instance(nat, i_role, i_type, (U16)dataType, c_name, &id);
			if (FALSE != ret && id != BT_HDP_INSTANCE_INVALID_ID)
			{
				char c_mdepId[4];
				sprintf(c_mdepId, "%d", id);
				path = env->NewStringUTF(c_mdepId);
			}		
			else
			{
				ALOGE("[HDP]fail to register APP: result %d, id %d", ret, id);
			}
		}
		else
		{
			ALOGE("[HDP] invalid parms: role %d, type %d", i_role, i_type);
		}
	
		
		env->ReleaseStringUTFChars(role, c_role);
       	env->ReleaseStringUTFChars(name, c_name);
	}
#endif

    return path;
}

static jboolean unregisterHealthApplicationNative(JNIEnv *env, jobject object,
                                                    jstring path) {
    ALOGV("%s", __FUNCTION__);
    jboolean result = JNI_FALSE;
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_path = env->GetStringUTFChars(path, NULL);
        DBusError err;
        dbus_error_init(&err);
        DBusMessage *reply =
            dbus_func_args_timeout(env, nat->conn, -1,
                                   DBUS_HEALTH_MANAGER_PATH,
                                   DBUS_HEALTH_MANAGER_IFACE, "DestroyApplication",
                                   DBUS_TYPE_OBJECT_PATH, &c_path,
                                   DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(path, c_path);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            }
        } else {
            result = JNI_TRUE;
        }
    }
#endif

#ifdef __BTMTK__
	native_data_t *nat = get_native_data(env, object);
	if (nat) {
		const char *c_path = env->GetStringUTFChars(path, NULL);

		unsigned char addr[6];
    	U8 id;
        id = atoi(c_path);
	
		result = btmtk_hdp_deregister_instance(nat, id) ? JNI_TRUE : JNI_FALSE;		
		env->ReleaseStringUTFChars(path, c_path);
	}
#endif
    return result;
}

static jboolean createChannelNative(JNIEnv *env, jobject object,
                                       jstring devicePath, jstring appPath, jstring config,
                                       jint code) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_device_path = env->GetStringUTFChars(devicePath, NULL);
        const char *c_app_path = env->GetStringUTFChars(appPath, NULL);
        const char *c_config = env->GetStringUTFChars(config, NULL);
        int *data = (int *) malloc(sizeof(int));
        if (data == NULL) return JNI_FALSE;

        *data = code;
        bool ret = dbus_func_args_async(env, nat->conn, -1, onHealthDeviceConnectionResult,
                                        data, eventLoopNat, c_device_path,
                                        DBUS_HEALTH_DEVICE_IFACE, "CreateChannel",
                                        DBUS_TYPE_OBJECT_PATH, &c_app_path,
                                        DBUS_TYPE_STRING, &c_config,
                                        DBUS_TYPE_INVALID);


        env->ReleaseStringUTFChars(devicePath, c_device_path);
        env->ReleaseStringUTFChars(appPath, c_app_path);
        env->ReleaseStringUTFChars(config, c_config);

        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

#ifdef __BTMTK__
	ALOGI("[HDP]createChannelNative");
	native_data_t *nat = get_native_data(env, object);
	if (nat) {
		const char *c_devicepath = env->GetStringUTFChars(devicePath, NULL);
		const char *c_appPath = env->GetStringUTFChars(appPath, NULL);
		const char *c_config = env->GetStringUTFChars(config, NULL);
        bt_addr_struct addr;
		
    	U8 id;
		U8 i_config; 
		bool ret;		

		id = atoi(c_appPath);
		btmtk_util_convert_objpath2bdaddr(c_devicepath, &addr);
		i_config = btmtk_hdp_int_string2channel(c_config);
		
		ret = btmtk_hdp_connect(nat, &addr, id, i_config, code);
		
		env->ReleaseStringUTFChars(devicePath, c_devicepath);
		env->ReleaseStringUTFChars(appPath, c_appPath);
		env->ReleaseStringUTFChars(config, c_config);
		return ret ? JNI_TRUE : JNI_FALSE;
	}
#endif

	
    return JNI_FALSE;
}

static jboolean destroyChannelNative(JNIEnv *env, jobject object, jstring devicePath,
                                     jstring channelPath, jint code) {
    ALOGE("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    jobject eventLoop = env->GetObjectField(object, field_mEventLoop);
    struct event_loop_native_data_t *eventLoopNat =
            get_EventLoop_native_data(env, eventLoop);

    if (nat && eventLoopNat) {
        const char *c_device_path = env->GetStringUTFChars(devicePath, NULL);
        const char *c_channel_path = env->GetStringUTFChars(channelPath, NULL);
        int *data = (int *) malloc(sizeof(int));
        if (data == NULL) return JNI_FALSE;

        *data = code;
        bool ret = dbus_func_args_async(env, nat->conn, -1, onHealthDeviceConnectionResult,
                                        data, eventLoopNat, c_device_path,
                                        DBUS_HEALTH_DEVICE_IFACE, "DestroyChannel",
                                        DBUS_TYPE_OBJECT_PATH, &c_channel_path,
                                        DBUS_TYPE_INVALID);

        env->ReleaseStringUTFChars(devicePath, c_device_path);
        env->ReleaseStringUTFChars(channelPath, c_channel_path);

        return ret ? JNI_TRUE : JNI_FALSE;
    }
#endif

#ifdef __BTMTK__
	native_data_t *nat = get_native_data(env, object);
	bool ret;
	ALOGI("[HDP]destroyChannelNative");
	if (nat) {
		const char *c_devicePath = env->GetStringUTFChars(devicePath, NULL);
		const char *c_channelPath = env->GetStringUTFChars(channelPath, NULL);
		bt_addr_struct addr;
		
		U16 mdlId;
		ret = btmtk_hdp_parse_channel(c_channelPath, &addr,&mdlId);
		if (TRUE == ret)
		{
			btmtk_util_convert_objpath2bdaddr(c_devicePath, &addr);			
			ret = btmtk_hdp_disconnect(nat, &addr, mdlId, code);	
		}
		else
		{
			 ALOGI("[HDP][API] destroyChannelNative: invalide parms");
			 /*Although Java layer input an invalid parms, the operation is still considered success in*/
			 /*order to avoid a bug in BluetoothHealthProfileHandler when disconnecting channel*/
			 ret = TRUE;
		}
			
			
		env->ReleaseStringUTFChars(devicePath, c_devicePath);
		env->ReleaseStringUTFChars(channelPath, c_channelPath);
		return ret ? JNI_TRUE : JNI_FALSE;
	}
#endif

    return JNI_FALSE;
}

static jstring getMainChannelNative(JNIEnv *env, jobject object, jstring devicePath) {
    ALOGE("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_device_path = env->GetStringUTFChars(devicePath, NULL);
        DBusError err;
        dbus_error_init(&err);

        DBusMessage *reply = dbus_func_args(env, nat->conn,
                           c_device_path,
                           DBUS_HEALTH_DEVICE_IFACE, "GetProperties",
                           DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(devicePath, c_device_path);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            }
        } else {
            DBusMessageIter iter;
            jobjectArray str_array = NULL;
            if (dbus_message_iter_init(reply, &iter))
                str_array = parse_health_device_properties(env, &iter);
            dbus_message_unref(reply);
            jstring path = (jstring) env->GetObjectArrayElement(str_array, 1);

            return path;
        }
    }
#endif

#ifdef __BTMTK__
	/* the channel path should contain both device address and mdl ID */

	native_data_t *nat = get_native_data(env, object);
    const char *c_address = env->GetStringUTFChars(devicePath, NULL);
	bool ret; 
	U8 mdlId;
	jstring path = NULL;
	char channelPath[50]; 

    ALOGI("[HDP][API] getMainChannelNative: addr=%s", c_address);
    if(nat && c_address)
    {
        bt_addr_struct addr;
		btmtk_util_convert_objpath2bdaddr(c_address, &addr);

		ret = btmtk_hdp_get_main_channel(nat, &addr, &mdlId); 
		if (TRUE == ret)
		{
			btmtk_hdp_compose_channel(&addr, mdlId, channelPath);
			return env->NewStringUTF(channelPath);
		}
    }
    env->ReleaseStringUTFChars(devicePath, c_address);	
	return path;
#endif
    return NULL;
}

static jstring getChannelApplicationNative(JNIEnv *env, jobject object, jstring channelPath) {
    ALOGE("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_channel_path = env->GetStringUTFChars(channelPath, NULL);
        DBusError err;
        dbus_error_init(&err);

        DBusMessage *reply = dbus_func_args(env, nat->conn,
                                            c_channel_path,
                                            DBUS_HEALTH_CHANNEL_IFACE, "GetProperties",
                                            DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(channelPath, c_channel_path);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            }
        } else {
            DBusMessageIter iter;
            jobjectArray str_array = NULL;
            if (dbus_message_iter_init(reply, &iter))
                str_array = parse_health_channel_properties(env, &iter);
            dbus_message_unref(reply);

            jint len = env->GetArrayLength(str_array);

            jstring name, path;
            const char *c_name;

            for (int i = 0; i < len; i+=2) {
                name = (jstring) env->GetObjectArrayElement(str_array, i);
                c_name = env->GetStringUTFChars(name, NULL);

                if (!strcmp(c_name, "Application")) {
                    path = (jstring) env->GetObjectArrayElement(str_array, i+1);
                    env->ReleaseStringUTFChars(name, c_name);
                    return path;
                }
                env->ReleaseStringUTFChars(name, c_name);
            }
        }
    }
#endif

#ifdef __BTMTK__
	native_data_t *nat = get_native_data(env, object);
	if(nat)
	{
		const char *c_channelPath = env->GetStringUTFChars(channelPath, NULL);
		bt_addr_struct addr;
		jstring path = NULL;
		bool ret = FALSE;
		char c_mdepId[4];

		U16 mdlId;
		U8 mdepId;
		ret = btmtk_hdp_parse_channel(c_channelPath, &addr,&mdlId);
		if (ret)
		{
			ret = btmtk_hdp_get_instance(nat, &addr, mdlId, &mdepId); //id: mdl ID
			if (ret)
			{
				sprintf(c_mdepId, "%d", mdepId);
				path = env->NewStringUTF(c_mdepId);
			}

		}
		env->ReleaseStringUTFChars(channelPath, c_channelPath);	
		return path;
	}
#endif

    return NULL;
}

static jboolean releaseChannelFdNative(JNIEnv *env, jobject object, jstring channelPath) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_channel_path = env->GetStringUTFChars(channelPath, NULL);
        DBusError err;
        dbus_error_init(&err);

        DBusMessage *reply = dbus_func_args(env, nat->conn,
                                            c_channel_path,
                                            DBUS_HEALTH_CHANNEL_IFACE, "Release",
                                            DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(channelPath, c_channel_path);

        return reply ? JNI_TRUE : JNI_FALSE;
    }
#endif

#ifdef __BTMTK__
		native_data_t *nat = get_native_data(env, object);
		const char *c_channelPath = env->GetStringUTFChars(channelPath, NULL);
		bool ret = FALSE;
		ALOGE("[HDP] releaseChannelFdNative: %s", c_channelPath);
		
		if(nat && c_channelPath)
		{
			bt_addr_struct addr;			
			U16 mdlId;
			U16 l2capChnId;
			int socketFd = -1;
				
			ret = btmtk_hdp_parse_channel(c_channelPath, &addr, &mdlId);
			if (ret)
			{
		//		pthread_mutex_lock(&(nat->event_nat->thread_mutex));
				btmtk_hdp_remove_fd(nat, &addr, mdlId);	
		//		pthread_mutex_unlock(&(nat->event_nat->thread_mutex));
			}
			else
			{
				ALOGE("[HDP] releaseChannelFdNative: invalid parm");
			}
		}
		env->ReleaseStringUTFChars(channelPath, c_channelPath); 
		if (ret)
		{
			return JNI_TRUE;
		}
#endif

    return JNI_FALSE;
}




static jobject getChannelFdNative(JNIEnv *env, jobject object, jstring channelPath) {
    ALOGV("%s", __FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        const char *c_channel_path = env->GetStringUTFChars(channelPath, NULL);
        int32_t fd;
        DBusError err;
        dbus_error_init(&err);

        DBusMessage *reply = dbus_func_args(env, nat->conn,
                                            c_channel_path,
                                            DBUS_HEALTH_CHANNEL_IFACE, "Acquire",
                                            DBUS_TYPE_INVALID);
        env->ReleaseStringUTFChars(channelPath, c_channel_path);

        if (!reply) {
            if (dbus_error_is_set(&err)) {
                LOG_AND_FREE_DBUS_ERROR(&err);
            }
            return NULL;
        }

        fd = dbus_returns_unixfd(env, reply);
        if (fd == -1) return NULL;

        int flags = fcntl(fd, F_GETFL);
        if (flags < 0) {
           ALOGE("Can't get flags with fcntl(): %s (%d)",
                                strerror(errno), errno);
           releaseChannelFdNative(env, object, channelPath);
           close(fd);
           return NULL;
        }

        flags &= ~O_NONBLOCK;
        int status = fcntl(fd, F_SETFL, flags);
        if (status < 0) {
           ALOGE("Can't set flags with fcntl(): %s (%d)",
               strerror(errno), errno);
           releaseChannelFdNative(env, object, channelPath);
           close(fd);
           return NULL;
        }

        // Create FileDescriptor object
        jobject fileDesc = jniCreateFileDescriptor(env, fd);
        if (fileDesc == NULL) {
            // FileDescriptor constructor has thrown an exception
            releaseChannelFdNative(env, object, channelPath);
            close(fd);
            return NULL;
        }

        // Wrap it in a ParcelFileDescriptor
        jobject parcelFileDesc = newParcelFileDescriptor(env, fileDesc);
        if (parcelFileDesc == NULL) {
            // ParcelFileDescriptor constructor has thrown an exception
            releaseChannelFdNative(env, object, channelPath);
            close(fd);
            return NULL;
        }

        return parcelFileDesc;
    }
#endif

#ifdef __BTMTK__
		native_data_t *nat = get_native_data(env, object);
		const char *c_channelPath = env->GetStringUTFChars(channelPath, NULL);
		btmtk_hdp_fd_struct * fd = NULL;
		jobject parcelFileDesc = NULL;

		ALOGE("[HDP] getChannelFdNative invalid parm:%s", c_channelPath);
		if(nat && c_channelPath)
		{
			bt_addr_struct addr;			
			U16 mdlId;
			U16 l2capChnId;
			bool ret = FALSE;
			int socketFd = -1;
			btmtk_hdp_fd_struct *hdp_fd = NULL;
				
			ret = btmtk_hdp_parse_channel(c_channelPath, &addr, &mdlId);
			if (TRUE == ret)
			{
				/*Create socket FD when channale is created, and add the fd to list*/
				hdp_fd = btmtk_hdp_util_find_hdp_fd(nat, &addr, mdlId);
				if (NULL != hdp_fd && -1 != hdp_fd->fd) 
				{		
					socketFd = hdp_fd->fd;
				} 
				else 
				{
					ret = btmtk_hdp_get_l2cap_channel(nat, &addr, mdlId, &l2capChnId);
					if (TRUE == ret)
					{
						socketFd = btmtk_hdp_create_socket(l2capChnId);
						if (socketFd >= 0)
						{
							if (NULL == hdp_fd) 
							{
								btmtk_hdp_add_fd(nat, &addr, mdlId, socketFd);    
							}
							else 
							{
								hdp_fd->fd = socketFd;
							}
						}
					}
					else
					{
						ALOGE("[HDP] getChannelFdNative: fail to get l2cap channel");
					}
				}
			}
			else 
			{
				ALOGE("[HDP] getChannelFdNative invalid parm");
			}

			if (socketFd >= 0)
			{
				jobject fileDesc = jniCreateFileDescriptor(env, socketFd);
        		if (fileDesc == NULL) {
           			// FileDescriptor constructor has thrown an exception
            		releaseChannelFdNative(env, object, channelPath);
            		close(socketFd);
        		}
				else 
				{
        			parcelFileDesc = newParcelFileDescriptor(env, fileDesc);
        			if (parcelFileDesc == NULL) {
            			releaseChannelFdNative(env, object, channelPath);
            			close(socketFd);
        			}
				}
			}
		}
		env->ReleaseStringUTFChars(channelPath, c_channelPath); 
		return parcelFileDesc;
#endif
	return NULL;

}



static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"classInitNative", "()V", (void*)classInitNative},
    {"initializeNativeDataNative", "()V", (void *)initializeNativeDataNative},
    {"setupNativeDataNative", "()Z", (void *)setupNativeDataNative},
    {"tearDownNativeDataNative", "()Z", (void *)tearDownNativeDataNative},
    {"cleanupNativeDataNative", "()V", (void *)cleanupNativeDataNative},
    {"getAdapterPathNative", "()Ljava/lang/String;", (void*)getAdapterPathNative},

    {"isEnabledNative", "()I", (void *)isEnabledNative},
    {"enableNative", "()I", (void *)enableNative},
    {"disableNative", "()I", (void *)disableNative},

    {"getAdapterPropertiesNative", "()[Ljava/lang/Object;", (void *)getAdapterPropertiesNative},
    {"getDevicePropertiesNative", "(Ljava/lang/String;)[Ljava/lang/Object;",
      (void *)getDevicePropertiesNative},
    {"setAdapterPropertyStringNative", "(Ljava/lang/String;Ljava/lang/String;)Z",
      (void *)setAdapterPropertyStringNative},
    {"setAdapterPropertyBooleanNative", "(Ljava/lang/String;I)Z",
      (void *)setAdapterPropertyBooleanNative},
    {"setAdapterPropertyIntegerNative", "(Ljava/lang/String;I)Z",
      (void *)setAdapterPropertyIntegerNative},

    {"startDiscoveryNative", "(I)Z", (void*)startDiscoveryNative},
    {"stopDiscoveryNative", "()Z", (void *)stopDiscoveryNative},

    {"readAdapterOutOfBandDataNative", "()[B", (void *)readAdapterOutOfBandDataNative},
    {"createPairedDeviceNative", "(Ljava/lang/String;I)Z", (void *)createPairedDeviceNative},
    {"createPairedDeviceOutOfBandNative", "(Ljava/lang/String;I)Z",
                                    (void *)createPairedDeviceOutOfBandNative},
    {"cancelDeviceCreationNative", "(Ljava/lang/String;)Z", (void *)cancelDeviceCreationNative},
    {"removeDeviceNative", "(Ljava/lang/String;)Z", (void *)removeDeviceNative},
    {"getDeviceServiceChannelNative", "(Ljava/lang/String;Ljava/lang/String;I)I",
      (void *)getDeviceServiceChannelNative},

    {"setPairingConfirmationNative", "(Ljava/lang/String;ZI)Z",
            (void *)setPairingConfirmationNative},
    {"setPasskeyNative", "(Ljava/lang/String;II)Z", (void *)setPasskeyNative},
    {"setRemoteOutOfBandDataNative", "(Ljava/lang/String;[B[BI)Z", (void *)setRemoteOutOfBandDataNative},
    {"setAuthorizationNative", "(Ljava/lang/String;ZI)Z", (void *)setAuthorizationNative},
    {"setPinNative", "(Ljava/lang/String;Ljava/lang/String;I)Z", (void *)setPinNative},
    {"cancelPairingUserInputNative", "(Ljava/lang/String;I)Z",
            (void *)cancelPairingUserInputNative},
    {"setDevicePropertyBooleanNative", "(Ljava/lang/String;Ljava/lang/String;I)Z",
            (void *)setDevicePropertyBooleanNative},
    {"setDevicePropertyStringNative", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
            (void *)setDevicePropertyStringNative},
    {"createDeviceNative", "(Ljava/lang/String;)Z", (void *)createDeviceNative},
    {"discoverServicesNative", "(Ljava/lang/String;Ljava/lang/String;)Z", (void *)discoverServicesNative},
    {"addRfcommServiceRecordNative", "(Ljava/lang/String;JJS)I", (void *)addRfcommServiceRecordNative},
    {"removeServiceRecordNative", "(I)Z", (void *)removeServiceRecordNative},
    {"addReservedServiceRecordsNative", "([I)[I", (void *) addReservedServiceRecordsNative},
    {"removeReservedServiceRecordsNative", "([I)Z", (void *) removeReservedServiceRecordsNative},
    {"setLinkTimeoutNative", "(Ljava/lang/String;I)Z", (void *)setLinkTimeoutNative},
    {"setSSPDebugModeNative", "(Z)Z", (void*)setSSPDebugModeNative},
    // HID functions
    {"connectInputDeviceNative", "(Ljava/lang/String;)Z", (void *)connectInputDeviceNative},
    {"disconnectInputDeviceNative", "(Ljava/lang/String;)Z", (void *)disconnectInputDeviceNative},

    {"setBluetoothTetheringNative", "(ZLjava/lang/String;Ljava/lang/String;)Z",
              (void *)setBluetoothTetheringNative},
    {"connectPanDeviceNative", "(Ljava/lang/String;Ljava/lang/String;)Z",
              (void *)connectPanDeviceNative},
    {"disconnectPanDeviceNative", "(Ljava/lang/String;)Z", (void *)disconnectPanDeviceNative},
    {"disconnectPanServerDeviceNative", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
              (void *)disconnectPanServerDeviceNative},
    // Health function
    {"registerHealthApplicationNative",
              "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
              (void *)registerHealthApplicationNative},
    {"registerHealthApplicationNative",
            "(ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
            (void *)registerSinkHealthApplicationNative},

    {"unregisterHealthApplicationNative", "(Ljava/lang/String;)Z",
              (void *)unregisterHealthApplicationNative},
    {"createChannelNative", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)Z",
              (void *)createChannelNative},
    {"destroyChannelNative", "(Ljava/lang/String;Ljava/lang/String;I)Z",
              (void *)destroyChannelNative},
    {"getMainChannelNative", "(Ljava/lang/String;)Ljava/lang/String;", (void *)getMainChannelNative},
    {"getChannelApplicationNative", "(Ljava/lang/String;)Ljava/lang/String;",
              (void *)getChannelApplicationNative},
    {"getChannelFdNative", "(Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;", (void *)getChannelFdNative},
    {"releaseChannelFdNative", "(Ljava/lang/String;)Z", (void *)releaseChannelFdNative},
};


int register_android_server_BluetoothService(JNIEnv *env) {
    return AndroidRuntime::registerNativeMethods(env,
                "android/server/BluetoothService", sMethods, NELEM(sMethods));
}

} /* namespace android */
