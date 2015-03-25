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

import android.app.Service;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.IBinder;
import android.provider.MediaStore;

import com.mediatek.filemanager.FileInfo;
import com.mediatek.filemanager.FileInfoManager;
import com.mediatek.filemanager.MountPointManager;
import com.mediatek.filemanager.service.FileOperationTask.CopyPasteFilesTask;
import com.mediatek.filemanager.service.FileOperationTask.CreateFolderTask;
import com.mediatek.filemanager.service.FileOperationTask.CutPasteFilesTask;
import com.mediatek.filemanager.service.FileOperationTask.DeleteFilesTask;
import com.mediatek.filemanager.service.FileOperationTask.RenameTask;
import com.mediatek.filemanager.utils.DrmManager;
import com.mediatek.filemanager.utils.LogUtils;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;


public class FileManagerService extends Service {

    public static final int FILE_FILTER_TYPE_UNKOWN = -1;
    public static final int FILE_FILTER_TYPE_DEFAULT = 0;
    public static final int FILE_FILTER_TYPE_FOLDER = 1;
    public static final int FILE_FILTER_TYPE_ALL = 2;

    private static final String TAG = "FileManagerService";
    private final HashMap<String, FileManagerActivityInfo> mActivityMap = 
        new HashMap<String, FileManagerActivityInfo>();
    private ServiceBinder mBinder = null;

    private static class FileManagerActivityInfo {
        private BaseAsyncTask mTask = null;
        private FileInfoManager mFileInfoManager = null;
        private int mFilterType = FILE_FILTER_TYPE_DEFAULT;

        public void setTask(BaseAsyncTask task) {
            this.mTask = task;
        }

        public void setFileInfoManager(FileInfoManager fileInfoManager) {
            this.mFileInfoManager = fileInfoManager;
        }

        public void setFilterType(int filterType) {
            this.mFilterType = filterType;
        }

        BaseAsyncTask getTask() {
            return mTask;
        }

        FileInfoManager getFileInfoManager() {
            return mFileInfoManager;
        }

        int getFilterType() {
            return mFilterType;
        }
    }

    public interface OperationEventListener {
        int ERROR_CODE_NAME_VALID = 100;
        int ERROR_CODE_SUCCESS = 0;

        int ERROR_CODE_UNSUCCESS = -1;
        int ERROR_CODE_NAME_EMPTY = -2;
        int ERROR_CODE_NAME_TOO_LONG = -3;
        int ERROR_CODE_FILE_EXIST = -4;
        int ERROR_CODE_NOT_ENOUGH_SPACE = -5;
        int ERROR_CODE_DELETE_FAILS = -6;
        int ERROR_CODE_USER_CANCEL = -7;
        int ERROR_CODE_PASTE_TO_SUB = -8;
        int ERROR_CODE_UNKOWN = -9;
        int ERROR_CODE_COPY_NO_PERMISSION = -10;
        int ERROR_CODE_MKDIR_UNSUCCESS = -11;
        int ERROR_CODE_CUT_SAME_PATH = -12;
        int ERROR_CODE_BUSY = -100;
        int ERROR_CODE_DELETE_UNSUCCESS = -13;
        int ERROR_CODE_PASTE_UNSUCCESS = -14;
        int ERROR_CODE_DELETE_NO_PERMISSION = -15;
        int ERROR_CODE_COPY_GREATER_4G_TO_FAT32 = -16;

        /**
         * This method will be implemented, and called in onPreExecute of asynctask
         */
        void onTaskPrepare();

        /**
         * This method will be implemented, and called in onProgressUpdate function
         * of asynctask
         * 
         * @param progressInfo information of ProgressInfo, which will be updated on UI
         */
        void onTaskProgress(ProgressInfo progressInfo);

        /**
         * This method will be implemented, and called in onPostExecute of asynctask
         * 
         * @param result the result of asynctask's doInBackground()
         */
        void onTaskResult(int result);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mBinder = new ServiceBinder();
        AsyncTask.setDefaultExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
       LogUtils.d(TAG, "onStartCommand...");
       super.onStartCommand(intent, flags, startId);
       return START_NOT_STICKY;
    }

    public class ServiceBinder extends Binder {
        /**
         * This method gets instance of FileManagerService
         * @return instance of FileManagerService
         */
        public FileManagerService getServiceInstance() {
            return FileManagerService.this;
        }
    }

    /**
     * This method initializes FileInfoManager of certain activity.
     * 
     * @param activityName name of activity, which the FileInfoManager attached to
     * @return FileInforManager of certain activity
     */
    public FileInfoManager initFileInfoManager(String activityName) {
        FileManagerActivityInfo activityInfo = mActivityMap.get(activityName);
        if (activityInfo == null) {
            activityInfo = new FileManagerActivityInfo();
            activityInfo.setFileInfoManager(new FileInfoManager());
            mActivityMap.put(activityName, activityInfo);
        }
        return activityInfo.getFileInfoManager();
    }

    @Override
    public IBinder onBind(Intent intent) {
        DrmManager.getInstance().init(this);
        return mBinder;
    }

    /**
     * This method checks that weather the service is busy or not, which means id any task exist for
     * certain activity
     * 
     * @param activityName name of activity, which will be checked
     * @return true for busy, false for not busy
     */
    public boolean isBusy(String activityName) {
        boolean ret = false;
        FileManagerActivityInfo activityInfo = mActivityMap.get(activityName);
        if (activityInfo == null) {
            LogUtils.w(TAG, "isBusy return false,because activityInfo is null!");
            return ret;
        }
        BaseAsyncTask task = activityInfo.getTask();
        if (task != null) {
            return task.isTaskBusy();
        }
        return false;
    }

    private FileManagerActivityInfo getActivityInfo(String activityName) {
        FileManagerActivityInfo activityInfo = mActivityMap.get(activityName);
        if (activityInfo == null) {
            throw new IllegalArgumentException(
                    "this activity not init in Service");
        }
        return activityInfo;
    }

    /**
     * This method sets list filter, which which type of items will be listed in listView
     * 
     * @param type type of list filter
     * @param activityName name of activity, which operations attached to
     */
    public void setListType(int type, String activityName) {
        getActivityInfo(activityName).setFilterType(type);
    }

    /**
     * This method does create folder job by starting a new CreateFolderTask
     * 
     * @param activityName name of activity, which the CreateFolderTask attached to
     * @param destFolder information of file, which needs to be created
     * @param listener listener of CreateFolderTask
     */
    public void createFolder(String activityName, String destFolder,
            OperationEventListener listener) {
        LogUtils.d(TAG, " createFolder Start ");
        if (isBusy(activityName)) {
            listener.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
        } else {
            FileInfoManager fileInfoManager = getActivityInfo(activityName)
                    .getFileInfoManager();
            int filterType = getActivityInfo(activityName).getFilterType();
            if (fileInfoManager != null) {
                BaseAsyncTask task = new CreateFolderTask(fileInfoManager,
                        listener, this, destFolder, filterType);
                getActivityInfo(activityName).setTask(task);
                task.execute();
            }
        }
    }

    /**
     * This method does rename job by starting a new RenameTask
     * 
     * @param activityName name of activity, which the operations attached to
     * @param srcFile information of certain file, which needs to be renamed
     * @param dstFile information of new file after rename
     * @param listener listener of RenameTask
     */
    public void rename(String activityName, FileInfo srcFile, FileInfo dstFile,
            OperationEventListener listener) {
        LogUtils.d(TAG, " rename Start,activityName = " + activityName);

        if (isBusy(activityName)) {
            listener.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
        } else {
            FileInfoManager fileInfoManager = getActivityInfo(activityName)
                    .getFileInfoManager();
            int filterType = getActivityInfo(activityName).getFilterType();
            if (fileInfoManager != null) {
                BaseAsyncTask task = new RenameTask(fileInfoManager, listener,
                        this, srcFile, dstFile, filterType);
                getActivityInfo(activityName).setTask(task);
                task.execute();
            }
        }
    }

    private int filterPasteList(List<FileInfo> fileInfoList, String destFolder) {

        int remove = 0;
        Iterator<FileInfo> iterator = fileInfoList.iterator();
        while (iterator.hasNext()) {
            FileInfo fileInfo = iterator.next();
            if (fileInfo.isDirectory()) {
                if ((destFolder + MountPointManager.SEPARATOR)
                        .startsWith(fileInfo.getFileAbsolutePath()
                                + MountPointManager.SEPARATOR)) {
                    iterator.remove();
                    remove++;
                }
            }
        }
        return remove;
    }

    /**
     * This method does delete job by starting a new DeleteFilesTask.
     * 
     * @param activityName name of activity, which the operations attached to
     * @param fileInfoList list of files, which needs to be deleted
     * @param listener listener of the DeleteFilesTask
     */
    public void deleteFiles(String activityName, List<FileInfo> fileInfoList,
            OperationEventListener listener) {
        LogUtils.d(TAG, " deleteFiles Start,activityName = " + activityName);
        if (isBusy(activityName)) {
            listener.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
        } else {
            FileInfoManager fileInfoManager = getActivityInfo(activityName)
                    .getFileInfoManager();
            if (fileInfoManager != null) {
                BaseAsyncTask task = new DeleteFilesTask(fileInfoManager,
                        listener, this, fileInfoList);
                getActivityInfo(activityName).setTask(task);
                task.execute();
            }
        }
    }

    /**
     * This method cancel certain task
     * 
     * @param activityName name of activity, which the task attached to
     */
    public void cancel(String activityName) {
        LogUtils.d(TAG, " cancel service,activityName = " + activityName);
        BaseAsyncTask task = getActivityInfo(activityName).getTask();
        if (task != null) {
            task.cancel(true);
        }
    }

    /**
     * This method does paste job by starting a new CutPasteFilesTask or CopyPasteFilesTask according
     * to parameter of type
     * 
     * @param activityName name of activity, which the task and operations attached to
     * @param fileInfoList list of files which needs to be paste
     * @param dstFolder destination, which the files should be paste to
     * @param type indicate the previous operation is cut or copy
     * @param listener listener of the started task
     */
    public void pasteFiles(String activityName, List<FileInfo> fileInfoList,
            String dstFolder, int type, OperationEventListener listener) {
        LogUtils.d(TAG, " pasteFiles Start,activityName = " + activityName);
        if (isBusy(activityName)) {
            listener.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
            return;
        }
        if (filterPasteList(fileInfoList, dstFolder) > 0) {
            listener.onTaskResult(OperationEventListener.ERROR_CODE_PASTE_TO_SUB);
        }
        FileInfoManager fileInfoManager = getActivityInfo(activityName)
                .getFileInfoManager();
        if (fileInfoManager == null) {
            LogUtils.w(TAG, "mFileInfoManagerMap.get FileInfoManager = null");
            listener.onTaskResult(OperationEventListener.ERROR_CODE_UNKOWN);
            return;
        }
        BaseAsyncTask task = null;
        if (fileInfoList.size() > 0) {
            switch (type) {
            case FileInfoManager.PASTE_MODE_CUT:
                if (isCutSamePath(fileInfoList, dstFolder)) {
                        listener.onTaskResult(OperationEventListener.ERROR_CODE_CUT_SAME_PATH);
                    return;
                }
                task = new CutPasteFilesTask(fileInfoManager, listener, getApplicationContext(),
                        fileInfoList, dstFolder);
                getActivityInfo(activityName).setTask(task);
                task.execute();
                break;
            case FileInfoManager.PASTE_MODE_COPY:
                task = new CopyPasteFilesTask(fileInfoManager, listener, getApplicationContext(),
                        fileInfoList, dstFolder);
                getActivityInfo(activityName).setTask(task);
                task.execute();
                break;
            default:
                listener.onTaskResult(OperationEventListener.ERROR_CODE_UNKOWN);
                return;
            }

        }
    }

    private boolean isCutSamePath(List<FileInfo> fileInfoList, String dstFolder) {
        for (FileInfo fileInfo : fileInfoList) {
            if (fileInfo.getFileParentPath().equals(dstFolder)) {
                return true;
            }
        }
        return false;
    }

    /**
     * This method lists files of certain directory by starting a new ListFileTask.
     * 
     * @param activityName name of activity, which the ListFileTask attached to
     * @param path the path of certain directory
     * @param listener listener of the ListFileTask
     */
    public void listFiles(String activityName, String path, OperationEventListener listener) {
        LogUtils.d(TAG, "listFiles,activityName = " + activityName + ",path = " + path);
        /*if (isBusy(activityName)) {
            LogUtils.d(TAG, "listFiles,task is busy. ");
            listener.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
       } else {*/
       if (isBusy(activityName)) {
           LogUtils.d(TAG, "listFiles, cancel other background task...");
           BaseAsyncTask task = getActivityInfo(activityName).getTask();
           if (task != null) {
               task.removeListener();
               task.cancel(true);
           }
       }
       LogUtils.d(TAG, "listFiles,do list.");
       FileInfoManager fileInfoManager = getActivityInfo(activityName).getFileInfoManager();
       int filterType = getActivityInfo(activityName).getFilterType();
       if (fileInfoManager != null) {
           LogUtils.d(TAG, "listFiles fiterType = " + filterType);
           BaseAsyncTask task = new ListFileTask(getApplicationContext(), fileInfoManager, listener, path, filterType);
           getActivityInfo(activityName).setTask(task);
           task.execute();
       }
       // }
    }

    /**
     * This method gets detail information of a file (or directory) by starting a new
     * DetailInfotask.
     * 
     * @param activityName name of activity, which the task and operations attached to
     * @param file a certain file (or directory)
     * @param listener listener of the DetailInfotask
     */
    public void getDetailInfo(String activityName, FileInfo file,
            OperationEventListener listener) {
        LogUtils.d(TAG, "getDetailInfo,activityName = " + activityName);
        if (isBusy(activityName)) {
            listener.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
        } else {
            FileInfoManager fileInfoManager = getActivityInfo(activityName)
                    .getFileInfoManager();
            if (fileInfoManager != null) {
                BaseAsyncTask task = new DetailInfoTask(fileInfoManager,
                        listener, file);
                getActivityInfo(activityName).setTask(task);
                task.execute();
            }
        }
    }

    /**
     * This method removes listener from task when service disconnected.
     * 
     * @param activityName name of activity, which the task attached to
     */
    public void disconnected(String activityName) {
        LogUtils.d(TAG, "disconnected,activityName = " + activityName);
        BaseAsyncTask task = getActivityInfo(activityName).getTask();
        if (task != null) {
            task.removeListener();
        }
    }

    /**
     * This method reconnects to the running task by setting a new listener to the task, when dialog
     * is destroyed and recreated
     * 
     * @param activityName name of activity, which the task and dialog attached to
     * @param listener new listener for the task and dialog
     */
    public void reconnected(String activityName, OperationEventListener listener) {
        LogUtils.d(TAG, "reconnected,activityName = " + activityName);
        BaseAsyncTask task = getActivityInfo(activityName).getTask();
        if (task != null) {
            task.setListener(listener);
        }
    }

     /**
     * This method return whether background task is get detail info task.
     * @param activityName name of activity, which the task attached to
     * @return true if background task is detail info task, others false.
     */
    public boolean isDetailTask(String activityName) {
        BaseAsyncTask task = mActivityMap.get(activityName).getTask();
        if (task != null && task instanceof DetailInfoTask) {
            return true;
        }
        return false;
    }

    /**
     * M:#3gp#
     * A 3gpp file could be video type or audio type. The method try to find out its real MIME type
     * from database of MediaStore.
     * 
     * @param fileInfo information of a file
     * @return the file's real MIME type
     */
//    public String update3gppMimetype(FileInfo fileInfo) {
//        LogUtils.d(TAG, "update3gppMimetype...");
//        String mimeType = FileInfo.MIMETYPE_3GPP_VIDEO;
//        ContentResolver resolver = getContentResolver();
//        if (resolver != null && fileInfo != null) {
//           // fileInfo.setFileMimeType(FileInfo.MIMETYPE_3GPP_VIDEO);
//            final Uri uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
//            final String[] projection = new String[] { MediaStore.MediaColumns.MIME_TYPE };
//            final String selection = MediaStore.MediaColumns.DATA + "=?";
//            final String[] selectionArgs = new String[] { fileInfo
//                    .getFileAbsolutePath() };
//            Cursor cursor = null;
//            try {
//                cursor = resolver.query(uri, projection, selection,
//                        selectionArgs, null);
//                LogUtils.d(TAG, "update3gppMimetype,file:" + fileInfo.getFileAbsolutePath());
//                if (cursor != null && cursor.moveToFirst()) {
//                     mimeType = cursor.getString(cursor
//                            .getColumnIndex(MediaStore.MediaColumns.MIME_TYPE));
//                    LogUtils.d(TAG, "update3gppMimetype mimeType: " + mimeType);
//                } else {
//                    mimeType = fileInfo.getFileOriginMimeType();
//                    LogUtils.d(TAG, "update3gppMimetype, not find from media,origin mimeType: " + mimeType);
//                    if (mimeType == null) {
//                        mimeType = FileInfo.MIMETYPE_3GPP_VIDEO;
//                    }
//                }
//            } finally {
//                if (cursor != null) {
//                    cursor.close();
//                }
//            }
//        }
//
//        return mimeType;
//    }

    /**
     * This method do search job by starting a new search task
     * 
     * @param activityName name of activity which starts the search
     * @param searchName the search target
     * @param path the path to limit the search in
     * @param operationEvent the listener corresponds to this search task
     */
    public void search(String activityName, String searchName, String path,
            OperationEventListener operationEvent) {
        LogUtils.d(TAG, "search, activityName = " + activityName + ",searchName = " + searchName + ",path = " + path);
        if (isBusy(activityName)) {
            cancel(activityName);
            // operationEvent.onTaskResult(OperationEventListener.ERROR_CODE_BUSY);
        } else {
            FileInfoManager fileInfoManager = getActivityInfo(activityName)
                    .getFileInfoManager();
            if (fileInfoManager != null) {
                BaseAsyncTask task = new SearchTask(fileInfoManager,
                        operationEvent, searchName, path, getContentResolver());
                getActivityInfo(activityName).setTask(task);
                task.execute();
            }
        }
    }

}
