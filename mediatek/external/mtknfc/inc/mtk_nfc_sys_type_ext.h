/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/******************************************************************************
*[File] mtk_nfc_sys_type_ext.h
*[Version] v1.0
*[Revision Date] 2012-05-31
*[Author] LiangChi Huang, LiangChi.Huang@mediatek.com, 25609
*[Description]
*[Copyright]
*    Copyright (C) 2008 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/

#ifndef MTK_NFC_TYPE_EXT_H
#define MTK_NFC_TYPE_EXT_H

#include "mtk_nfc_sys_type.h"

#define SUPPORT_EM
#define SUPPORT_FM
#define SUPPORT_JNI

/* MESSAGE TYPE */
typedef enum {
    /* FOR DEMO TOOL*/
    MTK_NFC_CHIP_CONNECT_REQ = 0,
    MTK_NFC_CHIP_CONNECT_RSP,
    MTK_NFC_GET_SELIST_REQ,
    MTK_NFC_GET_SELIST_RSP,
    MTK_NFC_DISCOVERY_REQ,
    MTK_NFC_DISCOVERY_RSP,
    MTK_NFC_DISCOVERY_NTF,
    MTK_NFC_TAG_READ_REQ,
    MTK_NFC_TAG_READ_RSP,
    MTK_NFC_TAG_WRITE_REQ,
    MTK_NFC_TAG_WRITE_RSP,  // 10
    MTK_NFC_TAG_DISCONNECT_REQ,
    MTK_NFC_TAG_DISCONNECT_RSP,
    MTK_NFC_TAG_FORMAT_REQ,
    MTK_NFC_TAG_FORMAT_RSP,
    MTK_NFC_TAG_RAWCMD_REQ,
    MTK_NFC_TAG_RAWCMD_RSP,
    MTK_NFC_P2P_START_REQ,
    MTK_NFC_P2P_START_RSP,
    MTK_NFC_P2P_CONN_REQ,
    MTK_NFC_P2P_RECEV_RSP,  // 20
    MTK_NFC_P2P_DISC_REQ,
    MTK_NFC_P2P_DISC_RSP, 
    MTK_NFC_SE_NTF,
    MTK_NFC_MULTIPLE_TAG_NTF,
    MTK_NFC_MULTIPLE_TAG_SELECT_REQ,
    MTK_NFC_MULTIPLE_TAG_SELECT_RSP,
    MTK_NFC_STOP_REQ,
    MTK_NFC_STOP_RSP,
    MTK_NFC_OPTION_REQ,
    MTK_NFC_OPTION_RSP,     // 30
    MTK_NFC_PER_REQ,
    MTK_NFC_PER_RSP, 
    MTK_NFC_SNR_REQ,
    MTK_NFC_SNR_RSP, 
 
    MTK_NFC_P2P_CHK_LLCP_REQ,
    MTK_NFC_P2P_CHK_LLCP_RSP,
    MTK_NFC_P2P_LINK_REQ,
    MTK_NFC_P2P_ACTIVATE_REQ,
    MTK_NFC_P2P_ACTIVATE_RSP,
    MTK_NFC_P2P_LINK_NTF,    
    MTK_NFC_P2P_CREATE_SERVER_REQ,
    MTK_NFC_P2P_CREATE_SERVER_RSP,    
    MTK_NFC_P2P_CREATE_SERVER_NTF,
    MTK_NFC_P2P_ACCEPT_SERVER_REQ,
    MTK_NFC_P2P_ACCEPT_SERVER_RSP,    
    MTK_NFC_P2P_CREATE_CLIENT_REQ,
    MTK_NFC_P2P_CREATE_CLIENT_RSP,     
    MTK_NFC_P2P_CONNECT_CLIENT_REQ,
    MTK_NFC_P2P_CONNECT_CLIENT_RSP,
    MTK_NFC_P2P_CONNECTION_NTF,
    MTK_NFC_P2P_SOCKET_STATUS_NTF,
    MTK_NFC_P2P_GET_REM_SETTING_REQ,
    MTK_NFC_P2P_GET_REM_SETTING_RSP,
    MTK_NFC_P2P_SEND_DATA_REQ,
    MTK_NFC_P2P_SEND_DATA_RSP,
    MTK_NFC_P2P_RECV_DATA_REQ,
    MTK_NFC_P2P_RECV_DATA_RSP,    
 
    MTK_NFC_SE_MODE_SET_REQ,
    MTK_NFC_SE_MODE_SET_RSP,
    MTK_NFC_VIRTUAL_CARD_REQ, 
    MTK_NFC_VIRTUAL_CARD_RSP, 

    MTK_NFC_DEMO_P2P_DISC_REQ,
    MTK_NFC_DEMO_P2P_DISC_RSP, 

    MTK_NFC_DISABLE_AUTO_CHECKPRESENCE,
    
    MTK_NFC_FULLPOLL_REQ,
    MTK_NFC_FULLPOLL_RSP,    

    MTK_NFC_DEBUG_REQ,
    MTK_NFC_DEBUG_RSP,    
    
    #ifdef SUPPORT_EM
    MTK_NFC_EM_START_CMD = 100,
    MTK_NFC_EM_ALS_READER_MODE_REQ,
    MTK_NFC_EM_ALS_READER_MODE_RSP,
    MTK_NFC_EM_ALS_READER_MODE_OPT_REQ,
    MTK_NFC_EM_ALS_READER_MODE_OPT_RSP,
    MTK_NFC_EM_ALS_P2P_MODE_REQ,
    MTK_NFC_EM_ALS_P2P_MODE_RSP,
    MTK_NFC_EM_ALS_CARD_MODE_REQ,
    MTK_NFC_EM_ALS_CARD_MODE_RSP,
    MTK_NFC_EM_POLLING_MODE_REQ,
    MTK_NFC_EM_POLLING_MODE_RSP,
    MTK_NFC_EM_TX_CARRIER_ALS_ON_REQ,
    MTK_NFC_EM_TX_CARRIER_ALS_ON_RSP,
    MTK_NFC_EM_VIRTUAL_CARD_REQ,
    MTK_NFC_EM_VIRTUAL_CARD_RSP,
    MTK_NFC_EM_PNFC_CMD_REQ,
    MTK_NFC_EM_PNFC_CMD_RSP,
    MTK_NFC_EM_POLLING_MODE_NTF,
    MTK_NFC_EM_ALS_READER_MODE_NTF,
    MTK_NFC_EM_ALS_P2P_MODE_NTF,
    MTK_NFC_EM_STOP_CMD,
    MTK_NFC_EM_POLLING_MODE_UPT,
    MTK_NFC_EM_READER_MODE_UPT,
    MTK_NFC_EM_ALS_CARD_MODE_PROC,
    MTK_NFC_EM_VIRTUAL_CARD_PROC,
    MTK_NFC_EM_TX_CARRIER_ALS_ON_PROC,
    MTK_NFC_EM_PNFC_CMD_PROC,
    MTK_NFC_TESTMODE_SETTING_REQ,
    MTK_NFC_TESTMODE_SETTING_RSP,
    MTK_EM_LOOPBACK_TEST_REQ,
    MTK_EM_LOOPBACK_TEST_RSP,
    MTK_NFC_SW_VERSION_QUERY,
    MTK_NFC_SW_VERSION_RESPONSE,  
    MTK_NFC_EM_VIRTUAL_CARD_CK_SE,
    MTK_NFC_EM_EINT_CK_PROC,
    #endif
    
    #ifdef SUPPORT_FM
    MTK_NFC_FM_SETART_CMD = 200,
    MTK_NFC_FM_SWP_TEST_REQ,
    MTK_NFC_FM_SWP_TEST_NTF,
    MTK_NFC_FM_SWP_TEST_RSP,
    MTK_NFC_FM_READ_UID_TEST_REQ,
    MTK_NFC_FM_READ_UID_TEST_RSP,
    MTK_NFC_FM_READ_DEP_TEST_REQ,
    MTK_NFC_FM_READ_DEP_TEST_RSP,
    MTK_NFC_FM_CARD_MODE_TEST_REQ,
    MTK_NFC_FM_CARD_MODE_TEST_RSP,
    MTK_NFC_FM_PNFC_CMD_PROC,
    MTK_NFC_FM_STOP_CMD,
    MTK_NFC_FM_VIRTUAL_CARD_REQ,
    MTK_NFC_FM_VIRTUAL_CARD_RSP,
    #endif
 
    #ifdef SUPPORT_JNI
    MTK_NFC_INIT_REQ = 300,
    MTK_NFC_INIT_RSP,
    MTK_NFC_DEINIT_REQ,
    MTK_NFC_DEINIT_RSP,
    MTK_NFC_INIT_DEINIT_PROC,
    MTK_NFC_EXIT_REQ,
    MTK_NFC_EXIT_RSP,
    MTK_NFC_SET_CONFIG_REQ,
    MTK_NFC_SET_CONFIG_PROC,
    MTK_NFC_SET_CONFIG_RSP,
    MTK_NFC_GET_CONFIG_REQ,
    MTK_NFC_GET_CONFIG_PROC,
    MTK_NFC_GET_CONFIG_RSP,
    MTK_NFC_SET_DISCOVER_REQ,
    MTK_NFC_SET_DISCOVER_PROC,
    MTK_NFC_SET_DISCOVER_RSP,
    MTK_NFC_SET_DISCOVER_NTF_PROC,
    MTK_NFC_SET_DISCOVER_NTF,    
    MTK_NFC_DEV_SELECT_REQ,
    MTK_NFC_DEV_SELECT_PROC,
    MTK_NFC_DEV_SELECT_RSP,
    MTK_NFC_DEV_ACTIVATE_PROC,
    MTK_NFC_DEV_ACTIVATE_NTF,
    MTK_NFC_DEV_DEACTIVATE_REQ,
    MTK_NFC_DEV_DEACTIVATE_PROC,
    MTK_NFC_DEV_DEACTIVATE_RSP,
    MTK_NFC_DEV_DEACTIVATE_NTF_PROC,
    MTK_NFC_DEV_DEACTIVATE_NTF,
    MTK_NFC_JNI_TAG_READ_REQ,
    MTK_NFC_JNI_TAG_READ_RSP,
    MTK_NFC_JNI_TAG_WRITE_REQ,
    MTK_NFC_JNI_TAG_WRITE_RSP,
    MTK_NFC_JNI_CHECK_NDEF_REQ,
    MTK_NFC_JNI_CHECK_NDEF_RSP,
    MTK_NFC_JNI_TAG_PRESENCE_CK_REQ,
    MTK_NFC_JNI_TAG_PRESENCE_CK_RSP,
    MTK_NFC_JNI_TAG_FORMAT_REQ,
    MTK_NFC_JNI_TAG_FORMAT_RSP,
    MTK_NFC_JNI_TAG_MAKEREADONLY_REQ,
    MTK_NFC_JNI_TAG_MAKEREADONLY_RSP,
    MTK_NFC_JNI_TAG_TRANSCEIVE_REQ,
    MTK_NFC_JNI_TAG_TRANSCEIVE_RSP,
    MTK_NFC_JNI_SE_MODE_SET_REQ,
    MTK_NFC_JNI_SE_MODE_SET_RSP,
    MTK_NFC_JNI_SE_GET_CUR_SE_REQ,
    MTK_NFC_JNI_SE_GET_CUR_SE_RSP,
    MTK_NFC_JNI_P2P_TRANSCEIVE_REQ,
    MTK_NFC_JNI_P2P_TRANSCEIVE_RSP,    
    MTK_NFC_DEV_EVT_NTF_PROC,    
    MTK_NFC_DEV_EVT_NTF,
    MTK_NFC_JNI_SE_OPEN_CONN_REQ,
    MTK_NFC_JNI_SE_OPEN_CONN_RSP,
    MTK_NFC_JNI_SE_CLOSE_CONN_REQ,
    MTK_NFC_JNI_SE_CLOSE_CONN_RSP,
    MTK_NFC_JNI_SE_SEND_DATA_REQ,
    MTK_NFC_JNI_SE_SEND_DATA_RSP,

    #endif
 
    #if 1//def SUPPORT_HAL_TEST
    MTK_NFC_TEST_HAL_OPEN = 900,
    MTK_NFC_TEST_HAL_CLOSE,
    MTK_NFC_TEST_HAL_SELECT,
    MTK_NFC_TEST_HAL_DEACTIVATE,
    MTK_NFC_TEST_HAL_GET_CONFIG,
    MTK_NFC_TEST_TIMER_ONESHOT,
    MTK_NFC_TEST_TIMER_PERIODICAL,
    MTK_NFC_TEST_T2T_DATA_EXCHANGE,
    MTK_NFC_TEST_LOOPBACK_CONN_CREATE,
    MTK_NFC_TEST_LOOPBACK_CONN_SEND,     
    MTK_NFC_TEST_LOOPBACK_CONN_CLOSE,
    MTK_NFC_TEST_PNFC,
    MTK_NFC_TEST_PNFC_RSP_PROC,
    MTK_NFC_TEST_PNFC_RSP,
    MTK_NFC_TEST_T3T_POLLING,    
    #endif   
  
    MTK_NFC_META_GET_SELIST_REQ,
    MTK_NFC_META_GET_SELIST_RSP,
    
    MTK_NFC_MESSAGE_TYPE_ENUM_END
} MTK_NFC_MESSAGE_TYPE;

/* ENUM OF COMPORT_TYPE */ 
typedef enum {
    CONN_TYPE_UART = 0,
    CONN_TYPE_I2C,
    CONN_TYPE_SPI,
    CONN_TYPE_ENUM_END
} CONN_TYPE;

/* ENUM OF TRANSMISSION_BITS_RATE */
typedef enum {
    TRANSMISSION_BR_106 = 0,
    TRANSMISSION_BR_212,
    TRANSMISSION_BR_424,
    TRANSMISSION_BR_ALL,
    TRANSMISSION_BITS_RATE_ENUM_END    
} TRANSMISSION_BITS_RATE;

/* ENUM OF SE_TYPE */
typedef enum {
    SE_OFF = 0,
    SE_CONTACTLESS,
    SE_HOST_ACCESS,
    SE_ALL,
    SE_CONNECT_ENUM_END    
} SE_CONNECT_TYPE;

/* ENUM OF SE_PBF */
typedef enum {
    DISABLE = 0,
    ENABLE,
    SE_STATUS_ENUM_END    
} SE_STATUS;

typedef enum {
    UICC = 0,
    EMBEDDED_SE,
    uSD_CARD,
    SE_TYPE_ENUM_END
} SE_TYPE;

/* ENUM OF DETECTION TYPE */
typedef enum {
    DT_READ_MODE = 0,
    DT_P2P_MODE,
    DT_CARD_MODE,
    DT_ENUM_END 
} DETECTION_TYPE;

/* ENUM OF TAG_LIFECYCLE */
typedef enum {
    TAG_LC_INITIALIZE = 0,
    TAG_LC_READWRITE,
    TAG_LC_READONLY,
    TAG_LC_INVAILD,
    TAG_LC_ENUM_END,
} TAG_LIFECYCLE;

/* ENUM OF NDEF_FORMAT */
typedef enum {
    TAG_NDEF_FORMAT = 0,
    TAG_NONNDEF_FORMAT,
    TAG_NDEF_ENUM_END,
} e_TAG_NDEF_FORMAT;

/* ENUM OF TAG_INFOTYPE */
typedef enum {
    TAG_INFOTYPE1 = 0,
    TAG_INFOTYPE2,
    TAG_INFOTYPE3,
    TAG_INFOTYPE4,  // 4A
    TAG_INFOTYPE4B, // 4B
    TAG_INFOTYPEV,
    TAG_INFOTYPEK,
    TAG_INFOTYPEBP,
    TAG_INFOTYPE_MIFARE_CLASSIC,
    TAG_INFOTYPE_UNKNOWN,
    TAG_INFO_ENUM_END
} TAG_INFOTYPE;

/* ENUM OF P2P_CONNECT_TYPE */
typedef enum{
    P2P_CONNECT_TYPE_DEFAULT = 0,
    P2P_CONNECT_TYPE_CONNECTLESS,
    P2P_CONNECT_TYPE_CONNECTORIENT
}P2P_CONNECT_TYPE;

/* ENUM OF P2P_MODE */
typedef enum{
    P2P_MODE_ACTIVE = 0,
    P2P_MODE_PASSIVE,
    P2P_MODE_ENUM_END
}P2P_MODE;

/* Tag TYPE */
typedef enum mtk_nfc_tag_type{
    nfc_tag_DEFAULT    = 0,
    nfc_tag_MIFARE_UL  = 1,
    nfc_tag_MIFARE_STD = 2,
    nfc_tag_ISO1443_4A = 3,
    nfc_tag_ISO1443_4B = 4,
    nfc_tag_JEWEL      = 5,
    nfc_tag_NFC        = 6, //P2P mode
    nfc_tag_FELICA     = 7,
    nfc_tag_ISO15693   = 8,
    nfc_NDEF           = 9,
    nfc_raw_cmd        = 10
} e_mtk_nfc_tag_type;

typedef enum mtk_nfc_p2p_role {
    nfc_p2p_Initiator   = 0x00U,
    nfc_p2p_Target      = 0x01U,           
    nfc_p2p_All         = 0x02U
} e_mtk_nfc_p2p_role;

typedef enum mtk_nfc_p2p_type {
    nfc_p2p_DefaultP2PMode  = 0x00U,
    nfc_p2p_Passive106      = 0x01U, // A          
    nfc_p2p_Passive212      = 0x02U, // F            
    nfc_p2p_Passive424      = 0x04U, // F          
    nfc_p2p_Active          = 0x08U,                 
    nfc_p2p_P2P_ALL         = 0x0FU,                
    nfc_p2p_InvalidP2PMode  = 0xFFU         
} e_mtk_nfc_p2p_type; 

typedef enum mtk_nfc_ndef_lang_type {
    nfc_ndef_lang_DEFAULT = 0,
    nfc_ndef_lang_DE = 1,
    nfc_ndef_lang_EN = 2,
    nfc_ndef_lang_FR = 3
} e_mtk_nfc_ndef_lang_type;

typedef enum mtk_nfc_ndef_type {
    nfc_ndef_type_uri = 0,
    nfc_ndef_type_text,
    nfc_ndef_type_sp,
    nfc_ndef_type_others,
} e_mtk_nfc_ndef_type;

#define TAG_UID_MAX_LEN (10)

#define MTK_NFC_MAXNO_OF_SE (3)

/* BITMAP OF NFC_TECHNOLOGY */
#define TYPE_A  (1 << 0) 
#define TYPE_B  (1 << 1)
#define TYPE_V  (1 << 2)
#define TYPE_F  (1 << 3) 
#define TYPE_K  (1 << 4)
#define TYPE_BP (1 << 5)

/* BITMAP OF NFC_PROTOCOL */
#define PROTOCOL_ISO14443A  (1 << 0) 
#define PROTOCOL_ISO14443B  (1 << 1)
#define PROTOCOL_ISO15693   (1 << 2)
#define PROTOCOL_FELICA212  (1 << 3) 
#define PROTOCOL_FELICA424  (1 << 4) 
#define PROTOCOL_JEWEL      (1 << 5) 
#define PROTOCOL_KOVIO      (1 << 6) 
#define PROTOCOL_BP         (1 << 7) 

/* BITMAP OF SECURE ELEMENT */
#define SE_SIM1  (1 << 0) 
#define SE_SIM2  (1 << 1)
#define SE_SIM3  (1 << 2)

/* BITMAP OF P2P_ROLE */
#define P2P_ROLE_INITIATOR (1<<0)
#define P2P_ROLE_TARGET    (1<<1)



#define TAG_WRITE_MAXDATA (512)



#define MIFARE4K_LEN        (16)
#define MIFARE1K_LEN        (4)
#define ISO15693_LEN        (4)

#define NDEF_DATA_LEN       (512)
#define MTK_DEMO_TOOL_SE_NUM (3)
#define MTK_NFC_MAX_SE_NUM   (3)

#ifdef SUPPORT_EM
/* BITMAP OF EM_ALS_READER_M_TYPE*/
#define EM_ALS_READER_M_TYPE_A        (1 << 0) 
#define EM_ALS_READER_M_TYPE_B        (1 << 1)
#define EM_ALS_READER_M_TYPE_F        (1 << 2)
#define EM_ALS_READER_M_TYPE_V        (1 << 3) 
#define EM_ALS_READER_M_TYPE_BPrime   (1 << 4) 
#define EM_ALS_READER_M_TYPE_KOVIO    (1 << 5) 

/* BITMAP OF EM_ALS_CARD_M_TYPE*/
#define EM_ALS_CARD_M_TYPE_A        (1 << 0) 
#define EM_ALS_CARD_M_TYPE_B        (1 << 1)
#define EM_ALS_CARD_M_TYPE_BPrime   (1 << 2)
#define EM_ALS_CARD_M_TYPE_F212     (1 << 3) 
#define EM_ALS_CARD_M_TYPE_F424     (1 << 4)

/* BITMAP OF EM_ALS_READER_M_SPDRATE*/
#define EM_ALS_READER_M_SPDRATE_106        (1 << 0) 
#define EM_ALS_READER_M_SPDRATE_212        (1 << 1)
#define EM_ALS_READER_M_SPDRATE_424        (1 << 2)
#define EM_ALS_READER_M_SPDRATE_848        (1 << 3) 
#define EM_ALS_READER_M_SPDRATE_662        (1 << 4) 
#define EM_ALS_READER_M_SPDRATE_2648       (1 << 5) 

/* BITMAP OF EM_ALS_CARD_M_SW_NUM */
#define EM_ALS_CARD_M_SW_NUM_SWIO1        (1 << 0) 
#define EM_ALS_CARD_M_SW_NUM_SWIO2        (1 << 1)
#define EM_ALS_CARD_M_SW_NUM_SWIOSE       (1 << 2)

/* BITMAP OF EM_ENABLE_FUNC */
#define EM_ENABLE_FUNC_READER_MODE        (1 << 0) 
#define EM_ENABLE_FUNC_CARD_MODE          (1 << 1)
#define EM_ENABLE_FUNC_P2P_MODE           (1 << 2)

/* BITMAP OF EM_P2P_ROLE */
#define EM_P2P_ROLE_INITIATOR_MODE        (1 << 0) 
#define EM_P2P_ROLE_TARGET_MODE           (1 << 1)

/* BITMAP OF EM_P2P_MODE */
#define EM_P2P_MODE_PASSIVE_MODE          (1 << 0) 
#define EM_P2P_MODE_ACTIVE_MODE           (1 << 1)

/* BITMAP OF P2P CONNECTION */
#define MTK_NFC_P2P_CONNECTION_DEFAULT          (0)
#define MTK_NFC_P2P_CONNECTION_ACTIVE           (1)
#define MTK_NFC_P2P_CONNECTION_CREATE_SERVICE   (2)
#define MTK_NFC_P2P_CONNECTION_CREATE_CLIENT    (3)
#define MTK_NFC_P2P_CONNECTION_LISTENED         (4)
#define MTK_NFC_P2P_CONNECTION_ACCEPTED         (5)
#define MTK_NFC_P2P_CONNECTION_CONNECTED        (6)
#define MTK_NFC_P2P_CONNECTION_SEND             (7)
#define MTK_NFC_P2P_CONNECTION_RECEIVE          (8)
#define MTK_NFC_P2P_CONNECTION_CLOSED           (9)

#define TLV_ARRAY_MAX_ENTRY     32
#define TLV_ARRAY_MAX_DATALEN   32
#define DISCOVERY_DEV_MAX_NUM   15

#define TAG_TLV_MAX_NUM_OF_EVT_NTF   2
#define TAG_TLV_ARRAY_MAX_DATALEN  255

#define MTK_NFC_SE_SEND_DATA_MAX_LEN  (512)

/* ENUM OF EM_ACTION */
typedef enum {
   NFC_EM_ACT_START = 0,
   NFC_EM_ACT_STOP,
   NFC_EM_ACT_RUNINGB,   
} EM_ACTION;

/* ENUM OF EM_OPT_ACTION */
typedef enum {
   NFC_EM_OPT_ACT_READ = 0,
   NFC_EM_OPT_ACT_WRITE,
   NFC_EM_OPT_ACT_FORMAT, 
   NFC_EM_OPT_ACT_WRITE_RAW
} EM_OPT_ACTION;
#endif

typedef enum  MTK_NFC_LINK_STATUS
{
   MTK_NFC_LLCP_LINK_DEFAULT,
   MTK_NFC_LLCP_LINK_ACTIVATE,
   MTK_NFC_LLCP_LINK_DEACTIVATE,
   MTK_NFC_LLCP_LINK_INVALIDVER
}MTK_NFC_LINK_STATUS;


/* ######################################################################### */
#pragma pack(1)
/* ######################################################################### */

typedef struct mtkNfc_rTlv {
    UINT8 type;
    UINT8 length;
    UINT8 value[TLV_ARRAY_MAX_DATALEN];
} mtkNfc_rTlv_t;

typedef struct mtkNfc_rTagTlv {
    UINT8 tag;
    UINT8 len;
    UINT8 value[TAG_TLV_ARRAY_MAX_DATALEN];
} mtkNfc_rTagTlv_t;

/* NFC Main structure */
typedef struct mtk_nfc_main_msg_struct {
    UINT32 msg_type;   /* message identifier */
    UINT32 msg_length; /* length of 'data' */
} s_mtk_nfc_main_msg;

/* NFC_CONN_TYPE_UART */
typedef struct nfc_conn_type_uart {
    UINT32 comport;
    UINT32 buardrate;
} s_nfc_conn_type_uart;

/* NFC_CONN_TYPE_I2C */
typedef struct nfc_conn_type_i2c {
    UINT32 comport;
} s_nfc_conn_type_i2c;

/* NFC_CONN_TYPE_SPI */
typedef struct nfc_conn_type_spi {
    UINT32  comport;
} s_nfc_conn_type_spi;

typedef union {
    s_nfc_conn_type_uart type_uart;
    s_nfc_conn_type_i2c type_i2c;
    s_nfc_conn_type_spi type_spi;
} connid_u; 

/* Connect structure */
typedef struct mtk_nfc_dev_conn_req {
    UINT32 isconnect;       /* 1 : do connect, 0: do disconnect*/
    UINT32 conntype;        /*Please refer enum of CONN_TYPE*/     
    connid_u connid;
    UINT32 dumpdata;        /* 1: do dump debug data, 0 : don't dump debug data*/
    UINT32 forceDNL;        /* 1: do force download fw, 0 : don't force download fw*/    
} s_mtk_nfc_dev_conn_req;

typedef struct mtk_nfc_dev_conn_rsp {
    UINT32 result;          /* return connect result, 0: SUCCESS, 1 : FAIL*/
    UINT32 status;          /* return connect status, 1: connect, 0 : disconnect*/
    UINT32 isdumpdata;      /* 1: enable debug data dump function, 0: disable*/
    UINT32 isforceDNL;      /* 1: enable force download fw, 0: disable*/
    UINT32 sw_ver;          /* return software version*/ 
    UINT32 chipID;          /* return chip id*/
    UINT32 fw_ver;          /* return firmware version*/
    UINT32 tech_for_reader; /* support techonlogy for reader mode, please refer enum of NFC_TECHNOLOGY*/
    UINT32 tech_for_card;   /* support techonlogy for card mode, please refer enum of NFC_TECHNOLOGY*/
    UINT32 tech_for_p2p;    /* support techonlogy for p2p mode, please refer enum of NFC_TECHNOLOGY*/
} s_mtk_nfc_dev_conn_rsp;

typedef struct mtk_nfc_tag_listening {
    UINT32 status;          /* 1 : enable tag listing function, 0: disable */
    UINT32 tech_for_reader; /* bitmaps, please refer NFC_TECHNOLOGY related define */
    UINT8  u1BitRateA;      // bit-0:106, bit-1:212, bit-2:424, bit-3:848,     
    UINT8  u1BitRateB;      // bit-0:106, bit-1:212, bit-2:424, bit-3:848,     
    UINT8  u1BitRateF;      // bit-0:106, bit-1:212, bit-2:424, bit-3:848,     
    UINT8  u1BitRateV;      // 1 :  6.62 Kbits/s + subcarrier
                            // 2 :  6.62 Kbits/s + dual-subcarrier
                            // 3 : 26.48 Kbits/s + subcarrier
                            // 4 : 26.48 Kbits/s + dual-subcarrier
    UINT8  u1CodingSelect;  // 1 : coding selection 1/4
                            // 2 : coding selection 1/256
    UINT32 checkndef;       /* 1: enable check NDEF function, 0 disable */
    UINT32 autopolling;     /* 1: enable auto polling tag remove function, 0 disable */
    UINT32 supportmultupletag;  /*1: enable support multuple tag function, 0 disable */  
} s_mtk_nfc_tag_listening;

typedef struct mtk_nfc_tag_speed_designate {
    UINT32 ISO_14443A;      /* please refer ENUM OF TRANSMISSION_BITS_RATE*/
    UINT32 ISO_14443B;      /* please refer ENUM OF TRANSMISSION_BITS_RATE*/
    UINT32 ISO_15693;       /* 0 for 6Kbits/s, 1 : for 26Kbits/s, 3 : for no limit*/
} s_mtk_nfc_tag_speed_designate;

typedef struct mtk_nfc_p2p{
    UINT32 status;          /* 1 : enable P2P function, 0: disable*/
    UINT32 role;            /* 0: initiator, 1: target, 2: all  */
    UINT32 bm_mode;         /* device mode, please refer BITMAP of P2P_MODE*/
}s_mtk_nfc_p2p;

/* Securacy element */
typedef struct mtk_nfc_tool_se_info {
    UINT32            seid;
    SE_STATUS         status;        /* 1: enable, 0 : disable*/
    SE_TYPE           type; 
    SE_CONNECT_TYPE   connecttype;
    SE_STATUS         lowpowermode;  /* 1: enable low power mode, 0 : disable*/
    SE_STATUS         pbf;           /* Each SE current status of Power by field*/     
} s_mtk_nfc_tool_se_info ;

typedef struct s_mtk_nfc_tool_set_se {
    UINT32 status;          /* 1 : enable se function, 0: disable*/
    UINT32 senum;           /* total number of se */
    s_mtk_nfc_tool_se_info SE[MTK_DEMO_TOOL_SE_NUM];
    UINT32 notifyapp;       /* 1: enable , 0 : disable*/
    UINT32 clf;             /* 1: enable , 0 : disable*/    
}s_mtk_nfc_tool_set_se;

typedef struct mtk_nfc_get_selist_req{
    UINT32 status;          /* 1: enable se function, 0 : disable se function*/
}s_mtk_nfc_get_selist_req;

typedef struct mtk_nfc_get_selist_rsp {
    UINT32 result;          /* return get se result, 0: SUCCESS, 1 : FAIL*/
    UINT32 status;          /* return se status, 0: have not SE, 1 : have SE*/
    UINT32 senum;           /* total number of se */
    s_mtk_nfc_tool_set_se sedata; 
} s_mtk_nfc_get_selist_rsp;

typedef struct mtk_nfc_jni_se_set_mode_req{
    UINT8    seid;
    UINT32   enable;
    VOID     *pContext;          
}s_mtk_nfc_jni_se_set_mode_req_t;

typedef struct mtk_nfc_jni_se_set_mode_rsp{
    UINT8    sehandle;
    UINT32   status;        
}s_mtk_nfc_jni_se_set_mode_rsp_t;

typedef struct mtk_nfc_jni_se_get_cur_se_rsp{
    UINT8    sehandle;
    UINT32   status;        
}s_mtk_nfc_jni_se_get_cur_se_rsp_t;

typedef struct s_mtk_nfc_jni_se_open_conn_req{
    UINT8    seid;
    VOID     *pContext;          
}s_mtk_nfc_jni_se_open_conn_req_t;

typedef struct s_mtk_nfc_jni_se_open_conn_rsp{
    UINT32   status;        
    UINT8    conn_id;
}s_mtk_nfc_jni_se_open_conn_rsp_t;

typedef struct s_mtk_nfc_jni_se_close_conn_req{
    UINT8    conn_id;
    VOID     *pContext;          
}s_mtk_nfc_jni_se_close_conn_req_t;

typedef struct s_mtk_nfc_jni_se_close_conn_rsp{
    UINT32   status;        
}s_mtk_nfc_jni_se_close_conn_rsp_t;

typedef struct s_mtk_nfc_jni_se_send_data_req{
    UINT8     conn_id;
    UINT32    length;
    UINT8     data[MTK_NFC_SE_SEND_DATA_MAX_LEN];
    VOID     *pContext;          
}s_mtk_nfc_jni_se_send_data_req_t;

typedef struct s_mtk_nfc_jni_se_send_data_rsp{
    UINT32    status;        
    UINT32    length;
    UINT8     data[MTK_NFC_SE_SEND_DATA_MAX_LEN];
    VOID     *pContext;
}s_mtk_nfc_jni_se_send_data_rsp_t;

typedef struct mtk_nfc_jni_se_info {
    UINT8             seid;
    SE_STATUS         status;        /* 1: enable, 0 : disable*/
    SE_TYPE           type;      
} s_mtk_nfc_jni_se_info_t ;

typedef struct mtk_nfc_jni_se_get_list_req{
    UINT8    *pse_count;
    UINT8    *pse_list;         
}s_mtk_nfc_jni_se_get_list_req_t;

typedef struct mtk_nfc_jni_se_get_list_rsp{
    UINT32   status;          
    UINT8                     SeCount; 
    s_mtk_nfc_tool_se_info    SeInfor[MTK_NFC_MAX_SE_NUM];
}s_mtk_nfc_jni_se_get_list_rsp_t;


/* Discovery structure */
typedef struct mtk_nfc_discovery_req {
    UINT32 action;                      /* discovery action, 1: start discovery, 0 : stop discovery*/
    s_mtk_nfc_tag_listening tag_setting;  /* Tag related setting, please refer structure of "s_mtk_tag_listening"*/ 
    s_mtk_nfc_tool_set_se se_setting;   /* Secure element related setting, please refer structure of "s_mtk_secureEle"*/ 
    s_mtk_nfc_p2p         p2p_setting;  /* p2p related setting, please refer structure of "s_mtk_p2p"*/    
} s_mtk_nfc_discovery_req;

typedef struct mtk_nfc_discovery_rsp {
    UINT32 result;          /* return discovery result, 0: SUCCESS, 1 : FAIL*/	
    UINT32 status;          /* return discovery status, 1: start discovery, 0 : stop discovery*/
} s_mtk_nfc_discovery_rsp;

/* TAG TYPE 1 INFORMATION Setucture */
typedef struct cardinfo_type1 {
    UINT32 cardtype;
    UINT8  uid_length;
    UINT8  uid[TAG_UID_MAX_LEN];
    UINT8  sak;
    UINT8  atqa[2];
} s_cardinfo_type1;

/* TAG TYPE 2 INFORMATION Setucture */
typedef struct cardinfo_type2 {
    UINT32 cardtype;
    UINT8  uid_length;
    UINT8  uid[TAG_UID_MAX_LEN];
    UINT8  sak;
    UINT8  atqa[2];
} s_cardinfo_type2;

/* TAG TYPE 3 INFORMATION Setucture */
typedef struct cardinfo_type3 {
    UINT32 cardtype;
    UINT8  idm_len;
    UINT8  idm[8];
    UINT8  pm[8];
    UINT8  systemcode[2];
    UINT8  bitrate;    
} s_cardinfo_type3;

/* TAG TYPE 4A INFORMATION Setucture */
typedef struct cardinfo_type4a {
    UINT32 cardtype;
    UINT8  uid_length;
    UINT8  uid[TAG_UID_MAX_LEN];
    UINT8  sak;
    UINT8  atqa[2];
} s_cardinfo_type4a;

/* TAG TYPE 4B INFORMATION Setucture */
typedef struct cardinfo_type4b {
    UINT32 cardtype;
    UINT8  pupi[4];
    UINT8  appdata[4];
    UINT8  protoinfo[3];
    UINT8  afi;
} s_cardinfo_type4b;

/* TAG TYPE V INFORMATION Setucture */
typedef struct cardinfo_typev {
    UINT32 cardtype;
    UINT8  uid_length;
    UINT8  uid[TAG_UID_MAX_LEN];
    UINT8  uidrawdata[TAG_UID_MAX_LEN];
    UINT8  dsfid;
    UINT8  flags;
    UINT8  afi;
} s_cardinfo_typev;

/* TAG TYPE K INFORMATION Setucture */
typedef struct cardinfo_typek {
    UINT32 cardtype;
    UINT8  manufid;
    UINT8  datafmt;
    UINT8  cc[4];
    UINT8  data[32];
} s_cardinfo_typek;

/* TAG TYPE BP INFORMATION Setucture */
typedef struct cardinfo_typebp {
    UINT32 cardtype;
    UINT8  uid_length;    
    UINT8  uid[TAG_UID_MAX_LEN];
    UINT8  data_length;
    UINT8  data[256];
} s_cardinfo_typebp;

/* TAG TYPE UNKNOW INFORMATION Setucture */
typedef struct cardinfo_typeunknow {
    UINT8  uid_length;    
    UINT8  uid[TAG_UID_MAX_LEN];
} s_cardinfo_typeunknow;


typedef union cardinfo {
    s_cardinfo_type1  type1;
    s_cardinfo_type2  type2;
    s_cardinfo_type3  type3;
    s_cardinfo_type4a type4a; 
    s_cardinfo_type4b type4b; 
    s_cardinfo_typev  typev;
    s_cardinfo_typek  typek;
    s_cardinfo_typebp typebp;
    s_cardinfo_typeunknow typeunknow;
}u_cardinfo;

/* TAG INFO structure*/
typedef struct mtk_nfc_tag_info {
    UINT32 lifecycle;       /* please refer ENUM OF TAG_LIFECYCLE*/
    UINT32 isndef;          /*please refer ENUM OF TAG_NDEF_FORMAT*/
    UINT32 bmprotocol;      /*please refer BITMAP OF NFC_PROTOCOL*/
    UINT32 cardinfotype;    /* please refer ENUM OF TAG_INFOTYPE*/     
    u_cardinfo cardinfo;   
} s_mtk_nfc_tag_info;

/* P2P INFO Setucture*/
typedef struct mtk_nfc_p2p_info {
    UINT32 p2p_role;        /* please refer BITMAP OF P2P_ROLE*/
    UINT32 p2p_mode;        /* please refer ENUM OF P2P_MODE*/
    UINT32 p2p_speed;       /* please refer ENUM OF TRANSMISSION_BITS_RATE*/
} s_mtk_nfc_p2p_info;

typedef union {
    s_mtk_nfc_tag_info taginfo; /* TAG Information*/
    s_mtk_nfc_p2p_info p2pinfo; /* P2P Information*/
    //s_mtk_nfc_se_info  seinfo /* SE Information*/
} u_discovery_type;

/* Discovery detect response */
typedef struct mtk_nfc_discovery_detect_rsp {
    UINT32 detectiontype;   /* please refer ENUM OF DETECTION TYPE*/   
    u_discovery_type discovery_type;
}s_mtk_nfc_discovery_detect_rsp;

#if 1
/* NFC Tag Read Request */
//nfc_tag_read_request
typedef struct mtk_nfc_tag_read_request {
   e_mtk_nfc_tag_type read_type;    /* which type want to read*/
   UINT32 sector;                   /* for Mifare STD used*/
   UINT32 block;                    /* for Mifare STD/MifareUL/ISO15693 used*/
   UINT8  AuthentificationKey;      /* for Mifare STD, KEY_A:0 , KEY_B:1 */
} s_mtk_nfc_tag_read_req;

typedef struct mtk_nfc_tag_read_Mifare {
   UINT32 Length;
   UINT8  data[MIFARE4K_LEN];    
} s_mtk_nfc_tag_read_Mifare;

typedef struct mtk_nfc_tag_read_ndef {
   e_mtk_nfc_ndef_type ndef_type;
   UINT8  lang[3];
   UINT8  recordFlags;
   UINT8  recordId[32];
   UINT8  recordTnf;
   UINT32 length;      
   UINT8  data[NDEF_DATA_LEN];
} s_mtk_nfc_tag_read_ndef;

typedef struct mtk_nfc_tag_read_ISO15693 {
   UINT32 Length;
   UINT8  data[ISO15693_LEN];
} s_mtk_nfc_tag_read_ISO15693;

typedef struct mtk_nfc_tag_write_rawdata {
    UINT32 length;
    UINT8  data[TAG_WRITE_MAXDATA]; //TAG_WRITE_MAXDATA = 512  
} s_mtk_nfc_tag_write_rawdata,s_mtk_nfc_tag_read_rawdata;

typedef union mtk_nfc_tag_read_result_response_u {
    s_mtk_nfc_tag_read_Mifare   nfc_tag_read_Mifare_resp;
    s_mtk_nfc_tag_read_ndef     nfc_tag_read_ndef_resp;
    s_mtk_nfc_tag_read_ISO15693 nfc_tag_read_ISO15693_resp;
    s_mtk_nfc_tag_read_rawdata  nfc_tag_read_raw_data;
}mtk_nfc_tag_read_result_response_u;

/* NFC Tag Read Response */
//nfc_tag_read_response
typedef struct mtk_nfc_tag_read_response {
    UINT32 status;              /*  return read status, 0 success*/
    e_mtk_nfc_tag_type type;    /*  Check nfc_tag_type */
    mtk_nfc_tag_read_result_response_u nfc_tag_read_result;
} s_mtk_nfc_tag_read_rsp;

/*NFC Tag Write Request */
typedef struct mtk_nfc_tag_write_typeMifare {
    UINT32 sector;              /* Mifare STD */
    UINT32 block;               /* Mifare STD, Mifare UL */
    UINT32 length;              /* Mifare STD, Mifare UL */
    UINT8  data[MIFARE4K_LEN];  /* Mifare STD, Mifare UL */
    UINT8  AuthentificationKey; /* Mifare STD, Mifare UL,KEY_A:0 , KEY_B:1 */
} s_mtk_nfc_tag_write_typeMifare;

typedef struct mtk_nfc_tag_write_typeISO15693 {
    UINT32 block;
    UINT32 length;
    UINT8  data[ISO15693_LEN];
} s_mtk_nfc_tag_write_typeISO15693;

typedef struct Vcard {
    CHAR   Name[64];
    CHAR   Compagny[64];
    CHAR   Titlep[64];
    CHAR   Tel[32];
    CHAR   Email[64];
    CHAR   Adress[128];
    CHAR   PostalCode[32];
    CHAR   City[64];
    CHAR   CompagnyUrl[64];
} Vcard_t;

typedef struct SmartPoster {
    UINT8  Compagny[64];
    UINT16 CompagnyLength;
    UINT8  CompagnyUrl[64];
    UINT16 CompagnyUrlLength;
} SmartPoster_t;

typedef struct Text {
    UINT8  data[TAG_WRITE_MAXDATA];
    //UINT8 data[128];
    UINT16 DataLength;
} Text_t;

typedef struct URL {
    //ndef_url_type URLtype;
    UINT8  URLData[64];
    UINT16 URLLength;
} URL_t;

typedef struct EXTTag {
    CHAR   EXTTagType[64];
    CHAR   EXTData[TAG_WRITE_MAXDATA];
    UINT16 EXTLength;
} EXTTag_t;

typedef union mtk_nfc_tag_write_ndef_data {
    SmartPoster_t  SP_Data;
    Vcard_t        VC_Data;
    Text_t         TX_Data;
    URL_t          URL_Data;
    EXTTag_t       EXT_Data;
} s_mtk_nfc_tag_write_ndef_data;

typedef struct mtk_nfc_tag_write_ndef {
    e_mtk_nfc_ndef_type             ndef_type;
    e_mtk_nfc_ndef_lang_type        language;
    UINT32                          length;
    s_mtk_nfc_tag_write_ndef_data   ndef_data;
} s_mtk_nfc_tag_write_ndef;

typedef union mtk_nfc_tag_write_data_request_u {
    s_mtk_nfc_tag_write_typeMifare   nfc_tag_write_typeMifare_data;
    s_mtk_nfc_tag_write_typeISO15693 nfc_tag_write_typeISO15693_data;
    s_mtk_nfc_tag_write_ndef         nfc_tag_write_ndef_data;
    s_mtk_nfc_tag_write_rawdata      nfc_tag_write_raw_data;
} mtk_nfc_tag_write_data_request_u;

//nfc_tag_write_request
typedef struct mtk_nfc_tag_write_request {
    e_mtk_nfc_tag_type               write_type; /*  which type want to write*/
    mtk_nfc_tag_write_data_request_u nfc_tag_write_data;
} s_mtk_nfc_tag_write_req;

/*NFC Tag Write Response*/
//nfc_tag_write_response
typedef struct mtk_nfc_tag_write_response {
    e_mtk_nfc_tag_type  type;   /*  return writed type*/
    UINT32              status; /*  return read status, 0 success*/               
} s_mtk_nfc_tag_write_rsp;
#endif

typedef union sapvalue {
    UINT32 value;
    UINT8  string[64]; /*TBD*/
} sapvalue_u; 

/* p2p service start */
typedef struct mtk_nfc_p2p_service_start_req {
    P2P_CONNECT_TYPE connecttype; /* connect method, please refer ENUM OF P2P_CONNECT_TYPE*/
    UINT32 sapsettype;  /*0 is value type , 1 is string type */
    sapvalue_u sap;
    UINT32 windoesize; /**/
    UINT32 datasize;
} s_mtk_nfc_p2p_service_start_req;

/* p2p service connect */
typedef struct mtk_nfc_p2p_service_conn_req {
    UINT32 sapsettype; /*0 is value type , 1 is string type */    
    sapvalue_u sap;
    s_mtk_nfc_tag_write_ndef  ndef_data;    
}s_mtk_nfc_p2p_service_conn_req;

typedef struct mtk_nfc_p2p_service_conn_mw_req {
    P2P_CONNECT_TYPE connecttype; /* connect method, please refer ENUM OF P2P_CONNECT_TYPE*/    
    UINT32 sapsettype; /*0 is value type , 1 is string type */    
    sapvalue_u sap;
    UINT32 windoesize; /**/
    UINT32 datasize;    
    UINT32  datalength;    
    UINT8   data[512];
}mtk_nfc_p2p_service_conn_mw_req;

/* p2p service disconnect */
typedef struct mtk_nfc_p2p_service_disconn_req {
    UINT32 action; /* 1: disconnection */
} s_mtk_nfc_p2p_service_disconn_req;

/* p2p command resp, read data */
typedef struct mtk_nfc_p2p_rsp {
    UINT32 command;     /* Read = 0, Write =1, Connect = 2,Start = 3,disconn = 4*/
    UINT32 result;      /* return p2p_setting result, 0: SUCCESS, 1 : FAIL*/
    s_mtk_nfc_tag_read_ndef nfc_tag_read_ndef_resp;
} s_mtk_nfc_p2p_setting_rsp;

/* SE Notify */
typedef struct mtk_nfc_SE_notify {
    UINT32 SEId;
    UINT32 trigger;
    UINT32 notifylen;
    UINT8  message[256];
} s_mtk_nfc_SE_notify;

/* Multiple Tag Notify */
typedef struct mtk_nfc_MultipleTag_notify {
    UINT32 tagnumber;       /* tag detected number, MAX support 10 Tags*/
    UINT32 uidlength[10];
    UINT8  uid[10][10];
} s_mtk_nfc_MultipleTag_notify;

/* Multiple Tag Select */
typedef struct mtk_nfc_MultipleTag_select_req {
    UINT32 uidlength;
    UINT8  data[10];
} s_mtk_nfc_MultipleTag_select_req;

typedef struct mtk_nfc_MultipleTag_select_rsp {
    UINT32 result;          /* return result, 0: SUCCESS, 1 : FAIL*/
    UINT32 status;          /* return status, 1: Success, 0 : fail, not support*/
} s_mtk_nfc_MultipleTag_select_rsp;

typedef struct mtk_nfc_general_rsp {
    UINT32 result;          /* return result, 0: SUCCESS, 1 : FAIL*/
    UINT32 status;          /* return status, 1: Success, 0 : fail, not support*/
} s_mtk_nfc_general_rsp;

typedef struct mtk_nfc_stop_rsp {
    UINT32 tupe;            /* TBD, now always set 0*/
    UINT32 action;          /* TBD, now always set 0*/
} s_mtk_nfc_stop_rsp;

/* Packet-Error-Rate structure */
typedef struct mtk_nfc_per_req {
    UINT32 action;          /* 1 : start PER, 2: stop PER */
} s_mtk_nfc_per_req;

typedef struct mtk_nfc_per_rsp {
    UINT32 result;          /* return result, 0: SUCCESS, 1 : FAIL */    
    UINT32 pass_cnt;
    UINT32 fail_cnt;
} s_mtk_nfc_per_rsp;

/* Signal-to-Noise Ratio structure */
typedef struct mtk_nfc_snr_req {
    UINT32 function;        /* 1 : Get AGC gain , 2: Use specific Gain */
    UINT32 pga_gain;        /* Note: This field MUST set to 0 when use AGC */
} s_mtk_nfc_snr_req;

typedef struct mtk_nfc_snr_rsp {
    UINT32 result;          /* return result, 0: SUCCESS, 1 : FAIL */    
    UINT32 tx_power;
    UINT32 signal_avg_power;
    UINT32 signal_peak_power;
    UINT32 noise_avg_power;
    UINT32 noise_peak_power;
    UINT32 pga_gain;
} s_mtk_nfc_snr_rsp;


/***************************************************************************** 
 * EM
 *****************************************************************************/ 
#ifdef SUPPORT_EM
/* --------------------------------------------------------*/
/* OTHER_FUNCTIONS                                         */
/* --------------------------------------------------------*/
typedef struct mtk_nfc_em_pnfc_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
    UINT32 datalen;
    UINT8  data[256];
} s_mtk_nfc_em_pnfc_req;

typedef struct mtk_nfc_em_pnfc_rsp {
    UINT32 result;          /* 0:Success,1:Fail*/
} s_mtk_nfc_em_pnfc_rsp;

typedef struct mtk_nfc_em_pnfc_new_rsp {
    UINT32 result;          /* 0:Success,1:Fail*/
    UINT32 datalen;
    UINT8  data[256];
} s_mtk_nfc_em_pnfc_new_rsp;


typedef struct mtk_nfc_pnfc_req {
    UINT32 u4ReqMsg;        /* MTK_NFC_MESSAGE_TYPE*/
    UINT32 u4action;
    s_mtk_nfc_em_pnfc_req rEmPnfcReq;
} s_mtk_nfc_pnfc_req;

typedef struct mtk_nfc_pnfc_rsq {
    UINT32 u4ReqMsg;
    UINT32 u4action;
    UINT32 u4result;
} s_mtk_nfc_pnfc_rsq;

typedef struct mtk_nfc_em_tx_carr_als_on_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
} s_mtk_nfc_em_tx_carr_als_on_req;

typedef struct mtk_nfc_em_tx_carr_als_on_rsp {
    UINT32 result;          /* 0:Success,1:Fail*/
}s_mtk_nfc_em_tx_carr_als_on_rsp;

/* --------------------------------------------------------*/
/* P2P_MODE_RELATED                                        */
/* --------------------------------------------------------*/
typedef struct mtk_nfc_em_als_p2p_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
    UINT32 supporttype;     /* supporttype, please refer BITMAP of EM_ALS_READER_M_TYPE*/
    UINT32 typeA_datarate;  /* TypeA,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    UINT32 typeF_datarate;  /* TypeV,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    UINT32 mode;            /* BITMAPS bit0: Passive mode, bit1: Active mode, please refer BITMAP of EM_P2P_MODE*/
    UINT32 role;            /* BITMAPS bit0: Initator, bit1: Target, please refer BITMAP of EM_P2P_ROLE*/
    UINT32 isDisableCardM;  /* 0: , 1: disable card mode*/
} s_mtk_nfc_em_als_p2p_req;

typedef struct mtk_nfc_em_als_p2p_ntf {
    INT32  link_status;     /* 1:llcp link is up,0:llcp link is down*/
    //UINT32  datalen;
    //UINT8 data[256];
} s_mtk_nfc_em_als_p2p_ntf;

typedef struct mtk_nfc_em_als_p2p_rsp {
    INT32  result;          /* 0:Success,1:Fail*/
} s_mtk_nfc_em_als_p2p_rsp;

/* -------------------------------------------------------- */
/* CARD_MODE_RELATED                                        */
/* -------------------------------------------------------- */
typedef struct mtk_nfc_em_als_cardm_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
    UINT32 SWNum;           /* SWNum, please refer BITMAP of EM_ALS_CARD_M_SW_NUM*/
    UINT32 supporttype;     /* supporttype, please refer BITMAP of EM_ALS_READER_M_TYPE*/
    UINT32 fgvirtualcard;   /* 1:enable virtual card, 0:disable virtual card(default)   */
} s_mtk_nfc_em_als_cardm_req;

typedef struct mtk_nfc_em_als_cardm_rsp {
    INT32  result;          /*0:Success,1:Fail,0xE1:No SIM*/
} s_mtk_nfc_em_als_cardm_rsp;

typedef struct mtk_nfc_em_virtual_card_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
    UINT32 supporttype;     /* supporttype, please refer BITMAP of EM_ALS_READER_M_TYPE*/
    //UINT32 typeA_datarate;  /* TypeA,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    //UINT32 typeB_datarate;  /* TypeB,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    UINT32 typeF_datarate;  /* TypeF,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/ 
} s_mtk_nfc_em_virtual_card_req;

typedef struct mtk_nfc_em_virtual_card_rsp {
    UINT32 result;          /* 0:Success,1:Fail*/
} s_mtk_nfc_em_virtual_card_rsp;

/* -------------------------------------------------------- */
/* READER_MODE_RELATED                                      */
/* -------------------------------------------------------- */
typedef struct mtk_nfc_em_als_readerm_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
    UINT32 supporttype;     /* supporttype, please refer BITMAP of EM_ALS_READER_M_TYPE*/
    UINT32 typeA_datarate;  /* TypeA,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    UINT32 typeB_datarate;  /* TypeB,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    UINT32 typeV_datarate;  /* TypeV,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/
    UINT32 typeF_datarate;  /* TypeF,datarate, please refer BITMAP of EM_ALS_READER_M_SPDRATE*/    
    UINT32 typeV_subcarrier;/* 0: subcarrier, 1 :dual subcarrier*/
} s_mtk_nfc_em_als_readerm_req;

typedef struct mtk_nfc_em_als_readerm_ntf {
    INT32  result;          /* 0:Success,Tag connected, 1: Fail, 2:Tag disconnected*/
    UINT32 isNDEF;          /* 1:NDEF, 0: Non-NDEF*/
    UINT32 UidLen;
    UINT8  Uid[10];
} s_mtk_nfc_em_als_readerm_ntf;

typedef struct mtk_nfc_em_als_readerm_rsp {
    INT32  result;          /* 0:Success, 1: Fail,*/
} s_mtk_nfc_em_als_readerm_rsp;

typedef struct mtk_nfc_em_als_readerm_opt_req {
    INT32  action;          /* Action, please refer ENUM of EM_OPT_ACTION*/   
    s_mtk_nfc_tag_write_ndef ndef_write;
} s_mtk_nfc_em_als_readerm_opt_req;

typedef struct mtk_nfc_em_als_readerm_opt_rsp {
    INT32  result;          /* 0:Success,1:Fail*/
    s_mtk_nfc_tag_read_ndef ndef_read;
} s_mtk_nfc_em_als_readerm_opt_rsp;

/* -------------------------------------------------------- */
/* POLLING_MODE_RELATED                                     */
/* -------------------------------------------------------- */
typedef struct mtk_nfc_em_polling_req {
    UINT32 action;          /* Action, please refer ENUM of EM_ACTION*/
    UINT32 phase;           /* 0:Listen phase, 1:Pause phase*/
    UINT32 Period;
    UINT32 enablefunc;      /* enablefunc, please refer BITMAP of EM_ENABLE_FUNC*/
    s_mtk_nfc_em_als_p2p_req     p2pM;
    s_mtk_nfc_em_als_cardm_req   cardM;
    s_mtk_nfc_em_als_readerm_req readerM;
} s_mtk_nfc_em_polling_req;

typedef struct mtk_nfc_em_polling_rsp {
    INT32  result;          /* 0:Success,1:Fail*/
} s_mtk_nfc_em_polling_rsp;

typedef union {
    s_mtk_nfc_em_als_p2p_ntf     p2p;
    s_mtk_nfc_em_als_cardm_rsp   card;
    s_mtk_nfc_em_als_readerm_ntf reader;
} s_mtk_nfc_em_polling_func_ntf;

typedef struct mtk_nfc_em_polling_ntf {
    INT32 detecttype;       /* enablefunc, please refer ENUM of EM_ENABLE_FUNC*/
    s_mtk_nfc_em_polling_func_ntf ntf;
} s_mtk_nfc_em_polling_ntf;
#endif

/***************************************************************************** 
 * FACTORY MODE
 *****************************************************************************/ 
#ifdef SUPPORT_FM
typedef struct mtk_nfc_fm_swp_test_req { 
    INT32  action;
    INT32  SEmap; // bit 0 - SIM1 , bit 1 - SIM2, bit2 - uSD
} s_mtk_nfc_fm_swp_test_req;

typedef struct mtk_nfc_fm_swp_test_rsp {
    INT32  result;
} s_mtk_nfc_fm_swp_test_rsp;
#endif

/***************************************************************************** 
 * JNI INTERFACE
 *****************************************************************************/ 
#ifdef SUPPORT_JNI
typedef struct mtk_nfc_option_req {
    UINT32 isdumpdata;      /* 1: enable debug data dump function, 0: disable*/
    UINT32 isforceDNL;      /* 1: enable force download fw, 0: disable*/
} s_mtk_nfc_option_req;

typedef struct mtk_nfc_init_rsp {
    UINT32 result;          /* 0:Success, 1: Fail */
    UINT32 isdumpdata;      /* 1: enable debug data dump function, 0: disable*/
    UINT32 isforceDNL;      /* 1: enable force download fw, 0: disable*/
    UINT32 sw_ver;          /* return software version*/ 
    UINT32 hw_ver;          /* return hardware chip id*/
    UINT32 fw_ver;          /* return firmware version*/
    UINT32 tech_for_reader; /* support techonlogy for reader mode, please refer enum of NFC_TECHNOLOGY*/
    UINT32 tech_for_card;   /* support techonlogy for card mode, please refer enum of NFC_TECHNOLOGY*/
    UINT32 tech_for_p2p;    /* support techonlogy for p2p mode, please refer enum of NFC_TECHNOLOGY*/
} s_mtk_nfc_init_rsp;

typedef struct mtk_nfc_deinit_rsp {
    UINT32 result;          /* 0:Success, 1: Fail */
} s_mtk_nfc_deinit_rsp;

typedef struct mtk_nfc_service_set_config_req {
    UINT32 u4NumParam;
    mtkNfc_rTlv_t rParam[TLV_ARRAY_MAX_ENTRY];
} s_mtk_nfc_service_set_config_req_t;

typedef struct mtk_nfc_service_set_config_resp {
    UINT32 u4Result;
    UINT32 u4NumParam;
    UINT8  au1Data[TLV_ARRAY_MAX_ENTRY];
} s_mtk_nfc_service_set_config_resp_t;

typedef struct mtk_nfc_service_get_config_req {
    UINT32 u4NumParam;
    UINT8  au1IdArray[TLV_ARRAY_MAX_ENTRY];
} s_mtk_nfc_service_get_config_req_t;

typedef struct mtk_nfc_service_get_config_resp {
    UINT32 u4Result;
    UINT32 u4NumParam;
    mtkNfc_rTlv_t rParam[TLV_ARRAY_MAX_ENTRY];
} s_mtk_nfc_service_get_config_resp_t;

typedef struct mtk_nfc_reader_discover {
    UINT8  fgEnTypeA;
    UINT8  u1BitRateA; // bit-0:106, bit-1:212, bit-2:424, bit-3:848,     
    UINT8  fgEnTypeB;
    UINT8  u1BitRateB; // bit-0:106, bit-1:212, bit-2:424, bit-3:848,     
    UINT8  fgEnTypeF;
    UINT8  u1BitRateF; // bit-0:106, bit-1:212, bit-2:424, bit-3:848,     
    UINT8  fgEnTypeV;
    UINT8  fgEnDualSubCarrier; // if dual sub-carrier, set true    
    UINT8  u1BitRateV; // bit-0: 6.62; bit-1: 26.48    
    UINT8  fgEnTypeBP;
    UINT8  fgEnTypeK;
    UINT8  fgEnTypeJewel;
    UINT8  fgAutoPresenceChk;
} s_mtk_nfc_reader_discover_t;

typedef struct mtk_nfc_p2p_discover {
    UINT8  fgEnTypeA;
    UINT8  u1BitRateA; // bit-0:106, bit-1:212, bit-2:424, bit-3:848
    UINT8  fgEnTypeF;
    UINT8  u1BitRateF; // bit-0:106, bit-1:212, bit-2:424, bit-3:848
    UINT8  fgEnPassiveMode;
    UINT8  fgEnActiveMode;
    UINT8  fgEnInitiatorMode;
    UINT8  fgEnTargetMode;
    UINT8  fgDisableCardMode;
} s_mtk_nfc_p2p_discover_t;

typedef struct mtk_nfc_card_discover {
    UINT8  u1Swio; // bit-0: SWIO-1; bit-1:SWIO-2; bit-2: SWIO-3    
    UINT8  fgEnTypeA;
    UINT8  u1BitRateA;
    UINT8  fgEnTypeB;
    UINT8  u1BitRateB;
    UINT8  fgEnTypeBP;
    UINT8  u1BitRateBP;
    UINT8  fgEnTypeF;
    UINT8  u1BitRateF;
} s_mtk_nfc_card_discover_t;

//set discover
typedef struct mtk_nfc_service_set_discover_req {
    UINT16 u2TotalDuration; // LiangChi: ZERO, SY:NON-ZERO, Nina: 
    UINT8  fgEnListen;      // enable listen phase or not. If disable, run IDLE with total duration
    UINT8  u1OpMode;        // bit-0: reader, bit-1:card emulation, bit-2:p2p 
    UINT8  u1DtaTestMode;   // DTA Test Mode, 0: platform, 1: operation, 2: P2p, 0xFF: not DTA
    INT32  i4DtaPatternNum; // DTA Test Pattern Num from 0 ~ 15
    s_mtk_nfc_reader_discover_t reader_setting; /* Tag related setting, please refer structure of "s_mtk_tag_listing"*/ 
    s_mtk_nfc_card_discover_t   card_setting;   /* Secure element related setting, please refer structure of "s_mtk_secureEle"*/ 
    s_mtk_nfc_p2p_discover_t    p2p_setting;    /* p2p related setting, please refer structure of "s_mtk_p2p"*/    
    UINT8  au1ConfigPath[60]; // mingyen : config path for operation test
} s_mtk_nfc_service_set_discover_req_t;

typedef struct mtk_nfc_service_set_discover_resp {
    UINT32 u4Result;
} s_mtk_nfc_service_set_discover_resp_t;

#define NCI_NFCID1_MAX_LEN 10
typedef struct mtk_nfc_poll_a_tech_param {
    UINT8  au1SensRes[2];   /* SENS_RES Response (ATQA). */
    UINT8  u1Nfcid1Len;     /* 4, 7 or 10 */
    UINT8  au1Nfcid1[NCI_NFCID1_MAX_LEN]; /* AKA NFCID1 */
    UINT8  u1SelRsp;        /* SEL_RSP (SAK)*/
} s_mtk_nfc_poll_a_tech_param_t;

#define NFC_SENSB_RES_MAX_LEN 12
#define NFC_NFCID0_MAX_LEN 4
typedef struct mtk_nfc_poll_b_tech_param {
    UINT8  u1SensbResLen;   /* Length of SENSB_RES*/
    UINT8  au1SensbRes[NFC_SENSB_RES_MAX_LEN]; /* SENSB_RES Response (ATQ) */
    UINT8  au1Nfcid0[NFC_NFCID0_MAX_LEN];
} s_mtk_nfc_poll_b_tech_param_t;

#define FELICA_ID_MAX_LEN 8
#define FELICA_PM_MAX_LEN 8
#define FELICA_SYS_CODE_MAX_LEN 2
typedef struct mtk_nfc_poll_f_tech_para {
    UINT8  au1IDm[FELICA_ID_MAX_LEN];
    UINT8  u1IDmLen;
    UINT8  au1PM[FELICA_PM_MAX_LEN];
    UINT8  au1SystemCode[FELICA_SYS_CODE_MAX_LEN];
    UINT8  u1BitRate; // 0:106, 1:212, 2:424
} s_mtk_nfc_poll_f_tech_param_t;

#define NFC_ISO15693_UID_MAX_LEN 8
typedef struct mtk_nfc_poll_v_tech_param {
    UINT8  u1Flag;
    UINT8  u1Dsfid;
    UINT8  au1Uid[NFC_ISO15693_UID_MAX_LEN];
} s_mtk_nfc_poll_v_tech_param_t;

#define NFC_KOVIO_DATA_MAX_LEN 32
typedef struct mtk_nfc_poll_k_tech_param {
    UINT8 u1ManufId;
    UINT8 u1DataFormat;
    UINT8 au1Data[NFC_KOVIO_DATA_MAX_LEN];
} s_mtk_nfc_poll_k_tech_param_t;

#define NCI_NFCID2_MAX_LEN 8
typedef struct mtk_nfc_listen_f_tech_param {
    UINT8 au1Nfcid2[NCI_NFCID2_MAX_LEN];  /* NFCID2 generated by the Local NFCC for NFC-DEP Protocol.Available for Frame Interface  */
} s_mtk_nfc_listen_f_tech_param_t;

typedef struct mtk_rf_tech_param {
    UINT8  u1Mode;
    union {
        s_mtk_nfc_poll_a_tech_param_t rTechParamPollA;
        s_mtk_nfc_poll_b_tech_param_t rTechParamPollB;
        s_mtk_nfc_poll_f_tech_param_t rTechParamPollF;
        s_mtk_nfc_poll_v_tech_param_t rTechParamPollV;
        s_mtk_nfc_poll_k_tech_param_t rTechParamPollK;
        s_mtk_nfc_listen_f_tech_param_t rTechParamListenF;
    } TechParam; /* Discovery Type specific parameters */
} s_mtk_rf_tech_param_t;

/* the data type associated with NFC_RESULT_DEVT */
typedef struct mtk_discover_device_ntf {
    UINT8  u1DiscoverId;
    UINT8  u1Protocol;
    s_mtk_rf_tech_param_t rRfTechParam;
    UINT8  fgMore; /* 0: last notified device*/
} s_mtk_discover_device_ntf_t;

typedef struct mtk_nfc_service_set_discover_ntf {
    UINT32 u4Result;
    UINT32 u4NumDicoverDev;
    s_mtk_discover_device_ntf_t rDiscoverDev[DISCOVERY_DEV_MAX_NUM];
} s_mtk_nfc_service_set_discover_ntf_t;

// dev select
//set discover
typedef struct mtk_nfc_service_dev_select_req {
    UINT8  u1ID;
    UINT8  u1Protocol;
    UINT8  u1Interface;
} s_mtk_nfc_service_dev_select_req_t;

typedef struct mtk_nfc_service_dev_select_resp {
    UINT32 u4Result;
} s_mtk_nfc_service_dev_select_resp_t;

#define NFC_ATS_MAX_LEN             60
#define NFC_HIS_BYTES_MAX_LEN       50
#define NFC_GEN_BYTES_MAX_LEN       48

typedef struct mtk_nfc_service_PA_iso_dep_param {
    UINT8  u1AtrResLen;                 /* Length of ATR RES                */
    UINT8  au1AtrRes[NFC_ATS_MAX_LEN];  /* ATR RES                          */
    UINT8  u1NadUsed;                   /* NAD is used or not               */
    UINT8  u1Fwi;                       /* Frame Waiting time Integer       */
    UINT8  u1Sfgi;                      /* Start-up Frame Guard time Integer*/
    UINT8  u1HistoricalByteLen;         /* len of historical bytes          */
    UINT8  au1HistoricalByte[NFC_HIS_BYTES_MAX_LEN];/* historical bytes             */
} s_mtk_nfc_service_PA_iso_dep_param_t;

typedef struct mtk_nfc_service_LA_iso_dep_param {
    UINT8 u1Rats;  /* RATS */
} s_mtk_nfc_service_LA_iso_dep_param_t;

typedef struct mtk_nfc_service_PA_nfc_dep_param {
    UINT8  u1AtrResLen;                /* Length of ATR_RES            */
    UINT8  au1AtrRes[NFC_ATS_MAX_LEN]; /* ATR_RES (Byte 3 - Byte 17+n) */
    UINT8  u1MaxPayloadSize;           /* 64, 128, 192 or 254          */
    UINT8  u1GenBytesLen;              /* len of general bytes         */
    UINT8  au1GenBytes[NFC_GEN_BYTES_MAX_LEN];/* general bytes           */
    UINT8  u1WaitingTime;              /* WT -> Response Waiting Time RWT = (256 x 16/fC) x 2WT */
} s_mtk_nfc_service_PA_nfc_dep_param_t;

/* Note: keep tNFC_INTF_PA_NFC_DEP data member in the same order as tNFC_INTF_LA_NFC_DEP */
typedef struct mtk_nfc_service_LA_nfc_dep_param {
    UINT8  u1AtrReqLen;                /* Length of ATR_REQ            */
    UINT8  au1AtrReq[NFC_ATS_MAX_LEN]; /* ATR_REQ (Byte 3 - Byte 18+n) */
    UINT8  u1MaxPayloadSize;           /* 64, 128, 192 or 254          */
    UINT8  u1GenBytesLen;              /* len of general bytes         */
    UINT8  au1GenBytes[NFC_GEN_BYTES_MAX_LEN];/* general bytes           */
} s_mtk_nfc_service_LA_nfc_dep_param_t;

typedef s_mtk_nfc_service_LA_nfc_dep_param_t s_mtk_nfc_service_LF_nfc_dep_param_t;
typedef s_mtk_nfc_service_PA_nfc_dep_param_t s_mtk_nfc_service_PF_nfc_dep_param_t;

#define NFC_ATTRIB_MAX_LEN      (10+NFC_GEN_BYTES_MAX_LEN)

typedef struct mtk_nfc_service_PB_iso_dep_param {
    UINT8  u1AttribResLen;                  /* Length of ATTRIB RES      */
    UINT8  au1AttribRes[NFC_ATTRIB_MAX_LEN];/* ATTRIB RES                */
    UINT8  u1HiInfoLen;                     /* len of Higher layer Info  */
    UINT8  au1HiInfo[NFC_GEN_BYTES_MAX_LEN];/* Higher layer Info         */
    UINT8  u1Mbli;                          /* Maximum buffer length.    */
} s_mtk_nfc_service_PB_iso_dep_param_t;

typedef struct mtk_nfc_service_LB_iso_dep_param {
    UINT8  u1AttribReqLen;                  /* Length of ATTRIB REQ      */
    UINT8  au1AttribReq[NFC_ATTRIB_MAX_LEN];/* ATTRIB REQ (Byte 2 - 10+k)*/
    UINT8  u1HiInfoLen;                     /* len of Higher layer Info  */
    UINT8  au1HiInfo[NFC_GEN_BYTES_MAX_LEN];/* Higher layer Info         */
    UINT8  au1Nfcid0[NFC_NFCID0_MAX_LEN];   /* NFCID0                    */
} s_mtk_nfc_service_LB_iso_dep_param_t;

#define NFC_RAW_PARAMS_MAX_LEN       16
typedef struct mtk_nfc_service_frame_rf_intf_param {
    UINT8  u1ParamLen;
    UINT8  au1Param[NFC_RAW_PARAMS_MAX_LEN];
} s_mtk_nfc_service_frame_rf_intf_param_t;

typedef struct mtk_nfc_service_interface_param {
    UINT8  u1Type;  /* Interface Type  1 Byte  See Table 67 */
    union {
        s_mtk_nfc_service_LA_iso_dep_param_t rLAIsoDep;
        s_mtk_nfc_service_PA_iso_dep_param_t rPAIsoDep;
        s_mtk_nfc_service_LB_iso_dep_param_t rLBIsoDep;
        s_mtk_nfc_service_PB_iso_dep_param_t rPBIsoDep;
        s_mtk_nfc_service_LA_nfc_dep_param_t rLANfcDep;
        s_mtk_nfc_service_PA_nfc_dep_param_t rPANfcDep;
        s_mtk_nfc_service_LF_nfc_dep_param_t rLFNfcDep;
        s_mtk_nfc_service_PF_nfc_dep_param_t rPFNfcDep;
        s_mtk_nfc_service_frame_rf_intf_param_t rFrameRf;
    } InterfaceParam;       /* Activation Parameters   0 - n Bytes */
} s_mtk_nfc_service_interface_param_t;

/* the data type associated with NFC_DEACTIVATE_DEVT and NFC_DEACTIVATE_CEVT */
typedef struct mtk_nfc_service_dev_deactivate_req {
    UINT8  u1DeactType;  
} s_mtk_nfc_service_dev_deactivate_req_t;

typedef struct mtk_nfc_service_dev_deactivate_resp {
    UINT32 u4Result;
} s_mtk_nfc_service_dev_deactivate_resp_t;

typedef struct mtk_nfc_service_dev_deactivate_ntf {
    UINT8  u1DeactType;
    UINT8  u1DeactReason;
} s_mtk_nfc_service_dev_deactivate_ntf_t;

typedef struct mtk_nfc_service_dev_event_ntf {
    UINT8  u1EventType;
    UINT8  u1NumOfTags;
    mtkNfc_rTagTlv_t tag_tlv[TAG_TLV_MAX_NUM_OF_EVT_NTF];
} s_mtk_nfc_service_dev_event_ntf_t;

/* the data type associated with NFC_ACTIVATE_DEVT */
typedef struct mtk_nfc_service_activate_param {
    UINT8  u1RfDiscId;       /* RF Discovery ID          */
    UINT8  u1Protocol;       /* supported protocol       */
    s_mtk_rf_tech_param_t rRfTechParam;  /* RF technology parameters */
    UINT8 u1TxBitrate;      /* Data Exchange Tx Bitrate */
    UINT8 u1RxBitrate;      /* Data Exchange Rx Bitrate */
    s_mtk_nfc_service_interface_param_t rIntfaceParam;     /* interface type and params*/
} s_mtk_nfc_service_activate_param_t;

#define T1T_HR_LEN              2 /* T1T HR length            */
#define T1T_UID_LEN             7 /* T1T UID length           */
#define T1T_CMD_UID_LEN         4 /* UID len for T1T cmds     */

/* Data for NFA_ACTIVATED_EVT */
typedef struct mtk_nfc_service_t1t_tag_param {
    UINT8  au1Hr[T1T_HR_LEN];       /* HR of Type 1 tag         */
    UINT8  au1Uid[T1T_CMD_UID_LEN]; /* UID used in T1T Commands */
} s_mtk_nfc_service_t1t_tag_param_t;

//#define TAG_UID_MAX_LEN 0x0A
typedef struct {
    UINT8  au1Uid[TAG_UID_MAX_LEN]; /* UID of T2T tag           */
} s_mtk_nfc_service_t2t_tag_param_t;

typedef struct mtk_nfc_service_t3t_tag_param {
    UINT8  u1NumSystemCodes;/* Number of system codes supporte by tag   */
    UINT16 *pu2SystemCodes; /* Pointer to list of system codes          */
} s_mtk_nfc_service_t3t_tag_param_t;

#define TAG_15693_UID_MAX_LEN 8
typedef struct mtk_nfc_service_15693_tag_param {
    UINT8  au1Uid[TAG_15693_UID_MAX_LEN];  /* UID[0]:MSB, ... UID[7]:LSB                   */
    UINT8  u1InfoFlags;         /* information flags                            */
    UINT8  u1Dsfid;             /* DSFID if I93_INFO_FLAG_DSFID                 */
    UINT8  u1Afi;               /* AFI if I93_INFO_FLAG_AFI                     */
    UINT16 u2NumBlock;          /* number of blocks if I93_INFO_FLAG_MEM_SIZE   */
    UINT8  u1BlockSize;         /* block size in byte if I93_INFO_FLAG_MEM_SIZE */
    UINT8  u1IcReference;       /* IC Reference if I93_INFO_FLAG_IC_REF         */
} s_mtk_nfc_service_15693_tag_param_t;

typedef struct mtk_nfc_service_tag_param {
    UINT8  u1TagType;
    union {
        s_mtk_nfc_service_t1t_tag_param_t rT1tParam;
        s_mtk_nfc_service_t2t_tag_param_t rT2tParam;
        s_mtk_nfc_service_t3t_tag_param_t rT3tParam;
        s_mtk_nfc_service_15693_tag_param_t r15693TagParam;
    } TagParam;
} s_mtk_nfc_service_tag_param_t;

typedef struct mtk_nfc_service_dev_activate_ntf {
    s_mtk_nfc_service_activate_param_t activate_ntf;   /* RF discovery activation details */
    s_mtk_nfc_service_tag_param_t sTagParams;
} s_mtk_nfc_service_dev_activate_ntf_t;
#endif


typedef struct mtk_nfc_CkNdef_CB
{
   UINT8   Cardinformation;
   UINT32  ActualNdefMsgLength;
   UINT32  MaxNdefMsgLength;

} mtk_nfc_CkNdef_CB_t;



typedef struct mtk_nfc_ndef_data
{
   UINT32 datalen;
   UINT8  databuffer;
} mtk_nfc_ndef_data_t;



typedef struct s_mtk_nfc_jni_transceive_data
{
   UINT32  raw;
   UINT32  result;
   UINT32  datalen;
   UINT8   databuffer;
} s_mtk_nfc_jni_transceive_data_t;




typedef struct mtk_nfc_jni_CkNdef_CB
{
    UINT32  result;
    mtk_nfc_CkNdef_CB_t ndefInfo;
} mtk_nfc_jni_CkNdef_CB_t;


typedef struct s_mtk_nfc_jni_rsp
{
    UINT32  result;
}s_mtk_nfc_jni_rsp_t;

typedef struct s_mtk_nfc_jni_doread_rsp
{
    UINT32  result;
    UINT32  len;
    UINT8   data;
}s_mtk_nfc_jni_doread_rsp_t;



typedef enum mtk_nfc_Tag_LifeCycleSM
{
   MTK_NFC_TAG_LCSM_IDLE        = 0,
   MTK_NFC_TAG_LCSM_INITIALIZE  = 1,
   MTK_NFC_TAG_LCSM_READWRITE   = 2,
   MTK_NFC_TAG_LCSM_READONLY    = 3,
   MTK_NFC_TAG_LCSM_INVAILD     = 4,
   MTK_NFC_TAG_LCSM_END
}mtk_nfc_Tag_LifeCycleSM_e;
    

typedef struct s_mtk_nfc_sw_Version_rsp
{
    CHAR   mw_ver[19];
    UINT16 fw_ver;
    UINT16 hw_ver;
}s_mtk_nfc_sw_Version_rsp_t;


typedef struct MTK_NFC_LLCP_LINK_STATUS
{
    INT32 ret;
    MTK_NFC_LINK_STATUS elink;
} MTK_NFC_LLCP_LINK_STATUS;


typedef struct s_mtk_nfc_loopback_test_req
{
    CHAR   action; /*0:Start, 1:stop*/
}s_mtk_nfc_loopback_test_req_t;

typedef struct s_mtk_nfc_loopback_test_rsp
{
    CHAR   result; /*0:success, 1:fail*/
}s_mtk_nfc_loopback_test_rsp_t;



typedef struct s_mtk_nfc_test_mode_Setting_req
{
    UINT16   forceDownLoad; /*1:enable, 0:disable*/
    UINT16   TagAutoPresenceChk; /*1:enable, 0:disable*/
}s_mtk_nfc_test_mode_Setting_req_t;

typedef struct s_mtk_nfc_test_mode_Setting_rsp
{
    CHAR   result; /*0:success, 1:fail*/
}s_mtk_nfc_test_mode_Setting_rsp_t;




/* ########################################################################## */
#pragma pack()
/* ########################################################################## */

#endif /* MTK_NFC_TYPE_EXT_H */

