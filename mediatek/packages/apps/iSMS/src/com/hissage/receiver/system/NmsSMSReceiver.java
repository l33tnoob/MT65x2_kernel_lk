package com.hissage.receiver.system;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SmsMessage;
import android.text.TextUtils;

import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.jni.engineadapter;
import com.hissage.message.smsmms.NmsSMSMMS;
import com.hissage.platfrom.NmsMtkPhone;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsSMSReceiver extends BroadcastReceiver {
    private static final String TAG = "NmsRegSMSInterrupt";
    private String mStrRegPhone = null;
    private static NmsSMSReceiver mInstance = null;

    public static NmsSMSReceiver getInstance() {
        if (null == mInstance) {
            mInstance = new NmsSMSReceiver();
        }
        return mInstance;
    }

    private NmsSMSReceiver() {

    }

    public void setRegPhone(String strPhoneNumIn) {
        mStrRegPhone = strPhoneNumIn;
    }

    @Override
    public void onReceive(Context context, Intent intent) {

        if (intent.getAction().equals(NmsSMSMMS.SMS_RECEIVED) && NmsConfig.mIsDBInitDone) {
            NmsLog.trace(TAG, "catch a sms broadcast.");

            Bundle bundle = intent.getExtras();
            if (null != bundle) {
                String smsAddress = "";
                String smsBody = "";
                Object[] messages = (Object[]) bundle.get("pdus");

                SmsMessage[] smsMessages = new SmsMessage[messages.length];
                NmsLog.trace(TAG, "real-time messages retrieve count is " + messages.length);
                for (int i = 0; i < messages.length; i++) {
                    if (null == messages[i]) {
                        NmsLog.trace(TAG,
                                "fail to process a sms broadcast, message cont is null.");
                        return;
                    }
                    smsMessages[i] = SmsMessage.createFromPdu((byte[]) messages[i]);
                }

                for (SmsMessage message : smsMessages) {
                    smsAddress = message.getOriginatingAddress();
                    smsBody += message.getMessageBody();
                }

                if (TextUtils.isEmpty(smsAddress)) {
                    NmsLog.trace(TAG, "fail to process a sms broadcast, address is null.");
                    return;
                }
                
                String nocPrefix = engineadapter.get().nmsUIGetNqNocPrefix() ;
                
                if ((!TextUtils.isEmpty(mStrRegPhone) && smsAddress.endsWith(mStrRegPhone)) 
                        || (nocPrefix != null && !TextUtils.isEmpty(nocPrefix) && smsBody.startsWith(nocPrefix))
                        || (engineadapter.get().nmsUIIsSelfRegSms(smsBody) != 0)) {
                    long simId = NmsPlatformAdapter.getInstance(context).getCurrentSimId();
                    int realSimId = NmsConsts.INVALID_SIM_ID;
                    NmsMtkPhone nmsMtkPhone = new NmsMtkPhone(context);
                    String simIdKey = nmsMtkPhone.getSimIdKey(context);
                    if(!TextUtils.isEmpty(simIdKey)){
                        realSimId = intent.getIntExtra(simIdKey, NmsConsts.INVALID_SIM_ID);
                    }
                    if(NmsConsts.INVALID_SIM_ID == realSimId){
                        realSimId = (int)simId;
                    }else{
                        realSimId = (int)NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(realSimId);
                    }
                    NmsLog.trace(TAG, "recv reg sms, address:" + smsAddress + ", cont:" + smsBody+", simId: "+realSimId);
                    com.hissage.jni.engineadapter.get().nmsProcessInterceptedSms(smsAddress,
                            null, smsBody, NmsSMSMMS.PROTOCOL_SMS, -1,
                            SNmsMsgKey.NMS_MSG_STATUS_INBOX, 0, 0, 0, 0, null, realSimId);
                    mStrRegPhone = null;
                    abortBroadcast();
                } 
//                else {
//                    NmsLog.trace(TAG, "recv normal sms, address:" + smsAddress + ", cont:"
//                            + smsBody);
//                    NmsSMSMMSManager.getInstance().saveSmsToInBox(smsBody, smsAddress);
//                }
//                abortBroadcast();
            }

        }
    }

}
