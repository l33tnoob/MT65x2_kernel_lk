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

package com.mediatek.bluetooth.opp.adp;

import android.os.Process;

import com.mediatek.bluetooth.opp.mmi.OppLog;

public class OppTaskWorkerThread extends Thread {

    private boolean mHasMoreTask = false;

    private OppTaskHandler mHandler;

    public OppTaskWorkerThread(String name, OppTaskHandler handler) {

        super(name + "WorkerThread");
        this.mHandler = handler;
    }

    @Override
    public void run() {

        OppLog.d("OppTask worker thread start: thread name - " + this.getName());

        // need to increase the priority (higher than message-listener thread)
        // or the event will be queued in EventQueue
        Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND - 3);

        // loop until be interrupted
        while (!this.isInterrupted()) {

            try {
                // before wait (for new task)
                OppLog.d("OppTaskWorkerThread[" + this.getName() + "]: 1. beforeWait()");
                if (!this.mHandler.beforeWait()) {

                    continue;
                }

                // wait for task
                OppLog.d("OppTaskWorkerThread[" + this.getName() + "]: 2. waitNewTask()");
                this.waitNewTask();

                // after wait (new task arrival)
                OppLog.d("OppTaskWorkerThread[" + this.getName() + "]: 3. afterWait()");
                this.mHandler.afterWait();

                // finish this loop
                OppLog.d("OppTaskWorkerThread[" + this.getName() + "]: 4. next loop");
            } catch (InterruptedException ex) {

                OppLog.i("OppTaskWorkerThread[" + this.getName() + "] interrupted: "
                        + ex.getMessage());
                break;
            }
        }
        OppLog.d("OppTaskWorkerThread[" + this.getName() + "] stopped.");
    }

    public synchronized void waitNewTask() throws InterruptedException {

        // only wait when no more task
        if (!this.mHasMoreTask) {

            this.wait();
        }

        // all tasks will be processed below
        this.mHasMoreTask = false;
    }

    public synchronized void notifyNewTask() {

        OppLog.d("notify new task for OppTaskWorkerThread[" + this.getName() + "]");

        this.mHasMoreTask = true;
        this.notify();
    }
}
