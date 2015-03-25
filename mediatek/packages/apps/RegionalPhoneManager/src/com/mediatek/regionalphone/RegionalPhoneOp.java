package com.mediatek.regionalphone;

import android.provider.BaseColumns;

// this class is defined for operator default database/table
class RegionalPhoneOp {

    /**
     * OP MMS/SMS table
     */
    public static final String TABLE_MMS_SMS = "op_mms_sms";

    public static final class MMS_SMS implements BaseColumns {

        /**
         * Default MMS creation mode ("Free", "Warning" or "Restricted")
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String MMS_CREATION_MODE = "creationMode";

        /**
         * SMS-C number
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String SMS_C_NUMBER = "CNumber";

        /**
         * MCC + MNC
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC = "mcc_mnc";
    }

    /**
     * OP APN table
     */
    public static final String TABLE_APN = "op_apn";

    public static final class APN implements BaseColumns {

        /**
         * MMS name
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String MMS_NAME = "mms_name";

        /**
         * MMS server
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String MMS_SERVER = "mms_server";

        /**
         * GPRS_APN
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String MMS_GPRS_APN = "mms_GPRS_APN";

        /**
         * SMS preferred bearer
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String SMS_PREFERRED_BEARER = "sms_preferredBearer";

        /**
         * MCC + MNC
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC = "mcc_mnc";
        public final static String MMS_PROXY = "mms_proxy";
        public final static String MMS_PORT = "mms_port";
    }

    /**
     * OP Settings table
     */
    public static final String TABLE_SETTINGS = "op_settings";

    public static final class SETTINGS implements BaseColumns {
        /**
         * NITZ auto update default value
         * <P>
         * Type:INTEGER(int) 0:OFF 1:ON
         * <P>
         */
        public static final String NITZ_AUTOUPDATE = "NITZAutoUpdate";

        /**
         * Wifi default setting
         * <P>
         * Type:INTEGER(int) 0:OFF 1:ON
         * <P>
         */
        public static final String WIFI_DEFAULT = "wifi";

        /**
         * MCC + MNC
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC = "mcc_mnc";
    }

    /**
     * OP Browser table
     */
    public static final String TABLE_BROWSER = "op_browser";

    public static final class BROWSER implements BaseColumns {
        // Book marks information for Browser

        /**
         * The user visible title for book mark.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String BOOKMARK_TITLE = "bookmarkTitle";

        /**
         * This column is valid when the row is not a folder. .
         * <P>
         * Type: TEXT (URL)
         * </P>
         */
        public static final String BOOKMARK_URL = "bookmarkURL";

        /**
         * A thumbnail of the page,may be NULL. Must decode via
         * {@link BitmapFactory#decodeByteArray}.
         * <p>
         * Type: BLOB (image)
         * </p>
         */
        public static final String THUMBNAIL = "thumbnail";

        /**
         * Flag indicating if an item is a folder or bookmark. Non-zero values indicate a folder and
         * zero indicates a bookmark.
         * <P>
         * Type: INTEGER (boolean)
         * </P>
         * 
         * @hide
         */
        public static final String IS_FOLDER = "folder";

        /**
         * The ID of the parent folder. ID 1 is the root folder.
         * <P>
         * Type: INTEGER (reference to item in the same table)
         * </P>
         */
        public static final String PARENT = "parent";

        /**
         * MCC + MNC
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC = "mcc_mnc";
    }

    /**
     * OP Search table
     */
    public static final String TABLE_SEARCHENGINE = "op_searchengine";

    public static final class SEARCHENGINE implements BaseColumns {
        /**
         * Search Engine Name for Search Engine Manager
         * <P>
         * Type:TEXT (all)
         * <P>
         */
        public static final String SEARCH_ENGINE_NAME = "searchEngineName";

        /**
         * Search Engine Label for Search Engine Manager
         * <P>
         * Type:TEXT (all)
         * <P>
         */
        public static final String SEARCH_ENGINE_LABEL = "searchEngineLabel";

        /**
         * Search Engine keyword for Search Engine Manager
         * <P>
         * Type:TEXT (all)
         * <P>
         */
        public static final String KEYWORD = "keyword";

        /**
         * Search Engine favicon for Search Engine Manager
         * <P>
         * Type:TEXT (all)
         * <P>
         */
        public static final String FAVICON = "favicon";

        /**
         * Search URL for Search Engine Manager
         * <P>
         * Type:TEXT (all)
         * <P>
         */
        public static final String SEARCH_URL = "searchURL";

        /**
         * Search encoding information for Search Engine Manager
         * <P>
         * Type:TEXT (all)
         * <P>
         */
        public static final String ENCODING = "encoding";

        /**
         * Search suggestion URL for Search Engine Manager
         * <P>
         * Type:TEXT (URL)
         * <P>
         */
        public static final String SUGGESTION_URL = "suggestionURL";

        /**
         * MCC + MNC
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC = "mcc_mnc";
    }

    /**
     * OP Wallpaper table
     */
    public static final String TABLE_WALLPAPER = "op_wallpaper";

    public static final class WALLPAPER implements BaseColumns {
        /**
         * wallpaper image file name
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String IMAGE_FILE_NAME = "fileName";

        /**
         * MCC + MNC + timestamp
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC = "mcc_mnc";
    }

    /**
     * OP MCC MNC table
     */
    public static final String TABLE_MCC_MNC = "op_mcc_mnc";

    public static final class MCC_MNC implements BaseColumns {
        /**
         * MCC
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String MCC = "mcc";

        /**
         * MNC
         * <P>
         * Type:TEXT
         * <P>
         */
        public static final String MNC = "mnc";
    }
}
