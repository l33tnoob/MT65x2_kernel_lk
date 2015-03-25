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

import android.app.Activity;
import android.app.Fragment;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ListView;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.mms.ipmessage.IpMessageConsts;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * A fragment act as a super class for other contacts and blacklist fragment.
 */
public abstract class BaseListFragment<T extends ContactBaseAdapter> extends Fragment implements
        OnItemClickListener, OnItemLongClickListener {
    public static final String TAG = "BaseListFragment";
    public static final int RESULT_CODE_CANCEL = 0;
    public static final int RESULT_CODE_ONE_TO_ONE = 1;
    public static final int RESULT_CODE_BLOCK = 2;
    public static final String KEY_ADD_CONTACTS = "addContacts";
    public static final String IPTEL_VITUAL_NUMBER = "+++88";
    protected Context mContext = null;
    protected T mAdapter;
    protected View mView;
    protected ListView mListView;
    boolean mIsSelectMode = false;
    protected String mTagName = ChatMainActivity.VALUE_ADD_CONTACTS;
    public static final int NO_PARTICIPANT_SIZE = 0;
    public static final int ONE_PARTICIPANT_SIZE = 1;
    // store participants list from chat fragment
    protected final List<RcsContact> mExistingContacts = new CopyOnWriteArrayList<RcsContact>();
    protected boolean mIsNeedOriginalContacts = false;
    public boolean FINISH_ACTIONMODE = true;

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        setContext(activity);
    }

    public void setContext(Context context) {
        mContext = context;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        setHasOptionsMenu(true);
        onCreateView(inflater, container);
        mAdapter = createListAdapter();
        mListView.setAdapter(mAdapter);
        return mView;
    }

    protected void onCreateView(LayoutInflater inflater, ViewGroup container) {
        setHasOptionsMenu(true);
        mView = inflateView(inflater, container);

        mListView = (ListView) mView.findViewById(R.id.contacts_list);
        if (mListView == null) {
            throw new RuntimeException("Your content must have a ListView whose id attribute is "
                    + "'android.R.id.list'");
        }
        mListView.setOnItemClickListener(this);
        mListView.setOnItemLongClickListener(this);
    }

    protected abstract View inflateView(LayoutInflater inflater, ViewGroup container);

    public T getAdapter() {
        return mAdapter;
    }

    @Override
    public View getView() {
        return mView;
    }

    public ListView getListView() {
        return mListView;
    }

    protected abstract T createListAdapter();

    protected void startChat() {
        Logger.d(TAG, "startChat()");
        if (mAdapter != null) {
            List<RcsContact> contacts = mAdapter.getContactsList();
            getSelectContactsFromListview(contacts);
        }
    }

    /**
     * It is just for TESTCASE:SelectContactsActivity.
     */
    private Intent mResult = null;

    /**
     * It is just for TESTCASE:SelectContactsActivity, get the intent generated
     * when tap the add contacts menu.
     * 
     * @return the intent generated when tap the add contacts menu.
     */
    public Intent getResult() {
        return mResult;
    }

    private void getSelectContactsFromListview(List<RcsContact> contacts) {
        Logger.d(TAG, "getSelectContactsFromListview(), contacts = " + contacts);
        if (contacts != null) {
            checkContacts(contacts);
        }
    }

    public void finishActionMode(ActionMode actionmode)
    {
    	Logger.d(TAG, "showWarningForLessParticipants Entry");
    	if(FINISH_ACTIONMODE)
    		actionmode.finish();
    }
    
    public void showWarningForLessParticipants(boolean showWarning)
    {
    	Logger.d(TAG, "showWarningForLessParticipants Entry");
    	Toast.makeText(getActivity(), "Please select more than one contacts to start group chat", Toast.LENGTH_LONG).show();
    }

    private void checkContacts(List<RcsContact> contacts) {
        Logger.d(TAG, "checkContacts()");
        ArrayList<Participant> participants = removeContactsExceed(contacts);
        Intent chat = new Intent();
        chat.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        int size = participants.size();
        if (size != NO_PARTICIPANT_SIZE) {
            if (mTagName == null) {
                Logger.v(TAG, "checkContacts() mTagName is null, that is add contacts");
                addContactsToExistGroupChat(chat);
                FINISH_ACTIONMODE = true;
            } else {
                Logger.v(TAG,
                        "checkContacts() mTagName is not null, that create a new chat window.");
                if (size == ONE_PARTICIPANT_SIZE && Logger.getIsIntegrationMode()) {
                    String number = participants.get(NO_PARTICIPANT_SIZE).getContact();
                    Logger.v(TAG, "checkContacts() the number is:" + number);
                    Intent intent = new Intent();
                    if(RcsSettings.getInstance().getMessagingUx() == 0) 
                    {        
                    number = IpMessageConsts.JOYN_START + participants.get(NO_PARTICIPANT_SIZE).getContact();           
                    intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
                    }
                    intent.setAction(Intent.ACTION_SENDTO);
                    Uri uri = Uri.parse(PluginProxyActivity.MMSTO + number);
                    
                    intent.setData(uri);                  
                    if(RcsSettings.getInstance().getMessagingUx() == 1)
                    {
                    FINISH_ACTIONMODE = false;
                    showWarningForLessParticipants(true);
                    }
                    else 
                    {                    	
                    	FINISH_ACTIONMODE = true;
                    	startActivity(intent);
                    }
                    
                } else {
                    createGroupChatWindow(chat, size);
                    FINISH_ACTIONMODE = true;
                }
            }
        }
        else
        	FINISH_ACTIONMODE = true;
    }

    /**
     * For example add contacts to group chat: mExistingContacts is [a,b,c,d]
     * contacts's selected items are [e,f,g,h,i,j,k,l] maxParticipants = 10 Then
     * should only add [e,f,g,h,i,j]. For example create a new group chat:
     * Remove the contacts select exceed max number of a group chat allowed
     * contacts's selected items are [e,f,g,h,i,j,k,l,m,n,o,p,q] maxParticipants
     * = 10. Then should only add [e,f,g,h,i,j,k,l,m,n]
     * 
     * @param contacts The contacts list to be checked.
     * @return The contacts which will be added to group chat
     */
    private ArrayList<Participant> removeContactsExceed(List<RcsContact> contacts) {
        int maxParticipants = RcsSettings.getInstance().getMaxChatParticipants() - 1;
        int contactsSize = contacts.size();
        ArrayList<Participant> participants = new ArrayList<Participant>();
        int existContactsNum = mExistingContacts == null ? 0 : mExistingContacts.size();
        if (existContactsNum > 0 && mIsNeedOriginalContacts) {
            for (RcsContact existingContact : mExistingContacts) {
                participants.add(new Participant(existingContact.mNumber, existingContact.mNumber));
            }
            Logger.d(TAG, "removeContactsExceed() add existing contact and participants is : " + participants);
        }
        int i = 0;
        for (i = 0; i < contactsSize; i++) {
            RcsContact contact = contacts.get(i);
            if (contact.mSelected) {
                Participant sentPart = null;
                if (contact.mNumber != null && contact.mNumber.startsWith(IPTEL_VITUAL_NUMBER)) {
                    Logger.v(TAG, "iptel account");
                    sentPart = new Participant(contact.mDisplayName, contact.mDisplayName);
                } else {
                    Logger.v(TAG, "non-iptel account");
                    sentPart = new Participant(contact.mNumber, contact.mDisplayName);
                }
                if ((participants.size() + existContactsNum) >= maxParticipants) {
                    Logger.d(TAG, "Create a new group chat window. The select num is exceed.");
                  showToast(mContext, mContext.getString(com.orangelabs.rcs.R.string.cannot_add_members_over,
                            maxParticipants));
                    for (; i < contactsSize; ++i) {
                        contact.mSelected = false;  
                }               
                    break;
                }
                participants.add(sentPart);
            }
        }        
        return participants;
    }

    private void addContactsToExistGroupChat(Intent intent) {
        Logger.d(TAG, "addContactsToExistGroupChat()");
        Activity activity = getActivity();
        Logger.w(TAG, "activity is " + activity);
        if (activity != null) {
            mResult = intent;
            activity.setResult(Activity.RESULT_OK, intent);
        }
    }

    private void createGroupChatWindow(Intent intent, int size) {
        Logger.d(TAG, "createGroupChatWindow()");
        intent.putExtra(KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        if (Logger.getIsIntegrationMode() && size > ONE_PARTICIPANT_SIZE) {
            intent.setAction(PluginGroupChatActivity.ACTION);
        } else {
            intent.setClass(mContext, ChatScreenActivity.class);
        }
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    private void showToast(Context context, String message) {
        Logger.v(TAG, "showToast() entry, context = " + context + ", message = " + message);
        if (context != null) {
            Toast toast = Toast.makeText(context, message, Toast.LENGTH_SHORT);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
        }
        Logger.v(TAG, "showToast() exit");
    }
}
