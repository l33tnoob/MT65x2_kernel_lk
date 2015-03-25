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
 */
package com.mediatek.videofavorites;

import android.content.Context;
import android.os.Environment;
import android.os.StatFs;
import android.os.storage.StorageManager;

import com.mediatek.xlog.Xlog;

import java.io.File;

public final class Storage {

    private static final String TAG = "Storage";

    public static final long AVAILABLE = 0L;
    public static final long UNAVAILABLE = -1L;
    public static final long PREPARING = -2L;
    public static final long UNKNOWN_SIZE = -3L;
    public static final long FULL_SDCARD = -4L;
    public static final long LOW_STORAGE_THRESHOLD = 524288;

    public static final String TRANSCODE_PATH_BASE =
        Environment.getExternalStorageDirectory().toString();
    public static final String TRANSCODE_PATH =
        TRANSCODE_PATH_BASE + "/VideoFavorite/";

    private Storage() {};

    private static StorageManager getStorageManager(Context c) {
        return (StorageManager) c.getSystemService(Context.STORAGE_SERVICE);
    }

    private static long checkStorageState(String mountPoint, Context context) {
        StorageManager sm = getStorageManager(context);
        if (sm == null) {
            return UNAVAILABLE;
        }

        String state = sm.getVolumeState(mountPoint);
        Xlog.d(TAG, "External storage state=" + state + ", mount point = " + TRANSCODE_PATH_BASE);
        if (Environment.MEDIA_CHECKING.equals(state)) {
            return PREPARING;
        }
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            return UNAVAILABLE;
        }
        return AVAILABLE;
    }

    public static boolean isAvailable(Context context) {
        return checkStorageState(TRANSCODE_PATH_BASE, context) == AVAILABLE;
    }

    public static long getAvailableSpace(String mountPoint, Context context) {

        long storageState = checkStorageState(mountPoint, context);
        if (storageState != AVAILABLE) {
            return storageState;
        }

        File dir = new File(TRANSCODE_PATH);
        if (!dir.mkdirs() && (!dir.isDirectory() || !dir.canWrite())) {
            Xlog.e(TAG, "directory create failed");            
            return FULL_SDCARD;
        }

        try {
            StatFs stat = new StatFs(TRANSCODE_PATH);
            return stat.getAvailableBlocksLong() * stat.getBlockSizeLong();
        } catch (IllegalArgumentException e) {
            Xlog.i(TAG, "Fail to access external storage", e);
        }
        return UNKNOWN_SIZE;
    }

    public static long getAvailableSpace(Context context) {
        return getAvailableSpace(TRANSCODE_PATH_BASE, context);
    }

}
