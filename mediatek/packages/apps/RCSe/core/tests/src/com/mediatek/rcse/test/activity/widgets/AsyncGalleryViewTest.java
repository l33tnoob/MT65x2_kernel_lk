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

import android.app.Activity;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;
import android.widget.ImageView;

import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
/**
 * This class is used to test AsyncGalleryView
 */
public class AsyncGalleryViewTest extends InstrumentationTestCase {
    private static final String TAG = "AsyncGalleryViewTest";
    private static final String NULL_FILE_PATH = null;
    private static final String EMPTY_FILE_PATH = "";
    private static final String TRUE_FILE_PATH = "/mockpath/mockfilename.jpg";
    private static final String FILE_PATH_WITHOUT_DOT = "/mockpath/mockfilename";
    private static final String FILE_PATH_WITHOUT_FORMAT = "/mockpath/mockfilename.";
    private static final String FILE_PATH_WITH_WRONG_FORMAT = "/mockpath/mockfilename.mp3";
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 8000;
    private AsyncGalleryView mAsyncGalleryView = null;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        // To initialize PICTURE_FORMAT which is used in isPictureFile()
        mAsyncGalleryView = new AsyncGalleryView(getInstrumentation().getTargetContext());
        assertNotNull(mAsyncGalleryView);
        mAsyncGalleryView = null;
        mAsyncGalleryView = new AsyncGalleryView(getInstrumentation().getTargetContext(), null);
        assertNotNull(mAsyncGalleryView);
    }

    @Override
    public void tearDown() throws Exception {
        mAsyncGalleryView = null;
        super.tearDown();
    }

    /**
     * Test the function AsyncGalleryView.isPictureFile()
     */
    public void testCase1_IsPictureFile() throws Throwable {
        Logger.v(TAG, "testCase1_IsPictureFile()");
        assertFalse(AsyncGalleryView.isPictureFile(NULL_FILE_PATH));
        assertFalse(AsyncGalleryView.isPictureFile(EMPTY_FILE_PATH));
        assertFalse(AsyncGalleryView.isPictureFile(FILE_PATH_WITHOUT_DOT));
        assertFalse(AsyncGalleryView.isPictureFile(FILE_PATH_WITHOUT_FORMAT));
        assertFalse(AsyncGalleryView.isPictureFile(FILE_PATH_WITH_WRONG_FORMAT));
        assertTrue(AsyncGalleryView.isPictureFile(TRUE_FILE_PATH));
    }

    /**
     * Test the function AsyncGalleryView.setAsyncImage()
     */
    public void testCase2_SetAsyncImage() throws Throwable {
        Logger.v(TAG, "testCase2_SetAsyncImage()");
        // Clear mImageMap in ImageLoader
        ImageLoader imageLoader = ImageLoader.getInstance();
        if (null != imageLoader) {
        	Method clearImageMap = Utils.getPrivateMethod(ImageLoader.class, "clearImageMap");
        	assertNotNull(clearImageMap);
        	clearImageMap.invoke(imageLoader);
        } else {
        	Logger.d(TAG, "testCase2_SetAsyncImage() imageLoader is null");
        }

        final String FILE_PATH = getFilePath();
        final AsyncGalleryView VIEW = mAsyncGalleryView;
        assertNotNull(FILE_PATH);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                VIEW.setAsyncImage(FILE_PATH);
                try {
                    Thread.sleep(SLEEP_TIME);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        Field fieldDrawableField = Utils.getPrivateField(ImageView.class, "mDrawable");
        fieldDrawableField.set(VIEW, new BitmapDrawable()); // Set bitmap to null
        assertNull(drawableToBitmap(VIEW.getDrawable()));
        long startTime = System.currentTimeMillis();
        while (imageIsNotLoaded()) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase2_SetAsyncImage() Load image timeout, cost time:"
                        + (System.currentTimeMillis() - startTime));
            }
        }
        assertNotNull(drawableToBitmap(VIEW.getDrawable()));

        // Clear mImageMap in ImageLoader
        imageLoader = ImageLoader.getInstance();
        if (null != imageLoader) {
            Method clearImageMap = Utils.getPrivateMethod(ImageLoader.class, "clearImageMap");
            assertNotNull(clearImageMap);
            clearImageMap.invoke(imageLoader);
        } else {
            Logger.d(TAG, "testCase2_SetAsyncImage() imageLoader is null");
        }
        final String EMPTY_FILE_PATH = "";
        final AsyncGalleryView EMPTY_VIEW = mAsyncGalleryView;
        assertNotNull(FILE_PATH);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                EMPTY_VIEW.setAsyncImage(EMPTY_FILE_PATH);
                try {
                    Thread.sleep(SLEEP_TIME);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        Field mCurrentListenerField = Utils.getPrivateField(AsyncGalleryView.class,
                "mCurrentListener");
        Object mCurrentListener = mCurrentListenerField.get(EMPTY_VIEW);
        long start = System.currentTimeMillis();
        while (mCurrentListener != null) {
            if (System.currentTimeMillis() - start > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
            mCurrentListener = mCurrentListenerField.get(EMPTY_VIEW);
        }
        assertNull(mCurrentListener);

        Logger.v(TAG, "testCase2_SetAsyncImage() out. Load image filePath:" + FILE_PATH + ", cost time:"
                + (System.currentTimeMillis() - startTime));
    }

    /**
     * Test the function AsyncGalleryView.isFileExist() and
     * setImageURI/setImagePath function.
     */
    public void testCase3_isFileExist() throws Throwable {
        Field pictureNameStringField = Utils.getPrivateField(AsyncGalleryView.class,
                "mPictureNameString");
        String oldPictureNameString = (String) pictureNameStringField.get(mAsyncGalleryView);
        Field mimeTypeField = Utils.getPrivateField(AsyncGalleryView.class, "mMimeType");
        String oldMimeType = (String) mimeTypeField.get(mAsyncGalleryView);
        Field pictureNameUriField = Utils
                .getPrivateField(AsyncGalleryView.class, "mPictureNameUri");
        Uri oldPictureNameUri = (Uri) pictureNameUriField.get(mAsyncGalleryView);

        String newPictureName = getFilePath() + "AsyncGalleryViewTest.img";
        pictureNameStringField.set(mAsyncGalleryView, newPictureName);
        mimeTypeField.set(mAsyncGalleryView, mAsyncGalleryView.getMimeType(newPictureName));
        pictureNameUriField.set(mAsyncGalleryView, Uri.parse(newPictureName));

        Method isFileExist = Utils.getPrivateMethod(AsyncGalleryView.class, "isFileExist");
        Boolean result = (Boolean) isFileExist.invoke(mAsyncGalleryView);
        assertEquals(false, result.booleanValue());

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_GET_CONTENT);
        filter.addDataType("image/*");
        ActivityMonitor am = getInstrumentation().addMonitor(filter, null, false);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                mAsyncGalleryView.onClick(mAsyncGalleryView);
            }
        });
        getInstrumentation().waitForIdleSync();
        Activity activity = am.waitForActivityWithTimeout(2000l);
        try {
            assertNull(activity);
        } finally {
            getInstrumentation().removeMonitor(am);
            if (activity != null) {
                activity.finish();
            }
        }

        try {
            mAsyncGalleryView.setImageURI(oldPictureNameUri);
        } catch (RuntimeException r) {
            r.printStackTrace();
        }

        Method setImagePath = Utils.getPrivateMethod(AsyncGalleryView.class, "setImagePath",
                String.class);
        String path = null;
        setImagePath.invoke(mAsyncGalleryView, path);
        assertNull(pictureNameStringField.get(mAsyncGalleryView));

        AsyncImageView testImageView = new AsyncImageView(getInstrumentation().getTargetContext(),
                null, 0);
        assertNotNull(testImageView);
        testImageView = null;

        // restore
        pictureNameStringField.set(mAsyncGalleryView, oldPictureNameString);
        mimeTypeField.set(mAsyncGalleryView, oldMimeType);
        pictureNameUriField.set(mAsyncGalleryView, oldPictureNameUri);
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
                fail("testCase2_SetAsyncImage() Cannot find image in sdcard");
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

    /**
     * Check whether the image is loaded, return true if loaded
     */
    private Boolean imageIsNotLoaded() {
        Logger.v(TAG, "isImageLoaded()");
        Bitmap bitmap = drawableToBitmap(mAsyncGalleryView.getDrawable());
        return (null == bitmap);
    }

    /**
     * Transform a Drawable object to a Bitmap object
     */
    private Bitmap drawableToBitmap(Drawable drawable) {
        Logger.v(TAG, "drawableToBitmap()");
        return ((BitmapDrawable) drawable).getBitmap();
    }
}
