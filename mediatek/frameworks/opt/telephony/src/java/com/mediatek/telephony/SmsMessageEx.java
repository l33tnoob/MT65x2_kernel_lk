package com.mediatek.telephony;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.telephony.Rlog;
import android.os.RemoteException;
import android.os.ServiceManager;

import android.telephony.SmsMessage;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ISms;
import com.android.internal.telephony.SmsConstants;


import com.mediatek.common.telephony.ISmsMessageExt;
import com.mediatek.common.featureoption.FeatureOption;

/**
 * Manages SMS raw data parsing functions.
 * @hide
 */
public class SmsMessageEx implements ISmsMessageExt{

    private static final String TAG = "SMS";

    private static final SmsMessageEx sInstance = new SmsMessageEx();

    private SmsMessageEx() {

    }

    /**
     * Gets the default instance of the SmsMessageEx
     *
     * @return the default instance of the SmsMessageEx
     */
    public static SmsMessageEx getDefault() {
        return sInstance;
    }

    /**
     * Gets the SMS service name by specific SIM ID.
     *
     * @param slotId SIM ID
     * @return The SMS service name
     */
    private static String getSmsServiceName(int slotId) {
        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            return "isms";
        } else if (slotId == PhoneConstants.GEMINI_SIM_2) {
            return "isms2";
        } else {
            return null;
        }
    }

    /**
     * @hide
     */
    protected static String getSmsFormat(int simId) {
        String isms = getSmsServiceName(simId);
        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                return iccISms.getFormat();
            } else {
                return android.telephony.SmsMessage.FORMAT_3GPP;
            }
        } catch (RemoteException ex) {
            return android.telephony.SmsMessage.FORMAT_3GPP;
        }
    }

    /**
     * Returns the tpdu from the pdu
     *
     * @return the tpdu for the message.
     * @hide
     */
    public byte[] getTpdu(SmsMessage msg, int slotId) {
        if(FeatureOption.EVDO_DT_VIA_SUPPORT == true) {
            String format = getSmsFormat(slotId);
            if (format.equals(SmsConstants.FORMAT_3GPP2)) {
                Rlog.d(TAG, "call getTpdu, format="+format+", slotId="+slotId);
                return msg.getPdu();
            }
        }

        Rlog.d(TAG, "call getTpdu, slotId="+slotId);
        byte[] pdu = msg.getPdu();
        if(pdu == null) {
            Rlog.d(TAG, "pdu is null");
            return null;
        }

        int smsc_len = (pdu[0] & 0xff) + 1;
        int tpdu_len = pdu.length - smsc_len;
        byte[] tpdu = new byte[tpdu_len];

        try {
            System.arraycopy(pdu, smsc_len, tpdu, 0, tpdu.length);
            return tpdu;
        } catch(ArrayIndexOutOfBoundsException e) {
            Rlog.e(TAG, "Out of boudns");
            return null;
        }
    }

    /**
     * Returns the smsc raw data from the pdu
     *
     * @return the raw smsc for the message.
     * @hide
     */
    public byte[] getSmsc(SmsMessage msg, int slotId) {
        if(FeatureOption.EVDO_DT_VIA_SUPPORT == true) {
            String format = getSmsFormat(slotId);
            if (format.equals(SmsConstants.FORMAT_3GPP2)) {
                Rlog.d(TAG, "call getSmsc, format="+format+", slotId="+slotId);
                return null;
            }
        }

        Rlog.d(TAG, "call getSmsc, slotId="+slotId);
        byte[] pdu = msg.getPdu();
        if(pdu == null) {
            Rlog.d(TAG, "pdu is null");
            return null;
        }

        int smsc_len = (pdu[0] & 0xff) + 1;
        byte[] smsc = new byte[smsc_len];

        try {
            System.arraycopy(pdu, 0, smsc, 0, smsc.length);
            return smsc;
        } catch(ArrayIndexOutOfBoundsException e) {
            Rlog.e(TAG, "Out of boudns");
            return null;
        }
    }

}
