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

package com.mediatek.bluetooth.pbap;

import android.text.TextUtils;
import android.util.Log;


public class BluetoothPbapPath {
    private static final String TAG = "BluetoothPbapPath";

    private static final boolean DEBUG = true;

    /* Constant definition */
    public static final int PBAP_PATH_FORWARD = 0; // Back to root

    public static final int PBAP_PATH_BACKWARD = 1; // backward one level

    public static final int PBAP_PATH_TO_ROOT = 2; // Forward to specified child
                                                   // folder

    /* type of folder. -1 for unknown path */
    public static final int FOLDER_TYPE_UNKNOWN = -1;

    public static final int FOLDER_TYPE_PB = 0;

    public static final int FOLDER_TYPE_ICH = 1;

    public static final int FOLDER_TYPE_OCH = 2;

    public static final int FOLDER_TYPE_MCH = 3;

    public static final int FOLDER_TYPE_CCH = 4;

    public static final int FOLDER_TYPE_SIM1_PB = 5;

    public static final int FOLDER_TYPE_SIM1_ICH = 6;

    public static final int FOLDER_TYPE_SIM1_OCH = 7;

    public static final int FOLDER_TYPE_SIM1_MCH = 8;

    public static final int FOLDER_TYPE_SIM1_CCH = 9;

    public static final String VALID_PATH[] = {
            "telecom/pb", "telecom/ich", "telecom/och", "telecom/mch", "telecom/cch",
            "SIM1/telecom/pb"
    // ,"SIM1/telecom/ich", "SIM1/telecom/och", "SIM1/telecom/mch",
    // "SIM1/telecom/cch"
    };

    private String mCurrentPath;

    public BluetoothPbapPath() {
        mCurrentPath = ""; // Set to root
    }

    public String getCurrentPath() {
        return mCurrentPath;
    }

    /*
     * if relative is true, get type of the path which is current path concated
     * by path
     */
    /* if relative is false, get type of path which is absolute path */
    public int getPathType(String path, boolean relative) {
        int i;
        int size = VALID_PATH.length;
        // String p = null;

        printLog("getPathType(" + path + "," + String.valueOf(relative) + ")");
        if (path != null) {
            path = formatPath(path);
        }
        printLog("path formatted=" + path);
        if (relative) {
            if (path != null && path.length() > 0) {
                if (!mCurrentPath.isEmpty()) {
                    path = mCurrentPath + "/" + path;
                }
            } else {
                path = mCurrentPath;
            }
        }
        printLog("absolute path is " + path);
        for (i = 0; i < size; i++) {
            if (path.equalsIgnoreCase(VALID_PATH[i])) {
                return i;
            }
        }
        return FOLDER_TYPE_UNKNOWN;
    }

    public boolean setPath(int op, String path) {
        boolean ret = true;
        int idx;
        printLog("[API] setPath(" + op + "," + path + ")");
        switch (op) {
            case PBAP_PATH_TO_ROOT:
                mCurrentPath = "";
                break;
            case PBAP_PATH_BACKWARD:
                idx = mCurrentPath.lastIndexOf("/");
                if (idx <= 0) {
                    // Not found or current path is "/"
                    mCurrentPath = "";
                } else {
                    mCurrentPath = mCurrentPath.substring(0, mCurrentPath.lastIndexOf('/'));
                }
                break;
            case PBAP_PATH_FORWARD:
                path = formatPath(path);
                if (!TextUtils.isEmpty(mCurrentPath)) {
                    path = mCurrentPath + "/" + path;
                }
                printLog("PBAP_PATH_FORWARD : " + path);
                if (isValidPath(path)) {
                    mCurrentPath = path;
                } else {
                    ret = false;
                }
                break;
            default:
                errorLog("Unknown set path operation");
                break;
        }
        return ret;
    }

    private String formatPath(String path) {
        // path = path.replace('\', '/');
        if (path.startsWith("/")) {
            path = path.substring(1, path.length());
        }
        if (path.endsWith("/")) {
            path = path.substring(0, path.length() - 1);
        }
        return path;
    }

    private boolean isValidPath(String path) {
        int i;
        int size = VALID_PATH.length;
        int length = path.length();
        String vp = null;

        path = formatPath(path);
        for (i = 0; i < size; i++) {
            vp = VALID_PATH[i];
            if (length <= vp.length() && path.equalsIgnoreCase(vp.substring(0, length))
                    && (vp.length() == length || vp.charAt(length) == '/')) {
                return true;
            }
        }
        errorLog("[ERR] invalid path : " + path);
        return false;
    }

    /* Util functions */
    private void printLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    private void errorLog(String msg) {
        Log.e(TAG, msg);
    }
}
