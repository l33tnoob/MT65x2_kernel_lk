package com.mediatek.common.perfservice;

//import android.os.Bundle;

public interface IPerfServiceWrapper {

    public static final int SCN_NONE       = 0;
    public static final int SCN_APP_SWITCH = 1; /* apply for both launch/exit */
    public static final int SCN_APP_ROTATE = 2;
    public static final int SCN_SW_CODEC   = 3;
    public static final int SCN_SW_CODEC_BOOST = 4;
    public static final int SCN_APP_TOUCH      = 5;
    
    public static final int STATE_PAUSED    = 0;
    public static final int STATE_RESUMED   = 1;
    public static final int STATE_DESTROYED = 2;
    public static final int STATE_DEAD      = 3;

    public void boostEnable(int scenario);
    public void boostDisable(int scenario);
    public void boostEnableTimeout(int scenario, int timeout);
    public void notifyAppState(String packName, String className, int state);

    public int  userReg(int scn_core, int scn_freq);
    public void userUnreg(int handle);
    public void userEnable(int handle);
    public void userEnableTimeout(int handle, int timeout);
    public void userDisable(int handle);
    public void userResetAll();
    public void userDisableAll();
}
