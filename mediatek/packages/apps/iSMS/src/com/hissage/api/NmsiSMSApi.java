package com.hissage.api;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.util.data.NmsConverter;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class NmsiSMSApi {

    // get NmsContact by dbid
    public static NmsContact nmsGetContactInfoViaDbId(short ipDbId) {
        NmsLog.trace(HissageTag.api, "Get ContactInfo by DbId:" + ipDbId);
        if (ipDbId < 0) {
            NmsLog.error(HissageTag.api, "ipDbId is invalid!");
            return null;
        }

        short engineContactId = (short) engineadapter.get().nmsContactKeyRecordId(ipDbId);
        if (engineContactId <= 0) {
            NmsLog.error(HissageTag.api, "engineContactId is invalid!");
            return null;
        }

        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(engineContactId);

        return contact;
    }

    /* set message category, and return total number of message */
    /*
     * public int[] nmsGetMsgCategoryList(nmsMsgCategory category) {return
     * null;} ;
     */
    public static int nmsSetMsgCategory(int category) {

        NmsLog.trace(HissageTag.api, "set message category:" + category);

        int ret = 0;
        if (category < 0) {
            NmsLog.trace(HissageTag.api, "category is out of index,category:" + category);
        } else {
            ret = engineadapter.get().nmsUISetMsgCategory((byte) category, 1, 0);
        }

        NmsLog.trace(HissageTag.api, "The number of category is:" + ret);

        return ret;
    }

    /*
     * set im mode with indicated contact, and return total number of message
     * with indicated contact
     */
    public synchronized static SNmsImMsgCountInfo nmsSetImMode(int contactId, int flag, int readMode) {
        NmsLog.trace(HissageTag.api, "Set Im mode, the contacId:" + contactId);
        SNmsImMsgCountInfo ret = engineadapter.get().nmsUISetImMode(contactId, 0, flag, readMode);
        NmsLog.trace(HissageTag.api, "Set Im mode, return numbers of message:" + ret.allMsgCount
                + ", unread numbers:" + ret.unreadMsgCount);
        return ret;
    }

    /* search message in db, and return total number of message */
    public static int nmsSearchMsg(String str) {
        NmsLog.trace(HissageTag.api, "Search message, the search string:" + str);
        if (str == null) {
            str = "";
        }

        int ret = engineadapter.get().nmsUISetMsgSearchString(str, 0);

        NmsLog.trace(HissageTag.api, "The number of search message list is:" + ret);
        return ret;
    }

    /*
     * After use bellow 3 function, you can use nmsGetMessage to get all message
     * in a loop
     */
    public static NmsIpSessionMessage nmsGetMessage(int index) {
        NmsLog.trace(HissageTag.api, "get message by index:" + index);

        if (index < 0) {
            NmsLog.trace(HissageTag.api, "The index valule is out of message index:" + index);
            return null;
        }

        NmsIpSessionMessage ipMessage = engineadapter.get().nmsUIGetMsgSummary(index);
        if (null != ipMessage) {
            if (ipMessage.ipMsg.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
                NmsIpLocationMessage locationMessage = (NmsIpLocationMessage) ipMessage.ipMsg;
                ipMessage.ipMsg = NmsIpLocationMessage.formatLocationMsg(locationMessage);
            }
        } else {
            NmsLog.trace(HissageTag.api, "The message object is null, and index:" + index);
        }

        return ipMessage;
    }

    public static NmsIpMessage nmsGetIpMsgInfoViaDbId(short ipdbId) {
        if (ipdbId <= 0) {
            NmsLog.error(HissageTag.api, "nmsGetIpMsgInfo param error: " + ipdbId);
            return null;
        }

        NmsIpMessage ipMessage = engineadapter.get().nmsUIGetMsgKey(ipdbId);

        if (null != ipMessage) {
            if (ipMessage.type == NmsIpMessageConsts.NmsIpMessageType.PICTURE
                    || ipMessage.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
                NmsIpImageMessage imageMessage = (NmsIpImageMessage) ipMessage;
                BitmapFactory.Options options = new BitmapFactory.Options();
                options.inJustDecodeBounds = true;
                if (imageMessage.path != null) {
                    try {
                        BitmapFactory.decodeFile(imageMessage.path, options);
                        options.inJustDecodeBounds = false;
                        imageMessage.widthInPixel = options.outWidth;
                        imageMessage.heightInPixel = options.outHeight;
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                ipMessage = imageMessage;
            } else if (ipMessage.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
                NmsIpLocationMessage locationMessage = (NmsIpLocationMessage) ipMessage;
                ipMessage = NmsIpLocationMessage.formatLocationMsg(locationMessage);
            }
        } else {
            NmsLog.trace(HissageTag.api, "The detail message object is null, and the messageId:"
                    + ipdbId);
        }

        return ipMessage;
    }
}
