#ifndef _LTE_INFO_PARSER_H_
#define _LTE_INFO_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

/* !!! All definition in this file are copyed from Modem header files */

typedef unsigned int kal_uint32;
typedef unsigned short kal_uint16;
typedef unsigned char kal_uint8;
typedef unsigned char kal_bool;
typedef int kal_int32;
typedef short kal_int16;
typedef char kal_int8;
typedef char kal_char;

#define LOCAL_PARA_HDR \
   kal_uint8    ref_count; \
   kal_uint16    msg_len;

typedef struct _plmn_id_struct
{
    kal_uint8 mcc1;
    kal_uint8 mcc2;
    kal_uint8 mcc3;
    kal_uint8 mnc1;
    kal_uint8 mnc2;
    kal_uint8 mnc3;
} plmn_id_struct;

typedef struct {
    kal_uint8 ip_addr_type;
    kal_uint8 ipv4[4];
    kal_uint8 ipv6[16];
} ip_addr_struct;

// EM types
// Copyed from em_public_struct.h
typedef enum __attribute__ ((__packed__))
{
   /* RR */ 
   /* Begin of RR EM INFO Request enum */
   RR_EM_CELL_SELECT_PARA_INFO = 0, RR_EM_INFO_BEGIN = RR_EM_CELL_SELECT_PARA_INFO,
   RR_EM_CHANNEL_DESCR_INFO,
   RR_EM_CTRL_CHANNEL_DESCR_INFO,
   RR_EM_RACH_CTRL_PARA_INFO,
   RR_EM_LAI_INFO,
   RR_EM_RADIO_LINK_COUNTER_INFO,
   RR_EM_MEASUREMENT_REPORT_INFO,
   /* ZY : Temp solution : Cell allocation list*/
   RR_EM_CA_LIST_INFO,
   /* RR new structure */
   RR_EM_CONTROL_MSG_INFO,
   RR_EM_SI2Q_INFO_STRUCT_INFO, 
   RR_EM_MI_INFO_STRUCT_INFO,
   RR_EM_BLK_INFO,
   RR_EM_TBF_INFO,
   RR_EM_GPRS_GENERAL_INFO,
   /* GAS MM EM INFO */
   RRM_EM_IR_PARAMETER_STATUS_IND_STRUCT_INFO,
   RRM_EM_IR_RESELECT_STATUS_IND_STRUCT_INFO,
   RRM_EM_IR_3G_NEIGHBOR_MEAS_STATUS_IND_STRUCT_INFO,
   RRM_EM_IR_3G_NEIGHBOR_MEAS_INFO_IND_STRUCT_INFO,
   RRM_EM_IR_4G_NEIGHBOR_MEAS_STATUS_IND_STRUCT_INFO,
   RRM_EM_IR_4G_NEIGHBOR_MEAS_INFO_IND_STRUCT_INFO,
   RR_EM_INFO_END = RRM_EM_IR_4G_NEIGHBOR_MEAS_INFO_IND_STRUCT_INFO,

   /*End of RR EM INFO Request enum*/
   
   /* CC */ 
   CC_EM_CHANNEL_INFO = 50,
   CC_EM_CALL_INFO,
   
   /* SS */
   SS_EM_INFO,
   
   /* MM */
   MM_EM_INFO,
   /*EM ehancement for RR new structure*/
   MMRR_EM_PLMN_INFO_STRUCT_INFO,
                                        
   /* UEM */                            
   UEM_EM_BATTERY_INFO,
   
   /* gprs em begins */
   GMM_EM_INFO,
//   TCM_EM_EXT_PDP_INFO,
//   TCM_EM_INT_PDP_INFO,
//   TCM_EM_CONTEXT_INFO, //new
//   SNDCP_EM_INFO,
   LLC_EM_INFO,
   /* PPP , removed because of no use*/
   //PPP_EM_INFO,
   SM_EM_INFO,

//#ifdef __VIDEO_CALL_SUPPORT__       
    /* VT EM Display, 2007/11/30 */
    /* VT owner comments VT EM enum is not needed in WR8 */
    //VT_EM_CALL_STATE_INFO = 50, VT_EM_BEGIN = VT_EM_CALL_STATE_INFO,/* vt_em_call_state_choice */
    //VT_EM_MASTER_SLAVE_STATUS_INFO,     /* vt_em_master_slave_status_choice */
    //VT_EM_RETRANSMISSION_PROTOCOL_INFO, /* vt_em_retransmission_protocol_choice */
    //VT_EM_INCOMING_AUDIO_CHANNEL_INFO,  /* vt_em_audio_channel_info_struct */
    //VT_EM_OUTGOING_AUDIO_CHANNEL_INFO,  /* vt_em_audio_channel_info_struct */
    //VT_EM_INCOMING_VIDEO_CHANNEL_INFO,  /* vt_em_video_channel_info_struct */
    //VT_EM_OUTGOING_VIDEO_CHANNEL_INFO,  /* vt_em_video_channel_info_struct */
    //VT_EM_ADM_MEM_MAX_USED_INFO,        /* kal_uint32 */
    //VT_EM_STATISTIC_INFO,               /* vt_em_statistic_info_struct */
    //VT_EM_ROUND_TRIP_DELAY_INFO,        /* kal_uint32 */
    //VT_EM_INCOMING_XSRP_INFO,           /* vt_em_incoming_xSRP */
    //VT_EM_OUTGOING_XSRP_INFO,           /* vt_em_outgoing_xSRP */
    //VT_EM_END = VT_EM_OUTGOING_XSRP_INFO,
//#endif

//#ifdef __UMTS_RAT__          //For MAUI, MONZA2G exclude 3G
   /**
    * Gibran 20061228
    * UAS MEME/CSCE measuremnt and cell status structure
    */
   EM_URR_3G_GENERAL_STATUS_IND = 70, URR_EM_INFO_BEGIN = EM_URR_3G_GENERAL_STATUS_IND,
   /* Put 1st XXX_STATUS_IND_STRUCT_INFO in front of XXX_EM_INFO_BEGIN 
      in order to show enum_name in XXX_STATUS_IND_STRUCT_INFO not in XXX_EM_INFO_BEGIN. */

   EM_SIBE_3G_SIB_IND_STRUCT_INFO,
   EM_CSCE_SERV_CELL_IND_STRUCT_INFO = 75,
   EM_CSCE_NEIGH_CELL_IND_STRUCT_INFO,
   EM_CSCE_R_STATUS_IND_STRUCT_INFO,
   EM_CSCE_H_STATUS_IND_STRUCT_INFO,
   EM_CSCE_APBCR_STATUS_IND_STRUCT_INFO,
   EM_CSCE_MEAS_RULE_STATUS_IND_STRUCT_INFO,
   EM_CSCE_MULTIPLE_PLMN_IND_STRUCT_INFO,
   
   EM_MEME_INFO_DCH_UMTS_CELL_INFO = 90, MEME_EM_INFO_BEGIN = EM_MEME_INFO_DCH_UMTS_CELL_INFO,
   EM_MEME_INFO_DCH_GSM_CELL_INFO,
   EM_MEME_INFO_DCH_LTE_CELL_INFO,
   EM_MEME_INFO_EVENT_TYPE_1_PARAMETER_STRUCT_INFO,
   EM_MEME_INFO_EVENT_TYPE_2_PARAMETER_STRUCT_INFO,
   EM_MEME_INFO_EVENT_TYPE_3_PARAMETER_STRUCT_INFO,
//   EM_MEME_INFO_EVENT_TYPE_4_PARAMETER_STRUCT_INFO,
//   EM_MEME_INFO_EVENT_TYPE_5_PARAMETER_STRUCT_INFO,
//   EM_MEME_INFO_EVENT_TYPE_6_PARAMETER_STRUCT_INFO,
   EM_MEME_INFO_DCH_H_SERVING_CELL_INFO,
   EM_MEME_INFO_DCH_3G_BLER_INFO, MEME_EM_INFO_END = EM_MEME_INFO_DCH_3G_BLER_INFO,

   EM_RRCE_TGPS_STATUS_IND= 110,  
   EM_SLCE_SRNCID_STATUS_IND,
//#ifdef __UMTS_TDD128_MODE__
   EM_UAS_3G_TDD128_HANDOVER_SEQUENCE_IND = 130,
//#endif
   EM_SLCE_PS_DATA_RATE_STATUS_IND = 140,
   EM_RRCE_DCH_STATE_CONFIGURATION_STATUS_IND = 155,   
   EM_RRCE_FACH_STATE_CONFIGURATION_STATUS_IND,   
   EM_RRCE_CS_OVER_HSPA_STATUS_IND,
   URR_EM_INFO_END = EM_RRCE_CS_OVER_HSPA_STATUS_IND,
//#endif /* __UMTS_RAT__ */   
//#ifdef __UMTS_R8__
   /* __UL1_EM_MODE__ */
   UL1_EM_HS_DSCH_CONFIGURATION_INFO = 170, UL1_EM_INFO_BEGIN = UL1_EM_HS_DSCH_CONFIGURATION_INFO,
   UL1_EM_EDCH_CONFIGURATION_INFO,
   UL1_EM_CPC_CONFIGURATION_INFO,
   UL1_EM_SECONDARY_HS_CONFIGURATION_STATUS_INFO,
   UL1_EM_PRIMARY_HS_DSCH_BLER_INFO,
   UL1_EM_SECONDARY_HS_DSCH_BLER_INFO,
   UL1_EM_EDCH_ACK_RATE_INFO,   UL1_EM_INFO_END = UL1_EM_EDCH_ACK_RATE_INFO,
//#endif
//#if defined(__UMTS_RAT__) && defined(__UMTS_TDD128_MODE__)
   /* __UL2_EM_MODE__ */
   UL2_EM_ADM_POOL_STATUS_IND_STRUCT_INFO = 185, UL2_EM_INFO_BEGIN = UL2_EM_ADM_POOL_STATUS_IND_STRUCT_INFO,
   UL2_EM_PS_DATA_RATE_STATUS_IND_STRUCT_INFO, 
   UL2_EM_HSDSCH_RECONFIG_STATUS_IND_STRUCT_INFO,
   UL2_EM_URLC_EVENT_STATUS_IND_STRUCT_INFO,
   UL2_EM_3G_BLER_IND_STRUCT_INFO,
   /*UMAC new EM Arch*/
   /***HSUPA SI***/
   UL2_EM_HSUPA_SI_IND_STRUCT_INFO,
   /***HSUPA SI***/
   UL2_EM_INFO_END = UL2_EM_HSUPA_SI_IND_STRUCT_INFO,
//#endif

   /*ERRC_EM_MODE, here is the start of errc em info definition*/
   ERRC_EM_MOB_MEAS_INTRARAT_INFO = 210, ERRC_EM_INFO_BEGIN = ERRC_EM_MOB_MEAS_INTRARAT_INFO,
   ERRC_EM_MOB_MEAS_INTERRAT_UTRAN_INFO,
   ERRC_EM_MOB_MEAS_INTERRAT_GERAN_INFO,
   ERRC_EM_AUTOS_CSG_INFO,
   ERRC_EM_CARRS_EVENT_IND,
   ERRC_EM_SIB_EVENT_IND,   
   ERRC_EM_MOB_EVENT_IND,
   ERRC_EM_SEC_PARAM,
   ERRC_EM_REEST_INFO,
   ERRC_EM_RECONF_INFO,
   ERRC_EM_RCM_SIM_STS_INFO,
   ERRC_EM_SYS_SIB_RX_STS_INFO,
   ERRC_EM_ERRC_STATE_IND,
   ERRC_EM_OVER_PROC_DELAY_WARNING,
   ERRC_EM_LTE_SUPPORTED_BAND_INFO,   
   ERRC_EM_INFO_END = ERRC_EM_LTE_SUPPORTED_BAND_INFO,
   
   /* __ESM_EM_MODE__ */
   ESM_ESM_INFO,
    ESM_L4C_ESM_INFO,
   
   /* __EMM_EM_MODE__*/
   EMM_EM_SEC_INFO = 230, EMM_EM_INFO_BEGIN = EMM_EM_SEC_INFO,
   EMM_EM_PLMNSEL_INFO,
   EMM_EM_CONN_INFO,
   EMM_EM_NASMSG_INFO,
   EMM_EM_CALL_INFO,
   EMM_EM_REG_ATTACH_INFO,
   EMM_EM_REG_DETACH_INFO,   
   EMM_EM_REG_TAU_INFO,
   EMM_EM_REG_COMMON_INFO,
   EMM_EM_SV_INFO,
   EMM_EM_RATBAND_INFO,
   EMM_EM_TIMERSRV_INFO,
   EMM_EM_USIMSRV_INFO,
   EMM_EM_NVMSRV_INFO,
   EMM_EM_INFO_END = EMM_EM_NVMSRV_INFO,
   
    EMM_L4C_EMM_INFO,

   EM_EL2_OV_STATUS,
   EM_EL1_OV_STATUS,
   EM_QBM_STATUS,
   EM_UPCM_STATUS,

   /* EL1TX */
   EL1TX_EM_TX_INFO,

   EM_CSR_STATUS_IND,

   NUM_OF_EM_INFO
} em_info_enum;


// EMM & ESM structures
// Copyed from em_public_struct.h

/********************* begin of EMM definition ****************************/

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_EU1_UPDATE = 0,
    EM_EMM_EU2_NOT_UPDATE,
    EM_EMM_EU3_ROAMING_NOT_ALLOWED
} em_emm_update_status_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_SIM_REMOVE,
    EM_EMM_SIM_INVALID,
    EM_EMM_SIM_VALID,
} em_emm_sim_status_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_TIN_DELETED    = 0,
    EM_EMM_TIN_PTMSI,
    EM_EMM_TIN_RAT_RELATED_TMSI,
    EM_EMM_TIN_GUTI
} em_emm_tin_enum;

typedef struct
{
    plmn_id_struct  plmn_id;
    kal_uint8   mme_gid[2];
    kal_uint8   mme_code;
    kal_uint8   mtmsi[4];
} em_emm_guti_struct;

typedef enum __attribute__ ((__packed__))
{
    EM_ECM_IDLE=0,
    EM_ECM_CONNECTED
} em_emm_ecm_status_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_NORMAL_CELL    = 0,
    EM_CSG_CELL,
    EM_HYBRID_CELL
} em_csg_access_mode_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_DUPLEX_TYPE_FDD    = 0,
    EM_DUPLEX_TYPE_TDD,
    EM_DUPLEX_TYPE_UNKNOWN
} em_lte_duplex_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_TYPE_NONE                    = 0,
    EM_TYPE_SERVICE_REQ,
    EM_TYPE_EXTENDED_SERVICE_REQ,
    EM_TYPE_MAX
} em_sr_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_CAUSE_MO_SIGNAL              = 0,
    EM_CAUSE_MO_DATA,
    EM_CAUSE_MT,
    EM_CAUSE_EMERGENCY,
    EM_CAUSE_INVALID
} em_sr_cause_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_MO_CSFB_TYPE_INVALID         = 0,
    EM_MO_CSFB_TYPE_NORMAL_CALL,
    EM_MO_CSFB_TYPE_EMERGENCY_CALL,
    EM_MO_CSFB_TYPE_SS,
    EM_MO_CSFB_TYPE_LCS,
    EM_MO_CSFB_TYPE_RESERVED
} em_mo_csfb_cause_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_MT_CSFB_PAGING_ID_INVALID    = 0,
    EM_MT_CSFB_PAGING_ID_IMSI,
    EM_MT_CSFB_PAGING_ID_TMSI
} em_mt_csfb_paging_id_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_CAUSE_CAUSE_NONE = 0x00,
    EM_EMM_CAUSE_IMSI_UNKNOWN_IN_HSS    = 0x02,
    EM_EMM_CAUSE_ILLEGAL_UE    = 0x03,
    EM_EMM_CAUSE_IMEI_NOT_ACCEPTED    = 0x05,
    EM_EMM_CAUSE_ILLEGAL_ME    = 0x06,
    EM_EMM_CAUSE_EPS_NOT_ALLOWED    = 0x07,
    EM_EMM_CAUSE_EPS_NON_EPS_NOT_ALLOWED    = 0x08,
    EM_EMM_CAUSE_UE_ID_NOT_DERIVED_BY_NW    = 0x09,
    EM_EMM_CAUSE_IMPLICIT_DETACH    = 0x0a,
    EM_EMM_CAUSE_PLMN_NOT_ALLOWED    = 0x0b,
    EM_EMM_CAUSE_TA_NOT_ALLOWED    = 0x0c,
    EM_EMM_CAUSE_ROAMING_NOT_ALLOWED_IN_TA    = 0x0d,
    EM_EMM_CAUSE_EPS_NOT_ALLOWED_IN_PLMN    = 0x0e,
    EM_EMM_CAUSE_NO_SUITABLE_CELL_IN_TA    = 0x0f,
    EM_EMM_CAUSE_MSC_NOT_REACHABLE    = 0x10,
    EM_EMM_CAUSE_NW_FAILURE    = 0x11,
    EM_EMM_CAUSE_CS_NOT_AVAILABLE    = 0x12,
    EM_EMM_CAUSE_ESM_FAILURE    = 0x13,
    EM_EMM_CAUSE_MAC_FAILURE    = 0x14,
    EM_EMM_CAUSE_SYNCH_FAILURE    = 0x15,
    EM_EMM_CAUSE_CONGESTION    = 0x16,
    EM_EMM_CAUSE_UE_SEC_CAPA_MISMATCH    = 0x17,
    EM_EMM_CAUSE_SEC_MODE_REJ_UNSPECIFIED    = 0x18,
    EM_EMM_CAUSE_NOT_AUTH_FOR_CSG    = 0x19,
    EM_EMM_CAUSE_NON_EPS_AUTH_UNACCEPTABLE    = 0x1a,
    EM_EMM_CAUSE_CS_TEMP_NOT_AVAILABLE    = 0x27,
    EM_EMM_CAUSE_NO_EPS_CTXT_ACT    = 0x28,
    EM_EMM_CAUSE_SEMANTIC_INCORRECT    = 0x5f,
    EM_EMM_CAUSE_INVALID_MANDATORY_INFO    = 0x60,
    EM_EMM_CAUSE_MSG_TYPE_NOT_EXIST    = 0x61,
    EM_EMM_CAUSE_MSG_TYPE_NOT_COMPATIBLE    = 0x62,
    EM_EMM_CAUSE_IE_NOT_EXIST    = 0x63,
    EM_EMM_CAUSE_CONDITIONAL_IE_ERROR    = 0x64,
    EM_EMM_CAUSE_MESSAGE_NOT_COMPATIBLE    = 0x65,
    EM_EMM_CAUSE_PROTOCOL_ERROR    = 0x6f,

    EM_EMM_CAUSE_FORBIDDEN_PLMN, /* 0x70 */
    EM_EMM_CAUSE_NO_COVERAGE                       = 0x72,
    EM_EMM_CAUSE_SIM_REMOVED                       = 0x76,
    EM_EMM_CAUSE_SIM_INVALID_FOR_PS                = 0x78,
    EM_EMM_CAUSE_SIM_INVALID_FOR_CS,
    EM_EMM_CAUSE_SIM_INVALID_FOR_CS_AND_PS
} em_emm_cause_enum;


typedef enum __attribute__ ((__packed__))
{
    EM_EMM_ATTACH_TYPE_INVALID = 0,
    EM_EMM_ATTACH_TYPE_EPS_ATTACH    = 1,
    EM_EMM_ATTACH_TYPE_COMBINED_ATTACH    = 2,
    EM_EMM_ATTACH_TYPE_EMERGENCY_ATTACH    = 6
} em_emm_attach_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_ATTACH_RESULT_EPS_ONLY_ATTACHED    = 1,
    EM_EMM_ATTACH_RESULT_COMBINED_ATTACHED
} em_emm_attach_result_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_TYPE_NO_ADDITIONAL_INFO = 1,
    EM_EMM_TYPE_SMS_ONLY
} em_emm_additional_update_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_RESULT_NO_ADDITIONAL_INFO = 0,
    EM_EMM_RESULT_CSFB_NOT_PREFERRED = 1,
    EM_EMM_RESULT_SMS_ONLY           = 2
} em_emm_additional_update_result_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_UPDATE_TYPE_TAU    = 0,
    EM_EMM_UPDATE_TYPE_COMBINED_TAU,
    EM_EMM_UPDATE_TYPE_COMBINED_TAU_IMSI_ATTACH,
    EM_EMM_UPDATE_TYPE_PERIODIC_TAU,
    EM_EMM_UPDATE_TYPE_INVALID
} em_emm_update_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_UPDATE_RESULT_TA_UPDATED = 0,
    EM_EMM_UPDATE_RESULT_COMBINED_UPDATED = 1,
    EM_EMM_UPDATE_RESULT_TA_UPDATED_ISR_ACTIVATED = 4,
    EM_EMM_UPDATE_RESULT_COMBINED_UPDATED_ISR_ACTIVATED = 5,
    EM_EMM_UPDATE_RESULT_INVALID
} em_emm_update_result_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_USER_DETACH_EPS_DETACH  = 1,
    EM_EMM_USER_DETACH_IMSI_DETACH ,
    EM_EMM_USER_DETACH_COMBINED_EPS_IMSI_DETACH
} em_emm_user_detach_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_NW_DETACH_TYPE_RE_ATTACH_REQUIRED = 1,
    EM_EMM_NW_DETACH_TYPE_RE_ATTACH_NOT_REQUIRED,
    EM_EMM_NW_DETACH_TYPE_IMSI_DETACH
} em_emm_nw_detach_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_IMS_VOICE_OVER_PS_SESSION_IN_S1_MODE_NOT_SUPPORT = 0,
    EM_EMM_IMS_VOICE_OVER_PS_SESSION_IN_S1_MODE_SUPPORT
} em_emm_ims_service_ind_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_EMERGENCY_BEARER_SERVICE_IN_S1_MODE_NOT_SUPPORT = 0,
    EM_EMM_EMERGENCY_BEARER_SERVICE_IN_S1_MODE_SUPPORT
} em_emm_emergency_service_ind_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_ATTACH_NEEDED = 0,
    EM_EMM_ATTACH_REQUEST,
    EM_EMM_ATTACH_COMPLETE,
    EM_EMM_ATTACH_SUCCESS,
    EM_EMM_23G_ATTACH_SUCCESS
} em_emm_attach_status;

typedef enum __attribute__ ((__packed__))
{
    EM_EMM_TAU_ONGOING_BEFORE_TAU_REQ = 0,
    EM_EMM_TAU_REQUEST,
    EM_EMM_TAU_COMPLETE,
    EM_EMM_TAU_SUCCESS,
    EM_EMM_TAU_FAIL,
    EM_EMM_TAU_NO_TAU_EVER
} em_emm_tau_status;

typedef struct
{
    em_sr_type_enum              service_request_type;
    em_sr_cause_enum             service_request_cause;
    em_mo_csfb_cause_enum        mo_csfb_cause;
    em_mt_csfb_paging_id_enum    mt_csfb_paging_id;

} l4c_em_emm_call_para_struct;

typedef struct
{
    em_emm_ecm_status_enum ecm_status;

} l4c_em_emm_conn_para_struct;


typedef struct
{
    em_emm_attach_type_enum                 eps_attach_type;
    em_emm_additional_update_type_enum      attach_additional_update_type;
    em_emm_attach_result_enum               eps_attach_result;
    em_emm_additional_update_result_enum    attach_additional_update_result;
    em_emm_cause_enum                       attach_emm_cause;
    kal_uint32                              attach_attempt_count;
    em_emm_attach_status                    attach_status;

    em_emm_update_type_enum                 tau_req_update_type;
    em_emm_additional_update_type_enum      tau_additional_update_type;
    em_emm_update_result_enum               tau_update_result;
    em_emm_additional_update_result_enum    tau_additional_update_result;
    em_emm_cause_enum                       tau_emm_cause;
    kal_uint32                              tau_attempt_count;
    em_emm_tau_status                       tau_status;

    em_emm_user_detach_type_enum            user_detach_type;
    em_emm_nw_detach_type_enum              nw_detach_type;
    em_emm_cause_enum                       nw_detach_emm_cause;
    kal_uint32                              detach_attempt_count;

    em_emm_ims_service_ind_enum             ims_service_ind;
    em_emm_emergency_service_ind_enum       emergency_service_ind;

} l4c_em_emm_reg_para_struct;

typedef struct
{
    plmn_id_struct selected_plmn;
    kal_uint16        tac;
    em_csg_access_mode_enum  csg_access_mode;
    kal_uint32        csg_id;
    em_lte_duplex_type_enum  duplex_type;

} l4c_em_emm_plmnsel_para_struct;

typedef struct
{
    em_emm_update_status_enum update_status;
    em_emm_sim_status_enum cs_sim_status;
    em_emm_sim_status_enum ps_sim_status;
    em_emm_guti_struct guti;

} l4c_em_emm_usimsrv_para_struct;

typedef struct
{
    em_emm_tin_enum tin;
} l4c_em_emm_nvmsrv_para_struct;

typedef struct
{
    l4c_em_emm_call_para_struct             emm_call_para;
    l4c_em_emm_conn_para_struct             emm_conn_para;
    l4c_em_emm_reg_para_struct              emm_reg_para;
    l4c_em_emm_plmnsel_para_struct          emm_plmnsel_para;
    l4c_em_emm_usimsrv_para_struct          emm_usimsrv_para;
    l4c_em_emm_nvmsrv_para_struct           emm_nvmsrv_para;
} em_emm_l4c_emm_info_ind_struct;


/********************* end of EMM definition ****************************/


/********************* begin of ESM definition ****************************/

typedef enum __attribute__ ((__packed__))
{
    EM_ESM_SYSTEM_STATE_ATTACH_NONE,
    EM_ESM_SYSTEM_STATE_ATTACH_INIT,
    EM_ESM_SYSTEM_STATE_ATTACH_ING,
    EM_ESM_SYSTEM_STATE_ATTACH_NORMAL,
    EM_ESM_SYSTEM_STATE_ATTACH_EMERGENCY,
    EM_ESM_SYSTEM_STATE_RAT_CHANGE,
    EM_ESM_SYSTEM_STATE_RAT_2G3G
} l4c_em_esm_system_state_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_ESM_RAT_STATE_INACTIVE,
    EM_ESM_RAT_STATE_4G,
    EM_ESM_RAT_STATE_4G23_PENDING,
    EM_ESM_RAT_STATE_4G23_CONTEXT_MAP,
    EM_ESM_RAT_STATE_23G,
    EM_ESM_RAT_STATE_23G4_PENDING,
    EM_ESM_RAT_STATE_23G4_CONTEXT_MAP
} l4c_em_esm_rat_state_enum;

typedef struct
{
    l4c_em_esm_system_state_enum     em_esm_sys_state;
    l4c_em_esm_rat_state_enum         em_esm_rat_state;

    kal_uint8                                            em_esm_active_pt_num;
    kal_uint8                                            em_esm_active_epsb_num;
    kal_uint8                                            em_esm_active_drb_num;

} l4c_em_esm_status_struct;

typedef enum __attribute__ ((__packed__))
{
    EM_ESM_PT_REQ_REASON_NULL,
    EM_ESM_PT_REQ_REASON_REGISTER,
    EM_ESM_PT_REQ_REASON_ADD_PDN_CONN,
    EM_ESM_PT_REQ_REASON_ALLOC_BEARER_RSC,
    EM_ESM_PT_REQ_REASON_MOD_BEARER_RSC,
    EM_ESM_PT_REQ_REASON_DEACT_DED_BEARER,
    EM_ESM_PT_REQ_REASON_RMV_PDN_CONN
} l4c_em_esm_pt_req_reason_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_ESM_PT_STATE_TERM,
    EM_ESM_PT_STATE_INACTIVE,
    EM_ESM_PT_STATE_WAIT_MM_RES,
    EM_ESM_PT_STATE_PENDING,
    EM_ESM_PT_STATE_WAIT_RESEND,
    EM_ESM_PT_STATE_COMPLETE,
    EM_ESM_PT_STATE_RESERVED,
    EM_ESM_PT_STATE_WAIT_TERM
} l4c_em_esm_pt_state_enum;


typedef struct
{
    kal_bool                                                    is_active;
    kal_uint8                                                    pti;

    l4c_em_esm_pt_req_reason_enum            pt_req_reason;
    l4c_em_esm_pt_state_enum                    pt_state;

    kal_uint8                                                    cid;
    kal_uint8                                                    ebi;

} l4c_em_esm_pt_struct;

typedef enum __attribute__ ((__packed__))
{
    EM_ESM_EPSBC_TYPE_INVALID,
    EM_ESM_EPSBC_TYPE_DEFAULT_EPSB,
    EM_ESM_EPSBC_TYPE_DEDICATED_EPSB
} l4c_em_esm_epsbc_bearer_type_enum;


#define EM_MIN_APN_LEN 1
#define EM_MAX_APN_LEN 100

typedef struct
{
    kal_uint8            length;
    kal_uint8            data[EM_MAX_APN_LEN];

} em_apn_struct;

typedef struct
{
    kal_bool                                                        is_active;
    kal_uint8                                                        ebi;
    l4c_em_esm_epsbc_bearer_type_enum        bearer_type;
    kal_bool                                                        is_emergency_bearer;

    kal_uint8                                                        linked_ebi;
    kal_uint8                                                        qci;

    ip_addr_struct                                            ip_addr;
    em_apn_struct                                                apn;

} l4c_em_esm_epsbc_struct;

#define EM_L4C_ESM_MAX_PT_NUM                    10
#define EM_L4C_ESM_MAX_EPSB_NUM                11

typedef struct
{
    l4c_em_esm_status_struct            esm_status;
    l4c_em_esm_pt_struct                    esm_pt[EM_L4C_ESM_MAX_PT_NUM];
    l4c_em_esm_epsbc_struct                esm_epsbc[EM_L4C_ESM_MAX_EPSB_NUM];
} em_esm_l4c_esm_info_ind_struct;


/********************* end of ESM definition ****************************/


// 2/3G structures
// Copyed from mmdc_em_mode.txt
typedef struct                                                                                       
{                                                                                                           
    kal_uint8    mcc[3];    //MCC                                                                            
    kal_uint8    mnc[3];    //MNC                                                                            
    kal_uint8    lac[2];    //LAC                                                                            
    kal_uint16    cell_id;     //cell ID                                                                   
    kal_uint8    nc_info_index; // index in rlc array to acquire the corresponding arfcn, bsic, rxlevel...
    kal_uint8       rac;//RAC EM_RAC                                                                 
//#ifdef __PS_SERVICE__                                                                                
    /*091014 WISDOM_EM Michael Shuang NMO*/                                                          
    kal_uint8 nmo;                                                                                   
//#endif                                                                                               
    kal_uint8 supported_Band;                                                                        
} rr_em_lai_info_struct;   
typedef struct
{
    kal_bool    is_valid;
    kal_uint8   cs_report_type;
    kal_bool    is_invalid_bsic_rp;
    kal_uint8   cs_serv_band_rp;
    kal_uint8   cs_multi_band_rp;
    /* When cs_rp_threshold_850 is eqaul to 0xFF, it means never */
    kal_uint8   cs_rp_threshold_850;
    kal_uint8   cs_rp_offset_850;
    /* When cs_rp_threshold_900 is eqaul to 0xFF, it means never */
    kal_uint8   cs_rp_threshold_900;
    kal_uint8   cs_rp_offset_900;
    /* When cs_rp_threshold_1800 is eqaul to 0xFF, it means never */
    kal_uint8   cs_rp_threshold_1800;
    kal_uint8   cs_rp_offset_1800;
    /* When cs_rp_threshold_1900 is eqaul to 0xFF, it means never */
    kal_uint8   cs_rp_threshold_1900;
    kal_uint8   cs_rp_offset_1900; 
} rr_em_cs_meas_param_struct;

typedef struct
{
    kal_bool    is_valid;
    kal_uint8   ps_report_type;
    kal_bool    is_invalid_bsic_rp;
    kal_uint8   ps_serv_band_rp;
    kal_uint8   ps_multi_band_rp;
    /* When ps_rp_threshold_850 is eqaul to 0xFF, it means never */
    kal_uint8   ps_rp_threshold_850;
    kal_uint8   ps_rp_offset_850;
    /* When ps_rp_threshold_900 is eqaul to 0xFF, it means never */
    kal_uint8   ps_rp_threshold_900;
    kal_uint8   ps_rp_offset_900;
    /* When ps_rp_threshold_1800 is eqaul to 0xFF, it means never */
    kal_uint8   ps_rp_threshold_1800;
    kal_uint8   ps_rp_offset_1800;
    /* When ps_rp_threshold_1900 is eqaul to 0xFF, it means never */
    kal_uint8   ps_rp_threshold_1900;
    kal_uint8   ps_rp_offset_1900;
} rr_em_ps_meas_param_struct;

typedef struct
{

    kal_uint8            rr_state;                    /* NULL: 0, INACTIVE: 1, SELECTION: 2
                                                       * IDLE: 3, ACCESS: 4. PKT_TRANSFER: 6
                                                       * DEDICATED:6, RESELCTION: 7 */
    kal_uint8            meas_mode;                   /* GSM: 0, GPRS_CCCH: 1, GPRS_PMO_CCCH: 2
                                                       * GPRS_PCCCH = 3 */
    kal_uint16           serving_arfcn;               /* serving cell ARFCN */
    kal_uint8            serving_bsic;                /* serving cell BSIC */
    kal_uint8            serving_current_band;
    kal_uint8            serv_gprs_supported;         /* serving cell support GPRS or not */
    kal_int16            serv_rla_in_quarter_dbm;     /* RSSI level for serving cell 
                                                       * 1. BCCH in IDLE-state
                                                       * 2. TCH in DEDI-state
                                                       * 3. PDTCH in TRANSFER-state */
    kal_uint8            serv_rla_reported_value;     /* Reported value of RSSI level for serving cell */
    kal_bool             is_serv_BCCH_rla_valid;      /* To indicate if the serv_BCCH RSSI is valid*/               
    kal_int16            serv_BCCH_rla_in_dedi_state; /*RSSI level for serving cell (BCCH) in DEDI-State */
    kal_uint8            quality;                     /* serving cell -TCH measured quality */
    kal_bool             gprs_pbcch_present;          /* To indicate if the current cell supports GPRS 
                                                       * PBCCH is present */
    kal_bool             gprs_c31_c32_enable;         /* To indicate if the current mode is GPRS_PMO_MODE 
                                                       * or GPRS_PCCCH_MODE.
                                                       */  
    kal_int16            c1_serv_cell;                /* C1 value for the serving cell */
    kal_int16            c2_serv_cell;                /* if gprs_c31_c32_enable is false, this field is shown 
                                                       * as c2_value for serv_cell.
                                                       * if gprs_c31_c32_enable is true, this field is shown 
                                                       * as c32_value for serv_cell */
    kal_int16            c31_serv_cell;               /* if gprs_c31_c32_enable is false, this field is ignore
                                                       * if gprs_c31_c32_enable is true, this field shall be 
                                                       * shown in based on report c31_value */
    kal_uint8            num_of_carriers;             /* number of carriers in the BA list */
    kal_uint16           nc_arfcn[32];                /* ARFCN value in the BA list (The list will be sorted 
                                                       * by the RSSI level */
    kal_int16            rla_in_quarter_dbm[32];      /* rssi level for each carrier */
    kal_uint8            rla_in_reported_value[32];   /* Reported value of RSSI level for each carrier. */
    kal_uint8            nc_info_status[32];          /* Bit0 = 0: "nc_bsic","frame_offset","ebit_offset" is invalid
                                                       * Bit0 = 1: "nc_bsic","frame_offset","ebit_offset" is valid
                                                       * Bit1 = 0: "c1","c2" is invalid
                                                       * Bit1 = 1: "c1","c2" is valid 
                                                       * Bit2 = 0: "gprs_status" is invalid
                                                       * Bit2 = 1: "gprs_status" is valid */
    kal_uint8            nc_bsic[32];                 /* neighbor cell BSIC */
    kal_int32            frame_offset[32];            /* frame offset for each carrier */
    kal_int32            ebit_offset[32];             /* ebit offset for each carrier */
    kal_int16            c1[32];                      /* C1 value for the neighbor cell */
    kal_int16            c2[32];                      /* if gprs_c31_c32_enable is false, this field is shown as 
                                                       * c2_value for nbr_cell
                                                       * if gprs_c31_c32_enable is true, this field is shown as 
                                                       * c32_value for nbr_cell */
    kal_int16            c31[32];                     /* C31 value for the neighbor cell */ 
    kal_uint8            multiband_report;            /* MULTIBAND_REPORT value */
    kal_uint8            timing_advance;              /* Timing advance, range is 0 - 63 */
    kal_int16            tx_power_level; 
    kal_int16            serv_rla_full_value_in_quater_dbm;
    kal_uint8            nco;
    kal_uint8            rxqual_sub;                  /* rx quality (sub), range is 0 - 7 */
    kal_uint8            rxqual_full;                 /* RX quality (full), range is 0 - 7 */
    kal_int16            using_tx_power_in_dbm;       /* DL_DTX_AND_TX_POWER */
//#ifdef __AMR_SUPPORT__
	kal_bool			 amr_info_valid;
	kal_uint8			 cmr_cmc_cmiu_cmid;
	kal_uint8			 c_i;
	kal_uint16			 icm;
	kal_uint16			 acs;
	kal_bool			 dl_dtx_used;				  /* DL_DTX_AND_TX_POWER */ 
	kal_uint8 cmr;
	kal_uint8 cmc;
	kal_uint8 amr_ul_mode;
	kal_uint8 amr_dl_mode;
//#endif /* __AMR_SUPPORT__ */
#ifdef __FWP_NC_LAI_INFO__
    kal_uint8            num_of_nc_lai;               /* how many valid LAI info in nc_lai[] array */
    rr_em_lai_info_struct nc_lai[6];                  /* store LAI and CID for at most 6 strongest neighbor cells */
#endif /* __FWP_NC_LAI_INFO__ */
	rr_em_cs_meas_param_struct cs_meas_param;
	rr_em_ps_meas_param_struct ps_meas_param;
} rr_em_measurement_report_info_struct;

typedef struct
{
    rr_em_measurement_report_info_struct rr_em_measurement_report_info;
} em_rrm_measurement_report_info_ind_struct;

typedef struct
{
   kal_uint8         cellidx;
   kal_uint16        uarfcn_DL;   /* UARFCN */  //MMI
   kal_uint16        psc;                       //MMI
   kal_bool          is_s_criteria_satisfied;   //MMI
   /* is_s_criteria_satisfied, 0: FALSE, 1: TRUE */
   kal_int8          qQualmin;  /* Qualmin */   //MMI
   kal_int8          qRxlevmin; /* Qrxlevmin */ //MMI
   /* the following value should be divided by 4096, the result may be float */
   kal_int32         srxlev;                    //MMI                   
   kal_int32         squal;                     //MMI               
   kal_int32         rscp;                      //MMI
//#ifdef __UMTS_FDD_MODE__   
   kal_int32         ec_no;                     //MMI
//#endif   
   kal_uint16        cycleLength;               //MMI
//#ifdef __UMTS_FDD_MODE__   
/* number as k, value is 2k/100 in sec, float */
   kal_uint8         quality_measure;   /* 0: RSCP, 1: EcN0 */ //MMI
//#endif
   /**
    * Dennis Weng 20101015
    * add band, rssi, cell_identity
    */   
   kal_uint8         band;
   kal_int32         rssi;
   kal_uint32        cell_identity;
//#ifdef __UMTS_R8__
/* csg_id: 0xFFFFFFFF means not show this csg_id */
   kal_uint32        csg_id;
/* apbcr_priority: -2 means not show apbcr_priority, Sprio_search1, Sprio_search2, and Threshserv_low */
   kal_int8          apbcr_priority;             //MMI
   kal_uint8         sprio_search1;              //MMI
   kal_uint8         sprio_search2;              //MMI
   kal_uint8         threshserv_low;             //MMI
   kal_uint8         threshserv_low2;            //MMI, 0xFF means invalid
//#endif
} csce_em_serv_cell_s_status;

typedef struct
{
    csce_em_serv_cell_s_status serv_cell;
} em_csce_serv_cell_s_status_ind_struct;

typedef struct
{
    kal_uint16 UARFCN;
    kal_uint16 CELLPARAID;
    kal_int32 RSCP;
    kal_int32 ISCP[6];
    kal_bool isServingCell;
} meme_umts_cell_struct;

typedef struct
{
    kal_uint16 workingFreq; // FDD only
    kal_uint8 num_cells;
    kal_uint8 supportBand;
    kal_uint8 sinr;
    meme_umts_cell_struct umts_cell_list[64];
} meme_em_info_umts_cell_status_struct;

// 4G structures
// Copyed from errc_em_str_n_enum.h
#define ERRC_MOB_EM_UPDATE_MAX_INTRA_CELL_QUAN       16
#define ERRC_MOB_EM_UPDATE_MAX_INTER_FREQ_CELLS_QUAN 6
#define ERRC_MOB_EM_UPDATE_MAX_INTER_FREQ_QUAN       4
#define ERRC_MOB_EM_UPDATE_MAX_IR_UTRAN_FREQ_QUAN    3
#define ERRC_MOB_EM_UPDATE_MAX_IR_GERAN_CELL_QUAN    6
#define ERRC_MOB_EM_UPDATE_MAX_IR_UTRAN_CELL_QUAN    6

typedef struct
{
    kal_uint16 earfcn;
    kal_uint16 pci;    
    kal_uint32 csg_id;    
} em_errc_csg_info_struct;

typedef struct
{
    kal_uint8               num_cells;
    em_errc_csg_info_struct detected_csg_cell[3];
} em_errc_autos_info_struct;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_CARRS_TRIG_TYPE_NAS,
    EM_ERRC_CARRS_TRIG_TYPE_AS,
    EM_ERRC_CARRS_TRIG_TYPE_IRAT    
} em_errc_carrs_trigger_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_CARRS_TYPE_PLMN_LIST,
    EM_ERRC_CARRS_TYPE_CSG_LIST,
    EM_ERRC_CARRS_TYPE_FINGER_PRINT,
    EM_ERRC_CARRS_TYPE_MCC_LEARNING,
    EM_ERRC_CARRS_TYPE_BACKGROUND_SEARCH,
    EM_ERRC_CARRS_TYPE_AUTO_SEARCH,
    EM_ERRC_CARRS_TYPE_OOS
} em_errc_carrs_type_enum;

typedef struct
{
    em_errc_carrs_trigger_type_enum trigger_type;
    em_errc_carrs_type_enum         carrs_type;    
} em_errc_carrs_event_struct;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_SIB_EVENT_3H_TIMEOUT,
    EM_ERRC_SIB_EVENT_SIB_MODIFY,
    EM_ERRC_SIB_EVENT_PWS
} em_errc_sib_event_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_MOB_TYPE_CR,
    EM_ERRC_MOB_TYPE_REDT,
    EM_ERRC_MOB_TYPE_CCO,
    EM_ERRC_MOB_TYPE_HO,
    EM_ERRC_MOB_TYPE_REEST    
} em_errc_mob_type_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_MOB_DIR_INTRA_LTE,
    EM_ERRC_MOB_DIR_TO_LTE,
    EM_ERRC_MOB_DIR_FROM_LTE
} em_errc_mob_dir_enum;

typedef struct
{
    em_errc_mob_type_enum           mob_type;
    em_errc_mob_dir_enum            mob_dir;    
} em_errc_mob_event_struct;

typedef struct
{
    LOCAL_PARA_HDR
    em_info_enum em_info;
} em_errc_mob_meas_info_ind_struct;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_RCM_SIM_STS_REMOVED,
    EM_ERRC_RCM_SIM_STS_INVALID_BY_NAS,
    EM_ERRC_RCM_SIM_STS_VALID    
} em_errc_rcm_sim_sts_enum;

typedef enum __attribute__ ((__packed__))
{
    EM_ERRC_SYS_SIB_RX_STS_IDLE,
    EM_ERRC_SYS_SIB_RX_STS_WAIT,
    EM_ERRC_SYS_SIB_RX_STS_CMPL,
    EM_ERRC_SYS_SIB_RX_STS_STORED    
} em_errc_sys_sib_rx_sts_enum;

typedef struct
{
    kal_bool                    is_rxing;
    kal_uint32                  earfcn;
    kal_uint16                  pci;
    em_errc_sys_sib_rx_sts_enum mib_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib1_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib2_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib3_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib4_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib5_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib6_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib7_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib9_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib10_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib11_rx_sts;
    em_errc_sys_sib_rx_sts_enum sib12_rx_sts;
} em_errc_sib_rx_info_struct;

/*MSG_ID_EM_ERRC_AUTOS_CSG_INFO_IND*/
typedef struct
{
    em_errc_autos_info_struct autos_info;
} em_errc_autos_csg_info_ind_struct;

/*MSG_ID_EM_ERRC_CARRS_EVENT_IND*/
typedef struct
{
    em_errc_carrs_event_struct carrs_evt;
} em_errc_carrs_event_ind_struct;

/*MSG_ID_EM_ERRC_SIB_EVENT_IND*/
typedef struct
{
    LOCAL_PARA_HDR
    em_info_enum em_info;
    em_errc_sib_event_enum sib_evt;
} em_errc_sib_event_ind_struct;


/*MSG_ID_EM_ERRC_MOB_EVENT_IND*/
typedef struct
{
    em_errc_mob_event_struct mob_evt;
} em_errc_mob_event_ind_struct;

typedef struct
{
    kal_bool                     key_chg_ind;
    //kal_uint32                   ul_nas_cnt;
    kal_uint8                    ncc;
    kal_uint8                      int_algo;
    kal_uint8                      enc_algo;
    //kal_uint8                    smc_maci[4];
    //kal_uint8                    gen_maci[4];
    //kal_uint8                    short_maci[2];
    //kal_uint8                    kasme[32];
    //kal_uint8                    kenb[32];
} em_errc_sec_param_ind_struct;

typedef enum __attribute__ ((__packed__))
{
    ERRC_REEST_CAUSE_LTE_HO_FAIL,
    ERRC_REEST_CAUSE_LTE_MFROM_FAIL,
    ERRC_REEST_RLC_ACK_TIMEOUT,
    ERRC_REEST_SIB_UPDT_FAIL,
    ERRC_REEST_CAUSE_L1_RLF,
    ERRC_REEST_CAUSE_MAC_RLF,
    ERRC_REEST_CAUSE_RLC_RLF,
    ERRC_REEST_CAUSE_INTECHK_FAIL,
    ERRC_REEST_CAUSE_RECONF_FAIL
} em_errc_reest_info_cause_enum;

typedef struct
{
    em_errc_reest_info_cause_enum cause;
} em_errc_reest_info_ind_struct;

typedef enum __attribute__ ((__packed__))
{
    ERRC_NORMAL_RECONF,
    ERRC_INTRA_CEL_HO,
    ERRC_INTER_CEL_HO,
    ERRC_INTER_RAT_HO
} em_errc_reconf_info_type_enum;

typedef struct
{
    em_errc_reconf_info_type_enum reconf_type;
    kal_bool                      is_cell_info_valid;
    kal_uint32                    earfcn;
    kal_uint16                    pci;
    kal_uint16                    crnti;
    kal_uint32                    t311;
    kal_uint32                    t301;    
    kal_uint32                    t304;
} em_errc_reconf_info_ind_struct;

typedef enum __attribute__ ((__packed__))
{
      //This enum must be consistent with errc_procedure_id_enum
      EM_ERRC_PROC_EST,
      EM_ERRC_PROC_RECONF_RR_MEAS_CONFIG,
      EM_ERRC_PROC_RECONF_INTRALTEMOBILITY,
      EM_ERRC_PROC_REEST,
      EM_ERRC_PROC_INITIAL_SEC,
      EM_ERRC_UE_CAPABILITY,
      EM_ERRC_COUNTER_CHECK,
      EM_ERRC_UE_INFORMATION,
} em_errc_delay_proc_enum;

typedef struct
{
    em_errc_delay_proc_enum delay_proc_id;
    kal_uint32                             delay_time;
} em_errc_over_proc_delay_warning_ind_struct;

typedef struct
{
    em_errc_rcm_sim_sts_enum   sim_sts;
} em_errc_rcm_sim_sts_info_ind_struct;

typedef struct
{
    em_errc_sib_rx_info_struct   sib_rx_info[3]; /*Serving BCCH, Neighbor BCCH1, Neighbor BCCH2*/
} em_errc_sys_sib_rx_sts_info_ind_struct;

typedef enum __attribute__ ((__packed__))
{
    ERRC_INITIAL,
    ERRC_STANDBY,
    ERRC_IDLE,
    ERRC_CONNECTED,
    ERRC_FLIGHT,
    ERRC_IDLE_IRSUS,
    ERRC_CONN_IRSUS
} em_errc_state_enum;

typedef struct
{
    em_errc_state_enum errc_sts;
} em_errc_state_ind_struct;


/*MSG_ID_EM_LTE_SUPPORTED_BAND_INFO_IND*/
typedef struct
{
    kal_uint8    num_supported_band;
    kal_uint8    supported_band[64];
} em_lte_supported_band_info_ind_struct;


typedef enum __attribute__ ((__packed__)) _errc_mob_spj_state_enum {
    ERRC_MOB_HIGH_MOBILITY,       // 0x00           /* High Mobility                   */
    ERRC_MOB_MEDIUM_MOBILITY,     // 0x01           /* Medium Mobility                 */
    ERRC_MOB_NORMAL_MOBILITY      // 0x02           /* Normal Mobility                 */
} errc_mob_spj_state_enum;


typedef enum __attribute__ ((__packed__)) {
    ERRC_MOB_MEAS_BANDWIDTH_6_RB,        //0x00
    ERRC_MOB_MEAS_BANDWIDTH_15_RB,       //0x01
    ERRC_MOB_MEAS_BANDWIDTH_25_RB,       //0x02
    ERRC_MOB_MEAS_BANDWIDTH_50_RB,       //0x03
    ERRC_MOB_MEAS_BANDWIDTH_75_RB,       //0x04
    ERRC_MOB_MEAS_BANDWIDTH_100_RB,      //0x05
    ERRC_MOB_MEAS_BANDWIDTH_INVALID=0xFF   //0xFF
} errc_mob_bandwidth_enum;



/*ERRC_MOB_EM_MODE*/
typedef struct _errc_mob_em_serving_info_struct{
    kal_uint16 earfcn;
    kal_uint16 pci;
    kal_int32 rsrp;
    kal_int32 rsrq;
    errc_mob_spj_state_enum mobility_state;
    kal_uint8 S_intra_search_p;
    kal_uint8 S_intra_search_q;
    kal_uint8 S_nonintra_search_p;
    kal_uint8 S_nonintra_search_q ;
    kal_uint8 thresh_serving_low_p;
    kal_uint8 thresh_serving_low_q;
    kal_uint8 tresel;
}  errc_mob_em_serving_info_struct;

typedef struct _errc_mob_em_intrarat_intra_info_ecell_struct{
    kal_bool valid;
    kal_uint16 pci;
    kal_int32 rsrp;
    kal_int32 rsrq; 
}  errc_mob_em_intrarat_intra_info_ecell_struct;

typedef struct _errc_mob_em_intrarat_intra_info_struct{
    kal_int8 priority;
    errc_mob_bandwidth_enum bandwidth;
    kal_bool is_blacklist_present;
    kal_uint16 pcomp;
    kal_uint8 cell_num;
    errc_mob_em_intrarat_intra_info_ecell_struct intra_cell[ERRC_MOB_EM_UPDATE_MAX_INTRA_CELL_QUAN];
} errc_mob_em_intrarat_intra_info_struct;

typedef struct _errc_mob_em_intrarat_inter_info_ecell_struct{
    kal_bool valid;
    kal_uint16 pci;
    kal_int32 rsrp;
    kal_int32 rsrq;
} errc_mob_em_intrarat_inter_info_ecell_struct;

typedef struct _errc_mob_em_intrarat_inter_info_inter_freq_struct{
    kal_bool valid;
    kal_uint16 earfcn;
    kal_uint16 pcomp;
    kal_int8 priority;
    errc_mob_bandwidth_enum bandwidth;
    kal_bool is_blacklist_present;
    kal_uint8 treselection;
    kal_uint8 thresh_x_high_p;
    kal_uint8 thresh_x_high_q;
    kal_uint8 thresh_x_low_p;
    kal_uint8 thresh_x_low_q;
    kal_uint8 cell_num;
    errc_mob_em_intrarat_inter_info_ecell_struct inter_cell[ERRC_MOB_EM_UPDATE_MAX_INTER_FREQ_CELLS_QUAN];
} errc_mob_em_intrarat_inter_info_inter_freq_struct;

typedef struct _errc_mob_em_intrarat_inter_info_struct{
    kal_uint8 freq_num;
    errc_mob_em_intrarat_inter_info_inter_freq_struct inter_freq[ERRC_MOB_EM_UPDATE_MAX_INTER_FREQ_QUAN];
} errc_mob_em_intrarat_inter_info_struct;


typedef struct _em_errc_mob_meas_intrarat_info_ind_struct{
    errc_mob_em_serving_info_struct serving_info;
    errc_mob_em_intrarat_intra_info_struct intra_info;
    errc_mob_em_intrarat_inter_info_struct inter_info;
} em_errc_mob_meas_intrarat_info_ind_struct;

typedef struct _errc_mob_em_ir_geran_gcell_struct{
    kal_bool valid; /*only used in sorting the em result*/
    kal_int8 priority;
    kal_bool band_ind;
    kal_uint16 arfcn;
    kal_uint8 bsic;
    kal_int32 rssi;
    kal_uint8 thresh_x_high;
    kal_uint8 thresh_x_low;
} errc_mob_em_ir_geran_gcell_struct;

typedef struct _em_errc_mob_meas_interrat_geran_info_ind_struct{
    kal_uint8 gcell_num;
    errc_mob_em_ir_geran_gcell_struct gcell[ERRC_MOB_EM_UPDATE_MAX_IR_GERAN_CELL_QUAN];
} em_errc_mob_meas_interrat_geran_info_ind_struct;

typedef struct _errc_mob_em_ir_utran_ucell_struct{
    kal_bool valid;
    kal_uint16 psc;
    kal_int32 rscp;
    kal_int32 ec_n0;
} errc_mob_em_ir_utran_ucell_struct;

typedef struct _errc_mob_em_ir_utran_freq_struct{
    kal_bool valid;
    kal_uint16 uarfcn;
    kal_int8 priority;
    kal_uint8 threshx_high_p;
    kal_uint8 threshx_high_q;
    kal_uint8 threshx_low_p;
    kal_uint8 threshx_low_q;
    kal_uint8 ucell_num;
    errc_mob_em_ir_utran_ucell_struct ucell[ERRC_MOB_EM_UPDATE_MAX_IR_UTRAN_CELL_QUAN];
} errc_mob_em_ir_utran_freq_struct;


typedef struct _em_errc_mob_meas_interrat_utran_info_ind_struct{
    kal_uint8 freq_num;
    errc_mob_em_ir_utran_freq_struct inter_freq[ERRC_MOB_EM_UPDATE_MAX_IR_UTRAN_FREQ_QUAN];
} em_errc_mob_meas_interrat_utran_info_ind_struct;

// Copyed from el1_em_str.txt
typedef struct
{
    kal_uint16 qpsk_cnt;
    kal_uint16 qam16_cnt;
    kal_uint16 qam64_cnt;
    
    /* CQI, PMI, RI*/
    /* temp not used */
    kal_int16  cqi;
    kal_int16  pmi;
    kal_int16  ri;
    
    /* SRC info*/
    kal_uint16 tx_rb_cnt[22];          //0~110, step = 5, RB number
    kal_uint16 dci_cnt[11];            //0/1/1A/1B/1C/1D/2/2A/2B/3/3A
    kal_uint16 tx_format[9];           //PUCCH:1,1a,1b,2, 2_etd_cp, 2a,2b, PUSCH, PRACH

    /* PWR info */
    kal_uint16 max_power_pusch_cnt;
    kal_uint16 max_power_pucch_cnt;
    kal_uint16 prach_tx_power_cnt[12]; //-50~23dBm
    kal_uint16 pucch_tx_power_cnt[12]; //-50~23dBm
    kal_uint16 pusch_tx_power_cnt[12]; //-50~23dBm
    kal_uint16 srs_tx_power_cnt[12];   //-50~23dBm
    kal_int16  prach_tx_power_ave;
    kal_int16  pucch_tx_power_ave;
    kal_int16  pusch_tx_power_ave;
    kal_int16  srs_tx_power_ave;
    kal_int16  tx_power_ave;           // all channel tx power average
    kal_uint16 fi_cnt[10];             // -4, -1, 1, 4,-1, 0, 1, 3, -1, 1 pusch tpc command
    kal_uint16 gi_cnt[6];              // -1, 0, 1, 3, -1, 1 pusch tpc command
    kal_int8   fi_ave;
    kal_int8   gi_ave;
}em_el1tx_status_ind_struct;

#ifdef __cplusplus
}
#endif
#endif
