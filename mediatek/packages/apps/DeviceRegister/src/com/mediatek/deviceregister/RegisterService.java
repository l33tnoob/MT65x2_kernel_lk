package com.mediatek.deviceregister;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.dm.DmAgent;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.telephony.SmsManagerEx;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Timer;
import java.util.TimerTask;

public class RegisterService extends Service {

    private static final String SUB_TAG = Const.TAG + "RegisterService";

    private static int sSendMessageRetryTimes = 0;
    private static int sEsnLength = 5;

    private DmAgent mAgent;
    private TelephonyManager mTelephonyManager;
    private TelephonyManagerEx mTelephonyManagerEx;

    private SmsSendReceiver mSmsSendReceiver;

    private TimerTask mTask = new TimerTask() {

        public void run() {
            sendRegisterMessage();
        }

    };

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(SUB_TAG, "onCreate");
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            Log.d(SUB_TAG, "Not gemini phone, stop self.");
            this.stopSelf();
        }
        if (mTelephonyManager == null) {
            mTelephonyManager = TelephonyManager.getDefault();
        }
        if (mTelephonyManagerEx == null) {
            mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        }

        if (mAgent == null) {
            Log.i(SUB_TAG, "get DmAgent");
            IBinder binder = ServiceManager.getService("DmAgent");
            if (binder == null) {
                Log.i(SUB_TAG, "get DmAgent fail, binder is null!");
                return;
            }
            mAgent = DmAgent.Stub.asInterface(binder);
        }

    }

    private String meidToEsn(String meid) {
        if (meid == null || meid.length() == 0) {
            return null;
        }
        byte[] meidByte = hexStringToBytes(meid);
        MessageDigest md;
        String pEsn = null;
        try {
            md = MessageDigest.getInstance(Const.MEID_TO_PESN_HASH_NAME);
            md.update(meidByte);
            String result = bytesToHexString(md.digest());
            int length = result.length();
            if (length > 6) {
                pEsn = Const.PESN_PREFIX + result.substring(length - 6, length);
            } else {
                Log.e(SUB_TAG, "digest result length < 6, it is not valid:" + result);
            }
        } catch (NoSuchAlgorithmException e) {
            Log.e(SUB_TAG, "No such algorithm:" + Const.MEID_TO_PESN_HASH_NAME);
            e.printStackTrace();
        }
        if (pEsn != null) {
            pEsn = pEsn.toUpperCase();
        }
        return pEsn;
    }

    private byte[] getReverseBytes(byte[] byteSrc) {
        byte[] resultByte = new byte[byteSrc.length + 1];
        int i = 0;
        resultByte[i] = (byte) byteSrc.length;
        for (int j = byteSrc.length - 1; j >= 0; j--) {
            i++;
            resultByte[i] = byteSrc[j];
        }
        return resultByte;
    }

    private byte[] getRealBytes(byte[] byteSrc) {
        byte[] resultByte = new byte[byteSrc.length - 1];
        for (int i = 0, j = byteSrc.length - 1; j > 0; j--, i++) {
            resultByte[i] = byteSrc[j];
        }
        return resultByte;
    }

    private String bytesToHexString(byte[] src) {
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

    private byte[] hexStringToBytes(String hexString) {
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

    private byte charToByte(char c) {
        return (byte) "0123456789ABCDEF".indexOf(c);
    }

    @Override
    public void onDestroy() {
        Log.d(SUB_TAG, "onDestroy");
        if (mSmsSendReceiver != null) {
            unregisterReceiver(mSmsSendReceiver);
        }
        super.onDestroy();
    }

    @Override
    public void onRebind(Intent intent) {
        super.onRebind(intent);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(SUB_TAG, "onStartCommand");
        if (intent != null) {
            String action = intent.getAction();
            if (action.equalsIgnoreCase(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE)) {
                Log.d(SUB_TAG, "Auto register feasible.");

                new Thread() {
                    public void run() {
                        if (needRegister()) {
                            Timer timer = new Timer();
                            timer.schedule(mTask, Const.ONE_MINUTE);
                        } else {
                            Log.d(SUB_TAG, "Phone is not meet the requirement of register.");
                            RegisterService.this.stopSelf();
                        }
                    }
                }.start();

            } else if (action.equalsIgnoreCase(Const.ACTION_CT_CONFIRMED_MESSAGE)) {
                setRegisterFlag(checkRegisterResult(intent));
                stopSelf();
            } else if (action.equalsIgnoreCase(Const.ACTION_REGISTER_MESSAGE_SEND)) {
                setRegisterFlag(false);
                writeImsiToDevice();

                new Thread() {
                    public void run() {
                        Log.v(SUB_TAG,"writeEsnToUim,current thread: "+Thread.currentThread().getId());
                        writeEsnToUim();
                    }
                }.start();
                stopSelf();
            }
        } else {
            Log.w(SUB_TAG, "Intent is null");
            stopSelf();
        }
        return START_STICKY;
    }

    private boolean needRegister() {
        Log.v(SUB_TAG, "[needRegister]current thread: " + Thread.currentThread().getId());
        if (mTelephonyManager != null) {

            // check if there is UIM card in phone
            // it is impossible that there is no card in phone in normal case.

            int currentLogonUim = getCurrentLogonUim();
            if (currentLogonUim == Const.UIM_NONE) {
                Log.w(SUB_TAG, "There is no UIM card is connected CDMA net or it is roaming.");
                return false;
            }

            if (!isRegistered()) {
                Log.d(SUB_TAG, "Register flag is not true.");
                return true;
            } else {
                Log.d(SUB_TAG, "Register flag is true, compare the imsi and meid.");
                if (isImsiSame() && isEsnSame()) {
                    Log.d(SUB_TAG, "imsi and meid are the same, have registered already.");
                    return false;
                } else {
                    return true;
                }
            }
        } else {
            Log.w(SUB_TAG, "telephony manager is null.");
            return false;
        }

    }

    private int getCurrentLogonUim() {
        int currentLogonUim = Const.UIM_NONE;

        for (int uimId : Const.UIM_ID_LIST) {
            if (isUimAvailable(uimId)) {
                currentLogonUim = uimId;
                break;
            }
        }
        Log.d(SUB_TAG, "Current logon UIM is " + currentLogonUim);

        return currentLogonUim;
    }

    /**
     * Whether uim's network operator, UIM operator and phone type is correct
     * 
     * @param uimId
     * @return true or false
     */
    private boolean isUimAvailable(int uimId) {
        Log.v(SUB_TAG, "[isUimAvailable] begin uimId: " + uimId);
        if (mTelephonyManagerEx.hasIccCard(uimId)) {
            int phoneType = mTelephonyManagerEx.getPhoneType(uimId);
            Log.v(SUB_TAG, "[isUimAvailable] phone type of uim (" + uimId + ") = " + phoneType);
            if (TelephonyManager.PHONE_TYPE_CDMA == phoneType) {
                Log.v(SUB_TAG, "[isUimAvailable] UIM (" + uimId + ") is PHONE_TYPE_CDMA");
                String netwOperator = mTelephonyManager.getNetworkOperatorGemini(uimId);
                Log.v(SUB_TAG, "[isUimAvailable] networkOperator of uim (" + uimId + ") is "
                        + netwOperator);
                if (Const.OPERATOR_CT.equals(netwOperator)
                        || Const.OPERATOR_CT_MAC.equals(netwOperator)) {
                    String simOperator = mTelephonyManager.getSimOperatorGemini(uimId);
                    Log.v(SUB_TAG, "[isUimAvailable] simOperator of uim (" + uimId + ") is "
                            + simOperator);
                    if (netwOperator.equals(simOperator)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private String[] getSavedImsi() {

        if (mAgent == null) {
            Log.d(SUB_TAG, "get IMSI failed, DmAgent is null!");
            return null;
        }

        String[] imsiArr = new String[2];
        try {
            byte[] imsi1 = mAgent.readImsi1();
            byte[] imsi2 = mAgent.readImsi2();
            if (imsi1 != null) {
                imsiArr[0] = new String(imsi1);
            }
            if (imsi2 != null) {
                imsiArr[1] = new String(imsi2);
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "remote exception when get IMSI!" + re);
        }

        return imsiArr;
    }

    private boolean isRegistered() {
        // read register flag from nvram by DmAgent
        if (mAgent == null) {
            Log.d(SUB_TAG, "get registerFlag failed, DmAgent is null!");
            return false;
        }
        int registerFlag = 0;

        try {
            byte[] readData = mAgent.readRegisterFlag();
            if (readData != null && readData.length > 0) {
                try {
                    registerFlag = Integer.parseInt(new String(readData));
                } catch (NumberFormatException nfe) {
                    Log.w(SUB_TAG, "registerFlag parse int failed!");
                }
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "remote exception when readRegisterFlag!" + re);
        }

        return registerFlag == 1;
    }

    public String getDeviceId() {
        String meid = null;
        if (mTelephonyManagerEx != null) {
            int uimId = getCurrentLogonUim();
            if (uimId != Const.UIM_NONE) {
                meid = mTelephonyManagerEx.getDeviceId(uimId);
                Log.v(SUB_TAG, "[getDeviceId]DeviceId of uim (" + uimId + ") is " + meid);
            }

        }
        if (meid != null) {
            meid = meid.toUpperCase();
        }

        Log.i(SUB_TAG, "[getDeviceId]meid = " + meid);
        return meid;

    }

    private void writeEsnToUim() {
        Log.d(SUB_TAG, "write pESN to UIM.");
        String meid = getDeviceId();
        Log.d(SUB_TAG, "Device id:" + meid);
        if (meid == null || meid.length() == 0) {
            Log.w(SUB_TAG, "Device id is null or empty.");
            return;
        }
        int writeCommand = 222;
        int fileId = 0x6F38;
        String path = "3F007F25";

        String pEsn = meidToEsn(meid);
        Log.d(SUB_TAG, "pESN:" + pEsn);
        byte[] pEsnByte = hexStringToBytes(pEsn);
        byte[] pEsnByteReverse = getReverseBytes(pEsnByte);
        String pEsnReverse = bytesToHexString(pEsnByteReverse);

        sEsnLength = pEsnByteReverse.length;
        Log.d(SUB_TAG, "content to write:" + pEsn + ", length:" + sEsnLength);
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        try {
            byte[] writeResult = iTel.transmitIccSimIoEx(fileId, writeCommand, 0, 0, sEsnLength, path, pEsnReverse,
                    null, 0);
            Log.d(SUB_TAG, "Write pEsn result:" + bytesToHexString(writeResult));

        } catch (RemoteException e) {
            Log.e(SUB_TAG, "write failed!" + e.getMessage());
            e.printStackTrace();
        }
    }

    private boolean writeImsiToDevice() {
        Log.d(SUB_TAG, "write IMSI to device.");
        if (mAgent == null) {
            Log.d(SUB_TAG, "get registerFlag failed, DmAgent is null!");
            return false;
        }

        String[] imsiArr = getImsiFromUim();
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
            result1 = mAgent.writeImsi1(imsi1.getBytes(), imsi1.length());
            if (!result1) {
                Log.w(SUB_TAG, "write IMSI1 failed!");
            }
            result2 = mAgent.writeImsi2(imsi2.getBytes(), imsi2.length());
            if (!result2) {
                Log.w(SUB_TAG, "write IMSI2 failed!");
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "remote exception when write IMSI!" + re);
        }

        return result1 && result2;

    }

    public String getCurrentLogonImsi() {
        String currentLogonImsi = null;
        if (mTelephonyManagerEx != null) {
            int currentUim = getCurrentLogonUim();
            if (currentUim != Const.UIM_NONE) {
                currentLogonImsi = mTelephonyManagerEx.getSubscriberId(currentUim);
                Log.d(SUB_TAG, "[getCurrentLogonImsi] = " + currentLogonImsi);
            }
        } else {
            Log.e(SUB_TAG, "mTelephonyManager is null!");
        }
        return currentLogonImsi;
    }

    private String[] getImsiFromUim() {
        if (mTelephonyManagerEx != null) {

            int[] uimIdList = Const.UIM_ID_LIST;
            int length = uimIdList.length;
            String[] imsiArr = new String[length];
            for (int i = 0; i < length; ++i) {
                int uimId = uimIdList[i];
                if (isUimAvailable(uimId)) {
                    imsiArr[i] = mTelephonyManagerEx.getSubscriberId(uimId);
                } else {
                    imsiArr[i] = Const.IMSI_DEFAULT_VALUE;
                }
                Log.d(SUB_TAG, "[getImsiFromUim] imsiArr[i]: " + imsiArr[i]);
            }
            return imsiArr;
        } else {
            Log.e(SUB_TAG, "[getImsiFromUim]mTelephonyManager is null!");
        }
        return null;
    }

    private String getEsnFromUim() {
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

        if (pEsn != null) {
            pEsn = pEsn.toUpperCase();
        }

        return pEsn;
    }

    private void sendRegisterMessage() {
        Log.v(SUB_TAG, "sendRegisterMessage,current thread: " + Thread.currentThread().getId());
        mSmsSendReceiver = new SmsSendReceiver();
        registerReceiver(mSmsSendReceiver, new IntentFilter(Const.ACTION_REGISTER_MESSAGE_SEND));
        byte[] registerMessage = new RegisterMessage(this).getRegisterMessage();
        Log.d(SUB_TAG, "message length:" + registerMessage.length);
        int currentLogonUim = getCurrentLogonUim();
        if (currentLogonUim != Const.UIM_NONE) {
            SmsManagerEx.getDefault().sendDataMessage(Const.SERVER_ADDRESS, null, Const.PORT, registerMessage,
                    getSendPendingIntent(), null, currentLogonUim);
            Log.d(SUB_TAG, "send message...");
        } else {
            Log.e(SUB_TAG, "there is no UIM is logon CDMA net now. Can't send message.");
        }
    }

    private boolean checkRegisterResult(Intent intent) {
        // get data from intent, and analyze it to check if register is
        // successful.
        Log.d(SUB_TAG, "Check register result.");
        byte[] pduByte = intent.getByteArrayExtra("pdu");
        if (pduByte != null && pduByte.length > 0) {
            SmsMessage message = SmsMessage.createFromPdu(pduByte, SmsMessage.FORMAT_3GPP2);
            String originAddr = message.getOriginatingAddress();
            Log.d(SUB_TAG, "message origination address:" + originAddr);
            if (originAddr.equals(Const.SERVER_ADDRESS)) {
                byte[] data = message.getUserData();
                Log.d(SUB_TAG, "message user data:" + bytesToHexString(data));
                if (data != null && data.length > 1) {
                    byte confirmByte = data[1];
                    if (confirmByte == Const.COMMAND_TYPE_RECEIVED) {
                        Log.d(SUB_TAG, "Register success!");
                        return true;
                    } else {
                        Log.d(SUB_TAG, "Register failed!");
                    }
                } else {
                    Log.d(SUB_TAG, "message data is not valid!");
                }
            } else {
                Log.d(SUB_TAG, "origin address is not valid!");
            }
        } else {
            Log.d(SUB_TAG, "Pdu is not valid!");
        }
        return false;
    }

    private boolean setRegisterFlag(boolean flag) {
        if (mAgent == null) {
            Log.d(SUB_TAG, "set registerFlag failed, DmAgent is null!");
            return false;
        }
        Log.d(SUB_TAG, "set registerFlag:" + flag);
        String registerFlag = flag ? "1" : "0";
        boolean result = false;
        try {
            result = mAgent.setRegisterFlag(registerFlag.getBytes(), registerFlag.length());
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "remote exception when setRegisterFlag!" + re);
        }

        return result;

    }

    private boolean isImsiSame() {

        String[] imsiFromUim = getImsiFromUim();
        String[] imsiFromDevice = getSavedImsi();
        if (imsiFromUim.length != imsiFromDevice.length) {
            Log.d(SUB_TAG, "imsi is not same: array length is not same.");
            return false;
        } else {
            for (int i = 0; i < imsiFromUim.length; i++) {
                boolean equal = false;
                for (int j = 0; j < imsiFromDevice.length; j++) {
                    if (imsiFromUim[i].equals(imsiFromDevice[j])) {
                        equal = true;
                        break;
                    }
                }
                if (!equal) {
                    Log.d(SUB_TAG, "can't find the same imsi from saved ones:" + imsiFromUim[i]);
                    return false;
                }
            }
            Log.d(SUB_TAG, "IMSIs are the same.");
            return true;
        }
    }

    private boolean isEsnSame() {
        String uimSavedEsn = getEsnFromUim();
        String deviceEsn = meidToEsn(getDeviceId());
        if (uimSavedEsn != null && uimSavedEsn.equalsIgnoreCase(deviceEsn)) {
            Log.d(SUB_TAG, "ESN is same.");
            return true;
        } else {
            Log.d(SUB_TAG, "ESN is not same:" + uimSavedEsn + "--" + deviceEsn);
            return false;
        }
    }

    // public String getDeviceIdWithChecksum() {
    // String meid = getDeviceId();
    //
    // if (meid != null) {
    // if (isAllNumeric(meid)) {
    // meid = meid + getCheckNo(meid);
    // } else {
    // meid = meid + getCheckNo2(meid);
    // }
    // }
    // return meid.toUpperCase();
    // }

    // private boolean isAllNumeric(String value) {
    // Pattern numericPattern = Pattern.compile("[0-9]*");
    // Matcher matchResult = numericPattern.matcher(value);
    // if (matchResult.matches()) {
    // return true;
    // } else {
    // return false;
    // }
    // }

    // private String getCheckNo(String meid) {
    // char[] meidCharArr = meid.toCharArray();
    // int checkNo = 0;
    // for (int i = 0; i < meidCharArr.length; i++) {
    // int a = Integer.parseInt(String.valueOf(meidCharArr[i]));
    // i++;
    // final int temp = Integer.parseInt(String.valueOf(meidCharArr[i])) * 2;
    // final int b = temp < 10 ? temp : temp - 9;
    // checkNo += a + b;
    // }
    // checkNo %= 10;
    // checkNo = checkNo == 0 ? 0 : 10 - checkNo;
    // return String.valueOf(checkNo);
    // }

    // private String getCheckNo2(String meid) {
    // char[] meidCharArr = meid.toCharArray();
    // int checkNo = 0;
    // for (int i = 0; i < meidCharArr.length; i++) {
    // int a = Integer.parseInt(String.valueOf(meidCharArr[i]), 16);
    // i++;
    // final int temp = Integer.parseInt(String.valueOf(meidCharArr[i]), 16) *
    // 2;
    // final int b = temp < 16 ? temp : temp - 15;
    // checkNo += a + b;
    // }
    // checkNo %= 16;
    // checkNo = checkNo == 0 ? 0 : 16 - checkNo;
    // return Integer.toHexString(checkNo);
    // }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    private PendingIntent getSendPendingIntent() {
        Log.d(SUB_TAG, "get send pending intent");
        Intent mIntent = new Intent();
        mIntent.setAction(Const.ACTION_REGISTER_MESSAGE_SEND);

        PendingIntent mSendPendingIntent = PendingIntent.getBroadcast(this, 0, mIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        Log.d(SUB_TAG, "return a pending intent");
        return mSendPendingIntent;
    }

    class SmsSendReceiver extends BroadcastReceiver {
        private static final String SUB_TAG = Const.TAG + "SmsSendReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(SUB_TAG, "onReceive");
            if (intent != null) {
                String action = intent.getAction();
                if (action.equalsIgnoreCase(Const.ACTION_REGISTER_MESSAGE_SEND)) {
                    int resultCode = getResultCode();
                    Log.d(SUB_TAG, "get result code:" + resultCode);
                    if (resultCode == Activity.RESULT_OK) {
                        Log.d(SUB_TAG, "result ok! send register message success.");
                        // start service
                        Log.d(SUB_TAG, "start service");
                        intent.setClass(context, RegisterService.class);
                        context.startService(intent);
                    } else {
                        Log.d(SUB_TAG, "send message failed, retry:" + sSendMessageRetryTimes);
                        if (sSendMessageRetryTimes < Const.SEND_MESSAGE_RETRY_TIMES_MAX) {
                            sendRegisterMessage();
                            sSendMessageRetryTimes++;
                        } else {
                            stopSelf();
                        }

                    }
                } else {
                    Log.d(SUB_TAG, "action is not valid." + action);
                }

            } else {
                Log.d(SUB_TAG, "intent is null.");
            }

        }
    }
}
