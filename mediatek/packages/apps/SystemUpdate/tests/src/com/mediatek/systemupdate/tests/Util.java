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

package com.mediatek.systemupdate.tests;

import android.app.AlarmManager;
import android.app.Instrumentation;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.StatFs;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.storage.IMountService;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Calendar;

final class Util {
    public static final String TAG = "SystemUpdate/TestUtil";

    private static String sAvailablePath;

    private static final String EXTERNAL_USB_STORAGE = "usbotg";

    private static String sExternalSdCardPath = null;

    private static IMountService mMountService;

    private Util() {
    }

    static class OTAresult {
        static final int ERROR_RUN = -1;
        static final int CHECK_OK = 0;
        static final int OTA_FILE_UNZIP_OK = 30;
        static final int ERROR_INVALID_ARGS = 1;
        static final int ERROR_OTA_FILE = 2;
        static final int ERROR_FILE_OPEN = 3;
        static final int ERROR_FILE_WRITE = 4;
        static final int ERROR_OUT_OF_MEMORY = 5;
        static final int ERROR_PARTITION_SETTING = 6;
        static final int ERROR_ONLY_FULL_CHANGE_SIZE = 7;
        static final int ERROR_ACCESS_SD = 8;
        static final int ERROR_ACCESS_USERDATA = 9;
        static final int ERROR_SD_FREE_SPACE = 10;
        static final int ERROR_SD_WRITE_PROTECTED = 11;
        static final int ERROR_USERDATA_FREE_SPACE = 12;
        static final int ERROR_MATCH_DEVICE = 13;
        static final int ERROR_DIFFERENTIAL_VERSION = 14;
        static final int ERROR_BUILD_PROP = 15;
    }
    static class PathName {
        public static final String DATA_PATH = "data/data/com.mediatek.systemupdate/";
        public static final String INTERNAL_ADDRESS_FILE = "/system/etc/system_update/address.xml";
        public static final String PKG_INFO_IN_DATA = DATA_PATH + "pkgInfos.xml";
        public static final String UPDATE_TYPE_IN_DATA = DATA_PATH + "type.txt";
        public static final String OTA_PKG_FOLDER = "/system_update";
        public static final String TEMP_DIR = "/temp";
        public static final String PACKAGE_NAME = "/update.zip";
        public static final String TEMP_PKG_NAME = "/package.zip";
        public static final String MD5_FILE_NAME = "/md5sum";
        public static final String ENTRY_TYPE = "type.txt";
        public static final String ENTRY_SCATTER = "scatter.txt";
        public static final String ENTRY_CONFIGURE = "configure.xml";
    }

    public static class Action {
        public static final String ACTION_BOOT_COMPLETED = Intent.ACTION_BOOT_COMPLETED;
        public static final String ACTION_AUTO_QUERY_NEWVERSION = "com.mediatek.systemupdate.AUTO_QUERY_NEWVERSION";
        public static final String ACTION_AUTO_DOWNLOAD = "com.mediatek.systemupdate.ACTION_AUTO_DOWNLOAD";
        public static final String ACTION_UPDATE_REMIND = "com.mediatek.systemupdate.UPDATE_REMIND";
        public static final String ACTION_UPDATE_REPORT = "com.mediatek.systemupdate.UPDATE_REPORT";
        public static final String ACTION_REPORT_ACTIVITY = "com.mediatek.systemupdate.UpdateReport";
        public static final String ACTION_REFRESH_TIME_OUT = "com.mediatek.systemupdate.REFRESH_TIME_OUT";
        public static final String ACTION_AUTO_DL_TIME_OUT = "com.mediatek.systemupdate.AUTO_DL_TIME_OUT";
        public static final String ACTION_UPDATE_TYPE_OPTION = "com.mediatek.systemupdate.UPDATE_TYPE_OPTION";
        public static final String ACTION_MEDIA_MOUNT_UPDATEUI = "com.mediatek.systemupdate.ACTION_MEDIA_MOUNTED";
        public static final String ACTION_MEDIA_UNMOUNT_UPDATEUI = "com.mediatek.systemupdate.ACTION_MEDIA_UNMOUNTED";
        public static final String ACTION_MEDIA_UNMOUNT = Intent.ACTION_MEDIA_UNMOUNTED;
        public static final String ACTION_MEDIA_BAD_REMOVAL = Intent.ACTION_MEDIA_BAD_REMOVAL;
        public static final String ACTION_MEDIA_NOFS = Intent.ACTION_MEDIA_NOFS;
        public static final String ACTION_MEDIA_MOUNTED = Intent.ACTION_MEDIA_MOUNTED;
        public static final String ACTION_OTA_MANAGER = "com.mediatek.systemupdate.OtaPkgClient";
        public static final String ACTION_INSTALL_REMINDER = "com.mediatek.systemupdate.InstallReminder";
        public static final String ACTION_SYSTEM_UPDATE_ENTRY = "com.mediatek.intent.System_Update_Entry";
        public static final String ACTION_SHUTDOWN = Intent.ACTION_SHUTDOWN;
    }

    static boolean isSdcardAvailable(Context context) {

        if (FeatureOption.MTK_2SDCARD_SWAP || FeatureOption.MTK_SHARED_SDCARD) {

            sExternalSdCardPath = getExternalSDCardPath(context);

            Xlog.i(TAG, "sExternalSdCardPath = " + sExternalSdCardPath);

            if (sExternalSdCardPath != null) {
                StorageManager storManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);

                if (storManager == null) {
                    return false;
                }

                return Environment.MEDIA_MOUNTED.equals(storManager.getVolumeState(sExternalSdCardPath));

            } else {

                return false;

            }

        }

        return Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState());

    }

    static String getExternalSDCardPath(Context context) {

        StorageManager storManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);

        if (storManager == null) {
            return null;
        }

        StorageVolume[] volumes = storManager.getVolumeList();

        if (volumes == null) {
            return null;
        }

        for (int i = 0; i < volumes.length; i++) {
            if ((volumes[i] != null) && (volumes[i].isRemovable())) {

                String path = volumes[i].getPath();
                if ((path != null) && (!path.contains(EXTERNAL_USB_STORAGE))) {
                    return path;
                }

            }
        }

        return null;

    }

    static String getAvailablePath(Context context) {
        return "/storage/sdcard0";
//        if (sAvailablePath == null) {
//            if (FeatureOption.MTK_2SDCARD_SWAP || FeatureOption.MTK_SHARED_SDCARD) {
//                if (sExternalSdCardPath == null) {
//                    getExternalSDCardPath(context);
//                }
//                sAvailablePath = sExternalSdCardPath;
//            } else {
//                File sdcardSystem = Environment.getExternalStorageDirectory();
//                sAvailablePath = sdcardSystem.getPath();
//            }
//        }
//        return sAvailablePath;
    }

    static boolean isNetWorkAvailable(Context context, String typeName) {

        Xlog.i(TAG, "isNetWorkAvailable: context = " + context + "typeName = " + typeName);

        boolean ret = false;

        ConnectivityManager connetManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connetManager == null) {
            Xlog.e(TAG, "isNetWorkAvailable connetManager = null");
            return ret;
        }
        NetworkInfo[] infos = connetManager.getAllNetworkInfo();
        if (infos == null) {
            return ret;
        }
        if ((typeName == null) || (typeName.length() <= 0)) {
            for (int i = 0; i < infos.length && infos[i] != null; i++) {
                if (infos[i].isConnected() && infos[i].isAvailable()) {
                    ret = true;
                    break;
                }
            }
        } else {
            for (int i = 0; i < infos.length && infos[i] != null; i++) {
                if (infos[i].getTypeName().equalsIgnoreCase(typeName) && infos[i].isConnected() && infos[i].isAvailable()) {
                    Xlog.i(TAG, "isNetWorkAvailable name is : " + infos[i].getTypeName());
                    ret = true;
                    break;
                }
            }
        }

        Xlog.i(TAG, "isNetWorkAvailable result is : " + ret);
        return ret;
    }

    static String getPackagePathName(Context context) {

        String packagePath = getAvailablePath(context) + PathName.OTA_PKG_FOLDER;
        File dir = new File(packagePath);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        // if (FeatureOption.MTK_2SDCARD_SWAP ||
        // FeatureOption.MTK_SHARED_SDCARD) {
        // if (sExternalSdCardPath == null) {
        // getExternalSDCardPath(context);
        //
        // }
        // return sExternalSdCardPath + DirAndFiles.OTA_PKG_FOLDER;
        //
        // } else {
        // File sdcardSystem = Environment.getExternalStorageDirectory();
        //
        // if (sdcardSystem != null) {
        // return sdcardSystem.getPath() + DirAndFiles.OTA_PKG_FOLDER;
        // } else {
        // return null;
        // }
        //
        // }
        return packagePath;
    }

    public static String getPackageFileName(Context context) {
        String path = getPackagePathName(context) + PathName.PACKAGE_NAME;
        Xlog.i(TAG, "getPackageFileName = " + path);
        return path;
    }

    void setUpdateTypeFromEM(String type) {

        File f = new File(PathName.UPDATE_TYPE_IN_DATA);
        try {
            FileOutputStream outputStream = new FileOutputStream(f);
            outputStream.write(type.getBytes());
            outputStream.getFD().sync();
            outputStream.close();
        } catch (java.io.FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return;
    }

    static void setAlarm(Context context, int alarmType, long time, String action) {
        Xlog.i(TAG, "setAlarm enter, time = " + time + ", current time = " + Calendar.getInstance().getTimeInMillis());
        Intent it = new Intent(action);
        AlarmManager alarmMgr = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent operation = PendingIntent.getBroadcast(context, 0, it, PendingIntent.FLAG_CANCEL_CURRENT);

        if ((alarmMgr != null) && (operation != null)) {
            alarmMgr.cancel(operation);
            alarmMgr.set(alarmType, time, operation);
        }

    }

    static void cancelAlarm(Context context, String action) {
        Xlog.i(TAG, "cancelAlarm enter, action = " + action + ", current time = "
                + Calendar.getInstance().getTimeInMillis());
        Intent it = new Intent(action);
        AlarmManager alarmMgr = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent operation = PendingIntent.getBroadcast(context, 0, it, PendingIntent.FLAG_CANCEL_CURRENT);

        if ((alarmMgr != null) && (operation != null)) {
            alarmMgr.cancel(operation);
        }

    }

    static String getUpdateTypeFromEM() {

        String type = "";
        File f = new File(PathName.UPDATE_TYPE_IN_DATA);
        if (!f.exists()) {
            Xlog.v(TAG, PathName.UPDATE_TYPE_IN_DATA + "doesn's exist");
            return "";
        }

        try {
            FileReader reader = new FileReader(PathName.UPDATE_TYPE_IN_DATA);
            BufferedReader bfReader = new BufferedReader(reader);

            type = bfReader.readLine();

            Xlog.v(TAG, "Get Type from EM is " + type);

            reader.close();
            bfReader.close();

        } catch (IOException e) {
            e.printStackTrace();
        }

        return type;
    }

    private synchronized static IMountService getMountService() {
        if (mMountService == null) {
            IBinder service = ServiceManager.getService("mount");
            if (service != null) {
                mMountService = IMountService.Stub.asInterface(service);
            } else {
                Xlog.e(TAG, "Can't get mount service");
            }
        }
        return mMountService;
    }

    static void unmountSdCard(Instrumentation instr, final String mountPoint) {
        final IMountService mountService = getMountService();
        Thread unmountThread = new Thread() {
            @Override
            public void run() {
                Xlog.d(TAG, "unmountVolume:" + mountPoint);
                try {
                    mountService.unmountVolume(mountPoint, true, false);
                } catch (RemoteException e) {
                    // Informative dialog to user that unmount failed.
                    Xlog.e(TAG, "Unmount, RemoteException");
                }
            }
        };
        instr.waitForIdle(unmountThread);
        unmountThread = null;

        try {
            Xlog.v(TAG, "unmountVolume:sleep 15000: " + Thread.currentThread().getName());
            Thread.sleep(15000);
        } catch (Exception e) {
            Xlog.e(TAG, "Thread sleep 15000, broken");
        }
    }

    static void mountSdCard(Instrumentation instr, final String mountPoint) {
        final IMountService mountService = getMountService();
        Thread mountThread = new Thread() {
            @Override
            public void run() {
                Xlog.d(TAG, "mountVolume" + mountPoint);
                try {
                    mountService.mountVolume(mountPoint);
                } catch (RemoteException e) {
                    // Informative dialog to user that unmount failed.
                    Xlog.e(TAG, "Mount, RemoteException");
                }
            }
        };
        instr.waitForIdle(mountThread);
        mountThread = null;

        try {
            Xlog.v(TAG, "mountVolume:sleep 12000: " + Thread.currentThread().getName());
            Thread.sleep(12000);
        } catch (Exception e) {
            Xlog.e(TAG, "Thread sleep 12000, broken");
        }
    }
}
