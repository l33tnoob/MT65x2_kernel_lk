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

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.ParcelFileDescriptor;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

public class Stereo3DSource {
    private static final String TAG = "Stereo3DSource";
    private Bitmap[] mPair = new Bitmap[2]; // two bitmaps
    private Bitmap mImage2D;

    /**
     * The method retrieves left and right bitmaps
     */
    protected void getPairBitmaps() {
        if (mPair[0] == null && mPair[1] == null) {
            mPair[0] = getBitmap(Stereo3DWallpaperManagerService.LEFT_WALLPAPER_FILE);
            mPair[1] = getBitmap(Stereo3DWallpaperManagerService.RIGHT_WALLPAPER_FILE);
        }
    }

    /**
     * The method creates a bitmap from a given file
     *
     * @param file the file used to generate the bitmap
     * @return a bitmap generated from the given file
     */
    private Bitmap getBitmap(File file) {
        ParcelFileDescriptor fd = openFile(file);

        if (fd != null) {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inDither = false;
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            Bitmap bitmap = BitmapFactory.decodeFileDescriptor(fd.getFileDescriptor(), null, options);

            try {
                fd.close();
            } catch (IOException e) {
                Stereo3DLog.log(TAG, "IOException - Failed to close file");
            }

            return bitmap;
        }

        return null;
    }

    /**
     * The method creates a new ParcelFileDescriptor accessing a given file
     *
     * @param file the file to be opened
     * @return a new ParcelFileDescriptor pointing to the given file
     */
    private ParcelFileDescriptor openFile(File file) {
        try {
            if (!file.exists()) {
                return null;
            }
            return ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
        } catch (FileNotFoundException e) {
            Stereo3DLog.log(TAG, "FileNotFoundException - Cannot open a file");
        }
        return null;
    }

    /**
     * The method retrieves the left image of a 3D content
     *
     * @return the left image
     */
    protected Bitmap getLeftImage() {
        return mPair[0];
    }

    /**
     * The method retrieves the right image of a 3D content
     *
     * @return the right image
     */
    protected Bitmap getRightImage() {
        return mPair[1];
    }

    /**
     * The method retrieves the image for 2D display
     *
     */
    protected Bitmap get2DImage() {
        if (mImage2D == null) {
            mImage2D = Stereo3DUtility.create2DImageFromHalfSideImage(mPair[0]);
        }

        return mImage2D;
    }

    /**
     * The method checks whether left and right bitmaps are valid
     *
     * @return true if both bitmaps are not null, false otherwise
     *
     */
    protected boolean isValid() {
        if (mPair[0] == null || mPair[1] == null) {
            return false;
        }

        return true;
    }

    /**
     * The method releases all bitmap resources
     *
     */
    protected void release() {
        for (int i = 0; i < mPair.length; i++) {
            if (mPair[i] != null) {
                mPair[i].recycle();
                mPair[i] = null;
            }
        }

        if (mImage2D != null) {
            mImage2D.recycle();
            mImage2D = null;
        }
    }
}