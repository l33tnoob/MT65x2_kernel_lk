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

package com.mediatek.videoplayer;

import android.content.Context;
import android.text.format.Formatter;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

public class CachedVideoInfo {
    private static final String TAG = "CachedVideoInfo";
    private static final String COLON = ":";
    private static final String ZERO = "0";
    private static final int THOUSAND = 1000;
    private static final int ONE_MINUTE = 60;
    private static final int ONE_HOUR = 3600;
    private static final int TEN = 10;
    
    private Locale mLocale;
    private final HashMap<Long, String> mDurations = new HashMap<Long, String>();
    private final HashMap<Long, String> mDateTimes = new HashMap<Long, String>();
    private final HashMap<Long, String> mFileSizes = new HashMap<Long, String>();
    private boolean mCanOpitmized;
    private final ArrayList<Locale> mCanOptimizedLocales = new ArrayList<Locale>();
    
    public CachedVideoInfo() {
        mCanOptimizedLocales.add(Locale.ENGLISH);
        mCanOptimizedLocales.add(Locale.CHINA);
        mCanOptimizedLocales.add(Locale.TAIWAN);
        mCanOptimizedLocales.add(Locale.UK);
        mCanOptimizedLocales.add(Locale.US);
        mCanOptimizedLocales.add(Locale.FRANCE);
        mCanOptimizedLocales.add(Locale.GERMANY);
        mCanOptimizedLocales.add(Locale.ITALY);
        setLocale(Locale.getDefault());
    }
    
    public synchronized void setLocale(final Locale locale) {
        MtkLog.v(TAG, "setLocale(" + locale + ") mLocale=" + mLocale + ", mCanOpitmized=" + mCanOpitmized);
        if (locale == null) {
            mDateTimes.clear();
            mDurations.clear();
            mFileSizes.clear();
        } else {
            if (!locale.equals(mLocale)) {
                mLocale = locale;
                mDateTimes.clear();
                mFileSizes.clear();
                boolean newOptimized = false;
                if (mCanOptimizedLocales.contains(mLocale)) {
                    newOptimized = true;
                }
                if (!mCanOpitmized || !newOptimized) {
                    mCanOpitmized = newOptimized;
                    mDurations.clear();
                }
            }
        }
        MtkLog.v(TAG, "setLocale() mCanOpitmized=" + mCanOpitmized);
    }
    
    public synchronized String getFileSize(final Context context, final Long size) {
        String fileSize = mFileSizes.get(size);
        if (fileSize == null) {
            fileSize = Formatter.formatFileSize(context, size);
            mFileSizes.put(size, fileSize);
        }
        return fileSize;
    }
    
    public synchronized String getTime(final Long millis) {
        String time = mDateTimes.get(millis);
        if (time == null) {
            time = MtkUtils.localTime(millis);
            mDateTimes.put(millis, time);
        }
        return time;
    }
    
    public synchronized String getDuration(final Long millis) {
        String duration = mDurations.get(millis);
        if (duration == null) {
            if (mCanOpitmized) {
                duration = stringForDurationOptimized(millis);
            } else {
                duration = MtkUtils.stringForTime(millis);
            }
            mDurations.put(millis, duration);
        }
        return duration;
    }
    
    private String stringForDurationOptimized(final long millis) {
        final int totalSeconds = (int) millis / THOUSAND;
        final int seconds = totalSeconds % ONE_MINUTE;
        final int minutes = (totalSeconds / ONE_MINUTE) % ONE_MINUTE;
        final int hours = totalSeconds / ONE_HOUR;
        //optimize format time, but not support special language
        final StringBuilder builder = new StringBuilder(10);
        if (hours > 0) {
            builder.append(hours)
            .append(COLON);
        }
        if (minutes < TEN) {
            builder.append(ZERO);
        }
        builder.append(minutes);
        builder.append(COLON);
        if (seconds < TEN) {
            builder.append(ZERO);
        }
        builder.append(seconds);
        return builder.toString();
    }
}
