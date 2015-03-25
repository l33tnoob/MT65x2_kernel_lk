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

package com.mediatek.rcse.test.model;

import android.content.Context;
import android.test.AndroidTestCase;

import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Collection;
import java.util.List;
import java.util.UUID;

/**
 * This class is used to test the basic function of SentFileTransferManager in
 * ModelImpl
 */
public class SentFileTransferManagerTest extends AndroidTestCase {

    private static final String FIELD_NAME_MANAGER = "mOutGoingFileTransferManager";

    private static final String FIELD_NAME_PENDING_LIST = "mPendingList";

    private static final String FIELD_NAME_ACTIVE_LIST = "mActiveList";

    private static final String FIELD_NAME_RESENDABLE_LIST = "mResendableList";

    private static final String METHOD_NAME_ON_ADD_SENT_FILE_TRANSFER = "onAddSentFileTransfer";

    private Object mManagerInstance = null;

    private Method mMethodAddSentFileTransfer = null;

    private Collection<ModelImpl.SentFileTransfer> mPendingList = null;

    private Collection<ModelImpl.SentFileTransfer> mActiveList = null;

    private Collection<ModelImpl.SentFileTransfer> mResendableList = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Method initializeMethod = ApiManager.class.getDeclaredMethod("initialize", Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), this.mContext);
        mManagerInstance = getManagerInstance();
        assertNotNull(mManagerInstance);
        mPendingList = getPendingList();
        assertNotNull(mPendingList);
        mActiveList = getActiveList();
        assertNotNull(mActiveList);
        mResendableList = getResendableList();
        assertNotNull(mResendableList);
        mMethodAddSentFileTransfer = getAddSentFileTransferMethod();
        assertNotNull(mMethodAddSentFileTransfer);
    }

    private Object getManagerInstance() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException, InstantiationException,
            InvocationTargetException {
        IChatManager model = ModelImpl.getInstance();
        Field fieldFileManager = ModelImpl.class.getDeclaredField(FIELD_NAME_MANAGER);
        fieldFileManager.setAccessible(true);
        Object realInstance = fieldFileManager.get(model);
        Constructor<?> sentFileManagerConstructors = realInstance.getClass()
                .getDeclaredConstructor();
        sentFileManagerConstructors.setAccessible(true);
        return sentFileManagerConstructors.newInstance();
    }

    private Method getAddSentFileTransferMethod() throws NoSuchMethodException {
        return mManagerInstance.getClass().getDeclaredMethod(METHOD_NAME_ON_ADD_SENT_FILE_TRANSFER,
                ModelImpl.SentFileTransfer.class);
    }

    private Collection<ModelImpl.SentFileTransfer> getPendingList() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field fieldPendingList = mManagerInstance.getClass().getDeclaredField(
                FIELD_NAME_PENDING_LIST);
        fieldPendingList.setAccessible(true);
        return (Collection<ModelImpl.SentFileTransfer>) fieldPendingList.get(mManagerInstance);
    }

    private Collection<ModelImpl.SentFileTransfer> getActiveList() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field fieldActiveList = mManagerInstance.getClass()
                .getDeclaredField(FIELD_NAME_ACTIVE_LIST);
        fieldActiveList.setAccessible(true);
        return (Collection<ModelImpl.SentFileTransfer>) fieldActiveList.get(mManagerInstance);
    }

    private Collection<ModelImpl.SentFileTransfer> getResendableList() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field fieldResendableList = mManagerInstance.getClass().getDeclaredField(
                FIELD_NAME_RESENDABLE_LIST);
        fieldResendableList.setAccessible(true);
        return (Collection<ModelImpl.SentFileTransfer>) fieldResendableList.get(mManagerInstance);
    }

    /**
     * This method is used to test the active queue: 1. When adding a new sent
     * file transfer, if the active queue is available, send it immediately 2.
     * When adding a new sent file transfer, if the active queue is busy, put it
     * into the pending list
     * 
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase1_ActiveQueue() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        MockSentFileTransfer firstSentFileTransfer = new MockSentFileTransfer(null, null, null, null, mManagerInstance);
        MockSentFileTransfer secondSentFileTransfer = new MockSentFileTransfer(null, null, null, null, mManagerInstance);
        mMethodAddSentFileTransfer.invoke(mManagerInstance, firstSentFileTransfer);
        assertFalse(mPendingList.contains(firstSentFileTransfer));
        assertTrue(mActiveList.contains(firstSentFileTransfer));
        assertFalse(mResendableList.contains(firstSentFileTransfer));
        assertTrue(firstSentFileTransfer.getSentStatus());
        mMethodAddSentFileTransfer.invoke(mManagerInstance, secondSentFileTransfer);
        assertTrue(mPendingList.contains(secondSentFileTransfer));
        assertFalse(mActiveList.contains(secondSentFileTransfer));
        assertFalse(mResendableList.contains(secondSentFileTransfer));
        assertFalse(secondSentFileTransfer.getSentStatus());
    }

    /**
     * This method is used to test the result of each finished sent file
     * transfer: If the result is MOVEABLE, the finished sent file transfer will
     * be just removed from the active list.
     * 
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase2_FinishWithRemovableResult() throws IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        MockSentFileTransfer sentFileTransfer = new MockSentFileTransfer(null, null, null, null, mManagerInstance);
        mMethodAddSentFileTransfer.invoke(mManagerInstance, sentFileTransfer);
        sentFileTransfer.finishWithRemovable();
        assertFalse(mPendingList.contains(sentFileTransfer));
        assertFalse(mActiveList.contains(sentFileTransfer));
        assertFalse(mResendableList.contains(sentFileTransfer));
    }

    /**
     * This method is used to test the result of each finished sent file
     * transfer: If the result is RESENDABLE, the finished sent file transfer
     * will be moved to the re-sendable list.
     * 
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase3_FinishWithResendableResult() throws IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        MockSentFileTransfer sentFileTransfer = new MockSentFileTransfer(null, null, null, null, mManagerInstance);
        mMethodAddSentFileTransfer.invoke(mManagerInstance, sentFileTransfer);
        sentFileTransfer.finishWithResendable();
        assertFalse(mPendingList.contains(sentFileTransfer));
        assertFalse(mActiveList.contains(sentFileTransfer));
        assertTrue(mResendableList.contains(sentFileTransfer));
    }

    /**
     * This is a mock sent file transfer, which is used to test the sent file
     * transfer manager
     */
    private class MockSentFileTransfer extends ModelImpl.SentFileTransfer {

        public MockSentFileTransfer(Object chatTag, IGroupChatWindow groupChat,
				String filePath, List<String> participants,
				Object fileTransferTag) {
			super(chatTag, groupChat, filePath, participants, fileTransferTag);
			// TODO Auto-generated constructor stub
		}

		private boolean mIsSent = false;

      

        @Override
        protected void send() {
            mIsSent = true;
        }

        public boolean getSentStatus() {
            return mIsSent;
        }

        public void finishWithRemovable() {
            onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
        }

        public void finishWithResendable() {
            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
        }
    }
}
