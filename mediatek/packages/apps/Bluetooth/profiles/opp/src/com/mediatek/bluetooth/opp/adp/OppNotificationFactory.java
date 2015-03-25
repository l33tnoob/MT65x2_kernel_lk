/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.bluetooth.opp.adp;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.UserHandle;
import android.text.format.Formatter;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.BluetoothReceiver;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.opp.mmi.OppCancelActivity;
import com.mediatek.bluetooth.opp.mmi.OppLog;
import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;
import com.mediatek.bluetooth.share.BluetoothShareTask.Direction;

/**
 * @author Jerry Hsu
 */
public class OppNotificationFactory {

    private static final Intent NULL_INTENT = new Intent();

    public static final int NID_TRANSFER_MGMT = BluetoothProfile.BT_PROFILE_RANGE - 1;

    /**
     * get profile specific notification
     *
     * @param id
     * @return
     */
    // protected static Notification getNotification( int id ){
    //
    // int pid = NotificationFactory.getProfileNotificationId(
    // BluetoothProfile.ID_OPP, id );
    // return NotificationFactory.getProfileNotification( pid );
    // }

    /**
     * get opp simple notification
     *
     * @param id
     * @param iconDrawableId
     * @param tickerText
     * @param contentTitle
     * @param contentText
     * @param flags
     * @param intent
     * @param context
     * @return
     */
    protected static Notification getSimpleNotification(int id, int iconDrawableId, long when,
            String tickerText, String contentTitle, String contentText, PendingIntent intent,
            Context context) {

        // // notification object
        // Notification n = getNotification( id );
        // n.icon = iconDrawableId;
        // n.when = when;
        // n.tickerText = tickerText;
        // n.flags = flags;
        // n.setLatestEventInfo( context, contentTitle, contentText, intent );
        // return n;

        Notification.Builder b = new Notification.Builder(context);
        b.setSmallIcon(iconDrawableId);
        b.setWhen(when);
        b.setTicker(tickerText);
        b.setContentTitle(contentTitle);
        b.setContentText(contentText);
        b.setContentIntent(intent);
        b.setOngoing(true);
        return b.getNotification();
    }

    /**
     * get opp progress notification
     *
     * @param id
     * @param iconDrawableId
     * @param tickerText
     * @param progressText
     * @param message
     * @param total
     * @param progress
     * @param flags
     * @param intent
     * @return
     */
    protected static Notification getProgressNotification(int id, int iconDrawableId, long when,
            String tickerText, String progressText, String message, int total, int progress,
            PendingIntent intent, Context context) {

        // notification object
        // Notification n = getNotification( id );
        // n.icon = iconDrawableId;
        // n.when = when;
        // n.tickerText = tickerText;
        // n.flags = flags;
        // n.contentIntent = intent;
        //
        // // view
        // RemoteViews expandedView = new RemoteViews(
        // Options.APPLICATION_PACKAGE_NAME,
        // R.layout.bt_opp_notification_progress );
        // expandedView.setImageViewResource( R.id.notification_icon,
        // iconDrawableId );
        // expandedView.setTextViewText( R.id.progress_text, progressText );
        // expandedView.setTextViewText( R.id.description, message );
        // expandedView.setProgressBar( R.id.progress_bar, total, progress,
        // false );
        // n.contentView = expandedView;
        //
        // return n;

        Notification.Builder b = new Notification.Builder(context);
        b.setSmallIcon(iconDrawableId);
        b.setWhen(when);
        b.setTicker(tickerText);
        b.setContentInfo(progressText);
        b.setContentTitle(message);
        b.setContentIntent(intent);
        b.setProgress(total, progress, (total < 1));
        b.setOngoing(true);
        return b.getNotification();
    }

    /**
     * Incoming Notification
     *
     * @param context
     * @param task
     * @return
     */
    public static Notification getOppIncomingNotification(Context context, BluetoothShareTask task) {

        // prepare Intent
        Intent intent = new Intent(OppConstants.OppsAccessRequest.ACTION_PUSH_REQUEST);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // required for
                                                        // startActivity()
                                                        // outside the Activity
        intent.putExtra(OppConstants.OppsAccessRequest.EXTRA_PEER_NAME, task.getPeerName());
        intent.putExtra(OppConstants.OppsAccessRequest.EXTRA_OBJECT_NAME, task.getObjectName());
        intent.putExtra(OppConstants.OppsAccessRequest.EXTRA_TOTAL_BYTES, task.getTotalBytes());
        intent.putExtra(OppConstants.OppsAccessRequest.EXTRA_TASK_ID, task.getId());

        Notification n = getSimpleNotification(task.getId(), R.drawable.bt_opp_in, task
                .getCreationDate(), context
                .getString(R.string.bt_opp_notification_in_coming_ticker), context.getString(
                R.string.bt_opp_notification_in_coming_title, task.getPeerName()), context
                .getString(R.string.bt_opp_notification_in_coming_message), PendingIntent
                .getActivityAsUser(context, task.getId(), intent, PendingIntent.FLAG_ONE_SHOT, null, UserHandle.CURRENT), context);

        n.flags |= Notification.FLAG_ONGOING_EVENT;
        n.defaults = (Notification.DEFAULT_SOUND | Notification.DEFAULT_VIBRATE);
        return n;
    }

    /**
     * Ongoing Notification
     *
     * @param context
     * @param task
     * @return
     */
    public static Notification getOppOngoingNotification(Context context, BluetoothShareTask task) {

        int icon;
        int ticker;
        int message;
        Direction direction = task.getDirection();
        if (direction == Direction.in) {

            icon = R.drawable.bt_opp_in;
            ticker = R.string.bt_opp_notification_in_ongoing_ticker;
            message = R.string.bt_opp_notification_in_ongoing_message;
        } else if (direction == Direction.out) {
            icon = R.drawable.bt_opp_out;
            ticker = R.string.bt_opp_notification_out_ongoing_ticker;
            message = R.string.bt_opp_notification_out_ongoing_message;
        } else {
            OppLog.e("unsupport feature for getOppOngoingNotification():" + direction.toString());
            throw new IllegalArgumentException("unsupported feature: " + direction.toString());
        }

        long total = task.getTotalBytes();
        int done = (total == 0) ? 0 : (int) (task.getDoneBytes() * 100L / total);

        return getProgressNotification(task.getId(), icon, task.getCreationDate(), context
                .getString(ticker), (done + "%"), context.getString(message, task.getObjectName(),
                Formatter.formatFileSize(context, total)), 100, done, createCancelIntent(context,
                task.getId()), context);
    }

    /**
     * @param context
     * @param id
     * @return
     */
    private static PendingIntent createCancelIntent(Context context, int id) {

        Intent intent = new Intent(context, OppCancelActivity.class);
        intent.setData(Uri.withAppendedPath(BluetoothShareTaskMetaData.CONTENT_URI, Integer
                .toString(id)));
        return PendingIntent.getActivityAsUser(context, 0, intent, 0, null, UserHandle.CURRENT);
    }

    private static PendingIntent createCancelingIntent(Context context, int taskId) {

        Intent intent = new Intent(BluetoothReceiver.ACTION_SHOW_TOAST);
        intent.putExtra(BluetoothReceiver.EXTRA_TEXT, context
                .getString(R.string.bt_opp_canceling_task_message));
        return PendingIntent.getBroadcastAsUser(context, taskId, intent,
                PendingIntent.FLAG_UPDATE_CURRENT, UserHandle.CURRENT);
    }
}
