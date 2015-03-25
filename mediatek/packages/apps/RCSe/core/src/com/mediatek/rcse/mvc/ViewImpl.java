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

import android.os.Message;
import android.os.ParcelUuid;

import com.mediatek.rcse.activities.ChatListFragment.One2OneChatMap;
import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
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
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.view.ReceivedChatMessage;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.One2OneChatWindowAdapter;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This is the virtual View part in the MVC pattern
 */
public class ViewImpl implements IChatWindowManager {
    public static final String TAG = "ViewImpl";
    private static volatile ViewImpl sInstance = null;
    private final CopyOnWriteArrayList<IChatWindowManager> mChatWindowManagerList =
            new CopyOnWriteArrayList<IChatWindowManager>();
    private final ConcurrentHashMap<Object, ChatWindowDispatcher> mChatWindowDispatcherMap =
            new ConcurrentHashMap<Object, ViewImpl.ChatWindowDispatcher>();

    public static synchronized ViewImpl getInstance() {
        if (null == sInstance) {
            sInstance = new ViewImpl();
        }
        return sInstance;
    }

    protected ViewImpl() {
    }

    @Override
    public IGroupChatWindow addGroupChatWindow(Object tag, List<ParticipantInfo> participantList) {
        GroupChatWindowDispatcher chatWindow =
                new GroupChatWindowDispatcher(tag,
                        (CopyOnWriteArrayList<ParticipantInfo>) participantList);
        chatWindow.setmChatID(participantList.get(0).getmChatID());
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindow.onAddChatWindowManager(chatWindowManager);
        }
        mChatWindowDispatcherMap.put(tag, chatWindow);
        return chatWindow;
    }

    @Override
    public IOne2OneChatWindow addOne2OneChatWindow(Object tag, Participant participant) {
        One2OneChatWindowDispatcher chatWindow = new One2OneChatWindowDispatcher(tag, participant);
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindow.onAddChatWindowManager(chatWindowManager);
        }
        mChatWindowDispatcherMap.put(tag, chatWindow);
        return chatWindow;
    }

    @Override
    public boolean removeChatWindow(IChatWindow chatWindow) {
        if (chatWindow != null && chatWindow instanceof ChatWindowDispatcher) {
            ((ChatWindowDispatcher) chatWindow).onDestroy();
            return mChatWindowDispatcherMap.values().remove(chatWindow);
        } else {
            Logger.e(TAG, "removeChatWindow() remove IChatWindow, invalid chatWindow :" + chatWindow);
            return false;
        }
    }

    /**
     * Called by a single View to register itself to the Module.
     * 
     * @param chatWindowManager The chat window manager of the View.
     * @param autoShow True if it should automatically show all window, else
     *            false.
     */
    public synchronized void addChatWindowManager(IChatWindowManager chatWindowManager,
            boolean autoShow) {
        Logger
                .d(TAG, "addChatWindowManager() entry, the chatWindowManager is "
                        + chatWindowManager);
        if (null != chatWindowManager) {
            if (mChatWindowManagerList.contains(chatWindowManager)) {
                Logger.w(TAG, "addChatWindowManager() chatWindowManager has already added");
            }
            mChatWindowManagerList.add(chatWindowManager);
            Logger.v(TAG, "addChatWindowManager() chat window manager size: "
                    + mChatWindowManagerList.size());
            if (!autoShow) {
                Logger.d(TAG, "addChatWindowManager autoShow is false.");
                return;
            }
            for (ChatWindowDispatcher chatWindowDispatcher : mChatWindowDispatcherMap.values()) {
                Logger.w(TAG, "addChatWindowManager() chatWindowManager: " + chatWindowManager);
                chatWindowDispatcher.onAddChatWindowManager(chatWindowManager);
            }
        } else {
            Logger.e(TAG, "addChatWindowManager() chatWindowManager is null");
        }
    }

    /**
     * Called by a single View to unregister itself from the Module
     * 
     * @param chatWindowManager The chat window manager of the View
     */
    public synchronized void removeChatWindowManager(IChatWindowManager chatWindowManager) {
        Logger.d(TAG, "removeChatWindowManager() entry, the chatWindowManager is "
                + chatWindowManager);
        if (null != chatWindowManager) {
            boolean result = false;
            int count = 0;
            do {
                result = mChatWindowManagerList.remove(chatWindowManager);
                if (result) {
                    count++;
                    for (ChatWindowDispatcher chatWindowDispatcher : mChatWindowDispatcherMap.values()) {
                        chatWindowDispatcher.onRemoveChatWindowManager(chatWindowManager);
                    }
                }
            } while (result);
            Logger.d(TAG, "removeChatWindowManager() done, count: " + count
                    + ", current size: " + mChatWindowManagerList.size());
        } else {
            Logger.e(TAG, "removeChatWindowManager() chatWindowManager is null");
        }
    }

    private abstract static class ChatWindowDispatcher implements IChatWindow {
        protected Object mTag = null;
        protected final CopyOnWriteArrayList<IChatItemDispatcher> mChatItemList =
                new CopyOnWriteArrayList<IChatItemDispatcher>();
        protected ConcurrentHashMap<IChatWindowManager, IChatWindow> mChatWindowMap =
                new ConcurrentHashMap<IChatWindowManager, IChatWindow>();

        public ChatWindowDispatcher(Object tag) {
            mTag = tag;
        }

        @Override
        public void updateAllMsgAsRead() {
            for (IChatItemDispatcher chatItem : mChatItemList) {
                if (chatItem instanceof IReceivedChatMessage) {
                    ((ReceivedChatMessage) chatItem).updateStatus(true);
                }
            }
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.updateAllMsgAsRead();
            }

        }

        @Override
        public void updateAllMsgAsReadForContact(Participant participant) {
            
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.updateAllMsgAsReadForContact(participant);
            }

        }

        abstract void onDestroy();

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            IChatWindowMessage message = getChatMessage(messageId);
            Logger.i(TAG, "getSentChatMessage() messageId: " + messageId + " , message: " + message);
            if (message instanceof ISentChatMessage) {
                return message;
            }
            return null;
        }

        private IChatWindowMessage getChatMessage(String messageId) {
            Logger.i(TAG, "getChatMessage() id is" + messageId);
            if (null != messageId) {
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof ChatMessage
                            && messageId.equals(((ChatMessage) chatItem).getId())) {
                        return (IChatWindowMessage) chatItem;
                    }
                }
            } else {
                Logger
                        .e(TAG,
                                "getChatMessage() cannot find the target message with id, messageId is null!");
            }
            return null;
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            Logger.i(TAG, "addRecievedMessage() message is" + message);
            if (null != getChatMessage(message.getMessageId())) {
                Logger.d(TAG, "addReceivedMessage() already added this message");
                return null;
            }
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            ChatMessage receivedMessage = new ReceivedChatMessage(message, isRead);
            for (IChatWindow chatWindow : chatWindows) {
                receivedMessage.onAddChatWindow(chatWindow);
            }
            mChatItemList.add(receivedMessage);
            return (IReceivedChatMessage) receivedMessage;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            Logger.i(TAG, "addSentMessage() message is" + message + " messageTag:" + messageTag);
            if (null != getChatMessage(message.getMessageId())) {
                Logger.d(TAG, "addSentMessage() already added this message");
                return null;
            }
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            ChatMessage sentMessage = new SentChatMessage(message, messageTag);
            for (IChatWindow chatWindow : chatWindows) {
                sentMessage.onAddChatWindow(chatWindow);
            }
            mChatItemList.add(sentMessage);
            return (ISentChatMessage) sentMessage;
        }

        public abstract void onAddChatWindowManager(IChatWindowManager chatWindowManager);

        public void onRemoveChatWindowManager(IChatWindowManager chatWindowManager) {
            Logger.d(TAG, "onRemoveChatWindowManager() entry, the chatWindowManager is "
                    + chatWindowManager);
            IChatWindow chatWindow = mChatWindowMap.get(chatWindowManager);
            if (null != chatWindow) {
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onRemoveChatWindow(chatWindow);
                }
                mChatWindowMap.remove(chatWindowManager);
            } else {
                Logger
                        .e(TAG,
                                "onRemoveChatWindowManager() mChatWindowMap doesn't contain this chatWindowManager");
            }
        }

        @Override
        public void removeAllMessages() {
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.removeAllMessages();
            }
            mChatItemList.clear();
        }

        @Override
        public void addLoadHistoryHeader(boolean showHeader) {
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.addLoadHistoryHeader(showHeader);
            }
        }

    }

    private static class One2OneChatWindowDispatcher extends ChatWindowDispatcher implements
            IOne2OneChatWindow {
        public static final String TAG = "One2OneChatWindow";
        private Participant mParticipant = null;
        private boolean mIsComposing = false;
        private boolean mIsOffline = false;
        private int mFileTransferDisableReason = 0;
        private boolean chatLoadedFromDatabase = false;

        public One2OneChatWindowDispatcher(Object tag, Participant participant) {
            super(tag);
            mParticipant = participant;
        }

        @Override
        public void onAddChatWindowManager(IChatWindowManager chatWindowManager) {
            Logger.d(TAG, "onAddChatWindowManager() entry, the chatWindowManager is "
                    + chatWindowManager);
            IChatWindow chatWindow = chatWindowManager.addOne2OneChatWindow(mTag, mParticipant);
            if (null != chatWindow) {
                mChatWindowMap.put(chatWindowManager, chatWindow);
                ((IOne2OneChatWindow) chatWindow).setIsComposing(mIsComposing);
                ((IOne2OneChatWindow) chatWindow).setRemoteOfflineReminder(mIsOffline);
                ((IOne2OneChatWindow) chatWindow).setFileTransferEnable(mFileTransferDisableReason);
                
                 if (chatWindow instanceof OneOneChatWindow) {
                    if (((OneOneChatWindow) chatWindow)
                            .getmOneOneChatFragment() != null && !chatLoadedFromDatabase) {
                        ArrayList<Integer> messageIdArray = null;
                        chatLoadedFromDatabase = true;
                        mChatItemList.clear();
                        if(RichMessagingDataProvider.getInstance()==null)
                            RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
                        messageIdArray = RichMessagingDataProvider
                                .getInstance().getRecentChatForContact(
                                        mParticipant.getContact(),0);
                        Collections.reverse(messageIdArray);                        
                        if (!messageIdArray.isEmpty()) {
                        try {
                            Message controllerMessage = ControllerImpl
                                    .getInstance()
                                    .obtainMessage(
                                            ChatController.EVENT_RELOAD_MESSAGE,
                                            mTag.toString(), messageIdArray);
                            controllerMessage.sendToTarget();
                        } catch (Exception e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }
                }
                }
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onAddChatWindow(chatWindow);
                }
                
            } else {
                Logger.e(TAG, "onAddChatWindowManager() chatWindow is null");
            }
        }

        @Override
        public void setFileTransferEnable(int reason) {
            mFileTransferDisableReason = reason;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    ((IOne2OneChatWindow) chatWindow).setFileTransferEnable(reason);
                }
            }
        }

        @Override
        public void setIsComposing(boolean isComposing) {
            mIsComposing = isComposing;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    ((IOne2OneChatWindow) chatWindow).setIsComposing(isComposing);
                }
            }
        }

        @Override
        public void setRemoteOfflineReminder(boolean isOffline) {
            Logger.d(TAG, "setRemoteOfflineReminder() entry, isOffline is " + isOffline);
            mIsOffline = isOffline;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    ((IOne2OneChatWindow) chatWindow).setRemoteOfflineReminder(isOffline);
                }
            }
            Logger.d(TAG, "setRemoteOfflineReminder() exit");
        }

        @Override
        void onDestroy() {
            Set<IChatWindowManager> managers = mChatWindowMap.keySet();
            for (IChatWindowManager manager : managers) {
                manager.removeChatWindow(mChatWindowMap.get(manager));
            }
        }

        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
            Logger.i(TAG, "addReceivedFileTransfer() FileStruct is" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new ReceivedFileTransferDispatcher(file, isAutoAccept);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    fileTransferDispatcher.onAddChatWindow((IOne2OneChatWindow) chatWindow);
                } else {
                    Logger.d(TAG,
                            "addReceivedFileTransfer() not a one-2-one chat window, just pass it");
                }
            }
            mChatItemList.add(fileTransferDispatcher);
            return fileTransferDispatcher;
        }

        @Override
        public void removeChatMessage(String messageId) {
            if (null != messageId) {
                Collection<IChatWindow> chatWindows = mChatWindowMap.values();
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof ChatMessage
                            && messageId.equals(((ChatMessage) chatItem).getId())) {
                        Logger
                        .e(TAG,
                                "removeChatMessage() found the message !" + (ChatMessage) chatItem);
                        mChatItemList.remove(chatItem);
                    }
                    if (chatItem instanceof FileTransferDispatcher
                            && messageId.equals(((FileTransferDispatcher) chatItem).mFileStruct.mFileTransferTag.toString())) {
                        Logger
                        .e(TAG,
                                "removeChatMessage() found the message !" + (FileTransferDispatcher) chatItem);
                        mChatItemList.remove(chatItem);
                    }
                }               
              
                for (IChatWindow chatWindow : chatWindows) {
                    chatWindow.removeChatMessage(messageId);
                }
                
            } else {
                Logger
                        .e(TAG,
                                "removeChatMessage() cannot find the target message with id, messageId is null!");
            }            
        }

        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            Logger.i(TAG, "addSentFileTransfer() FileStruct is" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new SentFileTransferDispatcher(file);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    fileTransferDispatcher.onAddChatWindow((IOne2OneChatWindow) chatWindow);
                } else {
                    Logger
                            .d(TAG,
                                    "addSentFileTransfer() not a one-2-one chat window, just pass it");
                }
            }
            mChatItemList.add(fileTransferDispatcher);
            return fileTransferDispatcher;
        }
    }

    private static class GroupChatWindowDispatcher extends ChatWindowDispatcher implements
            IGroupChatWindow {
        public static final String TAG = "GroupChatWindowDispatcher";
        private CopyOnWriteArrayList<ParticipantInfo> mParticipantInfos = null;
        private int mStatus = 0;
        private boolean groupChatLoadedFromDatabase = false;
        private String mGroupChatSubject ="";
        private int mFileTransferDisableReason = 0;
        private String mChatID = "";
        

        public String getmChatID() {
            return mChatID;
        }

        public void setmChatID(String mChatID) {
            this.mChatID = mChatID;
        }


        public GroupChatWindowDispatcher(Object tag,
                CopyOnWriteArrayList<ParticipantInfo> participantList) {
            super(tag);
            if (participantList == null) {
                Logger.e(TAG, "when create group chat, participantSet must not be null");
                throw new RuntimeException("when create group chat,participantSet must not be null");
            }
            if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
                Logger.e(TAG, "when create group chat, participantSet's number must more than 2");
                throw new RuntimeException(
                        "when create group chat, participantSet's number must more than 2");
            }
            mTag = tag;
            mParticipantInfos = new CopyOnWriteArrayList<ParticipantInfo>(participantList);
        }

       @Override
        public void setFileTransferEnable(int reason) {
            mFileTransferDisableReason = reason;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).setFileTransferEnable(reason);
                }
            }
        }
       @Override
       public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
           Logger.i(TAG, "addReceivedFileTransfer() FileStruct is" + file);
           Collection<IChatWindow> chatWindows = mChatWindowMap.values();
           FileTransferDispatcher fileTransferDispatcher = new ReceivedFileTransferDispatcher(file, isAutoAccept);
           for (IChatWindow chatWindow : chatWindows) {
               if (chatWindow instanceof IGroupChatWindow) {
                   fileTransferDispatcher.onAddChatWindow((IGroupChatWindow) chatWindow);
               } else {
                   Logger.d(TAG,
                           "addReceivedFileTransfer() not a group chat window, just pass it");
               }
           }
           mChatItemList.add(fileTransferDispatcher);
           return fileTransferDispatcher;
       }
       
       @Override
       public IFileTransfer addSentFileTransfer(FileStruct file) {
           Logger.i(TAG, "addSentFileTransfer() FileStruct is" + file);
           Collection<IChatWindow> chatWindows = mChatWindowMap.values();
           FileTransferDispatcher fileTransferDispatcher = new SentFileTransferDispatcher(file);
           for (IChatWindow chatWindow : chatWindows) {
               if (chatWindow instanceof IGroupChatWindow) {
                   fileTransferDispatcher.onAddChatWindow((IGroupChatWindow) chatWindow);
               } else {
                   Logger
                           .d(TAG,
                                   "addSentFileTransfer() not a one-2-one chat window, just pass it");
               }
           }
           mChatItemList.add(fileTransferDispatcher);
           return fileTransferDispatcher;
       }
        @Override
        public void onAddChatWindowManager(IChatWindowManager chatWindowManager) {
            Logger.d(TAG, "onAddChatWindowManager() entry, the chatWindowManager is "
                    + chatWindowManager);
            IGroupChatWindow chatWindow = null;
            chatWindow = chatWindowManager.addGroupChatWindow(mTag, mParticipantInfos);
            if (null != chatWindow) {
                mChatWindowMap.put(chatWindowManager, chatWindow);               
                ((IGroupChatWindow) chatWindow).updateChatStatus(mStatus);
                if (chatWindow instanceof GroupChatWindow) {
                    if (((GroupChatWindow) chatWindow)
                            .getmGroupChatFragment() != null)
                        
                ((GroupChatWindow) chatWindow).addgroupSubject(mGroupChatSubject);
            }
                if (chatWindow instanceof GroupChatWindow) {
                    if (((GroupChatWindow) chatWindow)
                            .getmGroupChatFragment() != null && !groupChatLoadedFromDatabase) {
                        ArrayList<Integer> messageIdArray = null;
                        groupChatLoadedFromDatabase = true; 
                        mChatItemList.clear();
                        ArrayList<Participant> participantList = new ArrayList<Participant>();
                        for(ParticipantInfo participantinfo : mParticipantInfos)
                        {
                            participantList.add(participantinfo.getParticipant());  
                        }                        
                        if(RichMessagingDataProvider.getInstance()==null)
                            RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
                        messageIdArray = RichMessagingDataProvider
                                .getInstance().getRecentChatsForGroup(mChatID,0);
                        if (!messageIdArray.isEmpty()) {
                        Collections.reverse(messageIdArray);
                        
                        /*try {
                            Message controllerMessage = ControllerImpl
                                    .getInstance()
                                    .obtainMessage(
                                            ChatController.EVENT_RELOAD_MESSAGE,
                                            mTag.toString(), messageIdArray);
                            controllerMessage.sendToTarget();
                        } catch (Exception e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }*/
                    }
                }
                }
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onAddChatWindow(chatWindow);
                }
                
            } else {
                Logger.e(TAG, "onAddChatWindowManager() chatWindow is null");
            }
        }

        @Override
        public void updateParticipants(List<ParticipantInfo> participantList) {
            Set<IChatWindowManager> keys = mChatWindowMap.keySet();
            mParticipantInfos = new CopyOnWriteArrayList<ParticipantInfo>(participantList);
            for (IChatWindowManager chatWindowManager : keys) {
                IChatWindow chatWindow = mChatWindowMap.get(chatWindowManager);
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).updateParticipants(mParticipantInfos);
                } else {
                    Logger.e(TAG, "updateParticipants() chatWindow is " + chatWindow);
                }
            }
        }

        @Override
        public void setIsComposing(boolean isComposing, Participant participant) {
            Logger.d(TAG, "setIsComposing isComposing" + isComposing + "participant is "
                    + participant);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).setIsComposing(isComposing, participant);
                } else {
                    Logger.e(TAG, "the chatwindow is not a group chat window");
                }
            }
        }

        @Override
        void onDestroy() {
            Set<IChatWindowManager> managers = mChatWindowMap.keySet();
            for (IChatWindowManager manager : managers) {
                manager.removeChatWindow(mChatWindowMap.get(manager));
            }
        }

        @Override
        public void updateChatStatus(int status) {
            Logger.d(TAG, "updateChatStatus() status: " + status);
            mStatus = status;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).updateChatStatus(status);
                }
            }
        }

        @Override
        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            Logger.i(TAG, "addChatEventInformation() chatEventStruct is" + chatEventStruct);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            ChatEventInformationDispatcher chatEventDispatcher =
                    new ChatEventInformationDispatcher(chatEventStruct);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    chatEventDispatcher.onAddChatWindow((IGroupChatWindow) chatWindow);
                } else {
                    Logger
                            .d(TAG,
                                    "addChatEventInformation() not a group chat window, just pass it");
                }
            }
            mChatItemList.add(chatEventDispatcher);
            return (IChatEventInformation) chatEventDispatcher;
        }
        
        @Override
        public void removeChatMessage(String messageId) {
            if (null != messageId) {
                Collection<IChatWindow> chatWindows = mChatWindowMap.values();
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof ChatMessage
                            && messageId.equals(((ChatMessage) chatItem).getId())) {
                        Logger
                        .e(TAG,
                                "removeChatMessage() found the message !" + (ChatMessage) chatItem);
                        mChatItemList.remove(chatItem);
                    }
                }
                for (IChatWindow chatWindow : chatWindows) {
                    chatWindow.removeChatMessage(messageId);
                }
            } else {
                Logger
                        .e(TAG,
                                "removeChatMessage() cannot find the target message with id, messageId is null!");
            }            
        }

        @Override
        public void addgroupSubject(String subject) {
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();            
            for (IChatWindow chatWindow : chatWindows) {                
                if((chatWindow instanceof IGroupChatWindow))                {
                    mGroupChatSubject = subject;
                    ((IGroupChatWindow)chatWindow).addgroupSubject(subject);
                }            
               
            }            
        }
    }

    @Override
    public void switchChatWindowByTag(ParcelUuid uuidTag) {
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindowManager.switchChatWindowByTag(uuidTag);
        }
    }

    /**
     * This is a common interface for the item listed in a chat window
     */
    private interface IChatItemDispatcher {
        /**
         * Called by the ViewImpl while there is a new ChatWindowManager
         * registered
         * 
         * @param chatWindow
         */
        void onAddChatWindow(IChatWindow chatWindow);

        void onRemoveChatWindow(IChatWindow chatWindow);
    }

    /**
     * This class defines a base chat message type in a chat window, which is
     * used to transfer Model statuses to multiple Views
     */
    public abstract static class ChatMessage implements IChatWindowMessage, IChatItemDispatcher {
        protected InstantMessage mMessage = null;

        protected ConcurrentHashMap<IChatWindow, IChatWindowMessage> mChatWindowMap =
                new ConcurrentHashMap<IChatWindow, IChatWindowMessage>();

        protected ChatMessage(InstantMessage message) {
            mMessage = message;
        }

        @Override
        public final String getId() {
            if (null != mMessage) {
                return mMessage.getMessageId();
            } else {
                Logger.e(TAG, "ChatMessage->getId() mMessage is null!");
                return null;
            }
        }
    }

    private abstract static class FileTransferDispatcher implements IFileTransfer,
            IChatItemDispatcher {
        public static final String TAG = "FileTransferDispatcher";

        private FileStruct mFileStruct = null;

        private Status mStatus = null;

        private long mProgress = -1;

        protected ConcurrentHashMap<IChatWindow, IFileTransfer> mChatWindowMap =
                new ConcurrentHashMap<IChatWindow, IFileTransfer>();

        public FileTransferDispatcher(FileStruct fileStruct) {
            mFileStruct = fileStruct;
        }

        public void onAddChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onAddChatWindow() entry");
            if (null != chatWindow) {
                IFileTransfer fileTransfer = getFileTransfer(chatWindow, mFileStruct);
                if (null != fileTransfer) {
                    if (null != mStatus) {
                        fileTransfer.setStatus(mStatus);
                    }
                    if (-1 != mProgress) {
                        fileTransfer.setProgress(mProgress);
                    }
                    mChatWindowMap.put(chatWindow, fileTransfer);
                } else {
                    Logger.e(TAG, "onAddChatWindow() fileTransfer is null");
                }
            } else {
                Logger.e(TAG, "onAddChatWindow() chatWindow is null");
            }
        }

        public void onRemoveChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onRemoveChatWindow() entry");
            mChatWindowMap.remove(chatWindow);
        }

        @Override
        public void setProgress(long progress) {
            Logger.i(TAG, "setProgress() entry, progress is " + progress);
            mProgress = progress;
            Collection<IFileTransfer> fileTransfers = mChatWindowMap.values();
            for (IFileTransfer fileTransfer : fileTransfers) {
                fileTransfer.setProgress(mProgress);
            }
        }

        @Override
        public void setStatus(Status status) {
            if (mStatus == status) {
                Logger
                        .d(TAG,
                                "setStatus() the new status equals the old one, so no need to update");
            } else {
                Logger.i(TAG, "setStatus() entry, status is " + status);
                mStatus = status;
                Collection<IFileTransfer> fileTransfers = mChatWindowMap.values();
                for (IFileTransfer fileTransfer : fileTransfers) {
                    fileTransfer.setStatus(mStatus);
                }
            }
        }

        @Override
        public void setFilePath(String filePath) {
            Logger.i(TAG, "setFilePath() entry, filePath is " + filePath);
            if (mFileStruct != null) {
                mFileStruct.mFilePath = filePath;
            } else {
                Logger.e(TAG, "setFilePath, mFileStruct is null!");
            }

            if (mChatWindowMap != null) {
                Collection<IFileTransfer> fileTransfers = mChatWindowMap.values();
                for (IFileTransfer fileTransfer : fileTransfers) {
                    fileTransfer.setFilePath(filePath);
                }
            } else {
                Logger.e(TAG, "setFilePath, mChatWindowMap is null!");
            }

        }

        @Override
        public void updateTag(String transferTag, long size) {
            Logger.i(TAG, "updateTag() entry, transferTag is " + transferTag);
            if (mFileStruct != null) {
                mFileStruct.mFileTransferTag = transferTag;
            } else {
                Logger.e(TAG, "updateTag, mFileStruct is null!");
            }

            if (mChatWindowMap != null) {
                Collection<IFileTransfer> fileTransfers = mChatWindowMap.values();
                for (IFileTransfer fileTransfer : fileTransfers) {
                    fileTransfer.updateTag(transferTag, size);
                }
            } else {
                Logger.e(TAG, "updateTag, mChatWindowMap is null!");
            }
        }

        abstract IFileTransfer getFileTransfer(IChatWindow chatWindow, FileStruct fileStruct);
    }

    private static class SentFileTransferDispatcher extends FileTransferDispatcher {

        public SentFileTransferDispatcher(FileStruct fileStruct) {
            super(fileStruct);
        }

        @Override
        IFileTransfer getFileTransfer(IChatWindow chatWindow, FileStruct fileStruct) {
        	if(chatWindow instanceof OneOneChatWindow)
        	{
                return ((IOne2OneChatWindow) chatWindow).addSentFileTransfer(fileStruct);
        	}
        	else if(chatWindow instanceof GroupChatWindow)
        	{
        		return ((IGroupChatWindow) chatWindow).addSentFileTransfer(fileStruct);
        	}
        	else if(chatWindow instanceof ChatWindowManagerAdapter || chatWindow instanceof One2OneChatWindowAdapter)
        	{
        		return ((IOne2OneChatWindow) chatWindow).addSentFileTransfer(fileStruct);
        	}
        	else
        	{
        		return null;
        	}
        }

    }

    private static class ReceivedFileTransferDispatcher extends FileTransferDispatcher {

        private boolean mIsAutoAccept = false;

        public ReceivedFileTransferDispatcher(FileStruct fileStruct, boolean isAutoAccept) {
            super(fileStruct);
            mIsAutoAccept = isAutoAccept;
        }

        @Override
        IFileTransfer getFileTransfer(IChatWindow chatWindow, FileStruct fileStruct) {
        	if(chatWindow instanceof OneOneChatWindow)
        	{
                return ((IOne2OneChatWindow) chatWindow).addReceivedFileTransfer(fileStruct, mIsAutoAccept);
        	}
        	else if(chatWindow instanceof GroupChatWindow)
        	{
        		return ((IGroupChatWindow) chatWindow).addReceivedFileTransfer(fileStruct, mIsAutoAccept);
        	}
        	else if(chatWindow instanceof ChatWindowManagerAdapter || chatWindow instanceof One2OneChatWindowAdapter)
        	{
        		return ((IOne2OneChatWindow) chatWindow).addReceivedFileTransfer(fileStruct, mIsAutoAccept);
        	}
        	else
        	{
        		return null;
        	}
        }
    }

    private static class ChatEventInformationDispatcher implements IChatItemDispatcher,
            IChatEventInformation {
        public static final String TAG = "ChatEventInformationDispatcher";
        protected ConcurrentHashMap<IChatWindow, IChatEventInformation> mChatWindowMap =
                new ConcurrentHashMap<IChatWindow, IChatEventInformation>();
        private ChatEventStruct mChatEventStruct = null;

        public ChatEventInformationDispatcher(ChatEventStruct chatEventStruct) {
            mChatEventStruct = chatEventStruct;
        }

        @Override
        public void onAddChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onAddChatWindow() entry");
            if (null != chatWindow) {
                IChatEventInformation chatEvent =
                        ((IGroupChatWindow) chatWindow).addChatEventInformation(mChatEventStruct);
                if (null != chatEvent) {
                    mChatWindowMap.put(chatWindow, chatEvent);
                } else {
                    Logger.e(TAG, "onAddChatWindow() chatEvent is null");
                }
            } else {
                Logger.e(TAG, "onAddChatWindow() chatWindow is null");
            }

        }

        @Override
        public void onRemoveChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onRemoveChatWindow() entry");
            mChatWindowMap.remove(chatWindow);
        }
    }
}
