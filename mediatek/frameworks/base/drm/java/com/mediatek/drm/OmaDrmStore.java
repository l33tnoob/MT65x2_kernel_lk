/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.drm;

/**
 * Defines constants that are used by the OMA DRM v1.0
 *
 */
public class OmaDrmStore {
    /**
     * Class definition for the columns that represent DRM constraints.
     * The constants defined in this class
     * represent three most common types of constraints: count-based,
     * date-based, and duration-based. Two or more constraints can be used
     * at the same time to represent more sophisticated constraints.
     */
    public static class ConstraintsColumns {
        /**
         * This is a count-based constraint. It represents the maximum
         * repeat count that can be performed on an action.
         * Type: INTEGER
         */
        public static final String MAX_REPEAT_COUNT = "max_repeat_count";

        /**
         * This is a count-based constraint. It represents the remaining
         * repeat count that can be performed on an action.
         * Type: INTEGER
         */
        public static final String REMAINING_REPEAT_COUNT = "remaining_repeat_count";

        /**
         * This is a date-based constraint. It represents the time before which
         * an action can be performed on the rights-protected content.
         * Type: TEXT
         */
        public static final String LICENSE_START_TIME = "license_start_time";

        /**
         * This is a date-based constraint. It represents the time after which
         * an action can not be performed on the rights-protected content.
         * Type: TEXT
         */
        public static final String LICENSE_EXPIRY_TIME = "license_expiry_time";

        /**
         * This is a duration-based constaint. It represents the available time left
         * before the license expires.
         * Type: TEXT
         */
        public static final String LICENSE_AVAILABLE_TIME = "license_available_time";

        /**
         * This is a user-defined constraint. It represents the additional constraint
         * using extended metadata.
         * Type: TEXT
         */
        public static final String EXTENDED_METADATA = "extended_metadata";
    }

    /**
     * Defines DRM object types.
     */
    public static class DrmObjectType {
        /**
         * An unknown object type.
         */
        public static final int UNKNOWN = 0x00;
        /**
         * A rights-protected file object type.
         */
        public static final int CONTENT = 0x01;
        /**
         * A rights information object type.
         */
        public static final int RIGHTS_OBJECT = 0x02;
        /**
         * A trigger information object type.
         */
        public static final int TRIGGER_OBJECT = 0x03;
    }

    /**
     * Defines playback states for content.
     */
    public static class Playback {
        /**
         * Playback started.
         */
        public static final int START = 0x00;
        /**
         * Playback stopped.
         */
        public static final int STOP = 0x01;
        /**
         * Playback paused.
         */
        public static final int PAUSE = 0x02;
        /**
         * Playback resumed.
         */
        public static final int RESUME = 0x03;

        /* package */ static boolean isValid(int playbackStatus) {
            boolean isValid = false;

            switch (playbackStatus) {
                case START:
                case STOP:
                case PAUSE:
                case RESUME:
                    isValid = true;
            }
            return isValid;
        }
    }

    /**
     * Defines actions that can be performed on rights-protected content.
     */
    public static class Action {
        /**
         * The default action.
         */
        public static final int DEFAULT = 0x00;
        /**
         * The rights-protected content can be played.
         */
        public static final int PLAY = 0x01;
        /**
         * The rights-protected content can be set as a ringtone.
         */
        public static final int RINGTONE = 0x02;
        /**
         * The rights-protected content can be transferred.
         */
        public static final int TRANSFER = 0x03;
        /**
         * The rights-protected content can be set as output.
         */
        public static final int OUTPUT = 0x04;
        /**
         * The rights-protected content can be previewed.
         */
        public static final int PREVIEW = 0x05;
        /**
         * The rights-protected content can be executed.
         */
        public static final int EXECUTE = 0x06;
        /**
         * The rights-protected content can be displayed.
         */
        public static final int DISPLAY = 0x07;
        /**
         * The rights-protected content can be printed.
         */
        public static final int PRINT = 0x08;
        /**
         * The rights-protected content can be set as wallpaper.
         */
        public static final int WALLPAPER = 0x09; // FL only

        /* package */ static boolean isValid(int action) {
            boolean isValid = false;

            switch (action) {
                case DEFAULT:
                case PLAY:
                case RINGTONE:
                case TRANSFER:
                case OUTPUT:
                case PREVIEW:
                case EXECUTE:
                case DISPLAY:
                case PRINT:
                case WALLPAPER:
                    isValid = true;
            }
            return isValid;
        }
    }

    /**
     * Defines status notifications for digital rights.
     */
    public static class RightsStatus {
        /**
         * The digital rights are valid.
         */
        public static final int RIGHTS_VALID = 0x00;
        /**
         * The digital rights are invalid.
         */
        public static final int RIGHTS_INVALID = 0x01;
        /**
         * The digital rights have expired.
         */
        public static final int RIGHTS_EXPIRED = 0x02;
        /**
         * The digital rights have not been acquired for the rights-protected content.
         */
        public static final int RIGHTS_NOT_ACQUIRED = 0x03;
        /**
         * The digital rights can't be used because Secure timer in invalid state.
         */
        public static final int SECURE_TIMER_INVALID = 0x04;
    }

    /**
     * defines media mime type prefix for OMA DRM v1.0
     */
    public static class MimePrefix {
        /**
         * Constant field signifies that image prefix
         */
        public static final String IMAGE = "image/";
        /**
         * Constant field signifies that audio prefix
         */
        public static final String AUDIO = "audio/";
        /**
         * Constant field signifies that video prefix
         */
        public static final String VIDEO = "video/";
    }

    /**
     * defines the key of DCF file metadata.
     */
    public static class MetadataKey {
        public static final String META_KEY_IS_DRM = "is_drm";
        public static final String META_KEY_CONTENT_URI = "drm_content_uri";
        public static final String META_KEY_OFFSET = "drm_offset";
        public static final String META_KEY_DATALEN = "drm_dataLen";
        public static final String META_KEY_RIGHTS_ISSUER = "drm_rights_issuer";
        public static final String META_KEY_CONTENT_NAME = "drm_content_name";
        public static final String META_KEY_CONTENT_DESCRIPTION = "drm_content_description";
        public static final String META_KEY_CONTENT_VENDOR = "drm_content_vendor";
        public static final String META_KEY_ICON_URI = "drm_icon_uri";
        public static final String META_KEY_METHOD = "drm_method";
        public static final String META_KEY_MIME = "drm_mime_type";
    }

    /**
     * defines the string for OMA DRM v1.0 object mime type.
     */
    public static class DrmObjectMime {
        public static final String MIME_RIGHTS_XML = "application/vnd.oma.drm.rights+xml";
        public static final String MIME_RIGHTS_WBXML = "application/vnd.oma.drm.rights+wbxml";
        public static final String MIME_DRM_CONTENT = "application/vnd.oma.drm.content";
        public static final String MIME_DRM_MESSAGE = "application/vnd.oma.drm.message";
    }

    /**
     * defines the suffix of OMA DRM v1.0 file.
     */
    public static class DrmFileExt {
        public static final String EXT_RIGHTS_XML = ".dr";
        public static final String EXT_RIGHTS_WBXML = ".drc";
        public static final String EXT_DRM_CONTENT = ".dcf";
        public static final String EXT_DRM_MESSAGE = ".dm";
    }

    /**
     * defines method type of OMA DRM v1.0
     */
    public static class DrmMethod {
        public static final int METHOD_NONE = 0;
        public static final int METHOD_FL = 1;
        public static final int METHOD_CD = 2;
        public static final int METHOD_SD = 4;
        public static final int METHOD_FLDCF = 8;
    }

    /**
     * defines the drm extra key & value for OMA DRM v1.0
     */
    public static class DrmExtra {
        public static final String EXTRA_DRM_LEVEL = "android.intent.extra.drm_level";

        public static final int DRM_LEVEL_FL = 1;
        public static final int DRM_LEVEL_SD = 2;
        public static final int DRM_LEVEL_ALL = 4;
    }

    public static class DrmRequestType {
        public static final int TYPE_SET_DRM_INFO = 2021;
        public static final int TYPE_GET_DRM_INFO = 2022;
    }

    public static class DrmRequestKey {
        public static final String KEY_ACTION = "action";
        public static final String KEY_DATA = "data";
        public static final String KEY_DATA_EXTRA_1 = "data_extra_1";
        public static final String KEY_DATA_EXTRA_2 = "data_extra_2";
    }

    public static class DrmRequestAction {
        public static final String ACTION_GET_CONTENT_ID = "getContentId";
        public static final String ACTION_INSTALL_DRM_MSG = "installDrmMsg";
        public static final String ACTION_CONSUME_RIGHTS = "consumeRights";

        public static final String ACTION_UPDATE_CLOCK = "updateClock";
        public static final String ACTION_UPDATE_TIME_BASE = "updateTimeBase";
        public static final String ACTION_UPDATE_OFFSET = "updateOffset";
        public static final String ACTION_LOAD_CLOCK = "loadClock";
        public static final String ACTION_SAVE_CLOCK = "saveClock";
        public static final String ACTION_CHECK_CLOCK = "checkClock";
        public static final String ACTION_LOAD_DEVICE_ID = "loadDeviceId";
        public static final String ACTION_SAVE_DEVICE_ID = "saveDeviceId";
        public static final String ACTION_LOAD_SECURE_TIME = "loadSecureTime";
    }

    public static class DrmRequestResult {
        public static final String RESULT_SUCCESS = "success";
        public static final String RESULT_FAILURE = "failure";
    }
}

