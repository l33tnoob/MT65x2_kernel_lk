package com.mediatek.common.perfservice;

interface IPerfService {

    void boostEnable(int scenario);
    void boostDisable(int scenario);
    void boostEnableTimeout(int scenario, int timeout);
    void notifyAppState(String packName, String className, int state);

    int  userReg(int scn_core, int scn_freq);
    void userUnreg(int handle);
    void userEnable(int handle);
    void userEnableTimeout(int handle, int timeout);
    void userDisable(int handle);
    void userResetAll();
    void userDisableAll();
}
