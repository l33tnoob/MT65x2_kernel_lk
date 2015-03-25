package com.mediatek.regionalphone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import com.mediatek.xlog.Xlog;

public class SIMChangedBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = Common.LOG_TAG;

    // Broadcast Action: The sim card state has changed.
    private static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";
    // The extra data for broacasting intent INTENT_ICC_STATE_CHANGE
    private static final String INTENT_KEY_ICC_STATE = "ss";
    // LOADED means all ICC records, including IMSI, are loaded
    private static final String INTENT_VALUE_ICC_LOADED = "LOADED";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Xlog.d(TAG, "SIMChangedBroadcastReceiver " + action.toString());
        if (action != null && action.equals(ACTION_SIM_STATE_CHANGED)) {
            String newState = intent.getStringExtra(INTENT_KEY_ICC_STATE);
            Xlog.d(TAG, "SIMChangedBroadcastReceiver(), monitor SIM state change, new state="
                    + newState);
            if (newState.equals(INTENT_VALUE_ICC_LOADED) && RPMUtils.isStartRPMService()) {
                Intent rps_intent = new Intent();
                rps_intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                rps_intent.setAction(RegionalPhoneService.ACTION);
                rps_intent.putExtra(Common.SIM_LOADED, true);
                context.startService(rps_intent);
            }
        }
    }

}
