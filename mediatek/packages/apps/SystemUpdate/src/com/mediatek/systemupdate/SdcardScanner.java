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

package com.mediatek.systemupdate;

import android.app.AlarmManager;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.os.storage.StorageManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.Enumeration;
import java.util.List;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class SdcardScanner {
    private static final String TAG = "SystemUpdate/SdcardScanner";

    private static final String OEM_DEV = SystemProperties.get("ro.product.manufacturer");
    private static final String LANG_DEV = SystemProperties.get("ro.product.locale.language");
    private static final String PRODUCT_DEV = SystemProperties.get("ro.product.device");
    private static final String OPER_DEV = SystemProperties.get("ro.operator.optr");
    private static final String FLAVOR_DEV = SystemProperties.get("ro.build.flavor");
    private static final String BUILDTIME_DEV = getBuildTime();

    private static final int SPACE_TOAST = 0;

    private Handler mSpaceHandler;

    private Context mContext;
    private StorageManager mStorageManager;
    private List<UpdatePackageInfo> mUpdateInfoList = new ArrayList<UpdatePackageInfo>();
    private String mCrashPath;
    private String mMountedPath;
    private String mTempUnzipDir;
    private String mConfigPath;

    private Handler mHandler;
    private int mErrorCode;

    private static SdcardScanner sSdcardScanner;

    private static final Comparator<UpdatePackageInfo> COMPARATOR = new Comparator<UpdatePackageInfo>() {
        public int compare(UpdatePackageInfo info1, UpdatePackageInfo info2) {
            return info1.publishTime.compareTo(info2.publishTime);
        }
    };

    public static SdcardScanner getInstance(Context context) {
        if (sSdcardScanner == null) {
            sSdcardScanner = new SdcardScanner(context);
        }
        return sSdcardScanner;
    }

    private SdcardScanner(Context context) {
        Xlog.i(TAG, "[SdcardScanner] construction");
        mContext = context;
        mStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        // mStorageManager.registerListener(mStorageListener);

        mTempUnzipDir = Util.getPackagePathName(context);
        mConfigPath = mTempUnzipDir + File.separator + Util.PathName.ENTRY_CONFIGURE;

        new Thread() {
            public void run() {
                Xlog.v(TAG, "thread run " + Thread.currentThread().getName());
                Looper.prepare();
                mSpaceHandler = new Handler(Looper.myLooper()) {

                    @Override
                    public void handleMessage(Message msg) {
                        switch (msg.what) {
                        case SPACE_TOAST:
                            Toast.makeText(
                                    mContext,
                                    mContext.getString(R.string.query_sd_insufficient_space,
                                            (String) msg.obj), Toast.LENGTH_LONG).show();

                            break;
                        default:
                            break;
                        }
                    }
                };
                Looper.loop();
            }
        }.start();
    }

    void setMessageHandler(Handler handler) {
        Xlog.i(TAG, "[setMessageHandler], mHandler = " + handler);
        mHandler = handler;
    }

    void resetMessageHandler(Handler handler) {
        Xlog.i(TAG, "[resetMessageHandler], mHandler = " + handler);
        if (mHandler == handler) {
            mHandler = null;
        }
    }

    void onSdcardCrashed(String crashPath) {
        mCrashPath = crashPath;
        if (mCrashPath.equals(mMountedPath)) {
            Xlog.i(TAG, "crashPath equals mountedPath, set mountedPath to null");
            mMountedPath = null;
        }
    }

    void onSdcardMounted(String mountedPath) {
        mMountedPath = mountedPath;
        if (mMountedPath.equals(mCrashPath)) {
            Xlog.i(TAG, "mountedPath equals crashPath, set crashPath to null");
            mCrashPath = null;
        }
    }

    private static String getBuildTime() {
        Xlog.v(TAG, "[getBuildTime]");
        String buildTime = null;
        // String time = Util.getFormattedKernelVersion();
        String time = SystemProperties.get("ro.build.date");
        try {
            // if (!Util.ERROR_VERSION.equals(time)) {
            Log.i(TAG, "[getBuildTime]version time = " + time);
            SimpleDateFormat df = new SimpleDateFormat("yyyyMMddhhmmss", new Locale("US"));
            Date versionTime = new Date(Date.parse(time));
            buildTime = df.format(versionTime);
            // } else {
            // Log.e(TAG, "[getBuildTime]get build date error");
            // }
        } catch (NullPointerException e) {
            Log.e(TAG,
                    "[getBuildTime]There is some NullPointerException accured parse date string!");
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "[getBuildTime]There is some exception accured parse date string!");
            e.printStackTrace();
        }
        return buildTime;
    }

    // query SD card packages
    public List<UpdatePackageInfo> querySdcardPackage() {
        mCrashPath = null;
        mMountedPath = null;
        mErrorCode = Util.SDresult.VERIFY_OK;

        mUpdateInfoList.clear();
        String[] pathList = mStorageManager.getVolumePaths();
        if (pathList == null || pathList.length == 0) {
            mErrorCode = Util.SDresult.ERROR_NO_SDCARD;
        } else {
            for (String path : pathList) {
                if (!"not_present".equals(mStorageManager.getVolumeState(path))) {
                    if (mCrashPath != null && path.contains(mCrashPath)) {
                        mCrashPath = null;
                        continue;
                    }
                    if (mMountedPath != null && path.contains(mMountedPath)) {
                        mMountedPath = null;
                    }
                    Log.v(TAG, "------------add a sd card path to get package------------" + path);
                    getPackageFile(new File(path));
                }
            }

            if (mMountedPath != null
                    && !"not_present".equals(mStorageManager.getVolumeState(mMountedPath))) {
                Log.v(TAG, "------------add a sd card path to get package------------"
                        + mMountedPath);
                getPackageFile(new File(mMountedPath));
            }

            int size = mUpdateInfoList.size();
            Xlog.d(TAG, "[querySdcardPackage], mUpdateInfo size" + size);
            if (size > 1) {
                Collections.sort(mUpdateInfoList, COMPARATOR);
            }

            if (size > 0) {
                int i = 0;
                for (UpdatePackageInfo info : mUpdateInfoList) {
                    try {
                        if (mCrashPath != null && info.path.contains(mCrashPath)) {
                            Xlog.v(TAG, "[querySdcardPackage]remove crash path's package:"
                                    + info.path);
                            mUpdateInfoList.remove(info);
                        } else {
                            PackageInfoWriter.getInstance().writeInfo(info, "SdPackage" + i);
                            ++i;
                        }
                    } catch (IOException e) {
                        Xlog.e(TAG, "[unpackAndVerify], WriteInfo fail: " + info.path);
                        e.printStackTrace();
                    }
                }

            }
        }

        DownloadInfo downloadInfo = DownloadInfo.getInstance(mContext.getApplicationContext());
        boolean isNotFoundOta = downloadInfo.getDLSessionStatus() != DownloadInfo.STATE_NEWVERSION_READY;
        boolean isNotFoundSd = mUpdateInfoList.isEmpty();
        boolean isNeedRefresh = isNotFoundOta && isNotFoundSd;
        downloadInfo.setIfNeedRefresh(isNeedRefresh);

        downloadInfo.setIfNeedRefreshMenu((!isNotFoundOta && isNotFoundSd)
                || ((isNotFoundOta && mUpdateInfoList.size() == 1)));

        if (!isNeedRefresh) {
            PackageInfoWriter.getInstance().close();
            NotifyManager notifymanager = new NotifyManager(mContext);
            if (mHandler == null) {
                Xlog.i(TAG, "[querySdcardPackage], background, notify user available packages");
                notifymanager.showNewVersionNotification();
            }
            Util.setAlarm(mContext, AlarmManager.RTC_WAKEUP, Calendar.getInstance()
                    .getTimeInMillis() + Util.REFRESHTIME, Util.Action.ACTION_REFRESH_TIME_OUT);
        }

        if (mHandler != null) {
            mHandler.sendMessage(mHandler.obtainMessage(
                    SystemUpdateService.MSG_SDCARDPACKAGESDETECTED, mUpdateInfoList));
        } else {
            Xlog.i(TAG, "[querySdcardPackage], detected done, mHandler is null");
        }

        return mUpdateInfoList;
    }

    private void getPackageFile(File file) {
        if (mCrashPath != null && file.getPath().contains(mCrashPath)) {
            return;
        }
        File[] fileList = file.listFiles();
        if (fileList == null) {
            // if null ,stands for the file is not a directory
            return;
        }
        for (File f : fileList) {
            if (f.isDirectory()) {
                getPackageFile(f);
            } else {
                String fileName = f.getName();
                int start = fileName.lastIndexOf(".") + 1;
                int end = fileName.length();
                String extension = fileName.substring(start, end);
                if ("zip".equals(extension)) {
                    unpackAndVerify(f);
                }
            }
        }
    }

    private void unpackAndVerify(File updateFile) {
        String absolutPath = updateFile.getPath();
        Log.i(TAG, "[unpackAndVerify]++ start unpacking & verification ++" + absolutPath);

        if (!unzip(updateFile)) {
            Xlog.w(TAG, "[unpackAndVerify] unzip fail");
            Util.deleteFile(mTempUnzipDir);
            return;
        }

        // Check Version
        PackageInfoReader sdPkgInfoReader = new PackageInfoReader(mContext, mConfigPath);
        if (!checkVersion(sdPkgInfoReader)) {
            Xlog.w(TAG, "[unpackAndVerify] checkVersion fail");
            Util.deleteFile(mTempUnzipDir);
            return;
        }

        // Check OTA
        CheckPkg checkPkg = new CheckPkg(mContext, UpgradePkgManager.SD_PACKAGE, absolutPath);
        int otaResult = checkPkg.execForResult();
        if (otaResult != Util.OTAresult.CHECK_OK) {
            Xlog.w(TAG, "[checkUpgradePackage], check_ota result = " + otaResult);
            mErrorCode = otaResult;
            Util.deleteFile(mTempUnzipDir);
            return;
        }

        Util.deleteFile(mTempUnzipDir);

        getUpdateInfo(sdPkgInfoReader, absolutPath);
        Log.d(TAG, "[unpackAndVerify]++ done!!! ++");
    }

    private boolean unzip(File updateFile) {

        if (Util.getExtraSpaceNeeded(mContext, (long) (1.5 * updateFile.length())) < 0) {
            Log.e(TAG, "[unzip] Storage not enough");
            mErrorCode = Util.SDresult.ERROR_OUT_OF_MEMORY;
            mSpaceHandler
                    .sendMessage(mSpaceHandler.obtainMessage(SPACE_TOAST, updateFile.getPath()));
            return false;
        }

        ZipFile zipFile = null;
        try {
            zipFile = new ZipFile(updateFile.getPath());
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (zipFile == null) {
            Xlog.w(TAG, "[unzip] zipfile is null");
            return false;
        }

        int availableFileNum = 0;
        Enumeration<?> entries = zipFile.entries();
        while (entries.hasMoreElements()) {
            ZipEntry entry = ((ZipEntry) entries.nextElement());
            String name = entry.getName();
            Log.i(TAG, "====[unzip]Entry name = " + name);
            if (Util.PathName.ENTRY_PACKAGE_NAME.equals(name)
                    || Util.PathName.ENTRY_TYPE.equals(name)
                    || Util.PathName.ENTRY_SCATTER.equals(name)
                    || Util.PathName.ENTRY_CONFIGURE.equals(name)
                    || Util.PathName.ENTRY_BUILD_INFO.equals(name)) {
                ++availableFileNum;
            }
        }

        if (availableFileNum != 5) {
            Log.w(TAG, "[unzip]availableFileNum = " + availableFileNum);
            try {
                zipFile.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            return false;
        }

        int result = Util.unzipFileElement(zipFile, Util.PathName.ENTRY_CONFIGURE, mConfigPath);
        if (Util.OTAresult.OTA_FILE_UNZIP_OK != result) {
            Log.w(TAG, "[unzip] unzip configure.xml file fail : " + result);
            mErrorCode = result;
            try {
                zipFile.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            return false;
        }

        return true;
    }

    private boolean checkVersion(PackageInfoReader sdPkgInfoReader) {

        if (OEM_DEV == null || !OEM_DEV.equals(sdPkgInfoReader.getOem())) {
            Xlog.w(TAG, "not correct oem for this package, oem = " + OEM_DEV);
            mErrorCode = Util.SDresult.ERROR_VERSION_ERR;
            return false;
        }

        if (PRODUCT_DEV == null || !PRODUCT_DEV.equals(sdPkgInfoReader.getProduct())) {
            Xlog.w(TAG, "not correct product for this package, product = " + PRODUCT_DEV);
            mErrorCode = Util.SDresult.ERROR_VERSION_ERR;
            return false;
        }

        String operPkg = sdPkgInfoReader.getOperator();
        if (!((TextUtils.isEmpty(OPER_DEV) && TextUtils.isEmpty(operPkg)) || OPER_DEV
                .equals(operPkg))) {
            Xlog.w(TAG, "not correct operator for this package, operator = " + OPER_DEV);
            mErrorCode = Util.SDresult.ERROR_VERSION_ERR;
            return false;
        }

        String flavorPkg = sdPkgInfoReader.getFlavor();
        if (!((TextUtils.isEmpty(FLAVOR_DEV) && TextUtils.isEmpty(flavorPkg)) || FLAVOR_DEV
                .equals(flavorPkg))) {
            Xlog.w(TAG, "not correct flavor for this package, flavor = " + FLAVOR_DEV);
            mErrorCode = Util.SDresult.ERROR_VERSION_ERR;
            return false;
        }

        String langPkg = sdPkgInfoReader.getLanguage();
        if (!((TextUtils.isEmpty(LANG_DEV) && TextUtils.isEmpty(langPkg)) || LANG_DEV
                .equals(langPkg))) {
            Xlog.w(TAG, "not correct language for this package, language = " + LANG_DEV);
            mErrorCode = Util.SDresult.ERROR_VERSION_ERR;
            return false;
        }

        String time = sdPkgInfoReader.getPublishTime();
        if (time == null || time.compareTo(BUILDTIME_DEV) <= 0) {
            Xlog.w(TAG, "not correct buildTime for this package, buildTime = " + BUILDTIME_DEV);
            mErrorCode = Util.SDresult.ERROR_VERSION_ERR;
            return false;
        }
        return true;
    }

    private void getUpdateInfo(PackageInfoReader sdPkgInfoReader, String absolutPath) {
        UpdatePackageInfo info = new UpdatePackageInfo();
        info.path = absolutPath;
        info.androidNumber = sdPkgInfoReader.getAndroidNum();
        info.version = sdPkgInfoReader.getVersionName();
        info.notes = sdPkgInfoReader.getNotes();
        info.publishTime = sdPkgInfoReader.getPublishTime();
        mUpdateInfoList.add(info);
    }

}
