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

package com.mediatek.rcse.test.activity;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;

import com.mediatek.rcse.activities.ChatGalleryWallPaperSettingsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

/**
 * Test case class for SettingsActivity
 */
public class ChatGalleryWallPaperSettingsActivityTest extends
        ActivityInstrumentationTestCase2<ChatGalleryWallPaperSettingsActivity> {
    private static final String TAG = "ChatGalleryWallPaperSettingsActivityTest";
    private RcsSettings rcsSettings = null;
    private Activity mActivity = null;
    private static final int TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;
    private static final String WALLPAPER_DEFAULT_VALUE = "0";

    public ChatGalleryWallPaperSettingsActivityTest() {
        super(ChatGalleryWallPaperSettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "ChatGalleryWallPaperSettingsActivityTest setUp() entry");
        super.setUp();
        // set wallpaper to default value
        RcsSettings.createInstance(getInstrumentation().getTargetContext());
        long startTime = System.currentTimeMillis();
        while (RcsSettings.getInstance() == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }

        rcsSettings = RcsSettings.getInstance();
        String chatWallPaper = rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER);
        if (!chatWallPaper.equals(WALLPAPER_DEFAULT_VALUE)) {
            rcsSettings
                    .writeParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER, WALLPAPER_DEFAULT_VALUE);
        }

        assertEquals(WALLPAPER_DEFAULT_VALUE,
                rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        Logger.d(TAG, "ChatGalleryWallPaperSettingsActivityTest setUp() exit");
    }

    public void testCase1_setChatGalleryWallPaper() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        Logger.d(TAG, "testCase1_setChatGalleryWallPaper() entry");
        // mock new wallpaper
        mActivity = getActivity();
        assertNotNull(mActivity);
        Intent mockIntent = new Intent();
        String imgPath = getFilePath();
        Logger.d(TAG, "imgPath:" + imgPath);
        assertNotNull(imgPath);
        Method methodGetWallPaperFromGallery = Utils.getPrivateMethod(
                ChatGalleryWallPaperSettingsActivity.class, "getWallPaperFromGallery");
        assertNotNull(methodGetWallPaperFromGallery);
        methodGetWallPaperFromGallery.invoke(mActivity);
        mockIntent.setData(MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        Method methodOnActivityResult = Utils.getPrivateMethod(
                ChatGalleryWallPaperSettingsActivity.class, "onActivityResult", int.class,
                int.class, Intent.class);
        assertNotNull(methodOnActivityResult);
        methodOnActivityResult.invoke(mActivity, 0, -1, mockIntent);
        getInstrumentation().waitForIdleSync();
        // finish the Gallery started by ChatGalleryWallPaperSettingsActivity
        mActivity.finishActivity(0);
        getInstrumentation().waitForIdleSync();
        long startTime = System.currentTimeMillis();
        Logger.d(
                TAG,
                "Before change for RCSE_CHAT_WALLPAPER:"
                        + rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        // wait for blacklist contact loaded
        while (rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER).equals(
                WALLPAPER_DEFAULT_VALUE)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        // new paper already be inserted into database
        Logger.d(
                TAG,
                "After change for RCSE_CHAT_WALLPAPER:"
                        + rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        assertEquals(imgPath, rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        // new paper must be changed
        assertFalse(WALLPAPER_DEFAULT_VALUE.equals(imgPath));
        Logger.d(TAG, "testCase1_setChatGalleryWallPaper() exit");
    }

    /**
     * Get a image file path from database
     */
    private String getFilePath() {
        Logger.v(TAG, "getFilePath()");
        Context context = getInstrumentation().getTargetContext();
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            } else {
                fail("testCase1_setChatGalleryWallPaper() Cannot find image in sdcard");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.v(TAG, "getFilePath() out, filePath is " + filePath);
        return filePath;
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown() entry");
        rcsSettings.writeParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER, WALLPAPER_DEFAULT_VALUE);
        assertEquals(WALLPAPER_DEFAULT_VALUE,
                rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.d(TAG, "tearDown() exit");
    }

}
