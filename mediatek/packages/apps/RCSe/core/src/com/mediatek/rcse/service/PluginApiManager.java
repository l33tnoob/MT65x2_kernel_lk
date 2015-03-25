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

package com.mediatek.rcse.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.text.TextUtils;
import android.util.LruCache;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.CapabilityApi.ICapabilityListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.plugin.contacts.ContactExtention.OnPresenceChangedListener;

import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.capability.CapabilityApiIntents;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class manages the APIs which are used by plug-in, providing a convenient
 * way for API invocations.
 */
public class PluginApiManager implements ICapabilityListener {
    public static final String TAG = "PluginApiManager";

    private static PluginApiManager sInstance = null;
    private boolean mNeedReconnectManagedAPi = false;
    private ManagedCapabilityApi mManagedCapabilityApi = null;
    private ManagedRegistrationApi mManagedRegistrationApi = null;

    private RegistrationApi mRegistrationApi = null;
    private CapabilityApi mCapabilitiesApi = null;
    private boolean mIsRegistered = false;
    private Context mContext = null;
    private static final int MAX_CACHE_SIZE = 2048;
    private final LruCache<String, ContactInformation> mContactsCache =
            new LruCache<String, ContactInformation>(MAX_CACHE_SIZE);
    private final LruCache<Long, List<String>> mCache =
            new LruCache<Long, List<String>>(MAX_CACHE_SIZE);
    private final ConcurrentHashMap<String, Integer> mUnreadMessageCountMap =
        new ConcurrentHashMap<String, Integer>();
    private static final String CONTACT_CAPABILITIES =
            "com.orangelabs.rcs.capability.CONTACT_CAPABILITIES";
    private static final String CONTACT_UNREAD_NUMBER =
        "com.mediatek.action.UNREAD_CHANGED_CONTACT";
    public static final String RCS_CONTACT_UNREAD_NUMBER_CHANGED = "android.intent.action.RCS_CONTACT_UNREAD_NUMBER_CHANGED";

    private final List<CapabilitiesChangeListener> mCapabilitiesChangeListenerList =
            new ArrayList<CapabilitiesChangeListener>();
    private final List<RegistrationListener> mRegistrationListeners =
            new CopyOnWriteArrayList<RegistrationListener>();
    private final List<RichCallApiListener> mRichCallApiListeners =
            new CopyOnWriteArrayList<RichCallApiListener>();
    private Cursor mCursor = null;
    private List<Long> mQueryOngoingList = new ArrayList<Long>();
    private final ConcurrentHashMap<Long, OnPresenceChangedListener> mPresenceListeners 
            = new ConcurrentHashMap<Long, OnPresenceChangedListener>();
    /**
     * MIME type for RCSE capabilities
     */
    private static final String MIMETYPE_RCSE_CAPABILITIES =
            "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    private static final int RCS_CONTACT = 1;
    private static final int RCS_CAPABLE_CONTACT = 2;

    public interface CallLogPresenceListener {
        void onPresenceChanged(String number, int presence);
    }

    /**
     * The CapabilitiesChangeListener defined as a listener to notify the
     * specify observer that the capabilities has been changed
     */
    public interface CapabilitiesChangeListener {
        void onCapabilitiesChanged(String contact, ContactInformation contactInformation);

        /**
         * Called when CapabilityApi connected status is changed.
         * 
         * @param isConnected True if CapabilityApi is connected.
         */
        void onApiConnectedStatusChanged(boolean isConnected);
    }

    /**
     * Add presence changed listener.
     * 
     * @param listener The presence changed listener.
     * @param contactId The contact id.
     */
    public void addOnPresenceChangedListener(OnPresenceChangedListener listener, long contactId) {
        mPresenceListeners.put(contactId, listener);
    }

    /**
     * Register the CapabilitiesChangeListener
     * 
     * @param listener The CapabilitiesChangeListener used to register
     */
    public void addCapabilitiesChangeListener(CapabilitiesChangeListener listener) {
        Logger.v(TAG, "addCapabilitiesChangeListener(), listener = " + listener);
        mCapabilitiesChangeListenerList.add(listener);
    }

    /**
     * Unregister the CapabilitiesChangeListener
     * 
     * @param listener The CapabilitiesChangeListener used to unregister
     */
    public void removeCapabilitiesChangeListener(CapabilitiesChangeListener listener) {
        Logger.v(TAG, "removeCapabilitiesChangeListener(), listener = " + listener);
        mCapabilitiesChangeListenerList.remove(listener);
    }

    /**
     * The RegistrationListener defined as a listener to notify the specify
     * observer that the registration status has been changed and
     * RegistrationAPi connected status.
     */
    public interface RegistrationListener {
        /**
         * Called when RegistrationApi connected status is changed.
         * 
         * @param isConnected True if RegistrationApi is connected
         */
        void onApiConnectedStatusChanged(boolean isConnected);

        /**
         * Called when the status of RCS-e account is registered.
         * 
         * @param status Current status of RCS-e account.
         */
        void onStatusChanged(boolean status);

        /**
         * Called when the rcse core service status has been changed.
         * 
         * @param status Current status of rcse core service.
         */
        void onRcsCoreServiceStatusChanged(int status);

    }

    /**
     * Register the RegistrationListener
     * 
     * @param listener The RegistrationListener used to register
     */
    public void addRegistrationListener(RegistrationListener listener) {
        Logger.v(TAG, "addRegistrationStatusListener(), listener = " + listener);
        mRegistrationListeners.add(listener);
    }

    /**
     * Unregister the RegistrationListener
     * 
     * @param listener The RegistrationListener used to unregister
     */
    public void removeRegistrationListener(RegistrationListener listener) {
        Logger.v(TAG, "removeRegistrationStatusListener(), listener = " + listener);
        mRegistrationListeners.remove(listener);
    }

    /**
     * The RichCallApiListener defined as a listener to notify the specify
     * observer that RichCallApiListener connected status.
     */
    public interface RichCallApiListener {
        /**
         * Called when CapabilityApi connected status is changed.
         * 
         * @param isConnected True if CapabilityApi is connected.
         */
        void onApiConnectedStatusChanged(boolean isConnected);
    }

    /**
     * Register the RichCallApiListener
     * 
     * @param listener The RichCallApiListener used to register
     */
    public void addRichCallApiListener(RichCallApiListener listener) {
        Logger.v(TAG, "addRichCallApiListener(), listener = " + listener);
        mRichCallApiListeners.add(listener);
    }

    /**
     * Unregister the RichCallApiListener
     * 
     * @param listener The RichCallApiListener used to unregister
     */
    public void removeRichCallApiListener(RichCallApiListener listener) {
        Logger.v(TAG, "removeRichCallApiListener(), listener = " + listener);
        mRichCallApiListeners.remove(listener);
    }

    /**
     * Get the instance of RegistrationApi
     * 
     * @return The instance of RegistrationApi
     */
    public RegistrationApi getRegistrationApi() {
        Logger.v(TAG, "getRegistrationApi(), mRegistrationApi = " + mRegistrationApi);
        return mRegistrationApi;
    }

    /**
     * Get the instance of CapabilityApi
     * 
     * @return The instance of CapabilityApi
     */
    public CapabilityApi getCapabilityApi() {
        Logger.v(TAG, "getCapabilityApi(), mCapabilitiesApi = " + mCapabilitiesApi);
        return mCapabilitiesApi;
    }

    /**
     * This class defined some instance used in RCS-e action
     */
    public static class RcseAction {
        /**
         * Defined as the IM action
         */
        public static final String IM_ACTION = "com.mediatek.rcse.action.CHAT_INSTANCE_MESSAGE";

        /**
         * Defined as the file transfer action
         */
        public static final String FT_ACTION = "com.mediatek.rcse.action.CHAT_FILE_TRANSFER";
        
        public static final String SHARE_URL_ACTION = "com.mediatek.rcse.action.CHAT_SEND_MESSAGE";
        
        /**
         * Defined as the select contacts action
         */
        public static final String SELECT_PLUGIN_CONTACT_ACTION = "com.mediatek.rcse.action.PluginSelectContact";
        
        /**
         * Defined as the proxy activity action
         */
        public static final String PROXY_ACTION = "com.mediatek.rcse.action.PROXY";

        /**
         * Defined as the contacts selection activity action
         */
        public static final String SELECT_CONTACT_ACTION =
                "com.mediatek.rcse.action.SELECT_CONTACT_BY_MULTIMEDIA";

        /**
         * Defined single file transfer action sent by File
         * manager,Gallery,Camera
         */
        public static final String SINGLE_FILE_TRANSFER_ACTION = "android.intent.action.SEND";

        /**
         * Defined multiple file transfer action sent by File
         * manager,Gallery,Camera
         */
        public static final String MULTIPLE_FILE_TRANSFER_ACTION =
                "android.intent.action.SEND_MULTIPLE";

        /**
         * Data name for display name
         */
        public static final String CONTACT_NAME = "rcs_display_name";

        /**
         * Data name for phone number
         */
        public static final String CONTACT_NUMBER = "rcs_phone_number";

        /**
         * Data name for Im or Ft capability.
         */
        public static final String CAPABILITY_SUPPORT = "isSupported";

        /**
         * Data name for single file URI
         */
        public static final String SINGLE_FILE_URI = "rcs_single_file_uri";

        /**
         * Data name for multiple file URI
         */
        public static final String MULTIPLE_FILE_URI = "rcs_multiple_file_uri";
        
        public static final String SHARE_URL = "rcs_share_url";
        
        public static final String GROUP_CHAT_PARTICIPANTS = "rcs_group_chat_participants";
        
        
    }

    /**
     * The class including some informations of contact: whether it is an Rcse
     * contact, the capabilities of IM, file transfer,CS call,image and video
     * share.
     */
    public static class ContactInformation {
        public int isRcsContact = 0;// 0 indicate not Rcs, 1 indicate Rcs
        public boolean isImSupported = false;
        public boolean isFtSupported = false;
        public boolean isImageShareSupported = false;
        public boolean isVideoShareSupported = false;
        public boolean isCsCallSupported = false;
        public int unreadNumber = 0;
    }

    /**
     * Get the presence of number.
     * 
     * @param number The number whose presence to be queried.
     * @return The presence of the number.
     */
    public int getContactPresence(final String number) {
        Logger.d(TAG, "getContactPresence entry, number is " + number);
        ContactInformation info = null;
        synchronized (mContactsCache) {
            info = mContactsCache.get(number);
        }
        if (info != null) {
            Logger.d(TAG, "getContactPresence number " + number + " with cacheable: "
                    + info.isRcsContact);
            return info.isRcsContact;
        } else {
            Logger.d(TAG, "getContactPresence uncacheable, retry to query presence");
            ContactInformation defaultInfo = new ContactInformation();
            synchronized (mContactsCache) {
                mContactsCache.put(number, defaultInfo);
            }
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    queryContactPresence(number);
                }
            });
        }
        return 0;
    }

    private void initData() {
        Logger.d(TAG, "initData entry");
        Thread thread = new Thread() {
            public void run() {
                Looper.prepare();
                registerObserver();
                queryContactsPresence();
                IntentFilter filter = new IntentFilter();
                filter.addAction(CONTACT_CAPABILITIES);
                filter.addAction(CONTACT_UNREAD_NUMBER);
                mContext.registerReceiver(mBroadcastReceiver, filter);
            }
        };
        thread.start();
        Logger.d(TAG, "initData exit");
    }

    
    public int getUnreadMessageCount(String contact)
    {
    	Logger.d(TAG, "getUnreadMessageCount "+ contact );
        if(mUnreadMessageCountMap != null)
        {
        	if(mUnreadMessageCountMap.containsKey(contact))
        	{
    	        return mUnreadMessageCountMap.get(contact);
        	}
        	else
        	{
        		return 0;
        	}
        }
        else
        {
        	return 0;
        }
    }
    /**
     * Get presence of contact id.
     * 
     * @param contactId The contact id whose presence to be queried.
     * @return The presence of the contact.
     */
    public int getContactPresence(final long contactId) {
        final List<String> numbers = mCache.get(contactId);
        Logger.d(TAG, "getContactPresence() entry, contactId: " + contactId + " numbers: "
                + numbers);
        if (numbers != null) {
            synchronized (mPresenceListeners) {
                mPresenceListeners.remove(contactId);
            }
            ContactInformation info = null;
            for (String number : numbers) {
                info = mContactsCache.get(number);
                if (info != null && info.isRcsContact == 1) {
                    return 1;
                }
            }
            if (mQueryOngoingList.contains(contactId)) {
                Logger.d(TAG, "getContactPresence contact id " + contactId
                        + " query presence operation is ongoing");
                return 0;
            }
            mQueryOngoingList.add(contactId);
            if (info == null) {
                Logger.d(TAG, "getContactPresence info is null, so retry to query");
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        queryPresence(contactId, numbers);
                        mQueryOngoingList.remove(contactId);
                    }
                });
            }
        } else {
            if (mQueryOngoingList.contains(contactId)) {
                Logger.d(TAG, "getContactPresence contact id " + contactId
                        + " query presence operation is ongoing");
                return 0;
            }
            mQueryOngoingList.add(contactId);
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    final List<String> list = getNumbersByContactId(contactId);
                    queryPresence(contactId, list);
                    mQueryOngoingList.remove(contactId);
                }
            });
        }
        return 0;
    }

    private void registerObserver() {
        Logger.d(TAG, "registerObserver entry");
        if (mCursor != null && mCursor.isClosed()) {
            Logger.d(TAG, "registerObserver close cursor");
            mCursor.close();
        }
        // Query contactContracts phone database
        mCursor = mContext.getContentResolver().query(Phone.CONTENT_URI, null, null, null, null);
        if (mCursor != null) {
            // Register content observer
            Logger.d(TAG, "registerObserver begin to registerContentObserver");
            mCursor.registerContentObserver(new ContentObserver(new Handler()) {
                @Override
                public void onChange(boolean selfChange) {
                    super.onChange(selfChange);
                    Logger.d(TAG, "onChange entry" + selfChange);
                    if (mCache == null) {
                        Logger.d(TAG, "onChange mCache is null");
                        return;
                    }
                    Map<Long, List<String>> map = mCache.snapshot();
                    if (map != null) {
                        Set<Long> keys = map.keySet();
                        for (Long key : keys) {
                            getNumbersByContactId(key);
                        }
                    } else {
                        Logger.d(TAG, "onChange map is null");
                    }
                }
            });
        } else {
            Logger.d(TAG, "registerObserver mCursor is null");
        }
        Logger.d(TAG, "registerObserver exit");
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
        	String action = intent.getAction();
        	if(CONTACT_UNREAD_NUMBER.equals(action))
        	{
        		String contact = intent.getStringExtra("number");
        		boolean isUpdateAll = intent.getBooleanExtra("updateall", false);
        		int count = intent.getIntExtra("count", 0);
        		Logger.d(TAG, "onReceive(),unread number intent received "+contact);
        		//int unreadCount = 0;
        		if(mUnreadMessageCountMap != null && contact != null)
        		{
	        		/*if(mUnreadMessageCountMap.containsKey(contact))
	        		{
	        		    unreadCount = mUnreadMessageCountMap.get(contact);
	        		}*/
	        		if(isUpdateAll == true)
	        		{
	        			synchronized (mUnreadMessageCountMap) {
	        				mUnreadMessageCountMap.put(contact, 0);
						}
	        		}
	        		else
	        		{
	        			synchronized (mUnreadMessageCountMap) {
	        		        mUnreadMessageCountMap.put(contact, count);
            	}
        	}
        		}
        		else
        		{
        			Logger.d(TAG, "onReceive(),mUnreadMessageCountMap is null");
            	}
        		
        		//broadcast to contact app about this
        		Intent intent_unread = new Intent(RCS_CONTACT_UNREAD_NUMBER_CHANGED);
				mContext.sendBroadcast(intent_unread);
            	
        	}
        	else if(CONTACT_CAPABILITIES.equals(action))
        	{
        	
            String number = intent.getStringExtra("contact");
            Capabilities capabilities = intent.getParcelableExtra("capabilities");
            ContactInformation info = new ContactInformation();
            if (capabilities == null) {
                Logger.d(TAG, "onReceive(),capabilities is null");
                return;
            }
            info.isRcsContact = capabilities.isSupportedRcseContact() ? 1 : 0;
            info.isImSupported = capabilities.isImSessionSupported();
            info.isFtSupported = capabilities.isFileTransferSupported();
            info.isImageShareSupported = capabilities.isImageSharingSupported();
            info.isVideoShareSupported = capabilities.isVideoSharingSupported();
            info.isCsCallSupported = capabilities.isCsVideoSupported();
            Logger.d(TAG, "onReceive getRcseContact contact is: " + number + " "
                    + info.isRcsContact);
            synchronized (mContactsCache) {
                mContactsCache.put(number, info);
            }
            for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                if (listener != null) {
                    listener.onCapabilitiesChanged(number, info);
                    return;
                }
            }
        }
        }
    };

    private void queryContactsPresence() {
        Logger.d(TAG, "queryContactsPresence entry");
        List<String> rcsNumbers = new ArrayList<String>();
        String[] projection =
                {
                        Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DATA3, Data.DATA5, Data.DATA6,
                        Data.DATA7, Data.DATA8, Data.DATA9
                };
        // Filter the mime types
        String selection =
                Data.MIMETYPE + "=? AND " + "(" + Data.DATA4 + "=? OR " + Data.DATA4 + "=? )";
        String[] selectionArgs =
                {
                        MIMETYPE_RCSE_CAPABILITIES, Long.toString(RCS_CONTACT),
                        Long.toString(RCS_CAPABLE_CONTACT)
                };
        Cursor cursor =
                mContext.getContentResolver().query(Data.CONTENT_URI, projection, selection,
                        selectionArgs, null);
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    int imIndex = cursor.getColumnIndex(Data.DATA5);
                    int ftIndex = cursor.getColumnIndex(Data.DATA6);
                    int imageSharingIndex = cursor.getColumnIndex(Data.DATA7);
                    int videoSharingIndex = cursor.getColumnIndex(Data.DATA8);
                    int csVideoIndex = cursor.getColumnIndex(Data.DATA9);
                    ContactInformation info = new ContactInformation();
                    info.isRcsContact = 1;
                    info.isImSupported = cursor.getInt(imIndex) == 1 ? true : false;
                    info.isFtSupported = cursor.getInt(ftIndex) == 1 ? true : false;
                    info.isImageShareSupported =
                            cursor.getInt(imageSharingIndex) == 1 ? true : false;
                    info.isVideoShareSupported =
                            cursor.getInt(videoSharingIndex) == 1 ? true : false;
                    info.isCsCallSupported = cursor.getInt(csVideoIndex) == 1 ? true : false;
                    String number = cursor.getString(1);
                    Logger.d(TAG, "queryContactsPresence number: " + number);
                    synchronized (mContactsCache) {
                        mContactsCache.put(number, info);
                    }
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Logger.d(TAG, "getRcsContacts exit, size: " + rcsNumbers.size());
    }

    private void queryContactPresence(final String number) {
        Logger.d(TAG, "queryContactsPresence entry, number is " + number);
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "queryContactPresence() in async task");
                if (mCapabilitiesApi == null) {
                    Logger.d(TAG, "queryContactPresence mCapabilitiesApi is null, number is "
                            + number);
                } else {
                    Capabilities capabilities = mCapabilitiesApi.getContactCapabilities(number);
                    ContactInformation info = new ContactInformation();
                    if (capabilities == null) {
                        capabilities = new Capabilities();
                    }
                    info.isRcsContact = capabilities.isSupportedRcseContact() ? 1 : 0;
                    info.isImSupported = capabilities.isImSessionSupported();
                    info.isFtSupported = capabilities.isFileTransferSupported();
                    info.isImageShareSupported = capabilities.isImageSharingSupported();
                    info.isVideoShareSupported = capabilities.isVideoSharingSupported();
                    info.isCsCallSupported = capabilities.isCsVideoSupported();
                    synchronized (mContactsCache) {
                        mContactsCache.put(number, info);
                    }
                }
                Logger.d(TAG, "queryContactPresence() leave async task");
            }
        });
        Logger.d(TAG, "queryContactPresence exit");
    }

    /**
     * Query a series of phone number
     * 
     * @param numbers The phone numbers list need to query
     */
    public void queryNumbersPresence(List<String> numbers) {
        Logger.d(TAG, "queryNumbersPresence entry, numbers: " + numbers + ", mCapabilitiesApi= "
                + mCapabilitiesApi);
        if (mCapabilitiesApi != null) {
            for (String number : numbers) {
                Logger.d(TAG, "queryNumbersPresence number: " + number);
                ContactInformation info = new ContactInformation();
                Capabilities capabilities = mCapabilitiesApi.getContactCapabilities(number);
                if (capabilities == null) {
                    capabilities = new Capabilities();
                }
                info.isRcsContact = capabilities.isSupportedRcseContact() ? 1 : 0;
                info.isImSupported = capabilities.isImSessionSupported();
                info.isFtSupported = capabilities.isFileTransferSupported();
                info.isImageShareSupported = capabilities.isImageSharingSupported();
                info.isVideoShareSupported = capabilities.isVideoSharingSupported();
                info.isCsCallSupported = capabilities.isCsVideoSupported();
                synchronized (mContactsCache) {
                    mContactsCache.put(number, info);
                }
            }
        }
    }

    /**
     * Query a series of phone number
     * 
     * @param contactId The contact id
     * @param numbers The phone numbers list need to query
     */
    private void queryPresence(long contactId, List<String> numbers) {
        Logger.d(TAG, "queryPresence() entry, contactId: " + contactId + " numbers: " + numbers
                + " mCapabilitiesApi: " + mCapabilitiesApi);
        if (mCapabilitiesApi != null) {
            boolean needNotify = false;
            for (String number : numbers) {
                ContactInformation info = new ContactInformation();
                Capabilities capabilities = null;
                ContactInformation cachedInfo = mContactsCache.get(number);
                if (cachedInfo == null) {
                    capabilities = mCapabilitiesApi.getContactCapabilities(number);
                } else {
                    if (cachedInfo.isRcsContact == 1) {
                        needNotify = true;
                    }
                    continue;
                }
                if (capabilities == null) {
                    capabilities = new Capabilities();
                }
                info.isRcsContact = capabilities.isSupportedRcseContact() ? 1 : 0;
                info.isImSupported = capabilities.isImSessionSupported();
                info.isFtSupported = capabilities.isFileTransferSupported();
                info.isImageShareSupported = capabilities.isImageSharingSupported();
                info.isVideoShareSupported = capabilities.isVideoSharingSupported();
                info.isCsCallSupported = capabilities.isCsVideoSupported();
                synchronized (mContactsCache) {
                    mContactsCache.put(number, info);
                }
                if (info.isRcsContact == 1) {
                    needNotify = true;
                }
            }
            synchronized (mPresenceListeners) {
                if (needNotify) {
                    OnPresenceChangedListener listener = mPresenceListeners.get(contactId);
                    if (listener != null) {
                        listener.onPresenceChanged(contactId, 1);
                    }
                }
                mPresenceListeners.remove(contactId);
            }
            Logger.d(TAG, "queryPresence() contactId: " + contactId + " needNotify: " + needNotify);
        }
    }

    /**
     * Obtain the phone numbers from a specific contact id
     * 
     * @param contactId The contact id
     * @return The phone numbers of the contact id
     */
    public List<String> getNumbersByContactId(long contactId) {
        Logger.d(TAG, "getNumbersByContactId entry, contact id is: " + contactId);
        List<String> list = new ArrayList<String>();
        String[] projection = {
            Phone.NUMBER
        };
        String selection = Phone.CONTACT_ID + "=? ";
        String[] selectionArgs = {
            Long.toString(contactId)
        };
        Cursor cur =
                mContext.getContentResolver().query(Phone.CONTENT_URI, projection, selection,
                        selectionArgs, null);
        try {
            if (cur != null) {
                while (cur.moveToNext()) {
                    String number = cur.getString(0);
                    if (!TextUtils.isEmpty(number)) {
                        list.add(number.replace(" ", ""));
                    } else {
                        Logger.w(TAG, "getNumbersByContactId() invalid number: " + number);
                    }
                }
            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
        mCache.put(contactId, list);
        Logger.d(TAG, "getNumbersByContactId exit, list: " + list);
        return list;
    }

    /**
     * Return whether Im is supported.
     * 
     * @param number The number whose capability is to be queried.
     * @return True if Im is supported, else false.
     */
    public boolean isImSupported(final String number) {
        Logger.d(TAG, "isImSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "isImSupported exit");
            return info.isImSupported;
        } else {
            Logger.d(TAG, "isImSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether file transfer is supported.
     * 
     * @param number The number whose capability is to be queried.
     * @return True if file transfer is supported, else false.
     */
    public boolean isFtSupported(final String number) {
        Logger.d(TAG, "isFtSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "isFtSupported exit");
            return info.isFtSupported;
        } else {
            Logger.d(TAG, "isFtSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether image sharing is supported.
     * 
     * @param number The number whose capability is to be queried.
     * @return True if image sharing is supported, else false.
     */
    public boolean isImageShareSupported(final String number) {
        Logger.d(TAG, "isImageShareSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "sharing number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null && info.isImageShareSupported) {
            Logger.d(TAG, "sharing isImageShareSupported exit");
            return info.isImageShareSupported;
        } else {
            Logger.d(TAG, "sharing isImageShareSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether video sharing is supported.
     * 
     * @param number The number whose capability is to be queried.
     * @return True if video sharing is supported, else false.
     */
    public boolean isVideoShareSupported(final String number) {
        Logger.d(TAG, "isVideoShareSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null && info.isVideoShareSupported) {
            Logger.d(TAG, "isVideoShareSupported exit");
            return info.isVideoShareSupported;
        } else {
            Logger.d(TAG, "isVideoShareSupported info is null, or the capability is false.");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether CS call is supported.
     * 
     * @param number The number whose capability is to be queried.
     * @return True if CS call is supported, else false.
     */
    public boolean isCsCallShareSupported(final String number) {
        Logger.d(TAG, "isCsCallShareSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "isCsCallShareSupported exit");
            return info.isCsCallSupported;
        } else {
            Logger.d(TAG, "isCsCallShareSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether Im is supported.
     * 
     * @param contactId The contactId whose capability is to be queried.
     * @return True if Im is supported, else false.
     */
    public boolean isImSupported(final long contactId) {
        Logger.d(TAG, "isImSupported entry, with contact id: " + contactId);
        final List<String> numbers = mCache.get(contactId);
        if (numbers != null) {
            for (String number : numbers) {
                boolean isImSupported = isImSupported(number);
                if (isImSupported) {
                    Logger.d(TAG, "isImSupported exit with true");
                    return true;
                }
            }
            Logger.d(TAG, "isImSupported exit with false");
            return false;
        } else {
            Logger.d(TAG, "isImSupported numbers is null, exit with false");
            return false;
        }
    }

    /**
     * Return whether file transfer is supported.
     * 
     * @param contactId The contactId whose capability is to be queried.
     * @return True if file transfer is supported, else false.
     */
    public boolean isFtSupported(final long contactId) {
        Logger.d(TAG, "isFtSupported entry, with contact id: " + contactId);
        final List<String> numbers = mCache.get(contactId);
        if (numbers != null) {
            for (String number : numbers) {
                boolean isFtSupported = isFtSupported(number);
                if (isFtSupported) {
                    Logger.d(TAG, "isFtSupported exit with true");
                    return true;
                }
            }
            Logger.d(TAG, "isFtSupported exit with false");
            return false;
        } else {
            Logger.d(TAG, "isFtSupported numbers is null, exit with false");
            return false;
        }
    }

    /**
     * Check whether a number is a rcse account
     * 
     * @param number The number to query
     * @return True if number is a rcse account, otherwise return false.
     */
    public boolean isRcseContact(String number) {
        Logger.d(TAG, "sharing isRcseContact entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "sharing number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "sharing isCsCallShareSupported exit");
            return info.isRcsContact == 1;
        } else {
            Logger.d(TAG, "sharing isCsCallShareSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * This method should only be called from ApiService, for APIs
     * initialization.
     * 
     * @param context The Context of this application.
     * @return true If initialize successfully, otherwise false.
     */
    public static synchronized boolean initialize(Context context) {
        Logger.v(TAG, "initialize() entry");
        if (null != sInstance) {
            Logger
                    .w(TAG,
                            "initialize() sInstance has existed, is it really the first time you call this method?");
            return true;
        } else {
            if (null != context) {
                PluginApiManager apiManager = new PluginApiManager(context);
                sInstance = apiManager;
                return true;
            } else {
                Logger.e(TAG, "initialize() the context is null");
                return false;
            }
        }
    }

    /**
     * Get the context
     * 
     * @return Context
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Get the instance of PluginApiManager
     * 
     * @return The instance of ApiManager, or null if the instance has not been
     *         initialized.
     */
    public static PluginApiManager getInstance() {
        if (null == sInstance) {
            throw new RuntimeException("Please call initialize() before calling this method");
        }
        return sInstance;
    }

    private PluginApiManager(Context context) {
        Logger.v(TAG, "PluginApiManager(), context = " + context);
        mContext = context;
        mManagedCapabilityApi = new ManagedCapabilityApi(context);
        mManagedCapabilityApi.connect();
        mManagedRegistrationApi = new ManagedRegistrationApi(context);
        mManagedRegistrationApi.connect();
        initData();
    }

    /**
     * This class defined to manage the registration APIs and registration
     * status.
     */
    private class ManagedRegistrationApi extends RegistrationApi {
        public static final String TAG = "PluginApiManager/ManagedRegistrationApi";

        public ManagedRegistrationApi(Context context) {
            super(context);
        }

        @Override
        public void handleConnected() {
            Logger.v(TAG, "handleConnected() entry");
            for (RegistrationListener listener : mRegistrationListeners) {
                listener.onApiConnectedStatusChanged(true);
            }
            mRegistrationApi = this;
            mRegistrationApi.addRegistrationStatusListener(new IRegistrationStatusListener() {

                @Override
                public void onStatusChanged(boolean status) {
                    Logger.d(TAG, "onStatusChanged() entry, status is " + status);
                    for (RegistrationListener listener : mRegistrationListeners) {
                        listener.onStatusChanged(status);
                    }
                    mIsRegistered = status;
                }
            });
            mIsRegistered = mRegistrationApi.isRegistered();
            Logger.d(TAG, "handleConnected() mIsRegistered is: " + mIsRegistered);
        }

        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            Logger.d(TAG, "Notifiy plugin");
            for (RegistrationListener listener : mRegistrationListeners) {
                listener.onApiConnectedStatusChanged(false);
            }
            if (mRegistrationApi == this) {
                Logger.i(TAG, "handleDisconnected() mRegistrationApi disconnected");
                mRegistrationApi = null;
            } else {
                Logger.e(TAG, "handleDisconnected() another mRegistrationApi disconnected?");
            }
        }
    }

    /**
     * Get the registration status
     * 
     * @return Registration status
     */
    public boolean getRegistrationStatus() {
        if (mRegistrationApi == null) {
            Logger.w(TAG, "getRegisteredStatus()-mRegistrationApi is null");
            return false;
        }
        return mIsRegistered;
    }

    /**
     * Set the registration status
     * 
     * @param status registration status
     */
    public void setRegistrationStatus(boolean status) {
        Logger.w(TAG, "setRegistrationStatus()-status is " + status);
        mIsRegistered = status;
    }

    /**
     * This class defined to manage the capabilities APIs and capabilities.
     */
    private class ManagedCapabilityApi extends CapabilityApi {
        private static final String TAG = "PluginApiManager/ManagedCapabilityApi";

        public ManagedCapabilityApi(Context context) {
            super(context);
        	Logger.w(TAG, "PluginApiManager ManagedCapabilityApi() entry");
        }

        @Override
        public void handleConnected() {
            Logger.v(TAG, "handleConnected() entry");
            mCapabilitiesApi = this;
            mCapabilitiesApi.registerCapabilityListener(PluginApiManager.this);
            for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                listener.onApiConnectedStatusChanged(true);
            }
        }

        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                listener.onApiConnectedStatusChanged(false);
            }
            if (mCapabilitiesApi == this) {
                Logger.i(TAG, "handleDisconnected() mCapabilitiesApi disconnected");
                mCapabilitiesApi.unregisterCapabilityListener(PluginApiManager.this);
                mCapabilitiesApi = null;
            } else {
                Logger.e(TAG, "handleDisconnected() another mCapabilitiesApi disconnected?");
            }
        }
    }

    @Override
    public void onCapabilityChanged(String contact, Capabilities capabilities) {
        Logger.w(TAG, "options onCapabilityChanged(), contact = " + contact + ", capabilities = "
                + capabilities + ", mContactsCache= " + mContactsCache);
        if (null != contact && capabilities != null) {
            Logger.v(TAG, "Remove from cache");
            ContactInformation info = mContactsCache.remove(contact);
            Logger.v(TAG, "after remove from cache");
            if (info == null) {
                Logger.v(TAG, "cache does not exist, so create a object.");
                info = new ContactInformation();
            }
            info.isRcsContact = capabilities.isSupportedRcseContact() ? 1 : 0;
			Logger.v(TAG, "Options  is RCS Contact:" + info.isRcsContact);
			if(capabilities.isSupportedRcseContact()){
				Logger.w(TAG, "Options It is RCS Contact");
			}
            info.isImSupported = capabilities.isImSessionSupported();
            info.isFtSupported = capabilities.isFileTransferSupported();
            info.isImageShareSupported = capabilities.isImageSharingSupported();
            info.isVideoShareSupported = capabilities.isVideoSharingSupported();
			if(info.isImageShareSupported || info.isVideoShareSupported){
				Logger.w(TAG, "Options Image/Video share supported");
			}
            info.isCsCallSupported = capabilities.isCsVideoSupported();
            mContactsCache.put(contact, info);
            Logger.w(TAG, "put capability into cache");
            for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                if (listener != null) {
                    Logger.w(TAG, "Notify the listener");
                    listener.onCapabilitiesChanged(contact, info);
                }
            }
        } else {
            Logger.d(TAG, "onCapabilityChanged()-invalid contact or capabilities");
        }
    }

    /**
     * This constructor is just used for test case.
     */
    public PluginApiManager() {

    }

    /**
     * Clear all the information in the mContactsCache.
     */
    public void cleanContactCache() {
        Logger.d(TAG, "cleanContactCache() entry");
        mContactsCache.evictAll();
    }

    /**
     * @param needReconnect Indicate whether need to reconnect API
     */
    public void setManagedApiStatus(boolean needReconnect) {
        mNeedReconnectManagedAPi = true;
    }

    /**
     * @return True if need to reconnect API, otherwise return false
     */
    public boolean getManagedApiStatus() {
        return mNeedReconnectManagedAPi;
    }

    /**
     * Reconnect ManagedCapabilityApi
     */
    public void reConnectManagedApi() {
        Logger.d(TAG, "reConnectManagedApi():");
        mNeedReconnectManagedAPi = false;
        mManagedCapabilityApi.connect();
    }

}
