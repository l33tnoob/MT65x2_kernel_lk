package com.mediatek.datatransfer.utils;



import android.content.Context;
import android.os.Environment;
import android.os.Looper;
import android.os.RemoteException;
import android.os.storage.StorageManager;
import com.mediatek.storage.StorageManagerEx;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.datatransfer.R;
import com.mediatek.datatransfer.utils.Constants.LogTag;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.IOException;


public class SDCardUtils {

    public final static int MINIMUM_SIZE = 512;

    public static String getInternalStorage() {
        return StorageManagerEx.getInternalStoragePath();
    }

    public static String getExternalStoragePath(Context context) {
        String storagePath = null;
        StorageManager storageManager = null;

        storageManager = StorageManager.from(context);

        storagePath = StorageManagerEx.getExternalStoragePath();
        if (storagePath == null || storagePath.isEmpty()) {
            MyLogger.logE("SDCardUtils", "storagePath is null");
            return null;
        }
        if (!Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(storagePath))) {
            return null;
        }
        return storagePath;
    }

    public static String getSDStatueMessage(Context context) {
        String message = context.getString(R.string.nosdcard_notice);
        /** M: Bug Fix for CR ALPS01271088 @{ */
        String status = null;
        StorageManager storageManager = StorageManager.from(context);
        String storagePath = StorageManagerEx.getExternalStoragePath();
        if (storagePath != null && !storagePath.isEmpty() && storageManager != null) {
            status = storageManager.getVolumeState(storagePath);
        }
        Log.d("SDCardUtils", "getSDStatueMessage: status is " + status);
        /** @} */
        if (Environment.MEDIA_SHARED.equals(status) ||
                Environment.MEDIA_UNMOUNTED.equals(status)) {
            message = context.getString(R.string.sdcard_busy_message);
        }
        return message;
    }

    public static String getStoragePath(Context context) {
        String storagePath = getExternalStoragePath(context);
        if (storagePath == null) {
            return null;
        }
        storagePath = storagePath + File.separator + "backup";
        Log.d(LogTag.LOG_TAG,
                "getStoragePath: path is " + storagePath);
        File file = new File(storagePath);
        if (file != null) {

            if (file.exists() && file.isDirectory()) {
                File temp = new File(storagePath + File.separator
                        + ".BackupRestoretemp");
                boolean ret;
                if (temp.exists()) {
                    ret = temp.delete();
                } else {
                    try {
                        ret = temp.createNewFile();
                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.e(LogTag.LOG_TAG,
                                "getStoragePath: " + e.getMessage());
                        ret = false;
                    } finally {
                        temp.delete();
                    }
                }
                if (ret) {
                    return storagePath;
                } else {
                    return null;
                }

            } else if (file.mkdir()) {
                return storagePath;
            }
        } else {
            MyLogger.logE(LogTag.LOG_TAG,
                    "getStoragePath: path is not ok");
            return null;
        }
        return null;
    }

    public static String getPersonalDataBackupPath(Context context) {
        String path = getStoragePath(context);
        if (path != null) {
            return path + File.separator + ModulePath.FOLDER_DATA;
        }

        return path;
    }

    public static String getAppsBackupPath(Context context) {
        String path = getStoragePath(context);
        if (path != null) {
            return path + File.separator + ModulePath.FOLDER_APP;
        }
        return path;
    }

    public static boolean isSdCardAvailable(Context context) {
        return (getStoragePath(context) != null);
    }

    public static long getAvailableSize(String file) {
        android.os.StatFs stat = new android.os.StatFs(file);
        long count = stat.getAvailableBlocks();
        long size = stat.getBlockSize();
        long totalSize = count * size;
        Log.v(LogTag.LOG_TAG, "file remain size = " + totalSize);
        return totalSize;
    }
}

