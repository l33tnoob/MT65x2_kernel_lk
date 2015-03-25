package com.hissage.platfrom;

import java.lang.reflect.Field;

import android.content.Context;

import com.hissage.util.log.NmsLog;

public class NmsMtkPhone extends NmsPlatformBase {
    private static final String TAG = "NmsMtkPhone";
    protected Class<?> mPhoneClass = null;
    private static final String CLASS_PATH = "com.android.internal.telephony.Phone";


    public NmsMtkPhone(Context context) {
        super(context);
        try {
            mPhoneClass = Class.forName(CLASS_PATH);
            mPlatfromMode = NMS_INTEGRATION_MODE;

        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.warn(TAG, e.toString());
        }
    }

    public String getSimIdKey(Context context){
        if (mPhoneClass != null && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Field field = mPhoneClass.getField("GEMINI_SIM_ID_KEY");
                String simIdKey = (String) field.get(mPhoneClass);

                NmsLog.trace(TAG, "getSimIdKey, return " + simIdKey + ", platfrom mode: "
                        + getModeString());
                return simIdKey;
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return null;
            }
        } else {
            return null;
        }
    }
}
