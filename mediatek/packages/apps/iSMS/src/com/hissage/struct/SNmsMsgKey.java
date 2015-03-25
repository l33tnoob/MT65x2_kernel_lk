package com.hissage.struct;

import java.io.Serializable;

public class SNmsMsgKey implements Serializable {
    private static final long serialVersionUID = -1668157397173599949L;
    public final static int NMS_MSG_NEW_SOURCE_MIN = 50;
    public final static int NMS_CONTACT_SOURCE_MIN = 4;

    public final static int NMS_MSG_SOURCE_SEP = 10;
    public final static String MsgKeyName = "SNmsMsgKey";

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

    // Message status
    public final static int NMS_MSG_STATUS_FAILED = 0;
    public final static int NMS_MSG_STATUS_OUTBOX_PENDING = 1;
    public final static int NMS_MSG_STATUS_OUTBOX = 2;
    public final static int NMS_MSG_STATUS_SENT = 3;
    public final static int NMS_MSG_STATUS_NOT_DELIVERED = 4;
    public final static int NMS_MSG_STATUS_DELIVERED = 5;
    public final static int NMS_MSG_STATUS_VIEWED = 6;
    public final static int NMS_MSG_STATUS_DRAFT = 7;
    public final static int NMS_MSG_STATUS_INBOX = 8;

    // flag
    public final static int NMS_MSG_FLAG_READ = 1 << 0;
    public final static int NMS_MSG_FLAG_UNREAD = 1 << 1;
    public final static int NMS_MSG_FLAG_LOCKED = 1 << 2;
    public final static int NMS_MSG_FLAG_BLOCKED = 1 << 3;
    public final static int NMS_MSG_FLAG_ATTACH = 1 << 4;
    public final static int NMS_MSG_FLAG_CFG = 1 << 5;
    public final static int NMS_MSG_FLAG_REFRESH_CACHE = 1 << 6;
    public final static int NMS_MSG_FLAG_GROUP = 1 << 7;
    public final static int NMS_MSG_FLAG_MULTI_SEND_CHILD = 1 << 8;
    public final static int NMS_MSG_FLAG_MULTI_SEND_PARENT = 1 << 9;
    public final static int NMS_MSG_FLAG_PLACE_HOLDER = 1 << 10; /*
                                                                  * for group
                                                                  * chat and
                                                                  * block
                                                                  * contact, the
                                                                  * place-holder
                                                                  * in their msg
                                                                  * category
                                                                  */
    public final static int NMS_MSG_FLAG_BURN_AFTER_READ = 1 << 13;
    public final static int NMS_MSG_FLAG_READEDBURN_NOT_RECEIVED = 1 << 14;

    public short recordId;
    public int flag;
    public byte source;
    public byte status;
    public byte readMode;
    public short contactRecId;
    public short remoteContactRecordId; /*
                                         * For sent msg it is the TO contact id,
                                         * for received msg it is the FROM
                                         * contact id
                                         */
    public int platformMsgId;
    public int receivedTime;
    public byte mailType;
    public String lineOne;
    public String lineTwo;
    public String attachPath;

    public int count;

    public SNmsMsgKey(short jrecordId, int jflag, byte jsource, byte jstatus, byte jreadMode,
            short jcontactRecId, int jplatformId, int jtime, byte jtype, String jlineOne,
            String jlineTwo, String jattachPath) {
        recordId = jrecordId;
        flag = jflag;
        source = jsource;
        status = jstatus;
        readMode = jreadMode;
        contactRecId = jcontactRecId;
        platformMsgId = jplatformId;
        receivedTime = jtime;
        mailType = jtype;
        lineOne = jlineOne;
        lineTwo = jlineTwo;
        attachPath = jattachPath;
    }
}