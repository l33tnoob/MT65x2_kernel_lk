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

import java.util.Date;
import android.content.ContentValues;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.util.Log;
import com.hissage.R;
import com.hissage.api.NmsIpMessageApi;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.util.data.NmsConsts;
import com.mediatek.mms.ipmessage.ActivitiesManager;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;

import com.mediatek.xlog.Xlog;

public class ActivitiesManagerExt extends ActivitiesManager {
    private static final String TAG = "imsp/ActivitiesManagerExt";

    private Context pluginContext = null;

    public ActivitiesManagerExt(Context context) {
        super(context);
        pluginContext = context;
    }

    public void startRemoteActivity(Context context, Intent intent) {
        Xlog.d(TAG, "startRemoteActivity: intent = " + intent);
        String actionStr = intent.getAction();
        String lastPath = Uri.parse(actionStr).getLastPathSegment();
        int actionIndex = Integer.parseInt(lastPath);
        switch (actionIndex) {
        case RemoteActivities.ID_CHAT_SETTINGS: { // chat settings
            long threadId = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_THREAD_ID, 0);
            NmsStartActivityApi.nmsStartChatSettingsActivity(context, threadId);
            break;
        }

        case RemoteActivities.ID_SYSTEM_SETTINGS: { // system_settings
            NmsStartActivityApi.nmsStartIpMessageSystemSettingsActivity(context);
            break;
        }

        case RemoteActivities.ID_ACTIVITION: { // activition
            int simId = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_SIM_ID,
                    NmsConsts.INVALID_SIM_ID);
            // /M: Activation Statistics{@
            int activityType = intent.getIntExtra(
                    IpMessageConsts.RemoteActivities.KEY_ACTIVATE_TYPE,
                    IpMessageConsts.ActivateType.OTHER);
            NmsStartActivityApi.nmsStartActivitionActivity(context, (long) simId, activityType);
            // /@}
            break;
        }

        case RemoteActivities.ID_LOCATION: { // location
            int requestCode = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_REQUEST_CODE,
                    0);
            NmsStartActivityApi.nmsStartLocationActivityForResult(context, requestCode);
            break;
        }

        case RemoteActivities.ID_ALL_MEDIA: { // all_media
            long threadId = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_THREAD_ID, 0);
            NmsStartActivityApi.nmsStartAllMediaActivity(context, threadId);
            break;
        }

        case RemoteActivities.ID_ALL_LOCATION: { // all_location
            long threadId = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_THREAD_ID, 0);
            NmsStartActivityApi.nmsStartAllLocationActivity(context, threadId);
            break;
        }

        case RemoteActivities.ID_CHAT_DETAILS_BY_THREAD_ID: { // chat_details_by_thread_id
            long threadId = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_THREAD_ID, 0);
            long messageId = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_MESSAGE_ID,
                    -1);
            boolean needShowLoadAllMsg = intent.getBooleanExtra(
                    IpMessageConsts.RemoteActivities.KEY_BOOLEAN, false);
            boolean needNewTask = intent.getBooleanExtra(
                    IpMessageConsts.RemoteActivities.KEY_NEED_NEW_TASK, true);
            ///M: Create a group chat shortcuts from conversation list,and then
            // delete the group thread,
            ///and finally through the shortcut can not re-enter the group chat
            // {@
            int contactId = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_CONTACT_ID, 0);
            NmsStartActivityApi.nmsStartChatDetailsActivity(context, threadId, contactId,
                    messageId, needShowLoadAllMsg, needNewTask);
            ///@}
            break;
        }

        case RemoteActivities.ID_CONTACT: { // contact_selection
            int requestCode = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_REQUEST_CODE,
                    0);
            int type = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_TYPE, 0);
            if (type == IpMessageConsts.SelectContactType.ALL) {
                NmsStartActivityApi.nmsStartAllContactSelectionActivityForResult(context,
                        requestCode);
            } else if (type == IpMessageConsts.SelectContactType.IP_MESSAGE_USER) {
                NmsStartActivityApi.nmsStartContactSelectionActivityForResult(context,
                        pluginContext, requestCode);
            } else if (type == IpMessageConsts.SelectContactType.NOT_IP_MESSAGE_USER) {
                NmsStartActivityApi.nmsStartNoniSMSContactSelectionActivityForResult(context,
                        requestCode);
            } else {
                Xlog.w(TAG, "unknown select contact type.");
            }
            break;
        }

        case RemoteActivities.ID_NON_IPMESSAGE_CONTACT: { // non_ipmessage_contact_selection
            int requestCode = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_REQUEST_CODE,
                    0);
            NmsStartActivityApi.nmsStartNoniSMSContactSelectionActivityForResult(context,
                    requestCode);
            break;
        }

        case RemoteActivities.ID_NEW_GROUP_CHAT: { // new_group_chat
            int simId = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_SIM_ID, 0);
            String[] contactIdStrs = intent
                    .getStringArrayExtra(IpMessageConsts.RemoteActivities.KEY_ARRAY);
            NmsStartActivityApi.nmsStartNewGroupChatActivity(context, contactIdStrs, simId);
            break;
        }

        case RemoteActivities.ID_QUICK_CONTACT: { // quick_contact
            long threadId = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_THREAD_ID, 0);
            NmsStartActivityApi.nmsStartQuickContactActivity(context, threadId);
            break;
        }

        case RemoteActivities.ID_MEDIA_DETAIL: { // media_detail
            int msgId = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_MESSAGE_ID, 0);
            NmsStartActivityApi.nmsStartMediaDetailActivity(context, msgId);
            break;
        }

        case RemoteActivities.ID_READEDBURN_DETAIL: { // readed_detail
            int msgId = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_MESSAGE_ID, 0);
            int time = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_MESSAGE_TIME, 0);
            NmsStartActivityApi.nmsStartReadedBurnDetailActivity(context, msgId, time);
            break;
        }

        case RemoteActivities.ID_READEDBURN_FINISH: {
            NmsStartActivityApi.nmsFinishReadedBurnDetailActivity(context);
            break;
        }

        case RemoteActivities.ID_SKETCH: { // sketch
            int requestCode = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_REQUEST_CODE,
                    0);
            NmsStartActivityApi.nmsStartSketchActivityForResult(context, requestCode);
            break;
        }

        case RemoteActivities.ID_AUDIO: { // audio
            int requestCode = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_REQUEST_CODE,
                    0);
            long maxFileSize = intent.getLongExtra(IpMessageConsts.RemoteActivities.KEY_SIZE, 0);
            NmsStartActivityApi.nmsStartAudioActivityForResult(context, requestCode, maxFileSize);
            break;
        }

        case RemoteActivities.ID_SERVICE_CENTER: { // service center
            NmsStartActivityApi.nmsStartServiceCenterActivity(context);
            break;
        }

        case RemoteActivities.ID_PROFILE: { // profile
            NmsStartActivityApi.nmsStartProfileActivity(context);
            break;
        }

        case RemoteActivities.ID_TERM: { // term
            int simId = intent.getIntExtra(IpMessageConsts.RemoteActivities.KEY_SIM_ID,
                    NmsConsts.INVALID_SIM_ID);
            Xlog.w(TAG, "startRemoteActivity(): term. simId = " + simId);
            NmsStartActivityApi.nmsStartTermActivity(context, (long) simId);
            break;
        }

        default: { // unknown activity
            Xlog.w(TAG, "Unknown activity.");
            break;
        }
        }
    }

    /// M: insert new sms from iSMS Service Center into OutBox {@
    public void createIPMsgServiceCenter(Context context) {

        ContentValues values = new ContentValues();
        values.put("address", "9++99999999");
        values.put("read", "0");
        values.put("body",
                "" + pluginContext.getResources().getString(R.string.STR_NMS_SERVICE_CENTER_BODY));
        values.put("status", -1);
        values.put("date", new Date().getTime());
        values.put("person", "test");
        values.put(Sms.STATUS, "-10000");

        context.getContentResolver().insert(Uri.parse("content://sms/inbox"), values);
    }
    ///@}

}
