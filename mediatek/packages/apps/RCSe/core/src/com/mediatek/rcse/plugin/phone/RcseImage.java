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

package com.mediatek.rcse.plugin.phone;

import android.content.ContentResolver;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Point;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.view.WindowManager;

import com.mediatek.rcse.api.Logger;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.IOException;

public final class RcseImage {
    private static final String TAG = "RcseImage";
    private final Uri mUri;
    private Context mContext;

    RcseImage(Context context, Uri uri) {
        mContext = context;
        mUri = uri;
    }

    private int getExifRotation(int orientation) {
        int degrees = 0;
        switch (orientation) {
            case ExifInterface.ORIENTATION_NORMAL:
                degrees = 0;
                break;
            case ExifInterface.ORIENTATION_ROTATE_90:
                degrees = 90;
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                degrees = 180;
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                degrees = 270;
                break;
            default:
                break;
        }
        return degrees;
    }

    private int getDegreesRotated() {
        Logger.v(TAG, "getDegreesRotated entry");
        int degreesRotated = 0;
        int orientation = 0;
        String dataPath = mUri.getPath();
        try {
            ExifInterface exif = new ExifInterface(dataPath);
            orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, 0);
            degreesRotated = getExifRotation(orientation);
        } catch (IOException ex) {
            Logger.d(TAG, "getDegreesRotated cannot read exif");
            ex.printStackTrace();
        }
        Logger.v(TAG, "getDegreesRotated exit with degreesRotated: " + degreesRotated);
        return degreesRotated;
    }

    private int computeSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        double w = options.outWidth;
        double h = options.outHeight;
        int roundedSize1;
        int roundedSize2;

        roundedSize1 = (int) Math.ceil(Math.sqrt(w * h / maxNumOfPixels));
        w = (w > h) ? h : w;
        roundedSize2 = (int) Math.floor(w / minSideLength);
        return (roundedSize1 > roundedSize2) ? roundedSize1 : roundedSize2;
    }

    private void closeSilently(ParcelFileDescriptor c) {
        Logger.v(TAG, "closeSilently entry");
        if (c == null) {
            return;
        }
        try {
            c.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private Bitmap makeBitmap(int minSideLength, int maxNumOfPixels, Uri uri, ContentResolver cr,
            ParcelFileDescriptor pfd, final BitmapFactory.Options options) {
        Logger.v(TAG, "makeBitmap entry");
        if (pfd == null) {
            Logger.d(TAG, "makeBitmap pfd is null");
            return null;
        }
        try {
            BitmapFactory.Options tempOptions = options;
            if (tempOptions == null) {
                tempOptions = new BitmapFactory.Options();
            }
            FileDescriptor fd = pfd.getFileDescriptor();
            tempOptions.inJustDecodeBounds = true;
            BitmapFactory.decodeFileDescriptor(fd, null, tempOptions);
            if (tempOptions.mCancel || tempOptions.outWidth == -1 || tempOptions.outHeight == -1) {
                return null;
            }
            tempOptions.inSampleSize =
                    computeSampleSize(tempOptions, minSideLength, maxNumOfPixels);
            tempOptions.inJustDecodeBounds = false;

            // for zoom pan performance enhancement,
            // load full size bitmap in format RGB_565, with dither option
            // on
            tempOptions.inDither = false;
            tempOptions.inPreferredConfig = Bitmap.Config.ARGB_8888;
            // options.inDither = true;
            // options.inPreferredConfig = Bitmap.Config.RGB_565;
            Logger.v(TAG, "makeBitmap exit");
            return BitmapFactory.decodeFileDescriptor(fd, null, tempOptions);
        } catch (OutOfMemoryError ex) {
            Logger.e(TAG, "got oom exception");
            ex.printStackTrace();
            return null;
        } finally {
            closeSilently(pfd);
        }
    }

    public Bitmap fullSizeBitmap(int maxNumberOfPixels, boolean rotateAsNeeded) {
        Logger.v(TAG, "fullSizeBitmap entry");
        int minSideLength = getProperWindowSize();
        String path = mUri.getPath();
        ParcelFileDescriptor pfdInput = null;
        try {
            pfdInput = ParcelFileDescriptor.open(new File(path), ParcelFileDescriptor.MODE_READ_ONLY);
        } catch (FileNotFoundException e) {
            Logger.e(TAG, "got exception decoding bitmap ");
            e.printStackTrace();
        }
        Bitmap bitmap = null;
        bitmap = makeBitmap(minSideLength, maxNumberOfPixels, null, null, pfdInput, null);
        if (bitmap != null && rotateAsNeeded) {
            Bitmap rotatedBitmap = rotate(bitmap, getDegreesRotated());
            if (rotatedBitmap != null && rotatedBitmap != bitmap) {
                Logger.v(TAG, "fullSizeBitmap rotate");
                bitmap.recycle();
                bitmap = rotatedBitmap;
            } else {
                Logger.d(TAG, "fullSizeBitmap no need to roate");
            }
        }
        Logger.v(TAG, "fullSizeBitmap exit");
        return bitmap;
    }

    private int getProperWindowSize() {
        int viewHeight;
        int viewWidth;
        WindowManager wm = (WindowManager) (mContext.getSystemService(Context.WINDOW_SERVICE));
        Point point = new Point();
        wm.getDefaultDisplay().getSize(point);
        viewWidth = point.x;
        viewHeight = point.y;
        viewWidth = (viewWidth > viewHeight) ? viewHeight : viewWidth;
        return viewWidth;
    }

    private Bitmap rotate(final Bitmap b, int degrees) {
        Logger.v(TAG, "rotate entry");
        Bitmap bitmap = b;
        if (degrees != 0 && bitmap != null) {
            Matrix m = new Matrix();
            m.setRotate(degrees, (float) bitmap.getWidth() / 2, (float) bitmap.getHeight() / 2);
            try {
                Bitmap bitmapRotated =
                        Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), m,
                                true);
                if (bitmap != bitmapRotated) {
                    bitmap.recycle();
                    bitmap = bitmapRotated;
                } else {
                    Logger.d(TAG, "no need to rotate");
                }
            } catch (OutOfMemoryError ex) {
                // We have no memory to rotate. Return the original bitmap.
                ex.printStackTrace();
            }
        } else {
            Logger.d(TAG, "rotate no need to rotate");
        }
        Logger.v(TAG, "rotate exit");
        return bitmap;
    }
}
