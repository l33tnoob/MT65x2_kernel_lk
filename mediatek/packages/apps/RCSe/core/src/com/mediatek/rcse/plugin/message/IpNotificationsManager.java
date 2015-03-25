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

import com.mediatek.mms.ipmessage.INotificationsListener;
import com.mediatek.mms.ipmessage.NotificationsManager;
import com.mediatek.rcse.api.Logger;

import java.util.Vector;

/**
 * Ip notification manager
 */
public class IpNotificationsManager extends NotificationsManager {
    private static final String TAG = "IpNotificationsManager";
    private static final Object LISTENER_SYNC = new Object();

    /**
     * IP message notification listeners
     */
    private static Vector<INotificationsListener> sNotificationsListeners = new Vector<INotificationsListener>();

    public IpNotificationsManager(Context context) {
        super(context);
    }

    public void registerNotificationsListener(INotificationsListener notiListener) {
        synchronized (LISTENER_SYNC) {
            Logger.d(TAG, "registerNotificationsListener,listener = " + notiListener);
            if (sNotificationsListeners.contains(notiListener)) {
                Logger.d(TAG, "registerNotificationsListener, already contains.");
                return;
            }
            sNotificationsListeners.addElement(notiListener);
            int num = sNotificationsListeners.size();
            Logger.d(TAG, "after add, num = " + num);
        }
    }

    public void unregisterNotificationsListener(INotificationsListener notiListener) {
        synchronized (LISTENER_SYNC) {
            Logger.d(TAG, "unregisterNotificationsListener, listener = " + notiListener);
            if (!sNotificationsListeners.contains(notiListener)) {
                Logger.d(TAG, "unregisterNotificationsListener, do not contain !!");
                return;
            }
            sNotificationsListeners.removeElement(notiListener);
            int num = sNotificationsListeners.size();
            Logger.d(TAG, "after remove, num = " + num);
        }
    }

    /**
     * Notify listeners when ip message notifications received
     */
    public static void notify(Intent intent) {
        synchronized (LISTENER_SYNC) {
            final int num = sNotificationsListeners.size();
            Logger.d(TAG, "notify(), num = " + num);
            INotificationsListener iNotiListener = null;
            for (int i = 0; i < num; i++) {
                iNotiListener = (INotificationsListener) sNotificationsListeners.elementAt(i);
                Logger.d(TAG, "notify(), notification listener = " + iNotiListener);
                iNotiListener.notificationsReceived(intent);
            }
        }
    }
}

