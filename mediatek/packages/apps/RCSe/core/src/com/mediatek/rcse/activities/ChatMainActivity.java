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

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.os.Message;
import android.os.ParcelUuid;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatListProvider;

import com.orangelabs.rcs.R;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

/**
 * Main layout of the application.
 */
public class ChatMainActivity extends Activity implements OnClickListener {
    public static final String KEY_ADD_CONTACTS = "addContacts";

    public static final String VALUE_ADD_CONTACTS = "fromChatMainActivity";
    
    public static final String ACTION_START_CONTACT = "android.intent.action.contacts.list.PICKMULTIDATAS";
    
    public static final String PICK_RESULT = "com.mediatek.contacts.list.pickdataresult";
    
    public static final String RESTRICT_LIST = "restrictlist";
    
    public static final String INTENT_TYPE = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";

    private static final String TAG = "ChatMainActivity";
    private ChatListFragment mChatListFragment = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        createViewsAndFragments(savedInstanceState);
        ArrayList<ChatListProvider> chatListFromDatabase = new ArrayList<ChatListProvider>();
        chatListFromDatabase = ModelImpl.getInstance().getChatListHistory(this);
        // Reversed to update chat list on recent chats sequence
        Collections.reverse(chatListFromDatabase);
        int chatListSize = chatListFromDatabase.size(); 
        if(chatListSize > 10)
            chatListSize = 10;
        mChatListFragment.setChatListHistoryFromDatabase(chatListFromDatabase);
        for (int i = 0; i < chatListSize; i++) {           
            Logger.v(TAG, "message IDs"
                    + chatListFromDatabase.get(i).getMessageId());
                loadChatList(chatListFromDatabase.get(i),chatListFromDatabase.get(i).getChatID());
      
            }                   
        }      
   
    
    void clearChatsFromMemory()
    {
	    /* remove from UI */
	    mChatListFragment.getChatListView().getChatContainer().removeAllViews();
		mChatListFragment.getChatListView().getInviteContainer().removeAllViews();
	    /* clear model , but database will still have the chats*/
		
	       ControllerImpl controller = ControllerImpl.getInstance();
	        Message controllerMessage = controller.obtainMessage(
	                ChatController.EVENT_CLOSE_ALL_WINDOW_FROM_MEMORY, null,null);
	    
    
    }
    @Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		/* TODO clear old stuff for bug */
		clearChatsFromMemory();
	}


    /**
     * It generates a new tag for the group chat window to be created first time
     * on restart
     */
    protected ParcelUuid newChatTag() {
        ParcelUuid parcelUuid = null;
        UUID uuid = UUID.randomUUID();
        parcelUuid = new ParcelUuid(uuid);
        return parcelUuid;
    }

    /**
     * Send event to controller for every opened chat, after reboot
     */
    public void loadChatList(ChatListProvider chatListItem, String chatId) {        
        ParcelUuid parcelUuid = null;        
        if(chatListItem.getParticipantlist().size()>1)
                parcelUuid = newChatTag();
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, parcelUuid,
                chatListItem.getParticipantlist());
        Bundle bundle = new Bundle();
        bundle.putString("chatId", chatId);
        controllerMessage.setData(bundle);
        controllerMessage.sendToTarget();    
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    private void createViewsAndFragments(Bundle savedState) {
        Logger.v(TAG, "createViewsAndFragments entry");
        setContentView(R.layout.chat_main_ics);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_CUSTOM);
        LayoutInflater inflater = LayoutInflater.from(getApplicationContext());
        ViewGroup view = (ViewGroup) inflater.inflate(R.layout.chatmain_actionbar_layout, null);
        ImageView addContactView = (ImageView) view.findViewById(R.id.add_contacts);
        if (addContactView != null) {
            addContactView.setOnClickListener(ChatMainActivity.this);
        } else {
            Logger.v(TAG, "addContactsView is null.");
        }
        TextView textView = (TextView) view.findViewById(R.id.text_title);
        textView.setText(R.string.text_chats);
        actionBar.setCustomView(view);
        final FragmentManager fragmentManager = getFragmentManager();
        final FragmentTransaction transaction = fragmentManager.beginTransaction();
        if (mChatListFragment == null) {
            mChatListFragment = new ChatListFragment();
            transaction.add(android.R.id.content, mChatListFragment);
        }
        transaction.commitAllowingStateLoss();
        fragmentManager.executePendingTransactions();
        Logger.v(TAG, "createViewsAndFragments exit");
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (this.getApplicationInfo().targetSdkVersion >= Build.VERSION_CODES.ECLAIR) {
            event.startTracking();
        } else {
            moveTaskToBack(false);
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (this.getApplicationInfo().targetSdkVersion >= Build.VERSION_CODES.ECLAIR) {
            if (KeyEvent.KEYCODE_BACK == keyCode && event.isTracking() && !event.isCanceled()) {
                if (moveTaskToBack(false)) {
                    Logger.d(TAG, "onKeyUp() Back key up, move task to back successfully");
                    return true;
                } else {
                    Logger.w(TAG, "onKeyUp() Back key up, move task to back failed, destroy it");
                    return super.onKeyUp(keyCode, event);
                }
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        Logger.v(TAG, "onPrepareOptionsMenu() entry");
        super.onPrepareOptionsMenu(menu);
        menu.clear();
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.actions, menu);
        List list = ModelImpl.getInstance().listAllChat();
        if (list != null && list.size() == 0) {
            menu.removeItem(R.id.menu_close_all);
        } else {
            Logger.d(TAG, "onPrepareOptionsMenu(), show close all item!");
        }
        return true;
    }

    public static class CloseAllDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "CloseAllDialog";

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(getActivity()).create();
            alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            alertDialog.setTitle(R.string.text_close_all_chat);
            alertDialog.setMessage(getString(R.string.text_close_all_chat_message));
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE, getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE, getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                ControllerImpl controllerImpl = ControllerImpl.getInstance();
                if (controllerImpl != null) {
                    Message controllerMessage =
                            controllerImpl.obtainMessage(ChatController.EVENT_CLOSE_ALL_WINDOW, null, null);
                    controllerMessage.sendToTarget();
                } else {
                    Logger.e(TAG, "onClick(), controllerImpl is null!");
                }
            } else {
                Logger.i(TAG, "onClick(), the user cancle close all action");
            }
            this.dismissAllowingStateLoss();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Logger.v(TAG, "onOptionsItemSelected() entry");
        super.onOptionsItemSelected(item);
        if ((item.getItemId() == R.id.menu_more_settings)) {
            final Intent intent = new Intent(this, SettingsActivity.class);
            startActivity(intent);
        } else if (item.getItemId() == R.id.menu_close_all) {
            CloseAllDialog closeAllDialog = new CloseAllDialog();
            closeAllDialog.show(getFragmentManager(), ChatMainActivity.TAG);
        } else {
            Logger.w(TAG, "onOptionsItemSelected(), unknown item id");
        }
        return true;
    }

    public void onClick(View v) {
        if (v.getId() == R.id.add_contacts) {
            addContacts();
        }
    }

    private void addContacts() {
        Logger.d(TAG, "addContacts() enty");
        if (ContactsListManager.IS_SUPPORT) {
            Intent intent = new Intent(ACTION_START_CONTACT);
            intent.setType(INTENT_TYPE);
            long[] phoneIdList = ContactsListManager.getInstance().getPhoneIdTobeShow(null);
            intent.putExtra(RESTRICT_LIST, phoneIdList);
            startActivityForResult(intent, ChatFragment.RESULT_CODE_ADD_CONTACTS);
        } else {
            Intent intent = new Intent();
            intent.putExtra(ChatMainActivity.KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
            intent.setClass(this, SelectContactsActivity.class);
            startActivity(intent);
        }
    }
    
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Logger.d(TAG, "onActivityResult() entry");
        if (data == null) {
            Logger.d(TAG, "onActivityResult() the data is null ");
        } else {
            ArrayList<Participant> participants = ContactsListManager.getInstance().parseParticipantsFromIntent(data);
            Logger.d(TAG, "onActivityResult() participants is " + participants);
            if (participants != null && !participants.isEmpty()) {
                Intent chat = new Intent();
                chat.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
                chat.putExtra(KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
                chat.setClass(this, ChatScreenActivity.class);
                chat.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(chat);
            } else {
                Logger.d(TAG, "onActivityResult() the participants size is 0 ");
            }
        }
    }
}
