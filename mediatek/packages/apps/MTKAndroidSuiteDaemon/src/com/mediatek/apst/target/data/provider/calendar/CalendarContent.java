/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.apst.target.data.provider.calendar;

import android.database.Cursor;
import android.net.Uri;

import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.calendar.Calendar;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

public class CalendarContent {

    public static final Uri CONTENT_URI = Uri
            .parse("content://com.android.calendar/calendars");

    /** Id. INTEGER(long). */
    public static final String COLUMN_ID = "_id";
    /**
     * The color of the calendar.
     * <P>
     * Type: INTEGER (color value).
     * </P>
     */
    public static final String COLOR = "color";

    /**
     * The level of access that the user has for the calendar.
     * <P>
     * Type: INTEGER (one of the values below)
     * </P>
     */
    public static final String ACCESS_LEVEL = "access_level";

    /** Cannot access the calendar */
    public static final int NO_ACCESS = 0;
    /** Can only see free/busy information about the calendar */
    public static final int FREEBUSY_ACCESS = 100;
    /** Can read all event details */
    public static final int READ_ACCESS = 200;
    public static final int RESPOND_ACCESS = 300;
    public static final int OVERRIDE_ACCESS = 400;
    /** Full access to modify the calendar, but not the access control settings */
    public static final int CONTRIBUTOR_ACCESS = 500;
    public static final int EDITOR_ACCESS = 600;
    /** Full access to the calendar */
    public static final int OWNER_ACCESS = 700;
    /** Domain admin */
    public static final int ROOT_ACCESS = 800;

    /**
     * Is the calendar selected to be displayed?
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String SELECTED = "selected";

    /**
     * The timezone the calendar's events occurs in
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String TIMEZONE = "timezone";

    /**
     * If this calendar is in the list of calendars that are selected for
     * syncing then "sync_events" is 1, otherwise 0.
     * <p>
     * Type: INTEGER (boolean)
     * </p>
     */
    public static final String COLUMN_SYNC_EVENTS = "sync_events";

    // /**
    // * Sync state data.
    // * <p>Type: String (blob)</p>
    // */
    // public static final String SYNC_STATE = "sync_state";

    /**
     * The account that was used to sync the entry to the device.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_ACCOUNT = "_sync_account";

    /**
     * The type of the account that was used to sync the entry to the device.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String COLUMN_SYNC_ACCOUNT_TYPE = "account_type";

    /**
     * The unique ID for a row assigned by the sync source. NULL if the row has
     * never been synced.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_ID = "_sync_id";

    /**
     * The last time, from the sync source's point of view, that this row has
     * been synchronized.
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String SYNC_TIME = "_sync_time";

    /**
     * The version of the row, as assigned by the server.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_VERSION = "_sync_version";

    /**
     * For use by sync adapter at its discretion; not modified by
     * CalendarProvider Note that this column was formerly named _SYNC_LOCAL_ID.
     * We are using it to avoid a schema change. TODO Replace this with
     * something more general in the future.
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String SYNC_DATA = "_sync_local_id";

    /**
     * Used only in persistent providers, and only during merging.
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String SYNC_MARK = "_sync_mark";

    /**
     * Used to indicate that local, unsynced, changes are present.
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String SYNC_DIRTY = "_sync_dirty";

    /**
     * The name of the account instance to which this row belongs, which when
     * paired with {@link #ACCOUNT_TYPE} identifies a specific account.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String ACCOUNT_NAME = "account_name";

    /**
     * The type of account to which this row belongs, which when paired with
     * {@link #ACCOUNT_NAME} identifies a specific account.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String ACCOUNT_TYPE = "account_type";
    /**
     * The default sort order for this table
     */
    public static final String DEFAULT_SORT_ORDER = "displayName";

    /**
     * The URL to the calendar
     * <P>
     * Type: TEXT (URL)
     * </P>
     */
    public static final String URL = "url";

    /**
     * The name of the calendar
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String COLUMN_NAME = "name";

    /**
     * The display name of the calendar
     * <P>
     * Type: TEXT
     * </P>
     */
    // Modify by Yu for ICS 2011-12-12
    public static final String COLUMN_DISPLAY_NAME = "calendar_displayName";

    /**
     * The location the of the events in the calendar
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String LOCATION = "location";

    /**
     * Should the calendar be hidden in the calendar selection panel?
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String HIDDEN = "hidden";

    /**
     * The owner account for this calendar, based on the calendar feed. This
     * will be different from the _SYNC_ACCOUNT for delegated calendars.
     * <P>
     * Type: String
     * </P>
     */
    public static final String COLUMN_OWNER_ACCOUNT = "ownerAccount";

    /**
     * Can the organizer respond to the event? If no, the status of the
     * organizer should not be shown by the UI. Defaults to 1
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String ORGANIZER_CAN_RESPOND = "organizerCanRespond";

    /**
     * @param cursor
     *            The cursor to query.
     * @return A calendar or null.
     */
    public static Calendar cursorToCalendar(final Cursor cursor) {
        if (null == cursor || cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            return null;
        }
        final Calendar calendar = new Calendar();
        try {
            int colId;
            // id
            colId = cursor.getColumnIndex(COLUMN_ID);
            if (-1 != colId) {
                calendar.setId(cursor.getLong(colId));
            }
            // name
            colId = cursor.getColumnIndex(COLUMN_NAME);
            if (-1 != colId) {
                calendar.setName(cursor.getString(colId));
            }
            // displayName
            colId = cursor.getColumnIndex(COLUMN_DISPLAY_NAME);
            if (-1 != colId) {
                calendar.setDisplayName(cursor.getString(colId));
            }
            // ownerAccount
            colId = cursor.getColumnIndex(COLUMN_OWNER_ACCOUNT);
            if (-1 != colId) {
                calendar.setOwnerAccount(cursor.getString(colId));
            }

        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
        }

        return calendar;
    }

    /**
     * @param cursor
     *            The cursor to parse.
     * @param buffer
     *            The buffer to store the info.
     * @return Result of operation.
     */
    public static int cursorToRaw(final Cursor cursor, final ByteBuffer buffer) {
        if (null == cursor) {
            Debugger.logW(new Object[] { cursor, buffer }, "Cursor is null.");
            return IRawBufferWritable.RESULT_FAIL;
        } else if (cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            Debugger.logW(new Object[] { cursor, buffer },
                    "Cursor has moved to the end.");
            return IRawBufferWritable.RESULT_FAIL;
        } else if (null == buffer) {
            Debugger.logW(new Object[] { cursor, buffer }, "Buffer is null.");
            return IRawBufferWritable.RESULT_FAIL;
        }
        // Mark the current start position of byte buffer in order to reset
        // later when there is not enough space left in buffer
        buffer.mark();
        try {
            int colId;
            // id
            colId = cursor.getColumnIndex(COLUMN_ID);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // name
            colId = cursor.getColumnIndex(COLUMN_NAME);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // displayName
            colId = cursor.getColumnIndex(COLUMN_DISPLAY_NAME);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // ownerAccount
            colId = cursor.getColumnIndex(COLUMN_OWNER_ACCOUNT);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // events
            buffer.putInt(0);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor, buffer }, null, e);
            buffer.reset();
            return IRawBufferWritable.RESULT_FAIL;
        } catch (final BufferOverflowException e) {
            /*
             * Debugger.logW(new Object[]{c, buffer},
             * "Not enough space left in buffer. ", e);
             */
            buffer.reset();
            return IRawBufferWritable.RESULT_NOT_ENOUGH_BUFFER;
        }
        return IRawBufferWritable.RESULT_SUCCESS;
    }

}
