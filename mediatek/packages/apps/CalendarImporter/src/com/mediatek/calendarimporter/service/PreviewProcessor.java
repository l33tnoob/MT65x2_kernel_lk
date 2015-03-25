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

import com.mediatek.calendarimporter.R;
import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.calendarimporter.utils.StringUtils;
import com.mediatek.vcalendar.ComponentPreviewInfo;
import com.mediatek.vcalendar.VCalParser;
import com.mediatek.vcalendar.VCalStatusChangeOperator;

public class PreviewProcessor extends BaseProcessor implements VCalStatusChangeOperator {
    private static final String TAG = "PreviewProcessor";
    private final VCalParser mParser;
    private final Handler mUiHandler;
    private final Context mContext;

    /**
     * Constructor for PreviewProcessor
     * 
     * @param context
     *            the context to get resource or resolver
     * @param fileUri
     *            the src file Uri.
     * @param uiHandler
     *            the UI thread handler.
     */
    public PreviewProcessor(Context context, Uri fileUri, Handler uiHandler) {
        mContext = context;
        mParser = new VCalParser(fileUri, context, this);
        mUiHandler = uiHandler;
    }

    @Override
    public void run() {
        LogUtils.d(TAG, TAG + ".run()");
        super.run();
        mParser.startParsePreview();
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        LogUtils.d(TAG, "cancel,mayInterruptIfRunning=" + mayInterruptIfRunning);
        mParser.close();
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
        mUiHandler.sendMessage(msg);
    }

    @Override
    public void vCalProcessStatusUpdate(int currentCnt, int totalCnt) {
        // do nothing
    }

    @Override
    public void vCalOperationStarted(int totalCnt) {
        LogUtils.d(TAG, "vCarOperationStarted: totalCnt: " + totalCnt);
    }

    @Override
    public void vCalOperationFinished(int successCnt, int totalCnt, Object obj) {
        LogUtils.i(TAG, "vCarOperationStarted: successCnt: " + successCnt + ",totalCnt:" + totalCnt);
        mParser.close();

        ComponentPreviewInfo previewInfo = (ComponentPreviewInfo) obj;
        StringBuilder builder = new StringBuilder();
        switch (previewInfo.componentCount) {
        case -1:
        case 0:
            LogUtils.w(TAG, "startParsePreview: No VEvent exsits in the file.");
            break;
        case 1:
            String title = previewInfo.eventSummary;
            String owner = previewInfo.eventOrganizer;
            String period = previewInfo.eventDuration;
            String emptyString = mContext.getResources().getString(R.string.null_name);

            title = StringUtils.isNullOrEmpty(title) ? emptyString : title;
            owner = StringUtils.isNullOrEmpty(owner) ? emptyString : owner;
            period = StringUtils.isNullOrEmpty(period) ? emptyString : period;

            builder.append(mContext.getResources().getString(R.string.title_lable) + title + "\n");
            builder.append(mContext.getResources().getString(R.string.calendar_lable) + owner + "\n");
            builder.append(mContext.getResources().getString(R.string.date_lable) + period);
            break;

        default: // more than one event.
            // owner = previewInfo.getFirstEventOrganizer();
            // if (StringUtils.isNullOrEmpty(owner)) {
            // owner = mContext.getResources().getString(R.string.null_name);
            // }
            // builder.append(mContext.getResources().getString(R.string.calendar_lable)).append(owner).append("\n");
            builder.append("Events Count:").append(previewInfo.componentCount).append("\n");
            break;
        }

        Message msg = Message.obtain();
        msg.what = ProcessorMsgType.PROCESSOR_FINISH;
        msg.arg1 = successCnt;
        msg.arg2 = totalCnt;
        msg.obj = builder.toString();

        if (msg.obj == null) {
            // Exception occurred
            msg.what = ProcessorMsgType.PROCESSOR_EXCEPTION;
        }
        mUiHandler.sendMessage(msg);
    }
}
