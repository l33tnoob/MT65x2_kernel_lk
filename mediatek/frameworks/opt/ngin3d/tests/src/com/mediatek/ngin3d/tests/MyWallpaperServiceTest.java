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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ngin3d.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.app.WallpaperInfo;
import android.app.WallpaperManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import java.io.IOException;

public class MyWallpaperServiceTest extends ActivityInstrumentationTestCase2<MyWallpaperTester> {
    private static final String TAG = "MyWallpaperServiceTest";
    public static final String WALLPAPER_SERVICE_NAME
        = "com.mediatek.ngin3d.tests.MyWallpaperService";
    public static final String ENGINE_INITIALIZE
        = "com.mediatek.ngin3d.tests.ENGINE_INITIALIZE";
    public static final String IMAGE_LOADING_FROM_RESOURCE
        = "com.mediatek.ngin3d.tests.IMAGE_LOADING_FROM_RESOURCE";
    public static final String IMAGE_LOADING_FROM_BITMAP
        = "com.mediatek.ngin3d.tests.IMAGE_LOADING_FROM_BITMAP";
    public static final String IMAGE_LOADING_FROM_FILE
        = "com.mediatek.ngin3d.tests.IMAGE_LOADING_FROM_FILE";
    public static final String UPDATE_SYSTEM_TEXT
        = "com.mediatek.ngin3d.tests.UPDATE_SYSTEM_TEXT";
    public static final String UPDATE_BITMAP_TEXT
        = "com.mediatek.ngin3d.tests.UPDATE_BITMAP_TEXT";
    public static final String RENDER_50_ACTOR
        = "com.mediatek.ngin3d.tests.RENDER_50_ACTOR";
    public static final String RENDER_100_ACTOR
        = "com.mediatek.ngin3d.tests.RENDER_100_ACTOR";
    public static final String START_50_ANIMATION
        = "com.mediatek.ngin3d.tests.START_50_ANIMATION";
    public static final String START_100_ANIMATION
        = "com.mediatek.ngin3d.tests.START_100_ANIMATION";
    public static final String TEST_SCREENSHOT
        = "com.mediatek.ngin3d.tests.TEST_SCREENSHOT";
    public static final String RENDER_25_LANDSCAPE
        = "com.mediatek.ngin3d.tests.RENDER_25_LANDSCAPE";

    private ComponentName mWallpaperComponent;
    private WallpaperManager mWallpaperManager;

    public MyWallpaperServiceTest() {
        super(MyWallpaperTester.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getInstrumentation().waitForIdleSync();
    }

    // This test case is used to start MyWallpaperService
    public void test00_startMyWallpaperService() throws InterruptedException {
        Log.d(TAG, "testSetWallpaper");
        // initialize MyWallpaperService components
        mWallpaperManager = (WallpaperManager)getActivity().getSystemService(
            Context.WALLPAPER_SERVICE);
        mWallpaperComponent = new ComponentName("com.mediatek.ngin3d.tests",
            MyWallpaperService.class.getName());

        setWallpaper();
    }

    public void test01_EngineInitialTime() throws InterruptedException {
        Log.d(TAG, "test01_ImageLoadingFromResourceTime");
        final Intent intent = new Intent(ENGINE_INITIALIZE);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test02_ImageLoadingFromResourceTime() throws InterruptedException {
        Log.d(TAG, "test02_ImageLoadingFromResourceTime");
        final Intent intent = new Intent(IMAGE_LOADING_FROM_RESOURCE);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test03_ImageLoadingFromBitmapTime() throws InterruptedException {
        Log.d(TAG, "test03_ImageLoadingFromBitmapTime");
        final Intent intent = new Intent(IMAGE_LOADING_FROM_BITMAP);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test04_ImageLoadingFromFile() throws InterruptedException {
        Log.d(TAG, "test04_ImageLoadingFromFile");
        final Intent intent = new Intent(IMAGE_LOADING_FROM_FILE);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test05_UpdateSystemTextContent() throws InterruptedException {
        Log.d(TAG, "test05_UpdateSystemTextContent");
        final Intent intent = new Intent(UPDATE_SYSTEM_TEXT);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test06_UpdateBitmapTextContent() throws InterruptedException {
        Log.d(TAG, "test06_UpdateBitmapTextContent");
        final Intent intent = new Intent(UPDATE_BITMAP_TEXT);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test07_Render50Actor() throws InterruptedException {
        Log.d(TAG, "test07_Render50Actor");
        final Intent intent = new Intent(RENDER_50_ACTOR);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test08_Render100Actor() throws InterruptedException {
        Log.d(TAG, "test08_Render100Actor");
        final Intent intent = new Intent(RENDER_100_ACTOR);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test09_Start50Animation() throws InterruptedException {
        Log.d(TAG, "test09_Start50Animation");
        final Intent intent = new Intent(START_50_ANIMATION);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test10_Start100Animation() throws InterruptedException {
        Log.d(TAG, "test10_Start100Animation");
        final Intent intent = new Intent(START_100_ANIMATION);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test11_ScreenShot() throws InterruptedException {
        Log.d(TAG, "test11_ScreenShot");
        final Intent intent = new Intent(TEST_SCREENSHOT);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    public void test12_Render25Landscapes() throws InterruptedException {
        Log.d(TAG, "test12_Render25Landscapes");
        final Intent intent = new Intent(RENDER_25_LANDSCAPE);
        getActivity().sendBroadcast(intent);
        Thread.sleep(3000);
    }

    // This test case is used to destroy MyWallpaperService
    public void test13_destroyMyWallpaperService() throws InterruptedException {
        Activity activity = getActivity();
        Log.d(TAG, "test13_destroyMyWallpaperService");
        final WallpaperManager wallpaperManager = WallpaperManager.getInstance(activity);
        try {
            wallpaperManager.setBitmap(Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888));
        } catch (IOException e) {
            e.printStackTrace();
        }

        Thread.sleep(3000);
    }

    private void setWallpaper() throws InterruptedException {
        try {
            WallpaperInfo info = mWallpaperManager.getIWallpaperManager().getWallpaperInfo();
            if (info == null || (WALLPAPER_SERVICE_NAME).compareTo(info.getServiceName()) != 0) {
                mWallpaperManager.getIWallpaperManager().setWallpaperComponent(mWallpaperComponent);
                Log.d(TAG, "setWallpaperComponent");
            }

        } catch (RemoteException e) {
            Log.d(TAG, "Failed to set wallpaper: " + e);
        }
    }
}
