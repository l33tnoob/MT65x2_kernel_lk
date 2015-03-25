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

import android.text.TextUtils;

import com.mediatek.filemanager.utils.LogUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;


public class FileInfoManager {
    private static final String TAG = "FileInfoManager";
    public static final int PASTE_MODE_CUT = 1;
    public static final int PASTE_MODE_COPY = 2;
    public static final int PASTE_MODE_UNKOWN = 0;

    private final List<FileInfo> mAddFilesInfoList = new ArrayList<FileInfo>();
    private final List<FileInfo> mRemoveFilesInfoList = new ArrayList<FileInfo>();
    private final List<FileInfo> mPasteFilesInfoList = new ArrayList<FileInfo>();
    private final List<FileInfo> mShowFilesInfoList = new ArrayList<FileInfo>();
    private int mPasteOperation = PASTE_MODE_UNKOWN;
    private String mLastAccessPath = null;
    protected long mModifiedTime = -1;

    /** Max history size */
    private static final int MAX_LIST_SIZE = 20;

    private final List<NavigationRecord> mNavigationList = new LinkedList<NavigationRecord>();

    /**
     * This method updates mPasteFilesInfoList.
     * 
     * @param pasteType previous operation before paste, copy or cut
     * @param fileInfos list of copied (or cut) files
     */
    public void savePasteList(int pasteType, List<FileInfo> fileInfos) {
        mPasteOperation = pasteType;
        mPasteFilesInfoList.clear();
        mPasteFilesInfoList.addAll(fileInfos);
    }

    /**
     * This method checks weather current path is modified.
     * 
     * @param path certain path to be checked
     * @return true for modified, and false for not modified
     */
    public boolean isPathModified(String path) {
        if (path != null && !path.equals(mLastAccessPath)) {
            return true;
        }
        if (mLastAccessPath != null
                && mModifiedTime != (new File(mLastAccessPath)).lastModified()) {
            return true;
        }
        return false;
    }

    /**
     * This method gets a ArrayList of FileInfo with content of mPasteFilesInfoList.
     * 
     * @return list of files, which paste operation involve
     */
    public List<FileInfo> getPasteList() {
        return new ArrayList<FileInfo>(mPasteFilesInfoList);
    }

    /**
     * This method gets previous operation before paste, copy or cut
     * 
     * @return copy or cut
     */
    public int getPasteType() {
        return mPasteOperation;
    }

    /**
     * This method add file to mAddFilesInfoList
     * 
     * @param fileInfo information of certain file
     */
    public void addItem(FileInfo fileInfo) {
        mAddFilesInfoList.add(fileInfo);
    }

    /**
     * This method adds file to mRemoveFilesInfoList
     * 
     * @param fileInfo information of certain file
     */
    public void removeItem(FileInfo fileInfo) {
        mRemoveFilesInfoList.add(fileInfo);
    }

    /**
     * This method updates all file lists according to parameter path and sortType, and called in
     * onTaskResult() of HeavyOperationListener, which corresponds to operations like delete,
     * copyPaste, cutPaste and so on.
     * 
     * @param currentPath current path
     * @param sortType sort type, which determine files' list sequence
     */
    public void updateFileInfoList(String currentPath, int sortType) {
        LogUtils.d(TAG, "updateFileInfoList,currentPath = " + currentPath + "sortType = "
                + sortType);
        mLastAccessPath = currentPath;
        mModifiedTime = (new File(mLastAccessPath)).lastModified();
        final FileInfo[] addFilesInfos = new FileInfo[mAddFilesInfoList.size()];
        mAddFilesInfoList.toArray(addFilesInfos);
        for (FileInfo fileInfo : addFilesInfos) {
            if (fileInfo.getFileParentPath().equals(mLastAccessPath)) {
                mShowFilesInfoList.add(fileInfo);
            }
        }
        mShowFilesInfoList.removeAll(mRemoveFilesInfoList);
        mPasteFilesInfoList.removeAll(mRemoveFilesInfoList);
        mAddFilesInfoList.clear();
        mRemoveFilesInfoList.clear();
        sort(sortType);
    }

    /**
     * This method adds one file to mShowFilesInfoList, and called in onTaskResult() of
     * LightOperationListener, which corresponds to operations like rename, createFolder and so on.
     * 
     * @param path current path
     * @param sortType sort type, which determine files' list sequence
     * @return information of file, which will be set selected after UI updated. null if size of
     *         mAddFilesInfoList is zero
     */
    public FileInfo updateOneFileInfoList(String path, int sortType) {
        LogUtils.d(TAG, "updateOneFileInfoList,path = " + path + "sortType = " + sortType);
        FileInfo fileInfo = null;
        mLastAccessPath = path;
        mModifiedTime = (new File(mLastAccessPath)).lastModified();
        if (mAddFilesInfoList.size() > 0) {
            fileInfo = mAddFilesInfoList.get(0);
            if (fileInfo.getFileParentPath().equals(mLastAccessPath)) {
                mShowFilesInfoList.add(fileInfo);
            }
        }
        mShowFilesInfoList.removeAll(mRemoveFilesInfoList);
        mPasteFilesInfoList.removeAll(mRemoveFilesInfoList);
        mAddFilesInfoList.clear();
        mRemoveFilesInfoList.clear();
        sort(sortType);

        return fileInfo;
    }

    /**
     * This method adds mAddFilesInfoList to loadFileInfoList
     *
     * @param path the current path to list files
     * @param sortType sort type, which determine files' sequence
     */
    public void loadFileInfoList(String path, int sortType) {
        LogUtils.d(TAG, "loadFileInfoList,path = " + path + ",sortType = " + sortType);
        mShowFilesInfoList.clear();
        mLastAccessPath = path;
        mModifiedTime = (new File(mLastAccessPath)).lastModified();
        if (MountPointManager.getInstance().isRootPath(path)) {
            mAddFilesInfoList.clear();
            List<FileInfo> fileInfoList = new ArrayList<FileInfo>();
            List<FileInfo> mountFileList = MountPointManager.getInstance().getMountPointFileInfo();
            LogUtils.d(this.getClass().getName(), "mountFileList size = " + mountFileList.size());
            if (mountFileList != null) {
                fileInfoList.addAll(mountFileList);
            }
            LogUtils.d(this.getClass().getName(), "fileInfoList size = " + fileInfoList.size());
            addItemList(fileInfoList);
        }
        LogUtils.d(TAG, " mAddFilesInfoList size :"+mAddFilesInfoList.size());
        final FileInfo[] addFilesInfos = new FileInfo[mAddFilesInfoList.size()];
        mAddFilesInfoList.toArray(addFilesInfos);
        for (FileInfo fileInfo : addFilesInfos) {
            LogUtils.d(TAG, "fileinfo is :"+fileInfo.toString());
            if (mLastAccessPath.equals(fileInfo.getFileParentPath())
                    || MountPointManager.getInstance().isMountPoint(
                fileInfo.getFileAbsolutePath())) {
                LogUtils.d(TAG, "mShowFilesInfoLis add fileinfo "+fileInfo.getFileName());
                mShowFilesInfoList.add(fileInfo);
            }
        }
        mAddFilesInfoList.clear();
        sort(sortType);
    }

    /**
     * This method adds mAddFilesInfoList to loadFileInfoList
     *
     * @param path the current path to list files
     * @param sortType sort type, which determine files' sequence
     * @param selectedFileInfo selected file info in edit mode
     */
    public void loadFileInfoList(String path, int sortType, FileInfo selectedFileInfo) {
        LogUtils.d(TAG, "loadFileInfoList,path = " + path + ",sortType = " + sortType);
        mShowFilesInfoList.clear();
        mLastAccessPath = path;
        mModifiedTime = (new File(mLastAccessPath)).lastModified();
        for (FileInfo fileInfo : mAddFilesInfoList) {
            //avoid the null point exception: when deleting, do unmount.
            if (fileInfo == null) {
                LogUtils.w(TAG, "loadFileInfoList,file info is null!");
                continue;
            }
            if (mLastAccessPath.equals(fileInfo.getFileParentPath())
                    || MountPointManager.getInstance().isMountPoint(fileInfo.getFileAbsolutePath())) {
                mShowFilesInfoList.add(fileInfo);
                if (selectedFileInfo != null && fileInfo.getFileName().equals(selectedFileInfo.getFileName())) {
                    fileInfo.setChecked(true);
                }
            }
        }
        mAddFilesInfoList.clear();
        sort(sortType);
    }

    /**
     * This method adds list to mAddFilesInfoList
     * 
     * @param fileInfoList list of files
     */
    public void addItemList(List<FileInfo> fileInfoList) {
        LogUtils.v(TAG, "addItemList");
        mAddFilesInfoList.clear();
        mAddFilesInfoList.addAll(fileInfoList);
    }

    /**
     * This method gets the previous navigation directory path
     * 
     * @return the previous navigation path
     */
    protected NavigationRecord getPrevNavigation() {
        while (!mNavigationList.isEmpty()) {
            NavigationRecord navRecord = mNavigationList.get(mNavigationList.size() - 1);
            removeFromNavigationList();
            String path = navRecord.getRecordPath();
            if (!TextUtils.isEmpty(path)) {
                if (new File(path).exists() || MountPointManager.getInstance().isRootPath(path)) {
                    return navRecord;
                }
            }
        }
        return null;
    }

    /**
     * This method adds a navigationRecord to the navigation history
     * 
     * @param navigationRecord the Record
     */
    protected void addToNavigationList(NavigationRecord navigationRecord) {
        if (mNavigationList.size() <= MAX_LIST_SIZE) {
            mNavigationList.add(navigationRecord);
        } else {
            mNavigationList.remove(0);
            mNavigationList.add(navigationRecord);
        }
    }

    /**
     * This method removes a directory path from the navigation history
     */
    protected void removeFromNavigationList() {
        if (!mNavigationList.isEmpty()) {
            mNavigationList.remove(mNavigationList.size() - 1);
        }
    }

    /**
     * This method clears the navigation history list. Keep the root path only
     */
    protected void clearNavigationList() {
        mNavigationList.clear();
    }

    public static class NavigationRecord {
        private final String mPath;
        private final int mTop;
        private final FileInfo mSelectedFile;

        /**
         * Constructor to construct a NavigationRecord
         * 
         * @param path directory path of NavigationRecord
         * @param selectedFile selected file in NavigationRecord's listView(the first item in
         *            visible listView)
         * @param top distance between selected file and top in pixel
         */
        public NavigationRecord(String path, FileInfo selectedFile, int top) {
            mPath = path;
            mSelectedFile = selectedFile;
            mTop = top;
        }

        /**
         * This method gets path of NavigationRecord
         * 
         * @return path of NavigationRecord
         */
        public String getRecordPath() {
            return mPath;
        }

        /**
         * This method gets distance between selected file and the top in pixel
         * 
         * @return distance between selected file and the top
         */
        public int getTop() {
            return mTop;
        }

        /**
         * This method gets selected file in NavigationRecord's listView.
         * 
         * @return selected file in NavigationRecord's listView
         */
        public FileInfo getSelectedFile() {
            return mSelectedFile;
        }
    }

    /**
     * This method checks weather certain item is included in paste list
     * 
     * @param currentItem certain item, which needs to be checked
     * @return status of weather the item is included in paste list
     */
    public boolean isPasteItem(FileInfo currentItem) {
        return mPasteFilesInfoList.contains(currentItem);
    }

    /**
     * This method gets count of files in PasteFileInfoList, which need to paste
     * 
     * @return number of files, which need to be pasted
     */
    public int getPasteCount() {
        return mPasteFilesInfoList.size();
    }

    /**
     * This method clears pasteList, which stores files need to paste(after copy , or cut)
     */
    public void clearPasteList() {
        mPasteFilesInfoList.clear();
        mPasteOperation = PASTE_MODE_UNKOWN;
    }

    /**
     * This method gets file list for show
     * 
     * @return file list for show
     */
    public List<FileInfo> getShowFileList() {
        LogUtils.d(TAG, "getShowFileList");
        return mShowFilesInfoList;
    }

    /**
     * This method sorts files with given sort type
     * 
     * @param sortType sort type
     */
    public void sort(int sortType) {
        LogUtils.d(TAG, "sort,sortType = " + sortType);
        Collections.sort(mShowFilesInfoList, FileInfoComparator.getInstance(sortType));
    }

    /**
     * This method updates search list, which stores search result
     */
    public void updateSearchList() {
        LogUtils.d(TAG, "updateSearchList");
        mShowFilesInfoList.addAll(mAddFilesInfoList);
        mAddFilesInfoList.clear();
    }
}
