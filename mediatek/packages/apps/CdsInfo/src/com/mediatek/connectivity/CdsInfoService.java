package com.mediatek.connectivity;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.net.INetworkStatsService;
import android.util.Log;

public class CdsInfoService extends Service {
    private static final String TAG = "CdsInfoService";
    private static final String BOOT = "boot";
    private static final String DATAFILE ="datafile";

    private static final String FW_ENABLED = "fw_enable";
    private static final String FW_GCFSTK = "fw_gcf_stk";

    private static final int        MDLOGGER_PORT = 30017;
    private static final int        ADB_PORT = 5037;
    private static final int        HTTP_PORT = 80;
    private static final String     ICMP_PRTO = "icmp";
    private static final String     LOCAL_INTERFACE = "lo";

    private Handler mHandler = new Handler();
    private INetworkManagementService mNetd;

    private boolean mPowerUp = false;

    public void onCreate() {
        Log.d(TAG, "onCreate");

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNetd = INetworkManagementService.Stub.asInterface(b);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent.getAction();

        Log.d(TAG, "onStartCommand");

        SharedPreferences sp = this.getSharedPreferences(DATAFILE, 0);
        boolean isEnabled = sp.getBoolean(FW_ENABLED, false);
        if(!isEnabled){
            Log.e(TAG, "No action");
            stopSelf();
            return Service.START_NOT_STICKY;
        }

        try {
            if(mNetd.isFirewallEnabled()) {
               mHandler.postDelayed(runFirewallRule, 3000);
            }else{
               stopSelf();
            }
        } catch(Exception e) {
            e.printStackTrace();
        }

        return Service.START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        //TODO for communication return IBinder implementation
        return null;
    }

    private void setFirewallEnabled(boolean enabled) {
        if(mNetd == null) {
            Log.e(TAG, "INetworkManagementService is null");
            return;
        }


        Log.d(TAG, "set firewall:" + enabled);

        try {        
        
            //If enable is true, enable before iptable rule configuration
            if(enabled) {
                mNetd.setFirewallEnabled(true);
                mNetd.setFirewallEgressProtoRule(ICMP_PRTO, true);
                mNetd.setFirewallInterfaceRule(LOCAL_INTERFACE, true);
            }

            mNetd.setFirewallEgressDestRule("", MDLOGGER_PORT, enabled);
            mNetd.setFirewallEgressDestRule("", ADB_PORT, enabled);

            //if enable is false, disable after iptable rule configuration
            if(!enabled) {
                mNetd.setFirewallEgressProtoRule(ICMP_PRTO, true);
                mNetd.setFirewallInterfaceRule(LOCAL_INTERFACE, false);
                mNetd.setFirewallEnabled(false);
            }
        } catch(Exception e) {
            e.printStackTrace();
        }

    }

    private Runnable runFirewallRule = new Runnable() {
        public void run() {
            setFirewallEnabled(false);
            stopSelf();
        }
    };



}