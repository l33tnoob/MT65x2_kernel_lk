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
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.PluginApiManager;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.util.HashMap;

/**
 * This class defined to implement the function interface of ICallLogExtention,
 * and achieve the main function here
 */
public class CallLogExtention extends ContextWrapper {

    private static final String TAG = "CallLogExtention";
    private static final int RCS_PRESENCE = 1;
    private final HashMap<String, OnPresenceChangedListener> mOnPresenceChangedListenerList = 
            new HashMap<String, OnPresenceChangedListener>();
    private static final String IM_CHAT = "IM";
    private PluginApiManager mInstance = null;
    private static final String CONTACT_CAPABILITIES = 
            "com.orangelabs.rcs.capability.CONTACT_CAPABILITIES";

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
            String number = intent.getStringExtra("contact");
            Capabilities capabilities = intent.getParcelableExtra("capabilities");
            if (capabilities != null) {
                int isRcsContact = capabilities.isSupportedRcseContact() ? 1 : 0;
                OnPresenceChangedListener listener = mOnPresenceChangedListenerList.get(number);
                if (listener != null) {
                    Logger.d(TAG, "listener is not null, number is " + number + "isRcsContact ="
                            + isRcsContact);
                    listener.onPresenceChanged(number, isRcsContact);
                } else {
                    Logger.d(TAG, "listener is null!");
                }
            } else {
                Logger.e(TAG, "onReceive(), capabilities is null!");
            }
        }
    };

    public CallLogExtention(Context context) {
        super(context);
        Logger.d(TAG, "CallLogExtention entry");
        PluginApiManager.initialize(context);
        mInstance = PluginApiManager.getInstance();
        IntentFilter filter = new IntentFilter();
        filter.addAction(CONTACT_CAPABILITIES);
        context.registerReceiver(mBroadcastReceiver, filter);
        Logger.d(TAG, "CallLogExtention exit");
    }

    public Drawable getAppIcon() {
        Logger.d(TAG, "getAppIcon entry");
        Resources resources = getResources();
        Drawable drawable = null;
        if (resources != null) {
            drawable = resources.getDrawable(R.drawable.icon_contact_indicaton);
        }
        Logger.d(TAG, "getAppIcon exit");
        return drawable;
    }

    public boolean isEnabled() {
        Logger.d(TAG, "isEnabled() entry");
        boolean isEnable = mInstance.getRegistrationStatus();
        Logger.d(TAG, "isEnabled() return: " + isEnable);
        return isEnable;
    }

    public Drawable getContactPresence(String number) {
        Logger.d(TAG, "getContactPresence entry, number is: " + number);
        if (number == null) {
            Logger.d(TAG, "getContactPresence exit, number is null");
            return null;
        }
        Drawable drawable = null;
        if (mInstance.getContactPresence(number) == RCS_PRESENCE) {
            Resources resources = getResources();
            if (resources != null) {
                drawable = resources.getDrawable(R.drawable.icon_contact_indicaton);
            } 
        } 
        Logger.d(TAG, "getContactPresence exit");
        return drawable;
    }

    public Action[] getContactActions(String number) {
        Logger.d(TAG, "getContactActions() entry");
        boolean imSupport = mInstance.isImSupported(number);
        boolean ftSupport = mInstance.isFtSupported(number);
        Action[] actions = new Action[2];
        Resources resources = getResources();
        Drawable imDrawable = null;
        Intent imIntent = null;
        if (imSupport) {
            Logger.d(TAG, "getContactActions() support IM");
            if (resources != null) {
                imDrawable = resources.getDrawable(R.drawable.btn_start_chat_nor);
            }
            imIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
            imIntent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);
        }

        Drawable ftDrawable = null;
        Intent ftIntent = null;
        if (ftSupport) {
            Logger.d(TAG, "getContactActions() support FT");
            if (resources != null) {
                ftDrawable = resources.getDrawable(R.drawable.btn_image_share_nor);
            }
            ftIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
            ftIntent.putExtra(PluginApiManager.RcseAction.FT_ACTION, true);
        }
        Action imAction = new Action();
        imAction.intentAction = imIntent;
        imAction.icon = imDrawable;
        actions[0] = imAction;
        Action ftAction = new Action();
        ftAction.intentAction = ftIntent;
        ftAction.icon = ftDrawable;
        actions[1] = ftAction;
        Logger.d(TAG, "getContactActions() exit actions[0]'intent is " + imIntent
                + " action[1]'intent is " + ftIntent);
        return actions;
    }

    public void addOnPresenceChangedListener(OnPresenceChangedListener listener, String number) {
        mOnPresenceChangedListenerList.put(number, listener);
        Logger.d(TAG, "Add OnPresenceChangedListener success");
    }

    public String getChatString() {
        Logger.d(TAG, "getChatString entry");
        String chat = IM_CHAT;
        Resources resources = getResources();
        if (resources != null) {
            chat = resources.getString(R.string.im_chat);
        } else {
            Logger.d(TAG, "getChatString resources is null");
        }
        Logger.d(TAG, "getChatString exit");
        return chat;
    }

    public static final class Action {
        public Intent intentAction;

        public Drawable icon;
    }

    public interface OnPresenceChangedListener {
        void onPresenceChanged(String number, int presence);
    }
}
