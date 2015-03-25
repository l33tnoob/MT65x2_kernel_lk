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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package mediatek.app.cts;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.test.AndroidTestCase;
import android.test.PerformanceTestCase;

import android.util.Log;


public class ActivityTestsBase extends AndroidTestCase implements PerformanceTestCase,
        LaunchpadActivity.CallingTest {
    public static final String PERMISSION_GRANTED = "android.app.cts.permission.TEST_GRANTED";
    public static final String PERMISSION_DENIED = "android.app.cts.permission.TEST_DENIED";

    private static final int TIMEOUT_MS = 60 * 1000;

    protected Intent mIntent;

    private PerformanceTestCase.Intermediates mIntermediates;
    private String mExpecting;

    // Synchronization of activity result.
    private boolean mFinished;
    private int mResultCode = 0;
    private Intent mData;
    private RuntimeException mResultStack = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIntent = new Intent(mContext, LaunchpadActivity.class);
        mIntermediates = null;
    }

    @Override
    protected void tearDown() throws Exception {
        mIntermediates = null;
        super.tearDown();
    }

    public boolean isPerformanceOnly() {
        return false;
    }

    public void setInternalIterations(int count) {
    }

    public void startTiming(boolean realTime) {
        if (mIntermediates != null) {
            mIntermediates.startTiming(realTime);
        }
    }

    public void addIntermediate(String name) {
        if (mIntermediates != null) {
            mIntermediates.addIntermediate(name);
        }
    }

    public void addIntermediate(String name, long timeInNS) {
        if (mIntermediates != null) {
            mIntermediates.addIntermediate(name, timeInNS);
        }
    }

    public void finishTiming(boolean realTime) {
        if (mIntermediates != null) {
            mIntermediates.finishTiming(realTime);
        }
    }

    public void activityFinished(int resultCode, Intent data, RuntimeException where) {
        finishWithResult(resultCode, data, where);
    }

    public Intent editIntent() {
        return mIntent;
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    public int startPerformance(Intermediates intermediates) {
        mIntermediates = intermediates;
        return 1;
    }

    public void finishGood() {
        finishWithResult(Activity.RESULT_OK, null);
    }

    public void finishBad(String error) {
        finishWithResult(Activity.RESULT_CANCELED, new Intent().setAction(error));
    }

    public void finishWithResult(int resultCode, Intent data) {
        final RuntimeException where = new RuntimeException("Original error was here");
        where.fillInStackTrace();
        finishWithResult(resultCode, data, where);
    }

    public void finishWithResult(int resultCode, Intent data, RuntimeException where) {
        synchronized (this) {
            mResultCode = resultCode;
            mData = data;
            mResultStack = where;
            mFinished = true;
            notifyAll();
        }
    }

    public int runLaunchpad(String action) {
        startLaunchpadActivity(action);
        return waitForResultOrThrow(TIMEOUT_MS);
    }

    private void startLaunchpadActivity(String action) {
        LaunchpadActivity.setCallingTest(this);

        synchronized (this) {
            mIntent.setAction(action);
            mFinished = false;
            mIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(mIntent);
        }
    }

    public int waitForResultOrThrow(int timeoutMs) {
        return waitForResultOrThrow(timeoutMs, null);
    }

    public int waitForResultOrThrow(int timeoutMs, String expected) {
        final int res = waitForResult(timeoutMs, expected);

        if (res == Activity.RESULT_CANCELED) {
            if (mResultStack != null) {
                throw new RuntimeException(mData != null ? mData.toString() : "Unable to launch",
                        mResultStack);
            } else {
                throw new RuntimeException(mData != null ? mData.toString() : "Unable to launch");
            }
        }
        return res;
    }

    public int waitForResult(int timeoutMs, String expected) {
        mExpecting = expected;

        final long endTime = System.currentTimeMillis() + timeoutMs;

        boolean timeout = false;
        synchronized (this) {
            while (!mFinished) {
                final long delay = endTime - System.currentTimeMillis();
                if (delay < 0) {
                    Log.v("hihi", "timeout = true ");
                    timeout = true;
                    break;
                }

                try {
                    wait(delay);
                } catch (final java.lang.InterruptedException e) {
                    // do nothing
                }
            }
        }

        mFinished = false;

        if (timeout) {
            Log.v("hihi", "timeout ");
            mResultCode = Activity.RESULT_CANCELED;
            onTimeout();
        }
        return mResultCode;
    }


    public int getResultCode() {
        return mResultCode;
    }

    public Intent getResultData() {
        return mData;
    }

    public RuntimeException getResultStack() {
        return mResultStack;
    }

    public void onTimeout() {
        final String msg = mExpecting == null ? "Timeout" : "Timeout while expecting " + mExpecting;
        finishWithResult(Activity.RESULT_CANCELED, new Intent().setAction(msg));
    }
}
