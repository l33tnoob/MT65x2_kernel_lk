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

package com.mediatek.rcse.test.activity;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.Instrumentation.ActivityMonitor;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.CalendarContract.Events;
import android.provider.ContactsContract;
import android.provider.MediaStore;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts.Data;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.PluginProxyActivity;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 * Tests for aquiring contacts or file content via PluginProxyActivity.
 */
public class PluginProxyActivityTest extends
        ActivityInstrumentationTestCase2<PluginProxyActivity> {

    private static final String TAG = "PluginProxyActivityTest";
    private static final String TEST_NUMBER = "+34245645623";
    private static final String TEST_NAME = "test name";
    private static final String TEST_NUMBER_MORE_CAPABILITY = "+34245645625";
    private static final String TEST_NUMBER_NO_CAPABILITY = "+34245645624";
    private static final String TEST_NUMBER_NO_CAPABILITY_NAME = "test no capability";
    private static final String FRAGMENT_TAG = "FtCapabilityAsyncTask";
    private static final String WARING_FRAGMENT_TAG = "PluginProxyActivity";
    private static final String MOCK_CONTACT_ONE = "+34200000111";
    private static final String MOCK_CONTACT_TWO = "+34200000222";
    private static final Participant MOCK_GROUP_PARTICIPANT_ONE =
            new Participant(MOCK_CONTACT_ONE, MOCK_CONTACT_ONE);
    private static final Participant MOCK_GROUP_PARTICIPANT_TWO =
            new Participant(MOCK_CONTACT_ONE, MOCK_CONTACT_TWO);
    public static final int LAUNCH_TIME_OUT = 1000;
    static final double SELECT_CONTACTS_PATIAL = 0.5;
    private static final long TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;
    private static final int SLEEP_TIME_FOR_MULIMEDIA = 3000;
    static final int MIN_NUMBER = 1;
    private PluginProxyActivity mActivity = null;
    private static final String MOCK_FILE_PATH = "/this/is/a/file/path";
    private static final String VCARD_DATA_TYPE = "text/x-vcard";
    private static final String VCALENDAR_DATA_TYPE = "text/x-vcalendar";
    private static final String MIMETYPE_RCSE_CAPABILITIES = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    private static final String VCALENDAR_SCHEMA = "content://com.mediatek.calendarimporter/";
    private static final String GALLERY_TYPE = "image/*";
    private ContentResolver mContentResolver = null;
    private long mRawContactId = -1;
    private MockRegistrationApi mockRegistrationApi = null;
    private MockCapabilityApi mockCapabilityApi = null;
     private static boolean ALLOW_COMPILE = true;
     
    public PluginProxyActivityTest() {
        super(PluginProxyActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() entry");
        super.setUp();
        Context targetContext = getInstrumentation().getTargetContext();
        Field fieldRegistrationApi = Utils.getPrivateField(ApiManager.class, "mRegistrationApi");
        Field fieldCapabilitiesApi = Utils.getPrivateField(ApiManager.class, "mCapabilitiesApi");
        ApiManager.initialize(targetContext);
        waitForRegistrationApiIsNotNull(fieldRegistrationApi);
        waitForCapabilitiesApiIsNotNull(fieldCapabilitiesApi);
        ApiManager apiManager = ApiManager.getInstance();
        mockRegistrationApi = new MockRegistrationApi(targetContext);
        mockCapabilityApi = new MockCapabilityApi(targetContext);
        fieldRegistrationApi.set(apiManager, mockRegistrationApi);
        fieldCapabilitiesApi.set(apiManager, mockCapabilityApi);
        ContactsListManager.initialize(targetContext);
        ContactsManager.createInstance(targetContext);
        mContentResolver = targetContext.getContentResolver();
        Logger.v(TAG, "setUp() exit");
    }
    
    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    private PluginProxyActivity getFtActionActivity(boolean ft_action_flag, String mockFilePath,
            String contactNumber) {
        Intent intent = new Intent();
        intent.setAction(PluginApiManager.RcseAction.PROXY_ACTION);
        intent.putExtra(PluginApiManager.RcseAction.FT_ACTION, ft_action_flag);
        intent.putExtra(PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION, mockFilePath);
        if (!contactNumber.equals("")) {
            intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, contactNumber);
        }
        setActivityIntent(intent);
        return getActivity();
    }

    /**
     * Test to send a file to a specific contact, but cancel when selecting contact.
     */
    public void testCase1_ResultCancel() throws Throwable {
        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase1_ResultCancel() entry");
        ActivityMonitor contactActivityMonitor = getInstrumentation().addMonitor(
                SelectContactsActivity.class.getName(), null, false);
        mActivity = getFtActionActivity(true, MOCK_FILE_PATH, "");
        Logger.d(TAG, "testCase1_ResultCancel() mActivity is " + mActivity);
        Activity contactActivity = getInstrumentation().waitForMonitorWithTimeout(
                contactActivityMonitor, TIME_OUT);
        Logger.d(TAG, "testCase1_ResultCancel() contactActivity is " + contactActivity);
        try {
            assertNotNull(mActivity);
            assertNotNull(contactActivity);
        } finally {
            getInstrumentation().removeMonitor(contactActivityMonitor);
            if (null != contactActivity) {
                contactActivity.finish();
            }
        }
        Method methodOnActivityResult = Utils.getPrivateMethod(PluginProxyActivity.class,
                "onActivityResult", int.class, int.class, Intent.class);
        methodOnActivityResult.invoke(mActivity, PluginProxyActivity.REQUEST_CODE_RCSE_CONTACT,
                Activity.RESULT_CANCELED, null);
        assertTrue(mActivity.isFinishing());
        Logger.d(TAG, "testCase1_ResultCancel() exit");
    }

    /**
     * Test to send a file to a specific contact after select a contact
     */
    public void testCase2_RequestCodeRcseContact() throws Throwable {
        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase2_RequestCodeRcseContact() entry");
        mActivity = getNullActionActivity();
        mActivity.getIntent().putExtra(PluginApiManager.RcseAction.SINGLE_FILE_URI, MOCK_FILE_PATH);
        assertNotNull(mActivity);
        Intent intent = new Intent();
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(new Participant(TEST_NUMBER, TEST_NAME));
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        Method methodOnActivityResult =
                Utils.getPrivateMethod(PluginProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        methodOnActivityResult.invoke(mActivity, PluginProxyActivity.REQUEST_CODE_RCSE_CONTACT,
                Activity.RESULT_OK, intent);
        waitForFragment(FRAGMENT_TAG);
        Logger.d(TAG, "testCase2_RequestCodeRcseContact() exit");
    }
    
    /**
     * Test to start a group chat.
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     * @throws InvocationTargetException
     */
    public void testCase3_ResultCodeStartGroup() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, NoSuchFieldException,
            InterruptedException, InvocationTargetException {
        if(ALLOW_COMPILE)return ;  mActivity = getActivity();
        Logger.d(TAG, "testCase3_ResultCodeStartGroup() entry");
        Field filedStartGroup =
                Utils.getPrivateField(PluginProxyActivity.class, "REQUEST_CODE_START_GROUP");
        waitForLoadContact();
        Intent intent = new Intent();
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(MOCK_GROUP_PARTICIPANT_ONE);
        participants.add(MOCK_GROUP_PARTICIPANT_TWO);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        boolean isIntegrationMode = Logger.getIsIntegrationMode();
        Method methodOnActivityResult =
                Utils.getPrivateMethod(PluginProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        // Test start PluginGroupChatActivity
        Logger.setIsIntegrationMode(true);
        methodOnActivityResult.invoke(mActivity, filedStartGroup.getInt(mActivity), -1, intent);
        ActivityMonitor pluginGroupChatActivityMonitor =
                getInstrumentation().addMonitor(PluginGroupChatActivity.class.getName(), null,
                        false);
        Activity pluginGroupChatActivity =
                getInstrumentation().waitForMonitorWithTimeout(pluginGroupChatActivityMonitor,
                        TIME_OUT);
        try {
            assertTrue(mActivity.isFinishing());
            assertNotNull(pluginGroupChatActivity);
        } finally {
            if (pluginGroupChatActivity != null) {
                pluginGroupChatActivity.finish();
            }
        }
        // Test start ChatScreenActivity
        Logger.setIsIntegrationMode(false);
        methodOnActivityResult.invoke(mActivity, filedStartGroup.getInt(mActivity), -1, intent);
        ActivityMonitor chatScreenActivityMonitor =
                getInstrumentation().addMonitor(ChatScreenActivity.class.getName(), null, false);
        Activity chatScreenActivity =
                getInstrumentation().waitForMonitorWithTimeout(chatScreenActivityMonitor, TIME_OUT);
        try {
            assertTrue(mActivity.isFinishing());
            assertNotNull(chatScreenActivity);
        } finally {
            if (chatScreenActivity != null) {
                chatScreenActivity.finish();
            }
        }
        Logger.setIsIntegrationMode(isIntegrationMode);
        Logger.d(TAG, "testCase3_ResultCodeStartGroup() exit");
    }

    /**
     * Test to share a file from Camera when the file name is valid
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase4_InvalidFileName() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException {
       if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase4_InvalidFileName() entry");
        mActivity = getNullActionActivity();
        assertNotNull(mActivity);
        Method methodOnActivityResult =
                Utils.getPrivateMethod(PluginProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        Field filedStartCamera =
                Utils.getPrivateField(PluginProxyActivity.class, "REQUEST_CODE_CAMERA");
        methodOnActivityResult.invoke(mActivity, filedStartCamera.getInt(mActivity),
                Activity.RESULT_OK, null);
        waitForFragment(WARING_FRAGMENT_TAG);
        Logger.d(TAG, "testCase4_InvalidFileName() exit");
    }
    
    /**
     * Test to share a file from Camera with the contact does not have file
     * transfer capability
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase5_RequestCodeCamera() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase5_RequestCodeCamera() entry");
        mActivity = getNullActionActivity();
        assertNotNull(mActivity);
        Method methodOnActivityResult =
                Utils.getPrivateMethod(PluginProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        Field filedCameraTempFileUri =
                Utils.getPrivateField(PluginProxyActivity.class, "mCameraTempFileUri");
        filedCameraTempFileUri.set(mActivity, getFileUri());
        Field filedStartCamera =
                Utils.getPrivateField(PluginProxyActivity.class, "REQUEST_CODE_CAMERA");
        methodOnActivityResult.invoke(mActivity, filedStartCamera.getInt(mActivity),
                Activity.RESULT_OK, null);
        waitForFragment(FRAGMENT_TAG);
        Logger.d(TAG, "testCase5_RequestCodeCamera() exit");
    }

    /**
     * Test to share a file from gallery with the contact have file transfer
     * capability
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase6_RequestCodeGallery() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase6_RequestCodeGallery() entry");
        mActivity = getNullActionActivity();
        assertNotNull(mActivity);
        clearTestCotnact();
        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        Logger.d(TAG, "testCase6_RequestCodeGallery(), rawContactId is " + rawContactId);
        addContact(values, rawContactId, TEST_NUMBER, true);
        ContactsManager.createInstance(getInstrumentation().getTargetContext());
        Capabilities capabilities = ContactsManager.getInstance().getContactCapabilities(
                TEST_NUMBER);
        Logger.d(
                TAG,
                "testCase6_RequestCodeGallery(), isFileTransferSupported "
                        + capabilities.isFileTransferSupported());
        Method methodOnActivityResult = Utils.getPrivateMethod(PluginProxyActivity.class,
                "onActivityResult", int.class, int.class, Intent.class);
        Intent intent = new Intent();
        intent.setData(getFileUri());
        Field filedStartGallery =
                Utils.getPrivateField(PluginProxyActivity.class, "REQUEST_CODE_GALLERY");
        methodOnActivityResult.invoke(mActivity, filedStartGallery.getInt(mActivity),
                Activity.RESULT_OK, intent);
        waitForActivity(ChatScreenActivity.class.getName());
        deleteContact();
        Logger.d(TAG, "testCase6_RequestCodeGallery() exit");
    }

    /**
     * Test to share a file from Vcard
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase7_RequestCodeVcard() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase7_RequestCodeVcard() entry");
        mActivity = getSingleFtActionActivity(
                PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION, VCARD_DATA_TYPE,
                getVcardUri());
        assertNotNull(mActivity);
        Method methodOnActivityResult =
                Utils.getPrivateMethod(PluginProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        Field filedVcard = Utils.getPrivateField(PluginProxyActivity.class, "REQUEST_CODE_VCARD");
        methodOnActivityResult.invoke(mActivity, filedVcard.getInt(mActivity), Activity.RESULT_OK,
                null);
        waitForActivity(SelectContactsActivity.class.getName());
        Field IS_SUPPORT_FIELD = Utils.getPrivateField(ContactsListManager.class, "IS_SUPPORT");
        ContactsListManager contactsListManager = ContactsListManager.getInstance();
        IS_SUPPORT_FIELD.setBoolean(contactsListManager, true);
        methodOnActivityResult.invoke(mActivity, filedVcard.getInt(mActivity), Activity.RESULT_OK,
                null);
        Thread.sleep(2000);
        mActivity.finishActivity(PluginProxyActivity.REQUEST_CODE_RCSE_CONTACT);
        Logger.d(TAG, "testCase7_RequestCodeVcard() exit");
    }

    /**
     * Test to share calendar
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase8_RequestCodeVcalendar() throws NoSuchMethodException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, InterruptedException {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase8_RequestCodeVcalender() entry");
        Uri uri = addCalendarAndGetVcalendarUri();
        mActivity = getSingleFtActionActivity(
                PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION, VCALENDAR_DATA_TYPE, uri);
        assertNotNull(mActivity);
        Method methodOnActivityResult = Utils.getPrivateMethod(PluginProxyActivity.class,
                "onActivityResult", int.class, int.class, Intent.class);
        Field REQUEST_CODE_VCALENDER_FIELD = Utils.getPrivateField(PluginProxyActivity.class,
                "REQUEST_CODE_VCALENDER");
        methodOnActivityResult.invoke(mActivity, REQUEST_CODE_VCALENDER_FIELD.getInt(mActivity),
                Activity.RESULT_OK, null);
        waitForActivity(SelectContactsActivity.class.getName());
        deleteCalendar(uri);
        Logger.d(TAG, "testCase8_RequestCodeVcalender() exit");
    }

    /**
     * start PluginPro,intent contains null uri and wrong type,and then
     * PluginProxy will finish by itself
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase9_ShowWarningDialogFragment() {
        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase9_ShowWarningDialogFragment() entry");
        mActivity = getSingleFtActionActivity(
                PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION, "test", null);
        assertNotNull(mActivity);
        assertTrue(mActivity.isFinishing());
        Logger.d(TAG, "testCase9_ShowWarningDialogFragment() exit");
    }

    /**
     * test for handleImAction and IsImSessionSupported is false,so will show
     * warning dialog
     * 
     * @throws Throwable
     */

    public void testCase10_handleImActionImSessionIsNotSupported() throws Throwable {
       if(ALLOW_COMPILE)return ;  String WarningDialogTAG = "ImCapabilityAsyncTask";
        Logger.d(TAG, "testCase10_handleImAction() entry");
        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        Logger.d(TAG, "testCase10_handleImAction(), rawContactId is " + rawContactId);
        addContact(values, rawContactId, TEST_NUMBER_NO_CAPABILITY, false);
        mActivity = getProxyActionActivity(PluginApiManager.RcseAction.PROXY_ACTION,
                TEST_NUMBER_NO_CAPABILITY, TEST_NUMBER_NO_CAPABILITY_NAME);
        assertNotNull(mActivity);

        // wait for warningDialogFragment
        waitForFragment(WarningDialogTAG);
        deleteContact();
        Logger.d(TAG, "testCase10_handleImAction() exit");
    }

    /**
     * test for handleImAction and IsImSessionSupported is false,so will show
     * warning dialog
     * 
     * @throws Throwable
     */

    /*
    public void testCase11_handleImActionImSessionIsSupported() throws Throwable {
        if(ALLOW_COMPILE)return ; String WarningDialogTAG = "ImCapabilityAsyncTask";
        Logger.d(TAG, "testCase11_handleImActionImSessionIsSupported() entry");
        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        Logger.d(TAG, "testCase11_handleImActionImSessionIsSupported(), rawContactId is "
                + rawContactId);
        addContact(values, rawContactId, TEST_NUMBER, false);
        ContactsManager.getInstance().setContactCapabilities(TEST_NUMBER,
                mockCapabilityApi.getContactCapabilities(TEST_NUMBER));
        mActivity = getProxyActionActivity(PluginApiManager.RcseAction.PROXY_ACTION, TEST_NUMBER,
                TEST_NAME);
        assertNotNull(mActivity);

        // wait for ChatMainActivity
        waitForActivity(ChatScreenActivity.class.getName());
        waitForActivityFinish();
        deleteContact();
        Logger.d(TAG, "testCase11_handleImActionImSessionIsSupported() exit");
    }
*/
    /**
     * test for SINGLE_FILE_TRANSFER_ACTION,send a img file
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase12_handleSingleFileActionImgFile() throws NoSuchMethodException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {

        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase12_handleSingleFileActionImgFile() entry");
        Logger.d(TAG, "waitForActivity() entry");
        ActivityMonitor monitor = getInstrumentation().addMonitor(
                SelectContactsActivity.class.getName(), null, false);

        Logger.d(TAG, "waitForActivity() exit");
        mActivity = getSingleFtActionActivity(
                PluginApiManager.RcseAction.SINGLE_FILE_TRANSFER_ACTION, GALLERY_TYPE,
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        assertNotNull(mActivity);
        Activity activity = getInstrumentation().waitForMonitorWithTimeout(monitor, TIME_OUT);
        Logger.d(TAG, "waitForActivity() activity is " + activity);
        try {
            assertNotNull(mActivity);
            assertNotNull(activity);
        } finally {
            getInstrumentation().removeMonitor(monitor);
            if (null != monitor) {
                activity.finish();
            }
        }
        Logger.d(TAG, "testCase12_handleSingleFileActionImgFile() exit");
    }

    /**
     * show option dialog and choose Gallery
     * 
     * @throws Throwable
     */
    public void testCase13_handleFtActionShowOptionDialogClickGallery() throws Throwable {

        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase13_handleFtActionShowOptionDialog()");
        String FT_TAG = "FtCapabilityAsyncTask";
        final int POSITION_GALLERY = 0;
        final int REQUEST_CODE_GALLERY = 11;

        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        addContact(values, rawContactId, TEST_NUMBER_MORE_CAPABILITY, true);
        Logger.d(
                TAG,
                "ft "
                        + ContactsManager.getInstance()
                                .getContactCapabilities(TEST_NUMBER_MORE_CAPABILITY)
                                .isFileTransferSupported());
        mActivity = getFtActionActivity(true, null, TEST_NUMBER_MORE_CAPABILITY);
        assertNotNull(mActivity);
        // wait for option dialog
        waitForFragment(FT_TAG);
        final DialogFragment optionDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(FT_TAG);
        assertNotNull(optionDialog);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        Thread.sleep(SLEEP_TIME_FOR_MULIMEDIA);
        Logger.d(TAG, "startGallery()");
        mActivity.finishActivity(REQUEST_CODE_GALLERY);
        Logger.d(TAG, "closeGallery()");
        deleteContact();
        Logger.d(TAG, "testCase13_handleFtActionShowOptionDialog() exit");
    }

    /**
     * show option dialog and choose Camera
     * 
     * @throws Throwable
     */
    public void testCase14_handleFtActionShowOptionDialogClickCamera() throws Throwable {

       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase14_handleFtActionShowOptionDialogClickCamera()");
        final String FT_TAG = "FtCapabilityAsyncTask";
        final int POSITION_CAMERA = 1;
        final int REQUEST_CODE_CAMERA = 10;

        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        addContact(values, rawContactId, TEST_NUMBER_MORE_CAPABILITY, true);
        Logger.d(TAG, "filetransfer capability: "
                + ContactsManager.getInstance().getContactCapabilities(TEST_NUMBER_MORE_CAPABILITY)
                        .isFileTransferSupported());
        mActivity = getFtActionActivity(true, null, TEST_NUMBER_MORE_CAPABILITY);
        assertNotNull(mActivity);
        // wait for option dialog
        waitForFragment(FT_TAG);
        final DialogFragment optionDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(FT_TAG);
        assertNotNull(optionDialog);

        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        Thread.sleep(TIME_OUT);
        Logger.d(TAG, "startCamera");
        mActivity.finishActivity(REQUEST_CODE_CAMERA);
        Logger.d(TAG, "closeCamera()");
        deleteContact();
        Logger.d(TAG, "testCase14_handleFtActionShowOptionDialogClickCamera() exit");
    }

    /**
     * show option dialog and choose FileManager
     * 
     * @throws Throwable
     */
    public void testCase15_handleFtActionShowOptionDialogClickFileManager() throws Throwable {

        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase15_handleFtActionShowOptionDialogClickFileManager()");
        String FT_TAG = "FtCapabilityAsyncTask";
        final int POSITION_FILE_MANAGER = 2;
        final int REQUEST_CODE_FILE_MANAGER = 12;
        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        addContact(values, rawContactId, TEST_NUMBER_MORE_CAPABILITY, true);
        Logger.d(TAG, "filetransfer capability: "
                + ContactsManager.getInstance().getContactCapabilities(TEST_NUMBER_MORE_CAPABILITY)
                        .isFileTransferSupported());
        mActivity = getFtActionActivity(true, null, TEST_NUMBER_MORE_CAPABILITY);
        assertNotNull(mActivity);
        // wait for option dialog
        waitForFragment(FT_TAG);
        final DialogFragment optionDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(FT_TAG);
        assertNotNull(optionDialog);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_DOWN,
                KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        Thread.sleep(SLEEP_TIME_FOR_MULIMEDIA);
        Logger.d(TAG, "startFileManager()");
        mActivity.finishActivity(REQUEST_CODE_FILE_MANAGER);
        Logger.d(TAG, "closeFileManagers()");
        deleteContact();
        Logger.d(TAG, "testCase15_handleFtActionShowOptionDialogClickFileManager() exit");
    }

    /**
     * test for startMultiChoiceActivity()
     * 
     * @throws Throwable
     */
    public void testCase16_startMultiChoiceActivity() throws Throwable {

        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase16_startMultiChoiceActivity()");
        final int REQUEST_CODE_RCSE_CONTACT = 13;
        mActivity = getActivity();
        Method startMultiChoiceActivityMethod;
        try {
            startMultiChoiceActivityMethod = Utils.getPrivateMethod(
                    PluginProxyActivity.class, "startMultiChoiceActivity");
            startMultiChoiceActivityMethod.invoke(mActivity);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
            fail();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            fail();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        getInstrumentation().waitForIdleSync();
        mActivity.finishActivity(REQUEST_CODE_RCSE_CONTACT);
        Logger.d(TAG, "testCase16_startMultiChoiceActivity() exit");
    }

    /**
     * test for handleMultiFileAction()
     */
    /*public void testCase17_handleMultiFileAction() {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase17_handleMultiFileAction() entry");
        final int REQUEST_CODE_RCSE_CONTACT = 13;
        ArrayList<Uri> originList = new ArrayList<Uri>();
        originList.add(MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        originList.add(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI);
        mActivity = getMulitFileActionActivity(
                PluginApiManager.RcseAction.MULTIPLE_FILE_TRANSFER_ACTION, originList);
        mActivity.finishActivity(REQUEST_CODE_RCSE_CONTACT);
        Logger.d(TAG, "testCase17_handleMultiFileAction() exit");
    }
 */
    /**
     * test for getFileFullPathFromUri()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase18_getFileFullPathFromUri() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase18_getFileFullPathFromUri() entry");
        mActivity = getActivity();
        Method getFileFullPathFromUriMethod = Utils.getPrivateMethod(PluginProxyActivity.class,
                "getFileFullPathFromUri", Uri.class);

        // File URI
        String originFilePath = "/this/is/a/file";
        final String FILE_SCHEMA = "file://";
        Uri fileUri = Uri.parse(FILE_SCHEMA + originFilePath);
        String filePathString = (String) getFileFullPathFromUriMethod.invoke(mActivity, fileUri);
        assertEquals(originFilePath, filePathString);

        // wrong VCARD URI
        final String INVALID_FILE_NAME = "invalid_file_name";
        String wrongVcardPath = "/this/is/a/vcard";
        final String VCARD_SCHEMA = "content://com.android.contacts/contacts/as_vcard";
        Uri VcardUri = Uri.parse(VCARD_SCHEMA + wrongVcardPath);
        String wrongVcardPathString = (String) getFileFullPathFromUriMethod.invoke(mActivity,
                VcardUri);
        assertEquals(INVALID_FILE_NAME, wrongVcardPathString);

        // wrong VCALENDAR URI
        final String VCALENDAR_SCHEMA = "content://com.mediatek.calendarimporter/";
        String wrongVCalendarPath = "/this/is/a/vcalendar";
        Uri VcalendarUri = Uri.parse(VCALENDAR_SCHEMA + wrongVCalendarPath);
        String wrongVCalendarPathString = (String) getFileFullPathFromUriMethod.invoke(mActivity,
                VcardUri);
        assertEquals(INVALID_FILE_NAME, wrongVCalendarPathString);

        // wrong type
        final String INVALID_FILE_TYPE = "invalid_file_type";
        final String WRONG_SCHEMA = "wrongcontent://";
        String wrongPath = "/this/is/a/wrongpath";
        Uri wrongUri = Uri.parse(WRONG_SCHEMA + wrongPath);
        String wrongPathString = (String) getFileFullPathFromUriMethod.invoke(mActivity, wrongUri);
        assertEquals(INVALID_FILE_TYPE, wrongPathString);

        // null URI
        Uri nullUri = Uri.EMPTY;
        nullUri = null;
        String nullUriString = (String) getFileFullPathFromUriMethod.invoke(mActivity, nullUri);
        assertEquals(INVALID_FILE_NAME, nullUriString);
        Logger.d(TAG, "testCase18_getFileFullPathFromUri() exit");
    }

    /**
     * test for startFileTransfer in IntegrationMode,has Multiple Different Type
     * files
     * 
     * @throws Throwable
     */
    public void testCase19_startFileTransferIsIntegrationModeFileSizeIsMultipleDifferentType()
            throws Throwable {

        if(ALLOW_COMPILE)return ; Logger.d(TAG,
                "testCase19_startFileTransferIsIntegrationModeFileSizeIsMultipleDifferentType()");
        boolean isIntegrationMode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        ArrayList<String> fileList = new ArrayList<String>();
        String imgFilePath = getFilePath();
        String audioFilePathString = getAudioFilePath();
        fileList.add(imgFilePath);
        fileList.add(audioFilePathString);
        mActivity = getActivity();
        Method getIntentMethod = Utils.getPrivateMethod(PluginProxyActivity.class, "getIntent",
                String.class, String.class, ArrayList.class);
        Intent intent = (Intent) getIntentMethod
                .invoke(mActivity, TEST_NUMBER, TEST_NAME, fileList);
        assertEquals(Intent.ACTION_SEND_MULTIPLE, intent.getAction());
        Logger.setIsIntegrationMode(isIntegrationMode);
        Logger.d(TAG,
                "testCase19_startFileTransferIsIntegrationModeFileSizeIsMultipleDifferentType() exit");
    }

    /**
     * test for startFileTransfer in IntegrationMode,has only one file
     * 
     * @throws Throwable
     */
    public void testCase20_startFileTransferIsIntegrationModeFileSizeIsOne() throws Throwable {

       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase20_startFileTransferIsIntegrationModeFileSizeIsOne()");
        boolean isIntegrationMode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        ArrayList<String> fileList = new ArrayList<String>();
        String imgFilePath = getFilePath();
        fileList.add(imgFilePath);
        mActivity = getActivity();
        Method getIntentMethod = Utils.getPrivateMethod(PluginProxyActivity.class, "getIntent",
                String.class, String.class, ArrayList.class);
        Intent intent = (Intent) getIntentMethod
                .invoke(mActivity, TEST_NUMBER, TEST_NAME, fileList);
        assertEquals(Intent.ACTION_SEND_MULTIPLE, intent.getAction());
        Logger.setIsIntegrationMode(isIntegrationMode);
        Logger.d(TAG, "testCase20_startFileTransferIsIntegrationModeFileSizeIsOne() exit");
    }

    /**
     * test for startFileTransfer in IntegrationMode,has only two file with the
     * same type
     * 
     * @throws Throwable
     */
    public void testCase21_startFileTransferIsIntegrationModeFileSizeIsMultipleSameFileType()
            throws Throwable {

        if(ALLOW_COMPILE)return ; Logger.d(TAG,
                "testCase21_startFileTransferIsIntegrationModeFileSizeIsMultipleSameFileType()");
        boolean isIntegrationMode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        ArrayList<String> fileList = new ArrayList<String>();
        String imgFilePath = getFilePath();
        fileList.add(imgFilePath);
        fileList.add(imgFilePath);
        mActivity = getActivity();
        Method getIntentMethod = Utils.getPrivateMethod(PluginProxyActivity.class, "getIntent",
                String.class, String.class, ArrayList.class);
        Intent intent = (Intent) getIntentMethod
                .invoke(mActivity, TEST_NUMBER, TEST_NAME, fileList);
        assertEquals(Intent.ACTION_SEND_MULTIPLE, intent.getAction());
        Logger.setIsIntegrationMode(isIntegrationMode);
        Logger.d(TAG,
                "testCase21_startFileTransferIsIntegrationModeFileSizeIsMultipleSameFileType() exit");
    }
    
    public void testCase22_onKeyUp() throws Throwable {
        if(ALLOW_COMPILE)return ; Logger.d(TAG, "testCase22_onKeyUp()");
        String FT_TAG = "FtCapabilityAsyncTask";
        final int POSITION_FILE_MANAGER = 2;
        final int REQUEST_CODE_FILE_MANAGER = 12;
        ContentValues values = new ContentValues();
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        addContact(values, rawContactId, TEST_NUMBER_MORE_CAPABILITY, true);
        Logger.d(TAG, "filetransfer capability: "
                + ContactsManager.getInstance().getContactCapabilities(TEST_NUMBER_MORE_CAPABILITY)
                        .isFileTransferSupported());
        mActivity = getFtActionActivity(true, null, TEST_NUMBER_MORE_CAPABILITY);
        assertNotNull(mActivity);
        // wait for option dialog
        waitForFragment(FT_TAG);
        final DialogFragment optionDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(FT_TAG);
        Method onKeyUpMethod=Utils.getPrivateMethod(PluginProxyActivity.class, "onKeyUp", int.class,KeyEvent.class);
        onKeyUpMethod.invoke(mActivity,KeyEvent.KEYCODE_BACK, null);
        getInstrumentation().waitForIdleSync();
        waitForActivityFinish();
        Logger.d(TAG, "testCase22_onKeyUp() exit");
    }
    
    public void testCase23_imChatIntegrationMode() throws Throwable {
       if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase23_imChatIntegrationMode()");
        boolean isIntegrationMode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        mActivity = getActivity();
        Method getImChatIntentMethod = Utils.getPrivateMethod(PluginProxyActivity.class, "getImChatIntent",
                String.class, String.class);
        Intent intent = (Intent) getImChatIntentMethod
                .invoke(mActivity,TEST_NAME , TEST_NUMBER);
        assertEquals(Intent.ACTION_SENDTO, intent.getAction());
        Logger.setIsIntegrationMode(isIntegrationMode);
        Logger.d(TAG, "testCase23_imChatIntegrationMode() exit");
    }

    private void waitForActivityFinish() throws InterruptedException {
        Logger.d(TAG, "waitForActivityFinish() entry");
        assertNotNull(mActivity);
        long startTime = System.currentTimeMillis();
        while (!mActivity.isFinishing()) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForActivityFinish() timeout");
            }
            Thread.sleep(SLEEP_TIME);
        }
        Logger.d(TAG, "waitForActivityFinish() exit");
    }

    private void waitForActivity(String className) {
        Logger.d(TAG, "waitForActivity() entry");
        ActivityMonitor monitor = getInstrumentation().addMonitor(className, null, false);

        Activity activity = getInstrumentation().waitForMonitorWithTimeout(monitor, TIME_OUT);
        Logger.d(TAG, "waitForActivity() activity is " + activity);
        try {
            assertNotNull(mActivity);
            assertNotNull(activity);
        } finally {
            getInstrumentation().removeMonitor(monitor);
            if (null != activity) {
                activity.finish();
            }
        }
        Logger.d(TAG, "waitForActivity() exit");
    }
    
    private void waitForFragment(String type) throws InterruptedException {
        Logger.d(TAG, "waitForFragment() entry with type is " + type);
        Fragment fragment = mActivity.getFragmentManager().findFragmentByTag(type);
        assertNull(fragment);
        long startTime = System.currentTimeMillis();
        while (fragment == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForFragment() timeout");
            }
            Thread.sleep(SLEEP_TIME);
            fragment = mActivity.getFragmentManager().findFragmentByTag(type);
        }
        Logger.d(TAG, "waitForFragment() exit with fragment is " + fragment);
    }

    private void waitForRegistrationApiIsNotNull(Field fieldRegistrationApi) throws Exception {
        Logger.d(TAG, "waitForApiManagerIsNotNull() entry");
        long startTime = System.currentTimeMillis();
        while (null == fieldRegistrationApi.get(ApiManager.getInstance())) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForFragment() timeout");
            }
            Thread.sleep(SLEEP_TIME);
        }
        Logger.d(TAG, "waitForApiManagerIsNotNull() exit ");
    }

    private void waitForCapabilitiesApiIsNotNull(Field fieldCapabilitiesApi) throws Exception {
        Logger.d(TAG, "waitForCapabilitiesApiIsNotNull() entry");
        long startTime = System.currentTimeMillis();
        while (null == fieldCapabilitiesApi.get(ApiManager.getInstance())) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForFragment() timeout");
            }
            Thread.sleep(SLEEP_TIME);
        }
        Logger.d(TAG, "waitForCapabilitiesApiIsNotNull() exit ");
    }

    private void addContact(ContentValues values, long rawContactId, String phoneNumber,
            boolean reg_flag) {
        Logger.d(TAG, "addContact() entry, rawContactId is " + rawContactId);
        values.clear();
        values.put(Data.RAW_CONTACT_ID, rawContactId);
        values.put(Data.MIMETYPE, MIMETYPE_RCSE_CAPABILITIES);
        values.put(Phone.NUMBER, phoneNumber);
        if (reg_flag) {
            values.put(Data.DATA3, 1);
        }
        values.put(Data.DATA6, true);
        mContentResolver.insert(ContactsContract.Data.CONTENT_URI, values);
        values.clear();
        values.put(Data.RAW_CONTACT_ID, rawContactId);
        values.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
        values.put(Phone.NUMBER, phoneNumber);
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
    
    private void clearTestCotnact() {
        Logger.d(TAG, "clearTestCotnact() entry");
        String selection = "(" + Data.MIMETYPE + "=? OR "+ Data.MIMETYPE + "=?) AND " + Data.DATA1 + "=?";
        String[] selectionArgs = { MIMETYPE_RCSE_CAPABILITIES,Phone.CONTENT_ITEM_TYPE, TEST_NUMBER };
        try {
            mContentResolver.delete(ContactsContract.Data.CONTENT_URI, selection, selectionArgs);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "clearTestCotnact() exit");
    }

    private Uri addCalendarAndGetVcalendarUri() {
        Logger.d(TAG, "addCalendar() entry ");
        ContentValues values = new ContentValues();
        values.put(Events.TITLE, "RCS TEST");
        values.put(Events.DTSTART, 1346461200000L);
        values.put(Events.CALENDAR_ID, 1);
        values.put(Events.EVENT_TIMEZONE, "UTC");
        values.put(Events.DTEND, 1346463200000L);
        Uri uri = mContentResolver.insert(Events.CONTENT_URI, values);
        String calendarId = String.valueOf(ContentUris.parseId(uri));
        uri = Uri.withAppendedPath(Uri.parse(VCALENDAR_SCHEMA), calendarId);
        return uri;
    }

    private boolean deleteCalendar(Uri uri) {
        return mContentResolver.delete(uri, null, null) < 0 ? false : true;
    }

    private void waitForLoadContact() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, InterruptedException {
        long startTime = System.currentTimeMillis();
        List contactsList = null;
        do {
            Field field = Utils.getPrivateField(ContactsListManager.class, "CONTACTS_LIST");
            assertNotNull(field);
            contactsList = (List) field.get(ContactsListManager.getInstance());
            assertNotNull(contactsList);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (contactsList.size() == 0);
    }

    // Mock capabilityApi to pass capability check
    private class MockCapabilityApi extends CapabilityApi {

        public MockCapabilityApi(Context context) {
            super(context);
        }

        @Override
        public void connect() {
            // Mock api do not need to do real connect
        }

        @Override
        public void disconnect() {
            // Mock api do not need to do real connect
        }

        @Override
        public Capabilities getContactCapabilities(String contact) {
            if (TEST_NUMBER.equals(contact)) {
                Capabilities capabilities = new Capabilities();
                capabilities.setFileTransferSupport(true);
                capabilities.setRcseContact(true);
                capabilities.setImSessionSupport(true);
                return capabilities;
            } else {
                return super.getContactCapabilities(contact);
            }
        }

        @Override
        public Capabilities getMyCapabilities() {
            Capabilities capabilities = new Capabilities();
            capabilities.setCsVideoSupport(true);
            capabilities.setFileTransferSupport(true);
            capabilities.setRcseContact(true);
            capabilities.setImSessionSupport(true);
            return capabilities;
        }
        
    }

    private class MockRegistrationApi extends RegistrationApi {

        public MockRegistrationApi(Context context) {
            super(context);
        }

        @Override
        public void connect() {
            // Mock api do not need to do real connect
        }

        @Override
        public void disconnect() {
            // Mock api do not need to do real connect
        }

        @Override
        public boolean isRegistered() {
            return true;
        }
    }

    private PluginProxyActivity getSingleFtActionActivity(String action, String intentType, Uri uri) {
        Intent intent = new Intent(action);
        if (!intentType.equals("")) {
            intent.setType(intentType);
        }
        intent.putExtra(Intent.EXTRA_STREAM, uri);
        setActivityIntent(intent);
        return getActivity();
    }

    private PluginProxyActivity getMulitFileActionActivity(String action, ArrayList<Uri> originList) {
        Intent intent = new Intent(action);
        intent.putExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_TRANSFER_ACTION, true);
        intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, originList);
        setActivityIntent(intent);
        return getActivity();
    }

    private PluginProxyActivity getProxyActionActivity(String action, String contactNumber,
            String contactName) {
        Intent intent = new Intent(action);
        intent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, contactNumber);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, contactName);
        setActivityIntent(intent);
        return getActivity();
    }

    private PluginProxyActivity getNullActionActivity() {
        Intent intent = new Intent();
        intent.setAction(null);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, TEST_NAME);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, TEST_NUMBER);
        setActivityIntent(intent);
        return getActivity();
    }

    private Uri getFileUri() {
        Cursor cursor = null;
        int imageId = 0;
        try {
            cursor =
                    mContentResolver.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null,
                            null, null, null);
            cursor.moveToFirst();
            imageId = cursor.getInt(cursor.getColumnIndex(MediaStore.Images.ImageColumns._ID));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri uri = Uri.parse("content://media/external/images/media/" + imageId);
        Logger.d(TAG, "getFileUri() exit with uri is " + uri);
        return uri;
    }

    private Uri getVcardUri() {
        Cursor cursor = null;
        String lookupKey = null;
        try {
            cursor =
                    mContentResolver.query(ContactsContract.Contacts.CONTENT_URI, null, null, null,
                            null);
            cursor.moveToFirst();
            lookupKey = cursor.getString(cursor.getColumnIndex(Contacts.LOOKUP_KEY));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri uri = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_VCARD_URI, lookupKey);
        Logger.d(TAG, "getVcardUri() exit with uri is " + uri);
        return uri;
    }

    /**
     * click the button of dialog showed
     * 
     * @param confirmDialog
     * @param dialogButtonIndicator
     */
    public void clickDialogButton(DialogFragment confirmDialog, final int dialogButtonIndicator) {
        final DialogInterface.OnClickListener dialogFragment = (DialogInterface.OnClickListener) confirmDialog;
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                dialogFragment.onClick(null, dialogButtonIndicator);
            }
        });
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
                fail("testCase1_setChatGalleryWallPaper() Cannot find image in sdcard");
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

    /**
     * Get a audio file path from database
     */
    private String getAudioFilePath() {
        Logger.v(TAG, "getFilePath()");
        Context context = getInstrumentation().getTargetContext();
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            } else {
                fail("testCase1_setChatGalleryWallPaper() Cannot find image in sdcard");
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
