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

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;

import com.mediatek.calendarimporter.utils.LogUtils;

public class ShowPreviewActivityTest extends ActivityInstrumentationTestCase2<ShowPreviewActivity> {

    private String TAG = "ShowPreviewActivityTest";
    private ShowPreviewActivity mPreviewActivity;
    private Uri mUri;
    private Context mTargetContext;
    private ContentResolver mResolver;

    public ShowPreviewActivityTest() {
        super(ShowPreviewActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mTargetContext = getInstrumentation().getTargetContext();
        mResolver = mTargetContext.getContentResolver();
    }

    public void test01_startPreviewFromDBUri() {
        LogUtils.i(TAG, "test_startPreviewActivity");
        mUri = TestUtils.addOneEventsToDB(mResolver); // add event
        Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        mPreviewActivity = getActivity();
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        assertNotNull(mPreviewActivity);
        mPreviewActivity.finish();
        TestUtils.removeTheAddedEvent(mResolver, mUri);// delete the added event
    }

    public void test02_startPreviewFromWrongFile() {
        File wrongVcsFile = TestUtils.addFile("wrongVcs.vcs", TestUtils.ONE_WRONG_VCS_DATA);
        mUri = Uri.fromFile(wrongVcsFile);
        Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        mPreviewActivity = getActivity();
        assertNotNull(mPreviewActivity);
        do {
            TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        } while (mPreviewActivity.findViewById(R.id.preview_loading).getVisibility() != View.GONE);
        assertTrue(mPreviewActivity.findViewById(R.id.import_error_certain).getVisibility() == View.VISIBLE);
        mPreviewActivity.finish();
        TestUtils.removeFile(wrongVcsFile);
    }

    public void test03_startJudgeAccountActivity() {
        TestUtils.addMockAccount(mTargetContext, "test");
        File rightVcsFile = TestUtils.addFile("rightVcs.vcs", TestUtils.ONE_RIGHT_VCS_DATA);
        mUri = Uri.fromFile(rightVcsFile);
        Intent i = new Intent();
        i.setData(mUri);
        setActivityIntent(i);
        mPreviewActivity = getActivity();
        assertNotNull(mPreviewActivity);
        do {
            TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        } while (mPreviewActivity.findViewById(R.id.preview_loading).getVisibility() != View.GONE);

        Button importButton = (Button) mPreviewActivity.findViewById(R.id.button_ok);
        try {
            TouchUtils.clickView(this, importButton);
        } catch (NullPointerException e) {
        }

        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        getInstrumentation().sendCharacterSync(KeyEvent.KEYCODE_BACK);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        assertTrue(mPreviewActivity.isFinishing());
        TestUtils.removeTestAccounts(mTargetContext);
        TestUtils.removeFile(rightVcsFile);
    }
}
