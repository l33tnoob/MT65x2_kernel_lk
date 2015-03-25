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
 */
package com.mediatek.calendarimporter.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

import com.mediatek.calendarimporter.utils.LogUtils;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;

public class VCalService extends Service {
    private static final String TAG = "VCalService";
    private MyBinder mBinder = null;

    public final ExecutorService mExecutorService = Executors.newSingleThreadExecutor();

    public class MyBinder extends Binder {
        /**
         * Get the binded service instance.
         * 
         * @return the binded service
         */
        public VCalService getService() {
            return VCalService.this;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mBinder = new MyBinder();
        LogUtils.d(TAG, "VCalService onCreate");
    }

    @Override
    public IBinder onBind(Intent intent) {
        LogUtils.d(TAG, "VCalService onBind");
        return mBinder;
    }

    /**
     * Finish the service
     */
    public void onFinish() {
        super.onDestroy();
    }

    /**
     * disconnect the service
     * 
     * @param name
     *            the context's name binded to this service
     */
    public void disconnected(String name) {
        LogUtils.i(TAG, "disconnected, the context = " + name);
    }

    /**
     * try to execute the processor
     * 
     * @param processor
     *            the created processor, such as previewProcessor etc.
     */
    public void tryExecuteProcessor(BaseProcessor processor) {
        LogUtils.d(TAG, "VCalService tryExecuteProcessor");
        tryExecute(processor);
    }

    /**
     * try to cancel the processor
     * 
     * @param processor
     *            the created processor, such as previewProcessor etc.
     */
    public void tryCancelProcessor(BaseProcessor processor) {
        LogUtils.d(TAG, "VCalService tryCancelProcessor");
        if (processor == null) {
            LogUtils.w(TAG, "The processor going to cancel is null");
            return;
        }
        processor.cancel(true);
    }

    /**
     * Tries to call {@link ExecutorService#execute(Runnable)} toward a given
     * processor.
     * @param processor
     *            the created processor, such as previewProcessor etc.
     * @return true when successful.
     */
    private synchronized boolean tryExecute(BaseProcessor processor) {
        try {
                mExecutorService.execute(processor);
            return true;
        } catch (RejectedExecutionException e) {
            LogUtils.e(TAG, "tryExecute: RejectedExecutionException.");
            return false;
        }
    }
}
