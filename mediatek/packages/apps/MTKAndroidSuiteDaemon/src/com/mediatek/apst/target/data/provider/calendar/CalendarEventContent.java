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
import com.mediatek.apst.util.entity.calendar.CalendarEvent;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

public class CalendarEventContent {
    public static final Uri CONTENT_URI = Uri
            .parse("content://com.android.calendar/events");

    /** Id. INTEGER(long). */
    public static final String COLUMN_ID = "_id";

    /**
     * The {@link Calendars#_ID} of the calendar the event belongs to. Column
     * name.
     * <P>
     * Type: INTEGER
     * </P>
     */
    public static final String CALENDAR_ID = "calendar_id";

    /**
     * The title of the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String TITLE = "title";

    /**
     * The description of the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String DESCRIPTION = "description";

    /**
     * Where the event takes place. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String EVENT_LOCATION = "eventLocation";

    /**
     * A secondary color for the individual event. Reserved for future use.
     * Column name.
     * <P>
     * Type: INTEGER
     * </P>
     */
    public static final String EVENT_COLOR = "eventColor";

    /**
     * The event status. Column name.
     * <P>
     * Type: INTEGER (one of {@link #STATUS_TENTATIVE}...)
     * </P>
     */
    public static final String STATUS = "eventStatus";

    public static final int STATUS_TENTATIVE = 0;
    public static final int STATUS_CONFIRMED = 1;
    public static final int STATUS_CANCELED = 2;

    /**
     * This is a copy of the attendee status for the owner of this event. This
     * field is copied here so that we can efficiently filter out events that
     * are declined without having to look in the Attendees table. Column name.
     * 
     * <P>
     * Type: INTEGER (int)
     * </P>
     */
    public static final String SELF_ATTENDEE_STATUS = "selfAttendeeStatus";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA1 = "sync_data1";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA2 = "sync_data2";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA3 = "sync_data3";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA4 = "sync_data4";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA5 = "sync_data5";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA6 = "sync_data6";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA7 = "sync_data7";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA8 = "sync_data8";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA9 = "sync_data9";

    /**
     * This column is available for use by sync adapters. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String SYNC_DATA10 = "sync_data10";

    /**
     * Used to indicate that a row is not a real event but an original copy of a
     * locally modified event. A copy is made when an event changes from
     * non-dirty to dirty and the event is on a calendar with
     * {@link Calendars#CAN_PARTIALLY_UPDATE} set to 1. This copy does not get
     * expanded in the instances table and is only visible in queries made by a
     * sync adapter. The copy gets removed when the event is changed back to
     * non-dirty by a sync adapter.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String LAST_SYNCED = "lastSynced";

    /**
     * The time the event starts in UTC millis since epoch. Column name.
     * <P>
     * Type: INTEGER (long; millis since epoch)
     * </P>
     */
    public static final String DTSTART = "dtstart";

    /**
     * The time the event ends in UTC millis since epoch. Column name.
     * <P>
     * Type: INTEGER (long; millis since epoch)
     * </P>
     */
    public static final String DTEND = "dtend";

    /**
     * The duration of the event in RFC2445 format. Column name.
     * <P>
     * Type: TEXT (duration in RFC2445 format)
     * </P>
     */
    public static final String DURATION = "duration";

    /**
     * The timezone for the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String EVENT_TIMEZONE = "eventTimezone";

    /**
     * The timezone for the end time of the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String EVENT_END_TIMEZONE = "eventEndTimezone";

    /**
     * Is the event all day (time zone independent). Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String ALL_DAY = "allDay";

    /**
     * Defines how the event shows up for others when the calendar is shared.
     * Column name.
     * <P>
     * Type: INTEGER (One of {@link #ACCESS_DEFAULT}, ...)
     * </P>
     */
    public static final String ACCESS_LEVEL = "accessLevel";

    /**
     * Default access is controlled by the server and will be treated as public
     * on the device.
     */
    public static final int ACCESS_DEFAULT = 0;
    /**
     * Confidential is not used by the app.
     */
    public static final int ACCESS_CONFIDENTIAL = 1;
    /**
     * Private shares the event as a free/busy slot with no details.
     */
    public static final int ACCESS_PRIVATE = 2;
    /**
     * Public makes the contents visible to anyone with access to the calendar.
     */
    public static final int ACCESS_PUBLIC = 3;

    /**
     * If this event counts as busy time or is still free time that can be
     * scheduled over. Column name.
     * <P>
     * Type: INTEGER
     * </P>
     */
    public static final String AVAILABILITY = "availability";

    /**
     * Indicates that this event takes up time and will conflict with other
     * events.
     */
    public static final int AVAILABILITY_BUSY = 0;
    /**
     * Indicates that this event is free time and will not conflict with other
     * events.
     */
    public static final int AVAILABILITY_FREE = 1;

    /**
     * Whether the event has an alarm or not. Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String HAS_ALARM = "hasAlarm";

    /**
     * Whether the event has extended properties or not. Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String HAS_EXTENDED_PROPERTIES = "hasExtendedProperties";

    /**
     * The recurrence rule for the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String RRULE = "rrule";

    /**
     * The recurrence dates for the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String RDATE = "rdate";

    /**
     * The recurrence exception rule for the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String EXRULE = "exrule";

    /**
     * The recurrence exception dates for the event. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String EXDATE = "exdate";

    /**
     * The {@link Events#_ID} of the original recurring event for which this
     * event is an exception. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String ORIGINAL_ID = "original_id";

    /**
     * The _sync_id of the original recurring event for which this event is an
     * exception. The provider should keep the original_id in sync when this is
     * updated. Column name.
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String ORIGINAL_SYNC_ID = "original_sync_id";

    /**
     * The original instance time of the recurring event for which this event is
     * an exception. Column name.
     * <P>
     * Type: INTEGER (long; millis since epoch)
     * </P>
     */
    public static final String ORIGINAL_INSTANCE_TIME = "originalInstanceTime";

    /**
     * The allDay status (true or false) of the original recurring event for
     * which this event is an exception. Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String ORIGINAL_ALL_DAY = "originalAllDay";

    /**
     * The last date this event repeats on, or NULL if it never ends. Column
     * name.
     * <P>
     * Type: INTEGER (long; millis since epoch)
     * </P>
     */
    public static final String LAST_DATE = "lastDate";

    /**
     * Whether the event has attendee information. True if the event has full
     * attendee data, false if the event has information about self only. Column
     * name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String HAS_ATTENDEE_DATA = "hasAttendeeData";

    /**
     * Whether guests can modify the event. Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String GUESTS_CAN_MODIFY = "guestsCanModify";

    /**
     * Whether guests can invite other guests. Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String GUESTS_CAN_INVITE_OTHERS = "guestsCanInviteOthers";

    /**
     * Whether guests can see the list of attendees. Column name.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String GUESTS_CAN_SEE_GUESTS = "guestsCanSeeGuests";

    /**
     * Email of the organizer (owner) of the event. Column name.
     * <P>
     * Type: STRING
     * </P>
     */
    public static final String ORGANIZER = "organizer";

    /**
     * Whether the user can invite others to the event. The
     * GUESTS_CAN_INVITE_OTHERS is a setting that applies to an arbitrary guest,
     * while CAN_INVITE_OTHERS indicates if the user can invite others (either
     * through GUESTS_CAN_INVITE_OTHERS or because the user has modify access to
     * the event). Column name.
     * <P>
     * Type: INTEGER (boolean, readonly)
     * </P>
     */
    public static final String CAN_INVITE_OTHERS = "canInviteOthers";

    // ---------------------------------------------------------------------------
    /**
     * Whether the row has been deleted. A deleted row should be ignored.
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String DELETED = "deleted";

    /**
     * modifyTime
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String MODIFY_TIME = "modifyTime";

    /**
     * createTime
     * <P>
     * Type: TEXT
     * </P>
     */
    public static final String CREATE_TIME = "createTime";

    /**
     * @param cursor
     *            The cursor about CalendarEvent.
     * @return A CalendarEvent or null.
     */
    public static CalendarEvent cursorToEvent(final Cursor cursor) {
        if (null == cursor || cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            return null;
        }
        final CalendarEvent event = new CalendarEvent();
        try {
            int colId;
            // id
            colId = cursor.getColumnIndex(COLUMN_ID);
            if (-1 != colId) {
                event.setId(cursor.getLong(colId));
            }
            // calendar id
            colId = cursor.getColumnIndex(CALENDAR_ID);
            if (-1 != colId) {
                event.setCalendarId(cursor.getLong(colId));
            }
            // title
            colId = cursor.getColumnIndex(TITLE);
            if (-1 != colId) {
                event.setTitle(cursor.getString(colId));
            }
            // timeFrom
            colId = cursor.getColumnIndex(DTSTART);
            if (-1 != colId) {
                event.setTimeFrom(cursor.getLong(colId));
            }
            // timeTo
            colId = cursor.getColumnIndex(DTEND);
            if (-1 != colId) {
                event.setTimeTo(cursor.getLong(colId));
            }
            // isAllDay
            colId = cursor.getColumnIndex(ALL_DAY);
            if (-1 != colId) {
                final boolean isAllDay = (cursor.getInt(colId) == 1) ? true
                        : false;
                event.setAllDay(isAllDay);
            }
            // where
            colId = cursor.getColumnIndex(EVENT_LOCATION);
            if (-1 != colId) {
                event.setEventLocation(cursor.getString(colId));
            }
            // description
            colId = cursor.getColumnIndex(DESCRIPTION);
            if (-1 != colId) {
                event.setDescription(cursor.getString(colId));
            }
            // calendar owner
            colId = cursor.getColumnIndex(ORGANIZER);
            if (-1 != colId) {
                event.setCalendarOwner(cursor.getString(colId));
            }
            // attendees

            // repetition
            colId = cursor.getColumnIndex(RRULE);
            if (-1 != colId) {
                event.setRepetition(cursor.getString(colId));
            }
            // modifyTime
            colId = cursor.getColumnIndex(MODIFY_TIME);
            if (-1 != colId) {
                event.setModifyTime(cursor.getLong(colId));
            }
            // createTime
            colId = cursor.getColumnIndex(CREATE_TIME);
            if (-1 != colId) {
                event.setCreateTime(cursor.getLong(colId));
            }
            // event.setModifyTime(1000000);
            // event.setCreateTime(1000000);
            // duration
            colId = cursor.getColumnIndex(DURATION);
            if (-1 != colId) {
                event.setDuration(cursor.getString(colId));
            }
            // timeZone
            colId = cursor.getColumnIndex(EVENT_TIMEZONE);
            if (-1 != colId) {
                event.setTimeZone(cursor.getString(colId));
            }
            // // transparency
            // colId = c.getColumnIndex(TRANSPARENCY);
            // if (-1 != colId) {
            // event.setTransparency(c.getInt(colId));
            // }
            // // privacy
            // colId = c.getColumnIndex(VISIBILITY);
            // if (-1 != colId) {
            // event.setPrivacy(c.getInt(colId));
            // }
            // availability
            colId = cursor.getColumnIndex(AVAILABILITY);
            if (-1 != colId) {
                event.setAvailability(cursor.getInt(colId));
            }
            // accessLevel
            colId = cursor.getColumnIndex(ACCESS_LEVEL);
            if (-1 != colId) {
                event.setAccessLevel(cursor.getInt(colId));
            }

        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
        }
        return event;
    }

    /**
     * @param cursor
     *            The cursor to parse.
     * @param buffer
     *            The buffer to store info.
     * @return The Result of this operation.
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
            // calendar id
            colId = cursor.getColumnIndex(CALENDAR_ID);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // title
            colId = cursor.getColumnIndex(TITLE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // timeFrom
            colId = cursor.getColumnIndex(DTSTART);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // timeTo
            colId = cursor.getColumnIndex(DTEND);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // isAllDay
            colId = cursor.getColumnIndex(ALL_DAY);
            if (-1 != colId) {
                final boolean isAllDay = (cursor.getInt(colId) == 1) ? true
                        : false;
                RawTransUtil.putBoolean(buffer, isAllDay);
            } else {
                RawTransUtil.putBoolean(buffer, false);
            }
            // eventLocation
            colId = cursor.getColumnIndex(EVENT_LOCATION);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // description
            colId = cursor.getColumnIndex(DESCRIPTION);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // calendar owner
            colId = cursor.getColumnIndex(ORGANIZER);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // attendees
            buffer.putInt(0);
            // repetition
            colId = cursor.getColumnIndex(RRULE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // reminders
            buffer.putInt(0);
            // modifyTime
            colId = cursor.getColumnIndex(MODIFY_TIME);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            colId = cursor.getColumnIndex(CREATE_TIME);
            // createTime
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // duration
            colId = cursor.getColumnIndex(DURATION);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // timeZone
            colId = cursor.getColumnIndex(EVENT_TIMEZONE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // // transparency
            // colId = c.getColumnIndex(RANSPARENCY);
            // if (-1 != colId) {
            // buffer.putInt(c.getInt(colId));
            // } else {
            // buffer.putInt(0);
            // }
            // // privacy
            // colId = c.getColumnIndex(VISIBILITY);
            // if (-1 != colId) {
            // buffer.putInt(c.getInt(colId));
            // } else {
            // buffer.putInt(0);
            // }
            // availability
            colId = cursor.getColumnIndex(AVAILABILITY);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
            } else {
                buffer.putInt(cursor.getInt(0));
            }
            // accessLevel
            colId = cursor.getColumnIndex(ACCESS_LEVEL);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
            } else {
                buffer.putInt(cursor.getInt(0));
            }

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
