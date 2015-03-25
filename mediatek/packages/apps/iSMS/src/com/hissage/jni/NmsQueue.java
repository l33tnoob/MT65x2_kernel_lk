package com.hissage.jni;

import java.net.InetAddress;

import android.content.Intent;
import android.util.Log;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsProfileSettings;
import com.hissage.contact.NmsContact;
import com.hissage.db.NmsDBUtils;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageStatus;
import com.hissage.message.ip.NmsIpMessageConsts.NmsNewMessageAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsRefreshContactList;
import com.hissage.message.ip.NmsIpMessageConsts.NmsRefreshMsgList;
import com.hissage.message.ip.NmsIpMessageConsts.NmsUpdateGroupAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsUpdateSystemContactAction;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.pn.Config;
import com.hissage.pn.hpnsReceiver;
import com.hissage.receiver.app.NmsAssertAlertReceiver;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsAssertAlertMsgData;
import com.hissage.struct.SNmsInviteInfo;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.struct.SNmsMsgType;
import com.hissage.struct.SNmsRegistrationOverData;
import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;
import com.hissage.util.queue.NmsMessage;
import com.hissage.util.queue.NmsMessagePump;

public class NmsQueue extends Thread {

    private static final String TAG = "NmsMain";

    private static NmsMessagePump mMsgPump = new NmsMessagePump();

    private static final int NMS_SPACE_DB_MSG_COUNT_FULL = 1;
    private static final int NMS_SPACE_TOO_SMALL = 2;
    private static final int NMS_SPACE_WARNING = 3;
    private static final int NMS_SPACE_STOP_ENGINE = 4;
    private static final int NMS_SPACE_DB_CONTACT_COUNT_FULL = 5;
    private static final int NMS_SPACE_DB_MSG_ALMOST_FULL = 6;
    private static final int NMS_SPACE_DB_MSG_WILL_FULL = 7;

    public static final int NMS_PLAY_TONE_ISMS_SUCCEED = 0;
    public static final int NMS_PLAY_TONE_ISMS_FAILED = 1;
    public static final int NMS_PLAY_TONE_SMS_SUCCEED = 2;
    public static final int NMS_PLAY_TONE_SMS_FAIELD = 3;

    public void create() {
        this.start();
        NmsLog.trace(TAG, "UI Msg Q for Jni is running.");
    }

    public NmsQueue() {

    }

    public void run() {
        NmsMessage msg = mMsgPump.getMessage();
        while (msg != null) {
            synchronized (msg) {
                dispatchMessage(msg);
                msg.notifyAll();
            }
            msg = NmsQueue.getMessage();
        }
    }

    private void showNetworkErrorMsgViaMainUI() {
        Intent intent = new Intent();
        intent.putExtra("regStatus", SNmsMsgType.NMS_UI_MSG_CHOOSE_IAP);
        NmsLog.trace(TAG, "is sent to UI reg sendMessage:uiMsgID ="
                + SNmsMsgType.NMS_UI_MSG_CHOOSE_IAP);
        intent.setAction(NmsIntentStrId.NMS_REG_STATUS);
        NmsService.getInstance().sendBroadcast(intent);
    }

    private void showReminedUserOtherLogin() {
        // Intent intent = new Intent();
        // intent.setClass(NmsService.context, MyMain.class);
        // intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        // intent.setAction(MyMain.INTENT_ASK_USR_LOGIN);
        // NmsService.context.startActivity(intent);
    }

    private void showReminedUserRegNewSIMCARD(int msgType) {
        // Intent intent = new Intent();
        // intent.setClass(NmsService.context, MyMain.class);
        // intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        // intent.putExtra("EngineMsgType", msgType);
        // intent.setAction(MyMain.INTENT_ASK_USR_REG_NEW_SIM);
        // NmsService.context.startActivity(intent);
    }

    private void updateChatSettingsMute(SNmsMsgKey msgKey) {

        if (null == msgKey
                || (((int) msgKey.readMode >= NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG) && (int) msgKey.readMode <= NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG)) {
            NmsLog.error(TAG, "this msg should not update settings, msgKey: " + msgKey);
            return;
        }

        NmsChatSettings chatSettings = new NmsChatSettings(NmsService.getInstance(),
                msgKey.contactRecId);
        if (chatSettings.mContactId <= 0) {
            return;
        }
        if (chatSettings.mMute_start > 0) {
            int currentTime = (int) (System.currentTimeMillis() / 1000);
            if ((chatSettings.mMute * 3600 + chatSettings.mMute_start) <= currentTime) {
                chatSettings.mMute = 0;
                chatSettings.mMute_start = 0;
                NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).nmsSetChatSettings(
                        chatSettings);
            }
        }
    }

    private void updateInviteInfo(SNmsMsgKey msgKey) {
//         if (SNmsMsgKey.NMS_IS_HESINE_MSG(msgKey.source)) {
//         NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).nmsDeleteInviteRecd(
//         msgKey.contactRecId);
//         } else if (SNmsMsgKey.NMS_IS_MMS_MSG(msgKey.source)
//         || SNmsMsgKey.NMS_IS_SMS_MSG(msgKey.source)) {
//         SNmsInviteInfo info = new SNmsInviteInfo();
//         info.updateInvite(NmsService.getInstance(), msgKey.contactRecId);
//         }
        if(msgKey.status == 8){
            return ;
        }
        SNmsInviteInfo info = new SNmsInviteInfo();
        info.updateInvite(NmsService.getInstance(), msgKey.contactRecId);
    }

    private void nmsProcessNewMsgNotify(NmsMessage msg) {
        SNmsMsgKey key = msg.getNmsMsgKey();
        if (null == key || key.recordId <= 0) {
            NmsLog.error(TAG, "recv new msg notify, but msg key is null.");
            return;
        }
        updateChatSettingsMute(key);
        updateInviteInfo(key);

        if (!NmsConfig.mIsDBInitDone) {
            NmsLog.error(TAG, "sync sms not over, so ignore this new msg notify.");
            return;
        }

        NmsIpMessage newMsg = engineadapter.get().nmsUIGetMsgKey(key.recordId);
        if (null == newMsg) {
            NmsLog.error(TAG, "recv new msg notify, but can not get the ipmessage via recordId: "
                    + key.recordId);
            return;
        }

        Intent intent = new Intent();
        intent.setAction(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);

        // this key for iSMS standalone only.
        intent.putExtra(SNmsMsgKey.MsgKeyName, key);
        intent.putExtra(NmsNewMessageAction.NMS_IP_MESSAGE, newMsg);
        boolean show = true;
        if (NmsConfig.getCurrentConversation() == key.contactRecId) {
            show = false;
        }
        intent.putExtra(NmsNewMessageAction.NMS_NEED_SHOW_NOTIFICATION, show);
        NmsService.getInstance().sendBroadcast(intent);
        NmsLog.trace(TAG, "new msg brodcast is sent, msg from: " + newMsg.from);
    }

    private void playSendMsgStatus(NmsMessage msg) {
        int status = msg.getMsgDataLen();

        switch (status) {
        case NMS_PLAY_TONE_ISMS_SUCCEED:
        case NMS_PLAY_TONE_SMS_SUCCEED:
        case NMS_PLAY_TONE_ISMS_FAILED:
        case NMS_PLAY_TONE_SMS_FAIELD:
            break;
        default:
            break;
        }
    }

    private void inputPhoneNumber(NmsMessage msg) {
        Intent intent = new Intent();
        // if (msg.getMsgDataLen() != 1) {
        // intent.setAction(NmsIntentStrId.NMS_REG_SMS_ERROR);
        // } else {
        intent.setAction(NmsIntentStrId.NMS_REG_INPUT_PHONENUM);
        // }

        NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG, "send brodcast: " + intent.getAction());
    }

    private void newUserNotify(NmsMessage msg) {
        Intent intent = new Intent();
        intent.putExtra(NmsIntentStrId.NMS_ACTIVE, 0);
        intent.setAction(NmsIntentStrId.NMS_INTENT_REGISTRATION);
        NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG, "brodcast " + NmsIntentStrId.NMS_INTENT_REGISTRATION
                + " is sent, Extra: " + intent.getIntExtra(NmsIntentStrId.NMS_ACTIVE, -1));
    }

    private void refreshGroupInfo(NmsMessage msg) {
        if (!NmsConfig.mIsDBInitDone) {
            NmsLog.error(TAG, "db not init done, so ignore this msg: " + msg.getIdentity());
            return;
        }
        int groupId = msg.getMsgDataLen();
        Intent intent = new Intent();
        intent.setAction(NmsUpdateGroupAction.NMS_UPDATE_GROUP);
        intent.putExtra(NmsUpdateGroupAction.NMS_UPDATE_GROUP, SNmsMsgType.NMS_UI_MSG_GROUP_CHANGED);
        intent.putExtra(NmsUpdateGroupAction.NMS_GROUP_ID, groupId);
        NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG, "Update group brodcast is send, msg id: " + msg.getIdentity());
    }

    private void refreshOrUpdateMsgList(NmsMessage msg) {
        if (!NmsConfig.mIsDBInitDone) {
            NmsLog.error(TAG, "db not init done, so ignore this msg: " + msg.getIdentity());
            return;
        }
        Intent intent = new Intent();
        intent.setAction(NmsRefreshMsgList.NMS_REFRESH_MSG_LIST);
        NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG,
                "Refresh or update msg list brodcast is send, msg id: " + msg.getIdentity());
    }

    private void refreshOrUpdateMsg(NmsMessage msg) {
        if (!NmsConfig.mIsDBInitDone) {
            NmsLog.error(TAG, "db not init done, so ignore this msg: " + msg.getIdentity());
            return;
        }
        if (null == msg.getNmsMsgKey()) {
            NmsLog.error(TAG, "recv update msg, but msgkey is null");
            return;
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageStatus.NMS_MESSAGE_STATUS_ACTION);
        short recdId = msg.getNmsMsgKey().recordId;
        intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_RECD_ID, recdId);
        intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_SYS_ID, msg.getNmsMsgKey().platformMsgId);
        intent.putExtra(NmsIpMessageConsts.STATUS, msg.getNmsMsgKey().status);
		NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG,
                "Refresh or update one msg brodcast is send, msg id: " + msg.getIdentity()
                        + ", msg recd id: " + recdId);
    }

    private void notifyRegStatus(NmsMessage msg) {
        Intent intent = new Intent();
        int msgId = msg.getIdentity();
        intent.putExtra("regStatus", msg.getIdentity());
        if (SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER == msgId) {
            
            if (Config.PN) {
                hpnsReceiver.loadPn(NmsService.getInstance().getApplicationContext());
                NmsLog.trace(TAG,"LoadPn complete.") ;
            }
            
            SNmsRegistrationOverData data = msg.getRegOverData();
            if (data != null && data.isNewUser == 1) {
                intent.putExtra("isNewUser", true);
            }
        }
        intent.setAction(NmsIntentStrId.NMS_REG_STATUS);
        NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG, "is sent to UI reg sendMessage:uiMsgID =" + msg.getIdentity());
    }

	private void notifyDbFullStatus(NmsMessage msg) {
		Intent intent = new Intent();
		int msgId = msg.getIdentity();
		
        if (SNmsMsgType.NMS_UI_MSG_DB_FULL == msgId)
            intent.putExtra(NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_STATUS , NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_FULL);
        else
            intent.putExtra(NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_STATUS , NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_ERR);
            
		intent.setAction(NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_STATUS_ACTION);

		NmsService.getInstance().sendBroadcast(intent);

		NmsLog.trace(TAG, "is send to UI DB Full notication: uiMsgID=" + msg.getIdentity());
	}

    private void refreshContact(NmsMessage msg) {
        if (!NmsConfig.mIsDBInitDone) {
            NmsLog.error(TAG, "db not init done, so ignore this msg: " + msg.getIdentity());
            return;
        }
        int engContactId = msg.getMsgDataLen();
        Intent intent = new Intent();
        intent.setAction(NmsRefreshContactList.NMS_REFRESH_CONTACTS_LIST);
        intent.putExtra(NmsRefreshContactList.NMS_REFRESH_CONTACTS_LIST,
                SNmsMsgType.NMS_UI_MSG_REFRESH_CONTACT);
        intent.putExtra(NmsRefreshContactList.NMS_CONTACT_ID, engContactId);
        NmsService.getInstance().sendBroadcast(intent);

        NmsLog.trace(TAG, "refresh contact brodcast is send, msg id: " + msg.getIdentity());
    }

    private void contactUpdate(NmsMessage msg) {
        int contactId = msg.getMsgDataLen();
        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId((short) contactId);
        if (null == contact) {
            NmsLog.error(TAG, "contact update, but can not get contact via isms db id: "
                    + contactId);
            return;
        }

        Intent intent = new Intent();
        intent.putExtra(NmsIpMessageConsts.NmsImStatus.NMS_CONTACT_CURRENT_STATUS, contact);
        intent.setAction(NmsIpMessageConsts.NmsImStatus.NMS_IM_STATUS_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
        NmsLog.trace(TAG, "update contact brodcast is sent, isms db contact id: " + contactId);
    }

    private void notifySimStatus(NmsMessage msg) {
        int status = msg.getMsgDataLen();
        Intent intent = new Intent();
        intent.putExtra(NmsIpMessageConsts.NmsSimStatus.NMS_SIM_STATUS, status);
        intent.setAction(NmsIpMessageConsts.NmsSimStatus.NMS_SIM_STATUS_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
        NmsLog.trace(TAG, "sim card status is sent, status is: " + status);
    }
    
    private void notifySimInfoChanged(NmsMessage msg) {
        int simId = msg.getMsgDataLen();
        NmsLog.trace(TAG, "get sims info chaned msg, simId: " + simId) ;
        
        if(!NmsPlatformAdapter.getInstance(NmsService.getInstance()).isMtkGeminiSupport()){
            simId = NmsConsts.SINGLE_CARD_SIM_ID;
            NmsLog.trace(TAG, "notifySimInfoChanged single card simId: " + simId) ;
        }
        Intent intent = new Intent();
        intent.putExtra(NmsIpMessageConsts.NmsSimInfoChanged.NMS_SIM_ID, simId);
        intent.setAction(NmsIpMessageConsts.NmsSimInfoChanged.NMS_SIM_INFO_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
    }
    
    private void notifyContactImgUpdated(NmsMessage msg) {
        NmsLog.trace(TAG, "get contact img updated msg") ;
        Intent intent = new Intent();
        intent.setAction(NmsUpdateSystemContactAction.NMS_UPDATE_CONTACT);
        NmsService.getInstance().sendBroadcast(intent);
    }

    private void processCmdResult(int code) {

        int cmdCode = code >> 16;
        int cmdResult = code & 0xFFFF;
        NmsLog.trace(TAG, "Recv Cmd Result, cmd: " + cmdCode + ", Result: " + cmdResult);
        if (NmsProfileSettings.CMD_GET_HESINE_INFO_ACK == cmdCode
                || NmsProfileSettings.CMD_SET_HESINE_INFO_ACK == cmdCode) {

            Intent intent = new Intent();
            intent.putExtra(NmsProfileSettings.CMD_RESULT_KEY, cmdResult);
            intent.putExtra(NmsProfileSettings.CMD_CODE, cmdCode);
            intent.setAction(NmsProfileSettings.CMD_RESULT_KEY);
            NmsService.getInstance().sendBroadcast(intent);
        }
    }

    private void processAssertAlert(NmsMessage msg) {
        SNmsAssertAlertMsgData data = msg.getAssertAlertMsgData();
        if (data != null) {
            Intent intent = new Intent();
            intent.setAction(NmsAssertAlertReceiver.INTENT_ACTION);
            intent.putExtra(NmsAssertAlertReceiver.INTENT_KEY, data);
            if (NmsService.getInstance() != null) {
                NmsService.getInstance().sendBroadcast(intent);
            }
        }
    }

    private void doFetchDNS() {

        if (engineadapter.get().nmsUIIsActivated()) {
            NmsLog.trace(TAG, "iSMS is activated now, dont do the fetch ip from DNS logic");
            return;
        }

        try {
            new Thread() {
                public void run() {
                    try {
                        InetAddress host = InetAddress.getByName(engineadapter.get()
                                .nmsGetDNSHost());
                        NmsLog.trace(TAG, "fetch ip from Isms DNS: " + host);

                    } catch (Exception e) {
                        NmsLog.nmsPrintStackTrace(e);
                    }
                }
            }.start();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private void createAllCatchPath() {
        NmsCommonUtils.getAudioCachePath(NmsService.getInstance());
        NmsCommonUtils.getPicCachePath(NmsService.getInstance());
        NmsCommonUtils.getVideoCachePath(NmsService.getInstance());
        NmsCommonUtils.getVcardCachePath(NmsService.getInstance());
        NmsCommonUtils.getVcalendarCachePath(NmsService.getInstance());
        NmsCommonUtils.getCachePath(NmsService.getInstance());
    }

    private void dispatchMessage(NmsMessage msg) {

        NmsLog.trace(TAG, "process msg from engine, msgId: " + msg.getIdentity());

        switch (msg.getIdentity()) {
        case SNmsMsgType.NMS_UI_MSG_CHOOSE_IAP:
            showNetworkErrorMsgViaMainUI();
            break;

        case SNmsMsgType.NMS_UI_MSG_INPUT_MOBILE_NUM:
            inputPhoneNumber(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_NEW_USER_NOTIFY:
            newUserNotify(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER:
            // do not break;
        case SNmsMsgType.NMS_UI_MSG_REGISTRATION_FAIL:
            // do not break;
            // case SNmsMsgType.NMS_UI_MSG_ENGINE_STOPPED:
        case SNmsMsgType.NMS_UI_MSG_REGISTRATION_IN_PROGRESS:
        case SNmsMsgType.NMS_UI_MSG_REMIND_USER_TO_SET_NAME:
            notifyRegStatus(msg);
            break;

        case SNmsMsgType.NMS_UI_CFG_CMD_RESULT:
            processCmdResult(msg.getMsgDataLen());
            break;

        case SNmsMsgType.NMS_UI_MSG_REFRESH:
            refreshOrUpdateMsgList(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_UPDATE_MSG:
            refreshOrUpdateMsg(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_NEW_MSG:
            Log.i("test", ".........NMS_UI_MSG_NEW_MSG..7777......key.recordId.");
            nmsProcessNewMsgNotify(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_PLAY_SEND_MSG_TONE:
            playSendMsgStatus(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_CLIENT_UPGRADE:
            NmsConfig.setUpdataVersionFlag(1);
            break;

        case SNmsMsgType.NMS_UI_MSG_STORAGE_ERROR:
            // TODO jhnie, notify storage error
            // showWarningNotification(SNmsMsgType.NMS_UI_MSG_STORAGE_ERROR, 0);
            break;

        case SNmsMsgType.NMS_UI_MSG_DB_FULL:
            // TODO jhnie, notify DB error here
            // int errorcode = msg.getMsgDataLen();
            // showWarningNotification(SNmsMsgType.NMS_UI_MSG_DB_FULL,
            // errorcode);
            notifyDbFullStatus(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_ENGINE_INIT_DONE:
            NmsSMSMMSManager.getInstance(null).nmsSyncSMS();
            if(!NmsUpgradeManager.mUpgradeStarted){
                NmsLog.trace(TAG, "nms upgrade from engine init done");
                NmsUpgradeManager.start(false);
            }
            doFetchDNS();
            createAllCatchPath();
            break;

        case SNmsMsgType.NMS_UI_MSG_CONTACT_REFRESH:
            refreshContact(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_REMIND_USER_OTHER_LOGINED:
            showReminedUserOtherLogin();
            break;

        case SNmsMsgType.NMS_UI_MSG_CHANGE_ACTUAL_IMSI:
            showReminedUserRegNewSIMCARD(SNmsMsgType.NMS_UI_MSG_CHANGE_ACTUAL_IMSI);
            break;

        case SNmsMsgType.NMS_UI_MSG_CHANGE_VIRTUAL_IMSI:
            showReminedUserRegNewSIMCARD(SNmsMsgType.NMS_UI_MSG_CHANGE_VIRTUAL_IMSI);
            break;

        case SNmsMsgType.NMS_UI_MSG_ASK_USER_SEND_REGSMS:
            int i = engineadapter.msgtype.NMS_ENG_MSG_USER_ALLOW_SEND_REGSMS.ordinal();
            engineadapter.get().nmsSendMsgToEngine(i, 0, 0);
            break;

        case SNmsMsgType.NMS_UI_MSG_GROUP_CHANGED:
            refreshGroupInfo(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_CONTACT_STATUS_UPDATED:
            contactUpdate(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_ASSERT_ALERT:
            processAssertAlert(msg);
            break;

        case SNmsMsgType.NMS_UI_MSG_OP_RESULT:
            notifySimStatus(msg);
            break;
            
        case SNmsMsgType.NMS_UI_MSG_SIM_INFO_CHANGED:
            notifySimInfoChanged(msg);
            break;
            
        case SNmsMsgType.NMS_UI_MSG_CONTACT_IMG_UPDATED:
            notifyContactImgUpdated(msg);
            break;

        default:
            NmsLog.trace(TAG, "not necessary handle case: " + msg.getIdentity());
            break;

        }
    }

    public static void postMessage(NmsMessage msg) {
        mMsgPump.postMessage(msg);
    }

    public static NmsMessage getMessage() {
        return mMsgPump.getMessage();
    }
}
