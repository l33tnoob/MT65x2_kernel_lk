package com.hissage.api;

import java.io.File;
import java.util.Date;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.text.TextUtils;
import android.util.Log;

import com.hissage.R;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.config.NmsProfileSettings;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactStauts;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.db.NmsDBUtils;
import com.hissage.download.NmsDownloadManager;
import com.hissage.jni.engineadapter;
import com.hissage.message.NmsDownloadMessageHistory;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDelIpMessageAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsSaveHistory;
import com.hissage.message.ip.NmsIpMsgApiUtils;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.smsmms.NmsCreateSmsThread;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.pn.HpnsApplication;
import com.hissage.receiver.system.NmsSMSReceiver;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsInviteInfo;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;
//M: Activation Statistics
import com.hissage.util.statistics.NmsPrivateStatistics;
import com.hissage.util.statistics.NmsStatistics;

public class NmsIpMessageApiNative {

    public static boolean nmsServiceIsReady() {
        return NmsConfig.mIsDBInitDone;
    }

    /* Get indicated sim card current activation status */
    public static int nmsGetActivationStatus(int sim_id) {
        if (NmsConsts.SINGLE_CARD_SIM_ID == sim_id) {
            NmsLog.error(HissageTag.api, "single card simId " + sim_id);
            sim_id = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        if (sim_id < 0) {
            NmsLog.error(HissageTag.api, "error param at nmsGetActivationStatus: " + sim_id);
            return -1;
        }

        SNmsSimInfo simInfo = nmsGetSimInfoViaSimId(sim_id);
        if (simInfo != null) {
            return simInfo.status;
        } else {
            return NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST;
        }
    };

    /*
     * simId: if simId == -1, that mean activated all sim card If the
     * nmsActivationStatus() tell that service is not activate yet, UI need to
     * call this function to do the activation
     */
    public static void nmsActivateIpService(int sim_id) {

        if (NmsConsts.SINGLE_CARD_SIM_ID == sim_id) {
            NmsLog.error(HissageTag.api, "single card simId " + sim_id);
            sim_id = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        if (sim_id < 0) {
            NmsLog.error(HissageTag.api, "error param at nmsActivateIpService: " + sim_id);
            return;
        }
        engineadapter.get().nmsUIActivateSimCard(sim_id);
    };

    /*
     * When service send a nmsNotification.ActivationStatusChanged msg and msg's
     * argument is nmsActivationStatus.WaitForUIInputNumber UI should show an
     * input phone number dialog, and call this function with what user inputed
     * .
     */
    public static void nmsInputNumberForActivation(String phoneNumber, int sim_id) {

        if (NmsConsts.SINGLE_CARD_SIM_ID == sim_id) {
            NmsLog.error(HissageTag.api, "single card simId " + sim_id);
            sim_id = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        if (TextUtils.isEmpty(phoneNumber) || (sim_id < 0 && sim_id != NmsConsts.ALL_SIM_CARD)) {
            NmsLog.error(HissageTag.api, "error param at nmsInputNumberForActivation: "
                    + phoneNumber + ", id: " + sim_id);
            return;
        }
        NmsSMSReceiver.getInstance().setRegPhone(phoneNumber);

        engineadapter.get().nmsInputNumberForActivation(phoneNumber, sim_id);

    };

    /*
     * Start the Ip message service, must call this first before using below api
     * , and notification: ServiceStatus will be received
     */
    public static void nmsEnableIpService(int sim_id) {

        if (NmsConsts.SINGLE_CARD_SIM_ID == sim_id) {
            NmsLog.error(HissageTag.api, "single card simId " + sim_id);
            sim_id = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        if (sim_id < 0) {
            NmsLog.error(HissageTag.api, "error param at nmsEnableIpService: " + sim_id);
            return;
        }
        engineadapter.get().nmsUIEnableSimService(sim_id);
    };

    /*
     * Turn off the IP message service, so mobile won't connect to iSMS server
     * anymore until nmsTurnOnService is called
     */
    public static void nmsDisableIpService(int sim_id) {

        if (NmsConsts.SINGLE_CARD_SIM_ID == sim_id) {
            NmsLog.error(HissageTag.api, "single card simId " + sim_id);
            sim_id = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        if (sim_id < 0) {
            NmsLog.error(HissageTag.api, "error param at nmsDisableIpService: " + sim_id);
            return;
        }
        engineadapter.get().nmsUIDisableSimService(sim_id);
    };

    /* Get the message detail */
    public static NmsIpMessage nmsGetIpMsgInfo(long msgId) {
        if (msgId <= 0) {
            NmsLog.error(HissageTag.api, "nmsGetIpMsgInfo param error: " + msgId);
            return null;
        }
        short messageId = NmsSMSMMSManager.getInstance(null).getNmsRecordIDViaSysId((int) msgId);

        NmsIpMessage ipMsg = nmsGetIpMsgInfoViaISmsId(messageId);

        if (ipMsg == null) {
            NmsLog.error(HissageTag.api, "nmsGetIpMsgInfo get ipMsg info error id: " + msgId);
            ipMsg = new NmsIpTextMessage((int) msgId, 0, 0,
                    NmsIpMessageConsts.NmsIpMessageType.TEXT,
                    NmsIpMessageConsts.NmsIpMessageStatus.INBOX,
                    NmsIpMessageConsts.NmsMessageProtocol.IP, true, 0, 0, 0, "11111111",
                    "11111111", NmsService.getInstance().getString(
                            R.string.STR_NMS_SHORTCUT_REFRESHING));
        }

        return ipMsg;
    }

    public static NmsIpMessage nmsGetIpMsgInfoViaISmsId(short messageId) {
        NmsLog.trace(HissageTag.api, "get detail message by messageId:" + messageId);

        if (messageId <= 0) {
            NmsLog.trace(HissageTag.api, "The messageId valule is out of messageId:" + messageId);
            return null;
        }

        NmsIpMessage ipMessage = engineadapter.get().nmsUIGetMsgKey((short) messageId);

        if (null != ipMessage) {
            if (ipMessage.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
                NmsIpLocationMessage locationMessage = (NmsIpLocationMessage) ipMessage;
                ipMessage = NmsIpLocationMessage.formatLocationMsg(locationMessage);
            }
        } else {
            NmsLog.trace(HissageTag.api, "The detail message object is null, and the messageId:"
                    + messageId);
        }

        return ipMessage;
    }

    /*
     * 
     * Save new IP msg to DB. If message status is set to oubox, UI will receive
     * notification:nmsMessageStatus later. For parameter sendMsgMode, if it is
     * set to NORMAL, it means the message will be kept in server if it could
     * not be delivered, but it will be delivered again once the recipient is
     * online again. If this parameter is set to Auto, the message will be
     * re-sent via SMS/MMS if the message could not be delivered.
     */
    public static int nmsSaveIpMsg(NmsIpMessage msg, int sendMsgMode, boolean delDraft,
            boolean delDraftInSysDb) {
        if (sendMsgMode == NmsIpMessageConsts.NmsIpMessageSendMode.AUTO) {
            if (!NmsConfig.getSendAsSMSFlag()) {
                sendMsgMode = NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL;
            }
        }
        return NmsIpMsgApiUtils.nmsSaveMsgIntoDB(msg, sendMsgMode, delDraft, delDraftInSysDb);
    }

    /* Delete messages in DB */
    public static void nmsDeleteIpMsg(final short[] ids, final boolean delImportant,
            final boolean isDelMsgInSmsDb) {
        if (null == ids || ids.length <= 0) {
            NmsLog.error(HissageTag.api, "nmsDeleteIpMsg param error: " + ids);
            return;
        }

        new Thread() {
            @Override
            public void run() {
                engineadapter.get().nmsUIDeleteMsg(ids, ids.length, delImportant ? 1 : 0,
                        isDelMsgInSmsDb ? 1 : 0);

                Intent intent = new Intent();
                intent.setAction(NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE);
                intent.putExtra(NmsDelIpMessageAction.NMS_IP_MESSAGE_DB_ID, ids);
                NmsService.getInstance().sendBroadcast(intent);
            }
        }.start();

    };

    public static void nmsDeleteIpMsgViaThreadId(long threadId, boolean isDelMsgInSmsDb) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "nmsDeleteIpMsgViaThreadId param error: " + threadId);
            return;
        }

        NmsContact contact = nmsGetContactInfoViaThreadId(threadId);
        if (null == contact) {
            NmsLog.error(HissageTag.api, "get contact via thread id is error: " + threadId);
            return;
        }

        engineadapter.get().nmsUIDeleteMsgViaContactRecId(contact.getId(), 1, 1,
                isDelMsgInSmsDb ? 1 : 0);

    }

    /* tell ip msg service that an Ip msg readed by user */
    public static void nmsSetIpMsgAsViewed(short nmsRecdId) {
        if (nmsRecdId <= 0) {
            NmsLog.error(HissageTag.api, "nmsSetIpMsgAsViewed nmsRecdId error: " + nmsRecdId);
            return;
        }
        engineadapter.get().nmsUpdateIpMsgReaded(nmsRecdId);
    };

    /*
     * Download message attachment, asynchronous, notification: DownloadStatus
     * will be received
     */
    public static void nmsDownloadAttach(long msgId, long smsId) {
        NmsDownloadManager.getInstance().nmsDownload(msgId, smsId);
    };

    /*
     * cancel downloading, asynchronous, notification: DownloadStatus will be
     * received, status is Failed
     */
    public static void nmsCancelDownloading(long msgId, long smsId) {
        NmsDownloadManager.getInstance().nmsCancelDownload(msgId, smsId);
    };

    public static boolean nmsIsDownloading(long msgId, long smsId) {
        return NmsDownloadManager.getInstance().nmsIsDownloading(msgId, smsId);
    }

    public static int nmsGetDownloadProgress(long msgId, long smsId) {
        return NmsDownloadManager.getInstance().nmsGetProgress(msgId, smsId);
    }

    public static NmsContact nmsGetContactInfoViaThreadId(long threadId) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "threadId is invalid!");
            return null;
        }

        short engineContactId = NmsContactApi.getInstance(HpnsApplication.mGlobalContext)
                .getEngineContactIdViaSystemThreadId(threadId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid!");
            return null;
        }

        return nmsGetContactInfoViaEngineId(engineContactId);
    }

    /* Get contact information with message id */
    public static NmsContact nmsGetContactInfoViaMsgId(long msgId) {
        if (msgId <= 0) {
            NmsLog.error(HissageTag.api, "msgId is invalid!");
            return null;
        }

        short engineContactId = NmsContactApi.getInstance(null).getEngineContactIdViaSystemMsgId(
                msgId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engine ContactId is invalid!");
            return null;
        }

        return nmsGetContactInfoViaEngineId(engineContactId);
    }

    /* Get hissage information with a phone number */
    public static NmsContact nmsGetContactInfoViaNumber(String phoneNumber) {
        if (TextUtils.isEmpty(phoneNumber)) {
            NmsLog.error(HissageTag.api, "phoneNumber is empty!");
            return null;
        }

        short engineContactId = NmsContactApi.getInstance(null).getEngineContactIdViaNumber(
                phoneNumber);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engine ContactId is invalid!");
            return null;
        }

        return nmsGetContactInfoViaEngineId(engineContactId);
    };

    public static NmsContact nmsGetContactInfoViaEngineId(short engineContactId) {
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid!");
            return null;
        }

        NmsContact contact = engineadapter.get().nmsUIGetContact(engineContactId);
        if (contact != null && contact.getType() != NmsContactType.HISSAGE_BROADCAST
                && contact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
            String name = NmsContactApi.getInstance(null).getSystemNameViaNumber(
                    contact.getNumber());
            if (!TextUtils.isEmpty(name)) {
                contact.setName(name);
            }
        }

        return contact;
    }

    public static Bitmap nmsGetContactAvatarViaThreadId(long threadId) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "threadId is invalid!");
            return null;
        }

        short engineContactId = NmsContactApi.getInstance(null)
                .getEngineContactIdViaSystemThreadId(threadId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid!");
            return null;
        }

        return nmsGetContactAvatarViaEngineId(engineContactId);
    }

    public static Bitmap nmsGetContactAvatarViaMsgId(long msgId) {
        if (msgId <= 0) {
            NmsLog.error(HissageTag.api, "msgId is invalid!");
            return null;
        }

        short engineContactId = NmsContactApi.getInstance(null).getEngineContactIdViaSystemMsgId(
                msgId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engine ContactId is invalid!");
            return null;
        }

        return nmsGetContactAvatarViaEngineId(engineContactId);
    }

    public static Bitmap nmsGetContactAvatarViaNumber(String phoneNumber) {
        if (TextUtils.isEmpty(phoneNumber)) {
            NmsLog.error(HissageTag.api, "phoneNumber is empty!");
            return null;
        }

        short engineContactId = NmsContactApi.getInstance(null).getEngineContactIdViaNumber(
                phoneNumber);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engine ContactId is invalid!");
            return null;
        }

        return nmsGetContactAvatarViaEngineId(engineContactId);
    }

    public static Bitmap nmsGetContactAvatarViaEngineId(short engineContactId) {
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid!");
            return null;
        }

        return NmsContactApi.getInstance(null).getAvatarViaEngineContactId(engineContactId);
    }

    public static boolean nmsIsiSMSNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(HissageTag.api, "nmsIsiSMSNumber. number is empty!");
            return false;
        }

        return NmsContactApi.getInstance(null).isHissageNumber(number);
    }

    public static boolean nmsNeedShowInviteDlg(Context context, short contactId) {
        SNmsInviteInfo info = new SNmsInviteInfo();
        return info.needShowInviteDlg(context, contactId);
    }

    public static int nmsNeedShowReminderDlg(Context context, short contactId) {
        SNmsInviteInfo info = new SNmsInviteInfo();
        return info.needShowReminderDlg(context, contactId);
    }

    public static boolean nmsHandleInviteDlgLaterCmd(Context context, short contactId) {
        SNmsInviteInfo info = new SNmsInviteInfo();
        return info.setLater(context, contactId);
    }

    public static boolean nmsHandleInviteDlgInviteCmd(Context context, short contactId) {
        SNmsInviteInfo info = new SNmsInviteInfo();
        return info.setInvite(context, contactId);
    }

    public static boolean nmsNeedShowSwitchAcctDlg(Context context, short contactId) {

        String[] allSimCard = NmsPlatformAdapter.getInstance(context).getAllImsi();
        if (allSimCard == null || TextUtils.isEmpty(allSimCard[NmsConsts.SIM_CARD_SLOT_1])
                || TextUtils.isEmpty(allSimCard[NmsConsts.SIM_CARD_SLOT_2])) {
            NmsLog.trace(HissageTag.api, "no sim card or single card");
            return false;
        }

        long[] simIds = new long[NmsConsts.SIM_CARD_COUNT];

        for (int i = 0; i < NmsConsts.SIM_CARD_COUNT; ++i) {
            simIds[i] = NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(i);
        }

        if (simIds[NmsConsts.SIM_CARD_SLOT_1] <= 0 || simIds[NmsConsts.SIM_CARD_SLOT_2] <= 0) {
            NmsLog.trace(HissageTag.api, "no sim card or single card mode");
            return false;
        }

        SNmsSimInfo simInfo1 = NmsIpMessageApiNative
                .nmsGetSimInfoViaSimId((int) simIds[NmsConsts.SIM_CARD_SLOT_1]);
        SNmsSimInfo simInfo2 = NmsIpMessageApiNative
                .nmsGetSimInfoViaSimId((int) simIds[NmsConsts.SIM_CARD_SLOT_2]);

        if (null == simInfo1 || null == simInfo2) {
            return false;
        }

        if (simInfo1.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED
                && simInfo2.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
            return false;
        }

        long currentSimId = NmsPlatformAdapter.getInstance(context).getCurrentSimId();
        if (currentSimId == simIds[NmsConsts.SIM_CARD_SLOT_1]) {
            if (simInfo1.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                return false;
            }
        } else if (currentSimId == simIds[NmsConsts.SIM_CARD_SLOT_2]) {
            if (simInfo2.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                return false;
            }
        } else {
            return false;
        }

        try {
            NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
            if (null == contact) {
                return false;
            }

            if (contact.getType() == NmsContactType.HISSAGE_USER) {
                return true;
            } else {
                return false;
            }
        } catch (Exception e) {
            NmsLog.error(HissageTag.api, "get ip msg from contactId: " + contactId + " exception:");
            NmsLog.nmsPrintStackTrace(e);
            return false;
        }
    }

    /* Set user profile, will cause the notification: SetProfileResult */
    public void nmsSetProfile(Context context, String name, String status, byte[] avatar) {
        if (TextUtils.isEmpty(name) || TextUtils.isEmpty(status) || null == avatar) {
            NmsLog.error(HissageTag.api, "nmsSetProfile param error, name: " + name + "status: "
                    + status);
            return;
        }

        if (!NmsCommonUtils.getSDCardStatus()) {
            NmsLog.error(HissageTag.api, "SD card not mounted");
            return;
        }

        NmsProfileSettings userProfile = new NmsProfileSettings();
        String mCachePath = NmsCommonUtils.getSDCardPath(context) + "/";
        String mPhotoFilePath = mCachePath + "avatar.jpg";
        userProfile.fileName = mPhotoFilePath;
        userProfile.setImgPath(avatar);
        engineadapter.get().nmsUISetUserInfo(userProfile);
    };

    public NmsProfileSettings nmsGetProfile(Context context) {
        return engineadapter.get().nmsUIGetUserInfo();
    }

    public static short nmsCreateGroup(int sim_id, String name, List<String> members) {

        if (NmsConsts.SINGLE_CARD_SIM_ID == sim_id) {
            NmsLog.error(HissageTag.api, "single card simId " + sim_id);
            sim_id = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        if (sim_id < 0) {
            NmsLog.error(HissageTag.api, "error param at sim_id: " + sim_id);
            return -1;
        }

        if (TextUtils.isEmpty(name)) {
            NmsLog.error(HissageTag.api, "name is null/empty!");
            return -1;
        }
        if (name.length() > NmsCustomUIConfig.GROUPNAME_MAX_LENGTH) {
            NmsLog.error(HissageTag.api, "The name is too long!");
            return -1;
        }
        if (members == null || members.isEmpty()) {
            NmsLog.error(HissageTag.api, "members is null/empty!");
            return -1;
        }
        if (members.size() > NmsCustomUIConfig.GROUPMEM_MAX_COUNT) {
            NmsLog.error(HissageTag.api, "Number of members exceeds the limit!");
            return -1;
        }

        short groupId = -1;
        String memberStr = NmsCommonUtils.formatGroupMembers(members);

        if (!TextUtils.isEmpty(memberStr)) {
            // SNmsSimInfo info = NmsIpMessageApi.nmsGetSimInfoViaSimId(0);
            groupId = engineadapter.get().nmsUICreateGroup(sim_id, name, memberStr);
            if (groupId > 0) {
                NmsGroupChatContact groupContact = nmsGetGroupInfoViaGroupId(groupId);
                if (groupContact != null) {
                    String guid = groupContact.getGuid();
                    if (!TextUtils.isEmpty(guid)) {
                        NmsCreateSmsThread.getOrCreateThreadId(NmsService.getInstance(), guid);
                    } else {
                        NmsLog.error(HissageTag.api, "guid error!");
                    }
                }
            }
        }

        return groupId;
    };

    public static boolean nmsModifyGroupName(short groupId, String name) {
        if (groupId <= 0) {
            NmsLog.error(HissageTag.api, "groupId is invalid!");
            return false;
        }
        if (TextUtils.isEmpty(name)) {
            NmsLog.error(HissageTag.api, "name is null/empty!");
            return false;
        }
        if (name.length() > NmsCustomUIConfig.GROUPNAME_MAX_LENGTH) {
            NmsLog.error(HissageTag.api, "The name is too long!");
            return false;
        }

        int res = engineadapter.get().nmsUIModifyGroupName(groupId, name);

        if (res == 0) {
            return true;
        } else {
            return false;
        }
    };

    public static boolean nmsAddMembersToGroup(short groupId, List<String> members) {
        if (groupId <= 0) {
            NmsLog.error(HissageTag.api, "groupId is invalid!");
            return false;
        }
        if (members == null || members.isEmpty()) {
            NmsLog.error(HissageTag.api, "members is null/empty!");
            return false;
        }
        if (members.size() > NmsCustomUIConfig.GROUPMEM_MAX_COUNT) {
            NmsLog.error(HissageTag.api, "Number of members exceeds the limit!");
            return false;
        }

        int res = -1;
        String memberStr = NmsCommonUtils.formatGroupMembers(members);

        if (!TextUtils.isEmpty(memberStr)) {
            res = engineadapter.get().nmsUIAddMembersToGroup(groupId, memberStr);
        }

        if (res == 0) {
            return true;
        } else {
            return false;
        }
    };

    /* Exit from group */
    public static boolean nmsExitFromGroup(short groupId, boolean isClearMsg) {

        NmsLog.trace(HissageTag.api, "The exit from group chat by groupId:" + groupId);

        int ret = 0;
        if (groupId <= 0) {
            NmsLog.trace(HissageTag.api, "The groupId is not exist");
            return false;
        } else {
            ret = engineadapter.get().nmsUIQuitFromGroup((short) groupId, isClearMsg ? 1 : 0);
        }

        NmsLog.trace(HissageTag.api, "Quit from group return:" + ret);

        if (0 == ret) {
            return true;
        } else {
            return false;
        }
    }

    /* Get group id list */
    public static short[] nmsGetGroupIdList() {
        short[] ids = engineadapter.get().nmsUIGetGroupList();

        if (ids == null) {
            NmsLog.trace(HissageTag.api, "The groupId list is null");

        } else {
            NmsLog.trace(HissageTag.api, "Get group Id list by UI, The group lenght:" + ids.length);
        }

        return ids;
    }

    public static NmsGroupChatContact nmsGetGroupInfoViaThreadId(long threadId) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "threadId is invalid!");
            return null;
        }

        short groupId = NmsSMSMMSManager.getInstance(null).getEngineContactIdViaThreadId(threadId);
        if (groupId <= 0) {
            NmsLog.trace(HissageTag.api, "The groupId:" + groupId);
            return null;
        }

        NmsGroupChatContact contact = nmsGetGroupInfoViaGroupId(groupId);

        return contact;
    }

    public static NmsGroupChatContact nmsGetGroupInfoViaGroupId(short groupId) {
        if (groupId <= 0) {
            NmsLog.error(HissageTag.api, "groupId is invalid!");
            return null;
        }

        NmsContact contact = nmsGetContactInfoViaEngineId(groupId);

        if (contact == null) {
            NmsLog.error(HissageTag.api, "get group info error!");
            return null;
        }

        if (!(contact instanceof NmsGroupChatContact)) {
            NmsLog.error(HissageTag.api, "groupId(" + groupId + ") is not group!");
            return null;
        }

        if (contact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
            NmsLog.error(HissageTag.api, "contact type wrong!");
            return null;
        }

        return (NmsGroupChatContact) contact;
    }

    public static void nmsEnterChatMode(String number) {
        short contactId = NmsCommonUtils.getSetIMStatusContactId(number);
        if (contactId == -1) {
            NmsLog.error(HissageTag.api, "enter IM status failed, invalid contact");
            return;
        }
        engineadapter.get().nmsUIEnterIMStatus(contactId);
    }

    public static void nmsSendChatMode(String number, int status) {
        if (status < NmsContactStauts.TYPING || status > NmsContactStauts.STOP_SKETCHING) {
            NmsLog.error(HissageTag.api, "send IM status failed, invalid status " + status);
            return;
        }
        short contactId = NmsCommonUtils.getSetIMStatusContactId(number);
        if (contactId == -1) {
            NmsLog.error(HissageTag.api, "send IM status failed, invalid contact, number: "
                    + number);
            return;
        }
        engineadapter.get().nmsUISendIMStatus(contactId, status);
    }

    public static void nmsExitFromChatMode(String number) {
        short contactId = NmsCommonUtils.getSetIMStatusContactId(number);
        if (contactId == -1) {
            NmsLog.error(HissageTag.api, "exit IM status failed, invalid contact");
            return;
        }
        engineadapter.get().nmsUIExitIMStatus(contactId);
    }

    public static void nmsSaveChatHistory(long threadId[]) {

        if (null == threadId || threadId.length <= 0) {
            NmsLog.error(HissageTag.api, "nmsSaveChatHistory parameter error.");
            NmsDownloadMessageHistory.sendStatus(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE,
                    NmsSaveHistory.NMS_ERROR, "");
            return;
        }

        short contactId[] = new short[threadId.length];
        for (int i = 0; i < threadId.length; ++i) {
            short id = NmsSMSMMSManager.getInstance(null)
                    .getEngineContactIdViaThreadId(threadId[i]);
            if (id <= 0) {
                NmsLog.error(HissageTag.api,
                        "nmsSaveChatHistory get contactid from hissage db via threadid: "
                                + threadId[i]);
                NmsDownloadMessageHistory.sendStatus(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE,
                        NmsSaveHistory.NMS_ERROR, "");
                return;
            }
            contactId[i] = id;
        }

        String zipFile = NmsCommonUtils.getSDCardPath(NmsService.getInstance()) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY + File.separator;
        zipFile += contactId[0] + "_" + NmsConsts.SDF1.format(new Date()) + ".zip";

        NmsDownloadMessageHistory.dumpContactMessages(contactId, zipFile);

    }

    public static SNmsSimInfo nmsGetSimInfoViaSimId(int simId) {

        if (NmsConsts.SINGLE_CARD_SIM_ID == simId) {
            NmsLog.error(HissageTag.api, "single card simId " + simId);
            simId = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance())
                    .getCurrentSimId();
        }

        return engineadapter.get().nmsUIGetImsiInfoViaSimId(simId);
    }
    //MMS used
    public static boolean nmsAddContactToSpamList(final short[] engineContactIds) {

        if (engineContactIds == null) {
            NmsLog.error(HissageTag.api,
                    "add contact to spam list failed for engineContactIds is null");
            return false;
        }

        for (int i = 0; i < engineContactIds.length; ++i) {
            short contactId = engineContactIds[i];
            if (contactId <= 0) {
                continue;
            }

            NmsChatSettings settings = new NmsChatSettings(NmsService.getInstance(), contactId);
            settings.mContactId = contactId;
            settings.mNotification = 0;
            NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).nmsSetChatSettings(settings);
        }
        engineadapter.get().nmsUIBlockContacts(engineContactIds, engineContactIds.length);
        return true;
    }
    //iSMS used
    public static boolean nmsAsyncAddContactToSpamList(final short[] engineContactIds) {

        if (engineContactIds == null) {
            NmsLog.error(HissageTag.api,
                    "Group add contact to spam list failed for engineContactIds is null");
            return false;
        }
        new Thread(new Runnable() {

            @Override
            public void run() {
                nmsAddContactToSpamList(engineContactIds);
                Intent intent = new Intent();
                intent.setAction(NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE);
                NmsService.getInstance().sendBroadcast(intent);
            }
        }).start();
        return true;
    }
    //MMS used
    public static boolean nmsDeleteContactFromSpamList(final short[] engineContactIds) {

        if (engineContactIds == null) {
            NmsLog.error(HissageTag.api,
                    "delete contact from spam list failed for engineContactId is null");
            return false;
        }

        for (int i = 0; i < engineContactIds.length; ++i) {
            short contactId = engineContactIds[i];
            if (contactId <= 0) {
                continue;
            }

            NmsChatSettings settings = new NmsChatSettings(NmsService.getInstance(), contactId);
            settings.mNotification = 1;
            settings.mContactId = contactId;
            NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).nmsSetChatSettings(settings);
        }
        engineadapter.get().nmsUIUnBolckContacts(engineContactIds, engineContactIds.length);
        return true;
    }
	//iSMS used
    public static boolean nmsAsyncDeleteContactFromSpamList(final short[] engineContactIds) {

        if (engineContactIds == null) {
            NmsLog.error(HissageTag.api,
                    "delete contact from spam list failed for engineContactId is null");
            return false;
        }
        new Thread(new Runnable() {

            @Override
            public void run() {
                nmsDeleteContactFromSpamList(engineContactIds);
                Intent intent = new Intent();
                intent.setAction(NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE);
                NmsService.getInstance().sendBroadcast(intent);
            }
        }).start();

        return true;
    }

    public static boolean nmsAddMsgToImportantList(final short[] engineIds) {
        if (engineIds == null) {
            NmsLog.error(HissageTag.api,
                    "add msg to important list failed for convert system msg ids to engineIds error");
            return false;
        }

        new Thread(new Runnable() {

            @Override
            public void run() {
                engineadapter.get().nmsUISetMsgFlag(engineIds, engineIds.length,
                        (short) NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT);
                Intent intent = new Intent();
                intent.setAction(NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE);
                NmsService.getInstance().sendBroadcast(intent);
            }
        }).start();

        return true;
    }

    public static boolean nmsDeleteMsgFromImportantList(final short[] engineIds) {
        if (engineIds == null) {
            NmsLog.error(HissageTag.api,
                    "delete msg from important list failed for convert system msg ids to engineIds error");
            return false;
        }

        new Thread(new Runnable() {

            @Override
            public void run() {
                engineadapter.get().nmsUICancelMsgFlag(engineIds, engineIds.length,
                        (short) NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT);

                Intent intent = new Intent();
                intent.setAction(NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE);
                NmsService.getInstance().sendBroadcast(intent);
            }
        }).start();
        return true;
    }

    public static int nmsGetIpMsgCountOfThread(long threadId) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "threadId is invalid in nmsGetIpMsgCountOfThread");
            return 0;
        }

        short engineContactId = NmsContactApi.getInstance(null)
                .getEngineContactIdViaSystemThreadId(threadId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid in nmsGetIpMsgCountOfThread");
            return 0;
        }

        return engineadapter.get().nmsUIGetContactIpMsgCount(engineContactId);
    }

    public static boolean nmsGetCaptionFlag() {
        return NmsConfig.getCaptionFlag();
    }

    public static boolean nmsGetPhotoCaptionFlag() {
        return NmsConfig.getPhotoCaptionFlag();
    }

    public static boolean nmsGetVideoCaptionFlag() {
        return NmsConfig.getVideoCaptionFlag();
    }

    public static boolean nmsGetAudioCaptionFlag() {
        return NmsConfig.getAudioCaptionFlag();
    }

    public static boolean nmsGetAutoDownloadFlag() {
        return NmsConfig.getAutoDownloadFlag();
    }

    public static void nmsResendMsg(long simId, long msgId) {
        short recdId = NmsSMSMMSManager.getInstance(null).getNmsRecordIDViaSysId((int) msgId);
        if (recdId <= 0) {
            NmsLog.error(HissageTag.api, "nmsReSendFailedMsg get record id error, msgId: " + msgId);
            return;
        }
        engineadapter.get().nmsUIResendMsg((int) simId, recdId, 0);
    }

    public static int nmsGetIpMsgCountOfTypeInThread(long threadId, int type) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "threadId is invalid in nmsGetIpMsgCountOfTypeInThread");
            return 0;
        }

        short engineContactId = NmsContactApi.getInstance(null)
                .getEngineContactIdViaSystemThreadId(threadId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api,
                    "engineContactId is invalid in nmsGetIpMsgCountOfTypeInThread");
            return 0;
        }

        return engineadapter.get().nmsUIGetContactIPMsgCountOfReadMode(engineContactId, type);
    }

    public static int nmsDeleteDraftMsgInThread(long threadId) {
        if (threadId <= 0) {
            NmsLog.error(HissageTag.api, "threadId is invalid in nmsDeleteDraftMsgInThread");
            return 0;
        }

        short engineContactId = NmsContactApi.getInstance(null)
                .getEngineContactIdViaSystemThreadId(threadId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid in nmsDeleteDraftMsgInThread");
            return 0;
        }

        return engineadapter.get().nmsUIDeleteContactDraftMsg(engineContactId, 0);
    }
    //M: Activation Statistics
    public static void nmsAddActivatePromptStatistics(int type) {

        if (type > NmsIpMessageConsts.NmsUIActivateType.AUTO 
                && type < NmsIpMessageConsts.NmsUIActivateType.TYPE_COUNT) {
            
           int keyIndex = NmsStatistics.KEY_OTHER_ACTIVATE_PROMPT ;

            if (type == NmsIpMessageConsts.NmsUIActivateType.EMOTION) 
                keyIndex = NmsStatistics.KEY_EMO_ACTIVATE_PROMPT ;
            else if (type == NmsIpMessageConsts.NmsUIActivateType.MULTI_MEDIA) 
                keyIndex = NmsStatistics.KEY_MEDIA_ACTIVATE_PROMPT ;
            else if (type == NmsIpMessageConsts.NmsUIActivateType.SETTING)
                keyIndex = NmsStatistics.KEY_SETTING_ACTIVATE_PROMPT ;
            else if (type == NmsIpMessageConsts.NmsUIActivateType.DIALOG) 
                keyIndex = NmsStatistics.KEY_DLG_ACTIVATE_PROMPT ;
            else if(type == NmsIpMessageConsts.NmsUIActivateType.PROMPT){
                keyIndex = NmsStatistics.tips_activate_prompt ;
            } else if(type == NmsIpMessageConsts.NmsUIActivateType.MESSAGE){
                keyIndex = NmsStatistics.sms_activate_prompt ;
            }
            NmsStatistics.incKeyVal(keyIndex) ;
        }
    }

    public static boolean nmsIsIpMessageSrvNumber(String number){        
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(HissageTag.api, "nmsIsIpMessageSrvNumber. number is empty!");
            return false;
        }

        return NmsContactApi.getInstance(null).isIpMessageSrvNumber(number);
    }

    public static String nmsGetIpMessageSrvNumberName(String number){        
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(HissageTag.api, "nmsGetIpMessageSrvNumberName. number is empty!");
            return null;
        }

        return NmsContactApi.getInstance(null).getIpMessageSrvNumberName(number);
    }  

	public static boolean nmsIsIpMsgSendable(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(HissageTag.api, "nmsIsIpMsgSendable. number is empty!");
            return false;
        }

		short engineContactId = NmsContactApi.getInstance(null).getEngineContactIdViaNumber(number);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "nmsIsIpMsgSendable got ContactId is invalid! number: " + number);
            return false;
        }

        return engineadapter.get().nmsUIIsIpMsgSendable(engineContactId);
    }

    
    // / M: Private Statistics
    public static void addPrivateClickTimeStatistics(int clickTime) {
        NmsPrivateStatistics.updatePrivateClickTimeStatistics(clickTime);
    }

    public static void addPrivateOpenFlagStatistics(int openFlag) {
        NmsPrivateStatistics.updatePrivateOpenFlagStatistics(openFlag);
    }

    public static void addPrivateContactsStatistics(int contacts) {
        NmsPrivateStatistics.updatePrivateContactsStatistics(contacts);
    }

    public static String getUnifyPhoneNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(HissageTag.api, "getUnifyPhoneNumber. number is empty!");
            return null;
        }
        return engineadapter.get().nmsUIGetUnifyPhoneNumber(number);
    }    
    
    public static void nmsCheckDefaultSmsAppChanged() {
        if (NmsConfig.isAndroidKitKatOnward) {
            engineadapter.get().nmsCheckDefaultSmsAppChanged() ;
        }
    }
}
