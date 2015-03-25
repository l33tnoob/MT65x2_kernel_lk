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
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.net.Uri;
import android.provider.Telephony;
import android.provider.Telephony.ThreadSettings;
import android.text.TextUtils;

import com.mediatek.mms.ipmessage.ContactManager;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.IntegrationMode;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager.WindowTagGetter;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.binder.Cacher;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.RichMessaging;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import android.provider.Telephony.MmsSms;


/**
 * Provide contact management related interface
 */
public class IpMessageContactManager extends ContactManager {

    private static final String TAG = "IpMessageContactManager";
    private static final Map<String, Integer> STATUS_MAP = new ConcurrentHashMap<String, Integer>();
    private static final Map<String, Integer> NON_RCSE_MAP = new ConcurrentHashMap<String, Integer>();
    private static final Map<String, String> NAME_NUMBER_MAP = new ConcurrentHashMap<String, String>();
    private static final Map<Long, String> NAME_THREAD_MAP = new ConcurrentHashMap<Long, String>();
    private static final String DEFAULT_GROUP_NAME = "Joyn chat";
    private static final Map<String, Integer> STRANGER_MAP = new ConcurrentHashMap<String, Integer>();
    private static final Map<String, Integer> ISLOCAL_MAP = new ConcurrentHashMap<String, Integer>();
    private static final Bitmap DEFAULT_BITMAP;
    private static final int PARTICIPANT_NUM_ZERO = 0;
    private static final int PARTICIPANT_NUM_ONE = 1;
    private static final int PARTICIPANT_NUM_TWO = 2;
	private static final String SEMI_COLON = ";";
    private final Cacher<Short, String> mContactIdTranslater = new Cacher<Short, String>();

    static {
        Resources resource = null;
        try {
            resource = AndroidFactory.getApplicationContext().getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        if (null != resource) {
            Logger.d(TAG, "ImageLoader resource is not null");
            DEFAULT_BITMAP = BitmapFactory.decodeResource(resource, R.drawable.default_header);
        } else {
            Logger.d(TAG, "ImageLoader resource is  null");
            DEFAULT_BITMAP = null;
        }
    }

    /**
     * Save the status of a special Rcse contact. Used in package.
     * 
     * @param number The phone number.
     * @param status The status of the Rcse contact.
     */
    static void putStatusByNumber(String number, Integer status) {
        Logger.d(TAG, "putStatusByNumber() entry with number is " + number + " and the status is "
                + status);
        STATUS_MAP.put(number, status);
    }
    
    static void putJoynStatusByNumber(String number, Integer status) {
        Logger.d(TAG, "putJoynStatusByNumber() entry with number is " + number + " and the status is "
                + status);
        NON_RCSE_MAP.put(number, status);
    }
    
    static void putStrangerByNumber(String number, Integer status) {
        Logger.d(TAG, "putStatusByNumber() entry with number is " + number + " and the status is "
                + status);
        STATUS_MAP.put(number, status);
    }
    
    static void putLocalByNumber(String number, Integer status) {
        Logger.d(TAG, "putStatusByNumber() entry with number is " + number + " and the status is "
                + status);
        STATUS_MAP.put(number, status);
    }

    public static void putNameByNumber(String name, String number) {
        Logger.d(TAG, "putNameByNumber() entry with number is " + number + " and the name is "
                + name);
        NAME_NUMBER_MAP.put(number, name);       
    }
    
    public static void putNameByThreadId(String name, Long thread) {
        Logger.d(TAG, "putNameByThreadId() entry with thread is " + thread + " and the name is "
                + name);
        NAME_THREAD_MAP.put(thread, name);
    }


    public IpMessageContactManager(Context context) {
        super(context);
    }

    @Override
    public Bitmap getAvatarByNumber(String number) {
        Logger.d(TAG, "getAvatarByNumber() entry number is " + number);
        WindowTagGetter window = PluginChatWindowManager.findWindowTagIndex(number);
        if (window instanceof PluginGroupChatWindow) {
            return ((PluginGroupChatWindow) window).getAvatarBitmap();
        } else {
            return null;
        }
    }
    
    @Override
    public String getNumberByEngineId(short groupId) {
        return PluginChatWindowManager.getNumberByEngineId(groupId);
    }

    @Override
    public Bitmap getAvatarByThreadId(long threadId) {
        Logger.d(TAG, "getAvatarByThreadId() entry threadId is " + threadId);
        String tag = ThreadTranslater.translateThreadId(threadId);
        WindowTagGetter window = PluginChatWindowManager.findWindowTagIndex(tag);
        if (window instanceof PluginGroupChatWindow) {
            Logger.d(TAG, "getAvatarByThreadId() window is  PluginGroupChatWindow ");
            return ((PluginGroupChatWindow) window).getAvatarBitmap();
        } else {
            Bitmap avatar = null;
            if (DEFAULT_BITMAP != null) {
                Logger.d(TAG, "getAvatarByThreadId() get default group chat avatar");
                List<Bitmap> bitMapList = new ArrayList<Bitmap>();
                bitMapList.add(DEFAULT_BITMAP);
                bitMapList.add(DEFAULT_BITMAP);
                bitMapList.add(DEFAULT_BITMAP);
                avatar = processBitmaps(bitMapList);
            } else {
                Logger.e(TAG, "getAvatarByThreadId() DEFAULT_BITMAP is null");
            }
            Logger.d(TAG, "getAvatarByThreadId() window is  not PluginGroupChatWindow ");
            return DEFAULT_BITMAP;
        }
    }
    
    private Bitmap processBitmaps(List<Bitmap> bitMapList) {
        Logger.d(TAG, "processBitmaps() entry");
        Bitmap one = bitMapList.get(PARTICIPANT_NUM_ZERO);
        Bitmap two = bitMapList.get(PARTICIPANT_NUM_ONE);
        Bitmap three = bitMapList.get(PARTICIPANT_NUM_TWO);
        if (one == null || two == null || three == null) {
            Logger.e(TAG, "processBitmaps() one/two/three is/are invalid!");
            return null;
        }

        if (one.getWidth() < 96 || one.getHeight() < 96) {
            one = resizeImage(one, 96, 96, false);
        }

        int block = one.getWidth() / 16;

        one = Bitmap.createBitmap(one, block * 4, 0, block * 9, one.getHeight());
        two = resizeImage(two, block * 7, one.getHeight() / 2, false);
        three = resizeImage(three, block * 7, one.getHeight() / 2, false);

        Bitmap newbmp = Bitmap.createBitmap(one.getWidth() + 1 + two.getWidth(),
                one.getHeight(), Config.ARGB_8888);
        Canvas canvas = new Canvas(newbmp);
        canvas.drawBitmap(one, 0, 0, null);
        canvas.drawBitmap(two, one.getWidth() + 1, 0, null);
        canvas.drawBitmap(three, one.getWidth() + 1, two.getHeight() + 1, null);
        canvas.save(Canvas.ALL_SAVE_FLAG);
        canvas.restore();
        one.recycle();
        two.recycle();
        three.recycle();
        return newbmp;
    }

    private Bitmap resizeImage(Bitmap bitmap, int w, int h, boolean needRecycle) {
        Logger.d(TAG, "resizeImage() entry ");
        if (null == bitmap) {
            return null;
        }
        Bitmap bitmapOrg = bitmap;
        int width = bitmapOrg.getWidth();
        int height = bitmapOrg.getHeight();
        int newWidth = w;
        int newHeight = h;

        float scaleWidth = ((float) newWidth) / width;
        float scaleHeight = ((float) newHeight) / height;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);

        Bitmap resizedBitmap = Bitmap
                .createBitmap(bitmapOrg, 0, 0, width, height, matrix, true);
        if (needRecycle && !bitmapOrg.isRecycled()) {
            bitmapOrg.recycle();
        }
        return resizedBitmap;
    }
    
    public int getIntegratedModeForContact(String number)
    {
    	Logger.d(TAG, "getIntegratedModeForContact() entry number is " + number);
    	CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
        if (capabilityApi != null) {
            Capabilities capability = capabilityApi.getContactCapabilities(number);
            if (capability != null) {
            	if(capability.isIntegeratedMessagingMode())
            	{
            		Logger.d(TAG, "getIntegratedModeForContact() Fully Integarted Mode");
    	return IpMessageConsts.IntegrationMode.FULLY_INTEGRATED;
    }
 
            }
         }
    	return IpMessageConsts.IntegrationMode.CONVERGED_INBOX;
    }
    @Override
    public String getNameByNumber(String number) {
        Logger.d(TAG, "getNameByNumber() entry number is " + number);
                       
        String name = number;
        if(PluginUtils.getMessagingMode() == 0)
{
        if (!number.startsWith(IpMessageConsts.JOYN_START)) {
            return name;
        } else {
            number = number.substring(4);
        }
}
        if (NAME_NUMBER_MAP.containsKey(number)) {
            String displayName = NAME_NUMBER_MAP.get(number);
            Logger.d(TAG, "getNameByNumber() map exit with name " + displayName);           
            return displayName;
        }
        if (null != number) {
            if (number.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                WindowTagGetter window = PluginChatWindowManager.findWindowTagIndex(number);
                if (window instanceof PluginGroupChatWindow) {
                    Logger.d(TAG, "getNameByNumber() window is  PluginGroupChatWindow ");
                    name = ((PluginGroupChatWindow) window).getDisplayName();
                } else {
                    Logger.d(TAG, "getNameByNumber() window is  not PluginGroupChatWindow "
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
						if(name.equals(""))
						name = currentList.get(i).mDisplayName;
						else
						name = name + SEMI_COLON
								+ currentList.get(i).mDisplayName;
					}
				}
				if (name.equals("")) {
					if(RichMessaging.getInstance() == null)
						RichMessaging.createInstance(AndroidFactory.getApplicationContext());
					name = RichMessaging.getInstance().getContactAliasName(number);
					if(name!=null && name.equals(ChatUtils.ANOMYNOUS_URI))
					{
						Logger.e(TAG,
								"getNameByNumber() number is not present in CONTACT_LIST ANOMYNOUS_URI = " + name);
						name = number;
					}
					else if(name == null)
						name = number;
					Logger.e(TAG,
							"getNameByNumber() number is not present in CONTACT_LIST Alias = " + name);
				}
				Logger.e(TAG,
						"getNameByNumber() number not have group beginner");
            }
        } else {
            Logger.e(TAG, "getNameByNumber() number is null");
        }
        Logger.d(TAG, "getNameByNumber() exit name is " + name);
        if(name.equals(""))
        	name = number;
        putNameByNumber(name, number);       
        return name;
    }

    @Override
    public String getNameByThreadId(long threadId) {
        Logger.d(TAG, "getNameByThreadId() entry threadId is " + threadId);
        String tag = ThreadTranslater.translateThreadId(threadId);
        if (NAME_THREAD_MAP.containsKey(threadId)) {
            String displayName = NAME_THREAD_MAP.get(threadId);
            Logger.d(TAG, "getNameByThreadId() exit map with name " + displayName);
            return displayName;
        }
        String groupSubject = DEFAULT_GROUP_NAME;      
        
        //GET GROUP SUBJECT BY THERAD ID
        groupSubject = getSubjectByID(threadId);      
        WindowTagGetter window = PluginChatWindowManager.findWindowTagIndex(tag);        
        Logger.d(TAG, "getNameByThreadId subejct & window are " + groupSubject +"&" + window);
        if(groupSubject!=null && !groupSubject.equals(""))
        {
        	if(PluginUtils.getMessagingMode() == 0)
            {
            	if (groupSubject.startsWith(IpMessageConsts.JOYN_START)) {
            		groupSubject = groupSubject.substring(4);
            	}
            }        
            Logger.d(TAG, "getNameByThreadId() subject is " + groupSubject);
            putNameByThreadId(groupSubject, threadId);
        	return groupSubject;
        }        
        if (window instanceof PluginGroupChatWindow) {
            Logger.d(TAG, "getNameByThreadId() window is  PluginGroupChatWindow ");
            return ((PluginGroupChatWindow) window).getDisplayName();
        } else {
            Logger.d(TAG, "getNameByThreadId() window is  not PluginGroupChatWindow " + window);
            return DEFAULT_GROUP_NAME;
        }
    }

   // GET GROUP SUBJECT BY THERAD ID
    private String getSubjectByID(long threadID){
    	 Cursor cursor = null;
    	 String grpSubject = "";
    
    	 try {
			cursor = AndroidFactory.getApplicationContext().getContentResolver().
			            query(RichMessagingData.CONTENT_URI_INTEGRATED, null, 
			            "" + RichMessagingData.KEY_INTEGRATED_MODE_THREAD_ID + " = " + threadID +" ",
			         null ,
			         null
			        );

			if(cursor!=null && cursor.moveToFirst()){
			   	
				grpSubject = cursor.getString(cursor.getColumnIndex(RichMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT));
			}
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	 finally{
    		 if(cursor!=null){
    			 cursor.close();
    			 cursor = null;
    		 }
    	 }
    	 
		return grpSubject;
    }
    
    /**
     * Get contact id by a specific phone number
     * 
     * @param number The phone number of the contact id
     * @return The contact id
     */
    public short getContactIdByNumber(String number) {
        Logger.d(TAG, "getContactIdByNumber() entry with number " + number);
        short contactId = ContactsListManager.getInstance().getContactIdByNumber(number);
        Logger.d(TAG, "getContactIdByNumber() contactId: " + contactId);
        return generateOrStoreContactId(contactId, number);
    }

    private short generateOrStoreContactId(short contactId, String number) {
        Logger.d(TAG, "generateOrStoreContactId() entry, contactId: " + contactId + ", number: " + number);
        if (contactId > 0) {
            Logger.d(TAG, "generateOrStoreContactId() local contact");
            if (null == mContactIdTranslater.getValue(contactId)) {
                Logger.d(TAG, "generateOrStoreContactId() store local contact number: " + number);
                mContactIdTranslater.putValue(contactId, number);
            }
            return contactId;
        } else {
            short hashcode = (short) number.hashCode();
            hashcode = (short) (hashcode > 0 ? (-hashcode) : hashcode);
            Logger.d(TAG, "generateOrStoreContactId() hashcode: " + hashcode);
            if (null == mContactIdTranslater.getValue(hashcode)) {
                Logger.d(TAG, "generateOrStoreContactId() store non-local contact number" + number);
                mContactIdTranslater.putValue(hashcode, number);
            }
            return hashcode;
        }
    }

    /**
     * Get stranger status for a specific phone number
     * 
     * @param number The phone number of the contact id
     * @return true if it is stranger
     */
    
    public boolean isStrangerContact(String number){
    	ContactsListManager contactListManager = ContactsListManager.getInstance();
    	Logger.d(TAG, "isStrangerContact() number is " + number);
    	if(PluginUtils.getMessagingMode() == 0)
    	{    
    	if (!number.startsWith(IpMessageConsts.JOYN_START)) {
    	            return false;
    	        } else {
    	            number = number.substring(4);
    	        }
    	}
    	if (contactListManager.isStranger(number)) {
            Logger.d(TAG, "isStrangerContact() isStranger " + number);
            return true;
    	}else if (contactListManager.isLocalContact(number)) {
            Logger.d(TAG, "isStrangerContact() isLocalContact " + number);
            return false;        
        }
    	return true;
    }

    @Override
    public boolean isIpMessageNumber(String number) {
        Logger.d(TAG, "isIpMessageNumber() entry number is " + number);
		if (PluginUtils.getMessagingMode() == 0) {
if (!number.startsWith(IpMessageConsts.JOYN_START)) {
            return false;
        } else {
            number = number.substring(4);
        }
}
        ContactsListManager contactListManager = ContactsListManager.getInstance();
        if (contactListManager != null) {
            if (contactListManager.isLocalContact(number)) {
                Logger.d(TAG, "isIpMessageNumber() isLocalContact ");
                return true;
            } else if (contactListManager.isStranger(number)) {
                Logger.d(TAG, "isIpMessageNumber() isStranger ");
                return true;
            } else if(NON_RCSE_MAP.containsKey(number))
            {     
            	Logger.d(TAG, "isIpMessageNumber() present in non rcse map " + number);	
                    int status = NON_RCSE_MAP.get(number);
                    {
                    	if(status ==1)
                    		return false;
                    }              
            }
            else {
                CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
                if (capabilityApi != null) {
                    Capabilities capability = capabilityApi.getContactCapabilities(number);
                    if (capability != null) {
                        if (capability.isSupportedRcseContact()) {
                            ContactsListManager.getInstance().setStrangerList(number, true);
                            Logger.d(TAG, "isIpMessageNumber() capability is rcse contact ");
                            return true;
                        } else {
                        	
                            Logger.d(TAG, "isIpMessageNumber() the number not Rcse Contact");
                        }
                    }
                }
            }
        } else {
            Logger.e(TAG, "isIpMessageNumber() the contactListManager is null");
        }
        Logger.d(TAG, "isIpMessageNumber() number is not rcse contact ");
        if(NON_RCSE_MAP.containsKey(number))
    		NON_RCSE_MAP.remove(number);
    	NON_RCSE_MAP.put(number, 1);
        return false;
    }

    /**
     * Get status by a specific phone number
     * 
     * @param number The phone number of the contact id
     * @return The status
     */
    public int getStatusByNumber(String number) {
        Logger.d(TAG, "getStatusByNumber() entry with number " + number);
		if (PluginUtils.getMessagingMode() == 0) {
           if (!number.startsWith(IpMessageConsts.JOYN_START)) {
            return IpMessageConsts.ContactStatus.OFFLINE;
             } else {
            number = number.substring(4);
             }

        }
        
        if (!TextUtils.isEmpty(number) && STATUS_MAP.containsKey(number)) {
            int status = STATUS_MAP.get(number);
            Logger.d(TAG, "getStatusByNumber() exit with status " + status);
            return status;
        } else {
            Logger.d(TAG,
                    "getStatusByNumber() the target number is not in the cache, return default value");
            return IpMessageConsts.ContactStatus.OFFLINE;
        }
    }

    @Override
    public boolean addContactToSpamList(int[] contactIds) {
        return markThreadAsSpam(contactIds, true);
    }

    @Override
    public String getNumberByMessageId(long messageId) {
        Logger.d(TAG, "getNumberByMessageId() messageId = " + messageId);
        IpMessage message = IpMessageManager.getMessage(messageId);
        if (message != null) {
        	String contact = message.getFrom();
        	if(PluginUtils.getMessagingMode() ==0)
            {
              if (!contact.startsWith(IpMessageConsts.JOYN_START)
                    && !contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                contact = IpMessageConsts.JOYN_START + contact;
               }
             }
            return contact;
        } else {
            return super.getNumberByMessageId(messageId);
        }
    }

    @Override
    public boolean deleteContactFromSpamList(int[] contactIds) {
        return markThreadAsSpam(contactIds, false);
    }

    // MMS will call to get blocked time of the contact
    public long getSpamTime(String number)
    {
    	long spamTime = System.currentTimeMillis();;
    	//ContactsManager.getInstance().getTimeStampForBlockedContact(number);
        return spamTime;
    }

    private boolean markThreadAsSpam(int[] contactIds, boolean isSpam) {
        if (null == contactIds || contactIds.length < 1) {
            Logger.w(TAG, "markThreadAsSpam() invalid thread id array");
            return false;
        }
        ContentResolver resolver = mContext.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(ThreadSettings.SPAM, isSpam ? 1 : 0);
        values.put(ThreadSettings.NOTIFICATION_ENABLE, isSpam ? 0 : 1);
        values.put(ThreadSettings.MUTE, isSpam ? 1 : 0);
        StringBuilder whereStringBuilder = new StringBuilder();
        ArrayList<String> threadIdList = new ArrayList<String>();
        for (int contactId : contactIds) {
            whereStringBuilder.append(" OR " + ThreadSettings.THREAD_ID + "=?");
            threadIdList.add(String.valueOf(getThreadIdByContactId(contactId)));
            //Adding the blocked contact in blocked list
            String contact = mContactIdTranslater.getValue((short) contactId);
            ContactsManager.createInstance(AndroidFactory.getApplicationContext());
            if(isSpam)
            {
            	ContactsManager.getInstance().setImBlockedForContact(contact, true);
            	Calendar c = Calendar.getInstance();               
                SimpleDateFormat df = new SimpleDateFormat("HH:mm:ss dd-MM-yyyy");
                ContactsManager.getInstance().setTimeStampForBlockedContact(contact, df.format(c.getTime())); 
            }
            else
            	ContactsManager.getInstance().setImBlockedForContact(contact, false);
        }
        String whereString = whereStringBuilder.toString();
        whereString = whereString.replaceFirst(" OR ", "");
        int count = resolver.update(PluginUtils.THREAD_SETTINGS, values, whereString,
                threadIdList.toArray(new String[contactIds.length]));
        Logger.d(TAG, "markThreadAsSpam() mark thread settings id:" + threadIdList + "as " + (isSpam ? "spam" : "non-spam")
                + " , result count: " + count);
        return count > 0;
    }

    private long getThreadIdByContactId(int contactId) {
        String contact = mContactIdTranslater.getValue((short) contactId);
        Logger.d(TAG, "getThreadIdByContactId() contact of contactId: " + contactId + " is " + contact);
        if (contact.contains(IpMessageManager.COMMA)) {
            long threadId = Telephony.Threads.getOrCreateThreadId(AndroidFactory.getApplicationContext(), IpMessageManager
                    .collectMultiContact(contact));
            Logger.d(TAG, "getThreadIdByContactId() multi contact found and get threadId: " + threadId);
            return threadId;
        } else {
            long threadId = Telephony.Threads.getOrCreateThreadId(AndroidFactory.getApplicationContext(), contact);
            Logger.d(TAG, "getThreadIdByContactId() single contact threadId: " + threadId);
            return threadId;
        }
    }

}
