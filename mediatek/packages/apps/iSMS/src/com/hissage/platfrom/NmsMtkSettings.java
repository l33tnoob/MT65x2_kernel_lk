package com.hissage.platfrom;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

import android.content.ContentResolver;
import android.content.Context;

import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsMtkSettings extends NmsPlatformBase {

    private static final String TAG = "NmsMtkSettings";
    protected Class<?> mSystemSettingsClass = null;
    private String SMS_SIM_SETTING = null;
    private long DEFAULT_SIM_SETTING_ALWAYS_ASK = -1;
    private long DEFAULT_SIM_NOT_SET = -1;
    private static final String CLASS_PATH = "android.provider.Settings$System";

    public NmsMtkSettings(Context context) {
        super(context);
        try {
            mSystemSettingsClass = Class.forName(CLASS_PATH);
            mPlatfromMode = NMS_INTEGRATION_MODE;

            Field field0 = mSystemSettingsClass.getField("SMS_SIM_SETTING");
            SMS_SIM_SETTING = (String) field0.get(mSystemSettingsClass);

            Field field1 = mSystemSettingsClass.getField("DEFAULT_SIM_SETTING_ALWAYS_ASK");
            DEFAULT_SIM_SETTING_ALWAYS_ASK = (Long) field1.get(mSystemSettingsClass);

            Field field2 = mSystemSettingsClass.getField("DEFAULT_SIM_NOT_SET");
            DEFAULT_SIM_NOT_SET = (Long) field2.get(mSystemSettingsClass);

        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.warn(TAG, e.toString());
        }
    }

    private long rand(Context context) {
        long simId = NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_1);
        if (simId > 0) {
            return simId;
        }
        simId = NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(NmsConsts.SIM_CARD_SLOT_2);
        if (simId > 0) {
            return simId;
        }

        simId = getGoogleDefaultSimId(context);
        return simId;

    }

    public  boolean isDefultSimNotSet(Context context){
        if (mSystemSettingsClass != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Method method = mSystemSettingsClass.getMethod("getLong", ContentResolver.class,
                        String.class, long.class);
                long simId = (Long) method.invoke(mSystemSettingsClass,
                         context.getContentResolver(), SMS_SIM_SETTING, DEFAULT_SIM_NOT_SET);
                if (simId == DEFAULT_SIM_NOT_SET || simId == DEFAULT_SIM_SETTING_ALWAYS_ASK) {
                     return true;
                   }
            } catch (Exception e) {
                return false;
            }
        }
        return false;
    }
    public long getCurrentSimId(Context context) {
        if (mSystemSettingsClass != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Method method = mSystemSettingsClass.getMethod("getLong", ContentResolver.class,
                        String.class, long.class);

                long simId = (Long) method.invoke(mSystemSettingsClass,
                        context.getContentResolver(), SMS_SIM_SETTING, DEFAULT_SIM_NOT_SET);
                if (simId == DEFAULT_SIM_NOT_SET || simId == DEFAULT_SIM_SETTING_ALWAYS_ASK) {
                    simId = rand(context);
                }

                NmsLog.trace(TAG, "getCurrentSimId, return " + simId + ", platfrom mode: "
                        + getModeString());
                return simId;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return rand(context);
            }
        } else {
            return getGoogleDefaultSimId(context);
        }
    }
    
    public long getNmsCurrentSimId(Context context) {
        if (mSystemSettingsClass != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Method method = mSystemSettingsClass.getMethod("getLong", ContentResolver.class,
                        String.class, long.class);

                long simId = (Long) method.invoke(mSystemSettingsClass,
                        context.getContentResolver(), SMS_SIM_SETTING, DEFAULT_SIM_NOT_SET);
                NmsLog.trace(TAG, "getCurrentSimId, return " + simId + ", platfrom mode: "
                        + getModeString());
                return simId;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return DEFAULT_SIM_NOT_SET;
            }
        } else {
            return getGoogleDefaultSimId(context);
        }
    }
    
    
    public boolean setCurrentSimId(Context context, long simId) {
        if (mSystemSettingsClass != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                boolean ret = false;
                Method method = mSystemSettingsClass.getMethod("putLong", ContentResolver.class,
                        String.class, long.class);

                ret = (Boolean)method.invoke(mSystemSettingsClass,
                        context.getContentResolver(), SMS_SIM_SETTING, simId);
                  NmsLog.trace(TAG, "put CurrentSimId, return " + ret + ", platfrom mode: "
                        + getModeString());
                return ret;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return false;
            }
        } else {
            return false;
        }
    }

}
