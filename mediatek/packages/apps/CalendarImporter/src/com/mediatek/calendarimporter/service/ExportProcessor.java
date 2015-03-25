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

import android.accounts.Account;
import android.content.ContentUris;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.CalendarContract.Events;

import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.vcalendar.VCalComposer;
import com.mediatek.vcalendar.VCalStatusChangeOperator;

import java.io.File;

public class ExportProcessor extends BaseProcessor implements VCalStatusChangeOperator {
    private static final String TAG = "ExportProcessor";

    private final VCalService mService;
    private final VCalComposer mComposer;
    private final Handler mUiHandler;

    private final int mMode;

    public static final int FILE_MODE = 0;
    public static final int MEMORY_FILE_MODE = 1;

    /**
     * Constructor for Memory file compose.
     * 
     * @param service
     *            the service to control this processor
     * @param uiHandler
     *            the main thread handler
     * @param eventsUri
     *            the source event
     * @throws IllegalArgumentException
     *             the eventId is illegal
     */
    public ExportProcessor(VCalService service, Handler uiHandler, Uri eventsUri) throws IllegalArgumentException {
        mService = service;
        // TODO get selection from eventsUri
        long eventId = -1;
        eventId = ContentUris.parseId(eventsUri);
        if (eventId < 0) {
            LogUtils.e(TAG, "Constructor,The given eventId is inlegal or empty, eventId :" + eventId);
            throw new IllegalArgumentException(eventsUri.toString());
        }
        String selection = "_id=" + String.valueOf(eventId) + " AND " + Events.DELETED + "!=1";
        LogUtils.i(TAG, "Constructor: the going query selection = \"" + selection + "\"");
        mUiHandler = uiHandler;
        mComposer = new VCalComposer(mService, selection, this);
        mMode = MEMORY_FILE_MODE;
    }

    @Override
    public void run() {
        super.run();
        LogUtils.d(TAG, "ExportProcessor.run() has been called,mode=" + mMode);
        if (mMode == MEMORY_FILE_MODE) {
            //do not thing , memory file has used the sync flow
            LogUtils.w(TAG, "ExportProcessor.run() MEMORY_FILE_MODE should not be called.");
        }
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        LogUtils.i(TAG, "cancel,mayInterruptIfRunning:" + mayInterruptIfRunning);
        if (mComposer != null) {
            mComposer.cancelCurrentCompose();
        }
        return super.cancel(mayInterruptIfRunning);
    }

    @Override
    public void vCalOperationCanceled(int finishedCnt, int totalCnt) {
        LogUtils.i(TAG, "vCalOperationCanceled,finishedCnt:" + finishedCnt + ",totalCnt:" + totalCnt);
        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_CANCEL;
        msg.arg1 = finishedCnt;
        msg.arg2 = totalCnt;

        mUiHandler.sendMessage(msg);
    }

    @Override
    public void vCalOperationExceptionOccured(int finishedCnt, int totalCnt, int type) {
        LogUtils.w(TAG, "vCalOperationExceptionOccured,finishedCnt:" + finishedCnt + ",totalCnt:" + totalCnt + ",type:"
                + type);
        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_EXCEPTION;
        msg.arg1 = finishedCnt;
        msg.arg2 = totalCnt;

        mUiHandler.sendMessage(msg);
    }

    @Override
    public void vCalProcessStatusUpdate(int currentCnt, int totalCnt) {
        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_STATUS_UPDATE;
        msg.arg1 = currentCnt;
        msg.arg2 = totalCnt;
        mUiHandler.sendMessage(msg);
    }

    @Override
    public void vCalOperationStarted(int totalCnt) {
        LogUtils.d(TAG, "vCarOperationStarted: totalCnt: " + totalCnt);
    }

    @Override
    public void vCalOperationFinished(int successCnt, int totalCnt, Object obj) {
        LogUtils.i(TAG, "vCalOperationFinished: successCnt:" + successCnt + ",totalCnt:" + totalCnt);
        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_FINISH;
        msg.arg1 = successCnt;
        msg.arg2 = totalCnt;
        msg.obj = obj;
        mUiHandler.sendMessage(msg);
    }
}
