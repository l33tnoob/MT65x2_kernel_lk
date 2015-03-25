#ifndef NFC_CMD_H
#define NFC_CMD_H
#include "mtk_nfc_ext_msg.h"
#include "atcid_util.h"


#define SOCKET_NFC_PORT (7500)

//========================================================
//====Define NFC Service Handler Return Setting/Bitmap ===
//========================================================

//==================================================
//====Define NFC Service Handler Return Status =====
//==================================================

/** \ingroup grp_retval    The function indicates successful completion. */
#define NFCSTATUS_SUCCESS                                       (0x0000)
/** \ingroup grp_retval    At least one paramter could not be properly interpreted. */
#define NFCSTATUS_INVALID_PARAMETER                             (0x0001)
/** \ingroup grp_retval    The buffer provided by the caller is too small. */
#define NFCSTATUS_BUFFER_TOO_SMALL                              (0x0003)
/** \ingroup grp_retval    Device specifier/handle value is invalid for the operation. */
#define NFCSTATUS_INVALID_DEVICE                                (0x0006)
/** \ingroup grp_retval
    The function executed successfully but could have returned
    more information than space provided by the caller. */
#define NFCSTATUS_MORE_INFORMATION                              (0x0008)
/** \ingroup grp_retval
    No response from the remote device received: Time-out.*/
#define NFCSTATUS_RF_TIMEOUT                                    (0x0009)
/** \ingroup grp_retval
    RF Error during data transaction with the remote device.*/
#define NFCSTATUS_RF_ERROR                                      (0x000A)
/** \ingroup grp_retval
    Not enough resources (e.g. allocation failed.). */
#define NFCSTATUS_INSUFFICIENT_RESOURCES                        (0x000C)

/** \ingroup grp_retval
    A non-blocking function returns this immediately to indicate
    that an internal operation is in progress. */
#define NFCSTATUS_PENDING                                       (0x000D)

/** \ingroup grp_retval
    A board communication error occurred
    (e.g. configuration went wrong). */
#define NFCSTATUS_BOARD_COMMUNICATION_ERROR                     (0x000F)

/** \ingroup grp_retval
    Invalid State of the particular state machine
 */
#define NFCSTATUS_INVALID_STATE                                 (0x0011)


/** \ingroup grp_retval
    This Layer is Not initialised, hence initialisation required.
 */
#define NFCSTATUS_NOT_INITIALISED                               (0x0031)


/** \ingroup grp_retval
    The Layer is already initialised, hence initialisation repeated.
 */
#define NFCSTATUS_ALREADY_INITIALISED                           (0x0032)


/** \ingroup grp_retval
    Feature not supported . */
#define NFCSTATUS_FEATURE_NOT_SUPPORTED                         (0x0033)

/** \ingroup grp_retval
    The Unregistration command has failed because the user wants to unregister on
    an element for which he was not registered*/
#define NFCSTATUS_NOT_REGISTERED                                (0x0034)


/** \ingroup grp_retval
    The Registration command has failed because the user wants to register on
    an element for which he is already registered*/
#define NFCSTATUS_ALREADY_REGISTERED                            (0x0035)

/** \ingroup grp_retval
    Single Tag with Multiple
	Protocol support detected. */
#define NFCSTATUS_MULTIPLE_PROTOCOLS                           (0x0036)

/** \ingroup grp_retval
    Feature not supported . */
#define NFCSTATUS_MULTIPLE_TAGS	 	                            (0x0037)

/** \ingroup grp_retval
    A DESELECT event has occurred. */
#define NFCSTATUS_DESELECTED                                    (0x0038)

/** \ingroup grp_retval
    A RELEASE event has occurred. */
#define NFCSTATUS_RELEASED                                      (0x0039)

/** \ingroup grp_retval
    The operation is currently not possible or not allowed */
#define NFCSTATUS_NOT_ALLOWED                                   (0x003A)

/** \ingroup grp_retval
     The sytem is busy with the previous operation.
*/
#define NFCSTATUS_BUSY                                          (0x006F)


/* NDEF Mapping error codes */

/** \ingroup grp_retval
    The remote device (type) is not valid for this request. */
#define NFCSTATUS_INVALID_REMOTE_DEVICE                         (0x001D)

/** \ingroup grp_retval
    Smart tag functionality not supported */
#define NFCSTATUS_SMART_TAG_FUNC_NOT_SUPPORTED                  (0x0013)

/** \ingroup grp_retval
    Read operation failed */
#define NFCSTATUS_READ_FAILED                                   (0x0014)

/** \ingroup grp_retval
    Write operation failed */
#define NFCSTATUS_WRITE_FAILED                                  (0x0015)

/** \ingroup grp_retval
    Non Ndef Compliant */
#define NFCSTATUS_NO_NDEF_SUPPORT                               (0x0016)

/** \ingroup grp_retval
    Could not proceed further with the write operation: reached card EOF*/
#define NFCSTATUS_EOF_NDEF_CONTAINER_REACHED                    (0x001A)

/** \ingroup grp_retval
    Incorrect number of bytes received from the card*/
#define NFCSTATUS_INVALID_RECEIVE_LENGTH                        (0x001B)

/** \ingroup grp_retval
    The data format/composition is not understood/correct. */
#define NFCSTATUS_INVALID_FORMAT                                (0x001C)


/** \ingroup grp_retval
    There is not sufficient storage available. */
#define NFCSTATUS_INSUFFICIENT_STORAGE                          (0x001F)

/** \ingroup grp_retval
    The Ndef Format procedure has failed. */
#define NFCSTATUS_FORMAT_ERROR                                  (0x0023)


/* 0x70 to 0xCF Values are Component Specific Error Values */

/** \ingroup grp_retval
    Max number of Status Codes*/
#define NFCSTATUS_FAILED                                        (0x00FF)

//================================================
//===========OP enum definition of NFC module=====
//================================================
typedef enum NFC_OP
{
    NFC_OP_SETTING = 0       //0
    ,NFC_OP_REG_NOTIFY        // 1
    ,NFC_OP_SECURE_ELEMENT    // 2
    ,NFC_OP_DISCOVERY         // 3
    ,NFC_OP_TAG_READ          // 4
    ,NFC_OP_TAG_WRITE         //5
    ,NFC_OP_TAG_DISCONN       //6
    ,NFC_OP_TAG_FORMAT_NDEF   //7
    ,NFC_OP_TAG_RAW_COMM       //8
    ,NFC_OP_P2P_COMM          //9
    ,NFC_OP_RD_COMM           //10
    ,NFC_OP_TX_ALWAYSON_TEST       //11
    ,NFC_OP_TX_ALWAYSON_WO_ACK_TEST       //12
    ,NFC_OP_CARD_MODE_TEST       //13
    ,NFC_OP_READER_MODE_TEST       //14
    ,NFC_OP_P2P_MODE_TEST       //15
    ,NFC_OP_SWP_SELF_TEST       //16
    ,NFC_OP_ANTENNA_SELF_TEST       //17
    ,NFC_OP_TAG_UID_RW          //18
    ,NFC_OP_END
} NFC_OP;

//======================================================
//====OP REQ/CNF parameters definition of NFC module====
//======================================================

//================================================
//========NFC Meta Message Structure==============
//================================================

typedef union META_NFC_CMD_U
{
    nfc_setting_request     m_setting_req;
    nfc_reg_notif_request   m_reg_notify_req;
    nfc_se_set_request      m_se_set_req;
    nfc_dis_notif_request   m_dis_notify_req;
    nfc_tag_read_request    m_tag_read_req;
    nfc_tag_write_request   m_tag_write_req;
    nfc_tag_disconnect_request  m_tag_discon_req;
    nfc_tag_fromat2Ndef_request m_tag_fromat2Ndef_req;
    nfc_tag_raw_com_request m_tag_raw_com_req;
    nfc_p2p_com_request     m_p2p_com_req;
    nfc_rd_com_request      m_rd_com_req;
    nfc_script_request      m_script_req;
    nfc_script_uid_request  m_script_uid_req;
    nfc_card_emulation_request m_nfc_card_emulation_req;
    nfc_tx_alwayson_request m_nfc_tx_alwayson_req;
} META_NFC_CMD_U;

typedef struct NFC_REQ
{
//    FT_H	       header;  //module do not need care it
    NFC_OP		   op;
    META_NFC_CMD_U  cmd;
} NFC_REQ;

typedef union META_NFC_CNF_U
{
    nfc_setting_response    m_setting_cnf;
    nfc_reg_notif_response  m_reg_notify_cnf;
    nfc_se_set_response     m_se_set_cnf;
    nfc_dis_notif_response  m_dis_notify_cnf;
    nfc_tag_read_response   m_tag_read_cnf;
    nfc_tag_write_response  m_tag_write_cnf;
    nfc_tag_disconnect_response m_tag_discon_cnf;
    nfc_tag_fromat2Ndef_response m_tag_fromat2Ndef_cnf;
    nfc_tag_raw_com_response  m_tag_raw_com_cnf;
    nfc_p2p_com_response    m_p2p_com_cnf;
    nfc_rd_com_response     m_rd_com_cnf;
    nfc_script_response     m_script_cnf;
    nfc_script_uid_response m_script_uid_cnf;
} META_NFC_CNF_U;

typedef struct NFC_CNF
{
//	FT_H		    header;  //module do not need care it
    NFC_OP			op;
    META_NFC_CNF_U  result;
    unsigned int	status;

} NFC_CNF;

//================================================
//========NFC Service Handler Message Structure===
//================================================

#define MTK_NFC_MAX_ILM_BUFFER_SIZE  (1024)

typedef enum MTK_NFC_MSG_TYPE
{
    MSG_ID_NFC_DEFERRED_CALL_REQ = 0,
    MSG_ID_NFC_WRITE_REQ,
    MSG_ID_NFC_LIB_INIT,
    MSG_ID_NFC_LIB_DEINIT,
    MSG_ID_NFC_TEST_REQ,
    MSG_ID_NFC_TEST_RSP,

    /* App -> NFC Service */
    MSG_ID_NFC_NTF_INITIAL_REQ,
    MSG_ID_NFC_REMOTE_DEV_NTF_REGISTER_REQ,
    MSG_ID_NFC_REMOTE_DEV_NTF_UNREGISTER_REQ,
    MSG_ID_NFC_REMOTE_DEV_CONNECT_REQ,
    MSG_ID_NFC_REMOTE_DEV_DISCONNECT_REQ,
    MSG_ID_NFC_REMOTE_DEV_TRANSCEIVE_REQ,
    MSG_ID_NFC_NDEF_CHECK_NDEF_REQ,
    MSG_ID_NFC_NDEF_WRITE_REQ,
    MSG_ID_NFC_NDEF_READ_REQ,
    MSG_ID_NFC_MGT_CONFIGURE_DISCOVERY_REQ,
    MSG_ID_NFC_SE_NTF_REGISTER_REQ,
    MSG_ID_NFC_SE_NTF_UNREGISTER_REQ,
    MSG_ID_NFC_SE_GET_SECURE_ELEMENT_LIST_REQ,
    MSG_ID_NFC_SE_SET_MODE_REQ,

    /* NFC Service -> App for RSP*/
    MSG_ID_NFC_NTF_INITIAL_RSP,
    MSG_ID_NFC_REMOTE_DEV_NTF_REGISTER_RSP,
    MSG_ID_NFC_REMOTE_DEV_NTF_UNREGISTER_RSP,
    MSG_ID_NFC_REMOTE_DEV_CONNECT_RSP,
    MSG_ID_NFC_REMOTE_DEV_DISCONNECT_RSP,
    MSG_ID_NFC_REMOTE_DEV_TRANSCEIVE_RSP,
    MSG_ID_NFC_NDEF_CHECK_NDEF_RSP,
    MSG_ID_NFC_NDEF_WRITE_RSP,
    MSG_ID_NFC_NDEF_READ_RSP,
    MSG_ID_NFC_MGT_CONFIGURE_DISCOVERY_RSP,
    MSG_ID_NFC_SE_NTF_REGISTER_RSP,
    MSG_ID_NFC_SE_NTF_UNREGISTER_RSP,
    MSG_ID_NFC_SE_GET_SECURE_ELEMENT_LIST_RSP,
    MSG_ID_NFC_SE_SET_MODE_RSP,
    MSG_ID_NFC_CHECK_OWNER_RSP,

    /* NFC Service -> App for CB */
    MSG_ID_NFC_REMOTE_DEV_NTF_REGISTER_CB_IND,
    MSG_ID_NFC_REMOTE_DEV_CONNECT_CB_IND,
    MSG_ID_NFC_REMOTE_DEV_DISCONNECT_CB_IND,
    MSG_ID_NFC_REMOTE_DEV_TRANSCEIVE_CB_IND,
    MSG_ID_NFC_NDEF_CHECK_NDEF_CB_IND,
    MSG_ID_NFC_NDEF_WRITE_CB_IND,
    MSG_ID_NFC_NDEF_READ_CB_IND,
    MSG_ID_NFC_MGT_CONFIGURE_DISCOVERY_CB_IND,
    MSG_ID_NFC_SE_NTF_REGISTER_CB_IND,
    MSG_ID_NFC_SE_SET_MODE_CB_IND,

    /* NFC Lib -> NFC Service */
    MSG_ID_NFC_DEFERRED_CALL_CB_IND,

    /*others*/
    MSG_ID_NFC_PUSH_INFO_REGISTER,
    MSG_ID_NFC_PUSH_REGISTRY_NOTIFICATION
} MTK_NFC_MSG_TYPE;

typedef enum MTK_NFC_MOD_TYOE
{
    MOD_NFC,
    MOD_NFC_APP
} MTK_NFC_MOD_TYOE;

typedef enum MTK_NFC_SAP_TYPE
{
    NFC_SAP,
    NFC_APP_SAP,
} MTK_NFC_SAP_TYPE;

typedef struct ilm_struct
{
    unsigned char used;
    MTK_NFC_MSG_TYPE msg_id;
    MTK_NFC_MOD_TYOE src_mod_id;
    MTK_NFC_MOD_TYOE dest_mod_id;
    MTK_NFC_SAP_TYPE sap_id;
    // void *peer_buff_ptr;
    // unsigned char ilm_data[MTK_NFC_MAX_ILM_BUFFER_SIZE];
    unsigned char local_para_ptr[MTK_NFC_MAX_ILM_BUFFER_SIZE];
} ilm_struct;


//================================================
//============External function of NFC module=====
//================================================
int nfc_cmd_handler(char* cmdline, ATOP_t at_op, char* response);


#endif
