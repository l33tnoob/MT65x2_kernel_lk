#ifndef _KAL_GENERAL_TYPES_H
#define _KAL_GENERAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************************************************
 * Type Definitions
 *******************************************************************************/

// Fixed width integer types
#ifdef _MSC_VER
    // Special type handle for enum2str codegen.
#ifdef __ENUM_GEN__
    typedef signed char         S8;
    typedef signed short        S16;
    typedef signed long         S32;
    typedef signed long long    S64;
    typedef unsigned char       U8;
    typedef unsigned short      U16;
    typedef unsigned long       U32;
    typedef unsigned long long  U64;
#else  // __ENUM_GEN__
    typedef __int8              S8;
    typedef __int16             S16;
    typedef __int32             S32;
    typedef __int64             S64;
    typedef unsigned __int8     U8;
    typedef unsigned __int16    U16;
    typedef unsigned __int32    U32;
    typedef unsigned __int64    U64;
#endif  // __ENUM_GEN__
#else  // _MSC_VER
#include <inttypes.h>
typedef int8_t              S8;
typedef int16_t             S16;
typedef int32_t             S32;
typedef int64_t             S64;
typedef uint8_t             U8;
typedef uint16_t            U16;
typedef uint32_t            U32;
typedef uint64_t            U64;
#endif

typedef char                kal_char;
typedef U16                 kal_wchar;

typedef U8                  kal_uint8;
typedef U16                 kal_uint16;
typedef U32                 kal_uint32;
typedef U64                 kal_uint64;
typedef S8                  kal_int8;
typedef S16                 kal_int16;
typedef S32                 kal_int32;
typedef S64                 kal_int64;

typedef enum {
    KAL_FALSE,
    KAL_TRUE
} kal_bool;

/*******************************************************************************
 * Constant definition
 *******************************************************************************/
#ifndef NULL
#define NULL               0
#endif

/*******************************************************************************
 * Module, SAP, Message
 *******************************************************************************/

typedef enum {
    MOD_NIL,
    MOD_AS,
    MOD_GPS,
    MOD_GPSTASK,    //add for CP,Baochu
    MOD_L4C,
    MOD_L4C_2,
    MOD_L4C_3,
    MOD_L4C_4,
    MOD_MMI,
    MOD_MMI_2_CCCI, //add for CP,Baochu
    MOD_MMI_MEDIA_APP,  // Used by kal_wap_trace() only. But our implementation does not care about mod_id
    MOD_PMTK,
    MOD_RRLP,
    MOD_SIM,
    MOD_SIM_2,
    MOD_SIM_3,
    MOD_SIM_4,
    MOD_SOC,
    MOD_SUPL,
    MOD_SUPL_CONN,
    MOD_TIMER,
    MOD_TLS,
    MOD_TST_READER,
    MOD_UAGPS_UP,
    MOD_ABM,
    MOD_ANY,
    MOD_CNT,
} module_enum;

typedef enum
{
    GPS_SUPL_SAP,
    MMI_MMI_SAP,
    SUPL_LCSP_SAP,
    SUPL_MMI_SAP,
    SUPL_INT_SAP,
    LCSP_APP_SAP,
    GPS_LCSP_SAP,
    MMI_L4C_SAP,
    MMI_L4C_SS_SAP,
    MMI_GPSTASK_SAP,
	ANY_SAP,
} sap_enum;

typedef enum
{
    // SIM
    MSG_ID_SIM_READY_IND    = 0,

    // L4C.
    MSG_ID_L4C_NBR_CELL_INFO_REG_REQ    = 1,
    MSG_ID_L4C_NBR_CELL_INFO_REG_CNF    = 2,
    MSG_ID_L4C_NBR_CELL_INFO_DEREG_REQ  = 3,
    MSG_ID_L4C_NBR_CELL_INFO_DEREG_CNF  = 4,
    MSG_ID_L4C_NBR_CELL_INFO_IND        = 5,

    // MMI.
    // TODO(Aknow): modify message name.
    //  CREATE_REQ => SESSION_CREATE_REQ
    //  ABORT_REQ => SESSION_ABORT_REQ
    //  CLOSE_IND => SESSION_CLOSE_IND
    MSG_ID_SUPL_MMI_CONFIG_REQ      = 6,
    MSG_ID_SUPL_MMI_CREATE_REQ      = 7,
    MSG_ID_SUPL_MMI_ABORT_REQ       = 8,
    MSG_ID_SUPL_MMI_NOTIFY_IND      = 9,
    MSG_ID_SUPL_MMI_NOTIFY_RSP      = 10,
//C.K. add-->SUPL2 con-034 To replace g_agps_verify_cond
    MSG_ID_SUPL_MMI_NOTIFY_TRIGGER  = 11,
//C.K. add<--
    MSG_ID_SUPL_MMI_START_POS_IND   = 12,
    MSG_ID_SUPL_MMI_START_POS_RSP   = 13,
    MSG_ID_SUPL_MMI_HISTORICAL_IND  = 14,
    MSG_ID_SUPL_MMI_HISTORICAL_RSP  = 15,
    MSG_ID_SUPL_MMI_LOCATION_IND    = 16,
    MSG_ID_SUPL_MMI_DISPLAY_LOCATION_IND    = 17,
    MSG_ID_SUPL_MMI_CLOSE_IND   = 18,

    // LCSP.
    MSG_ID_LCSP_START_REQ,
    MSG_ID_LCSP_END_REQ,
    MSG_ID_SUPL_LCSP_DATA_IND,
    MSG_ID_SUPL_LCSP_DATA_RSP,
    MSG_ID_SUPL_LCSP_DATA_REQ,
    MSG_ID_SUPL_LCSP_DATA_CNF,
    MSG_ID_SUPL_LCSP_ABORT_REQ,

    // Internal Messages
    MSG_ID_SUPL_CONN_CREATE_REQ,
    MSG_ID_SUPL_CONN_CREATE_CNF,
    MSG_ID_SUPL_CONN_SEND_REQ,
    MSG_ID_SUPL_CONN_SEND_CNF,
    MSG_ID_SUPL_CONN_RECV_IND,
    MSG_ID_SUPL_CONN_FAIL_IND,
    MSG_ID_SUPL_CONN_CLOSE_REQ,


    MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND,
    MSG_ID_APP_SOC_NOTIFY_IND,
    MSG_ID_APP_TLS_NOTIFY_IND,
    MSG_ID_APP_SOC_BEARER_INFO_IND,
    MSG_ID_APP_SOC_DEACTIVATE_CNF,
    MSG_ID_APP_TLS_INVALID_CERT_IND,
    MSG_ID_APP_TLS_CLIENT_AUTH_IND,
    MSG_ID_APP_TLS_ALERT_IND,
    MSG_ID_APP_CBM_BEARER_INFO_IND,

    MSG_ID_RR_RRLP_CELL_INFO_REPORT,

    //== add in porting ==//
    MSG_ID_MMI_MMI_START_TIMER,
    MSG_ID_MMI_GPS_MGR_OPEN_GPS,
    MSG_ID_MMI_GPS_MGR_CLOSE_GPS,
    MSG_ID_MMI_UART_SEND_ALL_ASSIST_IND, // inform MMI that SI/MB has sent all assist data
    //==

    MSG_ID_GPS_UART_OPEN_REQ,
    MSG_ID_GPS_UART_READ_REQ,
    MSG_ID_GPS_UART_WRITE_REQ,
    MSG_ID_GPS_UART_CLOSE_REQ,
    MSG_ID_GPS_UART_NMEA_LOCATION,
    MSG_ID_GPS_UART_NMEA_SENTENCE,
    MSG_ID_GPS_UART_RAW_DATA,
    MSG_ID_GPS_UART_DEBUG_RAW_DATA,
    MSG_ID_GPS_UART_P_INFO_IND,
    MSG_ID_GPS_UART_OPEN_SWITCH_REQ,
    MSG_ID_GPS_UART_CLOSE_SWITCH_REQ,

    MSG_ID_GPS_POS_GAD_CNF,
    MSG_ID_GPS_LCSP_MSG_CODE_BEGIN = MSG_ID_GPS_POS_GAD_CNF,
    MSG_ID_GPS_LCSP_MEAS_GAD_CNF,
    MSG_ID_GPS_LCSP_ASSIST_DATA_CNF,
    MSG_ID_GPS_LCSP_MSG_CODE_END = MSG_ID_GPS_LCSP_ASSIST_DATA_CNF,
    MSG_ID_GPS_POS_GAD_REQ,
    MSG_ID_GPS_LCSP_MEAS_GAD_REQ,
    MSG_ID_GPS_LCSP_ASSIST_DATA_REQ,
    MSG_ID_GPS_LCSP_ABORT_REQ,

    MSG_ID_GPS_ASSIST_BIT_MASK_IND,
    MSG_ID_GPS_LCT_POS_REQ,
    MSG_ID_GPS_LCT_POS_RSP,
    MSG_ID_GPS_LCT_OP_ERROR,

    //!! bugfix for pmtk thread to start gps timer
    MSG_ID_GPS_UART_SEND_ALL_ASSIST_IND,
    MSG_ID_GPS_UART_UPDATE_HIS_POS_FOR_SIMB, //for TAS IOT Phase3

    /* RTC -> GPS */
    MSG_ID_RTC_GPS_TIME_CHANGE_IND,

    /* GPS EINT HISR -> GPS */
    MSG_ID_GPS_HOST_WAKE_UP_IND,

    /*UART*/
    MSG_ID_UART_READY_TO_READ_IND,
    MSG_ID_UART_PLUGOUT_IND,



    MSG_ID_TIMER_TIMEOUT,

    MSG_ID_END
} msg_enum;

typedef enum
{
   TRACE_FUNC,
   TRACE_STATE,
   TRACE_INFO,
   TRACE_WARNING,
   TRACE_ERROR,
   TRACE_GROUP_1,
   TRACE_GROUP_2,
   TRACE_GROUP_3,
   TRACE_GROUP_4,
   TRACE_GROUP_5,
   TRACE_GROUP_6,
   TRACE_GROUP_7,
   TRACE_GROUP_8,
   TRACE_GROUP_9,
   TRACE_GROUP_10,
   TRACE_PEER
} trace_group_enum;

typedef unsigned int    task_indx_type;
typedef module_enum     module_type;
typedef sap_enum        sap_type;
typedef msg_enum        msg_type;

#ifdef __cplusplus
}
#endif

#endif

