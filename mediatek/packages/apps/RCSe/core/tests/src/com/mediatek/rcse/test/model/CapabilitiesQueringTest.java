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

import android.app.Instrumentation.ActivityMonitor;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import android.test.AndroidTestCase;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.service.ApiManager;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;


/**
 * This class is used to test the functions of checking the capabilities 
 * before inital a chat and quering the capabilities before close a chat 
 * in Model part
 */
public class CapabilitiesQueringTest extends AndroidTestCase {
   
    private final static String MESSAGE = "Test";
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Method initializeMethod = ApiManager.class.getDeclaredMethod("initialize", Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), this.mContext);
        Field messageApiField = ApiManager.class.getDeclaredField("mMessagingApi");
        messageApiField.setAccessible(true);
        messageApiField.set(ApiManager.getInstance(), new MessagingApi(this.mContext));
    }
    
    /**
     * Used to test the capabilities checking before inviting a chat in the
     * model part
     */
    public void testCase1_checkCapabilities() throws Throwable {
        MockChatSession session = null;
        MockOne2OneChat one2OneChatWithNullSession = new MockOne2OneChat((ModelImpl)ModelImpl.getInstance(),
                new Object());
        assertNotNull(one2OneChatWithNullSession);
        one2OneChatWithNullSession.setSession(session);
        Method sendMessageViaInviteMethod = One2OneChat.class.getDeclaredMethod("sendMessageViaInvite", String.class);
        sendMessageViaInviteMethod.setAccessible(true);
        sendMessageViaInviteMethod.invoke(one2OneChatWithNullSession, MESSAGE);
        assertTrue(one2OneChatWithNullSession.isCheckCapabilitiesCalled());

        MockOne2OneChat one2OneChatWithNotNullSession = new MockOne2OneChat(
                (ModelImpl)ModelImpl.getInstance(), new Object());
        assertNotNull(one2OneChatWithNotNullSession);
        session = new MockChatSession();
        assertNotNull(session);
        one2OneChatWithNotNullSession.setSession(session);
        Method sendMessageViaSessionMethod = One2OneChat.class.getDeclaredMethod("sendMessageViaSession", IChatSession.class, String.class);
        sendMessageViaSessionMethod.setAccessible(true);
        sendMessageViaSessionMethod.invoke(one2OneChatWithNullSession, session, MESSAGE);
        assertFalse(one2OneChatWithNotNullSession.isCheckCapabilitiesCalled());
    }

    /**
     * Used to test the capabilities quering after terminating a chat in the
     * model part
     */
    public void testCase2_queryCapabilities() {
        MockOne2OneChat chat = new MockOne2OneChat((ModelImpl)ModelImpl.getInstance(), new Object());
        assertNotNull(chat);
        chat.onDestroy();
        assertTrue(chat.isQueryCapabilitiesCalled());
    }

    /**
     * The class defined to mock a One2OneChat inplementation
     */
    private class MockOne2OneChat extends One2OneChat{
        
        protected MockOne2OneChat(ModelImpl modelImpl,Object tag) {
            super(modelImpl,null,new Participant("", ""),tag);
        }
        
        private boolean mIsCheckCapabilitiesCalled = false;
        private boolean mIsQueryCapabilitieCalled = false;
        
        /**
         * Check whether the checkCapabilities() was called
         */
        protected boolean isCheckCapabilitiesCalled(){
            return mIsCheckCapabilitiesCalled;
        }
        
        /**
         * Check whether the queryCapabilities() was called
         */
        protected boolean isQueryCapabilitiesCalled(){
            return mIsQueryCapabilitieCalled;
        }
        
        /**
         * Set the chat session to current chat
         * 
         * @param chatSession The chat session set to current chat
         */
        protected void setSession(IChatSession chatSession){
            mCurrentSession.set(chatSession);
        }
        
        @Override
        protected void checkCapabilities(){
            mIsCheckCapabilitiesCalled = true;
        }

        @Override
        protected void queryCapabilities(){
            mIsQueryCapabilitieCalled = true;
        }
        
        @Override
        protected void onDestroy(){
            super.onDestroy();
        }
        
        @Override
        protected boolean getRegistrationState() {
            Logger.d(TAG, "CapabilitiesQueringTest-getRegistrationState()");
            return true;
        }
    }
    
    /**
     * This class mock a ChatSession
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
