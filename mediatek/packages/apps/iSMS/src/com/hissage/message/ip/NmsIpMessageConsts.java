package com.hissage.message.ip;

public class NmsIpMessageConsts {
    public static final int SWITCHVARIABLE = 0;// UINT32 read burn(8192) + activation(4096) + private message(2048) + close menu setting shortcut(1024): 14336 
    public static final String STATUS = "status";
    public static final String RESULT = "result";
    public static final String REGSTATUS = "regStatus";

    public static final String PACKAGE_NAME_NMS = "com.hissage";

    public static final String ACTION_CHAT_SETTINGS = "com.hissage.ui.activity.NmsChatSettingsActivity";
    public static final String ACTION_MTK_CHAT_SETTINGS = "com.android.mms.intent.action.CHAT_SETTING";
    public static final String ACTION_SYSTEM_SETTINGS = "com.hissage.ui.activity.NmsSystemSettingsActivity";
    public static final String ACTION_ACTIVATION = "com.hissage.ui.activity.NmsActivationActivity";
    public static final String ACTION_TERM = "com.hissage.ui.activity.NmsTermActivity";
    public static final String ACTION_PROFILE = "com.hissage.ui.activity.NmsProfileSettingsActivity";
    public static final String ACTION_LOCATION = "com.hissage.ui.activity.NmsLocationActivity";
    public static final String ACTION_LOCATION_NO_MAP = "com.hissage.ui.activity.NmsLocationNoMapActivity";
    public static final String ACTION_ALL_MEDIA = "com.hissage.ui.activity.NmsAllMediaActivity";
    public static final String ACTION_ALL_LOCATION = "com.hissage.ui.activity.NmsAllLocationsActivity";
    public static final String ACTION_CHAT_DETAILS = "com.hissage.ui.activity.NmsChatDetailsActivity";
    public static final String ACTION_CONTACT_SELECTION = "com.hissage.ui.activity.NmsContactSelectionActivity";
    public static final String ACTION_NEW_EDIT_GROUP_CHAT = "com.hissage.ui.activity.NmsNewEditGroupChatActivity";
    public static final String ACTION_ALL_MEDIA_DETAILS = "com.hissage.ui.activity.NmsAllMediaDetailsActivity";
    public static final String ACTION_READED_BURN_DETAILS = "com.hissage.ui.activity.NmsReadedBurnDetailsActivity";
    public static final String ACTION_QUICK_CONTACT = "com.hissage.ui.activity.NmsQuickContactActivity";
    public static final String ACTION_SKETCH = "com.hissage.ui.activity.NmsSketchActivity";
    public static final String ACTION_MAKE_AUDIO = "com.hissage.ui.activity.NmsAudioActivity";
    public static final String ACTION_SERVICE_CENTER = "com.hissage.ui.activity.NmsServiceCenterActivity";
    public static final String ACTION_READENBURN = "com.hissage.ui.activity.NmsReadedBurnDetailsActivity";

    public static final String CLASS_NAME_CHAT_SETTINGS = "com.hissage.ui.activity.NmsChatSettingsActivity";
    public static final String CLASS_NAME_SYSTEM_SETTINGS = "com.hissage.ui.activity.NmsSystemSettingsActivity";
    public static final String CLASS_NAME_ACTIVATION = "com.hissage.ui.activity.NmsActivationActivity";
    public static final String CLASS_NAME_TERM = "com.hissage.ui.activity.NmsTermActivity";
    public static final String CLASS_NAME_PROFILE = "com.hissage.ui.activity.NmsProfileSettingsActivity";
    public static final String CLASS_NAME_LOCATION = "com.hissage.ui.activity.NmsLocationActivity";
    public static final String CLASS_NAME_LOCATION_NO_MAP = "com.hissage.ui.activity.NmsLocationNoMapActivity";
    public static final String CLASS_NAME_ALL_MEDIA = "com.hissage.ui.activity.NmsAllMediaActivity";
    public static final String CLASS_NAME_ALL_LOCATION = "com.hissage.ui.activity.NmsAllLocationsActivity";
    public static final String CLASS_NAME_CHAT_DETAILS = "com.hissage.ui.activity.NmsChatDetailsActivity";
    public static final String CLASS_NAME_CONTACT_SELECTION = "com.hissage.ui.activity.NmsContactSelectionActivity";
    public static final String CLASS_NAME_NEW_EDIT_GROUP_CHAT = "com.hissage.ui.activity.NmsNewEditGroupChatActivity";
    public static final String CLASS_NAME_ALL_MEDIA_DETAILS = "com.hissage.ui.activity.NmsAllMediaDetailsActivity";
    public static final String CLASS_NAME_READED_BURN_DETAILS = "com.hissage.ui.activity.NmsReadedBurnDetailsActivity";
    public static final String CLASS_NAME_QUICK_CONTACT = "com.hissage.ui.activity.NmsQuickContactActivity";
    public static final String CLASS_NAME_SKETCH = "com.hissage.ui.activity.NmsSketchActivity";
    public static final String CLASS_NAME_AUDIO = "com.hissage.ui.activity.NmsAudioActivity";
    public static final String CLASS_NAME_SERVICE_CENTER = "com.hissage.ui.activity.NmsServiceCenterActivity";

    public final static String NMS_INTENT_SERVICE_READY = "com.isms.service.ready";
    public final static String NMS_SHARE_LOCATION_DONE = "com.isms.location.done";
    public final static String NMS_ENGINE_CONTACT_ID = "engineContactId";
    public final static String NMS_SHOW_LOAD_ALL_MESSAGE = "show_load_all_message";

    public final static String NMS_SELECTION_CONTACTID = "contactId";
    public final static String NMS_SELECTION_SIMID = "SIMID";
    

    public final static String NMS_ACTION_VIEW = "com.isms.view";
    
    public static final String ACTION_OPEN_GROUP = "com.mediatek.mms.action.opengroup";

    public static final class NmsShareLocationDone {
        public static final String NMS_LOCATION_LATITUDE = "com.isms.nms.location.latitude";
        public static final String NMS_LOCATION_LONGITUDE = "com.isms.nms.location.longitude";
        public static final String NMS_LOCATION_ADDRESS = "com.isms.nms.location.address";
        public static final String NMS_LOCATION_PATH = "com.isms.nms.location.path";
    }

    public static final class NmsCancelNotification {
        public static final String NMS_CANCEL_NOTIFICATION = "com.isms.nms.cancelnotify";
        public static final String NMS_CANCEL_NONTFICATION_ID = "com.isms.nms.canelnotify";
    }

    public static final class NmsNewMessageAction {
        public static final String NMS_NEW_MESSAGE_ACTION = "com.isms.nms.newMessage";
        public static final String NMS_NEED_SHOW_NOTIFICATION = "nmsNeedShowNotification";
        public static final String NMS_IP_MESSAGE = "nmsIpMessageKey";
    }

    public static final class NmsDelIpMessageAction {
        public static final String NMS_DEL_IP_MSG_DONE = "com.isms.delIpMsgDone";
        public static final String NMS_IP_MESSAGE_DB_ID = "ipDbId";
    }

    public static final class NmsUpdateGroupAction {
        public static final String NMS_UPDATE_GROUP = "com.isms.nms.updateGroup";
        public static final String NMS_GROUP_ID = "nms_group_id";
    }

    public static final class NmsUpdateSystemContactAction {
        public static final String NMS_UPDATE_CONTACT = "com.isms.nms.updateSystemContact";
    }

    public static final class NmsRefreshContactList {
        public static final String NMS_REFRESH_CONTACTS_LIST = "com.isms.nms.refreshContactList";
        public static final String NMS_CONTACT_ID = "nms_contact_id";
    }

    public static final class NmsRefreshGroupList {
        public static final String NMS_REFRESH_GROUP_LIST = "com.isms.nms.refreshGroupList";
    }

    public static final class NmsRefreshMsgList {
        public static final String NMS_REFRESH_MSG_LIST = "com.isms.nms.refreshMsgList";
    }

    public static final class NmsServiceStatus {
        public static final String NMS_SERVICE_STATUS_ACTION = "com.isms.nmsServiceStatus";
        public static final int ON = 1;
        public static final int OFF = 0;
    }

    public static final class NmsImStatus {
        public static final String NMS_IM_STATUS_ACTION = "com.isms.nmsIMStatus";
        public static final String NMS_CONTACT_CURRENT_STATUS = "com.isms.IM.ContactStatus";
    }

    public static final class NmsSimStatus {
        public static final String NMS_SIM_STATUS_ACTION = "com.isms.nmsSIMStatus";
        public static final String NMS_SIM_STATUS = "nmsSIMStatus";
    }
    
    public static final class NmsSimInfoChanged {
        public static final String NMS_SIM_INFO_ACTION = "com.isms.nmsSIMInfoChanged";
        public static final String NMS_SIM_ID = "nmsSIMId";        
    }

    public static final class NmsStoreStatus {
        public static final String NMS_STORE_STATUS_ACTION = "com.isms.nmsStoreStatus";        
        public static final String NMS_STORE_STATUS = "storeStatus";
        
        public final static int NMS_STORE_FULL = 0;
        public final static int NMS_STORE_ERR  = 1;
    }


    public static final class NmsConnectionStatus {
        // public static final int NMS_UNKONW = -2;
        // public static final int NMS_STATUS_INIT = 0;
        public static final int NMS_STATUS_UNCONNECTED = 1;
        // public static final int NMS_STATUS_BLOCKING = 2;
        public static final int NMS_STATUS_CONNECTING = 3;
        public static final int NMS_STATUS_CONNECTED = 4;
    }

    public static final class NmsSaveHistory {
        public static final String NMS_ACTION_DOWNLOAD_HISTORY = "nms.isms.saveHistory";
        public static final String NMS_DOWNLOAD_HISTORY_PROGRESS = "nms.isms.saveHistoryProgress";
        public static final String NMS_DOWNLOAD_HISTORY_DONE = "nms.isms.saveHistoryDone";
        public static final String NMS_DOWNLOAD_HISTORY_FILE = "nms.isms.saveHistoryFile";
        public static final int NMS_OK = 0;
        public static final int NMS_ERROR = -1;
        public static final int NMS_EMPTY = -2;
    }

    //
    // public static final class NmsActivationStatus {
    // public static final String NMS_ACTIVATION_STATUS_ACTION =
    // "com.isms.nmsActivationStatus";
    // public static final int FAILED_TO_ACTIVATE = -1;
    // public static final int NOT_ACTIVATED = 0;
    // public static final int ACTIVATING = 1;
    // public static final int WAITING_INPUT_NUMBER = 2;
    // public static final int ACTIVATED = 3;
    //
    // }

    public static final class NmsIpMessageStatus {
        public static final String NMS_READEDBURN_TIME_ACTION = "com.isms.nmsReadedburnTime";
        public static final String NMS_MESSAGE_STATUS_ACTION = "com.isms.nmsMessageStatus";
        public static final String NMS_IP_MSG_RECD_ID = "com.isms.nmsMessageRecdId";
        public static final String NMS_IP_MSG_SYS_ID = "com.isms.nmsMessageSysId";
        public static final String NMS_IP_MSG_IPDB_ID = "com.isms.nmsMessageIpDbId";
        public static final String NMS_IP_MSG_STATUS = "com.isms.nmsMessageStatus";
        public static final String NMS_IP_MSG_TIME = "com.isms.nmsMessageTime";
        public static final String NMS_IP_MSG_POSITION = "com.isms.nmsMessagePosition";
        public static final int FAILED = 0;
        public static final int OUTBOX_PENDING = 1; /* not ready to send yet */
        public static final int OUTBOX = 2;
        public static final int SENT = 3;
        public static final int NOT_DELIVERED = 4;
        public static final int DELIVERED = 5;
        public static final int VIEWED = 6;
        public static final int DRAFT = 7;
        public static final int INBOX = 8;
    }

    public static final class NmsDownloadAttachStatus {
        public static final String NMS_DOWNLOAD_ATTACH_STATUS_ACTION = "com.isms.nmsDownloadAttachStatus";
        public static final String NMS_DOWNLOAD_PERCENTAGE = "nmsDownloadPercentage";
        public static final String NMS_DOWNLOAD_MSG_ID = "nmsDownlaodMsgId";
        public static final int FAILED = -1;
        public static final int STARTING = 0;
        public static final int DOWNLOADING = 1; // argument is the downloading
                                                 // percentage
        public static final int DONE = 2;
    }

    public static final class NmsSetProfileResult {
        public static final String NMS_SET_PROFILE_RESULT_ACTION = "com.isms.nmsSetProfileResult";
        public static final int SUCCEED = 0;
        public static final int FAILED = 1; // or -1 is better?
    }

    public static final class NmsBackupMsgStatus {
        public static final String NMS_BACKUP_MSG_STATUS_ACTION = "com.isms.nmsBackupMsgStatus";
        public static final String NMS_UPLOADING_PERCENTAGE = "nmsUploadingPercentage";
        public static final int STARTING = 0;
        public static final int UPLOADING = 1; // argument is the downloading
                                               // percentage
        public static final int FAILED = 2; // or -1 is better?
    }

    public static final class NmsRestoreMsgStatus {
        public static final String NMS_RESTORE_MSG_STATUS_ACTION = "com.isms.nmsRestoreMsgStatus";
        public static final String NMS_DOWNLOAD_PERCENTAGE = "nmsDownloadPercentage";
        public static final int STARTING = 0;
        public static final int DOWNLOADING = 1; // argument is the downloading
                                                 // percentage
        public static final int FAILED = 2; // or -1 is better?
    }

    public static final class NmsIpMessageType {
        public static final int TEXT = 0;
        public static final int GROUP_CREATE_CFG = 1;
        public static final int GROUP_ADD_CFG = 2;
        public static final int GROUP_QUIT_CFG = 3;
        public static final int PICTURE = 4;
        public static final int VOICE = 5;
        public static final int VCARD = 6;
        public static final int LOCATION = 7;
        public static final int SKETCH = 8;
        public static final int VIDEO = 9;
        public static final int CALENDAR = 10;
        public static final int UNKNOWN_FILE = 11;
        public static final int COUNT = 12;
        public static final int READEDBURN = 13;
    }

    public static final class NmsIpMessageFlag {
        public final static int NMS_MSG_FLAG_READ = 1 << 0;
        public final static int NMS_MSG_FLAG_IMPORTANT = 1 << 2;
        public final static int NMS_MSG_FLAG_SPAM = 1 << 3;
        public final static int NMS_MSG_FLAG_GROUP = 1 << 7;
        public final static int NMS_MSG_FLAG_BURN_AFTER_READ = 1 << 13;
        public final static int NMS_MSG_FLAG_READEDBURN_NOT_RECEIVED = 1 << 14;
    }
    public static final class NmsFeatureSupport {
        public final static int NMS_MSG_FLAG_ACTIVATE_PROMPT = 1 << 12;
        public final static int NMS_MSG_FLAG_PRIVATE_MESSAGE = 1 << 11;
    }

    public static final class NmsIpMessageCategory {
        public static final int ALL = 0;
        public static final int Favourite = 1;
        public static final int GroupChat = 2;
        public static final int Spam = 3;
    }

    public static final class NmsMessageProtocol {
        public static final int IP = 1;
        public static final int SMS = 2;
        public static final int MMS = 3;
    }

    public static final class NmsIpMessageSendMode {
        public static final int NORMAL = 0;
        public static final int AUTO = 1;
    }
    
    public static final class NmsIpMessageMediaTypeFlag {
        public static final int PICTURE = 1 << NmsIpMessageType.PICTURE;
        public static final int VOICE = 1 << NmsIpMessageType.VOICE;
        public static final int VCARD = 1 << NmsIpMessageType.VCARD;
        public static final int LOCATION = 1 << NmsIpMessageType.LOCATION;
        public static final int SKETCH = 1 << NmsIpMessageType.SKETCH;
        public static final int VIDEO = 1 << NmsIpMessageType.VIDEO;
        public static final int CALENDAR = 1 << NmsIpMessageType.CALENDAR;
        
        public static final int ALL = PICTURE | VOICE | VCARD | LOCATION | SKETCH | VIDEO | CALENDAR ;
    }
    //M: Activation Statistics
    public static final class NmsUIActivateType {
        public static final int AUTO = 0 ;
        public static final int OTHER = 1 ;
        public static final int EMOTION = 2 ;
        public static final int MULTI_MEDIA = 3 ;
        public static final int SETTING = 4 ;
        public static final int DIALOG = 5 ;
        public static final int PROMPT = 7 ;
        public static final int MESSAGE = 8 ;  
        public static final int TYPE_COUNT = 9 ;
    }
}
