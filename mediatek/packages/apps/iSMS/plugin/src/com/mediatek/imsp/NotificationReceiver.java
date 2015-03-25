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

package com.mediatek.imsp;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.AsyncTask;

import com.hissage.api.NmsIpMessageApi;
import com.hissage.contact.NmsContact;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageStatus;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.mms.ipmessage.IpMessageConsts;

import com.mediatek.xlog.Xlog;

/**
 * NotificationReceiver receives some kinds of ip message notification and call
 * {@link com.mediatek.imsp.NotificationManagerExt NotificationManagerExt} to
 * notify the listeners.
 */
public class NotificationReceiver extends BroadcastReceiver {
    private static final String TAG = "imsp/NotificationReceiver";
    private static NotificationReceiver sInstance;

    public static NotificationReceiver getInstance() {
        if (sInstance == null) {
            sInstance = new NotificationReceiver();
        }
        return sInstance;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Xlog.d(TAG, "onReceive: " + "Action = " + intent.getAction());
        new ReceiveNotificationTask(context).execute(intent);
    }

    private class ReceiveNotificationTask extends AsyncTask<Intent, Void, Void> {
        private Context mContext = null;

        ReceiveNotificationTask(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Intent... intents) {
            Intent intent = intents[0];
            String action = intent.getAction();
            if (NmsIpMessageConsts.NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION.equals(action)) {
                handleNewMessage(intent);
            } else if (NmsIpMessageConsts.NMS_INTENT_SERVICE_READY.equals(action)) {
                handleServiceReady();
            } else if (NmsIpMessageConsts.NmsImStatus.NMS_IM_STATUS_ACTION.equals(action)) {
                handleImStatus(intent);
            } else if (NmsIpMessageConsts.NmsIpMessageStatus.NMS_MESSAGE_STATUS_ACTION
                    .equals(action)) {
                handleIpMessageStatus(intent);
            } else if (NmsIpMessageConsts.NmsIpMessageStatus.NMS_READEDBURN_TIME_ACTION
                    .equals(action)) {
                handleReadedBurnTime(intent);
            } else if (NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION
                    .equals(action)) {
                handleDownloadAttachStatus(intent);
            } else if (NmsIpMessageConsts.NmsSaveHistory.NMS_ACTION_DOWNLOAD_HISTORY.equals(action)) {
                handleSaveHistory(intent);
            } else if (NmsIpMessageConsts.NmsUpdateGroupAction.NMS_UPDATE_GROUP.equals(action)) {
                handleUpdateGroup(intent);
            } else if (NmsIpMessageConsts.NmsUpdateSystemContactAction.NMS_UPDATE_CONTACT
                    .equals(action)) {
                handleUpdateContact(intent);
            } else if (NmsIpMessageConsts.NmsSimInfoChanged.NMS_SIM_INFO_ACTION.equals(action)) {
                handleSimInfoChanged(intent);
            } else if (NmsConsts.NmsIntentStrId.NMS_MMS_RESTART_ACTION.equals(action)) {
                handleUpgrade();
            } else if (NmsIntentStrId.NMS_REG_STATUS.equals(action)) {
                handleRegStatus(intent);
            } else if (NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_STATUS_ACTION.equals(action)) {
                handleStoreStatus(intent);
            } else {
                Xlog.w(TAG, "unknown notification type.");
            }

            return null;
        }

        private void handleNewMessage(Intent intent) {
            Xlog.d(TAG, "handleNewMessage");
            NmsIpMessage msg = (NmsIpMessage) intent
                    .getSerializableExtra(NmsIpMessageConsts.NmsNewMessageAction.NMS_IP_MESSAGE);
            boolean needNotify = intent.getBooleanExtra(
                    NmsIpMessageConsts.NmsNewMessageAction.NMS_NEED_SHOW_NOTIFICATION, false);
            if (null == msg) {
                Xlog.e(TAG, "get NmsIpMessage is null.");
                return;
            }
            if (!needNotify) {
                Xlog.d(TAG, "no need to notify.");
                return;
            }
            Xlog.d(TAG, "msg type = " + msg.type + "msg status" + msg.status + "; msg id = "
                    + msg.id);
            if (msg.protocol != NmsIpMessageConsts.NmsMessageProtocol.IP
                    || msg.status != NmsIpMessageConsts.NmsIpMessageStatus.INBOX
                    || ((msg.type >= NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG && msg.type <= NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG) || msg.type >= NmsIpMessageConsts.NmsIpMessageType.UNKNOWN_FILE)) {
                return;
            }

            Intent it = new Intent();
            it.setAction(IpMessageConsts.NewMessageAction.ACTION_NEW_MESSAGE);
            it.putExtra(IpMessageConsts.NewMessageAction.IP_MESSAGE_KEY, msg.id);

            mContext.sendBroadcast(it);
            // NotificationsManagerExt.notify(it);
        }

        private void handleServiceReady() {
            Xlog.d(TAG, "handleServiceReady");

            Intent it = new Intent();
            it.setAction(IpMessageConsts.ACTION_SERVICE_READY);

            NotificationsManagerExt.notify(it);
        }

        private void handleImStatus(Intent intent) {
            Xlog.d(TAG, "handleImStatus");
            NmsContact contact = (NmsContact) intent
                    .getSerializableExtra(NmsIpMessageConsts.NmsImStatus.NMS_CONTACT_CURRENT_STATUS);
            if (null == contact) {
                Xlog.e(TAG, "get NmsContact is null.");
                return;
            }

            Intent it = new Intent();
            it.setAction(IpMessageConsts.ImStatus.ACTION_IM_STATUS);
            it.putExtra(IpMessageConsts.NUMBER, contact.getNumber());

            NotificationsManagerExt.notify(it);
        }

        private void handleIpMessageStatus(Intent intent) {
            Xlog.d(TAG, "handleIpMessageStatus");
            int status = intent.getIntExtra(NmsIpMessageConsts.STATUS, 0);
            int msgId = intent.getIntExtra(NmsIpMessageConsts.NmsIpMessageStatus.NMS_IP_MSG_SYS_ID,
                    0);

            Intent it = new Intent();
            it.setAction(IpMessageConsts.IpMessageStatus.ACTION_MESSAGE_STATUS);
            it.putExtra(IpMessageConsts.STATUS, status);
            it.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID, msgId);

            NotificationsManagerExt.notify(it);
        }

        private void handleReadedBurnTime(Intent intent) {
            Xlog.d(TAG, "handleReadedBurnTime");
            int time = intent.getIntExtra(NmsIpMessageConsts.NmsIpMessageStatus.NMS_IP_MSG_TIME, 0);
            long msgId = intent.getLongExtra(
                    NmsIpMessageConsts.NmsIpMessageStatus.NMS_IP_MSG_SYS_ID, 0);

            Intent it = new Intent();
            it.setAction(IpMessageConsts.IpMessageStatus.ACTION_MESSAGE_TIME);
            it.putExtra(IpMessageConsts.IpMessageStatus.MESSAGE_TIME, time);
            it.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID, msgId);

            NotificationsManagerExt.notify(it);
        }

        private void handleDownloadAttachStatus(Intent intent) {
            long msgId = intent.getLongExtra(
                    NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_MSG_ID, 0);
            int percentage = intent.getIntExtra(
                    NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_PERCENTAGE, 0);
            int status = intent.getIntExtra(
                    NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsIpMessageConsts.NmsDownloadAttachStatus.STARTING);

            Intent it = new Intent();
            it.setAction(IpMessageConsts.DownloadAttachStatus.ACTION_DOWNLOAD_ATTACH_STATUS);
            it.putExtra(IpMessageConsts.DownloadAttachStatus.DOWNLOAD_MSG_STATUS, status);
            it.putExtra(IpMessageConsts.DownloadAttachStatus.DOWNLOAD_PERCENTAGE, percentage);
            it.putExtra(IpMessageConsts.DownloadAttachStatus.DOWNLOAD_MSG_ID, msgId);

            NotificationsManagerExt.notify(it);
        }

        private void handleSaveHistory(Intent intent) {
            int done = intent.getIntExtra(
                    NmsIpMessageConsts.NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE, 1);
            String filePath = intent
                    .getStringExtra(NmsIpMessageConsts.NmsSaveHistory.NMS_DOWNLOAD_HISTORY_FILE);

            Intent it = new Intent();
            it.setAction(IpMessageConsts.SaveHistroy.ACTION_SAVE_HISTROY);
            it.putExtra(IpMessageConsts.SaveHistroy.SAVE_HISTRORY_DONE, done);
            it.putExtra(IpMessageConsts.SaveHistroy.DOWNLOAD_HISTORY_FILE, filePath);

            NotificationsManagerExt.notify(it);
        }

        private void handleUpdateGroup(Intent intent) {
            int groupId = intent.getIntExtra(NmsIpMessageConsts.NmsUpdateGroupAction.NMS_GROUP_ID,
                    -1);

            Intent it = new Intent();
            it.setAction(IpMessageConsts.UpdateGroup.UPDATE_GROUP_ACTION);
            it.putExtra(IpMessageConsts.UpdateGroup.GROUP_ID, groupId);

            NotificationsManagerExt.notify(it);
        }

        private void handleUpdateContact(Intent intent) {
            Intent it = new Intent();
            it.setAction(IpMessageConsts.ContactStatus.CONTACT_UPDATE);

            NotificationsManagerExt.notify(it);
        }

        private void handleSimInfoChanged(Intent intent) {
            int simId = intent.getIntExtra(NmsIpMessageConsts.NmsSimInfoChanged.NMS_SIM_ID, -1);
            if (simId == -1) {
                return;
            }

            Intent it = new Intent();
            it.setAction(IpMessageConsts.SimInfoChanged.SIM_INFO_ACTION);
            it.putExtra(IpMessageConsts.SimInfoChanged.SIM_ID, simId);
            NotificationsManagerExt.notify(it);

            ServiceManagerExt.refreshNmsSimInfo(mContext, simId);
        }

        private void handleUpgrade() {
            Xlog.d(TAG, "handleUpgrade");
            Intent it = new Intent();
            it.setAction(IpMessageConsts.ACTION_UPGRADE);
            mContext.sendBroadcast(it);
        }

        private void handleRegStatus(Intent intent) {
            int regstatus = intent.getIntExtra(NmsIpMessageConsts.REGSTATUS, 0);

            Xlog.d(TAG, "handleRegStatus");
            Intent it = new Intent();
            it.setAction(IpMessageConsts.RegStatus.ACTION_REG_STATUS);
            it.putExtra(IpMessageConsts.RegStatus.REGSTATUS, regstatus);

            NotificationsManagerExt.notify(it);
        }

        private void handleStoreStatus(Intent intent) {
            Xlog.d(TAG, "handleStoreStatus");
            int storestatus = intent.getIntExtra(
                    NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_STATUS, 0);

            Intent it = new Intent();
            it.setAction(IpMessageConsts.StoreStatus.ACTION_STORE_STATUS);
            it.putExtra(IpMessageConsts.StoreStatus.STORESTATUS, storestatus);

            NotificationsManagerExt.notify(it);

        }
    }
}
