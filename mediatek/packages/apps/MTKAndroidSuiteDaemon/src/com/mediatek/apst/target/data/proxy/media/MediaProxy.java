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

package com.mediatek.apst.target.data.proxy.media;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.text.TextUtils;

import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.media.MediaInfo;

import java.io.File;
import java.util.ArrayList;

/**
 * Class Name: MediaProxy
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.media
 * <p>
 * Created on: 2010-8-12
 * <p>
 * <p>
 * Description:
 * <p>
 * Facade of the sub system of media content related operations.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public final class MediaProxy extends ContextBasedProxy {
    /** Singleton instance. */
    private static MediaProxy sInstance = null;
    
    private ContentResolver contentResolver;

    // private StorageManager mStorageManager = null;
    /**
     * @param context
     *            The context of the media proxy.
     */
    private MediaProxy(Context context) {
        super(context);
        setProxyName("MediaProxy");
        // mStorageManager = (StorageManager)
        // context.getSystemService(Context.STORAGE_SERVICE);
        contentResolver = context.getContentResolver();
    }

    /**
     * @param context
     *            The current proxy.
     * @return A instance of the MediaProxy.
     */
    public static synchronized MediaProxy getInstance(Context context) {
        if (null == sInstance) {
            sInstance = new MediaProxy(context);
        } else {
            sInstance.setContext(context);
        }
        return sInstance;
    }

    /**
     * @return Whether the sdcard is mounted.
     */
    public boolean isSdMounted() {
        return SystemInfoProxy.isSdMounted();
    }

    /**
     * @return Whether the sdcard is writeable.
     */
    public boolean isSdWriteable() {
        return SystemInfoProxy.isSdWriteable();
    }

    /**
     * @return false.
     */
    public boolean existFile() {
        return false;
    }

    /**
     * @return The array of the MediaInfo.
     */
    public MediaInfo[] getContentDirectories() {
        if (!isSdMounted()) {
            Debugger.logE("SD card is not mounted.");
            return null;
        }
        if (!isSdWriteable()) {
            Debugger.logE("SD card is not writeable.");
            return null;
        }

        MediaInfo[] dirs = new MediaInfo[9];
        File path;
        // Alarms
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_ALARMS);
        path.mkdirs();
        dirs[0] = new MediaInfo(MediaInfo.ALARMS, path.getAbsolutePath());
        // DCIM
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM);
        path.mkdirs();
        dirs[1] = new MediaInfo(MediaInfo.DCIM, path.getAbsolutePath());
        // Downloads
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        path.mkdirs();
        dirs[2] = new MediaInfo(MediaInfo.DOWNLOADS, path.getAbsolutePath());
        // Movies
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES);
        path.mkdirs();
        dirs[3] = new MediaInfo(MediaInfo.MOVIES, path.getAbsolutePath());
        // Music
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
        path.mkdirs();
        dirs[4] = new MediaInfo(MediaInfo.MUSIC, path.getAbsolutePath());
        // Notifications
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_NOTIFICATIONS);
        path.mkdirs();
        dirs[5] = new MediaInfo(MediaInfo.NOTIFICATIONS, path.getAbsolutePath());
        // Pictures
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        path.mkdirs();
        dirs[6] = new MediaInfo(MediaInfo.PICTURES, path.getAbsolutePath());
        // Podcasts
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PODCASTS);
        path.mkdirs();
        dirs[7] = new MediaInfo(MediaInfo.PODCASTS, path.getAbsolutePath());
        // Ringtones
        path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_RINGTONES);
        path.mkdirs();
        dirs[8] = new MediaInfo(MediaInfo.RINGTONES, path.getAbsolutePath());

        return dirs;
    }

    /**
     * @param contentType
     *            The type of the content.
     * @return The File path for the directory.
     */
    public File getFile(int contentType) {

        if (!isSdMounted()) {
            Debugger.logE("SD card is not mounted.");
            return null;
        }
        if (!isSdWriteable()) {
            Debugger.logE("SD card is not writeable.");
            return null;
        }
        File file = null;
        switch (contentType) {
        case MediaInfo.ALARMS:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_ALARMS);
            file.mkdirs();
            break;
        case MediaInfo.DCIM:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM);
            file.mkdirs();
            break;
        case MediaInfo.DOWNLOADS:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            file.mkdirs();
            break;
        case MediaInfo.MOVIES:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES);
            file.mkdirs();
            break;
        case MediaInfo.MUSIC:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
            file.mkdirs();
            break;
        case MediaInfo.NOTIFICATIONS:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_NOTIFICATIONS);
            file.mkdirs();
            break;
        case MediaInfo.PICTURES:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
            file.mkdirs();
            break;
        case MediaInfo.PODCASTS:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PODCASTS);
            file.mkdirs();
            break;
        case MediaInfo.RINGTONES:
            file = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_RINGTONES);
            file.mkdirs();
            break;
        default:
            break;
        }
        return file;
    }

    /**
     * @param requestedContentTypes
     *            The content type of the media.
     * @return The array of the media information.
     */
    public MediaInfo[] getFiles(int requestedContentTypes) {
        if (!isSdMounted()) {
            Debugger.logE(new Object[] { requestedContentTypes }, "SD card is not mounted.");
            return null;
        }

        MediaInfo[] results;
        ArrayList<MediaInfo> files = new ArrayList<MediaInfo>();
        File dir;

        if ((requestedContentTypes & MediaInfo.ALARMS) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_ALARMS);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.ALARMS);
        }
        if ((requestedContentTypes & MediaInfo.DCIM) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.DCIM);
        }
        if ((requestedContentTypes & MediaInfo.DOWNLOADS) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.DOWNLOADS);
        }
        if ((requestedContentTypes & MediaInfo.MOVIES) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.MOVIES);
        }
        if ((requestedContentTypes & MediaInfo.MUSIC) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.MUSIC);
        }
        if ((requestedContentTypes & MediaInfo.NOTIFICATIONS) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_NOTIFICATIONS);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.NOTIFICATIONS);
        }
        if ((requestedContentTypes & MediaInfo.PICTURES) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.PICTURES);
        }
        if ((requestedContentTypes & MediaInfo.PODCASTS) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PODCASTS);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.PODCASTS);
        }
        if ((requestedContentTypes & MediaInfo.RINGTONES) > 0) {
            dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_RINGTONES);
            getFilesUnder(dir, files, dir.getAbsolutePath().length() + 1, MediaInfo.RINGTONES);
        }

        results = new MediaInfo[files.size()];
        files.toArray(results);
        return results;
    }

    /**
     * @param dir
     * @param output
     * @param trimLeft
     * @param contentType
     */
    private void getFilesUnder(File dir, ArrayList<MediaInfo> output, int trimLeft, int contentType) {
        if (null == output) {
            return;
        }
        if (null == dir || !dir.exists()) {
            return;
        }

        // Get all files and sub directories under the current directory
        File[] files = dir.listFiles();
        if (null == files) {
            // Current file is not a directory
            MediaInfo info = new MediaInfo(contentType, dir.getAbsolutePath().substring(trimLeft));
            info.setFileLength(dir.length());
            info.setLastModified(dir.lastModified());
            output.add(info);
            return;
        } else {
            // Current file is a directory
            for (int i = 0; i < files.length; i++) {
                getFilesUnder(files[i], output, trimLeft, contentType);
            }
        }
    }

    /**
     * @param path
     * @param oldPaths
     */
    public void getFilesUnder(String path, ArrayList<String> oldPaths) {
        File dir = new File(path);
        if (null == oldPaths) {
            return;
        }
        if (null == dir || !dir.exists()) {
            return;
        }

        // Get all files and sub directories under the current directory
        File[] files = dir.listFiles();
        if (null == files) {
            // Current file is not a directory
            oldPaths.add(dir.getAbsolutePath());
            return;
        } else {
            // Current file is a directory
            for (int i = 0; i < files.length; i++) {
                getFilesUnder(files[i].getAbsolutePath(), oldPaths);
            }
        }
    }

    /**
     * @param path
     * @param oldPaths
     * @param newPaths
     */
    public void getFilesUnder(String path, ArrayList<String> oldPaths, ArrayList<String> newPaths) {
        File dir = new File(path);
        if (null == oldPaths) {
            return;
        }
        if (null == dir || !dir.exists()) {
            return;
        }

        // Get all files and sub directories under the current directory
        File[] files = dir.listFiles();
        if (null == files) {
            // Current file is not a directory
            oldPaths.add(dir.getAbsolutePath());
            String internalStorgePath = SystemInfoProxy.getInternalStoragePathSD();
            String externalStorgePath = SystemInfoProxy.getExternalStoragePath();
            if (internalStorgePath == null || (0 == internalStorgePath.trim().length())) {
                internalStorgePath = " ";
            }
            if (externalStorgePath == null || (0 == externalStorgePath.trim().length())) {
                externalStorgePath = " ";
            }
            if (path.startsWith(internalStorgePath)) {
                newPaths.add(internalStorgePath);
            } else if (path.startsWith(externalStorgePath)) {
                newPaths.add(externalStorgePath);
            } else {
                newPaths.add(dir.getParent());
            }
            return;
        } else {
            // Current file is a directory
            for (int i = 0; i < files.length; i++) {
                getFilesUnder(files[i].getAbsolutePath(), oldPaths, newPaths);
            }
        }
    }

    /**
     * @param dirs
     * @param oldPaths
     * @param newPaths
     */
    public void getFilesUnderDirs(ArrayList<String> dirs, ArrayList<String> oldPaths, ArrayList<String> newPaths) {

        if (null == dirs) {
            return;
        }
        if (null == oldPaths) {
            return;
        }
        if (null == newPaths) {
            return;
        }

        for (String dir : dirs) {
            getFilesUnder(dir, oldPaths, newPaths);
        }
    }

    /**
     * @param path
     * @param end
     * @param oldPaths
     * @param newPaths
     */
    public void getFilesEndWith(String path, String end, ArrayList<String> oldPaths, ArrayList<String> newPaths) {

        File dir = new File(path);
        if (null == oldPaths) {
            return;
        }
        if (null == dir || !dir.exists()) {
            return;
        }

        // Get all files and sub directories under the current directory
        File[] files = dir.listFiles();
        if (null == files) {
            // Current file is not a directory
            String oldFilePath = dir.getAbsolutePath();
            String newFilePath = dir.getParent();
            if (oldFilePath.endsWith(end)) {
                oldPaths.add(oldFilePath);
                newPaths.add(newFilePath);
                Debugger.logE("old path :" + oldFilePath);
                Debugger.logE("new path :" + newFilePath);
            }
            return;
        } else {
            // Current file is a directory
            for (int i = 0; i < files.length; i++) {
                getFilesEndWith(files[i].getAbsolutePath(), end, oldPaths, newPaths);
            }
        }
    }

    /**
     * @param dir
     * @param dirs
     */
    private void getDirectory(File dir, ArrayList<File> dirs) {

        // Get all files and sub directories under the current directory
        File[] files = dir.listFiles();
        if (null == files) {
            // Current file is not a directory
            return;
        } else {
            // Current file is a directory
            dirs.add(dir);
            Debugger.logI("get directory :" + dir.getPath());
            for (int i = 0; i < files.length; i++) {
                getDirectory(files[i], dirs);
            }
        }
    }

    /**
     * @param paths
     *            The path of the file.
     * @param dirs
     *            The directory of the file.
     */
    public void getDirectiries(ArrayList<String> paths, ArrayList<File> dirs) {
        for (String path : paths) {
            File file = new File(path);
            if (file.exists()) {
                getDirectory(file, dirs);
            }
        }
    }

    /**
     * @param path
     *            The path of the directory to create.
     * @return Whether success to create the directory.
     */
    public boolean createDirectory(String path) {
        boolean result = false;
        File file = new File(path);
        if ((null != file && !file.exists())) {
            Debugger.logI("Create file :" + file.getPath());
            result = file.mkdirs();
        } else {
            result = true;
        }
        return result;
    }

    /**
     * @param dir
     *            The directory of the file.
     */
    public void deleteAllFileUnder(File dir) {

        File[] files = dir.listFiles();
        if (null == files) {
            // Current file is not a directory
            dir.delete();
            deleteFileInMediaProxy(dir.getPath());
            Debugger.logI("Delete file :" + dir.getPath());
        } else {
            for (int i = 0; i < files.length; i++) {
                deleteAllFileUnder(files[i]);
            }
        }
    }

    /**
     * @param path
     *            The path of file.
     */
    public void deleteAllFileUnder(String path) {
        File file = new File(path);
        if (file.exists()) {
            File[] files = file.listFiles();
            if (null == files) {
                // Current file is not a directory
                file.delete();
                deleteFileInMediaProxy(file.getPath());
                Debugger.logI("Delete file :" + file.getPath());
            } else {
                for (int i = 0; i < files.length; i++) {
                    deleteAllFileUnder(files[i]);
                }
            }
        }
    }

    /**
     * @param paths
     *            The path of all content.
     */
    public void deleteAllFiles(ArrayList<String> paths) {
        for (String path : paths) {
            File file = new File(path);
            if (file.exists()) {
                deleteAllFileUnder(file);
            }
        }
    }

    /**
     * @param dirsPath
     *            The list of directory path.
     */
    public void deleteAllDirectorys(ArrayList<String> dirsPath) {
        ArrayList<File> dirs = new ArrayList<File>();
        getDirectiries(dirsPath, dirs);
        for (int i = dirs.size() - 1; i >= 0; i--) {
            File temp = dirs.get(i);
            if (!dirsPath.contains(temp.getAbsolutePath())) {
                boolean isDelete = temp.delete();
                deleteFileInMediaProxy(temp.getPath());
                Debugger.logI(String.valueOf(isDelete) + temp.getPath());
            }
        }
    }

    /**
     * @param path
     *            The path of the content.
     */
	public void deleteDirectory(String path) {
        File file = new File(path);
        if (file.exists()) {
            ArrayList<File> dirs = new ArrayList<File>();
            getDirectory(file, dirs);
            for (int i = dirs.size() - 1; i >= 0; i--) {
                File deleteFile = dirs.get(i);
                if (deleteFile.exists()) {
                    boolean isDelete = deleteFile.delete();
                    deleteFileInMediaProxy(deleteFile.getPath());
                    Debugger.logI(String.valueOf(isDelete) + deleteFile.getPath());
                }
            }
        }
    }

    /**
     * @param size
     *            The size of the content.
     * @param contentType
     *            The type of the content.
     * @return 1, if available space in sdcard is enough for the content or 0, if not.
     */
    public int checkStoreState(long size, int contentType) {
        File file = getFile(contentType);
        long temp = file.length();
        long flag = SystemInfoProxy.getSdAvailableSpace() - temp + size;
        if (flag <= 0) {
            return 0;
        } else {
            return 1;
        }
    }

    /**
     * @param path
     *            The path of the file.
     * @return Whether the file is existed.
     */
    public boolean isFileExisted(String path) {
        File file = new File(path);
        return file.exists();
    }

    /**
     * @param oldPath
     *            The old path of the file.
     * @param newPath
     *            The new path of the file.
     * @return A array of boolean, its element representing the result of the rename for a file.
     */
    public boolean renameFile(String oldPath, String newPath) {
        if (null == oldPath) {
            Debugger.logE(new Object[] { oldPath, newPath }, "Old path should not be null.");
            return false;
        }
        if (null == newPath) {
            Debugger.logE(new Object[] { oldPath, newPath }, "New path should not be null.");
            return false;
        }
        if (oldPath.equals("")) {
            Debugger.logE(new Object[] { oldPath, newPath }, "Old path should not be empty.");
            return false;
        }
        if (newPath.equals("")) {
            Debugger.logE(new Object[] { oldPath, newPath }, "New path should not be empty.");
            return false;
        }

        boolean result = false;
        File src = new File(oldPath);
        if (src.exists()) {
            File dest = new File(newPath);
            File destDir = dest.getParentFile();
            if (null != destDir) {
                // Ensure the destination directory exists
                destDir.mkdirs();
            }
            try {
                result = src.renameTo(dest);
            } catch (SecurityException e) {
                Debugger.logE(new Object[] { oldPath, newPath }, null, e);
            } finally {
                if (src.exists()) {
                    // Delete the source file
                    src.delete();
                }
            }
        }
        return result;
    }

    /**
     * @param oldPath
     *            The old path of the file.
     * @param newPath
     *            The new path of the file.
     * @return A array of boolean, its element representing the result of the rename for a file.
     */
    public boolean renameFileForBackup(String oldPath, String newPath) {
        if (null == oldPath) {
            Debugger.logE(new Object[] { oldPath, newPath }, "Old path should not be null.");
            return false;
        }
        if (null == newPath) {
            Debugger.logE(new Object[] { oldPath, newPath }, "New path should not be null.");
            return false;
        }
        if (oldPath.equals("")) {
            Debugger.logE(new Object[] { oldPath, newPath }, "Old path should not be empty.");
            return false;
        }
        if (newPath.equals("")) {
            Debugger.logE(new Object[] { oldPath, newPath }, "New path should not be empty.");
            return false;
        }

        boolean result = false;
        File src = new File(oldPath);
        if (src.exists()) {
            File dest = new File(newPath);
            File destDir = dest.getParentFile();
            if (null != destDir) {
                // Ensure the destination directory exists
                destDir.mkdirs();
            }
            try {
                result = src.renameTo(dest);
            } catch (SecurityException e) {
                Debugger.logE(new Object[] { oldPath, newPath }, null, e);
            } finally {
                if (src.exists()) {
                    // Delete the source file
                    src.delete();
                }
            }
        }
        return result;
    }

    /**
     * @param oldPaths
     *            The old path of the file.
     * @param newPaths
     *            The new path of the file.
     * @return A array of boolean, its element representing the result of the rename for a file.
     */
    public boolean[] renameFiles(String[] oldPaths, String[] newPaths) {
        if (null == oldPaths) {
            Debugger.logE(new Object[] { oldPaths, newPaths }, "Old paths list should not be null.");
            return null;
        }
        if (null == newPaths) {
            Debugger.logE(new Object[] { oldPaths, newPaths }, "New paths list should not be null.");
            return null;
        }
        if (oldPaths.length != newPaths.length) {
            Debugger.logE(new Object[] { oldPaths, newPaths }, "Old paths list size does not match new paths list size.");
            return null;
        }

        boolean[] results = new boolean[oldPaths.length];
        for (int i = 0; i < oldPaths.length; i++) {
            results[i] = renameFile(oldPaths[i], newPaths[i]);
            Debugger.logI("rename oldPaths[" + i + "]: " + oldPaths[i]);
            Debugger.logI("rename newPaths[" + i + "]: " + newPaths[i]);
        }

        return results;
    }

    /**
     * This method checks whether SDcard is mounted or not
     * 
     * @param mountPoint
     *            the mount point that should be checked
     * @return true if SDcard is mounted, false otherwise
     */
    // protected boolean checkSDCardMount(String mountPoint) {
    // if(mountPoint == null){
    // return false;
    // }
    //       
    // String state = null;
    // state = mStorageManager.getVolumeState(mountPoint);
    // return Environment.MEDIA_MOUNTED.equals(state);
    // }
    //    

    // public boolean[] checkSDCardState() {
    // // check media availability
    // boolean[] SDstate = new boolean[2];
    // String mSDCardPath = null , mSDCard2Path = null;
    // String[] storagePathList = mStorageManager.getVolumePaths();
    // if (storagePathList != null) {
    // if (storagePathList.length >= 2) {
    // mSDCardPath = storagePathList[0];
    // mSDCard2Path = storagePathList[1];
    // } else if (storagePathList.length == 1) {
    // mSDCardPath = storagePathList[0];
    // }
    // if (null != mSDCardPath) {
    // String state = null;
    // state = mStorageManager.getVolumeState(mSDCardPath);
    // SDstate[0] = Environment.MEDIA_MOUNTED.equals(state);
    // } else {
    // SDstate[0] = false;
    // }
    // if (null != mSDCard2Path) {
    // String state = null;
    // state = mStorageManager.getVolumeState(mSDCard2Path);
    // SDstate[1] = Environment.MEDIA_MOUNTED.equals(state);
    // } else {
    // SDstate[1] = false;
    // }
    // }
    // return SDstate;
    // }
    /**
     * Sync over,notify Music and Gallery refresh.
     * 
     * @param context
     *            The current context.
     */
    public void scan(Context context) {
        File musicFile = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
        File[] musicFileList = musicFile.listFiles();
        String[] musicFilePaths = new String[musicFileList.length];
        for (int i = 0; i < musicFileList.length; i++) {
            musicFilePaths[i] = musicFileList[i].getPath();
            // Log.i("", musicFilePaths[i]);
        }

        File pictureFile = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        File[] pictureFileList = pictureFile.listFiles();
        String[] pictureFilePaths = new String[pictureFileList.length];
        for (int i = 0; i < pictureFileList.length; i++) {
            pictureFilePaths[i] = pictureFileList[i].getPath();
            // Log.i("", pictureFilePaths[i]);
        }

        android.media.MediaScannerConnection.scanFile(context, musicFilePaths, null, null);
        android.media.MediaScannerConnection.scanFile(context, pictureFilePaths, null, null);
        Debugger.logI(new Object[] {}, "Invoke android.media.MediaScannerConnection.scanFile() to refresh Music/Gallery");
    }

    /**
     * Sync over,notify Music and Gallery refresh.
     * 
     * @param context
     *            The current context.
     * @param filePath
     *            The path of the file.
     */
    public void scan(Context context, String filePath) {
        if (null != filePath) {
            File file = new File(filePath);
            if (file.exists()) {
                ArrayList<String> pathsArray = new ArrayList<String>();
                getFilesUnder(filePath, pathsArray);
                String[] paths = new String[pathsArray.size()];
                for (int i = 0; i < pathsArray.size(); i++) {
                    paths[i] = pathsArray.get(i);
                }
                android.media.MediaScannerConnection.scanFile(context, paths, null, null);
            }
            Debugger.logI(new Object[] { filePath }, "Refresh Media");
        }
    }
    
    /**
     * Delete the MediaProxy db record after delete files
     * 
     * @param filePath
     *            The path of the file.
     */
    public void deleteFileInMediaProxy(String filePath)
    {
    	if (TextUtils.isEmpty(filePath)) {
            return;
        }
    	
    	Uri uri = MediaStore.Files.getContentUri("external");
        String where = MediaStore.Files.FileColumns.DATA + "=?";
        String[] whereArgs = new String[] { filePath };
        
        try {
        	contentResolver.delete(uri, where, whereArgs);
        } catch (UnsupportedOperationException e) {
        	Debugger.logI(new Object[] { filePath, uri}, "Error, database is closed!!!");
            
        }
    }
}
