/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.apst.target.service;

import android.database.ContentObserver;
import android.os.Handler;

import com.mediatek.apst.target.data.proxy.ISelfChangeObserver;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.event.Event;
import com.mediatek.apst.target.event.EventDispatcher;
import com.mediatek.apst.target.event.ICalendarEventListener;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.calendar.CalendarEvent;

public class CalendarEventObserver extends ContentObserver implements
        ISelfChangeObserver {

    private CalendarProxy mProxy;
    private boolean mSelfChangingContent = false;
    private boolean mObservering = false;
    private long mCalendarEventId = 0;

    /**
     * @param handler handler The handler to run {@link #onChange} on. 
     * @param proxy A instance of the calendar proxy.
     */
    public CalendarEventObserver(Handler handler, CalendarProxy proxy) {
        super(handler);
        if (handler == null) {
            Debugger.logW("Argument 'handler' is null in constructor!");
        }
        this.mProxy = proxy;
    }

    /**
     * @return Whether is self change content. 
     */
    public boolean isSelfChangingContent() {
        return mSelfChangingContent;
    }

    /**
     * 
     */
    public void start() {
        if (null == mProxy) {
            Debugger.logE("Proxy is null.");
            return;
        }
        if (!mObservering) {
            // Initialize the largest event id.
            mCalendarEventId = mProxy.getMaxEventId();
            mObservering = true;
            Debugger.logI("Start observering message content.");
        }
    }

    public void stop() {
        mObservering = false;
        Debugger.logI("Stop observering calendar event content.");
    }

    @Override
    public void onChange(boolean selfChange) {
        long maxCalendarEventId = mProxy.getMaxEventId();
        Debugger.logW(new Object[] { selfChange },
                "synchronized maxCalendarEventId = " + maxCalendarEventId);
        synchronized (this) {
            if (mObservering) {
                if (!isSelfChangingContent()) {
                    Debugger
                            .logI(new Object[] { selfChange },
                                    "CalendarEvent content is changed by other applications!");

                    if (maxCalendarEventId > mCalendarEventId) {
                        Debugger.logD(new Object[] { selfChange },
                                "New event, id=" + maxCalendarEventId);
                        CalendarEvent event = mProxy.getEvent(
                                maxCalendarEventId, true, true);

                        EventDispatcher
                                .dispatchCalendarEventChangedEvent(new Event()
                                        .put(ICalendarEventListener.BY_SELF,
                                                false)
                                        .put(
                                                ICalendarEventListener.CALENDAREVENT,
                                                event));
                    }
                } else {
                    Debugger.logI(new Object[] { selfChange },
                            "Event content is changed by self.");
                    this.notify();
                }
            }
            mCalendarEventId = maxCalendarEventId;
        }
    }

    @Override
    public boolean deliverSelfNotifications() {
        return false;
    }

    public void onSelfChangeStart() {
        mSelfChangingContent = true;
        Debugger.logI("CalendarEvent onSelfChangeStart()!");
    }

    public void onSelfChangeDone() {
        mSelfChangingContent = false;
        Debugger.logI("CalendarEvent onSelfChangeDone()!");
    }

}
