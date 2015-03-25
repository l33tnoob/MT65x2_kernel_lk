package com.hissage.message.ip;

public class NmsHesineApiConsts {

    public static final class NmsMessageCategory {
        /* normal and saved */
        public static final int NMS_MSG_CATEGORY_ID_ALL = 1;
        /* only saved(important) */
        public static final int NMS_MSG_CATEGORY_ID_SAVED = 2;
        /* block(spam) */
        public static final int NMS_MSG_CATEGORY_ID_BLOCKED = 3;
        /* group chat */
        public static final int NMS_MSG_CATEGORY_ID_GROUPCHAT = 4;
    }

    public static final class NmsImFlag {
        /* only show message without saved and blocked flag */
        public static final int NMS_IM_FLAG_NORMAL = 1 << 0;
        /* show message with saved flag */
        public static final int NMS_IM_FLAG_SAVED = 1 << 1;
        /* show message with blocked flag */
        public static final int NMS_IM_FLAG_BLOCKED = 1 << 2;

        public static final int NMS_IM_FLAG_CHECK_ATTACH_FINISHED = 1 << 10;

        public static final int NMS_IM_FLAG_ALL = (NMS_IM_FLAG_NORMAL | NMS_IM_FLAG_SAVED | NMS_IM_FLAG_BLOCKED);
    }

    public static final class NmsImReadMode {
        public static final int NMS_IM_READ_MODE_TEXT = 1 << 0;
        public static final int NMS_IM_READ_MODE_GROUP_CREATE_CFG = 1 << 1;
        public static final int NMS_IM_READ_MODE_GROUP_ADD_CFG = 1 << 2;
        public static final int NMS_IM_READ_MODE_GROUP_QUIT_CFG = 1 << 3;
        public static final int NMS_IM_READ_MODE_PIC = 1 << 4;
        public static final int NMS_IM_READ_MODE_TAPE = 1 << 5;
        public static final int NMS_IM_READ_MODE_VCARD = 1 << 6;
        public static final int NMS_IM_READ_MODE_LOCATION = 1 << 7;
        public static final int NMS_IM_READ_MODE_SKETCH = 1 << 8;
        public static final int NMS_IM_READ_MODE_VIDEO = 1 << 9;
        public static final int NMS_IM_READ_MODE_CALENDAR = 1 << 10;
        public static final int NMS_IM_READ_MODE_UNKNOWN = 1 << 11;
        public static final int NMS_IM_READ_MODE_EXT_MMS = 1 << 12;

        public static final int NMS_IM_READ_MODE_ALL_MEDIA = (NMS_IM_READ_MODE_PIC
                | NMS_IM_READ_MODE_SKETCH | NMS_IM_READ_MODE_VIDEO | NMS_IM_READ_MODE_TAPE);
        public static final int NMS_IM_READ_MODE_ALL = (NMS_IM_READ_MODE_TEXT
                | NMS_IM_READ_MODE_GROUP_CREATE_CFG | NMS_IM_READ_MODE_GROUP_ADD_CFG
                | NMS_IM_READ_MODE_GROUP_QUIT_CFG | NMS_IM_READ_MODE_PIC | NMS_IM_READ_MODE_TAPE
                | NMS_IM_READ_MODE_VCARD | NMS_IM_READ_MODE_LOCATION | NMS_IM_READ_MODE_SKETCH
                | NMS_IM_READ_MODE_CALENDAR | NMS_IM_READ_MODE_VIDEO | NMS_IM_READ_MODE_UNKNOWN | NMS_IM_READ_MODE_EXT_MMS);
        public static final int NMS_IM_READ_MODE_SELECT = (NMS_IM_READ_MODE_TEXT
                | NMS_IM_READ_MODE_PIC | NMS_IM_READ_MODE_TAPE
                | NMS_IM_READ_MODE_VCARD | NMS_IM_READ_MODE_LOCATION | NMS_IM_READ_MODE_SKETCH
                | NMS_IM_READ_MODE_CALENDAR | NMS_IM_READ_MODE_VIDEO | NMS_IM_READ_MODE_UNKNOWN | NMS_IM_READ_MODE_EXT_MMS);
    }

    public static final class NmsGetContactMsgRecordIdListFlag {
        public static final int NMS_GET_CONTACT_MSG_ALL = 0;
        public static final int NMS_GET_CONTACT_MSG_UNREAD = 1;
        public static final int NMS_GET_CONTACT_MSG_SAVED = 2;
        public static final int NMS_GET_CONTACT_MSG_MULTIMEDIA = 3 ;
    }
}
