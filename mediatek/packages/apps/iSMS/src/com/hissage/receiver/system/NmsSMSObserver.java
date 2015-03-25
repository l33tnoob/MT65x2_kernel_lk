package com.hissage.receiver.system;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.hissage.service.NmsAutoStartWhenReady;
import com.hissage.service.NmsService;
import com.hissage.util.log.NmsLog;

public class NmsSMSObserver extends BroadcastReceiver {
    private static final String TAG = "NmsSMSObserver";
    private static final String mAction[] = new String[] {
            "android.provider.Telephony.SMS_RECEIVED", "android.provider.Telephony.SMS_REJECTED",
            "android.provider.Telephony.WAP_PUSH_RECEIVED",
            "android.intent.action.DATA_SMS_RECEIVED" };

    @Override
    public void onReceive(Context context, Intent intent) {
        String a = intent.getAction();
        if (mAction[0].equals(a) || mAction[1].equals(a) || mAction[2].equals(a)
                || mAction[3].equals(a)) {
            if (!NmsService.bCEngineRunflag) {
                Intent i = new Intent(context, NmsService.class);
                // if (NmsConfig.isDBInitDone) {
                // i.putExtra(NmsService.pushEngineConnecd, true);
                // }
                NmsLog.trace(TAG, "start engine service by NmsSMSObserver.");
                // context.startService(i);
                NmsAutoStartWhenReady.getInstance(context, i).startEngineWhenSimCardReady();
            } else {
                NmsLog.trace(TAG, "enging is running, so NmsSMSObserver ignore this action.");
            }
        }
    }
}