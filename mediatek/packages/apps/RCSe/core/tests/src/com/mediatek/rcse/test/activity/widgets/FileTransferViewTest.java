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

package com.mediatek.rcse.test.activity.widgets;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.rcse.activities.SettingsActivity;
import com.mediatek.rcse.activities.widgets.FileTransferView;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

/**
 * This class is used to test FileTransferView
 */
public class FileTransferViewTest extends ActivityInstrumentationTestCase2<SettingsActivity> {
    private static final String TAG = "FileTransferViewTest";
    private static final String MOCK_FILE_PATH_NOT_PICTURE = "/mockpath/mockfilename.mp3";
    private static final String MOCK_FILE_PATH_NO_MIMETYPE = "/mockpath/mockfilename.";
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 5000;
    private SettingsActivity mActivity = null;
    FileTransferView fileTransferView = null;

    public FileTransferViewTest() {
        super(SettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() entry");
        super.setUp();
        getInstrumentation().waitForIdleSync();
        fileTransferView = new FileTransferView(getInstrumentation().getTargetContext());
        assertNotNull(fileTransferView);
        fileTransferView = null;
        Intent intent = new Intent(getInstrumentation().getTargetContext(), SettingsActivity.class);
        setActivityIntent(intent);
        mActivity = getActivity();
        assertNotNull(mActivity);
        Logger.v(TAG, "setUp() exit");
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Test the function setFilePath()
     */
    public void testCase1_SetFilePath() throws Throwable {
        Logger.v(TAG, "testCase1_SetFilePath() enter");
        // filePath is a picture
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mActivity.setContentView(R.layout.chat_item_received_file_transfer_finished);
                fileTransferView = (FileTransferView) mActivity
                        .findViewById(R.id.file_transfer_view);
                assertNotNull(fileTransferView);
                String filePath = getFilePath();
                fileTransferView.setFile(filePath);
            }
        });
        getInstrumentation().waitForIdleSync();
        Method getFileNameUri = Utils.getPrivateMethod(FileTransferView.class,"getFileNameUri",String.class);
        Uri fileNameUri = (Uri)getFileNameUri.invoke(fileTransferView, "");
        assertNull(fileNameUri);
        Field fieldFileIcon = Utils.getPrivateField(FileTransferView.class, "mFileIcon");
        ImageView fileIcon = (ImageView) fieldFileIcon.get(fileTransferView);
        assertEquals(View.GONE, fileIcon.getVisibility());
        Field fieldFileName = Utils.getPrivateField(FileTransferView.class, "mFileName");
        TextView fileName = (TextView) fieldFileName.get(fileTransferView);
        assertEquals(View.GONE, fileName.getVisibility());

        // filePath is no a picture
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                fileTransferView.setFile(MOCK_FILE_PATH_NOT_PICTURE);
            }
        });
        getInstrumentation().waitForIdleSync();
        fileIcon = (ImageView) fieldFileIcon.get(fileTransferView);
        assertEquals(View.VISIBLE, fileIcon.getVisibility());
        fileName = (TextView) fieldFileName.get(fileTransferView);
        assertEquals(View.VISIBLE, fileName.getVisibility());

        // filePath is with no mimetype, should return a default bitmap
        fileIcon = (ImageView) fieldFileIcon.get(fileTransferView);
        Field fieldDrawableField = Utils.getPrivateField(ImageView.class, "mDrawable");
        fieldDrawableField.set(fileIcon, new BitmapDrawable()); // Set bitmap to null
        assertNull(drawableToBitmap(fileIcon.getDrawable()));
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                fileTransferView.setFile(MOCK_FILE_PATH_NO_MIMETYPE);
            }
        });
        getInstrumentation().waitForIdleSync();
        long startTime = System.currentTimeMillis();
        while (null == drawableToBitmap(fileIcon.getDrawable())) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase1_SetFilePath() Load image timeout, cost time:"
                        + (System.currentTimeMillis() - startTime));
            }
        }
        fileIcon = (ImageView) fieldFileIcon.get(fileTransferView);
        Bitmap expected = BitmapFactory.decodeResource(mActivity.getResources(),
                R.drawable.rcs_ic_ft_default_preview);
        Bitmap result = drawableToBitmap(fileIcon.getDrawable());
        assertTrue(expected.sameAs(result));
        Logger.v(TAG, "testCase1_SetFilePath() exit");
    }

    /**
     * Transform a Drawable object to a Bitmap object
     */
    private Bitmap drawableToBitmap(Drawable drawable) {
        Logger.v(TAG, "drawableToBitmap()");
        return ((BitmapDrawable) drawable).getBitmap();
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
                fail("getFilePath() Cannot find image in sdcard");
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
}
