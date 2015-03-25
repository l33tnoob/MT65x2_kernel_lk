package com.mediatek.schpwronoff;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;

import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class AlarmReceiverService extends IntentService {
    private static final String TAG = "AlarmReceiverService";
    private static final String ACTION_BROADCAST = "broadcast_receiver";
    private static final String SCHPWRS_DB_PATH = "data/data/com.mediatek.schpwronoff/databases/schpwrs.db";
    private static final String TEMP_DB_PATH = "/data/schpwrs.db";

    private static Context mContext;

    public AlarmReceiverService() {
        super("AlarmReceiverService");
    }

    public static void processBroadcastIntent(Context context, Intent broadcastIntent) {
        // Launch the Service
        mContext = context;
        Intent i = new Intent(context, AlarmReceiverService.class);
        i.setAction(ACTION_BROADCAST);
        i.putExtra(Intent.EXTRA_INTENT, broadcastIntent);
        Xlog.d(TAG, "startService");
        context.startService(i);
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Xlog.d(TAG, "onHandleIntent");

        final String action = intent.getAction();
        if (!ACTION_BROADCAST.equals(action)) {
            return;
        }

        final Intent broadcastIntent = intent.getParcelableExtra(Intent.EXTRA_INTENT);
        final String broadcastAction = broadcastIntent.getAction();
        Xlog.i(TAG, "action= " + broadcastAction);
        if (Intent.ACTION_BOOT_COMPLETED.equals(broadcastAction)) {
            // ALPS00448092. 
            boolean b = copyDbFileFromDataPath();
            Xlog.w(TAG, "copy db file result " + b);
            Alarms.saveSnoozeAlert(this, -1, -1);
            Alarms.disableExpiredAlarms(this);
            Alarms.setNextAlert(this);
        } else if ("android.intent.action.LAUNCH_POWEROFF_ALARM".equals(broadcastAction)){
            // @ CL: 2051498
            if (bootFromPoweroffAlarm()) {
                Alarms.setNextAlert(this);
            }
        } else {
            Alarms.setNextAlert(this);
        }
    }

    // get the boot reason
    private boolean bootFromPoweroffAlarm() {
        String bootReason = SystemProperties.get("sys.boot.reason");
        boolean ret = (bootReason != null && bootReason.equals("1")) ? true : false;
        return ret;
    }

    private boolean copyDbFileFromDataPath() {
        Xlog.w(TAG, "copyDbFileFromDataPath");
        File tempDbDirFile = new File(TEMP_DB_PATH);
        if (!tempDbDirFile.exists()) {
            Xlog.w(TAG, "/data/schpwrs.db does not exist");
            return false;
        }
        File dbPathFile = new File("/data/data/com.mediatek.schpwronoff/databases/");
        if (!dbPathFile.exists() || !dbPathFile.isDirectory()) {
            dbPathFile.mkdirs();
        }
        File schPwrsDbFile = new File(SCHPWRS_DB_PATH);
        FileInputStream fis = null;
        FileOutputStream fos = null;
        try {
            fis = new FileInputStream(tempDbDirFile);
            fos = new FileOutputStream(schPwrsDbFile);
        } catch (FileNotFoundException e) {
            Xlog.w(TAG, "FileNotFoundException " + e.getMessage());
            return false;
        }
        byte[] buffer = new byte[1024];
        int length = 0;
        try {
            while ((length = fis.read(buffer)) > 0) {
                fos.write(buffer, 0, length);
            }
            fos.flush();
            fos.close();
            fis.close();
        } catch (IOException e) {
            Xlog.w(TAG, "IOException " + e.getMessage());
            return false;
        }
        if (!tempDbDirFile.delete()) {
            Xlog.w(TAG, "delete temp db file failed.");
        }
        return true;
    }
}
