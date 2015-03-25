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

import android.content.Context;
import android.content.pm.IPackageInstallObserver;
import android.content.pm.PackageManager;
import android.net.Uri;

import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.util.List;

public class AppRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AppRestoreComposer";
    private int mIndex;
    private List<String> mFileNameList;
    private Object mLock = new Object();

    public AppRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_APP;
    }

    public int getCount() {
        int count = 0;
        if (mFileNameList != null) {
            count = mFileNameList.size();
        }
        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;
        if(mParams != null) {
            mFileNameList = mParams;
            result = true;
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ", count:" + getCount());
        return result;
    }

    public boolean isAfterLast() {
        boolean result = true;
        if (mFileNameList != null) {
            result = (mIndex >= mFileNameList.size());
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    public boolean implementComposeOneEntity() {
        boolean result = false;
        if(mFileNameList != null && mIndex < mFileNameList.size()){
            try {
                String apkFileName = mFileNameList.get(mIndex++);
                File apkFile = new File(apkFileName);
                if (apkFile != null && apkFile.exists()) {
                    PackageManager packageManager = mContext.getPackageManager();
                    PackageInstallObserver installObserver = new PackageInstallObserver();

                    packageManager.installPackage(Uri.fromFile(apkFile), installObserver,
                                                  PackageManager.INSTALL_REPLACE_EXISTING, "test");

                    synchronized (mLock) {
                        while (!installObserver.finished) {
                            try {
                                mLock.wait();
                            } catch (InterruptedException e) {
                            }
                        }

                        if (installObserver.result == PackageManager.INSTALL_SUCCEEDED) {
                            result = true;
                            MyLogger.logD(CLASS_TAG, "install success");
                        } else {
                            MyLogger.logD(CLASS_TAG, "install fail, result:" + installObserver.result);
                        }
                    }
                } else {
                    MyLogger.logD(CLASS_TAG, "install failed");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return result;
    }

    // private void delteTempFolder() {
    //     String folder = SDCardUtils.getStoragePath() + File.separator + ModulePath.FOLDER_TEMP;
    //     File file = new File(folder);
    //     if (file.exists() && file.isDirectory()) {
    //         File files[] = file.listFiles();
    //         for (File f : files) {
    //             if (f.getName().matches(ModulePath.ALL_APK_FILES)) {
    //                 f.delete();
    //             }
    //         }
    //         file.delete();
    //     }
    // }

    public void onStart() {
        super.onStart();
        //delteTempFolder();
    }

    public void onEnd() {
        super.onEnd();
        if (mFileNameList != null) {
            mFileNameList.clear();
        }
        //delteTempFolder();
        MyLogger.logD(CLASS_TAG, "onEnd()");
    }

    class PackageInstallObserver extends IPackageInstallObserver.Stub {
        private boolean finished = false;
        private int result;

        @Override
        public void packageInstalled(String name, int status) {
            synchronized (mLock) {
                finished = true;
                result = status;
                mLock.notifyAll();
            }
        }
    }

}
