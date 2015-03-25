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
import android.os.IBinder;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.service.ApiManager;

import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

/**
 * This class is used to test the functions of ComposingManager in Model part
 */
public class ComposingManagerTest extends AndroidTestCase {
    public static final String TAG = "IsComposingManagerTest";

    private static final String METHOD_NAME_HASTEXT = "hasText";

    private static final String FIELD_NAME_ISCOMPOSING = "mIsComposing";

    private static final String FIELD_NAME_SIDLETIMEOUT = "sIdleTimeout";

    private static final int MOCK_TIME_OUT_MS = 500;

    private static final int WAITING_TIME_MS = 100;
    protected Object mComposingManager = null;

    protected MockChatImpl mChat = null;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        RcsSettings.createInstance(mContext);
        Method initializeMethod = ApiManager.class.getDeclaredMethod("initialize", Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), this.mContext);
        mChat = new MockChatImpl();
    }

    /*
     * This test case is for the case that when "mCurrentSession" variable in a
     * ChatImpl is null
     */
    public void testCase1_NullCurrentSession() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, NoSuchMethodException,
            InvocationTargetException {
        Logger.v(TAG, "testCase1_NullCurrentSession()");
        Method hasTextMethod = mComposingManager.getClass().getDeclaredMethod(METHOD_NAME_HASTEXT, Boolean.class);
        Field isComposingField = mComposingManager.getClass().getDeclaredField(
                FIELD_NAME_ISCOMPOSING);
        isComposingField.setAccessible(true);
        assertFalse(isComposingField.getBoolean(mComposingManager));
        hasTextMethod.invoke(mComposingManager, false);
        assertFalse(isComposingField.getBoolean(mComposingManager));
    }

    /*
     * This test case is for the case that when "mCurrentSession" variable in a
     * ChatImpl is not null
     */
    public void testCase2_NotNullCurrentSession() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, NoSuchMethodException,
            InvocationTargetException, InterruptedException {
        Logger.v(TAG, "testCase2_NotNullCurrentSession()");
        Method hasTextMethod = mComposingManager.getClass().getDeclaredMethod(METHOD_NAME_HASTEXT, Boolean.class);
        Field isComposingField = mComposingManager.getClass().getDeclaredField(
                FIELD_NAME_ISCOMPOSING);
        Field idleTimeOutField = ModelImpl.class.getDeclaredField(FIELD_NAME_SIDLETIMEOUT);
        idleTimeOutField.setAccessible(true);
        isComposingField.setAccessible(true);
        assertFalse(isComposingField.getBoolean(mComposingManager));
        mChat.setCurrentSession(new MockChatSession());
        idleTimeOutField.setInt(null, MOCK_TIME_OUT_MS);
        hasTextMethod.invoke(mComposingManager, false);
        Thread.sleep(MOCK_TIME_OUT_MS / 2);
        assertTrue(isComposingField.getBoolean(mComposingManager));
        Thread.sleep(MOCK_TIME_OUT_MS / 2 + WAITING_TIME_MS);
        assertFalse(isComposingField.getBoolean(mComposingManager));
    }

    /*
     * This class mock a One2OneChat for test
     */
    protected class MockChatImpl extends One2OneChat {

        private boolean mIsSetSession = false;
        protected MockChatImpl() {
            super((ModelImpl)ModelImpl.getInstance(), null, null, new Object());
             ComposingManagerTest.this.mComposingManager = this.mComposingManager;
        }

        public void setCurrentSession(IChatSession chatSession) {
            mIsSetSession = true;
        }
        
        @Override
        protected boolean setIsComposing(boolean isComposing) {
            return mIsSetSession;
        }
    }

    /*
     * This class mock a ChatSession for test
     */
    protected class MockChatSession implements IChatSession {

        @Override
        public boolean isInComing() {
            return true;
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }

        @Override
        public void acceptSession() throws RemoteException {

        }

        @Override
        public void addParticipant(String participant) throws RemoteException {

        }

        @Override
        public void addParticipants(List<String> participants) throws RemoteException {

        }

        @Override
        public void addSessionListener(IChatEventListener listener) throws RemoteException {

        }

        @Override
        public void cancelSession() throws RemoteException {

        }

        @Override
        public InstantMessage getFirstMessage() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getParticipants() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getInivtedParticipants() throws RemoteException {
            return null;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return null;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return null;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return 0;
        }

        @Override
        public boolean isSessionIdle() throws RemoteException {
            return false;
        }

        @Override
        public boolean isGroupChat() throws RemoteException {
            return false;
        }

        @Override
        public boolean isStoreAndForward() throws RemoteException {
            return false;
        }

        @Override
        public void rejectSession() throws RemoteException {

        }

        @Override
        public void removeSessionListener(IChatEventListener listener) throws RemoteException {

        }

        @Override
        public String sendMessage(String text) throws RemoteException {
            return null;
        }

        @Override
        public void setIsComposingStatus(boolean status) throws RemoteException {
            Logger.i(TAG, "setIsComposingStatus status is " + status);

        }

        @Override
        public void setMessageDeliveryStatus(String msgId, String status) throws RemoteException {

        }

        @Override
        public IBinder asBinder() {
            return null;
        }

        @Override
        public String getChatID() throws RemoteException {
            // TODO
            return null;
        }
        
        @Override
        public void setMessageDisplayedStatusBySipMessage(String contact, String msgId, String status) {
        }
        
        @Override
        public String getReferredByHeader(){
            return null;
        }
        
        public String getSubject() throws RemoteException {
        	return null;
        }
        
        @Override
        public int getMaxParticipants() throws RemoteException {
        	return 0;
        }
        
        @Override
        public int getMaxParticipantsToBeAdded() throws RemoteException {
        	return 0;
        }

		@Override
		public boolean isGeolocSupported() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public String sendGeoloc(GeolocPush geoloc) throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public boolean isFileTransferSupported() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public IFileTransferSession sendFile(String file, boolean thumbnail)
				throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void sendMessageWithMsgId(String text, String msgid)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

    }
}
