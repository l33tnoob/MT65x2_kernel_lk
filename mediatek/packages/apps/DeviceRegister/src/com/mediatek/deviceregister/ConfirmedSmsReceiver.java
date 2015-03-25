package com.mediatek.deviceregister;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class ConfirmedSmsReceiver extends BroadcastReceiver {

    private static final String SUB_TAG = Const.TAG + "ConfirmedSmsReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(SUB_TAG, "onReceive");
        if (intent != null) {
            String action = intent.getAction();
            if (action.equalsIgnoreCase(Const.ACTION_CT_CONFIRMED_MESSAGE)) {
                // start service
                Log.d(SUB_TAG, "start service");
                intent.setClass(context, RegisterService.class);
                context.startService(intent);
            } else {
                Log.d(SUB_TAG, "action is not valid." + action);
            }

        } else {
            Log.d(SUB_TAG, "intent is null.");
        }

    }
}