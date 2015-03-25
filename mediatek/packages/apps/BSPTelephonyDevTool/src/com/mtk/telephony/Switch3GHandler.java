package com.mtk.telephony;

import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.common.telephony.ITelephonyEx;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.gemini.GeminiPhone;

public class Switch3GHandler {
    private ITelephonyEx mTelephonyEx;
    private ProgressDialog mProgressDialog;
    private Runnable mUpdateUIRunnable;

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TelephonyIntents.EVENT_PRE_3G_SWITCH.equals(action)) {
                mUpdateUIRunnable.run();
                mProgressDialog.show();
            } else if (TelephonyIntents.EVENT_3G_SWITCH_START_MD_RESET.equals(action)) {
                mUpdateUIRunnable.run();
            } else if (TelephonyIntents.EVENT_3G_SWITCH_DONE.equals(action)) {
                mUpdateUIRunnable.run();
                mProgressDialog.dismiss();
            }
        }
    };

    public Switch3GHandler(Context context, ITelephonyEx iTelephonyEx, Runnable updateUIRunnable) {
        mTelephonyEx = iTelephonyEx;
        mUpdateUIRunnable = updateUIRunnable;
        mProgressDialog = new ProgressDialog(context);
        mProgressDialog.setCancelable(false);
        mProgressDialog.setTitle(R.string.perform_switch);
        mProgressDialog.setMessage(context.getText(R.string.please_wait));

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.EVENT_PRE_3G_SWITCH);
        intentFilter.addAction(TelephonyIntents.EVENT_3G_SWITCH_START_MD_RESET);
        intentFilter.addAction(TelephonyIntents.EVENT_3G_SWITCH_DONE);
        context.registerReceiver(mBroadcastReceiver, intentFilter);
    }

    public void set3GCapabilitySIM(int simId) {
        try {
            mTelephonyEx.set3GCapabilitySIM(simId);
        } catch(RemoteException e) {
            e.printStackTrace();
        }
    }

    public int get3GCapabilitySIM() {
        int sim3G = 0;
        try {
            sim3G = mTelephonyEx.get3GCapabilitySIM();
        } catch(RemoteException e) {
            e.printStackTrace();
        }
        return sim3G;
    }
}
