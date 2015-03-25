package com.hissage.notification;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.RingtoneManager;
import android.net.Uri;
import android.text.TextUtils;

import com.hissage.R;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsConfig;
import com.hissage.download.NmsDownloadManager;
import com.hissage.jni.engineadapterforjni;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpMessageConsts.NmsNewMessageAction;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;

public class NmsNotificationManager {
    private static final String TAG = "NmsNotificationManager";
    private static NmsNotificationManager mInstance = null;

    private NmsNotificationManager() {

    }

    private static Context mContext = null;

    private short mCurrentContactId = 0;
    private short mLastMsgContactRecordId = -1;
    private long mLastSmsId = -1;
    private long mLastMmsId = -1;

    private long mStartTime = 0;
    private long mEndTime = 0;

    private int mCount = 0;
    private static NmsNotificationCenter mNotificationCenter = null;

    public static NmsNotificationManager getInstance(Context c) {
        if (null == mInstance) {
            if (c != null)
                mContext = c.getApplicationContext();
            mInstance = new NmsNotificationManager();
            mNotificationCenter = new NmsNotificationCenter();
            IntentFilter filter = new IntentFilter();
            filter.addAction(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);
            filter.addAction(NmsIntentStrId.NMS_INTENT_CANCEL_ISMS_NOTIFY);
            filter.addAction(NmsIntentStrId.NMS_INTENT_CANCEL_MMSSMS_NOTIFY);
            c.registerReceiver(mNotificationCenter, filter);

        }
        return mInstance;
    }

    public void init() {

    }

    public void nmsEnterChatMode(short contactId) {
        mCurrentContactId = contactId;
        nmsCancelISMSNotification(contactId);
    }

    public void nmsExitOrPauseChatMode() {
        mCurrentContactId = 0;
    }

    private boolean isUsedIMMode(int flag) {
        boolean result = false;
        result = mCurrentContactId != 0 && ((flag & SNmsMsgKey.NMS_MSG_FLAG_REFRESH_CACHE) != 0);
        return result;
    }

    private boolean getVibrate(short contactRecdId) {
        boolean vibrate = false;

        if (false) {// system set silence flag
            return vibrate;
        } else {
            vibrate = true;
        }

        NmsChatSettings chatSettings = new NmsChatSettings(mContext, contactRecdId);
        if (chatSettings.mContactId <= 0) {
            return vibrate;
        }

        if (chatSettings.isNotificationOn()) {
            return chatSettings.isVibrate() && vibrate;
        } else {
            return false;
        }
    }

    private Uri getRingtone(short contactRecdId) {

        Uri ringtone = null;

        if (false) {// system set silence flag

            return ringtone;
        } else {
            ringtone = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);

            // vibrate = true;
        }

        NmsChatSettings chatSettings = new NmsChatSettings(mContext, contactRecdId);
        if (chatSettings.mContactId <= 0) {
            // return vibrate;
            return ringtone;
        }

        if (chatSettings.isNotificationOn() && !TextUtils.isEmpty(chatSettings.mRingtone)) {
            ringtone = Uri.parse(chatSettings.mRingtone);
        } else {
            ringtone = null;
        }

        if (chatSettings.isMute()) {
            ringtone = null;
        }

        return ringtone;

    }

    private String getDefaultDisplayString(int readMode) {
        String result = "";
        switch (readMode) {
        case NmsIpMessageConsts.NmsIpMessageType.TEXT:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG:
            NmsLog.trace(TAG, "getDefaultDisplayString for text is null.");
            break;

        case NmsIpMessageConsts.NmsIpMessageType.PICTURE:
            result = NmsService.getInstance().getString(R.string.STR_NMS_PICTURE);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VOICE:
            result = NmsService.getInstance().getString(R.string.STR_NMS_TAPE);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VCARD:
            result = NmsService.getInstance().getString(R.string.STR_NMS_VCARD);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.LOCATION:
            result = NmsService.getInstance().getString(R.string.STR_NMS_LOCATION);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.SKETCH:
            result = NmsService.getInstance().getString(R.string.STR_NMS_SKETCH);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VIDEO:
            result = NmsService.getInstance().getString(R.string.STR_NMS_VIDEO);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.CALENDAR:
            result = NmsService.getInstance().getString(R.string.STR_NMS_CALENDAR);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.UNKNOWN_FILE:
        default:
            result = NmsService.getInstance().getString(R.string.STR_NMS_UNKNOWN);
        }
        return result;
    }

    public void nmsShowNotification(SNmsMsgKey msgKey) {
        if (null == msgKey
                || !SNmsMsgKey.NMS_IS_HESINE_MSG(msgKey.source)
                || msgKey.status != SNmsMsgKey.NMS_MSG_STATUS_INBOX
                || isUsedIMMode(msgKey.flag)
                || mCurrentContactId == msgKey.contactRecId
                || (((int) msgKey.readMode >= NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG) && (int) msgKey.readMode <= NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG)) {
            NmsLog.trace(TAG, "this msg should not show, msgKey: " + msgKey);
            return;
        }

        mCount++;
        mStartTime = System.currentTimeMillis();
        String strDisplay = "";
        if (mCount > 1) {
            if (msgKey.contactRecId != mLastMsgContactRecordId) {
                msgKey.lineOne = mContext.getString(R.string.STR_NMS_NEW_MSG_TITLE);
            }
            strDisplay = String
                    .format(mContext.getString(R.string.STR_NMS_NEW_SMG_CONTENT), mCount);
        } else {

            if (TextUtils.isEmpty(msgKey.lineTwo)) {
                msgKey.lineTwo = "";
            }

            if (((int) msgKey.readMode <= NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG)
                    || ((int) msgKey.readMode == NmsIpMessageConsts.NmsIpMessageType.UNKNOWN_FILE)) {
                strDisplay = msgKey.lineTwo;
            } else {
                strDisplay = "[" + getDefaultDisplayString(msgKey.readMode) + "]";
            }
        }
        Notification notification = new Notification(R.drawable.isms, msgKey.lineOne, mStartTime);
        notification.flags = Notification.FLAG_AUTO_CANCEL;
        notification.defaults = Notification.DEFAULT_LIGHTS;

        notification.sound = getRingtone(msgKey.contactRecId);
        if (getVibrate(msgKey.contactRecId)) {
            notification.vibrate = new long[] { 50, 200, 100, 200 };
        }

        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.NMS_ACTION_VIEW);
        intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, msgKey.contactRecId);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent pending = PendingIntent.getActivity(mContext, msgKey.recordId, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        notification.setLatestEventInfo(mContext, msgKey.lineOne, strDisplay, pending);

        NotificationManager notiManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        notiManager.notify(1, notification);
        mLastMsgContactRecordId = msgKey.contactRecId;
        if (SNmsMsgKey.NMS_IS_SMS_MSG(msgKey.source)) {
            mLastSmsId = msgKey.platformMsgId;
        } else if (SNmsMsgKey.NMS_IS_MMS_MSG(msgKey.source)) {
            mLastMmsId = msgKey.platformMsgId;
        }
    }

    public void nmsCancelISMSNotification(short contactId) {
        if (contactId == mLastMsgContactRecordId) {
            NotificationManager notiManager = (NotificationManager) mContext
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notiManager.cancel(1);
            mCount = 0;
        }
    }

    public void nmsCancelSMSMMSNotification(long msgId) {
        if (msgId == mLastSmsId || msgId == mLastMmsId) {
            NotificationManager notiManager = (NotificationManager) mContext
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notiManager.cancel(1);
            mCount = 0;
        }
    }

    public void autoDownlaod(NmsIpMessage newMsg, SNmsMsgKey key) {
        if (newMsg == null || key == null) {
            NmsLog.error(TAG, "newMsg/key is/are null");
            return;
        }

        boolean isAutoDown = false;
        if (NmsIpMessageType.LOCATION == newMsg.type) {
            isAutoDown = true;
        } else if (NmsConfig.getAutoDownloadFlag()
                && (NmsIpMessageType.PICTURE == newMsg.type
                        || NmsIpMessageType.SKETCH == newMsg.type
                        || NmsIpMessageType.VIDEO == newMsg.type || NmsIpMessageType.VOICE == newMsg.type)) {
            isAutoDown = true;
        } else {
            isAutoDown = false;
        }

        if (isAutoDown) {
            String strUrl = null;
            if (newMsg.type == NmsIpMessageType.PICTURE && (newMsg instanceof NmsIpImageMessage)) {
                strUrl = ((NmsIpImageMessage) newMsg).url;
            } else if (newMsg.type == NmsIpMessageType.SKETCH
                    && (newMsg instanceof NmsIpImageMessage)) {
                strUrl = ((NmsIpImageMessage) newMsg).url;
            } else if (newMsg.type == NmsIpMessageType.VIDEO
                    && (newMsg instanceof NmsIpVideoMessage)) {
                strUrl = ((NmsIpVideoMessage) newMsg).url;
            } else if (newMsg.type == NmsIpMessageType.VOICE
                    && (newMsg instanceof NmsIpVoiceMessage)) {
                strUrl = ((NmsIpVoiceMessage) newMsg).url;
            } else if (newMsg.type == NmsIpMessageType.LOCATION
                    && (newMsg instanceof NmsIpLocationMessage)) {
                strUrl = ((NmsIpLocationMessage) newMsg).url;
            }
            if (TextUtils.isEmpty(strUrl)) {
                NmsLog.warn(TAG, "url is null, can not download auto");
                return;
            }

            NmsLog.trace(TAG, "recv new mutilmedia msg, and auto download switch is on.sysDbId:"
                    + newMsg.id + "nmsDbId" + newMsg.ipDbId + ", from:" + newMsg.from + ", to: "
                    + newMsg.to);
            NmsDownloadManager.getInstance().nmsDownload(newMsg.ipDbId, newMsg.id);

        }
    }

    private static class NmsNotificationCenter extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (null == intent) {
                return;
            }

            NmsLog.trace(TAG, "notification center recv new intent: " + intent.getAction());

            String action = intent.getAction();
            if (NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION.equals(action)) {
                SNmsMsgKey key = (SNmsMsgKey) intent.getSerializableExtra(SNmsMsgKey.MsgKeyName);
                NmsIpMessage newMsg = (NmsIpMessage) intent
                        .getSerializableExtra(NmsNewMessageAction.NMS_IP_MESSAGE);

                NmsNotificationManager.getInstance(context).autoDownlaod(newMsg, key);

               // NmsNotificationManager.getInstance(context).nmsShowNotification(key);

            } else if (NmsIntentStrId.NMS_INTENT_CANCEL_ISMS_NOTIFY.equals(action)) {

            } else if (NmsIntentStrId.NMS_INTENT_CANCEL_MMSSMS_NOTIFY.equals(action)) {
                long msgId = intent
                        .getLongExtra(NmsIntentStrId.NMS_INTENT_CANCEL_MMSSMS_NOTIFY, -1);
                if (msgId <= 0) {
                    return;
                }
                NmsNotificationManager.getInstance(context).nmsCancelSMSMMSNotification(msgId);
            }
        }

    }

}
