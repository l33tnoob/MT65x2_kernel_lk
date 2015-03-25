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

package com.mediatek.apst.target.service;

import android.database.ContentObserver;
import android.os.Handler;

import com.mediatek.apst.target.data.proxy.ISelfChangeObserver;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.event.Event;
import com.mediatek.apst.target.event.EventDispatcher;
import com.mediatek.apst.target.event.IContactsListener;
import com.mediatek.apst.target.util.Debugger;

public class ContactsObserver extends ContentObserver implements
        ISelfChangeObserver {

    private ContactsProxy mProxy;

    private long mLastCallTime;

    private boolean mSelfChangingContent = false;

    private boolean mObservering = false;

    // ==============================================================
    // Constructors
    // ==============================================================
    public ContactsObserver(Handler handler, ContactsProxy proxy) {
        super(handler);
        if (handler == null) {
            Debugger.logW("Argument 'handler' is null in constructor!");
        }
        this.mProxy = proxy;
    }

    // ==============================================================
    // Getters
    // ==============================================================
    public boolean isSelfChangingContent() {
        return mSelfChangingContent;
    }

    // ==============================================================
    // Setters
    // ==============================================================

    // ==============================================================
    // Methods
    // ==============================================================
    public void start() {
        if (null == mProxy) {
            Debugger.logE("Proxy is null.");
            return;
        }
        if (!mObservering) {
            // Initialize the last call time
            mLastCallTime = mProxy.getLastCallTime();
            mObservering = true;
            Debugger.logI("Start observering contacts content.");
        }
    }

    public void stop() {
        mObservering = false;
        Debugger.logI("Stop observering contacts content.");
    }

    @Override
    public void onChange(boolean selfChange) {
        synchronized (this) {
            if (mObservering) {
                if (!isSelfChangingContent()) {
                    Debugger
                            .logI(new Object[] { selfChange },
                                    "Contacts content is changed by other applications!");
                    // Check whether the change is because of a call event
                    if (hasNewCall()) {
                        // Change is because of a call event, ignore
                        Debugger.logI(new Object[] { selfChange },
                                "Contacts content is changed because of new call "
                                        + "event, ignored.");
                        return;
                    }

                    EventDispatcher
                            .dispatchContactsContentChangedEvent(new Event()
                                    .put(IContactsListener.BY_SELF, false));
                } else {
                    Debugger.logI(new Object[] { selfChange },
                            "Contacts content is changed by self.");
                    EventDispatcher
                            .dispatchContactsContentChangedEvent(new Event()
                                    .put(IContactsListener.BY_SELF, true));
                    this.notify();
                }
            }
        }

    }

    // @Override
    public boolean deliverSelfNotifications() {
        return false;
    }

    private boolean hasNewCall() {
        // Get the last call time
        long newLastCallTime = mProxy.getLastCallTime();
        if (newLastCallTime > mLastCallTime) {
            mLastCallTime = newLastCallTime;
            return true;
        } else {
            return false;
        }
    }

    // @Override
    public void onSelfChangeStart() {
        mSelfChangingContent = true;
    }

    // @Override
    public void onSelfChangeDone() {
        mSelfChangingContent = false;
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================
}
