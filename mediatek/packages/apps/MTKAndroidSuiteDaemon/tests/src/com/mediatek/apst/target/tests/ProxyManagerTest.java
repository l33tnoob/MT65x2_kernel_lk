/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.apst.target.tests;

import android.content.Context;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.data.proxy.ProxyManager;
import com.mediatek.apst.target.data.proxy.app.ApplicationProxy;
import com.mediatek.apst.target.data.proxy.bookmark.BookmarkProxy;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.media.MediaProxy;
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;

public class ProxyManagerTest extends AndroidTestCase {
    private ContextBasedProxy mBaseProxy;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();

    }

    @Override
    protected void tearDown() throws Exception {
        mContext = null;
        mBaseProxy = null;
        super.tearDown();
    }

    public void test_getSystemInfoProxy() {
        mBaseProxy = ProxyManager.getSystemInfoProxy(mContext);
        assertTrue(mBaseProxy instanceof SystemInfoProxy);
    }

    public void test_getContactsProxy() {
        mBaseProxy = ProxyManager.getContactsProxy(mContext);
        assertTrue(mBaseProxy instanceof ContactsProxy);
    }

    public void test_getMessageProxy() {
        mBaseProxy = ProxyManager.getMessageProxy(mContext);
        assertTrue(mBaseProxy instanceof MessageProxy);

    }

    public void test_getCalendarProxy() {
        mBaseProxy = ProxyManager.getCalendarProxy(mContext);
        assertTrue(mBaseProxy instanceof CalendarProxy);
    }

    public void test_getApplicationProxy() {
        mBaseProxy = ProxyManager.getApplicationProxy(mContext);
        assertTrue(mBaseProxy instanceof ApplicationProxy);
    }

    public void test_getMediaProxy() {
        mBaseProxy = ProxyManager.getMediaProxy(mContext);
        assertTrue(mBaseProxy instanceof MediaProxy);
    }

    public void test_getBookmarkProxy() {
        mBaseProxy = ProxyManager.getBookmarkProxy(mContext);
        assertTrue(mBaseProxy instanceof BookmarkProxy);
    }
}
