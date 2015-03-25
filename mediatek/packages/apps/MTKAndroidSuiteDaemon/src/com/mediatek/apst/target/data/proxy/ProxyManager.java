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

package com.mediatek.apst.target.data.proxy;

import android.content.Context;

import com.mediatek.apst.target.data.proxy.app.ApplicationProxy;
import com.mediatek.apst.target.data.proxy.bookmark.BookmarkProxy;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.media.MediaProxy;
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;

/**
 * Class Name: ProxyManager
 * <p>
 * Package: com.mediatek.apst.target.data.proxy
 * <p>
 * Created on: 2010-6-18
 * <p>
 * <p>
 * Description:
 * <p>
 * An factory class for creating instances of proxy classes. External code
 * should get new proxy instances with this factory class's creator methods, but
 * not with the proxy classes' constructor directly. This factory class will
 * create and return new instances of appropriate classes according to arguments
 * provided by the caller.
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public abstract class ProxyManager {
    /**
     * Create and return an instance of system info proxy.
     * @param context 
     * @return An instance of system info proxy.
     * @see SystemInfoProxy
     */
    public static final SystemInfoProxy getSystemInfoProxy(Context context) {
        return SystemInfoProxy.getInstance(context);
    }

    /**
     * Create and return an instance of contacts proxy.
     * 
     * @param context
     *            Context needed by the the proxy to get a ContentResolver.
     * @return An instance of contacts proxy.
     * @see ContactsProxy
     */
    public static final ContactsProxy getContactsProxy(Context context) {
        return ContactsProxy.getInstance(context);
    }

    /**
     * Create and return an instance of message proxy.
     * 
     * @param context
     *            Context needed by the the proxy to get a ContentResolver.
     * @return An instance of message proxy.
     * @see MessageProxy
     */
    public static final MessageProxy getMessageProxy(Context context) {
        return MessageProxy.getInstance(context);
    }

    /**
     * Create and return an instance of calendar proxy.
     * 
     * @param context
     *            Context needed by the the proxy to get a ContentResolver.
     * @return An instance of calendar proxy.
     * @see CalendarProxy
     */
    public static final CalendarProxy getCalendarProxy(Context context) {
        return CalendarProxy.getInstance(context);
    }

    /**
     * Create and return an instance of application proxy.
     * 
     * @param context
     *            Context needed by the the proxy to get a ContentResolver.
     * @return An instance of application proxy.
     * @see ApplicationProxy
     */
    public static final ApplicationProxy getApplicationProxy(Context context) {
        return ApplicationProxy.getInstance(context);
    }

    /**
     * Create and return an instance of media proxy.
     * 
     * @param context
     *            Context needed by the the proxy to get a ContentResolver.
     * @return An instance of media proxy.
     * @see MediaProxy
     */
    public static final MediaProxy getMediaProxy(Context context) {
        return MediaProxy.getInstance(context);
    }

    /**
     * Create and return an instance of bookmark Proxy.
     * 
     * @param context Context
     *            needed by the the proxy to get a ContentResolver.
     * @return An instance of media proxy.
     * @see MediaProxy
     */
    public static final BookmarkProxy getBookmarkProxy(Context context) {
        return BookmarkProxy.getInstance(context);
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================

}
