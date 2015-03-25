package com.hissage.platfrom;

import java.lang.reflect.Method;
import java.util.ArrayList;

import android.app.PendingIntent;
import android.content.Context;
import android.telephony.SmsManager;

import com.hissage.util.log.NmsLog;

public class NmsMtkSmsManager extends NmsPlatformBase {

    private static final String TAG = "NmsMtkSmsManager";
    private Class<?> mSmsManagerClass = null;
//    private static final String CLASS_PATH = "com.mediatek.telephony.gemini.SmsManager";
    private static final String CLASS_PATH = "android.telephony.gemini.GeminiSmsManager";
    public NmsMtkSmsManager(Context context) {
        super(context);
        try {
            mSmsManagerClass = Class.forName(CLASS_PATH);
            mPlatfromMode = NMS_INTEGRATION_MODE;

        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.warn(TAG, e.toString());
        }
    }

    @SuppressWarnings("unchecked")
    public ArrayList<String> divideMessage(String text) {
        if (null != mSmsManagerClass && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {
                Method method = mSmsManagerClass.getMethod("divideMessage", String.class);
                return (ArrayList<String>) (method.invoke(mSmsManagerClass, text));
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
                return SmsManager.getDefault().divideMessage(text);
            }

        } else {
            return SmsManager.getDefault().divideMessage(text);
        }
    }

    public void sendMultipartTextMessage(NmsMtkSmsManager manager, String destinationAddress, String scAddress,
            ArrayList<String> parts, long simId, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        NmsLog.trace(TAG, "call mtk FW: " + destinationAddress + ", parts: " + parts.toString()
                + ", mode: " + getModeString());
        if (null != mSmsManagerClass && NMS_INTEGRATION_MODE == mPlatfromMode) {
            int slotId = NmsPlatformAdapter.getInstance(mContext).getSlotIdBySimId(simId);
            try {
                Method method = mSmsManagerClass.getMethod("sendMultipartTextMessageGemini",
                        String.class, String.class, ArrayList.class, int.class, ArrayList.class,
                        ArrayList.class);
                method.invoke(manager, destinationAddress, scAddress, parts, slotId, sentIntents,
                        deliveryIntents);
            } catch (Exception e) {
                NmsLog.warn(TAG, e.toString());
            }

        } else {
            SmsManager.getDefault().sendMultipartTextMessage(destinationAddress, scAddress, parts,
                    sentIntents, deliveryIntents);
        }
    }
}
