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
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.os.RemoteException;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.MenuItem.OnMenuItemClickListener;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.rcse.activities.ChatListView.ChatsStruct;
import com.mediatek.rcse.activities.ChatListView.InvitationStruct;
import com.mediatek.rcse.activities.ChatListView.OnSelectedItemListener;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatListProvider;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.RcsNotification.GroupInviteChangedListener;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Fragment used to show group chat invitation list and chat list
 */
public class ChatListFragment extends Fragment implements OnSelectedItemListener,
        IChatWindowManager, GroupInviteChangedListener {

    private static final String TAG = "ChatListFragment";
    private Activity mActivity = null;
    private ChatListView mChatListView = null;
    private Handler mUiHandler = null;
    private static final String ICON = "icon";
    private static final String SUBJECT = "subject";
    public static final String NUMBER = "number";
    private static final int CONTEXT_MENU_DELETE = 0;
    private static final int CONTEXT_MENU_BLOCK = 1;
    private static final int ONEONECHAT_MENU_GROUP = 0;
    private static final int GROUPCHAT_MENU_GROUP = 1;
    private static final int PARTICIPANTS_INDEX_ZERO = 0;
    private static final int PARTICIPANTS_INDEX_ONE = 1;
    private static final String ELLIPSIS = "...";
    private static final String COMMA = ", ";
    private static final int PARTICIPANTS_NUMBER_TWO = 2;
    private static final long THREAD_ID_MAIN = 1;
    public static ArrayList<ChatListProvider> chatListHistoryFromDatabase;


    public static ArrayList<ChatListProvider> getChatListHistoryFromDatabase() {
        return chatListHistoryFromDatabase;
    }

    public static void setChatListHistoryFromDatabase(
            ArrayList<ChatListProvider> chatListHistoryFromDatabase) {
        ChatListFragment.chatListHistoryFromDatabase = chatListHistoryFromDatabase;
    }

    public ChatListFragment() {

    }

    public ChatListView getChatListView(){
    	return mChatListView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mActivity = activity;
        mUiHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Logger.d(TAG, "onCreate() entry");
        super.onCreate(savedInstanceState);
        mChatListView = new ChatListView(mActivity, this);
        mChatListView.setContextMenuListener(this);
        ViewImpl.getInstance().addChatWindowManager(this, true);
        loadGroupInvitation();
        RcsNotification.getInstance().registerGroupInviteChangedListener(this);
    }

    @Override
    public void onStart() {
        super.onStart();
        RcsNotification instance = RcsNotification.getInstance();
        if (instance != null) {
            instance.setIsInChatMainActivity(true);
        } else {
            Logger.w(TAG, "onStart(), the instance of RcsNotification is null!");
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        RcsNotification instance = RcsNotification.getInstance();
        if (instance != null) {
            instance.setIsInChatMainActivity(false);
        } else {
            Logger.w(TAG, "onStop(), The RcsNotification.getInstance() is null!");
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if (mChatListView == null) {
            Logger.d(TAG, "onCreateView mChatListView is null");
            return super.onCreateView(inflater, container, savedInstanceState);
        }
        return mChatListView.getRootView();
    }

    public void onDestroy() {
        Logger.d(TAG, "onDestroy() entry");
        ViewImpl.getInstance().removeChatWindowManager(this);
        RcsNotification.getInstance().unregisterGroupInviteChangedListener();
        mChatListView.destroy();
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        mActivity = null;
        super.onDetach();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        Object obj = mChatListView.getData(v);
        String displayName = null;
        if (obj instanceof ChatsStruct) {
            ChatsStruct chat = (ChatsStruct) obj;
            if (!chat.isGroupChat()) {
                One2OneChatMap oneOneChat = (One2OneChatMap) chat.getChatMap();
                final Object tag = oneOneChat.getTag();
                final Participant participant = oneOneChat.getParticipant();
                displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                        participant.getContact());
                // handle 1-2-1 close menu.
                MenuItem closeItem = menu.add(ONEONECHAT_MENU_GROUP, CONTEXT_MENU_DELETE, 0,
                        R.string.text_close_chat);
                closeItem.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        ConfirmOneOneCloseDialog dialog = new ConfirmOneOneCloseDialog();
                        Bundle arguments = new Bundle();
                        arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) tag);
                        dialog.setArguments(arguments);
                        dialog.show(getFragmentManager(), ConfirmOneOneCloseDialog.TAG);
                        return true;
                    }
                });
                // handle 1-2-1 block menu.
                MenuItem blockItem = menu.add(ONEONECHAT_MENU_GROUP, CONTEXT_MENU_BLOCK, 0,
                        R.string.text_block);
                blockItem.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        ConfirmOneOneBlockDialog dialog = new ConfirmOneOneBlockDialog();
                        Bundle arguments = new Bundle();
                        arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) tag);
                        arguments.putParcelable(Utils.CHAT_PARTICIPANT, (Parcelable) participant);
                        dialog.setArguments(arguments);
                        dialog.show(getFragmentManager(), ConfirmOneOneBlockDialog.TAG);
                        return true;
                    }
                });
            } else {
                GroupChatMap groupChat = (GroupChatMap) chat.getChatMap();
                if (groupChat != null) {
                    Object numberList = groupChat.get(ChatListFragment.NUMBER);
                    Logger.d(TAG, "onCreateContextMenu() the obj is " + obj);
                    if (numberList instanceof List) {
                        List<String> numbers = (List<String>) numberList;
                        displayName = ChatFragment.getParticipantsName(numbers
                                .toArray(new String[1]));
                        Logger.d(TAG, "onCreateContextMenu() the display name is " + displayName);
                        groupChat.put(SUBJECT, displayName);
                        final Object tag = groupChat.getTag();
                        // handle group chat delete.
                        MenuItem deleteItem = menu.add(GROUPCHAT_MENU_GROUP, CONTEXT_MENU_DELETE,
                                0, R.string.text_close_chat);
                        deleteItem.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                            @Override
                            public boolean onMenuItemClick(MenuItem item) {
                                ConfirmGroupDeleteDialog dialog = new ConfirmGroupDeleteDialog();
                                Bundle arguments = new Bundle();
                                arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) tag);
                                dialog.setArguments(arguments);
                                dialog.show(getFragmentManager(), ConfirmGroupDeleteDialog.TAG);
                                return true;
                            }
                        });
                    } else {
                        Logger.e(TAG, "onCreateContextMenu() obj is not a List");
                    }
                } else {
                    Logger.e(TAG, "onCreateContextMenu the groupchatmap is null");
                }
            }
            menu.setHeaderTitle(displayName);
        } else {
            Logger.d(TAG, "onCreateContextMenu it' not a chat view");
        }

    }

    // Confirm dialog for closing 1-2-1 chat.
    public class ConfirmOneOneCloseDialog extends DialogFragment {
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
            return new AlertDialog.Builder(mActivity, AlertDialog.THEME_HOLO_LIGHT)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(getString(R.string.text_close_chat))
                    .setMessage(getString(R.string.text_close_chat))
                    .setPositiveButton(getString(R.string.rcs_dialog_positive_button),
                            new OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    ControllerImpl controller = ControllerImpl.getInstance();
                                    if (null != controller) {
                                        Message controllerMessage = controller.obtainMessage(
                                                ChatController.EVENT_CLOSE_WINDOW, tag, null);
                                        controllerMessage.sendToTarget();
                                    } else {
                                        Logger.d(TAG, "ConfirmOneOneCloseDialog controller is null");
                                    }
                                    dismiss();
                                }
                            })
                    .setNegativeButton(getString(R.string.rcs_dialog_negative_button),
                            new OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    dismiss();
                                }
                            }).create();
        }
    }

    // Confirm dialog for block 1-2-1 chat.
    public class ConfirmOneOneBlockDialog extends DialogFragment {
        private static final String TAG = "ConfirmOneOneBlockDialog";

        public ConfirmOneOneBlockDialog() {

        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Object tag = getArguments().getParcelable(Utils.CHAT_TAG);
            final Participant participant = getArguments().getParcelable(Utils.CHAT_PARTICIPANT);
            String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                    participant.getContact());
            return new AlertDialog.Builder(mActivity)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(getString(R.string.oneonechat_block_title, displayName))
                    .setMessage(getString(R.string.oneonechat_block_message, displayName))
                    .setPositiveButton(getString(R.string.rcs_dialog_positive_button),
                            new OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    new AsyncTask<Object, Void, Object>() {
                                        @Override
                                        protected Object doInBackground(Object... params) {
                                            String contactNum = participant.getContact();
                                            if (contactNum != null) {
                                                String mNum = contactNum
                                                        .substring(CONTEXT_MENU_BLOCK);
                                                Long.parseLong(mNum);
                                                ContactsManager.getInstance()
                                                        .setImBlockedForContact(contactNum, true);
                                            } else {
                                                Logger.e(TAG,
                                                        "context_menu_item_to_block the block number is null");
                                            }

                                            return null;
                                        }
                                    }.execute();

                                    ControllerImpl controller = ControllerImpl.getInstance();
                                    if (null != controller) {
                                        Message controllerMessage = controller.obtainMessage(
                                                ChatController.EVENT_CLOSE_WINDOW, tag, null);
                                        controllerMessage.sendToTarget();

                                    } else {
                                        Logger.d(TAG, "ConfirmOneOneBlockDialog controller is null");
                                    }
                                    dismiss();
                                }
                            })
                    .setNegativeButton(getString(R.string.rcs_dialog_negative_button),
                            new OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    dismiss();
                                }
                            }).create();
        }
    }

    // Confirm dialog for deleting group chat .
    public class ConfirmGroupDeleteDialog extends DialogFragment {
        private static final String TAG = "ConfirmGroupDeleteDialog";

        public ConfirmGroupDeleteDialog() {

        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Object tag = getArguments().getParcelable(Utils.CHAT_TAG);
            return new AlertDialog.Builder(mActivity)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(getString(R.string.text_close_chat))
                    .setMessage(getString(R.string.text_close_chat))
                    .setPositiveButton(getString(R.string.rcs_dialog_positive_button),
                            new OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    ControllerImpl controller = ControllerImpl.getInstance();
                                    if (null != controller) {
                                        // clear history and quit group chat.
                                        Message controllerMessage = controller.obtainMessage(
                                                ChatController.EVENT_CLEAR_CHAT_HISTORY, tag, null);
                                        controllerMessage.sendToTarget();
                                        controllerMessage = controller.obtainMessage(
                                                ChatController.EVENT_CLOSE_WINDOW, tag, null);
                                        controllerMessage.sendToTarget();
                                    } else {
                                        Logger.d(TAG, "ConfirmGroupDeleteDialog controller is null");
                                    }
                                    dismiss();
                                }
                            })
                    .setNegativeButton(getString(R.string.rcs_dialog_negative_button),
                            new OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    dismiss();
                                }
                            }).create();
        }
    }

    @Override
    public void onPause() {
        Logger.d(TAG, "onPause()");
        super.onPause();
    }

    @Override
    public void onResume() {
        Logger.d(TAG, "onResume()");
        super.onResume();
        if (mChatListView != null) {
            mChatListView.updateView();
        }
        RcsNotification.getInstance().cancelNotification();
    }

    private void loadGroupInvitation() {
        Logger.d(TAG, "loadGroupInvitation() entry");
        ConcurrentHashMap<IChatSession, Intent> groupInviteMap = RcsNotification.getInstance()
                .getTempGroupInvite();
        for (Map.Entry<IChatSession, Intent> entry : groupInviteMap.entrySet()) {
            IChatSession key = entry.getKey();
            Intent intentValue = entry.getValue();
            Logger.d(TAG, "loadGroupInvitation() key: " + key + " value: " + intentValue);
            onAddedGroupInvite(key, intentValue);
        }
        Logger.d(TAG, "loadGroupInvitation() exit");
    }

    private IChatSession getChatSession(String sessionId) {
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.i(TAG, "ApiManager instance is null");
            return null;
        }
        MessagingApi messageApi = instance.getMessagingApi();
        if (messageApi == null) {
            Logger.d(TAG, "MessageingApi instance is null");
            return null;
        }
        IChatSession chatSession = null;
        try {
            chatSession = messageApi.getChatSession(sessionId);
            return chatSession;
        } catch (ClientApiException e) {
            Logger.e(TAG, "Get chat session failed");
            e.printStackTrace();
            chatSession = null;
            return chatSession;
        }
    }

    @Override
    public void onSelectedItem(View view, int position) {
        Object obj = mChatListView.getData(view);
        if (obj instanceof ChatsStruct) {
            ChatsStruct chat = (ChatsStruct) obj;
            Intent intent = new Intent(mActivity, ChatScreenActivity.class);
            if (!chat.isGroupChat()) {
                intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG,
                        (ParcelUuid) (((One2OneChatMap) chat.getChatMap()).mTag));
                mActivity.startActivity(intent);
            } else {
                intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, (ParcelUuid) (((GroupChatMap) chat
                        .getChatMap()).mTag));
                mActivity.startActivity(intent);
            }
        } else if (obj instanceof InvitationStruct) {
            InvitationStruct invite = (InvitationStruct) obj;
            Intent intent = invite.getIntent();
            intent.putExtra(RcsNotification.NOTIFY_CONTENT, invite.getInformation());
            intent.setClass(mActivity, InvitationDialog.class);
            if (Logger.getIsIntegrationMode()) {
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
            } else {
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_GROUP_INVITATION);
            }
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mActivity.startActivityForResult(intent, 0);
        } else {
            return;
        }
    }

    @Override
    public void onAddedGroupInvite(final IChatSession chatSession, final Intent intent) {
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                if (null == chatSession || null == intent) {
                    Logger.d(TAG, "Illegal state: IChatSession is "
                            + (null == chatSession ? "null" : "not null") + "Intent is "
                            + (null == intent ? "null" : "not null"));
                    return;
                }
                if (mActivity == null || mChatListView == null) {
                    Logger.d(TAG, "The mActivity is null or mChatListView is null");
                    return;
                }
                Drawable drawable = null;
                Resources resources = mActivity.getResources();
                if (resources != null) {
                    drawable = resources.getDrawable(R.drawable.icon_chat_121);
                } else {
                    Logger.d(TAG, "The resources is null");
                }
                Date date = null;
                InstantMessage message = null;
                try {
                    message = chatSession.getFirstMessage();
                } catch (RemoteException remoteException) {
                    Logger.e(TAG, "getChatSession fail");
                    remoteException.printStackTrace();
                }
                String info = (String) intent.getExtra(RcsNotification.NOTIFY_INFORMATION);
                if (message != null) {
                    date = message.getDate();
                } else {
                    Logger.e(TAG, "The invitation date is null");
                }
                InvitationStruct invite =
                        new InvitationStruct(drawable, date, info, intent, chatSession);
                mChatListView.addInviteItem(invite);
            }
        });
    }

    @Override
    public void onRemovedGroupInvite(final String sessionId) {
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                mChatListView.removeInviteItem(sessionId);
            }
        });
    }

    private IGroupChatWindow addGroupChatWindowByInfos(final Object tag,
            final List<Participant> participantList, String chatid) {
        Logger
                .v(TAG, "addGroupChatWindow(),tag = " + tag + ",participantList = "
                        + participantList);
        final GroupChatMap chatMap = new GroupChatMap(tag, participantList);
        chatMap.setChatID(chatid);
        addMapToChatList(chatMap);
        return chatMap;
    }

    @Override
    public IGroupChatWindow addGroupChatWindow(Object tag, List<ParticipantInfo> participantList) {
        List<Participant> participants = new ArrayList<Participant>();
        for (ParticipantInfo participant : participantList) {
            participants.add(participant.getParticipant());
        }
        return addGroupChatWindowByInfos(tag, participants,participantList.get(0).getmChatID());
    }

    @Override
    public IOne2OneChatWindow addOne2OneChatWindow(final Object tag, final Participant participant) {
        Logger
                .d(TAG, "addOne2OneChatWindow() entry, tag: " + tag + ", participant: "
                        + participant);
        final One2OneChatMap chatMap = new One2OneChatMap(tag, participant);
        addMapToChatList(chatMap);
        return chatMap;
    }

    private void addMapToChatList(final ChatMap chatMap) {
        Logger.d(TAG, "addMapToChatList() entry, chatMap: " + chatMap);
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                if (mActivity == null || mChatListView == null) {
                    Logger.d(TAG,
                            "addMapToChatList() The mActivity is null or mChatListView is null");
                    return;
                }
                ChatsStruct chatStruct = null;
                if (chatMap instanceof One2OneChatMap) {
                    chatStruct = new ChatsStruct(false, chatMap);
                } else {
                    chatStruct = new ChatsStruct(true, chatMap);
                    chatStruct.setmSubject(((GroupChatMap)chatMap).mSubject);
                }
                mChatListView.addChatItem(chatStruct);
                
                //Reload Chats on restart
                ArrayList<ChatListProvider> chatListFromDatabase = new ArrayList<ChatListProvider>();
                chatListFromDatabase = getChatListHistoryFromDatabase();
                if (chatListFromDatabase == null)
                    chatListFromDatabase = ModelImpl.getInstance()
                            .getChatListHistory(getActivity());
                
                if (chatStruct.getLatestMessage() == null) {
                    ArrayList<Integer> messageIdArray = new ArrayList<Integer>();
                    if (chatStruct.isGroupChat()) {
                        for (int i = 0; i < chatListFromDatabase.size(); i++) {

                            if (chatListFromDatabase.get(i).getChatID() == ((GroupChatMap) chatMap)
                                    .getChatID()) {
                                    messageIdArray.clear();
                                messageIdArray.add(chatListFromDatabase.get(i)
                                        .getMessageId());
                                    break;
                                }
                        }                        
                    } else {
                        for (int i = 0; i < chatListFromDatabase.size(); i++) {
                            Participant participant = chatListFromDatabase
                                    .get(i).getParticipantlist().get(0);
                            if (chatMap.getmParticipant().equals(participant)) {
                                messageIdArray.clear();
                                messageIdArray.add(chatListFromDatabase.get(i)
                                        .getMessageId());
                                break;
                            }
            }
                    }
                    try {
                        Message controllerMessage = ControllerImpl
                                .getInstance()
                                .obtainMessage(
                                        ChatController.EVENT_RELOAD_MESSAGE,
                                        chatMap.mTag.toString(), messageIdArray);
                        controllerMessage.sendToTarget();
                    } catch (Exception e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
            }
                }
            }

        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.getId() == THREAD_ID_MAIN) {
            Logger.d(TAG, "addMapToChatList() ran on main thread");
            worker.run();
        } else {
            if (mUiHandler != null) {
                Logger.d(TAG, "addMapToChatList() post to main thread");
                mUiHandler.post(worker);
            } else {
                Logger.w(TAG, "addMapToChatList(), mUiHandler is null");
            }
        }
    }

    @Override
    public boolean removeChatWindow(final IChatWindow chatWindow) {

        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                ChatListFragment.this.mChatListView.removeChatItem(chatWindow);
            }
        });
        return true;
    }

    @Override
    public void switchChatWindowByTag(ParcelUuid uuidTag) {

    }

    abstract static class ChatMap extends HashMap<String, Object> implements IChatWindow {
        protected Object mTag = null;

        /**
         * Get chat tag.
         * 
         * @return The tag of the chat.
         */
        public Object getTag() {
            return mTag;
        }

        public List<Participant> getmParticipantList() {
            // TODO Auto-generated method stub
            return null;
    }

        public Object getmParticipant() {
            // TODO Auto-generated method stub
            return null;
        }
    }

    public class One2OneChatMap extends ChatMap implements IOne2OneChatWindow {
        public static final String TAG = "One2OneChatMap";
        private Participant mParticipant = null;
        private int mFileTransferCount = 0;
        private final TreeSet<ChatDateUpdater> mMessageList = new TreeSet<ChatDateUpdater>();
        private final Date mLatestDate = new Date();

        public One2OneChatMap(Object tag, Participant participant) {
            mTag = tag;
            mParticipant = participant;
            this.put(ICON, R.drawable.icon_chat_121);
            this.put(SUBJECT, participant.getDisplayName());
            this.put(NUMBER, participant.getContact());
        }

        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages() entry mTag is " + mTag);
            mChatListView.removeMessageForWinTag(mTag);
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public void setFileTransferEnable(int reason) {
            // TODO Auto-generated method stub

        }

        public Participant getmParticipant() {
            return mParticipant;
        } 

        @Override
        public void setIsComposing(boolean isComposing) {
            // TODO Auto-generated method stub

        }

        @Override
        public void setRemoteOfflineReminder(boolean isOffline) {
            // TODO Auto-generated method stub
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(final InstantMessage message,
                final boolean isRead) {
            Logger.d(TAG, "addReceivedMessage() entry the message is " + message);
            if (message != null && !message.getIsHistory()) {
                final ChatDateUpdater dateUpdater = new ChatDateUpdater(message);
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mMessageList.add(dateUpdater);
                        onDateUpdated(dateUpdater, isRead);
                    }
                });
                return dateUpdater;
            } else {
                Logger.d(TAG, "addReceivedMessage the message is from load history ");
            }
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(final InstantMessage message, int messageTag) {
            Logger.d(TAG, "addSentMessage() entry the message is " + message);
            if (message != null && !message.getIsHistory()) {
                final ChatDateUpdater dateUpdater = new ChatDateUpdater(message);
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mMessageList.add(dateUpdater);
                        onDateUpdated(dateUpdater, true);
                    }
                });
                return dateUpdater;
            } else {
                Logger.d(TAG, "addSentMessage the message is from load history ");
            }
            return null;
        }

        private void onDateUpdated(ChatDateUpdater currentDateUpdater, boolean isRead) {
            Logger.d(TAG, "onDateUpdated() currentDateUpdater: " + currentDateUpdater
                    + ", isRead is " + isRead);
            Date currentDate = currentDateUpdater.mDate;            
                mChatListView.addMessage(mTag, currentDateUpdater.mMessage, isRead);
                mLatestDate.setTime(currentDate.getTime());            
            Logger.d(TAG, "onDateUpdated() exit");
        }

        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
            return new ActiveFileTransferRecord();
        }

        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return new ActiveFileTransferRecord();
        }

        private class ActiveFileTransferRecord implements IFileTransfer {
            boolean mIsActive = false;

            @Override
            public void setFilePath(String filePath) {
            }

            @Override
            public void setProgress(long progress) {
            }

            @Override
            public void setStatus(Status status) {
                handleFileTransferStateChaged(status);
            }

            @Override
            public void updateTag(String transferTag, long size) {
            }

            private synchronized void handleFileTransferStateChaged(Status status) {
                Logger.d(TAG, "handleFileTransferStateChaged, status = " + status);

                switch (status) {
                    case PENDING:
                    case WAITING:
                    case TRANSFERING:
                        if (!mIsActive) {
                            mIsActive = true;
                            mFileTransferCount++;
                            Logger.d(TAG,
                                    "handleFileTransferStateChaged, The status changed, so change the count, count = "
                                            + mFileTransferCount);
                        } else {
                            Logger.d(TAG, "handleFileTransferStateChaged, The same status change!");
                        }
                        break;
                    case FAILED:
                    case CANCEL:
                    case CANCELED:
                    case REJECTED:
                    case FINISHED:
                        if (mIsActive) {
                            mIsActive = false;
                            mFileTransferCount--;
                            Logger.d(TAG,
                                    "handleFileTransferStateChaged, The status changed, so change the count, count = "
                                            + mFileTransferCount);
                        } else {
                            Logger.d(TAG, "handleFileTransferStateChaged, The same status change!");
                        }
                        break;
                    default:
                        break;
                }

                if (null != mChatListView) {
                    if (mFileTransferCount > 0) {
                        mChatListView.onFileTransferStateChanged(mTag, true);
                    } else {
                        mChatListView.onFileTransferStateChanged(mTag, false);
                    }
                } else {
                    Logger.w(TAG, "handleFileTransferStateChaged, mChatListView is null!");
                }
            }

        }

        /**
         * This class is used to update ChatListView when the latest message date is modified
         */
        private class ChatDateUpdater implements ISentChatMessage, IReceivedChatMessage, Comparable<ChatDateUpdater> {
            private static final String TAG = "ChatDateUpdater";

            private InstantMessage mMessage = null;
            private Date mDate = new Date();

            public ChatDateUpdater(InstantMessage message) {
                mMessage = message;
                mDate = (Date) mMessage.getDate().clone();
            }

            @Override
            public void updateDate(final Date date) {
                Logger.d(TAG, "updateDate() update date for messageId: " + getId() + ", date: " + date);
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mMessageList.remove(ChatDateUpdater.this);
                        mDate = (Date) date.clone();
                        mMessageList.add(ChatDateUpdater.this);
                        Logger.d(TAG, "updateDate()->run() mMessageList: " + mMessageList);
                        onDateUpdated(ChatDateUpdater.this, true);
                    }
                });
            }

            @Override
            public void updateStatus(Status status) {
                // Do nothing
            }

            @Override
            public String getId() {
                return mMessage.getMessageId();
            }

            @Override
            public int compareTo(ChatDateUpdater another) {
                return mDate.compareTo(another.mDate);
            }

            @Override
            public boolean equals(Object o) {
                if (o instanceof ChatDateUpdater) {
                    return this.hashCode() == o.hashCode();
                } else {
                    return false;
                }
            }

            @Override
            public int hashCode() {
                return mDate.hashCode() << 1 + super.hashCode();
            }

            @Override
            public String toString() {
                return TAG + "@" + getId();
            }

			public void updateStatus(Status status, String Contact) {
				// TODO Auto-generated method stub
				
			}
        }
        /**
         * Get participant of the chat.
         * 
         * @return The participant of the chat.
         */
        public Participant getParticipant() {
            return mParticipant;
        }

        @Override
        public void addLoadHistoryHeader(boolean showHeader) {
            // TODO Auto-generated method stub

        }

        @Override
        public void updateAllMsgAsRead() {
            mChatListView.updateAllMsgAsRead(mTag);

        }

        @Override
        public void removeChatMessage(String messageId) {
            mChatListView.removeMessageForId(mTag, messageId);
            
        }

		@Override
		public void updateAllMsgAsReadForContact(Participant participant) {
			// TODO Auto-generated method stub
			
		}
    }

    private class GroupChatMap extends ChatMap implements IGroupChatWindow {
        public static final String TAG = "GroupChatMap";
        public List<Participant> mParticipantList = new ArrayList<Participant>();
        public String mSubject ="";
        public String chatID ="";
        public String getChatID() {
            return chatID;
        }

        public void setChatID(String chatID) {
            this.chatID = chatID;
        }
        public List<Participant> getmParticipantList() {
            return mParticipantList;
        }

        public GroupChatMap(Object tag, List<Participant> participantList) {
            mTag = tag;
            mParticipantList = participantList;
            this.put(ICON, R.drawable.default_group_avatar);
            this.put(SUBJECT, SUBJECT);
            List<String> numbers = convertParticipantsToNumbers(participantList);
            if (null != numbers) {
                this.put(NUMBER, numbers);
            } else {
                Logger.e(TAG, "GroupChatMap() numbers is null");
            }
        }

        @Override
        public void setFileTransferEnable(int reason) {
            // TODO Auto-generated method stub

        }
        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
            //return new ActiveFileTransferRecord();
        	return null;
        }

        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            //return new ActiveFileTransferRecord();
        	return null;
        }
        
        
        private List<String> convertParticipantsToNumbers(List<Participant> participantList) {
            Logger.d(TAG, "convertParticipantsToNumbers() entry, participantList is "
                    + participantList);
            if (null != participantList) {
                List<String> numbers = new ArrayList<String>();
                for (Participant participant : participantList) {
                    numbers.add(participant.getContact());
                }
                return numbers;
            } else {
                Logger.e(TAG, "convertParticipantsToNumbers() participantList is null");
                return null;
            }
        }

        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages() entry mTag is " + mTag);
            mChatListView.removeMessageForWinTag(mTag);
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public void updateParticipants(List<ParticipantInfo> participantList) {
            Logger.d(TAG, "updateParticipants() entry, current participant in chat " + mTag
                    + " is " + participantList);
            if (null != participantList) {
                ArrayList<Participant> participants = new ArrayList<Participant>();
                List<String> numbers = new ArrayList<String>();
                for (ParticipantInfo info : participantList) {
                    participants.add(info.getParticipant());
                    numbers.add(info.getContact());
                }
                
                if(mSubject.equals(""))                
                put(SUBJECT, ChatFragment.getParticipantsName(participants
                        .toArray(new Participant[1]))
                        + ChatListView.LEFT_BRACKET
                        + participants.size()
                        + ChatListView.RIGHT_BRACKET);

                if (null != numbers) {
                    this.put(NUMBER, numbers);
                } else {
                    Logger.e(TAG, "updateParticipants() numbers is null");
                }
                if (mUiHandler == null) {
                    Logger.v(TAG, "updateParticipants() mUiHandler is null");
                    return;
                }
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        //mChatListView.updateChat(mTag);
                    }
                });
            } else {
                Logger.e(TAG, "updateParticipants() participantList is null");
            }

        }

        @Override
        public void setIsComposing(boolean isComposing, Participant participant) {
            // TODO Auto-generated method stub

        }

        @Override
        public IReceivedChatMessage addReceivedMessage(final InstantMessage message,
                final boolean isRead) {
            Logger.d(TAG, "addReceivedMessage() entry the message is " + message);
            if (message != null && !message.getIsHistory()) {
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mChatListView.addMessage(mTag, message, isRead);
                    }
                });
            } else {
                Logger.d(TAG, "addReceivedMessage the message is from load history ");
            }
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(final InstantMessage message, int messageTag) {
            Logger.d(TAG, "addSentMessage() entry the message is " + message);
            if (message != null && !message.getIsHistory()) {
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mChatListView.addMessage(mTag, message, true);
                    }
                });
            } else {
                Logger.d(TAG, "addSentMessage the message is from load history ");
            }
            return null;
        }

        @Override
        public void addLoadHistoryHeader(boolean showHeader) {
            // TODO Auto-generated method stub

        }

        @Override
        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            return null;

        }

        @Override
        public void updateChatStatus(int status) {
        }

        @Override
        public void updateAllMsgAsRead() {
            mChatListView.updateAllMsgAsRead(mTag);
        }
@Override
        public void removeChatMessage(String messageId) {
            mChatListView.removeMessageForId(mTag, messageId);          
        }
        @Override
        public void addgroupSubject(final String subject) {
            
                mSubject = subject;
                put(SUBJECT,mSubject);
                if (mUiHandler == null) {
                    Logger.v(TAG, "updateParticipants() mUiHandler is null");
                    return;
                }
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mChatListView.addGroupSubject(mTag, subject);
                    }
                });                        
        }

		@Override
		public void updateAllMsgAsReadForContact(Participant participant) {
			// TODO Auto-generated method stub
			
		}

    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        // Override this method to workaround a google issue happen when API level > 11
        outState.putString("work_around_tag", "work_around_content");
        super.onSaveInstanceState(outState);
    }
}
