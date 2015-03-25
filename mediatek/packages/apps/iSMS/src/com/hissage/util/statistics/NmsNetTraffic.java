package com.hissage.util.statistics;

import com.hissage.service.NmsService;
import com.hissage.util.log.NmsLog;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

public class NmsNetTraffic {
    Context mContext;
    public static final String TAG = "NmsNetTraffic";

    public static final int UNKONW_NET_TYPE = -1;
    public static final int NO_SIM_CONNECT = -2;
    public static final int WIFI = 1;
    public static final int GPRS = 2;

    public NmsNetTraffic(Context context) {
        if (null != context) {
            this.mContext = context;
        }
    }

    public static NmsNetTraffic getInstance() {
        return NmsService.netTraffic;

    }

    public int getNetType() {

        ConnectivityManager connMgr = (ConnectivityManager) mContext
                .getSystemService(mContext.CONNECTIVITY_SERVICE);

        NetworkInfo ni = connMgr.getActiveNetworkInfo();

        if (ni != null) {
            if (ni.getType() == ConnectivityManager.TYPE_WIFI) {

                return WIFI;
            } else if (ni.getType() == ConnectivityManager.TYPE_MOBILE) {

                return GPRS;
            }
        }

        return UNKONW_NET_TYPE;

    }

    public int getNetConnSimId() {

        ConnectivityManager mConnMgr = (ConnectivityManager) mContext
                .getSystemService(mContext.CONNECTIVITY_SERVICE);

        NetworkInfo mInfo = mConnMgr.getActiveNetworkInfo();
        if (null != mInfo) {
            return mInfo.getSimId();
        } else {
            return NO_SIM_CONNECT;
        }

    }

  
}
