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
import com.mediatek.apst.util.entity.calendar.CalendarEvent;
import com.mediatek.apst.util.entity.calendar.Reminder;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.data.provider.calendar.AttendeeContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarEventContent;
import com.mediatek.apst.target.data.provider.calendar.ReminderContent;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class CalendarEventContentTest extends AndroidTestCase {
    private static final String Tag = "CalendarEventContentTest";
    private Context mContext;
    private CalendarProxy mCalendarProxy;
    private Attendee mAttendee;
    private ArrayList<Attendee> mAttendeeList;
    private Reminder mReminder;
    private ArrayList<Reminder> mReminderList;
    private CalendarEvent mEvent;

    private byte[] raw = { 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0,
            0, 0, 1, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0, 0, 1, 57, -88,
            83, 96, 0, 0, 0, 1, 57, -88, 83, 96, 0, 1, 0, 0, 0, 4, 0, 49, 0,
            50, 0, 51, 0, 52, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0, 0, 0,
            21, 0, 87, 0, 101, 0, 110, 0, 120, 0, 105, 0, 97, 0, 110, 0, 103,
            0, 32, 0, 32, 0, 90, 0, 104, 0, 111, 0, 110, 0, 103, 0, 32, 0, 40,
            -108, -97, 101, -121, 121, 101, 0, 41, 0, 0, 0, 0, -1, -1, -1, -1,
            0, 0, 0, 0, 0, 0, 1, 57, -85, -2, 40, -27, 0, 0, 1, 57, -85, -35,
            -55, 35, -1, -1, -1, -1, 0, 0, 0, 13, 0, 65, 0, 115, 0, 105, 0, 97,
            0, 47, 0, 83, 0, 104, 0, 97, 0, 110, 0, 103, 0, 104, 0, 97, 0, 105,
            0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0,
            0, 1, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0, 0, 1, 57, -88, 83,
            96, 0, 0, 0, 1, 57, -88, 83, 96, 0, 1, 0, 0, 0, 4, 0, 49, 0, 50, 0,
            51, 0, 52, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0, 0, 0, 21, 0,
            87, 0, 101, 0, 110, 0, 120, 0, 105, 0, 97, 0, 110, 0, 103, 0, 32,
            0, 32, 0, 90, 0, 104, 0, 111, 0, 110, 0, 103, 0, 32, 0, 40, -108,
            -97, 101, -121, 121, 101, 0, 41, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0,
            0, 0, 0, 0, 1, 57, -85, -2, 6, -80, 0, 0, 1, 57, -85, -35, -54,
            -68, -1, -1, -1, -1, 0, 0, 0, 13, 0, 65, 0, 115, 0, 105, 0, 97, 0,
            47, 0, 83, 0, 104, 0, 97, 0, 110, 0, 103, 0, 104, 0, 97, 0, 105, 0,
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0,
            1, 0, 0, 0, 6, 0, 97, 0, 115, 0, 100, 0, 97, 0, 115, 0, 100, 0, 0,
            1, 58, 61, -85, -51, -67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0,
            97, 0, 115, 0, 100, 0, 97, 0, 115, 0, 100, 0, 0, 0, 9, 0, 97, 0,
            115, 0, 100, 0, 97, 0, 115, 0, 100, 0, 97, 0, 115, 0, 100, 0, 0, 0,
            21, 0, 87, 0, 101, 0, 110, 0, 120, 0, 105, 0, 97, 0, 110, 0, 103,
            0, 32, 0, 32, 0, 90, 0, 104, 0, 111, 0, 110, 0, 103, 0, 32, 0, 40,
            -108, -97, 101, -121, 121, 101, 0, 41, 0, 0, 0, 0, 0, 0, 0, 40, 0,
            70, 0, 82, 0, 69, 0, 81, 0, 61, 0, 87, 0, 69, 0, 69, 0, 75, 0, 76,
            0, 89, 0, 59, 0, 87, 0, 75, 0, 83, 0, 84, 0, 61, 0, 83, 0, 85, 0,
            59, 0, 66, 0, 89, 0, 68, 0, 65, 0, 89, 0, 61, 0, 77, 0, 79, 0, 44,
            0, 84, 0, 85, 0, 44, 0, 87, 0, 69, 0, 44, 0, 84, 0, 72, 0, 44, 0,
            70, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 57, -85,
            -35, -51, -100, 0, 0, 0, 6, 0, 80, 0, 49, 0, 55, 0, 57, 0, 57, 0,
            83, 0, 0, 0, 13, 0, 65, 0, 115, 0, 105, 0, 97, 0, 47, 0, 83, 0,
            104, 0, 97, 0, 110, 0, 103, 0, 104, 0, 97, 0, 105, 0, 0, 0, 0, 0,
            0, 0, 0 };

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
    public void test01_cursorToEvent() {
        Cursor eventCursor = null;
        CalendarEvent event = CalendarEventContent.cursorToEvent(eventCursor);
        assertNull(event);
    }

    /**
     * The cursor is not null.
     */
    public void test02_cursorToEvent() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        if (eventCursor.getPosition() == -1
                || eventCursor.getPosition() == eventCursor.getCount()) {
            CalendarEvent event = CalendarEventContent
                    .cursorToEvent(eventCursor);
            assertNull(event);
        } else {
            CalendarEvent event = CalendarEventContent
                    .cursorToEvent(eventCursor);
            assertNotNull(event);
        }
        eventCursor.close();
    }

    public void test03_cursorToEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }

        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        eventCursor.moveToFirst();
        if (eventCursor.getPosition() == -1
                || eventCursor.getPosition() == eventCursor.getCount()) {
            CalendarEvent event = CalendarEventContent
                    .cursorToEvent(eventCursor);
            assertNull(event);
        } else {
            CalendarEvent event = CalendarEventContent
                    .cursorToEvent(eventCursor);
            assertNotNull(event);
        }
        eventCursor.close();
    }

    public void test04_cursorToRaw() {
        Cursor eventCursor = null;
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = CalendarEventContent.cursorToRaw(eventCursor, buffer);
        assertTrue(result == 0);
    }

    public void test05_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null,
                CalendarEventContent.CALENDAR_ID + " = " + 10000, null, null);
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        int result = CalendarEventContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
    }

    public void test06_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null,
                CalendarEventContent.CALENDAR_ID + " = " + 10000, null, null);
        ByteBuffer buffer = null;
        int result = CalendarEventContent.cursorToRaw(eventCursor, buffer);
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
        int result = CalendarEventContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
        assertTrue(result >= 0);
    }

    public void test08_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        ByteBuffer buffer = null;
        int result = CalendarEventContent.cursorToRaw(eventCursor, buffer);
        eventCursor.close();
    }

    public void test09_cursorToRaw() {
        Cursor eventCursor = mContext.getContentResolver().query(
                CalendarEventContent.CONTENT_URI, null, null, null, null);
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        while (eventCursor.moveToNext()) {
            CalendarEventContent.cursorToRaw(eventCursor, buffer);
            CalendarEventContent.cursorToEvent(eventCursor);
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
