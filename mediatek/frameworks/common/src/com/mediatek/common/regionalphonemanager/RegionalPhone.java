/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.common.regionalphone;

import android.net.Uri;
import android.provider.BaseColumns;

/**
 * This class is defined for host APP/plug-in
 * 
 * @author MTK54355
 * 
 */
public class RegionalPhone {
    public static final String AUTHORITY = "com.mediatek.provider.regionalphone";
    public static final String DATABASE_NAME = "regionalphone.db";

    /**
     * The content Uri for tables
     */
    public final static Uri MMS_SMS_URI = Uri.parse("content://" + AUTHORITY + "/mms_sms");
    public final static Uri APN_URI = Uri.parse("content://" + AUTHORITY + "/apn");
    public final static Uri SETTINGS_URI = Uri.parse("content://" + AUTHORITY + "/settings");
    public final static Uri BROWSER_URI = Uri.parse("content://" + AUTHORITY + "/browser");
    public final static Uri WALLPAPER_URI = Uri.parse("content://" + AUTHORITY + "/wallpaper");
    public final static Uri SEARCHENGINE_URI = Uri.parse("content://" + AUTHORITY + "/searchengine");

    /**
     * MMS/SMS table
     */
    public static final String TABLE_MMS_SMS = "mms_sms";

    public static final class MMS_SMS implements BaseColumns {

        /**
         * Default creation mode ("Free", "Warning" or "Restricted")
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
         * MCC + MNC + timestamp
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC_TIMESTAMP = "mcc_mnc_timestamp";
    }

    /**
     * APN table
     */
    public static final String TABLE_APN = "apn";

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
         * MCC + MNC + timestamp
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC_TIMESTAMP = "mcc_mnc_timestamp";
        public final static String MMS_PROXY = "mms_proxy";
        public final static String MMS_PORT = "mms_port";
    }

    /**
     * Settings table
     */
    public static final String TABLE_SETTINGS = "settings";

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
         * MCC + MNC + timestamp
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC_TIMESTAMP = "mcc_mnc_timestamp";
    }

    /**
     * Browser table
     */
    public static final String TABLE_BROWSER = "browser";

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
         * MCC + MNC + timestamp
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC_TIMESTAMP = "mcc_mnc_timestamp";
    }

    /**
     * Search table
     */
    public static final String TABLE_SEARCHENGINE = "searchengine";

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
         * MCC + MNC + timestamp
         * <P>
         * Type:TEXT
         * <P>
         */
        public final static String MCC_MNC_TIMESTAMP = "mcc_mnc_timestamp";
    }

    /**
     * Wallpaper table
     */
    public static final String TABLE_WALLPAPER = "wallpaper";

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
        public final static String MCC_MNC_TIMESTAMP = "mcc_mnc_timestamp";
    }
}
