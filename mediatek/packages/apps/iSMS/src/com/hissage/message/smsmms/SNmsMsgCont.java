package com.hissage.message.smsmms;

import com.hissage.util.data.NmsConsts;

public class SNmsMsgCont {

    public final static String NMS_MSG_TAG = "msg_tag";

    public final static int NMS_MSG_NEW_SOURCE_MIN = 50;
    public final static int NMS_CONTACT_SOURCE_MIN = 4;

    public final static int NMS_MSG_SOURCE_SEP = 10;

    // Message source
    public final static int NMS_MSG_SOURCE_HESINE = 1;
    public final static int NMS_MSG_SOURCE_SMS = NMS_MSG_SOURCE_HESINE + NMS_MSG_SOURCE_SEP;
    public final static int NMS_MSG_SOURCE_MMS = NMS_MSG_SOURCE_SMS + NMS_MSG_SOURCE_SEP;
    public final static int NMS_MSG_SOURCE_END = NMS_MSG_SOURCE_MMS + NMS_MSG_SOURCE_SEP;

    /* TODO: add current simCard index to it */

    public static boolean NMS_IS_HESINE_MSG(int source) {
        return ((source) >= NMS_MSG_SOURCE_HESINE && (source) < NMS_MSG_SOURCE_SMS);
    }

    public static boolean NMS_IS_SYSTEM_MSG(int source) {
        return ((source) >= NMS_MSG_SOURCE_SMS && (source) < NMS_MSG_SOURCE_END);
    }

    public static boolean NMS_IS_SMS_MSG(int source) {
        return ((source) >= NMS_MSG_SOURCE_SMS && (source) < NMS_MSG_SOURCE_MMS);
    }

    public static boolean NMS_IS_MMS_MSG(int source) {
        return ((source) >= NMS_MSG_SOURCE_MMS && (source) < NMS_MSG_SOURCE_END);
    }

    public final static int NMS_MSG_UNLOCKED = 0;
    public final static int NMS_MSG_LOCKED = 1;

    public byte numOfAttach = 0;
    public byte encodingType;
    public byte readMode;
    public long msgId = -1;
    public int deleteTime;
    public int bodyLen;
    public int sendId;
    public int restLen;
    public int[] attachSize;
    public int msgType = -1;
    public byte readed;
    public int isLocked = NMS_MSG_UNLOCKED;
    public String[] pAttachName = new String[5];
    public String pFrom = null;
    public String pTo = null;
    public String pThreadNumber = null;
    public String pCc;
    public String pBcc;
    public String pSubject; // removed by luozheng, useless Subject;
    public String pBody;
    public String[] pAttachContent;
    public long simId = NmsConsts.INVALID_SIM_ID;

    // deleted
    public long status;
    public long threadId;

    public int source = -1;// don't use this field, just for nmsRefreshMsg
                           // whenIp service start.
}
