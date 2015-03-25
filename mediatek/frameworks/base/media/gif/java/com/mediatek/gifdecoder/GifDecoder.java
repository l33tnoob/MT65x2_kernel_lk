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

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.gifdecoder;

import com.mediatek.common.gifdecoder.IGifDecoder;

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import android.util.Log;

import android.graphics.Bitmap;
import android.graphics.Movie;

public class GifDecoder implements IGifDecoder {

    /**
     * Specify the minimal frame duration in GIF file, unit is ms.
     * Set as 100, then gif animation hehaves mostly as other gif viewer.
     * Patch 2012-07-25 change it to 40 to let GIF can be played smoothly
     */
    private static final int MINIMAL_DURATION = 40;

    /**
     * Invalid returned value of some memeber function, such as getWidth()
     * getTotalFrameCount(), getFrameDuration()
     */
    public static final int INVALID_VALUE = 0;


    /**
     * Movie object maitained by GifDecoder, it contains raw GIF info
     * like graphic control informations, color table, pixel indexes,
     * called when application is no longer interested in gif info. 
     * It is contains GIF frame bitmap, 8-bits per pixel, 
     * using SkColorTable to specify the colors, which is much
     * memory efficient than ARGB_8888 config. This is why we
     * maintain a Movie object rather than a set of ARGB_8888 Bitmaps.
     */
    private Movie mMovie;
    
    /**
     * Constructor of GifDecoder, which receives InputStream as
     * parameter. Decode an InputStream into Movie object. 
     * If the InputStream is null, no decoding will be performed
     *
     * @param is InputStream representing the file to be decoded.
     */
    public GifDecoder(InputStream is) {
android.util.Log.i("GifDecoder","GifDecoder(is="+is+")");
        if (is == null)
            return;
        mMovie = Movie.decodeStream(is);
    }

    public GifDecoder(byte[] data, int offset,int length) {
        if (data == null)
            return;
        mMovie = Movie.decodeByteArray(data, offset, length);
    }

    /**
     * Constructor of GifDecoder, which receives file path name as
     * parameter. Decode a file path into Movie object. 
     * If the specified file name is null, no decoding will be performed
     *
     * @param pathName complete path name for the file to be decoded.
     */
    public GifDecoder(String pathName) {
android.util.Log.i("GifDecoder","GifDecoder(pathName="+pathName+")");
        if (pathName == null)
            return;
        mMovie = Movie.decodeFile(pathName);
    }

    /**
     * Close gif file, release all informations like frame count,
     * graphic control informations, color table, pixel indexes,
     * called when application is no longer interested in gif info. 
     * It will release all the memory mMovie occupies. After close()
     * is call, GifDecoder should no longer been used.
     */
    public synchronized void close(){
        if (mMovie == null)
            return;
        mMovie.closeGif();
        mMovie = null;
    }

    /**
     * Get width of images in gif file. 
     * if member mMovie is null, returns INVALID_VALUE
     *
     * @return The total frame count of gif file,
     *         or INVALID_VALUE if the mMovie is null
     */
    public synchronized int getWidth() {
        if (mMovie == null)
            return INVALID_VALUE;
        return mMovie.width();
    }

    /**
     * Get height of images in gif file. 
     * if member mMovie is null, returns INVALID_VALUE
     *
     * @return The total frame count of gif file,
     *         or INVALID_VALUE if the mMovie is null
     */
    public synchronized int getHeight() {
        if (mMovie == null)
            return INVALID_VALUE;
        return mMovie.height();
    }

    /**
     * Get total duration of gif file. 
     * if member mMovie is null, returns INVALID_VALUE
     *
     * @return The total duration of gif file,
     *         or INVALID_VALUE if the mMovie is null
     */
    public synchronized int getTotalDuration() {
        if (mMovie == null)
            return INVALID_VALUE;
        return mMovie.duration();
    }

    /**
     * Get total frame count of gif file. 
     * if member mMovie is null, returns INVALID_VALUE
     *
     * @return The total frame count of gif file,
     *         or INVALID_VALUE if the mMovie is null
     */
    public synchronized int getTotalFrameCount() {
        if (mMovie == null)
            return INVALID_VALUE;
        return mMovie.gifTotalFrameCount();
    }

    /**
     * Get frame duration specified with frame index of gif file. 
     * if member mMovie is null, returns INVALID_VALUE
     *
     * @param frameIndex index of frame interested.
     * @return The duration of the specified frame,
     *         or INVALID_VALUE if the mMovie is null
     */
    public synchronized int getFrameDuration(int frameIndex) {
        if (mMovie == null)
            return INVALID_VALUE;
        int duration = mMovie.gifFrameDuration(frameIndex);
        if (duration < MINIMAL_DURATION)
            duration = MINIMAL_DURATION;
        return duration;
    }

    /**
     * Get frame bitmap specified with frame index of gif file. 
     * if member mMovie is null, returns null
     *
     * @param frameIndex index of frame interested.
     * @return The decoded bitmap, or null if the mMovie is null
     */
    public synchronized Bitmap getFrameBitmap(int frameIndex) {
        if (mMovie == null)
            return null;
        return mMovie.gifFrameBitmap(frameIndex);
    }
 
}
