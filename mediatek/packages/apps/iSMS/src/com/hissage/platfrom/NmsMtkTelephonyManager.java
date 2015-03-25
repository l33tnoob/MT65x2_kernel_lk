package com.hissage.platfrom;

import java.lang.reflect.Method;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.hissage.util.log.NmsLog;

public class NmsMtkTelephonyManager extends NmsPlatformBase {

    private static final String TAG = "NmsMtkTelephonyManager";
    private Class<?> mSystemTelephonyManagerClass = null;
    private Context mContext = null;
    private static final String CLASS_PATH = "com.mediatek.telephony.TelephonyManagerEx";

    public NmsMtkTelephonyManager(Context context) {
        super(context);
        try {
            mContext = context;
            mSystemTelephonyManagerClass = Class.forName(CLASS_PATH);
            mPlatfromMode = NMS_INTEGRATION_MODE;

        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.warn(TAG, e.toString());
        }

    }

    public String getSubscriberId(int slotId) {
        if (null != mSystemTelephonyManagerClass && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Method defaultmethod = mSystemTelephonyManagerClass.getMethod("getDefault");
                Object instance = defaultmethod.invoke(defaultmethod);
                Method method = mSystemTelephonyManagerClass
                        .getMethod("getSubscriberId", int.class);
                String imsi = (String) (method.invoke(instance, slotId));
                NmsLog.trace(TAG, "NmsMtkTelephonyManager get imsi from mtk FW: " + imsi
                        + ", slotId: " + slotId);
                return imsi;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return null;
            }

        } else {
            TelephonyManager telManager = (TelephonyManager) mContext
                    .getSystemService(Context.TELEPHONY_SERVICE);

            if (null != telManager) {
                return telManager.getSubscriberId();
            } else {
                NmsLog.trace(TAG, "IMSI is null from telManger or no sim card.");
                return null;
            }
        }
    }
    
    public String getLine1Number(int slotId) {
        if (null != mSystemTelephonyManagerClass && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Method defaultmethod = mSystemTelephonyManagerClass.getMethod("getDefault");
                Object instance = defaultmethod.invoke(defaultmethod);
                Method method = mSystemTelephonyManagerClass
                        .getMethod("getLine1Number", int.class);
                String number = (String) (method.invoke(instance, slotId));
                NmsLog.trace(TAG, "NmsMtkTelephonyManager getLine1Number from mtk FW: " + number
                        + ", slotId: " + slotId);
                return number;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return null;
            }

        } else {
            TelephonyManager telManager = (TelephonyManager) mContext
                    .getSystemService(Context.TELEPHONY_SERVICE);

            if (null != telManager) {
                return telManager.getLine1Number();
            } else {
                return null;
            }
        }
    }
}
