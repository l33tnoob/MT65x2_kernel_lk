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

package com.mediatek.videoplayer;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Debug.MemoryInfo;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.provider.MediaStore;
import android.view.Window;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.drm.OmaDrmClient;
import com.mediatek.drm.OmaDrmUiUtils;
import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Date;

public class MtkUtils {
    private static final String TAG = "MtkUtils";
    private static final String RTSP = "rtsp";
    private static final String HTTP = "http";
    private static final boolean IS_MTK_DRM_SUPPORT = FeatureOption.MTK_DRM_APP;
    private static final boolean IS_MTK_3D_SUPPORT = FeatureOption.MTK_S3D_SUPPORT;
    
    private MtkUtils() {}
    
    public static boolean isRtspStreaming(final Uri uri) {
        boolean rtsp = false;
        if (uri != null && RTSP.equalsIgnoreCase(uri.getScheme())) {
            rtsp = true;
        }
        MtkLog.v(TAG, "isRtspStreaming(" + uri + ") return " + rtsp);
        return rtsp;
    }
    
    public static boolean isHttpStreaming(final Uri uri) {
        boolean http = (uri != null && HTTP.equalsIgnoreCase(uri.getScheme()));
        if (uri != null && HTTP.equalsIgnoreCase(uri.getScheme())) {
            http = true;
        }
        MtkLog.v(TAG, "isHttpStreaming(" + uri + ") return " + http);
        return http;
    }
    
    public static boolean isLocalFile(final Uri uri) {
        final boolean local = (!isRtspStreaming(uri) && !isHttpStreaming(uri));
        MtkLog.v(TAG, "isLocalFile(" + uri + ") return " + local);
        return local;
    }
    
    //for drm
    private static OmaDrmClient sDrmClient;
    public static OmaDrmClient getDrmManager(final Context context) {
        if (sDrmClient == null) {
            sDrmClient = new OmaDrmClient(context);
        }
        return sDrmClient;
    }
    
    public static boolean isSupportDrm() {
        MtkLog.w(TAG, "isSupportDrm() return " + IS_MTK_DRM_SUPPORT);
        return IS_MTK_DRM_SUPPORT;
    }
    
    public static Bitmap overlayDrmIcon(final Context context, final String path, final int action, final Bitmap bkg) {
        final Bitmap bitmap = OmaDrmUiUtils.overlayDrmIcon(getDrmManager(context),context.getResources(),path,action,bkg);
        MtkLog.v(TAG, "overlayDrmIcon(" + path + ") return " + path);
        return bitmap;
    }
    
    public static void showDrmDetails(final Context context, final String path) {
        OmaDrmUiUtils.showProtectionInfoDialog(context,path);
    }
    
    public static boolean isMediaScanning(final Context context) {
        boolean result = false;
        final Cursor cursor = query(context, MediaStore.getMediaScannerUri(), 
                new String [] { MediaStore.MEDIA_SCANNER_VOLUME }, null, null, null);
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                final String scanVolumne = cursor.getString(0);
                result = "external".equals(scanVolumne);
                MtkLog.v(TAG, "isMediaScanning() scanVolumne=" + scanVolumne);
            }
            cursor.close(); 
        } 
        MtkLog.v(TAG, "isMediaScanning() cursor=" + cursor + ", result=" + result);
        return result;
    }
    
    public static Cursor query(final Context context, final Uri uri, final String[] projection,
            final String selection, final String[] selectionArgs, final String sortOrder) {
        return query(context, uri, projection, selection, selectionArgs, sortOrder, 0);
    }
    
    public static Cursor query(final Context context, Uri uri, final String[] projection,
            final String selection, final String[] selectionArgs, final String sortOrder, final int limit) {
        try {
            final ContentResolver resolver = context.getContentResolver();
            if (resolver == null) {
                return null;
            }
            if (limit > 0) {
                uri = uri.buildUpon().appendQueryParameter("limit", "" + limit).build();
            }
            return resolver.query(uri, projection, selection, selectionArgs, sortOrder);
         } catch (final UnsupportedOperationException ex) {
            return null;
        }
        
    }
    
    public static void enableSpinnerState(final Activity a) {
        MtkLog.v(TAG, "enableSpinnerState(" + a + ")");
        a.getWindow().setFeatureInt(
                Window.FEATURE_PROGRESS,
                Window.PROGRESS_START);
        a.getWindow().setFeatureInt(
                Window.FEATURE_PROGRESS,
                Window.PROGRESS_VISIBILITY_ON);
    }
    
    public static void disableSpinnerState(final Activity a) {
        MtkLog.v(TAG, "disableSpinnerState(" + a + ")");
        a.getWindow().setFeatureInt(
                Window.FEATURE_PROGRESS,
                Window.PROGRESS_END);
        a.getWindow().setFeatureInt(
                Window.FEATURE_PROGRESS,
                Window.PROGRESS_VISIBILITY_OFF);
    }
    
    public static boolean isMediaMounted(final Context context) {
        boolean mounted = false;
        String defaultStoragePath = null;
        String defaultStorageState = null;
        final String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state) ||
                Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            mounted = true;
        } else {
            final StorageManager storageManager = (StorageManager)context.getSystemService(Context.STORAGE_SERVICE);
            if (storageManager != null) {
                defaultStoragePath = StorageManagerEx.getDefaultPath();
                defaultStorageState = storageManager.getVolumeState(defaultStoragePath);
                if (Environment.MEDIA_MOUNTED.equals(defaultStorageState) ||
                        Environment.MEDIA_MOUNTED_READ_ONLY.equals(defaultStorageState)) {
                    mounted = true;
                }
            }
        }
        MtkLog.v(TAG, "isMediaMounted() return " + mounted + ", state=" + state
                + ", defaultStoragePath=" + defaultStoragePath + ", defaultStorageState=" + defaultStorageState);
        return mounted;
    }
    
    public static String stringForTime(final long millis) {
        final int totalSeconds = (int) millis / 1000;
        final int seconds = totalSeconds % 60;
        final int minutes = (totalSeconds / 60) % 60;
        final int hours = totalSeconds / 3600;
        if (hours > 0) {
            return String.format("%d:%02d:%02d", hours, minutes, seconds);
        } else {
            return String.format("%02d:%02d", minutes, seconds);
        }
    }
    
    private static Date sDate = new Date();//cause lots of CPU
    public static String localTime(final long millis) {
        sDate.setTime(millis);
        return sDate.toLocaleString();
    }
    
    public static void logMemory(final String title) {
        final MemoryInfo mi = new MemoryInfo();
        android.os.Debug.getMemoryInfo(mi);
        final String tagtitle = "logMemory() " + title;
        MtkLog.v(TAG, tagtitle + "         PrivateDirty    Pss     SharedDirty");
        MtkLog.v(TAG, tagtitle + " dalvik: " + mi.dalvikPrivateDirty + ", " + mi.dalvikPss
                + ", " + mi.dalvikSharedDirty + ".");
        MtkLog.v(TAG, tagtitle + " native: " + mi.nativePrivateDirty + ", " + mi.nativePss
                + ", " + mi.nativeSharedDirty + ".");
        MtkLog.v(TAG, tagtitle + " other: " + mi.otherPrivateDirty + ", " + mi.otherPss
                + ", " + mi.otherSharedDirty + ".");
        MtkLog.v(TAG, tagtitle + " total: " + mi.getTotalPrivateDirty() + ", " + mi.getTotalPss()
                + ", " + mi.getTotalSharedDirty() + ".");
    }
    
    /*
    public static void saveBitmap(final String tag, final String msg, final Bitmap bitmap) {
        if (bitmap == null) {
            MtkLog.v(tag, "[" + msg + "] bitmap=null");
        }
        final long now = System.currentTimeMillis();
        final String fileName = "/mnt/sdcard/nomedia/" + now + ".jpg";
        final File temp = new File(fileName);
        final File dir = temp.getParentFile();
        if (!dir.exists()) { //create debug folder
            dir.mkdir();
        }
        final File nomedia = new File("/mnt/sdcard/nomedia/.nomedia");
        if (!nomedia.exists()) { //add .nomedia file
            try {
                nomedia.createNewFile();
            } catch (final IOException e) {
                e.printStackTrace();
            }
        }
        FileOutputStream os = null;
        try {
            os = new FileOutputStream(temp);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, os);
            os.close();
        } catch (final IOException ex1) {
            ex1.printStackTrace();
        } catch (final SecurityException ex2) {
            ex2.printStackTrace();
        } finally  {
            if (os != null) {
                try {
                    os.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        MtkLog.v(tag, "[" + msg + "] write file filename=" + fileName);
    }
    */
    
    public static boolean isSupport3d() {
        MtkLog.w(TAG, "isSupport3d() return " + IS_MTK_3D_SUPPORT);
        return IS_MTK_3D_SUPPORT;
    }
    
    public static boolean isStereo3D(final int stereoType) {
        // These defined in MediaStore, here to avoid build error on some branch not define the value
        final int STEREO_TYPE_2D = 0; // Not use now
        final int STEREO_TYPE_FRAME_SEQUENCE = 1; // MediaStore.Video.Media.STEREO_TYPE_FRAME_SEQUENCE
        final int STEREO_TYPE_SIDE_BY_SIDE = 2; // MediaStore.Video.Media.STEREO_TYPE_SIDE_BY_SIDE
        final int STEREO_TYPE_TOP_BOTTOM = 3; // MediaStore.Video.Media.STEREO_TYPE_TOP_BOTTOM
        final int STEREO_TYPE_SWAP_LEFT_RIGHT = 4; // MediaStore.Video.Media.STEREO_TYPE_SWAP_LEFT_RIGHT
        final int STEREO_TYPE_SWAP_TOP_BOTTOM = 5; // MediaStore.Video.Media.STEREO_TYPE_SWAP_TOP_BOTTOM
        final int STEREO_TYPE_UNKNOWN = -1;
        boolean stereo3d = false;
        //Only S3D support has all of these values
        if (stereoType ==  STEREO_TYPE_FRAME_SEQUENCE ||
                stereoType ==  STEREO_TYPE_SIDE_BY_SIDE || 
                stereoType ==  STEREO_TYPE_TOP_BOTTOM ||
                stereoType ==  STEREO_TYPE_SWAP_LEFT_RIGHT ||
                stereoType ==  STEREO_TYPE_SWAP_TOP_BOTTOM ) {
            stereo3d = true;
        }
        MtkLog.v(TAG, "isStereo3D(" + stereoType + ") return " + stereo3d);
        return stereo3d;
    }
}
