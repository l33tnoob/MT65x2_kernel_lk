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

import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.PduComposer;
import com.google.android.mms.pdu.CharacterSets;
import com.google.android.mms.ContentType;
import com.google.android.mms.MmsException;

import android.provider.Telephony.Mms.Part;
import android.provider.Telephony.Mms.Addr;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;

import android.net.Uri;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;

import com.android.internal.telephony.Phone;

import android.net.ConnectivityManager;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.ContentValues;
import android.content.ContentUris;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.database.ContentObserver;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.os.Message;
import android.os.Handler;

import java.util.ArrayList;
import java.util.HashMap;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.FileNotFoundException;
import java.io.FileInputStream;


import com.mediatek.bluetooth.map.MAP;
import com.mediatek.bluetooth.map.MmsConnection;
import com.mediatek.bluetooth.map.cache.*;
import com.mediatek.bluetooth.map.util.*;
import com.mediatek.bluetooth.map.mime.*;

import android.text.format.Time;

import com.google.android.mms.pdu.*;

import java.util.Iterator;
import java.util.Map;

class MmsController extends Controller 
	implements MmsConnection.ConnectionListener, MessageObserver.ControllerHelper{

		private final String TAG = "MMSController";
	private String[] ID_PROJECTION = new String[]{
		Mms._ID
	};

	private String[] BASE_PROJECTION = new String[]{
		Mms._ID,
		Mms.READ, 
	//	Mms.FROM,
	//	Mms.TO,   
	};

	private static final int INVALID_THREAD_ID			= -1;

	private static final int BASE_ID_COLUMN				= 0;	
    private static final int BASE_READ_COLUMN 			= 1;
    private static final int BASE_FROM_COLUMN 			= 2;	
    private static final int BASE_TO_COLUMN 			= 3;

	private String[] EXTEND_PROJECTION = new String[]{
			Mms.DATE,
			Mms.SUBJECT,
			Mms.SUBJECT_CHARSET,
			Mms.PRIORITY,	 
	};
	private static final int EXTEND_DATE_COLUMN				= 0;
    private static final int EXTEND_SUBJECT_COLUMN 			= 1;	
    private static final int EXTEND_SUBJECT_CHARSET_COLUMN 	= 2;	
    private static final int EXTEND_PRIORITY_COLUMN 		= 3;

	private String[] SIZE_PROJECTION = new String[]{
			Mms.MESSAGE_SIZE,
	};

	
	private static final String[] DEFAULT_PROJECTION = new String[] {
		Mms._ID,			
		Mms.SUBJECT,
		Mms.SUBJECT_CHARSET, //we have to focus on the field
		Mms.DATE,
		Mms.FROM,
	//	Mms.TO,
	//	Mms.CC,
	//	Mms.MESSAGE_SIZE,
	//	Mms.PRIORITY,
	//	Mms.STATUS,
	//	Mms.READ,            // 1 for read, 0 for unread 
	//	Mms.THREAD_ID,
	//	Mms.MESSAGE_BOX,
	//	Mms.MESSAGE_ID,
	//	Mms.CONTENT_TYPE,
	//	Mms.CONTENT_LOCATION,
	//	Mms.MMS_VERSION,
	};

	
	private static final int ID_COLUMN				= 0;
    private static final int SUBJECT_COLUMN 		= 1;	
    private static final int SUBJECT_CHARSET_COLUMN = 2;
    private static final int DATE_COLUMN 			= 3;
    private static final int FROM_COLUMN 			= 4;
    private static final int TO_COLUMN 				= 5;
    private static final int CC_COLUMN 				= 6;
    private static final int MESSAGE_SIZE_COLUMN 	= 7;
    private static final int PRIORITY_COLUMN 		= 8;
    private static final int STATUS_COLUMN 			= 9;
    private static final int READ_COLUMN		 	= 10;
	private static final int THREAD_ID_COLUMN 		= 11;
	private static final int MESSAGE_BOX_COLUMN 	= 12;
	private static final int MESSAGE_ID_COLUMN 		= 13;
	private static final int CONTENT_TYPE_COLUMN 	= 14;
	private static final int MMS_VERSION_COLUMN 	= 15;

	private final String[] ADDRESS_PROJECTION = new String[]{
		Addr.ADDRESS,
		Addr.TYPE,
		Addr.CHARSET
	};
	private static final int ADDRESS_COLUMN				= 0;
    private static final int ADDRESS_TYPE_COLUMN 		= 1;
    private static final int ADDRESS_CHARSET_COLUMN 	= 2;

	
    private static final String[] PART_PROJECTION = new String[] {
        Part.CHARSET,
        Part.CONTENT_TYPE,
        Part.TEXT
    };
	private static final int PART_CHARSET_COLUMN				= 0;
    private static final int PART_CONTENT_TYPE_COLUMN 			= 1;
    private static final int PART_TEXT_COLUMN 					= 2;

	//address type
	private static final int ADDRESS_TYPE_BCC 			= PduHeaders.BCC;
    private static final int ADDRESS_TYPE_CC 			= PduHeaders.CC;
    private static final int ADDRESS_TYPE_FROM			= PduHeaders.FROM;
    private static final int ADDRESS_TYPE_TO			= PduHeaders.TO;
	

	//read status
	private static final int MMS_READ_STATUS		= 1;
	private static final int MMS_UNREAD_STATUS		= 0;

	//delete folder
	private static final int MESSAGE_TYPE_DELETE = 100;

	private Context mContext;
	private ContentResolver mContentResolver;
	private int mSimId;
	private int mInstanceId;
	private Instance mInstance;
	private MmsConnection mMmsConnection;
	private MessageObserver mMessageObserver ;

	private int 			mType = MAP.MSG_TYPE_MMS;	
	public MmsController (Context context, Instance instance, int simId) {
		mContext = context;
		mContentResolver = mContext.getContentResolver();
		mSimId = simId;
		mInstanceId = instance.getInstanceId();
		mInstance = instance;
		onStart();
	}

	public void onStart(){
		//none
	}
	public void onStop(){
		clearDeletedMessage();
		deregisterListener();
	}

	public void registerListener(ControllerListener listener) {	
		log("registerListener");
		super.registerListener(listener);
		if (mMessageObserver == null){
			mMessageObserver = new MessageObserver(this, mListener, mType);
		}
		mContentResolver.registerContentObserver(Mms.CONTENT_URI, true, mMessageObserver);	
		mContentResolver.registerContentObserver(Threads.CONTENT_URI, true, mMessageObserver);		
	}

	public void deregisterListener() {
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
			int maxSubjextLen = req.getMaxSubjectLen();
			int priority = req.getPriority();
			int index = 0;
			Cursor listCursor;
			long id;			
			Uri mailboxUri = Mms.CONTENT_URI;;
						
			StringBuilder selection = new StringBuilder();
			ArrayList<String> selectionArgs = new ArrayList<String>();
	
			String[] projection;	
			ContactsAdapter adapter = ContactsAdapter.getDefault(mContext);
			String orignatortAddrList = null;
			String recipientAddrList = null;
			if (orignator != null&& orignator.length()>0) {
				orignatortAddrList = adapter.queryNumber(orignator);
			} else if (recipient != null&& recipient.length()>0) {
				recipientAddrList = adapter.queryNumber(recipient);
			}

			//MMS does not support high priority
			if (priority == MAP.PRIORITY_STATUS_HIGH) {
				return null;
			}
			
			log("getMessageList(): mSimId is "+mSimId);	
					
			if(!convertFilterToSelection(req, selection,  selectionArgs))
			{
				return null;
			}
			
			if (mailbox != MESSAGE_TYPE_DELETE){
				mailboxUri = getMailboxUri(mailbox);
				if (mailboxUri == null) {
					log("unrecognized mailbox uri");
					return null;
				}
			} 
			
			try {
	
				listCursor = mContentResolver.query(mailboxUri,BASE_PROJECTION,
									selection.toString(),
									selectionArgs.toArray(new String[selectionArgs.size()]),
									Mms.DEFAULT_SORT_ORDER);									
	
			} catch (SQLiteException e) {
				e.printStackTrace();
				return null;
			}
			if (listCursor == null) {
				return null;
			}
	
			MessageListObject list = mInstance.getMsgListRspCache();
				
			log("listsize is "+ listSize+ ",current size"+list.getCurrentSize());
	
			//if maxListCount is euqals to zero, just ignore ListStartOffset, Subjectlength and parameterMask
			if (listSize > 0 && offset > 0 && !listCursor.move(offset)) { 
				return null;
			}
	
			boolean newMessageFlag = false;
			while(listCursor.moveToNext() && (listSize == 0 || list.getCurrentSize() < listSize)) {
				int read = listCursor.getInt(BASE_READ_COLUMN);
				if( read == MMS_READ_STATUS) {
					newMessageFlag = true;
				}

				long msgId = listCursor.getLong(BASE_ID_COLUMN);
				StringBuilder fromList = new StringBuilder();
				StringBuilder toList = new StringBuilder();

				getAddress(msgId, fromList, toList);

				if (orignator != null && orignator.length()>0) {
					if(!adapter.doesPhoneNumberMatch(normalizeString(fromList.toString()), orignatortAddrList, orignator)){
						continue;
					}
				}				
				if (recipient != null && recipient.length()>0) {
					if(!adapter.doesPhoneNumberMatch(normalizeString(toList.toString()), recipientAddrList, recipient)){
						continue;
					}
				}				
					
				if (listSize > 0 ) {
					list.addMessageItem(composeMessageItem(msgId,fromList.toString(),toList.toString(), read));
				}
				index++;
				
				list.addSize(1);
					
			}
			listCursor.close();
			if (newMessageFlag){
				list.setNewMessage(); 
			}
			req.declineListOffset(index);	
		
			return list;
		}
	
		public boolean pushMessage(BMessageObject obj) {
			log("pushMessage()");
			boolean ret = false;
			boolean isSave;
			Uri msgUri = null;
			PduPersister pduPersister = null;
			long size = obj.getContentSize();
			MmsMime mime = new MmsMime(mContentResolver);
			String folder = obj.getFolder();
			int mail = (folder == null) ? -1 : convertMailboxType(folder);
			Uri mailbox;
			MultimediaMessagePdu pdu;
			MimeParser parser;
			long messageid = -1;
			long sim = NetworkUtil.getSimIdBySlotId(mContext,mSimId);

			if (sim == -1 || mail == -1) {
				return false;
			} 

			FileInputStream in = null; 
			try {
				in = new FileInputStream(obj.getFile());
			} catch (FileNotFoundException e) {
					log(e.toString());
			}

			parser = new MimeParser(in, mime);			

			parser.parse();
			try {
				in.close();
			} catch (IOException e) {
				log(e.toString());
			}
			

			/*FROM or TO fields may be not include in message body, so we have to check it again*/
			if(!mime.isHeaderComplete()) {
				String from;
				ArrayList<String> tos = obj.getRecipient();
				String to;
				VCard vcard = new VCard();

				for (int index = 0; tos != null && index < tos.size(); index ++) {
					vcard.parse(tos.get(index));	
					to = normalizeString(vcard.getTelephone());
					mime.addToField(to);
					vcard.reset();	
					log("index:"+index + ":"+tos.get(index));
				}			
				
				vcard.parse(obj.getOrignator());
				from = normalizeString(vcard.getTelephone());
				mime.setFromField(from);
			}

			int type;
			if (mail == Mms.MESSAGE_BOX_OUTBOX) {
				type = PduHeaders.MESSAGE_TYPE_SEND_REQ;
			} else {
				type = PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF;
			}
			
			pdu = mime.generatePdu(type);
			if (pdu == null){
				return false;
			}
			
			isSave = !obj.isTransparent();
			pduPersister = PduPersister.getPduPersister(mContext);	
			if (isSave){
				mailbox = getMailboxUri(mail);					
			        try {
					msgUri = pduPersister.persist(pdu, mailbox);	
					ContentValues cv = new ContentValues();
					cv.put(Mms.READ, MMS_UNREAD_STATUS);
					cv.put(Mms.SIM_ID, sim);
					cv.put(Mms.MESSAGE_SIZE, size);
					mContentResolver.update(msgUri, cv, null, null);
					if (msgUri != null) {
						Cursor cs = mContentResolver.query(msgUri, new String[]{Mms._ID},null,null,null);
						if (cs != null && cs.moveToFirst()) {
							messageid = cs.getLong(0);	
							cs.close();	
						}
					}
				}
				catch (Exception e) {
				}
				}
			if (messageid < 0)
			{
				return false;
			}
			obj.setHandle(HandleUtil.getHandle(mType, messageid));
		
			if (mail == Mms.MESSAGE_BOX_OUTBOX){
				
					mMmsConnection = MmsConnection.getDefault(mContext, mSimId);	
				mMmsConnection.send(MmsConnection.NO_TOKEN,
												new PduComposer(mContext, (SendReq)pdu).make());
				try {
					if (msgUri != null) {
					pduPersister.move(msgUri, getMailboxUri(Mms.MESSAGE_BOX_SENT));
						}
					} catch (MmsException e) {
					log(e.toString());
				}
			}			
			return true;
			
		}

			
		public	BMessageObject getMessage(MessageRequest req) { 		
			String orignator = new String();
			PduBody body;
			PduPersister pduPersister;
			long id = req.getMessageId(); 
			Uri messageUri = getMessageUri(id);	
			GenericPdu pdu = null;
			MmsMime mime;
			VCard vCard;
			BMessageObject msg;
			boolean attachment = req.isAttachDelivered();
			long messageID = req.getMessageId();
			msg = mInstance.getBMessageObject();
			log("getMessage(): id is "+ messageID + ",attachment is "+ attachment);

			
			if (req.getFractionReq() == MAP.FRACTION_REQUEST_NEXT) {
				msg.retrieveNextPartion();
				return msg;
			}					

			try {
				pduPersister = PduPersister.getPduPersister(mContext);			
				pdu = pduPersister.load(messageUri);
			} catch (Exception e) {
				log(e.toString());
			}			
			mime = new MmsMime(mContentResolver, pdu);

			msg.reset();
			msg.setEncoding(MAP.ENCODING_8BIT);
			msg.setCharset(MAP.CHARSET_UTF8);
			msg.setLang(MAP.LANG_UNKNOWN);
			msg.setMessageType(MAP.MSG_TYPE_MMS);

			vCard = new VCard();
			//orignator (diff tel or email simplily)
			orignator = mime.getHeader().mFrom;
			if (isEmailAddress(orignator)) {
				vCard.setEmail(orignator);
			} else {
				vCard.setTelephone(orignator);
			}
			msg.setOrignator(vCard.toString());

			//to 			
			StringBuilder recipient = new StringBuilder();
			String[] toList = splitAddress(mime.getHeader().mTo);	
			if (toList != null) {
				for (String to: toList) {
					vCard.reset();
					if (isEmailAddress(to)) {
						vCard.setEmail(to);
					} else {
						vCard.setTelephone(to);
					}
					recipient.append(vCard.toString());
				}
				msg.addRecipient(recipient.toString());
			}
			
			//read : TODO
			msg.setReadStatus(MAP.READ_STATUS);			
		
			try {
				Rfc822Output.writeTo(msg.getFile(), mime);
			} catch (IOException e) {
				e.printStackTrace();
			}
			//TODO: get size from database
			msg.setContentSize(msg.getFile());

			msg.fragmentIfNeccessary();
			return msg;	
				
		}
	
		public	boolean deleteMessage(long id) {
			log("deleteMessage():id is "+id);
			boolean flag;
			String[] projection = new String[]{Mms.THREAD_ID};
			
			Uri uri =ContentUris.withAppendedId(Mms.CONTENT_URI, id);
	
			Cursor cs =  mContentResolver.query(uri, projection, null, null, null);
 			if(cs != null && cs.moveToFirst()) {
				int thread_id = cs.getInt(0);
				if (thread_id == INVALID_THREAD_ID) {
					mContentResolver.delete(uri,null,null);
					mDeleteFolder.remove(Long.valueOf(id));
				} else {
					ContentValues cv = new ContentValues();
					cv.put(Mms.THREAD_ID, Integer.valueOf(INVALID_THREAD_ID));
					mContentResolver.update(uri, cv, null, null);
					mDeleteFolder.put(Long.valueOf(id), Integer.valueOf(thread_id));			
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
			log("restoreMessage():id is "+id);
			boolean flag;
			String[] projection = new String[]{Mms.THREAD_ID};
			
			Uri uri =ContentUris.withAppendedId(Mms.CONTENT_URI, id);
	
			Cursor cs =  mContentResolver.query(uri, projection, null, null, null);
 			if(cs != null && cs.moveToFirst()) {
				int thread_id = cs.getInt(0);
				if (thread_id == INVALID_THREAD_ID) {
					Integer orignal_thread_id = mDeleteFolder.remove(Long.valueOf(id));
					if (orignal_thread_id != null) {
						ContentValues cv = new ContentValues();
						cv.put(Mms.THREAD_ID, Integer.valueOf(orignal_thread_id));
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
		//due to the delete folder is just valid for MAP in MMS, so when deivce is disconnected,
		//the messages in deleted folder have to be deleted.
		public void clearDeletedMessage(){
			log("clearDeletedMessage()");
			Uri uri;
			Long id;
			String[] projection = new String[]{Mms.THREAD_ID};
			int thread_id;
			
			Iterator iterator= mDeleteFolder.entrySet().iterator();
			while (iterator.hasNext()) {
				Map.Entry entry = (Map.Entry)iterator.next();
				id = (Long)entry.getKey();	
				uri =ContentUris.withAppendedId(Mms.CONTENT_URI, id);

				//we have to confirm the message is truely in deleted folder
				Cursor cs = mContentResolver.query(uri,projection,null,null,null);
				if (cs != null && cs.moveToFirst()) {
					thread_id = cs.getInt(0);
					if (thread_id == INVALID_THREAD_ID) {
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
	
		public boolean setMessageStatus(long id, int state){
			Uri uri =ContentUris.withAppendedId(Mms.CONTENT_URI, id);
			String[] projection = new String[]{
				Mms.READ};
			int newState = convertReadStatus(state);

			log("setMessageStatus():id is "+id+", state is "+ state);
	
			if (newState == -1) {
				log("the status to be set is invalid");
				return false;
			}		
	
			Cursor cs =  mContentResolver.query(uri, projection, null, null, null);
			if (cs != null && cs.moveToFirst()) {				
				if (cs.getInt(0) == newState) {
					log("state is same, no need to update");
				} else {
					ContentValues cv = new ContentValues();
					cv.put(Mms.READ, Integer.valueOf(newState));
					mContentResolver.update(uri, cv, null, null);
				}
				cs.close();
			} else {
				log("no record for the id");
			}
			return true;
		}
		
		public boolean updateInbox(){
			return true;
		}

		public void onSendResult(int result){
			if (result == MmsConnection.STATE_SUCCESS) {
				mMmsConnection.unregisterListener();
			}

		}

		private MessageItem composeMessageItem(long id, String from, String to, int read){
			MessageItem msg = new MessageItem();
			int size;
			int attachSize;
			int recipientStatus;	
			String text;
			int readStatus;

			log("composeMessageItem(): id is "+id+",from is "
				+from+",to is "+to+", read"+read);
			recipientStatus = revertLoadStatus(0);			
	
			readStatus = revertReadStatus(read);
			if (readStatus == -1) {
				log("invalid read status");
				return null;
			}

			text = getFirstTextFromParts(id);

			Uri msgUri = getMessageUri(id);
			Cursor msgCursor = mContentResolver.query(msgUri, EXTEND_PROJECTION,
								null,null,null);
			if (msgCursor == null || !msgCursor.moveToFirst()) {
				log("no the record:"+ id);
				return null;
			}

			boolean priority = revertPriority(msgCursor.getInt(EXTEND_PRIORITY_COLUMN));	
			
			msg.setHandle(HandleUtil.getHandle(mType,id));
			msg.setSenderAddr(from);
			msg.setSenderName(null);
			msg.setReplyAddr(null);
			msg.setRecipientName(null);
			msg.setRecipientAddr(to);
			msg.setMsgType(mType);
			msg.setText(text != null && text.length() > 0);			
			msg.setRecipientStatus(recipientStatus);
			msg.setReadStatus(readStatus);
			msg.setProtected(false);

			String subject = getUtf8String(msgCursor.getString(SUBJECT_COLUMN));	
			if (subject == null || subject.length() == 0) {
				subject = text;
			}
			msg.setSubject(subject);
			// Date column unit is seconds, instead of miliseconds 
			msg.setDatetime(msgCursor.getLong(EXTEND_DATE_COLUMN)*1000);
			
			
			//part is not attachement, so set attachment size as 0
			msg.setAttachSize(456);
			msg.setPriority(priority);
			

			msgCursor.close();

			//set size			
			Cursor sizeCursor = mContentResolver.query(msgUri, SIZE_PROJECTION,
								null,null,null);
			if(sizeCursor != null){
				if (sizeCursor.moveToFirst()) {
					msg.setSize(sizeCursor.getInt(0));
				} 
				sizeCursor.close();
			}			
			return msg;

		}
		
			
		private MessageItem composeMessageItem(Cursor cs, long id, String from, String to){
			MessageItem msg = new MessageItem();
			int size;
			int attachSize;
			int recipientStatus;	
			boolean isText;
			int readStatus;
	
			//to do:
			//now MMS has no proper attribute matching with load status, 
			//after confirming with MMS owner, the issue will be resolved
			recipientStatus = revertLoadStatus(cs.getInt(STATUS_COLUMN));
			if (recipientStatus == -1) {
				return null; 
			}
	
			readStatus = revertReadStatus(cs.getInt(READ_COLUMN));
			//if the message has been deleted, return directly
			if (readStatus == -1) {
				return null;
			}

			//the part field indicate the text info of the MMS message
			//
			isText = false;
			Uri partUri = Uri.parse(Mms.CONTENT_URI + Long.toString(id) + "/part");
			Cursor partCursor = mContentResolver.query(partUri, new String[]{Part.TEXT},
								null,null,null);
			while (partCursor !=null && partCursor.moveToNext()){
				if (cs.getString(0) != null) {
					isText = true;
					break;
				} 
			}
			partCursor.close();

			boolean priority = revertPriority(cs.getInt(PRIORITY_COLUMN));	
			
			String subject = getUtf8String(cs.getString(SUBJECT_COLUMN));	
			if (subject == null) {
				subject = getFirstTextFromParts(id);
			}
			msg.setSubject(subject);
			msg.setHandle(HandleUtil.getHandle(mType,cs.getLong(ID_COLUMN)));
			// Date column unit is seconds, instead of miliseconds 
			msg.setDatetime(cs.getLong(DATE_COLUMN)*1000);
			msg.setSenderAddr(from);
			msg.setSenderName(null);
			msg.setReplyAddr(null);
			msg.setRecipientName(null);
			msg.setRecipientAddr(to);
			msg.setMsgType(mType);
			msg.setSize(cs.getInt(MESSAGE_SIZE_COLUMN));
			msg.setText(isText);			
			msg.setRecipientStatus(recipientStatus);			
			//part is not attachement, so set attachment size as 0
			msg.setAttachSize(0);
			msg.setReadStatus(readStatus);
			msg.setProtected(false);
			msg.setPriority(priority);	
		
			return msg;
		}
	
		//MMS -> map
		private int revertLoadStatus(int mmsStatus) {
			//
			return MAP.RECEPIENT_STATUS_COMPLETE;
		}
		//map -> SMS
		private int convertReadStatus(int mapReadStatus) {
			switch(mapReadStatus) {
				case MAP.UNREAD_STATUS:
					return MMS_UNREAD_STATUS;
				case MAP.READ_STATUS:
					return MMS_READ_STATUS;
				default:
					log("other map state:"+mapReadStatus);
					return -1;
			}
		}
			//MMS -> map
		private int revertReadStatus(int mmsReadStatus){
			switch(mmsReadStatus) {
				case MMS_READ_STATUS:
					return MAP.READ_STATUS;
				case MMS_UNREAD_STATUS:
					return MAP.UNREAD_STATUS;
				default:
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
			String orignator = req.getOrignator();
			String recepiet = req.getRecipient();		

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
						selection.append(Mms.SIM_ID+"<>?");
						selectionArgs.add(Long.toString(simid));
					}
				}
			} else {			
				long simid = NetworkUtil.getSimIdBySlotId(mContext, mSimId);
				if (simid > 0) {
					if (selection.length() > 0) {
						selection.append(" AND ");
					}
					selection.append(Mms.SIM_ID+"=?");
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
				selection.append(Mms.THREAD_ID+"<>?");
			} else {
				selection.append(Mms.THREAD_ID+"=?");
			}
			selectionArgs.add(Integer.toString(INVALID_THREAD_ID));

			//date
			if (startTimeMillis > 0 && endTimeMillis > 0) {
				if (selection.length() > 0) {
					selection.append(" AND ");
				}
				selection.append(Mms.DATE);	
				selection.append(" between ? AND ?");	
				selection.append(" AND ");		
				selectionArgs.add(Long.toString(startTimeMillis));
				selectionArgs.add(Long.toString(endTimeMillis));
			} 
		
			if (readStatus != -1) {
				if (selection.length() > 0) {
					selection.append(" AND ");
				}
				selection.append(Mms.READ + "=?"); 
				selectionArgs.add(Integer.toString(readStatus));	
			}	
		
			return true;
		}

		public void queryMessage(HashMap<Long, Integer> info){
			Cursor messageCursor =mContentResolver.query(Mms.CONTENT_URI, 
										 new String[]{Mms._ID,
													 Mms.MESSAGE_BOX},
										 null,	 
										 null, 
										 null);
			if (messageCursor == null) {
					 return;
			}
			if (messageCursor.moveToFirst()) {
				do {
					info.put(messageCursor.getLong(0), messageCursor.getInt(1));				 
				} while(messageCursor.moveToNext());
			}
			messageCursor.close();
		}
	
		//MAP -> SMS
		private int convertMailboxType(String mapMailboxType) {
			if (mapMailboxType == null) {
				return -1;
			}
			if (mapMailboxType.equals(MAP.Mailbox.INBOX)) {
				return Mms.MESSAGE_BOX_INBOX;
			} else if (mapMailboxType.equals(MAP.Mailbox.OUTBOX)) {
				return Mms.MESSAGE_BOX_OUTBOX;
			} else if (mapMailboxType.equals(MAP.Mailbox.SENT)) {
				return Mms.MESSAGE_BOX_SENT;
			} else if (mapMailboxType.equals(MAP.Mailbox.DRAFT)) {
				return Mms.MESSAGE_BOX_DRAFTS;
			} else if (mapMailboxType.equals(MAP.Mailbox.DELETED)) {
				return MESSAGE_TYPE_DELETE;
			} 
			log("convertMailboxType(): the mail box is invalid->"+mapMailboxType);
			return -1;
		}
		public String revertMailboxType(int smsMailboxType) {
			int type;
			switch(smsMailboxType) {
				case Mms.MESSAGE_BOX_INBOX:
					return MAP.Mailbox.INBOX;
				case Mms.MESSAGE_BOX_OUTBOX:
					return MAP.Mailbox.OUTBOX;
				case Mms.MESSAGE_BOX_SENT:
					return MAP.Mailbox.SENT;
				case Mms.MESSAGE_BOX_DRAFTS:
					return MAP.Mailbox.DRAFT;
				case MESSAGE_TYPE_DELETE:
					return MAP.Mailbox.DELETED;
			}
			log("revertMailboxType(): the mail box is invalid->"+smsMailboxType);
			return null;
		}

		//following RFC4356, normal priority should be omitted. 
		//so we process it low priority
		private boolean revertPriority(int mmsPriority){
			switch(mmsPriority) {
				case PduHeaders.PRIORITY_LOW:
					return MAP.HIGH_PRIORITY;
				case PduHeaders.PRIORITY_HIGH:
				case PduHeaders.PRIORITY_NORMAL:
				default:
					return MAP.LOW_PRIORITY;
			}

		}

		/* The subject field in MMS database is not encoded as UTF-8, which is MAP required*/
		/* So we have to change it */
		private String getUtf8String(String rawData) {
			if (rawData != null) {
				EncodedStringValue v = new EncodedStringValue(
                        CharacterSets.UTF_8,
                        PduPersister.getBytes(rawData));
            	            return v.getString();
			} else {
			    return null;
			}
		}
		/*retrieve first text from parts if exists*/
		private String getFirstTextFromParts(long id) {
			String text = null;
			String charset = null;
			Cursor partCursor = mContentResolver.query(getPartUri(id),PART_PROJECTION, null,null,null);
			if (partCursor != null) {
				if (!partCursor.moveToFirst()) {
					partCursor.close();
					return null;
				}
				do {
					String type = partCursor.getString(PART_CONTENT_TYPE_COLUMN);
					log("round :"+type);
					if (ContentType.TEXT_PLAIN.equals(type)) {
						charset = partCursor.getString(PART_CHARSET_COLUMN);
						text = partCursor.getString(PART_TEXT_COLUMN);
						break;
					} else {
						log("type is "+type);
					}
				} while (partCursor.moveToNext());
				partCursor.close();
			} else {
				log("no any part ");
			}
			if (text != null && charset != null) {
				try {
					byte[] textBytes = text.getBytes(charset);
					text = new String(textBytes, CharacterSets.MIMENAME_UTF_8);
				} catch (UnsupportedEncodingException e) {
					log(e.toString());
				}			
			}
			return text;
		}

		private Uri getMailboxUri(int mailbox) {
			//ignore other type
			switch(mailbox) {
				case Mms.MESSAGE_BOX_INBOX:
					return Mms.Inbox.CONTENT_URI;
				case Mms.MESSAGE_BOX_OUTBOX:
					return Mms.Outbox.CONTENT_URI;
				case Mms.MESSAGE_BOX_SENT:
					return Mms.Sent.CONTENT_URI;
				case Mms.MESSAGE_BOX_DRAFTS:
					return Mms.Draft.CONTENT_URI;
				default:
					log("getMailboxUri(): the mail box is invalid->"+mailbox);
					return null;
			}
		}

		private Uri getAddressUri(long messageId){
			Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, messageId);
			return Uri.parse(uri + "/addr");
			
		}

		private Uri getMessageUri(long id){
			return ContentUris.withAppendedId(Mms.CONTENT_URI, id);
		}

		private Uri getPartUri(long messageId){
			Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, messageId);
			return Uri.parse(uri + "/part");
			
		}

		private void getAddress(long msgId, StringBuilder from, StringBuilder to){
			Uri uri = getAddressUri(msgId);
			Cursor addrCursor = mContentResolver.query(uri,ADDRESS_PROJECTION, null,null,null);
			while(addrCursor != null && addrCursor.moveToNext()){
				int type = addrCursor.getInt(ADDRESS_TYPE_COLUMN);
				switch(type) {
					case ADDRESS_TYPE_FROM:
						from.append(addrCursor.getString(ADDRESS_COLUMN));
						from.append(";");
						break;
					case ADDRESS_TYPE_TO:
						to.append(addrCursor.getString(ADDRESS_COLUMN));
						to.append(";");
						break;
					default:
						log("have no interest in the message type," + type);
				}
			}
			addrCursor.close();		

		}

		private boolean isEmailAddress(String address) {			
			String[] regions;
			if (address != null && (address.indexOf("@") != -1)) {				
				return true;
			}
			return false;
		}

		private String[] splitAddress(String addresses) {
			if (addresses == null) {
				return null;
			}
			String[] address = addresses.split(";");
			return address;
		}
	
	
		private String normalizeString (String text){ 
			if (text == null) {
				return null;
			}
				
			text = text.replaceAll(" ", "");
			text = text.replaceAll("-", "");
			return text;
		}
		private void log(String info){
			if (null != info){
				Xlog.v(TAG, info);
			}
		}
	
	/*	public boolean isNetworkAvailable(){
			ConnectivityManager mConnMgr = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        return mConnMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE_MMS).
                isAvailable();
	}*/	
}
