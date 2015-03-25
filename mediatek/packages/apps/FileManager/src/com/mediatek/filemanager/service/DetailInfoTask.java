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

import com.mediatek.filemanager.FileInfo;
import com.mediatek.filemanager.FileInfoManager;
import com.mediatek.filemanager.MountPointManager;
import com.mediatek.filemanager.service.FileManagerService.OperationEventListener;
import com.mediatek.filemanager.utils.LogUtils;

import java.io.File;


class DetailInfoTask extends BaseAsyncTask {
    private static final String TAG = "DetailInfoTask";
    
    private final FileInfo mDetailfileInfo;
    private long mTotal = 0;

    /**
     * Constructor of DetailInfoTask
     * 
     * @param fileInfoManager a instance of FileInfoManager, which manages information of files in
     *            FileManager.
     * @param operationEvent a instance of OperationEventListener, which is a interface doing things
     *            before/in/after the task.
     * @param file a instance of FileInfo, which contains all data about a file.
     */
    public DetailInfoTask(FileInfoManager fileInfoManager, OperationEventListener operationEvent,
            FileInfo file) {
        super(fileInfoManager, operationEvent);
        mDetailfileInfo = file;
    }

    @Override
    protected Integer doInBackground(Void... params) {
        LogUtils.d(TAG, "doInBackground...");
        if (mDetailfileInfo.isDirectory()) {
            if (!MountPointManager.getInstance().isRootPath(
                    mDetailfileInfo.getFileAbsolutePath())) {
                final File[] files = mDetailfileInfo.getFile().listFiles();
                int ret = OperationEventListener.ERROR_CODE_SUCCESS;
                if (files != null) {
                    for (File file : files) {
                        ret = getContentSize(file);
                        if (ret < 0) {
                            LogUtils.i(TAG, "doInBackground,ret = " + ret);
                            return ret;
                        }
                    }
                }
            }
        } else {
            long size = mDetailfileInfo.getFileSize();
            publishProgress(new ProgressInfo("", 0, size, 0, size));
        }
        
        return OperationEventListener.ERROR_CODE_SUCCESS;
    }

    /**
     * The method calculate a file's size(including its contains, if is a directory).
     * 
     * @param root a file which need calculate.
     * @return The file's content size.
     */
    public int getContentSize(File root) {
        LogUtils.d(TAG, "getContentSize...");
        int ret = OperationEventListener.ERROR_CODE_SUCCESS;
        if (root.isDirectory()) {
            File[] files = root.listFiles();
            for (File file : files) {
                if (isCancelled()) {
                    return OperationEventListener.ERROR_CODE_USER_CANCEL;
                }
                ret = getContentSize(file);
                if (ret < 0) {
                    LogUtils.i(TAG, "getContentSize ,ret = " + ret);
                    return ret;
                }
            }
        }
        mTotal += root.length();
        publishProgress(new ProgressInfo("", 0, mTotal, 0, mTotal));
        return OperationEventListener.ERROR_CODE_SUCCESS;
    }
}
