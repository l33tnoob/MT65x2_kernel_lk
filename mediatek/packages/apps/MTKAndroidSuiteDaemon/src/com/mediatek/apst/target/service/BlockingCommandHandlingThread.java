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

import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.command.BaseCommand;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Class Name: BlockingCommandHandlingThread
 * <p>
 * Package: com.mediatek.apst.target.service
 * <p>
 * Created on: 2010-7-5
 * <p>
 * <p>
 * Description:
 * <p>
 * Thread synchronously handles commands.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public abstract class BlockingCommandHandlingThread extends Thread {
    private static final int DEFAULT_CMD_QUEUE_CAPACITY = 1000;

    private boolean mShouldTerminate;

    private int mMaxPriority;

    private BlockingQueue<BaseCommand> mCommandQueue;

    /**
     * Constructors.
     * 
     * @param queueCapacity
     *            The capacity of the queue.
     */
    public BlockingCommandHandlingThread(int queueCapacity) {
        super();
        mShouldTerminate = false;
        // Initialize command sending queue
        mCommandQueue = new LinkedBlockingQueue<BaseCommand>(queueCapacity);
        mMaxPriority = Thread.MAX_PRIORITY;
    }

    /**
     * constrctor.
     */
    public BlockingCommandHandlingThread() {
        this(DEFAULT_CMD_QUEUE_CAPACITY);
    }

    /**
     * @return Whether to terminate the thread.
     */
    public boolean isShouldTerminate() {
        return mShouldTerminate;
    }

    /**
     * @return The class name "BlockingCommandHandlingThread".
     */
    public String getClassName() {
        return "BlockingCommandHandlingThread";
    }

    /**
     * Set the flag as true to terminate.
     */
    public void terminate() {
        mShouldTerminate = true;
    }

    /**
     * Override it to implement the handle logic.
     * 
     * @param cmd
     *            Command to handle.
     */
    protected abstract void handle(BaseCommand cmd);

    /**
     * @param cmd
     *            The base command to enqueue.
     * @param waitWhenFull
     *            Whether to wait when the queue is full.
     * @return Whether success to enqueue the base command.
     */
    public synchronized boolean enqueue(BaseCommand cmd, boolean waitWhenFull) {
        // Add the command into queue
        if (waitWhenFull) {
            try {
                mCommandQueue.put(cmd);
                return true;
            } catch (InterruptedException e) {
                Debugger.logE(this.getClassName(), "enqueue", new Object[] {
                        cmd, waitWhenFull }, null, e);
                return false;
            }
        } else {
            boolean b = mCommandQueue.offer(cmd);
            if (b) {
                return true;
            } else {
                Debugger.logW(this.getClassName(), "enqueue", new Object[] {
                        cmd, waitWhenFull }, "Failed, command queue is full.");
                return false;
            }
        }
    }

    /**
     * @param waitWhenEmpty
     *            Whether to wait when queue is empty.
     * @return A BaseCommand or null.
     */
    private BaseCommand dequeue(boolean waitWhenEmpty) {
        // Retrieves and removes the head of the sending queue
        if (waitWhenEmpty) {
            try {
                return mCommandQueue.take();
            } catch (InterruptedException e) {
                Debugger.logE(this.getClassName(), "dequeue",
                        new Object[] { waitWhenEmpty }, null, e);
                return null;
            }
        } else {
            return mCommandQueue.poll();
        }
    }

    /**
     * @param priority
     *            The max priority to set.
     */
    public void setMaxPriority(int priority) {
        int maxPriority = priority;
        if (maxPriority > Thread.MAX_PRIORITY) {
            maxPriority = Thread.MAX_PRIORITY;
        } else if (maxPriority < Thread.MIN_PRIORITY) {
            maxPriority = Thread.MIN_PRIORITY;
        }
        mMaxPriority = maxPriority;
    }

    @Override
    public void run() {
        Debugger
                .logI(this.getClassName(), "run", null, "Thread start running.");
        while (!isShouldTerminate()) {
            BaseCommand cmd = dequeue(false);
            if (cmd != null) {
                // Set max priority to gain more execution chances when busy
                setPriority(mMaxPriority);
            } else {
                // Reduce priority to occupy less execution chances when idle
                if (getPriority() > Thread.MIN_PRIORITY) {
                    setPriority(getPriority() - 1);
                }
            }
            handle(cmd);
        }
        Debugger.logI(this.getClassName(), "run", null, "Thread terminated.");
    }
}
