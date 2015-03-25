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

package com.mediatek.stereo3dwallpaper;

import android.content.ContentResolver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapRegionDecoder;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.webkit.MimeTypeMap;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.mpodecoder.IMpoDecoder;

import java.io.FileNotFoundException;
import java.io.IOException;

public final class Stereo3DUtility {
    private static final String TAG = "Stereo3DUtility";
    // must sync with Mpo_type.h
    static final int STEREO3D = 131074;
    static final int PANORAMA3D = 131073;
    static int sScreenWidth;
    static int sScreenHeight;
    static int sScreenMaxSide;

    private Stereo3DUtility() {
        // private constructor; it is a singleton class.
    }

    private static int getOrientation(ContentResolver resolver, Uri uri) {
        int rotation = 0;
        String filePath = null;
        boolean isRotationFound = false;

        Cursor c = resolver.query(uri,
                                  new String[] {MediaStore.Images.ImageColumns._ID,
                                                MediaStore.Images.ImageColumns.DATA,
                                                MediaStore.Images.ImageColumns.ORIENTATION},
                                  null, null, MediaStore.Images.Media.DEFAULT_SORT_ORDER);


        if (c != null && c.moveToFirst()) {
            filePath = c.getString(1);
            if (!c.isNull(2)) {
                rotation = c.getInt(2);
                isRotationFound = true;
            }
        }

        if (c != null) {
            c.close();
        }

        if (!isRotationFound && filePath != null) {
            // query DB failed, try exif interface
            try {
                ExifInterface exif = new ExifInterface(filePath);
                int exifRotation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
                switch (exifRotation) {
                case ExifInterface.ORIENTATION_NORMAL:
                    rotation = 0;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_90:
                    rotation = 90;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    rotation = 180;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    rotation = 270;
                    break;
                default:
                    Stereo3DLog.log(TAG, "Rotation in exif is not available!");
                }
            } catch (IOException e) {
                Stereo3DLog.log(TAG, "Exception when trying to fetch orientation from exif");
            }
        }

        return rotation;
    }

    /**
     * This method decodes a MPO URI and retrieves two or more JPEG components;
     * commonly a left view and a right view images
     *
     * @param resolver the content resolver that provides an access to the content model
     * @param uri the URI of the image
     * @param bitmapArray the bitmap array that contains two bitmaps
     *                    representing the left and right views.
     * @return true if MPO contains two views, false, otherwise.
     */
    public static int decodeMpoUri(ContentResolver resolver, Uri uri, Bitmap[] bitmapArray) {
        Stereo3DLog.log(TAG, "Decode mpo uri");

        IMpoDecoder mpoDecoder = MediatekClassFactory.createInstance(IMpoDecoder.class,
                                 IMpoDecoder.DECODE_URI, resolver, uri);

        // check if image orientation is specified in db or in Exif
        int rotation = getOrientation(resolver, uri);

        return decodeMpo(mpoDecoder, bitmapArray, rotation);
    }

    /**
     * This method parses two or more JPEG components from a MPO decoder;
     * commonly a left view and a right view images
     *
     * @param mpoDecoder the decoder that is used to parse the left and right images
     * @param bitmapArray the bitmap array that contains two bitmaps
     *                    representing the left and right views.
     * @param rotation rotate the bitmap according to the specified rotation
     * @return true if MPO contains two views, false, otherwise.
     */
    private static int decodeMpo(IMpoDecoder mpoDecoder, Bitmap[] bitmapArray, int rotation) {
        int result = -1;

        if (mpoDecoder != null) {
            int totalCount = mpoDecoder.frameCount();
            Stereo3DLog.log(TAG, mpoDecoder.width() + "x" + mpoDecoder.height());
            Stereo3DLog.log(TAG, "TotalCount: " + totalCount);

            int mpoType = mpoDecoder.suggestMtkMpoType();
            Stereo3DLog.log(TAG, "mpoType: " + mpoType);

            // re-scale image
            BitmapFactory.Options mOptions = new BitmapFactory.Options();
            mOptions.inSampleSize = calculateSampleSize(mpoDecoder.width(), mpoDecoder.height());

            if (totalCount < 2) {
                return result;
            }

            int curFrame = 0;
            int index = 0;


            while (index < 2) {
                bitmapArray[index] = mpoDecoder.frameBitmap(curFrame, mOptions);

                if (bitmapArray[index] != null) {
                    // after a rough re-scaling, get a real re-scaled bitmap
                    bitmapArray[index] = scaleHalfSideImage(bitmapArray[index], rotation);

                    Stereo3DLog.log(TAG, "bitmapArray[index] : " + bitmapArray[index].getWidth()
                                    + " x " + bitmapArray[index].getHeight());
                }

                // 3D panorama mpo file usually contains four frames:
                // two big frames and two small frames; the order is B S B S
                // therefore, we add two to skip S
                if (mpoType == PANORAMA3D && totalCount >= 4) {
                    curFrame = curFrame + 2;
                } else {
                    ++curFrame;
                }
                ++index;
            }
            result = mpoType;
        }

        return result;
    }

    /**
     * This method decodes a JPS file and splits the image into two bitmaps;
     * commonly a left view and a right view images
     *
     * @param resolver the content resolver that provides an access to the content model
     * @param uri the URI of the image
     * @param bitmapArray the bitmap array that contains two bitmaps
     *                    representing the left and right views.
     */
    public static void decodeJpsUri(ContentResolver resolver, Uri uri, Bitmap[] bitmapArray) {
        ParcelFileDescriptor fd = null;

        try {
            fd = resolver.openFileDescriptor(uri, "r");
            BitmapRegionDecoder regionDecoder =
                BitmapRegionDecoder.newInstance(fd.getFileDescriptor(), false);

            int width = regionDecoder.getWidth();
            int height = regionDecoder.getHeight();
            Stereo3DLog.log(TAG, "Jps uri: " + width + " x " + height);

            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = calculateSampleSize(width / 2, height);
            Bitmap bitmapL = regionDecoder.decodeRegion(new Rect(0, 0, width / 2, height), options);
            Bitmap bitmapR = regionDecoder.decodeRegion(new Rect(width / 2, 0, width, height), options);

            // check if image orientation is specified in db or in Exif
            int rotation = getOrientation(resolver, uri);

            if (bitmapL != null && bitmapR != null) {
                bitmapArray[0] = scaleHalfSideImage(bitmapL, rotation);
                bitmapArray[1] = scaleHalfSideImage(bitmapR, rotation);

                bitmapL.recycle();
                bitmapR.recycle();
                bitmapL = null;
                bitmapR = null;
            }

        } catch (FileNotFoundException e) {
            Stereo3DLog.log(TAG, "FileNotFoundException - Cannot create a file descriptor");
        } catch (IOException e) {
            Stereo3DLog.log(TAG, "IOException - Cannot decode jps file");
        } finally {
            try {
                if (fd != null) {
                    fd.close();
                }
            } catch (IOException e) {
                Stereo3DLog.log(TAG, "IOException - Cannot close file descriptor");
            }
        }
    }

    /**
     * This method scales the left/right image based on an original left/right image.
     * Note that the resolution of the width is half for 3D display.
     *
     * @param bitmap the original bitmap
     * @param rotation rotate the bitmap according to the specified rotation
     * @return the scaled bitmap
     */
    private static Bitmap scaleHalfSideImage(Bitmap bitmap, int rotation) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();

        int rotatedWidth = width;
        int rotatedHeight = height;

        Stereo3DLog.log(TAG, "Scale half side image width x height " + width + " x " + height
                        + " with rotation " + rotation);

        if (rotation == 90 || rotation == 270) {
            rotatedWidth = bitmap.getHeight();
            rotatedHeight = bitmap.getWidth();

            Stereo3DLog.log(TAG, "Rotated image width x height " + rotatedWidth + " x " + rotatedHeight);
        }

        float scaleW = (float)sScreenMaxSide / (float) rotatedWidth;
        float scaleH = (float)sScreenMaxSide / (float) rotatedHeight;

        Stereo3DLog.log(TAG, "scale" + scaleW + " x " + scaleH);

        float scale = Math.max(scaleW, scaleH);

        Matrix matrix = new Matrix();
        matrix.postRotate((float)rotation);
        matrix.postScale(scale * 0.5f, scale);

        return Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true);
    }

    /**
     * This method creates a full 2D image from a half side image.
     * Note that the resolution of the width is double for full 2D image.
     *
     * @param bitmap the original bitmap
     * @return the resized bitmap
     */
    protected static Bitmap create2DImageFromHalfSideImage(Bitmap bitmap) {
        Matrix matrix = new Matrix();

        // resolution of the width is double of the half side image
        matrix.postScale(2.0f, 1.0f);

        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
    }

    /**
     * This method calculates the sample size to shrink the bitmap
     *
     * @param width  the width of the bitmap
     * @param height the height of the bitmap
     * @return the sample size
     */
    private static int calculateSampleSize(int width, int height) {
        int sampleSize = 1;
        int wRatio = 0;
        int hRatio = 0;

        wRatio = (int) Math.ceil((float) width / (float) sScreenMaxSide);
        hRatio = (int) Math.ceil((float) height / (float) sScreenMaxSide);

        if (wRatio > 1 || hRatio > 1) {
            sampleSize = Math.min(wRatio, hRatio);
        }

        // if the sample size is not a power of 2, make it power of 2
        if ((sampleSize & -sampleSize) != sampleSize) {
            sampleSize = sampleSize << 1;
            sampleSize = Integer.highestOneBit(sampleSize);
        }

        // limit to size 1.5M approximately to prevent decoding failure
        int newSize = width / sampleSize * height / sampleSize;

        while (newSize > 1500000) {
            sampleSize *= 2;
            newSize = width / sampleSize * height / sampleSize;
        }

        Stereo3DLog.log(TAG, "Sample size: " + sampleSize);
        return sampleSize; // make it power of 2
    }

    /**
     * This method sets the current screen dimension
     *
     * @param width the screen width
     * @param height the screen height
     */
    protected static void setScreenDimension(int width, int height) {
        sScreenWidth = width;
        sScreenHeight = height;
        sScreenMaxSide = Math.max(sScreenWidth, sScreenHeight);
        Stereo3DLog.log(TAG, "Screen width: " + sScreenWidth);
        Stereo3DLog.log(TAG, "Screen height: " + sScreenHeight);
    }

    /**
     * This method gets the MIME type based on URI
     *
     * @param resolver the content resolver that provides an access to the content model
     * @param uri the URI of the image
     * @return the MIME type of the file
     */
    protected static String getMimeType(ContentResolver resolver, Uri uri) {
        if (ContentResolver.SCHEME_FILE.equals(uri.getScheme())) {
            String extension = MimeTypeMap.getFileExtensionFromUrl(uri.toString());
            String type = MimeTypeMap.getSingleton()
                          .getMimeTypeFromExtension(extension);
            if (type != null) {
                return type;
            }

            // this is for auto test
            if (extension.equalsIgnoreCase("jps")) {
                return "image/jps";
            } else if (extension.equalsIgnoreCase("mpo")) {
                return "image/mpo";
            }
        }

        return resolver.getType(uri);
    }
}