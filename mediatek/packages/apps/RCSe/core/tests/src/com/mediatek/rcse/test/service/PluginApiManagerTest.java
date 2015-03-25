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

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts.Data;
import android.test.InstrumentationTestCase;
import android.util.LruCache;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.CapabilitiesChangeListener;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.service.PluginApiManager.RegistrationListener;
import com.mediatek.rcse.service.PluginApiManager.RichCallApiListener;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test PluginApiManager.java
 */
public class PluginApiManagerTest extends InstrumentationTestCase {
    private final static String TAG = "PluginApiManagerTest";
    private PluginApiManager mPluginApiManager = null;
    private Context mContext = null;
    private ContentResolver mContentResolver = null;

    private final static int NON_RCS_CONTACT = 0;
    private final static int RCS_CONTACT = 1;
    private final static String TEST_CONTACT = "+34022222254";
    private final static String NUMBER = "10086";
    private static final int TIME_OUT = 3000;
    private static final int SLEEP_TIME = 200;
    private final static String TEST_CONTACT2 = "+34022222246";
    private final static String TEST_CONTACT3 = "+34022222247";
    private final static String TEST_CONTACT4 = "+34022222248";
    private final static String TEST_CONTACT5 = "+34022222249";

    private final Capabilities TEST_ACTIVE_CAPABILITIES = new Capabilities();
    private final Capabilities TEST_DEACTIVE_CAPABILITIES = new Capabilities();
    {
        TEST_ACTIVE_CAPABILITIES.setImSessionSupport(true);
        TEST_ACTIVE_CAPABILITIES.setRcseContact(true);
        TEST_ACTIVE_CAPABILITIES.setFileTransferSupport(true);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        Field fieldInstance = Utils.getPrivateField(PluginApiManager.class, "sInstance");
        fieldInstance.set(null, null);
        boolean ret = PluginApiManager.initialize(mContext);
        assertTrue(ret);
        mPluginApiManager = PluginApiManager.getInstance();
        mContentResolver = mContext.getContentResolver();
    }

    /**
     * Test the getPresence method in PluginApiManager
     */
    public void testCase1_PresenceByNumber() throws InterruptedException {

        int presence = mPluginApiManager.getContactPresence(TEST_CONTACT);
        assertEquals(NON_RCS_CONTACT, presence);
        addRcsContact(TEST_CONTACT);
        presence = mPluginApiManager.getContactPresence(TEST_CONTACT);
        assertEquals(RCS_CONTACT, presence);
        removeRcsContact();
        presence = mPluginApiManager.getContactPresence(TEST_CONTACT);
        assertEquals(NON_RCS_CONTACT, presence);
    }

    /**
     * Test the getPresence method of a specific contact id
     * 
     * @throws InterruptedException
     */
    public void testCase2_PresenceByContactId() throws InterruptedException {
        Logger.d(TAG, "testCase2_PresenceByContactId() entry");
        ContentValues values = new ContentValues();
        Uri rawContactUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContactUri);
        addContact(values, rawContactId, NUMBER);
        addRcsContact(NUMBER);
        int contactId = getContactIdByRawContactId(rawContactId);
        waitForPresence(contactId);
        mContentResolver.delete(ContactsContract.RawContacts.CONTENT_URI,
                ContactsContract.Contacts._ID + "=" + contactId, null);
    }

    private void addContact(ContentValues values, long rawContactId, String number) {
        Logger.d(TAG, "addContact() entry, rawContactId is " + rawContactId + " and number is "
                + number);
        values.clear();
        values.put(ContactsContract.Contacts.Data.RAW_CONTACT_ID, rawContactId);
        values.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
        values.put(Phone.TYPE, Phone.TYPE_MOBILE);
        values.put(Phone.NUMBER, number);
        mContentResolver.insert(ContactsContract.Data.CONTENT_URI, values);
    }

    private int getContactIdByRawContactId(long rawContactId) {
        Logger.d(TAG, "getContactIdByRawContactId() entry, rawContactId is " + rawContactId);
        int contactId = -1;
        String[] projection = {
            Phone.CONTACT_ID
        };
        String selection = ContactsContract.Contacts.Data.RAW_CONTACT_ID + "=" + rawContactId;
        Cursor cur = mContentResolver.query(Phone.CONTENT_URI, projection, selection, null, null);
        try {
            if (cur != null) {
                while (cur.moveToNext()) {
                    contactId = cur.getInt(0);
                }
            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
        Logger.d(TAG, "getContactIdByRawContactId() contactId is " + contactId);
        return contactId;
    }
    
    private void waitForPresence(int contactId) throws InterruptedException {
        long startTime = System.currentTimeMillis();
        do {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (mPluginApiManager.getContactPresence(contactId) == 0);
    }

    private void addRcsContact(String contact) {
        mPluginApiManager.onCapabilityChanged(contact, TEST_ACTIVE_CAPABILITIES);
    }

    private void removeRcsContact() {
        PluginApiManager.getInstance()
                .onCapabilityChanged(TEST_CONTACT, TEST_DEACTIVE_CAPABILITIES);
    }

    /**
     * Test getContactPresence
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    @SuppressWarnings("unchecked")
    public void testCase3_getContactPresence() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase3_getContactPresence");
        // Add capability to cache
        Field fieldContactsCache = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mContactsCache");
        LruCache<String, ContactInformation> capabilityCache = (LruCache<String, ContactInformation>) fieldContactsCache
                .get(mPluginApiManager);
        ContactInformation cInformation = new ContactInformation();
        capabilityCache.put(TEST_CONTACT, cInformation);
        assertEquals(0, mPluginApiManager.getContactPresence(TEST_CONTACT));
        // Remove capability from cache
        capabilityCache.remove(TEST_CONTACT);
        assertEquals(0, mPluginApiManager.getContactPresence(TEST_CONTACT));
        // Give a chance to the AsyncTask
        Thread.sleep(SLEEP_TIME);

        Field fieldCache = Utils.getPrivateField(mPluginApiManager.getClass(), "mCache");
        LruCache<Long, List<String>> cache = (LruCache<Long, List<String>>) fieldCache
                .get(mPluginApiManager);
        Field fieldQueryOngoingList = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mQueryOngoingList");
        List<Long> pendingQueryList = (List<Long>) fieldQueryOngoingList.get(mPluginApiManager);

        long conatctId = 123456;
        List<String> numbers = new ArrayList<String>();
        numbers.add(TEST_CONTACT);
        cache.put(conatctId, numbers);

        cInformation.isRcsContact = 1;
        capabilityCache.put(TEST_CONTACT, cInformation);
        assertEquals(1, mPluginApiManager.getContactPresence(conatctId));
        // Remove capability from cache
        capabilityCache.remove(TEST_CONTACT);
        assertEquals(0, mPluginApiManager.getContactPresence(conatctId));
        // Give a chance to the AsyncTask
        waitTheAsyncTaskExectued(pendingQueryList, conatctId);

        pendingQueryList.add(conatctId);
        assertEquals(0, mPluginApiManager.getContactPresence(conatctId));

        // Make sure the test go to getContactPresence(final long contactId)
        // else
        cache.remove(conatctId);
        long conatctId2 = 123457;
        pendingQueryList.add(conatctId2);
        assertEquals(0, mPluginApiManager.getContactPresence(conatctId2));
        pendingQueryList.remove(conatctId2);
        assertEquals(0, mPluginApiManager.getContactPresence(conatctId2));
        // Give a chance to the AsyncTask
        waitTheAsyncTaskExectued(pendingQueryList, conatctId2);
    }

    private void waitTheAsyncTaskExectued(List<Long> pendingQueryList, long contactId)
            throws InterruptedException {
        Logger.d(TAG, "waitTheAsyncTaskExectued() contactId = " + contactId);
        long beginTime = System.currentTimeMillis();
        while (pendingQueryList.contains(contactId)) {
            if ((System.currentTimeMillis() - beginTime) > TIME_OUT) {
                Logger.d(TAG, "waitTheAsyncTaskExectued time out");
                break;
            }
            Thread.sleep(SLEEP_TIME);
        }
    }

    /**
     * Test queryPresence
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     */
    @SuppressWarnings("unchecked")
    public void testCase4_queryPresence() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase4_queryPresence");
        // Add capability to cache
        Field fieldContactsCache = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mContactsCache");
        LruCache<String, ContactInformation> capabilityCache = (LruCache<String, ContactInformation>) fieldContactsCache
                .get(mPluginApiManager);
        ContactInformation cInformation = new ContactInformation();
        capabilityCache.put(TEST_CONTACT2, cInformation);

        List<String> numberList = new ArrayList<String>();
        numberList.add(TEST_CONTACT2);
        numberList.add(CapabilityApi.ME);
        Method methodQueryPresence = Utils.getPrivateMethod(mPluginApiManager.getClass(),
                "queryPresence", long.class, List.class);
        methodQueryPresence.invoke(mPluginApiManager, 1l, numberList);
        if(mPluginApiManager.getCapabilityApi() == null){
            Logger.d(TAG, "capabilityApi is null");
            assertNull(capabilityCache.get(CapabilityApi.ME));
        }else{
            Logger.d(TAG, "capabilityApi isnot null");
            assertNotNull(capabilityCache.get(CapabilityApi.ME));
        }

    }

    /**
     * Test queryNumbersPresence
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    @SuppressWarnings("unchecked")
    public void testCase5_queryNumbersPresence() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase5_queryNumbersPresence");
        Field fieldContactsCache = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mContactsCache");
        LruCache<String, ContactInformation> capabilityCache = (LruCache<String, ContactInformation>) fieldContactsCache
                .get(mPluginApiManager);
        List<String> numberList = new ArrayList<String>();
        numberList.add(TEST_CONTACT3);
        numberList.add(CapabilityApi.ME);
        mPluginApiManager.queryNumbersPresence(numberList);
        assertNotNull(capabilityCache.get(CapabilityApi.ME));
    }

    /**
     * Test handleDisconnected
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    public void testCase6_handleDisconnected() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InstantiationException {
        Logger.d(TAG, "testCase6_handleDisconnected");
        Class<?>[] clazzes = mPluginApiManager.getClass().getDeclaredClasses();
        for (Class<?> clazz : clazzes) {
            if ("ManagedRegistrationApi".equals(clazz.getSimpleName())) {
                Logger.d(TAG, "ManagedRegistrationApi");
                MockRegistrationListener registrationListener = new MockRegistrationListener();
                mPluginApiManager.addRegistrationListener(registrationListener);
                Constructor<?> cotr = clazz.getConstructor(PluginApiManager.class, Context.class);
                Object object = cotr.newInstance(mPluginApiManager, mContext);
                // first save mRegistrationApi
                Field fieldRegistrationApi = Utils.getPrivateField(mPluginApiManager.getClass(),
                        "mRegistrationApi");
                Object registrationApi = fieldRegistrationApi.get(mPluginApiManager);

                fieldRegistrationApi.set(mPluginApiManager, object);
                Method methodHandleDisconnected = Utils.getPrivateMethod(object.getClass(),
                        "handleDisconnected");
                methodHandleDisconnected.invoke(object);
                methodHandleDisconnected.invoke(object);
                fieldRegistrationApi.set(mPluginApiManager, registrationApi);
                assertEquals(false, registrationListener.mIsConnected);
                mPluginApiManager.removeRegistrationListener(registrationListener);

            } else if ("ManagedCapabilityApi".equals(clazz.getSimpleName())) {
                Logger.d(TAG, "ManagedCapabilityApi");
                MockCapabilitiesChangeListener capabilitiesChangeListener = new MockCapabilitiesChangeListener();
                mPluginApiManager.addCapabilitiesChangeListener(capabilitiesChangeListener);
                Constructor<?> cotr = clazz.getConstructor(PluginApiManager.class, Context.class);
                Object object = cotr.newInstance(mPluginApiManager, mContext);
                // first save mRegistrationApi
                Field fieldCapabilitiesApi = Utils.getPrivateField(mPluginApiManager.getClass(),
                        "mCapabilitiesApi");
                Object capabilitiesApi = fieldCapabilitiesApi.get(mPluginApiManager);

                fieldCapabilitiesApi.set(mPluginApiManager, object);
                Method methodHandleDisconnected = Utils.getPrivateMethod(object.getClass(),
                        "handleDisconnected");
                methodHandleDisconnected.invoke(object);
                methodHandleDisconnected.invoke(object);
                fieldCapabilitiesApi.set(mPluginApiManager, capabilitiesApi);
                assertEquals(false, capabilitiesChangeListener.mIsConnected);
                mPluginApiManager.removeCapabilitiesChangeListener(capabilitiesChangeListener);
            }
        }
    }

    /**
     * Test add listener, remove listener and setRegistrationStatus
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    @SuppressWarnings({
        "unchecked"
    })
    public void testCase7_addRemoveListener() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase7_addRemoveListener");
        mPluginApiManager.setRegistrationStatus(true);
        Field fieldIsRegistered = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mIsRegistered");
        assertEquals(true, fieldIsRegistered.getBoolean(mPluginApiManager));
        mPluginApiManager.getRegistrationApi();
        assertEquals(true && (mPluginApiManager.getRegistrationApi() != null),
                mPluginApiManager.getRegistrationStatus());

        // Test add listener and remove listener
        MockRichCallApiListener richCallApiListener = new MockRichCallApiListener();
        Field fieldRichCallApiListeners = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mRichCallApiListeners");
        List<RichCallApiListener> richCallApiListeners = (CopyOnWriteArrayList<RichCallApiListener>) fieldRichCallApiListeners
                .get(mPluginApiManager);
        Logger.d(TAG, "richCallApiListeners.size() = " + richCallApiListeners.size());
        mPluginApiManager.addRichCallApiListener(richCallApiListener);
        assertTrue(richCallApiListeners.contains(richCallApiListener));
        mPluginApiManager.removeRichCallApiListener(richCallApiListener);
        assertFalse(richCallApiListeners.contains(richCallApiListeners));

        RegistrationListener registrationListener = new MockRegistrationListener();
        Field fieldRegistrationListeners = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mRegistrationListeners");
        List<RegistrationListener> registrationListeners = (List<RegistrationListener>) fieldRegistrationListeners
                .get(mPluginApiManager);
        mPluginApiManager.addRegistrationListener(registrationListener);
        assertTrue(registrationListeners.contains(registrationListener));
        mPluginApiManager.removeRegistrationListener(registrationListener);
        assertFalse(registrationListeners.contains(registrationListener));

        CapabilitiesChangeListener capabilitiesChangeListener = new MockCapabilitiesChangeListener();
        Field fieldCapabilitiesChangeListeners = Utils.getPrivateField(
                mPluginApiManager.getClass(), "mCapabilitiesChangeListenerList");
        List<RegistrationListener> capabilitiesChangeListeners = (List<RegistrationListener>) fieldCapabilitiesChangeListeners
                .get(mPluginApiManager);
        mPluginApiManager.addCapabilitiesChangeListener(capabilitiesChangeListener);
        assertTrue(capabilitiesChangeListeners.contains(capabilitiesChangeListener));
        mPluginApiManager.removeCapabilitiesChangeListener(capabilitiesChangeListener);
        assertFalse(capabilitiesChangeListeners.contains(capabilitiesChangeListener));

    }

    /**
     * Test isRcseContact(), isImSupported(),
     * isFtSupported(),isImageShareSupported(),isVideoShareSupported() and so on
     * @throws NoSuchFieldException 
     * @throws IllegalAccessException 
     * @throws IllegalArgumentException 
     */
    @SuppressWarnings("unchecked")
    public void testCase8_isXXSupport() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Logger.d(TAG, "testCase8_isXXSupport");
        PluginApiManager.RcseAction action = new PluginApiManager.RcseAction();
        mPluginApiManager.getContext();
        mPluginApiManager.getCapabilityApi();

        // test the number is null
        assertEquals(false, mPluginApiManager.isCsCallShareSupported(null));
        assertEquals(false, mPluginApiManager.isFtSupported(null));
        assertEquals(false, mPluginApiManager.isImageShareSupported(null));
        assertEquals(false, mPluginApiManager.isImSupported(null));
        assertEquals(false, mPluginApiManager.isRcseContact(null));
        assertEquals(false, mPluginApiManager.isVideoShareSupported(null));

        // Clear cache
        mPluginApiManager.cleanContactCache();
        assertEquals(false, mPluginApiManager.isCsCallShareSupported(TEST_CONTACT4));
        assertEquals(false, mPluginApiManager.isFtSupported(TEST_CONTACT4));
        assertEquals(false, mPluginApiManager.isImageShareSupported(TEST_CONTACT4));
        assertEquals(false, mPluginApiManager.isImSupported(TEST_CONTACT4));
        assertEquals(false, mPluginApiManager.isRcseContact(TEST_CONTACT4));
        assertEquals(false, mPluginApiManager.isVideoShareSupported(TEST_CONTACT4));

        // Add TEST_CONTACT to cache
        Field fieldContactsCache = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mContactsCache");
        LruCache<String, ContactInformation> capabilityCache = (LruCache<String, ContactInformation>) fieldContactsCache
                .get(mPluginApiManager);
        ContactInformation cInformation = new ContactInformation();
        cInformation.isCsCallSupported = true;
        cInformation.isFtSupported = true;
        cInformation.isImageShareSupported = true;
        cInformation.isImSupported = true;
        cInformation.isRcsContact = 1;
        cInformation.isVideoShareSupported = true;
        capabilityCache.put(TEST_CONTACT5, cInformation);
        assertEquals(true, mPluginApiManager.isCsCallShareSupported(TEST_CONTACT5));
        assertEquals(true, mPluginApiManager.isFtSupported(TEST_CONTACT5));
        assertEquals(true, mPluginApiManager.isImageShareSupported(TEST_CONTACT5));
        assertEquals(true, mPluginApiManager.isImSupported(TEST_CONTACT5));
        assertEquals(true, mPluginApiManager.isRcseContact(TEST_CONTACT5));
        assertEquals(true, mPluginApiManager.isVideoShareSupported(TEST_CONTACT5));

        //
        Field fieldCache = Utils.getPrivateField(mPluginApiManager.getClass(), "mCache");
        LruCache<Long, List<String>> cache = (LruCache<Long, List<String>>) fieldCache
                .get(mPluginApiManager);
        cache.evictAll();
        long conatctId = 123456;
        assertEquals(false, mPluginApiManager.isImSupported(conatctId));
        assertEquals(false, mPluginApiManager.isFtSupported(conatctId));
        List<String> numbers = new ArrayList<String>();
        numbers.add(NUMBER);
        cache.put(conatctId, numbers);
        capabilityCache.evictAll();
        cInformation = new ContactInformation();
        cInformation.isCsCallSupported = true;
        cInformation.isFtSupported = true;
        cInformation.isImageShareSupported = true;
        cInformation.isImSupported = true;
        cInformation.isRcsContact = 1;
        cInformation.isVideoShareSupported = true;
        capabilityCache.put(NUMBER, cInformation);
        assertEquals(true, mPluginApiManager.isImSupported(conatctId));
        assertEquals(true, mPluginApiManager.isFtSupported(conatctId));
        cache.evictAll();
        capabilityCache.evictAll();
    }

    public void testCase09_otherMethods() throws Exception {
        mPluginApiManager.setRegistrationStatus(true);
        Field fieldIsRegistered = Utils.getPrivateField(mPluginApiManager.getClass(),
                "mIsRegistered");
        assertEquals(true, fieldIsRegistered.getBoolean(mPluginApiManager));
        mPluginApiManager.getRegistrationApi();
        assertEquals(true && (mPluginApiManager.getRegistrationApi() != null),
                mPluginApiManager.getRegistrationStatus());
        Capabilities capabilities = new Capabilities();
        capabilities.setRcseContact(true);
        capabilities.setImSessionSupport(true);
        capabilities.setFileTransferSupport(true);
        capabilities.setImageSharingSupport(true);
        capabilities.setCsVideoSupport(true);
        Field field = Utils.getPrivateField(mPluginApiManager.getClass(), "mCache");
        LruCache<Long, List<String>> cache = (LruCache<Long, List<String>>) field
                .get(mPluginApiManager);
        ArrayList<String> list = new ArrayList<String>();
        list.add("123");
        cache.put(1l, list);
        mPluginApiManager.onCapabilityChanged("123", capabilities);
        assertTrue(mPluginApiManager.isRcseContact("123"));
        assertTrue(mPluginApiManager.isRcseContact("123"));
        assertTrue(mPluginApiManager.isImSupported(1l));
        assertTrue(mPluginApiManager.isFtSupported("123"));
        assertTrue(mPluginApiManager.isFtSupported(1l));
        assertTrue(mPluginApiManager.isImSupported("123"));
        assertTrue(mPluginApiManager.isCsCallShareSupported("123"));
        assertTrue(mPluginApiManager.isImageShareSupported("123"));

        assertFalse(mPluginApiManager.isRcseContact(null));
        assertFalse(mPluginApiManager.isRcseContact("321"));
        assertFalse(mPluginApiManager.isImSupported(null));
        assertFalse(mPluginApiManager.isImSupported("321"));
        assertFalse(mPluginApiManager.isFtSupported(null));
        assertFalse(mPluginApiManager.isFtSupported("321"));
        assertFalse(mPluginApiManager.isCsCallShareSupported(null));
        assertFalse(mPluginApiManager.isCsCallShareSupported("321"));
        assertFalse(mPluginApiManager.isImageShareSupported(null));
        assertFalse(mPluginApiManager.isImageShareSupported("321"));

        mPluginApiManager.removeCapabilitiesChangeListener(null);
        mPluginApiManager.removeRegistrationListener(null);
        mPluginApiManager.addRichCallApiListener(null);
        mPluginApiManager.removeRichCallApiListener(null);
        mPluginApiManager.getContactPresence(2l);
        getInstrumentation().waitForIdleSync();
        assertNotNull(mPluginApiManager.getCapabilityApi());
        assertNotNull(mPluginApiManager.getContext());
        mPluginApiManager.queryNumbersPresence(list);
        mPluginApiManager.getRegistrationApi().onStatusChanged(true);
        assertTrue(mPluginApiManager.getRegistrationStatus());

        mPluginApiManager.cleanContactCache();
    }

    private class MockRegistrationListener implements RegistrationListener {
        private static final String TAG = "MockRegistrationListener";
        private boolean mIsConnected = true;

        @Override
        public void onApiConnectedStatusChanged(boolean isConnected) {
            Logger.d(TAG, "onApiConnectedStatusChanged");
            mIsConnected = isConnected;
        }

        @Override
        public void onStatusChanged(boolean status) {
        }

        @Override
        public void onRcsCoreServiceStatusChanged(int status) {
        }
    }

    private class MockCapabilitiesChangeListener implements CapabilitiesChangeListener {
        private static final String TAG = "MockCapabilitiesChangeListener";
        private boolean mIsConnected = false;

        @Override
        public void onCapabilitiesChanged(String contact, ContactInformation contactInformation) {

        }

        @Override
        public void onApiConnectedStatusChanged(boolean isConnected) {
            Logger.d(TAG, "onApiConnectedStatusChanged");
            mIsConnected = isConnected;
        }
    }
    
    private class MockRichCallApiListener implements RichCallApiListener{
        private static final String TAG = "MockCapabilitiesChangeListener";
        private boolean mIsConnected = false;
        
        @Override
        public void onApiConnectedStatusChanged(boolean isConnected) {
            Logger.d(TAG, "onApiConnectedStatusChanged");
            mIsConnected = isConnected;
        }
        
    }
}
