package com.hissage.receiver.app;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.hissage.jni.NmsQueue;
import com.hissage.jni.engineadapterforjni;
import com.hissage.message.smsmms.NmsSMSMMS;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.message.smsmms.NmsSendMessage;
import com.hissage.struct.SNmsMsgType;
import com.hissage.util.log.NmsLog;
public class NmsSendSmsBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "NmsSendSmsBroadcastReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        //
        if (NmsSendMessage.SMS_SENT_ACTION.equals(intent.getAction())) {
            Bundle bdData = intent.getExtras();
            long lSmsId = (int) bdData.getLong(NmsSendMessage.SMS_ID);
            int nErrorCode = getResultCode();
            NmsLog.trace(TAG, "BroadcastReceiver update sms id:" + lSmsId + " status:"
                    + NmsSMSMMS.SMS_TYPE_SENT + " resultCode:" + nErrorCode);
            processSentIntent(context, lSmsId, nErrorCode);

        } else if (NmsSendMessage.SMS_DELIVER_ACTION.equals(intent.getAction())) {
            Bundle bdData = intent.getExtras();
            int nSmsId = (int) bdData.getLong(NmsSendMessage.SMS_ID);
            byte[] pdu = bdData.getByteArray("pdu");
            
            NmsLog.trace(TAG, "Deliver report:" + pdu + ", BroadcastReceiver update sms id:"
                    + nSmsId + " deliver:" + NmsSMSMMS.SMS_DELIVER_OK);
            
            // Notes: can not handle deliver report perfectly, so we will not
            // add this feature now;
            // NmsSMSMMSManager.getInstance().updateSmsStatusViaId(NmsSMSMMS.PROTOCOL_SMS,
            // nSmsId, NmsSMSMMS.SMS_TYPE_SENT, NmsSMSMMS.SMS_DELIVER_OK);
        } else {
            // error handle;
            NmsLog.trace(TAG, "BroadcastReceiver unhandle action:" + intent.getAction());
        }
    }

    public void processSentIntent(Context context, long lSmsIdIn, int nErrorCode) {
        // need to be discussed, added
        // by luozheng in 12.04.05;
        if ((nErrorCode == Activity.RESULT_OK)) {
            NmsSMSMMSManager.getInstance(context).updateSmsStatusViaId(NmsSMSMMS.PROTOCOL_SMS,
                    (int) lSmsIdIn, NmsSMSMMS.SMS_TYPE_SENT, NmsSMSMMS.SMS_DELIVER_UNKNOWN);
            engineadapterforjni.nmsSendMsgToUI(SNmsMsgType.NMS_UI_MSG_PLAY_SEND_MSG_TONE, null,
                    NmsQueue.NMS_PLAY_TONE_SMS_SUCCEED);
            NmsSendMessage.getInstance().notifySmsSent();
        } else {
            NmsSMSMMSManager.getInstance(context).updateSmsStatusViaId(NmsSMSMMS.PROTOCOL_SMS,
                    (int) lSmsIdIn, NmsSMSMMS.SMS_TYPE_FAILED, NmsSMSMMS.SMS_DELIVER_UNKNOWN);
            engineadapterforjni.nmsSendMsgToUI(SNmsMsgType.NMS_UI_MSG_PLAY_SEND_MSG_TONE, null,
                    NmsQueue.NMS_PLAY_TONE_SMS_FAIELD);
            NmsSendMessage.getInstance().notifySmsSent();
        }

    }
}