package com.mediatek.engineermode.wifi;

import com.mediatek.engineermode.Elog;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;

public class WifiStateReceiver extends BroadcastReceiver {
    
    private static final String TAG = "WifiStateReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(intent.getAction())) {
            int state = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, 0);
            switch (state) {
            case WifiManager.WIFI_STATE_ENABLED:
                onWifiEnbled(context);
                break;
            case WifiManager.WIFI_STATE_DISABLED:
                onWifiDisabled(context);
                break;
            default:
                Elog.d(TAG, "Unhandled wifi state:" + state);
                break;
            }
        }

    }
    
    private void onWifiEnbled(Context context) {
        if (MtkCTIATestDialog.isWifiCtiaEnabled(context)) {
            MtkCTIATestDialog.initWifiCtiaOnEnabled(context);
            MtkCTIATestDialog.notifyCtiaEnabled(context);
        }
    }
    
    private void onWifiDisabled(Context context) {
        MtkCTIATestDialog.dismissCtiaEnabledNotify(context);
    }

}
