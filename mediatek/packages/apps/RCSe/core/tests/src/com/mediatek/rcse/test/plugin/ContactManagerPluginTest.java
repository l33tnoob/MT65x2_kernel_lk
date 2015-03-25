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

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts.Data;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.plugin.message.IpMessageContactManager;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginOne2OneChatWindow;
import com.mediatek.rcse.plugin.message.PluginOne2OneChatWindow.RcseContactStatus;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.binder.IRemoteGroupChatWindow;
import com.mediatek.rcse.service.binder.TagTranslater;
import com.mediatek.rcse.service.binder.ThreadTranslater;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * This class is used to test IpMessageContactManager.
 */
public class ContactManagerPluginTest extends InstrumentationTestCase {
    private static final String TAG = "ContactManagerPluginTest";
    private Context mContext = null;
    private IpMessageContactManager mIpMessageContactManager = null;
    private IpMessageManager mIpMessageManager = null;
    private static final String NAME = "dummy";
    private static final String MOCK_NUMBER = "+34200000123";
    private static final Participant MOCK_PARTICIPANT = new Participant(MOCK_NUMBER, MOCK_NUMBER);
    private static final short CONTACT_ID = (short) 1;
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 6000;
    private static final String CAPABILITY_API = "mCapabilitiesApi";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MOCK_GROUP_TAG = "12345";
    private static final Long MOCK_THREAD_ID = 100L;
    private static final String MOCK_NUMBER_ONE = "+3444444221";
    private static final String MOCK_NUMBER_TWO = "+3444444222";
    private static final String MOCK_NAME_ONE = "group1";
    private static final String MOCK_NAME_TWO = "group2";
    private ContentResolver mContentResolver = null;
    private static final String RCS_CONTACT = "1";
    private static final String MIMETYPE_RCS_CONTACT =
            "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";

    private short mContactId = Short.MIN_VALUE;
    private long mRawContactId = -1;
    private static final String DEFAULT_GROUP_NAME = "Group chat";
    private static final String MOCK_WINDOW_TAG = "mock window tag";

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mContentResolver = mContext.getContentResolver();
        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        Logger.d(TAG, "setUp(), rawContactId is " + rawContactId);
        addContact(values, rawContactId, MOCK_NUMBER);
        Short contactId = getContactIdByNumber(MOCK_NUMBER);
        Logger.d(TAG, "setUp(), contactId is " + contactId);
        mContactId = contactId;
        mIpMessageManager = new IpMessageManager(mContext);
        mIpMessageContactManager = new IpMessageContactManager(mContext);
        Field apiManagerfield = ApiManager.class.getDeclaredField(API_MANAGER_INSTANCE);
        apiManagerfield.setAccessible(true);
        apiManagerfield.set(ApiManager.class, null);
        Method initializeMethod =
                ApiManager.class.getDeclaredMethod(API_MANAGER_INITIALIZE, Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, mContext);
        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull("setUp apiManager is null", apiManager);
        CapabilityApi capabilityApi = apiManager.getCapabilityApi();
        long startTime = System.currentTimeMillis();
        while (capabilityApi == null) {
            Logger.d(TAG, "setUp() capabilityApi is null");
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            capabilityApi = apiManager.getCapabilityApi();
        }
        Field capabilityApiField = Utils.getPrivateField(ApiManager.class, CAPABILITY_API);
        MockCapabilityApi mockCapabilityApi = new MockCapabilityApi(mContext);
        capabilityApiField.set(apiManager, mockCapabilityApi);
        Logger.d(TAG, "setUp() exit");
    }

    /**
     * Test get contactId by a special phone number
     * 
     * @throws InterruptedException
     */
    public void testCase1_GetContactIdByNumber() throws InterruptedException {
        Logger.d(TAG, "testCase1_GetContactIdByNumber() entry");
        waitForLoad(mContactId);
        Logger.d(TAG, "testCase1_GetContactIdByNumber() exit");
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown() entry");
        deleteContact();
        Logger.d(TAG, "tearDown() exit");
        super.tearDown();
    }

    /**
     * Test if the contact number is RCSe contact.
     */
    public void testCase2_IsIpMessageNumber() {
        Logger.d(TAG, "testCase2_IsIpMessageNumber() entry");
        List<RcsContact> CONTACTS_LIST = ContactsListManager.getInstance().CONTACTS_LIST;
        RcsContact rcsContact = new RcsContact(NAME, MOCK_NUMBER, null, null, CONTACT_ID);
        CONTACTS_LIST.add(rcsContact);
        assertTrue(mIpMessageContactManager.isIpMessageNumber(MOCK_NUMBER));
        CONTACTS_LIST.clear();
        ContactsListManager.getInstance().setStrangerList(MOCK_NUMBER, true);
        assertTrue(mIpMessageContactManager.isIpMessageNumber(MOCK_NUMBER));
        ContactsListManager.getInstance().setStrangerList(MOCK_NUMBER, false);
        assertTrue(mIpMessageContactManager.isIpMessageNumber(MOCK_NUMBER));
        Logger.d(TAG, "testCase2_IsIpMessageNumber() exit");
    }
    
    /**
     * Test get status by a special phone number
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws RemoteException
     */
    public void testCase3_GetNameByThreadId() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, RemoteException {
        Logger.d(TAG, "testCase3_GetNameByThreadId() entry");
        ThreadTranslater.saveThreadandTag(MOCK_THREAD_ID, PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER + MOCK_GROUP_TAG);
        PluginChatWindowManager pluginChatWindowManager = new PluginChatWindowManager(new IpMessageManager(mContext));
        ArrayList<ParticipantInfo> participantList = new ArrayList<ParticipantInfo>();
        ParticipantInfo participantOne = new ParticipantInfo(new Participant(MOCK_NUMBER_ONE, MOCK_NAME_ONE), null);
        ParticipantInfo participantTwo = new ParticipantInfo(new Participant(MOCK_NUMBER_TWO, MOCK_NAME_TWO), null);
        participantList.add(participantOne);
        participantList.add(participantTwo);
        IRemoteGroupChatWindow pluginGroupWindow = pluginChatWindowManager.addGroupChatWindow(MOCK_GROUP_TAG, participantList);
        ContactsListManager.getInstance().CONTACTS_LIST.add(new RcsContact(MOCK_NAME_ONE, MOCK_NUMBER_ONE));
        ContactsListManager.getInstance().CONTACTS_LIST.add(new RcsContact(MOCK_NAME_TWO, MOCK_NUMBER_TWO));
        String name = mIpMessageContactManager.getNameByThreadId(MOCK_THREAD_ID);
        assertEquals(MOCK_NAME_ONE + "," + MOCK_NAME_TWO, name);
        String defaultName = mIpMessageContactManager.getNameByThreadId(Long.MAX_VALUE);
        assertEquals(DEFAULT_GROUP_NAME, defaultName);
        Logger.d(TAG, "testCase3_GetNameByThreadId() exit");
    }

    /**
     * Test get status by a special phone number
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws RemoteException
     */
    public void testCase4_GetStatusByNumber() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, RemoteException {
        Logger.d(TAG, "testCase4_GetStatusByNumber() entry");
        PluginOne2OneChatWindow window = new PluginOne2OneChatWindow(MOCK_PARTICIPANT, MOCK_WINDOW_TAG,
                new IpMessageManager(mContext));
        assertNotNull(window.getPluginChatWindow());
        assertEquals(MOCK_PARTICIPANT, window.getParticipant());
        assertEquals(MOCK_WINDOW_TAG, window.getWindowTag());
        int actualStatus = mIpMessageContactManager.getStatusByNumber(MOCK_NUMBER);
        assertEquals(RcseContactStatus.OFFLINE, actualStatus);
        window.setRemoteOfflineReminder(false);
        actualStatus = mIpMessageContactManager.getStatusByNumber(MOCK_NUMBER);
        assertEquals(RcseContactStatus.ONLINE, actualStatus);
        window.setIsComposing(true);
        actualStatus = mIpMessageContactManager.getStatusByNumber(MOCK_NUMBER);
        assertEquals(RcseContactStatus.TYPING, actualStatus);
        window.setIsComposing(false);
        actualStatus = mIpMessageContactManager.getStatusByNumber(MOCK_NUMBER);
        assertEquals(RcseContactStatus.STOP_TYPING, actualStatus);
        Logger.d(TAG, "testCase4_GetStatusByNumber() exit");
    }

    /**
     * Test to add/delete spam list
     * @throws Throwable
     */
    public void testCase5_testSpamList() throws Throwable {
        assertFalse(mIpMessageContactManager.addContactToSpamList(null));
        Method methodGenerateOrStoreContactId = Utils.getPrivateMethod(IpMessageContactManager.class,
                "generateOrStoreContactId", short.class, String.class);
        Short contactId = (Short) methodGenerateOrStoreContactId.invoke(mIpMessageContactManager, Short.MIN_VALUE, MOCK_NUMBER);
        assertTrue(mIpMessageContactManager.addContactToSpamList(new int[]{contactId}));
        assertTrue(mIpMessageContactManager.deleteContactFromSpamList(new int[]{contactId}));
        contactId = (Short) methodGenerateOrStoreContactId.invoke(mIpMessageContactManager, Short.MIN_VALUE, MOCK_NUMBER + "," + MOCK_NUMBER);
        assertTrue(mIpMessageContactManager.addContactToSpamList(new int[]{contactId}));
        assertTrue(mIpMessageContactManager.deleteContactFromSpamList(new int[]{contactId}));
    }

    /**
     * Test to generate a multi contact avatar
     * @throws Throwable
     */
    public void testCase6_testGetAvatar() throws Throwable {
        assertNotNull(mIpMessageContactManager.getAvatarByThreadId(0L));
    }
    
    /**
     * Test to get group name by group id
     * 
     * @throws Throwable
     */
    public void testCase7_GetNumberByEngineId() throws Throwable {
        Logger.d(TAG, "testCase7_GetNumberByEngineId() entry");
        PluginGroupChatWindow chatWindow = getChatWidow();
        Field filedGroupId = Utils.getPrivateField(PluginGroupChatWindow.class, "mGroupId");
        filedGroupId.set(chatWindow, Short.MAX_VALUE);
        String groupName = chatWindow.getContactString();
        assertEquals(PluginChatWindowManager.getNumberByEngineId(Short.MAX_VALUE), groupName);
        Logger.d(TAG, "testCase7_GetNumberByEngineId() exit");
    }

    /**
     * Test get avatar by number
     * 
     * @throws Throwable
     */
    public void testCase8_GetAvatarByNumber() throws Throwable {
        Logger.d(TAG, "testCase8_GetAvatarByNumber() entry");
        PluginGroupChatWindow chatWindow = getChatWidow();
        Bitmap bitmap = waitForBitMap(chatWindow);
        Bitmap actualBitmap = chatWindow.getAvatarBitmap();
        bitmap.sameAs(actualBitmap);
        Logger.d(TAG, "testCase8_GetAvatarByNumber() exit");
    }
    
    /**
     * Test get name by number
     * 
     * @throws Throwable
     */
    public void testCase9_GetNameByNumber() throws Throwable {
        Logger.d(TAG, "testCase9_GetNameByNumber() entry");
        String actualName = mIpMessageContactManager.getNameByNumber(null);
        assertEquals(DEFAULT_GROUP_NAME, actualName);
        PluginGroupChatWindow chatWindow = getChatWidow();
        Field fieldContactString =
                Utils.getPrivateField(PluginGroupChatWindow.class, "mContactString");
        String name =
                mIpMessageContactManager.getNameByNumber((String) fieldContactString
                        .get(chatWindow));
        assertEquals(MOCK_NAME_ONE, name);
        Logger.d(TAG, "testCase9_GetNameByNumber() exit");
    }
    
    private Bitmap waitForBitMap(PluginGroupChatWindow chatWindow) throws Throwable {
        Logger.d(TAG, "waitForBitMap() entry, with chatWindow is " + chatWindow);
        Field fieldContactString =
                Utils.getPrivateField(PluginGroupChatWindow.class, "mContactString");
        Bitmap bitmap =
                mIpMessageContactManager.getAvatarByNumber((String) fieldContactString
                        .get(chatWindow));
        long startTime = System.currentTimeMillis();
        while (bitmap == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForBitMap() time out");
            }
            Thread.sleep(SLEEP_TIME);
            bitmap =
                    mIpMessageContactManager.getAvatarByNumber((String) fieldContactString
                            .get(chatWindow));
        }
        Logger.d(TAG, "waitForBitMap() exit, with bitmap is " + bitmap);
        return bitmap;
    }

    private PluginGroupChatWindow getChatWidow() throws Throwable {
        List<ParticipantInfo> participants = new ArrayList<ParticipantInfo>();
        Participant participant = new Participant(MOCK_NAME_ONE, MOCK_NAME_ONE);
        participants.add(new ParticipantInfo(participant, User.STATE_PENDING));
        ParcelUuid tag = new ParcelUuid(UUID.randomUUID());
        TagTranslater.saveTag(tag);
        Field fieldWindowList =
                Utils.getPrivateField(PluginChatWindowManager.class, "CHAT_WINDOW_LIST");
        PluginChatWindowManager pluginChatWindowManager =
                new PluginChatWindowManager(mIpMessageManager);
        List chatWindowList = (List) fieldWindowList.get(pluginChatWindowManager);
        PluginGroupChatWindow chatWindow =
                new PluginGroupChatWindow(tag.toString(), mIpMessageManager, participants);
        chatWindowList.add(chatWindow);
        chatWindow.setIsComposing(false, participant);
        chatWindow.addChatEventInformation(null);
        chatWindow.addLoadHistoryHeader(false);
        return chatWindow;
    }

    private class MockCapabilityApi extends CapabilityApi {
        public MockCapabilityApi(Context context) {
            super(context);
        }

        @Override
        public Capabilities getContactCapabilities(String number) {
            Logger.d(TAG, "getContactCapabilities() entry");
            Capabilities capability = new Capabilities();
            if (number.equals(MOCK_NUMBER)) {
                capability.setRcseContact(true);
            } else {
                capability.setRcseContact(false);
            }
            return capability;
        }
    }

    private void addContact(ContentValues values, long rawContactId, String number) {
        Logger.d(TAG, "addContact() entry, rawContactId is " + rawContactId + " and number is "
                + number);
        values.clear();
        values.put(Data.RAW_CONTACT_ID, rawContactId);
        values.put(Data.MIMETYPE, MIMETYPE_RCS_CONTACT);
        values.put(Phone.NUMBER, number);
        values.put(Data.DATA4, RCS_CONTACT);
        mContentResolver.insert(ContactsContract.Data.CONTENT_URI, values);
        Logger.d(TAG, "addContact() exit");
    }

    private void deleteContact() {
        Logger.d(TAG, "deleteContact() entry");
        String[] projection = {
            ContactsContract.Data.RAW_CONTACT_ID
        };
        String selection = Data.RAW_CONTACT_ID + "=" + mRawContactId;
        Cursor cur =
                mContentResolver.query(ContactsContract.Data.CONTENT_URI, projection, selection,
                        null, null);
        Logger.d(TAG, "deleteContact(), cur.getCount() is " + cur.getCount());
        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        if (cur.moveToFirst()) {
            do {
                long id = cur.getLong(cur.getColumnIndex(ContactsContract.Data.RAW_CONTACT_ID));
                ops.add(ContentProviderOperation.newDelete(
                        ContentUris.withAppendedId(RawContacts.CONTENT_URI, id)).build());
                try {
                    mContentResolver.applyBatch(ContactsContract.AUTHORITY, ops);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } while (cur.moveToNext());
            cur.close();
        }
        Logger.d(TAG, "deleteContact() exit");
    }

    private short getContactIdByNumber(String number) {
        Logger.d(TAG, "getContactIdByNumber() entry, number is " + number);
        short contactId = Short.MIN_VALUE;
        String[] projection = {
                Phone.CONTACT_ID, Data.DATA1
        };

        Cursor cur =
                mContentResolver.query(ContactsContract.Data.CONTENT_URI, projection, null, null,
                        null);
        Logger.d(TAG, "getContactIdByNumber() entry, cur.getCount() is " + cur.getCount());
        try {
            if (cur != null && cur.moveToFirst()) {
                while (!cur.isAfterLast()) {
                    if (PhoneUtils.compareNumbers(number, cur.getString(1))) {
                        contactId = cur.getShort(0);
                    }
                    cur.moveToNext();
                }
            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
        Logger.d(TAG, "getContactIdByNumber() contactId is " + contactId);
        return contactId;
    }

    private void waitForLoad(int contactId) throws InterruptedException {
        Logger.d(TAG, "waitForLoad() entry, with contactId is " + contactId);
        long startTime = System.currentTimeMillis();
        do {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (mIpMessageContactManager.getContactIdByNumber(MOCK_NUMBER) != contactId);
    }
}
