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
import com.mediatek.filemanager.utils.LogUtils;


public class ProgressInfo {
    private static final String TAG = "ProgressInfo";
    private String mUpdateInfo = null;
    private final int mProgress;
    private int mErrorCode = 0;
    private final long mTotal;
    private final boolean mIsFailInfo;
    private FileInfo mFileInfo = null;
    private final long mTotalNumber;
    private final int mCurrentNumber;

    /**
     * Constructor to construct a ProgressInfo
     * 
     * @param update the string which will be shown on ProgressDialogFragment
     * @param progeress current progress number
     * @param total total number
     */
    public ProgressInfo(String update, int progeress, long total, int currentNumber,
            long totalNumber) {
       // LogUtils.d(TAG, "ProgressInfo1,currentNumber=" + currentNumber + ",totalNumber = " + totalNumber);
        mUpdateInfo = update;
        mProgress = progeress;
        mTotal = total;
        mIsFailInfo = false;
        mCurrentNumber = currentNumber;
        mTotalNumber = totalNumber;
    }

    /**
     * Constructor to construct a ProgressInfo
     * 
     * @param fileInfo the fileInfo which will be associated with Dialog
     * @param progeress current progress number
     * @param total total number
     */
    public ProgressInfo(FileInfo fileInfo, int progeress, long total, int currentNumber,
            long totalNumber) {
       // LogUtils.d(TAG, "ProgressInfo2,currentNumber=" + currentNumber + ",totalNumber = " + totalNumber);
        mFileInfo = fileInfo;
        mProgress = progeress;
        mTotal = total;
        mIsFailInfo = false;
        mCurrentNumber = currentNumber;
        mTotalNumber = totalNumber;
    }

    /**
     * Constructor to construct a ProgressInfo
     * 
     * @param errorCode An int represents ERROR_CODE
     * @param isFailInfo status of task associated with certain progressDialog
     */
    public ProgressInfo(int errorCode, boolean isFailInfo) {
        LogUtils.d(TAG, "ProgressInfo,errorCode=" + errorCode);
        mErrorCode = errorCode;
        mProgress = 0;
        mTotal = 0;
        mIsFailInfo = isFailInfo;
        mCurrentNumber = 0;
        mTotalNumber = 0;
    }

    /**
     * This method gets status of task doing in background
     *
     * @return true for failed, false for no fail occurs in task
     */
    public boolean isFailInfo() {
        return mIsFailInfo;
    }

    /**
     * This method gets fileInfo, which will be updated on DetaiDialog
     * 
     * @return fileInfo, which contains file's information(name, size, and so on)
     */
    public FileInfo getFileInfo() {
        return mFileInfo;
    }

    /**
     * This method gets ERROR_CODE for certain task, which is doing in background.
     * 
     * @return ERROR_CODE for certain task
     */
    public int getErrorCode() {
        return mErrorCode;
    }

    /**
     * This method gets the content, which will be updated on ProgressDialog
     * 
     * @return content, which need update
     */
    public String getUpdateInfo() {
        return mUpdateInfo;
    }

    /**
     * This method gets current progress number
     * 
     * @return current progress number of progressDialog
     */
    public int getProgeress() {
        return mProgress;
    }

    /**
     * This method gets total number of progressDialog
     * 
     * @return total number
     */
    public long getTotal() {
        return mTotal;
    }

    /**
     * This method gets current number of files
     *
     * @return current number
     */
    public int getCurrentNumber() {
        return mCurrentNumber;
    }

    /**
     * This method gets total number of files
     *
     * @return total number
     */
    public long getTotalNumber() {
        return mTotalNumber;
    }
}
