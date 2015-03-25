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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Environment;
import android.os.Message;
import android.os.ParcelUuid;
import android.provider.Telephony;
import android.provider.Telephony.Sms;
import android.provider.Telephony.MmsSms;
import android.text.TextUtils;
import android.util.LruCache;
import android.webkit.MimeTypeMap;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatListProvider;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager.WindowTagGetter;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eventlogs.EventLogData;
import com.orangelabs.rcs.provider.messaging.RichMessaging;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.R;


import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Map.Entry;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Plugin Utils
 */
public class PluginUtils {

    private static final String TAG = "PluginUtils";
    public static final String IP_GROUP_INVITATION = "com.mediatek.plugin.groupinvitation";
    public static final String ACTION_FILE_SEND = "com.mediatek.mms.ipmessage.fileTransfer";
    public static final String ACTION_MODE_CHANGE = "com.mediatek.mms.ipmessage.modeChange";

    private static final String TYPE = "type";
    private static final String OUTBOX_URI = "content://sms/outbox";
    private static final String INBOX_URI = "content://sms/inbox";
    private static final String CONTENT_URI = "content://sms/";
    private static final String FILE_SCHEMA = "file://";
    private static final String[] PROJECTION_WITH_THREAD = {EventLogData.KEY_EVENT_ROW_ID, Sms.THREAD_ID, Sms.ADDRESS, Sms.BODY};
    private static final String[] PROJECTION_ONLY_ID = {EventLogData.KEY_EVENT_ROW_ID};
    private static final String[] PROJECTION_MESSAGE_ID = {EventLogData.KEY_EVENT_MESSAGE_ID};
    private static final String SELECTION_RELOAD_IP_MSG = Sms.IPMSG_ID + ">?";
    private static final String SELECTION_IP_MSG_ID = Sms.IPMSG_ID + "=?";
    private static final String SELECTION_MMS_THREAD_ID = Sms.THREAD_ID + "=?";
    public static final int INBOX_MESSAGE = 1;
    public static final int OUTBOX_MESSAGE = 2;
    public static final int DEFAULT_MESSAGE_ID = 0x100;
    public static final long DUMMY_SIM_ID = 1L;
    private static long sSimId = DUMMY_SIM_ID;
    public static final int STATUS_IS_READ = 1;
    public static final int STATUS_IS_NOT_READ = 0;
    public static final int SIZE_K = 1024;
    private static int mMessagingUx = -1;
    private static int mMaxTextLimit = -1;
    private static final int MAX_CACHE_SIZE = 200;
    public static final Uri SMS_CONTENT_URI = Uri.parse("content://sms");
    public static final Uri THREAD_SETTINGS = Uri.parse("content://mms-sms/thread_settings/");
    private static final LruCache<String, String> SUBJECT_CACHE = new LruCache<String, String>(
            MAX_CACHE_SIZE);
	private static final String SEMI_COLON = ";";	
	private static final LruCache<String, Integer> FAILED_MESSAGE_CACHE = new LruCache<String, Integer>(
			MAX_CACHE_SIZE);

	public static void saveThreadandTag(Integer failed, String contact) {
		Logger.d(TAG, "save failed message() entry failed is " + failed
				+ " contact is " + contact);
		synchronized (FAILED_MESSAGE_CACHE) {
			FAILED_MESSAGE_CACHE.put(contact, failed);
		}		
	}

	public static Integer translateThreadId(String contact) {
		Logger.d(TAG, "getfailed message() entry contact is " + contact);
		synchronized (FAILED_MESSAGE_CACHE) {
			Integer result = FAILED_MESSAGE_CACHE.get(contact);
			Logger.d(TAG, "getfailed() result is " + result);
			if(result == null)
				return 0;		
			return result;
		}		
		
	}


    /**
     * Insert a new item into Mms DB
     * @param body The body of this item
     * @param contact The address of this item
     * @param ipMsgId The ipmsg_id of this item
     * @param messageType The type of this item
     * @return The id in Mms DB
     */
    public static Long insertDatabase(String body, String contact, int ipMsgId, int messageType) {
        return insertDatabase(body, contact, ipMsgId, messageType, 0);
    }

    public static Long insertDatabase(String body, String contact, int ipMsgId, int messageType, long targetThreadId) {
        Logger.d(TAG, "InsertDatabase(), body = " + body + "contact is " + contact + " , messageType: " + messageType
                + " , threadId: " + targetThreadId);
        Long messageIdInMms = Long.MIN_VALUE;
        String messageUri = null;
        ContentValues cv = new ContentValues();
        if(getMessagingMode() ==0)
        {
          if (!contact.startsWith(IpMessageConsts.JOYN_START)
                && !contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
            contact = IpMessageConsts.JOYN_START + contact;
           }
         }
        cv.put(Sms.ADDRESS, contact);
        cv.put(Sms.BODY, body);
        cv.put(Sms.IPMSG_ID, ipMsgId);
        cv.put(Sms.SIM_ID, sSimId);
        if (targetThreadId > 0) {
            cv.put(Sms.THREAD_ID, targetThreadId);
            ThreadTranslater.saveThreadandTag(targetThreadId, contact);
        }
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();

        if (messageType == INBOX_MESSAGE) {
            cv.put(TYPE, INBOX_MESSAGE);
            Uri inboxUri = Uri.parse(INBOX_URI);
            messageUri = contentResolver.insert(inboxUri, cv).toString();
        } else if (messageType == OUTBOX_MESSAGE) {
            cv.put(TYPE, OUTBOX_MESSAGE);
            Uri outboxUri = Uri.parse(OUTBOX_URI);
            messageUri = contentResolver.insert(outboxUri, cv).toString();
        }

        if (null == messageUri) {
            Logger.w(TAG, "InsertDatabase() messageUri is null");
            return -1L;
        }
        if (!ThreadTranslater.tagExistInCache(contact)) {
            cacheThreadIdForGroupChat(contact, contentResolver);
        } else {
            Logger.d(TAG, "InsertDatabase() contact not start group identify or not cached");
        }

        String messageId = messageUri.replace(CONTENT_URI, "");
        Logger.d(TAG, "InsertDatabase(), messageId = " + messageId);
        messageIdInMms = Long.valueOf(messageId);

        Logger.d(TAG, "InsertDatabase(), messageIdInMms = " + messageIdInMms);
        return messageIdInMms;
    }

    public static void saveTagandSubject(String tag, String subject) {
        Logger.d(TAG, "saveThreadandSubject() entry tag is " + tag + " subject is " + subject);
        SUBJECT_CACHE.put(tag, subject);
    }
    
    public static String translateTag(String tag) {
        Logger.d(TAG, "translateThreadId() entry threadId is " + tag);
        String result = SUBJECT_CACHE.get(tag);
        Logger.d(TAG, "translateThreadId() result is " + result);
        return result;
    }

    public static void removeMessagesFromMMSDatabase()
    {
    	Logger.d(TAG, "removeMessagesFromdatabase(),");
    	ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
		contentResolver
					.delete(Uri.parse("content://sms/"), "ipmsg_id>0", null);
		
    	
    }

    private static boolean cacheThreadIdForGroupChat(String contact, ContentResolver contentResolver) {
        Cursor cursor = null;
        try {
            String[] args = {
                contact
            };
            String[] projection = {
                Sms.THREAD_ID
            };
            cursor = contentResolver.query(SMS_CONTENT_URI, projection, Sms.ADDRESS + "=?",
                    args, null);

            if (cursor.moveToFirst()) {
                Long threadId = cursor.getLong(cursor.getColumnIndex(Sms.THREAD_ID));
                Logger.d(TAG, "InsertDatabase() the contact is " + contact + ", threadId is " + +threadId);
                String tag = ThreadTranslater.translateThreadId(threadId);
                if (tag == null) {
                    ThreadTranslater.saveThreadandTag(threadId, contact);
                    if(!contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER))
                    {                    	
                    	insertThreadIDInDB(threadId, getNameByNumber(contact));
                    }
                    return true;
                } else {
                    Logger.d(TAG, "InsertDatabase() the thread to tag exist " + tag);
                }
            } else {
                Logger.e(TAG, "InsertDatabase() the cursor.moveToFirst() is false");
            }

        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return false;
    }

	public static String getNameByNumber(String number) {
		Logger.d(TAG, "getNameByNumber() entry number is " + number);
		String name = number;
		if (PluginUtils.getMessagingMode() == 0) {
			if (!number.startsWith(IpMessageConsts.JOYN_START)) {
				return name;
			} else {
        		number = number.substring(4);
        	}
        }    
		if (null != number) {
			if (number
					.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
				WindowTagGetter window = PluginChatWindowManager
						.findWindowTagIndex(number);
				if (window instanceof PluginGroupChatWindow) {
					Logger.d(TAG,
							"getNameByNumber() window is PluginGroupChatWindow ");
					name = ((PluginGroupChatWindow) window).getDisplayName();
				} else {
					Logger.d(TAG,
							"getNameByNumber() window is not PluginGroupChatWindow "
									+ window);
				}
			} else {
		List<RcsContact> currentList = new ArrayList<RcsContact>(
				ContactsListManager.getInstance().CONTACTS_LIST);
				name = "";
		for (int i = 0; i < currentList.size(); i++) {
			if (currentList.get(i).mNumber.equals(number)) {
				Logger.e(TAG,
						"getNameByNumber() number is present in CONTACT_LIST"
								+ currentList.get(i).mDisplayName);
				if (name.equals(""))
					name = currentList.get(i).mDisplayName;
				else
					name = name + SEMI_COLON
							+ currentList.get(i).mDisplayName;
			}

		}

				if (name.equals("")) {
			if(RichMessaging.getInstance() == null)
						RichMessaging.createInstance(AndroidFactory
								.getApplicationContext());
					name = RichMessaging.getInstance().getContactAliasName(
							number);
					if (name != null
							&& (name.equals(ChatUtils.ANOMYNOUS_URI) || name
									.equals(""))) {
				Logger.e(TAG,
								"getNameByNumber() number is not present in CONTACT_LIST ANOMYNOUS_URI = "
										+ name);
						name = number;
					} else if (name == null)
				name = number;
			
			Logger.e(TAG,
							"getNameByNumber() number is not present in CONTACT_LIST Alias = "
									+ name);
		}
		Logger.e(TAG,
				"getNameByNumber() number not have group beginner");
			}
		} else {
			Logger.e(TAG, "getNameByNumber() number is null");
		}
		Logger.d(TAG, "getNameByNumber() exit name is " + name);
		return name;
    
    }

    public static void insertThreadIDInDB(long threadId, String groupSubject){
    	Logger.d(TAG, "insertThreadIDInDB() with threadid = " + threadId + "subject=" + groupSubject);
    	 ContentValues values = new ContentValues();
         values.put(RichMessagingData.KEY_INTEGRATED_MODE_THREAD_ID, threadId);
   	     values.put(RichMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT, groupSubject);
   	     
   	     
         try {
 			Uri uri = AndroidFactory.getApplicationContext().getContentResolver().insert(RichMessagingData.CONTENT_URI_INTEGRATED, values);
 		} catch (Exception e) {
 			// TODO Auto-generated catch block
 			e.printStackTrace();
 		}
    }
    /*
     * Store a message into mms database
     */
    public static Long storeMessageInDatabase(String messageId, String text, String remote, int messageType) {
        Logger.d(TAG, "storeMessageInDatabase() with messageId = " + messageId + " ,text = " + text
                + " ,remote = " + remote + " ,and messageType = " + messageType);
        int ipMsgId = findIdInRcseDb(messageId);
        if (-1 < ipMsgId) {
            return storeMessageInMmsDb(ipMsgId, text, remote, messageType, 0);
        } else {
            Logger.w(TAG, "storeMessageInDatabase() message not found in Rcse DB");
            return -1L;
        }
    }

    public static Long storeMessageInDatabase(String messageId, String text, String remote, int messageType, long threadId) {
        Logger.d(TAG, "storeMessageInDatabase() with messageId = " + messageId + " ,text = " + text
                + " ,remote = " + remote + " ,and messageType = " + messageType);
        int ipMsgId = findIdInRcseDb(messageId);
        if (-1 < ipMsgId) {
            return storeMessageInMmsDb(ipMsgId, text, remote, messageType, threadId);
        } else {
            Logger.w(TAG, "storeMessageInDatabase() message not found in Rcse DB");
            return -1L;
        }
    }

    static long storeMessageInMmsDb(int ipMsgId, String text, String remote, int messageType, long threadId) {
            long idInMmsDb = getIdInMmsDb(ipMsgId);
            if (-1 != idInMmsDb) {
                Logger.d(TAG, "storeMessageInDatabase() message found in Mms DB, id: " + idInMmsDb);
                return idInMmsDb;
            } else {
                Logger.d(TAG, "storeMessageInDatabase() message not found in Mms DB");
                if (TextUtils.isEmpty(remote)) {
                    Logger.w(TAG, "storeMessageInDatabase() invalid remote: " + remote);
                    return -1L;
                }
                if (remote.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                    return insertDatabase(text, remote, ipMsgId, messageType, threadId);
                } else {
                    final String contact = PhoneUtils.extractNumberFromUri(remote);
                    return insertDatabase(text, contact, ipMsgId, messageType, threadId);
                }
            }
    }

    static boolean updatePreSentMessageInMmsDb(String messageId, int messageTag) {
        int ipMsgId = findIdInRcseDb(messageId);
        if (-1 < ipMsgId) {
            ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
            ContentValues values = new ContentValues();
            values.put(Sms.IPMSG_ID, ipMsgId);
            int count = contentResolver.update(SMS_CONTENT_URI, values, Sms.IPMSG_ID + "=?", new String[] {
                Integer.toString(messageTag)
            });
            Logger.d(TAG, "updatePreSentMessageInMmsDb() messageId: " + messageId + " ,messageTag: " + messageTag
                    + " ,count: " + count);
            return count == 1;
        } else {
            Logger.w(TAG, "updatePreSentMessageInMmsDb() ");
            return false;
        }
    }

    static long getIdInMmsDb(int ipMsgId) {
        Logger.d(TAG, "getIdInMmsDb() entry, ipMsgId: " + ipMsgId);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();

        Cursor cursor = null;
        try {
            final String[] args = {Integer.toString(ipMsgId)};
            cursor = contentResolver.query(SMS_CONTENT_URI, PROJECTION_WITH_THREAD, SELECTION_IP_MSG_ID, args, null);
            if (cursor.moveToFirst()) {
                long mmsDbId = cursor.getLong(cursor.getColumnIndex(Sms._ID));
                long threadId = cursor.getLong(cursor.getColumnIndex(Sms.THREAD_ID));
                String contact = cursor.getString(cursor.getColumnIndex(Sms.ADDRESS));
                Logger.d(TAG, "getIdInMmsDb() contact is " + contact + " threadId is " + threadId);
                if (contact != null
                        && contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                    String tag = ThreadTranslater.translateThreadId(threadId);
                    if (tag == null) {
                        Logger.d(TAG, "getIdInMmsDb() the thread to tag not exist ");
                        ThreadTranslater.saveThreadandTag(threadId, contact);
                    } else {
                        Logger.d(TAG, "getIdInMmsDb() the thread to tag exist " + tag);
                    }
                }
                Logger.d(TAG, "getIdInMmsDb() mmsDbId: " + mmsDbId);
                return mmsDbId;
            } else {
                Logger.d(TAG, "getIdInMmsDb() empty cursor");
                return -1l;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    /**
     * Initiate export complete chat text for one 2one chat
     * 
     * @param threadId Thread id for the chat in MMS DB
     * 
     */
    
    public static void initiateExportChat(long threadId) {
        Logger.d(TAG, "getIdInMmsDb() entry, threadId: " + threadId);
        String contact = "";
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
        String messageList = "";
        Cursor cursor = null;
        try {
            final String[] args = {Long.toString(threadId)};
            cursor = contentResolver.query(SMS_CONTENT_URI, PROJECTION_WITH_THREAD, SELECTION_MMS_THREAD_ID, args, null);
            Logger.d(TAG, "getIdInMmsDb() nonEmpty cursor count = " + cursor.getCount());
			if (cursor != null && cursor.moveToFirst()) {
				do {
					String message = cursor.getString(cursor
							.getColumnIndex(Sms.BODY));
					contact = cursor.getString(cursor
							.getColumnIndex(Sms.ADDRESS));
					Logger.d(TAG, "getIdInMmsDb() contact is " + contact
							+ " message is " + message);
					if(!message.equals(getStringInRcse(R.string.file_transfer_title)))
                messageList = messageList + "\n" + message;
				} while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "getIdInMmsDb() empty cursor");              
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        if(getMessagingMode() == 0)
        {
        	if (contact.startsWith(IpMessageConsts.JOYN_START)) {
            contact = contact.substring(4);
        	}
        }
		if (cursor != null && !messageList.equals("")) {
			String fileName = Environment.getExternalStorageDirectory()
					.getPath() + "/" + "Joyn" + "/" + contact + "_chat" + ".txt";
			File chatContentFile = null;
			try {
				chatContentFile = new File(fileName);
				chatContentFile.createNewFile();
				FileOutputStream fOut = new FileOutputStream(chatContentFile);
				OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);
				myOutWriter.append(messageList);
				myOutWriter.close();
				fOut.close();
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if (chatContentFile != null) {
				Uri uri = Uri.fromFile(chatContentFile);
				Intent shareIntent = new Intent();
				shareIntent.setAction(Intent.ACTION_SEND);
				shareIntent.putExtra(Intent.EXTRA_STREAM, uri);
				shareIntent.setType("text/plain");
				shareIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				AndroidFactory.getApplicationContext().startActivity(
						shareIntent);
			}
		}
    }

    static int findIdInRcseDb(String msgId) {
        Logger.d(TAG, "findIdInRcseDb() entry, msgId: " + msgId);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();

        if (TextUtils.isEmpty(msgId)) {
            Logger.e(TAG, "findIdInRcseDb(), invalid msgId: " + msgId);
            return DEFAULT_MESSAGE_ID;
        }
        String[] argument = {msgId};
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(RichMessagingData.CONTENT_URI, 
                    PROJECTION_ONLY_ID, EventLogData.KEY_EVENT_MESSAGE_ID + "=?", argument, null);
            if (null != cursor && cursor.moveToFirst()) {
                int rowId =  cursor.getInt(cursor.getColumnIndex(EventLogData.KEY_EVENT_ROW_ID));
                Logger.d(TAG, "findIdInRcseDb() row id for message: " + msgId + ", is " + rowId);
                return rowId;
            } else {
                Logger.w(TAG, "findIdInRcseDb() invalid cursor: " + cursor);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return -1;
    }

    static String findMessageTagInRcseDb(String msgId) {
        Logger.d(TAG, "findIdInRcseDb() entry, msgId: " + msgId);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();

        String[] argument = {msgId};
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(RichMessagingData.CONTENT_URI, 
            		PROJECTION_MESSAGE_ID, EventLogData.KEY_EVENT_ROW_ID + "=?", argument, null);
            if (null != cursor && cursor.moveToFirst()) {
                String messageTag =  cursor.getString(cursor.getColumnIndex(EventLogData.KEY_EVENT_MESSAGE_ID));
                Logger.d(TAG, "findIdInRcseDb() row id for message: " + msgId + ", is " + messageTag);
                return messageTag;
            } else {
                Logger.w(TAG, "findIdInRcseDb() invalid cursor: " + cursor);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return null;
    }

    /*package*/ static void reloadRcseMessages() {
        Logger.d(TAG, "reloadRcseMessages() entry");
        PluginGroupChatWindow.removeAllGroupChatInvitationsInMms();
        
        ArrayList<Integer> messageIdArray = new ArrayList<Integer>();
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
        Cursor cursor = null;
        ConcurrentHashMap<String, ArrayList<Integer>> groupMap = new ConcurrentHashMap<String, ArrayList<Integer>>();
        try {
            final String[] args = {"0"};
            cursor = contentResolver.query(SMS_CONTENT_URI, null, SELECTION_RELOAD_IP_MSG, args, null);
            if (cursor.moveToFirst()) {
                do {
                    int id = cursor.getInt(cursor.getColumnIndex(Sms.IPMSG_ID));
                    String address = cursor.getString(cursor.getColumnIndex(Sms.ADDRESS));
                    Logger.d(TAG, "reloadRcseMessages() ipMsgId: " + id + " , address: " + address);
                    if (address != null
                            && address
                                    .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                        if (groupMap.containsKey(address)) {
                            ArrayList<Integer> messageList = groupMap.get(address);
                            messageList.add(id);
                        } else {
                            ArrayList<Integer> messageList = new ArrayList<Integer>();
                            messageList.add(id);
                            groupMap.put(address, messageList);
                        }
                    } else {
                        if (id == Integer.MAX_VALUE || id == DEFAULT_MESSAGE_ID) {
                            Logger.d(TAG, "reloadRcseMessages(), it's a pending file transfer or invalid message," +
                                    " no need to rebuild!");
                            int smsId = cursor.getInt(cursor.getColumnIndex(Sms._ID));
                            contentResolver.delete(Uri.parse(SMS_CONTENT_URI + "/" + smsId), null,
                                    null);
                        } else {
                            Logger.d(TAG, "reloadRcseMessages(), need to rebuild!");
                            messageIdArray.add(id);
                        }
                    }
                } while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "reloadRcseMessages() cursor is empty");
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        Logger.i(TAG, "reloadRcseMessages() " + messageIdArray.size() + " Rcse message found: "
                + messageIdArray);
        PluginController.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE, messageIdArray)
                .sendToTarget();
        Set<Entry<String, ArrayList<Integer>>> tagSet = groupMap.entrySet();
        for (Entry<String, ArrayList<Integer>> tag : tagSet) {
            ArrayList<Integer> messageList = tag.getValue();
            Logger.i(TAG, "reloadRcseMessages() tag is  " + tag + " messageList: " + messageList);
            PluginController.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE, tag.getKey(),
                    messageList).sendToTarget();
        }
    }

    static int updateMessageIdInMmsDb(String oldId, String newId) {
        Logger.d(TAG, "updateMessageIdInMmsDb() entry");
        int idInRcse = findIdInRcseDb(newId);
        long idInMms = IpMessageManager.getMessageId(oldId);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
        if (contentResolver != null && idInMms != -1) {
            Uri uri = Uri.parse(SMS_CONTENT_URI + "/" + idInMms);
            ContentValues contentValues = new ContentValues();
            contentValues.put(Sms.IPMSG_ID, idInRcse);
            contentResolver.update(uri, contentValues, null, null);
        } else {
            Logger.e(TAG, "getIdInMmsDb(), cr is null!");
        }
        return idInRcse;
    }

    static boolean onViewFileDetials(String filePath, Context context) {
        Intent intent = generateFileDetailsIntent(filePath);
        if (intent != null) {
            context.startActivity(intent);
            return true;
        } else {
            Logger.w(TAG, "onViewFileDetials() intent is null");
            return false;
        }
    }

    private static Intent generateFileDetailsIntent(String filePath) {
        Logger.d(TAG, "generateFileDetailsIntent() entry, filePath: " + filePath);
        if (!TextUtils.isEmpty(filePath)) {
            Uri fileUri;
            if (!filePath.startsWith(FILE_SCHEMA)) {
                fileUri = Uri.parse(FILE_SCHEMA + filePath);
            } else {
                fileUri = Uri.parse(filePath);
            }
            String mimeType = AsyncGalleryView.getMimeType(filePath);
            if (null != mimeType) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(fileUri, mimeType);
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                        | Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                return intent;
            } else {
                Logger.e(TAG, "generateFileDetailsIntent() mMimeType is null");
            }
        }
        return null;
    }

    /**
     * Initialize sim id from Telephony
     * 
     * @param context Any context will be ok
     */
    public static void initializeSimIdFromTelephony(Context context) {
        Logger.d(TAG, "initializeSimIdFromTelephony() entry");
        int slotCount = getSimSlotCount(context);
        if (slotCount > 0) {
            List<SimInfoRecord> simInfos = SimInfoManager.getAllSimInfoList(context);
            int currentSlot = 0;
            while (currentSlot < slotCount) {
            	SimInfoRecord simInfo = SimInfoManager.getSimInfoBySlot(context, currentSlot);
                if (simInfo != null) {
                    sSimId = simInfo.mSimInfoId;
                    Logger.d(TAG, "initializeSimIdFromTelephony() slot " + currentSlot
                            + " simId is " + sSimId);
                    break;
                } else {
                    Logger.d(TAG, "initializeSimIdFromTelephony() slot " + currentSlot
                            + " simInfo is null");
                }
                currentSlot += 1;
            }
        } else {
            Logger.e(TAG, "initializeSimIdFromTelephony() simCount must more than 0, now is "
                    + slotCount);
        }
        // Error handle if get wrong sim id
        if (sSimId <= 0) {
            Logger.e(TAG, "initializeSimIdFromTelephony() get wrong simId");
            sSimId = DUMMY_SIM_ID;
        }
        Logger.d(TAG, "initializeSimIdFromTelephony() exit simId is " + sSimId);
    }

    /**
     * Get simcard slot count
     * 
     * @param context any context will be ok
     * @return the number of slot count
     */
    private static int getSimSlotCount(Context context) {
        int simCount = SimInfoManager.getAllSimCount(context);
        Logger.d(TAG, "getSimSlotCount() simCount is " + simCount);
        return simCount;
    }
    

    /**
     * Return the string value associated with a particular resource ID in RCSe.
     * 
     * @param resourceId The desired resource identifier, as generated by the
     *            aapt tool.
     * @return The string data associated with the resource, stripped of styled
     *         text information.
     */
    public static String getStringInRcse(int resourceId) {
        Resources resource = null;
        String string = null;
        try {
            resource =
                    AndroidFactory.getApplicationContext().getPackageManager()
                            .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        if (null != resource) {
            string = resource.getString(resourceId);
        } else {
            Logger.e(TAG, "getStringInRcse(), resource is null!");
        }
        return string;
    }
    
    /**
     * Return whether file transfer is supported.
     * 
     * @param number The number whose capability is to be queried.
     * @return True if file transfer is supported, else false.
     */
    public static boolean isFtSupportedInRcse(String contact) {
        Logger.d(TAG, "isFtSupportedInRcse() entry contact is " + contact);
        boolean support = false;
        PluginApiManager apiManager = PluginApiManager.getInstance();
        if (null != apiManager) {
            if (apiManager.isFtSupported(contact)) {
                support = true;
            } else {
                support = false;
            }
        } else {
            support = false;
        }
        Logger.d(TAG, "isFtSupportedInRcse() exit support is " + support);
        return support;
    }


    /**
     * Return the file transfer IpMessage
     * 
     * @param remote remote user
     * @param FileStructForBinder file strut
     * @return The file transfer IpMessage
     */
    public static IpMessage analysisFileType(String remote, FileStructForBinder fileTransfer) {
        String fileName = fileTransfer.fileName;
        if (fileName != null) {
            String mimeType = MediaFile.getMimeTypeForFile(fileName);
            if (mimeType == null) {
                mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                        Utils.getFileExtension(fileName));
            }
            if (mimeType != null) {
                if (mimeType.contains(Utils.FILE_TYPE_IMAGE)) {
                    return new PluginIpImageMessage(fileTransfer, remote);
                } else if (mimeType.contains(Utils.FILE_TYPE_AUDIO)
                        || mimeType.contains("application/ogg")) {
                    return new PluginIpVoiceMessage(fileTransfer, remote);
                } else if (mimeType.contains(Utils.FILE_TYPE_VIDEO)) {
                    return new PluginIpVideoMessage(fileTransfer, remote);
                } else if (fileName.toLowerCase().endsWith(".vcf")) {
                    return new PluginIpVcardMessage(fileTransfer, remote);
                } else {
                    // Todo
                    Logger.d(TAG, "analysisFileType() other type add here!");
                }
            }
        } else {
            Logger.w(TAG, "analysisFileType(), file name is null!");
        }
        return new PluginIpAttachMessage(fileTransfer, remote);
    }

    public static int getMessagingMode()
    {
		if (mMessagingUx == -1) {
			mMessagingUx = RcsSettings.getInstance().getMessagingUx();							
		}
		return mMessagingUx;
    }

    public static int getMaximumTextLimit()
    {
    	if (mMaxTextLimit == -1) {
    		Logger.d(TAG, "getMaximumTextLimit() DB entry, ");
    		mMaxTextLimit = RcsSettings.getInstance().getMaxChatMessageLength();							
		}
    	Logger.d(TAG, "getMaximumTextLimit() exit, limit = " + mMaxTextLimit);
		return mMaxTextLimit;
    }

    /**
     * Return the file transfer IpMessage
     * 
     * @param IpMessage Ipmessage in mms defined
     * @return IpMessage Ipmessage in plugin defined
     */
    public static IpMessage exchangeIpMessage(int messageType, FileStructForBinder fileStruct,
            String remote) {
        String newTag = fileStruct.fileTransferTag;
        Logger.d(TAG, "exchangeIpMessage() entry, newTag = " + newTag);
        switch (messageType) {
            case IpMessageConsts.IpMessageType.PICTURE:
                PluginIpImageMessage ipImageMessage = new PluginIpImageMessage(fileStruct, remote);
                ipImageMessage.setTag(newTag);
                return ipImageMessage;
            case IpMessageConsts.IpMessageType.VOICE:
                PluginIpVoiceMessage ipVoiceMessage = new PluginIpVoiceMessage(fileStruct, remote);
                ipVoiceMessage.setTag(newTag);
                return ipVoiceMessage;
            case IpMessageConsts.IpMessageType.VIDEO:
                PluginIpVideoMessage ipVideoMessage = new PluginIpVideoMessage(fileStruct, remote);
                ipVideoMessage.setTag(newTag);
                return ipVideoMessage;
            case IpMessageConsts.IpMessageType.VCARD:
                PluginIpVcardMessage ipVcardMessage = new PluginIpVcardMessage(fileStruct, remote);
                ipVcardMessage.setTag(newTag);
                return ipVcardMessage;
            default:
                PluginIpAttachMessage ipAttachMessage = new PluginIpAttachMessage(fileStruct,
                        remote);
                ipAttachMessage.setTag(newTag);
                return ipAttachMessage;
        }
    }
}
