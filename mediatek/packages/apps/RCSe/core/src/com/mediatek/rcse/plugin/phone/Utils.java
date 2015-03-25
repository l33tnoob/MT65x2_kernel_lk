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

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;
import android.hardware.Camera;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.StatFs;
import android.os.Vibrator;
import android.telephony.TelephonyManager;
import android.util.TypedValue;
import android.view.Surface;

import com.mediatek.rcse.api.Logger;

import java.io.File;

/**
 * A helper class which used to provide some common useful function.
 */
public class Utils {

    private static final String TAG = "Utils";

    /** Network type is unknown */
    public static final int NETWORK_TYPE_UNKNOWN = -1;
    /** Current network is WIFI */
    public static final int NETWORK_TYPE_WIFI = 0;
    /** Current network is GPRS */
    public static final int NETWORK_TYPE_GPRS = 1;
    /** Current network is EDGE */
    public static final int NETWORK_TYPE_EDGE = 2;
    /** Current network is UMTS */
    public static final int NETWORK_TYPE_UMTS = 3;
    /** Current network is CDMA: Either IS95A or IS95B */
    public static final int NETWORK_TYPE_CDMA = 4;
    /** Current network is EVDO revision 0 */
    public static final int NETWORK_TYPE_EVDO_0 = 5;
    /** Current network is EVDO revision A */
    public static final int NETWORK_TYPE_EVDO_A = 6;
    /** Current network is 1xRTT */
    public static final int NETWORK_TYPE_1XRTT = 7;
    /** Current network is HSDPA */
    public static final int NETWORK_TYPE_HSDPA = 8;
    /** Current network is HSUPA */
    public static final int NETWORK_TYPE_HSUPA = 9;
    /** Current network is HSPA */
    public static final int NETWORK_TYPE_HSPA = 10;
    /** Current network is iDen */
    public static final int NETWORK_TYPE_IDEN = 11;
    /** Current network is EVDO revision B */
    public static final int NETWORK_TYPE_EVDO_B = 12;
    /** Current network is LTE */
    public static final int NETWORK_TYPE_LTE = 13;
    /** Current network is eHRPD */
    public static final int NETWORK_TYPE_EHRPD = 14;
    /** Current network is HSPA+ */
    public static final int NETWORK_TYPE_HSPAP = 15;
    /** Current network is GSM */
    public static final int NETWORK_TYPE_GSM = 16;
    /** Current network is UMB */
    public static final int NETWORK_TYPE_UMB = 17;
    
    private static boolean sIsInImageSharing = false;
    private static boolean sIsInVideoSharing = false;
    public static final long MIN_VIBRATING_TIME = 500;
    public static final long MAX_VIBRATING_TIME = 2 * 1000;
    private static Vibrator sVibratorService = null;
    private static boolean sIsVibrated = false;
    private static final Object VIRATOR_LOCK = new Object();

    public static boolean isInImageSharing() {
        Logger.d(TAG, "isInImageSharing: " + sIsInImageSharing);
        return sIsInImageSharing;
    }

    public static boolean isInVideoSharing() {
        Logger.d(TAG, "isInVideoSharing: " + sIsInVideoSharing);
        return sIsInVideoSharing;
    }

    public static void setInImageSharing(boolean isInImageSharing) {
        sIsInImageSharing = isInImageSharing;
        Logger.d(TAG, "setInImageSharing: " + sIsInImageSharing);
    }

    public static void setInVideoSharing(boolean isInVideoSharing) {
        sIsInVideoSharing = isInVideoSharing;
        Logger.d(TAG, "setInVideoSharing: " + sIsInVideoSharing);
    }

    /**
     * Get current network type.
     * 
     * @param context A {@link Context } used to get system service.
     * @return current network type.
     */
    public static int getNetworkType(Context context) {
        Logger.w(TAG, "getNetworkType(), context = " + context);
        if (context == null) {
            return NETWORK_TYPE_UNKNOWN;
        }
        ConnectivityManager connectivityManager =
                (ConnectivityManager) context.getSystemService(ContextWrapper.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
        if (networkInfo == null) {
            Logger.w(TAG, "networkInfo is null");
            return NETWORK_TYPE_UNKNOWN;
        }
        int type = networkInfo.getType();
        Logger.v(TAG, "type = " + type);
        if (type == ConnectivityManager.TYPE_WIFI) {
            return NETWORK_TYPE_WIFI;
        } else {
            // In mobile network
            TelephonyManager telephonyManager =
                    (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            int subType = telephonyManager.getNetworkType();
            Logger.v(TAG, "subType = " + subType);
            if (subType != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                return subType;
            } else {
                return NETWORK_TYPE_UNKNOWN;
            }
        }
    }

    /**
     * Get the number of camera on the device.
     * 
     * @return The number of camera on the device.
     */
    public static int getCameraNums() {
        int camerNumber = Camera.getNumberOfCameras();
        Logger.v(TAG, "getCameraNums(),camerNumber = " + camerNumber);
        return camerNumber;
    }

    /**
     * Convert dip to pix
     * 
     * @param context A context
     * @param dip To be converted
     * @return The Related pix
     */
    public static int dip2px(Context context, int dip) {
        Logger.v(TAG, "dip2px(), dip = " + dip);
        int px = -1;
        if (context == null) {
            Logger.e(TAG, "context is null");
            return px;
        }
        Resources resources = context.getResources();
        px =
                (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dip, resources
                        .getDisplayMetrics());
        Logger.v(TAG, "px = " + px);
        return px;
    }

    /**
     * Rotate camera preview if necessary.
     * 
     * @param activity A activity
     * @param cameraId The camera id
     * @param camera The Camera object
     */
    public static void setCameraDisplayOrientation(Activity activity, int cameraId, Camera camera) {
        Logger.v(TAG, "setCameraDisplayOrientation(), activity = " + activity + ", cameraId = "
                + cameraId + ", camera = " + camera);
        if (activity == null || camera == null) {
            Logger.w(TAG, "Do not rotate");
            return;
        }
        Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
            default:
                break;
        }
        Logger.w(TAG, "degrees = " + degrees);
        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            Logger.d(TAG, "Front camera info.orientation = " + info.orientation);
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            Logger.d(TAG, "Back camera info.orientation = " + info.orientation);
            result = (info.orientation - degrees + 360) % 360;
        }
        Logger.v(TAG, "setDisplayOrientation degrees = " + result);
        camera.setDisplayOrientation(result);
    }

    /**
     * Rotate camera if necessary.
     * 
     * @param activity A activity
     * @param cameraId The camera id
     * @param camera The Camera object
     * @return The degrees of the data from the camera preview
     */
    public static int getRotationDegrees(Activity activity, int cameraId, Camera camera) {
        Logger.v(TAG, "getRotationDegrees(), activity = " + activity + ", cameraId = " + cameraId
                + ", camera = " + camera);
        if (activity == null || camera == null) {
            Logger.w(TAG, "getRotationDegrees(),Do not rotate");
            return 0;
        }
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
            default:
                break;
        }
        Logger.w(TAG, "getRotationDegrees(), degrees = " + degrees);
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            Logger.d(TAG, "getRotationDegrees(),front camera info.orientation = "
                    + info.orientation);
            rotation = (info.orientation - degrees + 360) % 360;
        } else { // back-facing
            Logger.d(TAG, "getRotationDegrees(), back camera info.orientation = "
                    + info.orientation);
            rotation = (info.orientation + degrees) % 360;
        }
        Logger.v(TAG, "getRotationDegrees(), return rotation = " + rotation);
        return rotation;
    }
    
    /**
     * Turn the vibrator on.
     * 
     * @param context The context.
     * @param time The number of milliseconds to vibrate.
     */
    public static boolean vibrate(Context context, long time) {
        Logger.e(TAG, "vibrate(), context = " + context + ", time = " + time);
        if (context == null || time < 0) {
            Logger.e(TAG, "vibrate(), context is null or time < 0");
            sIsVibrated = false;
            return false;
        }
        synchronized (VIRATOR_LOCK) {
            if (sVibratorService == null) {
                Logger.d(TAG, "sVibratorService is null then create it.");
                sVibratorService = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
            }
        }
        sVibratorService.vibrate(time);
        sIsVibrated = true;
        return true;
    }

    /**
     * Get the vibrated status
     * 
     * @return The vibrated status
     */
    public static boolean isVibrated() {
        return sIsVibrated;
    }

    /**
     * Reset the vibrated status.
     */
    public static void resetVibratedStatus() {
        sIsVibrated = false;
    }

    /**
     * Get the current available storage size in byte;
     * 
     * @return available storage size in byte; -1 for no external storage
     *         detected
     */
    public static long getFreeStorageSize() {
        boolean isExist = Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
        if (isExist) {
            File path = Environment.getExternalStorageDirectory();
            StatFs stat = new StatFs(path.getPath());
            int availableBlocks = stat.getAvailableBlocks();
            int blockSize = stat.getBlockSize();
            long result = (long) availableBlocks * blockSize;
            Logger.d(TAG, "getFreeStorageSize() blockSize: " + blockSize + " availableBlocks: "
                    + availableBlocks + " result: " + result);
            return result;
        }
        return -1;
    }
}
