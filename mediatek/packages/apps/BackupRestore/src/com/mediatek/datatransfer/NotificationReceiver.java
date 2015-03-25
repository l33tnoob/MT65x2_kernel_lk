package com.mediatek.datatransfer;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class NotificationReceiver extends BroadcastReceiver {

    public static final String CLASS_TAG = "NotificationReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        if (Constants.ACTION_NEW_DATA_DETECTED.equals(intent.getAction())) {
            int type = intent.getIntExtra(Constants.NOTIFY_TYPE, 0);
            String folder = intent.getStringExtra(Constants.FILENAME);
            MyLogger.logD(CLASS_TAG,
                    " NotificationReceiver----->ACTION_NEW_DATA_DETECTED Extra:Folder = " + folder);
            String intentFilter = ((type == NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_LIST) ? NotifyManager.FP_NEW_DETECTION_INTENT_LIST
                    : NotifyManager.RESTORE_PERSONALDATA_INTENT);
            MyLogger.logD(CLASS_TAG,
                    " NotificationReceiver----->ACTION_NEW_DATA_DETECTED Extra:Type = "
                            + intentFilter);
            Intent contentIntent = new Intent(intentFilter);
            contentIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            if (type != NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_LIST && folder != null
                    && folder.length() > 0) {
                contentIntent.putExtra(Constants.FILENAME, folder);
            }
            context.startActivity(contentIntent);
            MyLogger.logD(CLASS_TAG, " NotificationReceiver----->start Activity");
        }
    }
}

