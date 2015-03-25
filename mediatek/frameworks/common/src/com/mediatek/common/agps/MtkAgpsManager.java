package com.mediatek.common.agps;

import android.os.Bundle;

public interface MtkAgpsManager {
    
    // Notification and Verification Types
    public static final int AGPS_NOTIFY_NONE               = 0;  // No notification & no verification */
    public static final int AGPS_NOTIFY_ONLY               = 1;  // Notification Only */
    public static final int AGPS_NOTIFY_ALLOW_NO_ANSWER    = 2;  // Notification and verification, Allow on no answer. (If no answer is received from the SET User, the SET will assume that user consent has been granted and will proceed) */
    public static final int AGPS_NOTIFY_DENY_NO_ANSWER     = 3;  // Notification and verification, Deny on no answer. (If no answer is received from the SET User, the SET will assume that user consent has been denied and will abort) */
    public static final int AGPS_NOTIFY_PRIVACY            = 4;  // Privacy-Override. (Is used for preventing notification and verification without leaving any traces of a performed position fix or position fix attempt in terms of log files etc. on the SET). */
    public static final int AGPS_NOTIFY_CNT                = 5;

    //User's response types
    public static final int AGPS_NI_RESPONSE_NO_RESP    = 0;
    public static final int AGPS_NI_RESPONSE_ACCEPT     = 1;
    public static final int AGPS_NI_RESPONSE_DENY       = 2;
    
    //A-GPS modes
    public static final int AGPS_MODE_MA = 0;
    public static final int AGPS_MODE_MB = 1;
    public static final int AGPS_MODE_STANDALONE = 2;

    //Update the status to shared preference
    public static final String AGPS_PROFILE_UPDATE = "com.mediatek.agps.PROFILE_UPDATED";
    public static final String AGPS_STATUS_UPDATE  = "com.mediatek.agps.STATUS_UPDATED";
    public static final String AGPS_DISABLE_UPDATE = "com.mediatek.agps.DISABLE_UPDATED";
    public static final String AGPS_OMACP_PROFILE_UPDATE = "com.mediatek.agps.OMACP_UPDATED";
    public static final String AGPS_MESSAGE         = "com.mediatek.agps.AGPS_MESSAGE";
    public static final String AGPS_DIALOG_MESSAGE  = "com.mediatek.agps.AGPS_DIALOG_MESSAGE";

    public static final int EPO_EVENT_SERVER_CONNECT_BEGIN  = 22;
    public static final int EPO_EVENT_SERVER_CONNECTED      = 23;
    public static final int EPO_EVENT_SERVER_DATA_RECEIVED  = 24;
    public static final int EPO_EVENT_SERVER_DISCONNECTED   = 25;
    public static final int EPO_EVENT_SERVER_DATA_VALID     = 26;
    public static final String EPO_EVENT        = "com.mediatek.agps.EPO_EVENT";

    //AREA_EVENT_TYPE
    public static final int AREA_EVENT_TYPE_ENTERING    = 0;
    public static final int AREA_EVENT_TYPE_INSIDE      = 1;
    public static final int AREA_EVENT_TYPE_OUTSIDE     = 2;
    public static final int AREA_EVENT_TYPE_LEAVING     = 3;

    //TARGET_AREA_TYPE
    public static final int TARGET_AREA_TYPE_CIRCULAR   = 0;
    public static final int TARGET_AREA_TYPE_ELLIPTICAL = 1;
    public static final int TARGET_AREA_TYPE_POLYGON    = 2;

    //LAT_SIGN
    public static final int LAT_SIGN_NORTH              = 0;
    public static final int LAT_SIGN_SOUTH              = 1;

    //POS_METHOD
    public static final int POS_METHOD_SET_ASSIST       = 0;
    public static final int POS_METHOD_SET_BASE         = 1;
    public static final int POS_METHOD_SET_ASSIST_PREF  = 2;
    public static final int POS_METHOD_SET_BASE_PREF    = 3;
    public static final int POS_METHOD_AUTO             = 4;
    public static final int POS_METHOD_ECID             = 5;
    public static final int POS_METHOD_NO_POSITION      = 6;
    public static final int POS_METHOD_HISTORICAL       = 7;
    public static final int POS_METHOD_SESSION_INFO     = 8;

    //SI_MODE
    public static final int SI_MODE_MA          = 0;
    public static final int SI_MODE_MB          = 1;
    public static final int SI_MODE_STANDALONE  = 2;

    //SET_ID
    public static final int SET_ID_MSISDN = 0;
    public static final int SET_ID_IPV4   = 1;
    public static final int SET_ID_CNT    = 2;

    //=============version 2 API ==================
    public void enable();
    public void disable();
    public boolean getStatus();
    public void setConfig(MtkAgpsConfig config);
    public MtkAgpsConfig getConfig();
    public void setProfile(MtkAgpsProfile profile);
    public MtkAgpsProfile getProfile();
    
    //refer to AGPS MODE
    public void setMode(int mode);
    public int getMode();
    public void setNiEnable(boolean enable);
    public void setUpEnable(boolean enable);
    public void setCpEnable(boolean enable);
    public void setRoamingEnable(boolean enable);
    public boolean getNiStatus();
    public boolean getUpStatus();
    public boolean getCpStatus();
    public boolean getRoamingStatus();
    public int extraCommand(String command, Bundle extra);
    public void niUserResponse(int sessionId, int response);
    //==================== new for SUPL2.0 ====================
    public void startSILR();
    public void startSIPeiodic();
    public void startSIAreaEvent();
    public void stopTriggeredSession();
    public void setSI3PartyMsisdn(String[] msisdnList);
    public void setSILRMsisdn(String msisdn);
    public String[] getSI3PartyMsisdn();
    public String getSILRMsisdn();
    //==================== new for CDMA ====================
    void setCdmaProfile(MtkAgpsCdmaProfile p);
    MtkAgpsCdmaProfile getCdmaProfile();
}
