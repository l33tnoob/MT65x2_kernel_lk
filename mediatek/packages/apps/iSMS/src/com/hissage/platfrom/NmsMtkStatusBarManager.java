package com.hissage.platfrom;

import java.lang.reflect.Method;

import android.content.ComponentName;
import android.content.Context;

import com.hissage.util.log.NmsLog;

public class NmsMtkStatusBarManager extends NmsPlatformBase {

    private static final String TAG = "NmsMtkStatusBarManager";
    protected Class<?> mStatusBarManagerClass = null;
    private Object mStatusBarManager = null;
    private final static String SIM_SETTING = "sms_sim_setting";
    private final static String STATUS_BAR_SERVICE = "statusbar";

    public NmsMtkStatusBarManager(Context context) {
        super(context);
        try {
            mStatusBarManager = context.getSystemService(STATUS_BAR_SERVICE);
            if (null == mStatusBarManager) {
                NmsLog.warn(TAG, "get statusbar failed");
                mPlatfromMode = NMS_STANDEALONE_MODE;
            }
            mStatusBarManagerClass = mStatusBarManager.getClass();
            mPlatfromMode = NMS_INTEGRATION_MODE;
        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.error(TAG, e.toString());
        }
    }

    public void hideSIMIndicator(ComponentName componentName) {
        if (mStatusBarManagerClass != null && NMS_INTEGRATION_MODE == mPlatfromMode
                && null != mStatusBarManager) {
            try {
                Method method = mStatusBarManagerClass.getMethod("hideSIMIndicator",
                        ComponentName.class);

                method.invoke(mStatusBarManager, componentName);
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
            }
        } else {
            NmsLog.warn(TAG, "can't hide sim indicator");
        }
    }

    public void showSIMIndicator(ComponentName componentName) {
        if (mStatusBarManagerClass != null && NMS_INTEGRATION_MODE == mPlatfromMode
                && null != mStatusBarManager) {
            try {
                Method method = mStatusBarManagerClass.getMethod("showSIMIndicator",
                        ComponentName.class, String.class);

                method.invoke(mStatusBarManager, componentName, SIM_SETTING);
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
            }
        } else {
            NmsLog.warn(TAG, "can't show sim indicator");
        }
    }
}
