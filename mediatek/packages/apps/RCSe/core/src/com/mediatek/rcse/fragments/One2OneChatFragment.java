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
import android.app.Fragment;
import android.app.FragmentManager;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnClickListener;
import android.view.View.OnCreateContextMenuListener;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.drm.OmaDrmStore;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.activities.widgets.AttachmentTypeSelectorAdapter;
import com.mediatek.rcse.activities.widgets.ChatScreenWindow;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.MessageParsing;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.CapabilityApi.ICapabilityListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.Vector;
import java.util.concurrent.ConcurrentHashMap;

/**
 * A fragment for one2one chat
 */
public class One2OneChatFragment extends ChatFragment implements ChatView.IOne2OneChatWindow, ICapabilityListener , OnCreateContextMenuListener{

    public static final String SHOW_REMOTE_STRANGER = "c";

    public static final int ALPHA_VALUE_ENABLE = 255;

    public static final int ALPHA_VALUE_DISABLE = 75;

    /*public static final int RESULT_CODE_CAMERA = 10;

    public static final int RESULT_CODE_GALLERY = 11;

    public static final int RESULT_CODE_FILE_MANAGER = 12;*/

    //public static final String GALLERY_TYPE = "image/*";

    //public static final String CHOICE_FILEMANAGER_ACTION = "com.mediatek.filemanager.ADD_FILE";

    private static final long THREAD_ID_MAIN = 1;

    //private static final String IMAGE_TYPE = "image/";

    //private Uri mCameraTempFileUri = null;
    private static final String TAG = "One2OneChatFragment";
    private TextView mRemoteStrangerText = null;
    private Participant mParticipant = null;
    private List<String> mDateList = new ArrayList<String>();
    private List<IChatMessage> mMessageList = new Vector<IChatMessage>();
    //private int mFiletransferEnableStatus = One2OneChat.FILETRANSFER_ENABLE_OK;
    //private String mFileName = null;
    //private WarningDialog mWarningDialog = new WarningDialog();
    //private RepickDialog mRepickDialog = new RepickDialog();
    private boolean mShowHeader = false;
    public boolean mRemoteIsRcse = true;
    //private CompressDialog mCompressDialog = null;
    private final Object mLock = new Object();
    private int mMessageSequenceOrder = -1 ;
    private int itemID_position;

    public static final String FILE_SCHEMA = "file://";
    private static final String CONTENT_SCHEMA = "content://media";
    /*private static final String CONTENT_IMAGE_TYPE = "/images/";
    private static final String CONTENT_AUDIO_TYPE = "/audio/";
    private static final String CONTENT_VIDEO_TYPE = "/video/";
    private static final String VCARD_SCHEMA = "content://com.android.contacts/contacts/as_vcard";
    private static final String ALL_VCARD_SCHEMA =
            "content://com.android.contacts/contacts/as_multi_vcard/";
    private static final String VCARD_DATA_TYPE = "text/x-vcard";
    private static final String VCARD_SUFFIX = ".vcf";
    private static final String VCALENDAR_SCHEMA = "content://com.mediatek.calendarimporter/";
    private static final String VCALENDAR_DATA_TYPE = "text/x-vcalendar";
    private static final String VCALENDAR_SUFFIX = ".vcs";
    private static final String READABLE_RIGHT = "r";*/

    @Override
    protected void updateSendButtonState(String text) {
        Logger.d(TAG, "updateSendButtonState() mRemoteIsRcse: " + mRemoteIsRcse + " mBtnSend: "
                + mBtnSend);
        if (mBtnSend != null) {
            if (text.length() > 0 && mRemoteIsRcse) {
                mBtnSend.setEnabled(true);
                mBtnSend.setFocusable(true);
            } else {
                mBtnSend.setEnabled(false);
                mBtnSend.setFocusable(false);
            }
        }
    }

    /*private interface IChatMessage extends IChatWindowMessage {
        Date getMessageDate();
    }*/

    public void setTag(Object tag) {
        super.mTag = tag;
    }

    public void setParticipant(Participant participant) {
        mParticipant = participant;
        mParticipantList = new ArrayList<Participant>();
        mParticipantList.add(participant);
    }
    
    protected void handleClearTopReminder() {
        Logger.d(TAG, "handleClearTopReminder() entry");
        super.handleClearTopReminder();
        if (mRemoteStrangerText != null) {
            mRemoteStrangerText.setVisibility(View.GONE);
        }
        Logger.d(TAG, "handleClearTopReminder() exit");
    }

    protected void handleShowTopReminder(String reminder) {
        Logger.e(TAG, "handleShowTopReminder() entry reminder is " + reminder);
        if (SHOW_REMOTE_STRANGER.equals(reminder)) {
            if (mRemoteStrangerText != null) {
                if (mMgToOtherWinReminderText != null) {
                    mMgToOtherWinReminderText.setVisibility(View.GONE);
                }
                if (mNetworkErrorText != null) {
                    mNetworkErrorText.setVisibility(View.GONE);
                }
                if (mForwardToSettingsView != null) {
                    mForwardToSettingsView.setVisibility(View.GONE);
                }
                mRemoteStrangerText.setVisibility(View.VISIBLE);
            }
        } else {
            if (mRemoteStrangerText != null) {
                mRemoteStrangerText.setVisibility(View.GONE);
            }
            super.handleShowTopReminder(reminder);
        }
    }

    /**
     * Get the participant with this One2OneChatFragment instance
     * 
     * @return The participant with this One2OneChatFragment instance
     */
    public Participant getParticipant() {
        return mParticipant;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.menu_clear_history) {
            // TODO-Implemented clear history operation here
            return true;
        } else {
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onPause() {
        Logger.d(TAG, "onPause entry, mParticipant: " + mParticipant);
        // Fixed the issue when double tap on File Transfer Icon caused JE
        FragmentManager fm = getFragmentManager();
        Fragment fragment = fm.findFragmentByTag("MultiAlertDialog");        	
        if (fragment instanceof MessageParsing.MultiAlertDialog && fragment!=null) {        	
        fm.beginTransaction().remove(fragment).commitAllowingStateLoss();
        }      	
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        Logger.d(TAG, "onResume entry, mParticipant: " + mParticipant);
        judgeRemoteType();
        setFileTransferEnable(mFiletransferEnableStatus);
    }

    @Override
    public void onDestroy() {
        Logger.d(TAG, "onDestroy");
        ApiManager manager = ApiManager.getInstance();
        if (manager != null) {
            CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
            if (capabilityApi != null) {
                capabilityApi.unregisterCapabilityListener(this);
            } else {
                Logger.e(TAG, "onDestroy() the capablityApi is null");
            }
        } else {
            Logger.w(TAG, "onDestroy(), ApiManager is null!");
        }
        mPreFileTransferMap.clear();
        mPreMessageMap.clear();
        super.onDestroy();
    }

    @Override
    public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
        Logger.d(TAG, "addReceivedMessage() message: " + message + " isRead: " + isRead);
        if (message != null) {
            if (!mRemoteIsRcse) {
                mRemoteIsRcse = true;
                final String number = mParticipant.getContact();
                Thread currentThread = Thread.currentThread();
                if (THREAD_ID_MAIN == currentThread.getId()) {
                    Logger.w(TAG,
                            "addReceivedMessage  the currentThread is " + currentThread.getId());
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
                            Logger.w(TAG, "addReceivedMessage  it is mUiHandler");
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
            } else {
                Logger.d(TAG, "addReceivedMessage() the mRemoteIsRcse is " + mRemoteIsRcse);
            }
            final ReceivedMessage msg = new ReceivedMessage(message);
            Date date = message.getDate();
            if (mMessageAdapter == null) {
                Logger.d(TAG, "addReceivedMessage mMessageAdapter is null");
                return null;
            }
            addMessageAtomic(msg, date);
            if (!mIsBottom) {
                Thread currentThread = Thread.currentThread();
                if (mMessageReminderText != null) {
                    if (THREAD_ID_MAIN == currentThread.getId()) {
                        Logger.w(TAG, "addReceivedMessage  the currentThread is "
                                + currentThread.getId());
                        mMessageReminderText.setText(((ReceivedMessage) msg).getMessageText());
                        mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                        showReminderList();
                    } else {
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                Logger.w(TAG, "addReceivedMessage  it is mUiHandler");
                                mMessageReminderText.setText(((ReceivedMessage) msg)
                                        .getMessageText());
                                mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                                showReminderList();
                            }
                        });
                    }
                    mIsNewMessageNotify = Boolean.TRUE;
                }
            }
            return msg;
        } else {
            Logger.e(TAG, "addReceivedMessage(),message is null.");
            return null;
        }
    }

    private int onSentMessage(String content) {
        int messageTag = Utils.RANDOM.nextInt();
        InstantMessage message = new InstantMessage("", mParticipant.getContact(), content, true);        
        ISentChatMessage sentChatMessage = addSentMessage(message, messageTag);        
        return messageTag;
    }

    @Override
    public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
        Logger.d(TAG, "addSentMessage(), message: " + message);
        if (message != null) {
            if(messageTag == -1)
                messageTag = Utils.RANDOM.nextInt();
            SentMessage msg = (SentMessage) mPreMessageMap.get(messageTag);
            if (null == msg) {
                Logger.d(TAG, "addSentMessage() message with tag:" + messageTag + " not found");
                msg = new SentMessage(message);
                msg.setMessageTag(messageTag);
                Date date = message.getDate();
                if (mMessageAdapter == null) {
                    Logger.d(TAG, "addSentMessage mMessageAdapter is null");
                    return null;
                }
                addMessageAtomic(msg, date);
                mPreMessageMap.put(messageTag, msg);
            } else {
                Logger.d(TAG, "addSentMessage() message with tag:" + messageTag + " found");
                mPreMessageMap.remove(messageTag);
                msg.updateMessage(message);
            }
            return msg;
        }
        return null;
    }

    private void onMessageDateUpdated(SentMessage message, Date old, Date now) {
        int currentPosition = mMessageList.indexOf(message);
        int newPosition = searchMsgPosition(now);
        if (Math.abs(newPosition - currentPosition) <= 1) {
            Logger.d(TAG, "onMessageDateUpdated() same position: " + newPosition
                    + " , currentPosition: " + currentPosition);
            message.setMessageDate(now);
            if (DEFAULT_PRE_SEND_MESSAGE_DATE.equals(old)) {
                Logger.d(TAG, "onMessageDateUpdated() this is a present message, need to add one more date");
                addMessageDate(now);
            }
            // Test case will come here when test One2OneChat but did not create
            // fragment UI
            if (mMessageAdapter != null) {
                mMessageAdapter.notifyDataSetChanged();
            }
        } else {
            Logger.d(TAG, "onMessageDateUpdated() different position, old: "
                    + currentPosition + ", new: " + newPosition);
            //Check whether this message is the only item of a date index
            int lastItemPosition = currentPosition - 1;
            int nextItemPosition = currentPosition + 1;
            boolean isLastItemDate = false;
            boolean isNextItemDate = false;
            if (lastItemPosition >= 0 && mMessageList.get(lastItemPosition) instanceof DateMessage) {
                isLastItemDate = true;
            }
            if (nextItemPosition == mMessageList.size()
                    || (nextItemPosition < mMessageList.size() && mMessageList
                            .get(nextItemPosition) instanceof DateMessage)) {
                isNextItemDate = true;
            }
            boolean isNeedToRemoveLastItem = false;
            if (isLastItemDate && isNextItemDate) {
                isNeedToRemoveLastItem = true;
            }

            Logger.d(TAG, "onMessageDateUpdated() lastItemPosition: " + lastItemPosition
                    + " , nextItemPosition: " + nextItemPosition
                    + " , isLastItemDate: " + isLastItemDate
                    + " , isNextItemDate: " + isNextItemDate
                    + " , isNeedToRemoveLastItem: " + isNeedToRemoveLastItem);

            //Remove the message from current position
            mMessageAdapter.removeMessage(currentPosition);
            mMessageList.remove(currentPosition);
            if (isNeedToRemoveLastItem) {
                mMessageAdapter.removeMessage(lastItemPosition);
                mMessageList.remove(lastItemPosition);
                SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT_STRING);
                mDateList.remove(dateFormat.format(old));
            }

            //Then add the message at the new position
            message.setMessageDate(now);
            addMessageAtomic(message, now);
            mMessageAdapter.notifyDataSetChanged();
        }
    }

    /**
     * Search the position where a newly come message in the messageList.
     * 
     * @param currentMsgTime The newly come message time.
     * @return The position of the newly come message in the messageList.
     */
    public int searchMsgPosition(Date currentMsgTime) {
        int msgListSize = mMessageList.size();
        Logger.d(TAG, "searchMsgPosition() entry, messageList size is " + msgListSize
                + " currentMsgTime is " + currentMsgTime);
        int firstMsgPos = 0;
        int lastMsgPos = msgListSize - 1;
        if (msgListSize == 0) {
            Logger.d(TAG, "searchMsgPosition(), This is the first message");
            return 0;
        }
        Date firstMsgTime = mMessageList.get(firstMsgPos).getMessageDate();
        Date lastMsgTime = mMessageList.get(lastMsgPos).getMessageDate();
        Logger.d(TAG, "searchMsgPosition(), the lastMsgPos is " + lastMsgPos
                + " and the firstMsgPos is " + firstMsgPos);
        if (DEFAULT_PRE_SEND_MESSAGE_DATE.equals(currentMsgTime) || (currentMsgTime.after(lastMsgTime))) {
            Logger.d(TAG, "searchMsgPosition(), " +
                    "the currentMsgTime is after the lastMsgTime, so add the message at the end of the list");
            return msgListSize;
        } else if (currentMsgTime.before(firstMsgTime)) {
            Logger.d(TAG, "searchMsgPosition(), " +
                    "the currentMsgTime is before the firstMsgTime, so add the message at the begining of the list");
            return 0;
        } else {
            return halfSearch(firstMsgPos, lastMsgPos, currentMsgTime);
        }
    }

    private int halfSearch(int firstIndex, int lastIndex, Date date) {
        if (firstIndex + 1 == lastIndex) {
            return firstIndex + 1;
        } else {
            int middleIndex = (firstIndex + lastIndex) / 2;
            Date middleDate = mMessageList.get(middleIndex).getMessageDate();
            if (date.after(middleDate)) {
                return halfSearch(middleIndex, lastIndex, date);
            } else {
                return halfSearch(firstIndex, middleIndex, date);
            }
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
        if (DEFAULT_PRE_SEND_MESSAGE_DATE.equals(date)) {
            Logger.d(TAG, "addMessageDate() do not need to add a new date item for a pre-send message");
            return;
        }
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT_STRING);
        String currentDate = dateFormat.format(date);
        Date dateAddIntoList = new Date(date.getYear(), date.getMonth(), date.getDate(), 0, 0, 0);
        Logger.d(TAG, "currentDate is " + currentDate);
        if (mDateList.contains(currentDate) && (mDateList.get(mDateList.size()-1).equals(currentDate))) {            
            return;
        } else {
            mMessageSequenceOrder = mMessageSequenceOrder + 1;
            int position = mMessageSequenceOrder;
            if (mMessageAdapter == null) {
                Logger.d(TAG, "addMessageDate mMessageAdapter is null");
                return;
            }
            DateMessage dateMessage = new DateMessage(dateAddIntoList);
            synchronized (mLock) {
                mMessageAdapter.addMessage(date, position);
                mMessageList.add(position, dateMessage);
                mDateList.add(currentDate);
            }
        }
    }

    @Override
    public void removeAllMessages() {
        Logger.d(TAG, "removeAllMessages entry");
        mMessageList.clear();
        mDateList.clear();
        mMessageSequenceOrder = -1;
        if (mMessageAdapter != null) {
            mMessageAdapter.removeAllItems();
        } else {
            Logger.w(TAG, "removeAllMessages mMessageAdapter is null");
        }
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
                                btnAddView.setOnClickListener(new OnClickListener() {
                                    @Override
                                    public void onClick(View v) {
                                        queryCapablility();
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
    public void setIsComposing(boolean isComposing) {
        Logger.v(TAG, "setIsComposing status is " + isComposing);
        if (mTypingText != null) {
            if (getActivity() != null) {
                if (isComposing) {
                    if (mParticipant != null) {
                        final String isTyping =
                                getResources().getString(
                                        R.string.label_contact_is_composing,
                                        ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                                                mParticipant.getContact()));
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                mTypingText.setText(isTyping);
                                mTextReminderSortedSet.add(SHOW_IS_TYPING_REMINDER);
                                showReminderList();
                            }
                        });
                        Logger.w(TAG, "setIsComposing + isTyping" + mParticipant.getDisplayName());
                    } else {
                        Logger.e(TAG, "setIsComposing the participant is null");
                    }
                } else {
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mTextReminderSortedSet.contains(SHOW_IS_TYPING_REMINDER)) {
                                mTextReminderSortedSet.remove(SHOW_IS_TYPING_REMINDER);
                                Logger.d(TAG,
                                        "addReceivedMessage the SHOW_IS_TYPING_REMINDER  exists");
                            } else {
                                Logger
                                        .d(TAG,
                                                "addReceivedMessage the SHOW_IS_TYPING_REMINDER  not exists");
                            }
                            showReminderList();
                        }
                    });
                }
            } else {
                Logger.e(TAG, "the attach Activity is null");
            }
        } else {
            Logger.e(TAG, "setIsComposing the typingtext is null");
        }

    }

    @Override
    public void setRemoteOfflineReminder(boolean isOffline) {
        Logger.v(TAG, "setRemoteOfflineReminder(), isOffline: " + isOffline + " mParticipant: "
                + mParticipant + " mRemoteOfflineText: " + mRemoteOfflineText + " getActivity: "
                + getActivity());
        if (mRemoteOfflineText != null) {
            if (getActivity() != null) {
                if (isOffline) {
                    if (mParticipant != null) {
                        String displayName = mParticipant.getDisplayName();
                        final String isOfflineText = getResources().getString(
                                R.string.label_remote_is_offline, displayName, displayName);
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                mRemoteOfflineText.setText(isOfflineText);
                                mTextReminderSortedSet.add(SHOW_REMOTE_OFFLINE_REMINDER);
                                showReminderList();
                            }
                        });
                        Logger.i(TAG, "setRemoteOfflineReminder() " + displayName + " is offline");
                    }
                } else {
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mTextReminderSortedSet.contains(SHOW_REMOTE_OFFLINE_REMINDER)) {
                                mTextReminderSortedSet.remove(SHOW_REMOTE_OFFLINE_REMINDER);
                                Logger.d(TAG,
                                        "setRemoteOfflineReminder(), SHOW_REMOTE_OFFLINE_REMINDER is exists");
                            } else {
                                Logger.d(TAG,
                                        "setRemoteOfflineReminder(), SHOW_REMOTE_OFFLINE_REMINDER is not exists");
                            }
                            showReminderList();
                        }
                    });
                }
            }
        }
    }

    @Override
    protected void onSend(String message) {
        Logger.i(TAG, "onSend() The message is " + message);
        if (TextUtils.isEmpty(message) || null == mParticipant
                || TextUtils.isEmpty(mParticipant.getContact())) {
            return;
        }
        int messageTag = onSentMessage(message);
        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_SEND_MESSAGE, mTag, message);
        controllerMessage.arg1 = messageTag;
        controllerMessage.sendToTarget();
    }

    /**
     * When switch ChatFragment this method should be called to remove ui.
     */
    public void removeChatUi() {
        Activity activity = getActivity();
        if (activity != null) {
            activity.getActionBar().setCustomView(null);
        } else {
            Logger.w(TAG, "activity is null.");
        }
    }

    /**
     * Set chat screen's title.
     */
    public void setChatScreenTitle() {
        Logger.v(TAG, "setChatScreenTitle()");
        Activity activity = getActivity();
        if (activity != null) {
            int num = getParticipantsNum();
            Logger.w(TAG, "setChatScreenTitle() num: " + num);
            TextView titleView = (TextView) activity.findViewById(R.id.peer_name);
            if (titleView != null) {
                Logger.w(TAG, "titleView is not null");
                if (null != mParticipantList && mParticipantList.size() > 0) {
                    titleView.setText(getParticipantsName(mParticipantList
                            .toArray(new Participant[1])));
                }
            }
        }
    }

    private void handleGetHistory(int count) {
        Logger.i(TAG, "handleGetHistory()");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(ChatController.EVENT_GET_CHAT_HISTORY,
                mTag, count + "");
        controllerMessage.sendToTarget();
    }

   /* private void handleFileTransferInvite(String filePath) {
        Logger.i(TAG, "handleFileTransferInvite()");
        if (mFiletransferEnableStatus == One2OneChat.FILETRANSFER_ENABLE_OK) {
            ControllerImpl controller = ControllerImpl.getInstance();
            FileStruct fileStruct = FileStruct.from(filePath);
            Bundle data = onSentFile(fileStruct);
            Message controllerMessage = controller.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_INVITATION, mTag, filePath);
            controllerMessage.setData(data);
            controllerMessage.sendToTarget();
        } else {
            Activity activity = getActivity();
            if (activity != null) {
                switch (mFiletransferEnableStatus) {
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED:
                        Toast.makeText(activity,
                                getString(R.string.filetransfer_disable_reason_capability_failed),
                                Toast.LENGTH_LONG).show();
                        break;
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER:
                        Toast.makeText(activity, getString(R.string.file_transfer_off_line),
                                Toast.LENGTH_LONG).show();
                        break;
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE:
                        Toast.makeText(activity,
                                getString(R.string.file_transfer_receiver_offline),
                                Toast.LENGTH_LONG).show();
                        break;
                    default:
                        Logger.d(TAG, "handleFileTransferInvite the mFiletransferEnableStatus is "
                                + mFiletransferEnableStatus);
                }
            } else {
                Logger.e(TAG, "handleFileTransferInvite the activity is null");
            }
        }
    }*/

   /* private void startCamera() {
        boolean success = FileFactory.createDirectory(RCSE_FILE_DIR);
        success = FileFactory.createDirectory(RCSE_TEMP_FILE_DIR);
        Logger.d(TAG, "startCamera() success: " + success + " RCSE_TEMP_FILE_DIR: "
                + RCSE_TEMP_FILE_DIR);
        mCameraTempFileUri = Uri.fromFile(new File(ChatFragment.RCSE_TEMP_FILE_DIR,
                ChatFragment.RCSE_TEMP_FILE_NAME_HEADER
                        + String.valueOf(System.currentTimeMillis()) + ChatFragment.JPEG_SUFFIX));
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, mCameraTempFileUri);
        startActivityForResult(intent, RESULT_CODE_CAMERA);
    }

    private void startGallery() {
        Logger.v(TAG, "startGallery()");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(GALLERY_TYPE);
        startActivityForResult(intent, RESULT_CODE_GALLERY);
    }

    private void startFileManager() {
        Logger.v(TAG, "startFileManager()");
        Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        if (FeatureOption.MTK_DRM_APP) {
            intent.putExtra(OmaDrmStore.DrmExtra.EXTRA_DRM_LEVEL, OmaDrmStore.DrmExtra.DRM_LEVEL_SD);
        }
        startActivityForResult(intent, RESULT_CODE_FILE_MANAGER);
    }*/

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Logger.v(TAG, "onActivityResult() requestCode = " + requestCode + ",resultCode = "
                + resultCode + ",data = " + data);
        if (requestCode == RESULT_CODE_ADD_CONTACTS) {
            if (data != null) {
                ArrayList<Participant> participantList;
                participantList = ContactsListManager.getInstance().parseParticipantsFromIntent(
                        data);
                if (participantList != null && participantList.size() != 0) {
                    participantList.add(0, mParticipant);
                    addContactsToOne2OneChat(participantList);
                }
            }
        }
    }

    private void addContactsToOne2OneChat(ArrayList<Participant> participantList) {
        Logger.v(TAG, "addContactsToOne2OneChat entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        ChatScreenWindowContainer container = ChatScreenWindowContainer.getInstance();
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Message controllerMessage = controller.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                parcelUuid, participantList);
        controllerMessage.sendToTarget();
        ChatScreenActivity activity = (ChatScreenActivity) getActivity();
        activity.fromOne2OneToGroupChat = true;
        Logger.d(TAG, "addContactsToOne2OneChat activity: " + activity);
        if (activity != null) {
            ChatScreenWindow window = container.getFocusWindow();
            if (window != null) {
                window.pause();
            }
            container.clearCurrentStatus();
            container.focus(participantList);
            container.focus(parcelUuid);
            activity.getChatWindowManager().connect(true);
            window = container.getFocusWindow();
            if (window != null) {
                window.resume();
            }
        }
        Logger.v(TAG, "addContactsToOne2OneChat exit");
    }

    /*protected void onAddAttachment() {
        if (mAttachmentTypeSelectorAdapter == null) {
            mAttachmentTypeSelectorAdapter = new AttachmentTypeSelectorAdapter(getActivity());
        }
        // Fixed the issue when double tap on File Transfer Icon caused JE
        FragmentManager fm = getFragmentManager();
        Fragment fragment = fm.findFragmentByTag("DialogOfAddAttachment");        	
        if (fragment instanceof DialogOfAddAttachment && fragment!=null) {        	
        fm.beginTransaction().remove(fragment).commitAllowingStateLoss();
        }      		

        mDialogOfAddAttachment.show(getFragmentManager(), DialogOfAddAttachment.TAG);
    }*/

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle arguments = getArguments();
        mTag = arguments.getParcelable(Utils.CHAT_TAG);
        mParticipantList = arguments.getParcelableArrayList(Utils.CHAT_PARTICIPANTS);
        if (mParticipantList.size() > 0) {
            mParticipant = mParticipantList.get(0);
        }
        mRemoteStrangerText = (TextView) mContentView.findViewById(R.id.text_stranger_remind);
        mMessageListView.setOnCreateContextMenuListener(this);
        CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
        if (capabilityApi != null) {
            capabilityApi.registerCapabilityListener(this);
        } else {
            Logger.e(TAG, "onCreate() the capablityApi is null");
        }
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {            
            Logger.d(TAG, "onCreateContextMenu entry: ");            
            super.onCreateContextMenu(menu, v, menuInfo);            
            AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo)menuInfo; 
            menu.setHeaderTitle("Options");
            //get the postion of the elemet that is clicked
            itemID_position = info.position; 
            IChatMessage chatMessage = mMessageList.get(itemID_position- 1);
            MenuInflater inflater = getActivity().getMenuInflater();
                        
            if(chatMessage instanceof SentMessage && ((SentMessage)chatMessage).getStatus().equals(com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status.FAILED))
            {
                Logger.d(TAG, "onCreateContextMenu SentMessage: "); 
                inflater.inflate(R.menu.chatmessagewithresent, menu);               
            }
            else if(chatMessage instanceof DateMessage )
            {
                Logger.d(TAG, "onCreateContextMenu DateMessage: "); 
                // Don't Inflate menu
            } 
            else if(chatMessage instanceof FileTransfer && (((FileTransfer)chatMessage).getStatue().equals(Status.WAITING) || ((FileTransfer)chatMessage).getStatue().equals(Status.PENDING)))
            {
                Logger.d(TAG, "onCreateContextMenu FileTransfer: "); 
                // Don't Inflate menu
            } 
            else if(chatMessage instanceof ReceivedFileTransfer)
            {
            	Logger.d(TAG, "onCreateContextMenu FileTransfer: "); 
            	inflater.inflate(R.menu.chatmessagewithblock, menu);
            }
            else 
                inflater.inflate(R.menu.chatmessagemenu, menu);
            
            
       }    
  
    
    @Override
    public boolean onContextItemSelected(MenuItem item) {
        Log.d("Context Menu", "inside context menu item selected");        
        IChatMessage chatMessage = mMessageList.get(itemID_position- 1);
        Message controllerMessage;
        switch(item.getItemId())
        {

        case R.id.delete:  
                     
            controllerMessage = ControllerImpl.getInstance().obtainMessage(
                    ChatController.EVENT_DELETE_MESSAGE, mTag, chatMessage.getId());
            mMessageAdapter.removeMessage(itemID_position -1); 
            mMessageSequenceOrder = mMessageSequenceOrder - 1;
            controllerMessage.sendToTarget();
            Logger.d(TAG, "onContextItemSelected msgId: " + chatMessage.getId());
        
        break;  
        
        case R.id.info: 
        {
            MessageInfoDialog infoDialog = new MessageInfoDialog(new Date(), "");
            
            if(chatMessage instanceof SentMessage)
            {
                infoDialog = new MessageInfoDialog(((SentMessage)chatMessage).getMessageDate(),((SentMessage)chatMessage).getMessageText());
            }
            else if (chatMessage instanceof ReceivedMessage)
            {
                infoDialog = new MessageInfoDialog(((ReceivedMessage)chatMessage).getMessageDate(),((ReceivedMessage)chatMessage).getMessageText());
            }
            else if (chatMessage instanceof ReceivedFileTransfer)
            {
                infoDialog = new MessageInfoDialog(((ReceivedFileTransfer)chatMessage).getMessageDate(),"Image");
            }
            else if (chatMessage instanceof SentFileTransfer)
            {
                infoDialog = new MessageInfoDialog(((SentFileTransfer)chatMessage).getMessageDate(),"Image");
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
        	//mark future requests from this contact as spam
        	if(chatMessage instanceof ReceivedFileTransfer)
        	{
        		MarkSpamDialog spamDialog = new MarkSpamDialog(mParticipant.getContact());
        		spamDialog.show(getFragmentManager(), MarkSpamDialog.TAG);
        	}
        	break;
       
        } 
        return true;

    }    
    
    
    public class MarkSpamDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "MarkSpamDialog";
        private int mRequestCode = 0;
       String mContact;
       
 
        public MarkSpamDialog(String contact)
        {
            mContact = contact;
          
        }
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {            
            final AlertDialog alertDialog;
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();            
            alertDialog.setTitle(mParticipant.getDisplayName());
            String message = "Do you want to mark future file requests from "+ mParticipant.getDisplayName()+" as spam";            
            alertDialog.setMessage(message);
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this); 
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);     
            if (which == DialogInterface.BUTTON_POSITIVE) {
            	
            	new AsyncTask<Object, Void, Object>() {
                    @Override
                    protected Object doInBackground(Object... params) {
                        
                            ContactsManager.getInstance()
                                    .setFtBlockedForContact(mContact, true);

                        return null;
                    }

					@Override
					protected void onPostExecute(Object result) {
						//Toast.makeText(getActivity(), mContact +"is blocked for future file transfer requests", Toast.LENGTH_LONG);
					}
                    
                    
                }.execute();
            }
            
            this.dismissAllowingStateLoss();
            
        }
    }
    

    public class MessageInfoDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "RepickDialog";
        private int mRequestCode = 0;
       String mdate;
       String mText;
 
        public MessageInfoDialog(Date date, String text)
        {
            mdate = date.toString();
            mText = text;
          
        }
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {            
            final AlertDialog alertDialog;
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();            
            alertDialog.setTitle(mParticipant.getDisplayName());
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

    /*protected void addAttachment(int type) {
        Logger.v(TAG, "addAttachment(), type = " + type);
        switch (type) {
            case AttachmentTypeSelectorAdapter.ADD_FILE_FROM_GALLERY:
                startGallery();
                break;
            case AttachmentTypeSelectorAdapter.ADD_FILE_FROM_CAMERA:
                startCamera();
                break;
            case AttachmentTypeSelectorAdapter.ADD_FILE_FROM_FILE_MANAGER:
                startFileManager();
                break;
            default:
                Logger.w(TAG, "Can not handle attachment type of " + type);
                break;
        }
    }*/

    /**
     * DateMessage provided for window to get date.
     */
    public static class DateMessage implements IChatMessage {

        private Date mDate = null;

        public DateMessage(Date date) {
            mDate = (Date) date.clone();
        }

        @Override
        public Date getMessageDate() {
            return (Date) mDate.clone();
        }

        @Override
        public String getId() {
            return null;
        }

        @Override
        public boolean equals(Object o) {
            if (o instanceof DateMessage) {
                return mDate.equals(((DateMessage)o).mDate);
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
     * ReceivedMessage provided for window to get message.
     */
    public class ReceivedMessage implements IReceivedChatMessage, IChatMessage {

        private InstantMessage mMessage = null;

        public ReceivedMessage(InstantMessage msg) {
            mMessage = msg;
        }

        public String getDisplayName() {
            if (mParticipant != null) {
                return mParticipant.getDisplayName();
            } else {
                Logger.d(TAG, "getDisplayName mParticipant is null");
                return null;
            }
        }

        @Override
        public String getId() {
            if (mMessage != null) {
                return mMessage.getMessageId();
            } else {
                Logger.d(TAG, "getId mMessage is null");
                return null;
            }
        }

        public String getMessageText() {
            if (mMessage != null) {
                return mMessage.getTextMessage();
            } else {
                Logger.d(TAG, "getMessageText mMessage is null");
                return null;
            }
        }

        /**
         * Get the date when the message was received.
         * 
         * @return The date when the message was received.
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
    }

    /**
     * SentMessage provided for window to get message, and update message.
     */
    public class SentMessage implements ISentChatMessage, IChatMessage {
        private InstantMessage mMessage = null;

        private Status mStatus = Status.SENDING;
        private Date mDate = new Date();
        private int messageTag;

        public int getMessageTag() {
            return messageTag;
        }

        public void setMessageTag(int messageTag) {
            this.messageTag = messageTag;
        }

        public SentMessage(InstantMessage msg) {
            mMessage = msg;
            mDate = (Date) msg.getDate().clone();
        }

        @Override
        public String getId() {
            if (mMessage == null) {
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
                    if (mMessageAdapter == null) {
                        Logger.d(TAG, "updateStatus mMessageAdapter is null");
                        return;
                    }
                    mMessageAdapter.notifyDataSetChanged();
                }
            });
        }

        public Object getChatTag() {
            return mTag;
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
         * Get the date when the message was received.
         * 
         * @return The date when the message was received.
         */
        public Date getMessageDate() {
            return mDate;
        }

        public void setMessageDate(Date date) {
            mDate = (Date) date.clone();
        }

        protected void updateMessage(InstantMessage message) {
            Logger.d(TAG, "updateMessage() message: " + message);
            mMessage = message;
        }

        @Override
        public void updateDate(final Date date) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    Logger.d(TAG, "updateDate()->run() date: " + date);
                    Date old = getMessageDate();
                    //onMessageDateUpdated(SentMessage.this, old, date);
                }
            };

            if (Thread.currentThread().getId() != THREAD_ID_MAIN) {
                Logger.d(TAG, "updateDate() post to UI thread");
                mUiHandler.post(runnable);
            } else {
                Logger.d(TAG, "updateDate() ran on UI thread");
                runnable.run();
            }
        }

		public void updateStatus(Status status, String Contact) {
			// TODO Auto-generated method stub
			
		}
    }

    /**
     * A subclass of {@link AsyncTask}. This class is used to process the file
     * which was selected via camera,gallery or file manager.
     */
    /*class FileProcessingAsyncTask extends AsyncTask<Void, Void, String> {

        private int mRequestCode = -1;
        private int mResultCode = -1;
        private Intent mData = null;

        public FileProcessingAsyncTask(int requestCode, int resultCode, Intent data) {
            mRequestCode = requestCode;
            mResultCode = resultCode;
            mData = data;
        }

         @Override
        protected String doInBackground(Void... params) {
            Uri uri = null;
            if (Activity.RESULT_CANCELED == mResultCode) {
                Logger.w(TAG, "doInBackground(), the user cancel to select a picture");
                return null;
            }
            if (mRequestCode == RESULT_CODE_CAMERA) {
                uri = mCameraTempFileUri;
            } else if (mRequestCode == RESULT_CODE_GALLERY || mRequestCode == RESULT_CODE_FILE_MANAGER) {
                if (mData != null) {
                    String uriString = mData.getDataString();
                    if(uriString == null){
                    return null;
                }
                    Logger.d(TAG, "doInBackground(),uriString is " + uriString);
                    uri = Uri.parse(uriString);
                } else {
                    Logger.w(TAG,"doInBackground()-RESULT_CODE_GALLERY:mData is null");
                    return null;
                }
            } else {
                Logger.w(TAG, "doInBackground() unkown result");
                return null;
            }
            String fileFullName = getFileFullPathFromUri(uri);
            boolean isImage = isImageFile(fileFullName);
            Logger.d(TAG, "isImage = " + isImage);
            String compressedFileName = null;
            if (isImage && RcsSettings.getInstance().isEnabledCompressingImageFromDB()) {
                Logger.d(TAG, "Compress the image, do not hit the user");
                mCompressDialog = null;
                compressedFileName = doCompress(fileFullName);
            } else if (isImage && !RcsSettings.getInstance().isEnabledCompressingImageFromDB()) {
                boolean remind = RcsSettings.getInstance().restoreRemindCompressFlag();
                Logger.d(TAG, "Do hit the user to select whether to compress. remind = " + remind);
                if (remind) {
                    mCompressDialog = new CompressDialog();
                    Bundle arguments = new Bundle();
                    arguments.putString(Utils.MESSAGE, fileFullName);
                    mCompressDialog.setArguments(arguments);
                    mCompressDialog.show(getFragmentManager(), CompressDialog.TAG);
                } else {
                    Logger.d(TAG, "Do not compress image");
                    compressedFileName = fileFullName;
                }
            } else {
                Logger.d(TAG, "Do not compress non image file");
                compressedFileName = fileFullName;
            }
            Logger.d(TAG, "The compressed image file name = " + compressedFileName);
            return compressedFileName;
        }

        protected void onPostExecute(String fileName) {
            Logger.v(TAG, "onPostExecute(),fileName = " + fileName + ",mResultCode = "
                    + mResultCode);
            if (fileName != null) {
                mFileName = fileName;
                handleFileSizeWhenInvite(fileName);
            }
        }

        private String getFileFullPathFromUri(Uri uri) {
            if (uri == null) {
                Logger.e(TAG, "getFileFullPathFromUri()-uri is null");
                return null;
            }
            String uriString = Uri.decode(uri.toString());
            if (uriString == null) {
                Logger.e(TAG, "getFileFullPathFromUri()-uriString is null");
                return null;
            }
            Logger.d(TAG, "getFileFullPathFromUri()-The uri is:[" + uriString + "]");
            if (uriString.startsWith(FILE_SCHEMA)) {
                uriString = uriString.substring(FILE_SCHEMA.length(), uriString.length());
                return uriString;
            } else if (uriString.startsWith(CONTENT_SCHEMA)) {
                int findPos = uriString.indexOf(CONTENT_SCHEMA) + CONTENT_SCHEMA.length();
                String fileFullName = null;
                Cursor cursor = null;
                if (uriString.indexOf(CONTENT_IMAGE_TYPE) > findPos) {
                    cursor = One2OneChatFragment.this.getActivity().getContentResolver()
                            .query(uri, new String[] {
                                MediaStore.Images.ImageColumns.DATA
                            }, null, null, null);
                    if (cursor != null) {
                        cursor.moveToFirst();
                        fileFullName = cursor.getString(cursor
                                .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
                        cursor.close();
                    }
                } else if (uriString.indexOf(CONTENT_AUDIO_TYPE) > findPos) {
                    cursor = One2OneChatFragment.this.getActivity().getContentResolver()
                            .query(uri, new String[] {
                                MediaStore.Audio.AudioColumns.DATA
                            }, null, null, null);
                    if (cursor != null) {
                        cursor.moveToFirst();
                        fileFullName = cursor.getString(cursor
                                .getColumnIndex(MediaStore.Audio.Media.DATA));
                        cursor.close();
                    }
                } else if(uriString.indexOf(CONTENT_VIDEO_TYPE) > findPos){
                    cursor = One2OneChatFragment.this.getActivity().getContentResolver().query(uri,
                            new String[] {
                                    MediaStore.Video.VideoColumns.DATA,
                            }, null, null, null);
                    if(cursor != null){
                        cursor.moveToFirst();
                        fileFullName = cursor.getString(cursor
                                .getColumnIndex(MediaStore.Video.VideoColumns.DATA));
                        cursor.close();
                    }
            } else {
                    Logger.d(TAG, "getFileFullPathFromUri()-invalid media type");
                }
                return fileFullName;
            } else if (uriString.startsWith(VCARD_SCHEMA) || uriString.startsWith(ALL_VCARD_SCHEMA)) {
                String fileFullName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis()
                        + VCARD_SUFFIX;
                try {
                    AssetFileDescriptor fd = One2OneChatFragment.this.getActivity()
                            .getContentResolver().openAssetFileDescriptor(uri, READABLE_RIGHT);
                    FileInputStream fis = fd.createInputStream();
                    byte[] data = new byte[fis.available()];
                    fis.read(data);
                    fis.close();
                    File dir = new File(RCSE_TEMP_FILE_DIR);
                    if (!dir.exists()) {
                        if (!dir.mkdir()) {
                            Logger.e(TAG, "getFileFullPathFromUri()-create dir failed");
                            return null;
                        }
                    }
                    File file = new File(fileFullName);
                    file.setWritable(true);
                    file.setReadable(true);
                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(data);
                    fos.close();
                } catch (FileNotFoundException fileNotFoundException) {
                    Logger.e(TAG, "getFileFullPathFromUri()-fileNotFoundException");
                    fileFullName = null;
                    fileNotFoundException.printStackTrace();
                } catch (IOException iOException) {
                    Logger.e(TAG,"getFileFullPathFromUri()-iOException while accessing the stream");
                    fileFullName = null;
                    iOException.printStackTrace();
                } finally {
                    return fileFullName;
                }
            } else if (uriString.startsWith(VCALENDAR_SCHEMA)) {
                String fileFullName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis()
                        + VCALENDAR_SUFFIX;
                try {
                    AssetFileDescriptor fd = One2OneChatFragment.this.getActivity()
                            .getContentResolver().openAssetFileDescriptor(uri, READABLE_RIGHT);
                    FileInputStream fis = fd.createInputStream();
                    byte[] data = new byte[fis.available()];
                    fis.read(data);
                    fis.close();
                    File dir = new File(RCSE_TEMP_FILE_DIR);
                    if (!dir.exists()) {
                        if (!dir.mkdir()) {
                            Logger.e(TAG, "getFileFullPathFromUri()-create dir failed");
                            return null;
                        }
                    }
                    File file = new File(fileFullName);
                    file.setWritable(true);
                    file.setReadable(true);
                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(data);
                    fos.close();
                } catch (FileNotFoundException fileNotFoundException) {
                    Logger.e(TAG, "getFileFullPathFromUri()-fileNotFoundException");
                    fileFullName = null;
                    fileNotFoundException.printStackTrace();
                } catch (IOException iOException) {
                    Logger.e(TAG,"getFileFullPathFromUri()-iOException while accessing the stream");
                    fileFullName = null;
                    iOException.printStackTrace();
                } finally {
                    return fileFullName;
                }
            } else {
                Logger.e(TAG, "getFileFullPathFromUri()-invalid uri = " + uriString);
                return null;
            }
        }
    }// end FileProcessingAsyncTask
*/
    /*private void handleFileSizeWhenInvite(String fileName) {
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
        Logger.d(TAG, "handleFileSizeWhenInvite() maxFileSize: " + maxFileSize
                + " warningFileSize: " + warningFileSize + " shouldWarning: " + shouldWarning
                + " shouldRepick: " + shouldRepick);
        if (shouldRepick) {
            mFileName = null;
            mRepickDialog.show(getFragmentManager(), RepickDialog.TAG);
        } else if (shouldWarning) {
            Activity activity = getActivity();
            if (activity != null) {
                SharedPreferences prefer = PreferenceManager.getDefaultSharedPreferences(activity);
                Boolean remind = prefer.getBoolean(SettingsFragment.RCS_REMIND, true);
                Logger.w(TAG, "handleFileSizeWhenInvite() remind: " + remind);
                if (remind) {
                    mWarningDialog.show(getFragmentManager(), WarningDialog.TAG);
                } else {
                    handleFileTransferInvite(fileName);
                }
            }
        } else {
            handleFileTransferInvite(fileName);
        }
    }*/

   /* public class RepickDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "RepickDialog";
        private int mRequestCode = 0;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            mRequestCode = getArguments().getInt(Utils.MESSAGE);
            final AlertDialog alertDialog;
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            alertDialog.setTitle(R.string.large_file_repick_title);
            alertDialog.setMessage(getActivity().getText(R.string.large_file_repick_message));
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mRequestCode == RESULT_CODE_CAMERA) {
                    startCamera();
                } else if (mRequestCode == RESULT_CODE_FILE_MANAGER) {
                    startFileManager();
                } else {
                    startGallery();
                }
            }
            this.dismissAllowingStateLoss();
        }
    }*/

    /*public class WarningDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "WarningDialog";
        private CheckBox mCheckRemind = null;
        Activity mActivity = null;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .create();
            alertDialog.setTitle(R.string.file_size_warning);
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

        @Override
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
                }
                handleFileTransferInvite(mFileName);
            }
            this.dismissAllowingStateLoss();
        }
    }*/

    @Override
    public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
        Logger.d(TAG, "addFileTransferInvitation() file: " + file + " mRemoteIsRcse: "
                + mRemoteIsRcse + " mIsBottom: " + mIsBottom + " mMessageAdapter: " + mMessageAdapter);
        FileTransfer fileTransfer = null;
        if (file == null) {
            return null;
        }
        fileTransfer = new ReceivedFileTransfer(file, isAutoAccept);
        if (!mRemoteIsRcse) {
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
        }
        Date date = file.mDate;
        addMessageAtomic(fileTransfer, date);

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

    /*private Map<Object, SentFileTransfer> mPreFileTransferMap =
            new ConcurrentHashMap<Object, SentFileTransfer>();*/

    /*private Bundle onSentFile(FileStruct fileStruct) {
        Logger.d(TAG, "onSentFile() fileStruct: " + fileStruct);
        if (null != fileStruct) {
            Object fileTransferTag = fileStruct.mFileTransferTag;
            mPreFileTransferMap.put(fileTransferTag,
                    (SentFileTransfer) addSentFileTransfer(fileStruct));
            if (fileTransferTag instanceof Parcelable) {
                Bundle data = new Bundle();
                data.putParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG,
                        (Parcelable) fileTransferTag);
                return data;
            } else {
                Logger.w(TAG, "onSentFile() invalid file transfer tag: " + fileTransferTag);
                return null;
            }
        } else {
            Logger.w(TAG, "onSentFile() fileStruct is null");
            return null;
        }
    }*/

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
                addMessageAtomic(fileTransfer, date);
            } else {
                Logger.e(TAG, "addSentFileTransfer() fileTransfer is null");
            }
        }
        return fileTransfer;
    }

    /*public abstract class FileTransfer implements IFileTransfer, IChatMessage {
        protected FileStruct mFileStruct = null;

        protected Status mStatus = Status.WAITING;

        protected boolean mAutoAccept = false;

        protected Object mTag;

        protected long mProgress = 0;

        FileTransfer(FileStruct fileStruct) {
            mFileStruct = fileStruct;
        }

        *//**
         * Get file transfer contact.
         * 
         * @return contact of file transfer.
         *//*
        public String getContactName() {
            if (mParticipant != null) {
                return mParticipant.getDisplayName();
            } else {
                Logger.e(TAG, "getContactName mParticipant is null!");
                return null;
            }
        }

        *//**
         * Get file transfer contact number.
         * 
         * @return contact of file transfer.
         *//*
        public String getContactNum() {
            if (mParticipant != null) {
                return mParticipant.getContact();
            } else {
                Logger.e(TAG, "getContactName mParticipant is null!");
                return null;
            }
        }

        @Override
        public void setProgress(long progress) {
            mProgress = progress;
            mUiHandler.post(new Runnable() {

                @Override
                public void run() {
                    if (mMessageAdapter == null) {
                        Logger.d(TAG, "setProgress mMessageAdapter is null");
                        return;
                    }
                    mMessageAdapter.notifyDataSetChanged();
                }
            });
        }

        public long getProgress() {
            return mProgress;
        }

        @Override
        public void setStatus(Status status) {
            if (status == Status.TRANSFERING || status != mStatus) {
                mStatus = status;
                Runnable runnable = new Runnable() {

                    @Override
                    public void run() {
                        if (mMessageAdapter == null) {
                            Logger.d(TAG, "setStatus mMessageAdapter is null");
                            return;
                        }
                        mMessageAdapter.notifyDataSetChanged();
                    }
                };

                Thread currentThread = Thread.currentThread();
                if (currentThread.getId() == THREAD_ID_MAIN) {
                    runnable.run();
                } else {
                    mUiHandler.post(runnable);
                }
            } else {
                Logger.d(TAG, "setStatus(), should not update data. status now = " + status
                        + " status before =" + mStatus);
            }
        }

        public Status getStatue() {
            return mStatus;
        }

        public FileStruct getFileStruct() {
            return mFileStruct;
        }

        *//**
         * Get file transfer chat screen tag.
         * 
         * @return tag of file transfer screen.
         *//*
        public Object getTag() {
            return mTag;
        }

        *//**
         * Set file transfer chat screen tag.
         * 
         * @return void
         *//*
        public void setTag(Object tag) {
            mTag = tag;
        }

        *//**
         * Set the file path the file transfer, when received a file
         * 
         * @param file path
         *//*
        public void setFilePath(String filePath) {
            mFileStruct.mFilePath = filePath;
        }

        *//**
         * Update file transfer tag and size
         * 
         * @param received file path
         * @param received file size
         *//*
        public void updateTag(String transferTag, long transferSize) {
            mFileStruct.mFileTransferTag = transferTag;
            mFileStruct.mSize = transferSize;
        }

        *//**
         * Get the date when the file transfer was received.
         * 
         * @return The date when the file transfer was received.
         *//*
        public Date getMessageDate() {
            if (mFileStruct != null) {
                return mFileStruct.mDate;
            } else {
                Logger.e(TAG, "getMessageDate, mFileStruct is null!");
                return null;
            }
        }

        *//**
         * Get the message id.
         *//*
        public String getId() {
            return mFileStruct.mFileTransferTag.toString();
        }
    }*/

    public class SentFileTransfer extends FileTransfer {
        public SentFileTransfer(FileStruct fileStruct) {
            super(fileStruct);
            mStatus = Status.PENDING;
        }
        
        /**
         * Get file transfer contact.
         * 
         * @return contact of file transfer.
         */
        public String getContactName() {
            if (mParticipant != null) {
                return mParticipant.getDisplayName();
            } else {
                Logger.e(TAG, "getContactName mParticipant is null!");
                return null;
            }
        }

        /**
         * Get file transfer contact number.
         * 
         * @return contact of file transfer.
         */
        public String getContactNum() {
            if (mParticipant != null) {
                return mParticipant.getContact();
            } else {
                Logger.e(TAG, "getContactName mParticipant is null!");
                return null;
            }
        }
        
    }

    public class ReceivedFileTransfer extends FileTransfer {
        public ReceivedFileTransfer(FileStruct fileStruct, boolean isAutoAccept) {
            super(fileStruct);
            if(isAutoAccept)
                mStatus = Status.TRANSFERING;           

        }
        
        /**
         * Get file transfer contact.
         * 
         * @return contact of file transfer.
         */
        public String getContactName() {
            if (mParticipant != null) {
                return mParticipant.getDisplayName();
            } else {
                Logger.e(TAG, "getContactName mParticipant is null!");
                return null;
            }
        }

        /**
         * Get file transfer contact number.
         * 
         * @return contact of file transfer.
         */
        public String getContactNum() {
            if (mParticipant != null) {
                return mParticipant.getContact();
            } else {
                Logger.e(TAG, "getContactName mParticipant is null!");
                return null;
            }
        }
    }

    @Override
    public void addLoadHistoryHeader(boolean showHeader) {
        if (mMessageAdapter != null && mMessageAdapter.mHeaderView != null) {
            mShowHeader = showHeader;
            mMessageAdapter.mHeaderView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    handleGetHistory(One2OneChat.LOAD_DEFAULT);
                }
            });
            mUiHandler.post(new Runnable() {

                @Override
                public void run() {
                    if (!mRemoteIsRcse && mShowHeader) {
                        String number = mParticipant.getContact();
                        if (ContactsListManager.getInstance().isLocalContact(number)) {
                            showAsLocal(number);
                        } else if (ContactsListManager.getInstance().isStranger(number)) {
                            showAsStranger(number);
                        } else {
                            ContactsListManager.getInstance().setStrangerList(number, true);
                            showAsStranger(number);
                        }
                    } else {
                        Logger.d(TAG, "addLoadHistoryHeader() the mRemoteIsRcse is "
                                + mRemoteIsRcse);
                    }
                    mMessageAdapter.showHeaderView(mShowHeader);
                }
            });
        } else {
            Logger.w(TAG, "addLoadHistoryHeader, mMessageAdapter is not ready!");
        }
    }

    @Override
    protected void onAdapterPrepared() {
        Logger.i(TAG, "onAdapterPrepared, Check again when adapter is get ready!");
        handleGetHistory(One2OneChat.LOAD_ZERO_SHOW_HEADER);
    }

    @Override
    protected int getFragmentResource() {
        return R.layout.chat_fragment_one2one;
    }

    private void queryCapablility() {
        Logger.d(TAG, "queryCapablility() entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(ChatController.EVENT_QUERY_CAPABILITY,
                mTag, null);
        controllerMessage.sendToTarget();
        Logger.d(TAG, "queryCapablility() exit");
    }

    @Override
    public void updateAllMsgAsRead() {
        // Do Nothing
    }
    
    @Override
    public void onDisplayNameChanged() {
        Logger.d(TAG, "onDisplayNameChanged () entry mRemoteIsRcse is ");
        super.onDisplayNameChanged();
        judgeRemoteType();
    }

    @Override
    public void onCapabilityChanged(final String contact, final Capabilities capabilities) {
        Logger.d(TAG, "onCapabilityChanged() entry the contact is " + contact + " capabilities is "
                + capabilities);
        mUiHandler.post(new Runnable() {
            
            @Override
            public void run() {
                String participantNumber = mParticipant.getContact();
                if (participantNumber.equals(contact)) {
                    Logger.d(TAG,
                            "onCapabilityChanged() the participant equals the contact and number is "
                                    + contact);
                    if (ContactsListManager.getInstance().isLocalContact(participantNumber)) {
                        Logger.d(TAG, "onCapabilityChanged() the participant isLocalContact ");
                        showAsLocal(participantNumber);
                        return;
                    } else if (ContactsListManager.getInstance().isStranger(participantNumber)) {
                        Logger.d(TAG, "onCapabilityChanged() the participant isStranger ");
                        showAsStranger(participantNumber);
                        return;
                    }
                    if (capabilities.isSupportedRcseContact()) {
                        Logger.d(TAG,
                                "onCapabilityChanged() the participant is SupportedRcseContact ");
                        ContactsListManager.getInstance().setStrangerList(participantNumber, true);
                        showAsStranger(participantNumber);
                    } else {
                        Logger.d(TAG,
                                "onCapabilityChanged() the participant not SupportedRcseContact ");
                        ContactsListManager.getInstance().setStrangerList(participantNumber, false);
                        showAsNotRcseContact(participantNumber);
                    }
                } else {
                    Logger.d(TAG, "onCapabilityChanged the contact is not equals with "
                            + participantNumber);
                }
            }
        });
    }
    
    private void judgeRemoteType() {
        Logger.d(TAG, "judgeRemoteType() entry");
        if (mParticipant == null) {
            Logger.e(TAG, "judgeRemoteType(), mParticipant is null!");
            return;
        }
        String number = mParticipant.getContact();
        if (ContactsListManager.getInstance().isLocalContact(number)) {
            Logger.d(TAG, "judgeRemoteType() number isLocalContact " + number);
            showAsLocal(number);
        } else if (ContactsListManager.getInstance().isStranger(number)) {
            Logger.d(TAG, "judgeRemoteType() number isStranger " + number);
            showAsStranger(number);
        } else {
            Logger.d(TAG, "judgeRemoteType() number is not RCse contact " + number);
            showAsNotRcseContact(number);
        }
    }

    private void showAsLocal(String number) {
        Logger.d(TAG, "showAsLocal() entry, number: " + number + " mMessageEditor: "
                + mMessageEditor);
        mRemoteIsRcse = true;
        mTopReminderSortedSet.remove(SHOW_REMOTE_STRANGER);
        showTopReminder();
        if (mMessageEditor != null) {
            updateSendButtonState(mMessageEditor.getText().toString());
        }
    }

    private void showAsStranger(String number) {
        Logger.d(TAG, "showAsStranger() entry, number: " + number + " mRemoteStrangerText: "
                + mRemoteStrangerText + " mMessageEditor: " + mMessageEditor);
        mRemoteIsRcse = true;
        if (mRemoteStrangerText != null) {
            mRemoteStrangerText.setText(R.string.stranger_remind);
            mTopReminderSortedSet.add(SHOW_REMOTE_STRANGER);
        }
        showTopReminder();
        if (mMessageEditor != null) {
            updateSendButtonState(mMessageEditor.getText().toString());
        }
    }

    private void showAsNotRcseContact(String number) {
        Logger.d(TAG, "showAsNotRcseContact() entry, number: " + number + " mRemoteStrangerText: "
                + mRemoteStrangerText + " mMessageEditor: " + mMessageEditor);
        mRemoteIsRcse = false;
        if (mRemoteStrangerText != null) {
            String notRCSeInformation = getString(R.string.not_rcse_remind, number);
            mRemoteStrangerText.setText(notRCSeInformation);
            mTopReminderSortedSet.add(SHOW_REMOTE_STRANGER);
        }
        showTopReminder();
        if (mMessageEditor != null) {
            updateSendButtonState(mMessageEditor.getText().toString());
        }
    }

    /**
     * A dialog to hint the user that a picture will be compressed before
     * sending.
     */
    /*public class CompressDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "CompressDialog";
        private CheckBox mCheckNotRemind = null;
        private Activity mActivity = null;
        private String mOriginFileName;

        *//**
         * Constructor
         *//*
        public CompressDialog() {

        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Logger.d(TAG, "CompressDialog onCreateDialog entry");
            mOriginFileName = getArguments().getString(Utils.MESSAGE);
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .create();
            alertDialog.setTitle(R.string.compress_image_title);
            if (mActivity != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity.getApplicationContext());
                View customView = inflater.inflate(R.layout.warning_dialog, null);
                mCheckNotRemind = (CheckBox) customView.findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
                TextView contentView = (TextView) customView.findViewById(R.id.warning_content);
                contentView.setText(R.string.compress_image_content);
            } else {
                Logger.e(TAG, "activity is null in WarningDialog");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            RcsSettings.getInstance().saveRemindCompressFlag(mCheckNotRemind.isChecked());
            if (which == DialogInterface.BUTTON_POSITIVE) {
                handleOk();
            } else {
                Logger.d(TAG, "the user cancle compressing image");
                handleCancel();
            }
            this.dismissAllowingStateLoss();
        }

        private void handleOk() {
            Logger.i(TAG, "handleOk()");
            new AsyncTask<Void, Void, String>() {
                @Override
                protected String doInBackground(Void... params) {
                    if (mCheckNotRemind.isChecked()) {
                        Logger.d(TAG, "the user enable compressing image and not remind again");
                        RcsSettings.getInstance().setCompressingImage(true);
                    }
                    return doCompress(mOriginFileName);
                }

                @Override
                protected void onPostExecute(String result) {
                    Logger.v(TAG, "onPostExecute(),result = " + result);
                    if (result != null) {
                        mFileName = result;
                        handleFileSizeWhenInvite(result);
                    }
                }
            }.execute();
        }

        private void handleCancel() {
            Logger.i(TAG, "handleCancel()");
            mFileName = mOriginFileName;
            handleFileSizeWhenInvite(mOriginFileName);
        }
    }*/

    /**
     * Compress the image
     * 
     * @return The compressed image file full name.
     */
    /*private String doCompress(final String originFileName) {
        Logger.d(TAG, "doCompress(): originFileName = " + originFileName);
        return Utils.compressImage(originFileName);
    }

    private boolean isImageFile(String fileName) {
        Logger.d(TAG, "isImageFile() entry with fileName = " + fileName);
        if (fileName == null) {
            return false;
        }
        String type = AsyncGalleryView.getMimeType(fileName);
        if (type != null && type.startsWith(IMAGE_TYPE)) {
            return true;
        }
        return false;
    }*/

    /**
     * Add message to chat adpater and chat list atomic
     */
    private void addMessageAtomic(IChatMessage msg, Date date){
        synchronized (mLock) {
            addMessageDate(date);
            Logger.d(TAG, "addMessageAtomic() entry with msg = " + msg);
            mMessageSequenceOrder = mMessageSequenceOrder + 1;
            int position = mMessageSequenceOrder;
            if(mMessageAdapter !=null)
            {
                Logger.d(TAG, "addMessageAtomic() mMessageAdapter = " + mMessageAdapter);
            mMessageAdapter.addMessage(msg, position);
            }
            if(mMessageList !=null)
            {
                Logger.d(TAG, "addMessageAtomic() mMessageList = " + mMessageList);
            mMessageList.add(position, msg);            
        }
    }
}

    @Override
    public void removeChatMessage(String messageId) {
       
        IChatMessage chatMessage = mMessageList.get(itemID_position- 1);
        mMessageList.remove(itemID_position- 1);
        
        
    }

	@Override
	public void updateAllMsgAsReadForContact(Participant participant) {
		// TODO Auto-generated method stub
		
	}
}
