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

import java.lang.reflect.Method;
import java.util.Date;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.PluginChatWindowFileTransfer;
import com.mediatek.rcse.plugin.message.PluginChatWindowMessage;
import com.mediatek.rcse.plugin.message.PluginIpAttachMessage;
import com.mediatek.rcse.plugin.message.PluginIpImageMessage;
import com.mediatek.rcse.plugin.message.PluginIpVcardMessage;
import com.mediatek.rcse.plugin.message.PluginIpVideoMessage;
import com.mediatek.rcse.plugin.message.PluginIpVoiceMessage;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import android.test.InstrumentationTestCase;

/**
 * This is to test PluginChatWindowFileTransfer
 */
public class PluginChatWindowFileTransferTest extends InstrumentationTestCase {
	private static final String TAG = "PluginChatWindowFileTransferTest";
	private static final String MOCK_REMOTE = "mock remote";
	private static final String MOCK_FILEPATH = "/mock/filepath/";
	private static final String MOCK_FILEPATH_NEW = "/mock/filepath/new";
	private static final String MOCK_FILENAME = "mock file name";
	private static final int MOCK_FILESIZE = 100;
	private static final String MOCK_FILETAG = "file tag";
	private static final int MOCK_RCSE_ID = 24534214;
	private static final long MOCK_MSG_ID= 234324L;
	private static final long MOCK_MSG_ID_ZERO = 0L;
	private static final long MOCK_PROGRESS = 10L;
	
	/**
	 * This it to test convertStatus() OUTBOX
	 */
	public void testCase1_ConvertStatusOutBox() throws Throwable {
		Logger.d(TAG, "testCase1_ConvertStatus() entry");
		FileStruct fileStruct = new FileStruct(MOCK_FILEPATH, MOCK_FILENAME, MOCK_FILESIZE, MOCK_FILETAG, new Date());
		FileStructForBinder binder = new FileStructForBinder(fileStruct);
		PluginChatWindowFileTransfer fileTransfer = new PluginChatWindowFileTransfer(binder, PluginUtils.OUTBOX_MESSAGE, MOCK_REMOTE);
		
		Method convertStatus = Utils.getPrivateMethod(PluginChatWindowFileTransfer.class, "convertStatus", int.class);
	    assertNotNull(convertStatus);
	    
	    // WAITING
	    Object statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.WAITING.ordinal());
	    int result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_INVITE, result);
	    
	    // PENDING
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.PENDING.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_INVITE, result);
	    
	    // TRANSFERING
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.TRANSFERING.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_SENDING, result);
	    
	    // CANCEL
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.CANCEL.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_CANCEL, result);
	    
	    // CANCELED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.CANCELED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MT_CANCEL, result);
	    
	    // REJECTED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.REJECTED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_REJECTED, result);
	    
	    // FINISHED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.FINISHED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_SENT, result);
	    
	    // FAILED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.FAILED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.FAILED, result);
	}
	
	/**
	 * This it to test convertStatus() INTBOX
	 */
	public void testCase2_ConvertStatusInBox() throws Throwable {
		Logger.d(TAG, "testCase2_ConvertStatusInBox() entry");
		FileStruct fileStruct = new FileStruct(MOCK_FILEPATH, MOCK_FILENAME, MOCK_FILESIZE, MOCK_FILETAG, new Date());
		FileStructForBinder binder = new FileStructForBinder(fileStruct);
		PluginChatWindowFileTransfer fileTransfer = new PluginChatWindowFileTransfer(binder, PluginUtils.INBOX_MESSAGE, MOCK_REMOTE);
		
		Method convertStatus = Utils.getPrivateMethod(PluginChatWindowFileTransfer.class, "convertStatus", int.class);
	    assertNotNull(convertStatus);
	    
	    // WAITING
	    Object statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.WAITING.ordinal());
	    int result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MT_INVITED, result);
	    
	    // TRANSFERING
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.TRANSFERING.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MT_RECEIVING, result);
	    
	    // CANCEL
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.CANCEL.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MO_CANCEL, result);

	    // REJECTED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.REJECTED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MT_REJECT, result);
	    
	    // FINISHED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.FINISHED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MT_RECEIVED, result);
	    
	    // FAILED
	    statusInMms = convertStatus.invoke(fileTransfer, IFileTransfer.Status.FAILED.ordinal());
	    result = Integer.valueOf(statusInMms.toString());
	    assertEquals(IpMessageConsts.IpMessageStatus.MT_CANCEL, result);
	    
	}
	
	/**
	 * This is to test setFilePath()
	 */
	public void testCase3_SetFilePath() throws Throwable {
		FileStruct fileStruct = new FileStruct(MOCK_FILEPATH, MOCK_FILENAME, MOCK_FILESIZE, MOCK_RCSE_ID, new Date());
		FileStructForBinder binder = new FileStructForBinder(fileStruct);
		PluginChatWindowFileTransfer fileTransfer = new PluginChatWindowFileTransfer(binder, PluginUtils.INBOX_MESSAGE, MOCK_REMOTE);
		
		long messageIdInMms = MOCK_MSG_ID;
		IpAttachMessage ipAttachMessage = new IpAttachMessage();
		IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipAttachMessage);
	    
	    fileTransfer.setFilePath(MOCK_FILEPATH_NEW);
	    assertEquals(MOCK_FILEPATH_NEW, ipAttachMessage.getPath());
	    
	    // To cover other conditions
	    IpMessageManager.addMessage(MOCK_MSG_ID_ZERO, String.valueOf(MOCK_RCSE_ID), ipAttachMessage);
	    fileTransfer.setFilePath(MOCK_FILEPATH_NEW);
	    
	    IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), null);
	    fileTransfer.setFilePath(MOCK_FILEPATH_NEW);
	}
	
	/**
	 * This is to test setProgress()
	 */
	public void testCase4_SetProgress() throws Throwable {
		FileStruct fileStruct = new FileStruct(MOCK_FILEPATH, MOCK_FILENAME, MOCK_FILESIZE, MOCK_RCSE_ID, new Date());
		FileStructForBinder binder = new FileStructForBinder(fileStruct);
		PluginChatWindowFileTransfer fileTransfer = new PluginChatWindowFileTransfer(binder, PluginUtils.INBOX_MESSAGE, MOCK_REMOTE);
		
		long messageIdInMms = MOCK_MSG_ID + 1;
		PluginIpImageMessage ipImageMessage = new PluginIpImageMessage(binder, MOCK_REMOTE);
		IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipImageMessage);
		fileTransfer.setProgress(MOCK_PROGRESS);
		assertEquals(MOCK_PROGRESS, ipImageMessage.getProgress());
		
		PluginIpVideoMessage ipVideoMessage = new PluginIpVideoMessage(binder, MOCK_REMOTE);
		IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipVideoMessage);
		fileTransfer.setProgress(MOCK_PROGRESS + 1);
		assertEquals(MOCK_PROGRESS + 1, ipVideoMessage.getProgress());
		
		PluginIpVoiceMessage ipVoiceMessage = new PluginIpVoiceMessage(binder, MOCK_REMOTE);
		IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipVoiceMessage);
		fileTransfer.setProgress(MOCK_PROGRESS + 2);
		assertEquals(MOCK_PROGRESS + 2, ipVoiceMessage.getProgress());
		
		PluginIpVcardMessage ipVcardMessage = new PluginIpVcardMessage(binder, MOCK_REMOTE);
		IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipVcardMessage);
		fileTransfer.setProgress(MOCK_PROGRESS + 3);
		assertEquals(MOCK_PROGRESS + 3, ipVcardMessage.getProgress());
		
		PluginIpAttachMessage ipAttachMessage = new PluginIpAttachMessage(binder, MOCK_REMOTE);
		IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipAttachMessage);
		fileTransfer.setProgress(MOCK_PROGRESS + 4);
		assertEquals(MOCK_PROGRESS + 4, ipAttachMessage.getProgress());
	}
	
    /**
     * Test set status for a ip file message
     * 
     * @throws Throwable
     */
    public void testCase5_SetStatus() throws Throwable {
        Logger.d(TAG, "testCase5_SetStatus() entry");
        FileStruct fileStruct =
                new FileStruct(MOCK_FILEPATH, MOCK_FILENAME, MOCK_FILESIZE, MOCK_RCSE_ID,
                        new Date());
        FileStructForBinder binder = new FileStructForBinder(fileStruct);
        PluginChatWindowFileTransfer fileTransfer =
                new PluginChatWindowFileTransfer(binder, PluginUtils.INBOX_MESSAGE, MOCK_REMOTE);

        long messageIdInMms = MOCK_MSG_ID + 2;
        PluginIpImageMessage ipImageMessage = new PluginIpImageMessage(binder, MOCK_REMOTE);
        IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipImageMessage);
        fileTransfer.setStatus(Status.WAITING.ordinal());
        assertEquals(IpMessageConsts.IpMessageStatus.MT_INVITED, ipImageMessage.getStatus());

        PluginIpVideoMessage ipVideoMessage = new PluginIpVideoMessage(binder, MOCK_REMOTE);
        IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipVideoMessage);
        fileTransfer.setStatus(Status.TRANSFERING.ordinal());
        assertEquals(IpMessageConsts.IpMessageStatus.MT_RECEIVING, ipVideoMessage.getStatus());

        PluginIpVoiceMessage ipVoiceMessage = new PluginIpVoiceMessage(binder, MOCK_REMOTE);
        IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipVoiceMessage);
        fileTransfer.setStatus(Status.CANCEL.ordinal());
        assertEquals(IpMessageConsts.IpMessageStatus.MO_CANCEL, ipVoiceMessage.getStatus());

        PluginIpVcardMessage ipVcardMessage = new PluginIpVcardMessage(binder, MOCK_REMOTE);
        IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipVcardMessage);
        fileTransfer.setStatus(Status.CANCELED.ordinal());
        assertEquals(IpMessageConsts.IpMessageStatus.MT_CANCEL, ipVcardMessage.getStatus());

        PluginIpAttachMessage ipAttachMessage = new PluginIpAttachMessage(binder, MOCK_REMOTE);
        IpMessageManager.addMessage(messageIdInMms, String.valueOf(MOCK_RCSE_ID), ipAttachMessage);
        fileTransfer.setStatus(Status.REJECTED.ordinal());
        assertEquals(IpMessageConsts.IpMessageStatus.MT_REJECT, ipAttachMessage.getStatus());

        Logger.d(TAG, "testCase5_SetStatus() exit");
    }
}
