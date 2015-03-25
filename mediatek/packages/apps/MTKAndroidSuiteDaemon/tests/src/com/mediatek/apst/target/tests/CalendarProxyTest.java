/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.apst.target.tests;

import android.content.Context;
import android.test.AndroidTestCase;
import android.util.Log;
import android.database.Cursor;

import com.mediatek.apst.target.data.provider.calendar.AttendeeContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarContent;
import com.mediatek.apst.target.data.provider.calendar.CalendarEventContent;
import com.mediatek.apst.target.data.provider.calendar.ReminderContent;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.data.proxy.calendar.FastAttendeeCursorParser;
import com.mediatek.apst.target.data.proxy.calendar.FastCalendarCursorParser;
import com.mediatek.apst.target.data.proxy.calendar.FastEventCursorParser;
import com.mediatek.apst.target.data.proxy.calendar.FastReminderCursorParser;
import com.mediatek.apst.target.data.proxy.calendar.FastCalendarSyncFlagsCursorParser;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.calendar.Attendee;
import com.mediatek.apst.util.entity.calendar.CalendarEvent;
import com.mediatek.apst.util.entity.calendar.Reminder;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class CalendarProxyTest extends AndroidTestCase {
    private static final String Tag = "CalendarProxyTest";
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
     * Test singleton, if sInstance is null.
     */
    public void test01_getInstance01() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        assertNotNull(mCalendarProxy);
    }

    /**
     * Test singleton, if sInstance is not null.
     */
    public void test02_getInstance02() {
        CalendarProxy mCalendarProxy2 = null;
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        assertNotNull(mCalendarProxy);
        mCalendarProxy2 = CalendarProxy.getInstance(mContext);
        assertNotNull(mCalendarProxy2);
        assertSame(mCalendarProxy, mCalendarProxy2);
    }

    /**
     * Test insert a event of calendar.
     */
    public void test03_insertEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long id = mCalendarProxy.insertEvent(event);
        assertTrue(id >= 0);
    }

    /**
     * Test insert a attendee of calendar.
     */
    public void test04_insertAttendee() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long insertId = mCalendarProxy.insertEvent(event);
        Log.i(Tag, "insertId " + insertId);
        assertTrue(insertId >= 0);
        Attendee attendee = getAttendee();
        assertNotNull(attendee);
        attendee.setAttendeeType(AttendeeContent.TYPE_REQUIRED);
        Log.i(Tag, "eventID " + event.getId());
        long attendeeId = mCalendarProxy
                .insertAttendee(attendee, event.getId());
        Log.i(Tag, "attendeeId " + attendeeId);
    }

    /**
     * Test insert a reminder of calendar.
     */
    public void test05_insertReminder() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        Reminder reminder = getReminder();
        reminder.setEventId(eventId);
        mCalendarProxy.insertReminder(reminder, eventId);
    }

    public void test06_getEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        assertTrue(eventId > 0);
        mCalendarProxy.getEvent(eventId, true, true);
    }

    public void test07_deleteEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        int deleteCount = mCalendarProxy.deleteEvent(eventId);
        assertTrue(deleteCount >= 0);
    }

    public void test08_deleteEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        long[] eventIds = { eventId };
        boolean[] result = mCalendarProxy.deleteEvents(eventIds);
        assertTrue(result.length > 0);
    }

    public void test08_deleteAll() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        mCalendarProxy.deleteAll();
    }

    public void test09_deleteRemindersByEventId() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        int deleteCount = mCalendarProxy.deleteRemindersByEventId(eventId);
        assertTrue(deleteCount >= 0);
    }

    public void test10_deleteAttendeesByEventId() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        int deleteCount = mCalendarProxy.deleteAttendeesByEventId(eventId, "");
        assertTrue(deleteCount >= 0);
    }

    public void test11_deleteReminder() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        Reminder reminder = getReminder();
        long reminderId = mCalendarProxy.insertReminder(reminder, eventId);
        int deleteCount = mCalendarProxy.deleteReminder(reminderId);
        assertTrue(deleteCount >= 0);
    }

    public void test12_deleteAttendee() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        Attendee attendee = getAttendee();
        long attendeeId = mCalendarProxy.insertAttendee(attendee, eventId);
        int deleteCount = mCalendarProxy.deleteAttendee(attendeeId);
        assertTrue(deleteCount >= 0);
    }

    // public void test12_1_deleteAttendee() {
    // mCalendarProxy = CalendarProxy.getInstance(mContext);
    // CalendarEvent event = getCalendarEvent();
    // long eventId = mCalendarProxy.insertEvent(event);
    // Attendee attendee = getAttendee();
    // long attendeeId = mCalendarProxy.insertAttendee(attendee, eventId);
    // int deleteCount = mCalendarProxy.deleteAttendee(attendeeId);
    // assertTrue(deleteCount >= 0);
    // }

    public void test13_getCalendars() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.getCalendars(consumer, buffer);
        buffer.clear();
    }

    public void test14_getEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.getEvents(consumer, buffer);
        buffer.clear();
    }

    public void test15_getReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.getReminders(consumer, buffer);
        buffer.clear();
    }

    public void test16_getAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.getAttendees(consumer, buffer);
        buffer.clear();
    }

    public void test17_updateEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long id = mCalendarProxy.insertEvent(event);
        CalendarEvent newEvent = getCalendarEvent();
        newEvent.setTitle("test event");
        int updateCount = mCalendarProxy.updateEvent(id, newEvent);
        assertTrue(updateCount >= 0);
    }

    public void test18_updateAttendee() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        Attendee attendee = getAttendee();
        long id = mCalendarProxy.insertAttendee(attendee, eventId);
        Attendee newAttendee = getAttendee();
        newAttendee.setAttendeeName("test attendee");
        int updateCount = mCalendarProxy.updateAttendee(id, newAttendee);
        assertTrue(updateCount >= 0);
    }

    public void test19_updateReminder() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        Reminder reminder = getReminder();
        long id = mCalendarProxy.insertReminder(reminder, eventId);
        Reminder newReminder = getReminder();
        newReminder.setMethod(ReminderContent.METHOD_EMAIL);
        int updateCount = mCalendarProxy.updateReminder(id, newReminder);
        assertTrue(updateCount >= 0);
    }

    public void test20_getMaxEventId() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long id = mCalendarProxy.insertEvent(event);
        assertTrue(id >= 0);
        long maxEventId = mCalendarProxy.getMaxEventId();
        assertTrue(maxEventId >= 0);
    }

    public void test21_isSyncNeedReinit() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        assertNotNull(mCalendarProxy.isSyncNeedReinit());
    }

    public void test22_getLastSyncDate() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        assertTrue(mCalendarProxy.getLastSyncDate() >= 0);
    }

    public void test23_slowSyncGetAllEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.slowSyncGetAllEvents(1000, consumer, buffer);
        buffer.clear();
    }

    public void test24_slowSyncGetAllReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.slowSyncGetAllReminders(1000, consumer, buffer);
        buffer.clear();
    }

    public void test25_slowSyncGetAllAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.slowSyncGetAllAttendees(1000, consumer, buffer);
        buffer.clear();
    }

    public void test26_insertAllEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        ArrayList<CalendarEvent> eventList = new ArrayList<CalendarEvent>();
        eventList.add(event);
        long insertCount = mCalendarProxy.insertAllEvents(eventList);
        assertTrue(insertCount >= 0);
    }

    public void test27_getLocalAccountId() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long accountId = mCalendarProxy.getLocalAccountId();
        assertTrue(accountId >= 0);
    }

    public void test28_isSyncAble() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        assertNotNull(mCalendarProxy.isSyncAble());
    }

    public void test29_getSyncFlags() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        assertNotNull(mCalendarProxy.getSyncFlags(0, 1000));
    }

    public void test30_getSyncFlags() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long[] idSet = new long[1000];
        for (int i = 0; i < 1000; i++) {
            idSet[i] = i;
        }
        assertNotNull(mCalendarProxy.getSyncFlags(idSet));
    }

    public void test31_fastSyncGetAllSyncFlags() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.fastSyncGetAllSyncFlags(consumer, buffer);
    }

    public void test32_fastSyncGetEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy.fastSyncGetEvents(requestedEventIds, consumer, buffer);
    }

    public void test33_fastSyncGetReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetReminders(requestedEventIds, consumer, buffer);
    }

    public void test34_fastSyncGetAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetAttendees(requestedEventIds, consumer, buffer);
    }

    public void test35_fastDeleteEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long[] ids = new long[1000];
        for (int i = 0; i < 1000; i++) {
            ids[i] = i;
        }
        mCalendarProxy.fastDeleteEvents(ids);
    }

    public void test36_updateSyncDate() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long lastSyncDate = mCalendarProxy.getLastSyncDate();
        assertNotNull(mCalendarProxy.updateSyncDate(lastSyncDate));
    }

    public void test37_getPcSyncEventsCount() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        int count = mCalendarProxy.getPcSyncEventsCount();
        assertTrue(count >= 0);
    }

    public void test38_fastSyncAddEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        final byte[] raw = null;
        byte[] result = mCalendarProxy.fastSyncAddEvents(raw);
        assertNull(result);
    }

    public void test39_fastSyncAddEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        final byte[] raw = { 1, 1 };
        byte[] result = mCalendarProxy.fastSyncAddEvents(raw);
        assertNull(result);
    }

    public void test40_fastSyncAddEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        byte[] eventsInRaw = { 0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
                0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0,
                0, 1, 57, -85, -62, 75, 84, 0, 0, 1, 57, -85, -62, 75, 84, 1,
                0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0, 0, 0, 4, 0, 49, 0,
                50, 0, 51, 0, 52, 0, 0, 0, 21, 0, 87, 0, 101, 0, 110, 0, 120,
                0, 105, 0, 97, 0, 110, 0, 103, 0, 32, 0, 32, 0, 90, 0, 104, 0,
                111, 0, 110, 0, 103, 0, 32, 0, 40, -108, -97, 101, -121, 121,
                101, 0, 41, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 1, 57, -102, 82, 5, -1, -1, -1, -1, -1, 0, 0,
                0, 13, 0, 65, 0, 115, 0, 105, 0, 97, 0, 47, 0, 83, 0, 104, 0,
                97, 0, 110, 0, 103, 0, 104, 0, 97, 0, 105, 0, 0, 0, 1, 0, 0, 0,
                0 };
        byte[] result = mCalendarProxy.fastSyncAddEvents(eventsInRaw);
        assertNotNull(result);
    }

    public void test41_fastSyncUpdateEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        final byte[] raw = null;
        byte[] result = mCalendarProxy.fastSyncUpdateEvents(raw);
        assertNull(result);
    }

    public void test42_fastSyncUpdateEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        final byte[] raw = { 1, 1 };
        byte[] result = mCalendarProxy.fastSyncUpdateEvents(raw);
        assertNull(result);
    }

    public void test43_fastSyncUpdateEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        byte[] eventsInRaw = { 0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
                0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0,
                0, 1, 57, -85, -62, 75, 84, 0, 0, 1, 57, -85, -62, 75, 84, 1,
                0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 52, 0, 0, 0, 4, 0, 49, 0,
                50, 0, 51, 0, 52, 0, 0, 0, 21, 0, 87, 0, 101, 0, 110, 0, 120,
                0, 105, 0, 97, 0, 110, 0, 103, 0, 32, 0, 32, 0, 90, 0, 104, 0,
                111, 0, 110, 0, 103, 0, 32, 0, 40, -108, -97, 101, -121, 121,
                101, 0, 41, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 1, 57, -102, 82, 5, -1, -1, -1, -1, -1, 0, 0,
                0, 13, 0, 65, 0, 115, 0, 105, 0, 97, 0, 47, 0, 83, 0, 104, 0,
                97, 0, 110, 0, 103, 0, 104, 0, 97, 0, 105, 0, 0, 0, 1, 0, 0, 0,
                0 };
        byte[] result = mCalendarProxy.fastSyncUpdateEvents(eventsInRaw);
        assertNotNull(result);
    }

    public void test43_deleteEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        // long eventId = mCalendarProxy.insertEvent(event);
        long[] eventIds = null;
        boolean[] result = mCalendarProxy.deleteEvents(eventIds);
        assertNull(result);
    }
    
    public void test44_fastSyncGetEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = null;
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy.fastSyncGetEvents(requestedEventIds, consumer, buffer);
    }
    
    public void test45_fastSyncGetEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy.fastSyncGetEvents(requestedEventIds, consumer, buffer);
    }
    
    public void test46_slowSyncAddEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        byte[] result = mCalendarProxy.slowSyncAddEvents(raw);
        //assertNotNull(result);
    }

    public void test47_getAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.getAttendees(consumer, buffer);
        buffer.clear();
    }

    public void tes48_onParseCursorToRaw() {
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i=0;i<eventList.size();i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        Cursor c = mContext.getContentResolver().query(
                CalendarContent.CONTENT_URI, null, null, null, null);
        c.moveToFirst();
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        FastAttendeeCursorParser faseAttendeeCursorParser = new FastAttendeeCursorParser(
                c, consumer);
        int result = faseAttendeeCursorParser.onParseCursorToRaw(c, buffer);
        faseAttendeeCursorParser.parse();
        c.close();
        assertTrue(result >= 0);
    }

    public void tes49_onParseCursorToRaw() {
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i=0;i<eventList.size();i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        Cursor c = mContext.getContentResolver().query(
                CalendarContent.CONTENT_URI, null, null, null, null);
        c.moveToFirst();
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        FastCalendarCursorParser faseCalendarCursorParser = new FastCalendarCursorParser(
                c, consumer);
        int result = faseCalendarCursorParser.onParseCursorToRaw(c, buffer);
        faseCalendarCursorParser.parse();
        c.close();
        assertTrue(result >= 0);
    }

    public void tes50_onParseCursorToRaw() {
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i=0;i<eventList.size();i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        Cursor c = mContext.getContentResolver().query(
                CalendarContent.CONTENT_URI, null, null, null, null);
        c.moveToFirst();
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        FastEventCursorParser faseEventCursorParser = new FastEventCursorParser(
                c, consumer);
        int result = faseEventCursorParser.onParseCursorToRaw(c, buffer);
        faseEventCursorParser.parse();
        c.close();
        assertTrue(result >= 0);
    }

    public void tes51_onParseCursorToRaw() {
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i=0;i<eventList.size();i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        Cursor c = mContext.getContentResolver().query(
                CalendarContent.CONTENT_URI, null, null, null, null);
        c.moveToFirst();
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        FastReminderCursorParser faseReminderCursorParser = new FastReminderCursorParser(
                c, consumer);
        int result = faseReminderCursorParser.onParseCursorToRaw(c, buffer);
        faseReminderCursorParser.parse();
        c.close();
        assertTrue(result >= 0);
    }

    public void tes52_onParseCursorToRaw() {
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i=0;i<eventList.size();i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        Cursor c = mContext.getContentResolver().query(
                CalendarContent.CONTENT_URI, null, null, null, null);
        c.moveToFirst();
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        FastCalendarSyncFlagsCursorParser faseCalendarSyncCursorParser = new FastCalendarSyncFlagsCursorParser(
                c, consumer);
        int result = faseCalendarSyncCursorParser.onParseCursorToRaw(c, buffer);
        faseCalendarSyncCursorParser.parse();
        c.close();
        assertTrue(result >= 0);
    }

    public void test53_deleteAttendee() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long attendeeId = 10000;
        int deleteCount = mCalendarProxy.deleteAttendee(attendeeId);
        assertTrue(deleteCount >= 0);
    }

    public void test54_deleteReminder() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long reminderId = 10000;
        int deleteCount = mCalendarProxy.deleteReminder(reminderId);
        assertTrue(deleteCount >= 0);
    }

    public void test55_slowSyncAddEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        byte[] eventsInRaw = { 0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
                0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 6, 0, 97, 0, 115, 0, 100, 0, 97,
                0, 115, 0, 100, 0, 0, 1, 58, 61, -85, -51, -67, 0, 0, 1, 58,
                61, -57, 68, -4, 0, 0, 0, 0, 6, 0, 97, 0, 115, 0, 100, 0, 97,
                0, 115, 0, 100, 0, 0, 0, 9, 0, 97, 0, 115, 0, 100, 0, 97, 0,
                115, 0, 100, 0, 97, 0, 115, 0, 100, 0, 0, 0, 21, 0, 87, 0, 101,
                0, 110, 0, 120, 0, 105, 0, 97, 0, 110, 0, 103, 0, 32, 0, 32, 0,
                90, 0, 104, 0, 111, 0, 110, 0, 103, 0, 32, 0, 40, -108, -97,
                101, -121, 121, 101, 0, 41, 0, 0, 0, 0, 0, 0, 0, 40, 0, 70, 0,
                82, 0, 69, 0, 81, 0, 61, 0, 87, 0, 69, 0, 69, 0, 75, 0, 76, 0,
                89, 0, 59, 0, 87, 0, 75, 0, 83, 0, 84, 0, 61, 0, 83, 0, 85, 0,
                59, 0, 66, 0, 89, 0, 68, 0, 65, 0, 89, 0, 61, 0, 77, 0, 79, 0,
                44, 0, 84, 0, 85, 0, 44, 0, 87, 0, 69, 0, 44, 0, 84, 0, 72, 0,
                44, 0, 70, 0, 82, 0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 57, -98, 73,
                -120, 85, 0, 0, 0, 6, 0, 80, 0, 49, 0, 55, 0, 57, 0, 57, 0, 83,
                0, 0, 0, 13, 0, 65, 0, 115, 0, 105, 0, 97, 0, 47, 0, 83, 0,
                104, 0, 97, 0, 110, 0, 103, 0, 104, 0, 97, 0, 105, 0, 0, 0, 0,
                0, 0, 0, 0 };
        byte[] result = mCalendarProxy.slowSyncAddEvents(eventsInRaw);
        //assertNotNull(result);
    }
    
    public void test56_getEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        mCalendarProxy.getEvents(consumer, buffer);
        buffer.clear();
    }
    
    public void test57_getEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = null;
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        mCalendarProxy.getEvents(consumer, buffer);
        buffer.clear();
    }
    
    public void test58_getEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = null;
        mCalendarProxy.getEvents(consumer, buffer);
        //buffer.clear();
    }
    
    public void test59_deleteAttendee() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        for (int i = 0; i < eventList.size(); i++) {
            mCalendarProxy.insertEvent(eventList.get(i));
        }
        
        List<Attendee> attendeeList = null;
        for (int i = 0; i < eventList.size(); i++) {
            attendeeList = eventList.get(i).getAttendees();
            for (int j = 0;j < attendeeList.size();j++) {
                mCalendarProxy.insertAttendee(attendeeList.get(j), eventList.get(i).getCalendarId());
            }
        }

        for (int i = 0;i < attendeeList.size(); i++) {
            mCalendarProxy.deleteAttendee(attendeeList.get(i).getEventId());
        }
    }
    
//    public void test60_deleteEvents() {
//        mCalendarProxy = CalendarProxy.getInstance(mContext);
//        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
//        long[] eventIds = new long[eventList.size()];
//        
//        for (int i = 0; i < eventList.size(); i++) {
//            eventIds[i] = mCalendarProxy.insertEvent(eventList.get(i));
//        }
//    
//        boolean[] result = mCalendarProxy.deleteEvents(eventIds);
//        assertNull(result);
//    }
    
    public void test61_getEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        assertTrue(eventId > 0);
        mCalendarProxy.getEvent(eventId, false, true);
        mCalendarProxy.getEvent(eventId, true, false);
        mCalendarProxy.getEvent(eventId, false, false);
    }
    
    public void test62_getEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long eventId = 100000;
        mCalendarProxy.getEvent(eventId, true, true);
        mCalendarProxy.getEvent(eventId, false, true);
        mCalendarProxy.getEvent(eventId, true, false);
        mCalendarProxy.getEvent(eventId, false, false);
    }
    
    public void test63_getEvent() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        ArrayList<CalendarEvent> eventList = getCalendarEvents(5);
        long[] eventIds = new long[eventList.size()];
        
        for (int i = 0; i < eventList.size(); i++) {
            eventIds[i] = mCalendarProxy.insertEvent(eventList.get(i));
        }
        
        for (int i = 0; i < eventIds.length; i++) {
            mCalendarProxy.getEvent(eventIds[i], true, true);
            mCalendarProxy.getEvent(eventIds[i], false, true);
            mCalendarProxy.getEvent(eventIds[i], true, false);
            mCalendarProxy.getEvent(eventIds[i], false, false);
        } 
    }
    
    public void test64_fastSyncGetEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = ByteBuffer.wrap(raw);
        long[] requestedEventIds = {-1};
        mCalendarProxy.fastSyncGetEvents(requestedEventIds, consumer, buffer);
    }
    
    public void test65_fastSyncGetEvents() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = null;
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy.fastSyncGetEvents(requestedEventIds, consumer, buffer);
    }
    
    public void test66_insertReminder() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long eventId = mCalendarProxy.insertEvent(event);
        Reminder reminder = null;
        mCalendarProxy.insertReminder(reminder, eventId);
    }
    
    public void test67_deleteRemindersByEventId() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        long eventId = 10000;
        int deleteCount = mCalendarProxy.deleteRemindersByEventId(eventId);
        //assertTrue(deleteCount >= 0);
    }
    
    public void test68_fastSyncGetReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = null;
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetReminders(requestedEventIds, consumer, buffer);
    }
    
    public void test69_fastSyncGetReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = null;
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetReminders(requestedEventIds, consumer, buffer);
    }
    
    public void test70_fastSyncGetReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = {-1, -2, -3};
        mCalendarProxy
                .fastSyncGetReminders(requestedEventIds, consumer, buffer);
    }
    
    public void test71_fastSyncGetReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = {-1};
        mCalendarProxy
                .fastSyncGetReminders(requestedEventIds, consumer, buffer);
    }
    
    public void test72_fastSyncGetReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        
        byte[] reminderData = { 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
                0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 25, 0,
                0, 0, 0, 0, 0, 0, 1 };
        
        ByteBuffer buffer = ByteBuffer.wrap(reminderData);
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetReminders(requestedEventIds, consumer, buffer);
    }
    
    public void test73_fastSyncGetAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = null;
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetAttendees(requestedEventIds, consumer, buffer);
    }
    
    public void test74_fastSyncGetAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = null;
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetAttendees(requestedEventIds, consumer, buffer);
    }
    
    public void test75_fastSyncGetAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        long[] requestedEventIds = {-1};
        mCalendarProxy
                .fastSyncGetAttendees(requestedEventIds, consumer, buffer);
    }
    
    public void test77_fastSyncGetAttendees() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        
        byte[] attendeeDate = { 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                1, -1, -1, -1, -1, 0, 0, 0, 19, 0, 116, 0, 101, 0, 115, 0, 116,
                0, 48, 0, 49, 0, 64, 0, 109, 0, 101, 0, 100, 0, 105, 0, 97, 0,
                116, 0, 101, 0, 107, 0, 46, 0, 99, 0, 111, 0, 109, 0, 0, 0, 0,
                0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0,
                0, 0, 1, -1, -1, -1, -1, 0, 0, 0, 7, 0, 80, 0, 67, 0, 32, 0,
                83, 0, 121, 0, 110, 0, 99, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 2, -1, -1, -1, -1,
                0, 0, 0, 19, 0, 116, 0, 101, 0, 115, 0, 116, 0, 48, 0, 50, 0,
                64, 0, 109, 0, 101, 0, 100, 0, 105, 0, 97, 0, 116, 0, 101, 0,
                107, 0, 46, 0, 99, 0, 111, 0, 109, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 2, -1,
                -1, -1, -1, 0, 0, 0, 7, 0, 80, 0, 67, 0, 32, 0, 83, 0, 121, 0,
                110, 0, 99, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 3, -1, -1, -1, -1, 0, 0, 0, 19,
                0, 116, 0, 101, 0, 115, 0, 116, 0, 48, 0, 51, 0, 64, 0, 109, 0,
                101, 0, 100, 0, 105, 0, 97, 0, 116, 0, 101, 0, 107, 0, 46, 0,
                99, 0, 111, 0, 109, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 3, -1, -1, -1, -1, 0, 0,
                0, 7, 0, 80, 0, 67, 0, 32, 0, 83, 0, 121, 0, 110, 0, 99, 0, 0,
                0, 0, 0, 0, 0, 2, 0, 0, 0, 0 };
        
        ByteBuffer buffer = ByteBuffer.wrap(attendeeDate);
        long[] requestedEventIds = new long[1000];
        for (int i = 0; i < 1000; i++) {
            requestedEventIds[i] = i;
        }
        mCalendarProxy
                .fastSyncGetAttendees(requestedEventIds, consumer, buffer);
    }
    
    public void test78_insertAttendee() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        CalendarEvent event = getCalendarEvent();
        long insertId = mCalendarProxy.insertEvent(event);
        Log.i(Tag, "insertId " + insertId);
        assertTrue(insertId >= 0);
        Attendee attendee = null;
        long attendeeId = mCalendarProxy
                .insertAttendee(attendee, event.getId());
        Log.i(Tag, "attendeeId " + attendeeId);
    }
    
    public void test79_slowSyncGetAllReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = null;
        ByteBuffer buffer = Global.getByteBuffer();
        mCalendarProxy.slowSyncGetAllReminders(1000, consumer, buffer);
        buffer.clear();
    }
    
    public void test80_slowSyncGetAllReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = null;
        mCalendarProxy.slowSyncGetAllReminders(1000, consumer, buffer);
    }
    
    public void test81_slowSyncGetAllReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        byte[] reminderData = { 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
                0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 25, 0,
                0, 0, 0, 0, 0, 0, 1 };
        
        ByteBuffer buffer = ByteBuffer.wrap(reminderData);
        mCalendarProxy.slowSyncGetAllReminders(1000, consumer, buffer);
        buffer.clear();
    }
    
    public void test82_slowSyncGetAllReminders() {
        mCalendarProxy = CalendarProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        byte[] reminderData = { 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
                0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 25, 0,
                0, 0, 0, 0, 0, 0, 1 };
        
        ByteBuffer buffer = ByteBuffer.wrap(reminderData);
        mCalendarProxy.slowSyncGetAllReminders(0, consumer, buffer);
        buffer.clear();
    }
    
    private CalendarEvent getCalendarEvent() {
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
            mEvent.setCalendarOwner("shi");
            mEvent.setDescription("there is a meeting");
            mEvent.setTimeTo(1345766400);
            mEvent.setTimeFrom(1345680000);
            mEvent.setAccessLevel(CalendarEventContent.ACCESS_PUBLIC);
            mEvent.setAvailability(CalendarEventContent.AVAILABILITY_FREE);
            mEvent.setCalendarOwner("test owner");
            mEvent.setTitle("test calender event");
        }

        return mEvent;

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
