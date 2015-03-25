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

package com.mediatek.rcse.test.service;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ExchangeMyCapability;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

/**
 * This class is used to test ExchangeMyCapability
 */
public class ExchangeMyCapabilityTest extends InstrumentationTestCase {
    private final static String TAG = "ExchangeMyCapabilityTest";
    private static final String MOCK_MESSAGE_ID = "mock message id";
    private static final String MOCK_CONTACT_NUMBER = "3420000090";
    private static final int SLEEP_TIME = 200;
    private ExchangeMyCapability mExchangeMyCapability = null;
    private Context mContext = null;
    private HashSet<String> mContactList = null;

    @SuppressWarnings("unchecked")
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        mExchangeMyCapability = ExchangeMyCapability.getInstance(mContext);
        Field fieldmContactList = Utils.getPrivateField(
                mExchangeMyCapability.getClass(), "mContactList");
        mContactList = (HashSet<String>) fieldmContactList
                .get(mExchangeMyCapability);
        Thread.sleep(SLEEP_TIME);

    }

    /**
     * Test SdcardReceiver
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase01_addImContactList() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase01_addImContactList() entry");
        Method methodaddImContactList = Utils.getPrivateMethod(
                mExchangeMyCapability.getClass(), "addImContactList",
                List.class);
        List<IBinder> sessions = new ArrayList<IBinder>();
        MockChatSession chatSession = new MockChatSession();
        sessions.add(chatSession);
        methodaddImContactList.invoke(mExchangeMyCapability, sessions);
        assertTrue(mContactList.containsAll(chatSession.mParticipantList));
    }

    /**
     * Test addFileTransferContactListt
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase02_addFileTransferContactListt()
            throws NoSuchMethodException, IllegalArgumentException,
            InstantiationException, IllegalAccessException,
            InvocationTargetException, InterruptedException {
        Logger.d(TAG, "testCase02_addFileTransferContactListt() entry");
        Method methodaddImContactList = Utils.getPrivateMethod(
                mExchangeMyCapability.getClass(), "addFileTransferContactList",
                List.class);
        List<IBinder> sessions = new ArrayList<IBinder>();
        MockFtSession ftSession = new MockFtSession();
        sessions.add(ftSession);
        methodaddImContactList.invoke(mExchangeMyCapability, sessions);
        assertTrue(mContactList.contains(MOCK_CONTACT_NUMBER));
    }

    /**
     * Test doCapabilityChanged
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_doCapabilityChanged() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase03_doCapabilityChanged() entry");
        Method methoddoCapabilityChanged = Utils.getPrivateMethod(
                mExchangeMyCapability.getClass(), "doCapabilityChanged",
                int.class, boolean.class);
        methoddoCapabilityChanged.invoke(mExchangeMyCapability, 3, true);
        methoddoCapabilityChanged.invoke(mExchangeMyCapability, 4, true);
        methoddoCapabilityChanged.invoke(mExchangeMyCapability, 0, true);
    }

    /**
     * Test getContactListOngoingRichCal
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase04_getContactListOngoingRichCall()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase04_getContactListOngoingRichCall() entry");
        Method methodgetContactListOngoingRichCall = Utils.getPrivateMethod(
                mExchangeMyCapability.getClass(),
                "getContactListOngoingRichCall");
        methodgetContactListOngoingRichCall.invoke(mExchangeMyCapability);
    }

    /**
     * Test getContactListOngoingImSessions
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InvocationTargetException
     */
    public void testCase05_getContactListOngoingImSessions()
            throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase05_getContactListOngoingImSessions() entry");
        ApiManager.initialize(mContext);
        if (ApiManager.getInstance().getMessagingApi() == null) {
            Field fieldmMessagingApi = Utils.getPrivateField(ApiManager
                    .getInstance().getClass(), "mMessagingApi");
            fieldmMessagingApi.set(ApiManager.getInstance(), new MessagingApi(
                    mContext));
        }
        Method methodgetContactListOngoingImSessions = Utils.getPrivateMethod(
                mExchangeMyCapability.getClass(),
                "getContactListOngoingRichCall");
        methodgetContactListOngoingImSessions.invoke(mExchangeMyCapability);
        Method methodgetContactListOngoingFileTransferSessions = Utils
                .getPrivateMethod(mExchangeMyCapability.getClass(),
                        "getContactListOngoingFileTransferSessions");
        methodgetContactListOngoingFileTransferSessions
                .invoke(mExchangeMyCapability);
    }

    /**
     * Mock chat session that always stay in connected
     */
    private class MockChatSession extends IChatSession.Stub {
        private ArrayList<String> mParticipantList = new ArrayList<String>();

        public MockChatSession() {
            mParticipantList.add(MOCK_CONTACT_NUMBER);
        }

        @Override
        public void acceptSession() throws RemoteException {
        }

        @Override
        public void addParticipant(String participant) throws RemoteException {
            mParticipantList.add(participant);
        }

        @Override
        public void addParticipants(List<String> participants)
                throws RemoteException {
            mParticipantList.addAll(participants);
        }

        @Override
        public void addSessionListener(IChatEventListener listener)
                throws RemoteException {
        }

        @Override
        public void cancelSession() throws RemoteException {
        }

        @Override
        public String getChatID() throws RemoteException {
            return null;
        }

        @Override
        public InstantMessage getFirstMessage() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getInivtedParticipants() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getParticipants() throws RemoteException {
            return mParticipantList;
        }

        @Override
        public String getReferredByHeader() throws RemoteException {
            return null;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return MOCK_CONTACT_NUMBER;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return null;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return SessionState.ESTABLISHED;
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
        public boolean isInComing() throws RemoteException {
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
        public void removeSessionListener(IChatEventListener listener)
                throws RemoteException {
        }

        @Override
        public String sendMessage(String text) throws RemoteException {
            return MOCK_MESSAGE_ID;
        }

        @Override
        public void setIsComposingStatus(boolean status) throws RemoteException {
        }

        @Override
        public void setMessageDeliveryStatus(String msgId, String status)
                throws RemoteException {
        }

        @Override
        public void setMessageDisplayedStatusBySipMessage(String contact,
                String msgId, String status) throws RemoteException {
        }

        @Override
        public IBinder asBinder() {
            return this;
        }

        public String getSubject() throws RemoteException {
            return null;
        }

        @Override
        public int getMaxParticipants() throws RemoteException {
            return 1;
        }

        @Override
        public int getMaxParticipantsToBeAdded() throws RemoteException {
            return 10;
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

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}
    }

    private class MockFtSession extends IFileTransferSession.Stub {

        @Override
        public String getSessionID() throws RemoteException {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return MOCK_CONTACT_NUMBER;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return 0;
        }

        @Override
        public String getFilename() throws RemoteException {
            return "test.txt";
        }

        @Override
        public long getFilesize() throws RemoteException {
            return 10;
        }

        @Override
        public void acceptSession() throws RemoteException {

        }

        @Override
        public void rejectSession() throws RemoteException {

        }

        @Override
        public void cancelSession() throws RemoteException {

        }

        @Override
        public void addSessionListener(IFileTransferEventListener listener)
                throws RemoteException {

        }

        @Override
        public void removeSessionListener(IFileTransferEventListener listener)
                throws RemoteException {

        }

		@Override
		public List<String> getContacts() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public boolean isGroupTransfer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public boolean isHttpTransfer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public String getChatID() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public String getChatSessionID() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public byte[] getFileThumbnail() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void pauseSession() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void resumeSession() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public String getFileThumbUrl() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public boolean isSessionPaused() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

    }

}
