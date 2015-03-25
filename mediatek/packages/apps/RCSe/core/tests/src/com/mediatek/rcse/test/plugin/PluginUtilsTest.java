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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.Telephony.Sms;
import android.test.InstrumentationTestCase;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.plugin.message.PluginChatWindowMessage;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginIpAttachMessage;
import com.mediatek.rcse.plugin.message.PluginIpImageMessage;
import com.mediatek.rcse.plugin.message.PluginIpVcardMessage;
import com.mediatek.rcse.plugin.message.PluginIpVideoMessage;
import com.mediatek.rcse.plugin.message.PluginIpVoiceMessage;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Date;

/**
* This class is to test PluginUtils class
*/
public class PluginUtilsTest extends InstrumentationTestCase {
   private static final String TAG = "PluginUtilsTest";
   private static final String MOCK_MSG_BODY = "mock msg body";
   private static final String MOCK_MSG_CONTACT = "7---ebbf616c-b616-497a-9f6f-bb497adf4b29";
   private static final String MOKC_NAME = "mock name";
   private static final int MOKC_IPMSG_ID = 1000;
   private static final String MOCK_RCSE_ID = "24534";
   private static final String MOCK_RCSE_ID2 = "232565";
   private static final String MOCK_REMOTE = "mock remote";
   private static final String MOCK_RECEIVED_FILE_TRANSFER_ID = "323456789-987654320";
   private static final int MOCK_IPMSG_TYPE_DEFAULT = 123;
   private static final String MOCK_IMAGE_FILE_PATH = "/mock/filepath/new.png";
   private static final String MOCK_UNKNOWN_FILE_PATH = "/mock/filepath/new";

   /**
    * Test the function updateMessageIdInMmsDb
    */
   public void testCase1_UpdateMessageIdInMmsDb() throws Throwable {
       Logger.d(TAG, "testCase1_UpdateMessageIdInMmsDb() entry");
       // Insert a message in mms db
       assertEquals(Long.valueOf(-1L), PluginUtils.insertDatabase(MOCK_MSG_BODY, MOCK_MSG_CONTACT, MOKC_IPMSG_ID, 0));
       Long messageIdInMms = PluginUtils.insertDatabase(MOCK_MSG_BODY, MOCK_MSG_CONTACT, MOKC_IPMSG_ID,
               PluginUtils.INBOX_MESSAGE);
       Logger.d(TAG, "testCase1_UpdateMessageIdInMmsDb() messageIdInMms is " + messageIdInMms);
       InstantMessage mMessage = new InstantMessage(MOCK_RCSE_ID, MOCK_REMOTE, MOCK_MSG_BODY, false);
       PluginChatWindowMessage pluginChatMessage = new PluginChatWindowMessage(mMessage);
       pluginChatMessage.asBinder();
       pluginChatMessage.storeInCache(messageIdInMms);
       String id = pluginChatMessage.getId();
       assertEquals(mMessage.getMessageId(), id);
       Method targetMethod = Utils.getPrivateMethod(PluginUtils.class, "updateMessageIdInMmsDb", String.class, String.class);
       assertNotNull(targetMethod);
       Object expectIdObject = targetMethod.invoke(null, MOCK_RCSE_ID, MOCK_RCSE_ID2);
       String expectId = expectIdObject.toString();
       Logger.d(TAG, "testCase1_UpdateMessageIdInMmsDb() expectId is " + expectId);
       String resultId = getIpMsgIdInMms(String.valueOf(messageIdInMms));
       assertEquals(expectId, resultId);
        // Clear the message in mms db
       String sessionId = PluginGroupChatWindow.getSessionIdByContact(MOCK_MSG_CONTACT);
       String contact = PluginGroupChatWindow.generateGroupChatInvitationContact(sessionId);
       assertTrue(contact.contains(sessionId));
       PluginGroupChatWindow.removeGroupChatInvitationInMms(MOCK_MSG_CONTACT);
   }
   
   /**
    * Test the function exchangeIpMessage
    */
   public void testCase2_ExchangeIpMessage() throws Throwable {
       Logger.d(TAG, "testCase2_ExchangeIpMessage() entry");
       FileStructForBinder fileStruct = new FileStructForBinder(new FileStruct(getFilePath(), MOKC_NAME, 0, MOCK_RECEIVED_FILE_TRANSFER_ID, new Date()));
       IpMessage result1 = PluginUtils.exchangeIpMessage(IpMessageConsts.IpMessageType.PICTURE, fileStruct, MOCK_REMOTE);
       assertTrue(result1 instanceof PluginIpImageMessage);
       IpMessage result2 = PluginUtils.exchangeIpMessage(IpMessageConsts.IpMessageType.VOICE, fileStruct, MOCK_REMOTE);
       assertTrue(result2 instanceof PluginIpVoiceMessage);
       IpMessage result3 = PluginUtils.exchangeIpMessage(IpMessageConsts.IpMessageType.VIDEO, fileStruct, MOCK_REMOTE);
       assertTrue(result3 instanceof PluginIpVideoMessage);
       IpMessage result4 = PluginUtils.exchangeIpMessage(IpMessageConsts.IpMessageType.VCARD, fileStruct, MOCK_REMOTE);
       assertTrue(result4 instanceof PluginIpVcardMessage);
       IpMessage result5 = PluginUtils.exchangeIpMessage(MOCK_IPMSG_TYPE_DEFAULT, fileStruct, MOCK_REMOTE);
       assertTrue(result5 instanceof PluginIpAttachMessage);
   }

    /**
     * Test error handling of PluginUtils methods
     */
    public void testCase3_testErrorInput() throws Throwable {
        Method methodFindIdInRcseDb = Utils.getPrivateMethod(PluginUtils.class, "findIdInRcseDb", String.class);
        assertEquals((Integer) PluginUtils.DEFAULT_MESSAGE_ID, ((Integer) methodFindIdInRcseDb.invoke(null, (String) null)));
        Method methodStoreMessageInMmsDb = Utils.getPrivateMethod(PluginUtils.class, "storeMessageInMmsDb", int.class,
                String.class, String.class, int.class, long.class);
        assertEquals(-1L, methodStoreMessageInMmsDb.invoke(null, 0, null, null, 0, 0L));
        Method methodCacheThreadIdForGroupChat = Utils.getPrivateMethod(PluginUtils.class, "cacheThreadIdForGroupChat",
                String.class, ContentResolver.class);
        ContentResolver contentResolver = getInstrumentation().getContext().getContentResolver();
        assertFalse((Boolean) methodCacheThreadIdForGroupChat.invoke(null, MOCK_REMOTE, contentResolver));
        prepareSingleSentMessage(contentResolver);
        Method methodUpdatePreSentMessageInMmsDb = Utils.getPrivateMethod(PluginUtils.class, "updatePreSentMessageInMmsDb", String.class, int.class);
        assertTrue((Boolean) methodUpdatePreSentMessageInMmsDb.invoke(null, MOCK_RCSE_ID, MOKC_IPMSG_ID));
        clearSingleSentMessage(contentResolver);
    }

    /**
     * Test OnViewFileDetails method in PluginUtils
     * 
     * @throws Throwable
     */
    public void testCase4_testOnViewFileDetails() throws Throwable {
        Method methodOnViewFileDetails = Utils.getPrivateMethod(PluginUtils.class, "onViewFileDetials", String.class,
                Context.class);
        assertFalse((Boolean) methodOnViewFileDetails.invoke(null, "", null));
        Method methodGenerateFileDetailsIntent = Utils.getPrivateMethod(PluginUtils.class, "generateFileDetailsIntent",
                String.class);
        assertNull(methodGenerateFileDetailsIntent.invoke(null, MOCK_UNKNOWN_FILE_PATH));
        assertNotNull(methodGenerateFileDetailsIntent.invoke(null, MOCK_IMAGE_FILE_PATH));
    }

    /**
     * Test initialize Sim info
     * 
     * @throws Throwable
     */
    public void testCase5_testInitializeSimInfo() throws Throwable {
        Context context = getInstrumentation().getContext();
        Method methodInitializeSimIdFromTelephony = Utils.getPrivateMethod(PluginUtils.class,
                "initializeSimIdFromTelephony", Context.class);
        methodInitializeSimIdFromTelephony.invoke(null, context);
        Field fieldSimId = Utils.getPrivateField(PluginUtils.class, "sSimId");
        fieldSimId.set(null, -1L);
        methodInitializeSimIdFromTelephony.invoke(null, context);
    }

    private void prepareSingleSentMessage(ContentResolver contentResolver) {
        ContentValues mockRcseValues = new ContentValues();
        mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_REMOTE);
        mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_MSG_BODY);
        mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RCSE_ID);
        mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE);
        mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
        contentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);

        mockRcseValues.clear();
        mockRcseValues.put(Sms.ADDRESS, MOCK_REMOTE);
        mockRcseValues.put(Sms.BODY, MOCK_MSG_BODY);
        mockRcseValues.put(Sms.IPMSG_ID, MOKC_IPMSG_ID);
        contentResolver.insert(Uri.withAppendedPath(PluginUtils.SMS_CONTENT_URI, "outbox"), mockRcseValues);
    }

    private void clearSingleSentMessage(ContentResolver contentResolver) {
        String[] arg = {MOCK_REMOTE};
        contentResolver.delete(RichMessagingData.CONTENT_URI, RichMessagingData.KEY_CONTACT + "=?", arg);
        contentResolver.delete(PluginUtils.SMS_CONTENT_URI, Sms.ADDRESS + "=?", arg);
    }

   private String getIpMsgIdInMms(String messageIdInMms) {
       Logger.d(TAG, "getIpMsgIdInMms() messageIdInMms is " + messageIdInMms);
       ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
       String resultId = null;
       Cursor cursor = null;
       String[] selectionArg = {
               String.valueOf(messageIdInMms)
       };
       try {
           cursor = contentResolver.query(PluginUtils.SMS_CONTENT_URI, null, Sms._ID + "=?", selectionArg, null);
           if (cursor != null) {
               if (cursor.moveToFirst()) {
                   resultId = cursor.getString(cursor.getColumnIndex(Sms.IPMSG_ID));
                   Logger.d(TAG, "testCase1_UpdateMessageIdInMmsDb() resultId is " + resultId);
               }
           }
       } finally {
           if (null != cursor) {
               cursor.close();
           }
       }
       return resultId;
   }
   
   /**
    * Get a image file path from database
    */
   private String getFilePath() {
       Logger.v(TAG, "getFilePath()");
       Context context = getInstrumentation().getTargetContext();
       Cursor cursor = null;
       String filePath = null;
       try {
           cursor = context.getContentResolver().query(
                   MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
           if (null != cursor && cursor.getCount() > 0) {
               cursor.moveToFirst();
               filePath = cursor.getString(cursor
                       .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
           } else {
               fail("getFilePath() Cannot find image in sdcard");
           }
       } finally {
           if (cursor != null) {
               cursor.close();
               cursor = null;
           }
       }
       Logger.v(TAG, "getFilePath() out, filePath is " + filePath);
       return filePath;
   }
}

