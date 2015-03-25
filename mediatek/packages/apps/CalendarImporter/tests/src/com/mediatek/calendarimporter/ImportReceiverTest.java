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

import android.app.Instrumentation;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.test.InstrumentationTestCase;
import android.text.StaticLayout;

import com.mediatek.calendarimporter.utils.LogUtils;

public class ImportReceiverTest extends InstrumentationTestCase {
    private String TAG = "ImportReceiverTest";
    private static final String ACTION = "com.mtk.intent.action.RESTORE";
    private static final String ACTION_RESULT = "com.mtk.intent.action.RESTORE.RESULT";
    private static final String VCS_CONTENT = "vcs_content";

    private ImportReceiver mReceiver = null;
    private ImportResultReceiver mResultReceiver = null;

    private Instrumentation mInst = null;
    private Context mTargetContext = null;

    private static class ImportResultReceiver extends BroadcastReceiver {
        private int broadCastCnt = 0;

        @Override
        public void onReceive(Context context, Intent intent) {
            broadCastCnt++;
        }

    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mTargetContext = mInst.getTargetContext();
        mReceiver = registImportReceiver(mTargetContext);
        mResultReceiver = registResultReceiver(mTargetContext);
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
        }
    }

    @Override
    protected void tearDown() throws Exception {
        mTargetContext.unregisterReceiver(mReceiver);
        mTargetContext.unregisterReceiver(mResultReceiver);
        super.tearDown();
    }

    public void test01_ImportReceiver() {
        LogUtils.i(TAG, "test01_ImportReceiver");
        final Intent intent = new Intent();
        intent.setAction(ACTION);
        intent.putExtra(VCS_CONTENT, TestUtils.eventString1.getBytes());
        final int resultCnt = mResultReceiver.broadCastCnt;
        mReceiver.onReceive(mTargetContext, intent);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME * 3);
        if (intent.getAction() == ACTION) {
            assertTrue(mResultReceiver.broadCastCnt > resultCnt);
        }
    }

    private static ImportReceiver registImportReceiver(Context context) {
        ImportReceiver receiver = new ImportReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION);
        context.registerReceiver(receiver, filter);
        return receiver;
    }

    private static ImportResultReceiver registResultReceiver(Context context) {
        ImportResultReceiver receiver = new ImportResultReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_RESULT);
        context.registerReceiver(receiver, filter);
        return receiver;
    }
}
