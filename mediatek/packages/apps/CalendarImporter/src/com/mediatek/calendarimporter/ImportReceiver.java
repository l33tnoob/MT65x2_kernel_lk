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

package com.mediatek.calendarimporter;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.mediatek.calendarimporter.BindServiceHelper.ServiceConnectedOperation;
import com.mediatek.calendarimporter.service.ImportProcessor;
import com.mediatek.calendarimporter.service.ProcessorMsgType;
import com.mediatek.calendarimporter.service.VCalService;
import com.mediatek.calendarimporter.utils.LogUtils;

import java.io.FileNotFoundException;

public class ImportReceiver extends BroadcastReceiver implements ServiceConnectedOperation {

    private static final String TAG = "ImportReceiver";

    private static final String ACTION = "com.mtk.intent.action.RESTORE";
    private static final String ACTION_RESULT = "com.mtk.intent.action.RESTORE.RESULT";
    private static final String VCS_CONTENT = "vcs_content";

    private BindServiceHelper mServiceHelper;
    private VCalService mService;
    private ImportProcessor mProcessor;
    private Context mContext;

    private String mVcsContent;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        mContext = context;
        Context appContext = mContext.getApplicationContext();
        LogUtils.i(TAG, "action = " + action);
        if (action.equals(ACTION)) {
            /// M: the vcs content may be null
            byte[] vcs = intent.getByteArrayExtra(VCS_CONTENT);
            mVcsContent = (vcs != null) ? new String(vcs) : new String("");
            LogUtils.d(TAG, "onReceive,file length: " + mVcsContent.length());
            mServiceHelper = new BindServiceHelper(appContext, this);
            LogUtils.d(TAG, "Context: " + appContext);
            mServiceHelper.onBindService();
        }
    }

    @Override
    public void serviceConnected(VCalService service) {
        LogUtils.d(TAG, "Receiver: service:" + service);
        mService = service;
        Handler handler = new Handler() {
            public void handleMessage(Message msg) {
                Intent intentResult = new Intent(ACTION_RESULT);
                switch (msg.what) {
                case ProcessorMsgType.PROCESSOR_FINISH:
                    Bundle eventInfo = (Bundle) msg.obj;
                    String eventTitle = eventInfo.getString("event_title", null);
                    intentResult.putExtra("event_title", eventTitle);
                    if (msg.arg1 == msg.arg2) {
                        intentResult.putExtra("isSuccess", true);
                    } else {
                        intentResult.putExtra("isSuccess", false);
                    }
                    mContext.sendBroadcast(intentResult);
                    LogUtils.d(TAG, "sendBroadcast, action= " + intentResult.getAction());
                    break;
                case ProcessorMsgType.PROCESSOR_EXCEPTION:
                    intentResult.putExtra("isSuccess", false);
                    mContext.sendBroadcast(intentResult);
                    break;

                default:
                    break;
                }
            }
        };
        try {
            mProcessor = new ImportProcessor(mService, mVcsContent, handler);
        } catch (FileNotFoundException e) {
            LogUtils.e(TAG, "Can not create the Processor for a empty VcsContent.");
            e.printStackTrace();
        }
        mService.tryExecuteProcessor(mProcessor);
    }

    @Override
    public void serviceUnConnected() {
        mProcessor.cancel(false);
        mService = null;
    }
}
