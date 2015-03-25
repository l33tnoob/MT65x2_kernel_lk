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

package com.mediatek.filemanager.service;

import android.content.Context;

import com.mediatek.filemanager.FileInfo;
import com.mediatek.filemanager.FileInfoManager;
import com.mediatek.filemanager.MountPointManager;
import com.mediatek.filemanager.service.FileManagerService.OperationEventListener;
import com.mediatek.filemanager.service.MultiMediaStoreHelper.DeleteMediaStoreHelper;
import com.mediatek.filemanager.service.MultiMediaStoreHelper.PasteMediaStoreHelper;
import com.mediatek.filemanager.utils.FileUtils;
import com.mediatek.filemanager.utils.LogUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

abstract class FileOperationTask extends BaseAsyncTask {
    private static final String TAG = "FileOperationTask";
    //to increase the copy/paste speed
    protected static final int BUFFER_SIZE = 2048 * 1024;
    //protected static final int BUFFER_SIZE = 512 * 1024;
    protected static final int TOTAL = 100;
    protected Context mContext;
    protected MediaStoreHelper mMediaProviderHelper;

    public FileOperationTask(FileInfoManager fileInfoManager,
            OperationEventListener operationEvent, Context context) {
        super(fileInfoManager, operationEvent);
        if (context == null) {
            LogUtils.e(TAG, "construct FileOperationTask exception! ");
            throw new IllegalArgumentException();
        } else {
            mContext = context;
            mMediaProviderHelper = new MediaStoreHelper(context, this);
        }
    }

    protected File getDstFile(HashMap<String, String> pathMap, File file, String defPath) {
        LogUtils.d(TAG, "getDstFile.");
        String curPath = pathMap.get(file.getParent());
        if (curPath == null) {
            curPath = defPath;
        }
        File dstFile = new File(curPath, file.getName());

        return checkFileNameAndRename(dstFile);
    }

    protected boolean deleteFile(File file) {
        if (file == null) {
            publishProgress(new ProgressInfo(OperationEventListener.ERROR_CODE_DELETE_UNSUCCESS,
                    true));
        } else {
            if (file.canWrite() && file.delete()) {
                return true;
            } else {
                LogUtils.d(TAG, "deleteFile fail,file name = " + file.getName());
                publishProgress(new ProgressInfo(
                        OperationEventListener.ERROR_CODE_DELETE_NO_PERMISSION, true));
            }
        }
        return false;
    }

    protected boolean mkdir(HashMap<String, String> pathMap, File srcFile, File dstFile) {
        LogUtils.d(TAG, "mkdir,srcFile = " + srcFile + ",dstFile = " + dstFile);
        if (srcFile.exists() && srcFile.canRead() && dstFile.mkdirs()) {
            pathMap.put(srcFile.getAbsolutePath(), dstFile.getAbsolutePath());
            return true;
        } else {
            publishProgress(new ProgressInfo(OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS,
                    true));
            return false;
        }
    }

    private long calcNeedSpace(List<File> fileList) {
        long need = 0;
        for (File file : fileList) {
            need += file.length();
        }
        return need;
    }
    
    protected boolean isGreaterThan4G(UpdateInfo updateInfo) {
    	long size = updateInfo.getTotal();
    	if(size > (4L * 1024 * 1024 * 1024)) {
    		LogUtils.d(TAG, "isGreaterThan4G true.");
    		return true;
    	}
    	
    	LogUtils.d(TAG, "isGreaterThan4G false.");
    	return false;
    }
    
    protected boolean isFat32Disk(String path){
    	MountPointManager mpm = MountPointManager.getInstance();
    	return mpm.isFat32Disk(path);
    } 
    
    protected boolean isEnoughSpace(UpdateInfo updateInfo, String dstFolder) {
        LogUtils.d(TAG, "isEnoughSpace,dstFolder = " + dstFolder);
        long needSpace = updateInfo.getTotal();
        File file = new File(dstFolder);
        long freeSpace = file.getFreeSpace();
        if (needSpace > freeSpace) {
            return false;
        }
        return true;
    }

    protected int getAllDeleteFiles(List<FileInfo> fileInfoList, List<File> deleteList) {
       // LogUtils.d(TAG, "getAllDeleteFiles... ");
        int ret = OperationEventListener.ERROR_CODE_SUCCESS;
        for (FileInfo fileInfo : fileInfoList) {
            ret = getAllDeleteFile(fileInfo.getFile(), deleteList);
            if (ret < 0) {
                break;
            }
        }
        return ret;
    }

    protected int getAllDeleteFile(File deleteFile, List<File> deleteList) {
        //LogUtils.d(TAG, "getAllDeleteFile... ");
        if (isCancelled()) {
            LogUtils.i(TAG, "getAllDeleteFile,cancel. ");
            return OperationEventListener.ERROR_CODE_USER_CANCEL;
        }

        if (deleteFile.isDirectory()) {
            deleteList.add(0, deleteFile);
            if (deleteFile.canWrite()) {
                File[] files = deleteFile.listFiles();
                if (files == null) {
                    LogUtils.i(TAG, "getAllDeleteFile,files is null. ");
                    return OperationEventListener.ERROR_CODE_UNSUCCESS;
                }
                for (File file : files) {
                    getAllDeleteFile(file, deleteList);
                }
            }
        } else {
            deleteList.add(0, deleteFile);
        }
        return OperationEventListener.ERROR_CODE_SUCCESS;
    }

    protected int getAllFileList(List<FileInfo> srcList, List<File> resultList,
            UpdateInfo updateInfo) {
        int ret = OperationEventListener.ERROR_CODE_SUCCESS;
        for (FileInfo fileInfo : srcList) {
            ret = getAllFile(fileInfo.getFile(), resultList, updateInfo);
            if (ret < 0) {
                break;
            }
        }
        //LogUtils.d(TAG, "getAllFileList,ret = " + ret);

        return ret;
    }

    protected int getAllFile(File srcFile, List<File> fileList, UpdateInfo updateInfo) {
        // LogUtils.d(TAG, "getAllFile...");
        if (isCancelled()) {
            LogUtils.i(TAG, "getAllFile, cancel.");
            return OperationEventListener.ERROR_CODE_USER_CANCEL;
        }

        fileList.add(srcFile);
        updateInfo.updateTotal(srcFile.length());
        updateInfo.updateTotalNumber(1);
        if (srcFile.isDirectory() && srcFile.canRead()) {
            File[] files = srcFile.listFiles();
            if (files == null) {
                return OperationEventListener.ERROR_CODE_UNSUCCESS;
            }
            for (File file : files) {
                int ret = getAllFile(file, fileList, updateInfo);
                if (ret < 0) {
                    return ret;
                }
            }
        }
        return OperationEventListener.ERROR_CODE_SUCCESS;
    }

    protected int copyFile(byte[] buffer, File srcFile, File dstFile, UpdateInfo updateInfo) {
        if ((buffer == null) || (srcFile == null) || (dstFile == null)) {
            LogUtils.i(TAG, "copyFile, invalid parameter.");
            return OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS;
        }

        FileInputStream in = null;
        FileOutputStream out = null;
        int ret = OperationEventListener.ERROR_CODE_SUCCESS;
        try {
            if (!dstFile.createNewFile()) {
                LogUtils.i(TAG, "copyFile, create new file fail.");
                return OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS;
            }
            if (!srcFile.exists()) {
                LogUtils.i(TAG, "copyFile, src file is not exist.");
                return OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS;
            }
            in = new FileInputStream(srcFile);
            out = new FileOutputStream(dstFile);

            int len = 0;
            while ((len = in.read(buffer)) > 0) {
                // Copy data from in stream to out stream
                if (isCancelled()) {
                    LogUtils.d(TAG, "copyFile,commit copy file cancelled; " + "break while loop "
                            + "thread id: " + Thread.currentThread().getId());
                    if (!dstFile.delete()) {
                        LogUtils.w(TAG, "copyFile,delete fail in copyFile()");
                    }
                    return OperationEventListener.ERROR_CODE_USER_CANCEL;
                }
                out.write(buffer, 0, len);
               // LogUtils.i(TAG, "copyFile, copyFile,len= " + len);
                updateInfo.updateProgress(len);

                updateProgressWithTime(updateInfo, srcFile);
            }
        } catch (IOException ioException) {
            LogUtils.e(TAG, "copyFile,io exception!");
            ioException.printStackTrace();
            ret = OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS;
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
                if (out != null) {
                    out.close();
                }
            } catch (IOException ioException) {
                LogUtils.e(TAG, "copyFile,io exception 2!");
                ioException.printStackTrace();
                ret = OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS;
            } finally {
                LogUtils.d(TAG, "copyFile,update 100%.");
                publishProgress(new ProgressInfo(srcFile.getName(), TOTAL, TOTAL, (int) updateInfo
                        .getCurrentNumber(), updateInfo.getTotalNumber()));
            }
        }

        return ret;
    }

    File checkFileNameAndRename(File conflictFile) {
        File retFile = conflictFile;
        while (true) {
            if (isCancelled()) {
                LogUtils.i(TAG, "checkFileNameAndRename,cancel.");
                return null;
            }
            if (!retFile.exists()) {
                LogUtils.i(TAG, "checkFileNameAndRename,file is not exist.");
                return retFile;
            }
            retFile = FileUtils.genrateNextNewName(retFile);
            if (retFile == null) {
                LogUtils.i(TAG, "checkFileNameAndRename,retFile is null.");
                return null;
            }
        }
    }

    protected void updateProgressWithTime(UpdateInfo updateInfo, File file) {
        if (updateInfo.needUpdate()) {
            int progress = (int) (updateInfo.getProgress() * TOTAL / updateInfo.getTotal());
            publishProgress(new ProgressInfo(file.getName(), progress, TOTAL, (int) updateInfo
                    .getCurrentNumber(), updateInfo.getTotalNumber()));
        }
    }

    protected void addItem(HashMap<File, FileInfo> fileInfoMap, File file, File addFile) {
        if (fileInfoMap.containsKey(file)) {
            FileInfo fileInfo = new FileInfo(addFile);
            mFileInfoManager.addItem(fileInfo);
        }
    }

    protected void removeItem(HashMap<File, FileInfo> fileInfoMap, File file, File removeFile) {
        if (fileInfoMap.containsKey(file)) {
            FileInfo fileInfo = new FileInfo(removeFile);
            mFileInfoManager.removeItem(fileInfo);
        }
    }

    static class DeleteFilesTask extends FileOperationTask {
        private static final String TAG = "DeleteFilesTask";
        private final List<FileInfo> mDeletedFilesInfo;

        public DeleteFilesTask(FileInfoManager fileInfoManager,
                OperationEventListener operationEvent, Context context, List<FileInfo> fileInfoList) {
            super(fileInfoManager, operationEvent, context);
            mDeletedFilesInfo = fileInfoList;
        }

        @Override
        protected Integer doInBackground(Void... params) {
            synchronized (mContext.getApplicationContext()) {
                LogUtils.i(TAG, "doInBackground...");
                List<File> deletefileList = new ArrayList<File>();
                UpdateInfo updateInfo = new UpdateInfo();
                int ret = getAllDeleteFiles(mDeletedFilesInfo, deletefileList);
                if (ret < 0) {
                    LogUtils.i(TAG, "doInBackground,ret = " + ret);
                    return ret;
                }

                DeleteMediaStoreHelper deleteMediaStoreHelper = new DeleteMediaStoreHelper(
                        mMediaProviderHelper);
                HashMap<File, FileInfo> deleteFileInfoMap = new HashMap<File, FileInfo>();
                for (FileInfo fileInfo : mDeletedFilesInfo) {
                    deleteFileInfoMap.put(fileInfo.getFile(), fileInfo);
                }
                updateInfo.updateTotal(deletefileList.size());
                updateInfo.updateTotalNumber(deletefileList.size());

                publishProgress(new ProgressInfo("", (int) updateInfo.getProgress(), updateInfo
                        .getTotal(), (int) updateInfo.getCurrentNumber(), updateInfo
                        .getTotalNumber()));

                for (File file : deletefileList) {
                    if (isCancelled()) {
                        deleteMediaStoreHelper.updateRecords();
                        LogUtils.i(TAG, "doInBackground,user cancel it.");
                        return OperationEventListener.ERROR_CODE_USER_CANCEL;
                    }
                    if (deleteFile(file)) {
                        deleteMediaStoreHelper.addRecord(file.getAbsolutePath());
                        removeItem(deleteFileInfoMap, file, file);
                    }
                    updateInfo.updateProgress(1);
                    updateInfo.updateCurrentNumber(1);
                    if (updateInfo.needUpdate()) {
                        publishProgress(new ProgressInfo(file.getName(), (int) updateInfo
                                .getProgress(), updateInfo.getTotal(), (int) updateInfo
                                .getCurrentNumber(), updateInfo.getTotalNumber()));
                    }
                }
                deleteMediaStoreHelper.updateRecords();
                LogUtils.i(TAG, "doInBackground,return sucsess..");
                return OperationEventListener.ERROR_CODE_SUCCESS;
            }
        }
    }

    static class UpdateInfo {
        protected static final int NEED_UPDATE_TIME = 200;
        private long mStartOperationTime = 0;
        private long mProgressSize = 0;
        private long mTotalSize = 0;
        private long mCurrentNumber = 0;
        private long mTotalNumber = 0;

        public UpdateInfo() {
            mStartOperationTime = System.currentTimeMillis();
        }

        public long getProgress() {
            return mProgressSize;
        }

        public long getTotal() {
            return mTotalSize;
        }

        public long getCurrentNumber() {
            return mCurrentNumber;
        }

        public long getTotalNumber() {
            return mTotalNumber;
        }

        public void updateProgress(long addSize) {
            mProgressSize += addSize;
        }

        public void updateTotal(long addSize) {
            mTotalSize += addSize;
        }

        public void updateCurrentNumber(long addNumber) {
            mCurrentNumber += addNumber;
        }

        public void updateTotalNumber(long addNumber) {
            mTotalNumber += addNumber;
        }

        public boolean needUpdate() {
            long operationTime = System.currentTimeMillis() - mStartOperationTime;
            if (operationTime > NEED_UPDATE_TIME) {
                mStartOperationTime = System.currentTimeMillis();
                return true;
            }
            return false;
        }

    }

    static class CutPasteFilesTask extends FileOperationTask {
        private static final String TAG = "CutPasteFilesTask";
        private final List<FileInfo> mSrcList;
        private final String mDstFolder;

        /** Buffer size for data read and write. */

        public CutPasteFilesTask(FileInfoManager fileInfoManager,
                OperationEventListener operationEvent, Context context, List<FileInfo> src,
                String destFolder) {
            super(fileInfoManager, operationEvent, context);

            mSrcList = src;
            mDstFolder = destFolder;
        }

        @Override
        protected Integer doInBackground(Void... params) {
            synchronized (mContext.getApplicationContext()) {
                LogUtils.i(TAG, "doInBackground...");
                if (mSrcList.isEmpty()) {
                    LogUtils.i(TAG, "doInBackground,src list is empty.");
                    return OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS;
                }

                if (isSameRoot(mSrcList.get(0).getFileAbsolutePath(), mDstFolder)) {
                    return cutPasteInSameCard();
                } else {
                    return cutPasteInDiffCard();
                }
            }
        }

        private boolean isSameRoot(String srcPath, String dstPath) {
            MountPointManager mpm = MountPointManager.getInstance();
            String srcMountPoint = mpm.getRealMountPointPath(srcPath);
            String dstMountPoint = mpm.getRealMountPointPath(dstPath);
            if (srcMountPoint != null && dstMountPoint != null
                    && srcMountPoint.equals(dstMountPoint)) {
                return true;
            }
            return false;
        }

        private Integer cutPasteInSameCard() {
            LogUtils.i(TAG, "cutPasteInSameCard.");
            UpdateInfo updateInfo = new UpdateInfo();
            updateInfo.updateTotal(mSrcList.size());
            updateInfo.updateTotalNumber(mSrcList.size());
            /** Unnecessary to show process when cut in the same card*/
            publishProgress(new ProgressInfo("", 0, TOTAL, 0, mSrcList.size()));

            PasteMediaStoreHelper pasteMediaStoreHelper = new PasteMediaStoreHelper(
                    mMediaProviderHelper);
            DeleteMediaStoreHelper deleteMediaStoreHelper = new DeleteMediaStoreHelper(
                    mMediaProviderHelper);

            // Set dstFolder so we can scan folder instead of scanning each file one by one.
            pasteMediaStoreHelper.setDstFolder(mDstFolder);

            for (FileInfo fileInfo : mSrcList) {
                File newFile = new File(mDstFolder + MountPointManager.SEPARATOR
                        + fileInfo.getFileName());
                newFile = checkFileNameAndRename(newFile);
                if (isCancelled()) {
                    pasteMediaStoreHelper.updateRecords();
                    deleteMediaStoreHelper.updateRecords();
                    return OperationEventListener.ERROR_CODE_USER_CANCEL;
                }

                if (newFile == null) {
                    LogUtils.i(TAG, "cutPasteInSameCard,newFile is null.");
                    publishProgress(new ProgressInfo(
                            OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS, true));
                    continue;
                }

                if (fileInfo.getFile().renameTo(newFile)) {
                    updateInfo.updateProgress(1);
                    updateInfo.updateCurrentNumber(1);
                    FileInfo newFileInfo = new FileInfo(newFile);
                    mFileInfoManager.addItem(newFileInfo);
                    // we need to update the records(uri,is_rintone,etc)in
                    // mediaprovider by recreating them.
                    // mMediaProviderHelper.updateInMediaStore(newFile.getAbsolutePath(),
                    // fileInfo.getFileAbsolutePath());
                    if (newFile.isDirectory()) {
                        //if cut directory,update the files in this directory also.
                        mMediaProviderHelper.updateInMediaStore(newFileInfo.getFileAbsolutePath(),
                                fileInfo.getFileAbsolutePath());
                    } else {
                        //if cut file,add it to the pasteMediaStoreHelper
                        deleteMediaStoreHelper.addRecord(fileInfo.getFileAbsolutePath());
                        pasteMediaStoreHelper.addRecord(newFile.getAbsolutePath());
                    }
                } else {
                    publishProgress(new ProgressInfo(
                            OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS, true));
                }
                /** Unnecessary to show process when cut in the same card*/
                updateProgressWithTime(updateInfo, fileInfo.getFile());
            }

            pasteMediaStoreHelper.updateRecords();
            deleteMediaStoreHelper.updateRecords();

            int currentNumber = (int)updateInfo.getCurrentNumber();
            long totalNumber = updateInfo.getTotalNumber();
            publishProgress(new ProgressInfo("", TOTAL, TOTAL, currentNumber, totalNumber));

            return OperationEventListener.ERROR_CODE_SUCCESS;
        }

        private Integer cutPasteInDiffCard() {
            LogUtils.i(TAG, "cutPasteInDiffCard...");
            int ret = OperationEventListener.ERROR_CODE_SUCCESS;
            List<File> fileList = new ArrayList<File>();
            UpdateInfo updateInfo = new UpdateInfo();
            ret = getAllFileList(mSrcList, fileList, updateInfo);
            if (ret < 0) {
                LogUtils.i(TAG, "cutPasteInDiffCard,ret = " + ret);
                return ret;
            }

            if(isGreaterThan4G(updateInfo) && isFat32Disk(mDstFolder)) {
            	LogUtils.i(TAG, "cutPasteInDiffCard, destination is FAT32.");
            	return OperationEventListener.ERROR_CODE_COPY_GREATER_4G_TO_FAT32;
            }

            if (!isEnoughSpace(updateInfo, mDstFolder)) {
                LogUtils.i(TAG, "cutPasteInDiffCard,not enough space.");
                return OperationEventListener.ERROR_CODE_NOT_ENOUGH_SPACE;
            }

            List<File> romoveFolderFiles = new LinkedList<File>();
            publishProgress(new ProgressInfo("", 0, TOTAL, 0, fileList.size()));
            byte[] buffer = new byte[BUFFER_SIZE];
            HashMap<String, String> pathMap = new HashMap<String, String>();
            if (!fileList.isEmpty()) {
                pathMap.put(fileList.get(0).getParent(), mDstFolder);
            }

            PasteMediaStoreHelper pasteMediaStoreHelper = new PasteMediaStoreHelper(
                    mMediaProviderHelper);
            DeleteMediaStoreHelper deleteMediaStoreHelper = new DeleteMediaStoreHelper(
                    mMediaProviderHelper);

            // Set dstFolder so we can scan folder instead of scanning each file one by one.
            pasteMediaStoreHelper.setDstFolder(mDstFolder);

            HashMap<File, FileInfo> cutFileInfoMap = new HashMap<File, FileInfo>();
            for (FileInfo fileInfo : mSrcList) {
                cutFileInfoMap.put(fileInfo.getFile(), fileInfo);
            }

            for (File file : fileList) {
                File dstFile = getDstFile(pathMap, file, mDstFolder);
                if (isCancelled()) {
                    pasteMediaStoreHelper.updateRecords();
                    deleteMediaStoreHelper.updateRecords();
                    LogUtils.i(TAG, "cutPasteInDiffCard,user cancel.");
                    return OperationEventListener.ERROR_CODE_USER_CANCEL;
                }
                if (dstFile == null) {
                    publishProgress(new ProgressInfo(
                            OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS, true));
                    continue;
                }

                if (file.isDirectory()) {
                    if (mkdir(pathMap, file, dstFile)) {
                        pasteMediaStoreHelper.addRecord(dstFile.getAbsolutePath());
                        addItem(cutFileInfoMap, file, dstFile);
                        updateInfo.updateProgress(file.length());
                        updateInfo.updateCurrentNumber(1);
                        romoveFolderFiles.add(0, file);
                        updateProgressWithTime(updateInfo, file);
                    }
                } else {
                    updateInfo.updateCurrentNumber(1);
                    ret = copyFile(buffer, file, dstFile, updateInfo);
                    LogUtils.i(TAG, "cutPasteInDiffCard ret2 = " + ret);
                    if (ret == OperationEventListener.ERROR_CODE_USER_CANCEL) {
                        pasteMediaStoreHelper.updateRecords();
                        deleteMediaStoreHelper.updateRecords();
                        return ret;
                    } else if (ret < 0) {
                        publishProgress(new ProgressInfo(
                                OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS, true));
                        updateInfo.updateProgress(file.length());
                        updateInfo.updateCurrentNumber(1);
                    } else {
                        addItem(cutFileInfoMap, file, dstFile);
                        pasteMediaStoreHelper.addRecord(dstFile.getAbsolutePath());
                        if (deleteFile(file)) {
                            deleteMediaStoreHelper.addRecord(file.getAbsolutePath());
                        }
                    }
                }
            }

            for (File file : romoveFolderFiles) {
                if (file.delete()) {
                    deleteMediaStoreHelper.addRecord(file.getAbsolutePath());
                }
            }
            pasteMediaStoreHelper.updateRecords();
            deleteMediaStoreHelper.updateRecords();
            LogUtils.i(TAG, "cutPasteInDiffCard,return success.");
            return OperationEventListener.ERROR_CODE_SUCCESS;
        }
    }

    static class CopyPasteFilesTask extends FileOperationTask {
        private static final String TAG = "CopyPasteFilesTask";
        List<FileInfo> mSrcList = null;
        String mDstFolder = null;

        public CopyPasteFilesTask(FileInfoManager fileInfoManager,
                OperationEventListener operationEvent, Context context, List<FileInfo> src,
                String destFolder) {
            super(fileInfoManager, operationEvent, context);
            mSrcList = src;
            mDstFolder = destFolder;
        }

        @Override
        protected Integer doInBackground(Void... params) {
            synchronized (mContext.getApplicationContext()) {
                LogUtils.i(TAG, "doInBackground...");
               //final long beforeTime  = System.currentTimeMillis();
                List<File> fileList = new ArrayList<File>();
                UpdateInfo updateInfo = new UpdateInfo();
                int ret = getAllFileList(mSrcList, fileList, updateInfo);
                if (ret < 0) {
                    LogUtils.i(TAG, "doInBackground,ret = " + ret);
                    return ret;
                }

                PasteMediaStoreHelper pasteMediaStoreHelper = new PasteMediaStoreHelper(
                        mMediaProviderHelper);

                // Set dstFolder so we can scan folder instead of scanning each file one by one.
                pasteMediaStoreHelper.setDstFolder(mDstFolder);

                HashMap<File, FileInfo> copyFileInfoMap = new HashMap<File, FileInfo>();
                for (FileInfo fileInfo : mSrcList) {
                    copyFileInfoMap.put(fileInfo.getFile(), fileInfo);
                }

                if(isGreaterThan4G(updateInfo) && isFat32Disk(mDstFolder)) {
                	LogUtils.i(TAG, "doInBackground, destination is FAT32.");
                	return OperationEventListener.ERROR_CODE_COPY_GREATER_4G_TO_FAT32;
                }
                
                if (!isEnoughSpace(updateInfo, mDstFolder)) {
                    LogUtils.i(TAG, "doInBackground, not enough space.");
                    return OperationEventListener.ERROR_CODE_NOT_ENOUGH_SPACE;
                }

                publishProgress(new ProgressInfo("", 0, TOTAL, 0, updateInfo.getTotalNumber()));

                byte[] buffer = new byte[BUFFER_SIZE];
                HashMap<String, String> pathMap = new HashMap<String, String>();
                if (!fileList.isEmpty()) {
                    pathMap.put(fileList.get(0).getParent(), mDstFolder);
                }
                for (File file : fileList) {
                    File dstFile = getDstFile(pathMap, file, mDstFolder);
                    if (isCancelled()) {
                        pasteMediaStoreHelper.updateRecords();
                        LogUtils.i(TAG, "doInBackground,user cancel.");
                        return OperationEventListener.ERROR_CODE_USER_CANCEL;
                    }
                    if (dstFile == null) {
                        publishProgress(new ProgressInfo(
                                OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS, true));
                        continue;
                    }
                    if (file.isDirectory()) {
                        if (mkdir(pathMap, file, dstFile)) {
                            pasteMediaStoreHelper.addRecord(dstFile.getAbsolutePath());
                            addItem(copyFileInfoMap, file, dstFile);
                            updateInfo.updateProgress(file.length());
                            updateInfo.updateCurrentNumber(1);
                            updateProgressWithTime(updateInfo, file);
                        }
                    } else {
                        if (FileInfo.isDrmFile(file.getName()) || !file.canRead()) {
                            publishProgress(new ProgressInfo(
                                    OperationEventListener.ERROR_CODE_COPY_NO_PERMISSION, true));
                            updateInfo.updateProgress(file.length());
                            updateInfo.updateCurrentNumber(1);
                            continue;
                        }
                        updateInfo.updateCurrentNumber(1);
                        ret = copyFile(buffer, file, dstFile, updateInfo);
                        if (ret == OperationEventListener.ERROR_CODE_USER_CANCEL) {
                            pasteMediaStoreHelper.updateRecords();
                            return ret;
                        } else if (ret < 0) {
                            publishProgress(new ProgressInfo(ret, true));
                            updateInfo.updateProgress(file.length());
                            updateInfo.updateCurrentNumber(1);
                        } else {
                            pasteMediaStoreHelper.addRecord(dstFile.getAbsolutePath());
                            addItem(copyFileInfoMap, file, dstFile);
                        }
                    }
                }
                pasteMediaStoreHelper.updateRecords();
                LogUtils.i(TAG, "doInBackground,return success.");
                //final long endTime  = System.currentTimeMillis();
               // LogUtils.i(TAG, "doInBackground, ret 2 = " + ret + ",time cost is:" + (endTime-beforeTime)/1000);
                return OperationEventListener.ERROR_CODE_SUCCESS;
            }
        }
    }

    static class CreateFolderTask extends FileOperationTask {
        private static final String TAG = "CreateFolderTask";
        private final String mDstFolder;
        int mFilterType;

        public CreateFolderTask(FileInfoManager fileInfoManager,
                OperationEventListener operationEvent, Context context, String dstFolder,
                int filterType) {
            super(fileInfoManager, operationEvent, context);
            mDstFolder = dstFolder;
            mFilterType = filterType;
        }

        @Override
        protected Integer doInBackground(Void... params) {
            synchronized (mContext.getApplicationContext()) {
                int ret = OperationEventListener.ERROR_CODE_UNSUCCESS;
                LogUtils.i(TAG, "doInBackground...");
                ret = FileUtils.checkFileName(FileUtils.getFileName(mDstFolder));
                if (ret < 0) {
                    LogUtils.i(TAG, "doInBackground,ret = " + ret);
                    return ret;
                }
                String dstFile = mDstFolder.trim();
                LogUtils.d(TAG, "Create a new folder,dstFile=" + dstFile);
                File dir = new File(dstFile);
                LogUtils.d(TAG, "The folder to be created exist: " + dir.exists());
                if (dir.exists()) {
                    LogUtils.i(TAG, "doInBackground,dir is exist.");
                    return OperationEventListener.ERROR_CODE_FILE_EXIST;
                }

                File path = new File(FileUtils.getFilePath(mDstFolder));
                if (path.getFreeSpace() <= 0) {
                    LogUtils.i(TAG, "doInBackground,not enough space.");
                    return OperationEventListener.ERROR_CODE_NOT_ENOUGH_SPACE;
                }
                if (dstFile.endsWith(".")) {
                    // our platform support vfat which doesn't allow the
                    // file/folder name end with '.'
                    LogUtils.i(TAG, "doInBackground,end with dot.");
                    while (dstFile.endsWith(".")) {
                        dstFile = dstFile.substring(0, dstFile.length() - 1);
                    }
                    dir = new File(dstFile);
                }

                if (dir.mkdirs()) {
                    FileInfo fileInfo = new FileInfo(dir);
                    if (!fileInfo.isHideFile()
                            || mFilterType == FileManagerService.FILE_FILTER_TYPE_ALL) {
                        mFileInfoManager.addItem(fileInfo);
                    }
                    mMediaProviderHelper.scanPathforMediaStore(fileInfo.getFileAbsolutePath());
                    LogUtils.i(TAG, "doInBackground, mkdir return success.");
                    return OperationEventListener.ERROR_CODE_SUCCESS;
                }

                return OperationEventListener.ERROR_CODE_UNSUCCESS;
            }
        }
    }

    static class RenameTask extends FileOperationTask {
        private static final String TAG = "RenameTask";
        private final FileInfo mDstFileInfo;
        private final FileInfo mSrcFileInfo;
        int mFilterType = 0;

        public RenameTask(FileInfoManager fileInfoManager, OperationEventListener operationEvent,
                Context context, FileInfo srcFile, FileInfo dstFile, int filterType) {
            super(fileInfoManager, operationEvent, context);
            mDstFileInfo = dstFile;
            mSrcFileInfo = srcFile;
            mFilterType = filterType;
        }

        @Override
        protected Integer doInBackground(Void... params) {
            synchronized (mContext.getApplicationContext()) {
                int ret = OperationEventListener.ERROR_CODE_UNSUCCESS;
                LogUtils.i(TAG, "doInBackground...");
                String dstFile = mDstFileInfo.getFileAbsolutePath();
                dstFile = dstFile.trim();
                LogUtils.d(TAG, "rename dstFile = " + dstFile);
                ret = FileUtils.checkFileName(FileUtils.getFileName(dstFile));
                if (ret < 0) {
                    LogUtils.i(TAG, "doInBackground,ret = " + ret);
                    return ret;
                }

                File newFile = new File(dstFile);
                File oldFile = new File(mSrcFileInfo.getFileAbsolutePath());

                if (newFile.exists()) {
                    LogUtils.i(TAG, "doInBackground,new file is exist.");
                    return OperationEventListener.ERROR_CODE_FILE_EXIST;
                    // } else if (oldFile.isFile() && dstFile.endsWith(".")) {
                } else if (dstFile.endsWith(".")) {
                    // our platform support vfat which doesn't allow the
                    // file/folder name end with '.'
                    LogUtils.i(TAG, "doInBackground,end with dot.");
                    while (dstFile.endsWith(".")) {
                        dstFile = dstFile.substring(0, dstFile.length() - 1);
                    }
                    newFile = new File(dstFile);
                }

                if (oldFile.renameTo(newFile)) {
                    FileInfo newFileInfo = new FileInfo(newFile);
                    mFileInfoManager.removeItem(mSrcFileInfo);
                    if (!newFileInfo.isHideFile()
                            || mFilterType == FileManagerService.FILE_FILTER_TYPE_ALL) {
                        mFileInfoManager.addItem(newFileInfo);
                    }
                    mMediaProviderHelper.updateInMediaStore(newFileInfo.getFileAbsolutePath(),
                            mSrcFileInfo.getFileAbsolutePath());
                    LogUtils.i(TAG, "doInBackground,return success.");
                    return OperationEventListener.ERROR_CODE_SUCCESS;
                }

                return OperationEventListener.ERROR_CODE_UNSUCCESS;
            }
        }
    }
}
