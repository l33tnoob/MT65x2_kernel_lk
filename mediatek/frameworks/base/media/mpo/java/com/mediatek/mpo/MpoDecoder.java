/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.mpo;

import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import android.util.Log;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory.Options;
import android.net.Uri;
import android.content.ContentResolver;

public class MpoDecoder {
    private static final String TAG = "MpoDecoder";

    private final int mNativeMpoDecoder;
    
    static {
        System.loadLibrary("mpojni");
    }

    private MpoDecoder(int nativeMpoDecoder) {
        if (nativeMpoDecoder == 0) {
            throw new RuntimeException("native mpo decoder creation failed");
        }
        mNativeMpoDecoder = nativeMpoDecoder;
    }

    public native int width();
    public native int height();
    public native int frameCount();
    public native int getMtkMpoType();
    public native int suggestMtkMpoType();
    public native Bitmap frameBitmap(int frameIndex, Options options);

    /**
     * This method release all the Info stored for MPO.
     * After this method is call, Movie Object should no longer be used.
     * eg. mMpoDecoder.closeMpo();
     *     mMpoDecoder = null;
     */
    public native void close();

    public static native MpoDecoder decodeFile(String pathName);

    public static MpoDecoder decodeUri(ContentResolver cr, Uri mpoUri) {
        Log.i(TAG,"decodeUri(mpoUri="+mpoUri+") ");
        if (null == mpoUri) return null;
        byte[] buffer = getByteBuffer(cr, mpoUri);
Log.v(TAG,"buffer="+buffer);
        if (null == buffer) {
            Log.e(TAG,"got null buffer from "+mpoUri);
            return null;
        }
        return decodeByteArray(buffer, 0, buffer.length);
    }

    private static byte[] getByteBuffer(ContentResolver cr, Uri uri) {
Log.i(TAG,"Image Uri:"+uri);
        InputStream mpoStream = null;
        try {
            mpoStream = cr.openInputStream(uri);
//Log.i(TAG,"mpoStream.available()="+mpoStream.available());
Log.v(TAG,"we want to get stream size..");
            final int BufSize = 4096*16;
            byte [] buffer = new byte[BufSize];
            int streamSize = 0;
            int readSize = 0;
            do {
                readSize = mpoStream.read(buffer);
                if (0 < readSize) {
                    streamSize += readSize;
                }
            } while (0 < readSize);
Log.i(TAG,"streamSize="+streamSize);
            if (streamSize <= 0) {
                Log.e(TAG,"got invalid stream length of MPO");
                return null;
            }
            //close the open stream
            mpoStream.close();
Log.v(TAG,"reopen stream");
            //reopen the stream
            mpoStream = cr.openInputStream(uri);
Log.v(TAG,"allocate bysste");
            //allocate buffer for mpo stream
            buffer = new byte[streamSize+1];
Log.v(TAG,"read stream..");
            //read the whole stream to buffer
            readSize = mpoStream.read(buffer);
            //now data is in buffer, stream is no longer used
            mpoStream.close();
Log.v(TAG,"read whole stream length:"+readSize);
            if (readSize != streamSize) {
                Log.w(TAG,"read length could be wrong?");
            }
            if (readSize < 0) {
                Log.e(TAG,"read whole stream failed");
            }
            return buffer;
        } catch (IOException ex) {
            Log.e(TAG,"Failed to open mpo stream "+uri);
            return null;
        }
    }

    private static native MpoDecoder decodeByteArray(byte[] data, int offset,
            int length);
}
