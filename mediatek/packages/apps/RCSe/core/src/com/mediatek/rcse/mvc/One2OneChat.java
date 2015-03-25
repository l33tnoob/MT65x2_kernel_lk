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

package com.mediatek.rcse.mvc;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.widget.Toast;

import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageReceived;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageSent;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettings.SelfCapabilitiesChangedListener;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.contacts.ContactInfo;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.GeolocMessage;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * This class is the implementation of a 1-2-1 chat model
 */
public class One2OneChat extends ChatImpl implements SelfCapabilitiesChangedListener {
    public static final String TAG = "One2OneChat";
  /*  public static final int FILETRANSFER_ENABLE_OK = 0;
    public static final int FILETRANSFER_DISABLE_REASON_NOT_REGISTER = 1;
    public static final int FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED = 2;
    public static final int FILETRANSFER_DISABLE_REASON_REMOTE = 3;*/

    public static final int LOAD_DEFAULT = 20;
    public static final int LOAD_ZERO_SHOW_HEADER = 0;
    public static final int TYPE_CHAT_SYSTEM_MESSAGE = 2;
    public static final int TYPE_GROUP_CHAT_SYSTEM_MESSAGE = 5;
    public static final int STATUS_TERMINATED = 1;
   // com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;

    private CopyOnWriteArrayList<IChatMessage> mAllMessages = new CopyOnWriteArrayList<IChatMessage>();

    

    public One2OneChat(ModelImpl modelImpl, IOne2OneChatWindow chatWindow, Participant participant,
            Object tag) {
        modelImpl.super(tag);
        Logger.d(TAG, "One2OneChat() entry with modelImpl is " + modelImpl + " chatWindow is "
                + chatWindow + " participant is " + participant + " tag is " + tag);
        mChatWindow = chatWindow;
        mParticipant = participant;
        mFileTransferController = new FileTransferController();
        RcsSettings rcsSetting = RcsSettings.getInstance();
        if (rcsSetting == null) {
            Logger.d(TAG, "One2OneChat() the rcsSetting is null ");
            Context context = ApiManager.getInstance().getContext();
            RcsSettings.createInstance(context);
            rcsSetting = RcsSettings.getInstance();
        }
        rcsSetting.registerSelfCapabilitiesListener(this);
    }

    /**
     * Set chat window for this chat.
     * 
     * @param chatWindow The chat window to be set.
     */
    public void setChatWindow(IChatWindow chatWindow) {
        Logger.d(TAG, "setChatWindow entry: mChatWindow = " + mChatWindow
                + ", chatWindow = " + chatWindow);
        super.setChatWindow(chatWindow);
    }

    protected void checkAllCapability() {
        Logger.d(TAG, "checkAllCapability() entry: mFileTransferController = "
                + mFileTransferController + ", mParticipant = " + mParticipant);
        final RegistrationApi registrationApi = ApiManager.getInstance().getRegistrationApi();
        if (mFileTransferController != null) {
            if (registrationApi != null && registrationApi.isRegistered()) {
                Logger.d(TAG, "checkAllCapability() already registered");
                mFileTransferController.setRegistrationStatus(true);
                CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
                Logger.v(TAG, "checkAllCapability() capabilityApi = " + capabilityApi);
                if (capabilityApi != null) {
                    Capabilities myCapablities = capabilityApi.getMyCapabilities();
                    if (null != mParticipant) {
                        String contact = mParticipant.getContact();
                        Capabilities remoteCapablities =
                                capabilityApi.getContactCapabilities(contact);
                        Logger.v(TAG, "checkAllCapability() myCapablities = "
                                + myCapablities + ",remoteCapablities = "
                                + remoteCapablities);
                        if (myCapablities != null) {
                            mFileTransferController.setLocalFtCapability(false);
                            if (myCapablities.isFileTransferSupported()) {
                                Logger.d(TAG,
                                        "checkAllCapability() my capability support filetransfer");
                                mFileTransferController
                                        .setLocalFtCapability(true);
                                if (remoteCapablities != null) {
                                    mFileTransferController
                                            .setRemoteFtCapability(false);
                                    if (remoteCapablities
                                            .isFileTransferSupported()) {
                                        Logger.d(TAG,
                                                "checkAllCapability() participant support filetransfer ");
                                        mFileTransferController
                                                .setRemoteFtCapability(true);
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                mFileTransferController.setRegistrationStatus(false);
            }
        }
        if (mFileTransferController != null) {
            mFileTransferController.controlFileTransferIconStatus();
        }
    }

    /**
     * Return the IChatWindow
     * 
     * @return IChatWindow
     */
    public IChatWindow getChatWindow() {
        return mChatWindow;
    }

    /**
     * Return the whole messages
     * 
     * @return The message list
     */
    public CopyOnWriteArrayList<IChatMessage> getAllMessages() {
        return mAllMessages;
    }

    /**
     * Clear the 1-2-1 chat history of a particular participant.
     * 
     * @return True if success, else False.
     */
    public boolean clearHistoryForContact() {
        Logger.d(TAG, "clearHistoryForContact() entry: mParticipant = " + mParticipant);
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.d(TAG, "clearHistoryForContact(), the ApiManager instance is null");
            return false;
        }
        Context context = instance.getContext();
        if (context == null) {
            Logger.d(TAG, "clearHistoryForContact(), the Context instance is null");
            return false;
        }
        final EventsLogApi eventsLogApi = new EventsLogApi(context);
        if (null != eventsLogApi) {
            if (mParticipant != null) {
                mWorkerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        String contact = mParticipant.getContact();
                        Logger.d(TAG, "mParticipant.getContact() is " + contact);
                        eventsLogApi.deleteMessagingLogForContact(contact);
                    }
                });
                clearChatWindowAndList();
                mChatWindow.addLoadHistoryHeader(false);
                Logger.d(TAG, "clearHistoryForContact() exit with true");
                return true;
            }
        }
        Logger.d(TAG, "clearHistoryForContact() exit with false");
        return false;
    }

    @Override
    protected synchronized void onDestroy() {
        mSentMessageManager.onChatDestroy();
        mMessageDepot.removeUnregisteredMessage();
        mAllMessages.clear();
        ContactsListManager.getInstance().setStrangerList(mParticipant.getContact(), false);
        RcsSettings rcsSetting = RcsSettings.getInstance();
        Logger.v(TAG, "onDestroy(): rcsSetting = " + rcsSetting);
        if (rcsSetting != null) {
            rcsSetting.unregisterSelfCapabilitiesListener(this);
        }
        MessagingApi messagingApi = ApiManager.getInstance().getMessagingApi();
        if (null != messagingApi) {
            List<IBinder> binders = null;
            try {
                binders = messagingApi.getChatSessionsWith(mParticipant.getContact());
                Logger.v(TAG, "onDestroy(): binders = " + binders);
                if (null != binders) {
                    for (IBinder binder : binders) {
                        IChatSession chatSession = IChatSession.Stub.asInterface(binder);
                        String sessionId = chatSession.getSessionID();
                        int state = chatSession.getSessionState();
                        switch (state) {
                            case SessionState.ESTABLISHED:
                            case SessionState.PENDING:
                                Logger.d(TAG, "onDestroy() sessionId:" + sessionId
                                        + " is in state " + state + " will be cancelled.");
                                chatSession.cancelSession();
                                break;
                            case SessionState.CANCELLED:
                            case SessionState.TERMINATED:
                            case SessionState.UNKNOWN:
                                Logger.d(TAG, "onDestroy() sessionId:" + sessionId
                                        + " is in state " + state + " has been cancelled.");
                                break;
                            default:
                                break;
                        }
                    }
                }
            } catch (ClientApiException e) {
                e.printStackTrace();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Logger.e(TAG, "getSessionByState() messagingApi is null");
        }
        super.onDestroy();
    }

    protected boolean getRegistrationState() {
        Logger.d(TAG, "One2OneChat-getRegistrationState()");
        ApiManager apiManager = ApiManager.getInstance();
        if (apiManager == null) {
            Logger.d(TAG, "getRegistrationState()-The apiManager is null");
            return false;
        } else {
            RegistrationApi registrationApi = apiManager.getRegistrationApi();
            if (registrationApi == null) {
                Logger.d(TAG, "getRegistrationState()-The registrationApi is null");
                return false;
            } else {
                return registrationApi.isRegistered();
            }
        }
    }

    private IChatSession getSessionByState(int sessionState) {
        Logger.d(TAG, "getSessionByState() entry, participant is " + mParticipant
                + " sessionState is " + sessionState);
        MessagingApi messagingApi = ApiManager.getInstance().getMessagingApi();
        Logger.v(TAG, "onDestroy(): messagingApi = " + messagingApi);
        if (null != messagingApi && null != mParticipant) {
            List<IBinder> binders = null;
            try {
                binders = messagingApi.getChatSessionsWith(mParticipant.getContact());
                Logger.v(TAG, "onDestroy(): binders = " + binders);
                if (null != binders) {
                    for (IBinder binder : binders) {
                        IChatSession chatSession = IChatSession.Stub.asInterface(binder);
                        if (chatSession.isStoreAndForward() || chatSession.isGroupChat()) {
                            Logger.d(TAG, "getSessionByState() S&F session found, skip it");
                            continue;
                        }
                        if (SessionState.PENDING != sessionState) {
                            if (sessionState == chatSession.getSessionState()) {
                                Logger.d(TAG, "getSessionByState() session with state "
                                        + sessionState + " found, session id:"
                                        + chatSession.getSessionID() + " chat id:"
                                        + chatSession.getChatID() + " is group:"
                                        + chatSession.isGroupChat());
                                return chatSession;
                            }
                        } else {
                            boolean isInComingSession = chatSession.isInComing();
                            if (isInComingSession) {
                                Logger.d(TAG, "getSessionByState() incoming pending session found,"
                                        + " session id:" + chatSession.getSessionID() + " chat id:"
                                        + chatSession.getChatID());
                                return chatSession;
                            }
                        }
                    }
                }
            } catch (ClientApiException e) {
                e.printStackTrace();
                return null;
            } catch (RemoteException e) {
                e.printStackTrace();
                return null;
            }
        }
        return null;
    }

    private InstantMessage sendMessageViaSession(IChatSession chatSession, String content) {
        try {
            String messageId = chatSession.sendMessage(content);
            if (!TextUtils.isEmpty(messageId) && mParticipant != null) {
                return new InstantMessage(messageId, mParticipant.getContact(), content, true, new Date());
            } else {
                Logger.e(TAG, "mParticipant is null or messageId is empty, " + "mParticipant is " + mParticipant
                        + " messageId is " + messageId);
                return null;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            return null;
        }
    }

    private InstantMessage sendMessageViaInvite(String content) {
        MessagingApi messagingApi = ApiManager.getInstance().getMessagingApi();
		CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
        if (null != messagingApi && null != mParticipant) {
			if(capabilityApi != null){
			Capabilities currentRemoteCapablities = capabilityApi.getContactCurentCapabilities(mParticipant.getContact());
			long delta = (System.currentTimeMillis()-currentRemoteCapablities.getTimestamp())/1000;
			if(currentRemoteCapablities == null || delta >= RcsSettings.getInstance().getCapabilityRefreshTimeout() || (delta < 0)){
                checkCapabilities();
			}
			}
			else
            checkCapabilities();
            try {
                IChatSession chatSession =
                        messagingApi.initiateOne2OneChatSession(mParticipant.getContact(), content);
                chatSession.addSessionListener(new One2OneChatListener(chatSession));
                return chatSession.getFirstMessage();
            } catch (ClientApiException e) {
                e.printStackTrace();
                return null;
            } catch (RemoteException e) {
                e.printStackTrace();
                return null;
            } catch (NullPointerException e) {
                e.printStackTrace();
                return null;
            }
        } else {
            Logger.e(TAG, "sendMessageViaInvite() messagingApi is " + messagingApi
                    + " ,mParticipant is " + mParticipant);
            return null;
        }
    }

    private InstantMessage sendRegisteredMessage(String content) {
        IChatSession chatSession = getSessionByState(SessionState.ESTABLISHED);
        if (null != chatSession) {
            return sendMessageViaSession(chatSession, content);
        } else {
            chatSession = getSessionByState(SessionState.PENDING);
            if (null != chatSession) {
                try {
                    chatSession.acceptSession();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
            /**
             * Changed to notify the user that the sip invite is failed.@{
             */
            InstantMessage message = null;
            message = sendMessageViaInvite(content);
            
            /*change to check null pointer */
            if(message!=null){
            message.setAsInviteMessage(true);
            } 
            return message;
            /**
             * @}
             */
        }
    }

    // This is a helper method for auto testing
    protected void sendMessage(final String content, Integer messageTag) {
        sendMessage(content, messageTag.intValue());
    }

    @Override
    public void sendMessage(final String content, final int messageTag) {
        Logger.d(TAG, "sendMessage() The content is " + content);
        if (messageTag == 0) {
            // messageTag is 0 means will send this meesage via sms
            InstantMessage messageViaSms = new InstantMessage(null, mParticipant.getContact(), content, false);
            if (mChatWindow != null) {
                mChatWindow.addSentMessage(messageViaSms, messageTag);
            }
            return;
        }
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                InstantMessage message = null;
                boolean isSuccess = false;
                if (getRegistrationState()) {
                    Logger.d(TAG, "sendMessage() send registered message");
                    message = sendRegisteredMessage(content);
                    if (null != message) {
						mMessageDepot.storeMessage(content, messageTag);
                        isSuccess = true;
                    } else {
                        Logger.d(TAG, "sendMessage() message is null");
                        message = mMessageDepot.storeMessage(content, messageTag);
                    }
                } else {
                    // Store the unregistered sending messages
                    Logger.d(TAG, "sendMessage() store unregistered message");
                    message = mMessageDepot.storeMessage(content, messageTag);
                }

                mComposingManager.messageWasSent();
                if (mChatWindow != null) {
                    ISentChatMessage sentMessage = mMessageDepot.checkIsResend(messageTag);
                    if (null == sentMessage) {
                        // This is a new message, not a resent one
                        sentMessage = mChatWindow.addSentMessage(message, messageTag);
                        mAllMessages.add(new ChatMessageSent(message));
                    }
                    /**
                     * Added to notify the user that the sip invite is failed.@{
                     */
                    if (message.isInviteMessage()) {
                        mLastISentChatMessageList.add(sentMessage);
                    }
                    /**
                     * @}
                     */

                    Logger.d(TAG, "sendMessage() sentMessage: " + sentMessage + " isSuccess: "
                            + isSuccess);

                    if (null != sentMessage) {
                        if (isSuccess) {
                            if (sentMessage instanceof SentChatMessage) {
                                Logger.d(TAG, "sendMessage(), is SentMessage, update message");
                                ((SentChatMessage) sentMessage).updateMessage(message);
                            }
							mMessageDepot.updateStoredMessage(messageTag, sentMessage);
                            mSentMessageManager.onMessageSent(sentMessage);
                            sentMessage.updateStatus(Status.SENDING);
                        } else {
                            mMessageDepot.updateStoredMessage(messageTag, sentMessage);
                            sentMessage.updateStatus(Status.FAILED);
                        }
                    }
                }
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.v(TAG, "sendMessage() run on worker thread");
            worker.run();
        } else {
            Logger.v(TAG, "sendMessage() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }

    @Override
    protected boolean setIsComposing(boolean isComposing) {
        IChatSession chatSession = getSessionByState(SessionState.ESTABLISHED);
        if (null != chatSession) {
            try {
                chatSession.setIsComposingStatus(isComposing);
                return true;
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    private void onChatListenerDestroy(One2OneChatListener chatListener) {
        IChatSession chatSession = getSessionByState(SessionState.ESTABLISHED);
        Logger.d(TAG, "onChatListenerDestroy() entry: chatSession = " + chatSession);
        if (null == chatSession) {
            Logger.d(TAG, "onChatListenerDestroy() chatSession is null");
            ((IOne2OneChatWindow) mChatWindow).setIsComposing(false);
            ((IOne2OneChatWindow) mChatWindow).setRemoteOfflineReminder(false);
        }
    }

    /**
     * This method is normally to handle the received delivery notifications via
     * SIP
     * 
     * @param messageId The message id of the delivery notification
     * @param status The type of the delivery notification
     */
    public void onMessageDelivered(final String messageId, final String status, final long timeStamp) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                mSentMessageManager.onMessageDelivered(messageId, formatStatus(status), timeStamp);
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.v(TAG, "onMessageDelivered() run on worker thread");
            worker.run();
        } else {
            Logger.v(TAG, "onMessageDelivered() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }

    private void onReceiveMessage(InstantMessage message) {
        String messageInfo =
                "messageId:" + message.getMessageId() + " message Text:" + message.getTextMessage();
        Logger.d(TAG, "onReceiveMessage() entry, " + messageInfo);
        Context context = AndroidFactory.getApplicationContext();
        if (message.isImdnDisplayedRequested()) {
            Logger.d(TAG, "onReceiveMessage() DisplayedRequested is true, " + messageInfo);
            if (!mIsInBackground) {
                /*
                 * Mark the received message as displayed when the chat window
                 * is not in background
                 */
                markMessageAsDisplayed(message);
            } else {
                /*
                 * Save the message and will mark it as displayed when the chat
                 * screen resumes
                 */
                One2OneChat.this.addUnreadMessage(message);
                /*
                 * Showing notification of a new incoming message when the chat
                 * window is in background
                 */
                RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, message,
                        null, 0);
            }
        } else {
            if (!mIsInBackground) {
                /*
                 * Mark the received message as read if the chat window is not
                 * in background
                 */
                markMessageAsRead(message);
            } else {
                /*
                 * Save the message and will mark it as read when the activity
                 * resumes
                 */
                One2OneChat.this.addUnreadMessage(message);
                /*
                 * Showing notification of a new incoming message when the chat
                 * window is in background
                 */
                RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, message,
                        null, 0);
            }
        }
        mChatWindow.addReceivedMessage(message, !mIsInBackground);
        mAllMessages.add(new ChatMessageReceived(message));
        mSentMessageManager.markSendingMessagesDisplayed();
    }

    @Override
    protected void markMessageAsDisplayed(InstantMessage msg) {
        String messageInfo =
                "message id:" + msg.getMessageId() + " message text:" + msg.getTextMessage();
        Logger.d(TAG, "markMessageAsDisplayed() entry, " + messageInfo);
        if (AndroidFactory.getApplicationContext() == null) {
            Logger.e(TAG, "getApplicationContext() return null");
            return;
        }
        SharedPreferences settings =
                PreferenceManager.getDefaultSharedPreferences(AndroidFactory
                        .getApplicationContext());
        boolean isSendReadReceiptChecked =
                settings.getBoolean(SettingsFragment.RCS_SEND_READ_RECEIPT, true);
        if (!isSendReadReceiptChecked) {
            Logger.d(TAG, "markMessageAsDisplayed() isSendReadReceiptChecked is false, "
                    + messageInfo);
            return;
        }
        IChatSession chatSession = getSessionByState(SessionState.ESTABLISHED);
        if (null != chatSession) {
            Logger.d(TAG, "markMessageAsDisplayed() established session found, " + messageInfo);
            try {
                if (chatSession.isStoreAndForward()) {
                    Logger.d(TAG, "markMessageAsDisplayed() this is a S&F session, " + messageInfo);
                    chatSession.setMessageDisplayedStatusBySipMessage(chatSession
                            .getReferredByHeader(), msg.getMessageId(),
                            ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                } else {
                    Logger.d(TAG, "markMessageAsDisplayed() this is a normal session, "
                            + messageInfo);
                    chatSession.setMessageDeliveryStatus(msg.getMessageId(),
                            ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            MessagingApi messagingApi = ApiManager.getInstance()
                    .getMessagingApi();
            Logger.d(TAG,
                    "markMessageAsDisplayed() established session not found, "
                            + messageInfo + ", messagingApi = " + messagingApi);
            if (null != messagingApi) {
                try {
                    Logger.d(TAG,
                            "markMessageAsDisplayed() will send displayed notification via SIP"
                                    + messageInfo);
                    messagingApi.setMessageDeliveryStatus(mParticipant.getContact(), msg
                            .getMessageId(), ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                } catch (ClientApiException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private class One2OneChatListener extends IChatEventListener.Stub {

        private static final String TAG = "One2OneChatListener";

        private IChatSession mCurrentSession = null;

        private String mSessionId = null;
        private String mCallId = null;

        public One2OneChatListener(IChatSession chatSession) {
            mCurrentSession = chatSession;
            if (null != mCurrentSession) {
                try {
                    mSessionId = mCurrentSession.getSessionID();
                    mCallId = mCurrentSession.getChatID();
                    Logger.d(TAG, "Constructor() session:" + mSessionId + " Call Id: " + mCallId);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        private void destroySelf() {
            Logger.d(TAG, "destroySelf entry : mCurrentSession = " + mCurrentSession);
            if (null != mCurrentSession) {
                try {
                    Logger.d(TAG, "destroySelf() session id is " + mSessionId + " Call Id: "
                            + mCallId);
                    mCurrentSession.removeSessionListener(this);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                mCurrentSession = null;
            }
            onChatListenerDestroy(this);
        }

        @Override
        public void handleAddParticipantFailed(String reason) throws RemoteException {
            // In a 1-2-1 chat, do not need to handle this event.
        }

        @Override
        public void handleAddParticipantSuccessful() throws RemoteException {
            // In a 1-2-1 chat, do not need to handle this event.
        }

        @Override
        public void handleConferenceEvent(String contact, String contactDisplayname, String state)
                throws RemoteException {
            // In a 1-2-1 chat, do not need to handle this event.
        }

        private static final int IM_INITIAL_ERROR = 2;

        @Override
        public void handleImError(final int error) throws RemoteException {
            Logger.d(TAG, "handleImError() entry, session:" + mSessionId + " Call Id: " + mCallId
                    + " error is " + error);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (error == IM_INITIAL_ERROR) {
                        try {
                            InstantMessage message = mCurrentSession.getFirstMessage();
                            if (message == null) {
                                Logger.d(TAG, "handleImError() message is null");
                            }
                            String messageId = message.getMessageId();
                            if (messageId == null) {
                                Logger.d(TAG, "handleImError() messageId is null");
                            } else {
                                Logger.d(TAG, "handleImError() update message status of messageId:"
                                        + messageId);
                            }
                            /**
                             * Added to notify the user that the sip invitation is failed.@{
                             */
                            for (int i = 0; i < mLastISentChatMessageList.size(); i++) {
                                ISentChatMessage sentChatMessage = mLastISentChatMessageList.get(i);
                                if (sentChatMessage.getId().equals(messageId)) {
                                    sentChatMessage.updateStatus(Status.FAILED);
                                    mLastISentChatMessageList.remove(i);
                                    break;
                                }
                            }
                            /**
                             * @}
                             */
                        } catch (RemoteException e) {
                            Logger.d(TAG, "handleImError() exception: " + e.toString());
                        }
                    }
                    destroySelf();
                }
            });
        }

        @Override
        public void handleIsComposingEvent(String contact, final boolean status)
                throws RemoteException {
            Logger.d(TAG, "handleIsComposingEvent()  session:" + mSessionId + " Call Id: "
                    + mCallId + " the contact is " + contact + " the status is " + status);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    ((IOne2OneChatWindow) mChatWindow).setIsComposing(status);
                }
            });
        }

        @Override
        public void handleMessageDeliveryStatus(final String msgId, final String status, final String contact, final  long timeStamp)
                throws RemoteException {
            Logger.d(TAG, "handleMessageDeliveryStatus()  session:" + mSessionId + " Call Id: "
                    + mCallId + " the msgId is " + msgId + " the status is " + status);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId, formatStatus(status), timeStamp);
                }
            });
        }

        @Override
        public void handleReceiveGeoloc(GeolocMessage msg)
        {
        	Toast.makeText(ApiManager.getInstance().getContext(), "Received", 200).show();
        }
        
        @Override
        public void handleReceiveMessage(final InstantMessage msg) throws RemoteException {
            Logger
                    .d(TAG, "handleReceiveMessage()  session:" + mSessionId + " Call Id: "
                            + mCallId + " message id:" + msg.getMessageId() + " message text:"
                            + msg.getTextMessage());
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    onReceiveMessage(msg);
                }
            });
        }

        @Override
        public void handleSessionAborted(int reason) throws RemoteException {
            checkAllCapability();
            Logger.d(TAG, "handleSessionAborted()  session:" + mSessionId + " Call Id: " + mCallId
                    + " entry");
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    destroySelf();
                }
            });
        }

        @Override
        public void handleSessionStarted() throws RemoteException {
            checkAllCapability();
            Logger.d(TAG, "handleSessionStarted()  session:" + mSessionId + " Call Id: " + mCallId
                    + " entry");
            // Do nothing
        }

        @Override
        public void handleSessionTerminatedByRemote() throws RemoteException {
            checkAllCapability();
            Logger.d(TAG, "handleSessionTerminatedByRemote()  session:" + mSessionId + " Call Id: "
                    + mCallId + " entry");
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    destroySelf();
                }
            });
        }
    }

    private static ISentChatMessage.Status formatStatus(String s) {
        Logger.d(TAG, "formatStatus entry with status: " + s);
        ISentChatMessage.Status status = ISentChatMessage.Status.SENDING;
        if (s == null) {
            return status;
        }
        if (s.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            status = ISentChatMessage.Status.DELIVERED;
        } else if (s.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
            status = ISentChatMessage.Status.DISPLAYED;
        } else {
            status = ISentChatMessage.Status.FAILED;
        }
        Logger.d(TAG, "formatStatus entry exit");
        return status;
    }

    /**
     * Judge whether participants is duplicated.
     * 
     * @param participants The participants to be compared.
     * @return True, if participants is duplicated, else false.
     */
    public boolean isDuplicated(Participant participant) {
        return mParticipant.equals(participant);
    }

    @Override
    protected void checkCapabilities() {
        checkAllCapability();
    }

    @Override
    protected void queryCapabilities() {
        checkAllCapability();
    }

    @Override
    public void handleInvitation(IChatSession chatSession, ArrayList<IChatMessage> messages, boolean isAutoAccept) {
        Logger.v(TAG, "handleInvitation entry, tag: " + mTag + " Participant: " + mParticipant);
        if (chatSession == null) {
            return;
        }
        try {
            if (chatSession.isStoreAndForward()) {
                Logger.v(TAG, "deal with store and forward");
                handleStoreAndFowardInvitation(chatSession, messages);
                return;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        try {
            chatSession.addSessionListener(new One2OneChatListener(chatSession));
        } catch (RemoteException e) {
            Logger.d(TAG, "chatSession acceptSession or addSessionListener fail");
            e.printStackTrace();
        }

        if (messages == null) {
            Logger.v(TAG, "handleInvitation messages is null");
            return;
        }
        int size = messages.size();
        Logger.d(TAG, "handleInvitation() message size: " + size);
        for (int i = 0; i < size; i++) {
            InstantMessage msg = messages.get(i).getInstantMessage();
            Logger.d(TAG, "handleInvitation()" + i + " message: "
                    + ((msg == null) ? null : msg.getTextMessage()));
            mChatWindow.addReceivedMessage(msg, !mIsInBackground);
            mAllMessages.add(new ChatMessageReceived(msg));
            mSentMessageManager.markSendingMessagesDisplayed();
            Logger.d(TAG, "handleInvitation() mIsInBackground:" + mIsInBackground);
            if (!mIsInBackground) {
                SharedPreferences settings =
                        PreferenceManager.getDefaultSharedPreferences(AndroidFactory
                                .getApplicationContext());
                boolean isSendReadReceiptChecked =
                        settings.getBoolean(SettingsFragment.RCS_SEND_READ_RECEIPT, true);
                Logger.d(TAG,
                        "handleInvitation() mIsInBackground is false and isSendReadReceiptChecked is "
                                + isSendReadReceiptChecked);
                try {
                    if (isSendReadReceiptChecked) {
                        chatSession.setMessageDisplayedStatusBySipMessage(chatSession
                                .getReferredByHeader(), msg.getMessageId(),
                                ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                    }

                    chatSession.acceptSession();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            } else {
                One2OneChat.this.addUnreadMessage(msg);

                RcsNotification.getInstance().onReceiveMessageInBackground(
                        AndroidFactory.getApplicationContext(), mTag, msg, null, 0);
            }
        }
        Logger.v(TAG, "handleInvitation exit");
    }

    private void handleStoreAndFowardInvitation(IChatSession chatSession,
            ArrayList<IChatMessage> messages) {
        Logger.i(TAG, "handleStoreAndFowardInvitation() entry");
        try {
            chatSession.acceptSession();
            chatSession.addSessionListener(new One2OneChatListener(chatSession));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        Context context = null;
        ApiManager instance = ApiManager.getInstance();
        int size = messages.size();
        for (int i = 0; i < size; i++) {
            InstantMessage msg = messages.get(i).getInstantMessage();
            mChatWindow.addReceivedMessage(msg, !mIsInBackground);
            mAllMessages.add(new ChatMessageReceived(msg));
            if (!mIsInBackground) {
                markMessageAsDisplayed(msg);
            } else {
                One2OneChat.this.addUnreadMessage(msg);
                if (instance != null) {
                    context = instance.getContext();
                    if (context != null) {
                        RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag,
                                msg, null, 0, true);
                    }
                }
            }
        }
        Logger.i(TAG, "handleStoreAndFowardInvitation() exit");
    }

    @Override
    protected synchronized void onResume() {
        super.onResume();
        Logger.v(TAG, "onResume() entry");
        RcsNotification.getInstance().cancelFileTransferNotificationWithContact(
                mParticipant.getContact());
        RcsNotification.getInstance().cancelReceiveMessageNotification(mTag);
        checkAllCapability();
        acceptPendingSession();
        Logger.v(TAG, "onResume() exit");
    }

    private void acceptPendingSession() {
        IChatSession pendingSession = getSessionByState(SessionState.PENDING);
        Logger.v(TAG, "acceptPendingSession() entry : pendingSession = " + pendingSession);
        if (null != pendingSession) {
            Logger.d(TAG, "acceptPendingSession() pending session found");
            try {
                pendingSession.acceptSession();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onStatusChanged(boolean status) {
        Logger.d(TAG, "onStatusChanged the status is " + status
                + ", mFileTransferController = " + mFileTransferController);
		Logger.d(TAG, "resumeFileSend onStatusChanged");
        if (status) {
            // Re-send the stored unregistered messages only if the status is
            // true
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mMessageDepot.resendStoredMessages();
                }
            });

            if (mFileTransferController != null) {
                mFileTransferController.setRegistrationStatus(true);
                mFileTransferController.controlFileTransferIconStatus();
            } 
			ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
			Logger.d(TAG, "resumeFileSend 03");
			if (instance != null) {
				instance.handleFileResumeAfterStatusChange();
				Logger.d(TAG, "resumeFileSend 04");
			}
        } else {
            if (mFileTransferController != null) {
                mFileTransferController.setRegistrationStatus(false);
                mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
			if(!RcsSettings.getInstance().isFtAlwaysOn()){
            if (instance != null) {
                instance.handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
            }
            mReceiveFileTransferManager.cancelReceiveFileTransfer();
        }
    }
    }

    /**
     * Get participant of this chat.
     * 
     * @return participant of this chat.
     */
    public Participant getParticipant() {
        return mParticipant;
    }

    @Override
    public void loadChatMessages(int count) {
        Logger.v(TAG, "loadChatMessage entry!");
        Context context;
        if (ApiManager.getInstance() != null) {
            context = ApiManager.getInstance().getContext();
        } else {
            Logger.e(TAG, "ApiManager getInstance is null!");
            return;
        }
        Logger.e(TAG, "Then load history , context = " + context + "mChatWindow = "
                + mChatWindow + "mAllMessages = " + mAllMessages);
        if (context != null && mChatWindow != null && mAllMessages != null) {
            QueryHandler queryHandler =
                    new QueryHandler(ApiManager.getInstance().getContext(), mChatWindow,
                            mAllMessages);

            // Do not take the chat terminated messages
            String chatTerminatedExcludedSelection =
                    " AND NOT((" + RichMessagingData.KEY_TYPE + "==" + TYPE_CHAT_SYSTEM_MESSAGE
                            + ") AND (" + RichMessagingData.KEY_STATUS + "== " + STATUS_TERMINATED
                            + "))";

            // Do not take the group chat entries concerning this contact
            chatTerminatedExcludedSelection +=
                    " AND NOT(" + RichMessagingData.KEY_TYPE + "=="
                            + TYPE_GROUP_CHAT_SYSTEM_MESSAGE + ")";

            // take all concerning this contact
            String firstMessageId = null;
            for (int i = 0; i < mAllMessages.size(); i++) {
                IChatMessage firstChatMessage = mAllMessages.get(i);
                if (firstChatMessage != null) {
                    InstantMessage firstInstantMessage = firstChatMessage.getInstantMessage();
                    if (firstInstantMessage != null) {
                        firstMessageId = firstInstantMessage.getMessageId();
                        break;
                    }
                }
            }
            queryHandler.startQuery(count, firstMessageId, RichMessagingData.CONTENT_URI, null,
                    RichMessagingData.KEY_CONTACT + "='"
                            + PhoneUtils.formatNumberToInternational(mParticipant.getContact())
                            + "'" + chatTerminatedExcludedSelection, null,
                    RichMessagingData.KEY_TIMESTAMP + " DESC");
        }
    }

    private static class QueryHandler extends AsyncQueryHandler {
        IChatWindow mWindow;
        CopyOnWriteArrayList<IChatMessage> mAllMessages;

        public QueryHandler(Context context, IChatWindow chatWindow,
                CopyOnWriteArrayList<IChatMessage> allMessages) {
            super(context.getContentResolver());
            mWindow = chatWindow;
            mAllMessages = allMessages;
        }

        private boolean judgeUnLoadedHistory(Cursor cursor, Object firstMessageId) {
            String firstMessage;
            if (firstMessageId == null) {
                firstMessage = "";
                Logger.d(TAG, "judgeUnLoadedHistory, firstMessageId is null");
            } else {
                firstMessage = firstMessageId.toString();
                Logger.d(TAG, "judgeUnLoadedHistory, firstMessageId = " + firstMessageId);
            }
            if (cursor != null && !cursor.isAfterLast()) {
                do {
                    int messageType =
                            cursor.getInt(cursor.getColumnIndex(RichMessagingData.KEY_TYPE));
                    String messageId =
                            cursor.getString(cursor
                                    .getColumnIndex(RichMessagingData.KEY_MESSAGE_ID));
                    if (messageType == EventsLogApi.TYPE_INCOMING_CHAT_MESSAGE
                            || messageType == EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE) {
                        if (null != messageId && !messageId.equals(firstMessage)) {
                            return true;
                        } else {
                            Logger.d(TAG, "judgeUnLoadedHistory, the two message is the same!");
                        }
                    }
                } while (cursor.moveToNext());
            } else {
                Logger.w(TAG, "judgeUnLoadedHistory, The cursor is null or cursor is last data!");
            }
            return false;
        }

        @Override
        protected void onQueryComplete(int count, Object loadedId, Cursor cursor) {
            Logger.v(TAG, "onQueryComplete enter!");
            String messageId = null;
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    if (loadedId != null) {
                        findLoadedMessage(cursor,loadedId);
                    }
                    // judge if have history, show the header
                    if (count == 0) {
                        if (judgeUnLoadedHistory(cursor, loadedId)) {
                            mWindow.addLoadHistoryHeader(true);
                        } else {
                            mWindow.addLoadHistoryHeader(false);
                        }
                        return;
                    }
                    loadMessage(cursor,messageId,loadedId,count);
                    // if have next message set header visible
                    if (judgeUnLoadedHistory(cursor, messageId)) {
                        mWindow.addLoadHistoryHeader(true);
                    } else {
                        mWindow.addLoadHistoryHeader(false);
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        /**
         * Find the message from cursor with a id loadedId and move the cursor to next
         * @param cursor A cursor
         * @param loadedId message id
         * @return True if the message with a loadedId exist in cursor,
         *         otherwise return false.
         */
        private boolean findLoadedMessage(Cursor cursor, Object loadedId) {
            do {
                String id = cursor.getString(cursor
                        .getColumnIndex(RichMessagingData.KEY_MESSAGE_ID));
                if (id != null && id.equals(loadedId)) {
                    if (!cursor.isAfterLast()) {
                        cursor.moveToNext();
                    }
                    return true;
                }
            } while (cursor.moveToNext());
            return false;
        }

        /**
         * Load message from cursor to memory
         * @param cursor A cursor
         * @param messageId A message Id, it is a output parameter
         * @param loadedId The loaded message Id
         * @param count The number of message to be loaded
         * @return True, always return true.
         */
        private boolean loadMessage(Cursor cursor, String messageId, Object loadedId, int count) {
            Logger.v(TAG, "loadMessage(): messageId = " + messageId + ", loadedId = " + loadedId
                    + ", count = " + count);
            int i = 0;
            if (!cursor.isAfterLast()) {
                do {
                    InstantMessage message = null;
                    messageId = cursor.getString(cursor
                            .getColumnIndex(RichMessagingData.KEY_MESSAGE_ID));
                    int messageType = cursor.getInt(cursor
                            .getColumnIndex(RichMessagingData.KEY_TYPE));
                    String messageData = cursor.getString(cursor
                            .getColumnIndex(RichMessagingData.KEY_DATA));
                    int messageStatus = cursor.getInt(cursor
                            .getColumnIndex(RichMessagingData.KEY_STATUS));
                    Date date = new Date(cursor.getLong(cursor
                            .getColumnIndex(RichMessagingData.KEY_TIMESTAMP)));
                    String remount = cursor.getString(cursor
                            .getColumnIndex(RichMessagingData.KEY_CONTACT));
                    message = new InstantMessage(messageId, remount, messageData, true, date);
                    message.setDate(date);
                    if (messageType == EventsLogApi.TYPE_INCOMING_CHAT_MESSAGE) {
                        if (messageId.equals(loadedId)) {
                            continue;
                        }
                        message.setIsHistory(true);
                        mWindow.addReceivedMessage(message, true);
                        mAllMessages.add(0, new ChatMessageReceived(message));
                        i++;
                    } else if (messageType == EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE) {
                        if (messageId.equals(loadedId)) {
                            continue;
                        }
                        message.setIsHistory(true);
                        ISentChatMessage sent = mWindow.addSentMessage(message, -1);
                        mAllMessages.add(0, new ChatMessageSent(message));
                        Logger.d(TAG, "sent = " + sent);
                        sent.updateStatus(getStatusEnum(messageStatus));
                        i++;
                    }
                } while (cursor.moveToNext() && i < count);
            }
            return true;
        }

    }

    /**
     * Get message status from database.
     * 
     * @return Status enum.
     */
    private static Status getStatusEnum(int status) {
        Logger.v(TAG, "getStatusEnum entry, status = " + status);
        switch (status) {
            case EventsLogApi.STATUS_DISPLAYED:
                return Status.DISPLAYED;
            case EventsLogApi.EVENT_INVITED:
                return Status.DISPLAYED;
            case EventsLogApi.STATUS_SENT:
                return Status.FAILED;
            case EventsLogApi.STATUS_DELIVERED:
                return Status.DELIVERED;
            default:
                return Status.FAILED;
        }
    }

    /**
     * Generate an sent file transfer instance in a specific chat window
     * 
     * @param filePath The path of the file to be sent
     * @return A sent file transfer instance in a specific chat window
     */
    public SentFileTransfer generateSentFileTransfer(String filePath, Object fileTransferTag) {
        return new SentFileTransfer(mTag, (IOne2OneChatWindow) mChatWindow, filePath, mParticipant,
                fileTransferTag);
    }
    
    

   

    /**
     * Handle a file transfer invitation
     */
    public void addReceiveFileTransfer(IFileTransferSession fileTransferSession, boolean isAutoAccept,boolean isGroup) {
    	Logger.v(TAG, "O2O addReceiveFileTransfer isAutoAccept" + isAutoAccept+" isGroup:"+isGroup);
        mReceiveFileTransferManager.addReceiveFileTransfer(fileTransferSession, isAutoAccept,isGroup);
    }

    /**
     * Handle reject a file transfer invitation
     */
    public void handleRejectFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "handleRejectFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.rejectFileTransferInvitation();
        }
    }

    /**
     * Handle accept a file transfer invitation
     */
    public void handleAcceptFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "handleAcceptFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.acceptFileTransferInvitation();
        }
    }

    /**
     * Handle cancel a file transfer invitation
     */
    public void handleCancelFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "handleCancelFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.cancelFileTransfer();
        }
    }

	/**
     * Handle pause a file transfer 
     */
    public void handlePauseReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "handlePauseReceiveFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			Logger.d(TAG,"handlePauseReceiveFileTransfer 1");
            receiveFileTransfer.onPauseReceiveTransfer();
        }
    }
    
   /**
     * Handle resume a file transfer 
     */
    public void handleResumeReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "handleResumeReceiveFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			Logger.d(TAG,"handleResumeReceiveFileTransfer 1");
            receiveFileTransfer.onResumeReceiveTransfer();
        }
    }

    @Override
    public void onCapabilityChanged(String contact, Capabilities capabilities) {
        Logger.w(TAG, "onCapabilityChanged() entry the contact is " + contact + " capabilities is "
                + capabilities);
        if (mParticipant == null) {
            Logger.d(TAG, "onCapabilityChanged() mParticipant is  null");
            return;
        }
        String participantNumber = mParticipant.getContact();
        Logger.w(TAG, "onCapabilityChanged() participantNumbert is " + participantNumber);
        if (participantNumber.equals(contact)) {
            Logger.d(TAG, "onCapabilityChanged() the participant equals the contact and number is "
                    + contact);
            if (capabilities.isSupportedRcseContact() || ContactsListManager.getInstance().isLocalContact(participantNumber)
                    || ContactsListManager.getInstance().isStranger(participantNumber)) {
                Logger.w(TAG, "onCapabilityChanged() the participant is rcse contact " + contact);
                onCapabilityChangedWhenRemoteIsRcse(contact, capabilities);
            } else {
                Logger.w(TAG, "onCapabilityChanged() the participant is not rcse contact "
                        + contact);
				if(!RcsSettings.getInstance().isFtAlwaysOn()){
                if (mFileTransferController != null) {
                    mFileTransferController.setRemoteFtCapability(false);
                    mFileTransferController.controlFileTransferIconStatus();
                }
                IChatManager instance = ModelImpl.getInstance();
                if (instance != null) {
                    ((ModelImpl) instance).handleFileTransferNotAvailable(mTag,
                            FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
                }
                mReceiveFileTransferManager.cancelReceiveFileTransfer();
					}
                Logger.w(TAG, "mChatWindow = " + mChatWindow);
                if (mChatWindow != null) {
                    ((IOne2OneChatWindow) mChatWindow)
                            .setRemoteOfflineReminder(false);
                }
            }
        }
        Logger.d(TAG, "onCapabilityChanged() exit");
    }
    
    private void onCapabilityChangedWhenRemoteIsRcse(String number, Capabilities capabilities) {
        Logger.d(TAG,
                "onCapabilityChangedWhenRemoteIsRcse() the participant support rcse "
                        + number + ", mFileTransferController = "
                        + mFileTransferController);
        if (capabilities.isFileTransferSupported()) {
            Logger.d(TAG, "onCapabilityChangedWhenRemoteIsRcse() is filetransfersupported");
            if (mFileTransferController != null) {
                mFileTransferController.setRemoteFtCapability(true);
                mFileTransferController.controlFileTransferIconStatus();
            }
        } else if(!RcsSettings.getInstance().isFtAlwaysOn()){
            Logger.d(TAG, "onCapabilityChanged isn't filetransfersupported");
            if (mFileTransferController != null) {
                mFileTransferController.setRemoteFtCapability(false);
                mFileTransferController.controlFileTransferIconStatus();
            }
            IChatManager instance = ModelImpl.getInstance();
            if (instance != null) {
                ((ModelImpl) instance).handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
            }
            mReceiveFileTransferManager.cancelReceiveFileTransfer();
        }
        ContactInfo info = ContactsManager.getInstance().getContactInfo(number);
        Logger.d(TAG, "onCapabilityChanged() contact info is " + info);
        int registrationState = info.getRegistrationState();
        if (ContactInfo.REGISTRATION_STATUS_ONLINE == registrationState ||  RcsSettings.getInstance().isImAlwaysOn()) {
            Logger.w(TAG, "onCapabilityChangedWhenRemoteIsRcse() ,the participant " + number
                    + " is online");
            ((IOne2OneChatWindow) mChatWindow).setRemoteOfflineReminder(false);
			mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mMessageDepot.resendStoredMessages();
                }
            });
        } else {
            boolean isLocalContact = ContactsListManager.getInstance().isLocalContact(number);
            if (isLocalContact) {
                Logger.w(TAG, "onCapabilityChangedWhenRemoteIsRcse() ,the participant " + number
                        + " is offline");
                ((IOne2OneChatWindow) mChatWindow).setRemoteOfflineReminder(true);
            }
        }
    }

    @Override
    public void onCapabilitiesChangedListener(Capabilities capabilities) {
        Logger.d(TAG, "onCapabilitiesChangedListener() entry : capabilites is "
                + capabilities + ", mFileTransferController = "
                + mFileTransferController);
        if (capabilities == null) {
            return;
        }
        if (capabilities.isFileTransferSupported()) {
            Logger.d(TAG, "onCapabilitiesChangedListener() self filetransfer support");
            if (mFileTransferController != null) {
                mFileTransferController.setLocalFtCapability(true);
                mFileTransferController.controlFileTransferIconStatus();
            } 
        } else {
            Logger.d(TAG, "onCapabilitiesChangedListener() self filetransfer not support");
            if (mFileTransferController != null) {
                mFileTransferController.setLocalFtCapability(false);
                mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
			if(!RcsSettings.getInstance().isFtAlwaysOn()){
            if (instance != null) {
                instance.handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
            }
            if (mReceiveFileTransferManager != null) {
                mReceiveFileTransferManager.cancelReceiveFileTransfer();
            }
        }
        }
        Logger.d(TAG, "onCapabilitiesChangedListener() exit ");
    }
    
    

    private final MessageDepot mMessageDepot = new MessageDepot();
    
    /**
     * Added to notify the user that the sip invite is failed.@{
     */
    private final ArrayList<ISentChatMessage> mLastISentChatMessageList = new ArrayList<ISentChatMessage>();
    /**
     * @}
     */
    
    /**
     * The class is used to manage the stored message sent in unregistered status
     */
    private class MessageDepot {
        public final String tag = "MessageDepot@" + mTag;
        private static final String WHERE = UnregMessageProvider.KEY_CHAT_TAG + "=?";
        private final String[] mSelectionArg = new String[] {
                mTag.toString()
        };

		

        private final Map<Integer, WeakReference<ISentChatMessage>> mMessageMap = 
                                    new ConcurrentHashMap<Integer, WeakReference<ISentChatMessage>>();

        /**
         * Check whether need to add a new message to View
         * @param messageTag
         * @return null if this message is not for resent, otherwise an instance of ISentChatMessage
         */
        public ISentChatMessage checkIsResend(int messageTag) {
            Logger.d(tag, "checkIsResend() entry, messageTag: " + messageTag);
            WeakReference<ISentChatMessage> reference = mMessageMap.get(messageTag);
            if (null != reference) {
                ISentChatMessage message = reference.get();
                mMessageMap.remove(messageTag);
                Logger.d(tag, "checkIsResend() message: " + message);
                return message;
            } else {
                return null;
            }
        }

        public void updateStoredMessage(int messageTag, ISentChatMessage message) {
            Logger.d(tag, "updateStoredMessage() messageTag: " + messageTag);
            mMessageMap.put(messageTag, new WeakReference<ISentChatMessage>(message));
        }

        public InstantMessage storeMessage(String content, int messageTag) {
            Logger.d(tag, "storeMessage() entry : content =" + content
                    + ",messageTag = " + messageTag);
            if (!TextUtils.isEmpty(content)) {
                ApiManager apiManager = ApiManager.getInstance();
                Logger.d(tag, "storeMessage() : apiManager ="
                        + apiManager);
                Context context = null;
                ContentResolver resolver = null;
                if (apiManager != null) {
                    context = apiManager.getContext();
                    Logger.d(tag, "storeMessage() : context ="
                            + context);
                    if (context != null && mParticipant != null) {
                        resolver = context.getContentResolver();
						InstantMessage msg = new InstantMessage(ChatUtils.generateMessageId(), mParticipant.getContact(), content, true, new Date());
                        ContentValues values = new ContentValues();
                        values.put(UnregMessageProvider.KEY_CHAT_TAG, mTag.toString());
                        values.put(UnregMessageProvider.KEY_MESSAGE_TAG, messageTag);
                        values.put(UnregMessageProvider.KEY_MESSAGE, content);
						values.put(UnregMessageProvider.KEY_MESSAGE_ID, msg.getMessageId());
                        resolver.insert(UnregMessageProvider.CONTENT_URI, values);
                        Logger.d(tag, "storeMessage() Store messages while the sender is not registered");
                       // return new InstantMessage(ChatUtils.generateMessageId(), mParticipant.getContact(), content, true, new Date());
                       return msg;
                    }
                }
            }
            return null;
        }

        public void resendStoredMessages() {
            ContentResolver resolver = ApiManager.getInstance().getContext().getContentResolver();

            // Query
            Cursor cursor = null;
            try {
                cursor = resolver.query(UnregMessageProvider.CONTENT_URI, null, WHERE, mSelectionArg, null);
                if (cursor != null) {
                    int messageIndex = cursor.getColumnIndex(UnregMessageProvider.KEY_MESSAGE);
                    int messageTagIndex = cursor.getColumnIndex(UnregMessageProvider.KEY_MESSAGE_TAG);
                    for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
                        String message = cursor.getString(messageIndex);
                        int messageTag = cursor.getInt(messageTagIndex);
                        One2OneChat.this.sendMessage(message, messageTag);
                        Logger.i(tag, "resendStoredMessages() chat[" + cursor.getString(0) + "] send message" + message
                                + " with message tag: " + messageTag);
                    }
                }
            } catch (SQLiteException e) {
                e.printStackTrace();
                Logger.e(tag, "resendStoredMessages() Query exception");
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
            // Delete
            /*
            try {
                int counts = resolver.delete(UnregMessageProvider.CONTENT_URI, WHERE, mSelectionArg);
                Logger.i(tag, "resendStoredMessages() Remove " + counts + " messages");
            } catch (SQLiteException e) {
                e.printStackTrace();
                Logger.e(tag, "resendStoredMessages() Query exception");
            }
            */
        }

        public void removeUnregisteredMessage() {
            ContentResolver resolver = ApiManager.getInstance().getContext().getContentResolver();
            int count = resolver.delete(UnregMessageProvider.CONTENT_URI, WHERE, mSelectionArg);
            Logger.i(tag, "removeUnregisteredMessage() Delete " + count + " unregistered sending messages");
            mMessageMap.clear();
        }
    }

    @Override
    protected void reloadFileTransfer(final FileStruct fileStruct, final int transferType, final int messageStatus) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "reloadFileTransfer()->run() entry, file transfer tag: " + fileStruct.mFileTransferTag
                        + "file transfer path: " + fileStruct.mFilePath + " , transferType: " + transferType
                        + ", messageStatus: " + messageStatus);
                IFileTransfer fileTransfer = null;
                if (EventsLogApi.TYPE_INCOMING_FILE_TRANSFER == transferType) {
                    fileTransfer = ((IOne2OneChatWindow) mChatWindow).addReceivedFileTransfer(fileStruct, false);
                    if (fileTransfer != null) {
                        if (fileStruct.mFilePath == null) {
                            if((mReceiveFileTransferManager!=null) &&(messageStatus == EventsLogApi.STATUS_STARTED || messageStatus == EventsLogApi.STATUS_IN_PROGRESS))
                            {   
                                if(mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag) == null) 
                                {
                                	fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                                }
                            	else
                                {
                                Logger.d(TAG,
                                        "reloadFileTransfer(), file path is null, set status Transferring!");                           
                                mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag).mFileTransfer = fileTransfer;                               
                                if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING)
                                fileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING);
                                else if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING)
                                    fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING);
                                else if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED)
                                    fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED);
                               }
                            }
                            else
                            {
                            Logger.d(TAG,
                                    "reloadFileTransfer(), file path is null, set status failed!");
                            fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                            }
                        } else {
                            Logger.d(TAG, "reloadFileTransfer(), set status finished!");
                            fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                        }
                    } else {
                        if(mReceiveFileTransferManager!=null)
                        {
                            fileTransfer = mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag).mFileTransfer;
                            fileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING);
                        Logger.e(TAG, "reloadFileTransfer(), fileTransfer is null!");
                    }
                    }
                } else if (EventsLogApi.TYPE_OUTGOING_FILE_TRANSFER == transferType) {
                    fileTransfer = ((IOne2OneChatWindow) mChatWindow).addSentFileTransfer(fileStruct);
                    Logger.d(TAG, "reloadFileTransfer(), messageStatus = " + messageStatus);
                    if (fileTransfer != null && messageStatus == EventsLogApi.STATUS_TERMINATED) {
                        fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                    }
                    /* We can have a single condition which sets Fail in all cases other then terminated */
                    else 
                        fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                }
            }
        };
        mWorkerHandler.post(worker);
    }

    @Override
    protected void reloadMessage(final InstantMessage message, final int messageType,
            final int messageStatus) {
        Logger.d(TAG, "reloadMessage() message " + message.getMessageId() + " " + this
                + " messageStatus is " + messageStatus);
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "reloadMessage()->run() entry, message id: " + message.getMessageId()
                        + "message text: " + message.getTextMessage() + " , messageType: "
                        + messageType);
                if (EventsLogApi.TYPE_INCOMING_CHAT_MESSAGE == messageType) {
                	if(mChatWindow != null)
                	{
                    Logger.d(TAG, "reloadMessage() the mchatwindow is " + mChatWindow);
                    mChatWindow.addReceivedMessage(message, true);
                    mAllMessages.add(new ChatMessageReceived(message));
                	}
                } else if (EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE == messageType) {
                    //Check if error condition
                	if(mChatWindow != null)
                	{
                    ISentChatMessage sentMessage = mChatWindow.addSentMessage(message, -1);
                    if (sentMessage != null) {
                        sentMessage.updateStatus(getStatusEnum(messageStatus));
                    } else {
                        Logger.d(TAG, "reloadMessage() the ISentChatMessage is " + null);
                    }
                    mAllMessages.add(new ChatMessageSent(message));
                }
            }
            }
        };
        mWorkerHandler.post(worker);
    }
}
