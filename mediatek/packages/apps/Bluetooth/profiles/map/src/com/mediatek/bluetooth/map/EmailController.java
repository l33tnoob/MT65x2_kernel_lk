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
import com.mediatek.bluetooth.map.BluetoothMapServerService;
import com.mediatek.bluetooth.map.Email.*;
import com.mediatek.bluetooth.map.MAP.*;
import com.mediatek.bluetooth.map.cache.*;
import com.mediatek.bluetooth.map.util.*;
import com.mediatek.bluetooth.map.mime.*;
import android.os.Message;
import android.os.Handler;
import android.database.Cursor;
import android.net.Uri;
import android.content.BroadcastReceiver;

import android.content.Context;
import android.content.ContentUris;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.net.Uri;

import android.database.ContentObserver;
import android.database.sqlite.SQLiteException;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.Iterator;
import java.util.Map;

import java.io.File;
import java.io.IOException;

import java.util.ArrayList;

import android.util.Log;
import com.mediatek.xlog.Xlog;

//when account is changed, we should check the ID and confirm whether the account we foucus on is modified.
//if it is, case is bad, we have to process it, or else, just ignore

class EmailController extends Controller implements MessageObserver.ControllerHelper{
	private final String TAG = "EmailController";
	//intent to send 
	private final String ACTION_SEND = "com.android.email.action.DIRECT_SEND";
	private final String ACTION_SEND_RESULT = "com.android.email.action.SEND_RESULT";
	private final String ACTION_DELIVER_RESULT = "com.android.email.action.DELIVER_RESULT";
	private final String ACTION_UPDATE_INBOX = "com.android.email.action.UPDATE_INBOX";
	private final String ACTION_UPDATE_INBOX_RESULT = "com.android.email.action.UPDATE_INBOX_RESULT";
	
	private final String EXTRA_ACCOUNT = "com.android.email.extra.ACCOUNT";
	private final String EXTRA_STREAM = "com.android.email.extra.STREAM";
	private final String EXTRA_FLAG = "com.android.email.extra.FLAG";
	private final String EXTRA_MAILBOX_TYPE = "com.android.email.extra.MAILBOX_TYPE";
	private final String EXTRA_RESULT = "com.android.email.extra.RESULT";
	private final String EXTRA_ACCOUNT_ARRAY = "com.android.email.extra.ACCOUNT_ARRAY";
	private final String EXTRA_NEED_RESULT = "com.android.email.extra.NEED_RESULT";
	

	//send flag definition 
	private final int SAVE_TO_INBOX = 0;
	private final int NO_SAVE_INBOX = 1;

	//send or deliver result
	private final int SUCCESS = 0;
	private final int FAILURE = 1;
	
	//timeout to wait send/updateinbox response: 30s(temp)
	private final int DELAYED_TIMEOUT = 30000;

	//handler event
	private final int MESSAGE_SEND_TIMEOUT 			= 0;
	private final int MESSAGE_UPDATE_INBOX_TIMEOUT 	= 1;
	private final int CONTROLLER_NOT_AVAILABLE		= 2;

	/*   match with the definition in EmailContent*/
	private static final String RECORD_ID = "_id";
	private static final String IS_DEFAULT = "isDefault";
	private static final String EMAIL_ADDRESS = "emailAddress";

	private static final String[] DEFAULT_ACCOUNT_ID_PROJECTION = new String[] {
            RECORD_ID, EMAIL_ADDRESS,IS_DEFAULT
    };

	//use for the case that MaxListSize is zero in message list request
	private static final String[] READ_STATUS_PROJECTION = new String[] {
            Email.MessageColumns.ID,Email.MessageColumns.TIMESTAMP,
				Email.MessageColumns.MESSAGE_ID,Email.MessageColumns.FLAG_READ
    };

	//size is required,but it seems not provided by EMAIL
	//size and attachment size are needed
	private static final String[] DEFAULT_MESSAGE_LIST_PROJECTION = new String[] {
		Email.MessageColumns.ID,			
		Email.MessageColumns.SUBJECT,
		Email.MessageColumns.TIMESTAMP,		
		Email.MessageColumns.FROM_LIST,
		Email.MessageColumns.REPLY_TO_LIST,		
		Email.MessageColumns.TO_LIST,
		Email.MessageColumns.FLAG_ATTACHMENT,
		Email.MessageColumns.FLAG_LOADED,
		Email.MessageColumns.FLAG_READ,
		Email.MessageColumns.MESSAGE_ID,
		Email.MessageColumns.ACCOUNT_KEY,
		Email.MessageColumns.MAILBOX_KEY,
		Email.MessageColumns.BCC_LIST,
		Email.MessageColumns.CC_LIST,
		Email.MessageColumns.SIZE  //
	};

	private final int COLUMN_ID = 0;
	private final int COLUMN_SUBJECT		= 1;
	private final int COLUMN_TIMESTATP		= 2;
	private final int COLUMN_FROM			= 3;
	private final int COLUMN_REPLAY			= 4;
	private final int COLUMN_TO				= 5;
	private final int COLUMN_ATTACHMENT		= 6;
	private final int COLUMN_LOADED			= 7;
	private final int COLUMN_READ			= 8;
	private final int COLUMN_MESSAGE_ID		= 9;
	private final int COLUMN_ACCOUT			= 10;
	private final int COLUMN_MAILBOX		= 11;
	private final int COLUMN_BCC			= 12;
	private final int COLUMN_CC				= 13;
	private final int COLUMN_SIZE			= 14;


	//mapping between mailbox type and mailbox id in a specific account
	
	public static final String CONTENT_BYTES = "content_bytes";

	private final int PROJECTION_COLUMM_TIMESTAMP = 0;
	

	private long mAccountId;
	private String mEmailAddress;
	private int mMasId;
	private Instance mInstance;

	private int mMsgType = MAP.MSG_TYPE_EMAIL;	
	private Context mContext;
	private ContentResolver mResolver;
//	private ControllerListener mListener;
	
	
	private MessageListObject mMessageListObject;

	
	private AccountObserver	mAccountObserver;
	private	MessageObserver mMessageObserver;

	//process the timeout event
	private final Handler mHandler = new Handler() {
		 @Override
		 public void handleMessage(Message msg) {
			log ("receive message:"+msg.what);
			switch(msg.what) {
				case MESSAGE_SEND_TIMEOUT:
				//	handleSendResult(long id,int result);
					break;
				case MESSAGE_UPDATE_INBOX_TIMEOUT:
				//	notifyChangeEvent();
					break;
				case CONTROLLER_NOT_AVAILABLE:
				//	disableController();
					//todo: disable controller
					
					break;
				default:
					log("unexpected message is received");
			}
		 }
	}; 

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("action:"+action);           
            if (action.equals(ACTION_SEND_RESULT)) {
                long account = intent.getLongExtra(EXTRA_ACCOUNT,-1);
				int result = intent.getIntExtra(EXTRA_RESULT, -1);
				if (account == mAccountId) {
					//TODO: EMAIL APP has to return the id
					handleSendResult(0, result);
				}                
			} else if (action.equals(ACTION_DELIVER_RESULT)) {
				long account = intent.getLongExtra(EXTRA_ACCOUNT,-1);
				int result = intent.getIntExtra(EXTRA_RESULT,-1);
				if (account == mAccountId) {
					//TODO: EMAIL APP has to return the id
					handleDeliverResult(0, result);
				} 
			} else if (action.equals(ACTION_UPDATE_INBOX_RESULT)) {
				long account = intent.getLongExtra(EXTRA_ACCOUNT,-1);
				int result = intent.getIntExtra(EXTRA_RESULT, -1);
				if (account == mAccountId) {
			//		handleUpdateInboxResult(result);
				} 
			} else {
				log("unexpected intent is received");
			}
           
        }
    };


	class AccountObserver extends ContentObserver{
		public AccountObserver(Handler handler) {
			super(handler);
		}
		 @Override
		public void onChange(boolean onSelf) {
			super.onChange(onSelf);
			log("AccountObserver:onChange");
			if (!queryAccount(mAccountId)){
				mAccountId = -1;
			}		
		}
	
	}
	
	public void handleDeliverResult(long id, int result) {
		mHandler.removeMessages(MESSAGE_SEND_TIMEOUT);
		mListener.onMessageDelivered(id, mMsgType, result);
	}

	public void handleSendResult(long id, int result) {
		mHandler.removeMessages(MESSAGE_SEND_TIMEOUT);
		mListener.onMessageSent(id, mMsgType, result);
	}



	EmailController(Context context, Instance instance, long accountId) {
		mContext = context;
		mMasId = instance.getInstanceId();
		mInstance = instance;
		mResolver = mContext.getContentResolver();

		//Notes: Avoid to query database in main thread
	//	mAccountId = (accountId < 0) ? queryDefaultAccount() : accountId;
		mAccountId = accountId;
		mMessageListObject = new MessageListObject();
		onStart();
	}

	public void onStart(){
	}
	public void onStop(){
		//just confirm listener has been deregistered
		deregisterListener();
	}

	public void registerListener(ControllerListener listener) {		
		super.registerListener(listener);
		if(mAccountObserver == null ) {
			mAccountObserver = new AccountObserver(new Handler());
		}
		if (mMessageObserver == null){
			mMessageObserver = new MessageObserver(this, mListener, mMsgType);			
		}

		mResolver.registerContentObserver(Email.CONTENT_URI, true, mAccountObserver);
		mResolver.registerContentObserver(Email.MESSAGE_URI, true, mMessageObserver);	
	}

	public void deregisterListener() {
		super.deregisterListener();
		if(mAccountObserver != null ) {
			mResolver.unregisterContentObserver(mAccountObserver);
			mAccountObserver = null;
		}
		if (mMessageObserver != null){
			mResolver.unregisterContentObserver(mMessageObserver);
			mMessageObserver = null;			
		}

	
	}

	//the API must be cautious to use
	public boolean setAccount(long account) {
		HashSet<Long> accoutset = new HashSet<Long>();
		log("setAccount():"+account);
	/*	if((account != -1) && queryAccount(account)) {
			mAccountId = account;
			log("new account is " + account);
			return true;
		} else {
			log("invalid account:" + account);
			return false;
		}
		*/
		mAccountId = account;
		return true;
	}

	public long getAccount(){
		return mAccountId;
	}

	public boolean queryAccount(long id) {
		Cursor accountCursor = null;
		
		try {
			accountCursor = mResolver.query(Email.ACCOUNT_URI, 
									new String[]{Email.AccountColumns.ID},
									Email.AccountColumns.ID+"=?",	
									new String[]{Long.toString(id)}, 
									null);
		} catch (SecurityException e) {
				e.printStackTrace();
				return false;	
		}
			if (accountCursor == null || !accountCursor.moveToNext()) {
				log("account is not in database at current");
				return false;
			}
			if (accountCursor != null){
				accountCursor.close();
			}
			return true;
	}

	public boolean queryAccounts(Set accountSet) {
		if (accountSet == null) {
			return false;
		}
		//find the default account ID and expose as the  MAP account
		Cursor accountCursor = null;
		
		try {
			mResolver.query(Email.ACCOUNT_URI, 
									new String[]{AccountColumns.ID},
									null,	
									null, 
									null);
		} catch (SecurityException e) {
				e.printStackTrace();
				return false;
		}						
		if (accountCursor == null) {
			return false;
		}
		while(accountCursor.moveToNext()) {
			accountSet.add(accountCursor.getLong(0));
		}
		accountCursor.close();
		return true;
	}
	/*EMAIL: accoount -> mailbox -> */
	public long queryDefaultAccount() {
		//find the default account ID and expose as the  MAP account
		long id = -1;
		Cursor accountCursor = null;
		try{
			accountCursor = mResolver.query(Email.ACCOUNT_URI, 
									DEFAULT_ACCOUNT_ID_PROJECTION,
									IS_DEFAULT + "=?",	
									new String[]{Integer.toString(Email.isAccoutDefault)}, 
									null);
		} catch (SecurityException e) {
				e.printStackTrace();
				return id;
		}							
		if (accountCursor != null) {
			if(accountCursor.moveToFirst()) {
				id = accountCursor.getLong(0);
			}
			accountCursor.close();
		}
		return id;
	}
	
	
	public BMessageObject getMessage(MessageRequest req) {
		//for EMAIl and MMS, the charset always be UTF-8		
		VCard vCard = new VCard();
		boolean attachment = req.isAttachDelivered();
		long messageID = req.getMessageId();
		//todo: fill the projection
		String [] PROJECTION = DEFAULT_MESSAGE_LIST_PROJECTION;
		Uri uri = ContentUris.withAppendedId(Email.MESSAGE_URI, messageID);
		Cursor messageCursor;

		log("getMessage(): id is "+ messageID + ",attachment is "+ attachment);
		
		try{
			messageCursor = mResolver.query(uri, PROJECTION,
							null,
							null, 
							null);
		} catch (SQLiteException e) {
				e.printStackTrace();
				return null;
		} catch (SecurityException e) {
				e.printStackTrace();
				return null;
		}
		if (messageCursor == null || !messageCursor.moveToFirst()) {
			log("no message record for the id");
			return null;
		}
		BMessageObject msg = mInstance.getBMessageObject();
		msg.reset();
		msg.setEncoding(MAP.ENCODING_8BIT);
		msg.setCharset(MAP.CHARSET_UTF8);
		msg.setLang(-1);
		msg.setMessageType(MAP.MSG_TYPE_EMAIL);

		
		//orignator
		String orignator = Address.unpackToString(messageCursor.getString(COLUMN_FROM));
		String[] regions = null;
		if (orignator != null) {
			regions = orignator.split("@");
		}
		if (regions != null) {
			vCard.setName(regions[0]);
		}
		vCard.setEmail(orignator);
		msg.setOrignator(vCard.toString());

		//to
		vCard.reset();
		StringBuilder recipient = new StringBuilder();
		Address[] toList = Address.unpack(messageCursor.getString(COLUMN_TO));
		String toAddress;
		for (Address to: toList) {
			toAddress = to.getAddress();
			regions = toAddress.split("@");
			if (regions != null) {
				vCard.setName(regions[0]);
			}
			vCard.setEmail(toAddress);
			recipient.append(vCard.toString());
		}
		msg.addRecipient(recipient.toString());
		msg.setReadStatus(convertReadStatus(messageCursor.getInt(COLUMN_READ)));

		EmailMime mime = new EmailMime(mResolver, messageID);
		
		try {
			Rfc822Output.writeTo(msg.getFile(), mime);
		} catch (IOException e) {
			e.printStackTrace();
		}
		//TODO: get size from database
		msg.setContentSize(msg.getFile());
		messageCursor.close();
		return msg;
	}

	/**/
	public MessageListObject getMessageList(MessageListRequest req) {
		int listSize =req.getListSize() ;
		int listOffset = req.getListOffset();
		int maxSubjectLen = req.getMaxSubjectLen();
		String recipient = req.getRecipient();
		String orignator = req.getOrignator();
		int mailbox = convertMailBoxType(req.getFolder());	
		long mailboxId = 0;
		mMessageListObject = mInstance.getMsgListRspCache();

		log("getMessageList(): listsize is "+listSize+",listoffset is"+listOffset
			+ ", maxSubjectLen is "+ maxSubjectLen + ",mailbox is "+mailbox);

		StringBuilder where = new StringBuilder();
		ArrayList<String> whereArgs = new ArrayList<String>();
		
		int index = 0;

		Cursor listCursor;
		
		int count;

		log("getMessageList(): Account id is "+ mAccountId);

		String[] PROJECTION  = DEFAULT_MESSAGE_LIST_PROJECTION;

		/*we have to get account ID, maibox id to find message*/

		if (listSize < 0 || listOffset < 0 || maxSubjectLen < 0 ) {
			log("message request is not avalaible");
			return null;
		} 
		
		if(mailbox < 0) {
			return null;
		}
	

		convertFilterToSelection(req,where,whereArgs);
		try{	
		listCursor =mResolver.query(Email.MESSAGE_URI, PROJECTION,
							where.toString(), 
							whereArgs.toArray(new String[whereArgs.size()]), 
							Email.KEY_TIMESTAMP_DESC);
		} catch (SQLiteException e) {
				e.printStackTrace();
				return null;
		} catch (SecurityException e) {
				e.printStackTrace();
				return null;
		}
		//set cursor to the beginning
		if (listCursor == null){
			return null;
		}

		
		//if maxListCount is euqals to zero, just ignore ListStartOffset, Subjectlength and parameterMask
		if (listSize > 0 && listOffset > 0 && !listCursor.move(listOffset)) {	
			listCursor.close();
			return null;
		}

		boolean newMessageFlag = false;
		while(listCursor.moveToNext() && (listSize == 0 || mMessageListObject.getCurrentSize() < listSize)) {
			//recipient filter
			if (recipient != null) {
				String toList = listCursor.getString(listCursor.getColumnIndex(Email.MessageColumns.TO_LIST));
				if (toList == null || toList.indexOf(recipient) == -1) {
					continue;
				}
			}
			
			//orignator filter
			if (orignator!= null) {
				String fromList = listCursor.getString(listCursor.getColumnIndex(Email.MessageColumns.FROM_LIST));
				if (fromList == null || fromList.indexOf(recipient) == -1) {
					continue;
				}
			}

			//priority filter not supported
				
			index++;
			if (listSize > 0) {
				mMessageListObject.addMessageItem(composeMessageItem(listCursor));
			}
			mMessageListObject.addSize(1);
				
		}
		listCursor.close();

		if (newMessageFlag) {
			mMessageListObject.setNewMessage();	
		}

		//at current, we have skip some items, so the offset has to be declined
		req.declineListOffset(index);			
		
		return mMessageListObject;
	
	}
	public boolean pushMessage(BMessageObject message) {
		log("pushMessage");
		File file = message.getFile();

		//TO DO:parse Email message
		//new file -> read message and parse -> save it to new file
		boolean tranparent = message.isTransparent();
 		int sendAndSave;
		String folder = message.getFolder();
		int mailbox = folder == null ? -1 : convertMailBoxType(folder);

		if (mailbox == Email.OUTBOX) {
			if (tranparent) {
				sendAndSave = Email.SEND_NO_SAVE;
			} else {
				sendAndSave = Email.SEND_AND_SAVE;
			}
		} else {
			sendAndSave = Email.NO_SEND_AND_SAVE;
		}

		if (mAccountId == -1 || mailbox == -1){
			log("invalid account ID or invalid mailbox");
			return false;
		}
				
		Intent intent = new Intent(ACTION_SEND);
		intent.putExtra(EXTRA_ACCOUNT, mAccountId);
		intent.putExtra(EXTRA_STREAM, Uri.fromFile(file));
		intent.putExtra(EXTRA_FLAG,sendAndSave);

		if (sendAndSave == Email.NO_SEND_AND_SAVE){
			intent.putExtra(EXTRA_MAILBOX_TYPE,mailbox);			
		}
		
		mContext.sendBroadcast(intent);
		Message msg = mHandler.obtainMessage(MESSAGE_SEND_TIMEOUT);
		mHandler.sendMessageDelayed(msg,DELAYED_TIMEOUT);

		//TODO: get handle
		message.setHandle(HandleUtil.getHandle(mMsgType, 1));
		
		return true;
	}

	public boolean updateInbox(){
		log("updateInbox()");
		Intent intent = new Intent(ACTION_UPDATE_INBOX);
		intent.putExtra(EXTRA_ACCOUNT, new long[] {mAccountId});
		intent.putExtra(EXTRA_NEED_RESULT,true);

		mContext.sendBroadcast(intent);
		Message msg = mHandler.obtainMessage(MESSAGE_UPDATE_INBOX_TIMEOUT);
		mHandler.sendMessageDelayed(msg,DELAYED_TIMEOUT);
		return true;
	}	

	public boolean setMessageStatus(long id, int state) {
		int emailState = reverseReadStatus(state);
		log("setMessageStatus():id is "+id+", state is "+state);
		if (emailState == -1) {
			return false;
		}
		Uri uri =ContentUris.withAppendedId(Email.MESSAGE_URI, id);
		ContentValues value = new ContentValues();
		value.put(Email.MessageColumns.FLAG_READ,emailState);
		try {            
			mResolver.update(uri,value,null,null);
        } catch (Exception e) {
        	log(e.toString());
			return false;
		}
		return true; 
		
	}
	
	private MessageItem composeMessageItem(Cursor cs){
		MessageItem msg = new MessageItem();
		int size = 0;
		int attachSize = 0;
		int recipientStatus = MAP.RECEPIENT_STATUS_COMPLETE;	
		boolean isText;
		int readStatus;

		//to do: obtain size
	/*	log("composeMessageItem()");
		recipientStatus = convertLoadStatus(cs.getInt(COLUMN_LOADED));
		//if the message has been deleted, return directly
		if (recipientStatus == -1) {
			log("invalid recipient Status: "+recipientStatus);
			return null;
		}
		*/

		readStatus = convertReadStatus(cs.getInt(COLUMN_READ));
		if(readStatus == -1) {
			readStatus = MAP.READ_STATUS;
		}

		isText = true; //email default has text in message  
		
		
		if (cs.getInt(COLUMN_LOADED) == 1) {
			
			long msgId = cs.getLong(COLUMN_MESSAGE_ID);			
			Uri attachUri = ContentUris.withAppendedId(Email.ATTACHMENT_URI, msgId);
			Cursor cursor =mResolver.query(attachUri, 
									new String[] {
										Email.AttachmentColumns.SIZE
									},
									null,
									null,
									null);
			if (cursor != null) {
				while (cursor.moveToNext()) {
					attachSize += cursor.getInt(0);
				}
				cursor.close();
			}
		}

		msg.setHandle(HandleUtil.getHandle(mMsgType,cs.getLong(COLUMN_ID)));
		msg.setSubject(cs.getString(COLUMN_SUBJECT));
		msg.setDatetime(cs.getLong(COLUMN_TIMESTATP));
		msg.setSenderAddr(Address.toString(Address.unpack(cs.getString(COLUMN_FROM))));
		msg.setSenderName(null);
		msg.setReplyAddr(Address.toString(Address.unpack(cs.getString(COLUMN_REPLAY))));
		msg.setRecipientName(null);
		log(cs.getString(COLUMN_TO));
		log(Address.toString(Address.unpack(cs.getString(COLUMN_TO))));
		msg.setRecipientAddr(Address.toString(Address.unpack(cs.getString(COLUMN_TO))));
		msg.setMsgType(mMsgType);
		msg.setSize(cs.getInt(COLUMN_SIZE));
		msg.setText(isText);
		msg.setRecipientStatus(recipientStatus);
		msg.setAttachSize(attachSize);
		msg.setReadStatus(readStatus);
		msg.setProtected(false);
		msg.setPriority(false);		
		return msg;
				  
	}
	public boolean restoreMessage (long messageID) {
		log("restoreMessage():id is "+ messageID);
		boolean flag;
		long deleteMailboxId = getMaiboxId(Email.TRASH);
		String[] projection = new String[] {
					Email.MessageColumns.MAILBOX_KEY};
		
		Uri uri =ContentUris.withAppendedId(Email.MESSAGE_URI, messageID);

		Cursor cs =  null ;
		try {
			cs = mResolver.query(uri, projection, null, null, null);
		} catch (SecurityException e) {
				e.printStackTrace();
				return false;
		}
		if(cs != null && cs.moveToFirst()) {
			int mailbox = cs.getInt(0);
			if (mailbox == deleteMailboxId) {
				Integer orignalFolder = mDeleteFolder.remove(Long.valueOf(messageID));
				if (orignalFolder != null) {
					ContentValues cv = new ContentValues();
           			cv.put(Email.MessageColumns.MAILBOX_KEY, orignalFolder);
            		mResolver.update(uri, cv, null, null);
				} else {
					log("no record in delete folder");						
				}					
			}
			flag = true;
		} else {
			log("the message does not exist in MMS provider");
			flag = false;
		}
		cs.close();
		return flag;
	}

		
	public boolean deleteMessage (long messageID) {
		String[] message_PROJECTION = new String[] {
					Email.MessageColumns.MAILBOX_KEY,
					Email.MessageColumns.FLAG_ATTACHMENT,
				};
		int MESSAGEID_TO_MAILBOXID_COLUMN_MAILBOXID = 1;
		long newMailboxId = getMaiboxId(Email.TRASH);
		long oldMailboxId = -1;		
		long id = messageID;
		Uri uri =ContentUris.withAppendedId(Email.MESSAGE_URI, id);

		log("deleteMessage(): id is "+ messageID);
		
		
		//we have to check the mailbox type
		//if the mailbiox is delete, delete it,
		//or else, move the message to delete

		Cursor messageCursor = null; 
		
		try {
			mResolver.query(uri, message_PROJECTION,
							Email.MessageColumns.ACCOUNT_KEY+"=? AND " + Email.MessageColumns.ID + "=?",
							new String[] {
								String.valueOf(mAccountId),
								String.valueOf(messageID)
							}, 
							null);		
		} catch (SecurityException e) {
				e.printStackTrace();
				return false;
		}	
		if (messageCursor == null) {
			log("fail to find message");
			return false;
		}
        try {
            oldMailboxId = messageCursor.moveToFirst()
                ? messageCursor.getLong(0)
                : -1;
        } finally {
            messageCursor.close();
        }	

		log("oldMailboxId is "+ oldMailboxId + ",newMailboxId is "+newMailboxId);
		
			
		if (oldMailboxId == newMailboxId) { 
			deleteAttachment(id);
			mResolver.delete(uri, null, null);
			log("delete the record for ever");
			mDeleteFolder.remove(Long.valueOf(id));
		} else {
			ContentValues cv = new ContentValues();
            cv.put(Email.MessageColumns.MAILBOX_KEY, newMailboxId);
            mResolver.update(uri, cv, null, null);
			mDeleteFolder.put(Long.valueOf(id), Integer.valueOf((int)oldMailboxId));
			log("move record to deleted folder");						
		}
		messageCursor.close();
		return true;
	}
	
	private void deleteAttachment (long messageID){
		//email attachment provider do nothing when delete attachment.
		//but we still process it as a procedure.
		log("deleteAttachment():"+messageID);
		Uri uri = ContentUris.withAppendedId(Email.MESSAGE_ID_URI, messageID);      
		Cursor attachCursor = mResolver.query(uri,
							null,
							null, null, null);
		if (attachCursor == null) {
			return;
		}
        try {
		    while (attachCursor.moveToNext()) {
				mResolver.delete(uri, null , null);
            }
        } catch (SQLiteException e) {
			e.printStackTrace();
		}
		attachCursor.close();;
	}

	//Email provider use a special way to construct mailbox provider
	//each account will construct a unique set of mailbox, so event the same mailbox type, 
	//so multi mailboxs may have the same mailbox type
	private long getMaiboxId(int mailboxType){		
		Cursor mailboxCursor;
		Uri mailboxUri = Email.MAILBOX_URI;
		long id = -1;
		String[] projection = new String[]{Email.MailboxColumns.ID};
		String selection = Email.MailboxColumns.ACCOUNT_KEY+"=?"+" AND "
			+Email.MailboxColumns.TYPE+"=?";
		String[] selectionArgs = new String[]{Long.toString(mAccountId), 
											Integer.toString(mailboxType)};

		log("getMaiboxId(): mAccountId is "+ mAccountId + "mailbox type is "+mailboxType);
		
		if (mAccountId < 0) {
			log("mAccountId is invalid");
			return -1;
		}
		try {
			mailboxCursor = mResolver.query(mailboxUri, projection, 
											selection, selectionArgs,null);

		} catch (SQLiteException e) {
			e.printStackTrace();
			return -1;
		} catch (SecurityException e) {
			e.printStackTrace();
			return -1;
		}
		if (mailboxCursor != null && mailboxCursor.moveToNext()){
			id = mailboxCursor.getLong(0);
		}
		mailboxCursor.close();
		log("no available mailbox for the account");
		return id;
	}	

	private int getMailboxType(long mailboxId){
		int type = -1;
		String[] projection = new String[]{Email.MailboxColumns.TYPE};		
		Uri uri = ContentUris.withAppendedId(Email.MAILBOX_URI, mailboxId);
		Cursor cursor = mResolver.query(uri, projection, null, null , null);
		if (cursor == null) {
			return -1;
		}
		if (cursor.moveToNext()){
			type = cursor.getInt(0);
		}
		cursor.close();
		return type;
	}

	private String [] convertMaskToProjection(Set mask) {
		String [] result = new String[mask.size()];
		ArrayList<String> projection = new ArrayList<String>();
		int index = 0;	

		return DEFAULT_MESSAGE_LIST_PROJECTION;
	
	}
	
	//email -> map
	private	int convertReadStatus(int emaillReadStatus){
		switch(emaillReadStatus) {
			case Email.UNREAD:
				return MAP.UNREAD_STATUS;
			case Email.READ:
				return MAP.READ_STATUS;
			default:
				log("error: the read status from email provider is invalid");
				return -1;
		}
	}

	private int reverseReadStatus(int mapReadStatus) {
		switch(mapReadStatus) {
			case MAP.UNREAD_STATUS:
				return Email.UNREAD;
			case MAP.READ_STATUS:
				return Email.READ;
			default:
				log("other map state");
				return -1;
		}
	}
			

	private int convertLoadStatus(int loadStatus) {
		switch(loadStatus) {
			case Email.FLAG_LOADED_COMPLETE:
				return MAP.RECEPIENT_STATUS_COMPLETE;
			case Email.FLAG_LOADED_PARTIAL:
				return MAP.RECEPIENT_STATUS_FRACTIONED;
			case Email.FLAG_LOADED_UNLOADED:
				return MAP.RECEPIENT_STATUS_NOTIFICATION;
			case Email.FLAG_LOADED_DELETED:
			default:
				return -1;
		}
			
	}

	private String[] convertFilterToSelection(MessageListRequest req, 
													StringBuilder selection,
													ArrayList<String> selectionArgs ){
		long startTimeMillis = req.getStartTime();
		long endTimeMillis = req.getEndTime();
		int readStatus = req.getReadStatus();
		int mailbox = convertMailBoxType(req.getFolder());
		long mailboxId;	

		//firstly check the maiboxs based on the account
		mailboxId = getMaiboxId(mailbox);
		
		
		//account key
		selection.append(Email.MessageColumns.ACCOUNT_KEY+"=?");
		selectionArgs.add(Long.toString(mAccountId));

		

		//mailbox	
		selection.append(" AND ");	
		selection.append(Email.MessageColumns.MAILBOX_KEY + "=?");		
		selectionArgs.add(Long.toString(mailboxId));

		if (startTimeMillis > 0 && endTimeMillis > 0) {
			selection.append(" AND ");
			selection.append(Email.MessageColumns.TIMESTAMP);	
			selection.append(" between ? AND ?");	
			selectionArgs.add(Long.toString(startTimeMillis));
			selectionArgs.add(Long.toString(endTimeMillis));
		} 

		if(readStatus == MAP.READ_STATUS) {
			selection.append(" AND ");
			selection.append(Email.MessageColumns.FLAG_READ + "=?");		
			selectionArgs.add(Integer.toString(Email.READ));
		} else if(readStatus == MAP.UNREAD_STATUS) {
			selection.append(" AND ");
			selection.append(Email.MessageColumns.FLAG_READ+ "=?");
			selectionArgs.add(Integer.toString(Email.UNREAD));
		}

		return null;		
	
	}
	private int convertMailBoxType(String mailboxType) {
		if (mailboxType == null) {
			return -1;
		}		
		if (mailboxType.equals(MAP.Mailbox.INBOX)) {
			return Email.INBOX;
		} else if(mailboxType.equals(MAP.Mailbox.OUTBOX)) {
			return Email.OUTBOX;
		} else if(mailboxType.equals(MAP.Mailbox.SENT)) {
			return Email.SENT;
		} else if(mailboxType.equals(MAP.Mailbox.DELETED)) {
			return Email.TRASH;
		} else if(mailboxType.equals(MAP.Mailbox.DRAFT)) {
			return Email.DRAFTS;
		} else {
			log("convertMailBoxType():invalid mail box type->"+mailboxType);
			return -1;
		}
	}

	public String revertMailboxType (int mailboxId) {
		int type = getMailboxType(mailboxId);
		switch(type){
			case Email.INBOX:
				return MAP.Mailbox.INBOX;
			case Email.OUTBOX:
				return MAP.Mailbox.OUTBOX;
			case Email.SENT:
				return MAP.Mailbox.SENT;
			case Email.TRASH:
				return MAP.Mailbox.DELETED;
			case Email.DRAFTS:
				return MAP.Mailbox.DRAFT;
			default:
				log("unsupported tye:"+type);
				return null;
		}			
	}

	public void queryMessage(HashMap<Long, Integer> info){
			Cursor messageCursor = null;
			try{ 
				messageCursor =mResolver.query(Email.MESSAGE_URI, 
									new String[]{Email.MessageColumns.ID,
												Email.MessageColumns.MAILBOX_KEY},
									Email.MessageColumns.ACCOUNT_KEY+"=?",	
									new String[]{Long.toString(mAccountId)}, 
									Email.KEY_TIMESTAMP_DESC);
			} catch (SQLiteException e) {
				e.printStackTrace();
			} catch (SecurityException e) {
				e.printStackTrace();
			}
			if (messageCursor == null){
				return;
			}
			while(messageCursor.moveToNext()){
				info.put(messageCursor.getLong(0), messageCursor.getInt(1));				
			}
			messageCursor.close();
	}

	private void log(String info) {
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}
