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

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;
import android.widget.CheckBox;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.mediatek.rcse.activities.widgets.ChatScreenWindow;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.activities.widgets.UnreadMessagesContainer;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.binder.TagTranslater;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The activity is a container which contains many chat fragments.
 */
public class ChatScreenActivity extends Activity {
    public static final String TAG = "ChatScreenActivity";
    public static final String KEY_CHAT_TAG = "tag";
    public static final String KEY_ADD_CONTACTS = "addContacts";
    public static final String KEY_EXSITING_PARTICIPANTS = "existParticipants";
    public static final String KEY_USED_CHATTAG = "usedChatTag";
    public static final int GROUP_NUMBER_ONE = 1;
    public static final int MAX_CHAT_MSG_LENGTH =
            RcsSettings.getInstance().getMaxChatMessageLength();
    public boolean newGroupChat = false;
    public boolean IsGroupInvite = false;
    public boolean fromOne2OneToGroupChat = false;
    protected final ChatWindowManager mChatWindowManager = new ChatWindowManager();
    private boolean mIsActivityBackground = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Logger.d(TAG, "onCreate entry");
        initialActionBarAndViews(savedInstanceState);
        Intent intent = getIntent();
        handleIntentAction(intent);
        Logger.d(TAG, "onCreate exit");
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        Logger.d(TAG, "onConfigurationChanged() enrty, the newConfig is " + newConfig);
        hideInputMethod();
        super.onConfigurationChanged(newConfig);
    }

    protected void handleIntentAction(Intent intent) {
        Logger.d(TAG, "handleIntentAction() enrty, the intent is " + intent);
        if (intent == null) {
            Logger.d(TAG, "handleIntentAction() enrty, the intent is null");
        } else {
            String action = intent.getAction();
            Logger.d(TAG, "handleIntentAction() the action is " + action);
            if (action == null) {
                handleIncomingIntent(intent);
            } else {
                if (action.matches(PluginApiManager.RcseAction.IM_ACTION)) {
                    String name = intent.getStringExtra(PluginApiManager.RcseAction.CONTACT_NAME);
                    String number = intent
                            .getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
                    Logger.d(TAG, "handleIntentAction()-The number is: " + number
                            + " ,the name is: " + name);
                    if (number != null) {
                        if (name == null) {
                            name = number;
                        }
                        Participant participant = new Participant(number, name);
                        ArrayList<Participant> list = new ArrayList<Participant>();
                        list.add(participant);
                        ChatImpl chat = (ChatImpl) ModelImpl.getInstance().addChat(list, null,null);
                        if (chat == null) {
                            Logger.d(TAG, "handleIntentAction()-current chat is null");
                            return;
                        } else {
                            ParcelUuid uuidTag = (ParcelUuid) chat.getChatTag();
                            if (uuidTag != null) {
                                ChatScreenWindowContainer.getInstance().focus(uuidTag);
                            } else {
                                Logger.d(TAG, "handleIntentAction()-current uuidTag is null");
                                return;
                            }
                            mChatWindowManager.connect(true);
                            ControllerImpl controller = ControllerImpl.getInstance();
                            Logger.d(TAG, "resume controller is: " + controller);
                            if (null != controller) {
                                Message controllerMessage =
                                        controller.obtainMessage(ChatController.EVENT_SHOW_WINDOW,
                                                chat.getChatTag(), null);
                                controllerMessage.sendToTarget();
                            }
                        }
                    }
                } else if (action.matches(PluginApiManager.RcseAction.FT_ACTION)) {
                	
                	boolean isGroup = intent.getBooleanExtra(Utils.IS_GROUP_CHAT, false);
                	List<Participant> participants = null;
                	if(isGroup == true)
                	{
                		Logger.d(TAG, "handleIntentAction()-group FT action");
                		//invitation for group FT
                		participants = intent.getParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST);
                		if (null == participants) {
	                        Logger.d(TAG, "handleIntentAction()-current participants list is null");
	                        return;
	                    }
                	}
                	else
                	{
                		Logger.d(TAG, "handleIntentAction()-one2one FT action");
                    String name = intent.getStringExtra(PluginApiManager.RcseAction.CONTACT_NAME);
                    String number =
                            intent.getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
                    if (null == number) {
                        Logger.d(TAG, "handleIntentAction()-current number is null");
                        return;
                    }
                    if (null == name) {
                        Logger.v(TAG, "handleIntentAction()-current name is nul");
                        name = number;
                    }
                    Participant participant = new Participant(number, name);
                    if (null == participant) {
                        Logger.d(TAG, "handleIntentAction()-current participant is null");
                        return;
                    }
	                    participants = new ArrayList<Participant>();
	                    participants.add(participant);
                	}
                    ArrayList<String> files =
                            intent
                                    .getStringArrayListExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI);
                    
                    if (null == files) {
                        Logger.d(TAG, "handleIntentAction()-current filePath is null");
                        return;
                    }
                    
                    ChatImpl chat = (ChatImpl) ModelImpl.getInstance().addChat(participants, null,null);
                    if (chat == null) {
                        Logger.d(TAG, "handleIntentAction()-current chat is null");
                        return;
                    } else {
                        ParcelUuid uuidTag = (ParcelUuid) chat.getChatTag();
                        ChatScreenWindowContainer.getInstance().focus(uuidTag);
                        mChatWindowManager.connect(true);
                        ControllerImpl controller = ControllerImpl.getInstance();
                        Logger.d(TAG, "handleIntentAction()-resume controller is: " + controller);
                        if (null != controller) {
                            Message controllerMessage =
                                    controller.obtainMessage(ChatController.EVENT_SHOW_WINDOW, chat
                                            .getChatTag(), null);
                            controllerMessage.sendToTarget();
                            int size = files.size();
                            for (int i = 0; i < size; i++) {
                                String file = files.get(i);
                                controllerMessage = null;
                                controllerMessage = controller.obtainMessage(
                                        ChatController.EVENT_FILE_TRANSFER_INVITATION,
                                        chat.getChatTag(), file);
                                boolean isSizeValidate = isFileSizeValidate(file, controllerMessage);
                                Logger.d(TAG, "handleIntentAction()- file size is validate: "
                                        + isSizeValidate);
                                if (isSizeValidate) {
                                    controllerMessage.sendToTarget();
                                }
                            }
                        }
                    }
                }
                else if (action.matches(PluginApiManager.RcseAction.SHARE_URL_ACTION))
                {
                    String name = intent.getStringExtra(PluginApiManager.RcseAction.CONTACT_NAME);
                    String number =
                            intent.getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
                    String url =
                            intent.getStringExtra(PluginApiManager.RcseAction.SHARE_URL);
                    if (null == number) {
                        Logger.d(TAG, "handleIntentAction()-current number is null");
                        return;
                    }
                    if (null == name) {
                        Logger.v(TAG, "handleIntentAction()-current name is nul");
                        name = number;
                    }                   
                    Participant participant = new Participant(number, name);
                    if (null == participant) {
                        Logger.d(TAG, "handleIntentAction()-current participant is null");
                        return;
                    }
                    ArrayList<Participant> list = new ArrayList<Participant>();
                    list.add(participant);
                    ChatImpl chat = (ChatImpl) ModelImpl.getInstance().addChat(list, null,null);
                    if (chat == null) {
                        Logger.d(TAG, "handleIntentAction()-current chat is null");
                        return;
                } else {
                        ParcelUuid uuidTag = (ParcelUuid) chat.getChatTag();
                        ChatScreenWindowContainer.getInstance().focus(uuidTag);
                        mChatWindowManager.connect(true);
                        ControllerImpl controller = ControllerImpl.getInstance();
                        if (null != controller) {
                            Message controllerMessage =
                                    controller.obtainMessage(ChatController.EVENT_SHOW_WINDOW, chat
                                            .getChatTag(), null);
                            controllerMessage.sendToTarget();                            
                                controllerMessage =
                                    controller.obtainMessage(
                                            ChatController.EVENT_SEND_MESSAGE,
                                            chat.getChatTag(), url);                               
                                    controllerMessage.sendToTarget();
                                
                            }                         
                    else {
                            Logger.e(TAG, "handleIntentAction()-resume controller is null");
                        }                   
                 
                }
                } 
                else {
                    handleIncomingIntent(intent);
                }
            }
        }
    }

    public boolean isFileSizeValidate(String fileName, Message addInvitationMessage) {
        File file = new File(fileName);
        long fileSize = file.length();
        long maxFileSize = ApiManager.getInstance().getMaxSizeforFileThransfer();
        long warningFileSize = ApiManager.getInstance().getWarningSizeforFileThransfer();
        boolean shouldWarning = false;
        boolean shouldRepick = false;
        if (warningFileSize != 0 && fileSize >= warningFileSize) {
            shouldWarning = true;
        }
        if (maxFileSize != 0 && fileSize >= maxFileSize) {
            shouldRepick = true;
        }
        Logger.d(TAG, "isFileSizeValidate() maxFileSize: " + maxFileSize + " warningFileSize: "
                + warningFileSize + " shouldWarning: " + shouldWarning + " shouldRepick: "
                + shouldRepick);
        if (shouldRepick) {
            Toast.makeText(this, R.string.over_max_file_size, Toast.LENGTH_SHORT).show();
            return false;
        } else if (shouldWarning) {
            SharedPreferences prefer = PreferenceManager.getDefaultSharedPreferences(this);
            Boolean remind = prefer.getBoolean(SettingsFragment.RCS_REMIND, false);
            Logger.w(TAG, "isFileSizeValidate() remind: " + remind);
            if (remind) {
                WarningDialog warningDialog = new WarningDialog();
                Bundle arguments = new Bundle();
                arguments.putParcelable(Utils.MESSAGE, addInvitationMessage);
                warningDialog.setArguments(arguments);
                warningDialog.show(getFragmentManager(), ChatScreenActivity.TAG);
                return false;
            } else {
                return true;
            }
        } else {
            return true;
        }
    }

    public static class WarningDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "WarningDialog";
        private CheckBox mCheckRemind = null;
        private Activity mActivity = null;
        private Message mAddInvitationMessage;

        public WarningDialog() {

        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            mAddInvitationMessage = getArguments().getParcelable(Utils.MESSAGE);
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle(R.string.file_size_warning);
            Logger.d(TAG, "In WarningDialog,mActivity is: " + mActivity);
            if (mActivity != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity.getApplicationContext());
                View customView = inflater.inflate(R.layout.warning_dialog, null);
                mCheckRemind = (CheckBox) customView.findViewById(R.id.remind_notification);
                alertDialog.setView(customView);

            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mCheckRemind != null) {
                    boolean isCheck = mCheckRemind.isChecked();
                    Logger.w(TAG, "WarningDialog onClick ischeck" + isCheck);
                    SharedPreferences sPrefer =
                            PreferenceManager.getDefaultSharedPreferences(mActivity);
                    Editor remind = sPrefer.edit();
                    remind.putBoolean(SettingsFragment.RCS_REMIND, !isCheck);
                    remind.commit();
                    mAddInvitationMessage.sendToTarget();
                }
            } else {
                Logger.i(TAG, "the user cancle this file transfer invitation");
            }
            this.dismissAllowingStateLoss();
        }
    }

    private void initialActionBarAndViews(Bundle savedState) {
        Logger.v(TAG, "initialActionBar entry");
        setContentView(R.layout.chat_screen_ics);
        ActionBar bar = getActionBar();
        bar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
        bar.setDisplayOptions(0, ActionBar.DISPLAY_SHOW_TITLE);
        bar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        bar.setDisplayHomeAsUpEnabled(true);
        Logger.v(TAG, "initialActionBar exit");
    }

    @Override
    protected void onPause() {
        mChatWindowManager.onPause();
        mIsActivityBackground = true;
        super.onPause();
    }

    @Override
    protected void onResume() {
        Logger.v(TAG, "ChatScreenActivity onResume");
        super.onResume();
        mChatWindowManager.onResume();
        mIsActivityBackground = false;
    }

    @Override
    protected void onDestroy() {
        Logger.v(TAG, "ChatScreenActivity onDestroy");
        ViewImpl.getInstance().removeChatWindowManager(mChatWindowManager);
        ChatScreenWindowContainer.getInstance().clearCurrentStatus();
        super.onDestroy();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Logger.v(TAG, "ChatScreenActivity onNewIntent");
        super.onNewIntent(intent);
        ChatScreenWindowContainer.getInstance().clearCurrentStatus();
        handleIntentAction(intent);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        MenuInflater inflater = getMenuInflater();
        ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
        if (window instanceof OneOneChatWindow) {
            inflater.inflate(R.menu.one2one_chat_screen_action_bar_menu, menu);
        } else if (window instanceof GroupChatWindow) {
            if (Logger.getIsIntegrationMode()) {
                inflater.inflate(R.menu.group_chat_screen_action_bar_menu_integrate_mode, menu);
            } else {
                inflater.inflate(R.menu.group_chat_screen_action_bar_menu, menu);
            }
        } else {
            Logger.e(TAG, "onCreateOptionsMenu error");
            return false;
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
        switch (item.getItemId()) {
            case R.id.menu_add_contact:
                addContacts();
                break;
            case R.id.menu_clear_history:
                ClearHistoryDialog dialog = new ClearHistoryDialog();
                dialog.show(getFragmentManager(), ClearHistoryDialog.TAG);
                break;
            case R.id.menu_block:
                Logger.d(TAG, "onOptionsItemSelected(), when block ,the window is: " + window);
                if (window instanceof OneOneChatWindow) {
                    Object tag = ((OneOneChatWindow) window).getTag();
                    Participant participant = ((OneOneChatWindow) window).getParticipant();
                    BlockContactDialog blockDialog = new BlockContactDialog();
                    Bundle arguments = new Bundle();
                    arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) tag);
                    arguments.putParcelable(Utils.CHAT_PARTICIPANT, (Parcelable) participant);
                    blockDialog.setArguments(arguments);
                    blockDialog.show(getFragmentManager(), BlockContactDialog.TAG);
                    hideSoftKeyboard();
                }
                break;
            case R.id.menu_close_chat:
                Logger.d(TAG, "onOptionsItemSelected(), when close chat, the window is: " + window);
                if (window instanceof OneOneChatWindow) {
                    Object tag = ((OneOneChatWindow) window).getTag();
                    ConfirmOneOneCloseDialog closeDialog = new ConfirmOneOneCloseDialog();
                    Bundle arguments = new Bundle();
                    arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) tag);
                    closeDialog.setArguments(arguments);
                    closeDialog.show(getFragmentManager(), ConfirmOneOneCloseDialog.TAG);
                    hideSoftKeyboard();
                }
                break;
            case R.id.menu_group_chat_quit:
                quitGroupChat();
                hideSoftKeyboard();
                break;
            case android.R.id.home:
                Logger.d(TAG, "onOptionsItemSelected(), when press home, the window is: " + window);
                if (window != null) {
                    window.removeChatUi();
                    ChatScreenWindowContainer.getInstance().setFocusWindow(window);
                }
                onBackPressed();
                break;
            case R.id.menu_set_wallpaper:
                setWallpaper();
                break;
            default:
                break;
        }
        return true;
    }

    // Confirm dialog for closing 1-2-1 chat.
    public static class ConfirmOneOneCloseDialog extends DialogFragment {
        public static final String TAG = "ConfirmOneOneCloseDialog";

        public ConfirmOneOneCloseDialog() {

        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Object tag = getArguments().getParcelable(Utils.CHAT_TAG);
            return new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(getString(R.string.text_close_chat))
                    .setMessage(getString(R.string.text_close_chat))
                    .setPositiveButton(getString(R.string.rcs_dialog_positive_button),
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    ControllerImpl controller = ControllerImpl.getInstance();
                                    Logger.d(TAG, "ConfirmOneOneCloseDialog controller is: "
                                            + controller);
                                    if (null != controller) {
                                        Message controllerMessage = controller.obtainMessage(
                                                ChatController.EVENT_CLOSE_WINDOW, tag, null);
                                        controllerMessage.sendToTarget();
                                    }
                                    dismiss();
                                }
                            })
                    .setNegativeButton(getString(R.string.rcs_dialog_negative_button),
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    dismiss();
                                }
                            }).create();
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            Logger.d(TAG, "onKeyUp(), the key code is KeyEvent.KEYCODE_BACK");
            ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
            Logger.d(TAG, "onKeyUp(), the window is " + (window == null ? "null" : "not null"));
            if (window != null) {
                Object tag = window.getTag();
                Logger.d(TAG, "onKeyUp(), the tag is " + (tag == null ? "null" : "not null"));
                if (tag != null) {
                    ChatFragment fragment = null;
                    fragment = (ChatFragment) this.getFragmentManager().findFragmentByTag(
                            tag.toString());
                    Logger.d(TAG, "onKeyUp(), the fragment is "
                            + (fragment == null ? "null" : "not null"));
                    if (null != fragment) {
                        Logger.d(TAG, "onKeyUp(), the emotions is "
                                + fragment.getEmotionsVisibility());
                        if (fragment.getEmotionsVisibility() == View.VISIBLE) {
                            fragment.hideEmotions();
                            return true;
                        }
                    }
                }
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    private void hideSoftKeyboard() {
        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
        Logger.d(TAG, "hideSoftKeyboard(), inputMethodManager is: " + inputMethodManager);
        if (inputMethodManager != null) {
            View view = this.getCurrentFocus();
            Logger.d(TAG, "hideSoftKeyboard(), view is: " + view);
            if (view != null) {
                IBinder binder = view.getWindowToken();
                inputMethodManager.hideSoftInputFromWindow(binder,
                        InputMethodManager.HIDE_NOT_ALWAYS);
            }
        }
    }

    protected void handleIncomingIntent(Intent intent) {
        Logger.d(TAG, "handleIncomingIntent() entry with intent is " + intent);
        Bundle data = intent.getExtras();
        Logger.d(TAG, "handleIncomingIntent() ,the data is " + data);
        boolean isInvitation = checkInvitation(intent);
        if (isInvitation) {
            IsGroupInvite = true;
            boolean isGroup = intent.getBooleanExtra(Utils.IS_GROUP_CHAT, false);
            if (isGroup) {
                newChatTag(intent);
            }
            boolean result = ModelImpl.getInstance().handleInvitation(intent, false);
            if (result) {
                mChatWindowManager.connect(true);
            }
            Logger.d(TAG, "handleIncomingIntent(), isGroup: " + isGroup + " result: " + result);
        } else {
            if (data != null) {
                if (data.containsKey(ChatScreenActivity.KEY_CHAT_TAG)) {
                    // Open an exist chat window.
                    Logger.v(TAG, "handleIncomingIntent() Open an exist chat window");
                    focusOnChatByTag(data.get(ChatScreenActivity.KEY_CHAT_TAG));
                } else if (data.containsKey(ChatMainActivity.KEY_ADD_CONTACTS)
                        && data.containsKey(Participant.KEY_PARTICIPANT_LIST)) {
                    ParcelUuid usedChatTag = intent.getParcelableExtra(KEY_USED_CHATTAG);
                    Logger.v(TAG, "handleIncomingIntent() usedChatTag: " + usedChatTag);
                    if (usedChatTag == null) {
                        addChatWindow(intent);
                    } else {
                        focusOnChatByTag(usedChatTag);
                    }
                }
            }
        }
    }

    protected ParcelUuid newChatTag(Intent intent) {
        ParcelUuid parcelUuid = null;
        UUID uuid = UUID.randomUUID();
        parcelUuid = new ParcelUuid(uuid);
        intent.putExtra(KEY_CHAT_TAG, parcelUuid);
        intent.putExtra(KEY_USED_CHATTAG, parcelUuid);
        ChatScreenWindowContainer.getInstance().focus(parcelUuid);
        return parcelUuid;
    }

    /**
     * Add contacts to an exist chat window or create a new chat window.
     * 
     * @param intent The intent
     */
    protected void addChatWindow(Intent intent) {
        Bundle data = intent.getExtras();
        Logger.e(TAG, "addChatWindow() entry with the data is " + data);
        String hashTag = data.getString(ChatScreenActivity.KEY_ADD_CONTACTS);
        List<Participant> participantList = data
                .getParcelableArrayList(Participant.KEY_PARTICIPANT_LIST);
        int size = participantList.size();
        Logger.d(TAG, "addChatWindow(), participantList: " + participantList + " size: " + size
                + " ,the hashTag is: " + hashTag);
        if (ChatMainActivity.VALUE_ADD_CONTACTS.equals(hashTag)) {
            ParcelUuid parcelUuid = null;
            if (size > GROUP_NUMBER_ONE) {
                parcelUuid = newChatTag(intent);
                newGroupChat = true;
            } else {
                ChatScreenWindowContainer.getInstance().focus(participantList);
            }
            mChatWindowManager.connect(true);
            ControllerImpl controller = ControllerImpl.getInstance();
            Message controllerMessage = controller.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                    parcelUuid, participantList);
            controllerMessage.sendToTarget();
        }
        Logger.d(TAG, "addChatWindow() exit");
    }

    protected void focusOnChatByTag(Object tag) {
        Logger.d(TAG, "focusOnChatByTag() entry, tag: " + tag);
        if (tag instanceof String) {
            Logger.d(TAG, "focusOnChatByTag() it's a String and need to be translated first");
            Object chatTag = TagTranslater.translateTag((String) tag);
            if (!(chatTag instanceof ParcelUuid)) {
                Logger.w(TAG, "focusOnChatByTag() tag not found!");
                finish();
                return;
            }
            ChatScreenWindowContainer.getInstance().focus((ParcelUuid)chatTag);
        } else if (tag instanceof ParcelUuid) {
            Logger.d(TAG, "focusOnChatByTag() it's a ParcelUuid");
            ChatScreenWindowContainer.getInstance().focus((ParcelUuid)tag);
        } else {
            Logger.w(TAG, "focusOnChatByTag() unknown tag!");
            finish();
            return;
        }

        mChatWindowManager.connect(true);
    }

    protected final boolean checkInvitation(Intent intent) {
        // add window ui.
        final String extraSessionId = "sessionId";
        String sessionId = intent.getStringExtra(extraSessionId);
        if (TextUtils.isEmpty(sessionId)) {
            Logger.w(TAG, "checkInvitation() invalid sessionId: " + sessionId);
            return false;
        }
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.w(TAG, "checkInvitation() instance is null");
            return false;
        }
        MessagingApi messageApi = instance.getMessagingApi();
        if (messageApi == null) {
            Logger.w(TAG, "checkInvitation() messageApi is null");
            return false;
        }
        try {
            IChatSession chatSession = messageApi.getChatSession(sessionId);
            if (chatSession == null) {
                Logger.e(TAG, "checkInvitation() chatSession is null");
                return false;
            }
            List<String> participants = chatSession.getInivtedParticipants();

            Logger.d(TAG, "checkInvitation() sessionId: " + sessionId + ", Participant: "
                    + participants);
            int participantCount = participants.size();
            List<Participant> participantsList = new ArrayList<Participant>();
            final String extraContactName = "contactDisplayname";
            if (participantCount == 1) {
                String remoteParticipant = participants.get(0);
                String number = PhoneUtils.extractNumberFromUri(remoteParticipant);
                String name = intent.getStringExtra(extraContactName);
                if (name == null) {
                    name = number;
                }
                Participant fromSessionParticipant = new Participant(number, name);
                participantsList.add(fromSessionParticipant);
            } else if (participantCount > 1) {
                for (int i = 0; i < participantCount; i++) {
                    String remoteParticipant = participants.get(i);
                    String number = PhoneUtils.extractNumberFromUri(remoteParticipant);
                    String name = intent.getStringExtra(extraContactName);
                    if (name == null) {
                        name = number;
                    }
                    Participant fromSessionParticipant = new Participant(number, name);
                    participantsList.add(fromSessionParticipant);
                }
            }
            ChatScreenWindowContainer.getInstance().focus(participantsList);
            Logger.d(TAG, "checkInvitation() The intent is a invitation!");
        } catch (ClientApiException e) {
            Logger.e(TAG, "checkInvitation() getChatSession fail");
            e.printStackTrace();
            return false;
        } catch (RemoteException e) {
            Logger.e(TAG, "checkInvitation() getParticipants fail");
            e.printStackTrace();
            return false;
        }
        return true;
    }

    private void loadLatestUnreadMessage() {
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
            }
        });
    }

    @Override
    public void onBackPressed() {
        Logger.v(TAG, "onBackPressed()");
        if (isFinishing()) {
            Logger.w(TAG, "onBackPressed() mIsActivityDestoryed is true");
            return;
        } else {
            super.onBackPressed();
        }
    }

    /**
     * Get the chat window manager.
     * 
     * @return The chat window manager.
     */
    public ChatWindowManager getChatWindowManager() {
        return mChatWindowManager;
    }

    /**
     * This class manages all the chat information in ChatScreenActivity
     */
    public class ChatWindowManager implements ChatView.IChatWindowManager, OnClickListener {
        public static final String TAG = "ChatScreenActivity#ChatWindowManager";

        private boolean mIsConnect = false;

        /**
         * Try to register this manager to ViewImpl
         * @param isAutoShow True if it should automatically show all window, otherwise false
         */
        public void connect(boolean isAutoShow) {
            if (mIsConnect) {
                Logger.w(TAG, "connect() already connected, just disconnected first");
                disconnect();
            }
            Logger.d(TAG, "connect() connect to ViewImpl");
            ViewImpl.getInstance().addChatWindowManager(this, isAutoShow);
            mIsConnect = true;
        }

        /**
         * Try to unregister this manager from ViewImpl
         */
        public void disconnect() {
            Logger.d(TAG, "disconnect() disconnect from ViewImpl");
            ViewImpl.getInstance().removeChatWindowManager(this);
            mIsConnect = false;
        }

        @Override
        public IGroupChatWindow addGroupChatWindow(Object tag, List<ParticipantInfo> participantList) {
            Logger.v(TAG, "addGroupChatWindow() tag: " + tag + " participantList: "
                    + participantList);
            List<Participant> participantFromInfos = new ArrayList<Participant>();
            for (ParticipantInfo participant : participantList) {
                participantFromInfos.add(participant.getParticipant());
            }
            final GroupChatWindow groupChatWindow = new GroupChatWindow(tag,
                    (CopyOnWriteArrayList<ParticipantInfo>) participantList);
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            if (tag != null && tag.equals(instance.getCurrentTag())) {
                addGroupFragment((ParcelUuid) tag, participantFromInfos, groupChatWindow);
            } else {
                Logger.d(TAG, "addGroupChatWindow() chat is not focused, do not add UI");
            }
            return groupChatWindow;
        }

        @Override
        public IOne2OneChatWindow addOne2OneChatWindow(final Object tag,
                final Participant participant) {
            Logger.d(TAG, "addOne2OneChatWindow the tag is " + tag + " participant is: "
                    + participant);
            if (tag == null) {
                Logger.d(TAG, "addOne2OneChatWindow the tag is null");
                return null;
            }
            final OneOneChatWindow oneOneChatWindow = new OneOneChatWindow(tag, participant);
            List<Participant> participantList = new ArrayList<Participant>();
            participantList.add(participant);
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            if (tag.equals(instance.getCurrentTag())) {
                // enter ChatScreenaAitivity through chat list.
                addOneOneFragment((ParcelUuid) tag, participantList, oneOneChatWindow);
            } else {
                // enter ChatScreenaAitivity through contact list or invitation.
                List<Participant> participants = instance.getCurrentParticipants();
                if (participants == null) {
                    Logger.d(TAG, "addOne2OneChatWindow participants is null");
                    return oneOneChatWindow;
                } else if (participants.size() == 1 && participants.get(0).equals(participant)) {
                    Logger.d(TAG, "addOne2OneChatWindow() the cparticipants.size is 1");
                    if (null == instance.getCurrentTag()) {
                        Logger
                                .d(TAG,
                                        "addOne2OneChatWindow() the currentTag is null so add one2one fragment");
                        addOneOneFragment((ParcelUuid) tag, participantList, oneOneChatWindow);
                    } else {
                        Logger.d(TAG, "addOne2OneChatWindow() the currentTag is not null");
                    }
                } else if (participants.size() == 0) {
                    Logger.d(TAG, "addOne2OneChatWindow participants size is 0");
                    return oneOneChatWindow;
                }
            }
            Logger.d(TAG, "addOne2OneChatWindow sucess, the tag is: " + tag);
            return oneOneChatWindow;
        }

        private void addOneOneFragment(ParcelUuid tag, List<Participant> participants,
                final OneOneChatWindow oneOneChatWindow) {
            Logger.d(TAG, "addOneOneFragment entry, the tag is: " + tag);
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            instance.focus(tag);
            instance.focus(participants);
            instance.setFocusWindow(oneOneChatWindow);
            instance.addWindow(tag, oneOneChatWindow);
            final One2OneChatFragment chatFragment = oneOneChatWindow.getFragment();
            instance.addWindow(tag, oneOneChatWindow);
            runOnUiThread(new Runnable() {
                public void run() {
                    addOne2OneChatUi(chatFragment);
                    oneOneChatWindow.resume();
                    loadLatestUnreadMessage();
                }
            });
            Logger.d(TAG, "addOneOneFragment exit");
        }

        private void addGroupFragment(ParcelUuid tag, List<Participant> participants,
                final GroupChatWindow groupChatWindow) {
            Logger.d(TAG, "addGroupFragment entry, the tag is: " + tag);
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            instance.focus(tag);
            instance.focus(participants);
            instance.setFocusWindow(groupChatWindow);
            instance.addWindow(tag, groupChatWindow);
            final GroupChatFragment chatFragment = groupChatWindow.getFragment();
            runOnUiThread(new Runnable() {
                public void run() {
                    addGroupChatUi(chatFragment, mChatWindowManager);
                    groupChatWindow.resume();
                    loadLatestUnreadMessage();
                }
            });
            Logger.d(TAG, "addGroupFragment exit");
        }

        public void onPause() {
            Logger.d(TAG, "onPause entry");
            ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
            if (window != null) {
                window.pause();
            } else {
                Logger.d(TAG, "onPause window is null");
            }
        }

        public void onResume() {
            Logger.d(TAG, "onResume entry");
            ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
            if (window != null) {
                window.resume();
            } else {
                Logger.d(TAG, "onResume window is null");
            }
        }

        @Override
        public boolean removeChatWindow(IChatWindow chatWindow) {
            removeChatUI();
            return true;
        }

        @Override
        public void switchChatWindowByTag(final ParcelUuid uuidTag) {
            Logger.d(TAG, "switchChatWindowByTag entry, the tag is " + uuidTag);
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            if (instance.getCurrentTag() != uuidTag) {
                Logger.d(TAG,
                        "switchChatWindowByTag entry, the window is not focused, so we skip it");
                return;
            }
            final ChatScreenWindow window = instance.getWindow(uuidTag);
            instance.focus((ParcelUuid) uuidTag);
            instance.setFocusWindow(window);
            Logger.d(TAG, "switchChatWindowByTag window is: " + window);
            if (window instanceof OneOneChatWindow) {
                final One2OneChatFragment chatFragment = ((OneOneChatWindow) window).getFragment();
                runOnUiThread(new Runnable() {
                    public void run() {
                        addOne2OneChatUi(chatFragment);
                        Logger.d(TAG, "switchChatWindowByTag() mIsActivityBackground is "
                                + mIsActivityBackground);
                        if (!mIsActivityBackground) {
                            window.resume();
                        }
                        loadLatestUnreadMessage();
                    }
                });
                Logger.d(TAG, "switchChatWindowByTag exit");
            } else if (window instanceof GroupChatWindow) {
                final GroupChatFragment chatFragment = ((GroupChatWindow) window).getFragment();
                runOnUiThread(new Runnable() {
                    public void run() {
                        addGroupChatUi(chatFragment, mChatWindowManager);
                        Logger.d(TAG, "switchChatWindowByTag() mIsActivityBackground is "
                                + mIsActivityBackground);
                        if (!mIsActivityBackground) {
                            window.resume();
                        }
                        loadLatestUnreadMessage();
                    }
                });
                Logger.d(TAG, "switchChatWindowByTag exit");
            }
        }

        @Override
        public void onClick(View v) {
            Logger.d(TAG, "onClick entry");
            ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
            if (window instanceof GroupChatWindow) {
                ((GroupChatWindow) window).getmGroupChatFragment().showParticpants();
            } else {
                Logger.d(TAG, "this window is not of group type");
            }
        }
    }

    private void addContacts() {
        ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
        Logger.d(TAG, "addContacts window is: " + window);
        if (window != null) {
            window.addContacts();
        }
    }

    private void quitGroupChat() {
        Logger.v(TAG, "quitGroupChat entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        Logger.d(TAG, "quitGroupChat controller is: " + controller);
        if (null != controller) {
            ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
            Object tag = null;
            Logger.d(TAG, "quitGroupChat window is: " + window);
            if (window != null) {
                tag = window.getTag();
                Message controllerMessage =
                        controller.obtainMessage(ChatController.EVENT_QUIT_GROUP_CHAT, tag, null);
                controllerMessage.sendToTarget();
            }
        }
        removeChatUI();
        Logger.v(TAG, "quitGroupChat exit");
    }

    public class ClearHistoryDialog extends DialogFragment implements
            DialogInterface.OnClickListener {

        private static final String TAG = "ClearHistoryDialog";

        public ClearHistoryDialog() {

        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog =
                    new AlertDialog.Builder(ChatScreenActivity.this).setIconAttribute(
                            android.R.attr.alertDialogIcon).create();
            alertDialog.setTitle(R.string.text_clear_history);
            alertDialog.setMessage(getString(R.string.clear_history_message));
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
                Logger.d(TAG, "window is: " + window);
                if (window != null) {
                    window.clearHistory();
                }
            }
            dismissAllowingStateLoss();
        }
    }

    /**
     * This class defined to display a dialog to achieve blocking operation
     */
    public static class BlockContactDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "BlockContactDialog";
        private Participant mParticipant = null;
        private Object mChatTag = null;

        /**
         * Constructor
         */
        public BlockContactDialog() {

        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            mChatTag = getArguments().getParcelable(Utils.CHAT_TAG);
            mParticipant = getArguments().getParcelable(Utils.CHAT_PARTICIPANT);
            AlertDialog dialog = new AlertDialog.Builder(this.getActivity()).create();
            String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                    mParticipant.getContact());
            dialog.setIconAttribute(android.R.attr.alertDialogIcon);
            dialog.setTitle(getString(R.string.oneonechat_block_title, displayName));
            dialog.setMessage(getString(R.string.oneonechat_block_message, displayName));
            dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return dialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                new AsyncTask<Object, Void, Object>() {
                    @Override
                    protected Object doInBackground(Object... params) {
                        String contactNum = mParticipant.getContact();
                        Logger.d(TAG, "context_menu_item_to_block the block number is: "
                                + contactNum);
                        if (contactNum != null) {
                            ContactsManager.getInstance().setImBlockedForContact(contactNum, true);
                        }
                        return null;
                    }
                }.execute();
                ControllerImpl controller = ControllerImpl.getInstance();
                Logger.d(TAG, "onOptionsItemSelected() menu_block controller is: " + controller);
                if (null != controller) {
                    Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_CLOSE_WINDOW, mChatTag, null);
                    controllerMessage.sendToTarget();
                }
                dismissAllowingStateLoss();
            } else if (which == DialogInterface.BUTTON_NEGATIVE) {
                dismissAllowingStateLoss();
            } else {
                Logger.v(TAG, "window is null");
            }
        }
    }

    private void removeChatUI() {
        Logger.v(TAG, "removeChatUI entry");
        final ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
        final ChatScreenWindow window = instance.getFocusWindow();
        Logger.d(TAG, "removeChatUI window is: " + window);
        if (null != window) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    finish();
                    instance.setFocusWindow(null);
                }
            });
        }
        Logger.v(TAG, "removeChatUI exit");
    }

    /**
     * The first time you add {@link ChatFragment} to {@link ChatScreenActivity}
     * . *
     * 
     * @param clickListener
     * @param tag The fragment's tag
     * @param participants The participant's display name. If this is a group
     *            chat then the display name link with ','.
     */
    public void addGroupChatUi(GroupChatFragment chatFragment, OnClickListener clickListener) {
        Logger.v(TAG, "addGroupChatUi(),chatFragment = " + chatFragment + ",clickListener= "
                + clickListener + ",isFinishing() = " + isFinishing());
        if (isFinishing()) {
            Logger.w(TAG, "addGroupChatUi() mIsActivityDestoryed is true");
            return;
        }
        LayoutInflater inflater = LayoutInflater.from(getApplicationContext());
        View customView = inflater.inflate(R.layout.group_chat_screen_title, null);
        getActionBar().setCustomView(customView);
        FragmentManager fragmentManager = getFragmentManager();
        FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
        if (!chatFragment.isAdded()) {
            Logger.w(TAG, "addGroupChatUi() fragment is already added");
            fragmentTransaction.add(R.id.chat_content, chatFragment);
        }
        fragmentTransaction.show(chatFragment);
        fragmentTransaction.commitAllowingStateLoss();
        fragmentManager.executePendingTransactions();
        chatFragment.addGroupChatMembersIcon();
        if((newGroupChat && !IsGroupInvite)|| fromOne2OneToGroupChat == true)
        {
            chatFragment.showSubjectDialog();
            newGroupChat = false;
            fromOne2OneToGroupChat = false;
        }
        else
        chatFragment.setChatScreenTitle();
        invalidateOptionsMenu();
        RelativeLayout groupChatTitleLayout =
                (RelativeLayout) findViewById(R.id.group_chat_title_layout_main);
        ImageButton expandGroupChat =
                (ImageButton) findViewById(R.id.group_chat_expand);
        expandGroupChat.setOnClickListener(clickListener);        
        groupChatTitleLayout.setOnClickListener(clickListener);
    }

    /**
     * The first time you add {@link ChatFragment} to {@link ChatScreenActivity}
     * . *
     * 
     * @param clickListener
     * @param tag The fragment's tag
     * @param participants The participant's display name. If this is a group
     *            chat then the display name link with ','.
     */
    public void addOne2OneChatUi(One2OneChatFragment chatFragment) {
        Logger.v(TAG, "addOne2OneChatUi(),chatFragment = " + chatFragment + ",isFinishing() = "
                + isFinishing());
        if (isFinishing()) {
            Logger.w(TAG, "addOne2OneChatUi() mIsActivityDestoryed is true");
            return;
        }
        LayoutInflater inflater = LayoutInflater.from(getApplicationContext());
        View customView = inflater.inflate(R.layout.one2one_chat_screen_title, null);
        getActionBar().setCustomView(customView);
        FragmentManager fragmentManager = getFragmentManager();
        FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
        if (!chatFragment.isAdded()) {
            Logger.w(TAG, "addOne2OneChatUi() fragment is already added");
            fragmentTransaction.add(R.id.chat_content, chatFragment);
        }
        fragmentTransaction.show(chatFragment);
        fragmentTransaction.commitAllowingStateLoss();
        fragmentManager.executePendingTransactions();
        chatFragment.setChatScreenTitle();
        invalidateOptionsMenu();
    }

    private void hideInputMethod() {
        InputMethodManager inputMethodManager =
                (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        Window window = this.getWindow();
        Logger.d(TAG, "hideInputMethod(), window: " + window + " , inputMethodManager: "
                + inputMethodManager);
        if (window != null && inputMethodManager != null) {
            View currentFocus = window.getCurrentFocus();
            Logger.d(TAG, "hideInputMethod(), currentFocus is: " + currentFocus);
            if (currentFocus != null) {
                inputMethodManager.hideSoftInputFromWindow(currentFocus.getWindowToken(), 0);
            }
        }
    }
    
    private void setWallpaper() {
        Logger.d(TAG, "setWallpaper");
        Intent intent = new Intent(DialogActivity.ACTION_CHOOSE_WALLPAPER);
        startActivity(intent);
    }
}
