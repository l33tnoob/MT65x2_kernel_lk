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
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_BLUETOOTH_COMMON_H
#define ANDROID_BLUETOOTH_COMMON_H

// Set to 0 to enable verbose bluetooth logging
#define LOG_NDEBUG 1

#include "jni.h"
#include "utils/Log.h"

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/poll.h>

#ifdef __BTMTK__
#include "bt_mmi.h"
#include "bt_message.h"
#include "bluetooth_struct.h"
#include "bluetooth_gap_struct.h"
#endif

#ifdef HAVE_BLUETOOTH
#include <dbus/dbus.h>
#include <bluetooth/bluetooth.h>
#endif

#ifdef __BTMTK__
#define ANDROID_EV_DUN_PPPD_START		MSG_ID_BT_CUSTOM_MSG_ID_BEGIN + 100
#define ANDROID_EV_DUN_PPPD_STOP			ANDROID_EV_DUN_PPPD_START + 1
/*redefine BTBM_ADP_MAX_NAME_LEN */
/*Notes that the value defined here has to be less than 247*/
#define BTBM_ADP_MAX_NAME_LEN 			(59)     
#endif

namespace android {

struct event_loop_native_data_t;

struct _Properties {
    char name[32];
    int type;
};

typedef struct _Properties Properties;

jfieldID get_field(JNIEnv *env,
                   jclass clazz,
                   const char *member,
                   const char *mtype);

// size of the dbus event loops pollfd structure, hopefully never to be grown
#define DEFAULT_INITIAL_POLLFD_COUNT 8

#ifdef HAVE_BLUETOOTH
#define BLUEZ_DBUS_BASE_PATH      "/org/bluez"
#define BLUEZ_DBUS_BASE_IFC       "org.bluez"
#define BLUEZ_ERROR_IFC           "org.bluez.Error"

// It would be nicer to retrieve this from bluez using GetDefaultAdapter,
// but this is only possible when the adapter is up (and hcid is running).
// It is much easier just to hardcode bluetooth adapter to hci0
#define BLUETOOTH_ADAPTER_HCI_NUM 0
#define BLUEZ_ADAPTER_OBJECT_NAME BLUEZ_DBUS_BASE_PATH "/hci0"

#define BTADDR_SIZE 18   // size of BT address character array (including null)

// size of the dbus event loops pollfd structure, hopefully never to be grown
#define DEFAULT_INITIAL_POLLFD_COUNT 8

jfieldID get_field(JNIEnv *env,
                   jclass clazz,
                   const char *member,
                   const char *mtype);

// ALOGE and free a D-Bus error
// Using #define so that __FUNCTION__ resolves usefully
#define LOG_AND_FREE_DBUS_ERROR_WITH_MSG(err, msg) \
    {   ALOGE("%s: D-Bus error in %s: %s (%s)", __FUNCTION__, \
        dbus_message_get_member((msg)), (err)->name, (err)->message); \
         dbus_error_free((err)); }
#define LOG_AND_FREE_DBUS_ERROR(err) \
    {   ALOGE("%s: D-Bus error: %s (%s)", __FUNCTION__, \
        (err)->name, (err)->message); \
        dbus_error_free((err)); }

struct event_loop_native_data_t {
    DBusConnection *conn;
    const char *adapter;

    /* protects the thread */
    pthread_mutex_t thread_mutex;
    pthread_t thread;
    /* our comms socket */
    /* mem for the list of sockets to listen to */
    struct pollfd *pollData;
    int pollMemberCount;
    int pollDataSize;
    /* mem for matching set of dbus watch ptrs */
    DBusWatch **watchData;
    /* pair of sockets for event loop control, Reader and Writer */
    int controlFdR;
    int controlFdW;
    /* our vm and env Version for future env generation */
    JavaVM *vm;
    int envVer;
    /* reference to our java self */
    jobject me;
    /* flag to indicate if the event loop thread is running */
    bool running;
};

struct _Properties {
    char name[32];
    int type;
};
typedef struct _Properties Properties;

dbus_bool_t dbus_func_args_async(JNIEnv *env,
                                 DBusConnection *conn,
                                 int timeout_ms,
                                 void (*reply)(DBusMessage *, void *, void *),
                                 void *user,
                                 void *nat,
                                 const char *path,
                                 const char *ifc,
                                 const char *func,
                                 int first_arg_type,
                                 ...);

DBusMessage * dbus_func_args(JNIEnv *env,
                             DBusConnection *conn,
                             const char *path,
                             const char *ifc,
                             const char *func,
                             int first_arg_type,
                             ...);

DBusMessage * dbus_func_args_error(JNIEnv *env,
                                   DBusConnection *conn,
                                   DBusError *err,
                                   const char *path,
                                   const char *ifc,
                                   const char *func,
                                   int first_arg_type,
                                   ...);

DBusMessage * dbus_func_args_timeout(JNIEnv *env,
                                     DBusConnection *conn,
                                     int timeout_ms,
                                     const char *path,
                                     const char *ifc,
                                     const char *func,
                                     int first_arg_type,
                                     ...);

DBusMessage * dbus_func_args_timeout_valist(JNIEnv *env,
                                            DBusConnection *conn,
                                            int timeout_ms,
                                            DBusError *err,
                                            const char *path,
                                            const char *ifc,
                                            const char *func,
                                            int first_arg_type,
                                            va_list args);

jint dbus_returns_int32(JNIEnv *env, DBusMessage *reply);
jint dbus_returns_uint32(JNIEnv *env, DBusMessage *reply);
jint dbus_returns_unixfd(JNIEnv *env, DBusMessage *reply);
jstring dbus_returns_string(JNIEnv *env, DBusMessage *reply);
jboolean dbus_returns_boolean(JNIEnv *env, DBusMessage *reply);
jobjectArray dbus_returns_array_of_strings(JNIEnv *env, DBusMessage *reply);
jobjectArray dbus_returns_array_of_object_path(JNIEnv *env, DBusMessage *reply);
jbyteArray dbus_returns_array_of_bytes(JNIEnv *env, DBusMessage *reply);

jobjectArray parse_properties(JNIEnv *env, DBusMessageIter *iter, Properties *properties,
                              const int max_num_properties);
jobjectArray parse_property_change(JNIEnv *env, DBusMessage *msg,
                                   Properties *properties, int max_num_properties);
jobjectArray parse_adapter_properties(JNIEnv *env, DBusMessageIter *iter);
jobjectArray parse_remote_device_properties(JNIEnv *env, DBusMessageIter *iter);
jobjectArray parse_remote_device_property_change(JNIEnv *env, DBusMessage *msg);
jobjectArray parse_adapter_property_change(JNIEnv *env, DBusMessage *msg);
jobjectArray parse_input_properties(JNIEnv *env, DBusMessageIter *iter);
jobjectArray parse_health_device_properties(JNIEnv *env, DBusMessageIter *iter);
jobjectArray parse_health_channel_properties(JNIEnv *env, DBusMessageIter *iter);
jobjectArray parse_input_property_change(JNIEnv *env, DBusMessage *msg);
jobjectArray parse_pan_property_change(JNIEnv *env, DBusMessage *msg);
jobjectArray parse_health_device_property_change(JNIEnv *env, DBusMessage *msg);

void append_dict_args(DBusMessage *reply, const char *first_key, ...);
void append_variant(DBusMessageIter *iter, int type, void *val);
int get_bdaddr(const char *str, bdaddr_t *ba);
void get_bdaddr_as_string(const bdaddr_t *ba, char *str);

bool debug_no_encrypt();


// Result codes from Bluez DBus calls
#define BOND_RESULT_ERROR                      -1
#define BOND_RESULT_SUCCESS                     0
#define BOND_RESULT_AUTH_FAILED                 1
#define BOND_RESULT_AUTH_REJECTED               2
#define BOND_RESULT_AUTH_CANCELED               3
#define BOND_RESULT_REMOTE_DEVICE_DOWN          4
#define BOND_RESULT_DISCOVERY_IN_PROGRESS       5
#define BOND_RESULT_AUTH_TIMEOUT                6
#define BOND_RESULT_REPEATED_ATTEMPTS           7

#define PAN_DISCONNECT_FAILED_NOT_CONNECTED  1000
#define PAN_CONNECT_FAILED_ALREADY_CONNECTED 1001
#define PAN_CONNECT_FAILED_ATTEMPT_FAILED    1002
#define PAN_OPERATION_GENERIC_FAILURE        1003
#define PAN_OPERATION_SUCCESS                1004

#define INPUT_DISCONNECT_FAILED_NOT_CONNECTED  5000
#define INPUT_CONNECT_FAILED_ALREADY_CONNECTED 5001
#define INPUT_CONNECT_FAILED_ATTEMPT_FAILED    5002
#define INPUT_OPERATION_GENERIC_FAILURE        5003
#define INPUT_OPERATION_SUCCESS                5004



#endif

#define HEALTH_OPERATION_SUCCESS               6000
#define HEALTH_OPERATION_ERROR                 6001
#define HEALTH_OPERATION_INVALID_ARGS          6002
#define HEALTH_OPERATION_GENERIC_FAILURE       6003
#define HEALTH_OPERATION_NOT_FOUND             6004
#define HEALTH_OPERATION_NOT_ALLOWED           6005

#ifdef __BTMTK__
#define BTMTK_SDP_UUID_SERIAL_PORT              0X1101
#define BTMTK_SDP_UUID_DIALUP_NETWORKING        0X1103
#define BTMTK_SDP_UUID_IRMC_SYNC                0X1104
#define BTMTK_SDP_UUID_OBEX_OBJECT_PUSH         0X1105
#define BTMTK_SDP_UUID_OBEX_FILE_TRANSFER       0X1106
#define BTMTK_SDP_UUID_HEADSET                  0X1108
#define BTMTK_SDP_UUID_AUDIO_SOURCE             0X110A
#define BTMTK_SDP_UUID_AUDIO_SINK               0X110B
#define BTMTK_SDP_UUID_AV_REMOTE_CONTROL_TARGET 0X110C
#define BTMTK_SDP_UUID_ADV_AUDIO_DISTRIBUTION   0X110D
#define BTMTK_SDP_UUID_AV_REMOTE_CONTROL        0X110E
#define BTMTK_SDP_UUID_FAX                      0X1111
#define BTMTK_SDP_UUID_HEADSET_AUDIO_GATEWAY    0X1112
#define BTMTK_SDP_UUID_PANU   									0X1115
#define BTMTK_SDP_UUID_PAN_NAP  								0X1116
#define BTMTK_SDP_UUID_PAN_GN   								0X1117
#define BTMTK_SDP_UUID_DIRECT_PRINTING          0X1118
#define BTMTK_SDP_UUID_IMAGING_RESPONDER        0X111B
#define BTMTK_SDP_UUID_HF_HANDSFREE             0X111E
#define BTMTK_SDP_UUID_AG_HANDSFREE             0X111F
#define BTMTK_SDP_UUID_BASIC_PRINTING           0X1122
#define BTMTK_SDP_UUID_HUMAN_INTERFACE          0X1124
#define BTMTK_SDP_UUID_SAP                      0X112D
#define BTMTK_SDP_UUID_PBAP_PCE                 0X112E
#define BTMTK_SDP_UUID_PBAP_PSE                 0X112F
#define BTMTK_SDP_UUID_GENERIC_AUDIO            0X1203
#define BTMTK_SDP_UUID_MAP                      0x1134

#define BTMTK_SDP_UUID_PRX						0x1803  /* Link loss (Mandatory) */

#define BTMTK_SDP_UUID_INSECURE					0x2001
#define BTMTK_SDP_UUID_SECURE					0x2002


#define BTMTK_SDP_ATTRIB_PROTOCOL_DESC_LIST     0x0004

// Result codes from Bluez DBus calls
#define BOND_RESULT_ERROR                      -1
#define BOND_RESULT_SUCCESS                     0
#define BOND_RESULT_AUTH_FAILED                 1
#define BOND_RESULT_AUTH_REJECTED               2
#define BOND_RESULT_AUTH_CANCELED               3
#define BOND_RESULT_REMOTE_DEVICE_DOWN          4
#define BOND_RESULT_DISCOVERY_IN_PROGRESS       5
#define BOND_RESULT_AUTH_TIMEOUT                6
#define BOND_RESULT_REPEATED_ATTEMPTS           7
#define BOND_RESULT_REMOVED				        9

#define PAN_DISCONNECT_FAILED_NOT_CONNECTED  1000
#define PAN_CONNECT_FAILED_ALREADY_CONNECTED 1001
#define PAN_CONNECT_FAILED_ATTEMPT_FAILED    1002
#define PAN_OPERATION_GENERIC_FAILURE        1003
#define PAN_OPERATION_SUCCESS                1004

#define INPUT_DISCONNECT_FAILED_NOT_CONNECTED  5000
#define INPUT_CONNECT_FAILED_ALREADY_CONNECTED 5001
#define INPUT_CONNECT_FAILED_ATTEMPT_FAILED    5002
#define INPUT_OPERATION_GENERIC_FAILURE        5003
#define INPUT_OPERATION_SUCCESS                5004

#define HEALTH_OPERATION_SUCCESS               6000
#define HEALTH_OPERATION_ERROR                 6001
#define HEALTH_OPERATION_INVALID_ARGS          6002
#define HEALTH_OPERATION_GENERIC_FAILURE       6003
#define HEALTH_OPERATION_NOT_FOUND             6004
#define HEALTH_OPERATION_NOT_ALLOWED           6005

typedef enum 
{
    BTMTK_PAIR_TYPE_PINCODE,
    BTMTK_PAIR_TYPE_JUST_WORK,
    BTMTK_PAIR_TYPE_USER_CONFIRM,
    BTMTK_PAIR_TYPE_PASSKEY_DISPLAY,
    BTMTK_PAIR_TYPE_PASSKEY_INPUT
} btmtk_pair_type_enum;

typedef enum
{
    ANDROID_EV_GAP_START = MSG_ID_BT_CUSTOM_MSG_ID_BEGIN,
    ANDROID_EV_GAP_DISCOVERY_START,
    ANDROID_EV_GAP_DISCOVERY_STOP,
    ANDROID_EV_GAP_LOCAL_NAME_CHANGE,
    ANDROID_EV_GAP_SCAN_MODE_CHANGE,
    ANDROID_EV_GAP_PAIRED_DEVICE_RENAME,
    ANDROID_EV_SDP_DEVICE_CREATE,
    ANDROID_EV_SDP_PAIRED_DEVICE_CREATE,
    ANDROID_EV_SDP_PAIRED_DEVICE_REMOVED,
    ANDROID_EV_SDP_DISCOVER,
    ANDROID_EV_DEVICE_TRUSTED,
    ANDROID_EV_GAP_SCAN_MODE_TIMEOUT_CHANGE,
    ANDROID_EV_GAP_POWER_STATE_CHANGE,
} btmtk_android_gap_event_enum;

typedef enum
{
    BTMTK_POWER_STATE_OFF,
    BTMTK_POWER_STATE_ON,
    BTMTK_POWER_STATE_RESET,
    BTMTK_POWER_STATE_TURNING_ON,
    BTMTK_POWER_STATE_TURNING_OFF
} btmtk_power_state_enum;

typedef enum
{
    BTMTK_DEVICE_PROP_ADDR,
    BTMTK_DEVICE_PROP_NAME,
//    BTMTK_DEVICE_PROP_NICKNAME,
//    BTMTK_DEVICE_PROP_ICON,
    BTMTK_DEVICE_PROP_CLASS,
    BTMTK_DEVICE_PROP_UUIDS,
    BTMTK_DEVICE_PROP_PAIRED,
    BTMTK_DEVICE_PROP_CONNECTED,
    BTMTK_DEVICE_PROP_TRUSTED,
    BTMTK_DEVICE_PROP_ALIAS,
//    BTMTK_DEVICE_PROP_NODES,
//    BTMTK_DEVICE_PROP_ADAPTER,
    BTMTK_DEVICE_PROP_LEGACY_PAIRING,
    BTMTK_DEVICE_PROP_RSSI,
//    BTMTK_DEVICE_PROP_TX,
    BTMTK_DEVICE_PROP_TOTAL,
} btmtk_device_property_enum;

typedef enum
{
    BTMTK_ADAPTER_PROP_ADDR,
    BTMTK_ADAPTER_PROP_NAME,
    BTMTK_ADAPTER_PROP_CLASS,
    BTMTK_ADAPTER_PROP_POWERED,
    BTMTK_ADAPTER_PROP_DISCOVERABLE,
    BTMTK_ADAPTER_PROP_DISCOVERABLE_TIMEOUT,
    BTMTK_ADAPTER_PROP_PAIRABLE,
//    BTMTK_ADAPTER_PROP_PAIRABLE_TIMEOUT,
    BTMTK_ADAPTER_PROP_DISCOVERING,
    BTMTK_ADAPTER_PROP_DEVICES,
    BTMTK_ADAPTER_PROP_UUIDS,
    BTMTK_ADAPTER_PROP_TOTAL,
} btmtk_adapter_property_enum;

#define BTMTK_CACHE_FOLDER  "data/@btmtk"
#define BTMTK_DEV_CACHE_PATH  "data/@btmtk/dev_cache"  /* paired device */
#define BTMTK_HOST_CACHE_PATH  "data/@btmtk/host_cache"  /* host setting, ex. name, mode */

typedef enum
{
    BTMTK_GAP_ACT_NONE = 0x0,
    BTMTK_GAP_ACT_INQUIRY = 0x1,
} btmtk_gap_activity_enum;

enum
{
	BTMTK_BOND_STATE_UNBOND,
	BTMTK_BOND_STATE_BONDED,	
	BTMTK_BOND_STATE_BONDING,
};

typedef unsigned char btmtk_bond_state;

typedef enum
{
    BTMTK_SDP_CREATE_DEVICE,
    BTMTK_SDP_CREATE_PAIRED_DEVICE,
    BTMTK_SDP_DISCOVERY,
    BTMTK_SDP_DISCOVERY_CHANNEL
} btmtk_sdp_activity_enum;

typedef ilm_struct ev_struct;

typedef struct _btmtk_list_header_struct
{
    struct _btmtk_list_header_struct *next;
} btmtk_list_header_struct;

typedef struct
{
    bt_addr_struct addr;
    char name[BTBM_ADP_MAX_NAME_LEN];
	char nickname[BTBM_ADP_MAX_NAME_LEN];
    /* icon */
    unsigned int cod;
    struct
    {
        unsigned char uuid_no;
        unsigned short uuid[BTBM_ADP_MAX_SDAP_UUID_NO];
        unsigned short channel[BTBM_ADP_MAX_SDAP_UUID_NO];  /* map to uuid */

        unsigned char app_uuid_no;
        unsigned char app_uuid[BTBM_ADP_MAX_SDAP_APP_UUID_NO][16];
        unsigned short app_channel[BTBM_ADP_MAX_SDAP_APP_UUID_NO];  /* map to uuid */
    } sdp;
    btmtk_bond_state paired;
    bool connected;
    bool trusted;
    /* alias */
    /* nodes */
    /* adapter */
    bool legacy_pairing;
    short rssi;
    btbm_device_type device_type;
    /* TX */
//	bool pairing;
} btmtk_device_entry_struct;

typedef struct _btmtk_sdp_req_struct
{
    struct _btmtk_sdp_req_struct *next;

    btmtk_sdp_activity_enum type;
    bt_addr_struct addr;
    unsigned char uuid_no;
    unsigned short uuid[BTBM_ADP_MAX_SDAP_UUID_NO];
    unsigned char app_uuid_no;
    unsigned char app_uuid[16];
} btmtk_sdp_req_struct;

typedef struct _btmtk_hdp_fd_struct
{
    struct _btmtk_hdp_fd_struct 	*next;
    bt_addr_struct 					addr;
    unsigned short 					mdl_id;
    int 							fd;
} btmtk_hdp_fd_struct;


typedef struct
{
    unsigned char scan_mode;           /* default: BTBM_ADP_P_ON_I_ON */
    char name[BTBM_ADP_MAX_NAME_LEN];  /* default: BTMTK_ANDROID_DEFAULT_LOCAL_NAME */
    unsigned int scan_mode_timeout;
} btmtk_host_cache_struct;

typedef enum
{
    BTMTK_A2DP_PROP_STATE,
    BTMTK_A2DP_PROP_CONNECTED,
    BTMTK_A2DP_PROP_PLAYING,
    BTMTK_A2DP_PROP_TOTAL,
} btmtk_a2dp_property_enum;

typedef struct
{
    LOCAL_PARA_HDR 

    int name_len;
    char name[BTBM_ADP_MAX_NAME_LEN];
} btmtk_android_gap_name_event_struct;

typedef struct
{
    LOCAL_PARA_HDR 

    bt_addr_struct addr;
    int name_len;
    char name[BTBM_ADP_MAX_NAME_LEN];
} btmtk_android_gap_remote_name_event_struct;

typedef struct
{
    LOCAL_PARA_HDR 

    bt_addr_struct addr;
    bool trusted;
} btmtk_android_gap_device_trusted_struct;

typedef struct
{
    LOCAL_PARA_HDR 

    bt_addr_struct addr;
    unsigned short uuid;
    unsigned short attrid;
} btmtk_android_sdp_discover_channel_struct;

typedef struct
{
    LOCAL_PARA_HDR 

    bt_addr_struct addr;
    int pattern_len;
    char pattern[16];
} btmtk_android_sdp_service_search_event_struct;


#define MIN(a,b) ((a < b) ? a : b)

#define BTMTK_ANDROID_DEFAULT_LOCAL_NAME "ANDROID BT"
#define BTMTK_ANDROID_DEFAULT_REMOTE_NAME "UNKNOWN DEVICE"
#define BTMTK_ANDROID_ADAPTER_PREFIX "MTKBT"
#define BTMTK_MAX_DEVICE_NAME_SIZE 80
#define BTMTK_MAX_PAIRED_LIST_NO   20
#define BTMTK_MAX_OBJECT_PATH_SIZE 32 /* "MTKBT/dev_XX_XX_XX_XX_XX_XX" + '\0' */
#define BTMTK_MAX_ADDR_STR_SIZE 20 /* "XX:XX:XX:XX:XX:XX" + '\0' */
#define BTMTK_MAX_UUID_STR_SIZE 37 /* "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX" + '\0' */
#define BTMTK_SDP_UUID_16_BIT_SIZE 2
#define BTMTK_SDP_UUID_32_BIT_SIZE 4
#define BTMTK_SDP_UUID_128_BIT_SIZE 16
#define BTMTK_MAX_STACK_TIMEOUT 15000
#define BTMTK_MAX_REMOTE_TIMEOUT 90000
#define BTMTK_MAX_HDP_CHANNEL_PATH_SIZE 40 /* "XX/MTKDEV_XX_XX_XX_XX_XX_XX" + '\0' */


/** Type code marking a 32-bit unsigned integer */
#define DBUS_TYPE_UINT32        ((int) 'u')
/** Type code marking a UTF-8 encoded, nul-terminated Unicode string */
#define DBUS_TYPE_STRING        ((int) 's')
/** Type code marking a D-Bus array type */
#define DBUS_TYPE_ARRAY         ((int) 'a')
/** Type code marking a boolean */
#define DBUS_TYPE_BOOLEAN       ((int) 'b')
/** Type code marking a 16-bit signed integer */
#define DBUS_TYPE_INT16         ((int) 'n')
/** Type code marking a D-Bus object path */
#define DBUS_TYPE_OBJECT_PATH   ((int) 'o')


typedef union {
    char *str_val;
    int int_val;
    char **array_val;
} property_value;

bool bt_sendmsg(int sock, void* ptr, int size);
int wait_response(int sock, unsigned long msg_id, ilm_struct* ilm, int timeout /* millisecond */);
int btmtk_util_utf8_endpos(char *str, int len);
bool btmtk_util_is_assigned_uuid(char *uuid128);
void btmtk_util_convert_bdaddr2string(char *dest, bt_addr_struct *source);
void btmtk_util_convert_string2bdaddr(const char *source, bt_addr_struct *dest);
void btmtk_util_convert_bdaddr2objpath(char *dest, bt_addr_struct *source);
void btmtk_util_convert_objpath2bdaddr(const char *source, bt_addr_struct *dest);
void btmtk_util_convert_uuid16_2_uuid128(char *uuid128, unsigned short uuid16);
unsigned short btmtk_util_convert_uuid128_2_uuid16(char *uuid128);
void btmtk_util_convert_uuid128_2_string(char *buf, char *uuid128);
void btmtk_util_convert_string_2_uuid128(char *uuid128, const char *buf);
void btmtk_util_convert_juuid_2_uuid128(char *buf, long long msb, long long lsb);
int btmtk_util_convert_uuidlist_2_uuid16( unsigned long service_list1,  /* 0x1100 ~ 0x111F */
    												unsigned long  service_list2,  /* 0x1120 ~ 0x113F */
    												unsigned long  service_list3,  /* 0x1200 ~ 0x121F */
    												unsigned long  service_list4,  /* 0x1300~ */
    												unsigned long  service_list5,
    												unsigned long  service_list6,
    												unsigned short uuid[]);

bool btmtk_util_equal_bdaddr(bt_addr_struct *addr1, bt_addr_struct *addr2);
void btmtk_util_list_append(btmtk_list_header_struct **list, btmtk_list_header_struct *node);
void btmtk_util_list_remove(btmtk_list_header_struct **list, btmtk_list_header_struct *node);
btmtk_sdp_req_struct *btmtk_util_find_sdp_channel_request(event_loop_native_data_t *nat, bt_addr_struct *addr, unsigned short uuid, unsigned short attrid);

btmtk_device_entry_struct *btmtk_paired_dev_cache_find(event_loop_native_data_t *nat, bt_addr_struct *addr);
btmtk_device_entry_struct *btmtk_inquired_dev_cache_find(event_loop_native_data_t *nat, bt_addr_struct *addr);
int btmtk_hdp_util_convert_result(U8 result);


void create_prop_array(JNIEnv *env, jobjectArray strArray, Properties *property,
                       property_value *value, int len, int *array_index );

#endif

typedef struct {
    JNIEnv *env;
#ifdef HAVE_BLUETOOTH
    DBusConnection *conn;
    const char *adapter;  // dbus object name of the local adapter
#endif
#ifdef __BTMTK__
    struct event_loop_native_data_t *event_nat;


	btmtk_hdp_fd_struct *hdp_fds;  /* HDP fds cache*/

    int servsock;
	int twinservsock;
    btmtk_power_state_enum state;
    pthread_mutex_t thread_mutex;
#endif
} bt_native_data_t;

struct event_loop_native_data_t {
#ifdef HAVE_BLUETOOTH
    DBusConnection *conn;
#endif
    const char *adapter;

    /* protects the thread */
    pthread_mutex_t thread_mutex;
    pthread_t thread;
    /* our comms socket */
    /* mem for the list of sockets to listen to */
    struct pollfd *pollData;
    int pollMemberCount;
    int pollDataSize;
#ifdef HAVE_BLUETOOTH
    /* mem for matching set of dbus watch ptrs */
    DBusWatch **watchData;
#endif
    /* pair of sockets for event loop control, Reader and Writer */
    int controlFdR;
    int controlFdW;
    /* our vm and env Version for future env generation */
    JavaVM *vm;
    int envVer;
    /* reference to our java self */
    jobject me;

    bool running;

#ifdef __BTMTK__
    int gapSock;
    int conn_no;
    
    bt_native_data_t *service_nat;
    btmtk_host_cache_struct host_cache;  /* host setting information */
    btmtk_gap_activity_enum activity;
    btmtk_sdp_req_struct *requests; /* SDP request in progressing */

    int paired_cache_no;
    int inquired_cache_no;
    btmtk_device_entry_struct paired_dev_cache[BTBM_ADP_MAX_PAIRED_LIST_NO];
    btmtk_device_entry_struct inquired_dev_cache[BTBM_ADP_MAX_INQUIRY_NO];
#endif
};

} /* namespace android */

#endif/*ANDROID_BLUETOOTH_COMMON_H*/
