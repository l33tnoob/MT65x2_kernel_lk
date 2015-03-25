package com.mediatek.systemupdate;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.xlog.Xlog;

public class RefreshReceiver extends BroadcastReceiver {

    private static final String TAG = "RefreshReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Xlog.v(TAG, "[onReceiver] " + intent.getAction());

        DownloadInfo downloadInfo = DownloadInfo.getInstance(context.getApplicationContext());
        downloadInfo.setIfNeedRefresh(true);
        (new NotifyManager(context)).clearNotification(NotifyManager.NOTIFY_NEW_VERSION);

    }

}