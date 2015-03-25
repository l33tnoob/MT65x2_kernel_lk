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

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;

import com.mediatek.rcse.activities.ChatCameraWallPaperSettingsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

/**
 * Test case class for SettingsActivity
 */
public class ChatCameraWallPaperSettingsActivityTest extends
        ActivityInstrumentationTestCase2<ChatCameraWallPaperSettingsActivity> {
    private static final String TAG = "ChatCameraWallPaperSettingsActivityTest";
    private RcsSettings rcsSettings = null;
    private Activity mActivity = null;
    private static final int TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;
    private static final String WALLPAPER_DEFAULT_VALUE = "0";
    public ChatCameraWallPaperSettingsActivityTest() {
        super(ChatCameraWallPaperSettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "ChatCameraWallPaperSettingsActivityTest setUp() entry");
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
        Logger.d(TAG, "ChatCameraWallPaperSettingsActivityTest setUp() exit");
    }

    public void testCase1_setChatCameraWallPaper() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        Logger.d(TAG, "testCase1_setChatCameraWallPaper() entry");
        // mock new wallpaper
        mActivity = getActivity();
        assertNotNull(mActivity);
        String imgPath = "file:///storage/sdcard0/Joyn/temp/tmp_joyn_1326400736934.jpg";
        Uri mockUri=Uri.parse(imgPath);

        Method methodGetWallPaperFromCamera = Utils.getPrivateMethod(
                ChatCameraWallPaperSettingsActivity.class, "getWallPaperFromCamera");
        assertNotNull(methodGetWallPaperFromCamera);
        methodGetWallPaperFromCamera.invoke(mActivity);

        // set CameraWallPaperUri
        Field field_mCameraWallPaperUri = Utils.getPrivateField(
                ChatCameraWallPaperSettingsActivity.class, "mCameraWallPaperUri");
        field_mCameraWallPaperUri.set(mActivity, mockUri);
        assertNotNull(field_mCameraWallPaperUri.get(mActivity));

        Method methodOnActivityResult = Utils.getPrivateMethod(
                ChatCameraWallPaperSettingsActivity.class, "onActivityResult", int.class,
                int.class, Intent.class);
        assertNotNull(methodOnActivityResult);
        methodOnActivityResult.invoke(mActivity, 0, -1, null);
        getInstrumentation().waitForIdleSync();
        // finish the camera acitiviy stared by
        // ChatCameraWallPaperSettingsActivity
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

        imgPath = imgPath.substring(One2OneChatFragment.FILE_SCHEMA.length(), imgPath.length());
        assertEquals(imgPath, rcsSettings.readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        // new paper must be changed
        assertFalse(WALLPAPER_DEFAULT_VALUE.equals(imgPath));
        Logger.d(TAG, "testCase1_setChatGalleryWallPaper() exit");
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
