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

import com.mediatek.videofavorites.AbsVideoFavoritesWidget;
import com.mediatek.videofavorites.VFWidgetTestActivity;
import com.mediatek.videofavorites.VideoFavoritesProviderValues;
import com.mediatek.videofavorites.VideoFavoritesRootView;
import com.mediatek.videofavorites.WidgetActionActivity;
import com.mediatek.videofavorites.WidgetAdapter;
import com.mediatek.xlog.Xlog;

import com.jayway.android.robotium.solo.Solo;

import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.IntentFilter;
import android.content.Intent;
import android.net.Uri;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.View;

import java.io.File;
import java.util.ArrayList;


/**
 *
 *
 */
public class VideoFavoritesWidgetTest extends ActivityInstrumentationTestCase2<VFWidgetTestActivity> {

    private static final String TAG = "VFWidgetTest";

    private Instrumentation mInstrumentation;
    private VFWidgetTestActivity mActivity;

    private View mWidgetRoot;
    private VideoFavoritesRootView mVFRoot;

    private Solo mSolo;

    public VideoFavoritesWidgetTest() {
        super(VFWidgetTestActivity.class);
    }


    private View findViewById(int id) {
        return mActivity.findWidgetViewById(mWidgetRoot, id);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mInstrumentation = getInstrumentation();
        mActivity = getActivity();
        mWidgetRoot = mActivity.findViewById(com.mediatek.videofavorites.R.id.root_view);
        mVFRoot = (VideoFavoritesRootView) findViewById(com.mediatek.videofavorites.R.id.widget_root);
        mSolo = new Solo(getInstrumentation(), getActivity());
        mInstrumentation.waitForIdleSync();
    }

    @Override
    protected void tearDown() throws Exception {
        mSolo.finishOpenedActivities();
        super.tearDown();
    }

    public void test010LaunchActivity() {
        mSolo.sleep(3000); // let video starts to play. (if any)
    }


    public void test020RecordIcon() {
        View root = mActivity.findViewById(com.mediatek.videofavorites.R.id.root_view);

        final View recordIcon = mActivity.findWidgetViewById(root,
                                com.mediatek.videofavorites.R.id.btn_record);
        Xlog.v(TAG, "finding recordIcon: " + recordIcon);

        assertTrue(recordIcon != null);
    }

    public void test030DeleteIcon() {
        mInstrumentation.waitForIdleSync();
        View root = mActivity.findViewById(com.mediatek.videofavorites.R.id.root_view);
        final View v = mActivity.findWidgetViewById(root, com.mediatek.videofavorites.R.id.btn_edit);

        ContentResolver resolver = mActivity.getContentResolver();
        clearDataBase(resolver);

        insertVideo(resolver, "android.resource://com.mediatek.videofavorites.tests/" + R.raw.video);

        sendRefreshBroadcast();

        mSolo.sleep(2000);

        assertTrue(v != null);
        mSolo.clickOnView(v);
        mInstrumentation.waitForIdleSync();
        if (mVFRoot.getVideoCount() != 0) {
            assertTrue(mVFRoot.isDeleteMode());
        }

        mSolo.sleep(500);

        mSolo.clickOnView(v);
        mInstrumentation.waitForIdleSync();
        assertFalse(mVFRoot.isDeleteMode());

        clearDataBase(resolver);
    }

    public void test040AddIcon() {

        View root = mActivity.findViewById(com.mediatek.videofavorites.R.id.root_view);

        if (mVFRoot.getVideoCount() == 4) {
            Xlog.v(TAG, "no more room, test040AddIcon skipped");
            return;
        }
        View addIcon = null;
        for (int i = 0; i < 10 && addIcon == null; i++) {
            addIcon = mActivity.findWidgetViewById(
                          root, com.mediatek.videofavorites.R.id.favorite_new);
            if (addIcon == null) {
                Xlog.v(TAG, "addIcon is null, try again in 1 sec");
                mSolo.sleep(1000);
            } else {
                Xlog.v(TAG, "found, " + addIcon);
            }
        }
        assertTrue(addIcon != null);
    }

    private void sendRefreshBroadcast() {
        Intent i = new Intent(AbsVideoFavoritesWidget.ACTION_REFRESH);
        mActivity.sendBroadcast(i);
    }

    public void test050RefreshBroadcast() {
        sendRefreshBroadcast();
    }

    private static final String KEY_VIDEO_URI = VideoFavoritesProviderValues.Columns.VIDEO_URI;
    private static final String KEY_NAME = VideoFavoritesProviderValues.Columns.NAME;
    private void insertVideo(ContentResolver r, String uriString) {
        ContentValues v = new ContentValues();
        v.put(KEY_VIDEO_URI, uriString);
        v.put(KEY_NAME, "Bunny");
        r.insert(VideoFavoritesProviderValues.Columns.CONTENT_URI, v);
    }

    private void clearDataBase(ContentResolver r) {
        r.delete(VideoFavoritesProviderValues.Columns.CONTENT_URI, null, null);
    }


    public void test07somebasicVideo() {
        ContentResolver resolver = mActivity.getContentResolver();
        clearDataBase(resolver);

        insertVideo(resolver, "android.resource://com.mediatek.videofavorites.tests/" + R.raw.video);

        sendRefreshBroadcast();
        Xlog.v(TAG, "should start to play videos");
        mSolo.sleep(20000);
        // we've done, clear it.
        clearDataBase(resolver);
    }
}
