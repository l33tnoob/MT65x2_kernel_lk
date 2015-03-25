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

import android.os.AsyncTask;

import com.mediatek.filemanager.FileInfoManager;
import com.mediatek.filemanager.service.FileManagerService.OperationEventListener;
import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.PDebug;


abstract class BaseAsyncTask extends AsyncTask<Void, ProgressInfo, Integer> {
    private static final String TAG = "BaseAsyncTask";
    
    protected OperationEventListener mListener = null;
    protected FileInfoManager mFileInfoManager = null;
    protected boolean mIsTaskFinished = true;

    /**
     * Constructor of BaseAsyncTask
     * 
     * @param fileInfoManager a instance of FileInfoManager, which manages information of files in
     *            FileManager.
     * @param listener a instance of OperationEventListener, which is a interface doing things
     *            before/in/after the task.
     */
    public BaseAsyncTask(FileInfoManager fileInfoManager, OperationEventListener listener) {
        if (fileInfoManager == null) {
            throw new IllegalArgumentException();
        }
        mFileInfoManager = fileInfoManager;
        mListener = listener;
    }

    @Override
    protected void onPreExecute() {
        mIsTaskFinished = false;
        if (mListener != null) {
            LogUtils.d(TAG, "onPreExecute");
            mListener.onTaskPrepare();
        }
    }

    @Override
    protected void onPostExecute(Integer result) {
        PDebug.Start("BaseAsyncTask --- onPostExecute");
        if (mListener != null) {
            LogUtils.d(TAG, "onPostExecute");
            mListener.onTaskResult(result);
            mListener = null;
        }
        mIsTaskFinished = true;
        PDebug.End("BaseAsyncTask --- onPostExecute");
    }

    @Override
    protected void onCancelled() {
        if (mListener != null) {
            LogUtils.d(TAG, "onCancelled()");
            mListener.onTaskResult(OperationEventListener.ERROR_CODE_USER_CANCEL);
            mListener = null;
        }
        mIsTaskFinished = true;
    };

    @Override
    protected void onProgressUpdate(ProgressInfo... values) {
        if (mListener != null && values != null && values[0] != null) {
            LogUtils.v(TAG, "onProgressUpdate");
            mListener.onTaskProgress(values[0]);
        }
    }

    /**
     * This method remove listener from task. Set listener associate with task to be null.
     */
    protected void removeListener() {
        if (mListener != null) {
            LogUtils.d(TAG, "removeListener");
            mListener = null;
        }
    }

    /**
     * This method set mListener with certain listener.
     * 
     * @param listener the certain listener, which will be set to be mListener.
     */
    public void setListener(OperationEventListener listener) {
        mListener = listener;
    }

    public boolean isTaskBusy() {
        LogUtils.d(TAG, "isTaskBusy,task status = " + getStatus());
        if (mIsTaskFinished || getStatus() == Status.FINISHED) {
            LogUtils.d(TAG, "isTaskBusy,retuen false.");
            return false;
        }
        LogUtils.d(TAG, "isTaskBusy,retuen true.");
        return true;
    }
}
