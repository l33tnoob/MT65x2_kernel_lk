package com.mediatek.datatransfer.utils;

public class Constants {
    public static final String BACKUP = "backup";
    public static final String RESTORE = "restore";

    public static final String BACKUP_FILE_EXT = "zip";
    public static final String BACKUP_FOLDER_NAME = ".backup";
    public static final String RECORD_XML = "record.xml";

    public static final String ACTION_NEW_DATA_DETECTED = "com.mediatek.datatransfer.newdata";
    public static final String ACTION_NEW_DATA_DETECTED_TRANSFER = "com.mediatek.datatransfer.newdata_transfer";
    public static final String ACTION_SCAN_DATA = "com.mediatek.datatransfer.scandata";

    public static final int TIME_SLEEP_WHEN_COMPOSE_ONE = 200;

    public static final String RESET_FLAG_FILE = "data/.backuprestore";

    public static final String KEY_SAVED_DATA = "data";
    //public static final String SDCARD2 = "%/mnt/sdcard2/%";

    public static final String ANDROID = "Android ";
    public static final String DATE = "date";
    public static final String SIZE = "size";
    public static final String FILE = "file";
    public static final String FILENAME = "filename";
    public static final String NOTIFY_TYPE = "notify_type";
    public static final String ITEM_TEXT = "text";
    public static final String ITEM_NAME = "name";
    public static final String ITEM_RESULT = "result";
    public static final String ITEM_PACKAGENAME = "packageName";
    public static final String RESULT_KEY = "result";
    public static final String INTENT_SD_SWAP = "com.mediatek.SD_SWAP";
    public static final String ACTION_SD_EXIST = "SD_EXIST";
    public static final String SCAN_RESULT_KEY_PERSONAL_DATA  = "personalData";
    public static final String SCAN_RESULT_KEY_APP_DATA  = "appData";

    public static final String URI_CALENDAR_IMPORTER_EVENTS = "content://com.mediatek.calendarimporter/events";
    public static final String URI_MMS_SMS = "content://mms-sms/conversations/";
    public static final String URI_MMS = "content://mms/";
    public static final String URI_SMS = "content://sms";
    public static final String URI_NOTEBOOK = "content://com.mediatek.notebook.NotePad/notes";

    public static final int NUMBER_IMPORT_CONTACTS_ONE_SHOT = 1500;
    public static final int NUMBER_IMPORT_CONTACTS_EACH = 480;
    public static final int NUMBER_IMPORT_MMS_EACH = 5;
    public static final int NUMBER_IMPORT_SMS_EACH = 20;

    public final static String MESSAGE_BOX_TYPE_INBOX = "1";
    public final static String MESSAGE_BOX_TYPE_SENT = "2";
    public final static String MESSAGE_BOX_TYPE_DRAFT = "3";
    public final static String MESSAGE_BOX_TYPE_OUTBOX = "4";

    public class ModulePath {
        public static final String FOLDER_APP = "App";
        public static final String FOLDER_DATA = "Data";
        public static final String FOLDER_CALENDAR = "calendar";
        public static final String FOLDER_TEMP = "temp";
        public static final String FOLDER_CONTACT = "Contact";
        public static final String FOLDER_MMS = "mms";
        public static final String FOLDER_SMS = "sms";
        public static final String FOLDER_MUSIC = "music";
        public static final String FOLDER_PICTURE = "picture";
        public static final String FOLDER_NOTEBOOK = "notebook";
        public static final String FOLDER_SETTINGS = "settings";
        public static final String FOLDER_BOOKMARK = "bookmark";

        public static final String NAME_CALENDAR = "calendar.vcs";
        public static final String NAME_CONTACT = "contact.vcf";
        public static final String NAME_MMS = "mms";
        public static final String NAME_SMS = "sms";

        public static final String FILE_EXT_APP = ".apk";
        //public static final String FILE_EXT_CALENDAR = ".vcs";
        //public static final String FILE_EXT_CONTACT = ".vcf";
        public static final String FILE_EXT_PDU = ".pdu";

        public static final String ALL_APK_FILES = ".*\\.apk";
        public static final String SCHEMA_ALL_APK = "apps/.*\\.apk";
        public static final String SCHEMA_ALL_CALENDAR = "calendar/calendar[0-9]+\\.vcs";
        public static final String SCHEMA_ALL_CONTACT = "contacts/contact[0-9]+\\.vcf";
        //public static final String SCHEMA_ALL_MMS = "mms/[0-9]+\\.pdu";
        public static final String SCHEMA_ALL_SMS = "sms/sms[0-9]+";
        public static final String SCHEMA_ALL_MUSIC = "music/.*";
        public static final String SCHEMA_ALL_PICTURE = "picture/.*";

        public static final String SMS_VMSG = "sms.vmsg";
        public static final String MMS_XML = "msg_box.xml";
        public static final String NOTEBOOK_XML = "notebook.xml";
        public static final String SETTINGS_XML = "settings.xml";
        
    }

    public class DialogID {
        public static final int DLG_RESTORE_CONFIRM = 2000;
        public static final int DLG_SDCARD_REMOVED = 2001;
        public static final int DLG_SDCARD_FULL = 2002;
        public static final int DLG_RESULT = 2004;
        public static final int DLG_LOADING = 2005;
        public static final int DLG_DELETE_AND_WAIT = 2006;
        public static final int DLG_NO_SDCARD = 2007;
        public static final int DLG_CANCEL_CONFIRM = 2008;
        public static final int DLG_CONTACT_CONFIG = 2009;
        public static final int DLG_EDIT_FOLDER_NAME = 2010;
        public static final int DLG_CREATE_FOLDER_FAILED = 2011;
        public static final int DLG_BACKUP_CONFIRM_OVERWRITE = 2012;
        public static final int DLG_RUNNING = 2013;
    }

    public class MessageID {
        public static final int PRESS_BACK = 0X501;
        public static final int SCANNER_FINISH = 0X502;
    }

    public class State {
        public static final int INIT = 0X00;
        public static final int RUNNING = 0X01;
        public static final int PAUSE = 0X02;
        public static final int CANCEL_CONFIRM = 0X03;
        public static final int CANCELLING = 0X04;
        public static final int FINISH = 0X05;
        public static final int ERR_HAPPEN = 0X06;
    }

    public class LogTag {
        public static final String LOG_TAG = "B&R";
        public static final String CONTACT_TAG = "contact";
        public static final String MESSAGE_TAG = "message";
        public static final String MUSIC_TAG = "music";
        public static final String NOTEBOOK_TAG = "notebook";
        public static final String PICTURE_TAG = "picture";
        public static final String SMS_TAG = "sms";
        public static final String MMS_TAG = "mms";
        public static final String SETTINGS_TAG = "settings";
        public static final String BOOKMARK_TAG = "bookmark";
        public static final String BACKUP_ENGINE_TAG = "backupEngine";
    }

    public class ContactType {
        public static final String ALL = "all";
        public static final String PHONE = "phone";
        public static final String SIM1 = "sim1";
        public static final String SIM2 = "sim2";
        public static final String SIM3 = "sim3";
        public static final String SIM4 = "sim4";
    }

    public class BackupScanType {
        public static final int MD5_EXIST = 1;
        public static final int MD5_NOT_EXIST = 0;
        public static final String PERFERENCE_TAG = "md5";
        public static final String COSMOS = "cosmos";
        public static final String COSMOS_CONTACT_PATH = "BackUpRestore";
        public static final String COSMOS_MMS_PATH = "@mms/mms_pdu";
        public static final String COSMOS_SMS_PATH = "@Tcard/SMS/PDU.o";
        public static final String PLUTO = "pluto";
        public static final String PLUTO_PATH = "~vcard.vcf";
        public static final String DATATRANSFER = "datatransfer";
        public static final String NON_DATATRANSFER = "non_datatransfer";
    }

}
