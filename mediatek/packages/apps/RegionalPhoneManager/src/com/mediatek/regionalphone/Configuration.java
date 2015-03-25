package com.mediatek.regionalphone;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.SystemProperties;
import com.mediatek.xlog.Xlog;

public class Configuration {
    // preference file
    private static final String PREFER_FILE = "mcc_mnc";

    public static final String MCC_MNC = "mcc_mnc";
    public static final String INVALID_MCC_MNC = "invalid_mcc_mnc";

    public static final String DEFAULT_MCC_MNC = "";
    public static final String DEFAULT_INVALID_MCC_MNC = "";

    private static final String PROP_GSM_SIM_OPERATOR_NUMERIC = "gsm.sim.operator.numeric";
    private static final String PROP_PERSIST_BOOTANIM_MNC = "persist.bootanim.mnc";

    private static Configuration sInstance = new Configuration();
    private Context mContext = null;
    private Configuration() {
    }

    public void init(Context context) {
        mContext = context;
    }

    public static Configuration getInstance() {
        return sInstance;
    }

    /**
     * read MCC+MNC of valid sim in mcc_mnc.xml,
     * 
     * @return default is ""
     */
    public String readMCCMNC() {
        SharedPreferences preferences = mContext.getSharedPreferences(PREFER_FILE,
                Context.MODE_PRIVATE);
        return preferences.getString(MCC_MNC, DEFAULT_MCC_MNC);
    }

    /**
     * save MCC+MNC of valid sim to mcc_mnc.xml.
     * @param MCCMNC
     */
    public void saveMCCMNC(String MCCMNC) {
        SharedPreferences preferences = mContext.getSharedPreferences(PREFER_FILE,
                Context.MODE_PRIVATE);
        preferences.edit().putString(MCC_MNC, MCCMNC).commit();
    }

    /**
     * read MCC+MNC of invalid sim in mcc_mnc.xml,
     * 
     * @return default is ""
     */
    public String readInvalidMCCMNC() {
        SharedPreferences preferences = mContext.getSharedPreferences(PREFER_FILE,
                Context.MODE_PRIVATE);
        return preferences.getString(INVALID_MCC_MNC, DEFAULT_MCC_MNC);
    }

    /**
     * save MCC+MNC of invalid sim to mcc_mnc.xml.
     * @param MCCMNC
     */
    public void saveInvalidMCCMNC(String invalidMCCMNC) {
        SharedPreferences preferences = mContext.getSharedPreferences(PREFER_FILE,
                Context.MODE_PRIVATE);
        preferences.edit().putString(INVALID_MCC_MNC, invalidMCCMNC).commit();
    }

    /**
     * get new MCC + MNC when there is a sim card insert.
     *
     * @return "" if MCC+MNC is null
     */
    public String readMCCMNCFromProperty() {
        // try bootanim property firstly
        String MCCMNC = SystemProperties.get(PROP_PERSIST_BOOTANIM_MNC);
        if (MCCMNC != null && MCCMNC.length() > 4) {
            Xlog.d(Common.LOG_TAG, "read mcc mnc property from boot anim: " + PROP_PERSIST_BOOTANIM_MNC);
            return MCCMNC;
        }
        // failed to read bootanim property, then try other property
        MCCMNC = SystemProperties.get(PROP_GSM_SIM_OPERATOR_NUMERIC);
        if (MCCMNC != null && MCCMNC.length() > 4) {
            Xlog.d(Common.LOG_TAG, "read mcc mnc property from " + PROP_GSM_SIM_OPERATOR_NUMERIC);
            return MCCMNC;
        }

        Xlog.d(Common.LOG_TAG, "failed to read mcc mnc from property");
        return "";
    }

    /**
     * get time stamp of system.
     * @return
     */
    public String getTimestamp() {
        return "" + System.currentTimeMillis();
    }
}
