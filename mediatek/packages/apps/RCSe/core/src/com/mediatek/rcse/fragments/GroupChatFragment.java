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

package com.mediatek.rcse.fragments;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;

import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.DataSetObserver;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.Parcel;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.os.Parcelable;
import android.text.InputFilter;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.view.View.OnCreateContextMenuListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;

import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;




import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.fragments.ChatFragment.FileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.server.gsma.GetContactCapabilitiesReceiver;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

public class GroupChatFragment extends ChatFragment implements ChatView.IGroupChatWindow,
        View.OnClickListener,OnCreateContextMenuListener {

    private static final String TAG = "GroupChatFragment";
    
    public static final int ALPHA_VALUE_ENABLE = 255;

    public static final int ALPHA_VALUE_DISABLE = 75;
    private int mGroupMemberHorMarginLeft = 6;
    private int mGroupMemberHorMarginRight = 6;
    private int mGroupMemberHorWidth = 48;
    private int mGroupMemberHorHeight = 48;
    private static final int ZERO_PARTICIPANT = 0;
    public static final String SHOW_REJOING_MESSAGE_REMINDER = "d";
    private List<String> mDateList = new ArrayList<String>();
    private List<IChatMessage> mMessageList = new Vector<IChatMessage>();
    private Activity mActivity = null;
    private String mPreviousDate = null;
    private int itemID_position;
    View mSubjectView;
    private static final String COLON = ": ";
    private final CopyOnWriteArrayList<Participant> mParticipantComposList =
            new CopyOnWriteArrayList<Participant>();
    protected TextView mRejoiningText = null;
    private List<ParticipantInfo> mGroupChatParticipantList = new ArrayList<ParticipantInfo>();
    // For test case to test whether max group chat participant mechanism work
    // when current participants is already the max number
    private boolean mIsMaxGroupChatParticipantsWork = false;
    private final Object mLock = new Object();
    private int mMessageSequenceOrder = -1 ;
    protected String mSubjectGroupChat = "";

    public String getmSubjectGroupChat() {
        return mSubjectGroupChat;
    }

    public void setmSubjectGroupChat(String mSubjectGroupChat) {
        if(mSubjectGroupChat!=null && !mSubjectGroupChat.equals(""))
        this.mSubjectGroupChat = mSubjectGroupChat;
    }

    public void setTag(Object tag) {
        mTag = tag;
    }

    public void setParticipantList(final CopyOnWriteArrayList<ParticipantInfo> participantList) {
        Logger.d(TAG, "setParticipantList() entry the participants is " + participantList);
        mGroupChatParticipantList = participantList;
        List<Participant> participants = new ArrayList<Participant>();
        for (ParticipantInfo participantInfo : participantList) {
            participants.add(participantInfo.getParticipant());
        }
        mParticipantList = participants;
    }

    /**
     * Get the participants list in the group chat fragment
     * @return participants list in the group chat fragment
     */
    public List<Participant> getParticipants() {
        return mParticipantList;
    }

    @Override
    public void onAttach(Activity activity) {
        mActivity = activity;
        super.onAttach(activity);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle arguments = getArguments();
        mTag = arguments.getParcelable(Utils.CHAT_TAG);
        mGroupChatParticipantList = arguments.getParcelableArrayList(Utils.CHAT_PARTICIPANTS);
        List<Participant> participants = new ArrayList<Participant>();
        for (ParticipantInfo participantInfo : mGroupChatParticipantList) {
            participants.add(participantInfo.getParticipant());
        }
        mParticipantList = participants;
        loadDimension(getResources());
        mMessageListView.setOnCreateContextMenuListener(this);        
        mMessageListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                // TODO Auto-generated method stub
                
            }
        });
        Configuration configuration = getResources().getConfiguration();
        mParticipantListDisplayer.onFragmentCreate(configuration.orientation);
        mRejoiningText = (TextView) mContentView.findViewById(R.id.text_rejoining_prompt);
    }

   
    public void showSubjectDialog() {
        final AlertDialog.Builder alert = new AlertDialog.Builder(getActivity());
        final String subject ;
        int maxLength = 15;
        mSubjectView = LayoutInflater.from(getActivity()).inflate(R.layout.group_subject_dialog, null);
        final TextView titleText = (TextView) mSubjectView.findViewById(R.id.titleTextView);
        final EditText input = (EditText) mSubjectView.findViewById(R.id.inputEditText);
        titleText.setText(" Please enter the group name");  
        InputFilter[] filterArray = new InputFilter[1];
        filterArray[0] = new InputFilter.LengthFilter(maxLength);
        input.setFilters(filterArray);
        mBtnEmotion = (ImageButton) mSubjectView.findViewById(R.id.btn_chat_emoticon_subject);
        mBtnEmotion.setOnClickListener(mBtnEmotionClickListener);
        input.setOnClickListener(mMessageEditorClickListener);
        super.updateEmoticonLayout(true, mSubjectView);
        alert.setView(mSubjectView);
        
        /*LinearLayout lila1 = new LinearLayout(getActivity());
        lila1.setOrientation(1); // 1 is for vertical orientation
        final TextView titleText = new TextView(getActivity());
        final EditText input = new EditText(getActivity());
        titleText.setText(" Please enter the group name");            
        InputFilter[] filterArray = new InputFilter[1];
        filterArray[0] = new InputFilter.LengthFilter(maxLength);
        input.setFilters(filterArray);
        lila1.addView(titleText);
        lila1.addView(input);
        alert.setView(lila1);    */    
        alert.setTitle("Group Title");
        alert.setCancelable(false);
                

        alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                setmSubjectGroupChat(input.getText().toString().trim());
                Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                        ChatController.ADD_GROUP_SUBJECT, mTag, getmSubjectGroupChat());                             
                controllerMessage.arg1 = 1;
                controllerMessage.sendToTarget();
                updateChatUi();
                GroupChatFragment.this.updateEmoticonLayout(false,GroupChatFragment.this.mSubjectView);

            }
        });
        alert.setNegativeButton("Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                                ChatController.EVENT_CLOSE_WINDOW, mTag, null);                         
                                               
                        controllerMessage.sendToTarget();                        
                        dialog.cancel();                        
                    }
                });
        alert.create();   
        alert.show();
    }
@Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {            
            Log.d("onCreateContextMenu", "inside ");            
            super.onCreateContextMenu(menu, v, menuInfo);            
            AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo)menuInfo; 
            menu.setHeaderTitle("Options");
            //get the postion of the elemet that is clicked
            itemID_position = info.position; 
            IChatMessage chatMessage = mMessageList.get(itemID_position- 1);
            MenuInflater inflater = getActivity().getMenuInflater();
                        
            if(chatMessage instanceof SentMessage && ((SentMessage)chatMessage).getStatus().equals(Status.FAILED))
            {
                inflater.inflate(R.menu.chatmessagewithresent, menu);               
            }
            else if(chatMessage instanceof DateMessage )
            {
            	 Logger.d(TAG, "onCreateContextMenu DateMessage: "); 
                // Don't Inflate menu
            } 
            else if(chatMessage instanceof FileTransfer )
            {
            	com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status status = ((FileTransfer)chatMessage).getStatue();
                if(status.equals(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCEL) || status.equals(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCELED) || status.equals(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED) )
                {
                    inflater.inflate(R.menu.chatmessagewithresent, menu); 
                }
                else
                {
                	inflater.inflate(R.menu.chatmessagemenu, menu); 
                }
            } 
            else if(chatMessage instanceof SentMessage )
            {
                 Logger.d(TAG, "onCreateContextMenu SentMessage: "); 
                 inflater.inflate(R.menu.chatmessagemenuwithstatus, menu);    
            }
            else 
            {
                inflater.inflate(R.menu.chatmessagemenu, menu);    
            }
            
       }    
  
    
    @Override
    public boolean onContextItemSelected(MenuItem item) {
        Log.d("Context Menu", "inside context menu item selected");        
        IChatMessage chatMessage = mMessageList.get(itemID_position- 1);
        switch(item.getItemId())
        {

        case R.id.delete:              
            mMessageAdapter.removeMessage(itemID_position -1);
            mMessageSequenceOrder = mMessageSequenceOrder - 1;
            Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                    ChatController.EVENT_DELETE_MESSAGE, mTag, chatMessage.getId());            
            controllerMessage.sendToTarget();
        break;  
        
        case R.id.info:         {
            MessageInfoDialog infoDialog = new MessageInfoDialog(new Date(), "", "");            
            if(chatMessage instanceof SentMessage)
            {
                infoDialog = new MessageInfoDialog(((SentMessage)chatMessage).getMessageDate(),((SentMessage)chatMessage).getMessageText(),"");
            }
            else if (chatMessage instanceof ReceivedMessage)
            {
                infoDialog = new MessageInfoDialog(((ReceivedMessage)chatMessage).getMessageDate(),((ReceivedMessage)chatMessage).getMessageText(),((ReceivedMessage)chatMessage).getMessageSender());
            }
                        
            infoDialog.show(getFragmentManager(), "RepickDialog");
        }                  
        
        break;       
        
        case R.id.resend:  
            
            if(chatMessage instanceof SentMessage)
            {
                int messageTag = ((SentMessage)chatMessage).getMessageTag();
                controllerMessage = ControllerImpl.getInstance().obtainMessage(
                        ChatController.EVENT_SEND_MESSAGE, mTag, ((SentMessage) chatMessage).getMessageText());
                controllerMessage.arg1 = messageTag;
                controllerMessage.sendToTarget(); 
            }                   
        
        break; 
              
        case R.id.block:
        	if(chatMessage instanceof FileTransfer)
        	{
        		//mark future requests for this contact as spam
        		
        	}
        	
        	break;
              
        case R.id.status:
            if(chatMessage instanceof SentMessage)
            {
                Logger.d(TAG, "show status dialog ");
                showStatusDialog(getActivity(), chatMessage);
            }
        break;
              
        } 
        return true;
    }    
    
    public void showStatusDialog(Context context, IChatMessage chatMessage) {
    
        Logger.d(TAG, "show status dialog enter ");
        Intent intent = new Intent(InvitationDialog.ACTION);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_GROUP_CHAT_MSG_RECEIVED_STATUS);
        if(chatMessage instanceof GroupChatFragment.SentMessage)
        {
            GroupChatFragment.SentMessage message = (GroupChatFragment.SentMessage) chatMessage;
            intent.putExtra("statusmap", ((GroupChatFragment.SentMessage) chatMessage).mMessageStatusMap);
        }
        else
        {
            Logger.d(TAG, "mMessage not instance of SentMessage ");
        }
        context.startActivity(intent);
        
    }

    public class MessageInfoDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "RepickDialog";
        private int mRequestCode = 0;
       String mdate;
       String mText;
       String mParticipant;
 
        public MessageInfoDialog(Date date, String text, String Participant)
        {
            mdate = date.toString();
            mText = text;
            if(Participant.equals(""))
                mParticipant = "You";
            else
                mParticipant = PhoneUtils.extractNumberFromUri(Participant);
          
        }
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {            
            final AlertDialog alertDialog;
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();            
            alertDialog.setTitle(mParticipant);
            StringBuilder mInfoMessage = new StringBuilder();
            String newline = System.getProperty("line.separator");          
            mInfoMessage.append("Message :" + mText);
            mInfoMessage.append(newline);
            mInfoMessage.append("Date :" + mdate);            
            alertDialog.setMessage(mInfoMessage);
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);            
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);            
            this.dismissAllowingStateLoss();
        }
    }


    @Override
    public void onDestroy() {
        Logger.d(TAG, "onDestroy");
        mPreMessageMap.clear();
        mPreviousDate = null;
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        mActivity = null;
        super.onDetach();
    }

    @Override
    protected void onSend(String message) {
        Logger.d(TAG, "onSend() The message is " + message);
        int messageTag = onSentMessage(message);
        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_SEND_MESSAGE, mTag, message);
        controllerMessage.arg1 = messageTag;
        Bundle bundle = new Bundle();
        bundle.putString("subject", mSubjectGroupChat);
        controllerMessage.setData(bundle);
        controllerMessage.sendToTarget();
    }

    private int onSentMessage(String content) {
        int messageTag = Utils.RANDOM.nextInt();
        InstantMessage message = new InstantMessage("", Utils.DEFAULT_REMOTE, content, true);
        message.setDate(new Date());
        ISentChatMessage sentChatMessage = addSentMessage(message, messageTag);
        if (sentChatMessage != null) {
            mPreMessageMap.put(messageTag, (SentMessage) sentChatMessage);
        }
        return messageTag;
    }

    protected boolean handleShowReminder(String reminder) {
        Logger.d(TAG, "handleShowReminder() reminder is " + reminder);
        if (SHOW_REJOING_MESSAGE_REMINDER.equals(reminder)) {
            if (mRejoiningText != null) {
                mRejoiningText.setVisibility(View.VISIBLE);
                super.handleClearReminder();
            } else {
                Logger.e(TAG, "handleShowReminder() the mRejoiningText is null");
            }
            return true;
        } else {
            if (mRejoiningText != null) {
                mRejoiningText.setVisibility(View.GONE);
            } else {
                Logger.e(TAG, "handleClearReminder() mTypingText is null");
            }
            return super.handleShowReminder(reminder);
        }
    }

    protected void handleClearReminder() {
        Logger.d(TAG, "handleClearReminder() entry");
        if (mRejoiningText != null) {
            mRejoiningText.setVisibility(View.GONE);
        } else {
            Logger.e(TAG, "showReminderList() mRejoiningText is null");
        }
        super.handleClearReminder();
    }

    @Override
    public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
        Logger.d(TAG, "addReceivedMessage() mIsBottom: " + mIsBottom + " message: " + message.getTextMessage()
                + " isRead: " + isRead);
        if (message != null) {
            final ReceivedMessage msg = new ReceivedMessage(message);
            Date date = message.getDate();
            if (mMessageAdapter == null) {
                Logger.d(TAG, "addReceivedMessage mMessageAdapter is null");
                return null;
            }
            addMessageDate(date);
            if (!mIsBottom) {
                String remote = ((ReceivedMessage) msg).getMessageSender();
                Logger.d(TAG, "addReceivedMessage() mIsBottom: " + mIsBottom + " remote: " + remote
                        + " mParticipantList: " + mParticipantList);
                if (remote != null) {
                    remote = PhoneUtils.extractNumberFromUri(remote);
                    String name = null;
                    if (mParticipantList != null) {
                        for (Participant participant : mParticipantList) {
                            Logger.d(TAG, "participant  = " + participant.toString());
                            if (participant.getContact().equals(remote)) {
                                name = participant.getDisplayName();
                                break;
                            }
                        }
                        Logger.d(TAG, "addReceivedMessage  the remote name is " + name);
                        Thread currentThread = Thread.currentThread();
                        if (THREAD_ID_MAIN == currentThread.getId()) {
                            mMessageReminderText.setText(SPACE);
                            if (name != null) {
                                mMessageReminderText.append(name);
                            } else {
                                if (getActivity() != null) {
                                    mMessageReminderText.append(getResources().getString(
                                            R.string.group_chat_stranger));
                                }
                                Logger.w(TAG, "name is null, so append stranger getActivity: "
                                        + getActivity());
                            }
                            mMessageReminderText.append(COLON);
                            String rcvMsg = ((ReceivedMessage) msg).getMessageText();
                            if (rcvMsg != null) {
                                mMessageReminderText.append(rcvMsg);
                            }
                            mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                            showReminderList();
                        } else {
                            final String contactName = name;
                            mUiHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    mMessageReminderText.setText(SPACE);
                                    if (contactName != null) {
                                        mMessageReminderText.append(contactName);
                                    }
                                    mMessageReminderText.append(COLON);
                                    String rcvMsg = ((ReceivedMessage) msg).getMessageText();
                                    if (rcvMsg != null) {
                                        mMessageReminderText.append(rcvMsg);
                                    }
                                    mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                                    showReminderList();
                                }
                            });
                        }
                        mIsNewMessageNotify = Boolean.TRUE;
                    }
                }
            }
            addMessageAtomic(msg);
            return msg;
        } else {
            Logger.d(TAG, "The received chat message is null");
            return null;
        }
    }

    
    @Override
    public void setFileTransferEnable(final int status) {
        Logger.d(TAG, " setFileTransferEnable() entry " + status);
        mFiletransferEnableStatus = status;
        if (mContentView != null) {
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    ImageButton btnAddView =
                            (ImageButton) mContentView.findViewById(R.id.btn_chat_add);
                    ApiManager manager = ApiManager.getInstance();
                    if (null != manager) {
                        switch (mFiletransferEnableStatus) {
                            case One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED:
                            case One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER:
                            case One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE:
                                btnAddView.setAlpha(ALPHA_VALUE_DISABLE);
                                btnAddView.setClickable(false);
                                btnAddView.setFocusable(false);
                                btnAddView.setEnabled(false);
                                break;
                            case One2OneChat.FILETRANSFER_ENABLE_OK:
                                btnAddView.setAlpha(ALPHA_VALUE_ENABLE);
                                btnAddView.setClickable(true);
                                btnAddView.setFocusable(true);
                                btnAddView.setEnabled(true);
                                btnAddView.setOnClickListener(new View.OnClickListener() {
                                    public void onClick(View v) {
                                        //queryCapablility();
                                        onAddAttachment();
                                    }
									
                                });
                                break;
                            default:
                                break;
                        }
                    } else {
                        Logger.e(TAG, "setFileTransferEnable() the manager is null");
                    }
                }
            });
        } else {
            Logger.e(TAG, "setFileTransferEnable the btnAddView is null");
        }
    }
    @Override
    public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
        Logger.d(TAG, "addFileTransferInvitation() file: " + file + " mRemoteIsRcse: "
                + "TODO" + " mIsBottom: " + mIsBottom + " mMessageAdapter: " + mMessageAdapter);
        FileTransfer fileTransfer = null;
        if (file == null) {
            return null;
        }
        fileTransfer = new ReceivedFileTransfer(file, isAutoAccept);
        /*if (!mRemoteIsRcse) {
            mRemoteIsRcse = true;
            final String number = mParticipant.getContact();
            Thread currentThread = Thread.currentThread();
            if (THREAD_ID_MAIN == currentThread.getId()) {
                Logger.w(TAG,
                        "addReceivedFileTransfer  the currentThread is " + currentThread.getId());
                if (ContactsListManager.getInstance().isLocalContact(number)) {
                    showAsLocal(number);
                } else if (ContactsListManager.getInstance().isStranger(number)) {
                    showAsStranger(number);
                } else {
                    ContactsListManager.getInstance().setStrangerList(number, true);
                    showAsStranger(number);
                }
            } else {
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Logger.w(TAG, "addReceivedFileTransfer  it is mUiHandler");
                        if (ContactsListManager.getInstance().isLocalContact(number)) {
                            showAsLocal(number);
                        } else if (ContactsListManager.getInstance().isStranger(number)) {
                            showAsStranger(number);
                        } else {
                            ContactsListManager.getInstance().setStrangerList(number, true);
                            showAsStranger(number);
                        }
                    }
                });
            }
        }*/
        Date date = file.mDate;
        addMessageAtomic(fileTransfer);

        if (null != fileTransfer && mMessageAdapter != null) {
            ((ReceivedFileTransfer) fileTransfer).setTag(mTag);
            if (!mIsBottom) {
                Thread currentThread = Thread.currentThread();
                if (mMessageReminderText != null) {

                    final String display =
                            ((ReceivedFileTransfer) fileTransfer).getFileStruct().mName + SPACE
                                    + getString(R.string.file_transfer_titile_after);
                    if (THREAD_ID_MAIN == currentThread.getId()) {
                        Logger.w(TAG, "addReceivedMessage  the currentThread is "
                                + currentThread.getId());
                        mMessageReminderText.setText(display);
                        mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                        showReminderList();
                    } else {
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                Logger.w(TAG, "addReceivedMessage  it is mUiHandler");
                                mMessageReminderText.setText(display);
                                mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                                showReminderList();
                            }
                        });
                    }
                    mIsNewMessageNotify = true;
                }
            }
        }
        return fileTransfer;
    }
    public class SentFileTransfer extends FileTransfer {
        public SentFileTransfer(FileStruct fileStruct) {
            super(fileStruct);
            mStatus = Status.PENDING;
        }
        
       
        
    }

    public class ReceivedFileTransfer extends FileTransfer {
        public ReceivedFileTransfer(FileStruct fileStruct, boolean isAutoAccept) {
            super(fileStruct);
            if(isAutoAccept)
                mStatus = Status.TRANSFERING;           

        }
        
        
    }
    @Override
    public IFileTransfer addSentFileTransfer(FileStruct file) {
        Logger.d(TAG, "addSentFileTransfer()  file: " + file);
        Object fileTransferTag = file.mFileTransferTag;
        SentFileTransfer fileTransfer = (SentFileTransfer)mPreFileTransferMap.get(fileTransferTag);
        if (null != fileTransfer) {
            Logger.d(TAG, "addSentFileTransfer()  fileTransfer with tag: " + fileTransferTag
                    + " found");
            mPreFileTransferMap.remove(fileTransferTag);
        } else {
            Logger.d(TAG, "addSentFileTransfer()  fileTransfer with tag: " + fileTransferTag
                    + " not found");
            fileTransfer = new SentFileTransfer(file);
            if (null != fileTransfer && mMessageAdapter != null) {
                fileTransfer.setTag(this.getChatFragmentTag());
                Date date = file.mDate;
                addMessageAtomic(fileTransfer);
            } else {
                Logger.e(TAG, "addSentFileTransfer() fileTransfer is null");
            }
        }
        return fileTransfer;
    }
    
    
    @Override
    public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
        Logger.d(TAG, "addSentMessage(), message: " + message);
        if (message != null) {
            SentMessage msg = (SentMessage) mPreMessageMap.get(messageTag);
            if (null == msg) {
                msg = new SentMessage(message);
                Date date = message.getDate();
                if (mMessageAdapter == null) {
                    Logger.d(TAG, "addSentMessage mMessageAdapter is null");
                    return null;
                }
                addMessageDate(date);
                addMessageAtomic(msg);
                return msg;
            } else {
                String messageRemote = message.getRemote();
                Logger.d(TAG, "addSentMessage(), messageRemote: " + messageRemote);
                if (!Utils.DEFAULT_REMOTE.equals(messageRemote)) {
                    mPreMessageMap.remove(messageTag);
                    msg.updateMessage(message);
                    return msg;
                }
            }
        }
        return null;
    }

    /*private interface IChatMessage extends IChatWindowMessage
    {
           Date getMessageDate();
    }*/

    public static class DateMessage implements IChatMessage
    {
        private Date mDate = null;
        
        public DateMessage(Date date)
        {
            mDate = (Date) date.clone();
        }

        @Override
        public String getId() {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public Date getMessageDate() {
            // TODO Auto-generated method stub
            return (Date)mDate.clone();
        }       
        
        @Override
        public boolean equals(Object o) {
            if (o instanceof DateMessage) {
                return mDate.equals(((DateMessage) o).mDate);
            } else {
                return false;
            }

        }

        @Override
        public int hashCode() {
            return mDate.hashCode();
        }

    }

    /**
     * Add message date in the ListView.Messages sent and received in the same
     * day have only one date.
     * 
     * @param date The date of the messages. Each date stands for a section in
     *            fastScroll.
     */
    public void addMessageDate(Date date) {
        Logger.d(TAG, "updateDateList() entry, the size of dateList is " + mDateList.size()
                + " and the date is " + date);
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT_STRING);
        String currentDate = dateFormat.format(date);
        Logger.d(TAG, "currentDate is " + currentDate);
        Date dateMsg = new Date(date.getYear(),date.getMonth(),date.getDate(),0,0,0);
        if (mPreviousDate == null || !mPreviousDate.equals(currentDate)) {
            if (mMessageAdapter == null) {
                Logger.d(TAG, "addMessageDate mMessageAdapter is null");
                return;
            }
            if (mMessageList == null) {
                Logger.d(TAG, "addMessageDate mMessageAdapter is null");
                return;
            }
            synchronized (mLock) {
                DateMessage dateMessage = new DateMessage(dateMsg);
                mMessageSequenceOrder = mMessageSequenceOrder + 1;
                int position = mMessageSequenceOrder;
                mMessageAdapter.addMessage(date, position);                
                mDateList.add(currentDate);
                mMessageList.add(position, dateMessage);
            }
        }
        mPreviousDate = currentDate;
    }

    @Override
    public void removeAllMessages() {
        Logger.d(TAG, "removeAllMessages() entry, mMessageAdapter: " + mMessageAdapter);
        if (mMessageAdapter != null) {
            mMessageAdapter.removeAllItems();
        }
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                mMessageList.clear();
                mDateList.clear();
            }
        });
        mPreviousDate = null;
        mMessageSequenceOrder = -1;
        Logger.d(TAG, "removeAllMessages exit");
    }

    @Override
    public IChatWindowMessage getSentChatMessage(String messageId) {
        for (IChatWindowMessage message : mMessageList) {
            if (message.getId().equals(messageId)) {
                return message;
            }
        }
        return null;
    }

    
    public void closeGroupChat()
    {
    	//find out current window? TODO
    	//only this participant remains in chat, others have left, chat window should close
    	Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_QUIT_GROUP_CHAT, mTag, null);                         
                               
        controllerMessage.sendToTarget(); 
        mUiHandler.post(new Runnable() {
			
			public void run() {
        removeChatUi();
			        hideSoftKeyboard();

    }
		});
     
        //show a reminder that group chat need to end due to all participant has left
        showAllParticipantLeftReminder();
    }
    
    public void showAllParticipantLeftReminder() {
        Logger.v(TAG, "showAllParticipantLeftReminder() mGroupParticipantsLeftText: " + mGroupParticipantsLeftText + " getActivity: "
                + getActivity());
        if (mGroupParticipantsLeftText != null) {
            if (getActivity() != null) {
                
                        final String groupParticipantsLeftText = getResources().getString(
                                R.string.label_all_participants_left);
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                            	mGroupParticipantsLeftText.setText(groupParticipantsLeftText);
                                mTextReminderSortedSet.add(SHOW_ALL_PARTICIPANTS_LEFT);
                                showReminderList();
                            }
                        });
                        
                    }
               
                }
            }
        
    
    public void hideSoftKeyboard() {
        InputMethodManager inputMethodManager = (InputMethodManager) getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
        Logger.d(TAG, "hideSoftKeyboard(), inputMethodManager is: " + inputMethodManager);
        if (inputMethodManager != null) {
            View view = getActivity().getCurrentFocus();
            Logger.d(TAG, "hideSoftKeyboard(), view is: " + view);
            if (view != null) {
                IBinder binder = view.getWindowToken();
                inputMethodManager.hideSoftInputFromWindow(binder,
                        InputMethodManager.HIDE_NOT_ALWAYS);
            }
        }
    }
    
    @Override
    public void updateParticipants(final List<ParticipantInfo> participantList) {
        Logger.d(TAG, "updateParticipants entry");
        if (mActivity == null) {
            Logger.d(TAG, "updateParticipants mActivity is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ArrayList<Participant> participants = new ArrayList<Participant>();
                String blockedParticipants = null;
                mGroupChatParticipantList.clear();
                mGroupChatParticipantList.addAll(participantList);
                if(mGroupChatParticipantList.size() == 0)//zero size
                {
                	//only this participant remains in chat, others have left, chat window should close
                	closeGroupChat();
                }
                Logger.d(TAG, "updateParticipants() mGroupChatParticipantList: "
                        + mGroupChatParticipantList);
                for (ParticipantInfo info : participantList) {
                    participants.add(info.getParticipant());
                }
                Logger.d(TAG, "updateParticipants() participants: " + participants);
                mParticipantList = participants;
                updateChatUi();
                if (Logger.getIsIntegrationMode()) {
                    ((PluginGroupChatActivity) mActivity).updateParticipants(mParticipantList);
                } else {
                    Logger.d(TAG, "updateParticipants() it is not integration mode.");
                }
                
                //if blocked contact is added, update top reminder
                List<String> blockList = ContactsManager.getInstance().getImBlockedContactsFromLocal();
                for(Participant participant: mParticipantList)
                {
                	if(blockList.contains(participant.getContact()))
                	{
                		//blockedParticipantsGroup.add(participant);
                		blockedParticipants = blockedParticipants + " " + participant.getDisplayName();
                	}
                }
                
                if (mBlockedParticipantText != null  && blockedParticipants != null) {
                	mBlockedParticipantText.setText("Blocked participant" + blockedParticipants + " is added to Group chat");
                    mTopReminderSortedSet.add(SHOW_BLOCKED_PARTICIPANT_MESSAGE_REMINDER);
                    showTopReminder();
                } else {
                    Logger.e(TAG, " mGroupParticipantsLeftText is null!");
                }
                
                
            }
        });
        Logger.d(TAG, "updateParticipants exit");
    }

    /**
     * Notify controller participants has been added.
     * 
     * @param participantList The participants to be added.
     */
    public void addParticipants(List<Participant> participantList) {
        Logger.d(TAG, "addParticipants entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_GROUP_ADD_PARTICIPANT, mTag, participantList);
        controllerMessage.sendToTarget();
    }

    @Override
    public void setIsComposing(boolean isComposing, final Participant participant) {
        Logger.v(TAG, "setIsComposing status is " + isComposing + "participant is" + participant);
        if (mTypingText != null) {
            if (isComposing) {
                if (participant != null) {
                    mParticipantComposList.add(participant);
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            showComposingInformation();
                        }
                    });
                } else {
                    Logger.e(TAG, "setIsComposing the participant is null");
                }
            } else {
                if (participant != null) {
                    int listSize = mParticipantComposList.size();
                    if (listSize > 0) {
                        mParticipantComposList.remove(participant);
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                showComposingInformation();
                            }
                        });
                    } else {
                        Logger.e(TAG, "setIsComposing false the listSize is " + listSize);
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                mTextReminderSortedSet.remove(SHOW_IS_TYPING_REMINDER);
                                showReminderList();
                            }
                        });
                    }
                } else {
                    Logger.e(TAG, "setIsComposing false participant is null");
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mTextReminderSortedSet.remove(SHOW_IS_TYPING_REMINDER);
                            showReminderList();
                        }
                    });
                }
            }
        } else {
            Logger.e(TAG, "setIsComposing the typingtext is null");
        }
    }

    private void showComposingInformation() {
        String moreTyping = getResources().getString(R.string.label_contact_imore_composing);
        ArrayList<Participant> tmpList = new ArrayList<Participant>(mParticipantComposList);
        int listSize = tmpList.size();
        Logger.w(TAG, "setIsComposing + listSize" + listSize);
        if (listSize == 0) {
            mTextReminderSortedSet.remove(SHOW_IS_TYPING_REMINDER);
            showReminderList();
        } else if (listSize == 1) {
            Participant participant = tmpList.get(ZERO_PARTICIPANT);
            final String isTyping =
                    getResources().getString(
                            R.string.label_contact_is_composing,
                            ContactsListManager.getInstance().getDisplayNameByPhoneNumber(participant
                                    .getContact()));
            mTypingText.setText(isTyping);
            mTextReminderSortedSet.add(SHOW_IS_TYPING_REMINDER);
            showReminderList();
        } else if (listSize > 1) {
            mTypingText.setText(moreTyping);
            mTextReminderSortedSet.add(SHOW_IS_TYPING_REMINDER);
            showReminderList();
        }
    }

    private void updateChatUi() {
        Logger.d(TAG, "updateChatUi entry");
        Activity activity = getActivity();
        if (activity != null) {
            LayoutInflater inflater = LayoutInflater.from(activity.getApplicationContext());
            View customView = inflater.inflate(R.layout.group_chat_screen_title, null);
            activity.getActionBar().setCustomView(customView);
            setChatScreenTitle();
            TextView groupTitle =
                    (TextView) activity.findViewById(R.id.peer_name);
            groupTitle.setOnClickListener(this);           
            
            RelativeLayout groupChatTitleLayout =
                    (RelativeLayout) activity.findViewById(R.id.group_chat_title_layout);
            groupChatTitleLayout.setOnClickListener(this);
            ImageButton expandGroupChat =
                    (ImageButton) activity.findViewById(R.id.group_chat_expand);
            expandGroupChat.setOnClickListener(this);            
            FragmentManager fragmentManager = getFragmentManager();
            FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
            fragmentTransaction.show(this);
            fragmentTransaction.commitAllowingStateLoss();
            fragmentManager.executePendingTransactions();
            addGroupChatMembersIcon();
            activity.invalidateOptionsMenu();
            Logger.d(TAG, "updateGroupChatUi exit");
        } else {
            Logger.w(TAG, "activity is null.");
        }
    }

    /**
     * When switch ChatFragment this method should be called to remove ui.
     */
    public void removeChatUi() {
        Activity activity = getActivity();
        if (activity != null) {
            HorizontalScrollView groupChatBannerScroller =
                    (HorizontalScrollView) activity.findViewById(R.id.group_chat_banner_scroller);
            if (groupChatBannerScroller != null) {
                Logger.v(TAG, "groupChatBannerScroller is not null");
                groupChatBannerScroller.setVisibility(View.GONE);
            } else {
                Logger.v(TAG, "groupChatBannerScroller is null.");
            }
            activity.getActionBar().setCustomView(null);
        } else {
            Logger.w(TAG, "activity is null.");
        }
    }

    public void addGroupChatMembersIcon() {
        List<ParticipantInfo> participantInfos =
                new ArrayList<ParticipantInfo>(mGroupChatParticipantList);
        mParticipantListDisplayer.updateBanner(participantInfos);
    }

    /**
     * Set chat screen's title.
     */
    public void setChatScreenTitle() {
        Logger.v(TAG, "setChatScreenTitle()");
        Activity activity = getActivity();
        if (activity != null) {
            int num = getParticipantsNum();
            if (num > ChatFragment.ONE) {
                if(getmSubjectGroupChat().equals(""))
                setGroupChatTitleNumbers();
            }
            TextView titleView = (TextView) activity.findViewById(R.id.peer_name);
            TextView numView = (TextView) activity.findViewById(R.id.peer_number);
            Logger.w(TAG, "setChatScreenTitle() num: " + num + " titleView: " + titleView);
            if (titleView != null) {
                if (null != mParticipantList && mParticipantList.size() > 0) {
                    if(getmSubjectGroupChat().equals(""))
                    titleView.setText(getParticipantsName(mParticipantList
                            .toArray(new Participant[1])));
                    else
                    {
                        titleView.setText(EmoticonsModelImpl.getInstance().formatMessage(mSubjectGroupChat));                        
                        setGroupChatTitleNumbers();
                    }
                }
            }
        }
    }

    private String getActiveUserCount()
    {
        int activeCount = 0;
        int inActiveCount = 0;
        String activeUserCount = "";
        for (int i = 0; i< mGroupChatParticipantList.size();i++)
        {
            if(mGroupChatParticipantList.get(i).getState().equals(User.STATE_CONNECTED))
                activeCount++;
            else
                inActiveCount++;  
        }
        activeUserCount =  "" + activeCount + "/" + mGroupChatParticipantList.size(); 
        return activeUserCount;
    }

    /**
     * Set group chat members's number.
     * 
     * @param num The participants number in your group chat.
     */
    private void setGroupChatTitleNumbers() {
        Logger.v(TAG, "setGroupChatTitleNumbers(),num = ");
        Activity activity = getActivity();
        if (activity != null) {
            TextView numView = (TextView) activity.findViewById(R.id.peer_number);
            Logger.w(TAG, "setGroupChatTitleNumbers() numView: " + numView);
            if (numView != null) {
                String numStr = ChatFragment.OPEN_PAREN + getActiveUserCount() + ChatFragment.CLOSE_PAREN;
                numView.setText(numStr);
            }
        }
    }

    public void expandGroupChat() {
        Logger.v(TAG, "expandGroupChat() entry");
        mParticipantListDisplayer.expand();
    }

    public void collapseGroupChat() {
        Logger.v(TAG, "collapseGroupChat() entry");
        mParticipantListDisplayer.collapse();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Logger.v(TAG, "onActivityResult() requestCode = " + requestCode + ",resultCode = "
                + resultCode + ",data = " + data);
        if (requestCode == RESULT_CODE_ADD_CONTACTS) {
            if (data != null) {
                ArrayList<Participant> participantList = ContactsListManager.getInstance()
                        .parseParticipantsFromIntent(data);
                Logger.d(TAG, "onActivityResult() participantList is " + participantList);
                if (participantList != null && participantList.size() != 0) {
                    participantList.addAll(0, mParticipantList);
                    addContactsToGroupChat(participantList);
                } else {
                    Logger.w(TAG,
                            "onActivityResult() participantList size is 0,so do not add member to group chat");
                }
            }
        }
    }

    private void addContactsToGroupChat(ArrayList<Participant> participantList) {
        Logger.v(TAG, "addContactsToGroupChat");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_GROUP_ADD_PARTICIPANT, mTag, participantList);
        controllerMessage.sendToTarget();
    }

    protected void addContactsToExistChatWindow(List<Participant> participantList) {
        int size = participantList == null ? 0 : participantList.size();
        Logger.w(TAG, "size = " + size);
        if (size == 0) {
            Logger.w(TAG, "participantList is null");
            return;
        }
        addParticipants(participantList);
    }

    /**
     * This is an Information for chat event
     */
    public static class ChatEventInformation implements IChatEventInformation {
        protected ChatEventStruct mChatEventStruct = null;

        public ChatEventInformation(ChatEventStruct chatEventStruct) {
            mChatEventStruct = chatEventStruct;
        }

        public Information getInformation() {
            if (mChatEventStruct != null) {
                return mChatEventStruct.information;
            } else {
                Logger.e(TAG, "getInformation the mChatEventStruct is null");
                return null;
            }
        }

        public Object getRelatedInfo() {
            if (mChatEventStruct != null) {
                return mChatEventStruct.relatedInformation;
            } else {
                Logger.e(TAG, "getInformation the mChatEventStruct is null");
                return null;
            }
        }

        public Date getDate() {
            if (mChatEventStruct != null) {
                return mChatEventStruct.date;
            } else {
                Logger.e(TAG, "getInformation the mChatEventStruct is null");
                return null;
            }
        }
    }  


    public class SentMessage implements ISentChatMessage, IChatMessage  {
        private InstantMessage mMessage = null;

        private Status mStatus = Status.SENDING;

        public ConcurrentHashMap<String, Integer> mMessageStatusMap = new ConcurrentHashMap<String, Integer>();
        
        public SentMessage(InstantMessage msg) {
            mMessage = msg;
            for(Participant participant : mParticipantList)
            {
            	mMessageStatusMap.put(participant.getContact(), Status.UNKNOWN.ordinal() );
        }
        }
        
        
        
        /*public Map<String, ISentChatMessage.Status> getStatusMap()
        {
        	return mMessageStatusMap;
        }*/

        public int getMessageTag() {
            // TODO Auto-generated method stub
            return 0;
        }

        @Override
        public String getId() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no id returned");
                return null;
            }
            return mMessage.getMessageId();
        }

        @Override
        public void updateStatus(Status s) {
            final Status status = s;
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    mStatus = status;
                    if (mMessageAdapter != null) {
                        mMessageAdapter.notifyDataSetChanged();
                    }
                }
            });
        }

        /**
         * Get message text.
         * 
         * @return Message text.
         */
        public String getMessageText() {
            if (mMessage == null) {
                return null;
            }
            return mMessage.getTextMessage();
        }

        /**
         * Get status of message.
         * 
         * @return status of message.
         */
        public Status getStatus() {
            return mStatus;
        }

        /**
         * Return the time when then message was sent.
         * 
         * @return Message sent time.
         */
        public Date getMessageDate() {
            if (mMessage != null) {
                return mMessage.getDate();
            } else {
                Logger.d(TAG, "getMessageText mMessage is null");
                return null;
            }
        }

        protected void updateMessage(InstantMessage message) {
            Logger.d(TAG, "updateMessage() message: " + message);
            mMessage = message;
        }

        @Override
        public void updateDate(Date date) {
            //Do nothing
        }

		public void updateStatus(Status s, String contact) {
			
            //update the map
            synchronized(mMessageStatusMap){
            	mMessageStatusMap.put(contact, s.ordinal());
                    }
			
                }

			
		}

    /**
     * ReceivedMessage provided for window to get message.
     */
    public static class ReceivedMessage implements IReceivedChatMessage, IChatMessage  {
        private Status mStatus = Status.SENDING;

        private InstantMessage mMessage = null;

        public ReceivedMessage(InstantMessage msg) {
            mMessage = msg;
        }

        @Override
        public String getId() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no id returned");
                return null;
            }
            return mMessage.getMessageId();
        }

        /**
         * Return the message text.
         * 
         * @return Message text.
         */
        public String getMessageText() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no text returned");
                return null;
            }
            return mMessage.getTextMessage();
        }

        /**
         * Return the time when then message was sent.
         * 
         * @return Message sent time.
         */
        public Date getMessageDate() {
            if (mMessage != null) {
                return mMessage.getDate();
            } else {
                Logger.d(TAG, "getMessageText mMessage is null");
                return null;
            }
        }

        /**
         * Return who send this message.
         * 
         * @return The sender.
         */
        public String getMessageSender() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no remote returned");
                return null;
            }
            return mMessage.getRemote();
        }

        /**
         * Get status of message.
         * 
         * @return status of message.
         */
        public Status getStatus() {
            return mStatus;
        }
    }

    @Override
    public void addLoadHistoryHeader(boolean showHeader) {
        // TODO Auto-generated method stub

    }

    @Override
    protected void onAdapterPrepared() {
        // TODO Auto-generated method stub

    }

    @Override
    protected int getFragmentResource() {
        return R.layout.chat_fragment_group;
    }

    @Override
    public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
        if (chatEventStruct != null) {
            Logger.d(TAG, "addChatEventInformation chatEventStruct is " + chatEventStruct);
            Date date = chatEventStruct.date;
            addMessageDate(date);
            IChatEventInformation chatEvent = new ChatEventInformation(chatEventStruct);
            Information information = ((ChatEventInformation) chatEvent).getInformation();
            switch (information) {
                case LEFT:
                case JOIN:
                    addChatEventInfo(chatEvent);
                    break;
                default:
                    Logger.e(TAG,
                            "addChatEventInformation the information is not defined and it is "
                                    + information);
                    break;
            }
            return null;
        } else {
            Logger.d(TAG, "The sent chat message is null");
            return null;
        }
    }

    private IChatEventInformation addChatEventInfo(IChatEventInformation chatEvent) {
        Logger.d(TAG, "addChatEventInfo entry");
        if (mMessageAdapter != null) {
            mMessageAdapter.addMessage(chatEvent);
            return chatEvent;
        } else {
            Logger.d(TAG, "addChatEventInformation mMessageAdapter is null");
            return null;
        }
    }

    @Override
    public void updateChatStatus(final int status) {
        Logger.d(TAG, "updateChatStatus() status: " + status);
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                final Activity activity = getActivity();
                if (activity == null) {
                    Logger.d(TAG, "updateChatStatus() activity is null");
                    return;
                }
                switch (status) {
                    case Utils.GROUP_STATUS_TERMINATED:
						mBtnSend.setVisibility(View.GONE);
						mMessageEditor.setVisibility(View.GONE);
                        mBtnEmotion.setVisibility(View.GONE);                      
                        mBtnSend.setOnClickListener(new View.OnClickListener() {
                            public void onClick(View v) {
                                Toast.makeText(activity, getString(R.string.group_terminated),
                                        Toast.LENGTH_SHORT).show();
                            }
                        });
                        break;
                    case Utils.GROUP_STATUS_REJOINING:
                        mBtnSend.setClickable(true);
                        mBtnSend.setOnClickListener(new View.OnClickListener() {
                            public void onClick(View v) {
                                Toast.makeText(activity, getString(R.string.group_rejoining),
                                        Toast.LENGTH_SHORT).show();
                            }
                        });
                        break;
                    case Utils.GROUP_STATUS_RESTARTING:
                        mBtnSend.setClickable(true);
                        mBtnSend.setOnClickListener(new View.OnClickListener() {
                            public void onClick(View v) {
                                Toast.makeText(activity, getString(R.string.group_restarting),
                                        Toast.LENGTH_SHORT).show();
                            }
                        });
                        break;
                    case Utils.GROUP_STATUS_CANSENDMSG:
						mBtnSend.setVisibility(View.VISIBLE);
						mMessageEditor.setVisibility(View.VISIBLE);
                        mBtnEmotion.setVisibility(View.VISIBLE);
                        mBtnSend.setClickable(true);
                        mMessageEditor.setFocusable(true);
                        mMessageEditor.setClickable(true);
                        mBtnEmotion.setClickable(true);
                        mBtnSend.setOnClickListener(mBtnSendClickListener);
                        break;
                    case Utils.GROUP_STATUS_UNAVIALABLE:
                        mBtnSend.setClickable(false);
                        mMessageEditor.setFocusable(false);
                        mMessageEditor.setClickable(false);
                        mBtnEmotion.setClickable(false);                        
                        mBtnSend.setOnClickListener(new View.OnClickListener() {
                            public void onClick(View v) {
                                Toast.makeText(activity, getString(R.string.group_terminated),
                                        Toast.LENGTH_SHORT).show();
					}
                        });
                        break;
                    default:
                        break;
                }
            }
        });
    }
    

    @Override
    public void updateAllMsgAsRead() {
        // Do Nothing
    }

    private void loadDimension(Resources resources) {
        mGroupMemberHorMarginLeft =
                resources.getDimensionPixelSize(R.dimen.group_member_hor_margin_left);
        mGroupMemberHorMarginRight =
                resources.getDimensionPixelSize(R.dimen.group_member_hor_margin_right);
        mGroupMemberHorHeight = resources.getDimensionPixelSize(R.dimen.group_member_hor_width);
        mGroupMemberHorWidth = resources.getDimensionPixelSize(R.dimen.group_member_hor_height);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mParticipantListDisplayer.onScreenSwitched(newConfig.orientation);
    }

    private ParticipantListDisplayer mParticipantListDisplayer = new ParticipantListDisplayer();

    /**
     * Defines a common interface for ParticipantListDisplayer
     */
    private interface IDisplayStrategy {
        void expand();

        void collapse();

        void show();

        void dismiss();

        void updateBanner(List<ParticipantInfo> participantInfos);
    }

    /**
     * This is a helper class, to manage the participant list banner
     */
    private class ParticipantListDisplayer {
        private static final String TAG = "ParticipantListDisplayer";

        private IDisplayStrategy mCurrentStrategy = null;

        private final LandscapeStrategy mLandscapeStrategy = new LandscapeStrategy();
        private final PortraitStrategy mPortraitStrategy = new PortraitStrategy();

        private List<ParticipantInfo> mParticipantInfos = null;

        public void onFragmentCreate(int orientation) {
            Logger.d(TAG, "onActivityCreate entry, orientation: " + orientation);
            onStatusUpdated(orientation, false);
        }

        public void expand() {
            if (null != mCurrentStrategy) {
                mCurrentStrategy.expand();
            } else {
                Logger.w(TAG, "expand() mCurrentStrategy is null");
            }
        }

        public void collapse() {
            if (null != mCurrentStrategy) {
                mCurrentStrategy.collapse();
            } else {
                Logger.w(TAG, "collapse() mCurrentStrategy is null");
            }
        }

        /**
         * This method should only be called from the main thread to update the
         * banner
         * 
         * @param participantInfos The latest participant list
         */
        public void updateBanner(List<ParticipantInfo> participantInfos) {
            mParticipantInfos = participantInfos;
            if (null != mCurrentStrategy) {
                mCurrentStrategy.updateBanner(participantInfos);
                if (mCurrentStrategy.equals(mPortraitStrategy)) {
                    mPortraitStrategy.show();
                    mLandscapeStrategy.dismiss();
                } else {
                    mPortraitStrategy.dismiss();
                    mLandscapeStrategy.show();
                }
            } else {
                Logger.w(TAG, "updateBanner() mCurrentStrategy is null");
            }
        }

        public void onScreenSwitched(int orientation) {
            onStatusUpdated(orientation, true);
        }

        public void onStatusUpdated(int orientation, boolean isNeedUpdate) {
            Logger.d(TAG, "onScreenSwitched entry, orientation: " + orientation);
            switch (orientation) {
                case Configuration.ORIENTATION_LANDSCAPE:
                    mPortraitStrategy.dismiss();
                    mLandscapeStrategy.show();
                    mCurrentStrategy = mLandscapeStrategy;
                    break;
                case Configuration.ORIENTATION_PORTRAIT:
                    mPortraitStrategy.show();
                    mLandscapeStrategy.dismiss();
                    mCurrentStrategy = mPortraitStrategy;
                    break;
                default:
                    Logger.w(TAG, "onScreenSwitched() unknown orientation: " + orientation);
                    break;
            }
            if (isNeedUpdate) {
                mCurrentStrategy.updateBanner(mParticipantInfos);
            }
        }
    }

    /**
     * The strategy used in Landscape screen
     */
    private class LandscapeStrategy extends BaseAdapter implements IDisplayStrategy {
        private static final String TAG = "LandscapeStrategy";

        private ListView mBanner = null;
        private View mArea = null;
        private LayoutInflater mInflator = null;
        private final List<ParticipantInfo> mParticipantInfoList = new ArrayList<ParticipantInfo>();

        private void checkArea() {
            if (null != mBanner) {
                Logger.d(TAG, "checkArea() already initialized");
            } else {
                Logger.d(TAG, "checkArea() not initialized");
                mArea = mContentView.findViewById(R.id.participant_list_area);
                mBanner = (ListView) mContentView.findViewById(R.id.participant_banner);
                mBanner.setAdapter(this);
                mInflator = LayoutInflater.from(getActivity().getApplicationContext());
            }
        }

        @Override
        public void collapse() {
            Logger.v(TAG, "collapse() entry");
            // Do nothing
        }

        @Override
        public void dismiss() {
            Logger.v(TAG, "dismiss() entry");
            checkArea();
            mArea.setVisibility(View.GONE);
        }

        @Override
        public void expand() {
            Logger.v(TAG, "expand() entry");
            // Do nothing
        }

        @Override
        public void show() {
            Logger.v(TAG, "show() entry");
            checkArea();
            mArea.setVisibility(View.VISIBLE);
        }

        @Override
        public void updateBanner(List<ParticipantInfo> participantInfos) {
            Logger.d(TAG, "updateBanner() entry, participantInfos: " + participantInfos);
            mParticipantInfoList.clear();
            mParticipantInfoList.addAll(participantInfos);
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return mParticipantInfoList.size();
        }

        @Override
        public Object getItem(int position) {
            return mParticipantInfoList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, final View convertView, ViewGroup parent) {
            Logger.v(TAG, "getView() entry, position: " + position);
            View itemView = convertView;
            if (null == itemView) {
                Logger.d(TAG, "getView() inflate a new item view");
                itemView = mInflator.inflate(R.layout.participant_list_item_vertical, null);
            } else {
                Logger.d(TAG, "getView() use convertView");
            }
            bindView(itemView, mParticipantInfoList.get(position));
            Logger.v(TAG, "getView() exit");
            return itemView;
        }

        private void bindView(View itemView, ParticipantInfo info) {
            Logger.d(TAG, "bindView() info:" + info);
            String contact = info.getContact();
            AsyncImageView avatar = (AsyncImageView) itemView.findViewById(R.id.peer_avatar);
            boolean active = User.STATE_CONNECTED.equals(info.getState());
            avatar.setAsyncContact(contact, !active);
            TextView statusView = (TextView) itemView.findViewById(R.id.participant_status);
            statusView.setText(active ? getString(R.string.group_active)
                    : getString(R.string.group_inactive));
            TextView remoteName = (TextView) itemView.findViewById(R.id.remote_name);
            remoteName.setText(ContactsListManager.getInstance().getDisplayNameByPhoneNumber(contact));
        }
    }

    /**
     * The strategy used in Portrait screen
     */
    private class PortraitStrategy implements IDisplayStrategy {

        private static final String TAG = "PortraitStrategy";
        private boolean mIsExpand = false;

        @Override
        public void collapse() {
            Logger.v(TAG, "collapse() entry");
            Activity activity = getActivity();
            if (null != activity) {
                setGroupChatBannerScrollerVisibility(activity, View.GONE);
                setGroupChatCollapseVisibility(activity, View.GONE);
                setGroupChatExpandVisibility(activity, View.VISIBLE);
            } else {
                Logger.w(TAG, "collapse() activity is null");
            }
            mIsExpand = false;

        }

        @Override
        public void dismiss() {
            Logger.v(TAG, "dismiss() entry");
            Activity activity = getActivity();
            if (null != activity) {
                setGroupChatBannerScrollerVisibility(activity, View.GONE);
                setGroupChatCollapseVisibility(activity, View.GONE);
                setGroupChatExpandVisibility(activity, View.GONE);
            } else {
                Logger.w(TAG, "dismiss() activity is null");
            }

        }

        @Override
        public void expand() {
            Logger.v(TAG, "expand() entry");
            Activity activity = getActivity();
            if (null != activity) {
                setGroupChatBannerScrollerVisibility(activity, View.VISIBLE);
                setGroupChatCollapseVisibility(activity, View.VISIBLE);
                setGroupChatExpandVisibility(activity, View.GONE);
            } else {
                Logger.w(TAG, "expand() activity is null");
            }
            mIsExpand = true;

        }

        @Override
        public void show() {
            if (mIsExpand) {
                Logger.d(TAG, "show() mIsExpand is true");
                expand();
            } else {
                Logger.d(TAG, "show() mIsExpand is false");
                collapse();
            }
        }

        @Override
        public void updateBanner(List<ParticipantInfo> participantInfos) {
            Logger.d(TAG, "updateBanner() entry, participantInfos: " + participantInfos);
            final Activity activity = getActivity();
            if (activity != null) {
                LinearLayout grouChatMemberIconsLayout =
                        (LinearLayout) activity.findViewById(R.id.group_chat_banner_container);
                if (grouChatMemberIconsLayout == null) {
                    Logger.w(TAG, "updateBanner() grouChatMemberIconsLayout is null");
                    return;
                }
                final int childCount = grouChatMemberIconsLayout.getChildCount();
                LayoutParams layoutParams =
                        new LayoutParams(mGroupMemberHorWidth, mGroupMemberHorHeight);
                layoutParams
                        .setMargins(mGroupMemberHorMarginLeft, 0, mGroupMemberHorMarginRight, 0);
                int num = 0;

                if (participantInfos != null) {
                    num = participantInfos.size();
                    Logger.d(TAG, "updateBanner() num: " + num + " , childCount: " + childCount);
                    int i = 0;
                    for (; i < num; ++i) {
                        Logger.d(TAG, "updateBanner() current i: " + i);
                        ParticipantInfo participant = participantInfos.get(i);
                        if (i < childCount) {
                            Logger.d(TAG, "updateBanner() use an existing view");
                            getView(activity, participant,
                                    (AsyncImageView) grouChatMemberIconsLayout.getChildAt(i));
                        } else {
                            Logger.d(TAG, "updateBanner() need to inflate a new view");
                            View itemView = getView(activity, participant, null);
                            if (null != itemView) {
                                grouChatMemberIconsLayout.addView(itemView, layoutParams);
                            } else {
                                Logger.w(TAG, "updateBanner() inflate failed");
                            }
                        }
                    }
                    Logger.d(TAG, "updateBanner() add view done, i: " + i);
                    int invalidItemCount = childCount - i;
                    if (invalidItemCount > 0) {
                        Logger.d(TAG, "updateBanner() need to remove child view from " + i
                                + " count " + invalidItemCount);
                        grouChatMemberIconsLayout.removeViews(i, invalidItemCount);
                    } else {
                        Logger.d(TAG, "updateBanner() no need to remove child view");
                    }
                } else {
                    Logger.e(TAG, "updateBanner() the participantInfos is null");
                }
            } else {
                Logger.w(TAG, "updateBanner() activity is null.");
            }
        }

        private View getView(final Context context, final ParticipantInfo info,
                final AsyncImageView convertView) {
            Logger.d(TAG, "getView() info: " + info + " , convertView: " + convertView);
            if (null == info) {
                Logger.w(TAG, "getView() info is null");
                return null;
            }
            AsyncImageView itemView = convertView;
            if (null == itemView) {
                itemView = inflateView(context);
            }
            String state = info.getState();
            String contact = info.getContact();
            Logger.d(TAG, "getView(), contact: " + contact + ", state: " + state);
            boolean isGrey = !(User.STATE_CONNECTED.equals(info.getState()));
            itemView.setAsyncContact(contact, isGrey);
            return itemView;
        }

        private AsyncImageView inflateView(Context context) {
            return new AsyncImageView(context);
        }

        private void setGroupChatBannerScrollerVisibility(Activity activity, int visible) {
            HorizontalScrollView groupChatBannerScrollerLayout =
                    (HorizontalScrollView) activity.findViewById(R.id.group_chat_banner_scroller);
            if (groupChatBannerScrollerLayout != null) {
                Logger
                        .v(TAG,
                                "setGroupChatBannerScrollerVisibility() groupChatBannerScroller is not null");
                groupChatBannerScrollerLayout.setVisibility(visible);
            } else {
                Logger.v(TAG,
                        "setGroupChatBannerScrollerVisibility() groupChatBannerScroller is null.visible = "
                                + visible);
            }
        }

        private void setGroupChatExpandVisibility(Activity activity, int visible) {
            
                Logger.v(TAG, "setGroupChatExpandVisibility() groupChatExpandView is null");
            
        }

        private void setGroupChatCollapseVisibility(Activity activity, int visible) {
            
                Logger.v(TAG, "setGroupChatCollapseVisibility() groupChatExpandView is null");
            }
    }

   public void showParticpants()
    {
        Intent intent = new Intent("com.mediatek.rcse.action.SHOW_PARTICIPANTS");           
        intent.putParcelableArrayListExtra("participantsinfo", (ArrayList<? extends Parcelable>) mGroupChatParticipantList);
        startActivity(intent);
    }


    @Override
    public void onClick(View v) {
        Logger.d(TAG, "onClick() entry");
        showParticpants();
        /*
        if (activity != null) {
            if (v.getId() == R.id.group_chat_expand) {
                expandGroupChat();
            } else if (v.getId() == R.id.group_chat_collapse) {
                collapseGroupChat();
            } else if (v.getId() == R.id.group_chat_title_layout) {
                Logger.v(TAG, "onClick() group_chat_title_layout clicked");
                // Now should judge current status is collapsed or expanded.
                HorizontalScrollView groupChatBannerScroller =
                        (HorizontalScrollView) activity
                                .findViewById(R.id.group_chat_banner_scroller);
                if (groupChatBannerScroller != null
                        && groupChatBannerScroller.getVisibility() == View.VISIBLE) {
                    // Now it is in expanded.
                    collapseGroupChat();
                } else {
                    // Now it is in collapsed.
                    expandGroupChat();
                }
            }
        } else {
            Logger.w(TAG, "onClick() activity is null");
        }
        */
    }
    
    /**
     * Add contacts to current chat fragment.
     */
    @Override
    public boolean addContacts() {
        Logger.d(TAG, "addContacts()");
        int currentParticipantsNum = getParticipantsNum();
        int maxNum = RcsSettings.getInstance().getMaxChatParticipants() - 1;
        Logger.d(TAG, "currentParticipantsNum = " + currentParticipantsNum + ", maxNum = " + maxNum);
        if (currentParticipantsNum >= maxNum) {
            mIsMaxGroupChatParticipantsWork = true;
            showToast(R.string.cannot_add_any_more_member);
            return false;
        }
        return super.addContacts();
    }

    /**
     * Check whether current participants is already max. It's used by test case
     * @return True if current participants is already max, otherwise return false.
     */
    public boolean isMaxGroupChatParticipantsWork() {
        return mIsMaxGroupChatParticipantsWork;
    }

    /**
     * Add message to chat adpater and chat list atomic
     */
    private void addMessageAtomic(IChatMessage  msg) {
        synchronized (mLock) {
            mMessageSequenceOrder = mMessageSequenceOrder +1;
            int position = mMessageSequenceOrder;
            // Adding this new message on this position 
            mMessageAdapter.addMessage(msg, position);
            mMessageList.add(position, msg);
        }
    }
@Override
    public void removeChatMessage(String messageId) {
        mMessageList.remove(itemID_position -1);
        
    }
    @Override
    public void addgroupSubject(final String subject) {
        
        if (mActivity == null) {
            Logger.d(TAG, "updateParticipants mActivity is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                setmSubjectGroupChat(subject);
                setChatScreenTitle(); 
            }
        });
               
    }

	@Override
	public void updateAllMsgAsReadForContact(Participant participant) {
		// TODO Auto-generated method stub
		
	}

}
