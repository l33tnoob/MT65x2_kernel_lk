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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   mtk_nfc_ext_msg.h
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *
 *
 *
 ****************************************************************************/
#ifndef _MTK_NFC_EXT_MSG_H_
#define _MTK_NFC_EXT_MSG_H_

//#include "kal_public_defs.h"

#define MTK_NFC_SUCCESS    (0)
#define MTK_NFC_FAIL       (1)

//========================================================
//====Define NFC Service Handler Return Setting/Bitmap ===
//========================================================
#define MTK_NFC_DIS_NOTIF_DURATION      (500)

#define MIFARE4K_LEN        (16)
#define MIFARE1K_LEN        (4)
#define ISO15693_LEN        (4)

#define MIFARE1K_PAGE       (4)
#define UID_DATA_LEN        (7)
#define TAG_RAW_DATA_LEN  (256)

#define NDEF_DATA_LEN       (512)
#define RAW_COMM_DATA_LEN   (256)
#define P2P_MAX_LENGTH      (256) //TBD

#define NFC_HEADER_LENGTH   (8)
//Reader/card mode/Register_notification Bitmap
#define NOTI_MIFARE_UL     (1<<0)
#define NOTI_MIFARE_STD    (1<<1)
#define NOTI_ISO14443_4A    (1<<2)
#define NOTI_ISO14443_4B    (1<<3)
#define NOTI_JEWEL         (1<<4)
#define NOTI_NFC           (1<<5)
#define NOTI_FELICA        (1<<6)
#define NOTI_ISO15693      (1<<7)

//secure element bitmap
#define START_OF_TRANSACTION     (1<<0)
#define END_OF_TRANSACTION       (1<<1)
#define TRANSACTION              (1<<2)
#define RF_FIELD_ON              (1<<3)
#define RF_FIELD_OFF             (1<<4)
#define CONNECTIVITY             (1<<5)


//Discovery notification bitmap
#define DIS_ISO1443_4A                (1<<0)
#define DIS_ISO1443_4B                (1<<1)
#define DIS_FELICA_212                (1<<2)
#define DIS_FELICA_424                (1<<3)
#define DIS_ISO15693                  (1<<4)
#define DIS_NFC_ACTIVE                (1<<5)
#define DIS_DISCARD_CARD_EMULATION    (1<<6)
#define DIS_DISABLE_P2P_IP_TARGET     (1<<7)

//Tag detect indicator
#define TAG_DETECTED              (0x01)
#define TAG_NEDF_DETECTED         (0x02)

//NDEF store data type indicator
#define NDEF_STORE_TYPE_URL             (0x01)
#define NDEF_STORE_TYPE_TEXT            (0x02)
#define NDEF_STORE_TYPE_SMARTPOST       (0x03)

//P2P TARGET/INITATOR
#define P2P_TARGET           (0x01)
#define P2P_INITIATOR        (0x02)



typedef enum NFC_MSG_TYPE
{
    MSG_ID_NFC_SETTING_REQ,
    MSG_ID_NFC_SETTING_RSP,
    MSG_ID_NFC_NOTIFICATION_REQ,
    MSG_ID_NFC_NOTIFICATION_RSP,
    MSG_ID_NFC_SE_SET_REQ,
    MSG_ID_NFC_SE_SET_RSP,
    MSG_ID_NFC_DISCOVERY_REQ,
    MSG_ID_NFC_DISCOVERY_RSP,
    MSG_ID_NFC_TAG_DET_RSP,
    MSG_ID_NFC_P2P_DET_RSP,
    MSG_ID_NFC_TAG_READ_REQ, //10
    MSG_ID_NFC_TAG_READ_RSP,
    MSG_ID_NFC_TAG_WRITE_REQ,
    MSG_ID_NFC_TAG_WRITE_RSP,
    MSG_ID_NFC_TAG_DISCONN_REQ,
    MSG_ID_NFC_TAG_DISCONN_RSP,
    MSG_ID_NFC_TAG_F2NDEF_REQ,
    MSG_ID_NFC_TAG_F2NDEF_RSP,
    MSG_ID_NFC_TAG_RAWCOM_REQ,
    MSG_ID_NFC_TAG_RAWCOM_RSP,
    MSG_ID_NFC_P2P_COMMUNICATION_REQ, //20
    MSG_ID_NFC_P2P_COMMUNICATION_RSP,
    MSG_ID_NFC_RD_COMMUNICATION_REQ,
    MSG_ID_NFC_RD_COMMUNICATION_RSP,
    MSG_ID_NFC_TX_ALWAYSON_TEST_REQ,
    MSG_ID_NFC_TX_ALWAYSON_TEST_RSP,
    MSG_ID_NFC_STOP_TX_ALWAYSON_TEST_REQ,
    MSG_ID_NFC_STOP_TX_ALWAYSON_TEST_RSP,
    MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_REQ,
    MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_RSP,
    MSG_ID_NFC_STOP_TX_ALWAYSON_WO_ACK_TEST_REQ, //30
    MSG_ID_NFC_STOP_TX_ALWAYSON_WO_ACK_TEST_RSP,
    MSG_ID_NFC_CARD_EMULATION_MODE_TEST_REQ,
    MSG_ID_NFC_CARD_EMULATION_MODE_TEST_RSP,
    MSG_ID_NFC_READER_MODE_TEST_REQ,
    MSG_ID_NFC_READER_MODE_TEST_RSP,
    MSG_ID_NFC_P2P_MODE_TEST_REQ,
    MSG_ID_NFC_P2P_MODE_TEST_RSP,
    MSG_ID_NFC_SWP_SELF_TEST_REQ,        /*NFC SWP self test request*/  //38
    MSG_ID_NFC_SWP_SELF_TEST_RSP,        /*NFC SWP self test response*/
    MSG_ID_NFC_ANTENNA_SELF_TEST_REQ,    /*NFC Antenna self test request*/
    MSG_ID_NFC_ANTENNA_SELF_TEST_RSP,    /*NFC Antenna self test response*/
    MSG_ID_NFC_TAG_UID_RW_REQ,           /*NFC Tag_Uid_read/write test request*/
    MSG_ID_NFC_TAG_UID_RW_RSP,	          /*NFC Tag_Uid_read/write test response*/
    MSG_ID_NFC_CARD_MODE_TEST_REQ,
    MSG_ID_NFC_CARD_MODE_TEST_RSP,
    MSG_ID_NFC_STOP_TEST_REQ,
    MSG_ID_NFC_STOP_TEST_RSP
} NFC_MSG_TYPE;

typedef enum
{
    SP_INVALID = -1,
    SP_SOFTWARE_STACK = 0,
    SP_RAW_DATA = 1,
} eSOFTWARE_PROTOCOL;

//======================================================
//====OP REQ/CNF parameters definition of NFC module====
//======================================================

#if !defined(LOCAL_PARA_HDR)
#define LOCAL_PARA_HDR              \
    kal_uint8    ref_count;         \
    kal_uint16   msg_len;
#endif

//======================================================
//====OP REQ/CNF parameters definition of NFC module====
//======================================================

/* The Interlayer Message structure, which is exchaged between modules. */


/* Tag TYPE */


typedef enum nfc_3110_tag_type {
    nfc_3110_tag_DEFAULT    = 0,
    nfc_3110_tag_MIFARE_UL  = 1,
    nfc_3110_tag_MIFARE_STD = 2,
    nfc_3110_tag_ISO1443_4A = 3,
    nfc_3110_tag_ISO1443_4B = 4,
    nfc_3110_tag_JEWEL      = 5,
    nfc_3110_tag_NFC        = 6, //P2P mode
    nfc_3110_tag_FELICA     = 7,
    nfc_3110_tag_ISO15693   = 8,
    nfc_3110_NDEF           = 9
} nfc_3110_tag_type;

typedef enum nfc_ndef_type {
    nfc_ndef_DEFAULT   = 0,
    nfc_ndef_URI       = 1,
    nfc_ndef_TEXT      = 2,
    nfc_ndef_SMART_POSTER = 3,
    nfc_ndef_VCARD     = 4,
    nfc_ndef_MEDIA     = 5,
    nfc_ndef_AbsoluteURI = 6,
    nfc_ndef_ForumExternal = 7,
    nfc_ndef_OTHERS    = 8
} nfc_ndef_type;

typedef enum nfc_ndef_lang_type {
    nfc_3110_ndef_lang_DEFAULT = 0,
    nfc_3110_nfc_ndef_lang_DE = 1,
    nfc_3110_nfc_ndef_lang_EN = 2,
    nfc_3110_nfc_ndef_lang_FR = 3
} nfc_ndef_lang_type;

typedef enum ndef_url_type
{
    ndef_url_DEFAULT = 0,
    ndef_url_webaddr = 1,
    ndef_url_mailaddr = 2,
    ndef_url_tel = 3
} ndef_url_type;


typedef struct nfc_msg_struct {
    //LOCAL_PARA_HDR
    unsigned int    msg_type;           /* message identifier */
    unsigned int    msg_length;         /* length of 'data' */
} nfc_msg_struct;


/*NFC Setting message structure*/
//nfc_setting_request
typedef struct nfc_setting_request {
    unsigned int    nfc_enable;               /* feature enable or disable , 0: disable, 1:enable */
    unsigned int    debug_enable;        /*debug enable or disable , 0: disable, 1:enable */
    unsigned int    sw_protocol;           /*SW protocol,  0: SW stack, 1: raw data mode */
    unsigned int     get_capabilities;      /*get chip capability 0: no request, 1: request*/
} nfc_setting_request;
//nfc_setting_response
typedef struct nfc_setting_response {
    unsigned int    status;                        /* return setting result*/
    unsigned int    nfc_enable;               /* return feature enable or disable */
    unsigned int    debug_enable;        /* return debug enable or disable*/
    unsigned int    sw_protocol;           /* return SW protocol,*/
    unsigned int    get_capabilities;      /* return chip capability*/
    unsigned int    sw_ver;                      /* return software version*/
    unsigned int    hw_ver;                     /* return hardware version*/
    unsigned int    fw_ver;                       /* return firmware version*/
    unsigned int    reader_mode;          /* return support  format bitmap*/
    unsigned int    card_mode;              /* return support  format bitmap */
} nfc_setting_response;

/*NFC Register Notification*/
//nfc_reg_notification_request
typedef struct nfc_reg_notif_request {
    unsigned int    reg_type;               /* register notification bitmap */
} nfc_reg_notif_request;

//nfc_reg_notification_response
typedef struct nfc_reg_notif_response {
    unsigned int     status;          /* return setting result,0 success, other: fail*/
    unsigned int     se;                /* secure element detect, 0: no detect, 1: se typ1 , 2: se type 2*/
    unsigned int     se_status; /* secure element status, 0:off, 1:Virtual, 2:Wired*/
    unsigned int     se_type;      /* secure element type, bitmap*/
    unsigned int length; /* length of data*/
} nfc_reg_notif_response;

/*NFC Secure Element*/
//nfc_se_set_request
typedef struct nfc_se_set_request {
    unsigned int     set_SEtype;      /* setting se  type, 0:off, 1:Virtual, 2:Wired */
} nfc_se_set_request;

//nfc_se_set_response
typedef struct nfc_se_set_response {
    unsigned int     status;          /* return setting result, 0:sucess*/
} nfc_se_set_response;


/*NFC Discovery Notification*/
//nfc_dis_notification_request
typedef struct nfc_dis_notif_request {
    unsigned int    dis_type;               /* discovery notification bitmap */
    unsigned int duration;                 /* set duration,(unit:ms) */
} nfc_dis_notif_request;

/*NFC Tag Detected Response*/
//nfc_tag_det_response
typedef struct nfc_tag_det_response {
    unsigned int    tag_type;               /* return detected tag type*/
    unsigned int    card_type;            /* card type, 1: Mifare classic 1K*/
    /* 2: Mifare classic 4K*/
    /* 3: NDEF*/
    unsigned char   uid[10];                        /* card Uid*/
    unsigned int    sak;                       /* card sak*/
    unsigned int    atag;                    /* card atag*/
    unsigned int    appdata;             /* card appdata */
    unsigned int    maxdatarate;    /* card maxdatarate */
} nfc_tag_det_response;

/*NFC Tag Read Request*/
//nfc_tag_read_request
typedef struct nfc_tag_read_request {
    nfc_3110_tag_type    read_type;               /*  which type want to read*/
    unsigned int    address;                  /* for Mifare classic 1K used*/
    unsigned int    sector;                    /* for Mifare classic 4K used*/
    unsigned int    block;                     /* for Mifare classic 4K used*/
    unsigned char   AuthentificationKey;       /* KEY_A:0 , KEY_B:1 */
} nfc_tag_read_request;

/*NFC Peer to Peer Detected Response*/
//nfc_p2p_det_response
typedef struct nfc_p2p_det_response {
    unsigned int    p2p_type;               /* return detected tag type*/
} nfc_p2p_det_response;

typedef union nfc_dis_notif_result_response_u {
    nfc_tag_det_response nfc_tag_det_resp;
    nfc_p2p_det_response nfc_p2p_det_resp;
} nfc_dis_notif_result_response_u;

//nfc_dis_notification_response
typedef struct nfc_dis_notif_response {
    unsigned int     status;          /* return setting result, 0:sucess */
    unsigned char    type;           /*1 : Tag ,2: p2p */
    nfc_dis_notif_result_response_u nfc_dis_notif_result;
} nfc_dis_notif_response;

typedef struct nfc_tag_read_Mifare1K {
    unsigned int    address;               /*  */
    unsigned short  data[MIFARE1K_LEN];
} nfc_tag_read_Mifare1K;

typedef struct nfc_tag_read_Mifare4K {
    unsigned int    sector;               /*  */
    unsigned int    block;
    unsigned short data[MIFARE4K_LEN];
} nfc_tag_read_Mifare4K;

typedef struct nfc_tag_read_ndef {
    nfc_ndef_type  ndef_type;
    unsigned  char lang[2];
    unsigned  char recordFlags;
    unsigned  char recordId[32];
    unsigned  char recordTnf;
    unsigned  int  length;
    unsigned  char data[NDEF_DATA_LEN];
} nfc_tag_read_ndef;

typedef struct nfc_tag_read_ISO15693 {
    unsigned int    sector;               /*  */
    unsigned int    block;
    unsigned char data[MIFARE4K_LEN];
} nfc_tag_read_ISO15693;

typedef union nfc_tag_read_result_response_u {
    nfc_tag_read_Mifare1K nfc_tag_read_Mifare1K_resp;
    nfc_tag_read_Mifare4K nfc_tag_read_Mifare4K_resp;
    nfc_tag_read_ndef nfc_tag_read_ndef_resp;
    nfc_tag_read_ISO15693 nfc_tag_read_ISO15693_resp;
} nfc_tag_read_result_response_u;

/*NFC Tag Read Response*/
//nfc_tag_read_response
typedef struct nfc_tag_read_response {
    unsigned int    status;               /*  return read status, 0 success*/
    nfc_3110_tag_type    type;                 /*  Check nfc_tag_type */
    nfc_tag_read_result_response_u nfc_tag_read_result;
} nfc_tag_read_response;

/*NFC Tag Write Request*/
typedef struct nfc_tag_write_typeMifare {
    unsigned int    sector;               /*  */
    unsigned int    block;
    unsigned char   data[MIFARE4K_LEN];
    unsigned char   AuthentificationKey;       /* KEY_A:0 , KEY_B:1 */
} nfc_tag_write_typeMifare;

typedef struct nfc_tag_write_typeISO15693 {
    unsigned int    sector;               /*  */
    unsigned int    block;
    unsigned char   data[MIFARE4K_LEN];
} nfc_tag_write_typeISO15693;



typedef struct nfc_Vcard
{
    char Name[64];
    char Compagny[64];
    char Titlep[64];
    char Tel[32];
    char Email[64];
    char Adress[128];
    char PostalCode[32];
    char City[64];
    char CompagnyUrl[64];
} nfc_Vcard_t;


typedef struct nfc_SmartPoster
{
    unsigned char  Compagny[64];
    unsigned short CompagnyLength;
    unsigned char CompagnyUrl[64];
    unsigned short CompagnyUrlLength;
} nfc_SmartPoster_t;


typedef struct nfc_Text
{
    unsigned char data[128];
    unsigned short DataLength;
} nfc_Text_t;

typedef struct nfc_URL
{
    ndef_url_type URLtype;
    unsigned char URLData[64];
    unsigned short URLLength;
} nfc_URL_t;

typedef struct nfc_EXTTag
{
    char EXTTagType[64];
    char EXTData[128];
    unsigned short EXTLength;
} nfc_EXTTag_t;


typedef union nfc_tag_write_ndef_data
{
    nfc_SmartPoster_t  SP_Data;
    nfc_Vcard_t        VC_Data;
    nfc_Text_t         TX_Data;
    nfc_URL_t          URL_Data;
    nfc_EXTTag_t       EXT_Data;
} nfc_tag_write_ndef_data;


typedef struct nfc_tag_write_ndef {
    nfc_ndef_type      ndef_type;
    nfc_ndef_lang_type language;
    unsigned  int  length;
    nfc_tag_write_ndef_data ndef_data;
} nfc_tag_write_ndef;

typedef union nfc_tag_write_data_request_u {
    nfc_tag_write_typeMifare   nfc_tag_write_typeMifare_data;
    nfc_tag_write_typeISO15693 nfc_tag_write_typeISO15693_data;
    nfc_tag_write_ndef   nfc_tag_write_ndef_data;
} nfc_tag_write_data_request_u;



//nfc_tag_write_request
typedef struct nfc_tag_write_request {
    nfc_3110_tag_type    write_type;               /*  which type want to write*/
    nfc_tag_write_data_request_u  nfc_tag_write_data;
} nfc_tag_write_request;

/*NFC Tag Write Response*/
//nfc_tag_write_response
typedef struct nfc_tag_write_response {
    nfc_3110_tag_type    type;      /*  return writed type*/
    unsigned int    status;                /*  return read status, 0 success*/
} nfc_tag_write_response;

/*NFC Tag Disconnect Request*/
//nfc_tag_disconnect_request
typedef struct nfc_tag_disconnect_request {
    unsigned int    action;     /* 1: disconnect, */
} nfc_tag_disconnect_request;
/*NFC Tag Disconnect Response*/
//nfc_tag_disconnect_response
typedef struct nfc_tag_disconnect_response {
    unsigned int    status;      /*0: success*/
} nfc_tag_disconnect_response;

/*NFC Tag format tp Ndef format Request*/
//nfc_tag_format2Ndef_request
typedef struct nfc_tag_fromat2Ndef_request {
    unsigned int    action;      /* 1: format to Ndef, */
} nfc_tag_fromat2Ndef_request;
/*NFC Tag Disconnect Response*/
//nfc_tag_format2Ndef_response
typedef struct nfc_tag_fromat2Ndef_response {
    unsigned int    status;      /*0: success*/
} nfc_tag_fromat2Ndef_response;


/*NFC Tag raw command Request*/
typedef struct nfc_tag_raw_com_req_typeA {
    unsigned int   length;
    unsigned char data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_req_typeA;

typedef struct nfc_tag_raw_com_req_typeB {
    unsigned int   length;
    unsigned char data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_req_typeB;

typedef struct nfc_tag_write_typeJewel {
    unsigned int   length;
    unsigned char  data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_req_typeJewel;

typedef struct nfc_tag_write_typeFelica {
    unsigned int   length;
    unsigned char  data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_req_typeFelica;


typedef  union nfc_tag_raw_com_req_data_u {
    nfc_tag_raw_com_req_typeA  req_data_typeA;
    nfc_tag_raw_com_req_typeB  req_data_typeB;
    nfc_tag_raw_com_req_typeJewel req_data_typeJewel;
    nfc_tag_raw_com_req_typeFelica req_data_typeFelica;
} nfc_tag_raw_com_req_data_u;


typedef struct nfc_tag_raw_com_rsp_typeA {
    unsigned int   length;
    unsigned char data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_rsp_typeA;

typedef struct nfc_tag_raw_com_rsp_typeB {
    unsigned int   length;
    unsigned char data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_rsp_typeB;

typedef struct nfc_tag_raw_com_rsp_typeJewel {
    unsigned int   length;
    unsigned char  data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_rsp_typeJewel;

typedef struct nfc_tag_raw_com_rsp_typeFelica {
    unsigned int   length;
    unsigned char  data[TAG_RAW_DATA_LEN];
} nfc_tag_raw_com_rsp_typeFelica;

typedef  union nfc_tag_raw_com_rsp_data_u {
    nfc_tag_raw_com_rsp_typeA  rsp_data_typeA;
    nfc_tag_raw_com_rsp_typeB  rsp_data_typeB;
    nfc_tag_raw_com_rsp_typeJewel  rsp_data_typeJewel;
    nfc_tag_raw_com_rsp_typeFelica rsp_data_typeFelica;
} nfc_tag_raw_com_rsp_data_u;

//nfc_tag_raw_com_request
typedef struct nfc_tag_raw_com_request {
    nfc_3110_tag_type    type;        /*Type A / Type B*/
    unsigned int    length;     /*Length of written data. Unit is byte. The data is stored in the Meta peer buffer*/
    nfc_tag_raw_com_req_data_u req_data;
} nfc_tag_raw_com_request;
/*NFC Tag Disconnect Response*/
//nfc_tag_raw_com_response
typedef struct nfc_tag_raw_com_response {
    nfc_3110_tag_type    type;        /*Type A / Type B*/
    unsigned int    status;      /*0 : success*/
    nfc_tag_raw_com_rsp_data_u   rsp_data;
} nfc_tag_raw_com_response;

// Need to check the length and data behavior
/*NFC Peer to Peer communication*/
//nfc_p2p_com_request
typedef struct nfc_p2p_com_request {
    unsigned int    action;      /* 1 : send, 2 receive*/
    unsigned int    length;      /*Length of written data. Unit is byte. The data is stored in the Meta peer buffer*/
    unsigned char   data[P2P_MAX_LENGTH];
} nfc_p2p_com_request;

//nfc_p2p_com_response
typedef struct nfc_p2p_com_response {
    unsigned int    status;   /* 0:success*/
    unsigned int    length;   /*Length of written data. Unit is byte.*/
    unsigned char   data[P2P_MAX_LENGTH];
} nfc_p2p_com_response;


/*NFC raw data communication*/
//nfc_rd_com_request
typedef struct nfc_rd_com_request {
    unsigned int    action;      /*1:start test, 0:stop test*/
    unsigned int    length;      /*Length of written data. Unit is byte. The data is stored in the Meta peer buffer*/
} nfc_rd_com_request;

//nfc_rd_com_response
typedef struct nfc_rd_com_response {
    unsigned int    status;   /*Test result, 0:success*/
    unsigned int    length;   /*Length of written data. Unit is byte.*/
} nfc_rd_com_response;


/*NFC test mode*/

//nfc_script_request
typedef struct nfc_script_request {
    unsigned int   type;           /*Test type: currently, always set 1*/
    unsigned int   action;	      /*Test action, 1: start test, 0:stop test*/
} nfc_script_request;
//nfc_script_reponse
typedef struct nfc_script_response {
    signed int   result;         /*Test result,0 :success*/
} nfc_script_response;


/*NFC test mode*/
//nfc_script_request
typedef struct nfc_script_uid_request {
    unsigned int   type;                /*Test type: currently, always set 1*/
    unsigned int   action;	      /*Test action, 1: start test, 0:stop test*/
    unsigned int   uid_type;        /* 1: uid 4bytes, 2 : uid 7bytes*/
    unsigned char data[UID_DATA_LEN];       /*uid content*/
} nfc_script_uid_request;
//nfc_script_reponse
typedef struct nfc_script_uid_response {
    signed int   result;                 /*Test result, 0: success*/
    unsigned int uid_type;         /* 1: uid 4bytes, 2 : uid 7bytes */
    unsigned char data[UID_DATA_LEN];      /*uid content*/
} nfc_script_uid_response;

typedef struct nfc_tx_alwayson_request {
    unsigned int   type;    /*Test type: currently, always set 1*/
    unsigned int   action;	/*Test action, 1: start test, 0:stop test*/
    unsigned char  modulation_type;   /* 0:type A, 1:type B, 2:type F, 3:No Modulation */
    unsigned char  bitrate;           /* 0:106kbps, 1:212kbps, 2:424kbps */
} nfc_tx_alwayson_request;


typedef struct nfc_card_emulation_request {
    unsigned int   type;           /*Test type: currently, always set 1*/
    unsigned int   action;	       /*Test action, 1: start test, 0:stop test*/
    unsigned short  technology;    /*bitmask: MifareUL=bit 0, MifareStd=bit1, ISO14443_4A=bit 2, ISO14443_4B=bit 3, Jewel=bit 4, Felica=bit 5, ISO15693=bit 6 */
    unsigned short  protocols;     /*bitmask: Iso14443A=bit 0, Iso14443B=bit 1, Felica212=bit 2, Felica424=bit 3, Iso15693=bit 4 */
} nfc_card_emulation_request;

#endif /* _MTK_NFC_EXT_MSG_H_ */
