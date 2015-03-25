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

package com.mediatek.bluetooth.share;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;

import com.mediatek.bluetooth.R;

public class BluetoothShareNotification {

    /**
     * Create Share Management Notification (nid: NotificationFactory.NID_SHARE_MGMT_NOTIFICATION)
     *
     * @param context
     * @return
     */
    public static Notification getShareManagementNotification(Context context) {

        // create pending intent to open BluetoothShareMgmtActivity
        Intent intent = new Intent(context, BluetoothShareMgmtActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        // create notification object
        // Notification n = NotificationFactory.getProfileNotification(
        // NotificationFactory.NID_SHARE_MGMT_NOTIFICATION );
        // n.icon = R.drawable.bt_share;
        // n.tickerText = context.getString(
        // R.string.bt_share_mgmt_notification_ticker );
        // n.setLatestEventInfo( context,
        // context.getString( R.string.bt_share_mgmt_notification_title ),
        // context.getString( R.string.bt_share_mgmt_notification_message ),
        // PendingIntent.getActivity( context, 0, intent, 0 ) );
        // return n;

        Notification.Builder b = new Notification.Builder(context);
        b.setSmallIcon(R.drawable.bt_share);
        b.setTicker(context.getString(R.string.bt_share_mgmt_notification_ticker));
        b.setContentTitle(context.getString(R.string.bt_share_mgmt_notification_title));
        b.setContentText(context.getString(R.string.bt_share_mgmt_notification_message));
        b.setContentIntent(PendingIntent.getActivityAsUser(context, 0, intent, 0, null, UserHandle.CURRENT));
        return b.getNotification();

        // set intent for "Clear"
        // Intent intent = new Intent( Constants.ACTION_COMPLETE_HIDE );
        // intent.setClassName( Options.APPLICATION_PACKAGE_NAME,
        // BluetoothOppReceiver.class.getName());
        // n.deleteIntent = PendingIntent.getBroadcast( context, 0, intent, 0 );
    }
}
