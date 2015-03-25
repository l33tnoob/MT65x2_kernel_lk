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

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.location.IGeocodeProvider;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts.Data;
import android.provider.ContactsContract.RawContacts;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;

import com.mediatek.rcse.activities.BlackListActivity;
import com.mediatek.rcse.activities.BlackListAdapter;
import com.mediatek.rcse.activities.BlackListFragment;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.eab.ContactsManager.StrangerBlocker;
import com.orangelabs.rcs.provider.eab.ContactsManagerException;

public class BlackListFragmentActivityTest extends
        ActivityInstrumentationTestCase2<BlackListActivity> {

    private ContentResolver mContentResolver = null;
    private Context mContext = null;
    private ContactsManager contactsManager = null;
    private static final String TAG = "BlackListFragmentActivityTest";
    private static final String DIALOG_TAG = "UnBlockingDialog";
    private static final String MOCK_CONTACT = "+34200111111";
    private static final String MOCK_STRANGER_CONTACT = "+34200000999";
    private static final String RCS_CONTACT = "1";
    private static final String MIMETYPE_RCS_CONTACT = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    public static final int LAUNCH_TIME_OUT = 1000;
    private static final int TIME_OUT = 50000;
    private static final int SLEEP_TIME = 200;
    private long mRawContactId = -1;

    private BlackListFragment blackListFragment = null;

    public BlackListFragmentActivityTest() {
        super(BlackListActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        // when testcase is started,we prepare two blocked contact(one is Rcse
        // contact,the other is stranger contact) for testing
        mContext = getInstrumentation().getTargetContext();
        mContentResolver = mContext.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(Phone.DISPLAY_NAME, "abcdef");
        Uri rawContUri = mContentResolver.insert(RawContacts.CONTENT_URI, values);
        long rawContactId = ContentUris.parseId(rawContUri);
        mRawContactId = rawContactId;
        Logger.d(TAG, "setUp(), rawContactId is " + mRawContactId);
        addContact(values, rawContactId, MOCK_CONTACT);

        ContactsManager.createInstance(getInstrumentation().getTargetContext());
        contactsManager = ContactsManager.getInstance();
        List<String> imBlockContactsList = contactsManager.getImBlockedContacts();
        List<String> rcsBlockContactsList = contactsManager.getRcsBlockedContacts();
        ContactsManager.StrangerBlocker.initialize(getInstrumentation().getTargetContext());
        Set<String> strangerBlockContactsList = ContactsManager.StrangerBlocker.getAllBlockedList();

        // clear blocked contact in the phone
        // clear stranger blocked contacts first
        if(strangerBlockContactsList!=null){
            Iterator<String> interator=strangerBlockContactsList.iterator();
            if(interator!=null){
            while(interator.hasNext()){
                String tmpString=interator.next();
                Logger.d("Stranger", tmpString);
                    ContactsManager.StrangerBlocker.unblockContact(tmpString);
                }
            }
        }
        // clear IM blocked contacts
        int imBlockContactsNum = imBlockContactsList.size();
        if (imBlockContactsNum > 0) {

            for (int i = 0; i < imBlockContactsNum; i++) {
                contactsManager.setImBlockedForContact(imBlockContactsList.get(i), false);
            }
        }

        // clear RCSe blocked contacts
        int rcsBlockContactsNum = rcsBlockContactsList.size();

        if (rcsBlockContactsNum > 0) {

            for (int i = 0; i < rcsBlockContactsNum; i++) {
                contactsManager.unblockContact(rcsBlockContactsList.get(i));
            }
        }

        // add a contact and set it as IM blocked contact
        contactsManager.setImBlockedForContact(MOCK_CONTACT, true);
        assertTrue(contactsManager.isImBlockedForContact(MOCK_CONTACT));
        // if we have no contact in stranger block contact list,we add one
        // contact in the list
        if (strangerBlockContactsList.size() == 0) {
            ContactsManager.StrangerBlocker.blockContact(MOCK_STRANGER_CONTACT);
        }

    }

    public void testCase1_OnClickBlackListItem() throws Throwable {

        final int selection = 1;
        final BlackListFragment blackListFragment = getFragment(getActivity());
        final BlackListAdapter blackListAdapter = blackListFragment.getAdapter();
        long startTime = System.currentTimeMillis();
        int count=0;
        // wait for blacklist contact loaded
        while ( count== 0) {
            count=blackListFragment.getAdapter().getCount();
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);

        }
        // get the phone number of the selected item
        String selectedNumer = blackListAdapter.getContactsList().get(selection).mNumber;
        final long itemId = blackListAdapter.getItemId(selection);

        // click blacklist
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                blackListFragment.onItemClick(null, null, selection, itemId);
            }
        });

        getInstrumentation().waitForIdleSync();
        DialogFragment unBlockingDialog = (DialogFragment) blackListFragment.getFragmentManager()
                .findFragmentByTag(DIALOG_TAG);
        assertNotNull(unBlockingDialog);
        clickDialogButton(unBlockingDialog, DialogInterface.BUTTON_NEGATIVE);
        getInstrumentation().waitForIdleSync();
        assertNull(blackListFragment.getFragmentManager().findFragmentByTag(DIALOG_TAG));

        // click blacklist
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                blackListFragment.onItemClick(null, null, selection, itemId);
            }
        });

        getInstrumentation().waitForIdleSync();
        unBlockingDialog = (DialogFragment) blackListFragment.getFragmentManager()
                .findFragmentByTag(DIALOG_TAG);
        assertNotNull(unBlockingDialog);

        // blocked status is not changed and dialog dismiss
        if (selectedNumer.equals(MOCK_CONTACT)) {
            assertTrue(contactsManager.isImBlockedForContact(selectedNumer));
            // do unblock operation,change the blocked status
            clickDialogButton(unBlockingDialog, DialogInterface.BUTTON_POSITIVE);
            startTime = System.currentTimeMillis();
            while (contactsManager.isImBlockedForContact(selectedNumer) == true) {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            }
            Logger.d("selecNum:", MOCK_CONTACT);
            assertFalse(contactsManager.isImBlockedForContact(selectedNumer));
        } else if (selectedNumer.equals(MOCK_STRANGER_CONTACT)) {
            assertTrue(ContactsManager.StrangerBlocker.isContactBlocked(MOCK_STRANGER_CONTACT));
            // do unblock operation,change the blocked status
            clickDialogButton(unBlockingDialog, DialogInterface.BUTTON_POSITIVE);
            startTime = System.currentTimeMillis();
            while (ContactsManager.StrangerBlocker.isContactBlocked(MOCK_STRANGER_CONTACT) == true) {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            }
            Logger.d("selecNum:", MOCK_STRANGER_CONTACT);
            assertFalse(ContactsManager.StrangerBlocker.isContactBlocked(MOCK_STRANGER_CONTACT));
        }
    }

    public void testCase2_LoadBlackListRight() throws ContactsManagerException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            InterruptedException {

        blackListFragment = getFragment(getActivity());

        Field field_mCurrentBlockedRcsContact = Utils.getPrivateField(BlackListFragment.class,
                "mCurrentBlockedRcsContact");
        long startTime = System.currentTimeMillis();
        while (field_mCurrentBlockedRcsContact.get(blackListFragment) == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        List<RcsContact> mCurrentBlockedRcsContact = (List<RcsContact>) field_mCurrentBlockedRcsContact
                .get(blackListFragment);

        int count = 0;
        count = mCurrentBlockedRcsContact.size();
        Set<String> mCurrentBlockedStrangerContactSet = ContactsManager.StrangerBlocker
                .getAllBlockedList();
        startTime = System.currentTimeMillis();
        while (mCurrentBlockedStrangerContactSet == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        count += mCurrentBlockedStrangerContactSet.size();
        assertEquals(2, count);

        // get block contact list in the database
        List<String> currentDatabaseBlockRcsContactList = contactsManager.getImBlockedContacts();
        int databaseBlockRcsContactNum = currentDatabaseBlockRcsContactList.size();
        assertEquals(databaseBlockRcsContactNum, count);
    }

    /**
     * Get the fragment in the activity.
     * 
     * @return The fragment in the activity.
     */
    private BlackListFragment getFragment(Activity activity) {
        Logger.d(TAG, "BlackListFragment()");
        assertNotNull(activity);
        View view = activity.findViewById(R.id.black_list_activity);
        assertNotNull(view);
        Fragment fragment = activity.getFragmentManager()
                .findFragmentById(R.id.black_list_activity);
        assertTrue(fragment instanceof BlackListFragment);
        return (BlackListFragment) fragment;
    }

    private void addContact(ContentValues values, long rawContactId, String number) {
        Logger.d(TAG, "addContact() entry, rawContactId is " + rawContactId + " and number is "
                + number);
        values.clear();
        values.put(Data.RAW_CONTACT_ID, rawContactId);
        values.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
        values.put(Phone.NUMBER, number);
        values.put(Data.DATA4, RCS_CONTACT);
        mContentResolver.insert(ContactsContract.Data.CONTENT_URI, values);

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
        Cursor cur = mContentResolver.query(ContactsContract.Data.CONTENT_URI, projection,
                selection, null, null);
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

    @Override
    protected void tearDown() throws Exception {
        // unblock the contact which is blocked by me
        boolean setSuccessFlag = false;//contactsManager.setImBlockedForContact(MOCK_CONTACT, false);
        assertTrue(setSuccessFlag);
        assertFalse(contactsManager.isImBlockedForContact(MOCK_CONTACT));
        ContactsManager.StrangerBlocker.unblockContact(MOCK_STRANGER_CONTACT);
        assertFalse(ContactsManager.StrangerBlocker.isContactBlocked(MOCK_STRANGER_CONTACT));
        // delete the contact I added
        deleteContact();
        Utils.clearAllStatus();
        super.tearDown();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
    }

}
