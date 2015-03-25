/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.map;

import android.telephony.SmsManager;
import android.telephony.SmsManager.*;

import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.app.Activity;
import android.database.Cursor;
import android.net.Uri;
import android.content.Context;
import android.content.IntentFilter;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.database.ContentObserver;
import android.content.Intent;
import android.provider.Telephony.Sms;

import android.telephony.TelephonyManager;
import android.telephony.SmsMessage;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;
import com.mediatek.telephony.SmsManagerEx;
import com.android.internal.telephony.GeminiSmsMessage;

//import com.android.internal.telephony.gsm.SmsMessage;
//import com.android.internal.telephony.cdma.SmsMessage;

//import android.provider.telephony.Threads;
import android.content.ContentUris;
import android.os.Message;
import android.os.Handler;

import android.util.Log;

import android.app.Activity;
import android.app.PendingIntent;

import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.bluetooth.map.MAP;
import com.mediatek.bluetooth.map.SmsMessageEntity;
import com.mediatek.bluetooth.map.cache.*;
import com.mediatek.bluetooth.map.util.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Set;
import java.util.Iterator;
import java.util.Map;
import java.io.UnsupportedEncodingException;


import android.database.sqlite.SQLiteException;
import com.mediatek.xlog.Xlog;

class SmsController extends Controller implements MessageObserver.ControllerHelper{	
	private final String TAG				= "MAP-SmsController";
	
	private final int SMS_READ_STATUS 			= 1;
	private final int SMS_UNREAD_STATUS 		= 0;

	//for delete folder
	private final int MESSAGE_TYPE_DELETE		= 100;
	private final int INVALID_THREAD_ID			= -1;
	
	private static final String[] DEFAULT_PROJECTION = new String[] {
		Sms._ID,			
		Sms.SUBJECT,
		Sms.DATE,	
		Sms.ADDRESS,
		Sms.STATUS,
		Sms.READ,
		Sms.PERSON,
		Sms.BODY,
		Sms.THREAD_ID,
		Sms.TYPE,
		Mms.MESSAGE_SIZE // size attribute is the same to mms 
	};

	
	private static final int ID_COLUMN = 0;
    private static final int SUBJECT_COLUMN = 1;
    private static final int DATE_COLUMN = 2;
    private static final int ADDRESS_COLUMN = 3;
    private static final int STATUS_COLUMN = 4;
    private static final int READ_COLUMN = 5;
    private static final int PERSON_COLUMN = 6;
    private static final int BODY_COLUMN = 7;
	private static final int THREAD_ID_COLUMN = 8;
	private static final int TYPE_COLUMN = 9;
	private static final int SIZE_COLUMN = 10;

	/******server center*****/
	private static final String[] SERVICE_CENTER_PROJECTION = new String[] {
        Sms.Conversations.REPLY_PATH_PRESENT,
        Sms.Conversations.SERVICE_CENTER,
    };
	private static final int REPLY_PATH_PRESENT_COLUMN = 0;
	private static final int SERVICE_CENTER_COLUMN = 1;
	/*****end*****/

	private final String MESSAGE_STATUS_DELIVERED_ACTION = 
				"com.mediatek.bluetooth.map.SmsController.action.DELIVER_RESULT";
	private final String MESSAGE_STATUS_SENT_ACTION = 
				"com.mediatek.bluetooth.map.SmsController.action.SENT_RESULT";

	private final String EXTRA_FINAL_MESSAGE = 
				"com.mediatek.bluetooth.map.SmsController.extra.FINAL_MESSAGE";
	private final String EXTRA_MESSAGE_ID = 
				"com.mediatek.bluetooth.map.SmsController.extra.MESSAGE_ID";
/*
	private static final String[] CALLER_ID_PROJECTION = new String[] {
                Phone.NUMBER,                   // 0
                Phone.LABEL,                    // 1
                Phone.DISPLAY_NAME,             // 2
                Phone.CONTACT_ID,               // 3
                Phone.CONTACT_PRESENCE,         // 4
                Phone.CONTACT_STATUS           // 5
    };*/

    private static final int PHONE_NUMBER_COLUMN = 0;
    private static final int PHONE_LABEL_COLUMN = 1;
    private static final int CONTACT_NAME_COLUMN = 2;
    private static final int CONTACT_ID_COLUMN = 3;
    private static final int CONTACT_PRESENCE_COLUMN = 4;
    private static final int CONTACT_STATUS_COLUMN = 5;

	private static final int INVALID_VALUE_ID = -1;
	
	private Context 					mContext;
	private ContentResolver 				mContentResolver;
	private int 						mSimId;
	private Instance 					mInstance;
	private int							mPhoneType;

	private ArrayList<BMessageObject> 	mpendinglist;
	
	private final int 					mType;
	private int 						mMasId;

	private MessageObserver 			mMessageObserver;
	private BroadcastReceiver mReceiver = new BroadcastReceiver(){

		 @Override
        public void onReceive(Context context, Intent intent) {
        	String action = intent.getAction();
			int resultCode = getResultCode();
        	log("action: "+ action);
			if (action.equals(MESSAGE_STATUS_DELIVERED_ACTION)) {
				handleDeliverResult(intent, resultCode);
			} else if (action.equals(MESSAGE_STATUS_SENT_ACTION)){
				handleSentResult(intent, resultCode);
			}
		}
	};
	

	SmsController(Context context, Instance instance, int simId, int type) {
		mContext = context;
		mContentResolver = context.getContentResolver();
		mSimId = simId;
		mInstance = instance;
		mType = type;
	}

	public void onStart(){
		//add intent filter
		IntentFilter filter = new IntentFilter();
		filter.addAction(MESSAGE_STATUS_SENT_ACTION);
		filter.addAction(MESSAGE_STATUS_DELIVERED_ACTION);
		mContext.registerReceiver(mReceiver, filter);
	}
	public void onStop(){
		clearDeletedMessage();
		mContext.unregisterReceiver(mReceiver);
		deregisterListener();
	}

	public void registerListener(ControllerListener listener) {
		log("registerListener()");
		super.registerListener(listener);
		if (mMessageObserver == null){
			mMessageObserver = new MessageObserver(this, mListener, mType);
		}
		mContentResolver.registerContentObserver(Sms.CONTENT_URI, true, mMessageObserver);
		mContentResolver.registerContentObserver(Threads.CONTENT_URI, true, mMessageObserver);		
	}
	public void deregisterListener() {
		log("deregisterListener");
		super.deregisterListener();
		if (mMessageObserver != null){
			mContentResolver.unregisterContentObserver(mMessageObserver);
			mMessageObserver = null;
		}
	}
	
		public MessageListObject getMessageList(MessageListRequest req) {
		int listSize = req.getListSize();
		int offset = req.getListOffset();
		int mailbox = convertMailboxType(req.getFolder());
		String orignator = req.getOrignator();
		String recipient = req.getRecipient();
		String orignatortAddrList = null;
		String recipientAddrList = null;
		int maxSubjextLen = req.getMaxSubjectLen();
		int priority = req.getPriority();
		int index = 0;	
		Uri mailboxUri = Sms.CONTENT_URI;			
		StringBuilder selection = new StringBuilder();
		ArrayList<String> selectionArgs = new ArrayList<String>();

		String[] projection = DEFAULT_PROJECTION;		
	
		Cursor messageCursor;
		String from = null;
		String to = null;
		
		log("getMessageList(): listsize is "+listSize+",listoffset is"+offset
			+ ", maxSubjectLen is "+ maxSubjextLen + ",mailbox is "+mailbox
			+ ", orignator is "+orignator+",recipient is "+recipient);
		
		ContactsAdapter adapter = ContactsAdapter.getDefault(mContext);
		if (adapter != null && orignator != null && orignator.length() > 0) {
			orignatortAddrList = adapter.queryNumber(orignator);
		} else if (adapter != null && recipient != null && recipient.length() > 0) {
			recipientAddrList = adapter.queryNumber(recipient);
		}
		//SMS does not support high priority
		if (priority == MAP.PRIORITY_STATUS_HIGH) {
			return null;
		}	
				
		if (!convertFilterToSelection(req, selection,  selectionArgs)) {
			return null;
		}
		
		if (mailbox != MESSAGE_TYPE_DELETE){
			mailboxUri = getMailboxUri(mailbox);
			if (mailboxUri == null) {
				log("unrecognized mailbox uri");
				return null;
			}
		} 

		try{
			messageCursor = mContentResolver.query(mailboxUri,projection,
								selection.toString(),
								selectionArgs.toArray(new String[selectionArgs.size()]),
								Sms.DEFAULT_SORT_ORDER);				
		} catch (SQLiteException e) {
				e.printStackTrace();
				log("fail to query");
				return null;
		}

		if (messageCursor == null) {
			return null;
		}
		
		MessageListObject list = mInstance.getMsgListRspCache();
		

		//if maxListCount is euqals to zero, just ignore ListStartOffset, Subjectlength and parameterMask
		//notes: offset > 0 is neccessary, or else, move(offset) will return as false
		if (listSize > 0 && offset > 0 && !messageCursor.move(offset)) {	
			log("list size is zero or no cursor");
			return null;
		}

		log("mSimId:"+mSimId);
		//log("messageCursor:"+messageCursor.moveToFirst());

		boolean newMessageFlag = false;
		while(messageCursor.moveToNext() && (listSize  == 0 || list.getCurrentSize() < listSize)) {
			if(messageCursor.getInt(READ_COLUMN) == SMS_READ_STATUS) {
				newMessageFlag = true;
			}			
			
			String address = messageCursor.getString(ADDRESS_COLUMN);
			if (mailbox == Sms.MESSAGE_TYPE_INBOX) {
				from = address;
			} else {
				to = address;
			}
			
			//recipient filter: we focus on box except Inbox
			if (recipient != null && recipient.length() > 0 ) {	
				if (adapter != null && !adapter.doesPhoneNumberMatch(normalizeString(to), recipientAddrList, recipient)) {
					continue;
				}
			}
			
			//orignator filter
			if (orignator!= null && orignator.length() > 0 && (mailbox == Sms.MESSAGE_TYPE_INBOX)) {
				if (adapter != null && !adapter.doesPhoneNumberMatch(normalizeString(from), orignatortAddrList, orignator)) {
					continue;
				}
			}
				
			if (listSize > 0 ) {
				list.addMessageItem(composeMessageItem(messageCursor,mailbox));
			}

			index ++;
			list.addSize(1);				
		}

		messageCursor.close();

		if (newMessageFlag) {
			list.setNewMessage();
		}
		req.declineListOffset(index);
		return list;
	}

	public boolean pushMessage(BMessageObject obj) {
		String text = null;
		int size = (int)obj.getContentSize();
		byte[] content = new byte[size];		
		SmsMessage smsmessage = null;
		String recipient;
		String orignator;
		long messageId = -1;
		boolean isSave;
		int result;
		int read ;
		int charset = obj.getCharset();
		VCard vcard = new VCard();
		String folder = obj.getFolder();
		int mailbox ;
		long sim = NetworkUtil.getSimIdBySlotId(mContext,mSimId);

		log("pushMessage():"+size);
		
		if (sim < 0){
			log("no sim card in current sim slot");
			return false;
		}

		mailbox = (folder == null) ? -1:convertMailboxType(folder);
		if (mailbox == -1){
			log("invalid folder");
			return false;
		} 

		isSave = !obj.isTransparent();
		result = obj.getContent(content);
			
		if (charset == MAP.CHARSET_UTF8){
			text = new String(content);
			if (text != null) {
				text = text.trim();
			}
		} else if (charset == MAP.CHARSET_NATIVE) {
			/*concatenated sms message pdu*/
			int index = 0, offset = 0;
			int fragmentSize = 0;
			StringBuilder textBuilder = new StringBuilder();
			while ((fragmentSize = obj.getContentSize(index)) > 0) {
				byte[] fragment = new byte[fragmentSize];
				System.arraycopy(content, offset, fragment, 0, fragmentSize);
				offset += fragmentSize;
				index ++;

		try{
			if (content != null && result > 0) {
						smsmessage = SmsMessage.createFromPdu(fragment);
				if (smsmessage != null){
							textBuilder.append(smsmessage.getMessageBody());
				}
			} else {
				log("fail to send SMS message, the content is null");
				return false;
			}
		}catch(Exception e){
			log(e.toString());
			return false;
		}
			}
			text = textBuilder.toString();
		}else {
			log("unknown charset:"+charset);
			return false;
		}

		vcard.parse(obj.getFinalRecipient());	
		recipient = normalizeString(vcard.getTelephone());
		vcard.reset();
		vcard.parse(obj.getOrignator());
		orignator = normalizeString(vcard.getTelephone());
		
		
		//save message 
		if (isSave) {
			ContentValues cv = new ContentValues();
			
			cv.put(Sms.TYPE, mailbox);
//				(mailbox == Sms.MESSAGE_TYPE_OUTBOX) ? Sms.MESSAGE_TYPE_SENT:mailbox);
			//size fileld is not supported in LG database, so remove it
	//		cv.put(Mms.MESSAGE_SIZE, size);
			cv.put(Sms.DATE,System.currentTimeMillis());
			if (mailbox == Sms.MESSAGE_TYPE_INBOX){
				cv.put(Sms.ADDRESS,orignator);
			} else {
				cv.put(Sms.ADDRESS,recipient);
			}

			read = convertReadStatus(obj.getReadStatus());
			if (read != -1) {
				cv.put(Sms.READ,read);
			}
			cv.put(Sms.BODY,text);
			cv.put(Sms.STATUS,Sms.STATUS_PENDING);
			
			cv.put(Sms.SEEN, 0);
			if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
				cv.put(Sms.SIM_ID, sim);
			}
			if (smsmessage != null){
			cv.put(Sms.PROTOCOL, smsmessage.getProtocolIdentifier());
			if (smsmessage.getPseudoSubject().length() > 0) {
				cv.put(Sms.SUBJECT, smsmessage.getPseudoSubject());
			}
			cv.put(Sms.REPLY_PATH_PRESENT, smsmessage.isReplyPathPresent() ? 1 : 0);
			cv.put(Sms.SERVICE_CENTER, smsmessage.getServiceCenterAddress());
			}			
					
			Uri uri = mContentResolver.insert(Sms.CONTENT_URI,cv);
			if (uri != null) {
				Cursor cs = mContentResolver.query(uri, new String[]{Sms._ID},null,null,null);
				if (cs != null && cs.moveToFirst()) {
					messageId = cs.getLong(0);	
					cs.close();
				}
			}
		} else {
			//if we donot save the message, we have to assign a unique handle for the message
			messageId = INVALID_VALUE_ID;
		}

		if (messageId == INVALID_VALUE_ID)
		{
			obj.setHandle(HandleUtil.getHandle(mType,0));
		}
		else
		{			
		obj.setHandle(HandleUtil.getHandle(mType,messageId));
		}
		
		if (mailbox == Sms.MESSAGE_TYPE_OUTBOX && recipient != null){
			//send message:
			SmsManager manager = SmsManager.getDefault();		
        	ArrayList<String> messages = null;
			if (text == null){
				return false;
			}
			messages = manager.divideMessage(text);

			ArrayList<PendingIntent> deliveryIntents =  new ArrayList<PendingIntent>(messages.size());
        	ArrayList<PendingIntent> sentIntents = new ArrayList<PendingIntent>(messages.size());
        	for (int i = 0; i < messages.size(); i++) {
				//deliver intent and sendIntent
				Intent deliveryIntent  = new Intent(MESSAGE_STATUS_DELIVERED_ACTION); 
             	Intent sendIntent  = new Intent(MESSAGE_STATUS_SENT_ACTION);
				sendIntent.putExtra(EXTRA_MESSAGE_ID, messageId);
            	deliveryIntent.putExtra(EXTRA_MESSAGE_ID, messageId);
				if (i == messages.size() -1) {
                	sendIntent.putExtra(EXTRA_FINAL_MESSAGE, true);
                	deliveryIntent.putExtra(EXTRA_FINAL_MESSAGE, true);
            	}

				/*Notes: The flag for the pending intent can not be set as 0,*/
				/*		or else intent received will keep the same with the first pending intent.*/
				/*		Another replacment way is to call cancal@PendingIntent */
				deliveryIntents.add(PendingIntent.getBroadcast(
                        mContext, 0,
                        deliveryIntent,
                        PendingIntent.FLAG_CANCEL_CURRENT));
            	sentIntents.add(PendingIntent.getBroadcast(
                    	mContext, i, 
                    	sendIntent, PendingIntent.FLAG_CANCEL_CURRENT));
        	}
		

			if(FeatureOption.MTK_GEMINI_SUPPORT != true) {
				manager.sendMultipartTextMessage(recipient, null, messages, sentIntents, deliveryIntents);
			} else {
				SmsManagerEx managerEx = SmsManagerEx.getDefault();
				managerEx.sendMultipartTextMessage(recipient, null, messages, sentIntents, deliveryIntents, mSimId);
			}
		}
	
		return true;
		
	}
	

	/*notes: for SMS deliver PDU(that is, messages in inbox), encoding is not used*/
	/*but the charset is UTF-8*/
	/*so we have to confirm the charset of text in provider */
	public  BMessageObject getMessage(MessageRequest req) {			
		log("getMessage()");
		boolean attachment = req.isAttachDelivered();
		long id = req.getMessageId();
		int charset = req.getCharset();
		Uri uri =ContentUris.withAppendedId(Sms.CONTENT_URI, id);
		
		Cursor messageCursor =mContentResolver.query(uri, DEFAULT_PROJECTION,
							null,
							null,
							null);

		if(messageCursor == null || !messageCursor.moveToFirst()) {
			log("find no record for the request : id is "+ id);
			return null;
		}

		String text = messageCursor.getString(BODY_COLUMN);
		String address = messageCursor.getString(ADDRESS_COLUMN);
	
		int mailbox = messageCursor.getInt(TYPE_COLUMN);

		String orignator = new String();
		String recipient = new String();
		int index = 0;

		if (mailbox == Sms.MESSAGE_TYPE_INBOX){
			orignator = address;
			normalizeString(orignator);
			recipient = NetworkUtil.getPhoneNumber(mSimId);
			normalizeString(recipient);
		} else {
			recipient = address;
			orignator = NetworkUtil.getPhoneNumber(mSimId);
			normalizeString(recipient);
			normalizeString(orignator);
		}
		
		
		
		BMessageObject bMessage = mInstance.getBMessageObject();
		bMessage.reset();

		// orignator
		VCard vCard = new VCard();
		vCard.setTelephone(orignator);
		
		bMessage.setOrignator(vCard.toString());
		
		vCard.reset();
		vCard.setTelephone(recipient);
		bMessage.addRecipient(vCard.toString());
		
		bMessage.setReadStatus(revertReadStatus(messageCursor.getInt(READ_COLUMN)));

		//charset
		bMessage.setCharset(charset);
		//ignore the encode field
		bMessage.setEncoding(-1);
		//ignore language
		bMessage.setLang(-1);
		bMessage.setMessageType(mType);

		
		if (charset == MAP.CHARSET_UTF8) {
			/*default charset is UTF-8 in android platform*/
			if (text != null) {
				bMessage.addContent(text.getBytes());
			}
		} else if (charset == MAP.CHARSET_NATIVE){
			SmsManager manager = SmsManager.getDefault();		
        	ArrayList<String> messages = null;
			messages = manager.divideMessage(text);	
			while (messages != null && index < messages.size()) {	
				String content = messages.get(index);
				byte[] bytes;
				if (mailbox == Sms.MESSAGE_TYPE_INBOX) {
					bytes = SmsMessageEntity.getDefault().getDeliverPdu(orignator,
					recipient, content, mSimId);
				} else {
					bytes = SmsMessageEntity.getDefault().getSubmitPdu(orignator,
						recipient, content,mSimId);
				}
			
			
				bMessage.addContent(bytes);				
				index ++;			
			}
		} else {
			log("unknown charset type:"+charset);
			messageCursor.close();
			return null;
		}		

		messageCursor.close();
		return bMessage;
			
	}

	public	boolean deleteMessage(long id) {
		log("deleteMessage():id is "+ id);
		boolean flag;
		Uri uri =ContentUris.withAppendedId(Sms.CONTENT_URI, id);
		String[] projection = new String[]{Sms.THREAD_ID};

		Cursor cs =  mContentResolver.query(uri, projection, null, null, null);
		if(cs != null && cs.moveToFirst()) {
			int thread_id = cs.getInt(0);
			if (thread_id == INVALID_VALUE_ID) {
				mContentResolver.delete(uri,null,null);
				mDeleteFolder.remove(Long.valueOf(id));
			} else {
				ContentValues cv = new ContentValues();
				cv.put(Sms.THREAD_ID, Integer.valueOf(INVALID_THREAD_ID));
				mContentResolver.update(uri, cv, null, null);
				mDeleteFolder.put(Long.valueOf(id), Integer.valueOf(thread_id));
				log("succeed");
			}
			flag = true;
			cs.close();
		} else {
			log("the message does not exist in SMS provider");
			flag = false;
		}
		return flag;
		
			
	}

	public boolean restoreMessage(long id){
		log("deleteMessage():id is "+ id);
		boolean flag;
		Uri uri =ContentUris.withAppendedId(Sms.CONTENT_URI, id);
		String[] projection = new String[]{Sms.THREAD_ID};

		Cursor cs =  mContentResolver.query(uri, projection, null, null, null);
		if(cs != null && cs.moveToFirst()) {
			int thread_id = cs.getInt(0);
			if (thread_id == INVALID_VALUE_ID) {
				Integer orignalThreadId = mDeleteFolder.remove(Long.valueOf(id));
				if (orignalThreadId != null) {
					ContentValues cv = new ContentValues();
					cv.put(Sms.THREAD_ID, Integer.valueOf(orignalThreadId));
					mContentResolver.update(uri, cv, null, null);
				} else {
					log("no record in delete folder");						
				}					
			}
			flag = true;
			cs.close();
		} else {
			log("the message does not exist in MMS provider");
			flag = false;
		}
			return flag;
	}
	//due to the delete folder is just valid for MAP in SMS, so when deivce is disconnected,
	//the messages in deleted folder have to be deleted.
	public void clearDeletedMessage(){
		log("clearDeletedMessage()");
		Uri uri;
		Long id;
		String[] projection = new String[]{Sms.THREAD_ID};
		int mailbox;

		uri = Sms.CONTENT_URI;
		
		Iterator iterator= mDeleteFolder.entrySet().iterator();
		while (iterator.hasNext()) {
				Map.Entry entry = (Map.Entry)iterator.next();
				id = (Long)entry.getKey();	
				uri =ContentUris.withAppendedId(Sms.CONTENT_URI, id);

				//we have to confirm the message is truely in deleted folder
				Cursor cs = mContentResolver.query(uri,projection,null,null,null);
				if (cs != null && cs.moveToFirst()) {
					mailbox = cs.getInt(0);
					if (mailbox == INVALID_THREAD_ID) {
						//maybe IllegalArgumentException will be thrown when delete message from ICC
						try {
							mContentResolver.delete(uri,null,null);
						} catch (IllegalArgumentException e) {
						}
					}
				}
				if (cs != null){
					cs.close();
				}
		}
		mDeleteFolder.clear();		
	}
	

	public boolean setMessageStatus(long id, int state) {
		log("setMessageStatus():id is "+ id + ", state is "+state);
		Uri uri =ContentUris.withAppendedId(Sms.CONTENT_URI, id);
		String[] projection = new String[]{
			Sms.READ};
		int newState = convertReadStatus(state);

		if (newState == -1) {
			log("the status to be set is invalid");
			return false;
		}
		Cursor cs =  mContentResolver.query(uri, null, null, null, null);
		if(cs != null && cs.moveToFirst()) {
			
			if (cs.getInt(0) == newState) {
				log("state is same, no need to update");
			} else {
				ContentValues cv = new ContentValues();
				cv.put(Sms.READ, Integer.valueOf(newState));
				mContentResolver.update(uri, cv, null, null);
			}
			cs.close();
		}
		return true;
		
	}
	
	public boolean updateInbox(){
		return true;
	}

	//SMS -> map
	private	int revertReadStatus(int smsReadStatus){
		switch(smsReadStatus) {
			case SMS_UNREAD_STATUS:
				return MAP.UNREAD_STATUS;
			case SMS_READ_STATUS:
				return MAP.READ_STATUS;
			default:
				log("error: the read status from sms provider is invalid:"+smsReadStatus);
				return -1;
		}
	}

	private MessageItem composeMessageItem(Cursor cs, int mailbox){
		MessageItem msg = new MessageItem();
		int size = 0;;
		int attachSize;
		int recipientStatus;	
		boolean isText;
		int readStatus;

		log("composeMessageItem()");
		
		recipientStatus = revertLoadStatus(cs.getInt(STATUS_COLUMN));
		//if the message has been deleted, return directly
		if (recipientStatus == -1) {
			return null; 
		}

		readStatus = revertReadStatus(cs.getInt(READ_COLUMN));
		//if the message has been deleted, return directly
		if (readStatus == -1) {
			return null;
		}

		isText = true;
		if (cs.getString(BODY_COLUMN) != null) {
			isText = true;
		} else {
			isText = false;
		}

		String orignator = null;
		String recipient = null;
		String address = cs.getString(ADDRESS_COLUMN);
			
		//recipient filter: we focus on box except Inbox
		if (mailbox == Sms.MESSAGE_TYPE_INBOX ) {				
			orignator = address; 
		} else {
			recipient = address;
		}	
		String subject = cs.getString(SUBJECT_COLUMN);
		if (subject == null) {
			subject = cs.getString(BODY_COLUMN);
		}
		msg.setHandle(HandleUtil.getHandle(mType,cs.getLong(ID_COLUMN)));
		msg.setSubject(subject);
		msg.setDatetime(cs.getLong(DATE_COLUMN));
		msg.setSenderAddr(orignator);
		msg.setSenderName(null);
		msg.setReplyAddr(null);
		msg.setRecipientName(null);
		msg.setRecipientAddr(recipient);
		msg.setMsgType(mType);
		msg.setSize(cs.getInt(SIZE_COLUMN));
		msg.setText(isText);
		msg.setRecipientStatus(recipientStatus);
		msg.setAttachSize(0);
		msg.setReadStatus(readStatus);
		msg.setProtected(false);
		msg.setPriority(false);	
		return msg;
	}

	private void handleDeliverResult(Intent intent, int resultCode){
		//	Uri uri = intent.getData();		
            byte[] pdu = (byte[]) intent.getExtra("pdu");
			long id = intent.getLongExtra(EXTRA_MESSAGE_ID, INVALID_VALUE_ID);
			Uri uri = ContentUris.withAppendedId(Sms.CONTENT_URI, id);
			String[] projection = new String[]{Sms._ID};

			log("handleDeliverResult: id is " + id + " pdu is empty? "+ (pdu == null) + "result is " + resultCode);

			if (pdu == null || resultCode != Activity.RESULT_OK){
				return;
			}
			SmsMessage message = SmsMessage.createFromPdu(pdu);
			if (message == null) {
				return;
			}
			Cursor cs = mContentResolver.query(uri, projection, null ,null,null);
			if (cs != null && cs.moveToFirst()) {
				ContentValues cv = new ContentValues();
				cv.put(Sms.STATUS, message.getStatus());
				mContentResolver.update(uri, cv, null, null);	
				log("update status");
			}
			if (mListener != null && id != INVALID_VALUE_ID){
				if (0 == message.getStatus()) {
					mListener.onMessageDelivered(id, mType, MAP.RESULT_OK);
				} else {
					mListener.onMessageDelivered(id, mType, MAP.RESULT_ERROR);
				}
			}
			if (cs != null){
				cs.close();
			}
	}
	private void handleSentResult(Intent intent, int resultCode){
			int ret = MAP.RESULT_ERROR;
			int error = intent.getIntExtra("errorCode", 0);
			boolean isfinal = intent.getBooleanExtra(EXTRA_FINAL_MESSAGE, false);
			long id = intent.getLongExtra(EXTRA_MESSAGE_ID, -1);			
			String[] projection = new String[]{Sms.TYPE};

			log("handleSentResult:result is "+resultCode+", error is "+error+", id is "+id);
			
			Uri uri = ContentUris.withAppendedId(Sms.CONTENT_URI, id);
			Cursor cs = mContentResolver.query(uri, projection, null ,null,null);
			if (cs == null) {
				return;
			} else if (!cs.moveToFirst()) {
				cs.close();
				return;
			} 			
				
			if(resultCode == Activity.RESULT_OK) {
				ret = MAP.RESULT_OK;
				int mailbox = cs.getInt(0);
				if (mailbox == Sms.MESSAGE_TYPE_OUTBOX){
					Sms.moveMessageToFolder(mContext, uri, Sms.MESSAGE_TYPE_SENT, error);
				} else {
					log("the message is not in outbox:"+mailbox);
				}		
			} else {
				Sms.moveMessageToFolder(mContext, uri, Sms.MESSAGE_TYPE_FAILED, error);
			}
			cs.close();
			if (mListener != null) {
				mListener.onMessageSent(id, mType, ret);
			}
	}	
	public void queryMessage(HashMap<Long, Integer> info){
			Cursor messageCursor =mContentResolver.query(Sms.CONTENT_URI, 
									new String[]{Sms._ID,
												Sms.TYPE},
									null,	
									null, 
									null);
			if (messageCursor == null) {
				return;
			}
			while(messageCursor.moveToNext()){
				info.put(messageCursor.getLong(0), messageCursor.getInt(1));				
			}
			messageCursor.close();
	}

	//SMS -> map
	private int revertLoadStatus(int SmsStatus) {
		return MAP.RECEPIENT_STATUS_COMPLETE;
	
	}
	//map -> SMS
	private int convertReadStatus(int mapReadStatus) {
		switch(mapReadStatus) {
			case MAP.UNREAD_STATUS:
				return SMS_UNREAD_STATUS;
			case MAP.READ_STATUS:
				return SMS_READ_STATUS;
			default:
				log("other map state: "+mapReadStatus);
				return -1;
		}
	}

	//map -> SMS
	private String [] convertMaskToProjection(int mask) {	
		return DEFAULT_PROJECTION;
	}

		//MAP -> SMS
	private boolean convertFilterToSelection(MessageListRequest req, 
													StringBuilder selection,
													ArrayList<String> selectionArgs ){
		long startTimeMillis = req.getStartTime();
		long endTimeMillis = req.getEndTime();
		int readStatus = convertReadStatus(req.getReadStatus());
		int mailbox = convertMailboxType(req.getFolder());

		//sim id
		/*Notes: if a message is received in one SIM card, and the card is removed later, the message will missing SLTO info*/
		/*		all message missing SLOT info will be exposed to client in instance 0*/
		if (NetworkUtil.SIM1 == mSimId) {
			int index;
			for (index = NetworkUtil.SIM2 ; index < NetworkUtil.getTotalSlotCount(); index ++) {
				long simid = NetworkUtil.getSimIdBySlotId(mContext, index);
				if (simid > 0)
				{
					if (selection.length() > 0) {
						selection.append(" AND ");
					}
					selection.append(Sms.SIM_ID+"<>?");
					selectionArgs.add(Long.toString(simid));
				}
			}
		} else {			
			long simid = NetworkUtil.getSimIdBySlotId(mContext, mSimId);
			if (simid > 0) {
				if (selection.length() > 0) {
					selection.append(" AND ");
				}
				selection.append(Sms.SIM_ID+"=?");
				selectionArgs.add(Long.toString(simid));
			} else {
				//no message belongs to the SIM card slot, so just return
				log("Err, No message is in SIM slot "+ mSimId);
				return false;
			}
		}
		
		//mail box
		if (selection.length() > 0) {
			selection.append(" AND ");
		}
		if (mailbox != MESSAGE_TYPE_DELETE){
			selection.append(Sms.THREAD_ID+"<>?");
		} else {
			selection.append(Sms.THREAD_ID+"=?");
		}
		selectionArgs.add(Integer.toString(INVALID_THREAD_ID));
	
		//date
		if (startTimeMillis > 0) {
			if (selection.length() > 0) {
					selection.append(" AND ");
				}
			selection.append(Sms.DATE);	
			selection.append(" >=?");	
			selectionArgs.add(Long.toString(startTimeMillis));
		} else if (endTimeMillis > 0) {
			if (selection.length() > 0) {
					selection.append(" AND ");
			}
			selection.append(Sms.DATE);	
			selection.append(" <=? ");	
			selectionArgs.add(Long.toString(endTimeMillis));
		}

		//read status
		if (readStatus != -1) {
			if (selection.length() > 0) {
					selection.append(" AND ");
			}
			selection.append(Sms.READ + "=?");	
			selectionArgs.add(Integer.toString(readStatus));	
		}
		return true;
	}

	//MAP -> SMS
	private int convertMailboxType(String mapMailboxType) {
			if (mapMailboxType == null) {
				return -1;
			}
			if (mapMailboxType.equals(MAP.Mailbox.INBOX)) {
				return Sms.MESSAGE_TYPE_INBOX;
			} else if (mapMailboxType.equals(MAP.Mailbox.OUTBOX)) {
				return Sms.MESSAGE_TYPE_OUTBOX;
			} else if (mapMailboxType.equals(MAP.Mailbox.SENT)) {
				return Sms.MESSAGE_TYPE_SENT;
			} else if (mapMailboxType.equals(MAP.Mailbox.DRAFT)) {
				return Sms.MESSAGE_TYPE_DRAFT;
			} else if(mapMailboxType.equals(MAP.Mailbox.DELETED)) {
				return MESSAGE_TYPE_DELETE;
			}
			return -1;
		}
	//TODO:mailbox.x defines as String
	public String revertMailboxType(int smsMailboxType) {
		int type;
		switch(smsMailboxType) {
			case Sms.MESSAGE_TYPE_INBOX:
				return MAP.Mailbox.INBOX;
			case Sms.MESSAGE_TYPE_OUTBOX:
				return MAP.Mailbox.OUTBOX;
			case Sms.MESSAGE_TYPE_SENT:
				return MAP.Mailbox.SENT;
			case Sms.MESSAGE_TYPE_DRAFT:
				return MAP.Mailbox.DRAFT;
			case MESSAGE_TYPE_DELETE:
				return MAP.Mailbox.DELETED;
		}		 
		return null;
	}

	private Uri getMailboxUri(int mailbox) {
		//ignore other type
		switch(mailbox) {
			case Sms.MESSAGE_TYPE_INBOX:
				return Sms.Inbox.CONTENT_URI;
			case Sms.MESSAGE_TYPE_OUTBOX:
				return Sms.Outbox.CONTENT_URI;
			case Sms.MESSAGE_TYPE_SENT:
				return Sms.Sent.CONTENT_URI;
			case Sms.MESSAGE_TYPE_DRAFT:
				return Sms.Draft.CONTENT_URI;
			default:
				return null;
		}
	}	

	private String normalizeString (String text){	
		if(text == null || text.length() == 0){
			return null;
		}
        text = text.replaceAll(" ", "");
        text = text.replaceAll("-", "");
		return text;
	}
	private void log(String info){
		if (info == null) {
			return;
		}
		Xlog.v(TAG,info);
	}
}
	

