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

package com.mediatek.apst.target.data.proxy.calendar;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.android.content.DefaultBulkInsertHelper;
import com.mediatek.android.content.MeasuredContentValues;
import com.mediatek.apst.target.data.provider.calendar.AttendeeContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarEventContent;
import com.mediatek.apst.target.data.provider.calendar.ReminderContent;
import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.target.util.SharedPrefs;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.calendar.Attendee;
import com.mediatek.apst.util.entity.calendar.CalendarEvent;
import com.mediatek.apst.util.entity.calendar.Reminder;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.util.List;

/**
 * Class Name: CalendarProxy
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.calendar
 * <p>
 * Created on: 2011-05-19
 * <p>
 * <p>
 * Description:
 * <p>
 * Facade of the sub system of calendar related operations.
 * <p>
 * 
 * @author mtk81022 Shaoying Han
 * @version V1.0
 */
public final class CalendarProxy extends ContextBasedProxy {

    /** Singleton instance. */
    private static CalendarProxy sInstance = null;

    /**
     * @param context
     *            The Context.
     */
    private CalendarProxy(final Context context) {
        super(context);
        setProxyName("CalendarProxy");
    }

    /**
     * @param context
     *            The current Context.
     * @return A instance of the CalendarProxy.
     */
    public static synchronized CalendarProxy getInstance(final Context context) {
        if (null == sInstance) {
            sInstance = new CalendarProxy(context);
        } else {
            sInstance.setContext(context);
        }
        return sInstance;
    }

    /**
     * @param event
     *            The event in Calendar.
     * @return The id of the inserted event.
     */
    public long insertEvent(final CalendarEvent event) {
        if (event == null) {
            Debugger.logW(new Object[] { event }, "Event passed in is null.");
            return DatabaseRecordEntity.ID_NULL;
        }
        long insertedId = DatabaseRecordEntity.ID_NULL;
        final ContentValues values = new ContentValues(19);
        values.put(CalendarEventContent.CALENDAR_ID, event.getCalendarId());
        values.put(CalendarEventContent.TITLE, event.getTitle());
        values.put(CalendarEventContent.DTSTART, event.getTimeFrom());
        values.put(CalendarEventContent.ALL_DAY, event.isAllDay() ? 1 : 0);
        values.put(CalendarEventContent.EVENT_LOCATION, event
                .getEventLocation());
        values.put(CalendarEventContent.DESCRIPTION, event.getDescription());
        values.put(CalendarEventContent.ORGANIZER, event.getCalendarOwner());
        values.put(CalendarEventContent.RRULE, event.getRepetition());

        // Modify Time
        values.put(CalendarEventContent.MODIFY_TIME, event.getModifyTime());
        // Create Time
        values.put(CalendarEventContent.CREATE_TIME, event.getCreateTime());
        // duration
        values.put(CalendarEventContent.DURATION, event.getDuration());
        // DEND: Added by Jinbo 2011-12-15
        if (event.getDuration() != null) { // duration != null
            values.put(CalendarEventContent.DTEND, (Long) null);
        } else { // duration == null
            if ((Long) event.getTimeTo() == null) {
                values.put(CalendarEventContent.DTEND, 0);
            } else {
                values.put(CalendarEventContent.DTEND, event.getTimeTo());
            }
        }

        values.put(CalendarEventContent.GUESTS_CAN_MODIFY,
                DatabaseRecordEntity.TRUE);
        // 14
        // values.put(CalendarEventContent.SELF_ATTENDEE_STATUS,
        // AttendeeContent.ATTENDEE_STATUS_ACCEPTED);
        if (event.getReminders() != null && event.getReminders().size() > 0) {
            values.put(CalendarEventContent.HAS_ALARM,
                    DatabaseRecordEntity.TRUE);
        } else {
            values.put(CalendarEventContent.HAS_ALARM,
                    DatabaseRecordEntity.FALSE);
        }

        values.put(CalendarEventContent.HAS_EXTENDED_PROPERTIES,
                DatabaseRecordEntity.TRUE);
        if (event.getAttendees() != null && event.getAttendees().size() > 0) {
            values.put(CalendarEventContent.HAS_ATTENDEE_DATA,
                    DatabaseRecordEntity.TRUE);
        } else {
            values.put(CalendarEventContent.HAS_ATTENDEE_DATA,
                    DatabaseRecordEntity.FALSE);
        }
        values.put(CalendarEventContent.EVENT_TIMEZONE, event.getTimeZone());
        // Modify by Yu 2011-12-11
        values.put(CalendarEventContent.AVAILABILITY, event.getAvailability());
        values.put(CalendarEventContent.ACCESS_LEVEL, event.getAccessLevel());

        try {
            final Uri uri = getObservedContentResolver().insert(
                    CalendarEventContent.CONTENT_URI, values);
            if (uri != null) {
                insertedId = Long.parseLong(uri.getLastPathSegment());
            }
            // else { // if insert fail,insertedId = -1 ,then also insert
            // reminder attendee ?
            // return insertedId;
            // }
            // Debugger.logE(new Object[] { event }, ">>InsertId"+insertedId);
            // // Yu
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { event }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { event }, null, e);
        }

        // Set the new id after insertion
        event.setId(insertedId);
        // reminders
        for (final Reminder reminder : event.getReminders()) {
            insertReminder(reminder, insertedId);
        }
        // attendees
        for (final Attendee attendee : event.getAttendees()) {
            insertAttendee(attendee, insertedId);
        }
        return insertedId;
    }

    /**
     * @param attendee
     *            The Attendee to be inserted.
     * @param eventId
     *            The id of the event.
     * @return The id of the inserted Attendee.
     */
    public long insertAttendee(final Attendee attendee, final long eventId) {
        if (attendee == null) {
            Debugger.logW(new Object[] { attendee },
                    "Attendee passed in is null.");
            return DatabaseRecordEntity.ID_NULL;
        }
        long insertedId = DatabaseRecordEntity.ID_NULL;
        final ContentValues values = new ContentValues(6);
        values.put(AttendeeContent.COLUMN_EVENT_ID, eventId);
        values.put(AttendeeContent.COLUMN_ATTENDEE_NAME, attendee
                .getAttendeeName());
        values.put(AttendeeContent.COLUMN_ATTENDEE_EMAIL, attendee
                .getAttendeeEmail());
        values.put(AttendeeContent.COLUMN_ATTENDEE_STATUS, attendee
                .getAttendeeStatus());
        values.put(AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP, attendee
                .getAttendeeRelationShip());
        values.put(AttendeeContent.COLUMN_ATTENDEE_TYPE, attendee
                .getAttendeeType());

        try {
            final Uri uri = getObservedContentResolver().insert(
                    AttendeeContent.CONTENT_URI, values);
            if (uri != null) {
                insertedId = Long.parseLong(uri.getLastPathSegment());
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { attendee }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { attendee }, null, e);
        }

        // Set the new id after insertion
        attendee.setId(insertedId);
        return insertedId;
    }

    /**
     * @param reminder
     *            The reminder to be inserted.
     * @param eventId
     *            The id of the event.
     * @return The id of the inserted reminder.
     */
    public long insertReminder(final Reminder reminder, final long eventId) {
        if (reminder == null) {
            Debugger.logW(new Object[] { reminder },
                    "Reminder passed in is null.");
            return DatabaseRecordEntity.ID_NULL;
        }
        long insertedId = DatabaseRecordEntity.ID_NULL;
        final ContentValues values = new ContentValues(3);
        values.put(ReminderContent.COLUMN_EVENT_ID, eventId);
        values.put(ReminderContent.COLUMN_METHOD, reminder.getMethod());
        values.put(ReminderContent.COLUMN_MINUTES, reminder.getMinutes());
        try {
            final Uri uri = getObservedContentResolver().insert(
                    ReminderContent.CONTENT_URI, values);
            if (uri != null) {
                insertedId = Long.parseLong(uri.getLastPathSegment());
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { reminder }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { reminder }, null, e);
        }
        reminder.setId(insertedId);
        return insertedId;
    }

    /**
     * @param eventId
     *            The id of the event.
     * @param withReminder
     *            Whether to get the reminder of the event.
     * @param withAttendee
     *            Whether to get the attendee of the event.
     * @return The event of the calendar.
     */
    public CalendarEvent getEvent(final long eventId,
            final boolean withReminder, final boolean withAttendee) {
        CalendarEvent event = new CalendarEvent();
        Cursor cEvent;
        cEvent = getContentResolver().query(
                CalendarEventContent.CONTENT_URI,
                new String[] { CalendarEventContent.COLUMN_ID,
                        CalendarEventContent.CALENDAR_ID,
                        CalendarEventContent.TITLE,
                        CalendarEventContent.DTSTART,
                        CalendarEventContent.DTEND,
                        CalendarEventContent.ALL_DAY,
                        CalendarEventContent.EVENT_LOCATION,
                        CalendarEventContent.DESCRIPTION,
                        CalendarEventContent.ORGANIZER,
                        CalendarEventContent.RRULE,
                        CalendarEventContent.MODIFY_TIME,
                        CalendarEventContent.CREATE_TIME,
                        CalendarEventContent.DURATION,
                        CalendarEventContent.EVENT_TIMEZONE,
                        CalendarEventContent.AVAILABILITY,
                        CalendarEventContent.ACCESS_LEVEL },
                CalendarEventContent.COLUMN_ID + "=" + eventId, null, null);
        if (null != cEvent && cEvent.moveToNext()) {
            event = CalendarEventContent.cursorToEvent(cEvent);
            cEvent.close(); // Yu
        } else {
            Debugger.logW(new Object[] { eventId }, "Do not find the event. ");
            return null;
        }

        if (!withReminder) {
            return event;
        }
        Cursor cReminder;
        cReminder = getContentResolver().query(
                ReminderContent.CONTENT_URI,
                new String[] { ReminderContent.COLUMN_ID,
                        ReminderContent.COLUMN_EVENT_ID,
                        ReminderContent.COLUMN_METHOD,
                        ReminderContent.COLUMN_MINUTES },
                ReminderContent.COLUMN_EVENT_ID + "=" + eventId, null, null);
        if (cReminder != null) {
            while (cReminder.moveToNext()) {
                Reminder reminder = new Reminder();
                reminder = ReminderContent.cursorToReminder(cReminder);
                if (event != null) {
                    event.addReminder(reminder);
                }

            }
            cReminder.close(); // Yu
        } else {
            Debugger.logW(new Object[] { eventId },
                    "Do not find the reminder. ");
        }

        if (!withAttendee) {
            return event;
        }

        Cursor cAttendee;
        cAttendee = getContentResolver().query(
                AttendeeContent.CONTENT_URI,
                new String[] { AttendeeContent.COLUMN_EVENT_ID,
                        AttendeeContent.COLUMN_ATTENDEE_NAME,
                        AttendeeContent.COLUMN_ATTENDEE_EMAIL,
                        AttendeeContent.COLUMN_ATTENDEE_STATUS,
                        AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP,
                        AttendeeContent.COLUMN_ATTENDEE_TYPE },
                AttendeeContent.COLUMN_EVENT_ID + "=" + eventId, null, null);
        if (cAttendee != null) {
            while (cAttendee.moveToNext()) {
                Attendee attendee = new Attendee();
                attendee = AttendeeContent.cursorToAttendee(cAttendee);
                if (event != null) {
                    event.addAttendee(attendee);
                }

            }
            cAttendee.close(); // Yu
        } else {
            Debugger.logW(new Object[] { eventId },
                    "Do not find the attendee. ");
        }

        return event;
    }

    /**
     * @param eventId
     *            The event of the id.
     * @return The number of deleted rows if succeeded or a negative number if
     *         failed.
     */
    public int deleteEvent(final long eventId) {

        int result = 0;

        result = getObservedContentResolver().delete(
                CalendarEventContent.CONTENT_URI,
                CalendarEventContent.COLUMN_ID + "=" + eventId, null);

        if (result < 0) {
            Debugger.logE(new Object[] { eventId },
                    "Failed to delete Event, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(new Object[] { eventId },
                    "Deleted several Events in one time, "
                            + "please check if it is normal.");
        }

        final int deleteRemindersCount = deleteRemindersByEventId(eventId);
        Debugger.logV(new Object[] { eventId }, "Delete Reminders Count is "
                + deleteRemindersCount);
        final int deleteAttendeesCount = deleteAttendeesByEventId(eventId, null);
        Debugger.logV(new Object[] { eventId }, "Delete Attendees Count is "
                + deleteAttendeesCount);

        return result;
    }

    /**
     * @param eventIds
     *            The array of the event id.
     * @return A array of boolean for results or null. the element of array if
     *         the event deleted succeeded is true, or is false.
     */
    public boolean[] deleteEvents(final long[] eventIds) {
        if (null == eventIds) {
            Debugger.logE(new Object[] { eventIds }, "EventIds is null.");
            return null;
        }
        final int length = eventIds.length;
        boolean[] results = new boolean[length];
        // for (int i = 0; i < length; i++) {
        // int count = deleteEvent(eventIds[i]);
        // if (count >= 1) {
        // results[i] = true;
        // }
        // }
        if (length == 1) { // Yu
            for (int i = 0; i < length; i++) {
                final int count = deleteEvent(eventIds[i]);
                if (count >= 1) {
                    results[i] = true;
                }
            }
        } else if (length > 1) {
            Debugger.logI(new Object[] { eventIds },
                    ">>>>Delete all event start");

            getObservedContentResolver().delete(
                    CalendarEventContent.CONTENT_URI, null, null);
            getObservedContentResolver().delete(ReminderContent.CONTENT_URI,
                    null, null);
            getObservedContentResolver().delete(AttendeeContent.CONTENT_URI,
                    null, null);
            Debugger
                    .logI(new Object[] { eventIds }, ">>>>Delete all event end");
            results = null;
        }

        return results;
    }

    /**
     * Delete all info about calendar.
     */
    public void deleteAll() {
        // getObservedContentResolver().delete(CalendarEventContent.CONTENT_URI,
        // CalendarEventContent.COLUMN_ID + " != -1" , null); // Modify by Yu
        // for ICS 2.11-12-16
        // getObservedContentResolver().delete(ReminderContent.CONTENT_URI,
        // CalendarEventContent.COLUMN_ID + " != -1" , null);
        // getObservedContentResolver().delete(AttendeeContent.CONTENT_URI,
        // CalendarEventContent.COLUMN_ID + " != -1" , null);

        getObservedContentResolver().delete(CalendarEventContent.CONTENT_URI,
                " 1 = 1 ", null); // Modify by Yu for ICS 2.11-12-16
        getObservedContentResolver().delete(ReminderContent.CONTENT_URI,
                " 1 = 1 ", null);
        getObservedContentResolver().delete(AttendeeContent.CONTENT_URI,
                " 1 = 1 ", null);
        Debugger.logI(new Object[] {}, ">>>>Delete all end");

    }

    /**
     * @param eventId
     *            The id of the event.
     * @return The number of rows deleted if succeeded or a negative number if
     *         failed.
     */
    public int deleteRemindersByEventId(final long eventId) {
        int result = 0;

        result = getObservedContentResolver().delete(
                ReminderContent.CONTENT_URI,
                ReminderContent.COLUMN_EVENT_ID + "=" + eventId, null);

        if (result < 0) {
            Debugger.logE(new Object[] { eventId },
                    "Failed to delete Reminders, result is " + result);
        }

        return result;
    }

    /**
     * @param eventId
     *            The id of the event.
     * @param organizer
     *            The organizer of the event.
     * @return The number of deleted row if succeeded, or a negative number if
     *         failed.
     */
    public int deleteAttendeesByEventId(final long eventId,
            final String organizer) {
        int result = 0;

        if (null != organizer) {
            result = getObservedContentResolver().delete(
                    AttendeeContent.CONTENT_URI,
                    AttendeeContent.COLUMN_EVENT_ID + "=" + eventId + " AND "
                            + AttendeeContent.COLUMN_ATTENDEE_EMAIL + " <> '"
                            + organizer + "'", null);
        } else {
            result = getObservedContentResolver().delete(
                    AttendeeContent.CONTENT_URI,
                    AttendeeContent.COLUMN_EVENT_ID + "=" + eventId, null);
        }

        if (result < 0) {
            Debugger.logE(new Object[] { eventId },
                    "Failed to delete Attendee, result is " + result);
        }
        return result;
    }

    /**
     * @param reminderId
     *            The id of the reminder.
     * @return The number of deleted row if succeeded, or a negative number if
     *         failed.
     */
    public int deleteReminder(final long reminderId) {

        int result = 0;

        result = getObservedContentResolver().delete(
                ReminderContent.CONTENT_URI,
                ReminderContent.COLUMN_ID + "=" + reminderId, null);

        if (result < 0) {
            Debugger.logE(new Object[] { reminderId },
                    "Failed to delete Reminder, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(new Object[] { reminderId },
                    "Deleted several Reminders in one time, "
                            + "please check if it is normal.");
        }

        return result;
    }

    /**
     * @param attendeeId
     *            The id of the attendee.
     * @return The number of the deleted row if succeeded, or a negative number
     *         if failed.
     */
    public int deleteAttendee(final long attendeeId) {

        int result = 0;

        result = getObservedContentResolver().delete(
                AttendeeContent.CONTENT_URI,
                AttendeeContent.COLUMN_ID + "=" + attendeeId, null);

        if (result < 0) {
            Debugger.logE(new Object[] { attendeeId },
                    "Failed to delete Attendee, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(new Object[] { attendeeId },
                    "Deleted several Attendees in one time, "
                            + "please check if it is normal.");
        }

        return result;
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of the calendar.
     */
    public void getCalendars(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;

        try {
            // Query all Calendars
            c = getContentResolver().query(
                    CalendarContent.CONTENT_URI,
                    new String[] { CalendarContent.COLUMN_ID,
                            CalendarContent.COLUMN_NAME,
                            CalendarContent.COLUMN_DISPLAY_NAME,
                            CalendarContent.COLUMN_OWNER_ACCOUNT }, null, null,
                    CalendarContent.COLUMN_ID + " ASC");

            final FastCalendarCursorParser parser = new FastCalendarCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of the event.
     */
    public void getEvents(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;

        try {
            // Query all Events
            c = getContentResolver().query(
                    CalendarEventContent.CONTENT_URI,
                    new String[] { CalendarEventContent.COLUMN_ID,
                            CalendarEventContent.CALENDAR_ID,
                            CalendarEventContent.TITLE,
                            CalendarEventContent.DTSTART,
                            CalendarEventContent.DTEND,
                            CalendarEventContent.ALL_DAY,
                            CalendarEventContent.EVENT_LOCATION,
                            CalendarEventContent.DESCRIPTION,
                            CalendarEventContent.ORGANIZER,
                            CalendarEventContent.RRULE,
                            CalendarEventContent.MODIFY_TIME,
                            CalendarEventContent.CREATE_TIME,
                            CalendarEventContent.DURATION,
                            CalendarEventContent.EVENT_TIMEZONE,
                            CalendarEventContent.AVAILABILITY,
                            CalendarEventContent.ACCESS_LEVEL },
                    CalendarEventContent.DELETED + "<>"
                            + DatabaseRecordEntity.TRUE, null,
                    CalendarEventContent.COLUMN_ID + " ASC");

            final FastEventCursorParser parser = new FastEventCursorParser(c,
                    consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the reminder.
     */
    public void getReminders(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;

        try {
            // Query all Reminders
            c = getContentResolver().query(
                    ReminderContent.CONTENT_URI,
                    new String[] { ReminderContent.COLUMN_ID,
                            ReminderContent.COLUMN_EVENT_ID,
                            ReminderContent.COLUMN_METHOD,
                            ReminderContent.COLUMN_MINUTES }, null, null,
                    ReminderContent.COLUMN_MINUTES + " ASC");

            final FastReminderCursorParser parser = new FastReminderCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            the buffer to save the attendee.
     */
    public void getAttendees(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;

        try {
            // Query all Reminders
            c = getContentResolver().query(
                    AttendeeContent.CONTENT_URI,
                    new String[] { AttendeeContent.COLUMN_ID,
                            AttendeeContent.COLUMN_EVENT_ID,
                            AttendeeContent.COLUMN_ATTENDEE_NAME,
                            AttendeeContent.COLUMN_ATTENDEE_EMAIL,
                            AttendeeContent.COLUMN_ATTENDEE_STATUS,
                            AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP,
                            AttendeeContent.COLUMN_ATTENDEE_TYPE }, null, null,
                    AttendeeContent.COLUMN_ID + " ASC");

            final FastAttendeeCursorParser parser = new FastAttendeeCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param id
     *            The id of the event to be deleted.
     * @param newEvent
     *            The event used to update.
     * @return The number of the deleted row if succeeded or a negative number
     *         if failed.
     */
    public int updateEvent(final long id, final CalendarEvent newEvent) {
        if (newEvent == null) {
            Debugger.logW(new Object[] { id, newEvent },
                    "New event passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(16);
        values.put(CalendarEventContent.CALENDAR_ID, newEvent.getCalendarId());
        values.put(CalendarEventContent.TITLE, newEvent.getTitle());
        values.put(CalendarEventContent.DTSTART, newEvent.getTimeFrom());
        // values.put(CalendarEventContent.DTEND, newEvent.getTimeTo());
        values.put(CalendarEventContent.ALL_DAY, newEvent.isAllDay() ? 1 : 0);
        values.put(CalendarEventContent.EVENT_LOCATION, newEvent
                .getEventLocation());
        values.put(CalendarEventContent.DESCRIPTION, newEvent.getDescription());
        values.put(CalendarEventContent.ORGANIZER, newEvent.getCalendarOwner());
        values.put(CalendarEventContent.RRULE, newEvent.getRepetition());
        values.put(CalendarEventContent.MODIFY_TIME, newEvent.getModifyTime()); // TODO
        values.put(CalendarEventContent.CREATE_TIME, newEvent.getCreateTime()); // TODO
        // if(null != newEvent.getDuration()) // Added by Yu for ICS
        values.put(CalendarEventContent.DURATION, newEvent.getDuration());
        // DEND: Added by Jinbo 2011-12-15
        if (newEvent.getDuration() != null) { // duration != null
            values.put(CalendarEventContent.DTEND, (Long) null);
        } else { // duration == null
            if ((Long) newEvent.getTimeTo() == null) {
                values.put(CalendarEventContent.DTEND, 0);
            } else {
                values.put(CalendarEventContent.DTEND, newEvent.getTimeTo());
            }
        }

        values.put(CalendarEventContent.EVENT_TIMEZONE, newEvent.getTimeZone());
        // Modify by Yu 2011-12-11
        values.put(CalendarEventContent.AVAILABILITY, newEvent
                .getAvailability());
        values
                .put(CalendarEventContent.ACCESS_LEVEL, newEvent
                        .getAccessLevel());
        values.put(CalendarEventContent.GUESTS_CAN_MODIFY,
                DatabaseRecordEntity.TRUE);

        deleteAttendeesByEventId(id, newEvent.getCalendarOwner());
        final List<Attendee> attendeeList = newEvent.getAttendees();
        if (attendeeList != null) {
            if (attendeeList.size() > 0) {
                for (final Attendee attendee : attendeeList) {
                    if (!attendee.getAttendeeEmail().equals(
                            newEvent.getCalendarOwner())) {
                        insertAttendee(attendee, id);
                    } else {
                        Debugger
                                .logI("The calender owner will not be inserted, "
                                        + "because it is not deleted.");
                    }
                }
            } else {
                Debugger.logI("attendeeList size is 0.");
            }
        } else {
            Debugger.logI("attendeeList is null.");
        }
        deleteRemindersByEventId(id);
        final List<Reminder> reminderList = newEvent.getReminders();
        if (reminderList != null) {
            if (reminderList.size() == 1) {
                for (final Reminder reminder : reminderList) {
                    insertReminder(reminder, id);
                }
            } else {
                Debugger.logI("attendeeList size is not 1.");
            }
        } else {
            Debugger.logI("reminderList is null");
        }
        // Comment by Jinbo 2011-12-15
        /*
         * if (0 == newEvent.getTimeTo()) {
         * values.remove(CalendarEventContent.DTEND);
         * Debugger.logW("Dtend should not be 0. The timeTo value has been removed."
         * ); }
         */

        final Uri uri = ContentUris.withAppendedId(
                CalendarEventContent.CONTENT_URI, id);
        updateCount = getObservedContentResolver().update(uri, values, null,
                null);

        if (updateCount < 0) {
            Debugger.logE("Failed to update event, eventId is " + id
                    + ", updateCount is " + updateCount);
        } else if (updateCount > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW("Updated several event in one time, "
                    + "please check if it is normal.");
        }

        return updateCount;
    }

    /**
     * @param id
     *            The id of the attendee to be updated.
     * @param attendee
     *            The attendee used to update.
     * @return The number of updated row if succeeded, or a negative number if
     *         failed.
     */
    public int updateAttendee(final long id, final Attendee attendee) {
        if (attendee == null) {
            Debugger.logW(new Object[] { id, attendee },
                    "New attendee passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(6);
        values.put(AttendeeContent.COLUMN_EVENT_ID, attendee.getEventId());
        values.put(AttendeeContent.COLUMN_ATTENDEE_NAME, attendee
                .getAttendeeName());
        values.put(AttendeeContent.COLUMN_ATTENDEE_EMAIL, attendee
                .getAttendeeEmail());
        values.put(AttendeeContent.COLUMN_ATTENDEE_STATUS, attendee
                .getAttendeeStatus());
        values.put(AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP, attendee
                .getAttendeeRelationShip());
        values.put(AttendeeContent.COLUMN_ATTENDEE_TYPE, attendee
                .getAttendeeType());

        updateCount = getObservedContentResolver().update(
                AttendeeContent.CONTENT_URI, values,
                AttendeeContent.COLUMN_ID + "=" + id, null);

        return updateCount;
    }

    /**
     * @param id
     *            The id of the reminder to be updated.
     * @param reminder
     *            The reminder used to update.
     * @return The number of updated row if succeeded, or a negative number if
     *         failed.
     */
    public int updateReminder(final long id, final Reminder reminder) {
        if (reminder == null) {
            Debugger.logW(new Object[] { id, reminder },
                    "New reminder passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(3);
        values.put(ReminderContent.COLUMN_EVENT_ID, reminder.getEventId());
        values.put(ReminderContent.COLUMN_METHOD, reminder.getMethod());
        values.put(ReminderContent.COLUMN_MINUTES, reminder.getMinutes());

        updateCount = getObservedContentResolver().update(
                ReminderContent.CONTENT_URI, values,
                ReminderContent.COLUMN_ID + "=" + id, null);

        return updateCount;
    }

    /**
     * @return The max event id.
     */
    public long getMaxEventId() {
        long maxId = 0L;
        Cursor c;
        c = getContentResolver().query(CalendarEventContent.CONTENT_URI,
                new String[] { CalendarEventContent.COLUMN_ID }, null, null,
                CalendarEventContent.COLUMN_ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        Debugger.logI("getMaxEventId: " + maxId);
        return maxId;
    }

    private long getMaxEventIdFromInsert() {
        long insertedId = 0l;
        ContentValues values = new ContentValues(7);
        values.put(CalendarEventContent.CALENDAR_ID, 1l);
        values.put(CalendarEventContent.TITLE, "Test");
        values.put(CalendarEventContent.DTSTART, 1l);
        values.put(CalendarEventContent.ALL_DAY, 1);
        values.put(CalendarEventContent.DTEND, 1);
        values.put(CalendarEventContent.EVENT_TIMEZONE, "1");        
        values.put(CalendarEventContent.ORGANIZER, "Test");
//        values.put(CalendarEventContent.RRULE, "Test");
        Uri uri = getObservedContentResolver().insert(CalendarEventContent.CONTENT_URI, values);
        if (uri != null) {
            insertedId = Long.parseLong(uri.getLastPathSegment());
            getObservedContentResolver().delete(CalendarEventContent.CONTENT_URI,
                    CalendarEventContent.COLUMN_ID + "=" + insertedId, null);
            Debugger.logI("getMaxEventIdFromInsert: " + insertedId);
        }
        return insertedId;
    }

    // Sync --------------------------------------------------------------------
    /**
     * @return Whether to re-initial.
     */
    public boolean isSyncNeedReinit() {
        boolean syncNeedReinit = true;
        try {
            syncNeedReinit = SharedPrefs.open(getContext()).getBoolean(
                    SharedPrefs.CALENDAR_SYNC_NEED_REINIT, true);
        } catch (final ClassCastException e) {
            Debugger.logE(e);
        }
        return syncNeedReinit;
    }

    /**
     * @return The date of the last synchronization.
     */
    public long getLastSyncDate() {
        long lastSyncDate = 0L;
        try {
            lastSyncDate = SharedPrefs.open(getContext()).getLong(
                    SharedPrefs.CALENDAR_LAST_SYNC_DATE, 0L);
        } catch (final ClassCastException e) {
            Debugger.logE(e);
        }
        return lastSyncDate;
    }

    /**
     * @param eventIdTo
     *            The id of the event.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of the event.
     */
    public void slowSyncGetAllEvents(final long eventIdTo,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { eventIdTo, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    CalendarEventContent.CONTENT_URI,
                    new String[] { CalendarEventContent.COLUMN_ID,
                            CalendarEventContent.CALENDAR_ID,
                            CalendarEventContent.TITLE,
                            CalendarEventContent.DTSTART,
                            CalendarEventContent.DTEND,
                            CalendarEventContent.ALL_DAY,
                            CalendarEventContent.EVENT_LOCATION,
                            CalendarEventContent.DESCRIPTION,
                            CalendarEventContent.ORGANIZER,
                            CalendarEventContent.RRULE,
                            CalendarEventContent.CREATE_TIME,
                            CalendarEventContent.MODIFY_TIME,
                            CalendarEventContent.DURATION,
                            CalendarEventContent.EVENT_TIMEZONE,
                            // Modify by Yu 2011-12-11
                            CalendarEventContent.AVAILABILITY,
                            CalendarEventContent.ACCESS_LEVEL },
                    CalendarEventContent.COLUMN_ID + "<=" + eventIdTo + " AND "
                            + CalendarEventContent.DELETED + "<>"
                            + DatabaseRecordEntity.TRUE, null, null);

            final FastEventCursorParser parser = new FastEventCursorParser(c,
                    consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param eventIdTo
     *            The id of the event.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of the reminder.
     */
    public void slowSyncGetAllReminders(final long eventIdTo,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { eventIdTo, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    ReminderContent.CONTENT_URI,
                    new String[] { ReminderContent.COLUMN_ID,
                            ReminderContent.COLUMN_EVENT_ID,
                            ReminderContent.COLUMN_METHOD,
                            ReminderContent.COLUMN_MINUTES },
                    ReminderContent.COLUMN_EVENT_ID + "<=" + eventIdTo, null,
                    null);

            final FastReminderCursorParser parser = new FastReminderCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param eventIdTo
     *            the if of the event.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of the attendee.
     */
    public void slowSyncGetAllAttendees(final long eventIdTo,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { eventIdTo, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    AttendeeContent.CONTENT_URI,
                    new String[] { AttendeeContent.COLUMN_EVENT_ID,
                            AttendeeContent.COLUMN_ATTENDEE_NAME,
                            AttendeeContent.COLUMN_ATTENDEE_EMAIL,
                            AttendeeContent.COLUMN_ATTENDEE_STATUS,
                            AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP,
                            AttendeeContent.COLUMN_ATTENDEE_TYPE },
                    AttendeeContent.COLUMN_EVENT_ID + "<=" + eventIdTo, null,
                    null);

            final FastAttendeeCursorParser parser = new FastAttendeeCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    // Faster, but not safe
    /**
     * @param raw
     *            The raw bytes of the event.
     * @return A array byte representing the result of the synchronization.
     */
    public byte[] slowSyncAddEvents(final byte[] raw) {
        if (null == raw) {
            Debugger.logE(new Object[] { raw }, "Raw data is null.");
            return null;
        }

        final ByteBuffer buffer = ByteBuffer.wrap(raw);
        // Events count in the raw data
        int count;
        long beginId;
        try {
            // The first 4 bytes tell events count in the raw data.
            count = buffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw },
                    "Can not get the events count in raw data ", e);
            return null;
        }

        if (count < 0) {
            Debugger
                    .logE(new Object[] { raw }, "Invalid events count " + count);
            return null;
        }

        beginId = getMaxEventId() + 1;
        if (beginId == 1) {
            beginId = getMaxEventIdFromInsert() + 1;
        }
        final DefaultBulkInsertHelper eventInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(CalendarEventContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "slowSyncAddEvents",
                            new Object[] { raw },
                            "Bulk insert events failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        final DefaultBulkInsertHelper attendeeInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(AttendeeContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "slowSyncAddEvents",
                            new Object[] { raw },
                            "Bulk insert attendees failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        final DefaultBulkInsertHelper reminderInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(ReminderContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "slowSyncAddEvents",
                            new Object[] { raw },
                            "Bulk insert reminders failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        boolean shouldExit = false;
        // Insert event -------------------------------------------------
        for (int i = 0; i < count; i++) {
            // Read a event from raw bytes
            final CalendarEvent newEvent = new CalendarEvent();
            newEvent.readRawWithVersion(buffer, Config.VERSION_CODE);
            final MeasuredContentValues values = new MeasuredContentValues(19);
            values.put(CalendarEventContent.CALENDAR_ID, newEvent
                    .getCalendarId());
            values.put(CalendarEventContent.TITLE, newEvent.getTitle());
            values.put(CalendarEventContent.DTSTART, newEvent.getTimeFrom());
            // values.put(CalendarEventContent.DTEND, newEvent.getTimeTo());
            values.put(CalendarEventContent.ALL_DAY, newEvent.isAllDay() ? 1
                    : 0);
            values.put(CalendarEventContent.EVENT_LOCATION, newEvent
                    .getEventLocation());
            values.put(CalendarEventContent.DESCRIPTION, newEvent
                    .getDescription());
            values.put(CalendarEventContent.ORGANIZER, newEvent
                    .getCalendarOwner());
            values.put(CalendarEventContent.RRULE, newEvent.getRepetition());
            values.put(CalendarEventContent.MODIFY_TIME, newEvent
                    .getModifyTime());
            values.put(CalendarEventContent.CREATE_TIME, newEvent
                    .getCreateTime());
            // if(null != newEvent.getDuration())
            values.put(CalendarEventContent.DURATION, newEvent.getDuration());
            // DEND: Added by Jinbo 2011-12-15
            if (newEvent.getDuration() != null) { // duration != null
                values.put(CalendarEventContent.DTEND, (Long) null);
            } else { // duration == null
                if ((Long) newEvent.getTimeTo() == null) {
                    values.put(CalendarEventContent.DTEND, 0);
                } else {
                    values
                            .put(CalendarEventContent.DTEND, newEvent
                                    .getTimeTo());
                }
            }

            values.put(CalendarEventContent.GUESTS_CAN_MODIFY,
                    DatabaseRecordEntity.TRUE);
            // 14
            // values.put(CalendarEventContent.SELF_ATTENDEE_STATUS,
            // AttendeeContent.ATTENDEE_STATUS_ACCEPTED);
            final List<Reminder> reminderList = newEvent.getReminders();
            if (reminderList != null && reminderList.size() > 0) {
                values.put(CalendarEventContent.HAS_ALARM,
                        DatabaseRecordEntity.TRUE);
                for (final Reminder reminder : reminderList) {
                    final MeasuredContentValues reminderValues = new MeasuredContentValues(
                            3);
                    reminderValues.put(ReminderContent.COLUMN_EVENT_ID, beginId
                            + i);
                    reminderValues.put(ReminderContent.COLUMN_METHOD, reminder
                            .getMethod());
                    reminderValues.put(ReminderContent.COLUMN_MINUTES, reminder
                            .getMinutes());
                    shouldExit = !reminderInserter.append(reminderValues);
                    if (shouldExit) {
                        Debugger.logE(new Object[] { raw },
                                "Error in bulk inserting reminders, "
                                        + "statusCode: "
                                        + reminderInserter.getStatusCode());
                        return null;
                    }
                }
            } else {
                values.put(CalendarEventContent.HAS_ALARM,
                        DatabaseRecordEntity.FALSE);
            }

            values.put(CalendarEventContent.HAS_EXTENDED_PROPERTIES,
                    DatabaseRecordEntity.TRUE);
            final List<Attendee> attendeeList = newEvent.getAttendees();
            if (attendeeList != null && attendeeList.size() > 0) {
                values.put(CalendarEventContent.HAS_ATTENDEE_DATA,
                        DatabaseRecordEntity.TRUE);
                for (final Attendee attendee : attendeeList) {
                    final MeasuredContentValues attendeeValues = new MeasuredContentValues(
                            6);
                    attendeeValues.put(AttendeeContent.COLUMN_EVENT_ID, beginId
                            + i);
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_NAME,
                            attendee.getAttendeeName());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_EMAIL,
                            attendee.getAttendeeEmail());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_STATUS,
                            attendee.getAttendeeStatus());
                    attendeeValues.put(
                            AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP,
                            attendee.getAttendeeRelationShip());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_TYPE,
                            attendee.getAttendeeType());
                    shouldExit = !attendeeInserter.append(attendeeValues);
                    if (shouldExit) {
                        Debugger.logE(new Object[] { raw },
                                "Error in bulk inserting atendees, "
                                        + "statusCode: "
                                        + attendeeInserter.getStatusCode());
                        return null;
                    }
                }
            } else {
                values.put(CalendarEventContent.HAS_ATTENDEE_DATA,
                        DatabaseRecordEntity.FALSE);
            }
            values.put(CalendarEventContent.EVENT_TIMEZONE, newEvent
                    .getTimeZone());
            values.put(CalendarEventContent.AVAILABILITY, newEvent
                    .getAvailability());
            values.put(CalendarEventContent.ACCESS_LEVEL, newEvent
                    .getAccessLevel());

            shouldExit = !eventInserter.append(values);
            if (shouldExit) {
                Debugger.logE(new Object[] { raw },
                        "Error in bulk inserting events, " + "statusCode: "
                                + eventInserter.getStatusCode());
                return null;
            }
        }
        // Bulk insert all event ContentValues left
        shouldExit = !eventInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw },
                    "Error in bulk inserting events, " + "statusCode: "
                            + eventInserter.getStatusCode());
            return null;
        }
        // Bulk insert all reminder ContentValues left
        shouldExit = !reminderInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw },
                    "Error in bulk inserting reminders, " + "statusCode: "
                            + reminderInserter.getStatusCode());
            return null;
        }
        // Bulk insert all attendee ContentValues left
        shouldExit = !attendeeInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw },
                    "Error in bulk inserting attendees, " + "statusCode: "
                            + attendeeInserter.getStatusCode());
            return null;
        }

        // 2. Get needed info and set sync flags with them
        return getSyncFlags(beginId, beginId + count);
    }

    /**
     * @param eventList
     *            The list of event to be inserted.
     * @return The number of the inserted events.
     */
    public long insertAllEvents(final List<CalendarEvent> eventList) {
        if (null == eventList) {
            Debugger.logE(new Object[] { eventList }, "eventList is null.");
            return DatabaseRecordEntity.ID_NULL;
        }
        final int count = eventList.size();
        long beginId;

        beginId = getMaxEventId() + 1;
        final DefaultBulkInsertHelper eventInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(CalendarEventContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "InsertAllEvents",
                            new Object[] {},
                            "Bulk insert events failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        final DefaultBulkInsertHelper attendeeInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(AttendeeContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "InsertAllEvents",
                            new Object[] {},
                            "Bulk insert attendees failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        final DefaultBulkInsertHelper reminderInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(ReminderContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "InsertAllEvents",
                            new Object[] {},
                            "Bulk insert reminders failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        boolean shouldExit = false;
        // Insert event -------------------------------------------------
        for (int i = 0; i < count; i++) {
            Debugger.logI(new Object[] {}, "----->count is " + count);
            final CalendarEvent newEvent = eventList.get(i);
            final MeasuredContentValues values = new MeasuredContentValues(20);
            values.put(CalendarEventContent.COLUMN_ID, newEvent.getId());
            Debugger.logI(new Object[] {}, "newEvent.getId() "
                    + newEvent.getId());
            values.put(CalendarEventContent.CALENDAR_ID, newEvent
                    .getCalendarId());
            values.put(CalendarEventContent.TITLE, newEvent.getTitle());
            values.put(CalendarEventContent.DTSTART, newEvent.getTimeFrom());
            // values.put(CalendarEventContent.DTEND, newEvent.getTimeTo());
            values.put(CalendarEventContent.ALL_DAY, newEvent.isAllDay() ? 1
                    : 0);
            values.put(CalendarEventContent.EVENT_LOCATION, newEvent
                    .getEventLocation());
            values.put(CalendarEventContent.DESCRIPTION, newEvent
                    .getDescription());
            values.put(CalendarEventContent.ORGANIZER, newEvent
                    .getCalendarOwner());
            values.put(CalendarEventContent.RRULE, newEvent.getRepetition());
            values.put(CalendarEventContent.MODIFY_TIME, newEvent
                    .getModifyTime());
            values.put(CalendarEventContent.CREATE_TIME, newEvent
                    .getCreateTime());
            // if(null != newEvent.getDuration()) // Added by Yu for ICS
            values.put(CalendarEventContent.DURATION, newEvent.getDuration());

            // DEND: Added by Jinbo 2011-12-15
            if (newEvent.getDuration() != null) { // duration != null
                values.put(CalendarEventContent.DTEND, (Long) null);
            } else { // duration == null
                if ((Long) newEvent.getTimeTo() == null) {
                    values.put(CalendarEventContent.DTEND, 0);
                } else {
                    values
                            .put(CalendarEventContent.DTEND, newEvent
                                    .getTimeTo());
                }
            }

            values.put(CalendarEventContent.GUESTS_CAN_MODIFY,
                    DatabaseRecordEntity.TRUE);
            // 14
            // values.put(CalendarEventContent.SELF_ATTENDEE_STATUS,
            // AttendeeContent.ATTENDEE_STATUS_ACCEPTED);
            final List<Reminder> reminderList = newEvent.getReminders();
            if (reminderList != null && reminderList.size() > 0) {
                values.put(CalendarEventContent.HAS_ALARM,
                        DatabaseRecordEntity.TRUE);
                for (final Reminder reminder : reminderList) {
                    final MeasuredContentValues reminderValues = new MeasuredContentValues(
                            3);
                    // reminderValues.put(ReminderContent.COLUMN_EVENT_ID,
                    // (beginId + i));
                    reminderValues.put(ReminderContent.COLUMN_EVENT_ID,
                            newEvent.getId());
                    Debugger.logI(new Object[] {},
                            "----->Reminder.COLUMN_EVENT_ID is "
                                    + newEvent.getId());
                    reminderValues.put(ReminderContent.COLUMN_METHOD, reminder
                            .getMethod());
                    reminderValues.put(ReminderContent.COLUMN_MINUTES, reminder
                            .getMinutes());
                    shouldExit = !reminderInserter.append(reminderValues);
                    if (shouldExit) {
                        Debugger.logE(new Object[] {},
                                "Error in bulk inserting reminders, "
                                        + "statusCode: "
                                        + reminderInserter.getStatusCode());
                        return DatabaseRecordEntity.FALSE;
                    }
                }
            } else {
                values.put(CalendarEventContent.HAS_ALARM,
                        DatabaseRecordEntity.FALSE);
            }

            values.put(CalendarEventContent.HAS_EXTENDED_PROPERTIES,
                    DatabaseRecordEntity.TRUE);
            final List<Attendee> attendeeList = newEvent.getAttendees();
            if (attendeeList != null && attendeeList.size() > 0) {
                values.put(CalendarEventContent.HAS_ATTENDEE_DATA,
                        DatabaseRecordEntity.TRUE);
                for (final Attendee attendee : attendeeList) {
                    final MeasuredContentValues attendeeValues = new MeasuredContentValues(
                            6);
                    // attendeeValues.put(AttendeeContent.COLUMN_EVENT_ID,
                    // beginId + i);
                    attendeeValues.put(AttendeeContent.COLUMN_EVENT_ID,
                            newEvent.getId());
                    Debugger.logI(new Object[] {},
                            AttendeeContent.COLUMN_EVENT_ID + " = "
                                    + newEvent.getId());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_NAME,
                            attendee.getAttendeeName());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_EMAIL,
                            attendee.getAttendeeEmail());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_STATUS,
                            attendee.getAttendeeStatus());
                    attendeeValues.put(
                            AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP,
                            attendee.getAttendeeRelationShip());
                    attendeeValues.put(AttendeeContent.COLUMN_ATTENDEE_TYPE,
                            attendee.getAttendeeType());
                    shouldExit = !attendeeInserter.append(attendeeValues);
                    if (shouldExit) {
                        Debugger.logE(new Object[] {},
                                "Error in bulk inserting atendees, "
                                        + "statusCode: "
                                        + attendeeInserter.getStatusCode());
                        return DatabaseRecordEntity.FALSE;
                    }
                }
            } else {
                values.put(CalendarEventContent.HAS_ATTENDEE_DATA,
                        DatabaseRecordEntity.FALSE);
            }
            values.put(CalendarEventContent.EVENT_TIMEZONE, newEvent
                    .getTimeZone());
            values.put(CalendarEventContent.AVAILABILITY, newEvent
                    .getAvailability());
            values.put(CalendarEventContent.ACCESS_LEVEL, newEvent
                    .getAccessLevel());

            shouldExit = !eventInserter.append(values);
            if (shouldExit) {
                Debugger.logE(new Object[] {},
                        "Error in bulk inserting events, " + "statusCode: "
                                + eventInserter.getStatusCode());
                return DatabaseRecordEntity.FALSE;
            }
        }
        // Bulk insert all event ContentValues left
        shouldExit = !eventInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] {}, "Error in bulk inserting events, "
                    + "statusCode: " + eventInserter.getStatusCode());
            return DatabaseRecordEntity.FALSE;
        }
        // Bulk insert all reminder ContentValues left
        shouldExit = !reminderInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] {},
                    "Error in bulk inserting reminders, " + "statusCode: "
                            + reminderInserter.getStatusCode());
            return DatabaseRecordEntity.FALSE;
        }
        // Bulk insert all attendee ContentValues left
        shouldExit = !attendeeInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] {},
                    "Error in bulk inserting attendees, " + "statusCode: "
                            + attendeeInserter.getStatusCode());
            return DatabaseRecordEntity.FALSE;
        }
        return getMaxEventId() - beginId + 1;
    }

    /**
     * @return The id of the local account.
     */
    public long getLocalAccountId() {
        long localAccountId = 0L;
        Cursor c;

        c = getContentResolver().query(
                CalendarContent.CONTENT_URI,
                new String[] { CalendarContent.COLUMN_ID },
                CalendarContent.COLUMN_SYNC_ACCOUNT_TYPE + " = 'local'"
                        + " or " + CalendarContent.COLUMN_SYNC_ACCOUNT_TYPE
                        + " = 'LOCAL'", null, null);
        if (null != c) {
            if (c.moveToNext()) {
                localAccountId = c.getLong(0);
            }
            c.close();
        }
        return localAccountId;
    }

    /**
     * @return Whether it is able to synchronization.
     */
    public boolean isSyncAble() {
        int syncEvents = -1;
        Cursor c;

        c = getContentResolver().query(
                CalendarContent.CONTENT_URI,
                new String[] { CalendarContent.COLUMN_SYNC_EVENTS },
                CalendarContent.COLUMN_SYNC_ACCOUNT_TYPE + " = 'local'"
                        + " or " + CalendarContent.COLUMN_SYNC_ACCOUNT_TYPE
                        + " = 'LOCAL'", null, null);
        if (null != c) {
            if (c.moveToNext()) {
                syncEvents = c.getInt(0);
            }
            c.close();
        }
        return syncEvents == 1;
    }

    /**
     * 
     * @param idFrom
     *            The started id.
     * @param idTo
     *            The ended id.
     * @return A array of byte representing result of synchronization.
     */
    public byte[] getSyncFlags(final long idFrom, final long idTo) {
        final ByteBuffer buffer = Global.getByteBuffer();
        // Sync flags count is 0 by default
        byte[] syncResultsRaw = new byte[4];
        Cursor c;

        c = getContentResolver().query(
                CalendarEventContent.CONTENT_URI,
                new String[] { CalendarEventContent.COLUMN_ID,
                        CalendarEventContent.MODIFY_TIME,
                        CalendarEventContent.CALENDAR_ID,
                        CalendarEventContent.TITLE,
                        CalendarEventContent.EVENT_TIMEZONE,
                        CalendarEventContent.DTSTART },
                CalendarEventContent.COLUMN_ID + ">=" + idFrom + " AND "
                        + CalendarEventContent.COLUMN_ID + "<=" + idTo
                        + " AND " + CalendarEventContent.DELETED + "<>"
                        + DatabaseRecordEntity.TRUE, null,
                CalendarEventContent.COLUMN_ID + " ASC");

        buffer.clear();
        if (null != c) {
            buffer.putInt(c.getCount());
            Debugger.logI(new Object[] { idFrom, idTo}, "getSyncFlags count: " + c.getCount());
            while (c.moveToNext()) {
                // _id
                buffer.putLong(c.getLong(0));
                // modifyTime
                buffer.putLong(c.getLong(1));
                // calendarId
                buffer.putLong(c.getLong(2));
                // title
                RawTransUtil.putString(buffer, c.getString(3));
                // timeZone
                RawTransUtil.putString(buffer, c.getString(4));
                // timeFrom
                buffer.putLong(c.getLong(5));
            }
            c.close();
        } else {
            buffer.putInt(0);
            Debugger.logE(new Object[] { idFrom, idTo}, "cursor is null");
        }
        buffer.flip();
        syncResultsRaw = new byte[buffer.limit()];
        buffer.get(syncResultsRaw);

        return syncResultsRaw;
    }

    /**
     * 
     * @param idSet
     *            <b>MUST</b> be in ascending order.
     * @return Raw bytes for result of synchronization.
     */
    public byte[] getSyncFlags(final long[] idSet) {
        if (null == idSet) {
            Debugger.logE(new Object[] { idSet }, "Target ID list is null.");
            // Sync flags count is 0 by default
            return new byte[4];
        }
        if (idSet.length <= 0) {
            Debugger.logE(new Object[] { idSet }, "Target ID list is empty.");
            // Sync flags count is 0 by default
            return new byte[4];
        }

        // Build selection
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append("(");
        strBuf.append(CalendarEventContent.COLUMN_ID + " IN(");
        for (int i = 0; i < idSet.length; i++) {
            strBuf.append(idSet[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")) AND " + CalendarEventContent.DELETED + "<>"
                + DatabaseRecordEntity.TRUE);
        selection = strBuf.toString();

        final ByteBuffer buffer = Global.getByteBuffer();
        // Sync flags count is 0 by default
        byte[] syncResultsRaw = new byte[4];
        Cursor c;

        c = getContentResolver().query(
                CalendarEventContent.CONTENT_URI,
                new String[] { CalendarEventContent.COLUMN_ID,
                        CalendarEventContent.MODIFY_TIME,
                        CalendarEventContent.CALENDAR_ID,
                        CalendarEventContent.TITLE,
                        CalendarEventContent.EVENT_TIMEZONE,
                        CalendarEventContent.DTSTART }, selection, null,
                CalendarEventContent.COLUMN_ID + " ASC");
        // c = getContentResolver().query(
        // CalendarEventContent.CONTENT_URI,
        // new String[] { CalendarEventContent.COLUMN_ID,
        // CalendarEventContent.COLUMN_MODIFY_TIME,
        // CalendarEventContent.COLUMN_CALENDAR_ID,
        // CalendarEventContent.COLUMN_TITLE,
        // CalendarEventContent.COLUMN_EVENT_TIMEZONE,
        // CalendarEventContent.COLUMN_DTSTART },
        // CalendarEventContent.COLUMN_ID + " = " + idSet[0],
        // null, CalendarEventContent.COLUMN_ID + " ASC");

        buffer.clear();
        buffer.putInt(idSet.length);
        int index = 0;
        if (null != c) {
            outer: while (c.moveToNext()) {
                final long id = c.getLong(0);
                for (; index <= idSet.length; index++) {
                    if (index == idSet.length) {
                        break outer;
                    }

                    if (0 >= idSet[index]) {
                        // id
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        // Modify time.
                        buffer.putLong(-1);
                        // Calendar Id
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        // title
                        RawTransUtil.putString(buffer, null);
                        // timeZone
                        RawTransUtil.putString(buffer, null);
                        // timeFrom
                        buffer.putLong(-1);
                        continue;
                    } else if (id == idSet[index]) {
                        // id
                        buffer.putLong(id);
                        // Modify time.
                        buffer.putLong(c.getLong(1));
                        // Calendar Id
                        buffer.putLong(c.getLong(2));
                        // title
                        RawTransUtil.putString(buffer, c.getString(3));
                        // timeZone
                        RawTransUtil.putString(buffer, c.getString(4));
                        // timeFrom
                        buffer.putLong(c.getLong(5));
                    } else if (id > idSet[index]) {
                        // id
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        // Modify time.
                        buffer.putLong(-1);
                        // Calendar Id
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        // title
                        RawTransUtil.putString(buffer, null);
                        // timeZone
                        RawTransUtil.putString(buffer, null);
                        // timeFrom
                        buffer.putLong(-1);
                        continue;
                    } else if (id < idSet[index]) {
                        break;
                    }
                }
            }
            c.close();
        }
        for (; index < idSet.length; index++) {
            // id
            buffer.putLong(DatabaseRecordEntity.ID_NULL);
            // Modify time.
            buffer.putLong(-1);
            // Calendar Id
            buffer.putLong(DatabaseRecordEntity.ID_NULL);
            // title
            RawTransUtil.putString(buffer, null);
            // timeZone
            RawTransUtil.putString(buffer, null);
            // timeFrom
            buffer.putLong(-1);
        }
        buffer.flip();
        syncResultsRaw = new byte[buffer.limit()];
        buffer.get(syncResultsRaw);

        return syncResultsRaw;
    }

    /**
     * @param raw
     *            The raw bytes of the event.
     * @return The A array byte representing the result of the synchronization.
     */
    public byte[] fastSyncAddEvents(final byte[] raw) {
        return slowSyncAddEvents(raw);
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the flags of synchronizations.
     */
    public void fastSyncGetAllSyncFlags(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    CalendarEventContent.CONTENT_URI,
                    new String[] { CalendarEventContent.COLUMN_ID,
                            CalendarEventContent.MODIFY_TIME,
                            CalendarEventContent.CALENDAR_ID,
                            CalendarEventContent.TITLE,
                            CalendarEventContent.EVENT_TIMEZONE,
                            CalendarEventContent.DTSTART },
                    CalendarEventContent.DELETED + "<>"
                            + DatabaseRecordEntity.TRUE, null, null);

            final FastCalendarSyncFlagsCursorParser parser = new FastCalendarSyncFlagsCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param requestedEventIds
     *            A array of the event's id.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content.
     */
    public void fastSyncGetEvents(final long[] requestedEventIds,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == requestedEventIds) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Requested events id list should not be null.");
            consumer.consume(null, 0, 0);
        }
        if (0 >= requestedEventIds.length) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Requested events id list is empty.");
            consumer.consume(null, 0, 0);
        }

        Cursor c = null;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // Build selection string
        strBuf.append("(" + CalendarEventContent.COLUMN_ID + " IN(");
        for (int i = 0; i < requestedEventIds.length; i++) {
            strBuf.append(requestedEventIds[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")) AND " + CalendarEventContent.DELETED + "<>"
                + DatabaseRecordEntity.TRUE);
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    CalendarEventContent.CONTENT_URI,
                    new String[] { CalendarEventContent.COLUMN_ID,
                            CalendarEventContent.CALENDAR_ID,
                            CalendarEventContent.TITLE,
                            CalendarEventContent.DTSTART,
                            CalendarEventContent.DTEND,
                            CalendarEventContent.ALL_DAY,
                            CalendarEventContent.EVENT_LOCATION,
                            CalendarEventContent.DESCRIPTION,
                            CalendarEventContent.ORGANIZER,
                            CalendarEventContent.RRULE,
                            CalendarEventContent.CREATE_TIME,
                            CalendarEventContent.MODIFY_TIME,
                            CalendarEventContent.DURATION,
                            CalendarEventContent.EVENT_TIMEZONE,
                            CalendarEventContent.AVAILABILITY,
                            CalendarEventContent.ACCESS_LEVEL }, selection,
                    null, null);

            final FastEventCursorParser parser = new FastEventCursorParser(c,
                    consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param requestedEventIds
     *            A array of the event's id.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of reminder.
     */
    public void fastSyncGetReminders(final long[] requestedEventIds,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == requestedEventIds) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Requested events id list should not be null.");
            consumer.consume(null, 0, 0);
        }
        if (0 >= requestedEventIds.length) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Requested events id list is empty.");
            consumer.consume(null, 0, 0);
        }

        Cursor c = null;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // Build selection string
        strBuf.append(ReminderContent.COLUMN_EVENT_ID + " IN(");
        for (int i = 0; i < requestedEventIds.length; i++) {
            strBuf.append(requestedEventIds[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    ReminderContent.CONTENT_URI,
                    new String[] { ReminderContent.COLUMN_ID,
                            ReminderContent.COLUMN_EVENT_ID,
                            ReminderContent.COLUMN_METHOD,
                            ReminderContent.COLUMN_MINUTES }, selection, null,
                    null);

            final FastReminderCursorParser parser = new FastReminderCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param requestedEventIds
     *            A array of the event's id.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the attendee.
     */
    public void fastSyncGetAttendees(final long[] requestedEventIds,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == requestedEventIds) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Requested events id list should not be null.");
            consumer.consume(null, 0, 0);
        }
        if (0 >= requestedEventIds.length) {
            Debugger.logE(new Object[] { requestedEventIds, consumer, buffer },
                    "Requested events id list is empty.");
            consumer.consume(null, 0, 0);
        }

        Cursor c = null;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // Build selection string
        strBuf.append(AttendeeContent.COLUMN_EVENT_ID + " IN(");
        for (int i = 0; i < requestedEventIds.length; i++) {
            strBuf.append(requestedEventIds[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    AttendeeContent.CONTENT_URI,
                    new String[] { AttendeeContent.COLUMN_EVENT_ID,
                            AttendeeContent.COLUMN_ATTENDEE_NAME,
                            AttendeeContent.COLUMN_ATTENDEE_EMAIL,
                            AttendeeContent.COLUMN_ATTENDEE_STATUS,
                            AttendeeContent.COLUMN_ATTENDEE_RELATIONSHIP,
                            AttendeeContent.COLUMN_ATTENDEE_TYPE }, selection,
                    null, null);

            final FastAttendeeCursorParser parser = new FastAttendeeCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param ids
     *            A array of the event's id.
     * @return the number of rows deleted.
     */
    public int fastDeleteEvents(final long[] ids) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids }, "List is null.");
            return 0;
        }

        int deleteCount = 0;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append(CalendarEventContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();

        final Uri deleteUri = CalendarEventContent.CONTENT_URI;

        deleteCount = getObservedContentResolver().delete(deleteUri, selection,
                null);

        return deleteCount;
    }

    /**
     * @param raw
     *            The raw bytes of event.
     * @return A array of byte representing the resulting of the updating.
     */
    public byte[] fastSyncUpdateEvents(final byte[] raw) {
        if (null == raw) {
            Debugger.logE(new Object[] { raw }, "Raw data is null.");
            return null;
        }

        final ByteBuffer buffer = ByteBuffer.wrap(raw);
        // Events count in the raw data
        int count;
        long[] updatedIds;
        try {
            // The first 4 bytes tell events count in the raw data.
            count = buffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw },
                    "Can not get the events count in raw data ", e);
            return null;
        }

        if (count < 0) {
            Debugger
                    .logE(new Object[] { raw }, "Invalid events count " + count);
            return null;
        }

        updatedIds = new long[count];
        // ContentValues eventsValues = new ContentValues(13);

        long updateId;
        Cursor c;
        for (int i = 0; i < count; i++) {
            // Read a event from raw bytes
            final CalendarEvent newEvent = new CalendarEvent();
            newEvent.readRawWithVersion(buffer, Config.VERSION_CODE);
            // Get the ID of the events to update
            updateId = newEvent.getId();
            // 1. Guarantee the events to update exist
            c = getContentResolver()
                    .query(CalendarEventContent.CONTENT_URI,
                            new String[] { CalendarEventContent.DELETED },
                            CalendarEventContent.COLUMN_ID + "=" + updateId,
                            null, null);
            if (null == c) {
                Debugger.logE(new Object[] { raw },
                        "Cursor is null. Failed to find the event " + updateId
                                + " to update.");
                return null;
            } else {
                // eventsValues.clear();
                // eventsValues.put(CalendarEventContent.COLUMN_ID, updateId);
                // eventsValues.put(CalendarEventContent.COLUMN_CALENDAR_ID,
                // newEvent.getCalendarId());
                // eventsValues.put(CalendarEventContent.COLUMN_TITLE,
                // newEvent.getTitle());
                // eventsValues.put(CalendarEventContent.COLUMN_DTSTART,
                // newEvent.getTimeFrom());
                // eventsValues.put(CalendarEventContent.COLUMN_DTEND,
                // newEvent.getTimeTo());
                // eventsValues.put(CalendarEventContent.COLUMN_ALL_DAY,
                // newEvent.isAllDay() ? 1 : 0);
                // eventsValues.put(CalendarEventContent.COLUMN_EVENT_LOCATION,
                // newEvent.getEventLocation());
                // eventsValues.put(CalendarEventContent.COLUMN_DESCRIPTION,
                // newEvent.getDescription());
                // eventsValues.put(CalendarEventContent.COLUMN_ORGANIZER,
                // newEvent.getCalendarOwner());
                // eventsValues.put(CalendarEventContent.COLUMN_RRULE,
                // newEvent.getRepetition());
                // eventsValues.put(CalendarEventContent.COLUMN_MODIFY_TIME,
                // newEvent.getModifyTime());
                // eventsValues.put(CalendarEventContent.COLUMN_CREATE_TIME,
                // newEvent.getCreateTime());
                // eventsValues.put(CalendarEventContent.COLUMN_DURATION,
                // newEvent.getDuration());
                if (c.getCount() <= 0) {
                    // Event to update does not exist, insert it
                    // getObservedContentResolver().insert(CalendarEventContent.CONTENT_URI,
                    // eventsValues);
                    insertEvent(newEvent);
                } else {
                    c.moveToFirst();
                    if (DatabaseRecordEntity.TRUE == c.getInt(0)) {
                        // Event to update is set to deleted
                        // Delete it permanently first
                        deleteEvent(updateId);
                        // insert it
                        // getObservedContentResolver().insert(CalendarEventContent.CONTENT_URI,
                        // eventsValues);
                        insertEvent(newEvent);
                    } else {
                        // update it
                        // Uri uri =
                        // ContentUris.withAppendedId(CalendarEventContent.CONTENT_URI,
                        // updateId);
                        // getObservedContentResolver().update(uri,
                        // eventsValues, null, null);

                        updateEvent(updateId, newEvent);
                    }
                }
                c.close();
            }

            // 2. Update one event finished
            updatedIds[i] = updateId;
        }

        // Get sync flags of updated events
        return getSyncFlags(updatedIds);
    }

    /**
     * @param lastSyncDate
     *            The date of the last synchronization.
     * @return Whether success to update the date of last synchronization.
     */
    public boolean updateSyncDate(final long lastSyncDate) {
        SharedPrefs.open(getContext()).edit().putLong(
                SharedPrefs.CALENDAR_LAST_SYNC_DATE, lastSyncDate).putBoolean(
                SharedPrefs.CALENDAR_SYNC_NEED_REINIT, false).commit();
        return true;
    }

    /**
     * @return The count of event to be synchronized.
     */
    public int getPcSyncEventsCount() {
        int count = 0;
        Cursor c;

        c = getContentResolver()
                .query(
                        CalendarEventContent.CONTENT_URI,
                        new String[] { CalendarEventContent.COLUMN_ID },
                        CalendarEventContent.DELETED + "<>"
                                + DatabaseRecordEntity.TRUE, null, null);
        if (c != null) {
            count = c.getCount();
            c.close();
        } else {
            count = 0;
        }

        return count;
    }

}
