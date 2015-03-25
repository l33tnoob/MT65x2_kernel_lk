
package com.mediatek.systemupdate;

import java.io.File;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class SystemUpdateUpgradeReceiver extends BroadcastReceiver {

    static final String TAG = "SystemUpdate/SystemUpdateUpgradeReceiver";

    private static final String GOOGLE_OTA_PRE_FILE_PATH = "/data/data/com.mediatek.systemupdate/shared_prefs/googleota.xml";
    private static final String SYSTEM_UPDATE_PRE_FILE_PATH = "/data/data/com.mediatek.systemupdate/shared_prefs/system_update.xml";

    @Override
    public void onReceive(Context context, Intent intent) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                File otaFile = new File(GOOGLE_OTA_PRE_FILE_PATH);
                boolean otaFileExist = otaFile.exists();
                Log.d(TAG, "googleota.xml exists " + otaFileExist);
                if (otaFileExist) {
                    File updateFile = new File(SYSTEM_UPDATE_PRE_FILE_PATH);
                    boolean updateFileExist = updateFile.exists();
                    Log.d(TAG, "system_update.xml exists " + updateFileExist);
                    if (updateFileExist) {
                        Util.deleteFile(GOOGLE_OTA_PRE_FILE_PATH);
                    } else {
                        boolean renameResult = otaFile.renameTo(new File(
                                SYSTEM_UPDATE_PRE_FILE_PATH));
                        Log.d(TAG, "rename result :" + renameResult);
                    }
                }
            }
        }).start();
    }

}
