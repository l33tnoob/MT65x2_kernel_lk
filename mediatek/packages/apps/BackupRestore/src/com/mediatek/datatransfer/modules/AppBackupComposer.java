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
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import android.content.res.Resources;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class AppBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AppBackupComposer";

    private List<ApplicationInfo> mUserAppInfoList = null;
    private int mAppIndex = 0;

    public AppBackupComposer(final Context context) {
        super(context);
    }

    @Override
    public final int getModuleType() {
        return ModuleType.TYPE_APP;
    }

    @Override
    public final int getCount() {
        int count = 0;
        if (mUserAppInfoList != null && mUserAppInfoList.size() > 0) {
            count = mUserAppInfoList.size();
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    @Override
    public final boolean init() {
        boolean result = false;

        if(mParams != null) {
            List<ApplicationInfo> tmpList = getUserAppInfoList(mContext);
            HashMap tmpMap = new HashMap<String, ApplicationInfo>();
            if(tmpList != null) {
                for(ApplicationInfo appInfo : tmpList) {
                    tmpMap.put(appInfo.packageName, appInfo);
                }
            }

            mUserAppInfoList = new ArrayList<ApplicationInfo>();
            for(int i = 0; i < mParams.size(); ++i) {
                ApplicationInfo appInfo = (ApplicationInfo)tmpMap.get(mParams.get(i));
                if(appInfo != null) {
                    mUserAppInfoList.add(appInfo);
                }
            }

            result = true;
            mAppIndex = 0;
        }

        MyLogger.logD(CLASS_TAG, "init():" + result);
        return result;
    }

    @Override
    public final boolean isAfterLast() {
        boolean result = true;
        if (mUserAppInfoList != null && mAppIndex < mUserAppInfoList.size()) {
            result = false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    @Override
    public final boolean implementComposeOneEntity() {
        boolean result = false;
        if (mUserAppInfoList != null && mAppIndex < mUserAppInfoList.size()) {
            ApplicationInfo appInfo = mUserAppInfoList.get(mAppIndex);
            String appSrc = appInfo.publicSourceDir;
            String appDest = mParentFolderPath + File.separator + appInfo.packageName + ModulePath.FILE_EXT_APP;
            CharSequence tmpLable = "";
            if (appInfo.uid == -1) {
                tmpLable = getApkFileLabel(mContext, appInfo.sourceDir, appInfo);
            } else {
                tmpLable = appInfo.loadLabel(mContext.getPackageManager());
            }
            String label = (tmpLable == null) ? appInfo.packageName : tmpLable.toString();
            MyLogger.logD(CLASS_TAG,
                    mAppIndex + ":" + appSrc + ",pacageName:" + appInfo.packageName
                            + ",sourceDir:" + appInfo.sourceDir + ",dataDir:" + appInfo.dataDir
                            + ",lable:" + label);

            try {
                copyFile(appSrc, appDest);
                //mZipHandler.addFileByFileName(appSrc, appDest);
                // mZipHandler.addFolder(
                //         appInfo.dataDir,
                //         ModulePath.FOLDER_APP
                //                 + File.separator
                //                 + appInfo.dataDir.subSequence(
                //                         appInfo.dataDir.lastIndexOf(File.separator) + 1,
                //                         appInfo.dataDir.length()).toString());

                MyLogger.logD(CLASS_TAG, "addFile " + appSrc + "success");
                result = true;
            } catch (IOException e) {
                if (super.mReporter != null) {
                    super.mReporter.onErr(e);
                }
                MyLogger.logD(CLASS_TAG, "addFile:" + appSrc + "fail");
                e.printStackTrace();
            }

            ++mAppIndex;
        }

        return result;
    }

    public static List<ApplicationInfo> getUserAppInfoList(final Context context) {
        List<ApplicationInfo> userAppInfoList = null;
        if (context != null) {
            List<ApplicationInfo> allAppInfoList = context.getPackageManager()
                    .getInstalledApplications(0);
            userAppInfoList = new ArrayList<ApplicationInfo>();
            for (ApplicationInfo appInfo : allAppInfoList) {
                if (!((appInfo.flags & ApplicationInfo.FLAG_SYSTEM) == ApplicationInfo.FLAG_SYSTEM)
                        && !appInfo.packageName.equalsIgnoreCase(context.getPackageName())) {
                    userAppInfoList.add(appInfo);
                }
            }
        }
        return userAppInfoList;
    }

    private CharSequence getApkFileLabel(final Context context, final String apkPath,
            final ApplicationInfo appInfo) {
        if (context == null || appInfo == null || apkPath == null || !(new File(apkPath).exists())) {
            return null;
        }

        Resources contextResources = mContext.getResources();
        AssetManager assetManager = new AssetManager();
        assetManager.addAssetPath(apkPath);

        Resources resources = new Resources(assetManager, contextResources.getDisplayMetrics(),
                contextResources.getConfiguration());

        CharSequence label = null;
        if (0 != appInfo.labelRes) {
            label = (String) resources.getText(appInfo.labelRes);
        }

        return label;
    }

    @Override
    public final void onEnd() {
        super.onEnd();
        if (mUserAppInfoList != null) {
            mUserAppInfoList.clear();
        }
        MyLogger.logD(CLASS_TAG, "onEnd()");
    }

    private void copyFile(String srcFile, String destFile) throws IOException {
        try {
            File f1 = new File(srcFile);
            if (f1.exists() && f1.isFile()) {
                InputStream inStream = new FileInputStream(srcFile);
                FileOutputStream outStream = new FileOutputStream(destFile);
                byte[] buf = new byte[1024];
                int byteRead = 0;
                while ((byteRead = inStream.read(buf)) != -1) {
                    outStream.write(buf, 0, byteRead);
                }
                outStream.flush();
                outStream.close();
                inStream.close();
            }
        } catch(IOException e) {
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public final void onStart() {
        super.onStart();
        if(getCount() > 0) {
            File path = new File(mParentFolderPath);
            if(!path.exists()) {
                path.mkdirs();
            }
        }
    }

}