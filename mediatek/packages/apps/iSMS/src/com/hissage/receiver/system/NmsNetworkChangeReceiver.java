package com.hissage.receiver.system;

import com.hissage.jni.engineadapter;
import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.log.NmsLog;
import com.hissage.util.statistics.NmsNetTraffic;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

public class NmsNetworkChangeReceiver extends BroadcastReceiver {
    public static final String TAG = "NmsNetworkChangeReceiver";
    public static NmsNetworkChangeReceiver mInstance = null;
    private boolean mFirstCall = true;
    public int netType = NmsNetTraffic.UNKONW_NET_TYPE ;
    public int connSimId = NmsNetTraffic.NO_SIM_CONNECT ;
    

    public static NmsNetworkChangeReceiver getInstance() {
        if (mInstance == null) {
            mInstance = new NmsNetworkChangeReceiver();
        }
        return mInstance;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        try {
            ConnectivityManager cm = (ConnectivityManager) context
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            NmsLog.trace(TAG, "nms network change onReceive");
            if (cm != null) {
                NetworkInfo activeNetInfo = cm.getActiveNetworkInfo();
                if (activeNetInfo != null && activeNetInfo.isConnected()) {
                    if (!NmsUpgradeManager.mUpgradeStarted) {
                        NmsLog.trace(TAG, "nms upgrade from netwotk change");
                        NmsUpgradeManager.start(false);
                    } else {
                        NmsUpgradeManager.handleNetworkEvent();
                         
                       netType = NmsNetTraffic.getInstance().getNetType() ;
                       connSimId = NmsNetTraffic.getInstance().getNetConnSimId() ;
                       engineadapter.get().nmsSetTraffic(netType, connSimId) ;
                       NmsLog.trace("TAG", "send nettype and connectsimid to engine .netType:"+netType+"connSimId:"+connSimId) ;
                        
                    }
                } else {
                    NmsLog.trace(TAG, "nms network change onReceive but network is inavailable");
                }
            }else{
                NmsLog.trace(TAG, "nms network change onReceive cm is null");
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "NmsNetworkChangeReceiver get Execption: " + e.toString());
        }
    }
}
