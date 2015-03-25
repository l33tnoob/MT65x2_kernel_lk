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

import java.io.File;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ListView;

import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.calendarimporter.utils.Utils;

public class HandleProgressActivityTest extends ActivityInstrumentationTestCase2<HandleProgressActivity> {
    private static final String TAG = "HandleProgressActivity";
    private HandleProgressActivity mProgressActivity;
    private Uri mUri;
    private static final int ID_DIALOG_NO_CALENDAR_ALERT = 1;
    private static final int ID_DIALOG_PROGRESS_BAR = 2;
    private Context mContext;
    private File mFile;

    public HandleProgressActivityTest() {
        super(HandleProgressActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        TestUtils.addMockAccount(mContext, "test");
        // create the file and set the uri
        mFile = TestUtils.addFile("testVcs.vcs", TestUtils.ONE_RIGHT_VCS_DATA);
        mUri = Uri.fromFile(mFile);
    }

    @Override
    protected void tearDown()  throws Exception {
        TestUtils.removeTestAccounts(mContext);
        // delete the file.
        TestUtils.removeFile(mFile);
        super.tearDown();
    }

    public void test01_testStartProgressActivity() {
        LogUtils.i(TAG, "test01_testHandleProgressActivity");
        setProgressActivity();
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        assertNotNull(mProgressActivity);
    }

    public void test02_testHandleProgress() {
        Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        LogUtils.i(TAG, "test01_testHandleProgressActivity");
        setProgressActivity();
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        assertNotNull(mProgressActivity);
        mProgressActivity.finish();
    }

    public void test03_DialogsLife() {
        Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        LogUtils.i(TAG, "test03_DialogsLife");
        setProgressActivity();
        mProgressActivity.showDialog(ID_DIALOG_NO_CALENDAR_ALERT);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        mProgressActivity.removeDialog(ID_DIALOG_NO_CALENDAR_ALERT);

        mProgressActivity.showDialog(ID_DIALOG_PROGRESS_BAR);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        mProgressActivity.removeDialog(ID_DIALOG_PROGRESS_BAR);
        mProgressActivity.finish();

    }

    public void test04_onNewIntent() {
        final Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        LogUtils.i(TAG, "test03_DialogsLife");
        setProgressActivity();

        try {
            runTestOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mProgressActivity.showDialog(ID_DIALOG_NO_CALENDAR_ALERT);
                    TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
                    mProgressActivity.onNewIntent(i);

                    mProgressActivity.showDialog(ID_DIALOG_PROGRESS_BAR);
                    TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
                    mProgressActivity.onNewIntent(i);
                }
            });

        } catch (Throwable e) {
        }

        mProgressActivity.finish();
    }

    public void test05_testStartParse() {
        Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        LogUtils.i(TAG, "test01_testHandleProgressActivity");
        setProgressActivity();
        assertNotNull(mProgressActivity);
        ListView accountList = (ListView) mProgressActivity.findViewById(R.id.account_list);

        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        if (Utils.hasExchangeOrGoogleAccount(mProgressActivity)) {
            assertTrue(accountList.getVisibility() == View.VISIBLE);
            if (accountList.getChildCount() > 0) {
                View view = (View) accountList.getChildAt(0);
                TouchUtils.clickView(this, view);
                TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
                getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
                TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
            }
        }
        if (mProgressActivity != null) {
            mProgressActivity.finish();
        }
    }

    public void setProgressActivity() {
        try {
            mProgressActivity = getActivity();
            TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        } catch (Exception e) {
        }

    }

}
