
#if !defined(NFC_COM_DEF_)
#define NFC_COM_DEF_

#ifndef NULL
#define NULL 0
#endif

#define ERR_OK 0
#define ERR_TIMEOUT 1
#define RESULT_STATUS_TIMEOUT 0xDEADBEEF

#define SIG_BYTEARRAY "[B"
#define SIG_OBJECT "Ljava/lang/Object;"
#define SIG_STRING "Ljava/lang/String;"
#define SIG_STRING_BUF "Ljava/lang/StringBuffer;"
#define SIG_SHORT_ARRAY "[S"
#define SIG_2D_SHORT_ARRAY "[[S"

#define SIG_SETTING_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_setting_request;"
#define SIG_SETTING_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_setting_response;"
#define SIG_REG_NOTIF_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_reg_notif_request;"
#define SIG_REG_NOTIF_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_reg_notif_response;"
#define SIG_DIS_NOTIF_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_dis_notif_request;"
#define SIG_DIS_NOTIF_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_dis_notif_response;"
#define SIG_DIS_NONE_DET_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_none_det_response;"
#define SIG_DIS_TAG_DET_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tag_det_response;"
#define SIG_DIS_P2P_DET_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_p2p_det_response;"

#define SIG_SE_SET_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_se_set_request;"
#define SIG_SE_SET_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_se_set_response;"


#define SIG_TAG_READ_MIFARE1K "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tag_read_Mifare1K;"
#define SIG_TAG_READ_MIFARE4K "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tag_read_Mifare4K;"

#define SIG_TAG_TEST_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_test_request;"
#define SIG_TAG_TEST_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_test_response;"

#define SIG_TAG_TEST_ALWAYSEON_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tx_alwayson_request;"
#define SIG_TAG_TEST_EMUL_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_card_emulation_request;"
#define SIG_TAG_TEST_UID_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_uid_request;"
#define SIG_TAG_TEST_UID_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_uid_response;"
#define SIG_TAG_TEST_SCRIPT_REQ "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_request;"
#define SIG_TAG_TEST_SCRIPT_RES "Lcom/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_response;"

//=========================================
#define CLASS_SETTING_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_setting_request"
#define CLASS_SETTING_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_setting_response"
#define CLASS_REG_NOTIF_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_reg_notif_request"
#define CLASS_REG_NOTIF_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_reg_notif_response"
#define CLASS_DIS_NOTIF_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_dis_notif_request"
#define CLASS_DIS_NOTIF_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_dis_notif_response"

#define CLASS_DIS_NONE_DET_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_none_det_response"
#define CLASS_DIS_TAG_DET_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tag_det_response"
#define CLASS_DIS_P2P_DET_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_p2p_det_response"

#define CLASS_SE_SET_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_se_set_request"
#define CLASS_SE_SET_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_se_set_response"

#define CLASS_TAG_READ_MIFARE1K "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tag_read_Mifare1K"
#define CLASS_TAG_READ_MIFARE4K "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tag_read_Mifare4K"

#define CLASS_TAG_TEST_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_test_request"
#define CLASS_TAG_TEST_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_test_response"

#define CLASS_TAG_TEST_ALWAYSEON_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_tx_alwayson_request"
#define CLASS_TAG_TEST_EMUL_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_card_emulation_request"
#define CLASS_TAG_TEST_UID_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_uid_request"
#define CLASS_TAG_TEST_UID_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_uid_response"
#define CLASS_TAG_TEST_SCRIPT_REQ "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_request"
#define CLASS_TAG_TEST_SCRIPT_RES "com/mediatek/engineermode/nfc/NfcNativeCallClass$nfc_script_response"


#endif
