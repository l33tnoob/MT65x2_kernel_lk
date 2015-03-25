/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.activities.widgets;

import android.content.Context;
import android.content.res.Resources;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.AttributeSet;
import android.widget.TextView;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;

import java.util.Date;

/*
 * this class is used for format date or time relevant to settings profile
 */
public class DateView extends TextView {
    public static final String TAG = "DateView";
    private static final int RATIO = 1000;
    private static final int SECONDS_ONE_DAY = 60 * 60 * 24;
    private static final String BLANK_STRING = "";

    /**
     * This enumerate defines the rule on how to generate the date time string
     */
    private static enum DATE_TIME_OPTION {
        DATE, // Just need to show the date, normally used in date index
        TIME, // Show the time for today, otherwise the date, used in chat list
        TIME_ONLY
        // Only show the time without date, used in a single chat item
    }

    private static Resources sResources = null;

    static {
        Context context = AndroidFactory.getApplicationContext();
        if (null != context) {
            Logger.d(TAG, "static statement: initialize resources");
            sResources = context.getResources();
        } else {
            Logger.e(TAG, "static statement: context is null");
        }
    }

    public DateView(Context context) {
        super(context);
    }

    public DateView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public DateView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public void setDate(Date date, boolean isTop) {
        if (isTop) {
            int left = 0;
            int top = getIndexPaddingTop();
            int right = 0;
            int bottom = 0;
            setPadding(left, top, right, bottom);
        }
        String dateValue = convertDate(date, mContext);
        Logger.d(TAG, "setDate(), dateValue is " + dateValue);
        setText(dateValue);
    }

    private static int sIndexPaddingTop = -1;

    private int getIndexPaddingTop() {
        if (-1 == sIndexPaddingTop) {
            sIndexPaddingTop =
                    sResources.getDimensionPixelOffset(R.dimen.date_view_index_padding_top);
        }
        return sIndexPaddingTop;
    }

    /**
     * Convert the date to the date style aligned with the format in Settings.
     * 
     * @param date The date to be converted.
     * @return The time after formatted.
     */
    public static String convertDate(Date date, Context context) {
        Logger.d(TAG, "convertDate() entry, date: " + date);
        return generateString(date, context, DATE_TIME_OPTION.DATE);
    }

    /**
     * Convert the date to the time style aligned with the format in Settings.
     * 
     * @param date The date to be converted
     * @return the time formatted.
     */
    public String convertTime(Date date) {
        Logger.d(TAG, "convertTime() entry, date: " + date);
        return convertTime(date, true);
    }

    private String convertTime(Date date, boolean isTimeOnly) {
        Logger.d(TAG, "convertTime() entry, date: " + date);
        if (isTimeOnly) {
            return generateString(date, mContext, DATE_TIME_OPTION.TIME_ONLY);
        } else {
            return generateString(date, mContext, DATE_TIME_OPTION.TIME);
        }
    }

    private static String generateString(Date date, Context context, DATE_TIME_OPTION option) {
        Logger.d(TAG, "generateString() entry, date: " + date);
        if (null == date) {
            Logger.w(TAG, "generateString() invalid date");
            return null;
        }
        String timeString = null;
        int formatFlags =
                DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
                        | DateUtils.FORMAT_CAP_AMPM;
        long thenMillis = date.getTime();
        Time then = new Time();
        then.set(thenMillis);
        switch (option) {
            case TIME_ONLY:
                formatFlags |= DateUtils.FORMAT_SHOW_TIME;
                timeString = DateUtils.formatDateTime(context, thenMillis, formatFlags);
                break;
            case DATE:
            case TIME:
                Time now = new Time();
                now.setToNow();
                now.set(0, 0, 0, now.monthDay, now.month, now.year);
                long todayMillis = now.toMillis(false);

                // Basic settings for formatDateTime() we want for all cases.
                if (isToday(todayMillis, thenMillis)) {
                    if (DATE_TIME_OPTION.TIME.equals(option)) {
                        formatFlags |= DateUtils.FORMAT_SHOW_TIME;
                        timeString = DateUtils.formatDateTime(context, thenMillis, formatFlags);
                    } else {
                        timeString = getToday();
                    }
                } else if (isYesterday(todayMillis, thenMillis)) {
                    timeString = getYesterday();
                } else if (now.year == then.year) {
                    formatFlags |= DateUtils.FORMAT_SHOW_DATE;
                    timeString = DateUtils.formatDateTime(context, thenMillis, formatFlags);
                } else {
                    formatFlags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
                    timeString = DateUtils.formatDateTime(context, thenMillis, formatFlags);
                }
                break;
            default:
                break;
        }
        Logger.d(TAG, "generateString() exit, timeString: " + timeString);
        return timeString;
    }

    private static boolean isToday(long todayMillis, long thenMillis) {
        int compareTime = (int) ((thenMillis - todayMillis) / RATIO);
        return compareTime > 0 && compareTime < SECONDS_ONE_DAY;
    }

    private static boolean isYesterday(long todayMillis, long thenMillis) {
        int compareTime = (int) ((todayMillis - thenMillis) / RATIO);
        return compareTime > 0 && compareTime < SECONDS_ONE_DAY;
    }

    private static String getToday() {
        return sResources.getString(R.string.text_today);
    }

    private static String getYesterday() {
        return sResources.getString(R.string.text_yesterday);
    }

    /**
     * Set the date to display as system time format
     * 
     * @param date The date to be set
     */
    public void setTime(Date date) {
        setTime(date, true);
    }

    /**
     * Set the date to display as system time format
     * 
     * @param date The date to be set
     * @param isTimeOnly Whether just show the time without date
     */
    public void setTime(Date date, boolean isTimeOnly) {
        Logger.d(TAG, "setTime() entry, date: " + date + ", isTimeOnly: " + isTimeOnly);
        if (date != null) {
            String timeString = convertTime(date, isTimeOnly);
            Logger.d(TAG, "setTime() timeString: " + timeString);
            setText(timeString);
        } else {
            setText(BLANK_STRING);
            Logger.e(TAG, "setTime the date is null");
        }
    }
}
