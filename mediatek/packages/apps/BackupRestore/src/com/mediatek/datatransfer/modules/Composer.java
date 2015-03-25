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

package com.mediatek.datatransfer.modules;

//import com.mediatek.backuprestore.utils.BackupZip;
import android.content.Context;
import com.mediatek.datatransfer.ProgressReporter;
import com.mediatek.datatransfer.utils.MyLogger;

import java.util.List;

public abstract class Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/Composer";

    protected Context mContext;
    protected ProgressReporter mReporter;
    // protected BackupZip mZipHandler;
    // protected String mZipFileName;
    protected boolean mIsCancel = false;
    private int mComposeredCount = 0;
    protected String mParentFolderPath;
    protected List<String> mParams;

    public Composer(Context context) {
        mContext = context;
    }

    // public void setZipHandler(BackupZip handler) {
    // mZipHandler = handler;
    // }

    public void setParentFolderPath(String path) {
        mParentFolderPath = path;
    }

    // public void setZipFileName(String fileName) {
    // mZipFileName = fileName;
    // }

    public void setReporter(ProgressReporter reporter) {
        mReporter = reporter;
    }

    synchronized public void setCancel(boolean cancel) {
        mIsCancel = cancel;
    }

    synchronized public boolean isCancel() {
        return mIsCancel;
    }

    public int getComposed() {
        return mComposeredCount;
    }

    public void increaseComposed(boolean result) {
        if (result) {
            ++mComposeredCount;
        }

        if (mReporter != null) {
            mReporter.onOneFinished(this, result);
        }
    }

    public void onStart() {
        if (mReporter != null) {
            mReporter.onStart(this);
        }
    }

    public void onEnd() {
        if (mReporter != null) {
            boolean bResult = (getCount() == mComposeredCount && mComposeredCount > 0);
            mReporter.onEnd(this, bResult);
            MyLogger.logD(CLASS_TAG, "onEnd: result is " + bResult);
            MyLogger.logD(CLASS_TAG, "onEnd: getCount is " + getCount()
                    + ", and composed count is " + mComposeredCount);
        }
    }

    public void setParams(List<String> params) {
        mParams = params;
    }

    public boolean composeOneEntity() {
        boolean result = implementComposeOneEntity();
        if (result) {
            ++mComposeredCount;
        }

        if (mReporter != null) {
            mReporter.onOneFinished(this, result);
        }
        return result;
    }

    abstract public int getModuleType();

    abstract public int getCount();

    abstract public boolean isAfterLast();

    abstract public boolean init();

    abstract protected boolean implementComposeOneEntity();
}
