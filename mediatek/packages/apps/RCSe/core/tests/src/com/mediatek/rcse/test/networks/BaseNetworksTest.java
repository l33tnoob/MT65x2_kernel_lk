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

package com.mediatek.rcse.test.networks;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.api.RegistrationApi.IRegistrationStatusListener;
import com.mediatek.rcse.service.ApiManager;

import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.provider.messaging.RichMessaging;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.GeolocMessage;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;

/**
 * This base test class is used to test each operator's RCS-e network server
 */
abstract public class BaseNetworksTest extends AndroidTestCase implements
        IRegistrationStatusListener {
    public static final String TAG = "BaseNetworksTest";

    private static final String SESSION_ID = "sessionId";

    protected static class RcseAccountConfiguration {
        protected String mAccountName = null;

        protected String mAccountPassword = null;

        protected String mAccountDisplayName = null;

        protected String mDomain = null;

        protected String mSipProxyPort = null;

        protected String mSipProxyAddress = null;
    }

    private static final String METHOD_NAME_INITIALIZE = "initialize";

    private static final int TIME_OUT_REGISTRATION = 100000;

    private static final int TIME_OUT_API = 3000;

    private static final int TIME_OUT_RECEIVED = 10000;

    private static final int TIME_OUT_SESSION_STARTED = 20000;

    private static final int TIME_OUT_DELIVERY = 20000;

    private static final int TIME_OUT_DISPLAYED = 20000;

    private static final int TIME_PIECE_FOR_WAIT = 200;

    public static final String TEST_TEXT_INVITE = "This is a test text";

    public static final String TEST_TEXT_RESPONSE = "Re:This is a test text";

    private RegistrationApi mRegistrationApi = null;

    private MessagingApi mMessagingApi = null;

    private EventsLogApi mEventsLogApi = null;

    private NetworkTestReceiver mCurrentReceiver = null;

    RcseAccountConfiguration mCurrentAccount = null;

    abstract protected RcseAccountConfiguration getAccountConfiguration();

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mCurrentAccount = getAccountConfiguration();
        assertNotNull(mCurrentAccount);
        prepareReceiver();
        prepareApiManager();
        prepareRegistrationApi();
        prepareParameters();
        performRegister();
        prepareMessagingApi();
        prepareEventsLogApi();
    }

    public void testCase1_One2OneChatReceived() throws InterruptedException, ClientApiException,
            RemoteException {
        Logger.v(TAG, "testCase1_One2OneChatReceived()");
        IChatSession chatSession = mMessagingApi.initiateOne2OneChatSession(
                mCurrentAccount.mAccountName, TEST_TEXT_INVITE);
        FirstMessageObserver sentObserver = new FirstMessageObserver(chatSession);
        ReceivedMessageObserver receivedObserver = new ReceivedMessageObserver();
        mCurrentReceiver.setOnReceiveListener(receivedObserver);
        chatSession.addSessionListener(sentObserver);
        receivedObserver.waitForReceivedInvitation();
    }

    public void testCase2_One2OneChatSessionStarted() throws InterruptedException,
            ClientApiException, RemoteException {
        Logger.v(TAG, "testCase2_One2OneChatSessionStarted()");
        IChatSession chatSession = mMessagingApi.initiateOne2OneChatSession(
                mCurrentAccount.mAccountName, TEST_TEXT_INVITE);
        FirstMessageObserver sentObserver = new FirstMessageObserver(chatSession);
        ReceivedMessageObserver receivedObserver = new ReceivedMessageObserver();
        mCurrentReceiver.setOnReceiveListener(receivedObserver);
        chatSession.addSessionListener(sentObserver);
        sentObserver.waitForSessionStarted();
    }

    public void testCase3_One2OneChatDelivered() throws InterruptedException, ClientApiException,
            RemoteException {
        Logger.v(TAG, "testCase3_One2OneChatDelivered()");
        IChatSession chatSession = mMessagingApi.initiateOne2OneChatSession(
                mCurrentAccount.mAccountName, TEST_TEXT_INVITE);
        FirstMessageObserver sentObserver = new FirstMessageObserver(chatSession);
        ReceivedMessageObserver receivedObserver = new ReceivedMessageObserver();
        mCurrentReceiver.setOnReceiveListener(receivedObserver);
        chatSession.addSessionListener(sentObserver);
        sentObserver.waitForDelivered();
    }

    public void testCase4_One2OneChatDisplayed() throws InterruptedException, ClientApiException,
            RemoteException {
        Logger.v(TAG, "testCase4_One2OneChatDisplayed()");
        IChatSession chatSession = mMessagingApi.initiateOne2OneChatSession(
                mCurrentAccount.mAccountName, TEST_TEXT_INVITE);
        FirstMessageObserver sentObserver = new FirstMessageObserver(chatSession);
        ReceivedMessageObserver receivedObserver = new ReceivedMessageObserver();
        mCurrentReceiver.setOnReceiveListener(receivedObserver);
        chatSession.addSessionListener(sentObserver);
        sentObserver.waitForDisplayed();
    }

    public void testCase5_One2OneChatMsrp() throws InterruptedException, ClientApiException,
            RemoteException {
        Logger.v(TAG, "testCase5_One2OneChatMsrp()");
        IChatSession chatSession = mMessagingApi.initiateOne2OneChatSession(
                mCurrentAccount.mAccountName, TEST_TEXT_INVITE);
        FirstMessageObserver sentObserver = new FirstMessageObserver(chatSession);
        ReceivedMessageObserver receivedObserver = new ReceivedMessageObserver();
        receivedObserver.setResponseMessage(TEST_TEXT_RESPONSE);
        mCurrentReceiver.setOnReceiveListener(receivedObserver);
        chatSession.addSessionListener(sentObserver);
        sentObserver.waitForSessionStarted();
        sentObserver.waitForReceivedMessage(TEST_TEXT_RESPONSE);
    }

    private void prepareReceiver() {
        mCurrentReceiver = new NetworkTestReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(MessagingApiIntents.CHAT_INVITATION);
        mContext.registerReceiver(mCurrentReceiver, filter);
    }

    private void prepareApiManager() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, InterruptedException {
        Method initializeMethod = ApiManager.class.getDeclaredMethod(METHOD_NAME_INITIALIZE,
                Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, mContext);
    }

    private void prepareRegistrationApi() throws InterruptedException {
        mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
        int overAllWaitingTime = 0;
        while (null == mRegistrationApi && overAllWaitingTime < TIME_OUT_API) {
            Thread.sleep(TIME_PIECE_FOR_WAIT);
            overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
        }
        assertNotNull(mRegistrationApi);
    }

    private void prepareMessagingApi() throws InterruptedException {
        mMessagingApi = ApiManager.getInstance().getMessagingApi();
        int overAllWaitingTime = 0;
        while (null == mMessagingApi && overAllWaitingTime < TIME_OUT_API) {
            Thread.sleep(TIME_PIECE_FOR_WAIT);
            overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            mMessagingApi = ApiManager.getInstance().getMessagingApi();
        }
        assertNotNull(mMessagingApi);
    }

    private void prepareEventsLogApi() throws InterruptedException {
        RichMessaging.createInstance(mContext);
        mEventsLogApi = ApiManager.getInstance().getEventsLogApi();
        int overAllWaitingTime = 0;
        while (null == mEventsLogApi && overAllWaitingTime < TIME_OUT_API) {
            Thread.sleep(TIME_PIECE_FOR_WAIT);
            overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            mEventsLogApi = ApiManager.getInstance().getEventsLogApi();
        }
        assertNotNull(mEventsLogApi);
        mEventsLogApi.clearHistoryForContact(mCurrentAccount.mAccountName);
    }

    private void prepareParameters() {
        RcsSettings.createInstance(mContext);
        RcsSettings settings = RcsSettings.getInstance();
        settings.setUserProfileImsUserName(mCurrentAccount.mAccountName);
        settings.setUserProfileImsDomain(mCurrentAccount.mDomain);
        settings.setUserProfileImsDisplayName(mCurrentAccount.mAccountDisplayName);
        String privateIdString = mCurrentAccount.mAccountName + "@" + mCurrentAccount.mDomain;
        if (privateIdString.startsWith("+34")) {
            privateIdString = privateIdString.substring(1);
        }
        settings.setUserProfileImsPrivateId(privateIdString);
        settings.setUserProfileImsPassword(mCurrentAccount.mAccountPassword);
        settings.setImsProxyAddrForMobile(mCurrentAccount.mSipProxyAddress);
        settings.setImsProxyAddrForWifi(mCurrentAccount.mSipProxyAddress);
        settings.setImsProxyPortForMobile(Integer.parseInt(mCurrentAccount.mSipProxyPort));
        settings.setImsProxyPortForWifi(Integer.parseInt(mCurrentAccount.mSipProxyPort));
        settings.writeParameter(RcsSettingsData.SIP_DEFAULT_PORT, mCurrentAccount.mSipProxyPort);
    }

    @Override
    protected void tearDown() throws Exception {
        performUnRegister();
        mContext.unregisterReceiver(mCurrentReceiver);
        super.tearDown();
    }

    private synchronized void performRegister() throws InterruptedException {
        if (RcsSettings.getInstance().isServiceActivated()) {
            RcsSettings.getInstance().setServiceActivationState(false);
            LauncherUtils.stopRcsService(mContext);
            Thread.sleep(TIME_OUT_API);
        }
        RcsSettings.getInstance().setServiceActivationState(true);
        LauncherUtils.launchRcsCoreService(mContext);
        mRegistrationApi.addRegistrationStatusListener(this);
        wait(TIME_OUT_REGISTRATION);
        assertTrue(mIsRegistered);
    }

    private synchronized void performUnRegister() throws InterruptedException {
        RcsSettings.getInstance().setServiceActivationState(false);
        LauncherUtils.stopRcsService(mContext);
        wait(TIME_OUT_REGISTRATION);
        assertFalse(mIsRegistered);
        mRegistrationApi.removeRegistrationStatusListener(this);
    }

    private boolean mIsRegistered = false;

    @Override
    synchronized public void onStatusChanged(boolean status) {
        mIsRegistered = status;
        notifyAll();
    }

    private class FirstMessageObserver extends IChatEventListener.Stub {

        private final static String TAG = "FirstMessageObserver";
        private InstantMessage mFirstMessage = null;

        private IChatSession mChatSession = null;

        private Boolean mIsSessionStarted = false;

        private Boolean mIsDelivered = false;

        private Boolean mIsDisplayed = false;

        private ArrayList<String> mReceivedMessageList = new ArrayList<String>();

        public void waitForSessionStarted() throws InterruptedException {
            int overAllWaitingTime = 0;
            while (!mIsSessionStarted && overAllWaitingTime < TIME_OUT_SESSION_STARTED) {
                Thread.sleep(TIME_PIECE_FOR_WAIT);
                overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            }
            assertTrue(mIsSessionStarted);
        }

        public void waitForDelivered() throws InterruptedException {
            int overAllWaitingTime = 0;
            while (!mIsDelivered && overAllWaitingTime < TIME_OUT_DELIVERY) {
                Thread.sleep(TIME_PIECE_FOR_WAIT);
                overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            }
            assertTrue(mIsDelivered);
        }

        public void waitForDisplayed() throws InterruptedException {
            int overAllWaitingTime = 0;
            while (!mIsDisplayed && overAllWaitingTime < TIME_OUT_DISPLAYED) {
                Thread.sleep(TIME_PIECE_FOR_WAIT);
                overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            }
            assertTrue(mIsDisplayed);
        }

        public void waitForReceivedMessage(String expectedResponse) throws InterruptedException {
            int overAllWaitingTime = 0;
            boolean isReceivedExceptedResponse = mReceivedMessageList.contains(expectedResponse);
            while (!isReceivedExceptedResponse && overAllWaitingTime < TIME_OUT_SESSION_STARTED) {
                Thread.sleep(TIME_PIECE_FOR_WAIT);
                overAllWaitingTime += TIME_PIECE_FOR_WAIT;
                isReceivedExceptedResponse = mReceivedMessageList.contains(expectedResponse);
            }
            assertTrue(isReceivedExceptedResponse);
        }

        private FirstMessageObserver(IChatSession chatSession) throws RemoteException {
            mChatSession = chatSession;
            mFirstMessage = mChatSession.getFirstMessage();
        }

        @Override
        public void handleAddParticipantFailed(String reason) throws RemoteException {
        }

        @Override
        public void handleMessageDeliveryStatus(String msgId, String status, String contact,long date) {
        
        }

        @Override
        public void handleAddParticipantSuccessful() throws RemoteException {
        }

        @Override
        public void handleConferenceEvent(String contact, String contactDisplayname, String state)
                throws RemoteException {
        }

        @Override
        public void handleImError(int error) throws RemoteException {
        }

        @Override
        public void handleIsComposingEvent(String contact, boolean status) throws RemoteException {
        }

        synchronized public void handleMessageDeliveryStatus(String msgId, String status, long date)
                throws RemoteException {
            String targetMessageId = mFirstMessage.getMessageId();
            if (msgId.equals(targetMessageId)) {
                if (ImdnDocument.DELIVERY_STATUS_DELIVERED.equals(status)) {
                    Logger.d(TAG,
                            "handleMessageDeliveryStatus() receive delivered notification for message "
                                    + mFirstMessage.getTextMessage());
                    mIsDelivered = true;
                } else if (ImdnDocument.DELIVERY_STATUS_DISPLAYED.equals(status)) {
                    Logger.d(TAG,
                            "handleMessageDeliveryStatus() receive displayed notification for message "
                                    + mFirstMessage.getTextMessage());
                    mIsDisplayed = true;
                } else {
                    Logger.e(TAG,
                            "handleMessageDeliveryStatus() receive unknown notification for message "
                                    + mFirstMessage.getTextMessage());
                }
            } else {
                Logger.d(TAG, "handleMessageDeliveryStatus() received message id is " + msgId
                        + ", but target id is " + targetMessageId);
            }
        }

        @Override
        public void handleReceiveMessage(InstantMessage msg) throws RemoteException {
            mReceivedMessageList.add(msg.getTextMessage().trim());
        }

        @Override
        public void handleSessionStarted() throws RemoteException {
            mIsSessionStarted = true;
        }

        @Override
        public void handleSessionTerminatedByRemote() throws RemoteException {
        }

		@Override
		public void handleSessionAborted(int reason) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleReceiveGeoloc(GeolocMessage geoloc)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}
    }

    private class ReceivedMessageObserver extends IChatEventListener.Stub implements
            NetworkTestReceiver.IOnReceiveListener {
        private final static String TAG = "ReceivedMessageObserver";

        private IChatSession mCurrentSession = null;

        private String mResponse = null;

        private Boolean mIsReceivedInvitation = false;

        private String mResponseMessageId = null;

        public void waitForReceivedInvitation() throws InterruptedException {
            int overAllWaitingTime = 0;
            while (!mIsReceivedInvitation && overAllWaitingTime < TIME_OUT_RECEIVED) {
                Thread.sleep(TIME_PIECE_FOR_WAIT);
                overAllWaitingTime += TIME_PIECE_FOR_WAIT;
            }
            assertTrue(mIsReceivedInvitation);
        }

        public void setResponseMessage(String response) {
            mResponse = response;
        }

        @Override
        public void onReceive(Intent intent) {
            String action = intent.getAction();
            assertEquals(action, MessagingApiIntents.CHAT_INVITATION);
            String sessionId = intent.getStringExtra(SESSION_ID);
            try {
                mCurrentSession = mMessagingApi.getChatSession(sessionId);
                InstantMessage firstMessage = mCurrentSession.getFirstMessage();
                String remoteContact = PhoneUtils.extractNumberFromUri(firstMessage.getRemote());
                if (mCurrentAccount.mAccountName.equals(remoteContact)) {
                    assertEquals(TEST_TEXT_INVITE, firstMessage.getTextMessage());
                    mIsReceivedInvitation = true;
                } else {
                    Logger
                            .d(
                                    TAG,
                                    "onReceive() receive a message from an different account "
                                            + remoteContact
                                            + ", but current account is mCurrentAccount.mAccountName, so ignore it");
                    return;
                }
                mCurrentSession.addSessionListener(this);
                mCurrentSession.acceptSession();
                mCurrentSession.setMessageDeliveryStatus(firstMessage.getMessageId(),
                        ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                Logger.v(TAG,
                        "onReceive() send out displayed notification for the first invitation");
            } catch (ClientApiException e) {
                e.printStackTrace();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void handleAddParticipantFailed(String reason) throws RemoteException {
        }

        @Override
        public void handleAddParticipantSuccessful() throws RemoteException {
        }

        @Override
        public void handleConferenceEvent(String contact, String contactDisplayname, String state)
                throws RemoteException {
        }

        @Override
        public void handleImError(int error) throws RemoteException {
        }

        @Override
        public void handleMessageDeliveryStatus(String msgId, String status, String contact,long date) {
        
        }

        @Override
        public void handleIsComposingEvent(String contact, boolean status) throws RemoteException {
        }

        public void handleMessageDeliveryStatus(String msgId, String status, long date) throws RemoteException {
        }

        @Override
        public void handleReceiveMessage(InstantMessage msg) throws RemoteException {
        }


        @Override
        public void handleSessionStarted() throws RemoteException {
            if (null != mResponse) {
                mResponseMessageId = mCurrentSession.sendMessage(mResponse);
                assertNotNull(mResponseMessageId);
            }
        }

        @Override
        public void handleSessionTerminatedByRemote() throws RemoteException {
        }

		@Override
		public void handleSessionAborted(int reason) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void handleReceiveGeoloc(GeolocMessage geoloc)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}
    }

    protected static class NetworkTestReceiver extends BroadcastReceiver {
        public static interface IOnReceiveListener {
            void onReceive(Intent intent);
        }

        public void setOnReceiveListener(IOnReceiveListener listener) {
            mCurrentListener = listener;
        }

        private IOnReceiveListener mCurrentListener = null;

        @Override
        public void onReceive(final Context context, final Intent intent) {
            if (null != mCurrentListener) {
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        mCurrentListener.onReceive(intent);
                    }
                });
            }
        }
    }
}
