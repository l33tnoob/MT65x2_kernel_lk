package com.mediatek.deviceregister;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {
    private static final String SUB_TAG = Const.TAG + "BootReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(SUB_TAG, "onReceive");
        if (intent != null) {
            String action = intent.getAction();
            if (action.equalsIgnoreCase(Const.ACTION_BOOTCOMPLETED)) {
                SharedPreferences mSharedPrf = context.getSharedPreferences(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                        Context.MODE_PRIVATE);
                mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, false).commit();
                Log.d(SUB_TAG, "update preference: set have received feasible broadcast false");
            } else {
                Log.d(SUB_TAG, "action is not valid." + action);
            }
        } else {
            Log.d(SUB_TAG, "intent is not valid.");
        }
        Log.d(SUB_TAG, "kill process.");
        android.os.Process.killProcess(android.os.Process.myPid());        
    }

}
