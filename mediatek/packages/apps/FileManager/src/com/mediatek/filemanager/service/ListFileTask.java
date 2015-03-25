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
import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.PDebug;

import java.io.File;
import java.util.ArrayList;
import java.util.List;


class ListFileTask extends BaseAsyncTask {
    private static final String TAG = "ListFileTask";
    private final String mPath;
    private final int mFilterType;
    private Context mContext;
    private static final int FIRST_NEED_PROGRESS = 250;
    private static final int NEXT_NEED_PROGRESS = 200;

    /**
     * Constructor for ListFileTask, construct a ListFileTask with certain
     * parameters
     * 
     * @param fileInfoManager a instance of FileInfoManager, which manages
     *            information of files in FileManager.
     * @param operationEvent a instance of OperationEventListener, which is a
     *            interface doing things before/in/after the task.
     * @param path ListView will list files included in this path.
     * @param filterType to determine which files will be listed.
     */
    public ListFileTask(Context context, FileInfoManager fileInfoManager,
            OperationEventListener operationEvent, String path, int filterType) {
        super(fileInfoManager, operationEvent);
        mContext = context;
        mPath = path;
        mFilterType = filterType;
    }

    @Override
    protected Integer doInBackground(Void... params) {
        PDebug.Start("ListFileTask --- doInBackground");
        synchronized (mContext.getApplicationContext()) {
            List<FileInfo> fileInfoList = new ArrayList<FileInfo>();
            File[] files = null;
            int total = 0;
            int progress = 0;
            long startLoadTime = System.currentTimeMillis();
            LogUtils.d(TAG, "doInBackground path = " + mPath);

            File dir = new File(mPath);
            if (dir.exists()) {
                files = dir.listFiles();
                if (files == null) {
                    LogUtils.w(TAG, "doInBackground,directory is null");
                    PDebug.End("ListFileTask --- doInBackground");
                    return OperationEventListener.ERROR_CODE_UNSUCCESS;
                }
            } else {
                LogUtils.w(TAG, "doInBackground,directory is not exist.");
                PDebug.End("ListFileTask --- doInBackground");
                return OperationEventListener.ERROR_CODE_UNSUCCESS;
            }
            total = files.length;
            long loadTime = 0;
            int nextUpdateTime = FIRST_NEED_PROGRESS;
            LogUtils.d(TAG, "doInBackground, total = " + total);
            for (int i = 0; i < files.length; i++) {
                if (isCancelled()) {
                    LogUtils.w(TAG, " doInBackground,calcel.");
                    PDebug.End("ListFileTask --- doInBackground");
                    return OperationEventListener.ERROR_CODE_UNSUCCESS;
                }

                if (mFilterType == FileManagerService.FILE_FILTER_TYPE_DEFAULT) {
                    if (files[i].getName().startsWith(".")) {
                        LogUtils.i(TAG, " doInBackground,start with.,contine.");
                        continue;
                    }
                }

                if (mFilterType == FileManagerService.FILE_FILTER_TYPE_FOLDER) {
                    if (!files[i].isDirectory()) {
                        LogUtils.i(TAG, " doInBackground,is not directory,continue..");
                        continue;
                    }
                }

                mFileInfoManager.addItem(new FileInfo(files[i]));
                loadTime = System.currentTimeMillis() - startLoadTime;
                progress++;

                if (loadTime > nextUpdateTime) {
                    startLoadTime = System.currentTimeMillis();
                    nextUpdateTime = NEXT_NEED_PROGRESS;
                    LogUtils.d(TAG, "doInBackground,pulish progress.");
                    publishProgress(new ProgressInfo("", progress, total, progress, total));

                }
            }
            LogUtils.d(TAG, "doInBackground ERROR_CODE_SUCCESS");
            PDebug.End("ListFileTask --- doInBackground");
            return OperationEventListener.ERROR_CODE_SUCCESS;
        }
    }
}
