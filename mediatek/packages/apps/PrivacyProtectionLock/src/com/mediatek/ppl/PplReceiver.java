package com.mediatek.ppl;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Trivial receiver to bring the service up.
 */
public class PplReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        // bring the service up
        intent.setClass(context, PplService.class);
        context.startService(intent);
    }

}
