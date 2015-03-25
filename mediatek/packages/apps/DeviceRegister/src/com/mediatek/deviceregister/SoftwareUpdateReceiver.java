package com.mediatek.deviceregister;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;

import java.io.File;

public class SoftwareUpdateReceiver extends BroadcastReceiver {

    private static final String SUB_TAG = Const.TAG + "SoftwareUpdateReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(SUB_TAG, "onReceive");

        File file = context.getSharedPrefsFile(Const.KEY_RECEIVED_FEASIBLE_BROADCAST);
        if (file.exists()) {
            resetRegisterFlag();
        }
        Log.d(SUB_TAG, "kill process.");
        android.os.Process.killProcess(android.os.Process.myPid());

    }

    private boolean resetRegisterFlag() {
        Log.d(SUB_TAG, "reset register flag");
        IBinder binder = ServiceManager.getService("DmAgent");
        if (binder == null) {
            Log.i(SUB_TAG, "get DmAgent fail, binder is null!");
            return false;
        }
        DmAgent agent = DmAgent.Stub.asInterface(binder);

        if (agent != null) {
            Log.d(SUB_TAG, "reset register flag to 0");
            try {
                agent.setRegisterFlag("0".getBytes(), "0".length());
                return true;
            } catch (RemoteException re) {
                Log.e(SUB_TAG, "remote exception when setRegisterFlag!" + re);
            }
        } else {
            Log.d(SUB_TAG, "get IMSI failed, DmAgent is null!");
        }

        return false;
    }

}
