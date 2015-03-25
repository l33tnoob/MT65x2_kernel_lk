/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.imsp;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;

import com.hissage.api.NmsIpMessageApi;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpCalendarMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.util.data.NmsConsts;
import com.mediatek.mms.ipmessage.MessageManager;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.message.IpImageMessage;
import com.mediatek.mms.ipmessage.message.IpLocationMessage;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpTextMessage;
import com.mediatek.mms.ipmessage.message.IpVCalendarMessage;
import com.mediatek.mms.ipmessage.message.IpVCardMessage;
import com.mediatek.mms.ipmessage.message.IpVideoMessage;
import com.mediatek.mms.ipmessage.message.IpVoiceMessage;
import com.mediatek.xlog.Xlog;

/**
 * Provide message management related interface
 */
public class MessageManagerExt extends MessageManager {
    private static final String TAG = "imsp/MessageManagerExt";

    public MessageManagerExt(Context context) {
        super(context);
    }

    public static IpMessage getIpMsgFromNmsIpMsg(NmsIpMessage nmsIpMsg) {
        if (nmsIpMsg == null) {
            Xlog.e(TAG, "ip message is null.");
            return null;
        }
        Xlog.d(TAG, "get ipmessage from nms ip message, type = " + nmsIpMsg.type);
        IpMessage ipMsg = null;
        switch (nmsIpMsg.type) {
        case NmsIpMessageConsts.NmsIpMessageType.TEXT: {
            ipMsg = new IpTextMessage();
            String body = ((NmsIpTextMessage) nmsIpMsg).body;
            ((IpTextMessage) ipMsg).setBody(body);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.PICTURE:
        case NmsIpMessageConsts.NmsIpMessageType.SKETCH: {
            ipMsg = new IpImageMessage();
            // for attach message
            ((IpImageMessage) ipMsg).setPath(((NmsIpImageMessage) nmsIpMsg).path);
            ((IpImageMessage) ipMsg).setUrl(((NmsIpImageMessage) nmsIpMsg).url);
            ((IpImageMessage) ipMsg).setSize(((NmsIpImageMessage) nmsIpMsg).size);

            // for image message
            ((IpImageMessage) ipMsg).setWidthInPixel(((NmsIpImageMessage) nmsIpMsg).widthInPixel);
            ((IpImageMessage) ipMsg).setHeightInPixel(((NmsIpImageMessage) nmsIpMsg).heightInPixel);
            if ((nmsIpMsg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_READEDBURN_NOT_RECEIVED) != 0) {
                ((IpImageMessage) ipMsg).setCaption("100");
            } else {
                ((IpImageMessage) ipMsg).setCaption(((NmsIpImageMessage) nmsIpMsg).caption);
            }
            ((IpImageMessage) ipMsg).setThumbPath(((NmsIpImageMessage) nmsIpMsg).thumbPath);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.VOICE: {
            ipMsg = new IpVoiceMessage();
            // for attach message
            ((IpVoiceMessage) ipMsg).setPath(((NmsIpVoiceMessage) nmsIpMsg).path);
            ((IpVoiceMessage) ipMsg).setUrl(((NmsIpVoiceMessage) nmsIpMsg).url);
            ((IpVoiceMessage) ipMsg).setSize(((NmsIpVoiceMessage) nmsIpMsg).size);

            // for voice message
            ((IpVoiceMessage) ipMsg).setDuration(((NmsIpVoiceMessage) nmsIpMsg).durationTime);
            ((IpVoiceMessage) ipMsg).setCaption(((NmsIpVoiceMessage) nmsIpMsg).caption);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.VIDEO: {
            ipMsg = new IpVideoMessage();
            // for attach message
            ((IpVideoMessage) ipMsg).setPath(((NmsIpVideoMessage) nmsIpMsg).path);
            ((IpVideoMessage) ipMsg).setUrl(((NmsIpVideoMessage) nmsIpMsg).url);
            ((IpVideoMessage) ipMsg).setSize(((NmsIpVideoMessage) nmsIpMsg).size);

            // for video message
            ((IpVideoMessage) ipMsg).setDuration(((NmsIpVideoMessage) nmsIpMsg).durationTime);
            ((IpVideoMessage) ipMsg).setCaption(((NmsIpVideoMessage) nmsIpMsg).caption);
            ((IpVideoMessage) ipMsg).setThumbPath(((NmsIpVideoMessage) nmsIpMsg).thumbPath);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.VCARD: {
            ipMsg = new IpVCardMessage();
            // for attach message
            ((IpVCardMessage) ipMsg).setPath(((NmsIpVCardMessage) nmsIpMsg).path);
            ((IpVCardMessage) ipMsg).setUrl(((NmsIpVCardMessage) nmsIpMsg).url);
            ((IpVCardMessage) ipMsg).setSize(((NmsIpVCardMessage) nmsIpMsg).size);

            // for vcard message
            ((IpVCardMessage) ipMsg).setName(((NmsIpVCardMessage) nmsIpMsg).name);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.LOCATION: {
            ipMsg = new IpLocationMessage();
            // for attach message
            ((IpLocationMessage) ipMsg).setPath(((NmsIpLocationMessage) nmsIpMsg).path);
            ((IpLocationMessage) ipMsg).setUrl(((NmsIpLocationMessage) nmsIpMsg).url);
            ((IpLocationMessage) ipMsg).setSize(((NmsIpLocationMessage) nmsIpMsg).size);

            // for location message
            ((IpLocationMessage) ipMsg).setLongitude(((NmsIpLocationMessage) nmsIpMsg).longitude);
            ((IpLocationMessage) ipMsg).setLatitude(((NmsIpLocationMessage) nmsIpMsg).latitude);
            ((IpLocationMessage) ipMsg).setAddress(((NmsIpLocationMessage) nmsIpMsg).address);
            ((IpLocationMessage) ipMsg).setThumbPath(((NmsIpLocationMessage) nmsIpMsg).thumbPath);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.CALENDAR: {
            ipMsg = new IpVCalendarMessage();
            // for attach message
            ((IpVCalendarMessage) ipMsg).setPath(((NmsIpCalendarMessage) nmsIpMsg).path);
            ((IpVCalendarMessage) ipMsg).setUrl(((NmsIpCalendarMessage) nmsIpMsg).url);
            ((IpVCalendarMessage) ipMsg).setSize(((NmsIpCalendarMessage) nmsIpMsg).size);

            // for vcalendar message
            ((IpVCalendarMessage) ipMsg).setSummary(((NmsIpCalendarMessage) nmsIpMsg).summary);
            break;
        }
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.COUNT:
        case NmsIpMessageConsts.NmsIpMessageType.UNKNOWN_FILE: {
            Xlog.w(TAG, "unsupported attachment type.");
            return null;
        }
        default: {
            Xlog.w(TAG, "undefined attachment type.");
            return null;
        }
        }

        ipMsg.setId((int) nmsIpMsg.id);
        ipMsg.setIpDbId(nmsIpMsg.ipDbId);
        ipMsg.setSimId(nmsIpMsg.simId);
        if ((nmsIpMsg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
            ipMsg.setType(IpMessageConsts.IpMessageType.READEDBURN);
        } else {
            ipMsg.setType(nmsIpMsg.type);
        }
        ipMsg.setStatus(nmsIpMsg.status);
        ipMsg.setTo(nmsIpMsg.to);
        ipMsg.setFrom(nmsIpMsg.from);

        return ipMsg;
    }

    public static NmsIpMessage getNmsIpMsgFromIpMsg(IpMessage ipMsg) {
        NmsIpMessage nmsIpMsg = null;
        Xlog.d(TAG, "get NmsIpMsg From IpMsg: ipmessage type = " + ipMsg.getType());
        switch (ipMsg.getType()) {
        case IpMessageConsts.IpMessageType.TEXT: {
            nmsIpMsg = new NmsIpTextMessage();
            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.TEXT;

            ((NmsIpTextMessage) nmsIpMsg).body = ((IpTextMessage) ipMsg).getBody();

            break;
        }
        case IpMessageConsts.IpMessageType.PICTURE:
        case IpMessageConsts.IpMessageType.SKETCH: {
            nmsIpMsg = new NmsIpImageMessage(NmsIpMessageConsts.NmsIpMessageType.PICTURE);

            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.PICTURE;

            // for attach message
            ((NmsIpImageMessage) nmsIpMsg).path = ((IpImageMessage) ipMsg).getPath();
            ((NmsIpImageMessage) nmsIpMsg).url = ((IpImageMessage) ipMsg).getUrl();
            ((NmsIpImageMessage) nmsIpMsg).size = ((IpImageMessage) ipMsg).getSize();

            // for image message
            ((NmsIpImageMessage) nmsIpMsg).widthInPixel = ((IpImageMessage) ipMsg)
                    .getWidthInPixel();
            ((NmsIpImageMessage) nmsIpMsg).heightInPixel = ((IpImageMessage) ipMsg)
                    .getHeightInPixel();
            ((NmsIpImageMessage) nmsIpMsg).caption = ((IpImageMessage) ipMsg).getCaption();
            ((NmsIpImageMessage) nmsIpMsg).thumbPath = ((IpImageMessage) ipMsg).getThumbPath();

            break;
        }

        case IpMessageConsts.IpMessageType.READEDBURN: {
            nmsIpMsg = new NmsIpImageMessage(NmsIpMessageConsts.NmsIpMessageType.PICTURE);
            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.PICTURE;
            nmsIpMsg.flag = NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ;
            // for attach message
            ((NmsIpImageMessage) nmsIpMsg).path = ((IpImageMessage) ipMsg).getPath();
            ((NmsIpImageMessage) nmsIpMsg).url = ((IpImageMessage) ipMsg).getUrl();
            ((NmsIpImageMessage) nmsIpMsg).size = ((IpImageMessage) ipMsg).getSize();

            // for image message
            ((NmsIpImageMessage) nmsIpMsg).widthInPixel = ((IpImageMessage) ipMsg)
                    .getWidthInPixel();
            ((NmsIpImageMessage) nmsIpMsg).heightInPixel = ((IpImageMessage) ipMsg)
                    .getHeightInPixel();
            ((NmsIpImageMessage) nmsIpMsg).caption = ((IpImageMessage) ipMsg).getCaption();
            ((NmsIpImageMessage) nmsIpMsg).thumbPath = ((IpImageMessage) ipMsg).getThumbPath();

            break;
        }

        case IpMessageConsts.IpMessageType.VOICE: {
            nmsIpMsg = new NmsIpVoiceMessage();

            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.VOICE;

            // for attach message
            ((NmsIpVoiceMessage) nmsIpMsg).path = ((IpVoiceMessage) ipMsg).getPath();
            ((NmsIpVoiceMessage) nmsIpMsg).url = ((IpVoiceMessage) ipMsg).getUrl();
            ((NmsIpVoiceMessage) nmsIpMsg).size = ((IpVoiceMessage) ipMsg).getSize();

            // for voice message
            ((NmsIpVoiceMessage) nmsIpMsg).durationTime = ((IpVoiceMessage) ipMsg).getDuration();
            ((NmsIpVoiceMessage) nmsIpMsg).caption = ((IpVoiceMessage) ipMsg).getCaption();

            break;
        }
        case IpMessageConsts.IpMessageType.VIDEO: {
            nmsIpMsg = new NmsIpVideoMessage();

            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.VIDEO;

            // for attach message
            ((NmsIpVideoMessage) nmsIpMsg).path = ((IpVideoMessage) ipMsg).getPath();
            ((NmsIpVideoMessage) nmsIpMsg).url = ((IpVideoMessage) ipMsg).getUrl();
            ((NmsIpVideoMessage) nmsIpMsg).size = ((IpVideoMessage) ipMsg).getSize();

            // for video message
            ((NmsIpVideoMessage) nmsIpMsg).durationTime = ((IpVideoMessage) ipMsg).getDuration();
            ((NmsIpVideoMessage) nmsIpMsg).caption = ((IpVideoMessage) ipMsg).getCaption();
            ((NmsIpVideoMessage) nmsIpMsg).thumbPath = ((IpVideoMessage) ipMsg).getThumbPath();

            break;
        }
        case IpMessageConsts.IpMessageType.VCARD: {
            nmsIpMsg = new NmsIpVCardMessage();

            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.VCARD;

            // for attach message
            ((NmsIpVCardMessage) nmsIpMsg).path = ((IpVCardMessage) ipMsg).getPath();
            ((NmsIpVCardMessage) nmsIpMsg).url = ((IpVCardMessage) ipMsg).getUrl();
            ((NmsIpVCardMessage) nmsIpMsg).size = ((IpVCardMessage) ipMsg).getSize();

            // for vcard message
            ((NmsIpVCardMessage) nmsIpMsg).name = ((IpVCardMessage) ipMsg).getName();

            break;
        }
        case IpMessageConsts.IpMessageType.LOCATION: {
            nmsIpMsg = new NmsIpLocationMessage();

            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.LOCATION;

            // for attach message
            ((NmsIpLocationMessage) nmsIpMsg).path = ((IpLocationMessage) ipMsg).getPath();
            ((NmsIpLocationMessage) nmsIpMsg).url = ((IpLocationMessage) ipMsg).getUrl();
            ((NmsIpLocationMessage) nmsIpMsg).size = ((IpLocationMessage) ipMsg).getSize();

            // for location message
            ((NmsIpLocationMessage) nmsIpMsg).longitude = ((IpLocationMessage) ipMsg)
                    .getLongitude();
            ((NmsIpLocationMessage) nmsIpMsg).latitude = ((IpLocationMessage) ipMsg).getLatitude();
            ((NmsIpLocationMessage) nmsIpMsg).address = ((IpLocationMessage) ipMsg).getAddress();
            ((NmsIpLocationMessage) nmsIpMsg).thumbPath = ((IpLocationMessage) ipMsg)
                    .getThumbPath();

            break;
        }
        case IpMessageConsts.IpMessageType.CALENDAR: {
            nmsIpMsg = new NmsIpCalendarMessage();

            // for common
            nmsIpMsg.type = NmsIpMessageConsts.NmsIpMessageType.CALENDAR;

            // for attach message
            ((NmsIpCalendarMessage) nmsIpMsg).path = ((IpVCalendarMessage) ipMsg).getPath();
            ((NmsIpCalendarMessage) nmsIpMsg).url = ((IpVCalendarMessage) ipMsg).getUrl();
            ((NmsIpCalendarMessage) nmsIpMsg).size = ((IpVCalendarMessage) ipMsg).getSize();

            // for vcalendar message
            ((NmsIpCalendarMessage) nmsIpMsg).summary = ((IpVCalendarMessage) ipMsg).getSummary();

            break;
        }
        case IpMessageConsts.IpMessageType.GROUP_CREATE_CFG:
        case IpMessageConsts.IpMessageType.GROUP_ADD_CFG:
        case IpMessageConsts.IpMessageType.GROUP_QUIT_CFG:
        case IpMessageConsts.IpMessageType.COUNT:
        case IpMessageConsts.IpMessageType.UNKNOWN_FILE: {
            break;
        }
        default: {
            Xlog.w(TAG, "undefined attachment type.");
            break;
        }
        }

        if (null == nmsIpMsg) {
            return nmsIpMsg;
        }

        nmsIpMsg.simId = ipMsg.getSimId();
        nmsIpMsg.protocol = NmsIpMessageConsts.NmsMessageProtocol.IP;
        nmsIpMsg.read = true;
        nmsIpMsg.status = (ipMsg.getStatus() == IpMessageConsts.IpMessageStatus.DRAFT) ? NmsIpMessageConsts.NmsIpMessageStatus.DRAFT
                : NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX;
        nmsIpMsg.to = ipMsg.getTo();

        return nmsIpMsg;
    }

    public IpMessage getIpMsgInfo(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getIpMsgInfo: get ip message failed, message id = " + msgId);
            return null;
        }
        return getIpMsgFromNmsIpMsg(nmsIpMessage);
    }

    public IpMessage getIpMessageFromIntent(Intent intent) {
        Xlog.d(TAG, "getIpMessageFromIntent: intent = " + intent);
        // NmsIpLocationMessage nmsIpMessage = (NmsIpLocationMessage)
        // intent.getExtra(NmsIpMessageConsts.NMS_SHARE_LOCATION_DONE);
        String shareAddress = intent
                .getStringExtra(NmsIpMessageConsts.NmsShareLocationDone.NMS_LOCATION_ADDRESS);
        double latitude = intent.getDoubleExtra(
                NmsIpMessageConsts.NmsShareLocationDone.NMS_LOCATION_LATITUDE, -1);
        double longitude = intent.getDoubleExtra(
                NmsIpMessageConsts.NmsShareLocationDone.NMS_LOCATION_LONGITUDE, -1);
        String path = intent
                .getStringExtra(NmsIpMessageConsts.NmsShareLocationDone.NMS_LOCATION_PATH);
        Xlog.d(TAG, "\t shareAddress = " + shareAddress);
        Xlog.d(TAG, "\t latitude = " + latitude);
        Xlog.d(TAG, "\t longitude = " + longitude);
        Xlog.d(TAG, "\t path = " + path);
        IpLocationMessage ipLocationMessage = new IpLocationMessage();
        ipLocationMessage.setAddress(shareAddress);
        ipLocationMessage.setLatitude(latitude);
        ipLocationMessage.setLongitude(longitude);
        ipLocationMessage.setThumbPath(path);
        return ipLocationMessage;
    }

    public int getIpDatabaseId(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getIpDatabaseId: get ip message failed, message id = " + msgId);
            return 0;
        }
        return nmsIpMessage.ipDbId;
    }

    public int getType(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getType: get ip message failed, message id = " + msgId);
            return 0;
        }
        int ipMessageType = 0;
        switch (nmsIpMessage.type) {
        case NmsIpMessageConsts.NmsIpMessageType.COUNT:
            ipMessageType = IpMessageConsts.IpMessageType.COUNT;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.CALENDAR:
            ipMessageType = IpMessageConsts.IpMessageType.CALENDAR;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG:
            ipMessageType = IpMessageConsts.IpMessageType.GROUP_ADD_CFG;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG:
            ipMessageType = IpMessageConsts.IpMessageType.GROUP_CREATE_CFG;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG:
            ipMessageType = IpMessageConsts.IpMessageType.GROUP_QUIT_CFG;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.LOCATION:
            ipMessageType = IpMessageConsts.IpMessageType.LOCATION;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.PICTURE:
            if ((nmsIpMessage.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                ipMessageType = IpMessageConsts.IpMessageType.READEDBURN;
            } else {
                ipMessageType = IpMessageConsts.IpMessageType.PICTURE;
            }
            break;
        case NmsIpMessageConsts.NmsIpMessageType.SKETCH:
            ipMessageType = IpMessageConsts.IpMessageType.SKETCH;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.TEXT:
            ipMessageType = IpMessageConsts.IpMessageType.TEXT;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.VCARD:
            ipMessageType = IpMessageConsts.IpMessageType.VCARD;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.VIDEO:
            ipMessageType = IpMessageConsts.IpMessageType.VIDEO;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.VOICE:
            ipMessageType = IpMessageConsts.IpMessageType.VOICE;
            break;
        case NmsIpMessageConsts.NmsIpMessageType.UNKNOWN_FILE:
            ipMessageType = IpMessageConsts.IpMessageType.UNKNOWN_FILE;
            break;
        }
        return ipMessageType;
    }

    public int getStatus(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getStatus: get ip message failed, message id = " + msgId);
            return 0;
        }
        return nmsIpMessage.status;
    }

    public int getTime(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getTime: get ip message failed, message id = " + msgId);
            return 0;
        }
        return nmsIpMessage.time;
    }

    public int getSimId(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getSimId: get ip message failed, message id = " + msgId);
            return 0;
        }
        return nmsIpMessage.simId;
    }

    public boolean isReaded(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "isReaded: get ip message failed, message id = " + msgId);
            return false;
        }
        return nmsIpMessage.read;
    }

    public String getTo(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getTo: get ip message failed, message id = " + msgId);
            return null;
        }
        return nmsIpMessage.to;
    }

    public String getFrom(long msgId) {
        NmsIpMessage nmsIpMessage = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(msgId);
        if (nmsIpMessage == null) {
            Xlog.e(TAG, "getFrom: get ip message failed, message id = " + msgId);
            return null;
        }
        return nmsIpMessage.from;
    }

    public int saveIpMsg(IpMessage msg, int sendMsgMode) {
        NmsIpMessage nmsIpMsg = getNmsIpMsgFromIpMsg(msg);
        if (null == nmsIpMsg) {
            return -1;
        }
        Xlog.d(TAG, "Call nmsSaveIpMsg, send Msg Mode = " + sendMsgMode);
        return NmsIpMessageApi.getInstance(mContext).nmsSaveIpMsg(nmsIpMsg, sendMsgMode);
    }

    public void deleteIpMsg(long[] ids, boolean delImportant, boolean delLocked) {
        NmsIpMessageApi.getInstance(mContext).nmsDeleteIpMsg(ids, delImportant, delLocked);
    }

    public void setIpMsgAsViewed(long msgId) {
        NmsIpMessageApi.getInstance(mContext).nmsSetIpMsgAsViewed(msgId);
    }

    public void setThreadAsViewed(long threadId) {
        NmsIpMessageApi.getInstance(mContext).nmsSetThreadAsViewed(threadId);
    }

    /*
     * Download message attachment, asynchronous, notification: DownloadStatus
     * will be received
     */
    public void downloadAttach(long msgId) {
        NmsIpMessageApi.getInstance(mContext).nmsDownloadAttach(msgId);
    }

    /*
     * cancel downloading, asynchronous, notification: DownloadStatus will be
     * received, status is Failed
     */
    public void cancelDownloading(long msgId) {
        NmsIpMessageApi.getInstance(mContext).nmsCancelDownloading(msgId);
    }

    public boolean isDownloading(long msgId) {
        return NmsIpMessageApi.getInstance(mContext).nmsIsDownloading(msgId);
    }

    public int getDownloadProcess(long msgId) {
        return NmsIpMessageApi.getInstance(mContext).nmsGetDownloadProgress(msgId);
    }

    public boolean addMessageToImportantList(long[] msgIds) {
        return NmsIpMessageApi.getInstance(mContext).nmsAddMsgToImportantList(msgIds);
    }

    public boolean deleteMessageFromImportantList(long[] msgIds) {
        return NmsIpMessageApi.getInstance(mContext).nmsDeleteMsgFromImportantList(msgIds);
    }

    public void resendMessage(long msgId) {
        NmsIpMessageApi.getInstance(mContext).nmsReSendFailedMsg(NmsConsts.SINGLE_CARD_SIM_ID,
                msgId);
    }

    public void resendMessage(long msgId, int simId) {
        NmsIpMessageApi.getInstance(mContext).nmsReSendFailedMsg((long) simId, msgId);
    }
}
