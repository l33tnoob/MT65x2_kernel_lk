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
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.calendar.Attendee;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

/**
 * class AttendeeContent, calendar's attendee.
 * 
 * @author
 * 
 */
public class AttendeeContent {

    public static final Uri CONTENT_URI = Uri
            .parse("content://com.android.calendar/attendees");

    /** Id. INTEGER(long). */
    public static final String COLUMN_ID = "_id";
    /**
     * The id of the event.
     * <P>
     * Type: INTEGER
     * </P>
     */
    public static final String COLUMN_EVENT_ID = "event_id";

    /**
     * The name of the attendee.
     * <P>
     * Type: STRING
     * </P>
     */
    public static final String COLUMN_ATTENDEE_NAME = "attendeeName";

    /**
     * The email address of the attendee.
     * <P>
     * Type: STRING
     * </P>
     */
    public static final String COLUMN_ATTENDEE_EMAIL = "attendeeEmail";

    /**
     * The relationship of the attendee to the user.
     * <P>
     * Type: INTEGER (one of {@link #RELATIONSHIP_ATTENDEE}, ...}.
     */
    public static final String COLUMN_ATTENDEE_RELATIONSHIP = "attendeeRelationship";

    public static final int RELATIONSHIP_NONE = 0;
    public static final int RELATIONSHIP_ATTENDEE = 1;
    public static final int RELATIONSHIP_ORGANIZER = 2;
    public static final int RELATIONSHIP_PERFORMER = 3;
    public static final int RELATIONSHIP_SPEAKER = 4;

    /**
     * The type of attendee.
     * <P>
     * Type: Integer (one of {@link #TYPE_REQUIRED}, {@link #TYPE_OPTIONAL})
     */
    public static final String COLUMN_ATTENDEE_TYPE = "attendeeType";

    public static final int TYPE_NONE = 0;
    public static final int TYPE_REQUIRED = 1;
    public static final int TYPE_OPTIONAL = 2;

    /**
     * The attendance status of the attendee.
     * <P>
     * Type: Integer (one of {@link #ATTENDEE_STATUS_ACCEPTED}, ...}.
     */
    public static final String COLUMN_ATTENDEE_STATUS = "attendeeStatus";

    public static final int ATTENDEE_STATUS_NONE = 0;
    public static final int ATTENDEE_STATUS_ACCEPTED = 1;
    public static final int ATTENDEE_STATUS_DECLINED = 2;
    public static final int ATTENDEE_STATUS_INVITED = 3;
    public static final int ATTENDEE_STATUS_TENTATIVE = 4;

    /**
     * @param cursor
     *            The cursor for Attendee.
     * @return A Attendee or null.
     */
    public static Attendee cursorToAttendee(final Cursor cursor) {
        if (null == cursor || cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            return null;
        }

        final Attendee attendee = new Attendee();
        try {
            int colId;
            // id
            colId = cursor.getColumnIndex(COLUMN_ID);
            if (-1 != colId) {
                attendee.setId(cursor.getLong(colId));
            }
            // eventId
            colId = cursor.getColumnIndex(COLUMN_EVENT_ID);
            if (-1 != colId) {
                attendee.setEventId(cursor.getLong(colId));
            }
            // attendeeName
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_NAME);
            if (-1 != colId) {
                attendee.setAttendeeName(cursor.getString(colId));
            }
            // attendeeEmail
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_EMAIL);
            if (-1 != colId) {
                attendee.setAttendeeEmail(cursor.getString(colId));
            }
            // attendeeStatus
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_STATUS);
            if (-1 != colId) {
                attendee.setAttendeeStatus(cursor.getInt(colId));
            }
            // attendeeRelationShip
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_RELATIONSHIP);
            if (-1 != colId) {
                attendee.setAttendeeRelationShip(cursor.getInt(colId));
            }
            // attendeeType
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_TYPE);
            if (-1 != colId) {
                attendee.setAttendeeType(cursor.getInt(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
        }
        return attendee;
    }

    /**
     * @param cursor
     *            The cursor to parse.
     * @param buffer
     *            The buffer to store information about Attendee.
     * @return the result.
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
                Debugger.logI(new Object[] { cursor, buffer },
                        "get Attendee: _id = " + cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // event id
            colId = cursor.getColumnIndex(COLUMN_EVENT_ID);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
                Debugger.logI(new Object[] { cursor, buffer },
                        "get Attendee: event_id = " + cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // attendeeName
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_NAME);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
                Debugger.logI(new Object[] { cursor, buffer },
                        "get Attendee: name = " + cursor.getLong(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // attendeeEmail
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_EMAIL);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
                Debugger.logI(new Object[] { cursor, buffer },
                        "get Attendee: email = " + cursor.getLong(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // attendeeStatus
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_STATUS);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
                Debugger.logI(new Object[] { cursor, buffer },
                        "get Attendee: statue = " + cursor.getLong(colId));
            } else {
                buffer.putInt(0);
            }
            // attendeeRelationShip
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_RELATIONSHIP);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
                Debugger
                        .logI(new Object[] { cursor, buffer },
                                "get Attendee: relationship = "
                                        + cursor.getLong(colId));
            } else {
                buffer.putInt(0);
            }
            // attendeeType
            colId = cursor.getColumnIndex(COLUMN_ATTENDEE_TYPE);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
                Debugger.logI(new Object[] { cursor, buffer },
                        "get Attendee: type = " + cursor.getLong(colId));
            } else {
                buffer.putInt(0);
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
