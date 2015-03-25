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
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Message;
import android.os.Parcelable;
import android.util.SparseBooleanArray;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.rcse.activities.ContactsAdapter.CapabilityHandler;
import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * A fragment act as contacts list fragment.
 */
public class ContactsListFragment extends BaseListFragment<ContactsAdapter> {
    private static final String TAG = "ContactsListFragment";
    private static final int SUB_STRING = 1;

    // Current ModeCallback instance
    private ModeCallback mModeCallback = null;
    private int mPosition = 0;

    // The flag to define is from select activity
    public boolean mIsFromSelectContactsActivity = false;

    // The flag to define is used for file transfer or IM chat, this flag is
    // valid while the mIsFromSelectContactsActivity is true
    private boolean mIsUsedForFt = false;

    // The flag to identify is ItemClick when enter onDestroyActionMode
    public boolean mIsItemClick = false;

    // Indicate loading contacts has been finished.
    private volatile boolean mIsLoadFinished = false;

    /**
     * Set whether this fragment is used for file transfer
     * 
     * @param flag True for file transfer, otherwise used for IM
     */
    public void setUsedForFileTransfer(boolean flag) {
        mIsUsedForFt = flag;
    }

    /**
     * Set the existing contacts by participants
     * 
     * @param participantList The participant list from chat fragment
     */
    public void setExistingContacts(ArrayList<Participant> participantList) {
        Logger.d(TAG, "setExistingContacts entry");
        if (participantList != null) {
            for (Participant participant : participantList) {
                RcsContact contact = new RcsContact(participant.getDisplayName(), participant
                        .getContact());
                mExistingContacts.add(contact);
            }
            Logger.d(TAG, "setExistingContacts the mExitstingContacts size is "
                    + mExistingContacts.size());
        } else {
            Logger.e(TAG, "setExistingContacts the participantlist is null");
        }
        Logger.d(TAG, "setExistingContacts exit");
    }

    /**
     * Set whether need to include the original contacts when creating a group chat.
     * @param isNeed
     */
    public void setIsNeedOriginalContacts(boolean isNeed) {
        Logger.d(TAG, "setIsNeedOriginalContacts() isNeed: " + isNeed);
        mIsNeedOriginalContacts = isNeed;
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mView = inflateView(inflater, container);
        mListView = (ListView) mView.findViewById(R.id.contacts_list);
        if (mListView == null) {
            throw new RuntimeException("Your content must have a ListView whose id attribute is "
                    + "'android.R.id.list'");
        }
        mListView.setOnItemClickListener(this);
        mListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);
        if (mModeCallback != null) {
            mModeCallback = null;
        }
        mModeCallback = new ModeCallback();
        mListView.setMultiChoiceModeListener(mModeCallback);

        mAdapter = createListAdapter();
        mListView.setAdapter(mAdapter);
        mListView.setOnScrollListener(mAdapter);

        // if from select contact activity to start action mode directly
        if (mIsFromSelectContactsActivity) {
            getActivity().startActionMode(mModeCallback);
        }
        if (mView != null) {
            View noContacts = mView.findViewById(R.id.no_contacts);
            View contactList = mView.findViewById(R.id.contacts_list);
            if (noContacts != null && contactList != null) {
                if (ContactsListManager.getInstance().CONTACTS_LIST.size() == 0) {
                    noContacts.setVisibility(View.VISIBLE);
                    contactList.setVisibility(View.GONE);
                } else {
                    //ContactsListManager.getInstance().removeBlockContacts();
                    List<RcsContact> currentList = new ArrayList<RcsContact>(
ContactsListManager.getInstance().CONTACTS_LIST);
                    currentList.removeAll(mExistingContacts);
                    if (currentList.size() == 0) {
                        noContacts.setVisibility(View.VISIBLE);
                        contactList.setVisibility(View.GONE);
                    } else {
                        noContacts.setVisibility(View.GONE);
                        contactList.setVisibility(View.VISIBLE);
                        mAdapter.setContactsList(currentList);
                        mAdapter.notifyDataSetChanged();
                        Logger.d(TAG, "onCreateView()-The CONTACTS_LIST is not empty "+ContactsListManager.getInstance().CONTACTS_LIST);
                    }
                }
            } else {
                Logger.e(TAG, "onCreateView() the noContacts is " + noContacts
                        + " the contactList is " + contactList);
            }
        } else {
            Logger.e(TAG, "onCreateView the view is null ");
        }
        return mView;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        Logger.v(TAG, "ContactsListFragment onDetach()");
    }

    /**
     * A MultiChoiceModeListener receives events for
     * {@link AbsListView#CHOICE_MODE_MULTIPLE_MODAL}, and also receives events
     * when the user selects and deselects list items.
     */
    private class ModeCallback implements ListView.MultiChoiceModeListener {
        private View mListViewContainer = null;
        private TextView mCheckedCountView = null;
        private LinearLayout mImageScrollView = null;
        private Map<Integer, View> mAvatarsMap = new HashMap<Integer, View>();

        private boolean mIsSelectAll = false;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            Logger.d(TAG, "onCreateActionMode entry");
            MenuInflater inflater = getActivity().getMenuInflater();
            if (mIsFromSelectContactsActivity) {
                if (mIsUsedForFt) {
                    inflater.inflate(R.menu.contacts_ft_menu, menu);
                } else {
                    inflater.inflate(R.menu.contacts_select_menu, menu);
                }
            } else {
                inflater.inflate(R.menu.contacts_menu, menu);
            }
            mListViewContainer = LayoutInflater.from(getActivity().getApplicationContext())
                    .inflate(R.layout.actionbar_select, null);
            mCheckedCountView = ((TextView) mListViewContainer.findViewById(R.id.number_selected));
            mImageScrollView = ((LinearLayout) mListViewContainer
                    .findViewById(R.id.selected_contact_container));
            mode.setCustomView(mListViewContainer);
            Logger.d(TAG, "onCreateActionMode exit, mIsSelectMode =" + mIsSelectMode);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            CapabilityHandler.getInstance().cancel();
            return false;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
                case R.id.menu_item_add_all:
                    if (mAdapter == null) {
                        Logger.d(TAG, "onActionItemClicked mAdapter is null");
                        break;
                    }
                    ListView listview = getListView();
                    final SparseBooleanArray array = listview.getCheckedItemPositions();
                    final int allCount = listview.getCount();
                    if (!mIsSelectAll) {
                        mIsSelectAll = !mIsSelectAll;
                        if (null == array) {
                            Logger.w(TAG, "onActionItemClicked() invalid array: null");
                            break;
                        }
                        for (int i = 0; i < allCount; i++) {
                            if (!array.get(i)) {
                                listview.setItemChecked(i, true);
                            }
                        }
                    } else {
                        mIsSelectAll = !mIsSelectAll;
                        mode.finish();
                    }
                    break;
                case R.id.menu_item_to_chat:
                    Logger.d(TAG, "onActionItemClicked() menu_item_to_chat");
                    mExistingContacts.clear();
                    startChat();
                    if(Logger.getIsIntegrationMode())
                    finishActionMode(mode);
                    else                    	
                    	mode.finish();
                    break;
                case R.id.menu_item_to_chat_select:
                case R.id.menu_item_to_ft_select:
                    Logger.d(TAG, "onActionItemClicked() menu_item_to_chat_select / menu_item_to_ft_select");
                    startChat();
                    if(Logger.getIsIntegrationMode())
                    finishActionMode(mode);
                    else                    	
                    	mode.finish();
                    break;
                default:
                    break;
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            Logger.d(TAG, "onDestroyActionMode entry");
            if (mAdapter == null) {
                Logger.d(TAG, "onDestroyActionMode mAdapter is null");
                return;
            }
            mAvatarsMap.clear();
            mAdapter.clear();
            if (mIsFromSelectContactsActivity) {
                // directly exit the activity
                if (!mIsItemClick || mIsSelectMode) {
                    getActivity().finish();
                }
            }
            mIsSelectMode = false;
            Logger.d(TAG, "onDestroyActionMode exit");
        }

        @Override
        public void onItemCheckedStateChanged(ActionMode mode, int position, long id,
                boolean checked) {
            Logger.d(TAG, "onItemCheckedStateChanged entry");
            RcsContact contact = (RcsContact) mAdapter.getItem(position);
            // filter out section index item
            if (contact != null && contact.mNumber != null && mImageScrollView != null) {
                mIsSelectMode = true;
                int checkedCount = 0;

                contact.mSelected = !contact.mSelected;
                final View rootView = LayoutInflater.from(mImageScrollView.getContext()).inflate(
                        R.layout.action_show_header, null, true);
                AsyncImageView avatar = (AsyncImageView) rootView.findViewById(R.id.icon);
                avatar.setAsyncContact(contact.mNumber);
                TextView textView = (TextView) rootView.findViewById(R.id.name);
                textView.setText(contact.mDisplayName);
                if (checked) {
                    mImageScrollView.addView(rootView, 0);
                    mAvatarsMap.put(position, rootView);
                } else {
                    View viewRemove = mAvatarsMap.get((Integer.valueOf(position)));
                    if (viewRemove != null) {
                        mImageScrollView.removeView(viewRemove);
                        mAvatarsMap.remove(position);
                    }
                }

                checkedCount = mImageScrollView.getChildCount();
                if (checkedCount == 0) {
                    mode.finish();
                }

                mCheckedCountView.setText(Integer.toString(checkedCount));
                mAdapter.notifyDataSetChanged();
            } else {
                if (mListView.getCheckedItemCount() == 1) {
                    mode.finish();
                    Logger.d(TAG, "onItemCheckedStateChanged, the first select is selection item, "
                            + "so finish the action mode!");
                } else {
                    Logger.d(TAG, "onItemCheckedStateChanged, not to finish the action mode!");
                }
                Logger.d(TAG, "onItemCheckedStateChanged contact is null");
            }
            Participant participant = null;
            if (contact.mNumber != null && contact.mNumber.startsWith(IPTEL_VITUAL_NUMBER)) {
                //Logger.v(TAG, "iptel account");
            	participant = new Participant(contact.mDisplayName, contact.mDisplayName);
            } else {
                Logger.v(TAG, "non-iptel account");
                participant = new Participant(contact.mNumber, contact.mDisplayName);
            }
            List<String> blockList = ContactsManager.getInstance().getImBlockedContactsFromLocal();
            if(checked == true && blockList.contains(participant.getContact()))
            {
	            mPosition = position;
	            UnBlockWarningDialog blockDialog = new UnBlockWarningDialog();
	            Bundle arguments = new Bundle();
	            //arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) tag);
	           
	            arguments.putParcelable(Utils.CHAT_PARTICIPANT, (Parcelable) participant);
	            //arguments.putParcelable("position",  new Integer(position));
	            blockDialog.setArguments(arguments);
	            blockDialog.show(getFragmentManager(), UnBlockWarningDialog.TAG);
            }
            Logger.d(TAG, "onItemCheckedStateChanged exit");
        }
    }

	/**
	 * 
	 * This class defined to display a dialog to achieve unblocking operation
	 */

	protected class UnBlockWarningDialog extends DialogFragment implements
			DialogInterface.OnClickListener {
		private static final String TAG = "BlockContactDialog";
		private Participant mParticipant = null;
		//private Object mChatTag = null;

		/**
		 * 
		 * Constructor
		 */
		public UnBlockWarningDialog() {

		}
		@Override
		public void onCancel(DialogInterface dialog) {

			super.onCancel(dialog);

		}
		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState) {

			//mChatTag = getArguments().getParcelable(Utils.CHAT_TAG);

			mParticipant = getArguments().getParcelable(Utils.CHAT_PARTICIPANT);

			AlertDialog dialog = new AlertDialog.Builder(this.getActivity())
					.create();
			String displayName = ContactsListManager.getInstance()
					.getDisplayNameByPhoneNumber(

					mParticipant.getContact());
			dialog.setIconAttribute(android.R.attr.alertDialogIcon);
			dialog.setTitle(getString(R.string.oneonechat_unblock_title,
					displayName));
			dialog.setMessage(getString(R.string.oneonechat_unblock_message,
					displayName));
			dialog.setButton(DialogInterface.BUTTON_POSITIVE,
			"Unblock", this);
			dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
			"Cancel", this);
			return dialog;
		}

		@Override
		public void onClick(DialogInterface dialog, int which) {
			if (which == DialogInterface.BUTTON_POSITIVE) {
				new AsyncTask<Object, Void, Object>() {

					@Override
					protected Object doInBackground(Object... params) {
						String contactNum = mParticipant.getContact();
						Logger.d(TAG,
								"context_menu_item_to_block the block number is: "

								+ contactNum);

						if (contactNum != null) {
							ContactsManager.getInstance()
									.setImBlockedForContact(contactNum, false);
						}
						return null;
					}
				}.execute();
				
				dismissAllowingStateLoss();
			} else if (which == DialogInterface.BUTTON_NEGATIVE) {
				//Do not unblock the contact, do not select the contact
				mIsItemClick = true;
				mListView.setItemChecked(mPosition, false);
				
				dismissAllowingStateLoss();
			} else {
				Logger.v(TAG, "window is null");
			}
		}
	}

    
    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long arg3) {
        Logger.d(TAG, "onItemClick() position is " + position);
        if (mIsFromSelectContactsActivity) {
            Logger.d(TAG, "onItemClick() position is " + position
                    + " mIsFromSelectContactsActivity is true");
            // if from select contact fragment, set mIsItemClick is true
            mIsItemClick = true;
            mListView.setItemChecked(position, true);
        } 
    }

    private void startOne2OneChat(Participant participant) {
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(participant);
        Intent chat = new Intent();
        chat.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        chat.putExtra(KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        chat.setClass(mContext, ChatScreenActivity.class);
        chat.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(chat);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long arg3) {
        return false;
    }

    @Override
    protected View inflateView(LayoutInflater inflater, ViewGroup container) {
        return inflater.inflate(R.layout.contacts_fragment, null);
    }

    @Override
    protected ContactsAdapter createListAdapter() {
        ContactsAdapter dapter = new ContactsAdapter(mContext, this);
        return dapter;
    }

    public void startSelectMode(String tag) {
        mTagName = tag;
        mIsFromSelectContactsActivity = true;
    }

    /**
     * Get the status of load contacts. The method just used for test case.
     * 
     * @return True if contacts load is finished, otherwise return false.
     */
    public boolean getLoadFinished() {
        return mIsLoadFinished;
    }
    
}
