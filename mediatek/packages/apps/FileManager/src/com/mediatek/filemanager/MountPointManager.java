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

package com.mediatek.filemanager;

import android.content.Context;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.text.TextUtils;

import com.mediatek.filemanager.utils.FileUtils;
import com.mediatek.filemanager.utils.LogUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import com.mediatek.storage.StorageManagerEx;

public final class MountPointManager {
    private static final String TAG = "MountPointManager";

    public static final String SEPARATOR = "/";
    public static final String HOME = "Home";
    public static final String ROOT_PATH = "/storage";

    private String mRootPath = "Root Path";
    private static MountPointManager sInstance = new MountPointManager();

    private StorageManager mStorageManager = null;
    private final CopyOnWriteArrayList <MountPoint> mMountPathList = new CopyOnWriteArrayList<MountPoint>();

    private MountPointManager() {
    }

    /**
     * This method initializes MountPointManager.
     * 
     * @param context Context to use
     */
    public void init(Context context) {
        mStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        final String defaultPath = getDefaultPath();
        LogUtils.d(TAG, "init,defaultPath = " + defaultPath);   
        if (!TextUtils.isEmpty(defaultPath)) {
            mRootPath = ROOT_PATH;
        }
        mMountPathList.clear();
        // check media availability to init mMountPathList
        StorageVolume[] storageVolumeList = mStorageManager.getVolumeList();
        if (storageVolumeList != null) {
            for (StorageVolume volume : storageVolumeList) {
                MountPoint mountPoint = new MountPoint();
                mountPoint.mDescription = volume.getDescription(context);
                mountPoint.mPath = volume.getPath();
                mountPoint.mIsMounted = isMounted(volume.getPath());
                mountPoint.mIsExternal = volume.isRemovable();
                mountPoint.mMaxFileSize = volume.getMaxFileSize();
                LogUtils.d(TAG, "init,description :" + mountPoint.mDescription + ",path : "
                        + mountPoint.mPath + ",isMounted : " + mountPoint.mIsMounted
                        + ",isExternal : " + mountPoint.mIsExternal + ", mMaxFileSize: " + mountPoint.mMaxFileSize);
                mMountPathList.add(mountPoint);
            }
        }
        IconManager.getInstance().init(context, defaultPath + SEPARATOR);
    }

    /**
     * This method gets instance of MountPointManager. Before calling this method, must call init().
     * 
     * @return instance of MountPointManager
     */
    public static MountPointManager getInstance() {
        return sInstance;
    }

    private static class MountPoint {
        String mDescription;
        String mPath;
        boolean mIsExternal;
        boolean mIsMounted;
        long mMaxFileSize;
    }

    /**
     * This method checks weather certain path is root path.
     * 
     * @param path certain path to be checked
     * @return true for root path, and false for not root path
     */
    public boolean isRootPath(String path) {
        return mRootPath.equals(path);
    }

    /**
     * This method gets root path
     * 
     * @return root path
     */
    public String getRootPath() {
        return mRootPath;
    }

    /**
     * This method gets informations of file of mount point path
     * 
     * @return fileInfos of mount point path
     */
    public List<FileInfo> getMountPointFileInfo() {
        List<FileInfo> fileInfos = new ArrayList<FileInfo>(0);
        for (MountPoint mp : mMountPathList) {
            if (mp.mIsMounted) {
                fileInfos.add(new FileInfo(mp.mPath));
            }
        }
        return fileInfos;
    }

    /**
     * This method gets count of mount, number of mount point(s)
     * 
     * @return number of mount point(s)
     */
    public int getMountCount() {
        int count = 0;
        for (MountPoint mPoint : mMountPathList) {
            if (mPoint.mIsMounted) {
                count++;
            }
        }
        LogUtils.d(TAG, "getMountCount,count = " + count);
        return count;
    }

    /**
     * This method gets default path from StorageManager
     * 
     * @return default path from StorageManager
     */
    public String getDefaultPath() {
        LogUtils.d(TAG, "getDefaultPath:" + StorageManagerEx.getDefaultPath());
        return StorageManagerEx.getDefaultPath();

    }

    /**
     * This method checks whether SDcard is mounted or not
     * 
     * @param mountPoint the mount point that should be checked
     * @return true if SDcard is mounted, false otherwise
     */
    protected boolean isMounted(String mountPoint) {
        LogUtils.d(TAG, "isMounted, mountPoint = " + mountPoint);
        if (TextUtils.isEmpty(mountPoint)) {
            return false;
        }
        String state = null;
        
        state = mStorageManager.getVolumeState(mountPoint);
        LogUtils.d(TAG, "state = " + state);
        return Environment.MEDIA_MOUNTED.equals(state);
    }

    /**
     * This method checks whether SDcard is mounted or not
     * 
     * @param path the path that should be checked
     * @return true if SDcard is mounted, false otherwise
     */
    protected boolean isRootPathMount(String path) {
        LogUtils.d(TAG, "isRootPathMount,  path = " +  path);
        boolean ret = false;
        if (path == null) {
            return ret;
        }
        ret = isMounted(getRealMountPointPath(path));
        LogUtils.d(TAG, "isRootPathMount,  ret = " + ret);
        return ret;
    }

    /**
     * This method gets real mount point path for certain path.
     * 
     * @param path certain path to be checked
     * @return real mount point path for certain path, "" for path is not mounted
     */
    public String getRealMountPointPath(String path) {
        LogUtils.d(TAG, "getRealMountPointPath ,path =" + path);
        for (MountPoint mountPoint : mMountPathList) {
            if ((path + SEPARATOR).startsWith(mountPoint.mPath + SEPARATOR)) {
                LogUtils.d(TAG, "getRealMountPointPath = " + mountPoint.mPath);
                return mountPoint.mPath;
            }
        }
        LogUtils.d(TAG, "getRealMountPointPath = \"\" ");
        return "";
    }
    
    /**
     * This method checks weather certain path is a FAT32 disk.
     * 
     * @param path certain path to be checked
     * @return true for FAT32, and false for not.
     */
    public boolean isFat32Disk(String path) {
        LogUtils.d(TAG, "isFat32Disk ,path =" + path);
        for (MountPoint mountPoint : mMountPathList) {
            if ((path + SEPARATOR).startsWith(mountPoint.mPath + SEPARATOR)) {
                LogUtils.d(TAG, "isFat32Disk = " + mountPoint.mPath);
                if(mountPoint.mMaxFileSize > 0) {
                	 LogUtils.d(TAG, "isFat32Disk = true." );
                	return true;
                }
                LogUtils.d(TAG, "isFat32Disk = false." );
                return false;
            }
        }
        
        LogUtils.d(TAG, "isFat32Disk = false." );
        return false;
    }

    /**
     * This method changes mount state of mount point, if parameter path is mount point.
     * 
     * @param path certain path to be checked
     * @param isMounted flag to mark weather certain mount point is under mounted state
     * @return true for change success, and false for fail
     */
    public boolean changeMountState(String path, Boolean isMounted) {
        boolean ret = false;
        for (MountPoint mountPoint : mMountPathList) {
            if (mountPoint.mPath.equals(path)) {
                if (mountPoint.mIsMounted == isMounted) {
                    break;
                } else {
                    mountPoint.mIsMounted = isMounted;
                    ret = true;
                    break;
                }
            }
        }
        LogUtils.d(TAG, "changeMountState ,path =" + path + ",ret = " + ret);
        
        return ret;
    }

    /**
     * This method checks weather certain path is mount point.
     * 
     * @param path certain path, which needs to be checked
     * @return true for mount point, and false for not mount piont
     */
    public boolean isMountPoint(String path) {
        boolean ret = false;
        LogUtils.d(TAG, "isMountPoint ,path =" + path);
        if (path == null) {
            return ret;
        }
        for (MountPoint mountPoint : mMountPathList) {
            if (path.equals(mountPoint.mPath)) {
                ret = true;
                break;
            }
        }
        LogUtils.d(TAG, "isMountPoint ,ret =" + ret);
        return ret;
    }

    /**
     * This method checks weather certain path is internal mount path.
     * 
     * @param path path which needs to be checked
     * @return true for internal mount path, and false for not internal mount path
     */
    public boolean isInternalMountPath(String path) {
        LogUtils.d(TAG, "isInternalMountPath ,path =" + path);
        if (path == null) {
            return false;
        }
        for (MountPoint mountPoint : mMountPathList) {
            if (!mountPoint.mIsExternal && mountPoint.mPath.equals(path)) {
                return true;
            }
        }
        return false;
    }

    /**
     * This method checks weather certain path is external mount path.
     * 
     * @param path path which needs to be checked
     * @return true for external mount path, and false for not external mount path
     */
    public boolean isExternalMountPath(String path) {
        LogUtils.d(TAG, "isExternalMountPath ,path =" + path);
        if (path == null) {
            return false;
        }
        for (MountPoint mountPoint : mMountPathList) {
            if (mountPoint.mIsExternal && mountPoint.mPath.equals(path)) {
                return true;
            }
        }
        return false;
    }

    /**
     * This method checks weather certain file is External File.
     * 
     * @param fileInfo certain file needs to be checked
     * @return true for external file, and false for not external file
     */
    public boolean isExternalFile(FileInfo fileInfo) {
        boolean ret = false;
        if (fileInfo != null) {
            String mountPath = getRealMountPointPath(fileInfo.getFileAbsolutePath());
            if (mountPath.equals(fileInfo.getFileAbsolutePath())) {
                LogUtils.d(TAG, "isExternalFile,return false .mountPath = " + mountPath);
                ret = false;
            }
            if (isExternalMountPath(mountPath)) {
                ret = true;
            }
        }
        
        LogUtils.d(TAG, "isExternalFile,ret = " + ret);
        
        return ret;
    }

    /**
     * This method gets description of certain path 
     * 
     * @param path certain path
     * @return description of the path
     */
    public String getDescriptionPath(String path) {
        LogUtils.d(TAG, "getDescriptionPath ,path =" + path);
        if (mMountPathList != null) {
            for (MountPoint mountPoint : mMountPathList) {
                if ((path + SEPARATOR).startsWith(mountPoint.mPath + SEPARATOR)) {
                    return path.length() > mountPoint.mPath.length() + 1 ? mountPoint.mDescription
                            + SEPARATOR + path.substring(mountPoint.mPath.length() + 1)
                            : mountPoint.mDescription;
                }
            }
        }
        return path;
    }

    public boolean isPrimaryVolume(String path) {
        LogUtils.d(TAG, "isPrimaryVolume ,path =" + path);
        if (mMountPathList.size() > 0) {
            return mMountPathList.get(0).mPath.equals(path);
        } else {
            LogUtils.w(TAG, "mMountPathList null!");
            return false;
        }
    }
}
