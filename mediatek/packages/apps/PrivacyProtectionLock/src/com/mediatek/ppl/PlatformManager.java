package com.mediatek.ppl;

import java.util.ArrayList;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.storage.IMountService;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.telephony.SmsManagerEx;

public class PlatformManager {
    protected static final String TAG = "PPL/PlatformManager";
    private final Context mContext;
    private final IPplAgent mAgent;
    private final SmsManagerEx mSmsManager;
    private final IMountService mMountService;
    private final ConnectivityManager mConnectivityManager;
    private final WakeLock mWakeLock;

    public static final int SIM_NUMBER;

    static {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                SIM_NUMBER = 4;
            } else if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                SIM_NUMBER = 3;
            } else {
                SIM_NUMBER = 2;
            }
        } else {
            SIM_NUMBER = 1;
        }
    }

    public PlatformManager(Context context) {
        mContext = context;
        IBinder binder = ServiceManager.getService("PPLAgent");
        if (binder == null) {
            throw new Error("Failed to get PPLAgent");
        }
        mAgent = IPplAgent.Stub.asInterface(binder);
        if (mAgent == null) {
            throw new Error("mAgent is null!");
        }
        mSmsManager = SmsManagerEx.getDefault();
        mMountService = IMountService.Stub.asInterface(ServiceManager.getService("mount"));
        mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "PPL_WAKE_LOCK");
    }

    public IPplAgent getPPLAgent() {
        return mAgent;
    }

    public void sendTextMessage(String destinationAddress, long id, String text, Intent sentIntent, int simId) {
        Log.d(TAG, "sendTextMessage(" + destinationAddress + ", " + id + ", " + text + ", " + simId + ")");
        ArrayList<String> segments = divideMessage(text);
        ArrayList<PendingIntent> pis = new ArrayList<PendingIntent>(segments.size());
        final int total = segments.size();
        for (int i = 0; i < total; ++i) {
            Intent intent = new Intent(sentIntent);
            Uri.Builder builder = new Uri.Builder();
            builder.authority(MessageManager.SMS_PENDING_INTENT_DATA_AUTH)
                    .scheme(MessageManager.SMS_PENDING_INTENT_DATA_SCHEME).appendPath(Long.toString(id))
                    .appendPath(Integer.toString(total)).appendPath(Integer.toString(i));
            Log.d(TAG, "sendTextMessage: uri string is " + builder.toString());
            intent.setData(builder.build());

            byte type = intent.getByteExtra(PendingMessage.KEY_TYPE, MessageManager.Type.INVALID);
            String number = intent.getStringExtra(PendingMessage.KEY_NUMBER);
            Log.d(TAG, "id is " + id + ", type is " + type + ", number is " + number);

            PendingIntent pi = PendingIntent.getBroadcast(mContext, 0, intent, PendingIntent.FLAG_ONE_SHOT);
            pis.add(pi);
        }
        sendMultipartTextMessage(destinationAddress, null, segments, pis, simId);
    }

    protected ArrayList<String> divideMessage(String text) {
        return mSmsManager.divideMessage(text);
    }

    protected void sendMultipartTextMessage(String destinationAddress, String scAddress, ArrayList<String> parts,
            ArrayList<PendingIntent> sentIntents, int simId) {
        mSmsManager.sendMultipartTextMessage(destinationAddress, scAddress, parts, sentIntents, null, simId);
    }

    public boolean isUsbMassStorageEnabled() {
        try {
            return mMountService.isUsbMassStorageEnabled();
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }

    public void setMobileDataEnabled(boolean enable) {
        mConnectivityManager.setMobileDataEnabled(enable);
    }
    
    public void acquireWakeLock() {
        mWakeLock.acquire();
    }
    
    public void releaseWakeLock() {
        mWakeLock.release();
    }
}
