/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.schpwronoff;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.Parcel;
import android.provider.Settings;
import android.text.format.DateFormat;

import com.mediatek.xlog.Xlog;

import java.util.Calendar;

/**
 * The Alarms provider supplies info about Alarm Clock settings
 */
public class Alarms {
    private static final String TAG = "Settings/Alarms";
    // This action triggers the AlarmReceiver as well as the AlarmKlaxon. It
    // is a public action used in the manifest for receiving Alarm broadcasts
    // from the alarm manager.
    public static final String ALARM_ALERT_ACTION = "com.android.alarmclock.ALARM_ALERT";

    // This is a private action used when the user clears all notifications.
    public static final String CLEAR_NOTIFICATION = "clear_notification";

    // This is a private action used by the AlarmKlaxon to update the UI to
    // show the alarm has been killed.
    public static final String ALARM_KILLED = "alarm_killed";

    // Extra in the ALARM_KILLED intent to indicate to the user how long the
    // alarm played before being killed.
    public static final String ALARM_KILLED_TIMEOUT = "alarm_killed_timeout";

    // This string is used to indicate a silent alarm in the db.
    public static final String ALARM_ALERT_SILENT = "silent";

    // This intent is sent from the notification when the user cancels the
    // snooze alert.
    public static final String CANCEL_SNOOZE = "cancel_snooze";

    // This string is used when passing an Alarm object through an intent.
    public static final String ALARM_INTENT_EXTRA = "intent.extra.alarm";

    // This extra is the raw Alarm object data. It is used in the
    // AlarmManagerService to avoid a ClassNotFoundException when filling in
    // the Intent extras.
    public static final String ALARM_RAW_DATA = "intent.extra.alarm_raw";

    // This string is used to identify the alarm id passed to SetAlarm from the
    // list of alarms.
    public static final String ALARM_ID = "alarm_id";

    static final String PREF_SNOOZE_ID = "snooze_id";
    static final String PREF_SNOOZE_TIME = "snooze_time";

    private static final String DM12 = "E h:mm aa";
    private static final String DM24 = "E k:mm";

    private static final String M12 = "h:mm aa";
    // Shared with DigitalClock
    static final String M24 = "kk:mm";

    /**
     * Creates a new Alarm.
     * @param contentResolver ContentResolver
     * @return Uri data for alarm uri
     */
    public static Uri addAlarm(ContentResolver contentResolver) {
        final int defaultHour = 8;
        ContentValues values = new ContentValues();
        values.put(Alarm.Columns.HOUR, defaultHour);
        return contentResolver.insert(Alarm.Columns.CONTENT_URI, values);
    }

    /**
     * Removes an existing Alarm. If this alarm is snoozing, disables snooze. Sets next alert.
     * @param context Context
     * @param alarmId int alarm id
     */
    public static void deleteAlarm(Context context, int alarmId) {

        ContentResolver contentResolver = context.getContentResolver();
        /* If alarm is snoozing, lose it */
        disableSnoozeAlert(context, alarmId);

        Uri uri = ContentUris.withAppendedId(Alarm.Columns.CONTENT_URI, alarmId);
        contentResolver.delete(uri, "", null);

        setNextAlert(context);
    }

    /**
     * Queries all alarms
     * @param contentResolver ContentResolver
     * @return cursor over all alarms
     */
    public static Cursor getAlarmsCursor(ContentResolver contentResolver) {
        return contentResolver.query(Alarm.Columns.CONTENT_URI, Alarm.Columns.ALARM_QUERY_COLUMNS, null, null,
                Alarm.Columns.DEFAULT_SORT_ORDER);
    }

    // Private method to get a more limited set of alarms from the database.
    private static Cursor getFilteredAlarmsCursor(ContentResolver contentResolver) {
        return contentResolver.query(Alarm.Columns.CONTENT_URI, Alarm.Columns.ALARM_QUERY_COLUMNS,
                Alarm.Columns.WHERE_ENABLED, null, null);
    }

    private static Cursor getFilteredAlarmsCursor(ContentResolver contentResolver, int alarmId) {
        return contentResolver.query(ContentUris.withAppendedId(Alarm.Columns.CONTENT_URI, alarmId),
                Alarm.Columns.ALARM_QUERY_COLUMNS, Alarm.Columns.WHERE_ENABLED, null, null);
    }

    /**
     * Return an Alarm object representing the alarm id in the database. 
     * Returns null if no alarm exists.
     * @param contentResolver ContentResolver
     * @param alarmId id
     * @return Alarm object 
     */
    public static Alarm getAlarm(ContentResolver contentResolver, int alarmId) {
        Cursor cursor = null;
        Alarm alarm = null;
        try {
            cursor = contentResolver.query(ContentUris.withAppendedId(Alarm.Columns.CONTENT_URI, alarmId),
                    Alarm.Columns.ALARM_QUERY_COLUMNS, null, null, null);
            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    alarm = new Alarm(cursor);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return alarm;
    }

    /**
     * A convenience method to set an alarm in the Alarms content provider.
     * @param context Context
     * @param id corresponds to the _id column
     * @param enabled corresponds to the ENABLED column
     * @param hour corresponds to the HOUR column
     * @param minutes corresponds to the MINUTES column
     * @param daysOfWeek corresponds to the DAYS_OF_WEEK column
     * @param vibrate corresponds to the VIBRATE column
     * @param message corresponds to the MESSAGE column
     * @param alert corresponds to the ALERT column
     */
    public static void setAlarm(Context context, int id, boolean enabled, int hour, int minutes,
            Alarm.DaysOfWeek daysOfWeek, boolean vibrate, String message, String alert) {
        final int initSize = 8;
        ContentValues values = new ContentValues(initSize);
        ContentResolver resolver = context.getContentResolver();
        // Set the alarm_time value if this alarm does not repeat. This will be
        // used later to disable expired alarms.
        long time = 0;
        if (!daysOfWeek.isRepeatSet()) {
            time = calculateAlarm(hour, minutes, daysOfWeek).getTimeInMillis();
        }

        Xlog.d(TAG, "**  setAlarm * idx " + id + " hour " + hour + " minutes " + minutes + " enabled " + enabled + " time "
                + time);

        values.put(Alarm.Columns.ENABLED, enabled ? 1 : 0);
        values.put(Alarm.Columns.HOUR, hour);
        values.put(Alarm.Columns.MINUTES, minutes);
        values.put(Alarm.Columns.ALARM_TIME, time);
        values.put(Alarm.Columns.DAYS_OF_WEEK, daysOfWeek.getCoded());
        values.put(Alarm.Columns.VIBRATE, vibrate);
        values.put(Alarm.Columns.MESSAGE, message);
        values.put(Alarm.Columns.ALERT, alert);
        resolver.update(ContentUris.withAppendedId(Alarm.Columns.CONTENT_URI, id), values, null, null);

        if (id == 1) {
            // power on
            setNextAlertPowerOn(context);
        } else if (id == 2) {
            // power off
            setNextAlertPowerOff(context);
        }
    }

    /**
     * A convenience method to enable or disable an alarm.
     * @param context Context
     * @param id corresponds to the _id column
     * @param enabled corresponds to the ENABLED column
     */
    public static void enableAlarm(final Context context, final int id, boolean enabled) {
        enableAlarmInternal(context, id, enabled);
        if (id == 1) {
            setNextAlertPowerOn(context);
        } else if (id == 2) {
            setNextAlertPowerOff(context);
        }
    }

    private static void enableAlarmInternal(final Context context, final int id, boolean enabled) {
        Alarm al = getAlarm(context.getContentResolver(), id);
        if (al != null) {
            enableAlarmInternal(context, al, enabled);
        }
    }

    private static void enableAlarmInternal(final Context context, final Alarm alarm, boolean enabled) {
        ContentResolver resolver = context.getContentResolver();

        ContentValues values = new ContentValues(2);
        values.put(Alarm.Columns.ENABLED, enabled ? 1 : 0);

        // If we are enabling the alarm, calculate alarm time since the time
        // value in Alarm may be old.
        if (enabled) {
            long time = 0;
            if (!alarm.mDaysOfWeek.isRepeatSet()) {
                time = calculateAlarm(alarm.mHour, alarm.mMinutes, alarm.mDaysOfWeek).getTimeInMillis();
            }
            alarm.mTime = time;
            values.put(Alarm.Columns.ALARM_TIME, time);
        }

        resolver.update(ContentUris.withAppendedId(Alarm.Columns.CONTENT_URI, alarm.mId), values, null, null);
    }

    /**
     * calculate Next Alert alarm
     * @param context final Context
     * @param alarmId final int
     * @return Alarm
     */
    public static Alarm calculateNextAlert(final Context context, final int alarmId) {
        Alarm alarm = null;
        Cursor cursor = null;
        long minTime = Long.MAX_VALUE;
        long now = System.currentTimeMillis();
        Xlog.d(TAG, "Alarms.calculateNextAlert()_now = " + now);
        try {
            cursor = getFilteredAlarmsCursor(context.getContentResolver(), alarmId);
            if (cursor != null) {
                if (cursor.moveToFirst()) {
                    do {
                        Alarm a = new Alarm(cursor);
                        Xlog.d(TAG, "Alarms.calculateNextAlert()_atime = " + a.mTime);
                        Xlog.d(TAG, "Alarms.calculateNextAlert()_min_time = " + minTime);
                        // A time of 0 indicates this is a repeating alarm, so
                        // calculate the time to get the next alert.
                        if (a.mTime == 0) {
                            a.mTime = calculateAlarm(a.mHour, a.mMinutes, a.mDaysOfWeek).getTimeInMillis();
                            Xlog.d(TAG, "Alarms.calculateNextAlert()_calculateAlarm = " + a.mTime);
                        } else if (a.mTime < now) {
                            Xlog.d(TAG, "Alarms.calculateNextAlert()_atime < now");
                            // Expired alarm, disable it and move along.
                            enableAlarmInternal(context, a, false);
                            continue;
                        } else if (a.mEnabled) {
                            enableAlarmInternal(context, a, true);
                        }
                        if (a.mTime < minTime) {
                            minTime = a.mTime;
                            alarm = a;
                        }
                    } while (cursor.moveToNext());
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return alarm;
    }

    /**
     * Disables non-repeating alarms that have passed. Called at boot.
     * @param context Context
     */
    public static void disableExpiredAlarms(final Context context) {
        Cursor cur = null;
        try {
            cur = getFilteredAlarmsCursor(context.getContentResolver());
            long now = System.currentTimeMillis();
            if (cur != null) {
                if (cur.moveToFirst()) {
                    do {
                        Alarm alarm = new Alarm(cur);
                        // A time of 0 means this alarm repeats. If the time is
                        // non-zero, check if the time is before now.
                        if (alarm.mTime != 0 && alarm.mTime < now) {
                            Xlog.d(TAG, "** DISABLE " + alarm.mId + " now " + now + " set " + alarm.mTime);
                            enableAlarmInternal(context, alarm, false);
                        }
                    } while (cur.moveToNext());
                }
            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
    }

    /**
     * Called at system startup, on time/timezone change, and whenever the user changes alarm settings. Activates snooze if
     * set, otherwise loads all alarms, activates next alert.
     * @param context Context
     */
    public static void setNextAlert(final Context context) {
        Alarm alarm;
        alarm = calculateNextAlert(context, 1);
        if (alarm == null) {
            disableAlertPowerOn(context);
        } else {
            enableAlertPowerOn(context, alarm, alarm.mTime);
        }

        alarm = calculateNextAlert(context, 2);
        if (alarm == null) {
            disableAlert(context);
        } else {
            enableAlert(context, alarm, alarm.mTime);
        }
    }

    /**
     * set Next Alert Power Off
     * @param context Context
     */
    public static void setNextAlertPowerOff(final Context context) {
        Alarm alarm = calculateNextAlert(context, 2);
        if (alarm == null) {
            disableAlert(context);
        } else {
            enableAlert(context, alarm, alarm.mTime);
        }
    }

    /**
     * Sets alert in AlarmManger and StatusBar. This is what will actually launch the alert when the alarm triggers.
     * 
     * @param alarm
     *            Alarm.
     * @param atTimeInMillis
     *            milliseconds since epoch
     */
    private static void enableAlert(Context context, final Alarm alarm, final long atTimeInMillis) {
        if (alarm == null) {
            return;
        }
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);

        Xlog.d(TAG, "** setAlert id " + alarm.mId + " atTime " + atTimeInMillis);

        // Intent intent = new Intent(ALARM_ALERT_ACTION);
        // Intent intent = new Intent(context, SchedulePowerOff.class);
        Intent intent = new Intent(context, com.mediatek.schpwronoff.SchPwrOffReceiver.class);

        // XXX: This is a slight hack to avoid an exception in the remote
        // AlarmManagerService process. The AlarmManager adds extra data to
        // this Intent which causes it to inflate. Since the remote process
        // does not know about the Alarm class, it throws a
        // ClassNotFoundException.
        //
        // To avoid this, we marshall the data ourselves and then parcel a plain
        // byte[] array. The AlarmReceiver class knows to build the Alarm
        // object from the byte[] array.
        Parcel out = Parcel.obtain();
        alarm.writeToParcel(out, 0);
        out.setDataPosition(0);
        intent.putExtra(ALARM_RAW_DATA, out.marshall());

        PendingIntent sender = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);

        Xlog.d(TAG, "Alarms.enableAlertPowerOff(): setAlert id " + alarm.mId + " atTime " + atTimeInMillis);
        am.setExact(AlarmManager.RTC_WAKEUP, atTimeInMillis, sender);

        Calendar c = Calendar.getInstance();
        c.setTime(new java.util.Date(atTimeInMillis));
        // String timeString = formatDayAndTime(context, c);
        // saveNextAlarm(context, timeString);
    }

    /**
     * Disables alert in AlarmManger and StatusBar.
     * 
     * @param id
     *            Alarm ID.
     */
    static void disableAlert(Context context) {
        Intent intent = new Intent(context, com.mediatek.schpwronoff.SchPwrOffReceiver.class);
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent sender = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
        Xlog.d(TAG, "Alarms.disableAlertPowerOff(): disableForPowerOff");
        am.cancel(sender);
        // setStatusBarIcon(context, false);
        // saveNextAlarm(context, "");
    }

    /**
     * Called at system startup, on time/timezone change, and whenever the user changes alarm settings. Activates snooze if
     * set, otherwise loads all alarms, activates next alert.
     * @param context Context
     */
    public static void setNextAlertPowerOn(final Context context) {
        Alarm alarm = calculateNextAlert(context, 1);
        if (alarm == null) {
            disableAlertPowerOn(context);
        } else {
            enableAlertPowerOn(context, alarm, alarm.mTime);
        }
    }

    /**
     * Sets alert in AlarmManger and StatusBar. This is what will actually launch the alert when the alarm triggers.
     * 
     * @param alarm
     *            Alarm.
     * @param atTimeInMillis
     *            milliseconds since epoch
     */
    private static void enableAlertPowerOn(Context context, final Alarm alarm, final long atTimeInMillis) {
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);

        Xlog.d(TAG, "** setAlert id " + alarm.mId + " atTime " + atTimeInMillis);

        // Intent intent = new Intent(ALARM_ALERT_ACTION);
        // Intent intent = new Intent(context, SchedulePowerOff.class);
        Intent intent = new Intent(context, com.mediatek.schpwronoff.SchPwrOnReceiver.class);

        // XXX: This is a slight hack to avoid an exception in the remote
        // AlarmManagerService process. The AlarmManager adds extra data to
        // this Intent which causes it to inflate. Since the remote process
        // does not know about the Alarm class, it throws a
        // ClassNotFoundException.
        //
        // To avoid this, we marshall the data ourselves and then parcel a plain
        // byte[] array. The AlarmReceiver class knows to build the Alarm
        // object from the byte[] array.
        Parcel out = Parcel.obtain();
        alarm.writeToParcel(out, 0);
        out.setDataPosition(0);
        intent.putExtra(ALARM_RAW_DATA, out.marshall());

        PendingIntent sender = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);

        am.setExact(7, atTimeInMillis, sender);
        Xlog.d(TAG, "Alarms.enableAlertPowerOn(): setAlert id " + alarm.mId + " atTime " + atTimeInMillis);

        // setStatusBarIcon(context, true);

        Calendar c = Calendar.getInstance();
        c.setTime(new java.util.Date(atTimeInMillis));
        // String timeString = formatDayAndTime(context, c);
        // saveNextAlarm(context, timeString);
    }

    /**
     * Disables alert in AlarmManger and StatusBar.
     * 
     * @param id
     *            Alarm ID.
     */
    static void disableAlertPowerOn(Context context) {
        Intent intent = new Intent(context, com.mediatek.schpwronoff.SchPwrOnReceiver.class);
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent sender = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
        am.setExact(7, 0, sender);
        am.cancelPoweroffAlarm(context.getPackageName());
        Xlog.d(TAG, "Alarms.disableAlertPowerOn(): disableForPowerOn");
        // am.cancel(sender);
        // setStatusBarIcon(context, false);
        // saveNextAlarm(context, "");
    }

    static void saveSnoozeAlert(final Context context, final int id, final long time) {
        Xlog.d(TAG, "Alarms.saveSnoozeAlert(): id " + id);
        SharedPreferences prefs = context.getSharedPreferences(AlarmClock.PREFERENCES, 0);
        SharedPreferences.Editor ed = prefs.edit();
        if (id == -1) {
            clearSnoozePreference(ed);
        } else {
            ed.putInt(PREF_SNOOZE_ID, id);
            ed.putLong(PREF_SNOOZE_TIME, time);
            ed.commit();
        }
        // Set the next alert after updating the snooze.
        setNextAlert(context);
    }

    /**
     * Disable the snooze alert if the given id matches the snooze id.
     */
    static void disableSnoozeAlert(final Context context, final int id) {
        SharedPreferences prefs = context.getSharedPreferences(AlarmClock.PREFERENCES, 0);
        int snoozeId = prefs.getInt(PREF_SNOOZE_ID, -1);
        if (snoozeId == -1) {
            // No snooze set, do nothing.
            return;
        } else if (snoozeId == id) {
            // This is the same id so clear the shared prefs.
            clearSnoozePreference(prefs.edit());
        }
    }

    // Helper to remove the snooze preference. Do not use clear because that
    // will erase the clock preferences.
    private static void clearSnoozePreference(final SharedPreferences.Editor ed) {
        ed.remove(PREF_SNOOZE_ID);
        ed.remove(PREF_SNOOZE_TIME);
        ed.commit();
    };

    /**
     * If there is a snooze set, enable it in AlarmManager
     * 
     * @return true if snooze is set
     */
    private static boolean enableSnoozeAlert(final Context context) {
        SharedPreferences prefs = context.getSharedPreferences(AlarmClock.PREFERENCES, 0);

        int id = prefs.getInt(PREF_SNOOZE_ID, -1);
        if (id == -1) {
            return false;
        }
        long time = prefs.getLong(PREF_SNOOZE_TIME, -1);

        // Get the alarm from the db.
        final Alarm alarm = getAlarm(context.getContentResolver(), id);
        // The time in the database is either 0 (repeating) or a specific time
        // for a non-repeating alarm. Update this value so the AlarmReceiver
        // has the right time to compare.
        if (alarm != null) {
            alarm.mTime = time;
        }
        enableAlert(context, alarm, time);
        return true;
    }

    /**
     * Tells the StatusBar whether the alarm is enabled or disabled
     */
    private static void setStatusBarIcon(Context context, boolean enabled) {
        Intent alarmChanged = new Intent(Intent.ACTION_ALARM_CHANGED);
        alarmChanged.putExtra("alarmSet", enabled);
        context.sendBroadcast(alarmChanged);
    }

    /**
     * Given an alarm in hours and minutes, return a time suitable for setting in AlarmManager.
     * 
     * @param hour
     *            Always in 24 hour 0-23
     * @param minute
     *            0-59
     * @param daysOfWeek
     *            0-59
     */
    static Calendar calculateAlarm(int hour, int minute, Alarm.DaysOfWeek daysOfWeek) {

        // start with now
        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(System.currentTimeMillis());

        int nowHour = c.get(Calendar.HOUR_OF_DAY);
        int nowMinute = c.get(Calendar.MINUTE);

        // if alarm is behind current time, advance one day
        if (hour < nowHour || hour == nowHour && minute <= nowMinute) {
            c.add(Calendar.DAY_OF_YEAR, 1);
        }
        c.set(Calendar.HOUR_OF_DAY, hour);
        c.set(Calendar.MINUTE, minute);
        c.set(Calendar.SECOND, 0);
        c.set(Calendar.MILLISECOND, 0);

        int addDays = daysOfWeek.getNextAlarm(c);
        /*
         * Log.v("** TIMES * " + c.getTimeInMillis() + " hour " + hour + " minute " + minute + " dow " +
         * c.get(Calendar.DAY_OF_WEEK) + " from now " + addDays);
         */
        if (addDays > 0) {
            c.add(Calendar.DAY_OF_WEEK, addDays);
        }
        return c;
    }

    static String formatTime(final Context context, int hour, int minute, Alarm.DaysOfWeek daysOfWeek) {
        Calendar c = calculateAlarm(hour, minute, daysOfWeek);
        return formatTime(context, c);
    }

    /* used by AlarmAlert */
    static String formatTime(final Context context, Calendar c) {
        String format = get24HourMode(context) ? M24 : M12;
        return (c == null) ? "" : (String) DateFormat.format(format, c);
    }

    /**
     * Shows day and time -- used for lock screen
     */
    private static String formatDayAndTime(final Context context, Calendar c) {
        String format = get24HourMode(context) ? DM24 : DM12;
        return (c == null) ? "" : (String) DateFormat.format(format, c);
    }

    /**
     * Save time of the next alarm, as a formatted string, into the system settings so those who care can make use of it.
     */
    static void saveNextAlarm(final Context context, String timeString) {
        Settings.System.putString(context.getContentResolver(), Settings.System.NEXT_ALARM_FORMATTED, timeString);
    }

    /**
     * @return true if clock is set to 24-hour mode
     */
    static boolean get24HourMode(final Context context) {
        return android.text.format.DateFormat.is24HourFormat(context);
    }
}
