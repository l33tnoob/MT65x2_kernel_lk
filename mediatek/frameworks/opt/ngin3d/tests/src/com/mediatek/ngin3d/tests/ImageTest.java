/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ngin3d.tests;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.j3m.AssetPool;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.j3m.J3mPresentationEngine;
import com.mediatek.ngin3d.presentation.BitmapGenerator;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ImageSource;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static android.test.MoreAsserts.assertNotEqual;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;


public class ImageTest extends Ngin3dInstrumentationTestCase {
    @SmallTest
    public void testAnchorPoint() {
        Image image = Image.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.android);
        image.setAnchorPoint(new Point(0.5f, 1.0f));
        assertEquals(image.getAnchorPoint().x, 0.5f);
        assertEquals(image.getAnchorPoint().y, 1.0f);
        assertEquals(image.getAnchorPoint().z, 0.0f);
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buf = new byte[1024];
        int len;
        while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
        }
        in.close();
        out.close();
    }

    // \todo Add support for getTotalCImageBytes and getTotalTextureBytes in
    // A3M. Put back inner code for testing by removing !Ngin3d.usingA3m()
    // conditional statements. Then, run instrumentation test to test A3M's
    // support for getTotalCImageBytes and getTotalTextureBytes.
    @SmallTest
    public void testImageFromResource() throws ExecutionException, InterruptedException {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;

        for (int i = 0 ; i < 10 ; i ++) {
            final Image bmpImage = Image.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.earth);
            final Image pngImage = Image.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.building_london);
            final Image jpgImage = Image.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.photo_01);

            FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    bmpImage.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

            BitmapFactory.decodeResource(getInstrumentation().getContext().getResources(), R.drawable.earth, options);
            assertEquals(options.outHeight, (int) bmpImage.getSize().height);
            assertEquals(options.outWidth, (int)bmpImage.getSize().width);

            task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    pngImage.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

            BitmapFactory.decodeResource(getInstrumentation().getContext().getResources(), R.drawable.building_london, options);
            assertEquals(options.outHeight, (int)pngImage.getSize().height);
            assertEquals(options.outWidth, (int)pngImage.getSize().width);

            task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    jpgImage.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

            BitmapFactory.decodeResource(getInstrumentation().getContext().getResources(), R.drawable.photo_01, options);
            assertEquals(options.outHeight, (int)jpgImage.getSize().height);
            assertEquals(options.outWidth, (int)jpgImage.getSize().width);

            task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    bmpImage.unrealize();
                    pngImage.unrealize();
                    jpgImage.unrealize();
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());
        }
    }

    // \todo Add support for getTotalCImageBytes and getTotalTextureBytes in
    // A3M. Put back inner code for testing by removing !Ngin3d.usingA3m()
    // conditional statements. Then, run instrumentation test to test A3M's
    // support for getTotalCImageBytes and getTotalTextureBytes.
    @SmallTest
    public void testImageFromBitmapGenerator() throws ExecutionException, InterruptedException {
        final int size = 32;
        BitmapGenerator generator = new BitmapGenerator() {
            public Bitmap generate() {
                Bitmap bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
                return bitmap;
            }
        };

        for (int i = 0; i < 10; i++) {
            final Image bmpImage = Image.createFromBitmapGenerator(generator);
            generator.cacheBitmap();

            FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    bmpImage.realize(mPresentationEngine);
                    assertEquals(size, (int)bmpImage.getSize().height);
                    assertEquals(size, (int)bmpImage.getSize().width);

                    bmpImage.unrealize();
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());
        }

        generator.cacheBitmap();
        assertNotNull(generator.getBitmap());
        generator.free();
        assertNull(generator.getBitmap());
    }

    // \todo Add support for getTotalCImageBytes and getTotalTextureBytes in
    // A3M. Put back inner code for testing by removing !Ngin3d.usingA3m()
    // conditional statements. Then, run instrumentation test to test A3M's
    // support for getTotalCImageBytes and getTotalTextureBytes.
    @SmallTest
    public void testImageFromFile() throws ExecutionException, InterruptedException {
        int totalImageBytes = 0;
        int totalTextureBytes = 0;

        String imagePath = "/sdcard/mage/";
        // For A3M, to load an image file it is required to register a file
        // path with the asset pool. A3M native engine then uses it search the
        // image file from it.

        AssetPool pool =
            ((J3mPresentationEngine) mPresentationEngine).getAssetPool();
        pool.registerSource(imagePath);


        String testFile1 = "photo_01_big.jpg";
        String testFile2 = "photo_02_big.jpg";
        String testFile3 = "photo_03_big.jpg";
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        for (int i = 0 ; i < 10 ; i ++) {
            final Image image1 = Image.createFromFile(testFile1);
            final Image image2 = Image.createFromFile(testFile2);
            final Image image3 = Image.createFromFile(testFile3);

            FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    image1.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

            BitmapFactory.decodeFile(imagePath + testFile1, options);
            assertEquals(options.outHeight, (int) image1.getSize().height);
            assertEquals(options.outWidth, (int)image1.getSize().width);

            task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    image2.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

            BitmapFactory.decodeFile(imagePath + testFile2, options);
            assertEquals(options.outHeight, (int)image2.getSize().height);
            assertEquals(options.outWidth, (int) image2.getSize().width);

            task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    image3.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

            BitmapFactory.decodeFile(imagePath + testFile3, options);
            assertEquals(options.outHeight, (int)image3.getSize().height);
            assertEquals(options.outWidth, (int)image3.getSize().width);

            task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    image1.unrealize();
                    image2.unrealize();
                    image3.unrealize();
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());
        }
    }

    @SmallTest
    public void testImageDecoding() throws Exception {
        AssetManager assetManager = getInstrumentation().getContext().getAssets();
        try {
            InputStream is = assetManager.open("photo_01.jpg");
            Bitmap bitmap = BitmapFactory.decodeStream(is);
            is.close();
            final Image image = Image.createFromBitmap(bitmap);
            FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    image.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

        } catch (IOException e) {
            throw e;
        }

        try {
            final Image image = Image.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.sydney);
            FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
                public Boolean call() {
                    image.realize(mPresentationEngine);
                    return true;
                }
            });

            mStageView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());

        } catch (Exception e) {
            throw e;
        }

        // test null case
        try {
            Image.createFromFile(null);
            fail("NullPointerException expected.");
        } catch (NullPointerException e) {
            // expected
        }

        try {
            Image.createFromBitmap(null);
            fail("NullPointerException expected.");
        } catch (NullPointerException e) {
            // expected
        }
    }

    @SmallTest
    public void testOpacitySet() {
        Image image = Image.createEmptyImage();
        image.setOpacity(0);
        assertEquals(image.getOpacity(), 0);

        image.setOpacity(255);
        assertEquals(image.getOpacity(), 255);

        try {
            image.setOpacity(300);
            fail("IllegalArgumentException expected.");
        } catch (IllegalArgumentException e) {
            // expected
        }
        assertEquals(image.getOpacity(), 255);

        try {
            image.setOpacity(-50);
            fail("IllegalArgumentException expected.");
        } catch (IllegalArgumentException e) {
            // expected
        }
        assertEquals(image.getOpacity(), 255);
    }

    public void testImageSource() {
        ImageDisplay.Resource res = new ImageDisplay.Resource(getInstrumentation().getContext().getResources(), R.drawable.android);
        ImageSource imageSource = new ImageSource(ImageSource.RES_ID, res);
        ImageSource imageSource2 = new ImageSource(ImageSource.RES_ID, res);
        assertEquals(imageSource2.toString(), imageSource.toString());

        ImageDisplay.Resource res1 = new ImageDisplay.Resource(getInstrumentation().getContext().getResources(), R.drawable.android);
        ImageDisplay.Resource res2 = new ImageDisplay.Resource(getInstrumentation().getContext().getResources(), R.drawable.earth);
        assertEquals(res, res1);
        assertNotEqual(res, res2);
        assertEquals(res.hashCode(), res1.hashCode());
    }

    public void testOtherMethod() {
        Image image = Image.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.earth);
        image.setFilterQuality(1);
        assertThat(image.getFilterQuality(), is(1));

        image.setRepeat(2, 2);
        assertThat(image.getRepeatX(), is(2));
        assertThat(image.getRepeatY(), is(2));

        image.setKeepAspectRatio(true);
        assertTrue(image.isKeepAspectRatio());
    }
}
