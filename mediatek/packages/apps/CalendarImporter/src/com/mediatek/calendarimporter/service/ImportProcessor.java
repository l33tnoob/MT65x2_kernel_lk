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

import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;

import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.calendarimporter.utils.StringUtils;
import com.mediatek.vcalendar.VCalParser;
import com.mediatek.vcalendar.VCalStatusChangeOperator;

import java.io.FileNotFoundException;

public class ImportProcessor extends BaseProcessor implements VCalStatusChangeOperator {
    static final String TAG = "ImportProcessor";

    private VCalParser mParser;
    private final Handler mUiHandler;
    private boolean mIsBytesMode = false;

    /**
     * Import processor constructor, to parse the file with the given Uri.
     * 
     * @param context
     *            the context to get the resolver
     * @param accountName
     *            the target account name which will be parsed to
     * @param uiHandler
     *            the UI thread handler
     * @param uri
     *            the file or memoryData Uri, will create InputStream from this
     *            uri
     */
    public ImportProcessor(Context context, String accountName, Handler uiHandler, Uri uri) {
        mUiHandler = uiHandler;
        if (StringUtils.isNullOrEmpty(accountName)) {
            mParser = new VCalParser(uri, context, this);
        } else {
            LogUtils.d(TAG, "The dst accountName :" + accountName);
            mParser = new VCalParser(uri, accountName, context, this);
        }
    }


    /**
     * This is a template Constructor to parse the bytes from ImportReceiver, to
     * support previous backup & restore.
     * 
     * @param context
     *            the context to get the resolver
     * @param vcsContent
     *            the Vcs string
     * @param uiHandler
     *            the UI thread handler
     * @throws FileNotFoundException
     *            vcsContent is empty
     */
    public ImportProcessor(Context context, String vcsContent, Handler uiHandler) throws FileNotFoundException {
        if (vcsContent.length() <= 0) {
            LogUtils.e(TAG, "Constructor: the given vcsContent is empty.");
            throw new FileNotFoundException();
        }
        // Default Account: PC Sync
        mUiHandler = uiHandler;
        mIsBytesMode = true;
        mParser = new VCalParser(vcsContent, context, this);
    }

    @Override
    public void run() {
        super.run();
        if (mIsBytesMode) {
            LogUtils.d(TAG, "run: startParseVcsContent()");
            mParser.startParseVcsContent();
        } else {
            LogUtils.d(TAG, "run: mParser.startParse()");
            mParser.startParse();
        }
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        if (mParser != null) {
            mParser.cancelCurrentParse();
            mParser.close();
        }
        return super.cancel(mayInterruptIfRunning);
    }

    @Override
    public void vCalOperationCanceled(int finishedCnt, int totalCnt) {
        LogUtils.i(TAG, "vCalOperationCanceled,finishedCnt:" + finishedCnt + ",totalCnt:" + totalCnt);
        mParser.close();
    }

    @Override
    public void vCalOperationExceptionOccured(int finishedCnt, int totalCnt, int type) {
        LogUtils.w(TAG, "vCalOperationExceptionOccured,finishedCnt:" + finishedCnt + ",totalCnt:" + totalCnt + ",type:"
                + type);
        mParser.close();
        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_EXCEPTION;
        msg.arg1 = finishedCnt;
        msg.arg2 = type;

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
        LogUtils.d(TAG, "vCalProcessStatusUpdate: totalCnt: " + totalCnt);
    }

    @Override
    public void vCalOperationFinished(int successCnt, int totalCnt, Object obj) {
        LogUtils.i(TAG, "vCalOperationFinished: successCnt:" + successCnt + ",totalCnt:" + totalCnt);
        mParser.close();
        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_FINISH;
        msg.arg1 = successCnt;
        msg.arg2 = totalCnt;
        msg.obj = obj;
        mUiHandler.sendMessage(msg);
    }
}
