package com.hissage;

import android.content.Context;
import android.content.Intent;

import com.google.android.gcm.GCMBaseIntentService;
import com.hissage.pn.hpnsReceiver;
import com.hissage.util.log.NmsLog;

public class GCMIntentService extends GCMBaseIntentService {

    private static final String TAG = "GCMIntentService";

    // Google API project id registered to use GCM.
    public static final String SENDER_ID = "61779891346";

    public GCMIntentService() {
        super(SENDER_ID);
    }

    @Override
    protected void onError(Context arg0, String arg1) {
        NmsLog.error(TAG, "GCM: onError. String:" + arg1);
        handlerError();
    }

    @Override
    protected void onMessage(Context arg0, Intent arg1) {
        NmsLog.trace(TAG, "GCM: onMessage");

        Intent intent = new Intent();
        intent.setAction(hpnsReceiver.action_receive);
        sendBroadcast(intent);
    }

    @Override
    protected void onRegistered(Context arg0, String arg1) {
        NmsLog.trace(TAG, "GCM: onRegistered. regId:" + arg1);

        Intent intent = new Intent();
        intent.putExtra("registration_id", arg1);
        intent.putExtra("code", hpnsReceiver.HPNS_CODE_SUCCESS);
        intent.setAction(hpnsReceiver.action_registration);
        sendBroadcast(intent);
    }

    @Override
    protected boolean onRecoverableError(Context context, String errorId) {
        NmsLog.error(TAG, "GCM: onRecoverableError. errorId:" + errorId);
        handlerError();
        return false;
        // return super.onRecoverableError(context, errorId);
    }

    @Override
    protected void onUnregistered(Context arg0, String arg1) {
        NmsLog.warn(TAG, "GCM: onUnregistered. String:" + arg1);
        handlerError();
    }

    private void handlerError() {
        Intent intent = new Intent();
        intent.setAction(hpnsReceiver.action_gcmError);
        sendBroadcast(intent);
    }

}
