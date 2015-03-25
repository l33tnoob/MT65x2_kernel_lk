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

/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.dcfdecoder;

import android.content.ContentResolver;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory.Options;
import android.net.Uri;
import android.provider.MediaStore.Images;
import android.util.Log;

import com.mediatek.common.dcfdecoder.IDcfDecoder;

public class DcfDecoder implements IDcfDecoder {
    private static final String TAG = "DRM/DcfDecoder";

    static {
        System.loadLibrary("dcfdecoderjni");
    }

    public DcfDecoder() {}

    public byte[] decryptFile(String pathName, boolean consume) {
        if (null == pathName) {
            Log.e(TAG, "decryptFile: find null file name!");
            return null;
        }
        return nativeDecryptFile(pathName, consume);
    }

    public byte[] forceDecryptFile(String pathName, boolean consume) {
        if (null == pathName) {
            Log.e(TAG, "forceDecryptFile: find null file name!");
            return null;
        }
        return nativeForceDecryptFile(pathName, consume);
    }

    public byte[] decryptUri(ContentResolver cr, Uri uri, boolean consume) {
        String filepath = getFilepathFromUri(cr, uri);
        if (null == filepath) {
            Log.e(TAG, "decryptUri: can not get file path from uri!");
            return null;
        }
        return decryptFile(filepath, consume);
    }

    public byte[] forceDecryptUri(ContentResolver cr, Uri uri, boolean consume) {
        String filepath = getFilepathFromUri(cr, uri);
        if (null == filepath) {
            Log.e(TAG, "forceDecryptUri: can not get file path from uri!");
            return null;
        }
        return forceDecryptFile(filepath, consume);
    }

    /**
     * Decode a file path into a bitmap. If the specified file name is null,
     * or cannot be decoded into a bitmap, the function returns null.
     *
     * @param pathName complete path name for the file to be decoded.
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @param consume True to consume rights, false not.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     */
    public Bitmap decodeFile(String pathName, Options opts, boolean consume) {
        if (null == pathName) {
            Log.e(TAG, "decodeFile: find null file name!");
            return null;
        }
        return nativeDecodeFile(pathName,opts,consume);
    }

    /**
     * Decode a file path into a bitmap, no matter whether DRM right expired.
     * If the specified file name is null, or cannot be decoded into a bitmap,
     * the function returns null.
     *
     * @param pathName complete path name for the file to be decoded.
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @param consume True to consume rights, false not.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     */
    public Bitmap forceDecodeFile(String pathName, Options opts, boolean consume) {
        if (null == pathName) {
            Log.e(TAG, "forceDecodeFile: find null file name!");
            return null;
        }
        return nativeForceDecodeFile(pathName,opts,consume);
    }

    /**
     * Decode a file uri into a bitmap. If the specified file uri is null,
     * or cannot be decoded into a bitmap, the function returns null.
     *
     * @param uri Uri for the file to be decoded.
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @param consume True to consume rights, false not.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     */
    public Bitmap decodeUri(ContentResolver cr, Uri uri, Options opts, boolean consume) {
        String filepath = getFilepathFromUri(cr, uri);
        if (null == filepath) {
            Log.e(TAG, "decodeUri: can not get file path from uri!");
            return null;
        }
        return decodeFile(filepath, opts, consume);
    }

    /**
     * Decode a file uri into a bitmap. , no matter whether DRM right expired.
     * If the specified file uri is null, or cannot be decoded into a bitmap,
     * the function returns null.
     *
     * @param uri Uri for the file to be decoded.
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @param consume True to consume rights, false not.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     */
    public Bitmap forceDecodeUri(ContentResolver cr, Uri uri, Options opts, boolean consume) {
        String filepath = getFilepathFromUri(cr, uri);
        if (null == filepath) {
            Log.e(TAG, "forceDecodeUri: can not get file path from uri!");
            return null;
        }
        return forceDecodeFile(filepath,opts,consume);
    }

    private native Bitmap nativeDecodeFile(String pathName, Options opts, boolean consume);

    private native Bitmap nativeForceDecodeFile(String pathName, Options opts, boolean consume);

    private native byte[] nativeDecryptFile(String pathName, boolean consume);

    private native byte[] nativeForceDecryptFile(String pathName, boolean consume);

    //if uri is context://, query file path from data base
    //(context://media is media database, but it maybe 
    //start with "context://com.android.providers.media.documents",
    //it is a documentsUi uri)
    //if uri is file://, get corresponding String
    //else return null
    private String getFilepathFromUri(ContentResolver cr, Uri uri) {
        String filepath = null;
		
		Log.v(TAG, "getFilepathFromUri: uri is " + uri);

        if (null == cr) {
            Log.e(TAG, "getFilepathFromUri: find null ContentResolver!");
            return null;
        }
        if (null == uri) {
            Log.e(TAG, "getFilepathFromUri: find null uri!");
            return null;
        }

        String uriStr = uri.toString();
        if (null == uriStr) {
            Log.e(TAG, "getFilepathFromUri: convert Uri object to String failed!");
            return null;
        }
        if (uriStr.startsWith("content://")) {
            //for content://media type uri, we first get its corresponding file path
            Cursor c = null;
            try {
                c = cr.query(uri, new String[]{Images.ImageColumns.DATA}, null, null, null);
                if (c == null) {
                    Log.e(TAG, "getFilepathFromUri: no cursor returned for Uri "+uri);
                    return null;
                }
                if (1 != c.getCount()) {
                    Log.e(TAG, "getFilepathFromUri: record number in returned cursor is "+c.getCount());
                    return null;
                }
                if (c.moveToFirst()) {
					int columnIndex = c.getColumnIndex(Images.ImageColumns.DATA);
					if (columnIndex != -1) {
						filepath = c.getString(columnIndex);
					}
                } else {
                    Log.e(TAG, "getFilepathFromUri: move to first record of cursor failed!");
                    return null;
                }
            } catch (Exception ex) {// becasue provider will throw different exception, so will catch Exception directly.
                Log.w(TAG, ex);
            } finally {
                if (c != null) {
                    c.close();
                }
            }
        }

        if (null == filepath) {
            if (!uri.getScheme().equals("file")) {
                Log.e(TAG, "getFilepathFromUri: the uri does not starts with file://, return null Bitmap");
                return null;
            } else {
                filepath = uri.getPath();
            }
        }
        return filepath;
    }
}
