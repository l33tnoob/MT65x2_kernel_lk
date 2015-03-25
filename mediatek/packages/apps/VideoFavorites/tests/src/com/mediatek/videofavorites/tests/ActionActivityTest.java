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

package com.mediatek.videofavorites.tests;

import com.mediatek.videofavorites.VideoFavoritesProvider;
import com.mediatek.videofavorites.VideoFavoritesProviderValues;
import com.mediatek.videofavorites.WidgetActionActivity;
import com.mediatek.videofavorites.WidgetAdapter;
import com.mediatek.xlog.Xlog;

import com.jayway.android.robotium.solo.Solo;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.ContentValues;
import android.content.Context;
import android.content.IntentFilter;
import android.content.Intent;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;

import java.io.File;
import java.util.ArrayList;
import java.util.Locale;

public class ActionActivityTest extends ActivityInstrumentationTestCase2<WidgetActionActivity> {

    private static final String TAG = "VF.ActionActivityTest";

    private Instrumentation mInstrumentation;
    private WidgetActionActivity mActivity;
    private Solo mSolo;

    public ActionActivityTest() {
        super(WidgetActionActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
    }

    // this activity is launched with various extras.
    protected void setUpActivityWithIntent(Intent i) {
        setActivityIntent(i);
        mActivity = getActivity();

        mSolo = new Solo(mInstrumentation, mActivity);
        mInstrumentation.waitForIdleSync();
    }

    @Override
    protected void tearDown() throws Exception {
        mSolo.finishOpenedActivities();
        super.tearDown();
    }

    private static final int DUMMY_ID = 999; // just a dummy data
    private Uri getDummyUri() {
        Uri uri = Uri.parse(String.format(Locale.US, "%s/%d",
                                          VideoFavoritesProviderValues.Columns.CONTENT_URI.toString() , DUMMY_ID));

        return uri;
    }

    public void test010DeleteAction() {
        Intent intent = new Intent(Intent.ACTION_DELETE, getDummyUri());
        intent.setAction(Intent.ACTION_DELETE).putExtra(WidgetAdapter.KEY_NAME, "John");
        setUpActivityWithIntent(intent);

        Xlog.v(TAG, mSolo.getString(com.mediatek.videofavorites.R.string.ok));
        mSolo.clickOnButton(mSolo.getString(com.mediatek.videofavorites.R.string.ok));
    }

    public void test020DeleteActioncancel() {
        Intent intent = new Intent(Intent.ACTION_DELETE, getDummyUri());
        intent.setAction(Intent.ACTION_DELETE).putExtra(WidgetAdapter.KEY_NAME, "John");
        setUpActivityWithIntent(intent);
        Xlog.v(TAG, mSolo.getString(com.mediatek.videofavorites.R.string.cancel));
        mSolo.clickOnButton(mSolo.getString(com.mediatek.videofavorites.R.string.cancel));
    }

    // test pick video activity result (transcode!)
    public void test030PickVideoResult() {
        // prepare activity result
        Intent resultIntent = new Intent();
        resultIntent.setData(Uri.parse("android.resource://com.mediatek.videofavorites.tests/"
                                       + R.raw.video));

        Instrumentation.ActivityResult activityResult =
            new Instrumentation.ActivityResult(Activity.RESULT_OK, resultIntent);
        IntentFilter filter = new IntentFilter(Intent.ACTION_PICK);
        try {
            filter.addDataType("video/*");
        } catch (IntentFilter.MalformedMimeTypeException mmte) {
            Xlog.e(TAG, "mime type failed");
        }
        Instrumentation.ActivityMonitor am =
            mInstrumentation.addMonitor(filter, activityResult, true);

        // trigger activity
        Intent intent = new Intent(Intent.ACTION_PICK, getDummyUri());
        intent.setAction(Intent.ACTION_DELETE).putExtra(WidgetActionActivity.KEY_ACTION_PICK_TYPE,
                WidgetActionActivity.CODE_PICK_VIDEO);
        setUpActivityWithIntent(intent);

        mInstrumentation.waitForMonitorWithTimeout(am, 3);

        mSolo.sleep(15000);
    }

    // test add contact activity result
}
