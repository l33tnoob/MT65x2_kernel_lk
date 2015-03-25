/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.bluetooth.util;

import java.util.HashMap;

import android.app.Notification;

import com.mediatek.bluetooth.BluetoothProfile;

/**
 * Notification Id Management
 */
public class NotificationFactory {

    public static final int NID_SHARE_MGMT_NOTIFICATION = getProfileNotificationId(BluetoothProfile.ID_COMMON, 1);

    //private static final int CACHE_SIZE = 8;
    //private static HashMap<Integer, Notification> cache = new HashMap<Integer, Notification>(CACHE_SIZE);

    /**
     * get profile notification id ( all profiles have their own id range )
     *
     * @param profileId
     * @param notificationId
     * @return
     */
    public static int getProfileNotificationId( int profileId, int notificationId ){

        return BluetoothProfile.getProfileStart(profileId) + notificationId;
    }

    /**
     * get notification by notification Id
     *
     * @param id
     * @return
     */
//    public static synchronized  Notification getProfileNotification( int id, Builder notificationBuilder ){
//
//        if( !cache.containsKey(id) ){
//
//            // reset cache
//            if( cache.size() > CACHE_SIZE ){
//
//                cache.clear();
//            }
//
//            Notification n = new Notification();
//            cache.put(id, n);
//        }
//
//        // need to update "when" (or StatusBar management may cause IndexOutOfBoundException)
//        Notification n = cache.get(id);
//        n.when = System.currentTimeMillis();
//        n.defaults = 0;
//        n.flags = 0;
//        return n;
//    }
}
