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

package com.mediatek.rcse.test.service;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.ImageLoader.OnLoadImageFinishListener;
import com.mediatek.rcse.test.Utils;

import java.io.FileNotFoundException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

/**
 * This class is used to test ImageLoader.java
 */
public class ImageLoaderTest extends InstrumentationTestCase {
    private static final String TAG = "ImageLoaderTest";
    private static final String PREFER_PHOTO_URI = "content://media/external/images/media/";
    private static final String IMAGE_MAP = "mImageMap";
    private static Context mContext = null;
    private static final int IMAGE_SIZE = 1;
    private static final int TIME_OUT = 3000;
    private static final int SLEEP_TIME = 200;
    private static final String MOCK_NUMBER_ONE = "+3455555118";
    private static final String MOCK_NUMBER_TWO = "+3455555115";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        ImageLoader.getInstance().clearImageMap();
        Method methodInitialize =
                Utils.getPrivateMethod(ApiManager.class, "initialize", Context.class);
        methodInitialize.invoke(new Boolean(true), mContext);
    }

    /**
     * Test to get a bitmap from a given file path.
     * 
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws NoSuchMethodException
     * @throws InterruptedException
     */
    public void testCase1_RequestImageFromFilePath() throws IllegalArgumentException,
            InstantiationException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, NoSuchMethodException, InterruptedException {
        Logger.d(TAG, "testCase1_RequestImageFromFilePath() entry");
        MockOnLoadImageFinishListener listener = new MockOnLoadImageFinishListener();
        MockOnLoadImageFinishListener listenerNext = new MockOnLoadImageFinishListener();
        String filePath = getFilePath(0);
        String filePathNext = getFilePath(1);
        assertNotSame(filePath, filePathNext);
        assertNotNull(filePath);
        Bitmap bitmap = ImageLoader.requestImage(filePath, listener);
        ImageLoader.requestImage(filePathNext, listenerNext);
        getInstrumentation().waitForIdleSync();
        ImageLoader.interrupt(filePathNext);
        assertNull(bitmap);
        listener.waitForImage();
        bitmap = ImageLoader.requestImage(filePath);
        assertNotNull(bitmap);
        Map imageMap = getImageMap(filePath, bitmap);
        assertEquals(IMAGE_SIZE, imageMap.size());
        bitmap = ImageLoader.requestImage(filePath);
        assertNotNull(bitmap);
        ImageLoader.getInstance().clearImageMap();
        ImageLoader.requestImage(filePath, listener);
        ImageLoader.requestImage(filePathNext, listenerNext);
        listener.waitForImage();
        ImageLoader.interrupt(filePath);
        ImageLoader.interrupt(filePathNext);
        ImageLoader.interrupt();
        getInstrumentation().waitForIdleSync();
        ImageLoader.getInstance().clearImageMap();
        ImageLoader.requestImage(filePath, listener);
        ImageLoader.requestImage(filePathNext, listenerNext);
        ImageLoader.interrupt();
        getInstrumentation().waitForIdleSync();
    }

    /**
     * Test to get a bitmap from a given photo uri.
     * 
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws NoSuchMethodException
     * @throws FileNotFoundException
     */
    public void testCase2_RequestImageFromPhotoUri() throws IllegalArgumentException,
            InstantiationException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, NoSuchMethodException, FileNotFoundException {
        Logger.d(TAG, "testCase2_RequestImageFromPhotoUri() entry");
        Uri photoUri = getPhotoUri();
        assertNotNull(photoUri);
        Bitmap bitmap = ImageLoader.requestImage(photoUri);
        assertNotNull(bitmap);
        Map imageMap = getImageMap(photoUri, bitmap);
        assertEquals(IMAGE_SIZE, imageMap.size());
        bitmap = ImageLoader.requestImage(photoUri);
        assertNotNull(bitmap);
    }

    /**
     * Test filleBitmapList in LoadParticipantImageTask
     * @throws Throwable
     */
    public void testCase3_testFillBitmapList() throws Throwable {
        Class taskClass = Utils.getPrivateClass(ImageLoader.class, "LoadParticipantsImageTask");
        assertNotNull(taskClass);
        Method fillBitmapListMethod = Utils.getPrivateMethod(taskClass, "fillBitmapList", int.class, List.class);
        String filePath = getFilePath(0);
        Bitmap bitmap = ImageLoader.requestImage(filePath);
        assertNotNull(bitmap);
        ArrayList<Bitmap> singleBitmapList = new ArrayList<Bitmap>();
        singleBitmapList.add(bitmap);

        assertNull(fillBitmapListMethod.invoke(null, 0, null));
        assertNull(fillBitmapListMethod.invoke(null, 0, singleBitmapList));
        assertNotNull(fillBitmapListMethod.invoke(null, 0, new ArrayList<Bitmap>()));

        singleBitmapList = new ArrayList<Bitmap>();
        singleBitmapList.add(bitmap);
        ArrayList<Bitmap> doubleBitmapList = new ArrayList<Bitmap>(singleBitmapList);
        doubleBitmapList.add(bitmap);
        assertNotNull(fillBitmapListMethod.invoke(null, 1, new ArrayList<Bitmap>()));
        assertNotNull(fillBitmapListMethod.invoke(null, 1, singleBitmapList));
        assertNull(fillBitmapListMethod.invoke(null, 1, doubleBitmapList));

        singleBitmapList = new ArrayList<Bitmap>();
        singleBitmapList.add(bitmap);
        doubleBitmapList = new ArrayList<Bitmap>(singleBitmapList);
        doubleBitmapList.add(bitmap);
        ArrayList<Bitmap> tripleBitmapList = new ArrayList<Bitmap>(doubleBitmapList);
        tripleBitmapList.add(bitmap);
        assertNotNull(fillBitmapListMethod.invoke(null, 2, new ArrayList<Bitmap>()));
        assertNotNull(fillBitmapListMethod.invoke(null, 2, singleBitmapList));
        assertNotNull(fillBitmapListMethod.invoke(null, 2, doubleBitmapList));
        assertNull(fillBitmapListMethod.invoke(null, 2, tripleBitmapList));

        singleBitmapList = new ArrayList<Bitmap>();
        singleBitmapList.add(bitmap);
        doubleBitmapList = new ArrayList<Bitmap>(singleBitmapList);
        doubleBitmapList.add(bitmap);
        tripleBitmapList = new ArrayList<Bitmap>(doubleBitmapList);
        tripleBitmapList.add(bitmap);
        ArrayList<Bitmap> fourBitmapList = new ArrayList<Bitmap>(tripleBitmapList);
        fourBitmapList.add(bitmap);
        assertNotNull(fillBitmapListMethod.invoke(null, 3, new ArrayList<Bitmap>()));
        assertNotNull(fillBitmapListMethod.invoke(null, 3, singleBitmapList));
        assertNotNull(fillBitmapListMethod.invoke(null, 3, doubleBitmapList));
        assertNotNull(fillBitmapListMethod.invoke(null, 3, tripleBitmapList));
        assertNull(fillBitmapListMethod.invoke(null, 3, fourBitmapList));
    }

    /**
     * Test to interrupt loading Participant Image
     * @throws Throwable
     */
    public void testCase4_testInterruptParticipantImage() throws Throwable {
        TreeSet<String> emptyParticipantList = new TreeSet<String>();
        TreeSet<String> participantListOne = new TreeSet<String>();
        participantListOne.add(MOCK_NUMBER_ONE);
        TreeSet<String> participantListTwo = new TreeSet<String>();
        participantListTwo.add(MOCK_NUMBER_TWO);
        MockOnLoadImageFinishListener listener = new MockOnLoadImageFinishListener();
        MockOnLoadImageFinishListener listenerNext = new MockOnLoadImageFinishListener();
        ImageLoader.requestImage(participantListOne, listener);
        ImageLoader.requestImage(participantListTwo, listenerNext);
        listener.waitForImage();
        listenerNext.waitForImage();
        ImageLoader.getInstance().clearImageMap();
        ImageLoader.requestImage(participantListOne, listener);
        ImageLoader.requestImage(participantListTwo, listenerNext);
        getInstrumentation().waitForIdleSync();
        ImageLoader.interrupt(participantListTwo);
        ImageLoader.interrupt(participantListOne);
        ImageLoader.interrupt(emptyParticipantList);
        getInstrumentation().waitForIdleSync();
    }

    /**
     * Get a photo uri.
     * 
     * @return A photo uri.
     */
    public static Uri getPhotoUri() {
        Logger.d(TAG, "getPhotoUri() entry");
        Cursor cursor = null;
        int imageId = 0;
        try {
            cursor =
                    mContext.getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            imageId = cursor.getInt(cursor.getColumnIndex(MediaStore.Images.ImageColumns._ID));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri photoUri = Uri.parse(PREFER_PHOTO_URI + imageId);
        Logger.d(TAG, "getPhotoUri() exit with the uri " + photoUri);
        return photoUri;
    }

    /**
     * Get a file path.
     * @param index The index of the file 
     * @return A file path.
     */
    public static String getFilePath(int index) {
        Logger.d(TAG, "getFilePath() entry");
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor =
                    mContext.getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            for (int i = 0 ; i < index ; i++) {
                cursor.moveToNext();
            }
            filePath = cursor.getString(cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.d(TAG, "getFilePath() exit with the file path " + filePath);
        return filePath;
    }

    private Map getImageMap(Object key, Bitmap bitmap) throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Logger.d(TAG, "getImageMap() entry, the key is " + key + " and the bitmap is " + bitmap);
        ImageLoader imageLoader = ImageLoader.getInstance();
        Method methodSetImage =
                Utils.getPrivateMethod(ImageLoader.class, "setImage", Object.class, Bitmap.class);
        methodSetImage.invoke(imageLoader, key, bitmap);
        Field imageMapField = Utils.getPrivateField(ImageLoader.class, IMAGE_MAP);
        Map imageMap = (Map) imageMapField.get(imageLoader);
        return imageMap;
    }

    private class MockOnLoadImageFinishListener implements OnLoadImageFinishListener {
        private boolean isLoaded = false;

        @Override
        public void onLoadImageFished(Bitmap image) {
            isLoaded = true;
        }

        public void waitForImage() throws InterruptedException {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (!isLoaded);
        }
    }
}
