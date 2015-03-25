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

import android.content.Context;
import android.content.ContextWrapper;
import com.hissage.api.NmsIpMessageApi;
import com.mediatek.mms.ipmessage.ChatManager;
import com.mediatek.xlog.Xlog;

/**
 * Provide chat management related interface
 */
public class ChatManagerExt extends ChatManager {
    private static final String TAG = "imsp/ChatManagerExt";

    public ChatManagerExt(Context context) {
        super(context);
    }

    public boolean needShowInviteDlg(long threadId) {
        Xlog.d(TAG, "needShowInviteDlg, thread id = " + threadId);
        return NmsIpMessageApi.getInstance(mContext).nmsNeedShowInviteDlg(threadId);
    }

    public boolean handleInviteDlgLater(long threadId) {
        Xlog.d(TAG, "handleInviteDlgLater, thread id = " + threadId);
        return NmsIpMessageApi.getInstance(mContext).nmsHandleInviteDlgLaterCmd(threadId);
    }

    public boolean handleInviteDlg(long threadId) {
        Xlog.d(TAG, "handleInviteDlg, thread id = " + threadId);
        return NmsIpMessageApi.getInstance(mContext).nmsHandleInviteDlgInviteCmd(threadId);
    }

    public int needShowReminderDlg(long threadId) {
        Xlog.d(TAG, "needShowReminderDlg, thread id = " + threadId);
        return NmsIpMessageApi.getInstance(mContext).nmsNeedShowReminderDlg(threadId);
    }

    public boolean needShowSwitchAcctDlg(long threadId) {
        Xlog.d(TAG, "needShowSwitchAcctDlg, thread id = " + threadId);
        return NmsIpMessageApi.getInstance(mContext).nmsNeedShowSwitchAcctDlg(threadId);
    }

    public void enterChatMode(String number) {
        Xlog.d(TAG, "enterChatMode, number = " + number);
        NmsIpMessageApi.getInstance(mContext).nmsEnterChatMode(number);
    }

    public void sendChatMode(String number, int status) {
        Xlog.d(TAG, "sendChatMode, number = " + number + ", status = " + status);
        NmsIpMessageApi.getInstance(mContext).nmsSendChatMode(number, status);
    }

    public void exitFromChatMode(String number) {
        Xlog.d(TAG, "exitFromChatMode, number = " + number);
        NmsIpMessageApi.getInstance(mContext).nmsExitFromChatMode(number);
    }

    public void saveChatHistory(long[] threadIds) {
        Xlog.d(TAG, "saveChatHistory, thread id list: ");
        for (long tId : threadIds) {
            Xlog.d(TAG, "\tthreadId = " + tId);
        }
        NmsIpMessageApi.getInstance(mContext).nmsSaveChatHistory(threadIds);
    }

    /**
     * Get ip message count of special type in a thread.
     * 
     * @param threadId
     *            thread ID
     * @param typeFlay
     *            refer to IpMessageMediaTypeFlag
     * @return int count of message
     */
    public int getIpMessageCountOfTypeInThread(long threadId, int typeFlag) {
        Xlog.d(TAG, "getIpMessageCountOfTypeInThread, threadId = " + threadId + ", typeFlag="
                + typeFlag);
        return NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgCountOfTypeInThread(threadId,
                typeFlag);
    }

    public boolean deleteDraftMessageInThread(long threadId) {
        int result = NmsIpMessageApi.getInstance(mContext).nmsDeleteDraftMsgInThread(threadId);
        return result >= 0 ? true : false;
    }

    ///M: Activation Statistics{@
    public void addActivatePromptStatistics(int type) {
        Xlog.d(TAG, "addActivateStatistics be called");
        NmsIpMessageApi.getInstance(mContext).nmsAddActivatePromptStatistics(type);
        return;
    }

    ///@}
    /// M: private Statistics @{
    public void addPrivateClickTimeStatistics(int clickTime) {
        NmsIpMessageApi.getInstance(mContext).addPrivateClickTimeStatistics(clickTime);
        return;
    }

    public void addPrivateOpenFlagStatistics(int openFlag) {
        NmsIpMessageApi.getInstance(mContext).addPrivateOpenFlagStatistics(openFlag);
        return;
    }

    public void addPrivateContactsStatistics(int contacts) {
        NmsIpMessageApi.getInstance(mContext).addPrivateContactsStatistics(contacts);
        return;
    }
    /// @}
}
