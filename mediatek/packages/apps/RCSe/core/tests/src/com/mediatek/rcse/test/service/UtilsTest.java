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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ExifInterface;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.R;

import java.io.FileNotFoundException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This class is used to test Utils.java
 */
public class UtilsTest extends InstrumentationTestCase {
    private final static String TAG = "UtilsTest";
    private Context mContext = null;
    private final static long SIZE_BETES = 1000;
    private final static long SIZE_KILO_BETES = 1000000;
    private final static long SIZE_M = 2000000;
    private final static String EXPECTED_SIZE_BETES = "1000 bytes";
    private final static String EXPECTED_SIZE_KILO_BETES = "976.56 K";
    private final static String EXPECTED_SIZE_M = "1.91 M";
    private final static String INITIALIZE = "initialize";
    private final static int SIZE_TYPE_BYTE = 1;
    private final static int SIZE_TYPE_K_BYTE = 2;
    private final static int SIZE_TYPE_M = 3;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        Method methodInitialize = com.mediatek.rcse.test.Utils.getPrivateMethod(ApiManager.class,
                INITIALIZE, Context.class);
        methodInitialize.invoke(null, mContext);
    }

    /**
     * Test whether a filesize is correctly formatted.
     */
    public void testCase1_FormatFileSizeToString() {
        Logger.d(TAG, "testCase1_FormatFileSizeToString() entry");

        testFileSize(SIZE_BETES, EXPECTED_SIZE_BETES, Utils.SIZE_TYPE_TOTAL_SIZE,
                R.string.file_transfer_bytes);

        testFileSize(SIZE_KILO_BETES, EXPECTED_SIZE_KILO_BETES, Utils.SIZE_TYPE_TOTAL_SIZE,
                R.string.file_transfer_k_bytes);

        testFileSize(SIZE_M, EXPECTED_SIZE_M, Utils.SIZE_TYPE_TOTAL_SIZE, R.string.file_transfer_m);

        testFileSize(SIZE_BETES, EXPECTED_SIZE_BETES, Utils.SIZE_TYPE_BYTE,
                R.string.file_transfer_bytes);

        testFileSize(SIZE_KILO_BETES, EXPECTED_SIZE_KILO_BETES, Utils.SIZE_TYPE_K_BYTE,
                R.string.file_transfer_k_bytes);

        testFileSize(SIZE_M, EXPECTED_SIZE_M, Utils.SIZE_TYPE_M, R.string.file_transfer_m);
    }

    /**
     * Test whether a fileType is correctly got.
     */
    public void testCase2_GetSizeType() {
        Logger.d(TAG, "testCase2_GetSizeType() entry");
        int sizeType = Utils.getSizeType(SIZE_BETES);
        assertEquals(SIZE_TYPE_BYTE, sizeType);
        sizeType = Utils.getSizeType(SIZE_KILO_BETES);
        assertEquals(SIZE_TYPE_K_BYTE, sizeType);
        sizeType = Utils.getSizeType(SIZE_M);
        assertEquals(SIZE_TYPE_M, sizeType);
    }

    private void testFileSize(long fileSize, String expectedSize, int sizeType, int resId) {
        Logger.d(TAG, "testFileSize() entry, fileSize is " + fileSize + ",expectedSize is"
                + expectedSize + ",sizeType is " + sizeType + ",resId is " + resId);
        String actualSize = Utils.formatFileSizeToString(fileSize, sizeType);
        assertEquals(expectedSize, actualSize);
    }

    /**
     * Test rotate
     * 
     * @throws FileNotFoundException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase3_rotate() throws FileNotFoundException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase3_rotate() entry");
        // to cover the constructor
        Utils utils = new Utils();
        String picturePath = com.mediatek.rcse.test.Utils.getAPictureFilePath(mContext);
        Logger.d(TAG, "picturePath = " + picturePath);
        int degress = utils.getDegreesRotated(picturePath);
        Logger.d(TAG, "degress = " + degress);
        Bitmap bitmap = BitmapFactory.decodeFile(picturePath);
        Bitmap bitmapResult = utils.rotate(bitmap, 90);
        if (bitmapResult == null) {
            Logger.d(TAG, "rotate failed");
            return;
        }
        bitmap = BitmapFactory.decodeFile(picturePath);
        int piex1 = bitmap.getPixel(0, 1);
        int piex2 = bitmapResult.getPixel(0, 1);
        Logger.d(TAG, "piex1 = " + piex1 + ",piex2 = " + piex2);
        bitmapResult.recycle();
        bitmap.recycle();
        Method methodGetExifRotation = com.mediatek.rcse.test.Utils.getPrivateMethod(Utils.class,
                "getExifRotation", int.class);
        assertEquals(0, methodGetExifRotation.invoke(null, ExifInterface.ORIENTATION_NORMAL));
        assertEquals(90, methodGetExifRotation.invoke(null, ExifInterface.ORIENTATION_ROTATE_90));
        assertEquals(180, methodGetExifRotation.invoke(null, ExifInterface.ORIENTATION_ROTATE_180));
        assertEquals(270, methodGetExifRotation.invoke(null, ExifInterface.ORIENTATION_ROTATE_270));
        assertEquals(0, methodGetExifRotation.invoke(null, 20));
    }

    /**
     * Test compressImage
     */
    public void testCase4_compressImage() {
        Logger.d(TAG, "testCase4_compressImage() entry");
        String picturePath = com.mediatek.rcse.test.Utils.getAPictureFilePath(mContext);
        Logger.d(TAG, "picturePath = " + picturePath);
        String compressedPicturePath = Utils.compressImage(picturePath);
        Bitmap bitmap = null;
        Bitmap bitmapResult = null;
        try {
            bitmap = BitmapFactory.decodeFile(picturePath);
            bitmapResult = BitmapFactory.decodeFile(compressedPicturePath);
            if (bitmap != null && bitmapResult != null) {
                assertTrue(bitmap.getHeight() * bitmap.getWidth() >= bitmapResult.getHeight()
                        * bitmapResult.getWidth());
            }
        } catch (OutOfMemoryError e) {
            Logger.d(TAG, "OOM:" + e.getMessage());
        } finally {
            if (bitmap != null) {
                bitmap.recycle();
                bitmap = null;
            }
            if (bitmapResult != null) {
                bitmapResult.recycle();
                bitmapResult = null;
            }
        }
        // test compress a non-exist file
        assertTrue("/sdcard/non0123456dfsfgeddd.jpg.jpg".endsWith(Utils
                .compressImage("/sdcard/non0123456dfsfgeddd.jpg.jpg")));
        assertEquals(0, Utils.getDegreesRotated("/sdcard/non0123456dfsfgeddd.jpg.jpg"));
    }

    /**
     * Test formatFileSizeToString
     */
    public void testCase5_formatFileSizeToString() {
        Logger.d(TAG, "testCase5_formatFileSizeToString() entry");
        testFileSize(mContext, SIZE_BETES, EXPECTED_SIZE_BETES, Utils.SIZE_TYPE_TOTAL_SIZE,
                R.string.file_transfer_bytes);
        testFileSize(mContext, SIZE_KILO_BETES, EXPECTED_SIZE_KILO_BETES,
                Utils.SIZE_TYPE_TOTAL_SIZE, R.string.file_transfer_k_bytes);
        testFileSize(mContext, SIZE_M, EXPECTED_SIZE_M, Utils.SIZE_TYPE_TOTAL_SIZE,
                R.string.file_transfer_m);
        testFileSize(mContext, SIZE_BETES, EXPECTED_SIZE_BETES, Utils.SIZE_TYPE_BYTE,
                R.string.file_transfer_bytes);
        testFileSize(mContext, SIZE_KILO_BETES, EXPECTED_SIZE_KILO_BETES, Utils.SIZE_TYPE_K_BYTE,
                R.string.file_transfer_k_bytes);
        testFileSize(mContext, SIZE_M, EXPECTED_SIZE_M, Utils.SIZE_TYPE_M, R.string.file_transfer_m);
    }

    private void testFileSize(Context context, long fileSize, String expectedSize, int sizeType,
            int resId) {
        Logger.d(TAG, "testFileSize() entry, fileSize is " + fileSize + ",expectedSize is"
                + expectedSize + ",sizeType is " + sizeType + ",resId is " + resId);
        String actualSize = Utils.formatFileSizeToString(context, fileSize, sizeType);
        assertEquals(expectedSize, actualSize);
    }

    /**
     * Test getFileExtension
     */
    public void testCase6_getFileExtension() {
        assertNull(Utils.getFileExtension(null));
        assertEquals("txt", Utils.getFileExtension("1.txt"));
    }

}
