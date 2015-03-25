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
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.event.Event;
import com.mediatek.apst.target.event.EventDispatcher;
import com.mediatek.apst.target.event.IMmsListener;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.message.Mms;

/**
 * Class Name: MulMessageObserver
 * <p>
 * Package: com.mediatek.apst.target.service
 * <p>
 * <p>
 * Description:
 * <p>
 * Observe MMS database
 * <p>
 * 
 * @author mtk54043 yu.chen
 * 
 */

public class MulMessageObserver extends ContentObserver implements
        ISelfChangeObserver {

    MessageProxy mProxy;

    private boolean mSelfChangingContent = false;

    private boolean mObservering = false;

    private long mMaxMmsId = 0;

    public MulMessageObserver(Handler handler, MessageProxy proxy) {
        super(handler);
        this.mProxy = proxy;
    }

    public boolean isSelfChangingContent() {
        return mSelfChangingContent;
    }

    public void start() {
        if (null == mProxy) {
            Debugger.logE("Proxy is null.");
            return;
        }
        if (!mObservering) {
            // Initialize the last call time
            mMaxMmsId = mProxy.getMaxMmsId();
            mObservering = true;
            Debugger.logE(new Object[] {}, "Start observering mms content.");
            Debugger.logI(new Object[] {}, "getMaxMmsId :" + mMaxMmsId);
        }
    }

    public void stop() {
        mObservering = false;
        Debugger.logI("Stop observering mms content.");
    }

    @Override
    public void onChange(boolean selfChange) {
        // TODO Auto-generated method stub
        super.onChange(selfChange);
        if (mObservering) {
            long maxMmsId = mProxy.getMaxMmsId();
            synchronized (this) {
                if (!isSelfChangingContent()) {
                    Debugger.logI(new Object[] { selfChange },
                                    "Provide this change to PC side..");
                    long count = maxMmsId - mMaxMmsId;
                    if (count > 200) {
                        // so many mms are inserted? mms provider's change maybe cause mMaxMmsId is wrong,
                        // get latest mms to PC side.
                        Debugger.logW(new Object[] { selfChange }, "so many mms are inserted,get latest mms to PC side");
                        mMaxMmsId = maxMmsId - 1;
                        count = 1;
                    }
                    if (count > 0) {
                        for (int i = 0; i < count; i++) {
                            Debugger.logI(new Object[] { selfChange },
                                    "New MMS insert, id=" + (mMaxMmsId + i + 1));
                            Mms newMms = mProxy.getMms(mMaxMmsId + i + 1);
                            if (null != newMms) {
                                EventDispatcher
                                        .dispatchMmsInsertedEvent(new Event()
                                                .put(IMmsListener.BY_SELF,
                                                        false).put(
                                                        IMmsListener.MMS,
                                                        newMms));
                            } else {
                                Debugger.logI(new Object[] { selfChange },
                                        "New Mms is null");
                            }
                        }
                    } else {
                        /** For mms provider change , max mms id only be decreased */
                        maxMmsId = mMaxMmsId;
                        Debugger.logW(new Object[] {}, "Isn't inserting mms , Max mms id = " + mMaxMmsId);
                    }
                } else {
                    Debugger.logI(new Object[] { selfChange },
                            "Message content is changed by self.");
                    this.notify();
                }
            }
            mMaxMmsId = maxMmsId;
        }
    }

    @Override
    public boolean deliverSelfNotifications() {
        return false;
    }

    public void onSelfChangeStart() {
        mSelfChangingContent = true;
        // Debugger.logI("MMS Observer onSelfChangeStart()!");
    }

    public void onSelfChangeDone() {
        mSelfChangingContent = false;
        // Debugger.logI("MMS Observer onSelfChangeDone()!");
    }
    
    public long getMaxMmsId() {
        return mMaxMmsId;
    }

    /**
     * 
     * @param maxMmsId
     *            set max mms id
     */
    public void setMaxMmsId(long maxMmsId) {
        mMaxMmsId = maxMmsId;
    }

}
