package com.hissage.message.ip;

import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContact;
import com.hissage.jni.engineadapter;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsIpMsgApiUtils {

    public static int nmsSaveMsgIntoDB(NmsIpMessage msg, int sendMsgMode, boolean delDraft, boolean delDraftInSysDb) {
        int ret = -1;
        if (null == msg) {
            NmsLog.error(HissageTag.api, "failed to save ip message for msg is null");
            return -1;
        }

        if (msg.protocol != NmsIpMessageConsts.NmsMessageProtocol.IP) {
            NmsLog.error(HissageTag.api, "failed to save ip message for invalid protocol");
            return -1;
        }

        if (msg.status != NmsIpMessageConsts.NmsIpMessageStatus.DRAFT
                && msg.status != NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX) {
            NmsLog.error(HissageTag.api, "failed to save ip message for status error");
            return -1;
        }

        if (TextUtils.isEmpty(msg.to)) {
            NmsLog.error(HissageTag.api, "failed to save ip message for no receiver");
            return -1;
        }

        if (!NmsCommonUtils.isInvalidNum(msg.to)) {
            NmsLog.error(HissageTag.api, "failed to save ip message for invalid receiver");
            return -1;
        }

        msg.to = NmsCommonUtils.getValidNum(msg.to);

        if (msg instanceof NmsIpTextMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not textmsg");
                return -1;
            }
            if (TextUtils.isEmpty(((NmsIpTextMessage) msg).body)) {
                NmsLog.error(HissageTag.api, "failed to save ip message for empty textmsg body");
                return -1;
            }
            if (((NmsIpTextMessage) msg).body.length() > NmsCustomUIConfig.MESSAGE_MAX_LENGTH) {
                NmsLog.error(HissageTag.api, "failed to save ip message for body length too long");
                return -1;
            }
        }

        if (msg instanceof NmsIpLocationMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not locationmsg");
                return -1;
            }

            if ((((NmsIpLocationMessage) msg).latitude > 180 || ((NmsIpLocationMessage) msg).latitude < -180)
                    || (((NmsIpLocationMessage) msg).longitude > 180 || ((NmsIpLocationMessage) msg).longitude < -180)) {
                NmsLog.error(HissageTag.api,
                        "failed to save ip message for invalid latitude or longitude");
                return -1;
            }

            if (((NmsIpLocationMessage) msg).address.length() > NmsCustomUIConfig.LOCATION_ADDR_MAX_LENGTH + 20) {
                NmsLog.error(HissageTag.api, "failed to save ip message for body length too long");
                return -1;
            }

            ((NmsIpLocationMessage) msg).address = NmsIpLocationMessage
                    .locMsg2Str((NmsIpLocationMessage) msg);

            if (!TextUtils.isEmpty(((NmsIpAttachMessage) msg).path)) {
                if (!NmsCommonUtils.isExistsFile(((NmsIpAttachMessage) msg).path)) {
                    ((NmsIpAttachMessage) msg).size = 0;
                } else {
                    String path = ((NmsIpLocationMessage) msg).path.toLowerCase();
                    if (!path.endsWith(".map.png")) {
                        NmsLog.error(HissageTag.api,
                                "failed to save ip message for invalid msg pic type");
                        return -1;
                    }
                    ((NmsIpAttachMessage) msg).size = NmsCommonUtils
                            .getFileSize(((NmsIpLocationMessage) msg).path);

                    if ((((NmsIpAttachMessage) msg).size < 0 || ((NmsIpAttachMessage) msg).size > NmsCustomUIConfig.MAX_ATTACH_SIZE)) {
                        NmsLog.error(HissageTag.api,
                                "failed to save ip location message for attach size is too long");
                        return -1;
                    }

                }
            } else {
                ((NmsIpAttachMessage) msg).size = 0;
            }
        }

        if (msg instanceof NmsIpVoiceMessage || msg instanceof NmsIpVideoMessage
                || msg instanceof NmsIpImageMessage || msg instanceof NmsIpVCardMessage || msg instanceof NmsIpCalendarMessage) {
            if (TextUtils.isEmpty(((NmsIpAttachMessage) msg).path)) {
                NmsLog.error(HissageTag.api, "failed to save ip message for path is empty");
                return -1;
            }

            if (!((NmsIpAttachMessage) msg).path.contains(".")) {
                NmsLog.error(HissageTag.api, "failed to save ip message for invalid path");
                return -1;
            }

            if (!NmsCommonUtils.isExistsFile(((NmsIpAttachMessage) msg).path)) {
                NmsLog.error(HissageTag.api, "failed to save ip message for file not exist");
                return -1;
            }

            if (NmsCommonUtils.getFileSize(((NmsIpAttachMessage) msg).path) <= 0
                    || NmsCommonUtils.getFileSize(((NmsIpAttachMessage) msg).path) > 2 * 1024 * 1024) {
                NmsLog.error(HissageTag.api, "failed to save ip message for invalid file size");
                return -1;
            }
            ((NmsIpAttachMessage) msg).size = NmsCommonUtils
                    .getFileSize(((NmsIpAttachMessage) msg).path);
        }

        if (msg instanceof NmsIpVCardMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.VCARD) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not vcardmsg");
                return -1;
            }

            String path = ((NmsIpVCardMessage) msg).path.toLowerCase();

            if (!path.endsWith(".vcf")) {
                NmsLog.error(HissageTag.api,
                        "failed to save ip message for suffix not support vcard");
                return -1;
            }

            if (!TextUtils.isEmpty(((NmsIpVCardMessage) msg).name)
                    && ((NmsIpVCardMessage) msg).name.length() > 100) {
                NmsLog.error(HissageTag.api, "failed to save ip message for vcard name too long");
                return -1;
            }
        }
        
        if (msg instanceof NmsIpCalendarMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.CALENDAR) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not calendar msg");
                return -1;
            }

            String path = ((NmsIpCalendarMessage) msg).path.toLowerCase();

            if (!path.endsWith(".vcs")) {
                NmsLog.error(HissageTag.api,
                        "failed to save ip message for suffix not support calendar");
                return -1;
            }

            if (!TextUtils.isEmpty(((NmsIpCalendarMessage) msg).summary)
                    && ((NmsIpCalendarMessage) msg).summary.length() > 100) {
                NmsLog.error(HissageTag.api, "failed to save ip message for calendar summary too long");
                return -1;
            }
        }

        if (msg instanceof NmsIpImageMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.PICTURE
                    && msg.type != NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not imgmsg");
                return -1;
            }

            String path = ((NmsIpImageMessage) msg).path.toLowerCase();

            if (msg.type == NmsIpMessageConsts.NmsIpMessageType.PICTURE) {
                if (!path.endsWith(".png") && !path.endsWith(".jpg") && !path.endsWith(".jpeg")
                        && !path.endsWith(".bmp") && !path.endsWith(".gif")) {
                    NmsLog.error(HissageTag.api,
                            "failed to save ip message for suffix not support pic");
                    return -1;
                }

                if (!TextUtils.isEmpty(((NmsIpImageMessage) msg).caption)
                        && ((NmsIpImageMessage) msg).caption.length() > 100) {
                    NmsLog.error(HissageTag.api,
                            "failed to save ip message for pic caption too long");
                    return -1;
                }
            }

            if (msg.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
                if (!path.endsWith(".ske.png")) {
                    NmsLog.error(HissageTag.api,
                            "failed to save ip message for suffix not support pic");
                    return -1;
                }
            }
        }

        if (msg instanceof NmsIpVideoMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not videomsg");
                return -1;
            }
            if (((NmsIpVideoMessage) msg).durationTime <= 0) {
                NmsLog.error(HissageTag.api, "failed to save ip message for duration too short");
                return -1;
            }

            String path = ((NmsIpVideoMessage) msg).path.toLowerCase();

            if (!path.endsWith(".3gp") && !path.endsWith(".mp4")) {
                NmsLog.error(HissageTag.api, "failed to save ip message for suffix not video");
                return -1;
            }

            if (!TextUtils.isEmpty(((NmsIpVideoMessage) msg).caption)
                    && ((NmsIpVideoMessage) msg).caption.length() > 100) {
                NmsLog.error(HissageTag.api, "failed to save ip message for caption too long");
                return -1;
            }
        }

        if (msg instanceof NmsIpVoiceMessage) {
            if (msg.type != NmsIpMessageConsts.NmsIpMessageType.VOICE) {
                NmsLog.error(HissageTag.api, "failed to save ip message for not voicemsg");
                return -1;
            }

            if (!TextUtils.isEmpty(((NmsIpVoiceMessage) msg).caption)
                    && ((NmsIpVoiceMessage) msg).caption.length() > 100) {
                NmsLog.error(HissageTag.api, "failed to save ip message for caption too long");
                return -1;
            }

            String path = ((NmsIpVoiceMessage) msg).path.toLowerCase();

            if (!MessageUtils.isAudio(path)) {
                NmsLog.error(HissageTag.api, "failed to save ip message for suffix not audio format");
                return -1;
            }
            if (((NmsIpVoiceMessage) msg).durationTime <= 0) {
                NmsLog.error(HissageTag.api, "failed to save ip message for duration too short");
                return -1;
            }
        }

        if (msg.status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX) {
            ret = engineadapter.get().nmsUISendNewMsg(msg, sendMsgMode);
            NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaNumber(msg.to);
            if (contact != null && delDraft) {
                engineadapter.get().nmsUIDeleteContactDraftMsg(contact.getId(), (delDraftInSysDb ? 1 : 0));
            }
        } else if (msg.status == NmsIpMessageConsts.NmsIpMessageStatus.DRAFT) {
            ret = engineadapter.get().nmsUISaveMsgIntoDraft(msg);
        }
        return ret;
    }

}
