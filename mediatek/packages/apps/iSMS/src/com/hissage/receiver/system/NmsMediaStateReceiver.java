package com.hissage.receiver.system;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.hissage.jni.engineadapter;
import com.hissage.util.log.NmsLog;

public class NmsMediaStateReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (null == intent) {
            return;
        }
        String action = intent.getAction();
        if (action.equals(Intent.ACTION_MEDIA_EJECT) ||
        		action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {
//            Toast.makeText(context, R.string.STR_NMS_NO_SDCARD, Toast.LENGTH_LONG).show();
            engineadapter.get().nmsSetDiskStatus(1);
            NmsLog.destroy();
        } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
//            Toast.makeText(context, R.string.STR_NMS_MOUNT_SDCARD, Toast.LENGTH_LONG).show();
            engineadapter.get().nmsSetDiskStatus(0);
            NmsLog.init(context);
        }
    }
}
