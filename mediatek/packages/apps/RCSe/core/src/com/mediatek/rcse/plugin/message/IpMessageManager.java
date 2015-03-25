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

package com.mediatek.rcse.plugin.message;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.MediaFile;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.text.TextUtils;
import android.widget.Toast;


import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.MessageManager;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.mms.ipmessage.message.IpTextMessage;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.binder.FileStructForBinder;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Random;
import java.util.Set;
import java.util.TreeSet;
import java.util.UUID;

/**
* Provide message management related interface
*/
public class IpMessageManager extends MessageManager {
    private static final String TAG = "IpMessageManager";
    private static HashMap<Long, IpMessage> sCacheRcseMessage = new HashMap<Long, IpMessage>();
    private static HashMap<String, ArrayList<Long>> numberToMessageIdsMap = new HashMap<String, ArrayList<Long>>();
    private static HashMap<String, Long> sMessageMap = new HashMap<String, Long>();
    private HashMap<Long, PresentTextMessage> mPreSentMessageList = new HashMap<Long, PresentTextMessage>();
    private static final int SAVE_SUCCESS = 1;
    private static final int SAVE_FAIL = -1;
    private static final Random RANDOM = new Random();
    private static final int MESSAGE_TAG_RANGE = 1000;
    private static final String SPACE = " ";
    private static final int FAILED = 5;
    private String mRejected = null;
    private String mCanceled = null;
    private String mYou = null;
    private String mFailed = null;
    private String mWarningNoFtCapability = null;
    static final String COMMA = ",";
    public Context mContext;
    private Handler mUiHandler = null;

    public IpMessageManager(Context context) {
        super(context);
        mContext = context;
        initializeStringInRcse();
        mUiHandler = new Handler(Looper.getMainLooper());
        Logger.d(TAG, "MessageManagerExt() entry ");
    }

    private void initializeStringInRcse() {
        mRejected = PluginUtils.getStringInRcse(R.string.file_transfer_rejected);
        mCanceled = PluginUtils.getStringInRcse(R.string.file_transfer_canceled);
        mYou = PluginUtils.getStringInRcse(R.string.file_transfer_you);
        mFailed = PluginUtils.getStringInRcse(R.string.file_transfer_failed);
        mWarningNoFtCapability = PluginUtils
                .getStringInRcse(R.string.warning_no_file_transfer_capability);
    }

    /*
     * Judge message is in the cache
     */
    public static boolean isInCache(Long messageIdInMms) {
        Logger.d(TAG, "isInCache() entry! messageIdInMms = " + messageIdInMms);
        synchronized (sCacheRcseMessage) {
            return (sCacheRcseMessage.containsKey(messageIdInMms));
        }
    }

    /**
     * Remove present message after it has been really sent out
     * @param messageTag The tag of the present message
     * @return TRUE if the message has been removed, otherwise FALSE
     */
    public boolean removePresentMessage(int messageTag) {
        synchronized (mPreSentMessageList) {
            Collection<PresentTextMessage> values = mPreSentMessageList.values();
            for (PresentTextMessage message : values) {
                if (messageTag == message.messageTag) {
                    Logger.d(TAG, "removePresentMessage() messageTag: " + messageTag + " found!");
                    values.remove(message);
                    return true;
                }
            }
            Logger.d(TAG, "removePresentMessage() messageTag: " + messageTag + " not found!");
            return false;
        }
    }

    // If true, MMS will only use XMS to send messages. Joyn will not be used
	public boolean isJoynMessagingDisabled() {
		if (RcsSettings.getInstance() == null)
			RcsSettings.createInstance(mContext);
		boolean joynDisabled = RcsSettings.getInstance()
				.getJoynMessagingDisabledFullyIntegrated();
		Logger.d(TAG, "isJoynMessagingDisabled() entry! disableStatus = "
				+ joynDisabled);
		return joynDisabled;
    }
    
    /**

     * Clear all history call from mms conversation list

     */
    public static void clearAllHistory()
    {
    	Logger.d(TAG, "clearAllHistory() entry!");
        synchronized (sCacheRcseMessage) {
            sCacheRcseMessage.clear();
        }
        synchronized (sMessageMap) {
            sMessageMap.clear();
        }
    	((ModelImpl) ModelImpl.getInstance()).clearAllChatHistory();
    }   
    
    private void clearAllChatHistory(List<Long> messageIdsinMMS, String number)
    {
    	Logger.d(TAG, "clearAllChatHistory() entry!");               
		for (Long messageId : messageIdsinMMS) {
			synchronized (sCacheRcseMessage) {
				sCacheRcseMessage.remove(messageId);
			}
			synchronized (sMessageMap) {
				sMessageMap.remove(getMessageIdinRcse(messageId));
			}
		}
		String tag = PluginChatWindowManager.findWindowTagIndex(number).getWindowTag();
        Logger.d(TAG, "clearHistory entry");        
        Message controllerMessage = PluginController.obtainMessage(
        ChatController.EVENT_CLEAR_CHAT_HISTORY, tag,"");
        controllerMessage.sendToTarget();
    }  
   
    public void clearChatHistory(String number)
    {
    	ArrayList<Long>messageIdsForNumber = null;
    	messageIdsForNumber = numberToMessageIdsMap.get(number);
    	if(messageIdsForNumber!=null && !messageIdsForNumber.isEmpty())
    	{
    		clearAllChatHistory(messageIdsForNumber, number);
    	}
    	//PluginUtils.removeMessagesFromDatabase(messageIdsForNumber);
    }
    	
    /*
     * Judge message is in the cache
     */
    public static boolean isInCache(String messageIdInRcse) {
        Logger.d(TAG, "isInCache() entry! messageIdInRcse = " + messageIdInRcse);
        synchronized (sMessageMap) {
            return (sMessageMap.containsKey(messageIdInRcse));
        }
    }
    /*
     * Max length of message a user can send
     */
    
    public int getMaxTextLimit()
    {
    	int limit = PluginUtils.getMaximumTextLimit();
    	Logger.d(TAG, "getMaxTextLimit " + limit);
    	return limit;
    }

    /*
     * Add message into cache, if it in cache, then update it
     */
    public static void addMessage(Long messageIdInMms, String messageIdInRcse, IpMessage message) {
        Logger.d(TAG, "addMessage() entry! messageIdInMms = " + messageIdInMms + " messageIdInRcse = " + messageIdInRcse
                + " message = " + message);
        ArrayList<Long> messageIds = new ArrayList<Long>();
        synchronized (sCacheRcseMessage) {
            // If the map contains the key(messageIdInMms), will update the
            // value(message)
            sCacheRcseMessage.put(messageIdInMms, message);
			if (numberToMessageIdsMap.containsKey(message.getTo())) {
				Logger.d(TAG,
						"addMessage() already contains key! messageIdInMms = "
								+ messageIdInMms + " messageIdInRcse = "
								+ messageIdInRcse + " message contact = "
								+ message.getTo());
				messageIds = numberToMessageIdsMap.get(message.getTo());
				messageIds.add(messageIdInMms);
				numberToMessageIdsMap.put(message.getTo(), messageIds);
			} else {
				Logger.d(TAG,
						"addMessage() doesnot contain key! messageIdInMms = "
								+ messageIdInMms + " messageIdInRcse = "
								+ messageIdInRcse + " message contact = "
								+ message.getTo());
				messageIds.add(messageIdInMms);
				numberToMessageIdsMap.put(message.getTo(), messageIds);
			}
                        
        }

        synchronized (sMessageMap) {
            // First remove the entry which value contains messageIdInMms, then
            // add a new entry to the map for update the messageIdInRcse
            sMessageMap.values().remove(messageIdInMms);
            sMessageMap.put(messageIdInRcse, messageIdInMms);
        }
    }

    public static void removeMessage(Long messageIdInMms, String messageIdInRcse, String number) {
        Logger.d(TAG, "removeMessage() entry! messageIdInMms = " + messageIdInMms + " messageIdInRcse = " + messageIdInRcse);
        synchronized (sCacheRcseMessage) {
            sCacheRcseMessage.remove(messageIdInMms);
        }
        synchronized (sMessageMap) {
            sMessageMap.remove(messageIdInRcse);
        }     
               
    }


    /*
     * Remove message into cache
     */
    public static void deleteMessage(Long messageIdInMms, String messageIdInRcse, String number) {
		Logger.d(TAG, "deleteMessage() entry! messageIdInMms = "
				+ messageIdInMms + " messageIdInRcse = " + messageIdInRcse);
        synchronized (sCacheRcseMessage) {
            sCacheRcseMessage.remove(messageIdInMms);
        }
        synchronized (sMessageMap) {
            sMessageMap.remove(messageIdInRcse);
        }

		Logger.d(TAG, "deleteMessage() " + messageIdInMms + "number ="
				+ number);
		String tag = PluginChatWindowManager.findWindowTagIndex(number)
				.getWindowTag();		
		Message controllerMessage = PluginController.obtainMessage(
				ChatController.EVENT_DELETE_MESSAGE, tag, messageIdInRcse);
		controllerMessage.sendToTarget();
    }

    // MMS will call this interface to remove message from RCSe, and send by SMS OR user has deleted the message
    public void deleteIpMsg(long[] ids, boolean delImportant, boolean delLocked) { 
		Logger.d(TAG, "deleteIpMsg() entry! " + ids);
		IpMessage message = null;
		if(ids.length!=0)
			message = getMessage(ids[0]);	    	
		if (message != null) {
		String number = message.getTo();
		if (number == null || number.equals(""))
			number = message.getFrom();
		ArrayList<Long> messageIdsForNumber = new ArrayList<Long>();
		for (long id : ids) {
			messageIdsForNumber.add(id);
		}
		if (messageIdsForNumber.size() > 1)
			clearAllChatHistory(messageIdsForNumber, number);
		else
			deleteMessage(ids[0], getMessageIdinRcse(ids[0]), number);
    }
    }
    /*
     * get message from cache
     */
    public static IpMessage getMessage(Long messageIdInMms) {
        synchronized (sCacheRcseMessage) {
            return sCacheRcseMessage.get(messageIdInMms);
        }
    }

    /*
     * get message id from cache
     */
    public static Long getMessageId(String messageIdInRcse) {
        synchronized (sMessageMap) {
            return sMessageMap.get(messageIdInRcse);
        }
    }

    /*
     * get rcse message id from cache
     */
    public static String getMessageIdinRcse(Long messageIdMms) {
        synchronized (sMessageMap) {
        	for (Entry<String, Long> e : sMessageMap.entrySet()) {
        		String key = e.getKey();
        		Long value = e.getValue();
        	    if(value.equals(messageIdMms))
        	    	return key;
        	}
        }
        return null;
    }

    public static void updateCache(String oldTag, FileStructForBinder fileStruct, String remote) {
        String newTag = fileStruct.fileTransferTag;
        Logger.d(TAG, "updateCache(), oldTag = " + oldTag + " newTag = " + newTag);
        long messageIdInMms = getMessageId(oldTag);
        int type = getMessage(messageIdInMms).getType();
        removeMessage(messageIdInMms, oldTag, remote);
        IpMessage message = PluginUtils.exchangeIpMessage(type, fileStruct, remote);
        addMessage(messageIdInMms, newTag, message);
    }

    @Override
    public int getStatus(long msgId) {
        Logger.d(TAG, "getStatus() entry with msgId " + msgId);
        Status status = null;
        IpMessage message = getMessage(msgId);
        if (message == null) {
            Logger.e(TAG, "getStatus(), message is null!");
            return 0;
        }
        if (message instanceof PluginIpTextMessage) {
            status = ((PluginIpTextMessage) message).getMessageStatus();
            return convertToMmsStatus(status);
        } else {
            Logger.w(TAG, "getStatus() ipMessage is " + message);
            int messageType = message.getType();
            switch (messageType) {
                case IpMessageConsts.IpMessageType.PICTURE:
                    if (message instanceof PluginIpImageMessage) {
                        return ((PluginIpImageMessage) message).getStatus();
                    } else {
                        return IpMessageConsts.IpMessageStatus.MO_INVITE;
                    }
                case IpMessageConsts.IpMessageType.VIDEO:
                    if (message instanceof PluginIpVideoMessage) {
                        return ((PluginIpVideoMessage) message).getStatus();
                    } else {
                        return IpMessageConsts.IpMessageStatus.MO_INVITE;
                    }
                case IpMessageConsts.IpMessageType.VOICE:
                    if (message instanceof PluginIpVoiceMessage) {
                        return ((PluginIpVoiceMessage) message).getStatus();
                    } else {
                        return IpMessageConsts.IpMessageStatus.MO_INVITE;
                    }
                case IpMessageConsts.IpMessageType.VCARD:
                    if (message instanceof PluginIpVcardMessage) {
                        return ((PluginIpVcardMessage) message).getStatus();
                    } else {
                        return IpMessageConsts.IpMessageStatus.MO_INVITE;
                    }
                default:
                    return ((PluginIpAttachMessage) message).getStatus();
            }
        }
    }

    /*
     * Get the ip message information
     */
    public IpMessage getIpMsgInfo(long msgId) {
        Logger.d(TAG, "getIpMsgInfo() msgId = " + msgId);
        IpMessage ipMessage = getMessage(msgId);
        if (ipMessage != null) {
            Logger.d(TAG, "getIpMsgInfo() found in message cache");
           // Logger.d(TAG, "getIpMsgInfo()Status is " + ((PluginIpTextMessage)ipMessage).getMessageStatus());
            return ipMessage;
        } else {
            Logger.e(TAG, "Can not find nessage in the cache!");
            synchronized (mPreSentMessageList) {
                ipMessage = mPreSentMessageList.get(msgId);
            }
            if (ipMessage != null) {
                Logger.d(TAG, "getIpMsgInfo() found in present message list");
                return ipMessage;
            }
            Logger.e(TAG, "getIpMsgInfo() cannot find this message, forget to add it into cache?");
            return null;
        } 
    }
    
    /**
     * Convert status in RCSe to the status corresponding in Mms
     * 
     * @param status The status in RCSe
     * @return status in Mms
     */
    static public int convertToMmsStatus(Status status) {
        Logger.d(TAG, "convertToMmsStatus() entry with status is " + status);
        int statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
        if (status == null) {
            return IpMessageConsts.IpMessageStatus.INBOX;
        }
        switch (status) {
            case SENDING:
                statusInMms = IpMessageConsts.IpMessageStatus.OUTBOX;
                break;
            case DELIVERED:
                statusInMms = IpMessageConsts.IpMessageStatus.DELIVERED;
                break;
            case DISPLAYED:
                statusInMms = IpMessageConsts.IpMessageStatus.VIEWED;
                break;
            case FAILED:
                statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
                break;
            default:
                statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
                break;
        }
        Logger.d(TAG, "convertToMmsStatus() entry exit with statusInMms is " + statusInMms);
        return statusInMms;
    }
    
        
    /*
     * Sent an ip message
     */
    public int saveIpMsg(IpMessage msg, int sendMsgMode) {
        String contact = msg.getTo();
        if (TextUtils.isEmpty(contact)) {
            Logger.w(TAG, "saveIpMsg() invalid contact: " + contact);
            return SAVE_FAIL;
        }

        if (msg instanceof IpTextMessage) {
            String messageBody = ((IpTextMessage) msg).getBody();
            Logger.d(TAG, "saveIpMsg() send a text message: " + messageBody + " to contact: " + contact);
            return saveChatMsg(messageBody, contact);
        } else if (msg instanceof IpAttachMessage) {
            Logger.d(TAG, "saveIpMsg() send a file to contact: " + contact);
            return saveFileTransferMsg(msg, contact);
        } else {
            Logger.w(TAG, "saveIpMsg() unsupported ip message type");
            return SAVE_FAIL;
        }
    }

    /*
     * Get the transferring progress
     */
    @Override
    public int getDownloadProcess(long msgId) {
        Logger.d(TAG, "getDownloadProcess(), msgId = " + msgId);
        IpMessage message = getMessage(msgId);
        if (message == null) {
            Logger.e(TAG, "getDownloadProcess(), message is null!");
            return 0;
        }
        int messageType = message.getType();
        switch (messageType) {
            case IpMessageConsts.IpMessageType.PICTURE:
                return ((PluginIpImageMessage) message).getProgress();
            case IpMessageConsts.IpMessageType.VIDEO:
                return ((PluginIpVideoMessage) message).getProgress();
            case IpMessageConsts.IpMessageType.VOICE:
                return ((PluginIpVoiceMessage) message).getProgress();
            case IpMessageConsts.IpMessageType.VCARD:
                return ((PluginIpVcardMessage) message).getProgress();
            default:
                return ((PluginIpAttachMessage) message).getProgress();
        }
    }

    /*
     * Set transfer status when user click button in mms
     */
    @Override
    public void setIpMessageStatus(long msgId, int msgStatus) {
        Logger.d(TAG, "setIpMessageStatus(), msgId = " + msgId + " msgStatus = " + msgStatus);
        IpMessage message = getMessage(msgId);
        if (message == null) {
            Logger.e(TAG, "setIpMessageStatus(), message is null!");
            return;
        }
        String contact = message.getFrom();
    	if(contact == null)
    		contact = message.getTo();
        int messageType = message.getType();
        String messageTag;
        // Get file transfer tag
        switch (messageType) {
            case IpMessageConsts.IpMessageType.PICTURE:
                messageTag = ((PluginIpImageMessage) message).getTag();
                break;
            case IpMessageConsts.IpMessageType.VIDEO:
                messageTag = ((PluginIpVideoMessage) message).getTag();
                break;
            case IpMessageConsts.IpMessageType.VOICE:
                messageTag = ((PluginIpVoiceMessage) message).getTag();
                break;
            case IpMessageConsts.IpMessageType.VCARD:
                messageTag = ((PluginIpVcardMessage) message).getTag();
                break;
            default:
                messageTag = ((PluginIpAttachMessage) message).getTag();
                break;
        }

        Logger.d(TAG, "setIpMessageStatus(), messageTag is " + messageTag);
        // Sent message to rcse controller
        Message controllerMessage = null;
        switch (msgStatus) {
            case IpMessageConsts.IpMessageStatus.MO_INVITE:
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_RESENT, message.getFrom(), messageTag);
                break;
            case IpMessageConsts.IpMessageStatus.MO_CANCEL:
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_CANCEL, message.getFrom(), messageTag);
                break;
            case IpMessageConsts.IpMessageStatus.MT_RECEIVING:
            	onUserAccept(mContext,message,messageTag);                
                break;
            case IpMessageConsts.IpMessageStatus.MT_REJECT:
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_RECEIVER_REJECT, message.getFrom(),
                        messageTag);
                break;
            case IpMessageConsts.IpMessageStatus.MO_PAUSE:            	
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_PAUSE, contact,
                        messageTag);
                break;
            case IpMessageConsts.IpMessageStatus.MT_PAUSE:            	
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_PAUSE, contact,
                        messageTag);
                break;
            case IpMessageConsts.IpMessageStatus.MO_RESUME:            	
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_RESUME, contact,
                        messageTag);
                break;
            case IpMessageConsts.IpMessageStatus.MT_RESUME:            	
                controllerMessage = PluginController.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_RESUME, contact,
                        messageTag);
                break;
            default:
                break;
        }

        if (controllerMessage == null) {
            Logger.e(TAG, "setIpMessageStatus(), controllerMessage is null!");
            return;
        }
        controllerMessage.sendToTarget();
    }

    private void onUserAccept(Context context, IpMessage message, String tag) {
        long maxFileSize = ApiManager.getInstance().getMaxSizeforFileThransfer();
        long warningFileSize = ApiManager.getInstance().getWarningSizeforFileThransfer();
        Logger.v(TAG, "onUserAccept entry: maxFileSize = " + maxFileSize + ", warningFileSize = "
                + warningFileSize);
        
        IpAttachMessage ipAttachMessage = ((IpAttachMessage) message);
                
        long fileSize = ipAttachMessage.getSize();      
        if (fileSize >= warningFileSize && warningFileSize != 0) {
            SharedPreferences sPrefer = PreferenceManager.getDefaultSharedPreferences(context);
            Boolean isRemind = sPrefer.getBoolean(SettingsFragment.RCS_REMIND, false);
            Logger.w(TAG, "WarningDialog: isRemind = " + isRemind);
            if (isRemind) {
            	Logger.v(TAG, "onUserAccept Is Remind true");
                Intent intent = new Intent(InvitationDialog.ACTION);
                intent.putExtra(RcsNotification.CONTACT, message.getFrom());
                intent.putExtra(RcsNotification.SESSION_ID, tag);
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_FILE_TRANSFER_SIZE_WARNING);
                intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, true);
                String content = AndroidFactory.getApplicationContext().getString(R.string.file_size_warning_message);
                intent.putExtra(RcsNotification.NOTIFY_CONTENT, content);
                intent.putExtra(RcsNotification.NOTIFY_SIZE, String.valueOf(fileSize));
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
            } else {            	
            	onUserAcceptFile(mContext,message,tag);  
            }
        } else {
        	if(!RcsSettings.getInstance().isFileTransferAutoAccepted())
        	{
        		Logger.v(TAG, "onUserAccept Is File transfer Accept false");
        		Intent intent = new Intent(InvitationDialog.ACTION);
                intent.putExtra(RcsNotification.CONTACT, message.getFrom());
                intent.putExtra(RcsNotification.SESSION_ID, tag);
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_AUTO_ACCEPT_FILE);
                intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, true);
                String content = "Do you want to auto accept file";
                intent.putExtra(RcsNotification.NOTIFY_CONTENT, content);
                intent.putExtra(RcsNotification.NOTIFY_SIZE, String.valueOf(fileSize));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
        	}
        	onUserAcceptFile(mContext,message,tag);  
        }

        Logger.v(TAG, "onUserAccept exit");
    }
    
    private void onUserAcceptFile(Context context, IpMessage message, String tag) 
    {
    	Logger.v(TAG, "onUserAcceptFile");
    	Message controllerMessage = null;
    	controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_RECEIVER_ACCEPT, message.getFrom(),
                tag);
    	if (controllerMessage == null) {
            Logger.e(TAG, "setIpMessageStatus(), controllerMessage is null!");
            return;
        }
        controllerMessage.sendToTarget();
    }
    
    private int saveChatMsg(String message, String contact) {
        if (TextUtils.isEmpty(message)) {
            Logger.e(TAG, "saveChatMsg() invalid message: " + message);
            return SAVE_FAIL;
        }
        Logger.d(TAG, "saveChatMsg() message: " + message + " , contact: " + contact);
        if(PluginUtils.getMessagingMode() == 0)
{
        if (contact.startsWith(IpMessageConsts.JOYN_START)) {
            contact = contact.substring(4);
        }
}
        if (!contact.contains(COMMA)) {
            int messageTag = generateMessageTag();
            PresentTextMessage ipMessage = new PresentTextMessage(messageTag, contact, message);
            mPreSentMessageList.put(PluginUtils.storeMessageInMmsDb(messageTag, message, contact,
                    PluginUtils.OUTBOX_MESSAGE, 0), ipMessage);
            sentMessageViaRCSe(message, contact, messageTag);
        } else {
            Set<String> contactSet = collectMultiContact(contact);
            Logger.d(TAG, "saveChatMsg() send chat message to multi contact: " + contactSet);
            long threadId = Telephony.Threads.getOrCreateThreadId(AndroidFactory.getApplicationContext(), contactSet);
            for (String singleContact : contactSet) {
                int messageTag = generateMessageTag();
                sentMessageViaRCSe(message, singleContact, messageTag);
                PresentTextMessage ipMessage = new PresentTextMessage(messageTag, singleContact, message);
                mPreSentMessageList.put(PluginUtils.storeMessageInMmsDb(messageTag, message, singleContact,
                        PluginUtils.OUTBOX_MESSAGE, threadId), ipMessage);
            }
        }
        return SAVE_SUCCESS;
    }

    private void sentMessageViaRCSe(String message, String contact, int messageTag) {
        Logger.d(TAG, "sentMessageViaRCSe() message: " + message + " , contact: " + contact + ", messageTag: " + messageTag);
        Message controllerMessage = PluginController.obtainMessage(ChatController.EVENT_SEND_MESSAGE, PhoneUtils
                .formatNumberToInternational(contact), message);
        controllerMessage.arg1 = messageTag;
        controllerMessage.sendToTarget();
    }

    public void enableFileTransferAutoAccept(boolean autoAccept)
    {
    	// MMS will call this API to enable FT Auto paramter
        
    }
    
    public boolean isEnabledFileTransferAutoAccept()
    {
    	return false;
        
    }
    
    public long getWarningSizeForFileTransfer()
    {
    	return ApiManager.getInstance().getWarningSizeforFileThransfer();
    }
    

    private int saveFileTransferMsg(IpMessage msg, String contact) {
        IpAttachMessage ipAttachMessage = ((IpAttachMessage) msg);
        
        if(PluginUtils.getMessagingMode() == 0)
        {
        	if (contact.startsWith(IpMessageConsts.JOYN_START)) {
            contact = contact.substring(4);
        	}
        }
        String filePath = ipAttachMessage.getPath();
        int index = filePath.lastIndexOf("/");
        String fileName = filePath.substring(index + 1);
        String mimeType = MediaFile.getMimeTypeForFile(fileName);
        // If mms send true to compress image then we will compress
        if(mimeType.contains(Utils.FILE_TYPE_IMAGE) && RcsSettings.getInstance().isEnabledCompressingImageFromDB()/*&& (((IpImageMessage)msg).isNeedCompress)*/)
        		filePath = Utils.compressImage(filePath);
        
        // If mms send true to compress image then we will compress
        //filePath = Utils.compressImage(filePath);
        
        if (TextUtils.isEmpty(filePath)) {
            Logger.e(TAG, "saveFileTransferMsg() invalid filePath: " + filePath);
            return SAVE_FAIL;
        }
        File file = new File(filePath);
        if (!file.exists()) {
            Logger.e(TAG, "saveFileTransferMsg() file does not exist: " + filePath);
            return SAVE_FAIL;
        }
        long fileSize = file.length();
        long maxFileSize = ApiManager.getInstance().getMaxSizeforFileThransfer();
        if (fileSize >= maxFileSize && maxFileSize != 0) {
            Logger.d(TAG, "saveFileTransferMsg() file is too large, file size is " + fileSize);
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(AndroidFactory.getApplicationContext(),
                            PluginUtils.getStringInRcse(R.string.large_file_repick_message),
                            Toast.LENGTH_SHORT).show();
                }
            });
            return SAVE_FAIL;
        }
        //int index = filePath.lastIndexOf("/");
        //String fileName = filePath.substring(index + 1);
        String fileTransferString = PluginUtils.getStringInRcse(R.string.file_transfer_title);
        if (!contact.contains(COMMA)) {
            ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
            long fileTransferIdInMms = PluginUtils.insertDatabase(fileTransferString, contact, Integer.MAX_VALUE,
                    PluginUtils.OUTBOX_MESSAGE);

            FileStructForBinder fileStructForBinder = new FileStructForBinder(filePath, fileName,
                    ipAttachMessage.getSize(), uuid, null);
            IpMessage ipMessage = PluginUtils.analysisFileType(contact, fileStructForBinder);
            addMessage(fileTransferIdInMms, uuid.toString(), ipMessage);
            sentFileViaRCSe(filePath, contact, uuid);
        } else {
            Set<String> contactSet = collectMultiContact(contact);
            Logger.d(TAG, "saveFileTransferMsg() send file to multi contact: " + contactSet);
            long threadId = Telephony.Threads.getOrCreateThreadId(AndroidFactory.getApplicationContext(), contactSet);
            for (String singleContact : contactSet) {
                ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
                long fileTransferIdInMms = PluginUtils.insertDatabase(fileTransferString, singleContact, Integer.MAX_VALUE,
                        PluginUtils.OUTBOX_MESSAGE, threadId);
                FileStructForBinder fileStructForBinder = new FileStructForBinder(filePath, fileName, ipAttachMessage
                        .getSize(), uuid, null);
                IpMessage ipMessage = PluginUtils.analysisFileType(contact, fileStructForBinder);
                addMessage(fileTransferIdInMms, uuid.toString(), ipMessage);
                sentFileViaRCSe(filePath, singleContact, uuid);
            }
        }
        return SAVE_SUCCESS;
    }

	public boolean isEnabledCompressImage() {
		boolean isCompress = RcsSettings.getInstance()
				.isEnabledCompressingImageFromDB();
		Logger.d(TAG, "isEnabledCompressImage = " + isCompress);
		return isCompress;
	}

	public void setEnableCompressImage(boolean compressImage) {
		Logger.d(TAG, "isEnabledCompressImage = " + compressImage);
		RcsSettings.getInstance().setCompressingImage(compressImage);
	}

    private void sentFileViaRCSe(String filePath, String contact, ParcelUuid fileTransferTag) {
        Logger.d(TAG, "sentFileViaRCSe() filePath: " + filePath + " , contact: " + contact + " , fileTransferTag: "
                + fileTransferTag);
        Message controllerMessage = PluginController.obtainMessage(ChatController.EVENT_FILE_TRANSFER_INVITATION, contact,
                filePath);
        Bundle data = controllerMessage.getData();
        data.putParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG, fileTransferTag);
        controllerMessage.setData(data);
        controllerMessage.sendToTarget();
    }

    static Set<String> collectMultiContact(String contact) {
        String[] contacts = contact.split(COMMA);
        Set<String> contactSet = new TreeSet<String>();
        for (String singleContact : contacts) {
            contactSet.add(singleContact);
        }
        return contactSet;
    }

    @Override
    public void resendMessage(long msgId, int simId) {
        super.resendMessage(msgId, simId);
        resendMessage(msgId);
    }

    @Override
    public void resendMessage(long msgId) {
        Logger.d(TAG, "resendMessage() msgId + " + msgId);
        super.resendMessage(msgId);
        IpMessage msg = getIpMsgInfo(msgId);
		String messageTagInRcse = PluginUtils.findMessageTagInRcseDb(Long
				.toString(msgId));
		if (messageTagInRcse != null &&  !messageTagInRcse.equals("")) {
			if (msg instanceof IpAttachMessage)
				sentFileViaRCSe(((IpAttachMessage) msg).getPath(),
						((IpAttachMessage) msg).getTo(),
						ParcelUuid.fromString(messageTagInRcse));
		}

    }

    @Override
    public String getIpMessageStatusString(long msgId) {
        Logger.d(TAG, "getIpMessageStatusString(), msgId = " + msgId);
        IpMessage message = getMessage(msgId);
        long fileSize;
        if (message == null) {
            Logger.e(TAG, "getIpMessageStatusString(), message is null!");
            return null;
        }
        int messageType = message.getType();
        String remote = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                message.getFrom());
        int transferStatus;
        int rcsStatus;
        String fileName;
        // Get file transfer tag
        switch (messageType) {
            case IpMessageConsts.IpMessageType.PICTURE:
                fileName = ((PluginIpImageMessage) message).getName();
                fileSize = ((PluginIpImageMessage) message).getSize();    
                transferStatus = ((PluginIpImageMessage) message).getStatus();
                rcsStatus = ((PluginIpImageMessage) message).getRcsStatus();
                break;
            case IpMessageConsts.IpMessageType.VIDEO:
                fileName = ((PluginIpVideoMessage) message).getName();
                fileSize = ((PluginIpVideoMessage) message).getSize();    
                transferStatus = ((PluginIpVideoMessage) message).getStatus();
                rcsStatus = ((PluginIpVideoMessage) message).getRcsStatus();
                break;
            case IpMessageConsts.IpMessageType.VOICE:
                fileName = ((PluginIpVoiceMessage) message).getName();
                fileSize = ((PluginIpVoiceMessage) message).getSize();    
                transferStatus = ((PluginIpVoiceMessage) message).getStatus();
                rcsStatus = ((PluginIpVoiceMessage) message).getRcsStatus();
                if (transferStatus == IpMessageConsts.IpMessageStatus.MT_RECEIVED) {
                    if (((PluginIpVoiceMessage) message).getDuration() == 0) {
                        Logger.d(TAG, "getIpMessageStatusString(), need to get duration!");
                        ((PluginIpVoiceMessage) message).analysisAttribute();
                        Intent it = new Intent();
                        it.setAction(IpMessageConsts.IpMessageStatus.ACTION_MESSAGE_STATUS);
                        it.putExtra(IpMessageConsts.STATUS, transferStatus);
                        it.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID, msgId);
                        IpNotificationsManager.notify(it);
                    }
                }
                break;
            case IpMessageConsts.IpMessageType.VCARD:
                fileName = ((PluginIpVcardMessage) message).getName();
                fileSize = ((PluginIpVcardMessage) message).getSize();    
                transferStatus = ((PluginIpVcardMessage) message).getStatus();
                rcsStatus = ((PluginIpVcardMessage) message).getRcsStatus();
                break;
            default:
                fileName = ((PluginIpAttachMessage) message).getName();
                fileSize = ((PluginIpAttachMessage) message).getSize();    
                transferStatus = ((PluginIpAttachMessage) message).getStatus();
                rcsStatus = ((PluginIpAttachMessage) message).getRcsStatus();
                break;
        }

        switch (transferStatus) {            
            case IpMessageConsts.IpMessageStatus.MT_RECEIVING:
            case IpMessageConsts.IpMessageStatus.MT_RECEIVED:
            	return (fileSize + "KB") ;
            case IpMessageConsts.IpMessageStatus.MO_INVITE:
            case IpMessageConsts.IpMessageStatus.MO_SENDING:
            case IpMessageConsts.IpMessageStatus.MO_SENT:            
                return fileName;
            case IpMessageConsts.IpMessageStatus.MO_REJECTED:
                return remote + SPACE + mRejected + SPACE + fileName;
            case IpMessageConsts.IpMessageStatus.MO_CANCEL:
                if (!PluginUtils.isFtSupportedInRcse(message.getFrom())) {
                    Logger.d(TAG, "getIpMessageStatusString() mWarningNoFtCapability "
                            + mWarningNoFtCapability);
                    return mWarningNoFtCapability;
                } else {
                    Logger.d(TAG, "getIpMessageStatusString() support ft");
                    return mYou + SPACE + mCanceled + SPACE + "file sized " + fileSize + "KB";
                }
            case IpMessageConsts.IpMessageStatus.MT_CANCEL:
                if (rcsStatus == FAILED) {
                    return "File Transfer" + SPACE + mFailed;
                } else {
                    return remote + SPACE + mCanceled + SPACE + fileName;
                }
            case IpMessageConsts.IpMessageStatus.MT_INVITED:
                return (fileSize + "KB");
            case IpMessageConsts.IpMessageStatus.MT_REJECT:
                return mYou + SPACE + mRejected + SPACE + " file sized " + fileSize + "KB";
            default:
                return null;
        }
    }

    private static int generateMessageTag() {
        int messageTag = RANDOM.nextInt(MESSAGE_TAG_RANGE) + 1;
        messageTag = Integer.MAX_VALUE - messageTag;
        return messageTag;
    }

    /**
     * Use this class to be the cache when a message is in present status
     */
    private static class PresentTextMessage extends IpTextMessage {
        public int messageTag;
        private PresentTextMessage(int chatMessageTag, String contact, String text) {
            this.messageTag = chatMessageTag;
            this.setBody(text);
            this.setType(IpMessageConsts.IpMessageType.TEXT);
            super.setStatus(-1);
        }
    }
}


