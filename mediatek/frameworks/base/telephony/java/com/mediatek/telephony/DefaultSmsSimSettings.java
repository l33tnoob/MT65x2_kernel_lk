package com.mediatek.telephony;

import android.util.Log;
import android.content.Context;
import android.content.ContentResolver;
import android.provider.Settings;
import android.os.SystemProperties;
import com.android.internal.telephony.PhoneConstants;
import java.util.List;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.SimInfoManager;
import com.android.internal.telephony.TelephonyProperties;
import com.mediatek.common.sms.IDefaultSmsSimSettingsExt;

/**
 *@hide
 */
public class DefaultSmsSimSettings implements IDefaultSmsSimSettingsExt{
    private static final String TAG = "DefaultSmsSimSettings";

    private static final int STATUS_SIM2_INSERTED = 0x02;
    public static void setSmsTalkDefaultSim(ContentResolver contentResolver,
            List<SimInfoRecord> simInfos, long[] simIdForSlot, int nSIMCount) {
        if (!FeatureOption.MTK_BSP_PACKAGE) {
            String optr = SystemProperties.get("ro.operator.optr");
            Log.i("TAG", "nSIMCount" + nSIMCount + " , optr = " + optr);
            long oldSmsDefaultSIM = Settings.System.getLong(contentResolver,
                    Settings.System.SMS_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
            Log.i("TAG", "oldSmsDefaultSIM" + oldSmsDefaultSIM);
            long defSIM = Settings.System.DEFAULT_SIM_NOT_SET;
            if (nSIMCount > 1) {
                if (oldSmsDefaultSIM == Settings.System.DEFAULT_SIM_NOT_SET) {
                    if ("OP01".equals(optr)) {
                        Settings.System.putLong(contentResolver, Settings.System.SMS_SIM_SETTING,
                                Settings.System.SMS_SIM_SETTING_AUTO);
                    } else {
                        Settings.System.putLong(contentResolver, Settings.System.SMS_SIM_SETTING,
                                Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK);
                    }
                }
                if ("OP01".equals(optr)) {
                    defSIM = Settings.System.SMS_SIM_SETTING_AUTO;
                } else {
                    defSIM = Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK;
                }
            } else if (nSIMCount == 1) {
                long simId = simInfos.get(0).mSimInfoId;
                Settings.System.putLong(contentResolver, Settings.System.SMS_SIM_SETTING, simId);
                defSIM = simId;
            }
            Log.i("TAG", "defSIM" + defSIM);
            if (isSimRemoved(oldSmsDefaultSIM, simIdForSlot, PhoneConstants.GEMINI_SIM_NUM)) {
                Settings.System.putLong(contentResolver, Settings.System.SMS_SIM_SETTING, defSIM);
            }
        }
    }

    private static boolean isSimRemoved(long defSimId, long[] curSim, int numSim) {
        // there is no default sim if defSIMId is less than zero
        if (defSimId <= 0) {
            return false;
        }

        boolean isDefaultSimRemoved = true;
        for (int i = 0; i < numSim; i++) {
            if (defSimId == curSim[i]) {
                isDefaultSimRemoved = false;
                break;
            }
        }
        return isDefaultSimRemoved;
    }

    public int getSmsDefaultSim(Context mContext) {
        if (FeatureOption.MTK_GEMINI_ENHANCEMENT == true) {
            int simNo;
            long simIndex = Settings.System.DEFAULT_SIM_NOT_SET;
            simIndex = Settings.System.getLong(mContext.getContentResolver(),
                    Settings.System.SMS_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
            Log.d(TAG, "SMS default SIM index in db is " + simIndex);

            if (simIndex == Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK
                    || simIndex == Settings.System.SMS_SIM_SETTING_AUTO
                    || simIndex == Settings.System.DEFAULT_SIM_NOT_SET) {
                int simInsertedStatus = Integer.parseInt(SystemProperties.get(
                        TelephonyProperties.PROPERTY_GSM_SIM_INSERTED, "0"));
                if (simInsertedStatus == STATUS_SIM2_INSERTED) {
                    simNo = PhoneConstants.GEMINI_SIM_2;
                } else {
                    simNo = PhoneConstants.GEMINI_SIM_1;
                }
            } else {
                SimInfoRecord simInfo = SimInfoManager.getSimInfoById(mContext, simIndex);
                if (simInfo != null) {
                    simNo = simInfo.mSimSlotId;
                } else {
                    simNo = -1;
                }
            }

            Log.d(TAG, "final SMS default SIM is " + simNo);
            return simNo;
        } else {
            return SystemProperties.getInt(PhoneConstants.GEMINI_DEFAULT_SIM_PROP,
                    PhoneConstants.GEMINI_SIM_1);
        }
    }
}
