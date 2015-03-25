/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.test.fragments;

import android.app.Activity;
import android.app.Fragment;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.test.ActivityInstrumentationTestCase2;

import com.orangelabs.rcs.R;

import com.mediatek.rcse.activities.ChatWallPaperSettingsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.WallpaperChooserFragment;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

/**
 * Test case class for ChatWallPaperSettingsActivity
 */
public class WallPaperChooserFragmentTest extends
        ActivityInstrumentationTestCase2<ChatWallPaperSettingsActivity> {
    private static final String TAG = "WallPaperChooserFragmentTest";
    private WallpaperChooserFragment mWallpaperChooserFragment = null;
    private Field mBitmapField = null;
    private Field mLoaderField = null;
    private Field mWallpaperDrawableField = null;
    private final int FIRST_PICTURE_SELECTED = 2;
    private final int SECOND_PICTURE_SELECTED = 4;
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;

    Activity mActivity = null;

    public WallPaperChooserFragmentTest() {
        super(ChatWallPaperSettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() entry");
        super.setUp();
        // launch ChatWallPaperSettingsActivity
        mActivity = getActivity();
        assertNotNull(mActivity);
        mWallpaperChooserFragment = (WallpaperChooserFragment) getWallpaperChooserFragment(R.id.wallpaper_chooser_fragment);
        mBitmapField = Utils.getPrivateField(WallpaperChooserFragment.class, "mBitmap");
        mLoaderField = Utils.getPrivateField(WallpaperChooserFragment.class, "mLoader");
        mWallpaperDrawableField = Utils.getPrivateField(WallpaperChooserFragment.class,
                "mWallpaperDrawable");
        Logger.v(TAG, "setUp() exit");
    }

    // test when select two pictures at different position ,the WallPaper should
    // not be the same
    public void testCase1_selectGallery() throws Throwable {
        Object mLoader;
        // select one pictrue
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mWallpaperChooserFragment.onItemSelected(null, null, FIRST_PICTURE_SELECTED, 0);
            }
        });
        getInstrumentation().waitForIdleSync();
        mLoader = mLoaderField.get(mWallpaperChooserFragment);
        long startTimeA = System.currentTimeMillis();
        while (mLoader != null) {
            if (System.currentTimeMillis() - startTimeA > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            mLoader = mLoaderField.get(mWallpaperChooserFragment);
        }

        Object firstWallpaperDrawable = mWallpaperDrawableField.get(mWallpaperChooserFragment);
        Field firstBitmapField = firstWallpaperDrawable.getClass().getDeclaredField("mBitmap");
        firstBitmapField.setAccessible(true);
        Bitmap firstBitmap = (Bitmap) firstBitmapField.get(firstWallpaperDrawable);
        assertNotNull(firstBitmap);

        // select another pictrue
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mWallpaperChooserFragment.onItemSelected(null, null, SECOND_PICTURE_SELECTED, 0);
            }
        });
        getInstrumentation().waitForIdleSync();
        mLoader = mLoaderField.get(mWallpaperChooserFragment);
        long startTimeB = System.currentTimeMillis();
        while (mLoader != null) {
            if (System.currentTimeMillis() - startTimeB > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            mLoader = mLoaderField.get(mWallpaperChooserFragment);
        }
        Object secondWallpaperDrawable = mWallpaperDrawableField.get(mWallpaperChooserFragment);
        Field secondBitmapField = secondWallpaperDrawable.getClass().getDeclaredField("mBitmap");
        secondBitmapField.setAccessible(true);
        Bitmap secondBitmap = (Bitmap) secondBitmapField.get(secondWallpaperDrawable);
        assertNotNull(secondBitmap);

        // these two pictrue should not be same
        assertNotSame(firstBitmap, secondBitmap);
    }

    public void testCase02_selectWallpaper() throws Throwable {
        Method method = Utils.getPrivateMethod(mWallpaperChooserFragment.getClass(),
                "selectWallpaper", int.class);
        method.invoke(mWallpaperChooserFragment, 0);
        getInstrumentation().waitForIdleSync();
        assertFalse(getActivity().isResumed());
    }

    /**
     * Get the fragment in the activity.
     * 
     * @return The fragment in the activity.
     */
    private Fragment getWallpaperChooserFragment(int fragmentId) {
        Fragment fragment = mActivity.getFragmentManager().findFragmentById(fragmentId);
        assertTrue(fragment instanceof WallpaperChooserFragment);
        return fragment;
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }
}
