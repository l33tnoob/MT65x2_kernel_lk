package com.mediatek.batterywarning;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

import com.mediatek.xlog.Xlog;

public class BatteryWarningReceiver extends BroadcastReceiver {
    private static final String ACTION_IPO_BOOT = "android.intent.action.ACTION_BOOT_IPO";
    private static final String ACTION_BATTERY_WARNING = "android.intent.action.BATTERY_WARNING";
    private static final String TAG = "BatteryWarningReceiver";
    private static final String SHARED_PREFERENCES_NAME = "battery_warning_settings";
    private Context mContext;
    
    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Xlog.d(TAG,  "action = " + action);
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Xlog.d(TAG, action + " clear battery_warning_settings shared preference");
            SharedPreferences.Editor editor = getSharedPreferences().edit();
            editor.clear();
            editor.apply();
        } else if (ACTION_BATTERY_WARNING.equals(action)) {
        	Xlog.d(TAG, action + " start activity according to shared preference");
        	int type = intent.getIntExtra("type", -1);
        	Xlog.d(TAG, "type = " + type);
            type = (int)(Math.log(type) / Math.log(2));
            boolean showDialogFlag = getSharedPreferences().getBoolean(
            		mContext.getString(BatteryWarningActivity.sWarningTitle[type]), true);
            Log.d(TAG, "type = " + type + "showDialogFlag = " + showDialogFlag);
            if (showDialogFlag) {
                Intent activityIntent = new Intent();
                activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
                activityIntent.setComponent(new ComponentName("com.mediatek.batterywarning", 
                        "com.mediatek.batterywarning.BatteryWarningActivity"));
                activityIntent.putExtra("type", type);
                mContext.startActivity(activityIntent);
            }
        }
    }
    
    private SharedPreferences getSharedPreferences() {
        return mContext.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }
}
