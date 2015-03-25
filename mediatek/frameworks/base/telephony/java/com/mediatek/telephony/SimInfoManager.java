/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License..
 */

package com.mediatek.telephony;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Environment;
import android.os.Parcelable;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;
import android.util.Patterns;

import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.provider.BaseColumns;

//MTK-START [mtk04070][111121][ALPS00093395]MTK added
import android.content.ContentUris;
import android.database.DatabaseUtils;
import com.android.internal.telephony.PhoneConstants;
import java.util.ArrayList;
import java.util.List;
//MTK-END [mtk04070][111121][ALPS00093395]MTK added
/**
 *@hide
 */
public final class SimInfoManager implements BaseColumns {
    private static final String LOG_TAG = "PHONE";

    /** @internal */
    public static final Uri CONTENT_URI = 
            Uri.parse("content://telephony/siminfo");

    /** @internal */
    public static final String DEFAULT_SORT_ORDER = "name ASC";
    
    /**
     * <P>Type: TEXT</P>
     * @internal
     */
    public static final String ICC_ID = "icc_id";
    /**
     * <P>Type: TEXT</P>
     * @internal
     */
    public static final String DISPLAY_NAME = "display_name";
    /** @internal */
    public static final int DEFAULT_NAME_MIN_INDEX = 01;
    /** @internal */
    public static final int DEFAULT_NAME_MAX_INDEX= 99;
    /** @internal */
    public static final int DEFAULT_NAME_RES = com.mediatek.internal.R.string.new_sim;

    /**
     * <P>Type: INT</P>
     * @internal
     */
    public static final String NAME_SOURCE = "name_source";
    /** @internal */
    public static final int DEFAULT_SOURCE = 0;
    /** @internal */
    public static final int SIM_SOURCE = 1;
    /** @internal */
    public static final int USER_INPUT = 2;

    /**
     * <P>Type: TEXT</P>
     * @internal
     */
    public static final String NUMBER = "number";
    
    /**
     * 0:none, 1:the first four digits, 2:the last four digits.
     * <P>Type: INTEGER</P>
     * @internal
     */
    public static final String DISPLAY_NUMBER_FORMAT = "display_number_format";
    /** @internal */
    public static final int DISPALY_NUMBER_NONE = 0;
    /** @internal */
    public static final int DISPLAY_NUMBER_FIRST = 1;
    /** @internal */
    public static final int DISPLAY_NUMBER_LAST = 2;
    /** @internal */
    public static final int DISLPAY_NUMBER_DEFAULT = DISPLAY_NUMBER_FIRST;
    
    /**
     * Eight kinds of colors. 0-3 will represent the eight colors.
     * Default value: any color that is not in-use.
     * <P>Type: INTEGER</P>
     * @internal
     */
    public static final String COLOR = "color";
    /** @internal */
    public static final int COLOR_1 = 0;
    /** @internal */
    public static final int COLOR_2 = 1;
    /** @internal */
    public static final int COLOR_3 = 2;
    /** @internal */
    public static final int COLOR_4 = 3;
    /** @internal */
    public static final int COLOR_DEFAULT = COLOR_1;
    
    /**
     * 0: Don't allow data when roaming, 1:Allow data when roaming
     * <P>Type: INTEGER</P>
     * @internal
     */
    public static final String DATA_ROAMING = "data_roaming";
    /** @internal */
    public static final int DATA_ROAMING_ENABLE = 1;
    /** @internal */
    public static final int DATA_ROAMING_DISABLE = 0;
    /** @internal */
    public static final int DATA_ROAMING_DEFAULT = DATA_ROAMING_DISABLE;
    
    /**
     * <P>Type: INTEGER</P>
     * @internal
     */
    public static final String SLOT = "slot";
    /** @internal */
    public static final int SLOT_NONE = -1;
    
    public static final int ERROR_GENERAL = -1;
    public static final int ERROR_NAME_EXIST = -2;

    /**
     * <P>Type: TEXT</P>
     * @internal
     */
    public static final String OPERATOR = "operator";
    /// add by mtk80601 for CT Dual SIM Indicator Feature start
    /** @internal */
    static final int typeBackground = 0;
    /** @internal */
    static final int typeBackgroundDark = 1;
    /** @internal */
    static final int typeBackgroundLight = 2;
    /** @internal */
    static final int typeBackgroundDarkSmall = 3;
    /** @internal */
    static final int typeBackgroundLightSmall = 4;
    /** @internal */
    public static final int[] SimBackgroundRes = setSimBackgroundRes(typeBackground);

    // add by mtk02772 for Consistent UI Design start
    /** @internal */
    public static final int[] SimBackgroundDarkRes = setSimBackgroundRes(typeBackgroundDark);

    /** @internal */
    public static final int[] SimBackgroundLightRes = setSimBackgroundRes(typeBackgroundLight);
    // add by mtk02772 for Consistent UI Design end    
    /// add by mtk80601 for CT dual SIM indicator used by sms, call log. start
    /** @internal */
    public static final int[] SimBackgroundDarkSmallRes = setSimBackgroundRes(typeBackgroundDarkSmall);
    /** @internal */
    public static final int[] SimBackgroundLightSmallRes = setSimBackgroundRes(typeBackgroundLightSmall);
    /// add by mtk80601 for CT dual SIM indicator used by sms, call log. end
    /// add by mtk80601 for CT Dual SIM Indicator Feature end
    private SimInfoManager() {
    }
    
    public static class ErrorCode {
        public static final int ERROR_GENERAL = -1;
        public static final int ERROR_NAME_EXIST = -2;
    }
    
    /**
     * A SimInfoRecord instance represent one record in siminfo database
     * @param mSimInfoId SIM index in database
     * @param mIccId SIM IccId string
     * @param mDisplayName SIM display name shown in SIM management
     * @param mNameSource Source of mDisplayName, 0: default source, 1: SIM source, 2: user source
     * @param mNumber Phone number string
     * @param mDispalyNumberFormat Display format of mNumber, 0: display none, 1: display number first, 2: display number last
     * @param mColor SIM color, 0: blue, 1: oprange, 2: green, 3: purple
     * @param mDataRoaming Data Roaming enable/disable status, 0: Don't allow data when roaming, 1:Allow data when roaming
     * @param mSimSlotId SIM in slot, 0: SIM1, 1: SIM2, 2: SIM3, 3: SIM4
     * @param mSimBackgroundRes SIM icon shown in SIM management, 0: blue, 1: oprange, 2: green, 3: purple
     * @param mOperator SIM operator
     * @param mSimBackgroundDarkRes Consistent UI SIM icon shown in SIM management(dark background), 0: blue, 1: oprange, 2: green, 3: purple
     * @param mSimBackgroundLightRes Consistent UI SIM icon shown in SIM management(light background), 0: blue, 1: oprange, 2: green, 3: purple
     * @param mSimBackgroundDarkSmallRes CT defined SIM icon shown in SIM management(dark background), 0: blue, 1: oprange, 2: green, 3: purple
     * @param mSimBackgroundLightSmallRes CT defined SIM icon shown in SIM management(light background), 0: blue, 1: oprange, 2: green, 3: purple
     */
    public static class SimInfoRecord {
        /** @internal */
        public long mSimInfoId;
        /** @internal */
        public String mIccId;
        /** @internal */
        public String mDisplayName = "";
        /** @internal */
        public int mNameSource;
        /** @internal */
        public String mNumber = "";
        /** @internal */
        public int mDispalyNumberFormat = DISLPAY_NUMBER_DEFAULT;
        /** @internal */
        public int mColor;
        /** @internal */
        public int mDataRoaming = DATA_ROAMING_DEFAULT;
        /** @internal */
        public int mSimSlotId = SLOT_NONE;
        /** @internal */
        public int mSimBackgroundRes = SimBackgroundRes[COLOR_DEFAULT];
        /** @internal */
        public String mOperator = "";
        // add by mtk02772 for Consistent UI Design start
        /** @internal */
        public int mSimBackgroundDarkRes = SimBackgroundDarkRes[COLOR_DEFAULT];
        /** @internal */
        public int mSimBackgroundLightRes = SimBackgroundLightRes[COLOR_DEFAULT];
        // add by mtk02772 for Consistent UI Design end
        /// add by mtk80601 for CT dual SIM indicator used by sms, call log. start
        /** @internal */
        public int mSimBackgroundDarkSmallRes = SimBackgroundDarkSmallRes[COLOR_DEFAULT];
        /** @internal */
        public int mSimBackgroundLightSmallRes = SimBackgroundLightSmallRes[COLOR_DEFAULT];
        /// add by mtk80601 for CT dual SIM indicator used by sms, call log. end
        
        private SimInfoRecord() {
        }
    }

    /**
     * New SimInfoRecord instance and fill in detail info
     * @param cursor
     * @return the query result of desired SimInfoRecord(s)
     */    
    private static SimInfoRecord fromCursor(Cursor cursor) {
        SimInfoRecord info = new SimInfoRecord();
        info.mSimInfoId = cursor.getLong(cursor.getColumnIndexOrThrow(_ID));
        info.mIccId = cursor.getString(cursor.getColumnIndexOrThrow(ICC_ID));
        info.mDisplayName = cursor.getString(cursor.getColumnIndexOrThrow(DISPLAY_NAME));
        info.mNameSource = cursor.getInt(cursor.getColumnIndexOrThrow(NAME_SOURCE));
        info.mNumber = cursor.getString(cursor.getColumnIndexOrThrow(NUMBER));
        info.mDispalyNumberFormat = cursor.getInt(cursor.getColumnIndexOrThrow(DISPLAY_NUMBER_FORMAT));
        info.mColor = cursor.getInt(cursor.getColumnIndexOrThrow(COLOR));
        info.mDataRoaming = cursor.getInt(cursor.getColumnIndexOrThrow(DATA_ROAMING));
        info.mSimSlotId = cursor.getInt(cursor.getColumnIndexOrThrow(SLOT));
        info.mOperator = cursor.getString(cursor.getColumnIndexOrThrow(OPERATOR));
        // CMCC customization starts
        int size = SimBackgroundDarkRes.length;
        String optr = SystemProperties.get("ro.operator.optr");
        if (optr != null && optr.equals("OP01")) {
            if (info.mSimSlotId == 0) {
                info.mColor = 0;
            } else if (info.mSimSlotId == 1) {
                info.mColor = 1;
            }
        }
        // CMCC customization ends
        /// China Telecom SIM Customization start

        if (optr != null && optr.equals("OP09")) {
            /// Set Slot Color
            if (info.mSimSlotId == 0) {
                info.mColor = 1;
            } else if(info.mSimSlotId == 1) {
                info.mColor = 0;
            } else {
                /// to avoid multi-cards resolution
                info.mColor = info.mSimSlotId % 2;
            }
        }
        if (info.mColor >= 0 && info.mColor < size) {
            info.mSimBackgroundRes = SimBackgroundRes[info.mColor];
            // add by mtk02772 for Consistent UI Design start
            info.mSimBackgroundDarkRes = SimBackgroundDarkRes[info.mColor];
            info.mSimBackgroundLightRes = SimBackgroundLightRes[info.mColor];
            // add by mtk02772 for Consistent UI Design end
            /// add by mtk80601 for CT dual SIM indicator used by sms, call log. start
            info.mSimBackgroundDarkSmallRes = SimBackgroundDarkSmallRes[info.mColor%2];
            info.mSimBackgroundLightSmallRes = SimBackgroundLightSmallRes[info.mColor%2];
            /// add by mtk80601 for CT dual SIM indicator used by sms, call log. end
        }
        logd("[fromCursor] iccid:" + info.mIccId + " slot:" + info.mSimSlotId + " id:" + info.mSimInfoId
                + " displayName:" + info.mDisplayName + " color:" + info.mColor + " operator:" + info.mOperator);
        return info;
    }
    
    /**
     * Get the SimInfoRecord(s) of the currently inserted SIM(s)
     * @param ctx Context provided by caller
     * @return Array list of currently inserted SimInfoRecord(s)
     * @internal
     */
    public static List<SimInfoRecord> getInsertedSimInfoList(Context ctx) {
        logd("[getInsertedSimInfoList]+");
        ArrayList<SimInfoRecord> simList = new ArrayList<SimInfoRecord>();
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, SLOT + "!=" + SLOT_NONE, null, null);
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    simList.add(fromCursor(cursor));
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getInsertedSimInfoList]- " + simList.size() + " infos return");
        return simList;
    }
    
    /**
     * Get all the SimInfoRecord(s) in siminfo database
     * @param ctx Context provided by caller
     * @return Array list of all SimInfoRecords in database, include thsoe that were inserted before
     * @internal
     */
    public static List<SimInfoRecord> getAllSimInfoList(Context ctx) {
        logd("[getAllSimInfoList]+");
        ArrayList<SimInfoRecord> simList = new ArrayList<SimInfoRecord>();
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, null, null, null);
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    simList.add(fromCursor(cursor));
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getAllSimInfoList]- " + simList.size() + " infos return");
        return simList;
    }
    
    /**
     * Get the SimInfoRecord according to an index
     * @param ctx Context provided by caller
     * @param simInfoId The unique SimInfoRecord index in database
     * @return SimInfoRecord, maybe null
     * @internal
     */
    public static SimInfoRecord getSimInfoById(Context ctx, long simInfoId) {
        logd("[getSimInfoById]+ simInfoId:" + simInfoId);
        if (simInfoId <= 0) {
            logd("[getSimInfoById]- simInfoId <= 0");
            return null;
        }
        Cursor cursor = ctx.getContentResolver().query(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                null, null, null, null);
        try {
            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    logd("[getSimInfoById]- Info detail:");
                    return fromCursor(cursor);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getSimInfoById]- null info return");
        return null;
    }
    
    /**
     * Get the SimInfoRecord according to a SIM display name
     * @param ctx Context provided by caller
     * @param simName the display name of the SIM card
     * @return SimInfoRecord, maybe null
     * @internal
     */
    public static SimInfoRecord getSimInfoByName(Context ctx, String simName) {
        logd("[getSimInfoByName]+ name:" + simName);
        if (simName == null) {
            logd("[getSimInfoByName]- null name, retur null");
            return null;
        }
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, DISPLAY_NAME + "=?", new String[]{simName}, null);
        try {
            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    logd("[getSimInfoByName]- Info detail:");
                    return fromCursor(cursor);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getSimInfoByName]- null info return");
        return null;
    }
    
    /**
     * Get the SimInfoRecord according to slot
     * @param ctx Context provided by caller
     * @param simSlotId the slot which the SIM is inserted
     * @return SimInfoRecord, maybe null
     * @internal
     */
    public static SimInfoRecord getSimInfoBySlot(Context ctx, int simSlotId) {
        logd("[getSimInfoBySlot]+ simSlotId:" + simSlotId);
        if (simSlotId < 0) {
            logd("[getSimInfoBySlot]- simSlotId < 0");
            return null;
        }
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, SLOT + "=?", new String[]{String.valueOf(simSlotId)}, null);
        try {
            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    logd("[getSimInfoBySlot]- Info detail:");
                    return fromCursor(cursor);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getSimInfoBySlot]- null info return");
        return null;
    }
    
    /**
     * Get the SimInfoRecord according to an IccId
     * @param ctx Context provided by caller
     * @param iccId the IccId of SIM card
     * @return SimInfoRecord, maybe null
     * @internal
     */
    public static SimInfoRecord getSimInfoByIccId(Context ctx, String iccId) {
        logd("[getSimInfoByIccId]+ iccId:" + iccId);
        if (iccId == null) {
            logd("[getSimInfoByIccId]- null iccid");
            return null;
        }
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, ICC_ID + "=?", new String[]{iccId}, null);
        try {
            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    logd("[getSimInfoByIccId]- Info detail:");
                    return fromCursor(cursor);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getSimInfoByIccId]- null info return");
        return null;
    }

    /**
     * Get the SIM count of currently inserted SIM(s)
     * @param ctx Context provided by caller
     * @return currently inserted SIM count
     * @internal
     */
    public static int getInsertedSimCount(Context ctx) {
        logd("[getInsertedSimCount]+");
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, SLOT + "!=" + SLOT_NONE, null, null);
        try {
            if (cursor != null) {
                int count = cursor.getCount();
                logd("[getInsertedSimCount]- " + count + " sim inserted");
                return count;
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getInsertedSimCount]- return 0");
        return 0;
    }
    
    /**
     * Get the SIM count of all SIM(s) in siminfo database
     * @param ctx Context provided by caller
     * @return all SIM count in database, include what was inserted before
     * @internal
     */
    public static int getAllSimCount(Context ctx) {
        logd("[getAllSimCount]+");
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                null, null, null, null);
        try {
            if (cursor != null) {
                int count = cursor.getCount();
                logd("[getAllSimCount]- " + count + " sim in DB");
                return count;
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        logd("[getAllSimCount]- return 0");
        return 0;
    }

    /**
     * Get the operator of a SIM according to its slot Id
     * @param ctx  Context provided by caller
     * @param operator the operator of this SIM
     * @param simInfoId the unique SimInfoRecord index in database
     * @return -1 means general error, >0 means success
     * @internal
     */
    public static int setOperatorById(Context ctx, String operator, long simInfoId) {
        logd("[setOperatorById]+ operator:" + operator + " simInfoId:" + simInfoId);
        if(operator == null) {
            logd("[setOperatorById]- null operator, return -1");
            return ErrorCode.ERROR_GENERAL;
        }
        ContentValues value = new ContentValues(1);
        value.put(OPERATOR, operator);
        logd("[setOperatorById]- operator:" + operator + " set");
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }
    
    /**
     * Set display name by simInfoId
     * @param ctx Context provided by caller
     * @param displayName the display name of SIM card
     * @param simInfoId the unique SimInfoRecord index in database
     * @return -1 means general error, -2 means the name is exist. >0 means success
     * @internal
     */
    public static int setDisplayName(Context ctx, String displayName, long simInfoId) {
        logd("[setDisplayName]+ name:" + displayName + " simInfoId:" + simInfoId);
        if (displayName == null || simInfoId <= 0) {
            logd("[setDisplayName]- fail, return -1");
            return ErrorCode.ERROR_GENERAL;
        }
        // CMCC customization starts
        String optr = SystemProperties.get("ro.operator.optr");
        if ((!optr.equals("OP01")) && (!optr.equals("OP09"))) {
            Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                    new String[]{_ID}, DISPLAY_NAME + "=?", new String[]{displayName}, null);
            try {
                if (cursor != null) {
                    if (cursor.getCount() > 0) {
                        logd("[setDisplayName]- name exist, return -2");
                        return ErrorCode.ERROR_NAME_EXIST;
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        ContentValues value = new ContentValues(1);
        value.put(DISPLAY_NAME, displayName);
        logd("[setDisplayName]- displayName:" + displayName + " set");
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }

    /**
     * Set display name by simInfoId with name source
     * @param ctx Context provided by caller
     * @param displayName the display name of SIM card
     * @param simInfoId the unique SimInfoRecord index in database
     * @param source ex, SYSTEM_INPUT, USER_INPUT
     * @return -1 means general error, -2 means the name is exist. >0 means success
     * @internal
     */
    public static int setDisplayNameEx(Context ctx, String displayName, long simInfoId, long source) {
        logd("[setDisplayNameEx]+  name:" + displayName + " simInfoId:" + simInfoId + " source:" + source);
        if (displayName == null || simInfoId <= 0) {
            logd("[setDisplayNameEx]- fail, return -1");
            return ErrorCode.ERROR_GENERAL;
        }
        // CMCC customization starts
        String optr = SystemProperties.get("ro.operator.optr");
        if ((!optr.equals("OP01")) && (!optr.equals("OP09"))) {
            Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, 
                    new String[]{_ID}, DISPLAY_NAME + "=?", new String[]{displayName}, null);
            try {
                if (cursor != null) {
                    if (cursor.getCount() > 0) {
                        logd("[setDisplayNameEx]- name exist, return -2");
                        return ErrorCode.ERROR_NAME_EXIST;
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        ContentValues value = new ContentValues(1);
        value.put(DISPLAY_NAME, displayName);
        value.put(NAME_SOURCE, source);
        logd("[setDisplayNameEx]- displayName:" + displayName + " set, source:" + source);
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }

    /**
     * Set phone number by simInfoId
     * @param ctx Context provided by caller
     * @param number the phone number of the SIM
     * @param simInfoId the unique SimInfoRecord index in database
     * @return >0 means success
     * @internal
     */
    public static int setNumber(Context ctx, String number, long simInfoId) {
        logd("[setNumber]+ number:" + number + " simInfoId:" + simInfoId);
        if (number == null || simInfoId <= 0) {
            logd("[setNumber]- fail, return -1");
            return -1;
        }
        ContentValues value = new ContentValues(1);
        value.put(NUMBER, number);
        logd("[setNumber]- number:" + number + " set");
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }
    
    /**
     * Set SIM color by simInfoId
     * @param ctx Context provided by caller
     * @param color the color of the SIM
     * @param simInfoId the unique SimInfoRecord index in database
     * @return >0 means success
     * @internal
     */
    public static int setColor(Context ctx, int color, long simInfoId) {
        logd("[setColor]+ color:" + color + " simInfoId:" + simInfoId);
        int size = SimBackgroundRes.length;
        if (color < 0 || simInfoId <= 0 || color >= size) {
            logd("[setColor]- fail, return -1");
            return -1;
        }
        ContentValues value = new ContentValues(1);
        value.put(COLOR, color);
        logd("[setColor]- color:" + color + " set");
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }
    
    /**
     * Set the format.0: none, 1: the first four digits, 2: the last four digits
     * @param ctx Context provided by caller
     * @param format the display format of phone number
     * @param simInfoId the unique SimInfoRecord index in database
     * @return >0 means success
     * @internal
     */
    public static int setDispalyNumberFormat(Context ctx, int format, long simInfoId) {
        logd("[setDispalyNumberFormat]+ format:" + format + " simInfoId:" + simInfoId);
        if (format < 0 || simInfoId <= 0) {
            logd("[setDispalyNumberFormat]- fail, return -1");
            return -1;
        }
        ContentValues value = new ContentValues(1);
        value.put(DISPLAY_NUMBER_FORMAT, format);
        logd("[setDispalyNumberFormat]- format:" + format + " set");
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }
    
    /**
     * Set data roaming by simInfoId
     * @param ctx Context provided by caller
     * @param roaming 0:Don't allow data when roaming, 1:Allow data when roaming
     * @param simInfoId the unique SimInfoRecord index in database
     * @return >0 means success
     * @internal
     */
    public static int setDataRoaming(Context ctx, int roaming, long simInfoId) {
        logd("[setDataRoaming]+ roaming:" + roaming + " simInfoId:" + simInfoId);
        if (roaming < 0 || simInfoId <= 0) {
            logd("[setDataRoaming]- fail, return -1");
            return -1;
        }
        ContentValues value = new ContentValues(1);
        value.put(DATA_ROAMING, roaming);
        logd("[setDataRoaming]- roaming:" + roaming + " set");
        return ctx.getContentResolver().update(ContentUris.withAppendedId(CONTENT_URI, simInfoId), 
                value, null, null);
    }

    /**
     * Add a new SimInfoRecord to siminfo database if needed
     * @param ctx Context provided by caller
     * @param iccId the IccId of the SIM card
     * @param simSlotId the slot which the SIM is inserted
     * @return
     * @internal
     */
    public static Uri addSimInfoRecord(Context ctx, String iccId, int simSlotId) {
        logd("[addSimInfoRecord]+ iccId:" + iccId + " simSlotId:" + simSlotId);
        if (iccId == null) {
            logd("[addSimInfoRecord]- null iccId");
            throw new IllegalArgumentException("IccId should not null.");
        }
        Uri uri;
        ContentResolver resolver = ctx.getContentResolver();
        String selection = ICC_ID + "=?";
        Cursor cursor = resolver.query(CONTENT_URI, new String[]{_ID, SLOT}, selection, new String[]{iccId}, null);
        try {
            if (cursor == null || !cursor.moveToFirst()) {
                ContentValues values = new ContentValues();
                values.put(ICC_ID, iccId);
                values.put(COLOR, -1);
                values.put(SLOT, simSlotId);
                uri = resolver.insert(CONTENT_URI, values);
                logd("[addSimInfoRecord]- new sim inserted in slot:" + simSlotId);
            } else {
                long simInfoId = cursor.getLong(0);
                int oldSlot = cursor.getInt(1);
                uri = ContentUris.withAppendedId(CONTENT_URI, simInfoId);
                if (simSlotId != oldSlot) {
                    ContentValues values = new ContentValues(1);
                    values.put(SLOT, simSlotId);
                    resolver.update(uri, values, null, null);
                } 
                logd("[addSimInfoRecord]- existed sim in slot:" + simSlotId);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return uri;
    }

    /**
     * Set default name by simInfoId
     * @param ctx Context provided by caller
     * @param simInfoId the unique SimInfoRecord index in database
     * @param name the default name of the SIM card
     * @return -1 means general error, >0 means success
     * @internal
     */    
    public static int setDefaultName(Context ctx, long simInfoId, String name) {
        logd("[setDefaultName]+ simInfoId:" + simInfoId + " name:" + name);
        if (simInfoId <= 0) {
            logd("[setDefaultName]- simInfoId <= 0, return -1");
            return ErrorCode.ERROR_GENERAL;
        }
        String defaultName = ctx.getString(DEFAULT_NAME_RES);
        ContentResolver resolver = ctx.getContentResolver();
        Uri uri = ContentUris.withAppendedId(CONTENT_URI, simInfoId);
        if (name != null) {
            int result = setDisplayNameEx(ctx, name, simInfoId, DEFAULT_SOURCE);
            if (result > 0) {
                logd("[setDefaultName]- name:" + name + " set");
                return result;
            }
        }
        int index = getAppropriateIndex(ctx, simInfoId, name);
        String suffix = getSuffixFromIndex(index);
        ContentValues value = new ContentValues(1);
        String displayName;
        // CMCC/China Telecom customization starts
        String optr = SystemProperties.get("ro.operator.optr");
        if (optr != null && (optr.equals("OP01") || optr.equals("OP09")))
            displayName = defaultName;
        else
            displayName = (name == null ? defaultName + " " + suffix : name + " " + suffix);
        value.put(DISPLAY_NAME, displayName);
        logd("[setDefaultName]- mDisplayName:" + displayName + " set");
        return ctx.getContentResolver().update(uri, value, null, null);
    }

    /**
     * Set default name by simInfoId with name source
     * @param ctx Context provided by caller
     * @param simInfoId the unique SimInfoRecord index in database
     * @param name the default name of the SIM card
     * @param nameSource ex, SYSTEM_INPUT, USER_INPUT
     * @return -1 means general error, >0 means success
     * @internal
     */
    public static int setDefaultNameEx(Context ctx, long simInfoId, String name, long nameSource) {
        logd("[setDefaultNameEx]+ simInfoId:" + simInfoId + " name:" + name + " source:" + nameSource);
        if (simInfoId <= 0) {
            logd("[setDefaultNameEx]- simInfoId <= 0, return -1");
            return ErrorCode.ERROR_GENERAL;
        }
        String defaultName = ctx.getString(DEFAULT_NAME_RES);
        ContentResolver resolver = ctx.getContentResolver();
        Uri uri = ContentUris.withAppendedId(CONTENT_URI, simInfoId);
        if (name != null) {
            int result = setDisplayNameEx(ctx, name, simInfoId, nameSource);
            if (result > 0) {
                logd("[setDefaultNameEx]- name:" + name + " set, source:" + nameSource);
                return result;
            }
        }
        int index = getAppropriateIndex(ctx, simInfoId, name);
        String suffix = getSuffixFromIndex(index);
        ContentValues value = new ContentValues(1);
        String displayName;
        // CMCC/China Telecom customization starts
        String optr = SystemProperties.get("ro.operator.optr");
        if (optr != null && (optr.equals("OP01") || optr.equals("OP09"))) {
            displayName = defaultName;
        } else {
            displayName = (name == null ? defaultName + " " + suffix : name + " " + suffix);
        }
        value.put(DISPLAY_NAME, displayName);
        value.put(NAME_SOURCE, nameSource);
        logd("[setDefaultNameEx]- mDisplayName:" + displayName + " set, source:" + nameSource);
        return ctx.getContentResolver().update(uri, value, null, null);
    }

    private static String getSuffixFromIndex(int index) {
        logd("[getSuffixFromIndex]");
        if (index < 10) {
            return "0" + index;
        } else {
            return String.valueOf(index);
        }
    }

    private static int getAppropriateIndex(Context ctx, long simInfoId, String name) {
        String defaultName = ctx.getString(DEFAULT_NAME_RES);
        StringBuilder sb = new StringBuilder(DISPLAY_NAME + " LIKE ");
        if (name == null) {
            DatabaseUtils.appendEscapedSQLString(sb, defaultName + '%');
        } else {
            DatabaseUtils.appendEscapedSQLString(sb, name + '%');
        }
        sb.append(" AND (");
        sb.append(_ID + "!=" + simInfoId);
        sb.append(")");
        
        Cursor cursor = ctx.getContentResolver().query(CONTENT_URI, new String[]{_ID, DISPLAY_NAME},
                sb.toString(), null, DISPLAY_NAME);
        ArrayList<Long> array = new ArrayList<Long>();
        int index = DEFAULT_NAME_MIN_INDEX;
        if (cursor != null) {
            while (cursor.moveToNext()) {
                String displayName = cursor.getString(1);
                if (displayName != null) {
                    int length = displayName.length();
                    if (length >= 2) {
                        String sub = displayName.substring(length -2);
                        if (TextUtils.isDigitsOnly(sub)) {
                            long value = Long.valueOf(sub);
                            array.add(value);
                        }
                    }
                }
            }
            cursor.close();
        }
        for (int i = DEFAULT_NAME_MIN_INDEX; i <= DEFAULT_NAME_MAX_INDEX; i++) {
            if (array.contains((long)i)) {
                continue;
            } else {
                index = i;
                break;
            }
        }
        logd("[getAppropriateIndex] index:" + index);
        return index;
    }  

    private static void logd(String msg) {
        Log.d(LOG_TAG, "[SimInfoManager]" + msg);
    }
    /// add by mtk80601 for CT Dual SIM Indicator Feature start
    private static int[] setSimBackgroundRes(int type) {
        int[] tmpSimBackgroundRes = null;
        String optr = SystemProperties.get("ro.operator.optr");
        switch (type) {
            case typeBackground:
                tmpSimBackgroundRes = new int[] {
                    com.mediatek.internal.R.drawable.sim_background_blue,
                    com.mediatek.internal.R.drawable.sim_background_orange,
                    com.mediatek.internal.R.drawable.sim_background_green,
                    com.mediatek.internal.R.drawable.sim_background_purple
                };
                break;
            case typeBackgroundDark:
                if (optr != null && optr.equals("OP09")) {
                    tmpSimBackgroundRes = new int[] {
                        com.mediatek.internal.R.drawable.dark_large_sim_2,
                        com.mediatek.internal.R.drawable.dark_large_sim_1
                    };
                } else {
                    tmpSimBackgroundRes = new int[] {
                        com.mediatek.internal.R.drawable.sim_dark_blue,
                        com.mediatek.internal.R.drawable.sim_dark_orange,
                        com.mediatek.internal.R.drawable.sim_dark_green,
                        com.mediatek.internal.R.drawable.sim_dark_purple
                    };
                }
                break;
            case typeBackgroundLight:
                if (optr != null && optr.equals("OP09")) {
                    tmpSimBackgroundRes = new int[] {
                        com.mediatek.internal.R.drawable.light_large_sim_2,
                        com.mediatek.internal.R.drawable.light_large_sim_1
                    };
                } else {
                    tmpSimBackgroundRes = new int[] {
                        com.mediatek.internal.R.drawable.sim_light_blue,
                        com.mediatek.internal.R.drawable.sim_light_orange,
                        com.mediatek.internal.R.drawable.sim_light_green,
                        com.mediatek.internal.R.drawable.sim_light_purple
                    };
                }
                break;
            case typeBackgroundDarkSmall:
                tmpSimBackgroundRes = new int[] {
                    com.mediatek.internal.R.drawable.dark_small_sim_2,
                    com.mediatek.internal.R.drawable.dark_small_sim_1
                };
                break;
            case typeBackgroundLightSmall:
                tmpSimBackgroundRes = new int[] {
                    com.mediatek.internal.R.drawable.light_small_sim_2,
                    com.mediatek.internal.R.drawable.light_small_sim_1
                };
                break;
        }
        return tmpSimBackgroundRes;
    }
    /// add by mtk80601 for CT Dual SIM Indicator Feature end
    
}
