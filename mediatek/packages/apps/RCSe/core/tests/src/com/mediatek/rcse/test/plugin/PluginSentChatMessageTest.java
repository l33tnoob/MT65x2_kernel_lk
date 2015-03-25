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

package com.mediatek.rcse.test.plugin;

import android.content.Context;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.plugin.message.IpMessageContactManager;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.PluginIpTextMessage;
import com.mediatek.rcse.plugin.message.PluginSentChatMessage;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

/**
 * This class is used to test PluginSentChatMessage
 */
public class PluginSentChatMessageTest extends InstrumentationTestCase {

    private static final String TAG = "PluginSentChatMessageTest";
    private Context mContext;
    private static final String MESSAGE_ID_IN_RCSE = "messageId";
    private static final Long MESSAGE_ID_IN_MMS = Long.MAX_VALUE;
    private static final String MOCK_REMOTE = "+34200000888";
    private static final Long INVILIDE_MESSAGE_ID = Long.MIN_VALUE;
    private static final String INVILIDE_NAME = "";
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getContext();
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown() entry");
        IpMessageManager.removeMessage(MESSAGE_ID_IN_MMS, MESSAGE_ID_IN_RCSE, null);
        Logger.d(TAG, "tearDown() exit");
    }

    /**
     * Test update message status
     * 
     * @throws RemoteException
     */
    public void testCase1_UpdateStatus() throws RemoteException {
        Logger.d(TAG, "testCase1_UpdateStatus() entry");
        InstantMessage instantMessage = new InstantMessage(MESSAGE_ID_IN_RCSE, MOCK_REMOTE, null, true);
        PluginSentChatMessage sentChatMessage =
                new PluginSentChatMessage(new IpMessageManager(mContext), instantMessage, 0);
        long date = Long.MAX_VALUE;
        sentChatMessage.updateDate(date);
        assertEquals(date, sentChatMessage.getDate());
        assertNotNull(sentChatMessage.getPluginChatWindowMessage());
        assertNull(sentChatMessage.getId());
        IpMessageManager.removeMessage(-1L, MESSAGE_ID_IN_RCSE, null);
        IpMessage message = new PluginIpTextMessage(instantMessage);
        IpMessageManager.addMessage(MESSAGE_ID_IN_MMS, MESSAGE_ID_IN_RCSE, message);
        IpMessageContactManager contactManager = new IpMessageContactManager(mContext);
        assertEquals(MOCK_REMOTE, contactManager.getNumberByMessageId(MESSAGE_ID_IN_MMS));
        assertEquals(INVILIDE_NAME, contactManager.getNumberByMessageId(INVILIDE_MESSAGE_ID));
        String status = ISentChatMessage.Status.SENDING.name();
        sentChatMessage.updateStatus(status);
        assertEquals(sentChatMessage.getStatus(), Status.valueOf(status));
        IpMessage ipMessage = IpMessageManager.getMessage(MESSAGE_ID_IN_MMS);
        assertEquals(status, ((PluginIpTextMessage) ipMessage).getMessageStatus().toString());
        status = ISentChatMessage.Status.DELIVERED.name();
        sentChatMessage.updateStatus(status);
        assertEquals(status, ((PluginIpTextMessage) ipMessage).getMessageStatus().toString());
        status = ISentChatMessage.Status.DISPLAYED.name();
        sentChatMessage.updateStatus(status);
        assertEquals(status, ((PluginIpTextMessage) ipMessage).getMessageStatus().toString());
        status = ISentChatMessage.Status.FAILED.name();
        sentChatMessage.updateStatus(status);
        assertEquals(status, ((PluginIpTextMessage) ipMessage).getMessageStatus().toString());
        Logger.d(TAG, "testCase1_UpdateStatus() exit");
    }
}
