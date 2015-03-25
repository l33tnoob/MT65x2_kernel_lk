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

package com.mediatek.rcse.test.fragments;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.test.ActivityInstrumentationTestCase2;
import android.text.TextUtils;
import android.view.ActionMode;
import android.view.ActionProvider;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.ListView;
import android.widget.AbsListView.MultiChoiceModeListener;
import android.widget.AbsListView.OnScrollListener;

import com.mediatek.rcse.activities.BaseListFragment;
import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.ContactBaseAdapter;
import com.mediatek.rcse.activities.ContactsAdapter;
import com.mediatek.rcse.activities.ContactsListFragment;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.GroupChat.GroupChatParticipants;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;

/**
 * This class is used to test the functions of ContactsListFragment
 */
public class ContactsListFragmentTest extends
        ActivityInstrumentationTestCase2<SelectContactsActivity> {

    private static final String TAG = "ContactsListFragmentTest";
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;
    private static final String MIMETYPE_RCS_CONTACT = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    private static final int RCS_CONTACT = 1;
    private static final int RCS_CAPABLE_CONTACT = 2;
    private static final String[] projection = {
        Data.DATA1
    };

    private static final String selection = Data.MIMETYPE + "=? AND " + "(" + Data.DATA4 + "=? OR "
            + Data.DATA4 + "=? )";
    private static final String[] selectionArgs = {
            MIMETYPE_RCS_CONTACT, Long.toString(RCS_CONTACT), Long.toString(RCS_CAPABLE_CONTACT)
    };
    public static final String sortOrder = Contacts.SORT_KEY_PRIMARY;
    public final static String KEY_ADD_CONTACTS = "addContacts";

    private ActivityMonitor mSelectContactActivityMonitor = new ActivityMonitor(
            SelectContactsActivity.class.getName(), null, false);
    private SelectContactsActivity mActivity = null;
    private ContactsListFragment mFragment = null;
    private ArrayList<Participant> mContactsList = new ArrayList<Participant>();

    public ContactsListFragmentTest() {
        super(SelectContactsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Context context = getInstrumentation().getTargetContext();
        assertNotNull(context);
        if (ContactsListManager.getInstance() != null && ContactsListManager.getInstance().CONTACTS_LIST.size() == 0) {
            Logger.v(TAG, "setUp(): reset ContactsListManager");
            try {
                Field instance = Utils.getPrivateField(ContactsListManager.class, "sInstance");
                instance.set(ContactsListManager.class, null);
            } catch (Exception e) {
                e.printStackTrace();
            }
            getInstrumentation().waitForIdleSync();
        }
        ContactsListManager.initialize(getInstrumentation().getTargetContext());
        getInstrumentation().waitForIdleSync();
        mActivity = getActivity();
        Class<? extends SelectContactsActivity> clazz = mActivity.getClass();
        assertTrue(clazz != null);
        Field fragmentField = Utils.getPrivateField(SelectContactsActivity.class,
                "mContactsListFragment");
        mFragment = (ContactsListFragment) fragmentField.get(mActivity);
    }

    /**
     * Test the contacts' number
     */
    @SuppressWarnings("unchecked")
    public void testCase01_testRcseContacts() throws Throwable {
        Logger.v(TAG, "testCase01_testRcseContacts()");
        String firstContactNumber = waitForLoadContacts();
        assertTrue(!TextUtils.isEmpty(firstContactNumber));
        Method methodGetOne2OneChat = Utils.getPrivateMethod(ModelImpl.class, "getOne2OneChat",
                Participant.class);
        assertNotNull(methodGetOne2OneChat);
        Field fieldChatMap = Utils.getPrivateField(ModelImpl.class, "mChatMap");
        Map<Object, IChat> chatMap = (Map<Object, IChat>) fieldChatMap.get(ModelImpl.getInstance());
        chatMap.clear();
        Object chat = methodGetOne2OneChat.invoke(ModelImpl.getInstance(), new Participant(
                firstContactNumber, firstContactNumber));
        assertNull(chat);
        Method methodStartOne2OneChat = Utils.getPrivateMethod(ContactsListFragment.class,
                "startOne2OneChat", Participant.class);
        assertNotNull(methodStartOne2OneChat);
        methodStartOne2OneChat.invoke(mFragment, new Participant(firstContactNumber,
                firstContactNumber));
        Activity chatScreen = mSelectContactActivityMonitor.waitForActivityWithTimeout(TIME_OUT);
        try {
            long startTime = System.currentTimeMillis();
            do {
                chat = methodGetOne2OneChat.invoke(ModelImpl.getInstance(), new Participant(
                        firstContactNumber, firstContactNumber));
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (null == chat);
        } finally {
            assertNotNull(chat);
            ModelImpl.getInstance().removeChat(((One2OneChat) chat).getChatTag());
            if (null != chatScreen) {
                chatScreen.finish();
            }
        }
    }

    /**
     * Test the contacts match contact database
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase02_testRcseContactsFromDatabase() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.v(TAG, "testCase02_testRcseContactsFromDatabase()");
        ListView contactListView = mFragment.getListView();
        int listViewCount = contactListView.getCount();
        Logger.d(TAG,
                "testCase2_testRcseContactsFromDatabase, (from Listview)rcse contacts count = "
                        + listViewCount);
        List<String> numberList = new ArrayList<String>();
        Cursor cursor = mActivity.getContentResolver().query(Data.CONTENT_URI, projection,
                selection, selectionArgs, sortOrder);
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    String phoneNumber = cursor.getString(cursor.getColumnIndex(Data.DATA1));
                    numberList.add(phoneNumber);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.d(
                TAG,
                "testCase02_testRcseContactsFromDatabase, (from CONTACTS_LIST)rcse contacts from database, size is "
                        + numberList.size());

        // check the contacts in the database
        for (String number : numberList) {
            int index = 0;
            assertNotNull(number);
            while (index < listViewCount) {
                Object item = contactListView.getItemAtPosition(index);
                assertNotNull(item);
                Field fieldNumber = Utils.getPrivateField(item.getClass(), "mNumber");
                String contactNumber = (String) fieldNumber.get(item);
                if (contactNumber == null) {
                    index++;
                    continue;
                }
                if (contactNumber.equals(number)) {
                    Logger.d(TAG, "testCase02_testRcseContactsFromDatabase, equals, index = "
                            + index + " contactNumber = " + contactNumber + ", number = " + number);
                    break;
                }
                Logger.d(TAG, "testCase02_testRcseContactsFromDatabase, not equals, index = "
                        + index + " contactNumber = " + contactNumber + ", number = " + number);
                index++;
            }
            assertTrue(
                    "testCase02_testRcseContactsFromDatabase, Number not shown in the list! number = "
                            + number, index <= listViewCount);
        }
    }

    /**
     * Test to block and unblock a contact
     */
    public void testCase03_testBlockContact() throws NoSuchFieldException, NoSuchMethodException,
            IllegalAccessException, InterruptedException {
        Logger.v(TAG, "testCase03_testBlockContact() entry");
        String contact = waitForLoadContacts();
        Logger.d(TAG, "testCase03_testBlockContact(), the contact is " + contact);
        assertTrue(!TextUtils.isEmpty(contact));
        ContactsManager contactsManager = ContactsManager.getInstance();
        boolean isBlocked = false;
        boolean result = false;
        isBlocked = contactsManager.isImBlockedForContact(contact);
        assertFalse("testCase03_testBlockContact(), the contact " + contact
                + " has not been blocked", isBlocked);
        contactsManager.setImBlockedForContact(contact, true);
        assertTrue("testCase03_testBlockContact(), block contact " + contact + " failed", result);
        isBlocked = contactsManager.isImBlockedForContact(contact);
        assertTrue("testCase03_testBlockContact(), the contact " + contact
                + " has not been blocked", isBlocked);
        contactsManager.setImBlockedForContact(contact, false);
        assertTrue("testCase03_testBlockContact(), unblock contact " + contact + " failed", result);
        isBlocked = contactsManager.isImBlockedForContact(contact);
        assertFalse("testCase03_testBlockContact(), the contact " + contact + " has been blocked",
                isBlocked);
        Logger.v(TAG, "testCase03_testBlockContact() exit");
    }

    /**
     * Test the start a group chat
     */
    public void testCase04_testStartGroupChat() throws Throwable {
        Logger.v(TAG, "testCase04_testStartGroupChat()");
        waitForLoadContacts();
        Logger.v(TAG, "mContactsList size is =" + mContactsList.size());
        assertNull(getGroupChat(mContactsList));
        Intent chatIntent = new Intent();
        // Because the contact list may have no RCSe contact, mock 2 contact at
        // least
        if (mContactsList.size() < 2) {
            Logger.v(TAG, "RCSe contacts so little mock some");
            Participant participant1 = new Participant("+34200000100", "test1");
            mContactsList.add(participant1);
            Participant participant2 = new Participant("+34200000101", "test2");
            mContactsList.add(participant2);
        }
        assertTrue(mContactsList.size() > 1);
        chatIntent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, mContactsList);
        Logger.v(TAG, "mTagName is not null, that create a new chat window.");
        chatIntent.putExtra(KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        chatIntent.setClass(mActivity, ChatScreenActivity.class);
        chatIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivity.startActivity(chatIntent);
        Activity chatScreen = mSelectContactActivityMonitor.waitForActivityWithTimeout(TIME_OUT);
        IChat chat = null;
        try {
            long startTime = System.currentTimeMillis();
            do {
                chat = getGroupChat(mContactsList);
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (null == chat);
        } finally {
            assertNotNull(chat);
            ModelImpl.getInstance().removeChat(((GroupChat) chat).getChatTag());
            if (null != chatScreen) {
                chatScreen.finish();
            }
        }
    }

    /**
     * Test whether the max number of group chat participants works
     * 
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws InterruptedException
     * @throws InvocationTargetException
     * @throws NoSuchMethodException
     */
    public void testCase05_AddParticipantsExceed() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException, InterruptedException,
            InvocationTargetException, NoSuchMethodException {
        Logger.v(TAG, "testCase05_AddParticipantsExceed() entry");
        // Prepare some test conditions.
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        Field fieldChatMap = Utils.getPrivateField(modelImpl.getClass(), "mChatMap");
        Map<Object, IChat> chatMap = (Map<Object, IChat>) fieldChatMap.get(modelImpl);
        chatMap.clear();
        Method methodStartChat = Utils.getPrivateMethod(ContactsListFragment.class.getSuperclass(),
                "startChat");
        assertNotNull(methodStartChat);
        ListView contactListView = mFragment.getListView();
        assertNotNull(contactListView);
        ContactBaseAdapter adapter = (ContactBaseAdapter) contactListView.getAdapter();
        assertNotNull(adapter);

        int maxParticipants = RcsSettings.getInstance().getMaxChatParticipants();

        // Put some contacts into the member mExistingContacts in class
        // BaseListFragment.
        mFragment.setIsNeedOriginalContacts(true);
        Field fieldExistingContacts = Utils.getPrivateField(mFragment.getClass().getSuperclass(),
                "mExistingContacts");
        List<RcsContact> mExistingContacts = (List<RcsContact>) fieldExistingContacts
                .get(mFragment);
        String existContactName = "ExistContactName";
        String existContactNum = "+34200000000";
        RcsContact existContact = new RcsContact(existContactName, existContactNum);
        existContact.mSelected = true;
        existContact.mSortKey = "+34200000";
        mExistingContacts.add(existContact);

        // Mock a IPTel contact and add it into contact ListView.
        String iptelContactName = "iptelContactName";
        String iptelContactNum = BaseListFragment.IPTEL_VITUAL_NUMBER + "123456789";
        RcsContact iptelContact = new RcsContact(iptelContactName, iptelContactNum);
        iptelContact.mSelected = true;
        iptelContact.mSortKey = BaseListFragment.IPTEL_VITUAL_NUMBER;
        adapter.getContactsList().add(iptelContact);

        // Mock selecting more contacts than maxParticipants
        generateContacts(adapter, maxParticipants + 2);

        // Make sure the activity finish
        ActivityMonitor monitor = null;
        Activity activity = null;
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(PluginGroupChatActivity.ACTION);
        monitor = new ActivityMonitor(intentFilter, null, false);
        
        try {
            getInstrumentation().addMonitor(monitor);
            
            // Invoke the the method startChat in class BaseListFragment.
            mFragment.startSelectMode(ChatMainActivity.VALUE_ADD_CONTACTS);
            methodStartChat.invoke(mFragment);
            
            checkGroupChatParticipants(maxParticipants);
            activity = monitor.waitForActivityWithTimeout(TIME_OUT);
            
            assertNotNull(activity);
        } finally {
            getInstrumentation().removeMonitor(monitor);
            if (null != activity) {
                activity.finish();
            }
        }

        // Invoke the the method startChat in class BaseListFragment.
        mFragment.startSelectMode(null);
        methodStartChat.invoke(mFragment);
        Intent resultIntent = mFragment.getResult();
        assertNotNull(resultIntent);
        ArrayList<Participant> participants = resultIntent.getParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST);
        assertNotNull(participants);

        Logger.v(TAG, "testCase05_AddParticipantsExceed() exit");
    }

    /**
     * Test adding a Participant.
     * 
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws InterruptedException
     * @throws InvocationTargetException
     * @throws NoSuchMethodException
     */
    public void testCase06_AddAParticipant() throws IllegalArgumentException, NoSuchFieldException,
            IllegalAccessException, InterruptedException, InvocationTargetException,
            NoSuchMethodException {
        Logger.v(TAG, "testCase06_AddAParticipant() entry");
        // Prepare some test conditions.
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        Field fieldChatMap = Utils.getPrivateField(modelImpl.getClass(), "mChatMap");
        Map<Object, IChat> chatMap = (Map<Object, IChat>) fieldChatMap.get(modelImpl);
        chatMap.clear();
        Method methodStartChat = Utils.getPrivateMethod(ContactsListFragment.class.getSuperclass(),
                "startChat");
        assertNotNull(methodStartChat);
        ListView contactListView = mFragment.getListView();
        assertNotNull(contactListView);
        ContactBaseAdapter adapter = (ContactBaseAdapter) contactListView.getAdapter();
        assertNotNull(adapter);

        // Mock selecting a contact
        generateContacts(adapter, 1);

        // Monitor whether a activity is started.
        ActivityMonitor monitor = null;
        Activity activity = null;

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_SENDTO);
        monitor = new ActivityMonitor(intentFilter, null, false);
        try {
            getInstrumentation().addMonitor(monitor);

            // Invoke the the method startChat in class BaseListFragment.
            mFragment.startSelectMode(null);
            methodStartChat.invoke(mFragment);

            activity = (Activity) monitor.waitForActivityWithTimeout(TIME_OUT);
        } finally {
            getInstrumentation().removeMonitor(monitor);
            if (null != activity) {
                activity.finish();
            }
        }

        Logger.v(TAG, "testCase06_AddAParticipant() exit");
    }

    /**
     * Test scrolling the method onScrollStateChanged in ContactsAdapter
     * 
     * @throws Throwable
     * @throws Exception
     */
    public void testCase07_onScrollStateChanged() throws Throwable, Exception {
        Logger.v(TAG, "testCase07_onScrollStateChanged() entry");
        final ListView contactListView = mFragment.getListView();
        assertNotNull(contactListView);

        // Make the ListView is visible.
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                contactListView.setVisibility(View.VISIBLE);
            }
        });

        // Make the listview has contacts.
        getInstrumentation().waitForIdleSync();
        mFragment.startSelectMode(ChatMainActivity.VALUE_ADD_CONTACTS);
        final ContactsAdapter contactsAdapter = (ContactsAdapter) contactListView.getAdapter();
        assertNotNull(contactsAdapter);
        generateContacts(contactsAdapter, 30);

        // Refresh the ListView.
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                contactsAdapter.notifyDataSetChanged();
            }
        });

        getInstrumentation().waitForIdleSync();
        // Scroll the ListView.
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                contactsAdapter.onScrollStateChanged(contactListView,
                        OnScrollListener.SCROLL_STATE_IDLE);
            }
        });

        getInstrumentation().waitForIdleSync();

        Logger.v(TAG, "testCase07_onScrollStateChanged() exit");
    }

    /**
     * Test other methods in ContactBaseAdapter.
     * 
     * @throws Throwable
     * @throws Exception
     */
    public void testCase08_otherMethodInContactBaseAdapter() throws Throwable, Exception {
        Logger.v(TAG, "testCase08_otherMethodInContactBaseAdapter() entry");
        final ListView contactListView = mFragment.getListView();
        assertNotNull(contactListView);

        // Make the ListView is visible.
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                contactListView.setVisibility(View.VISIBLE);
            }
        });

        // Make the listview has contacts.
        getInstrumentation().waitForIdleSync();
        mFragment.startSelectMode(ChatMainActivity.VALUE_ADD_CONTACTS);
        final ContactsAdapter contactsAdapter = (ContactsAdapter) contactListView.getAdapter();
        assertNotNull(contactsAdapter);

        // Mock a contack and add it into ContactsAdapter.
        ArrayList<RcsContact> contactList = new ArrayList<RcsContact>();
        String numberString = "+34200000246";
        RcsContact contact = new RcsContact(numberString, numberString);
        contact.mSectionIndex = 0;
        contact.mSelected = true;
        contact.mSortKey = numberString;
        contactList.add(contact);
        contactsAdapter.setContactsList(contactList);

        Field fieldSectionToPosition = Utils.getPrivateField(ContactBaseAdapter.class,
                "mSectionToPosition");
        Map<Integer, Integer> sectionToPositionMap = (Map<Integer, Integer>) fieldSectionToPosition
                .get(contactsAdapter);
        assertNotNull(sectionToPositionMap);
        assertEquals(1, sectionToPositionMap.size());

        // Test method getSectionForPosition
        int position = 0;
        int section = contactsAdapter.getSectionForPosition(position);
        assertEquals(0, section);

        // Test method getSections
        String[] sections = contactsAdapter.getSections();
        assertEquals(1, sections.length);

        // Test method setSelectAll and getSelectAll;
        contactsAdapter.setSelectAll(true);
        assertEquals(true, contactsAdapter.getSelectAll());
        contactsAdapter.setSelectAll(false);
        assertEquals(false, contactsAdapter.getSelectAll());

        // Test method removeItems
        contactsAdapter.removeItems(contact);
        assertFalse(contactsAdapter.getContactsList().contains(contact));

        getInstrumentation().waitForIdleSync();

        Logger.v(TAG, "testCase08_otherMethodInContactBaseAdapter() exit");
    }

    /**
     * Check the number of group chat participants .
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InterruptedException
     */
    @SuppressWarnings("unchecked")
    private void checkGroupChatParticipants(int num) throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "getGroupChatParticipants");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        Field fieldChatMap = Utils.getPrivateField(modelImpl.getClass(), "mChatMap");
        Map<Object, IChat> chatMap = (Map<Object, IChat>) fieldChatMap.get(modelImpl);
        Logger.d(TAG, "chatMap.size = " + chatMap.size());
        chatMap.values();
        long beginTime = System.currentTimeMillis();
        while (true) {
            if (chatMap.size() > 0 || (System.currentTimeMillis() - beginTime) > TIME_OUT) {
                Logger.d(TAG, "time out or data is ready");
                break;
            }
            Thread.sleep(200);
        }
        Logger.d(TAG, "chatMap.size = " + chatMap.size());
        GroupChat chat = (GroupChat) chatMap.values().iterator().next();
        int size = chat.getGroupChatParticipants().convertToParticipants().size();
        assertEquals(num, size);
        mActivity.finish();
        Method methodRemoveChat = Utils.getPrivateMethod(modelImpl.getClass(), "removeChat",
                ChatImpl.class);
        methodRemoveChat.invoke(modelImpl, chat);
    }

    /**
     * Mock a contacts selected to start a group chat.
     * 
     * @param adapter A ContactBaseAdapter
     * @param count The number of contacts.
     */
    private void generateContacts(ContactBaseAdapter adapter, int count) {
        Logger.v(TAG, "generateContacts(), count = " + count);
        ArrayList<RcsContact> contactList = new ArrayList<RcsContact>();
        int number = 246;
        for (int i = 0; i < count; ++i) {
            number += i;
            String numberString = "+34200000" + number;
            RcsContact contact = new RcsContact(numberString, numberString);
            contact.mSectionIndex = i;
            contact.mSelected = true;
            contact.mSortKey = numberString;
            contactList.add(contact);
        }

        adapter.setContactsList(contactList);
    }

    private IChat getGroupChat(List<Participant> participants) throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        List<IChat> chats = ModelImpl.getInstance().listAllChat();
        Field field = Utils.getPrivateField(GroupChat.class, "mCurrentParticipants");

        for (IChat iChat : chats) {
            if (null != iChat && iChat instanceof GroupChat) {
                GroupChatParticipants participantsInGroup = (GroupChatParticipants) field
                        .get((GroupChat) iChat);
                List<Participant> parList = participantsInGroup.convertToParticipants();
                if (compareParticipantList(parList, participants)) {
                    return iChat;
                }
            }
        }
        return null;
    }

    private boolean compareParticipantList(List<Participant> source, List<Participant> target) {
        if (source.size() != target.size()) {
            return false;
        }
        int size = target.size();
        for (Participant participantSource : source) {
            int i = 0;
            for (; i < size; i++) {
                if (target.get(i).equals(participantSource)) {
                    break;
                }
            }
            if (i == size) {
                return false;
            }
        }
        return true;
    }

    /**
     * Wait until ContactsListManager
     * 
     * @param contactsListFragment
     * @return The first valid contact number
     * @throws NoSuchFieldException
     * @throws NoSuchMethodException
     * @throws IllegalAccessException
     * @throws InterruptedException
     */
    private String waitForLoadContacts() throws NoSuchFieldException, NoSuchMethodException,
            IllegalAccessException, InterruptedException {
        long startTime = System.currentTimeMillis();
        assertNotNull(mFragment);
        List contactsList = null;
        do {
            // Get a static private object
            Field field = Utils.getPrivateField(ContactsListManager.class, "CONTACTS_LIST");
            assertNotNull(field);
            contactsList = (List) field.get(ContactsListManager.getInstance());
            assertNotNull(contactsList);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (contactsList.size() == 0);
        ListView contactListView = mFragment.getListView();
        List<String> contactNumberList = new ArrayList<String>();
        for (Object rcsContact : contactsList) {
            Field fieldNumber = Utils.getPrivateField(rcsContact.getClass(), "mNumber");
            String contactNumber = (String) fieldNumber.get(rcsContact);
            assertNotNull(contactNumber);
            contactNumberList.add(contactNumber);
        }

        int listViewCount = contactListView.getCount();
        Logger.d(TAG, "listViewCount = " + listViewCount);
        List<String> listViewContacts = new ArrayList<String>();
        for (int i = 0; i < listViewCount; i++) {
            Object rcsContact = contactListView.getItemAtPosition(i);
            Field fieldNumber = Utils.getPrivateField(rcsContact.getClass(), "mNumber");
            String contactNumber = (String) fieldNumber.get(rcsContact);
            if (null != contactNumber) {
                assertTrue(contactNumberList.contains(contactNumber));
                Logger.d(TAG, "contactNumber = " + contactNumber);
                listViewContacts.add(contactNumber);
            } else {
                // This item is an indexer
            }
        }

        List<String> blockContactsList = ContactsManager.getInstance()
                .getImBlockedContactsFromLocal();
        String firstContactNumber = null;
        for (String contactNumber : contactNumberList) {
            boolean isInListView = listViewContacts.contains(contactNumber);
            boolean isInBlockList = blockContactsList.contains(contactNumber);
            if (!isInBlockList && null == firstContactNumber) {
                firstContactNumber = contactNumber;
            }
            Participant participant = new Participant(contactNumber, contactNumber);
            mContactsList.add(participant);
        }
        return firstContactNumber;
    }
    
    public void testCase09_testModeCallback() throws Throwable {
        Logger.d(TAG, "testCase09_testModeCallback entry");
        Field field = Utils.getPrivateField(ContactsListFragment.class, "mModeCallback");
        assertNotNull("field is null", field);
        final ListView.MultiChoiceModeListener mModeCallback = (MultiChoiceModeListener) field.get(mFragment);
        assertNotNull("mModeCallback is null", mModeCallback);
        runTestOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                mModeCallback.onActionItemClicked(new CallbackActionMode(), new CallbackMenuItem(
                        R.id.menu_item_add_all));
            }
        });
        
        field = Utils.getPrivateField(mModeCallback.getClass(), "mIsSelectAll");
        assertNotNull("mIsSelectAll Field is null", field);
        field.set(mModeCallback, true);
        getInstrumentation().waitForIdleSync();
        runTestOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                mModeCallback.onActionItemClicked(new CallbackActionMode(), new CallbackMenuItem(
                        R.id.menu_item_add_all));
            }
        });
        
        getInstrumentation().waitForIdleSync();
        runTestOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                mModeCallback.onActionItemClicked(new CallbackActionMode(), new CallbackMenuItem(
                        R.id.menu_item_to_chat));
            }
        });
        
        getInstrumentation().waitForIdleSync();
        runTestOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                mModeCallback.onActionItemClicked(new CallbackActionMode(), new CallbackMenuItem(
                        R.id.menu_item_to_chat_select));
            }
        });
        
        field = Utils.getPrivateField(mFragment.getClass().getSuperclass(), "mAdapter");
        assertNotNull("mAdapter Field is null", field);
        field.set(mFragment, null);
        getInstrumentation().waitForIdleSync();
        runTestOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                CallbackActionMode mode = new CallbackActionMode();
                mModeCallback.onActionItemClicked(mode, new CallbackMenuItem(
                        R.id.menu_item_add_all));
                mModeCallback.onDestroyActionMode(mode);
            }
        });
    }
    
    public void testCase10_testOtherItems() throws Throwable {
        Logger.d(TAG, "testCase10_testOtherItems entry");
        assertNotNull(mFragment);
        final ListView listView = mFragment.getListView();
        assertNotNull(listView);
//        mFragment.mIsFromSelectContactsActivity = false;
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                listView.setFocusable(true);
                listView.setFocusableInTouchMode(true);
                boolean focus = listView.requestFocus();
                listView.setSelection(0);
                Logger.v(TAG, "focurs = " + focus);
            }
        });
        getInstrumentation().waitForIdleSync();
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        getInstrumentation().waitForIdleSync();
        assertEquals(false, mFragment.onItemLongClick(null, null, 0, 0));
        
        mFragment.setExistingContacts(null);
        assertEquals(false, mFragment.getLoadFinished());
    }
    
    private class CallbackActionMode extends ActionMode {
        
        public CallbackActionMode() {
        }

        @Override
        public void finish() {
            
        }

        @Override
        public View getCustomView() {
            return null;
        }

        @Override
        public Menu getMenu() {
            return null;
        }

        @Override
        public MenuInflater getMenuInflater() {
            return null;
        }

        @Override
        public CharSequence getSubtitle() {
            return null;
        }

        @Override
        public CharSequence getTitle() {
            return null;
        }

        @Override
        public void invalidate() {
        }

        @Override
        public void setCustomView(View arg0) {
        }

        @Override
        public void setSubtitle(CharSequence arg0) {
        }

        @Override
        public void setSubtitle(int arg0) {
        }

        @Override
        public void setTitle(CharSequence arg0) {
        }

        @Override
        public void setTitle(int arg0) {
        }
        
    }
    
    private class CallbackMenuItem implements MenuItem {
        
        int mMenuItemId;
        
        public CallbackMenuItem(int itemId) {
            mMenuItemId = itemId;
        }

        @Override
        public boolean collapseActionView() {
            return false;
        }

        @Override
        public boolean expandActionView() {
            return false;
        }

        @Override
        public ActionProvider getActionProvider() {
            return null;
        }

        @Override
        public View getActionView() {
            return null;
        }

        @Override
        public char getAlphabeticShortcut() {
            return 0;
        }

        @Override
        public int getGroupId() {
            return 0;
        }

        @Override
        public Drawable getIcon() {
            return null;
        }

        @Override
        public Intent getIntent() {
            return null;
        }

        @Override
        public int getItemId() {
            return mMenuItemId;
        }

        @Override
        public ContextMenuInfo getMenuInfo() {
            return null;
        }

        @Override
        public char getNumericShortcut() {
            return 0;
        }

        @Override
        public int getOrder() {
            return 0;
        }

        @Override
        public SubMenu getSubMenu() {
            return null;
        }

        @Override
        public CharSequence getTitle() {
            return null;
        }

        @Override
        public CharSequence getTitleCondensed() {
            return null;
        }

        @Override
        public boolean hasSubMenu() {
            return false;
        }

        @Override
        public boolean isActionViewExpanded() {
            return false;
        }

        @Override
        public boolean isCheckable() {
            return false;
        }

        @Override
        public boolean isChecked() {
            return false;
        }

        @Override
        public boolean isEnabled() {
            return false;
        }

        @Override
        public boolean isVisible() {
            return false;
        }

        @Override
        public MenuItem setActionProvider(ActionProvider arg0) {
            return null;
        }

        @Override
        public MenuItem setActionView(View arg0) {
            return null;
        }

        @Override
        public MenuItem setActionView(int arg0) {
            return null;
        }

        @Override
        public MenuItem setAlphabeticShortcut(char alphaChar) {
            return null;
        }

        @Override
        public MenuItem setCheckable(boolean checkable) {
            return null;
        }

        @Override
        public MenuItem setChecked(boolean checked) {
            return null;
        }

        @Override
        public MenuItem setEnabled(boolean enabled) {
            return null;
        }

        @Override
        public MenuItem setIcon(Drawable icon) {
            return null;
        }

        @Override
        public MenuItem setIcon(int iconRes) {
            return null;
        }

        @Override
        public MenuItem setIntent(Intent intent) {
            return null;
        }

        @Override
        public MenuItem setNumericShortcut(char numericChar) {
            return null;
        }

        @Override
        public MenuItem setOnActionExpandListener(OnActionExpandListener arg0) {
            return null;
        }

        @Override
        public MenuItem setOnMenuItemClickListener(
                OnMenuItemClickListener menuItemClickListener) {
            return null;
        }

        @Override
        public MenuItem setShortcut(char numericChar, char alphaChar) {
            return null;
        }

        @Override
        public void setShowAsAction(int arg0) {
            
        }

        @Override
        public MenuItem setShowAsActionFlags(int arg0) {
            return null;
        }

        @Override
        public MenuItem setTitle(CharSequence title) {
            return null;
        }

        @Override
        public MenuItem setTitle(int title) {
            return null;
        }

        @Override
        public MenuItem setTitleCondensed(CharSequence title) {
            return null;
        }

        @Override
        public MenuItem setVisible(boolean visible) {
            return null;
        }
        
    }
    
}
