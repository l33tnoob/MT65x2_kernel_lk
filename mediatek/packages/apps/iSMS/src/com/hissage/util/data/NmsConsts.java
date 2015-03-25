package com.hissage.util.data;

import java.text.SimpleDateFormat;

public final class NmsConsts {

    public static final SimpleDateFormat SDF3 = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    public static final SimpleDateFormat SDF2 = new SimpleDateFormat("MM-dd HH:mm:ss");
    public static final SimpleDateFormat SDF1 = new SimpleDateFormat("MM-dd_HH-mm-ss");
    
    public static final int INVALID_SIM_ID = -1;
    public static final int SINGLE_CARD_SIM_ID = -2;
    public static final int ALL_SIM_CARD = -3;
    
    public static final String SIM_ID = "sim_id";
    public static final int SIM_CARD_COUNT = 2;
    public static final int SIM_CARD_SLOT_1 = 0;
    public static final int SIM_CARD_SLOT_2 = 1;
    //M:Activation Statistics
    public static final String ACTIVATE_TYPE = "activate_type" ;
    
    public static final class HissageTag {
        public static final String global = "Global";
        public static final String api = "[API]";
        public static final String vcard = "[vcard]";
    }

    public static final class NmsIntentStrId {

        public final static String NMS_INTENT_MMS_TRANSACTION = "com.isms.mmstransaction";
        public final static String NMS_INTENT_JUST_SHOW_DLG = "com.isms.justshowdlg";
        public final static String NMS_INTENT_RESTORE_SHOW_DLG = "com.isms.restore";
        public final static String NMS_INTENT_DELETE_SYS_MSG_DONE = "com.isms.deletesmsmmsdone";
        public final static String NMS_INTENT_UPDATE_SYS_MSG_DONE = "com.isms.updatesmsmmsdone";

        public final static String NMS_INTENT_DELETE_CONATACT = "com.isms.refresh_contact";
        public final static String NMS_INTENT_ADD_CONATACT = "com.isms.add_contact";
        public final static String NMS_INTENT_UPDATE_CONATACT = "com.isms.update_contact";
        public final static String NMS_INTENT_REGISTRATION = "com.isms.registration";
        public final static String NMS_INTENT_NO_SIMCARD = "com.isms.no_sim_card";

        public final static String NMS_INTENT_CANCEL_ISMS_NOTIFY = "com.isms.cancelIsmsNotify";
        public final static String NMS_INTENT_CANCEL_MMSSMS_NOTIFY = "com.isms.cancelMMSSMSNotify";
        public final static String NMS_INTENT_PHONE_NUMBER = "com.isms.phoneNumber";
        public final static String NMS_INTENT_MSG_BODY = "com.isms.msgBody";
        public final static String NMS_INTENT_COMPOSE_ACTION = "com.isms.composeAction";
        public final static String NMS_ACTION_INVITE = "invite";
        public final static String NMS_ACTION_FORWARD = "forward";

        public final static String NMS_CONNECT_CHANGE = "android.net.conn.CONNECTIVITY_CHANGE";

        public final static String NMS_CONTACTID = "contactId";

        public final static String NMS_REG_INPUT_PHONENUM = "com.isms.input_phonenum";
        public final static String NMS_REG_SMS_ERROR = "com.isms.notInterruptRegSms";
        public final static String NMS_REG_STATUS = "com.imsi.regStauts";
        
        public final static String NMS_DB_FULL_STATUS = "com.isms.dbfullstatus";

        public final static String NMS_INTENT_AIRPLANE_MODE = "com.isms.airplanemode";

        public final static String NMS_AIRPLANE_MODE = "airplaneMode";

        public final static String NMS_ACTIVE = "active_type";
        
        public final static String NMS_MMS_RESTART_ACTION = "com.android.mms.restart";

    }
}
