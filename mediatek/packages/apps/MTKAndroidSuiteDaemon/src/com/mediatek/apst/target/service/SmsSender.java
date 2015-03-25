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

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.telephony.SmsManager;

import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.message.Sms;

import java.util.ArrayList;
import java.util.List;

public class SmsSender {
    public static final String ACTION_SMS_SENT = "com.mediatek.apst.target.action.SMS_SENT";
    public static final String ACTION_SMS_DELIVERED = "com.mediatek.apst.target.action.SMS_DELIVERED";
    public static final String EXTRA_ID = "id";
    public static final String EXTRA_DATE = "date";
    // Singleton instance.
    private static SmsSender sInstance = null;
    private List<SendingTask> mSendingQueue;
    private boolean mShouldTerminate;
    private boolean mPause;
    private boolean mAllowSendNext;
    private BlockingSendingWorker mWorker;

    /**
     * Constructor.
     */
    private SmsSender() {
        this.mSendingQueue = new ArrayList<SendingTask>();
        this.mShouldTerminate = false;
        this.mAllowSendNext = true;
        this.mWorker = null;
    }

    /**
     * @return Whether should terminate the task;
     */
    public boolean isShouldTerminate() {
        return this.mShouldTerminate;
    }

    /**
     * @return Whether allow to send next.
     */
    public boolean isAllowSendNext() {
        return this.mAllowSendNext;
    }

    /**
     * @return An instance of the SmsSender.
     */
    public static synchronized SmsSender getInstance() {
        if (null == sInstance) {
            sInstance = new SmsSender();
        }
        return sInstance;
    }

    /**
     * Terminate the task.
     */
    public void terminate() {
        synchronized (this) {
            this.mShouldTerminate = true;
        }
    }

    /**
     * start the task.
     */
    public void start() {
        synchronized (this) {
            this.mShouldTerminate = false;
        }
    }

    /**
     * Set the flag of the mAllowSendNext as true.
     */
    public void allowSendNext() {
        synchronized (this) {
            this.mAllowSendNext = true;
        }
    }

    /**
     * Pause to task.
     */
    public void pause() {
        synchronized (this) {
            this.mPause = true;
        }
    }

    /**
     * Resume the task Set the flag of mPause as false.
     */
    public void resume() {
        synchronized (this) {
            this.mPause = false;
        }
    }

    /**
     * @param sms The sms to append.
     * @param context The current context.
     * @param simId The id of the sim.
     */
    public synchronized void appendTask(Sms sms, Context context, int simId) {
        mSendingQueue.add(new SendingTask(sms, context, simId));
        if (null == mWorker) {
            // Worker thread is not alive, start a new one
            mWorker = new BlockingSendingWorker();
            mWorker.start();
        }
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================
    private class BlockingSendingWorker extends Thread {

        /**
         * constructor.
         */
        public BlockingSendingWorker() {}

        @Override
        public void run() {
            long sleepInterval = 0L;
            SendingTask task = null;
            boolean sendThis;

            while (!isShouldTerminate()) {
                // This block should be synchronized
                synchronized (SmsSender.this) {
                    if (mSendingQueue.size() > 0) {
                        if (mAllowSendNext && !mPause) {
                            // Retrieves one task
                            task = mSendingQueue.remove(0);
                            sendThis = true;
                            mAllowSendNext = false;
                        } else {
                            task = null;
                            sendThis = false;
                        }
                    } else {
                        // No task left, terminate thread
                        break;
                    }
                }

                if (sendThis) {
                    Sms toSend = task.mSms;
                    SmsManager smsMgr = SmsManager.getDefault();
                    ArrayList<String> parts;
                    ArrayList<PendingIntent> sentIntents = null;
                    ArrayList<PendingIntent> deliveredIntents = null;
                    parts = smsMgr.divideMessage(toSend.getBody());
                    sentIntents = new ArrayList<PendingIntent>(parts.size());
                    /*
                     * deliveredIntents = new
                     * ArrayList<PendingIntent>(parts.size());
                     */
                    // Build pending intent on sent and delivered
                    /* String body = null; */
                    for (int i = 0; i < parts.size(); i++) {
                        /*
                         * if (null == body){ body = parts.get(0); } else { body
                         * += parts.get(0); }
                         */
                        sentIntents.add(PendingIntent.getBroadcast(
                                task.mContext, 0, new Intent(ACTION_SMS_SENT)
                                        // .putExtra("part", i)
                                        // .putExtra("total", parts.size())
                                        .putExtra(EXTRA_ID, toSend.getId())
                                        .putExtra(EXTRA_DATE, toSend.getDate())
                                // .putExtra("address", address)
                                // .putExtra("body", body)
                                , PendingIntent.FLAG_UPDATE_CURRENT));

                        /*
                         * deliveredIntents.add(PendingIntent.getBroadcast(
                         * task.context, 0, new Intent(ACTION_SMS_DELIVERED)
                         * //.putExtra("part", i) //.putExtra("total",
                         * parts.size()) .putExtra(EXTRA_ID, toSend.getId())
                         * .putExtra(EXTRA_DATE, toSend.getDate())
                         * //.putExtra("address", address) //.putExtra("body",
                         * body) , PendingIntent.FLAG_UPDATE_CURRENT));
                         */
                    }
                    // Do actual sending
                    if (Config.MTK_GEMINI_SUPPORT) {
                        // Dual-SIM
//                        android.telephony.gemini.GeminiSmsManager
//                                .sendMultipartTextMessageGemini(toSend
//                                        .getTarget().getAddress(), null, parts,
//                                        task.mSimId, sentIntents,
//                                        deliveredIntents);
										com.mediatek.telephony.SmsManagerEx.getDefault()
                                .sendMultipartTextMessage(toSend
                                        .getTarget().getAddress(), null, parts,
                                        sentIntents, deliveredIntents, task.mSimId);

                    } else {
                        // Single-SIM
                        smsMgr.sendMultipartTextMessage(toSend.getTarget()
                                .getAddress(), null, parts, sentIntents,
                                deliveredIntents);
                    }
                    sleepInterval = 0L;
                } else {
                    sleepInterval = 100L;
                }

                if (sleepInterval > 0) {
                    try {
                        sleep(sleepInterval);
                    } catch (InterruptedException e) {
                        Debugger.logE(e);
                    }
                }
            }
            // All current tasks done, terminate thread for saving resources
            // Set null to tell SmsSender to create new thread for later tasks
            mWorker = null;
        }

    }

    private class SendingTask {
        public Sms mSms;
        public Context mContext;
        public int mSimId;

        /**
         * @param sms The sms to send.
         * @param context The current context.
         * @param simId The id of the sim.
         */
        public SendingTask(Sms sms, Context context, int simId) {
            this.mSms = sms;
            this.mContext = context;
            this.mSimId = simId;
        }
    }
}
