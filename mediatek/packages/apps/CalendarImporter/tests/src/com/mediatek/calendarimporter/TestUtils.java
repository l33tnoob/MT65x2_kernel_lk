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

package com.mediatek.calendarimporter;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.os.Environment;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Calendars;
import android.provider.CalendarContract.Events;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

import com.mediatek.calendarimporter.utils.InjectedServices;
import com.mediatek.calendarimporter.utils.MockAccountManager;

public class TestUtils {
    static final String NL = "\r\n";
    public static final Uri WRONG_VCS_URI = Uri.parse("file:///sdcard/Download/wrongVcs.vcs");
    public static final Uri VCS_File_URI = Uri.parse("file:///sdcard/Download/testVcsFile.vcs");
    public static final String MAIL_TYPE_GOOGLE = "com.google";
    public static long DEFAULT_SLEEP_TIME = 300;
    private static ArrayList<Uri> mTestAccountUris = new ArrayList<Uri>();
    private static MockAccountManager mMockAccountManager ;
    private static String TAG = "TestUtils";

    public static final String ONE_WRONG_VCS_DATA = "VCALENDAR_TEST";

    public static final String ONE_RIGHT_VCS_DATA = "BEGIN:VCALENDAR\r\n"
            + "PRODID:-//SyncCalendar//archermind//EN\nVERSION:2.0\r\n"
            + "BEGIN:VTIMEZONE\r\nTZID:UTC\r\nBEGIN:STANDARD\r\nDTSTART:16010101T000000\r\n"
            + "TZOFFSETFROM:+0000\r\nTZOFFSETTO:+0000\r\nEND:STANDARD\r\n"
            + "BEGIN:DAYLIGHT\r\nDTSTART:16010101T000000\r\nTZOFFSETFROM:+0000\r\n"
            + "TZOFFSETTO:+0000\r\nEND:DAYLIGHT\r\nEND:VTIMEZONE\r\n"
            + "BEGIN:VEVENT\r\nUID:1387\r\nSUMMARY:newline\r\nSTATUS:CONFIRMED\r\n"
            + "DESCRIPTION:Old\r\nNew\r\nDTSTART:20120829T023000Z\r\n"
            + "DTEND:20120829T033000Z\r\nX-TIMEZONE:Asia/Chongqing\r\nBEGIN:VALARM\r\n"
            + "ACTION:AUDIO\r\nTRIGGER:-PT10M\r\nDESCRIPTION:Reminder\r\n"
            + "END:VALARM\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n";

    /**
     * sleep for time.
     * 
     * @param time
     *            The time to sleep
     */
    public static void sleepForTime(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    static Uri getVcsFileUri() {
        File oFile = new File("/mnt/sdcard/TestExchange.vcs");
        if (oFile.exists()) {
            return Uri.fromFile(oFile);
        } else {
            return null;
        }
    }

    public static String eventString1 = "BEGIN:VEVENT\r\nUID:127\r\nSUMMARY:测试 vevent\r\nSTATUS:CONFIRMED\r\nLOCATION:office\r\n"
            + "RRULE:FREQ=WEEKLY;WKST=SU;BYDAY=FR\r\nDESCRIPTION:空间了\n墨迹了\n回车了\r\nDTSTART:20120824T033000Z\r\nDURATION:PT23H\r\n"
            + "X-TIMEZONE:Asia/Chongqing\r\nATTENDEE;PARTSTAT=ACCEPTED;ROLE=CHAIR;X-RELATIONSHIP=ORGANIZER:mailto:mcd.test02@gmail.com\r\n"
            + "ATTENDEE;CN=1B;PARTSTAT=NEEDS-ACTION;X-RELATIONSHIP=ATTENDEE;ROLE=REQ-PARTICIPANT:mailto:mcd.test04@gmail.com\r\n"
            + "BEGIN:VALARM\r\nACTION:AUDIO\r\nTRIGGER:-P1D\r\nDESCRIPTION:Reminder\r\nEND:VALARM\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n";

    public static String getItemString(boolean withRIrregular) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("BEGIN:VCALENDAR").append(NL);
        stringBuilder.append("PRODID:-//SyncCalendar//archermind//EN").append(NL);
        stringBuilder.append("VERSION:1.0").append(NL);
        stringBuilder.append("x-CALENDAR_ID:2").append(NL);
        stringBuilder.append("BEGIN:VEVENT").append(NL);
        stringBuilder.append("UID:42").append(NL);
        stringBuilder.append("SUMMARY;CHARSET=UTF-8:TestExchange").append(NL);
        stringBuilder.append("DESCRIPTION;CHARSET=UTF-8:nothing").append(NL);
        stringBuilder.append("LOCATION;CHARSET=UTF-8:home").append(NL);
        stringBuilder.append("TIMEZONE:Asia/Chongqing").append(NL);
        stringBuilder.append("STATUS:0").append(NL);
        stringBuilder.append("DTSTART:20120705T080000Z").append(NL);
        stringBuilder.append("DTEND:20120705T090000Z").append(NL);
        stringBuilder.append("x-ALLDAY:0").append(NL);
        stringBuilder.append("BEGIN:VALARM").append(NL);
        stringBuilder.append("TRIGGER:10").append(NL);
        stringBuilder.append("ACTION:1").append(NL);
        stringBuilder.append("END:VALARM").append(NL);
        stringBuilder.append("RRULE:FREQ=WEEKLY;BYDAY=TH;WKST=MO").append(NL);
        if (withRIrregular) {
            stringBuilder.append("EXCEPTION:20120705T080000Z,20120705T090000Z").append(NL);
        }
        stringBuilder.append("ATTENDEE;ROLE=ORGANIZER;STATUS=ACCEPTED:mailto:mcd.test02@gmail.com").append(NL);
        stringBuilder.append("ATTENDEE;CN=Mcd 04;ROLE=ATTENDEE:mailto:mcd.test04@gmail.com").append(NL);
        stringBuilder.append("DALARM:20120705T075000Z").append(NL);
        stringBuilder.append("AALARM:20120705T075000Z").append(NL);
        stringBuilder.append("END:VEVENT").append(NL);
        stringBuilder.append("END:VCALENDAR").append(NL);

        return stringBuilder.toString();
    }

    public static Uri addOneEventsToDB(ContentResolver resolver) {
        ContentValues values = new ContentValues();
        values.put(Events.TITLE, "testInstance");
        values.put(Events.DTSTART, 1346461200000L);
        values.put(Events.CALENDAR_ID, 1);
        values.put(Events.EVENT_TIMEZONE, "UTC");
        values.put(Events.DTEND, 1346463200000L);
        return resolver.insert(Events.CONTENT_URI, values);
    }

    public static boolean removeTheAddedEvent(ContentResolver resolver, Uri uri) {
        return resolver.delete(uri, null, null) < 0 ? false : true;
    }

    /**
     * Add one Mock System Account, and add it to calendars DB. Use like this:
     *
     * public void testAccount(){
     *     ...
     *     TestCaseUtils.addMockAccount(mContext, "test");
     *     ...
     * }
     *
     * Get this account by AccountManager.get(getApplicationContext()).getAccounts().
     */
    public static void addMockAccount(Context context, String name) {
        final Account account = new Account(name, MAIL_TYPE_GOOGLE);
        InjectedServices services = new InjectedServices();
        mMockAccountManager = new MockAccountManager(context, new Account[] { account });
        services.setSystemService(Context.ACCOUNT_SERVICE, mMockAccountManager);
        ContentResolver.setIsSyncable(account, CalendarContract.AUTHORITY, 1);
        CalendarImporterApplication.injectServices(services);
        Uri newAccountUri = addCalenarToProvider(context, name, MAIL_TYPE_GOOGLE);
        mTestAccountUris.add(newAccountUri);
    }

    /** Remove test accounts.
     *
     * @param context must be ApplicationContext.
     */
    public static void removeTestAccounts(Context context) {
        mMockAccountManager.removeAccount();
        for(int i=0;i<mTestAccountUris.size();i++) {
            context.getContentResolver().delete(mTestAccountUris.get(i), null, null);
        }
    }

    /**
     * Add one calendar to Calendars DB.
     */
    public static Uri addCalenarToProvider(Context context, String name, String accountType) {
        ContentValues m = new ContentValues();
        m.put(Calendars.NAME, name);
        m.put(Calendars.CALENDAR_DISPLAY_NAME, name);
        m.put(Calendars.VISIBLE, 1);
        m.put(Calendars.OWNER_ACCOUNT, name);
        m.put(Calendars.ACCOUNT_NAME, name);
        m.put(Calendars.ACCOUNT_TYPE, accountType);
        m.put(Calendars.SYNC_EVENTS, 1);
        m.put(Calendars.CALENDAR_ACCESS_LEVEL, 700);
        Uri url = context.getContentResolver().insert(
                addSyncQueryParams(Calendars.CONTENT_URI, name, accountType), m);
        return url;
    }

    public static File addFile(String fileFullName, String description) {
        File downloads = Environment
                .getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        if (!downloads.exists()) {
            if (!downloads.mkdirs()) {
                Log.d(TAG, "TestUtils mkdirs failed");
                return null;
            }
        }
        File file = new File(downloads, "/" + fileFullName);
        if (file.isAbsolute()) {
            try {
                if (!file.exists()) {
                    file.createNewFile();
                }
            } catch (IOException e) {
                Log.d("VCalComposer", " IOException");
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        // to add the content to this file.
        if(description != null) {
            FileOutputStream out = null;
            try {
                out = new FileOutputStream(file);
                byte[] bytes = description.getBytes();
                out.write(bytes);
                out.flush();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } finally {
                if (out != null) {
                    try {
                        out.close();
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }
            }
        }
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        return file.exists() ? file : null;
    }

    private static Uri addSyncQueryParams(Uri uri, String account, String accountType) {
        return uri.buildUpon().appendQueryParameter(CalendarContract.CALLER_IS_SYNCADAPTER, "true")
                .appendQueryParameter(Calendars.ACCOUNT_NAME, account).appendQueryParameter(
                        Calendars.ACCOUNT_TYPE, accountType).build();
    }

    /**
     * to remove the test vcs file.
     */
    public static boolean removeFile(File file) {
        if (file == null || !file.isAbsolute()) {
            return false;
        }
        file.delete();
        if (file.exists()) {
            file.deleteOnExit();
        }
        return (!file.exists()) ? true : false;
    }

    public static String vtodoString  =
            "VERSION:1.0\r\n" +
            "BEGIN:VTODO\r\n" +
            "DTSTART:20130503T020000Z\r\n" +
            "DUE:20051224T160000Z\r\n" +
            "COMPLETED:20051224T133000Z\r\n" +
            "DESCRIPTION:Remember to wrap Christmas presents\r\n" +
            "SUMMARY:Wrap presents\r\n" +
            "PRIORITY:1\r\n" +
            "CATEGORIES:PERSONAL\r\n" +
            "END:VTODO\r\n";
}
