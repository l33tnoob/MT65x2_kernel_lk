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

package com.mediatek.rcse.plugin.contacts;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.provider.ContactsContract.Contacts;
import android.util.Log;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.PluginApiManager;
import com.orangelabs.rcs.R;

import java.util.HashMap;
import java.util.List;

/**
 * This class defined to implement the function interface of IContactExtention,
 * and achieve the main function here
 */
public class ContactExtention extends ContextWrapper {

    private static final String TAG = "ContactExtention";
    private static final int RCS_PRESENCE = 1;
    private final HashMap<OnPresenceChangedListener, Long> mOnPresenceChangedListenerList =
            new HashMap<OnPresenceChangedListener, Long>();
    private static final String RCS_MIMETYPE =
            "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    private PluginApiManager mInstance = null;
    private Context mContext = null;

    public ContactExtention(Context context) {
        super(context);
        mContext = context;
        Logger.d(TAG, "ContactExtention entry");
        PluginApiManager.initialize(context);
        mInstance = PluginApiManager.getInstance();
        Logger.d(TAG, "ContactExtention exit");
    }

    public Context getContext() {
    	return mContext;
    }

    public Drawable getAppIcon() {
        Logger.d(TAG, "getAppIcon entry");
        Resources resources = getResources();
        Drawable drawable = null;
        Log.i("ContactExtention"," resources : "+resources);
        if (resources != null) {
            drawable = resources.getDrawable(R.drawable.icon_contact_indicaton);
        } else {
            Logger.d(TAG, "getAppIcon resources is null");
        }
        Logger.d(TAG, "getAppIcon exit");
        return drawable;
    }

    public List<String> getNumbersByContactId(long contactId) {
    	return mInstance.getNumbersByContactId(contactId);
    }
    
    public int getUnreadMessageCount(String contact)
    {
    	return mInstance.getUnreadMessageCount(contact);
    }
    
    public boolean isEnabled() {
        Logger.d(TAG, "isEnabled() entry");
        boolean isEnable = mInstance.getRegistrationStatus();
        Logger.d(TAG, "isEnabled() return: " + isEnable);
        return isEnable;
    }

    public Drawable getContactPresence(long contactId) {
        Logger.d(TAG, "getContactPresence entry, contact id is: " + contactId);
        Drawable drawable = null;
        int presence = mInstance.getContactPresence(contactId);
        if (presence == RCS_PRESENCE) {
            Resources resources = getResources();
            if (resources != null) {
                drawable = resources.getDrawable(R.drawable.icon_contact_indicaton);
            }
        }
        Logger.d(TAG, "getContactPresence exit, presence: " + presence);
        return drawable;
    }

    public Action[] getContactActions() {
        Logger.d(TAG, "getContactActions entry");
        Action[] actions = new Action[2];
        Resources resources = getResources();
        Drawable imDrawable = null;
        if (resources != null) {
            imDrawable = resources.getDrawable(R.drawable.btn_start_chat_nor);
        } else {
            Logger.d(TAG, "getContactActions resources is null");
        }
        Drawable ftDrawable = null;
        if (resources != null) {
            ftDrawable = resources.getDrawable(R.drawable.btn_start_share_nor);
        } else {
            Logger.d(TAG, "getContactActions resources is null");
        }
        Intent imIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
        imIntent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);
        Intent ftIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
        ftIntent.putExtra(PluginApiManager.RcseAction.FT_ACTION, true);
        Action imAction = new Action();
        imAction.intentAction = imIntent;
        imAction.icon = imDrawable;
        actions[0] = imAction;
        Action ftAction = new Action();
        ftAction.intentAction = ftIntent;
        ftAction.icon = ftDrawable;
        actions[1] = ftAction;
        Logger.d(TAG, "getContactActions exit");
        return actions;
    }

    public Drawable getBlockingDrawable()
    {
    	Drawable blockingDrawable = null;
    	Resources resources = getResources();
    	if (resources != null) {
    		blockingDrawable = resources.getDrawable(R.drawable.block_contact);
        } 
    	return blockingDrawable;
    }

    public Drawable getXMSDrawable()
    {
    	Drawable XMSDrawable = null;
    	Resources resources = getResources();
    	if (resources != null) {
    		XMSDrawable = resources.getDrawable(R.drawable.ic_launcher_smsmms);
        } 
    	return XMSDrawable;
    }

    public String getBlockingTitle()
    {
        String blockingTitle = null;
        Resources resources = getResources();
        if (resources != null) {
            blockingTitle = resources.getString(R.string.block_contact_title);
        } 
        return blockingTitle;
    }

    public void addOnPresenceChangedListener(OnPresenceChangedListener listener, long contactId) {
        mInstance.addOnPresenceChangedListener(listener, contactId);
    }

    public String getAppTitle() {
        Logger.d(TAG, "getAppTitle entry");
        Resources resources = getResources();
        String title = null;
        if (resources != null) {
            title = resources.getString(R.string.joyn_title);
        } else {
            Logger.d(TAG, "getAppTitle resources is null");
        }
        Logger.d(TAG, "getAppTitle exit");
        return title;
    }

    public String getMimeType() {
        return RCS_MIMETYPE;
    }

    public void onContactDetailOpen(final Uri contactLookUpUri) {
        Logger.d(TAG, "onContactDetailOpen() contactLookUpUri: " + contactLookUpUri);
        if (null != contactLookUpUri) {
            final ContentResolver contentResolver = getContentResolver();
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    Cursor cursor = null;
                    long contactId = -1;
                    try {
                        cursor = contentResolver.query(contactLookUpUri, new String[] {
                            Contacts._ID
                        }, null, null, null);
                        if (null == cursor || !cursor.moveToFirst()) {
                            Logger.e(TAG, "onContactDetailOpen() error when loading cursor");
                            return;
                        }
                        int indexContactId = cursor.getColumnIndex(Contacts._ID);
                        do {
                            contactId = cursor.getLong(indexContactId);
                        } while (cursor.moveToNext());
                    } finally {
                        if (null != cursor) {
                            cursor.close();
                        }
                    }
                    if (-1 != contactId) {
                        checkCapabilityByContactId(contactId);
                    } else {
                        Logger.w(TAG, "onContactDetailOpen() contactLookUpUri " + contactLookUpUri);
                    }
                }
            });
        } else {
            Logger.w(TAG, "onContactDetailOpen() contactLookUpUri is null");
        }
    }

    private void checkCapabilityByContactId(long contactId) {
        Logger.d(TAG, "checkCapabilityByContactId() contactId: " + contactId);
        PluginApiManager apiManager = PluginApiManager.getInstance();
        apiManager.queryNumbersPresence(apiManager.getNumbersByContactId(contactId));
    }
    
    public static final class Action {
        public Intent intentAction;

        public Drawable icon;
    }

    /**
     * Interface for plugin to call back host that presence has changed.
     */
    public interface OnPresenceChangedListener {
        /**
         * Call back when presence changed.
         * 
         * @param contactId The contact id.
         * @param presence The presence.
         */
        void onPresenceChanged(long contactId, int presence);
    }

    /**
     *show rcs-e icon on the Detail Actvitiy's action bar
     * 
     * @return null if there shouldn't show rcs-e icon.
     */
    public Drawable getRCSIcon() {
        // Get the plug-in Resources instance by the plug-in Context instance
        Resources resource = this.getResources();
        if (resource == null) {
            Logger.d(TAG, "getRCSIcon()-the plugin resource is null");
            return null;
        }
        Drawable drawable = resource.getDrawable(R.drawable.ic_rcse_indicaton);
        return drawable;
    }

}
