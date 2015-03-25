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

import android.accounts.Account;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.test.AndroidTestCase;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.calendarimporter.BindServiceHelper;
import com.mediatek.calendarimporter.TestUtils;
import com.mediatek.calendarimporter.service.BaseProcessor;
import com.mediatek.calendarimporter.service.ExportProcessor;
import com.mediatek.calendarimporter.service.ImportProcessor;
import com.mediatek.calendarimporter.service.VCalService;
import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.VCalComposer;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

public class VCalServiceTest extends AndroidTestCase {
    private static final String TAG = "VCalServiceTest";
    private VCalService mService;
    BindServiceHelper mHelper;
    private Handler mUiHandler;

    // only to override the interface
    private class VCalServiceConnectedListener implements BindServiceHelper.ServiceConnectedOperation {

        @Override
        public void serviceUnConnected() {
            LogUtil.d(TAG, "serviceUnConnected.");
        }

        @Override
        public void serviceConnected(VCalService service) {
            LogUtil.d(TAG, "serviceConnected.");
            mService = service;
        }
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        super.tearDown();
        mService = null;
        mHelper.unBindService();
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mHelper = new BindServiceHelper(getContext(), new VCalServiceConnectedListener());
    }

    public void test01_ExportProcessor() {
        mHelper = new BindServiceHelper(getContext(), new VCalServiceConnectedListener());
        mHelper.onBindService();
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        int sleepTimes = 0;
        while (mService == null) {
            TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
            sleepTimes++;
            if (sleepTimes >= 10) {
                break;
            }
        }
        if (mService == null) {
            LogUtils.e(TAG, "Cannot bind to service.");
            return;
        }

        assertNotNull(mService);
        BaseProcessor processor = new BaseProcessor();
        mService.tryExecuteProcessor(processor);
        assertFalse(processor.cancel(true));
        try {
            assertNull(processor.get());
            assertNull(processor.get(1000, null));
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (ExecutionException e) {
            e.printStackTrace();
        } catch (TimeoutException e) {
            e.printStackTrace();
        }
        assertFalse(processor.isCancelled());
        assertFalse(processor.isDone());
        mService.tryCancelProcessor(processor);
        mService.tryCancelProcessor(null);

        createUiHandler();
        assertNotNull(mUiHandler);
        ExportProcessor exportProcessor = new ExportProcessor(mService, mUiHandler, Uri
                .parse("Content://com.android.calendar/1"));
        exportProcessor.vCalOperationCanceled(0, 0);
        exportProcessor.vCalOperationExceptionOccured(0, 0, 0);
        exportProcessor.vCalOperationFinished(0, 0, 0);
        exportProcessor.vCalProcessStatusUpdate(0, 0);
        exportProcessor.vCalOperationStarted(0);
        exportProcessor.cancel(true);
        mUiHandler = null;

        mService.disconnected(mHelper.getClass().getName());
        mService.onFinish();
    }

    public void test02_ImportProcessor() {
        createUiHandler();
        assertNotNull(mUiHandler);
        File vcsFile = TestUtils.addFile("testVcs.vcs", TestUtils.ONE_RIGHT_VCS_DATA);
        ImportProcessor processor = new ImportProcessor(getContext(), "PC Sync", mUiHandler, Uri.fromFile(vcsFile));
        processor.vCalOperationCanceled(0, 0);
        processor.vCalOperationExceptionOccured(0, 0, 0);
        processor.vCalOperationFinished(0, 0, 0);
        processor.vCalProcessStatusUpdate(0, 0);
        processor.vCalOperationStarted(0);
        processor.cancel(true);
        mUiHandler = null;
        TestUtils.removeFile(vcsFile);
    } 

    private void createUiHandler() {
        // try {
        // runTestOnUiThread(new Runnable() {
        // @Override
        // public void run() {
        // mUiHandler = new Handler() {
        // @Override
        // public void handleMessage(Message msg) {
        // super.handleMessage(msg);
        // }
        // };
        // }
        // });
        //
        // } catch (Throwable e) {
        // }
        mUiHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
            }
        };
    }
}
