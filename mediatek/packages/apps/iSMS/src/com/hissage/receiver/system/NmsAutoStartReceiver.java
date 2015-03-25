package com.hissage.receiver.system;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.hissage.service.NmsAutoStartWhenReady;
import com.hissage.service.NmsService;
import com.hissage.util.preference.NmsPreferences;

public class NmsAutoStartReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        NmsPreferences.initPreferences(context);
        // if(CommonUtils.getSDCardStatus()){
        // NmsService.bActiveNet = false;
        Intent i = new Intent(context, NmsService.class);
        // context.startService(i);
        NmsAutoStartWhenReady.getInstance(context, i).startEngineWhenSimCardReady();
        // }
    }
}
