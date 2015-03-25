package com.mediatek.deviceregister.test;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.dm.DmAgent;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.deviceregister.Const;
import com.mediatek.telephony.TelephonyManagerEx;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class CommonFunction {
    private static final String SUB_TAG = Const.TAG + "CommonFunction";
    private static int sEsnLength = 5;

    public static String getEsnFromUIM() {
        int readCommand = 176;
        int fileId = 0x6F38;
        String path = "3F007F25";
        String pEsn = null;

        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        try {
            byte[] readResult = iTel.transmitIccSimIoEx(fileId, readCommand, 0, 0, sEsnLength, path, null, null, 0);
            if (readResult != null && readResult.length > 2) {
                int realLength = readResult.length - 2;
                byte[] realResult = new byte[realLength];
                System.arraycopy(readResult, 0, realResult, 0, realLength);
                pEsn = bytesToHexString(getRealBytes(realResult));
            }

        } catch (RemoteException e) {
            Log.e(SUB_TAG, "write failed!" + e.getMessage());
            e.printStackTrace();
        }

        return pEsn.toUpperCase();

    }

    public static String meidToESN(String meid) {
        if (meid == null || meid.length() == 0) {
            return null;
        }
        byte[] meidByte = hexStringToBytes(meid);
        MessageDigest md;
        String pESN = null;
        try {
            md = MessageDigest.getInstance(Const.MEID_TO_PESN_HASH_NAME);
            md.update(meidByte);
            String result = bytesToHexString(md.digest());
            int length = result.length();
            if (length > 6) {
                pESN = Const.PESN_PREFIX + result.substring(length - 6, length);
            } else {
                Log.e(SUB_TAG, "digest result length < 6, it is not valid:" + result);
            }
        } catch (NoSuchAlgorithmException e) {
            Log.e(SUB_TAG, "No such algorithm:" + Const.MEID_TO_PESN_HASH_NAME);
            e.printStackTrace();
        }
        return pESN.toUpperCase();
    }

    private static byte[] hexStringToBytes(String hexString) {
        if (hexString == null || hexString.equals("")) {
            return null;
        }
        hexString = hexString.toUpperCase();
        int length = hexString.length() / 2;
        char[] hexChars = hexString.toCharArray();
        byte[] d = new byte[length];
        for (int i = 0; i < length; i++) {
            int pos = i * 2;
            d[i] = (byte) (charToByte(hexChars[pos]) << 4 | charToByte(hexChars[pos + 1]));
        }
        return d;
    }

    private static byte charToByte(char c) {
        return (byte) "0123456789ABCDEF".indexOf(c);
    }

    private static byte[] getRealBytes(byte[] byteSrc) {
        byte[] resultByte = new byte[byteSrc.length - 1];
        for (int i = 0, j = byteSrc.length - 1; j > 0; j--, i++) {
            resultByte[i] = byteSrc[j];
        }
        return resultByte;
    }

    public static String bytesToHexString(byte[] src) {
        StringBuilder stringBuilder = new StringBuilder("");
        if (src == null || src.length <= 0) {
            return null;
        }
        for (int i = 0; i < src.length; i++) {
            int v = src[i] & 0xFF;
            String hv = Integer.toHexString(v);
            if (hv.length() < 2) {
                stringBuilder.append(0);
            }
            stringBuilder.append(hv);
        }
        return stringBuilder.toString();
    }

    public static String getDeviceId() {
        String meid = null;
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        TelephonyManagerEx mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        if (mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_1) == TelephonyManager.PHONE_TYPE_CDMA) {
            meid = mTelephonyManagerEx.getDeviceId(PhoneConstants.GEMINI_SIM_1);
        } else if (mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_2) == TelephonyManager.PHONE_TYPE_CDMA) {
            meid = mTelephonyManagerEx.getDeviceId(PhoneConstants.GEMINI_SIM_2);
        }

        if (meid != null) {
            meid = meid.toUpperCase();
        }
        return meid;
    }

    private static byte[] getReverseBytes(byte[] byteSrc) {
        byte[] resultByte = new byte[byteSrc.length + 1];
        int i = 0;
        resultByte[i] = (byte) byteSrc.length;
        for (int j = byteSrc.length - 1; j >= 0; j--) {
            i++;
            resultByte[i] = byteSrc[j];
        }
        return resultByte;
    }

    public static boolean writeEsnToUIM() throws RemoteException {
        Log.d(SUB_TAG, "write pESN to UIM.");
        String meid = getDeviceId();
        Log.d(SUB_TAG, "Device id:" + meid);
        if (meid == null || meid.length() == 0) {
            Log.w(SUB_TAG, "Device id is null or empty.");
            return false;
        }
        int writeCommand = 222;
        int fileId = 0x6F38;
        String path = "3F007F25";

        String pEsn = meidToESN(meid);
        Log.d(SUB_TAG, "pEsn:" + pEsn);
        byte[] pEsnByte = hexStringToBytes(pEsn);
        byte[] pEsnByteReverse = getReverseBytes(pEsnByte);
        String pEsnReverse = bytesToHexString(pEsnByteReverse);

        sEsnLength = pEsnByteReverse.length;
        Log.d(SUB_TAG, "content to write:" + pEsnReverse + ", length:" + sEsnLength);
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        byte[] writeResult = iTel.transmitIccSimIoEx(fileId, writeCommand, 0, 0, sEsnLength, path, pEsnReverse, null,
                0);
        Log.d(SUB_TAG, "Write pEsn result:" + bytesToHexString(writeResult));

        return true;
    }

    public static String[] getImsiFromUIM() {
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        TelephonyManagerEx mTelephonyManagerEx = TelephonyManagerEx.getDefault();

        String[] imsiArr = new String[] { Const.IMSI_DEFAULT_VALUE, Const.IMSI_DEFAULT_VALUE };
        if (mTelephonyManager.hasIccCardGemini(PhoneConstants.GEMINI_SIM_1)
                && mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_1) == TelephonyManager.PHONE_TYPE_CDMA
                && Const.OPERATOR_CT.equals(mTelephonyManager.getSimOperatorGemini(PhoneConstants.GEMINI_SIM_1))) {
            imsiArr[0] = mTelephonyManagerEx.getSubscriberId(PhoneConstants.GEMINI_SIM_1);
        } else if (mTelephonyManager.hasIccCardGemini(PhoneConstants.GEMINI_SIM_2)
                && mTelephonyManagerEx.getPhoneType(PhoneConstants.GEMINI_SIM_2) == TelephonyManager.PHONE_TYPE_CDMA
                && Const.OPERATOR_CT.equals(mTelephonyManager.getSimOperatorGemini(PhoneConstants.GEMINI_SIM_2))) {
            imsiArr[1] = mTelephonyManagerEx.getSubscriberId(PhoneConstants.GEMINI_SIM_2);
        } else {
            Log.e(SUB_TAG, "there is no valid UIM card in the phone!");
            return null;
        }
        return imsiArr;
    }

    public static boolean writeImsiToDevice() {

        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent agent = DmAgent.Stub.asInterface(binder);

        Log.d(SUB_TAG, "write IMSI to device.");
        if (agent == null) {
            Log.d(SUB_TAG, "get registerFlag failed, DmAgent is null!");
            return false;
        }

        String[] imsiArr = getImsiFromUIM();
        if (imsiArr == null) {
            Log.w(SUB_TAG, "write IMSI failed, get IMSI is null!");
            return false;
        }
        String imsi1 = imsiArr[0];
        if (imsi1 == null) {
            Log.w(SUB_TAG, "IMSI1 is null!");
            imsi1 = Const.IMSI_DEFAULT_VALUE;
        }
        String imsi2 = imsiArr[1];
        if (imsi2 == null) {
            Log.w(SUB_TAG, "IMSI2 is null!");
            imsi2 = Const.IMSI_DEFAULT_VALUE;
        }
        boolean result1 = false;
        boolean result2 = false;

        try {
            result1 = agent.writeImsi1(imsi1.getBytes(), imsi1.getBytes().length) ;
            if (!result1) {
                Log.w(SUB_TAG, "write IMSI1 failed!");
            }
            result2 = agent.writeImsi2(imsi2.getBytes(), imsi2.getBytes().length);
            if (!result2) {
                Log.w(SUB_TAG, "write IMSI2 failed!");
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "remote exception when write IMSI!" + re);
        }

        return result1 && result2;

    }

}
