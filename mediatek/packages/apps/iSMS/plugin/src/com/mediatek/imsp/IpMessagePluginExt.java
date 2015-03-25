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

package com.mediatek.imsp;

import android.content.Context;

import com.mediatek.mms.ipmessage.ActivitiesManager;
import com.mediatek.mms.ipmessage.ChatManager;
import com.mediatek.mms.ipmessage.ContactManager;
import com.mediatek.mms.ipmessage.GroupManager;
import com.mediatek.mms.ipmessage.IpMessagePluginImpl;
import com.mediatek.mms.ipmessage.MessageManager;
import com.mediatek.mms.ipmessage.NotificationsManager;
import com.mediatek.mms.ipmessage.ResourceManager;
import com.mediatek.mms.ipmessage.ServiceManager;
import com.mediatek.mms.ipmessage.SettingsManager;

public class IpMessagePluginExt extends IpMessagePluginImpl {

    public Context mPluginContext = null;

    public IpMessagePluginExt(Context context) {
        super(context);
        mPluginContext = context;
    }

    public ActivitiesManager getActivitiesManager(Context context) {
        if (mActivityManager == null) {
            mActivityManager = new ActivitiesManagerExt(mPluginContext);
        }
        return mActivityManager;
    }

    public ChatManager getChatManager(Context context) {
        if (mChatManager == null) {
            mChatManager = new ChatManagerExt(context);
        }
        return mChatManager;
    }

    public ContactManager getContactManager(Context context) {
        if (mContactManager == null) {
            mContactManager = new ContactManagerExt(context);
        }
        return mContactManager;
    }

    public GroupManager getGroupManager(Context context) {
        if (mGroupManager == null) {
            mGroupManager = new GroupManagerExt(context);
        }
        return mGroupManager;
    }

    public MessageManager getMessageManager(Context context) {
        if (mMessageManager == null) {
            mMessageManager = new MessageManagerExt(context);
        }
        return mMessageManager;
    }

    public NotificationsManager getNotificationsManager(Context context) {
        if (mNotificationsManager == null) {
            mNotificationsManager = new NotificationsManagerExt(context);
        }
        return mNotificationsManager;
    }

    public ServiceManager getServiceManager(Context context) {
        if (mServiceManager == null) {
            mServiceManager = new ServiceManagerExt(context);
        }
        return mServiceManager;
    }

    public SettingsManager getSettingsManager(Context context) {
        if (mSettingsManager == null) {
            mSettingsManager = new SettingsManagerExt(context);
        }
        return mSettingsManager;
    }

    public ResourceManager getResourceManager(Context context) {
        if (mResourceManager == null) {
            mResourceManager = new ResourceManagerExt(mPluginContext);
        }
        return mResourceManager;
    }

    /**
     * Check if this is an implemented plugin or default plugin.
     * 
     * @return boolean true for actual plugin and false for default
     */
    public boolean isActualPlugin() {
        return true;
    }
}
