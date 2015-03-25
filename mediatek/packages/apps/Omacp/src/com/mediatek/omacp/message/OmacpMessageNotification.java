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

package com.mediatek.omacp.message;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.Cursor;
import android.graphics.Typeface;
import android.media.AudioManager;
import android.net.Uri;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.style.StyleSpan;

import com.google.android.mms.util.SqliteWrapper;
import com.mediatek.omacp.R;
import com.mediatek.omacp.provider.OmacpProviderDatabase;
import com.mediatek.omacp.utils.MTKlog;

import java.util.Comparator;
import java.util.HashSet;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

public class OmacpMessageNotification {

    private static final String TAG = "Omacp/OmacpMessageNotification";

    private static final boolean DEBUG = true;

    private static final int NOTIFICATION_ID = 126;

    private static final Uri URL_MESSAGES = OmacpProviderDatabase.CONTENT_URI;

    // This must be consistent with the column constants below.
    private static final String[] OMACP_STATUS_PROJECTION = new String[] {
            OmacpProviderDatabase._ID, OmacpProviderDatabase.DATE, OmacpProviderDatabase.TITLE,
            OmacpProviderDatabase.SUMMARY
    };

    private static final int COLUMN_ID = 0;

    private static final int COLUMN_DATE = 1;

    private static final int COLUMN_TITLE = 2;

    private static final int COLUMN_SUMMARY = 3;

    private static final String NEW_INCOMING_SM_CONSTRAINT = "(" + OmacpProviderDatabase.SEEN
            + " = 0)";

    public static final String NOTIFICATION_ENABLED = "pref_key_enable_notifications";

    public static final String NOTIFICATION_VIBRATE = "pref_key_vibrate";

    public static final String NOTIFICATION_RINGTONE = "pref_key_ringtone";

    private static final OmacpNotificationInfoComparator INFO_COMPARATOR = new OmacpNotificationInfoComparator();

    private OmacpMessageNotification() {
    }

    /**
     * Checks to see if there are any "unseen" messages or delivery reports.
     * Shows the most recent notification if there is one. Does its work and
     * query in a worker thread.
     *
     * @param context the context to use
     */

    public static void nonBlockingUpdateNewMessageIndicator(final Context context,
            final boolean isNew) {
        if (DEBUG) {
            MTKlog.d(TAG, " OmacpMessageNotification nonBlockingUpdateNewMessageIndicator");
        }

        new Thread(new Runnable() {
            public void run() {
                blockingUpdateNewMessageIndicator(context, isNew);
            }
        }).start();
    }

    /**
     * Checks to see if there are any unread messages or delivery reports. Shows
     * the most recent notification if there is one.
     *
     * @param context the context to use
     * @param isNew if notify a new message comes, it should be true, otherwise,
     *            false.
     */
    public static void blockingUpdateNewMessageIndicator(Context context, boolean isNew) {
        SortedSet<OmacpNotificationInfo> accumulator = new TreeSet<OmacpNotificationInfo>(
                INFO_COMPARATOR);
        Set<Long> threads = new HashSet<Long>(4);
        int count = 0;
        count += accumulateNotificationInfo(accumulator,
                getOmacpNewMessageNotificationInfo(context));

        cancelNotification(context, NOTIFICATION_ID);
        if (!accumulator.isEmpty()) {
            accumulator.first().deliver(context, isNew, count, threads.size());
        }
    }

    public static void updateAllNotifications(Context context) {
        nonBlockingUpdateNewMessageIndicator(context, false);
    }

    private static int accumulateNotificationInfo(SortedSet set, OmacpNotificationInfo info) {
        if (info != null) {
            set.add(info);
            return info.mCount;
        }
        return 0;
    }

    private static final class OmacpNotificationInfo {
        public Intent mClickIntent;

        public String mDescription;

        public int mIconResourceId;

        public CharSequence mTicker;

        public long mTimeMillis;

        public String mTitle;

        public int mCount;

        public OmacpNotificationInfo(Intent clickIntent, String description, int iconResourceId,
                CharSequence ticker, long timeMillis, String title, int count) {
            mClickIntent = clickIntent;
            mDescription = description;
            mIconResourceId = iconResourceId;
            mTicker = ticker;
            mTimeMillis = timeMillis;
            mTitle = title;
            mCount = count;
        }

        public void deliver(Context context, boolean isNew, int count, int uniqueThreads) {
            updateNotification(context, mClickIntent, mDescription, mIconResourceId, isNew,
                    (isNew ? mTicker : null), // only display the ticker if the
                                              // message is new
                    mTimeMillis, mTitle, count, uniqueThreads);
        }

        public long getTime() {
            return mTimeMillis;
        }
    }

    private static final class OmacpNotificationInfoComparator implements
            Comparator<OmacpNotificationInfo> {
        public int compare(OmacpNotificationInfo info1, OmacpNotificationInfo info2) {
            return Long.signum(info2.getTime() - info1.getTime());
        }
    }

    public static final OmacpNotificationInfo getOmacpNewMessageNotificationInfo(Context context) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = SqliteWrapper.query(context, resolver, URL_MESSAGES,
                OMACP_STATUS_PROJECTION, NEW_INCOMING_SM_CONSTRAINT, null,
                OmacpProviderDatabase.DATE + " desc");

        if (cursor == null) {
            return null;
        }

        try {
            if (!cursor.moveToFirst()) {
                return null;
            }
            // TODO get the description by query the table.
            long messageId = cursor.getLong(COLUMN_ID);
            long timeMillis = cursor.getLong(COLUMN_DATE) * 1000;
            String title = cursor.getString(COLUMN_TITLE);
            String summary = OmacpMessageUtils
                    .getSummary(context, cursor.getString(COLUMN_SUMMARY));
            String messageType = context.getString(R.string.configuration_message);

            OmacpNotificationInfo info = getNewMessageNotificationInfo(context, messageId, title,
                    summary, messageType, R.drawable.status_notify, timeMillis, cursor.getCount());
            return info;
        } finally {
            cursor.close();
        }
    }

    private static OmacpNotificationInfo getNewMessageNotificationInfo(Context context,
            long messageId, String title, String summary, String messageType, int iconResourceId,
            long timeMillis, int count) {

        // Intent clickIntent = OmacpMessageList.createIntent(context,
        // messageId);
        Intent clickIntent = new Intent(context, OmacpMessageList.class);
        clickIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);

        String senderInfo = buildTickerMessage(context, messageType, null).toString();
        String senderInfoName = senderInfo.substring(0, senderInfo.length() - 2);
        CharSequence ticker = buildTickerMessage(context, messageType, summary);

        return new OmacpNotificationInfo(clickIntent, summary, iconResourceId, ticker, timeMillis,
                senderInfoName, count);
    }

    public static void cancelNotification(Context context, int notificationId) {
        NotificationManager nm = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);

        nm.cancel(notificationId);
    }

    private static void updateNotification(Context context, Intent clickIntent, String description,
            int iconRes, boolean isNew, CharSequence ticker, long timeMillis, String title,
            int messageCount, int uniqueThreadCount) {

        Context otherAppContext = null;
        try {
            otherAppContext = context.createPackageContext("com.android.mms",
                    Context.CONTEXT_IGNORE_SECURITY);
        } catch (NameNotFoundException e) {
            MTKlog.e(TAG, "OmacpMessageNotification NameNotFoundException.");
            return;
        }
        if (otherAppContext == null) {
            MTKlog.e(TAG, "OmacpMessageNotification otherAppContext is null.");
            return;
        }

        SharedPreferences sp = otherAppContext.getSharedPreferences("com.android.mms_preferences",
                context.MODE_WORLD_READABLE);
        if (sp == null) {
            MTKlog.e(TAG, "OmacpMessageNotification sp is null......");
            return;
        }

        if (!sp.getBoolean(NOTIFICATION_ENABLED, true)) {
            return;
        }

        Notification notification = new Notification(iconRes, ticker, timeMillis);

        // If we have more than one unique thread, change the title (which would
        // normally be the contact who sent the message) to a generic one that
        // makes sense for multiple senders, and change the Intent to take the
        // user to the conversation list instead of the specific thread.
        Intent intent = null;
        String messageTitle;
        if (uniqueThreadCount > 1) {
            messageTitle = context.getString(R.string.notification_multiple_title);
            intent = new Intent(Intent.ACTION_MAIN);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);

            intent.setType("vnd.android-dir/mms-sms");
        } else {
            messageTitle = title;
            intent = clickIntent;
        }

        // If there is more than one message, change the description (which
        // would normally be a snippet of the individual message text) to
        // a string indicating how many unread messages there are.
        String messageDescription;
        if (messageCount > 1) {
            messageDescription = context.getString(R.string.notification_multiple,
                    Integer.toString(messageCount));
        } else {
            messageDescription = description;
        }

        // Make a startActivity() PendingIntent for the notification.
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        // Update the notification.
        notification.setLatestEventInfo(context, messageTitle, messageDescription, pendingIntent);

        if (isNew) {
            AudioManager audioManager = (AudioManager) context
                    .getSystemService(Context.AUDIO_SERVICE);

            boolean vibrate = audioManager
                    .getVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION) == AudioManager.VIBRATE_SETTING_ON;
            if (vibrate) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }

            String ringtoneStr = sp.getString(NOTIFICATION_RINGTONE, null);
            notification.sound = TextUtils.isEmpty(ringtoneStr) ? null : Uri.parse(ringtoneStr);
        }

        notification.flags |= Notification.FLAG_SHOW_LIGHTS;
        notification.ledARGB = 0xff00ff00;
        notification.ledOnMS = 500;
        notification.ledOffMS = 2000;

        NotificationManager nm = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);

        nm.notify(NOTIFICATION_ID, notification);
    }

    protected static CharSequence buildTickerMessage(Context context, String messageType,
            String summary) {

        StringBuilder buf = new StringBuilder(messageType);
        buf.append(':').append(' ');

        int offset = buf.length();

        if (!TextUtils.isEmpty(summary)) {
            String messageSummary = "";
            messageSummary = summary.replace('\n', ' ').replace('\r', ' ');
            buf.append(messageSummary);
        }

        SpannableString spanText = new SpannableString(buf.toString());
        spanText.setSpan(new StyleSpan(Typeface.BOLD), 0, offset,
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        return spanText;
    }

}
