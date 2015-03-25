package com.hissage.message.smsmms;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;

import android.app.PendingIntent;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.telephony.SmsManager;
import android.text.TextUtils;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.jni.NmsQueue;
import com.hissage.jni.engineadapterforjni;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsMsgType;
import com.hissage.util.log.NmsLog;

public class NmsSendMessage {

    private String TAG = "NmsSendMessage";
    public final static int UNSAVE_SMS = 0x00;
    public final static int SAVE_SMS = 0x01;
    public final static int MAX_SMS_LENGTH = 70;
    public final static String SMS_ID = "id";
    public final static String SMS_SENT_ACTION = "com.hissage.nmssendmessage.sent_action";
    public final static String SMS_DELIVER_ACTION = "com.hissage.nmssendmessage.deliver_action";

    public static NmsSendMessage mInstance = null;
    private Vector<SmsCont> mVecSms = new Vector<SmsCont>();

    private Context mContext = null;
    private long mTimeOut = 300000;
    private boolean mbThreadRunning = false;

    private String[] mArrayPhoneName = { "X10i", "X10" };

    class SmsCont {
        SmsCont(String strAddrIn, String strMsgIn, long lSmsIdIn, long simId) {
            strAddr = strAddrIn;
            strMsg = strMsgIn;
            lSmsId = lSmsIdIn;
            this.simId = simId;
        }

        public String strAddr;
        public String strMsg;
        public long lSmsId;
        public long simId;
    }

    private NmsSendMessage() {
        mContext = NmsService.getInstance();
    }

    public static NmsSendMessage getInstance() {
        if (null == mInstance) {
            mInstance = new NmsSendMessage();
        }
        return mInstance;
    }

    private boolean isThreadRunning() {
        return mbThreadRunning;
    }

    private void setThreadRunningFlag(boolean bFlag) {
        mbThreadRunning = bFlag;
    }

    class RunnableSendSms implements Runnable {
        public RunnableSendSms() {
        }

        @Override
        public void run() {
            NmsLog.trace(TAG, "send sms thread start!");
            setThreadRunningFlag(true);
            while (!mVecSms.isEmpty()) {
                SmsCont smsContent = mVecSms.firstElement();
                if (!isNetWorkOk()) {
                    processFail(smsContent.lSmsId);
                } else {
                    sendSms(smsContent);
                }
                mVecSms.remove(smsContent);
            }
            setThreadRunningFlag(false);
            NmsLog.trace(TAG, "send sms thread finish!");
        }
    }

    private void createThread() {
        if (!isThreadRunning()) {
            new Thread(new RunnableSendSms()).start();
        }
    }

    private boolean isNetWorkOk() {
        return true;
    }

    public boolean isAddressLegal(String strAddrIn, Set<String> setAddrIn) {
        if (TextUtils.isEmpty(strAddrIn)) {
            return false;
        }
        String[] strArrayAddr = strAddrIn.split(",");
        for (String strAddrTemp : strArrayAddr) {
            if (NmsCommonUtils.isPhoneNumberValid(strAddrTemp)) {
                setAddrIn.add(strAddrTemp);
            }
        }
        if (setAddrIn.size() > 0) {
            return true;
        }
        return false;
    }

    public int sendInviteSms(String strPhoneIn, long simId) {
        String strInviteSms = mContext.getString(R.string.STR_NMS_INVITE_SMS_CONTENT);
        return inserSmsToVector(null, strPhoneIn, strInviteSms, SAVE_SMS, simId);
    }

    public int resendSms(int nSmsIdIn) {
        int nSmsIdNew = -1;
        SNmsMsgCont msgCont = null;
        if (nSmsIdIn < 0) {
            NmsLog.trace(TAG, "resendSms, illegal sms id:" + nSmsIdIn);
            return -1;
        }

        msgCont = NmsSMSMMSManager.getInstance(null).getSmsMsgContViaId(nSmsIdIn);
        if (null == msgCont) {
            NmsLog.trace(TAG, "resendSms, can not get msgcont via id:" + nSmsIdIn);
            return -1;
        }

        NmsSMSMMSManager.getInstance(null).deleteSMS(NmsSMSMMS.PROTOCOL_SMS, new int[]{nSmsIdIn}, true);
        nSmsIdNew = inserSmsToVector(msgCont.pThreadNumber, msgCont.pTo, msgCont.pBody, SAVE_SMS,
                msgCont.simId);
        NmsLog.trace(TAG, "resendSms, delete old sms id:" + nSmsIdIn + ", create new sms id:"
                + nSmsIdNew);
        return nSmsIdNew;
    }

    public void processFail(long lSmdId) {
        NmsLog.trace(TAG, "process msg send fail, SmsId:" + lSmdId);
        NmsSMSMMSManager.getInstance(null).updateSmsStatusViaId(NmsSMSMMS.PROTOCOL_SMS, (int) lSmdId,
                NmsSMSMMS.SMS_TYPE_FAILED, NmsSMSMMS.SMS_DELIVER_UNKNOWN);
        engineadapterforjni.nmsSendMsgToUI(SNmsMsgType.NMS_UI_MSG_PLAY_SEND_MSG_TONE, null,
                NmsQueue.NMS_PLAY_TONE_SMS_FAIELD);
    }

    public int inserSmsToVector(String threadAddress, String address, String message, int saveFlag,
            long simId) {
        if (TextUtils.isEmpty(message)) {
            NmsLog.error(TAG, "inserSmsToVector, content of msg is empty, just return.");
            return -1;
        }
        Set<String> setAddr = new HashSet<String>();
        if (!isAddressLegal(address, setAddr)) {
            NmsLog.trace(TAG, "inserSmsToVector, illegal address, just return.addr: " + address);
            return -1;
        }
        if (TextUtils.isEmpty(threadAddress)) {
            threadAddress = address;
        }
        Set<String> setThreadAddr = new HashSet<String>();
        if (!isAddressLegal(threadAddress, setThreadAddr)) {
            NmsLog.trace(TAG, "inserSmsToVector, illegal thread address, just return.threadAddr:"
                    + setThreadAddr);
            return -1;
        }

        Iterator<String> iter = null;
        long lThreadId = -1;
        if (saveFlag != 0) {
            lThreadId = NmsCreateSmsThread.getOrCreateThreadId(mContext, setThreadAddr);
        }
        long lSmsId = -1;
        iter = setAddr.iterator();
        do {
            lSmsId = divideSms(message, iter.next().trim(), saveFlag, lThreadId, simId);
        } while (iter.hasNext());

        createThread();
        return (int) lSmsId;
    }

    public int sendSms(SmsCont smsContent) {
        nmsSendMultiPartSms(smsContent);
        return 0;
    }

    public boolean isMultiPartSmsOk() {
        boolean bRet = true;
        for (int i = 0; i < mArrayPhoneName.length; i++) {
            if (Build.MODEL.equals(mArrayPhoneName[i])) {
                bRet = false;
                break;
            }
        }
        NmsLog.trace(TAG, "check phone Model:" + Build.MODEL + ", its MultiPartSms cap:" + bRet);
        return bRet;
    }

    public long divideSms(String strMessageIn, String strAddrSingleIn, int saveFlagIn,
            long lThreadIdIn, long simId) {
        long lSmsId = -1;
        SmsCont smsContent = null;
        if (isMultiPartSmsOk()) {
            lSmsId = nmsStoreSms(strMessageIn, strAddrSingleIn, saveFlagIn, lThreadIdIn, simId);
            smsContent = new SmsCont(strAddrSingleIn, strMessageIn, lSmsId, simId);
            mVecSms.add(smsContent);
            return lSmsId;
        }

        ArrayList<String> arrayMsg = NmsPlatformAdapter.getInstance(mContext).divideMessage(
                strMessageIn);
        for (int i = 0; i < arrayMsg.size(); i++) {
            lSmsId = nmsStoreSms(arrayMsg.get(i), strAddrSingleIn, saveFlagIn, lThreadIdIn, simId);
            smsContent = new SmsCont(strAddrSingleIn, arrayMsg.get(i), lSmsId, simId);
            mVecSms.add(smsContent);
        }
        return lSmsId;
    }

    public long nmsStoreSms(String msg, String number, int flag, long threadId, long simId) {
        if (flag == SAVE_SMS && threadId > 0) {
            return NmsSMSMMSManager.getInstance(null).saveSmsToOutBox(msg, number, threadId, simId);
        }
        return -1;
    }

    private synchronized void nmsSendMultiPartSms(SmsCont smsContent) {
        // SmsManager manager = SmsManager.getDefault();
        long lStartTime = 0;
        long lCostTime = 0;
        String prefKey = "pref_key_sms_delivery_reports";
        boolean requestDeliveryReport = false;
        int messageCount;
        
        ArrayList<String> arrayMsg = NmsPlatformAdapter.getInstance(mContext).divideMessage(
                smsContent.strMsg);
        ArrayList<PendingIntent> arrayPiSent = new ArrayList<PendingIntent>();
        
        messageCount = arrayMsg.size();
        ArrayList<PendingIntent> arrayPiDeliver = new ArrayList<PendingIntent>(messageCount);

        try {
            Context context = mContext.createPackageContext("com.android.mms",
                    Context.CONTEXT_IGNORE_SECURITY);
            
            SharedPreferences prefs = context.getSharedPreferences("com.android.mms_preferences",
                    Context.MODE_PRIVATE | Context.MODE_MULTI_PROCESS);
            requestDeliveryReport = prefs.getBoolean(prefKey, false);
            
            if (requestDeliveryReport == false) {
                String sim_preKey = smsContent.simId + "_" + prefKey;
                NmsLog.trace(TAG, "sms delivery report key: " + sim_preKey);
                requestDeliveryReport = prefs.getBoolean(sim_preKey, false);
            }
            NmsLog.trace(TAG, "simId: " + smsContent.simId + " requestDeliveryReport: "
                    + requestDeliveryReport);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        
        for (int i = 0; i < messageCount; i++) {
            Intent intSent = null;
            Intent intDeliver = null;
            PendingIntent piSent = null;
            PendingIntent piDeliver = null;

            intSent = new Intent();
            intSent.setAction(SMS_SENT_ACTION);
            intSent.putExtra(SMS_ID, smsContent.lSmsId);
            piSent = PendingIntent.getBroadcast(mContext, 0, intSent,
                    PendingIntent.FLAG_CANCEL_CURRENT);
            arrayPiSent.add(piSent);
            
            if (requestDeliveryReport && i == messageCount -1) {
                intDeliver = new Intent();
                intDeliver.setAction("com.android.mms.transaction.MessageStatusReceiver.MESSAGE_STATUS_RECEIVED");
                Uri mUri = ContentUris.withAppendedId(NmsSMSMMS.SMS_CONTENT_URI, smsContent.lSmsId);
                intDeliver.setData(mUri);
                intDeliver.setClassName("com.android.mms", "com.android.mms.transaction.MessageStatusReceiver");
                //intDeliver.putExtra(SMS_ID, smsContent.lSmsId);

                piDeliver = PendingIntent.getBroadcast(mContext, 0, intDeliver,
                        PendingIntent.FLAG_CANCEL_CURRENT);
                arrayPiDeliver.add(piDeliver);
            } else {
                arrayPiDeliver.add(null);
            }
        }
        NmsLog.trace(TAG,
                "nmsSendMultiPartSms, smsId:" + smsContent.lSmsId + " smsNum:" + arrayMsg.size()
                        + ", simId: " + smsContent.simId + " messageCount:" + messageCount);
        if (-1 != smsContent.lSmsId) {
            NmsPlatformAdapter.getInstance(mContext).sendMultipartTextMessage(
                    smsContent.strAddr.trim(), null, arrayMsg, (int) smsContent.simId, arrayPiSent,
                    arrayPiDeliver);
            try {
                lStartTime = System.currentTimeMillis();
                wait(arrayMsg.size() * mTimeOut);
                lCostTime = System.currentTimeMillis() - lStartTime;
            } catch (InterruptedException e) {
                NmsLog.nmsPrintStackTrace(e);
            }
            timeoutProcess(lCostTime, smsContent.lSmsId);
        } else {
            NmsPlatformAdapter.getInstance(mContext).sendMultipartTextMessage(
                    smsContent.strAddr.trim(), null, arrayMsg, smsContent.simId, null, null);
        }

        return;
    }

    private synchronized void nmsSendSingleSms(SmsCont smsContent) {
        Intent intSent = null;
        Intent intDeliver = null;
        PendingIntent piSent = null;
        PendingIntent piDeliver = null;
        SmsManager manager = SmsManager.getDefault();
        long lStartTime = 0;
        long lCostTime = 0;
        if (smsContent.lSmsId != -1) // save to db;
        {
            intSent = new Intent();
            intSent.setAction(SMS_SENT_ACTION);
            intSent.putExtra(SMS_ID, smsContent.lSmsId);

            intDeliver = new Intent();
            intDeliver.setAction(SMS_DELIVER_ACTION);
            intDeliver.putExtra(SMS_ID, smsContent.lSmsId);
            NmsLog.trace(TAG, "SendSMS, smsId:" + smsContent.lSmsId);

            piSent = PendingIntent.getBroadcast(mContext, 0, intSent,
                    PendingIntent.FLAG_CANCEL_CURRENT);
            piDeliver = PendingIntent.getBroadcast(mContext, 0, intDeliver,
                    PendingIntent.FLAG_CANCEL_CURRENT);

            manager.sendTextMessage(smsContent.strAddr.trim(), null, smsContent.strMsg, piSent,
                    piDeliver);
            try {
                lStartTime = System.currentTimeMillis();
                wait(mTimeOut);
                lCostTime = System.currentTimeMillis() - lStartTime;
            } catch (InterruptedException e) {
                NmsLog.nmsPrintStackTrace(e);
            }
            timeoutProcess(lCostTime, smsContent.lSmsId);
        } else {
            manager.sendTextMessage(smsContent.strAddr.trim(), null, smsContent.strMsg, null, null);
        }
    }

    private void timeoutProcess(long lCostTimeIn, long lSmsIdIn) {
        if (lCostTimeIn < mTimeOut) {
            NmsLog.trace(TAG, "msg sent notify have been recv, SmsId:" + lSmsIdIn);
        } else {
            NmsLog.trace(TAG, "fail to send msg, because of no intent resp. SmsId:" + lSmsIdIn);
            processFail(lSmsIdIn);
        }
    }

    public synchronized void notifySmsSent() {
        notify();
    }

}
