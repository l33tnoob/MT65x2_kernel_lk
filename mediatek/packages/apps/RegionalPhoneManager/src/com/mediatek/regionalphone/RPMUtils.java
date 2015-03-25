package com.mediatek.regionalphone;

import java.io.File;
import com.mediatek.xlog.Xlog;

public class RPMUtils {
    private static final String TAG = Common.LOG_TAG;
    private static final String DB_SYSTEM_PATH = "/system/etc/regionalphone/";
    private static final String DB_CUSTOMER_PATH = "/custom/etc/regionalphone/";
    private static final String DB_NAME = "regionalphone.db";

    public static boolean isStartRPMService() {

        // This means there is no database.
        if (!new File(DB_CUSTOMER_PATH + DB_NAME).exists()
                && !new File(DB_SYSTEM_PATH + DB_NAME).exists()) {
            Xlog.d(TAG, DB_NAME + " not exist, can't start regionalphonemanager.");
            return false;
        }

        // If mcc_mnc.xml exist, that means the phone has been insert a SIM card before.
        if (new File(Common.PREFE_FILE).exists()) {
            String mcc_mnc = Configuration.getInstance().readMCCMNC();
            // This means the phone has been insert a valid SIM card before.
            if (!"".equals(mcc_mnc)) {
                return false;
            }

            String invalid_mcc_mnc = Configuration.getInstance().readInvalidMCCMNC();
            // This means the phone has been insert an invalid SIM card before.
            if (!"".equals(invalid_mcc_mnc)) {
                String sm_mcc_mnc = Configuration.getInstance().readMCCMNCFromProperty();
                // This means this is also an invalid SIM card.
                if(sm_mcc_mnc.equals(invalid_mcc_mnc)) {
                    return false;
                }
            }
        }

        return true;
    }
}
