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

package com.mediatek.apst.target.tests;

import android.content.Context;
import android.test.AndroidTestCase;
import android.util.Log;
import android.database.Cursor;

import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.calendar.CalendarEvent;
import com.mediatek.apst.util.entity.calendar.Attendee;
import com.mediatek.apst.util.entity.calendar.Reminder;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.data.provider.calendar.AttendeeContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarEventContent;
import com.mediatek.apst.target.data.provider.calendar.ReminderContent;

import java.nio.ByteBuffer;
import java.util.List;
import java.util.ArrayList;

public class ReminderContentTest extends AndroidTestCase {
    private static final String Tag = "ReminderContentTest";
    private Context mContext;
    private CalendarProxy mCalendarProxy;
    private Attendee mAttendee;
    private ArrayList<Attendee> mAttendeeList;
    private Reminder mReminder;
    private ArrayList<Reminder> mReminderList;
    private CalendarEvent mEvent;

    private byte[] raw = { 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
            0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 25, 0,
            0, 0, 0, 0, 0, 0, 1 };

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();

    }

    @Override
    protected void tearDown() throws Exception {

        mContext = null;
        super.tearDown();
    }

    /**
     * The cursor is null.
     */
    public void test01_cursorToReminder() {
        Cursor reminderCursor = null;
        Reminder reminder = ReminderContent.cursorToReminder(reminderCursor);
        assertNull(reminder);
    }

    /**
     * The cursor is not null.
     */
    public void test02_cursorToReminder() {
        Cursor reminderCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        reminderCursor.moveToFirst();
        if (reminderCursor.getPosition() == -1
                || reminderCursor.getPosition() == reminderCursor.getCount()) {
            Reminder reminder = ReminderContent
                    .cursorToReminder(reminderCursor);
            assertNull(reminder);
        } else {
            Reminder reminder = ReminderContent
                    .cursorToReminder(reminderCursor);
            assertNotNull(reminder);
        }
        reminderCursor.close();
    }

    public void test03_cursorToReminder() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }

        Cursor reminderCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        reminderCursor.moveToFirst();
        if (reminderCursor.getPosition() == -1
                || reminderCursor.getPosition() == reminderCursor.getCount()) {
            Reminder reminder = ReminderContent
                    .cursorToReminder(reminderCursor);
            assertNull(reminder);
        } else {
            Reminder reminder = ReminderContent
                    .cursorToReminder(reminderCursor);
            assertNotNull(reminder);
        }
        reminderCursor.close();
    }

    public void test04_cursorToRaw() {
        Cursor eventCursor = null;
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        assertTrue(result == 0);
    }

    public void test05_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
    }

    public void test06_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = null;
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
    }

    public void test07_cursorToRaw() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }

        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
        assertTrue(result >= 0);
    }

    public void test08_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                ReminderContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
    }

    public void test09_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                ReminderContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = null;
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
    }

    public void test10_cursorToRaw() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }

        Cursor eventCursor = mContext.getContentResolver().query(
                ReminderContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
        assertTrue(result >= 0);
    }

    public void test11_cursorToRaw() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }

        for (int i = 0; i < eventList.size(); i++) {
            List<Reminder> reminderList = eventList.get(i).getReminders();
            for (int j = 0; j < reminderList.size(); j++) {
                mCalendarProxy.insertReminder(reminderList.get(j), eventList
                        .get(i).getCalendarId());
            }
        }

        Cursor eventCursor = mContext.getContentResolver().query(
                ReminderContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = ReminderContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
        assertTrue(result >= 0);
    }

    public void test12_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                ReminderContent.CONTENT_URI, null, null, null, null);
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        while (eventCursor.moveToNext()) {
            ReminderContent.cursorToRaw(eventCursor, buffer);
            ReminderContent.cursorToReminder(eventCursor);
        }
        eventCursor.close();
    }

    private ArrayList<CalendarEvent> getCalendarEvents(int eventNum) {

        ArrayList<CalendarEvent> eventList = new ArrayList<CalendarEvent>();

        for (int i = 0; i < eventNum; i++) {
            if (mEvent == null) {
                mAttendee = getAttendee();
                mAttendeeList = new ArrayList<Attendee>();
                mAttendeeList.add(mAttendee);

                mReminder = getReminder();
                mReminderList = new ArrayList<Reminder>();
                mReminderList.add(mReminder);

                mEvent = new CalendarEvent();
                mEvent.addAttendee(mAttendee);
                mEvent.addReminder(mReminder);
                mEvent.setAllDay(true);
                mEvent.setAttendees(mAttendeeList);
                mEvent.setReminders(mReminderList);
                mEvent.setCalendarOwner("wenxiang");
                mEvent.setDescription("there is a meeting" + i);
                mEvent.setTimeTo(1345766400 + i);
                mEvent.setTimeFrom(1345680000 + i);
                mEvent.setAccessLevel(CalendarEventContent.ACCESS_PUBLIC);
                mEvent.setAvailability(CalendarEventContent.AVAILABILITY_FREE);
                mEvent.setCalendarOwner("test owner" + i);
                mEvent.setTitle("test calender event" + i);

                eventList.add(mEvent);
                mEvent = null;
            }
        }

        return eventList;
    }

    private Attendee getAttendee() {
        if (mAttendee == null) {
            mAttendee = new Attendee();
            mAttendee.setAttendeeEmail("calendarTest@163.com");
            mAttendee.setAttendeeName("attendeeName");
            mAttendee
                    .setAttendeeRelationShip(AttendeeContent.RELATIONSHIP_PERFORMER);
            mAttendee.setAttendeeType(AttendeeContent.TYPE_OPTIONAL);
            mAttendee.setEventId(CalendarEvent.ID_NULL);
            mAttendee
                    .setAttendeeStatus(AttendeeContent.ATTENDEE_STATUS_ACCEPTED);
        }
        return mAttendee;

    }

    private Reminder getReminder() {
        if (mReminder == null) {
            mReminder = new Reminder();
            mReminder.setMethod(ReminderContent.METHOD_ALERT);
            mReminder.setMinutes(20);
        }
        return mReminder;

    }
}
