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

package com.android.bluetooth.pbap;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class BluetoothPbapWriter {
    private static final String TAG = "BluetoothPbapWriter";

    public static final boolean DEBUG = true;

    String mResultPath;

    FileOutputStream mOutputStream;

    public BluetoothPbapWriter() {
        mResultPath = null;
        mOutputStream = null;
    }

    public boolean init(Context context) {
        if (mOutputStream == null) {
            try {
                File path = null;
                mOutputStream = context
                        .openFileOutput("btpbaptmp.vcf", Context.MODE_WORLD_READABLE);
                path = context.getFileStreamPath("btpbaptmp.vcf");
                if (path != null) {
                    mResultPath = path.getAbsolutePath();
                }
            } catch (FileNotFoundException e) {
                mOutputStream = null;
            }
            /*
             * File dir = context.getFilesDir(); dir.mkdirs(); File file = new
             * File(dir, "btpbaptmp.vcf"); try { if (!file.createNewFile()) { if
             * (!file.exists()) { Log.e(TAG, "Failed to create new File!");
             * return false; } } }catch (IOException e) { return false; }
             * mResultPath = file.getAbsolutePath(); try { mOutputStream = new
             * FileOutputStream(file); } catch (FileNotFoundException e){
             * mOutputStream = null; }
             */
        }
        return (mOutputStream != null);
    }

    public boolean write(String str) {
        try {
            if (mOutputStream != null) {
                mOutputStream.write(str.getBytes("UTF-8"));
            } else {
                Log.e(TAG, "mOutputStream is null when calling write");
                return false;
            }
        } catch (IOException e) {
            Log.e(TAG, "write output stream failed");
            return false;
        }
        return true;
    }

    public String getPath() {
        return mResultPath;
    }

    public void terminate() {
        if (mOutputStream != null) {
            try {
                mOutputStream.close();
            } catch (IOException e) {
                mOutputStream = null;
            }
        }
    }
}
