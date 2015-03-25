package com.hissage.struct;

public class SNmsMsgType {
    public final static int NMS_UI_MSG_CHOOSE_IAP = 0;
    public final static int NMS_UI_MSG_CHOOSE_SIM_CARD = 1;
    public final static int NMS_UI_MSG_REMIND_USER_TO_ACTIVATE = 2;
    public final static int NMS_UI_MSG_REMIND_USER_TO_SET_NAME = 3;
    public final static int NMS_UI_MSG_REMIND_USER_NO_SIM_CARD = 4;
    public final static int NMS_UI_MSG_INPUT_MOBILE_NUM = 5;
    public final static int NMS_UI_MSG_REGISTRATION_OVER = 6;
    public final static int NMS_UI_MSG_REGISTRATION_IN_PROGRESS = 7;
    public final static int NMS_UI_MSG_REGISTRATION_FAIL = 8;
    public final static int NMS_UI_MSG_ENGINE_STOPPED = 9;
    public final static int NMS_UI_MSG_REQ_POP3_PWD = 10;
    public final static int NMS_UI_MSG_NEW_MSG = 11;
    public final static int NMS_UI_MSG_DEVICE_INFO_ERROR = 12;
    public final static int NMS_UI_MSG_USER_INFO_ERROR = 13;
    public final static int NMS_UI_MSG_NETWORK_ERROR = 14;
    public final static int NMS_UI_MSG_STORAGE_ERROR = 15;
    public final static int NMS_UI_MSG_STORAGE_RESULT = 16;
    public final static int NMS_UI_MSG_STORAGE_CHANGED = 17;
    public final static int NMS_UI_MSG_FETCH_RESULT = 18;
    public final static int NMS_UI_MSG_UPDATE_SNS_RESULT = 19;
    public final static int NMS_UI_MSG_REFRESH = 20;
    public final static int NMS_UI_CFG_CMD_RESULT = 21;
    public final static int NMS_UI_MSG_UPDATE_MSG = 22;
    public final static int NMS_UI_MSG_CONFIG_RESULT = 23;
    public final static int NMS_UI_MSG_PROGRESS_STATE = 24;
    public final static int NMS_UI_MSG_READ_SMS_ERROR = 25; // UI internal
    public final static int NMS_UI_MSG_READ_SMS_DONE = 26; // UI internal
    public final static int NMS_UI_MSG_DB_FULL = 27;
    public final static int NMS_UI_MSG_ENGINE_INIT_DONE = 28;
    public final static int NMS_UI_MSG_SEND_SMS = 29;// UI internal
    public final static int NMS_UI_MSG_CONTACT_REFRESH = 30;
    public final static int NMS_UI_MSG_SHOW_POPUP_ADV = 31;
    public final static int NMS_UI_MSG_SYNC_VCARD_RESULT = 32;
    public final static int NMS_UI_MSG_BILL_PAY_NOTIFY = 33;
    public final static int NMS_UI_MSG_NEW_USER_NOTIFY = 34;
    public final static int NMS_UI_MSG_PASSWD_ERROR_NOTIFY = 35;
    public final static int NMS_UI_MSG_REMIND_USER_OTHER_LOGINED = 36;
    public final static int NMS_UI_MSG_INPUT_NUMBER_AND_COUNTRY = 37;
    public final static int NMS_UI_MSG_INPUT_AUTH_CODE = 38;
    public final static int NMS_UI_MSG_CHANGE_ACTUAL_IMSI = 39;
    public final static int NMS_UI_MSG_CHANGE_VIRTUAL_IMSI = 40;
    public final static int NMS_UI_MSG_ASK_USER_SEND_REGSMS = 41;
    public final static int NMS_UI_MSG_PLAY_SEND_MSG_TONE = 42;
    public final static int NMS_UI_MSG_DEBUG_DATA_CHANGE = 43;
    public final static int NMS_UI_MSG_CLIENT_UPGRADE = 44;
    public final static int NMS_UI_MSG_UPGRADE_DB_START = 45;
    public final static int NMS_UI_MSG_UPGRADE_DB_RESULT = 46;
    public final static int NMS_UI_MSG_RESOTRE_MSG_PROCESSING = 47;
    public final static int NMS_UI_MSG_RESOTRE_MSG_RESULT = 48;
    public final static int NMS_UI_MSG_BACKUP_MSG_PROCESSING = 49; /*
                                                                    * percent of
                                                                    * process
                                                                    */
    public final static int NMS_UI_MSG_BACKUP_MSG_RESULT = 50; /*
                                                                * 0: succeed,
                                                                * -1: normal
                                                                * failed, -2:
                                                                * message count
                                                                * is exceed to
                                                                * max
                                                                */
    public final static int NMS_UI_MSG_GROUP_CHANGED = 51;
    public final static int NMS_UI_MSG_CONTACT_STATUS_UPDATED = 52;
    public final static int NMS_UI_MSG_ASSERT_ALERT = 53;
    public final static int NMS_UI_MSG_OP_RESULT = 54;
    public final static int NMS_UI_MSG_SIM_INFO_CHANGED = 55 ;
    public final static int NMS_UI_MSG_CONTACT_IMG_UPDATED = 56 ;
    public final static int NMS_UI_MSG_REFRESH_CONTACT = 57;
    public final static int NMS_UI_MSG_MAX = 58;

    public final static int NMS_UI_MSG_VERIF_ING = NMS_UI_MSG_MAX + 1;
}
