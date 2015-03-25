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

package com.mediatek.rcse.activities;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.LoaderManager.LoaderCallbacks;

import android.content.CursorLoader;
import android.content.DialogInterface;
import android.content.Loader;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * A fragment act as blacklist fragment.
 */
public class BlackListFragment extends BaseListFragment<BlackListAdapter> implements 
        ContactsManager.StrangerBlocker.OnBlockedListChangedListener, LoaderCallbacks<Cursor> {
    private static final String TAG = "BlacklistFragment";
    private final UnBlockingDialog mUnBlockingDialog = new UnBlockingDialog();
    private String mNumber = null;
    private String mDisplayName = null;
    private RcsContact mContact = null;
    /**
     * MIME type for a block contact
     */
    private static final String MIMETYPE_IM_BLOCKED =
            "vnd.android.cursor.item/com.orangelabs.rcs.im-blocked";
    private static final long UPADE_THROTTLE = 3000;
    private static final String[] PROJECTION = {
            Data.DATA1, Data.DISPLAY_NAME, Contacts.SORT_KEY_PRIMARY, Phone._ID
    };

    private static final int COLUMN_NUMBER = 0;
    private static final int COLUMN_NAME = 1;

    private static final String SELECTION = Data.MIMETYPE + "=?";

    private static final String[] SELECTION_ARGS = {
        MIMETYPE_IM_BLOCKED
    };

    private static final int LOADER_BLACKLIST = 3;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getLoaderManager().initLoader(LOADER_BLACKLIST, null, this);
        ContactsManager.StrangerBlocker.addOnBlockedListChangedListener(this);
    }

    @Override
    public CursorLoader onCreateLoader(int id, Bundle args) {
        CursorLoader cursorLoader =
                new CursorLoader(mContext, Data.CONTENT_URI, PROJECTION, SELECTION, SELECTION_ARGS,
                        ContactsListManager.SORT_ORDER);
        cursorLoader.setUpdateThrottle(UPADE_THROTTLE);

        return cursorLoader;
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        List<RcsContact> contactsList = new ArrayList<RcsContact>();
        Logger.i(TAG, "onLoadFinished() + cursor is" + data + "------ data size is"
                + ((null == data) ? 0 : data.getCount()));
        if (data != null && data.getCount() != 0) {
            while (data.moveToNext()) {
                String displayName = data.getString(COLUMN_NAME);
                String number = data.getString(COLUMN_NUMBER);
                String sortKeyString =
                        data.getString(data.getColumnIndex(Contacts.SORT_KEY_PRIMARY));
                Long phoneId = data.getLong(data.getColumnIndex(Phone._ID));
                short contactId = data.getShort(data.getColumnIndex(Contacts._ID));
                RcsContact contact =
                        new RcsContact(displayName, number, sortKeyString, phoneId, contactId);
                contact.setBlockTime(ContactsManager.getInstance().getTimeStampForBlockedContact(number));
                Logger.i(TAG, "onLoadFinished() + numbe is" + number + "------ displayname is"
                        + displayName);
                if (!contactsList.contains(contact)) {
                    Logger.i(TAG, "a new contact, need to add" + contact.mNumber);
                    contactsList.add(contact);
                } else {
                    Logger.i(TAG, "a duplicate contact, not need to add");
                }
            }
        }
        mCurrentBlockedRcsContact = contactsList;
        refreshBlockList();
    }

    public void onLoaderReset(Loader<Cursor> loader) {
    }

    @Override
    protected View inflateView(LayoutInflater inflater, ViewGroup container) {

        return inflater.inflate(R.layout.contacts_fragment, null);
    }

    @Override
    protected BlackListAdapter createListAdapter() {
        BlackListAdapter dapter = new BlackListAdapter(mContext, this);
        return dapter;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        Logger.i(TAG, "onDetach");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getLoaderManager().destroyLoader(LOADER_BLACKLIST);
        ContactsManager.StrangerBlocker.removeOnBlockedListChangedListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        int position = arg2;
        List<RcsContact> contactsList = mAdapter.getContactsList();
        if (contactsList != null) {
            int size = contactsList.size();
            if (position >= 0 && position < size) {
                mContact = contactsList.get(position);
                mNumber = mContact.mNumber;
                mDisplayName = mContact.mDisplayName;
                Logger.d(TAG, "onItemClick(), mPosition is " + position + ",mNumber is " + mNumber
                        + ",mDisplayName is " + mDisplayName);
            } else {
                Logger.e(TAG, "onItemClick(), mPosition is " + position);
            }
        } else {
            Logger.e(TAG, "onItemClick(), contactsList is null");
        }
        mUnBlockingDialog.show(getFragmentManager(), UnBlockingDialog.TAG);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        return false;
    }

    @Override
    public void onBlackListChanged(final Set<String> currentBlockedList) {
        refreshBlockList(currentBlockedList);
    }

    private List<RcsContact> mCurrentBlockedRcsContact = null;
    private Handler mUiHandler = new Handler(Looper.getMainLooper());

    private void refreshBlockList() {
        refreshBlockList(ContactsManager.StrangerBlocker.getAllBlockedList());
    }

    private void refreshBlockList(Set<String> strangerBlockedList) {
        final List<RcsContact> blockedList = new ArrayList<RcsContact>();
        if (null != strangerBlockedList) {
            Logger.d(TAG, "refreshBlockList() strangerBlockedList is " + strangerBlockedList);
            for (String contact : strangerBlockedList) {
            	RcsContact blockedContact = new RcsContact(contact, contact, contact, null, Short.MIN_VALUE);
            	blockedContact.setBlockTime(ContactsManager.getInstance().getTimeStampForBlockedContact(contact));
                blockedList.add(blockedContact);
            }
        } else {
            Logger.d(TAG, "refreshBlockList() strangerBlockedList is null");
        }
        for(RcsContact contact : mCurrentBlockedRcsContact)
        {
        	contact.setBlockTime(ContactsManager.getInstance().getTimeStampForBlockedContact(contact.mNumber));
        }
        blockedList.addAll(mCurrentBlockedRcsContact);
        if (null != mAdapter) {
            mUiHandler.post(new Runnable() {
                public void run() {
                    Logger.d(TAG, "refreshBlockList() set block list " + blockedList);
                    View view = getView();
                    if (view != null) {
                        View noContacts = view.findViewById(R.id.no_contacts);
                        View contactList = view.findViewById(R.id.contacts_list);
                        if (noContacts != null && contactList != null) {
                            if (blockedList.size() == 0) {
                                noContacts.setVisibility(View.VISIBLE);
                                contactList.setVisibility(View.GONE);
                            } else {
                                noContacts.setVisibility(View.GONE);
                                contactList.setVisibility(View.VISIBLE);
                                mAdapter.setContactsList(blockedList);
                                mAdapter.notifyDataSetChanged();
                            }
                        } else {
                            Logger.e(TAG, "refreshBlockList() the noContacts is " + noContacts
                                    + " the contactList is " + contactList);
                        }
                    } else {
                        Logger.e(TAG, "refreshBlockList the view is null");
                    }
                }
            });
        } else {
            Logger.e(TAG, "refreshBlockList() mAdapter is null");
        }
    }
    
    /**
     * This class defined to display a dialog to achieve unblocking operation
     */
    public class UnBlockingDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "UnBlockingDialog";

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            AlertDialog dialog = new AlertDialog.Builder(this.getActivity()).create();
            dialog.setIconAttribute(android.R.attr.alertDialogIcon);
            dialog.setTitle(getString(R.string.oneonechat_unblock_title, mDisplayName));
            dialog.setMessage(getString(R.string.oneonechat_unblock_message, mDisplayName));
            dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return dialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                new UnBlockServiceTask().execute();
            } else if (which == DialogInterface.BUTTON_NEGATIVE) {
                dismissAllowingStateLoss();
            } else {
                Logger.v(TAG, "onClick(),window is null");
            }
        }
    }

    private class UnBlockServiceTask extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... params) {
            if (mNumber != null) {
                ContactsManager.getInstance().setImBlockedForContact(mNumber, false);
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            Logger.d(TAG, "onPostExecute() mContact is" + mContact);
            mAdapter.getContactsList().remove(mContact);
            mAdapter.notifyDataSetChanged();
            mUnBlockingDialog.dismissAllowingStateLoss();
        }
    }
}
