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
import android.content.Intent;
import android.database.Cursor;

import com.mediatek.mms.ipmessage.INotificationsListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpNotificationsManager;
import com.mediatek.rcse.plugin.message.PluginIpAttachMessage;
import com.mediatek.rcse.plugin.message.PluginIpImageMessage;
import com.mediatek.rcse.plugin.message.PluginIpVcardMessage;
import com.mediatek.rcse.plugin.message.PluginIpVideoMessage;
import com.mediatek.rcse.plugin.message.PluginIpVoiceMessage;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;
import java.util.Date;
import java.util.UUID;
import java.util.Vector;

import android.net.Uri;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

/**
 * This class is used to test MessagePlugin.
 */
public class NotificationsManagerTest extends InstrumentationTestCase {

    private static final String TAG = "NotificationsManagerTest";
    private static final String MOCK_FILE_PATH = "file:///storage/sdcard0/Joyn/temp/tmp_joyn_1326400736934.jpg";
    private static final String MOCK_VCARD_PATH = "file:///storage/sdcard0/Joyn/temp/tmp_joyn_1326400736934.vcf";
    private static final String MOCK_FILE_NAME = "tmp_joyn_1326400736934.jpg";
    private static final String MOCK_VCARD_NAME = "tmp_joyn_1326400736934.vcf";
    private static final long MOCK_FILE_SIZE = 10232;
    private static final UUID MOCK_FILE_TAG = new UUID(10232, 10232);
    private static final Date MOCK_FILE_DATE = new Date();
    private static final String MOCK_CONTACT = "+34200000250";
    private static final long MOCK_FILE_PROGRESS = 1024;
    private static final int MOCK_STATUS = 10;
    private static final String MOCK_FILE_TRANSFER_TAG = "file_transfer_tag";

    public void testCase1_registerNotificationsListener() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        IpNotificationsManager notificationsManager = new IpNotificationsManager(getInstrumentation().getContext());
        Field field = Utils.getPrivateField(IpNotificationsManager.class, "sNotificationsListeners");
        Vector vector = (Vector) field.get(notificationsManager);
        assertNotNull(vector);
        assertEquals(0, vector.size());
        INotificationsListener listener = new MockNotificationsListener();
        notificationsManager.registerNotificationsListener(listener);
        vector = (Vector) field.get(notificationsManager);
        assertNotNull(vector);
        assertEquals(1, vector.size());
        IpNotificationsManager.notify(null);
        notificationsManager.registerNotificationsListener(listener);

        notificationsManager.unregisterNotificationsListener(listener);
        vector = (Vector) field.get(notificationsManager);
        assertNotNull(vector);
        assertEquals(0, vector.size());
        notificationsManager.unregisterNotificationsListener(listener);
    }

    public void testCase2_attachIpMessage() {
        FileStructForBinder fileStruct = new FileStructForBinder(MOCK_FILE_PATH, MOCK_FILE_NAME, MOCK_FILE_SIZE, MOCK_FILE_TAG, MOCK_FILE_DATE);
        PluginIpAttachMessage attachMessage = new PluginIpAttachMessage(fileStruct, MOCK_CONTACT);
        assertEquals(MOCK_FILE_NAME, attachMessage.getName());
        attachMessage.setProgress(MOCK_FILE_PROGRESS);
        assertEquals(MOCK_FILE_PROGRESS, attachMessage.getProgress());
        attachMessage.setStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getStatus());
        attachMessage.setRcsStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getRcsStatus());
        attachMessage.setTag(MOCK_FILE_TRANSFER_TAG);
        assertEquals(MOCK_FILE_TRANSFER_TAG, attachMessage.getTag());
    }

    public void testCase3_IpImageMessage() {
        String imagePath = Utils.getFilePath(getInstrumentation().getContext(),
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        String imageName = imagePath.substring(imagePath.lastIndexOf("/") + 1);
        FileStructForBinder fileStruct = new FileStructForBinder(imagePath, imageName, MOCK_FILE_SIZE,
                MOCK_FILE_TAG, MOCK_FILE_DATE);
        PluginIpImageMessage attachMessage = new PluginIpImageMessage(fileStruct, MOCK_CONTACT);
        assertEquals(imageName, attachMessage.getName());
        attachMessage.setProgress(MOCK_FILE_PROGRESS);
        assertEquals(MOCK_FILE_PROGRESS, attachMessage.getProgress());
        attachMessage.setStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getStatus());
        attachMessage.setRcsStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getRcsStatus());
        attachMessage.setTag(MOCK_FILE_TRANSFER_TAG);
        assertEquals(MOCK_FILE_TRANSFER_TAG, attachMessage.getTag());
    }

    public void testCase4_IpVcardMessage() {
        FileStructForBinder fileStruct = new FileStructForBinder(MOCK_VCARD_PATH, MOCK_VCARD_NAME, MOCK_FILE_SIZE,
                MOCK_FILE_TAG, MOCK_FILE_DATE);
        PluginIpVcardMessage attachMessage = (PluginIpVcardMessage) PluginUtils.analysisFileType(MOCK_CONTACT, fileStruct);;
        assertEquals(MOCK_VCARD_NAME, attachMessage.getName());
        attachMessage.setProgress(MOCK_FILE_PROGRESS);
        assertEquals(MOCK_FILE_PROGRESS, attachMessage.getProgress());
        attachMessage.setStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getStatus());
        attachMessage.setRcsStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getRcsStatus());
        attachMessage.setTag(MOCK_FILE_TRANSFER_TAG);
        assertEquals(MOCK_FILE_TRANSFER_TAG, attachMessage.getTag());
    }

    public void testCase5_IpVideoMessage() {
        String videoPath = Utils.getFilePath(getInstrumentation().getContext(),
                MediaStore.Video.Media.EXTERNAL_CONTENT_URI);
        String videoName = videoPath.substring(videoPath.lastIndexOf("/") + 1);
        FileStructForBinder fileStruct = new FileStructForBinder(videoPath, videoName, MOCK_FILE_SIZE, MOCK_FILE_TAG,
                MOCK_FILE_DATE);
        PluginIpVideoMessage attachMessage = (PluginIpVideoMessage) PluginUtils.analysisFileType(MOCK_CONTACT, fileStruct);
        assertEquals(videoName, attachMessage.getName());
        attachMessage.setProgress(MOCK_FILE_PROGRESS);
        assertEquals(MOCK_FILE_PROGRESS, attachMessage.getProgress());
        attachMessage.setStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getStatus());
        attachMessage.setRcsStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getRcsStatus());
        attachMessage.setTag(MOCK_FILE_TRANSFER_TAG);
        assertEquals(MOCK_FILE_TRANSFER_TAG, attachMessage.getTag());
    }

    public void testCase6_IpAudioMessage() {
        String audioPath = Utils.getFilePath(getInstrumentation().getContext(),
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI);
        String audioName = audioPath.substring(audioPath.lastIndexOf("/") + 1);
        FileStructForBinder fileStruct = new FileStructForBinder(audioPath, audioName, MOCK_FILE_SIZE, MOCK_FILE_TAG,
                MOCK_FILE_DATE);
        PluginIpVoiceMessage attachMessage = (PluginIpVoiceMessage) PluginUtils.analysisFileType(MOCK_CONTACT, fileStruct);
        assertEquals(audioName, attachMessage.getName());
        attachMessage.setProgress(MOCK_FILE_PROGRESS);
        assertEquals(MOCK_FILE_PROGRESS, attachMessage.getProgress());
        attachMessage.setStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getStatus());
        attachMessage.setRcsStatus(MOCK_STATUS);
        assertEquals(MOCK_STATUS, attachMessage.getRcsStatus());
        attachMessage.setTag(MOCK_FILE_TRANSFER_TAG);
        assertEquals(MOCK_FILE_TRANSFER_TAG, attachMessage.getTag());
    }

    class MockNotificationsListener implements INotificationsListener {

        @Override
        public void notificationsReceived(Intent arg0) {
            // TODO Auto-generated method stub

        }

    }
}
