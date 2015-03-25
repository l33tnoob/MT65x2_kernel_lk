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

package com.mediatek.lowstorage;
 
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import android.util.Slog;
import android.util.Log;
import android.os.StatFs; 
import android.os.Environment;
import android.os.Looper;
import android.os.Message;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ActivityNotFoundException;
import android.os.Handler;
import android.os.SystemProperties;

import java.io.File;
import java.io.OutputStream;
import java.io.IOException;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import com.mediatek.common.lowstorage.ILowStorageHandle;

public final class LowStorageHandle implements ILowStorageHandle {

    static final String TAG = "LowStorageHandle";
    private static LowStorageHandle sInstance;
    private Context mContext = null;
    /**
     * The Low Memory Flag, to record the low storage memory status.
     */

    public final static String PIGGYBANK_PATH = "/data/piggybank";

    public final static long PIGGYBANK_MAX_KB_SIZE = 4096;
    public final static long LSM_THRESHOLD_WARN = 4096;
    public final static long LSM_THRESHOLD_LOWMEM = 1536;
    public final static long LSM_THRESHOLD_FORCE_CLEAN = 0;
    public final static boolean FORCE_CLEAN_ENABLE = false;
    public final String DEL_FILENAME_PATTERN = "^core\\.[0-9]*";

    private void LSMRemoveCoredump() {
        Log.d(TAG, "remove system core dump file to save storge memory");
        String root_path = Environment.getDataDirectory().getPath() + "/core";

        File path = new File(root_path);
        File f_remove;
        if (path.list() != null) {
            for (String filename : path.list()) {
                if (filename.matches(DEL_FILENAME_PATTERN)) {
                    // find core file, we need to delete it to save storage
                    // space]
                    f_remove = new File(root_path + "/" + filename);
                    if (f_remove.exists()) {
                        Log.d(TAG, "find and remove system core dump file: " + filename + ";free :"
                                + f_remove.length());
                        f_remove.delete();
                    }
                }
            }
        }
    }

    public LowStorageHandle(Context context) {
        mContext = context;
    }

    public void registerFilter() {
        Slog.d(TAG, "register filter");

        if (mContext != null) {

            Slog.d(TAG, "register receiver");

            IntentFilter lsmFilter = new IntentFilter();
            lsmFilter.addAction(Intent.ACTION_DEVICE_STORAGE_LOW);
            lsmFilter.addAction(Intent.ACTION_DEVICE_STORAGE_FULL);
            lsmFilter.addAction(Intent.ACTION_DEVICE_STORAGE_NOT_FULL);
            lsmFilter.addAction(Intent.ACTION_DEVICE_STORAGE_OK);
            mContext.registerReceiver(new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    if (Intent.ACTION_DEVICE_STORAGE_LOW.equals(intent.getAction())) {
                        long freeKBStorage = 0;
                        StatFs dataFileStats = new StatFs(Environment.getDataDirectory().getPath());
                        Slog.e(TAG, "receive the storage low intent");
                        try {
                            dataFileStats.restat(Environment.getDataDirectory().getPath());
                            freeKBStorage = ((long) dataFileStats.getAvailableBlocks()
                                    * dataFileStats.getBlockSize() / 1024);
                        } catch (IllegalArgumentException e) {
                        }
                        if (freeKBStorage < LSM_THRESHOLD_LOWMEM) {
                            SystemProperties.set("sys.lowstorage_flag", "1");
                        }
                    } else if (Intent.ACTION_DEVICE_STORAGE_FULL.equals(intent.getAction())) {
                        Slog.d(TAG, "get storage full intent ");
                        SystemProperties.set("sys.lowstorage_flag", "1");
                    } else if (Intent.ACTION_DEVICE_STORAGE_NOT_FULL.equals(intent.getAction())) {
                        Slog.e(TAG, "get storage not full intent");
                        SystemProperties.set("sys.lowstorage_flag", "0");
                    } else if (Intent.ACTION_DEVICE_STORAGE_OK.equals(intent.getAction())) {
                        Slog.e(TAG, "receive the storage ok intent");
                        SystemProperties.set("sys.lowstorage_flag", "0");
                    }
                }
            }, lsmFilter);
        } else {
            Slog.e(TAG, "mContext is null");
        }

    }

    public boolean GetCurrentFlag() {
        return (SystemProperties.get("sys.lowstorage_flag", "0").equals("1"));
    }

    // static public void systemReadyLSM(Handler mHandler)
    public void systemReadyLSM() {
        StatFs dataFileStats;
        long freeKBStorage = 0;

        Log.d(TAG, " LSM_THRESHOLD_FORCE_CLEAN : " + LSM_THRESHOLD_FORCE_CLEAN +
                "; LSM_THRESHOLD_LOWMEM: " + LSM_THRESHOLD_LOWMEM + ";LSM_THRESHOLD_WARN :"
                + LSM_THRESHOLD_WARN);

        Log.d(TAG, " FORCE_CLEAN_ENABLE : " + FORCE_CLEAN_ENABLE);

        dataFileStats = new StatFs(Environment.getDataDirectory().getPath());
        try {
            dataFileStats.restat(Environment.getDataDirectory().getPath());
            freeKBStorage = ((long) dataFileStats.getAvailableBlocks()
                    * dataFileStats.getBlockSize() / 1024);
        } catch (IllegalArgumentException e) {

        }
        Log.d(TAG, "data.free.before KB: " + Long.toString(freeKBStorage));

        // the follow handle piggy bank file
        long piggyKBSize = PIGGYBANK_MAX_KB_SIZE;
        if ((freeKBStorage <= 2 * PIGGYBANK_MAX_KB_SIZE) && (freeKBStorage > PIGGYBANK_MAX_KB_SIZE)) {
            // use PIGGYBNK_MAX_BK_SIZE
            LSMRemoveCoredump();
        } else if ((freeKBStorage <= PIGGYBANK_MAX_KB_SIZE)
                && (freeKBStorage >= (PIGGYBANK_MAX_KB_SIZE / 2))) {
            piggyKBSize = (PIGGYBANK_MAX_KB_SIZE / 2);
            LSMRemoveCoredump();
        } else if (freeKBStorage < (PIGGYBANK_MAX_KB_SIZE / 2)) {
            // freeKBStorage < 1.5 M
            // we are in very danger case
            // we should procee auto clear procedure
            piggyKBSize = (long) (freeKBStorage * 0.8);
            LSMRemoveCoredump();
        }
        Log.d(TAG, "systemReady : want to create piggybank KB:" + Long.toString(piggyKBSize));

        File f = new File(PIGGYBANK_PATH);
        if (!f.exists()) {
            try {
                OutputStream out = new FileOutputStream(f);
                try {
                    byte[] buffer = new byte[2048];
                    int dataWrite = 0;
                    while (dataWrite < (piggyKBSize / 2)) {
                        out.write(buffer, 0, 2048);
                        dataWrite++;
                    }
                } finally {
                    out.close();
                }
            } catch (IOException e) {
                // f.delete();
                Log.d(TAG, " Can't create piggybank" + e);
            }
        }

        try {
            dataFileStats.restat(Environment.getDataDirectory().getPath());
            freeKBStorage = ((long) dataFileStats.getAvailableBlocks() * dataFileStats
                    .getBlockSize()) / 1024;
        } catch (IllegalArgumentException e) {
        }
        Log.d(TAG, " data.free.after KB: " + Long.toString(freeKBStorage));

        // we will end boot up notify when the free memroy lower than the
        // threshold -- 5 %
        /*
         * if(freeKBStorage < LSM_THRESHOLD_WARN) { // send memory low message
         * to show dialog Message msg = Message.obtain(); msg.what =
         * SHOW_MEM_LOW_MSG; mHandler.sendMessage(msg); }
         */
    }

    // M}
}
