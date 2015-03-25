package com.mediatek.deviceregister;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;

public class RegisterFeasibleReceiver extends BroadcastReceiver {

    private static final String SUB_TAG = Const.TAG + "RegisterFeasibleReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {

        Log.d(SUB_TAG, "onReceive");
        if (intent != null) {
            String action = intent.getAction();
            if (Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE.equalsIgnoreCase(action)) {
                SharedPreferences mSharedPrf = context.getSharedPreferences(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                        Context.MODE_PRIVATE);
                boolean haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                        Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
                if (!haveReceived) {
                    mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, true).commit();
                    // check the switch
                    if (isSwitchOpen()) {
                        // start service
                        Log.d(SUB_TAG, "start service");
                        intent.setClass(context, RegisterService.class);
                        context.startService(intent);
                    } else {
                        Log.d(SUB_TAG, "kill process.");
                        android.os.Process.killProcess(android.os.Process.myPid());
                    }
                } else {
                    Log.d(SUB_TAG, "It's not the first time get the feasible broadcast, do nothing.");
                }
            } else {
                Log.d(SUB_TAG, "action is not valid." + action);
            }
        } else {
            Log.d(SUB_TAG, "intent is not valid.");
        }
    }

    private boolean isSwitchOpen() {
        IBinder binder = ServiceManager.getService("DmAgent");
        if (binder == null) {
            Log.e(SUB_TAG, "get DmAgent fail! binder is null!");
            return false;
        }
        DmAgent mAgent = DmAgent.Stub.asInterface(binder);
        if (mAgent == null) {
            Log.e(SUB_TAG, "get switch value failed, DmAgent is null!");
            return false;
        }
        int switchValue = 1;
        try {
            byte[] ctaBytes = mAgent.getRegisterSwitch();
            if (ctaBytes != null && ctaBytes.length > 0) {
                switchValue = Integer.parseInt(new String(ctaBytes));
            }
        } catch (RemoteException e) {
            Log.e(SUB_TAG, "get cta cmcc switch failed, readCTA failed!");
            e.printStackTrace();
        } catch (NumberFormatException e) {
            Log.e(SUB_TAG, "NumberFormatException:" + e.getMessage());
        }
        Log.i(SUB_TAG, "Get the switch value = " + switchValue);
        if (switchValue == 1) {
            Log.i(SUB_TAG, "The switch is opened.");
            return true;
        } else {
            Log.i(SUB_TAG, "The switch is not opened.");
            return false;
        }

    }

}
