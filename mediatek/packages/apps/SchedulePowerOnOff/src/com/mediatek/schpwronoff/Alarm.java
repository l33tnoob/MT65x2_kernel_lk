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

import android.content.Context;
import android.database.Cursor;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.BaseColumns;

import com.mediatek.schpwronoff.R;
import com.mediatek.xlog.Xlog;

import java.text.DateFormatSymbols;
import java.util.Calendar;

public final class Alarm implements Parcelable {
    private static final String TAG = "Settings/Alarm";
    // ////////////////////////////
    // Parcelable apis
    // ////////////////////////////
    public static final Parcelable.Creator<Alarm> CREATOR = new Parcelable.Creator<Alarm>() {
        @Override
        public Alarm createFromParcel(Parcel p) {
            return new Alarm(p);
        }

        @Override
        public Alarm[] newArray(int size) {
            return new Alarm[size];
        }
    };

    /** {@inheritDoc} */
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel p, int flags) {
        p.writeInt(mId);
        p.writeInt(mEnabled ? 1 : 0);
        p.writeInt(mHour);
        p.writeInt(mMinutes);
        p.writeInt(mDaysOfWeek.getCoded());
        p.writeLong(mTime);
        p.writeInt(mVibrate ? 1 : 0);
        p.writeString(mLabel);
        p.writeParcelable(mAlert, flags);
        p.writeInt(mSilent ? 1 : 0);
    }

    // ////////////////////////////
    // end Parcelable apis
    // ////////////////////////////

    // ////////////////////////////
    // Column definitions
    // ////////////////////////////
    public static class Columns implements BaseColumns {
        /**
         * The content:// style URL for this table
         */
        public static final Uri CONTENT_URI = Uri.parse("content://com.android.settings/schpwr");

        /**
         * Hour in 24-hour localtime 0 - 23.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String HOUR = "hour";

        /**
         * Minutes in localtime 0 - 59
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String MINUTES = "minutes";

        /**
         * Days of week coded as integer
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String DAYS_OF_WEEK = "daysofweek";

        /**
         * Alarm time in UTC milliseconds from the epoch.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String ALARM_TIME = "alarmtime";

        /**
         * True if alarm is active
         * <P>
         * Type: BOOLEAN
         * </P>
         */
        public static final String ENABLED = "enabled";

        /**
         * True if alarm should vibrate
         * <P>
         * Type: BOOLEAN
         * </P>
         */
        public static final String VIBRATE = "vibrate";

        /**
         * Message to show when alarm triggers Note: not currently used
         * <P>
         * Type: STRING
         * </P>
         */
        public static final String MESSAGE = "message";

        /**
         * Audio alert to play when alarm triggers
         * <P>
         * Type: STRING
         * </P>
         */
        public static final String ALERT = "alert";

        /**
         * The default sort order for this table
         */
        public static final String DEFAULT_SORT_ORDER = _ID + " ASC";

        // Used when filtering enabled alarms.
        public static final String WHERE_ENABLED = ENABLED + "=1";

        static final String[] ALARM_QUERY_COLUMNS = { _ID, HOUR, MINUTES, DAYS_OF_WEEK, ALARM_TIME, ENABLED, VIBRATE,
                MESSAGE, ALERT };

        /**
         * These save calls to cursor.getColumnIndexOrThrow() THEY MUST BE KEPT IN SYNC WITH ABOVE QUERY COLUMNS
         */
        public static final int ALARM_ID_INDEX = 0;
        public static final int ALARM_HOUR_INDEX = 1;
        public static final int ALARM_MINUTES_INDEX = 2;
        public static final int ALARM_DAYS_OF_WEEK_INDEX = 3;
        public static final int ALARM_TIME_INDEX = 4;
        public static final int ALARM_ENABLED_INDEX = 5;
        public static final int ALARM_VIBRATE_INDEX = 6;
        public static final int ALARM_MESSAGE_INDEX = 7;
        public static final int ALARM_ALERT_INDEX = 8;
    }

    // ////////////////////////////
    // End column definitions
    // ////////////////////////////

    // Public fields
    public int mId;
    public boolean mEnabled;
    public int mHour;
    public int mMinutes;
    public DaysOfWeek mDaysOfWeek;
    public long mTime;
    public boolean mVibrate;
    public String mLabel;
    public Uri mAlert;
    public boolean mSilent;

    /**
     * Alarm object constructor
     * @param c Cursor
     */
    public Alarm(Cursor c) {
        mId = c.getInt(Columns.ALARM_ID_INDEX);
        mEnabled = c.getInt(Columns.ALARM_ENABLED_INDEX) == 1;
        mHour = c.getInt(Columns.ALARM_HOUR_INDEX);
        mMinutes = c.getInt(Columns.ALARM_MINUTES_INDEX);
        mDaysOfWeek = new DaysOfWeek(c.getInt(Columns.ALARM_DAYS_OF_WEEK_INDEX));
        mTime = c.getLong(Columns.ALARM_TIME_INDEX);
        mVibrate = c.getInt(Columns.ALARM_VIBRATE_INDEX) == 1;
        mLabel = c.getString(Columns.ALARM_MESSAGE_INDEX);
        String alertString = c.getString(Columns.ALARM_ALERT_INDEX);
        if (Alarms.ALARM_ALERT_SILENT.equals(alertString)) {
            Xlog.d(TAG, "Alarm is marked as silent");
            mSilent = true;
        } else {
            if (alertString != null && alertString.length() != 0) {
                mAlert = Uri.parse(alertString);
            }

            // If the database alert is null or it failed to parse, use the
            // default alert.
            if (mAlert == null) {
                mAlert = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_ALARM);
            }
        }
    }

    /**
     * contruct alarm object from parcel
     * @param p Parcel
     */
    public Alarm(Parcel p) {
        mId = p.readInt();
        mEnabled = p.readInt() == 1;
        mHour = p.readInt();
        mMinutes = p.readInt();
        mDaysOfWeek = new DaysOfWeek(p.readInt());
        mTime = p.readLong();
        mVibrate = p.readInt() == 1;
        mLabel = p.readString();
        mAlert = (Uri) p.readParcelable(null);
        mSilent = p.readInt() == 1;
    }

    /*
     * Days of week code as a single int. 0x00: no day 0x01: Monday 0x02: Tuesday 0x04: Wednesday 0x08: Thursday 0x10: Friday
     * 0x20: Saturday 0x40: Sunday
     */
    static final class DaysOfWeek {

        private static final int NO_DAY_BIT = 0x00;
        private static final int EVERY_DAY_BIT = 0x7f;
        private static final int WEEK_DAYS = 7;
        private static final int[] DAY_MAP = new int[] { Calendar.MONDAY, Calendar.TUESDAY, Calendar.WEDNESDAY,
                Calendar.THURSDAY, Calendar.FRIDAY, Calendar.SATURDAY, Calendar.SUNDAY, };

        // Bitmask of all repeating days
        private int mDays;

        DaysOfWeek(int days) {
            mDays = days;
        }

        public String toString(Context context, boolean showNever) {
            StringBuilder ret = new StringBuilder();

            // no days
            if (mDays == NO_DAY_BIT) {
                return showNever ? context.getText(R.string.never).toString() : "";
            }

            // every day
            if (mDays == EVERY_DAY_BIT) {
                return context.getText(R.string.every_day).toString();
            }

            // count selected days
            int dayCount = 0;
            int days = mDays;
            while (days > 0) {
                if ((days & 1) == 1) {
                    dayCount++;
                }
                days >>= 1;
            }

            // short or long form?
            DateFormatSymbols dfs = new DateFormatSymbols();
            String[] dayList = (dayCount > 1) ? dfs.getShortWeekdays() : dfs.getWeekdays();

            // selected days
            for (int i = 0; i < WEEK_DAYS; i++) {
                if ((mDays & (1 << i)) != 0) {
                    ret.append(dayList[DAY_MAP[i]]);
                    dayCount -= 1;
                    if (dayCount > 0) {
                        ret.append(context.getText(R.string.day_concat));
                    }
                }
            }
            return ret.toString();
        }

        private boolean isSet(int day) {
            return ((mDays & (1 << day)) > 0);
        }

        public void set(int day, boolean set) {
            if (set) {
                mDays |= (1 << day);
            } else {
                mDays &= ~(1 << day);
            }
        }

        public void set(DaysOfWeek dow) {
            mDays = dow.mDays;
        }

        public int getCoded() {
            return mDays;
        }

        // Returns days of week encoded in an array of booleans.
        public boolean[] getBooleanArray() {
            boolean[] ret = new boolean[WEEK_DAYS];
            for (int i = 0; i < WEEK_DAYS; i++) {
                ret[i] = isSet(i);
            }
            return ret;
        }

        public boolean isRepeatSet() {
            return mDays != 0;
        }

        /**
         * returns number of days from today until next alarm
         * 
         * @param c
         *            must be set to today
         */
        public int getNextAlarm(Calendar c) {
            final int days = 5;
            if (mDays == 0) {
                return -1;
            }

            int today = (c.get(Calendar.DAY_OF_WEEK) + days) % WEEK_DAYS;

            int day = 0;
            int dayCount = 0;
            for (; dayCount < WEEK_DAYS; dayCount++) {
                day = (today + dayCount) % WEEK_DAYS;
                if (isSet(day)) {
                    break;
                }
            }
            return dayCount;
        }
    }
}
