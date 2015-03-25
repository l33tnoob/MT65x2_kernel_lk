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

import android.content.Intent;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;

import com.mediatek.calendarimporter.R.string;
import com.mediatek.calendarimporter.utils.LogUtils;

public class ShowHandleResultActivityTest extends ActivityInstrumentationTestCase2<ShowHandleResultActivity> {
    private static final String TAG = "ShowHandleResultActivityTest"; 

    private ShowHandleResultActivity mActivity;

    public ShowHandleResultActivityTest() {
        super(ShowHandleResultActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        setActivityInitialTouchMode(false);
    }

    @Override
    protected void tearDown() throws Exception {
        if (mActivity != null) {
            mActivity.finish();
            mActivity = null;
        }
        super.tearDown();
    }

    public void test01_startShowResultActivity() {
        Intent i = new Intent();
        i.putExtra("eventStartTime", System.currentTimeMillis());
        i.putExtra("SucceedCnt", 0);
        i.putExtra("totalCnt", 0);
        setActivityIntent(i);
        LogUtils.i(TAG, "test01_startShowResultActivity get activity.");
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME * 3);
        mActivity = getActivity();
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        assertNotNull(mActivity);

        View openButton = mActivity.findViewById(R.id.button_open);

        LogUtils.i(TAG, "test01_startShowResultActivity sleep started.");
        int sleepTimes = 0;
        do {
            sleepTimes++;
            TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        } while (openButton.getVisibility() != View.VISIBLE && sleepTimes < 5);
        LogUtils.i(TAG, "test01_startShowResultActivity sleep ended.");

        TouchUtils.clickView(this, openButton);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        LogUtils.i(TAG, "test01_startShowResultActivity ended.");
    }
}
