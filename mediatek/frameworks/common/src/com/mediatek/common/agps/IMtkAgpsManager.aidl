package com.mediatek.common.agps;

import com.mediatek.common.agps.MtkAgpsCdmaProfile;
import com.mediatek.common.agps.MtkAgpsProfile;
import com.mediatek.common.agps.MtkAgpsConfig;
import android.os.Bundle;

interface IMtkAgpsManager {

    //=============version 2 API ==================
    void enable();
    void disable();
    boolean getStatus();
    
    void setConfig(in MtkAgpsConfig c);
    MtkAgpsConfig getConfig();
    
    void setProfile(in MtkAgpsProfile p);
    MtkAgpsProfile getProfile();
    
    void setMode(int mode);
    int getMode();
    
    void setNiEnable(boolean enable);
    void setUpEnable(boolean enable);
    void setCpEnable(boolean enable);
    void setRoamingEnable(boolean enable);
    
    boolean getNiStatus();
    boolean getUpStatus();
    boolean getCpStatus();
    boolean getRoamingStatus();
    
    int extraCommand(String command, in Bundle extra);
    void niUserResponse(int sessionId, int response);
    
    //==================== new for SUPL2.0 ====================
    void startSILR();
    void startSIPeiodic();
    void startSIAreaEvent();
    void stopTriggeredSession();

    //The size of msisdnList shouldn't be over 10
    //The length of each msisdn in the msisdnList shouldn't be over 31
    void setSI3PartyMsisdn(in String[] msisdnList);
    //The length of the msisdn shouldn't be over 31
    void setSILRMsisdn(in String msisdn);

    String[] getSI3PartyMsisdn();
    String getSILRMsisdn();
    //==================== new for CDMA ====================
    void setCdmaProfile(in MtkAgpsCdmaProfile p);
    MtkAgpsCdmaProfile getCdmaProfile();
    
}
