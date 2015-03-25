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
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.app.mtv;


import android.content.ContentResolver;

import android.content.ContentValues;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.location.Location;
import android.net.Uri;
import android.os.Environment;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.provider.MediaStore.Images;
import android.util.Log;

import java.io.Closeable;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import com.mediatek.storage.StorageManagerEx;



/**
 * ImageManager is used to retrieve and store images
 * in the media content provider.
 */
public class ImageManager {
    private static final String TAG = "ATV/ImageManager";

    private static final Uri STORAGE_URI = Images.Media.EXTERNAL_CONTENT_URI;

    // Inclusion
    public static final int INCLUDE_IMAGES = (1 << 0);
    public static final int INCLUDE_DRM_IMAGES = (1 << 1);
    public static final int INCLUDE_VIDEOS = (1 << 2);
    
    public static final long ERROR_NO_STORAGE = -1L;
    public static final long ERROR_MEDIA_CHECKING = -2L;
    
    
    public static final String MTV_GALLARY_SUBPATH = "/DCIM/ATV";
    private static StorageManager sStorageManager;

    private static StorageManager getStorageManager(Context c) {
        if (sStorageManager == null) {
            sStorageManager = (StorageManager)c.getSystemService(Context.STORAGE_SERVICE);
        }
        return sStorageManager;
    }
    
    /**
     * Get the default storage patch.
     * @param c The context ImageManager run in.
     * @return Storage patch.
     */
    public static String getDefaultStoragePath(Context c) {
        //return getStorageManager(c).getDefaultPath().toString() + MTV_GALLARY_SUBPATH;
return StorageManagerEx.getDefaultPath().toString() + MTV_GALLARY_SUBPATH;
    }

    
    /**
     * Matches code in MediaProvider.computeBucketValues. Should be a common
     * function.
     */
    public static String getGalleryId(Context c) {
        return String.valueOf(getDefaultStoragePath(c).toLowerCase().hashCode());
    }

    /**
     * Set captured image size.
     * @param cr ContentResolver.
     * @param uri The newly captured url.
     * @param size The newly captured image size.
     */
    public static void setImageSize(ContentResolver cr, Uri uri, long size) {
        ContentValues values = new ContentValues();
        values.put(Images.Media.SIZE, size);
        cr.update(uri, values, null, null);
    }

    /**
     *  Stores a bitmap or a jpeg byte array to a file (using the specified
     *  directory and filename). Also add an entry to the media store for
     *  this picture. The title, dateTaken, location are attributes for the
     *  picture. The degree is a one element array which returns the orientation
     *  of the picture.
     *  @param cr The ContentResolver.
     *  @param title The title of the image.
     *  @param dateTaken The time when we get the image.
     *  @param location The location where capture the image.
     *  @param directory Save the image in the directory.
     *  @param filename The name of the image.
     */
    public static Uri addImage(ContentResolver cr, String title, long dateTaken,
            Location location, String directory, String filename,
            Bitmap source, byte[] jpegData) {
        // We should store image data earlier than insert it to ContentProvider, otherwise
        // we may not be able to generate thumbnail in time.

        OutputStream outputStream = null;
        String filePath = directory + "/" + filename;
        try {
            File dir = new File(directory);
            if (!dir.exists()) {
                if (!dir.mkdirs()) {
                    XLogUtils.w(TAG, "addImage error in mkdirs:" + dir);
                    return null;
                }
            }
            File file = new File(directory, filename);
            outputStream = new FileOutputStream(file);
            if (source == null) {
                outputStream.write(jpegData);
            } else {
                source.compress(CompressFormat.JPEG, 75, outputStream);
            }
        } catch (FileNotFoundException ex) {
            Log.w(TAG, ex);
            return null;
        } catch (IOException ex) {
            Log.w(TAG, ex);
            return null;
        } finally {
            closeSilently(outputStream);
        }

        ContentValues values = new ContentValues(7);
        values.put(Images.Media.TITLE, title);

        // That filename is what will be handed to Gmail when a user shares a
        // photo. Gmail gets the name of the picture attachment from the
        // "DISPLAY_NAME" field.
        values.put(Images.Media.DISPLAY_NAME, filename);
        values.put(Images.Media.DATE_TAKEN, dateTaken);
        values.put(Images.Media.MIME_TYPE, "image/jpeg");
        values.put(Images.Media.ORIENTATION,0);
        values.put(Images.Media.DATA, filePath);

        if (location != null) {
            values.put(Images.Media.LATITUDE, location.getLatitude());
            values.put(Images.Media.LONGITUDE, location.getLongitude());
        }

        long tt1 = System.currentTimeMillis();
        Uri uri = cr.insert(STORAGE_URI, values);
        long tt2 = System.currentTimeMillis();
        XLogUtils.v(TAG, "cr.insert needs " + (tt2 - tt1));
        return uri;
    }

    private static boolean checkFsWritable(String dir) {
        // Create a temporary file to see whether a volume is really writeable.
        // It's important not to put it in the root directory which may have a
        // limit on the number of files.
        File directory = new File(dir + MTV_GALLARY_SUBPATH);
        if (!directory.isDirectory()) {
            if (!directory.mkdirs()) {
                return false;
            }
        }
        return directory.canWrite();
    }

    /**
     * Get the SD card's status.
     * @param c The context ImageManager run in.
     * @return SD card's status.
     */
    public static long getStorageStatus(Context c) {
        //String dir = getStorageManager(c).getDefaultPath().toString();
        String dir = StorageManagerEx.getDefaultPath().toString();
        String state = getStorageManager(c).getVolumeState(dir);

        if (Environment.MEDIA_MOUNTED.equals(state)) {
            boolean writable = checkFsWritable(dir);        
            long freebytes = getFreeSpace(dir);
            XLogUtils.i(TAG, "hasStorage writable = " + writable);
            
            if (!writable && freebytes > 0) {
                // not writeable and is not caused by disk being full.
                return ERROR_NO_STORAGE;                
            } else {
                if (freebytes <= 0) {
                    //disk is mounted so disk should be full.
                    XLogUtils.i(TAG, "disk full");
                    return 0;//ensure not to return minus value to caller because we use some minus values to indicate error.
                } 
                return freebytes;
            }
        } else if (Environment.MEDIA_CHECKING.equals(state)) {
            return ERROR_MEDIA_CHECKING;
        }
        return ERROR_NO_STORAGE;
    }


    private static long getFreeSpace(String dir) {
        StatFs stat = new StatFs(dir);
        return (long) stat.getAvailableBlocks()
                    * (long) stat.getBlockSize();
    }
    
    private static void closeSilently(Closeable c) {
        if (c == null) {
            return;
        }
        try {
            c.close();
        } catch (IOException e) {  //Throwable t
            //  do nothing
           XLogUtils.w(TAG, "closeSilently error");
        }
    }        
}
