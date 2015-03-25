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

package com.mediatek.systemupdate;

import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemProperties;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Calendar;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public final class Util {
    public static final String TAG = "SystemUpdate/Util";
    private static final int BUFF_SIZE = 1024;
    private static final String PROC_VERSION_PATH = "/proc/version";
    private static final String NETTYPE_WIFI = "WIFI";
    private static final int FILE_READER_SIZES = 256;
    private static final int PROC_VERSION_GROUP_LENGTH = 4;
    static final long M_SIZE = 1024 * 1024;
    static final long K_SIZE = 1024;
    private static String sAvailablePath;
    
    static final char LEFT_TO_RIGHT_EMBEDDING = '\u202A';
    static final char POP_DIRECTIONAL_FORMATTING = '\u202C';


    public static enum UPDATE_TYPES {
        OTA_UPDATE_ONLY, SDCARD_UPDATE_ONLY, OTA_SDCARD_UPDATE
    }

    public static final UPDATE_TYPES UPDATE_OPTION = UPDATE_TYPES.OTA_UPDATE_ONLY;

    enum SDCARD_STATUS {
        STATE_OK, STATE_LOST, STATE_UNMOUNT, STATE_INSUFFICIENT
    }

    enum NETWORK_STATUS {
        STATE_WIFI, STATE_GPRS, STATE_NONE_NETWORK
    }

    static final int MAX_PERCENT = 100;
    static final long REFRESHTIME = 3 * 3600 * 1000;
    static final double DECOMPRESS_RATIO = 1.5;
    static final String DATE_FORMAT = "yyyy-MM-dd";
    static final String ERROR_VERSION = "Unavailable";
    private static final String EXTERNAL_USB_STORAGE = "usbotg";

    private static String sExternalSdCardPath = null;

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

    static class SDresult {
        public static final int UNPACK_OK = -1;
        public static final int VERIFY_OK = 0;
        public static final int ERROR_OUT_OF_MEMORY = 1;
        public static final int ERROR_INVALID_ZIP = 2;
        public static final int ERROR_CHECKSUM_ERR = 3;
        public static final int ERROR_VERSION_ERR = 4;
        public static final int ERROR_NO_SDCARD = 5;
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
        public static final String ACTION_LCA_PROTECT = "com.mediatek.systemupdate.LCA_PROTECT";
        public static final String ACTION__PRE_BOOT_COMPLETED = "android.intent.action.PRE_BOOT_COMPLETED";
    }

    public static class PathName {
        public static final String DATA_PATH = "data/data/com.mediatek.systemupdate/";
        public static final String INTERNAL_ADDRESS_FILE = "/system/etc/system_update/address.xml";
        public static final String PKG_INFO_IN_DATA = DATA_PATH + "pkgInfos.xml";
        public static final String UPDATE_TYPE_IN_DATA = DATA_PATH + "type.txt";
        public static final String OTA_PKG_FOLDER = "/system_update";
        public static final String TEMP_DIR = "/temp";
        public static final String PACKAGE_NAME = "/update.zip";
        public static final String TEMP_PKG_NAME = "/package.zip";
        public static final String MD5_FILE_NAME = "/md5sum";
        public static final String ENTRY_PACKAGE_NAME = "update.zip";
        public static final String ENTRY_CONFIGURE = "configure.xml";
        public static final String ENTRY_TYPE = "type.txt";
        public static final String ENTRY_SCATTER = "scatter.txt";
        public static final String ENTRY_BUILD_INFO = "META-INF/com/google/android/updater-script";
    }

    public static class InfoXmlTags {
        public static final String XML_TAG_PRODUCT = "product";
        public static final String XML_TAG_FLAVOR = "flavor";
        public static final String XML_TAG_LANGUAGE = "language";
        public static final String XML_TAG_OEM = "oem";
        public static final String XML_TAG_OPERATOR = "operator";
        public static final String XML_TAG_ANDROID_NUM = "androidnumber";
        public static final String XML_TAG_VERSION_NAME = "versionname";
        public static final String XML_TAG_FINGERPRINT = "fingerprint";
        public static final String XML_TAG_PUBLISH_TIME = "publishtime";
        public static final String XML_TAG_NOTES = "notes";
        public static final String XML_TAG_PATH = "path";
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

        Xlog.i(TAG, "start scan and length = " + volumes.length);
        for (int i = 0; i < volumes.length; i++) {
            if ((volumes[i] != null) && (volumes[i].isRemovable())) {

                String path = volumes[i].getPath();
                if ((path != null) && (!path.contains(EXTERNAL_USB_STORAGE))) {
                    Xlog.i(TAG, " path = " + path);
                    return path;
                }

            }
        }

        return null;

    }

    /**
     * Check if the SD card is available and has enough space.
     * 
     * @param context
     *            environment context
     * @param miniSize
     *            the minimized size needed for checking, unit is byte.
     * 
     * @return the SD card status
     * @see SDCARD_STATUS
     * 
     */

    static SDCARD_STATUS checkSdcardState(Context context, long miniSize) {

        if (!isSdcardAvailable(context)) {
            return SDCARD_STATUS.STATE_LOST;
        }

        long insufficientSpace = getExtraSpaceNeeded(context, miniSize);

        if (insufficientSpace == -1) {
            Xlog.e(TAG, "checkSdcardIsAvailable false, card mount error");
            return SDCARD_STATUS.STATE_UNMOUNT;
        } else if (insufficientSpace > 0) {
            return SDCARD_STATUS.STATE_INSUFFICIENT;
        }

        return SDCARD_STATUS.STATE_OK;
    }

    /**
     * Calculate how much space is needed to be freed for the upgrade package.
     * 
     * @param context
     *            environment context
     * @param miniSize
     *            the minimized size needed for checking with unit byte.-1 means
     *            error occurs. 0 means the space is enough
     * 
     * @return the space needed to be freed to hold the file of the size of
     *         "miniSize"
     * 
     */
    static long getExtraSpaceNeeded(Context context, long miniSize) {
        Xlog.i(TAG, "checkSdcardSpaceNeeded miniSize = " + miniSize);

        String availablePath = getAvailablePath(context);
        if (availablePath == null) {
            return -1;
        }
        StatFs statfs = new StatFs(availablePath);
        if (statfs == null) {
            return -1;
        }

        long totalSize = (long) statfs.getBlockSize() * statfs.getAvailableBlocks();
        Xlog.i(TAG, "checkSdcardSpaceNeeded, totalSize = " + totalSize);
        if (totalSize < miniSize) {
            return miniSize - totalSize;
        }

        return 0;
    }

    static String getAvailablePath(Context context) {
        if (sAvailablePath == null) {
            if (FeatureOption.MTK_2SDCARD_SWAP || FeatureOption.MTK_SHARED_SDCARD) {
                if (sExternalSdCardPath == null) {
                    sExternalSdCardPath = getExternalSDCardPath(context);
                }
                sAvailablePath = sExternalSdCardPath;
            } else {
                File sdcardSystem = Environment.getExternalStorageDirectory();
                sAvailablePath = sdcardSystem.getPath();
            }
        }
        return sAvailablePath;
    }

    static long getFileSize(String path) {

        if (path == null) {

            return 0;
        }

        File file = new File(path);

        if (file.isFile() && file.exists()) {
            return file.length();
        }

        return 0;

    }

    static void deleteFile(String path) {
        Xlog.e(TAG, "[deleteFile], path is " + path);
        if (path == null) {
            Xlog.w(TAG, "path is null, return");
            return;
        }

        File file = new File(path);

        if (!file.exists()) {
            Xlog.e(TAG, path + "does not exist");
            return;
        }

        if (file.isFile()) {
            boolean result = file.delete();
            Xlog.i(TAG, "deleteFile result is:" + result);
            return;
        }

        if (file.isDirectory()) {

            String[] strFileList = file.list();

            if (strFileList == null) {
                return;
            }

            String folderPath = path.endsWith(File.separator) ? path : path + File.separator;
            for (String strName : strFileList) {

                deleteFile(folderPath + strName);

            }
        }

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

    static NETWORK_STATUS getNetworkType(Context context) {
        NETWORK_STATUS ret = NETWORK_STATUS.STATE_NONE_NETWORK;

        ConnectivityManager connetManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connetManager == null) {
            Xlog.e(TAG, "isNetWorkAvailable connetManager = null");
            return ret;
        }
        NetworkInfo[] infos = connetManager.getAllNetworkInfo();
        if (infos == null) {
            return ret;
        }
        for (int i = 0; i < infos.length && infos[i] != null; i++) {
            if (infos[i].isConnected() && infos[i].isAvailable()) {

                if (infos[i].getTypeName().equalsIgnoreCase(NETTYPE_WIFI)) {
                    ret = NETWORK_STATUS.STATE_WIFI;
                } else {
                    ret = NETWORK_STATUS.STATE_GPRS;
                }

                break;
            }
        }

        Xlog.i(TAG, "get network stype is : " + ret);
        return ret;

    }

    static class DeviceInfo {
        String mImei;
        String mSim;
        String mSnNumber;
        String mOperator;
    }

    static DeviceInfo getDeviceInfo(Context context) {

        if (context == null) {

            return null;
        }
        TelephonyManager teleMgr = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

        if (teleMgr != null) {
            DeviceInfo deviceInfo = new DeviceInfo();
            deviceInfo.mImei = teleMgr.getDeviceId();
            deviceInfo.mSim = teleMgr.getLine1Number();
            deviceInfo.mSnNumber = teleMgr.getSimSerialNumber();
            deviceInfo.mOperator = teleMgr.getNetworkOperator();

            return deviceInfo;
        } else {
            return null;
        }

    }

    private static boolean isGmsLoad(Context context) {

        String gmsFlagPath = context.getResources().getString(R.string.gms_load_flag);

        File file = new File(gmsFlagPath);

        return ((file != null) && file.exists());

    }

    static String getDeviceVersionInfo(Context context) {

        StringBuilder builder = new StringBuilder();
        String oem = SystemProperties.get("ro.product.manufacturer");
        String product = SystemProperties.get("ro.product.device");
        String lang = SystemProperties.get("ro.product.locale.language");
        String buildnumber = SystemProperties.get("ro.build.display.id");
        String oper = SystemProperties.get("ro.operator.optr");
        String flavor = SystemProperties.get("ro.build.flavor");

        if (oem == null) {
            oem = "null";
        }
        if (product == null) {
            product = "null";
        }
        if (lang == null) {
            lang = "null";
        }
        if (buildnumber == null) {
            buildnumber = "null";
        }
        if (oper == null) {
            oper = "null";
        }

        oem = oem.replaceAll("_", "\\$");

        if (!TextUtils.isEmpty(flavor)) {
            product = product + "[" + flavor + "]";
        }

        product = product.replaceAll("_", "\\$");

        builder.append(oem).append("_").append(product);

        if (isGmsLoad(context)) {
            builder.append("[gms]");
        }

        lang = lang.replaceAll("_", "\\$");
        buildnumber = buildnumber.replaceAll("_", "\\$");
        oper = oper.replaceAll("_", "\\$");

        builder.append("_").append(lang).append("_").append(buildnumber).append("_").append(oper);

        String versionInfo = builder.toString();
        Xlog.i(TAG, "getDeviceVersionInfo = " + versionInfo);

        return versionInfo;
    }

    static boolean checkIfTopActivity(Context context, String classname) {

        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> taskInfo = am.getRunningTasks(1);
        ComponentName componentInfo = taskInfo.get(0).topActivity;

        if (componentInfo != null) {

            String strClass = componentInfo.getClassName();
            Xlog.i(TAG, "strClass = " + strClass);

            return classname.equals(strClass);
        } else {
            return false;
        }

    }

    static void setAlarm(Context context, int alarmType, long time, String action) {
        Xlog.i(TAG, "setAlarm enter, time = " + time + ", current time = " + Calendar.getInstance().getTimeInMillis()
                + ", action = " + action);
        Intent it = new Intent(action);
        AlarmManager alarmMgr = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent operation = PendingIntent.getBroadcast(context, 0, it, PendingIntent.FLAG_UPDATE_CURRENT);

        if ((alarmMgr != null) && (operation != null)) {
            alarmMgr.set(alarmType, time, operation);
        }
        
    }
    


    static void cancelAlarm(Context context, String action) {
        Xlog.i(TAG, "cancelAlarm, action = " + action + ", current time = " + Calendar.getInstance().getTimeInMillis());
       Intent it = new Intent(action);
        AlarmManager alarmMgr = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent operation = PendingIntent.getBroadcast(context, 0, it, PendingIntent.FLAG_UPDATE_CURRENT);

        
        if ((alarmMgr != null) && (operation != null)) {
            alarmMgr.cancel(operation);
        }

    }

    public static String getPackagePathName(Context context) {

        String availablePath = getAvailablePath(context);
        if (availablePath != null) {
            String packagePath = availablePath + PathName.OTA_PKG_FOLDER;
            File dir = new File(packagePath);
            if (!dir.exists()) {
                dir.mkdirs();
            }
            Xlog.i(TAG, "packagePath = " + packagePath);
            return packagePath;
        } else {
            Xlog.e(TAG, "packagePath == null");
            return null;
        }
    }

    static String getTempPath(Context context) {

        String packagePath = getPackagePathName(context);
        if (packagePath != null) {
            String tempPath = packagePath + Util.PathName.TEMP_DIR;
            File dir = new File(tempPath);
            if (!dir.exists()) {
                dir.mkdirs();
            }
            Xlog.i(TAG, "getTempPath = " + tempPath);
            return tempPath;
        } else {
            return null;
        }
    }

    public static String getPackageFileName(Context context) {
        String packagePath = getPackagePathName(context);
        if (packagePath != null) {
            String path = packagePath + PathName.PACKAGE_NAME;
            Xlog.i(TAG, "getPackageFileName = " + path);
            return path;
        } else {
            return null;
        }
    }

    static int unzipFileElement(ZipFile zipFile, String strSrcFile, String strDesFile) {

        Xlog.i(TAG, "unzipFileElement:" + strSrcFile + ";" + strDesFile);
        ZipEntry zipEntry = zipFile.getEntry(strSrcFile);

        if (zipEntry == null) {
            return OTAresult.ERROR_OTA_FILE;
        }

        try {
            InputStream in = zipFile.getInputStream(zipEntry);

            if (in == null) {
                return OTAresult.ERROR_FILE_OPEN;
            }

            File desFile = new File(strDesFile);
            if (desFile == null) {
                return OTAresult.ERROR_FILE_OPEN;
            }
            if (desFile.exists()) {
                desFile.delete();
            }
            desFile.createNewFile();
            OutputStream out = new FileOutputStream(desFile);
            if (out == null) {
                return OTAresult.ERROR_FILE_OPEN;

            }
            byte[] buffer = new byte[BUFF_SIZE];
            int realLength = 0;
            while ((realLength = in.read(buffer)) > 0) {
                out.write(buffer, 0, realLength);
            }
            out.close();
            out = null;
            in.close();
            in = null;
        } catch (IOException e) {
            e.printStackTrace();
            return OTAresult.ERROR_FILE_WRITE;
        }

        return OTAresult.OTA_FILE_UNZIP_OK;
    }


    public static UPDATE_TYPES getUpdateType() {
        UPDATE_TYPES type = UPDATE_OPTION;
        String typeString = UpdateOption.getUpdateTypeFromEM();
        if (!TextUtils.isEmpty(typeString)) {
            type = UPDATE_TYPES.valueOf(typeString);
        }
        Xlog.v(TAG, "get update Type  = " + type);
        return type;
    }

}
