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

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.EmptyStackException;
import java.util.HashMap;
import java.util.List;
import java.util.Stack;
import java.util.TimerTask;
import java.util.concurrent.CopyOnWriteArrayList;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageReceived;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageSent;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.network.INetworkConnectivity;
import com.orangelabs.rcs.core.ims.network.NetworkConnectivityApi;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.provider.messaging.RichMessaging;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.GeolocMessage;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.server.messaging.ImSession;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;


/**
 * The implement of group chat. It provides session management, session event
 * handling, and message sending.
 */
public class GroupChat extends ChatImpl {
    public static final String TAG = "GroupChat";
    private ArrayList<InstantMessage> mReceivedMessagesDelayed = new ArrayList<InstantMessage>();
    private ArrayList<IChatMessage> mAllMessages = new ArrayList<IChatMessage>();
    private GroupChatParticipants mCurrentParticipants = new GroupChatParticipants();
    public static final int TIME_OUT = RcsSettings.getInstance().getRingingPeriod() * 1000 + 5000;
    private boolean mIsRegistered = false;
    public ImSession mchatSession;
    private NetworkConnectivityListener mNetworkListener = null;
    private Stack<SessionInfo> mSessionStack = new Stack<SessionInfo>();
    private HashMap<InstantMessage, Integer> mMessagesToSendDelayed = new HashMap<InstantMessage, Integer>();
    private Handler mUiHandler = new GroupChatHandler(Looper.getMainLooper());
    private final Context mContext;
    private/* final */MessagingApi mMessagingApi;
    private/* final */CapabilityApi mCapabilityApi;
    private final EventsLogApi mEventsLogApi;
    private RegistrationApi mRegistrationApi;
    private NetworkConnectivityApi mNetworkConnectivityApi;
    private String mChatId = null;
    private String mChatTitle = "";
    private boolean mInvite = false;
   
    public boolean ismInvite() {
		return mInvite;
	}

	public void setmInvite(boolean mInvite) {
		this.mInvite = mInvite;
	}

    /**
     * M: managing extra local chat participants that are 
     * not present in the invitation for sending them invite request.@{
     */
    private ArrayList<Participant> mExtraLocalparticipants = null;
    private boolean isResendGrpInvite = false;
    /**
	 * @}
	 */ 
    

    public void setmChatId(String mChatId) {
        this.mChatId = mChatId;
    }
    
    /**
     * Generate an sent file transfer instance in a specific chat window
     * 
     * @param filePath The path of the file to be sent
     * @return A sent file transfer instance in a specific chat window
     */
    public SentFileTransfer generateSentFileTransfer(String filePath, Object fileTransferTag) {

    	
    	List<String> participants = new ArrayList<String>();
    	for(ParticipantInfo p :mCurrentParticipants.mParticipants)
    	{
    		participants.add(p.getContact());
    	}
    		
    	 return new SentFileTransfer(mTag, (IGroupChatWindow) mChatWindow, filePath, participants,
                 fileTransferTag);
    }
    
    
    
    public void addGroupSubject(String title, int sendInvite)
    {    
        mChatTitle = title;
    	((IGroupChatWindow)mChatWindow).addgroupSubject (title);
    	if (mMessagingApi == null) {
            mMessagingApi = ApiManager.getInstance().getMessagingApi();
        }
        if (mMessagingApi == null) {
            Logger.e(TAG, "sendMessage(), mMessagingApi is null");
            return;
        }

    };
    
    public void addGroupSubjectFromInvite(String title)
    {    
        mChatTitle = title;
        ((IGroupChatWindow)mChatWindow).addgroupSubject (title);        
    }

	public  String getGroupSubject()
    {
       return mChatTitle;
    }

    /**
     * Handler to deal with some common function.
     */
    private class GroupChatHandler extends Handler {
        /**
         * Update send button.
         */
        public static final int UPDATE_SEND_BUTTON = 1;
        /**
         * No IM capability.
         */
        public static final int IM_FAIL_TOAST = 2;

        private GroupChatHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case UPDATE_SEND_BUTTON:
                    SessionStatus status = SessionStatus.UNKNOWN;
                    try {
                        SessionInfo info = mSessionStack.peek();
                        status = info.getSessionStatus();
                    } catch (EmptyStackException e) {
                        e.printStackTrace();
                    }
                    Logger.d(TAG, "handleMessage(), status: " + status);
                    if (!mIsRegistered) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_UNAVIALABLE);
                    } else if (status == SessionStatus.REJOINING
                            || status == SessionStatus.AUTO_REJOIN) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_REJOINING);
                    } else if (status == SessionStatus.RESTARTING) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_RESTARTING);
                    } else if (status == SessionStatus.TERMINATED) {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_TERMINATED);
                    } else {
                        ((IGroupChatWindow) mChatWindow)
                                .updateChatStatus(Utils.GROUP_STATUS_CANSENDMSG);
                    }
                    break;
                case IM_FAIL_TOAST:
                    Toast.makeText(mContext,
                            mContext.getString(R.string.im_capabillity_failed, msg.obj),
                            Toast.LENGTH_LONG).show();
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * Session info.
     */
    public static class SessionInfo {
        private IChatSession mSession;
        private SessionStatus mSessionStatus = SessionStatus.UNKNOWN;

        public SessionInfo(IChatSession session, SessionStatus status) {
            mSession = session;
            mSessionStatus = status;
        }

        /**
         * Set chat session.
         * 
         * @param session The chat session.
         */
        public void setSession(IChatSession session) {
            mSession = session;
        }

        /**
         * Get chat session.
         * 
         * @return Chat session.
         */
        public IChatSession getSession() {
            return mSession;
        }

        /**
         * Set session status.
         * 
         * @param status Session status.
         */
        public void setSessionStatus(SessionStatus status) {
            mSessionStatus = status;
        }

        /**
         * Get session status.
         * 
         * @return Session status.
         */
        public SessionStatus getSessionStatus() {
            return mSessionStatus;
        }

        /**
         * To string.
         */
        @Override
        public String toString() {
            return "session= " + mSession + " mSessionStatus= " + mSessionStatus;
        }
    }

    public class GroupChatParticipants {

        private class TimerOutTimer extends TimerTask {
            ParticipantInfo mTimerParticipantInfo = null;

            TimerOutTimer(ParticipantInfo info) {
                mTimerParticipantInfo = info;
            }

            @Override
            public void run() {
                if (mTimerParticipantInfo != null
                        && mTimerParticipantInfo.mState.equals(User.STATE_PENDING)) {
                    ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
                }
                cancel();
            }
        }

        private CopyOnWriteArrayList<ParticipantInfo> mParticipants = new CopyOnWriteArrayList<ParticipantInfo>();
        private CopyOnWriteArrayList<TimerOutTimer> mTimerOutTimers = new CopyOnWriteArrayList<TimerOutTimer>();

        private TimerOutTimer findTimerOutTimer(ParticipantInfo info) {
            for (TimerOutTimer timer : mTimerOutTimers) {
                if (timer.mTimerParticipantInfo.mParticipant.equals(info.mParticipant)) {
                    return timer;
                }
            }
            return null;
        }

        private void initicalGroupChatParticipant(List<Participant> participants) {
            if (null != participants && 0 != participants.size()) {
                for (Participant participant : participants) {
                    mParticipants.add(new ParticipantInfo(participant, User.STATE_PENDING));
                }
            }
        }

        private void addAll(List<Participant> toAdd) {
            if (null != toAdd && 0 != toAdd.size()) {
                for (Participant participant : toAdd) {
                    ParticipantInfo info = new ParticipantInfo(participant, User.STATE_PENDING);
                    ModelImpl.TIMER.schedule(new TimerOutTimer(info), TIME_OUT);
                    mParticipants.add(info);
                }
            }
            ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
        }

        /**
         * Convert participant info to participant
         * 
         * @return group chat participant list
         */
        public ArrayList<Participant> convertToParticipants() {
            ArrayList<Participant> participants = new ArrayList<Participant>();
            for (ParticipantInfo info : mParticipants) {
                participants.add(info.mParticipant);
            }
            return participants;
        }

        /**
         * Convert participant info to number Return group chat participant
         * number list
         */
        public ArrayList<String> convertParticipantsToNumbers() {
            ArrayList<String> participants = new ArrayList<String>();
            for (ParticipantInfo info : mParticipants) {
                participants.add(info.mParticipant.getContact());
            }
            return participants;
        }

        private void add(Participant participant, String state) {
            ParticipantInfo info = new ParticipantInfo(participant, state);
            mParticipants.add(info);
        }

        private void remove(ParticipantInfo participantInfo) {
            mParticipants.remove(participantInfo);
        }

        /**
         * Update model data
         * 
         * @param contact Remote contacts
         * @param contactDisplayname Remote contacts display name
         * @param state Remote user state
         * @param toBeInvited Participants to be invited.
         * @return True if contact is to be invited.
         */
        public boolean updateParticipants(String contact, String contactDisplayname, String state,
                List<String> toBeInvited) {
            Logger.v(TAG, "updateParticipants(), contact: " + contact + " contactDisplayname: "
                    + contactDisplayname + " state: " + state);
            boolean result = false;
            boolean isNewAdded = true;
            String displayName = null;
            String loacalDisplayName = ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact);
            if (loacalDisplayName != null) {
                displayName = loacalDisplayName;
            } else if (contactDisplayname != null) {
                displayName = contactDisplayname;
            } else {
                displayName = contact;
            }
            Participant participantToBeUpdated = new Participant(contact, displayName);
            ParticipantInfo infoToBeUpdated = new ParticipantInfo(participantToBeUpdated, state);
            int size = mParticipants.size();
            for (int index = 0; index < size; index++) {
                ParticipantInfo info = mParticipants.get(index);
                Participant participant = info.mParticipant;
                if (null != participant) {
                    String remoteContact = participant.getContact();
                    if (null != remoteContact && remoteContact.equals(contact)) {
                        info.mState = state;
                        participantToBeUpdated = participant;
                        infoToBeUpdated = info;
                        isNewAdded = false;
                        timerUnSchedule(info);
                        break;
                    }
                }
            }
            if (toBeInvited.contains(contact)) {
            	boolean isAlreadyAdded = false;
            	
            	
            		for(int j =0 ; j<mParticipants.size();j++)
            		{
	            		if(contact.equalsIgnoreCase(mParticipants.get(j).mParticipant.getContact()))
	            		{
	            			isAlreadyAdded = true;
	                        Logger.v(TAG, "ALready Added, contact: " + contact );
	                        break;

	            		}
            		}
            	
            	
            	
                if(!isAlreadyAdded)
                {
                
                isNewAdded = true;
                result = true;
            }
            }
            if (User.STATE_DECLINED.equals(state)) {
                remove(infoToBeUpdated);
                ((IGroupChatWindow) mChatWindow).setIsComposing(false, participantToBeUpdated);
            } else if (User.STATE_DISCONNECTED.equals(state)) {
                // remove(infoToBeUpdated);
                ((IGroupChatWindow) mChatWindow).setIsComposing(false, participantToBeUpdated);
            } else if (User.STATE_DEPARTED.equals(state)) {
                remove(infoToBeUpdated);
                ((IGroupChatWindow) mChatWindow).setIsComposing(false, participantToBeUpdated);
                if (!isNewAdded) {
                    ChatEventStruct chatEventStruct = new ChatEventStruct(Information.LEFT,
                            contact, new Date());
                    ((IGroupChatWindow) mChatWindow).addChatEventInformation(chatEventStruct);
                }
            } else if (User.STATE_CONNECTED.equals(state)) {
                if (isNewAdded) {
                    add(participantToBeUpdated, state);
                    ChatEventStruct chatEventStruct = new ChatEventStruct(Information.JOIN,
                            contact, new Date());
                    ((IGroupChatWindow) mChatWindow).addChatEventInformation(chatEventStruct);
                }
            }
            ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
            return result;
        }

        /**
         * Rest All connected participants' statuses to be disconnected.
         */
        public void resetAllStatus() {
            Logger.v(TAG, "resetAllStatus()");
            for (ParticipantInfo info : mParticipants) {
                if (User.STATE_CONNECTED.equalsIgnoreCase(info.mState)) {
                    info.mState = User.STATE_DISCONNECTED;
                }
            }
            ((IGroupChatWindow) mChatWindow).updateParticipants(mParticipants);
        }

        /**
         * Timer schedule
         * 
         * @param ParticipantInfo
         */
        public void timerSchedule(ParticipantInfo info) {
            Logger.d(TAG, "timerSchedule(): info = " + info);
            if (info != null) {
                TimerOutTimer timerTask = findTimerOutTimer(info);
                Logger.v(TAG, "timerSchedule(): timerTask = " + timerTask);
                if (info.mState.equals(User.STATE_PENDING)) {
                    if (timerTask == null) {
                        timerTask = new TimerOutTimer(info);
                        mTimerOutTimers.add(timerTask);
                        ModelImpl.TIMER.schedule(timerTask, TIME_OUT);
                    }
                }
            }
        }

        /**
         * Timer stop schedule
         * 
         * @param ParticipantInfo
         */
        public void timerUnSchedule(ParticipantInfo info) {
            Logger.d(TAG, "timerUnSchedule(): info = " + info);
            if (info != null) {
                TimerOutTimer timerTask = findTimerOutTimer(info);
                if (timerTask != null) {
                    timerTask.cancel();
                    mTimerOutTimers.remove(timerTask);
                }
            }
        }
    }

    /**
     * Get GroupChat Participant list
     * 
     * @return ParticipantInfo
     */
    public List<ParticipantInfo> getParticipantInfos() {
        return mCurrentParticipants.mParticipants;
    }

    public GroupChat(ModelImpl modelImpl, IGroupChatWindow chatWindow,
            List<Participant> participants, Object tag) {
        modelImpl.super(tag);
        Logger.v(TAG, "GroupChat constructor entry");
        mCurrentParticipants.initicalGroupChatParticipant(participants);
        mChatWindow = chatWindow;
        mContext = ApiManager.getInstance().getContext();
        mEventsLogApi = new EventsLogApi(mContext);
        mCapabilityApi = ApiManager.getInstance().getCapabilityApi();
        mMessagingApi = ApiManager.getInstance().getMessagingApi();
        mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
        mNetworkConnectivityApi = ApiManager.getInstance().getNetworkConnectivityApi();
        Logger.v(TAG, "GroupChat constructor exit");
    }

    @Override
    public void setChatWindow(IChatWindow chatWindow) {
        super.setChatWindow(chatWindow);
        Logger.d(TAG, "setChatWindow() entry");
        init();
        Logger.d(TAG, "setChatWindow() exit");
    }

    /**
     * Change session status.
     * 
     * @param info The old info.
     * @param status The new status.
     */
    private void changeSessionStatus(SessionInfo info, SessionStatus status) {
        Logger.d(TAG, "changeSessionStatus() entry, info: " + info + " status: " + status);
        int location = mSessionStack.indexOf(info);
        SessionStatus oldStatus = info.getSessionStatus();
        Logger.d(TAG, "changeSessionStatus() oldStatus: " + oldStatus);
        if (oldStatus == SessionStatus.TERMINATED) {
            return;
        }
        if (-1 != location) {
            mSessionStack.get(location).setSessionStatus(status);
        }
        String participants = "";
        List<ParticipantInfo> ListParticipant = mCurrentParticipants.mParticipants;

        for(ParticipantInfo currentPartc : ListParticipant){
        participants += currentPartc.mParticipant.getContact() + ";"; 

        }        
        IChatSession session = info.getSession();        
        try {
            if(info.mSessionStatus == SessionStatus.TERMINATED )
            RichMessaging.getInstance().addChatSessionTerminationOnQuitGroup(mChatId, session.getSessionID(), participants);

        Logger.d(TAG, "changeSessionStatus() exit, location: " + location);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Logger.d(TAG, "changeSessionStatus() exception " + location);
        }
    }

    private void init() {
        Logger.d(TAG, "init() entry, mNetworkListener: " + mNetworkListener);
        if (mNetworkListener == null) {
            if (mRegistrationApi == null) {
                mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
            }
            if (mNetworkConnectivityApi == null) {
                mNetworkConnectivityApi = ApiManager.getInstance().getNetworkConnectivityApi();
            }
            try {
                if (mNetworkConnectivityApi != null) {
                    mNetworkListener = new NetworkConnectivityListener();
                    mNetworkConnectivityApi.addNetworkConnectivityListener(mNetworkListener);
                }
            } catch (ClientApiException e) {
                e.printStackTrace();
            }
            if (mRegistrationApi != null) {
                mIsRegistered = mRegistrationApi.isRegistered();
            }
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
        }
        Logger.d(TAG, "init() exit, mNetworkConnectivityApi: " + mNetworkConnectivityApi
                + " mRegistrationApi: " + mRegistrationApi + " mIsRegistered: " + mIsRegistered);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Logger.d(TAG, "onDestroy() entry");
        quitGroup();
        mAllMessages.clear();
        mSessionStack.clear();
        mReceivedMessagesDelayed.clear();
        clearRestoredMessages();
        if (mNetworkConnectivityApi != null) {
            try {
                mNetworkConnectivityApi.removeNetworkConnectivityListener(mNetworkListener);
                mNetworkListener = null;
            } catch (ClientApiException e) {
                e.printStackTrace();
            }
        }
        Logger.d(TAG, "onDestroy() exit, mNetworkConnectivityApi: " + mNetworkConnectivityApi);
    }

    protected void onQuit() {
        Logger.d(TAG, "onQuit() entry");
        quitGroup();
        Logger.d(TAG, "onQuit() exit");
    }

    private void quitGroup() {
        Logger.d(TAG, "quitGroup() entry, mSessionStack size: " + mSessionStack.size());
        SessionStatus status = SessionStatus.TERMINATED;
        try {
            for (SessionInfo info : mSessionStack) {
                changeSessionStatus(info, status);
                IChatSession session = info.getSession();
                if(session!=null)
                session.cancelSession();
            }
        } catch (EmptyStackException e) {
            Logger.e(TAG, "quitGroup() EmptyStackException");
            e.printStackTrace();
        } catch (RemoteException e) {
            Logger.e(TAG, "quitGroup() RemoteException");
            e.printStackTrace();
        }
        Logger.d(TAG, "quitGroup() exit");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Logger.d(TAG, "onResume() entry, mRegistrationApi: " + mRegistrationApi);
        if (mRegistrationApi == null) {
            mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
        }
        if (mRegistrationApi != null) {
            mIsRegistered = mRegistrationApi.isRegistered();
        }
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        RcsNotification.getInstance().cancelReceiveMessageNotification(mTag);
        Logger.d(TAG, "onResume() exit, mIsRegistered: " + mIsRegistered);
    }

    /**
     * Get all the IM chat history
     * 
     * @return All messages
     */
    public ArrayList<IChatMessage> getAllMessages() {
        return mAllMessages;
    }

    /**
     * Clear the group chat history of this associated participant.
     * 
     * @return True if success, else False.
     */
    public boolean clearGroupHistory() {
        Logger.d(TAG, "clearGroupHistory() entry, mSessionStack size: " + mSessionStack.size());
        boolean success = false;
        try {
            for (SessionInfo info : mSessionStack) {
                IChatSession session = info.getSession();
                // this function is called in worker thread, so it's safe to
                // do I/O operation.
                if(session!=null)
                mEventsLogApi.deleteImSessionEntry(session.getSessionID());
            }
            
            //delete the group caht for this chatID
            RichMessagingDataProvider.getInstance().deleteGroupChat(mChatId);
            success = true;
        } catch (EmptyStackException e) {
            Logger.e(TAG, "clearGroupHistory() EmptyStackException");
            e.printStackTrace();
        } catch (RemoteException e) {
            Logger.e(TAG, "clearGroupHistory() RemoteException");
            e.printStackTrace();
        }
        clearChatWindowAndList();
        Logger.d(TAG, "clearGroupHistory() exit, success: " + success);
        return success;
    }

    /**
     * Get chat id of group chat.
     * 
     * @return Chat id.
     */
    public String getChatId() {
        return mChatId;
    }

    @Override
    public void sendMessage(String content, int messageTag) {
        Logger.d(TAG, "sendMessage() entry, content: " + content + " messageTag: " + messageTag);
        if (mMessagingApi == null) {
            mMessagingApi = ApiManager.getInstance().getMessagingApi();
        }
        if (mMessagingApi == null) {
            Logger.e(TAG, "sendMessage(), mMessagingApi is null");
            return;
        }
        IChatSession currentSession = null;
        SessionStatus status = SessionStatus.UNKNOWN;
        try {
            SessionInfo sessionInfo = mSessionStack.peek();
            currentSession = sessionInfo.getSession();
            status = sessionInfo.getSessionStatus();
            
        } catch (EmptyStackException e) {
            Logger.e(TAG, "sendMessage() EmptyStackException");
            e.printStackTrace();
        }
        Logger.d(TAG, "sendMessage() currentSession: " + currentSession + " status: " + status
                + " mChatId: " + mChatId);
        switch (status) {
            case UNKNOWN:
                sendInvite(getGroupSubject(),content , messageTag);
                break;
            case MANULLY_REJOIN:
                rejoinGroup(content, mChatId, messageTag);
                break;
            case MANULLY_RESTART:
                restartGroup(content, mChatId, messageTag);
                break;
            case ACTIVE:
                sendMsrpMessage(currentSession, content, messageTag);
                break;
            case INVITING:
            default:
                restoreMessages(content, messageTag);
                break;
        }
        Logger.d(TAG, "sendMessage() exit, content: " + content);
    }

    private void restoreMessages(final String message, final int messageTag) {
        Logger.d(TAG, "restoreMessages() message: " + message + " messageTag: " + messageTag);
        mWorkerHandler.post(new Runnable() {
            @Override
            public void run() {
                if (message != null) {
                	String id = ChatUtils.generateMessageId();
                    InstantMessage msg = new InstantMessage(id, Utils.DEFAULT_REMOTE, message,
                            true, new Date());
                    RichMessaging.getInstance().addEntry(EventsLogApi.TYPE_OUTGOING_GROUP_CHAT_MESSAGE, null, null,
            				msg.getMessageId(), msg.getRemote(), msg.getTextMessage(),
            				InstantMessage.MIME_TYPE, msg.getRemote(),
            				msg.getTextMessage().getBytes().length, msg.getDate(), EventsLogApi.STATUS_SENT);
                    synchronized(this)
                    {
                    	// TODO This is a hack because above operation is database, need to find better approach
                    	try {
							Thread.sleep(500);
						} catch (InterruptedException e) {
							
							e.printStackTrace();
						}
                    }
                   
                    mChatWindow.addSentMessage(msg, messageTag);
                    mMessagesToSendDelayed.put(msg, messageTag);
                    mAllMessages.add(new ChatMessageSent(msg));
                }
            }
        });
    }

    private void clearRestoredMessages() {
        mWorkerHandler.post(new Runnable() {
            @Override
            public void run() {
                mMessagesToSendDelayed.clear();
            }
        });
    }

    private void sendInvite(String subject , String firstMessage, int messageTag) {
        Logger.d(TAG, "sendInvite() entry, content: " + subject + " messageTag: " + messageTag);
        if (mCapabilityApi == null) {
            mCapabilityApi = ApiManager.getInstance().getCapabilityApi();
        }
        if (mCapabilityApi == null) {
            Logger.e(TAG, "sendInvite() mCapabilityApi is null");
            return;
        }
        InstantMessage message = null;
        List<String> pList = convertParticipantInfosToContacts(mCurrentParticipants.mParticipants);
        ArrayList<ParticipantInfo> nonImList = new ArrayList<ParticipantInfo>();
        ArrayList<ParticipantInfo> participantInfos = new ArrayList<ParticipantInfo>(
                mCurrentParticipants.mParticipants);
        int size = participantInfos.size();
        for (int i = 0; i < size; i++) {
            ParticipantInfo participant = participantInfos.get(i);
            String contact = participant.getContact();
            Capabilities remoteCapablities = mCapabilityApi.getContactCapabilities(contact);
            if (remoteCapablities != null) {
                boolean imSupported = remoteCapablities.isImSessionSupported();
                Logger.d(TAG, "sendInvite() contact: " + contact + " imSupported: " + imSupported);
                if (!imSupported) {
                    mCurrentParticipants.timerUnSchedule(participant);
                    nonImList.add(participant);
                    pList.remove(contact);
                }
            } else {
                Logger.e(TAG, "sendInvite() remoteCapabilites is null");
            }
        }
        int nonImsize = nonImList.size();
        Logger.d(TAG, "sendInvite() nonImsize: " + nonImsize);
        if (nonImsize > 0) {
            mCurrentParticipants.mParticipants.removeAll(nonImList);
            final String displayname = ChatFragment.getParticipantsName(nonImList
                    .toArray(new ParticipantInfo[1]));
            ((IGroupChatWindow) mChatWindow).updateParticipants(mCurrentParticipants.mParticipants);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.IM_FAIL_TOAST;
            msg.obj = displayname;
            mUiHandler.sendMessage(msg);
        }
        int inviterSize = pList.size();
        Logger.d(TAG, "sendInvite() inviterSize: " + inviterSize);
        if (inviterSize > 0) {
            try {
                IChatSession chatSession = mMessagingApi.initiateAdhocGroupChatSession(pList,subject,firstMessage);
                
                // caused by different processes.
                Logger.d(TAG, "sendInvite() chatSession: " + chatSession);
                if (chatSession != null) {
                    mCurrentSession.set(chatSession);
                    SessionInfo sessionInfo = new SessionInfo(chatSession, SessionStatus.INVITING);
                    GroupChatListener listener = new GroupChatListener(sessionInfo);
                    chatSession.addSessionListener(listener);
                    mChatId = chatSession.getChatID();
                    message = chatSession.getFirstMessage();
                    Message msg = Message.obtain();
                    msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
                    mUiHandler.sendMessage(msg);
                }
            } catch (ClientApiException e) {
                e.printStackTrace();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        for (ParticipantInfo info : mCurrentParticipants.mParticipants) {
            if (info.getState().equals(User.STATE_PENDING)) {
                mCurrentParticipants.timerSchedule(info);
            }
        }
        mComposingManager.messageWasSent();
        if (null != message) {
            mChatWindow.addSentMessage(message, messageTag);
        } else {
            Logger.e(TAG, "sendInvite() message is null");
        }
        mAllMessages.add(new ChatMessageSent(message));
        Logger.d(TAG, "sendInvite() exit, subject: " + subject + " mChatId: " + mChatId);
    }

    private boolean rejoinGroup(String content, String rejoinId, int messageTag) {
        Logger.d(TAG, "rejoinGroup() entry, content: " + content + " rejoinId: " + rejoinId);
        boolean success = false;
        if (rejoinId == null) {
            restoreMessages(content, messageTag);
            return success;
        }
        try {
            IChatSession session = mMessagingApi.rejoinGroupChatSession(rejoinId);
            // caused by different processes.
            Logger.d(TAG, "rejoinGroup() session: " + session);
            if (session != null) {
                SessionInfo sessionInfo = new SessionInfo(session, SessionStatus.REJOINING);
                GroupChatListener listener = new GroupChatListener(sessionInfo);
                session.addSessionListener(listener);
                success = true;
            }
        } catch (ClientApiException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        restoreMessages(content, messageTag);
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        Logger.d(TAG, "rejoinGroup() exit, success: " + success);
        return success;
    }

    private boolean restartGroup(String content, String restartId) {
        return restartGroup(content, restartId, -1);
    }

    private boolean restartGroup(String content, String restartId, int messageTag) {
        Logger.d(TAG, "restartGroup() entry, content: " + content + " restartId: " + restartId);
        boolean success = false;
        if (restartId == null) {
            restoreMessages(content, messageTag);
            return success;
        }
        try {
            IChatSession session = mMessagingApi.restartGroupChatSession(restartId);
            // caused by different processes.
            Logger.d(TAG, "restartGroup() session: " + session);
            if (session != null) {
                SessionInfo sessionInfo = new SessionInfo(session, SessionStatus.RESTARTING);
                GroupChatListener listener = new GroupChatListener(sessionInfo);
                session.addSessionListener(listener);
                success = true;
            }
        } catch (ClientApiException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        restoreMessages(content, messageTag);
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
        Logger.d(TAG, "restartGroup() exit, success: " + success);
        return success;
    }

    private void sendMsrpMessage(IChatSession chatSession, String content, int messageTag) {
        Logger.d(TAG, "sendMsrpMessage() entry, content: " + content + " messageTag: " + messageTag
                + " chatSession: " + chatSession);
        if (chatSession == null) {
            restoreMessages(content, messageTag);
            return;
        }
        try {
            String messageId = chatSession.sendMessage(content);
            InstantMessage firstMessage = null;
            String remote = null;
            firstMessage = chatSession.getFirstMessage();
            if (firstMessage != null) {
                remote = firstMessage.getRemote();
            }
            InstantMessage message = new InstantMessage(messageId, remote, content, true,
                    new Date());
            mComposingManager.messageWasSent();
            if (null != message) {
                mChatWindow.addSentMessage(message, messageTag);
            }
            mAllMessages.add(new ChatMessageSent(message));
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "sendMsrpMessage() exit, content: " + content + " messageTag: " + messageTag);
    }

    private List<Participant> convertContactsToParticipants(List<String> list) {
        List<Participant> participants = new ArrayList<Participant>();
        int size = list.size();
        for (int i = 0; i < size; i++) {
            String contact = list.get(i);
            String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                    contact);
            participants.add(new Participant(contact, displayName));
        }
        return participants;
    }

    private List<String> convertParticipantInfosToContacts(List<ParticipantInfo> participantInfos) {
        List<String> pList = new ArrayList<String>();
        int size = participantInfos.size();
        for (int i = 0; i < size; i++) {
            pList.add(participantInfos.get(i).getContact());
        }
        return pList;
    }

    private List<String> convertParticipantsToContacts(List<Participant> participants) {
        List<String> pList = new ArrayList<String>();
        int size = participants.size();
        for (int i = 0; i < size; i++) {
            pList.add(participants.get(i).getContact());
        }
        return pList;
    }

    /**
     * Provided for controller to call.
     * 
     * @param participants The participants that should be updated to group
     *            chat.
     * @return True if it is updated successfully, otherwise false;
     */
    public boolean addParticipants(List<Participant> toAdd) {
        Logger.d(TAG, "addParticipants entry");
        if (toAdd.size() > 0) {
            int index = 0;
            int size = mCurrentParticipants.mParticipants.size();
            for (; index < size; index++) {
                Participant participant = mCurrentParticipants.mParticipants.get(index).mParticipant;
                if (toAdd.contains(participant)) {
                    toAdd.remove(participant);
                    index -= 1;
                    size = mCurrentParticipants.mParticipants.size();
                }
            }
            ArrayList<Participant> iMCapabilityFailedList = new ArrayList<Participant>();
            CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
            if (capabilityApi == null) {
                Logger.w(TAG, "addParticipants() capabilityApi is null!");
            } else {
                for (Participant participant : toAdd) {
                    String contactNum = participant.getContact();
                    Capabilities remoteCapability = capabilityApi
                            .getContactCapabilities(contactNum);
                    Logger.d(TAG, "addParticipants() remoteCapability: " + remoteCapability);
                    if (remoteCapability != null) {
                        if (remoteCapability.isImSessionSupported()) {
                            Logger.d(TAG, "addParticipants the contactNum : " + contactNum
                                    + "supports the Im session");
                        } else {
                            iMCapabilityFailedList.add(participant);
                        }
                    }
                }
            }
            if (iMCapabilityFailedList.size() > 0) {
                final String displayname = ChatFragment.getParticipantsName(iMCapabilityFailedList
                        .toArray(new Participant[1]));
                Message msg = Message.obtain();
                msg.what = GroupChatHandler.IM_FAIL_TOAST;
                msg.obj = displayname;
                mUiHandler.sendMessage(msg);
                toAdd.removeAll(iMCapabilityFailedList);
            }
            Logger.d(
                    TAG,
                    "addParticipants() the toadd list is " + toAdd + " and size is  "
                            + toAdd.size());
            mCurrentParticipants.addAll(toAdd);
            IChatSession currentChatSession = mCurrentSession.get();
            if (currentChatSession == null) {
                Logger.d(TAG, "addParticipants mCurrentSession is null");
                return false;
            }
            try {
                currentChatSession.addParticipants(convertParticipantsToContacts(toAdd));
            } catch (RemoteException e) {
                Logger.d(TAG, "addParticipants fail");
                e.printStackTrace();
                return false;
            }
            Logger.d(TAG, "addParticipants exit with true");
            return true;
        } else {
            Logger.d(TAG, "addParticipants() no participant to add");
            return false;
        }
    }

    /**
     * Network connectivity listener.
     */
    public class NetworkConnectivityListener extends INetworkConnectivity.Stub {
        private boolean mShouldAutoRejoin = false;

        @Override
        public void prepareToDisconnect() throws RemoteException {
            Logger.d(TAG, "prepareToDisconnect() entry, mIsRegistered: " + mIsRegistered);
            if (!mIsRegistered) {
                return;
            }
            mIsRegistered = false;
            try {
                SessionInfo sessionInfo = mSessionStack.peek();
                IChatSession session = sessionInfo.getSession();
                //check if the session is not null
                if(session != null)
                {
	                int state = session.getSessionState();
	                Logger.d(TAG, "prepareToDisconnect() state: " + state);
	                if (state == SessionState.ESTABLISHED) {
	                    mShouldAutoRejoin = true;
	                    changeSessionStatus(sessionInfo, SessionStatus.AUTO_REJOIN);
	                } else {
	                    mShouldAutoRejoin = false;
	                }
                }
                 else{
                  Logger.d(TAG, "prepareToDisconnect() : session = null");
                }
                
            } catch (EmptyStackException e) {
                Logger.e(TAG, "prepareToDisconnect() EmptyStackException");
                e.printStackTrace();
            }
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.d(TAG, "prepareToDisconnect() exit, mShouldAutoRejoin: " + mShouldAutoRejoin);
        }

        @Override
        public void connect() throws RemoteException {
            Logger.d(TAG, "connect() entry, mShouldAutoRejoin: " + mShouldAutoRejoin + " mChatId: "
                    + mChatId + " mIsRegistered: " + mIsRegistered);
            if (mIsRegistered) {
                return;
            }
            mIsRegistered = true;
            if (mMessagingApi == null) {
                mMessagingApi = ApiManager.getInstance().getMessagingApi();
            }
            if (mMessagingApi == null) {
                Logger.e(TAG, "connect() mMessagingApi is null");
                return;
            }
            if (mShouldAutoRejoin) {
                try {
                    // this function is called in worker thread, so it's safe to
                    // do overload operation.
                    IChatSession session = mMessagingApi.rejoinGroupChatSession(mChatId);
                    Logger.d(TAG, "connect() session: " + session);
                    if (session != null) {
                        SessionInfo sessionInfo = new SessionInfo(session, SessionStatus.REJOINING);
                        GroupChatListener listener = new GroupChatListener(sessionInfo);
                        session.addSessionListener(listener);
                    }
                } catch (ClientApiException e) {
                    e.printStackTrace();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                mShouldAutoRejoin = false;
            }
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.d(TAG, "connect() exit");
        }
    }

    private void sendRestoredMessages(IChatSession session) {
        Logger.d(TAG, "sendRestoredMessages() mMessagesToSendDelayed size: "
                + mMessagesToSendDelayed.size());
        for (InstantMessage msg : mMessagesToSendDelayed.keySet()) {
            try {
                session.sendMessageWithMsgId(msg.getTextMessage(), msg.getMessageId());
                mComposingManager.messageWasSent();
            } catch (RemoteException e) {
                Logger.e(TAG, "sendRestoredMessages() mMessagesToSendDelayed RemoteException");
                e.printStackTrace();
            }
            //msg.setMsgId(messageId);
            mChatWindow.addSentMessage(msg, mMessagesToSendDelayed.get(msg));
        }
    }

    private void handleReceiveMessage(final InstantMessage msg, final GroupChatParticipants groupChatParticipants) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Context context = null;
                if (ApiManager.getInstance() != null) {
                    context = ApiManager.getInstance().getContext();
                }
                if (msg.isImdnDisplayedRequested()) {
                    if (!mIsInBackground) {
                        /*
                         * Mark the received message as displayed when the chat
                         * window is not in background
                         */
                        markMessageAsDisplayed(msg);
                    } else {
                        /*
                         * Save the message and will mark it as displayed when
                         * the chat screen resumes
                         */
                        GroupChat.this.addUnreadMessage(msg);
                        /*
                         * Showing notification of a new incoming message when
                         * the chat window is in background
                         */
                        RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, msg,
                                groupChatParticipants.convertToParticipants(), 0);
                    }
                } else {
                    if (!mIsInBackground) {
                        /*
                         * Mark the received message as read if the chat window
                         * is not in background
                         */
                        markMessageAsRead(msg);
                    } else {
                        /*
                         * Save the message and will mark it as read when the
                         * activity resumes
                         */
                        GroupChat.this.addUnreadMessage(msg);
                        /*
                         * Showing notification of a new incoming message when
                         * the chat window is in background
                         */
                        RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, msg,
                                groupChatParticipants.convertToParticipants(), 0);
                    }
                }
                mChatWindow.addReceivedMessage(msg, !mIsInBackground);
                mAllMessages.add(new ChatMessageReceived(msg));
                
                /**
                 * M: managing extra local chat participants that are 
                 * not present in the invitation for sending them invite request.@{
                 */
                if(isResendGrpInvite){
                	Logger.d(TAG, "handleReceiveMessage: adding mExtraLocalparticipants in chat and sending refer request");
                	isResendGrpInvite = false;
                	//add the extra local particpants and send them refer request
                	
                	//we add only if its not there
             /*   	for ( Participant p : mExtraLocalparticipants)
                	{
	                	if(mCurrentParticipants.mParticipants.contains(p))
	                	{
	                		//remove it
	                		mExtraLocalparticipants.remove(p);
	                	}
                	}*/
            		addParticipants(mExtraLocalparticipants);	
                }
                /**
            	 * @}
            	 */ 
                return null;
            }
        }.execute();
    }

    private class GroupChatListener extends IChatEventListener.Stub {

        public static final String TAG = "GroupChatListener";
        private SessionStatus mStatus = SessionStatus.UNKNOWN;
        protected IChatSession mSession = null;
        private SessionInfo mSessionInfo = null;
        private GroupChatParticipants mGroupChatParticipants = new GroupChatParticipants();
        private ArrayList<String> mParticipantsToBeInvited = new ArrayList<String>();

        protected GroupChatListener(SessionInfo info) {
            mSession = info.getSession();
            mSessionInfo = info;
            mSessionStack.push(mSessionInfo);
            List<String> pList = new ArrayList<String>();
            try {
                pList = mSession.getInivtedParticipants();
                if (pList == null) {
                    return;
                }
                mGroupChatParticipants
                        .initicalGroupChatParticipant(convertContactsToParticipants(pList));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            List<String> currentList = convertParticipantInfosToContacts(mCurrentParticipants.mParticipants);
            for (String contact : currentList) {
                if (!pList.contains(contact)) {
                    mParticipantsToBeInvited.add(contact);
                    Logger.d(TAG, "GroupChatListener mParticipantsToBeInvited contact: " + contact);
                }
            }
            mCurrentParticipants = mGroupChatParticipants;
            // TODO auto-invite OTHER participants.
        }

        private void onDestroy() {
            Logger.d(TAG, "onDestroy entry, mSession: " + mSession);
            mCurrentParticipants.resetAllStatus();
            if (null != mSession) {
                try {
                    Logger.d(TAG, "onDestroy() mSession" + mSession);
                    mSession.removeSessionListener(this);
                } catch (RemoteException e) {
                    Logger.e(TAG, "onDestroy() RemoteException" + e);
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void handleSessionStarted() {
            Logger.d(TAG, "handleSessionStarted entry, mStatus: " + mStatus);
            mCurrentSession.set(mSession);
            mStatus = SessionStatus.ACTIVE;
            changeSessionStatus(mSessionInfo, mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    sendRestoredMessages(mSession);
                    clearRestoredMessages();
                }
            });
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    Logger.d(TAG, "mReceivedMessagesDelayed = " + mReceivedMessagesDelayed);
                    if (mReceivedMessagesDelayed != null) {
                        int size = mReceivedMessagesDelayed.size();
                        for (int i = 0; i < size; i++) {
                            markMessageAsDisplayed(mReceivedMessagesDelayed.get(i));
                        }
                        mReceivedMessagesDelayed.clear();
                    }
                    return null;
                }
            }.execute();
        }

        @Override
        public void handleSessionAborted(int reason) throws RemoteException {
            Logger.d(TAG, "handleSessionAborted() entry, mStatus: " + mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            int location = mSessionStack.indexOf(mSessionInfo);
            if (location != -1) {
                mStatus = mSessionStack.get(location).getSessionStatus();
            }
            if (mStatus != SessionStatus.AUTO_REJOIN) {
                mStatus = SessionStatus.MANULLY_REJOIN;
            }
            changeSessionStatus(mSessionInfo, mStatus);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.d(TAG, "handleSessionAborted() exit, mStatus: " + mStatus);
            onDestroy();
        }

        @Override
        public void handleSessionTerminatedByRemote() throws RemoteException {
            Logger.d(TAG, "handleSessionTerminatedByRemote() entry, mStatus: " + mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            boolean rejoin = false;
            for (ParticipantInfo info : mGroupChatParticipants.mParticipants) {
             
            	//if we have even one participants .. restart the grp next time.
            	/**
                 * M: modified to start a new  grp chat if session is terminated  @(
                 * */
                    rejoin = true;
                   // mStatus = SessionStatus.MANULLY_RESTART;
                    if (mStatus != SessionStatus.AUTO_REJOIN) {
                        mStatus = SessionStatus.MANULLY_REJOIN;
                    }
                    break;
                /**
                  * @}
                */
                }
            if (!rejoin) {
                mStatus = SessionStatus.TERMINATED;
            }
            changeSessionStatus(mSessionInfo, mStatus);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            onDestroy();
            Logger.d(TAG, "handleSessionTerminatedByRemote() exit, mStatus: " + mStatus);
        }
        @Override
        public void handleReceiveGeoloc(GeolocMessage msg)
        {
        	Toast.makeText(ApiManager.getInstance().getContext(), msg.getTextMessage(), 300).show();

        }


        @Override
        public void handleReceiveMessage(final InstantMessage msg) throws RemoteException {
            Logger.d(TAG, "handleReceiveMessage() entry");
            GroupChat.this.handleReceiveMessage(msg, mGroupChatParticipants);

            Logger.d(TAG, "handleReceiveMessage() exit");
        }

        @Override
        public void handleImError(int error) throws RemoteException {
            Logger.d(TAG, "handleImError() entry, error: " + error + " mStatus: " + mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            int location = mSessionStack.indexOf(mSessionInfo);
            if (location != -1) {
                mStatus = mSessionStack.get(location).getSessionStatus();
            }
            if (mSessionStack.size() == 1 && mStatus == SessionStatus.INVITING) {
                mStatus = SessionStatus.UNKNOWN;
            } else if (mStatus == SessionStatus.RESTARTING) {
                
            	/**
    	         * M: added to restart a new grp chat in case of error  or timeout @(
    	         * */
                //mStatus = SessionStatus.TERMINATED;
                 Logger.d(TAG, "set mStatus = SessionStatus.MANULLY_RESTART");
            	mStatus = SessionStatus.MANULLY_RESTART;
            	/**
    	         * @}
    	         */
            	
            } else if (mStatus == SessionStatus.REJOINING) {
                mStatus = SessionStatus.MANULLY_RESTART;
                restartGroup(null, mChatId);
            } else {
                mStatus = SessionStatus.MANULLY_REJOIN;
            }
            changeSessionStatus(mSessionInfo, mStatus);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            onDestroy();
            Logger.v(TAG, "handleImError() exit, mStatus: " + mStatus);
        }

        @Override
        public void handleIsComposingEvent(String contact, final boolean status)
                throws RemoteException {
            Logger.d(TAG, "handleIsComposingEvent() contact: " + contact + " status: " + status);
            int size = mGroupChatParticipants.mParticipants.size();
            String contactNumber = PhoneUtils.extractNumberFromUri(contact);
            for (int index = 0; index < size; index++) {
                final Participant participant = mGroupChatParticipants.mParticipants.get(index).mParticipant;
                Logger.d(TAG, "handleIsComposingEvent() participant: " + participant);
                if (null != participant) {
                    String remoteContact = participant.getContact();
                    if (null != remoteContact && remoteContact.equals(contactNumber)) {
                        ((IGroupChatWindow) mChatWindow).setIsComposing(status, participant);
                    }
                }
            }
        }

        @Override
        public void handleConferenceEvent(final String contact, final String contactDisplayname,
                final String state) throws RemoteException {
            boolean result = mGroupChatParticipants.updateParticipants(contact, contactDisplayname,
                    state, mParticipantsToBeInvited);
            if (result) {
                mParticipantsToBeInvited.remove(contact);
            }
            Logger.d(TAG, "handleConferenceEvent() result: " + result);
        }

        @Override
        public void handleMessageDeliveryStatus(String msgId, String status, String contact,long date)
                throws RemoteException {
            Logger.d(TAG, "handleMessageDeliveryStatus() entry, msgId: " + msgId + " status: "
                    + status);
            ISentChatMessage msg = (ISentChatMessage) mChatWindow.getSentChatMessage(msgId);
            if (msg != null) {
                msg.updateStatus(Status.DELIVERED,contact);
            }
        }

        @Override
        public void handleAddParticipantSuccessful() throws RemoteException {
            Logger.d(TAG, "handleAddParticipantSuccessful() entry ");
        }

        @Override
        public void handleAddParticipantFailed(String reason) throws RemoteException {
            Logger.d(TAG, "handleAddParticipantFailed() entry  reason is " + reason);
        }
    };

    /**
     * Post a request to chat participants for transferring file .
     * 
     * @param file The file to be transferred.
     * @return True if it's successful to post a request, otherwise return
     *         false.
     */
    public boolean requestTransferFile(File file) {
        Logger.v(TAG, "GroupChatImpl requestTransferFile entry");
        Logger.v(TAG, "GroupChatImpl requestTransferFile exit");
        return false;
    }

    /**
     * Accept or reject to transfer file.
     * 
     * @param accept true if accepting to transfer file, else reject.
     * @return True if the operation is successfully handled, else return false.
     */
    public boolean acceptTransferFile(boolean accept) {
        Logger.v(TAG, "GroupChatImpl acceptTransferFile entry");
        Logger.v(TAG, "GroupChatImpl acceptTransferFile exit");
        return false;
    }

	/**
     * Handle a file transfer invitation
     */
    public void addReceiveFileTransfer(IFileTransferSession fileTransferSession, boolean isAutoAccept,boolean isGroup) {
    	Logger.v(TAG, "addReceiveFileTransfer isAutoAccept" + isAutoAccept+" isGroup:"+isGroup);
        mReceiveFileTransferManager.addReceiveFileTransfer(fileTransferSession, isAutoAccept,isGroup);
    }

    @Override
    protected void checkCapabilities() {
        if (mCapabilityApi == null) {
            mCapabilityApi = ApiManager.getInstance().getCapabilityApi();
        }
        Logger.d(TAG, "checkCapabilities() mCapabilityApi: " + mCapabilityApi);
        if (mCapabilityApi == null) {
            return;
        }
        if (!mCapabilityApi.isBinderAlive()) {
            Logger.w(TAG, "checkCapabilities() capabilityApi is dead!!");
            return;
        }
        ArrayList<ParticipantInfo> participants = new ArrayList<ParticipantInfo>(
                mCurrentParticipants.mParticipants);
        ArrayList<ParticipantInfo> uncapableParticipants = new ArrayList<ParticipantInfo>();
        for (ParticipantInfo participantInfo : participants) {
            Participant participant = participantInfo.getParticipant();
            String contact = participant.getContact();
            Capabilities capability = mCapabilityApi.getContactCapabilities(contact);
            if (capability == null || !capability.isImSessionSupported()) {
                Logger.w(TAG, "checkCapabilities() The capabilities of " + contact + " is "
                        + capability);
                uncapableParticipants.add(participantInfo);
            }
        }
        Logger.w(TAG, "checkCapabilities() uncapableParticipants: " + uncapableParticipants);
        mCurrentParticipants.mParticipants.removeAll(uncapableParticipants);
    }

    @Override
    protected void queryCapabilities() {
        checkCapabilities();
    }

    public void handleReInvitation(IChatSession chatSession, String participantList) {
        Logger.v(TAG, "group chat handleInvitation entry, chatSession: " + chatSession);
        if (chatSession == null) {
            return;
        }
        try {
            chatSession.acceptSession();
            
            /**
             * M: managing extra local chat participants that are 
             * not present in the invitation for sending them invite request.@{
             */
    		
    		
            String[]particpnts = participantList.split(";");
            List<String>ParticipantArray = Arrays.asList(particpnts);
           
            List<String>localParticipantArray = new ArrayList<String>();
            ArrayList<Participant> localParticipantList = mCurrentParticipants.convertToParticipants();
            for(Participant p :localParticipantList){
            	localParticipantArray.add(p.getContact());
            }
           
            if(ParticipantArray.size()>0){
            	//check if current invitation contains all the participants present locally
            	if(!ParticipantArray.containsAll(localParticipantArray)){
            		
            		
            		isResendGrpInvite = true; //flag to send the group invite to extra local particpants of the chat
            		
            		localParticipantArray.removeAll(ParticipantArray);
            		Logger.v(TAG, "handleReInvitation : ExtraLocalparticipants exists: " + localParticipantArray.toString());
            		
            		mExtraLocalparticipants = new ArrayList<Participant>();
            		for(String extraParticipant: localParticipantArray){
            			Participant p = new Participant(extraParticipant,extraParticipant);
            			mExtraLocalparticipants.add(p); //add the extra participants
            		}

            	}
            }
            /**
    		 * @}
    		 */   
            
            SessionInfo sessionInfo = new SessionInfo(chatSession, SessionStatus.INVITING);
            GroupChatListener listener = new GroupChatListener(sessionInfo);
            mChatId = chatSession.getChatID();
            chatSession.addSessionListener(listener);           
                   
        } catch (RemoteException e) {
            Logger.d(TAG, "handleInvitation() acceptSession or addSessionListener fail");
            e.printStackTrace();
        }       
    }

    @Override
    public void handleInvitation(IChatSession chatSession, ArrayList<IChatMessage> messages, boolean isAutoAccept) {
        Logger.v(TAG, "group chat handleInvitation entry, chatSession: " + chatSession);
        if (chatSession == null) {
            return;
        }
        try {
            chatSession.acceptSession();
            SessionInfo sessionInfo = new SessionInfo(chatSession, SessionStatus.INVITING);
            GroupChatListener listener = new GroupChatListener(sessionInfo);
            mChatId = chatSession.getChatID();
            chatSession.addSessionListener(listener);
        } catch (RemoteException e) {
            Logger.d(TAG, "handleInvitation() acceptSession or addSessionListener fail");
            e.printStackTrace();
        }

            Logger.v(TAG, "Group chat handleInvitation messages is null");
            return;

    }

    @Override
    public void loadChatMessages(int count) {
        // TODO
    }

	public void reloadSessionStack(int sessionStatus) {
		IChatSession reloadedSession = null;
		SessionInfo reloadedSessionInfo;
		Logger.w(TAG, "reloadSessionStack()");
//		if (sessionStatus == 1 ){
//		    Logger.w(TAG, "reloadSessionStack() Terminated ");
//		    reloadedSessionInfo = new SessionInfo(reloadedSession, SessionStatus.TERMINATED);
//		}
//		else {
            reloadedSessionInfo = new SessionInfo(reloadedSession, SessionStatus.UNKNOWN);
		//}	
		mSessionStack.push(reloadedSessionInfo);

	}

    @Override
    protected void reloadMessage(final InstantMessage message, final int messageType,
            final int status) {
	if(ismInvite() == false)
	{
        Logger.w(TAG, "reloadMessage() entry ");
        String groupChatSubject = RichMessaging.getInstance().getGroupChatSubjectByChatID(mChatId);
        
        ((IGroupChatWindow)mChatWindow).addgroupSubject(groupChatSubject); 
        Logger.d(TAG, "reloadMessage : groupChatSubject outer :-> "+ groupChatSubject +"Chat Id =" + mChatId);
        
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "reloadMessage()->run() entry, message id: " + message.getMessageId()
                        + "message text: " + message.getTextMessage() + " , messageType: "
                        + messageType);
                if(mChatWindow != null)
                {
                	/* Get from database */
                	//String groupChatSubject = RichMessaging.getInstance().getGroupChatSubjectByChatID(mChatId);
                	
                    
                  //Logger.d(TAG, "reloadMessage : groupChatSubject :-> "+ groupChatSubject +"Chat Id =" + mChatId);
                	               	   
                if (EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE == messageType) {
                    Logger.d(TAG, "reloadMessage() the mchatwindow is " + mChatWindow);
                    mChatWindow.addReceivedMessage(message, true);
                    mAllMessages.add(new ChatMessageReceived(message));
                } 
                else if (EventsLogApi.TYPE_OUTGOING_GROUP_CHAT_MESSAGE == messageType) {
                    mChatWindow.addSentMessage(message, -1);
                    mAllMessages.add(new ChatMessageSent(message));
                } 
                else if (EventsLogApi.TYPE_GROUP_CHAT_SYSTEM_MESSAGE == messageType) {
                    mChatWindow.addSentMessage(message, -1);
                    mAllMessages.add(new ChatMessageSent(message));
                } else {
                    Logger.e(TAG,
                            "reloadMessage() the messageType is not incoming or outgoing message "
                                    + messageType);
                }
            }
            }
        };
        mWorkerHandler.post(worker);
    }
}

    @Override
    public void onCapabilityChanged(String contact, Capabilities capabilities) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onStatusChanged(boolean status) {
        Logger.d(TAG, "onStatusChanged() status: " + status);
        mIsRegistered = status;
        if (status) {
            // to make sure that init() is called when registered.
            init();
        }
        Message msg = Message.obtain();
        msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
        mUiHandler.sendMessage(msg);
    }

    /**
     * Helper function for test case.
     * 
     * @return Group chat participants.
     */
    public GroupChatParticipants getGroupChatParticipants() {
        return mCurrentParticipants;
    }

    /**
     * Session status.
     */
    public static enum SessionStatus {
        /**
         * Unknown, initial status.
         */
        UNKNOWN,
        /**
         * Inviting, wait to accept.
         */
        INVITING,
        /**
         * Session is established.
         */
        ACTIVE,
        /**
         * Session rejoin is on-going.
         */
        REJOINING,
        /**
         * Session should be auto-rejoined.
         */
        AUTO_REJOIN,
        /**
         * Session should be manually rejoined.
         */
        MANULLY_REJOIN,
        /**
         * Session should be restarted.
         */
        MANULLY_RESTART,
        /**
         * Session restarting is on-going.
         */
        RESTARTING,
        /**
         * No need to try to rejoin.
         */
        TERMINATED
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

	/**
     * Handle pause a file transfer 
     */
    public void handlePauseReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "handlePauseReceiveFileTransfer group entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			 Logger.d(TAG,"handlePauseReceiveFileTransfer group 1");
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
                "handleResumeReceiveFileTransfer group entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			Logger.d(TAG,"handleResumeReceiveFileTransfer group 1");
            receiveFileTransfer.onResumeReceiveTransfer();
        }
    }

}

