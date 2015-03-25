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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

/**
 * The broadcast receiver to handle broadcast needed.
 * 
 * @author mtk80357
 * 
 */
public class SystemUpdateReceiver extends BroadcastReceiver {
    private static final int NEWVERSION_AUTO_QUERY_DAY_OF_WEEK1 = Calendar.MONDAY;
    private static final int NEWVERSION_AUTO_QUERY_DAY_OF_WEEK2 = Calendar.THURSDAY;

    private static final int QUERY_TIME_HOUR = 12;
    private static final int QUERY_TIME_DELAY = 30000;
    private static final int SHOW_RESULT_TIME_DELAY = 1000;

    private static final String TAG = "SystemUpdate/Receiver";

    private String mNandResultFilePath = null;

    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Xlog.i(TAG, "onReceive: action = " + action);
        if (action == null) {
            return;
        }
        if (action.equalsIgnoreCase(ConnectivityManager.CONNECTIVITY_ACTION)) {

            Util.NETWORK_STATUS networkStatus = Util.getNetworkType(context);

            if (networkStatus == Util.NETWORK_STATUS.STATE_NONE_NETWORK) {
                return;
            }

            DownloadInfo downloadInfo = DownloadInfo.getInstance(mContext.getApplicationContext());

            int status = downloadInfo.getDLSessionStatus();
            Xlog.i(TAG, "status = " + status);

            if ((status == DownloadInfo.STATE_PAUSEDOWNLOAD) && (downloadInfo.getOtaAutoDlStatus())) {

                if ((networkStatus == Util.NETWORK_STATUS.STATE_WIFI) && downloadInfo.getIfPauseWithinTime()) {
                    Intent service = new Intent(context, SystemUpdateService.class);
                    service.setAction(Util.Action.ACTION_AUTO_DOWNLOAD);
                    context.startService(service);

                    return;

                } else {
                    showDlReminderNotification(context, true);

                    return;

                }
            }

            String strQueryDate = downloadInfo.getQueryDate();
            if ((strQueryDate != null) && (hasQueryThisDay(strQueryDate))) {
                return;
            }

            if (status == DownloadInfo.STATE_QUERYNEWVERSION && getQueryRequest()) {
                Xlog.i(TAG, "query new version");
                Util.setAlarm(context, AlarmManager.RTC, Calendar.getInstance().getTimeInMillis() + QUERY_TIME_DELAY,
                        Util.Action.ACTION_AUTO_QUERY_NEWVERSION);
            } else {
                Util.setAlarm(context, AlarmManager.RTC, getNextAlarmTime(), Util.Action.ACTION_AUTO_QUERY_NEWVERSION);
                Xlog.i(TAG, "status = " + status + ". Need not query, set next query alarm");
            }
        } else if (action.equalsIgnoreCase(Util.Action.ACTION_AUTO_QUERY_NEWVERSION)) {

            if (!Util.isNetWorkAvailable(context, null)) {

                return;
            }
            DownloadInfo mDownLoadStatus = DownloadInfo.getInstance(context);

            int status = mDownLoadStatus.getDLSessionStatus();
            Xlog.i(TAG, "status = " + status);
            /*
             * boolean updateFlag = getUpdateStatus(); if (status ==
             * DownloadInfo.STATE_DLPKGCOMPLETE && updateFlag) {
             * resetDescriptionInfo(mDownLoadStatus); status =
             * DownloadInfo.STATE_QUERYNEWVERSION; Xlog.i(TAG,
             * "update complete"); }
             */

            if (status == DownloadInfo.STATE_QUERYNEWVERSION && getQueryRequest()) {
                Intent service = new Intent(context, SystemUpdateService.class);
                service.setAction(Util.Action.ACTION_AUTO_QUERY_NEWVERSION);
                context.startService(service);
            } else {
                Util.setAlarm(context, AlarmManager.RTC, getNextAlarmTime(), Util.Action.ACTION_AUTO_QUERY_NEWVERSION);
                Xlog.i(TAG, "status = " + status + ". Need not query, set next query alarm");
            }
        } else if (action.equalsIgnoreCase(Util.Action.ACTION_BOOT_COMPLETED)) {

            DownloadInfo mDownLoadStatus = DownloadInfo.getInstance(context);
            mDownLoadStatus.setOtaAutoDlStatus(false);
            mDownLoadStatus.setIfPauseWithinTime(false);

            int status = mDownLoadStatus.getDLSessionStatus();
            Xlog.i(TAG, "status = " + status);
            boolean updateFlag = getUpdateStatus();
            if (status == DownloadInfo.STATE_DOWNLOADING) {
                mDownLoadStatus.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);
                Xlog.i(TAG, "Abnormal stop during downloading.");
                showDlReminderNotification(context, true);

            } else if (status == DownloadInfo.STATE_PAUSEDOWNLOAD) {
                showDlReminderNotification(context, true);

            } else if (status == DownloadInfo.STATE_DLPKGCOMPLETE) {

                showDlReminderNotification(mContext, false);
                Intent installNotify = new Intent(mContext, ForegroundDialogService.class);
                installNotify.putExtra(ForegroundDialogService.DLG_ID, ForegroundDialogService.DIALOG_INSTALL_REMINDER);
                mContext.startService(installNotify);

                Xlog.i(TAG, "ACTION_BOOT_COMPLETED, download complete");
            } else if (status == DownloadInfo.STATE_PACKAGEUNZIPPING) {
                mDownLoadStatus.setDLSessionUnzipState(false);
                mDownLoadStatus.setDLSessionRenameState(false);
                UpgradePkgManager.resetPkg(context);
                mDownLoadStatus.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);
                showDlReminderNotification(context, true);
            }

            if (getUpdateStatus()) {
                Util.setAlarm(context, AlarmManager.RTC, Calendar.getInstance().getTimeInMillis() + SHOW_RESULT_TIME_DELAY,
                        Util.Action.ACTION_UPDATE_REPORT);
                if (status == DownloadInfo.STATE_QUERYNEWVERSION) {
                    deleteUpdatePkg();
                }

            }

            String strQueryDate = mDownLoadStatus.getQueryDate();
            if ((strQueryDate != null) && (hasQueryThisDay(strQueryDate))) {
                return;
            }

            if (status == DownloadInfo.STATE_QUERYNEWVERSION && getQueryRequest()) {
                Xlog.i(TAG, "ACTION_BOOT_COMPLETED: query day to query new version");
                Util.setAlarm(context, AlarmManager.RTC, Calendar.getInstance().getTimeInMillis() + QUERY_TIME_DELAY,
                        Util.Action.ACTION_AUTO_QUERY_NEWVERSION);
            } else {
                Util.setAlarm(context, AlarmManager.RTC, getNextAlarmTime(), Util.Action.ACTION_AUTO_QUERY_NEWVERSION);
                Xlog.i(TAG, "ACTION_BOOT_COMPLETED, status = " + status + ", need not query, set next query alarm");
            }
        } else if (action.equalsIgnoreCase(Util.Action.ACTION_UPDATE_REMIND)) {
            Xlog.i(TAG, "ACTION_UPDATE_REMIND");
            NotifyManager notification = new NotifyManager(context);
            notification.showDownloadCompletedNotification();
        } else if (action.equalsIgnoreCase(Util.Action.ACTION_UPDATE_REPORT)) {
            Xlog.i(TAG, "ACTION_UPDATE_REPORT");
            startReportActivity();
        } else if (action.equalsIgnoreCase(Util.Action.ACTION_AUTO_DL_TIME_OUT)) {
            Xlog.i(TAG, "ACTION_AUTO_DL_TIME_OUT");
            DownloadInfo downloadInfo = DownloadInfo.getInstance(context);
            downloadInfo.setIfPauseWithinTime(false);

        }
    }

    private void showDlReminderNotification(Context context, boolean isForDL) {
        Xlog.i(TAG, "showDlReminderNotification");
        NotifyManager notification = new NotifyManager(context);
        DownloadInfo downloadInfo = DownloadInfo.getInstance(context);

        if (isForDL) {
            notification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);

            if (Util.isSdcardAvailable(context)) {

                notification.showDownloadingNotificaton(downloadInfo.getVerNum(), (int) (((double) Util.getFileSize(Util
                        .getPackageFileName(mContext)) / (double) downloadInfo.getUpdateImageSize()) * 100), false);

            } else {
                notification.showDownloadingUnknownPer();
            }

        } else {
            notification.showDownloadCompletedNotification();
        }

    }

    private boolean hasQueryThisDay(String queryDate) {
        SimpleDateFormat dateFormat = new SimpleDateFormat(Util.DATE_FORMAT);
        String strDate = dateFormat.format(new Date());
        return strDate.equals(queryDate);
    }

    private long getNextAlarmTime() {

        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

        Calendar calendar = Calendar.getInstance();

        Xlog.i(TAG, "current time:" + format.format(calendar.getTime()));

        int currDayOfWeek = calendar.get(Calendar.DAY_OF_WEEK);

        int daysToAdd = 0;
        if (currDayOfWeek < NEWVERSION_AUTO_QUERY_DAY_OF_WEEK1) {
            daysToAdd = NEWVERSION_AUTO_QUERY_DAY_OF_WEEK1 - currDayOfWeek;
        } else if ((currDayOfWeek >= NEWVERSION_AUTO_QUERY_DAY_OF_WEEK1)
                && (currDayOfWeek < NEWVERSION_AUTO_QUERY_DAY_OF_WEEK2)) {
            daysToAdd = NEWVERSION_AUTO_QUERY_DAY_OF_WEEK2 - currDayOfWeek;
        } else {
            daysToAdd = NEWVERSION_AUTO_QUERY_DAY_OF_WEEK1 + Calendar.SATURDAY - currDayOfWeek;
        }

        calendar.add(Calendar.DATE, daysToAdd);
        calendar.set(Calendar.HOUR_OF_DAY, QUERY_TIME_HOUR);
        calendar.set(Calendar.MINUTE, 0);
        calendar.set(Calendar.SECOND, 0);
        Xlog.i(TAG, "alarm time:" + format.format(calendar.getTime()));
        return calendar.getTimeInMillis();
    }

    boolean getQueryRequest() {
        Calendar calendar = Calendar.getInstance();
        if (NEWVERSION_AUTO_QUERY_DAY_OF_WEEK1 == calendar.get(Calendar.DAY_OF_WEEK)
                || NEWVERSION_AUTO_QUERY_DAY_OF_WEEK2 == calendar.get(Calendar.DAY_OF_WEEK)) {
            return true;
        }
        return false;
    }

    private boolean getUpdateStatus() {
        Xlog.i(TAG, "getUpdateStatus");
        boolean upgradeStarted = false;

        DownloadInfo downloadStatus = DownloadInfo.getInstance(mContext.getApplicationContext());
        upgradeStarted = downloadStatus.getUpgradeStartedState();

        Xlog.i(TAG, "upgradeStarted: " + upgradeStarted);

        String pkgPath = Util.getPackageFileName(mContext);

        if ((upgradeStarted) && (pkgPath != null)
                && (downloadStatus.getDLSessionStatus() == DownloadInfo.STATE_QUERYNEWVERSION)) {
            Xlog.i(TAG, "getUpdateStatus, update finished");
            File imgf = new File(pkgPath);
            if (imgf.exists()) {
                Util.deleteFile(pkgPath);
            }
        }
        return upgradeStarted;
    }

    private String getResultFileName() {
        return mContext.getResources().getString(R.string.address_update_result);
    }

    private String getFullPathResultFileName() {

        if (mNandResultFilePath == null) {
            File dir = mContext.getFilesDir();
            String path = dir.getPath();
            mNandResultFilePath = path + "/" + getResultFileName();
        }
        Xlog.i(TAG, "mNandResultFilePath = " + mNandResultFilePath);
        return mNandResultFilePath;

    }

    private boolean getUpdateResult() {
        Xlog.i(TAG, "getUpdateResult");

        if (FeatureOption.MTK_EMMC_SUPPORT) {
            try {
                IBinder binder = ServiceManager.getService("GoogleOtaBinder");
                SystemUpdateBinder agent = SystemUpdateBinder.Stub.asInterface(binder);
                if (agent == null) {
                    Xlog.e(TAG, "Agent is null.");
                    return false;
                }
                return agent.readUpgradeResult();

            } catch (RemoteException e) {

                e.printStackTrace();
                return false;
            }
        } else {

            String strCurrentVersion = SystemProperties.get("ro.build.display.id");

            if (strCurrentVersion != null) {
                strCurrentVersion = strCurrentVersion.trim();

                Xlog.i(TAG, "strCurrentVersion = " + strCurrentVersion);
            }
            DownloadInfo downloadInfo = DownloadInfo.getInstance(mContext);

            String strTargetVersion = downloadInfo.getTargetVer();
            if (strTargetVersion != null) {
                strTargetVersion = strTargetVersion.trim();
                Xlog.i(TAG, "strTargetVersion = " + strTargetVersion);
            }

            return (strTargetVersion.equalsIgnoreCase(strCurrentVersion));

            /*
             * try { File file = new File(getFullPathResultFileName()); if
             * (!file.exists()) { Xlog.i(TAG,
             * "getUpdateResult, report file not exist"); return false; }
             * InputStream resStr = mContext
             * .openFileInput(getResultFileName()); if (resStr == null) {
             * Xlog.i(TAG, "getUpdateResult, inputstream error"); return false;
             * } byte[] b = new byte[10]; int num = resStr.read(b); Xlog.i(TAG,
             * "getUpdateResult, num = " + num); if (num < 0) { return false; }
             * resStr.close(); String string = new String(b, 0, num); int result
             * = Integer.valueOf(string); Xlog.i(TAG,
             * "getUpdateResult, result = " + result);
             * 
             * return (result == 1) ? true : false;
             * 
             * } catch (IOException e) { e.printStackTrace(); return false; }
             */

        }

    }

    private void resetDescriptionInfo(DownloadInfo ds) {
        Xlog.i(TAG, "resetDescriptionInfo");

        deleteUpdatePkg();
        ds.resetDownloadInfo();

    }

    private void startReportActivity() {

        boolean result = getUpdateResult();
        Xlog.i(TAG, "getUpdateResult=" + result);
        Intent resultNotify = new Intent(mContext, ForegroundDialogService.class);
        resultNotify.putExtra(ForegroundDialogService.DLG_ID, ForegroundDialogService.DIALOG_UPDATE_RESULT);
        resultNotify.putExtra(ForegroundDialogService.UPGRADE_RESULT_KEY, result);
        mContext.startService(resultNotify);

        resetUpdateResult();
    }

    private void deleteUpdatePkg() {
        String strPkgPath = Util.getPackageFileName(mContext);

        if (strPkgPath != null) {
            File imgf = new File(strPkgPath);
            if (imgf.exists()) {
                Xlog.i(TAG, "resetDescriptionInfo, image exist, delete it");
                Util.deleteFile(strPkgPath);
            }
        }
    }

    private void resetUpdateResult() {
        Xlog.i(TAG, "deleteUpdateResult");
        DownloadInfo downloadStatus = DownloadInfo.getInstance(mContext.getApplicationContext());
        downloadStatus.setUpgradeStartedState(false);
        File file = new File(getFullPathResultFileName());

        if (file.exists()) {
            Util.deleteFile(getFullPathResultFileName());
        }

    }
}
