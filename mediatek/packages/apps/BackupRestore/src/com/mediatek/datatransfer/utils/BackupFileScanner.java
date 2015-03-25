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

package com.mediatek.datatransfer.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Handler;
import android.os.Message;
import android.telephony.SmsMessage;
import android.telephony.SmsMessage.SubmitPdu;

import com.mediatek.datatransfer.BackupService;
import com.mediatek.datatransfer.R;
import com.mediatek.datatransfer.RecordXmlComposer;
import com.mediatek.datatransfer.RecordXmlInfo;
import com.mediatek.datatransfer.modules.MmsXmlComposer;
import com.mediatek.datatransfer.modules.MmsXmlInfo;
import com.mediatek.datatransfer.modules.SmsBackupComposer;
import com.mediatek.datatransfer.utils.Constants.MessageID;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;

public class BackupFileScanner {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupFileScanner";
    private Handler mHandler;
    private Context mContext;
    private Object object = new Object();
    private LinkedHashSet<BackupsHandler> scanTaskHandlers = new LinkedHashSet<BackupsHandler>();

    public BackupFileScanner(Context context, Handler handler) {
        mHandler = handler;
        mContext = context;
        scanTaskHandlers.clear();
        if (mHandler == null) {
            MyLogger.logE(CLASS_TAG, "constuctor maybe failed!cause mHandler is null");
        }
    }

    public boolean addScanHandler(BackupsHandler backupsHandler) {
        return scanTaskHandlers.add(backupsHandler);
    }

    public void setHandler(Handler handler) {
        synchronized (object) {
            mHandler = handler;
        }
    }

    ScanThread mScanThread;

    public void startScan() {
        mScanThread = new ScanThread();
        mScanThread.start();
    }

    public void quitScan() {
        synchronized (object) {
            if (mScanThread != null) {
                mScanThread.cancel();
                mScanThread = null;
                MyLogger.logV(CLASS_TAG, "quitScan");
            }
        }
    }

    public boolean isRunning() {
        if (mScanThread != null) {
            return mScanThread.isCanceled;
        }
        return false;
    }

    private class ScanThread extends Thread {
        boolean isCanceled = false;

        public void cancel() {
            isCanceled = true;
        }

        private File[] filterFile(File[] fileList) {
            if (fileList == null) {
                return null;
            }
            List<File> list = new ArrayList<File>();
            for (File file : fileList) {
                if (isCanceled) {
                    break;
                }

                if (!FileUtils.isEmptyFolder(file)) {
                    list.add(file);
                }
            }
            if (isCanceled) {
                return null;
            } else {
                return (File[]) list.toArray(new File[0]);
            }
        }

        private File[] scanBackupFiles(String path) {
            
            if (path != null && !isCanceled) {
                return filterFile(new File(path).listFiles());
            } else {
                return null;
            }
        }

        private File[] scanPersonalBackupFiles() {
            String path = SDCardUtils.getPersonalDataBackupPath(mContext);
            return scanBackupFiles(path);
        }

        @Override
        public void run() {
            isCanceled = false;
            if (scanTaskHandlers != null && !scanTaskHandlers.isEmpty()) {
                detectSDcardForOtherBackups();
            } else {
                initDataTransferBackupForList();
            }
        }

        private void detectSDcardForOtherBackups() {
            int notifyType = NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_DEAFAULT;
            List<File> result = new ArrayList<File>();
            for (BackupsHandler backupsHandler : scanTaskHandlers) {

                if (isCanceled) {
                    backupsHandler.cancel();
                    continue;
                }

                backupsHandler.reset();

                if (!backupsHandler.init()) {
                    continue;
                }

                backupsHandler.onStart();

                List<File> oneResult = backupsHandler.onEnd();
                if (oneResult != null) {
                    result.addAll(oneResult);
                }

                String backupType = backupsHandler.getBackupType();
                if (backupType.equals(Constants.BackupScanType.DATATRANSFER)) {
                    MyLogger.logV(CLASS_TAG, "Type is DATATRANSFER");
                    notifyType = isOtherBackup();
                }
            }
            String path = null;
            if (!result.isEmpty()) {
                List<String> newDetectList = checkMD5(result);
                if (result != null) {
                    try {
                        path = makeOneBackupHistory(result);
                        addMD5toPreference(newDetectList);
                    } catch (Exception e) {
                        // TODO Auto-generated catch block
                        MyLogger.logE(CLASS_TAG, "we must do some rollback option here.");
                        e.printStackTrace();
                        if (path != null && !path.equals("")) {
                            FileUtils.deleteFileOrFolder(new File(path));
                        }
                        return;
                    }
                }
            }
            if (notifyType == NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_LIST || !result.isEmpty()) {
                MyLogger.logE(CLASS_TAG, "notifyType = " + notifyType + "  path = " + path);
                NotifyManager.getInstance(mContext).showNewDetectionNotification(notifyType, path);
            }
            isCanceled = (isCanceled ? !isCanceled : isCanceled);
        }

        private int isOtherBackup() {
            // TODO Auto-generated method stub
            File[] files = scanPersonalBackupFiles();
            BackupFilePreview temPreview;
            if (files != null && files.length > 0) {
                for (File file : files) {
                    temPreview = new BackupFilePreview(file);
                    MyLogger.logW(CLASS_TAG, "temPreview.isOtherDeviceBackup() = "
                            + temPreview.isOtherDeviceBackup());
                    /*
                     * there are two kind cases need to notify list1.Found other
                     * Phone's backup on SDCard2.When (boot reset or clear
                     * application data) && some non-self backup history exist
                     */
                    if (temPreview.isOtherDeviceBackup()) {
                        return NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_LIST;
                    } else if ((isBootReset(false) && !temPreview.isSelfBackup())) {
                        MyLogger.logD(CLASS_TAG,
                                "Find other Phone's backup on SDCard && boot has been reseted "
                                        + !temPreview.isSelfBackup());
                        isBootReset(true);
                        return NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_LIST;
                    }
                }
            }
            return NotifyManager.FP_NEW_DETECTION_NOTIFY_TYPE_DEAFAULT;
        }

        /**
         * find the tag file in data,it'll be delete when phone reset.
         * 
         * @return true is reset
         */
        private boolean isBootReset(boolean insert) {
            // TODO Auto-generated method stub
            SharedPreferences sp = mContext.getSharedPreferences("boot_reset_flag",
                    Context.MODE_PRIVATE);
            if (insert) {
                sp.edit().putBoolean("boot_reset", false).commit();
            }
            if (sp.getBoolean("boot_reset", true)) {
                return true;
            }
            return false;
        }

        private void addMD5toPreference(List<String> newDetectList) {
            // TODO Auto-generated method stub
            SharedPreferences preferences = mContext.getSharedPreferences("md5_info",
                    Context.MODE_PRIVATE);
            Editor editor = preferences.edit();
            for (String md5 : newDetectList) {
                editor.putInt(md5, Constants.BackupScanType.MD5_EXIST);
            }
            editor.commit();
        }

        private String makeOneBackupHistory(List<File> result) throws IOException {
            // TODO Auto-generated method stub
            File newestFile = FileUtils.getNewestFile(result);
            if (newestFile == null) {
                return null;
            }
            long historySecond = newestFile.lastModified();
            SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmmss");
            String historyName = format.format(new Date(historySecond));
            String path = SDCardUtils.getPersonalDataBackupPath(mContext) + File.separator
                    + historyName;
            File destFile = new File(path);
            MyLogger.logV(CLASS_TAG, "makeOneBackupHistory <=====>path  " + path);
            if (destFile != null && destFile.exists() && destFile.isDirectory()
                    && !FileUtils.isEmptyFolder(destFile)) {
                MyLogger.logV(CLASS_TAG, "makeOneBackupHistory <=====>file has been exist ");
                return path;
            }
            destFile = FileUtils.createFile(path);
            File contactDestFile = FileUtils.createFile(path + File.separator
                    + Constants.ModulePath.FOLDER_CONTACT);
            File mmsDestFile = FileUtils.createFile(path + File.separator
                    + Constants.ModulePath.FOLDER_MMS);
            File smsDestFile = FileUtils.createFile(path + File.separator
                    + Constants.ModulePath.FOLDER_SMS);
            for (File file : result) {
                String name = file.getName();
                if (name.endsWith(".vcf")) { // find it is a contact file
                    org.apache.commons.io.FileUtils.copyFileToDirectory(file, contactDestFile);
                    MyLogger.logV(CLASS_TAG, "makeOneBackupHistory <=====>file = " + file.getName()
                            + "TO: " + destFile.getName());
                } else if (name.endsWith(".s") || name.endsWith(".m")) {
                    org.apache.commons.io.FileUtils.copyFileToDirectory(file, mmsDestFile);
                } else if (name.equals("sms.vmsg")) {
                    org.apache.commons.io.FileUtils.copyFileToDirectory(file, smsDestFile);
                }
            }

            FileUtils.combineFiles(Arrays.asList(contactDestFile.listFiles()), contactDestFile
                    .getAbsolutePath()
                    + File.separator + Constants.ModulePath.NAME_CONTACT);
            for (File file : contactDestFile.listFiles()) {
                if (file.getName().equals(Constants.ModulePath.NAME_CONTACT)) {
                    continue;
                }
                FileUtils.deleteFileOrFolder(file);
            }
            // add MMS record xml file
            if (mmsDestFile != null && mmsDestFile.isDirectory()
                    && mmsDestFile.listFiles().length > 0) {
                MmsXmlComposer composer = new MmsXmlComposer();
                composer.startCompose();
                for (File file : mmsDestFile.listFiles()) {
                    MmsXmlInfo record = new MmsXmlInfo();
                    record.setID(file.getName());
                    record.setIsRead("1");
                    record.setMsgBox(file.getName().endsWith(".m") ? "1" : "2");
                    record.setDate("" + file.lastModified());
                    record.setSize("" + file.length());
                    record.setSimId("0");
                    record.setIsLocked("0");
                    composer.addOneMmsRecord(record);
                }
                composer.endCompose();
                String xmlInfoString = composer.getXmlInfo();
                FileUtils.writeToFile(path + File.separator + ModulePath.FOLDER_MMS
                        + File.separator + ModulePath.MMS_XML, xmlInfoString.getBytes());
            }

            makeRootRecordXml(historySecond, path, destFile);
            FileUtils.deleteEmptyFolder(destFile);
            return path;
        }

        private void makeRootRecordXml(long historySecond, String path,
                File destFile) {
            RecordXmlInfo backupInfo = new RecordXmlInfo();
            backupInfo.setRestore(false);
            backupInfo.setDevice(Utils.getPhoneSearialNumber());
            backupInfo.setTime("" + historySecond);
            RecordXmlComposer xmlCompopser = new RecordXmlComposer();
            xmlCompopser.startCompose();
            xmlCompopser.addOneRecord(backupInfo);
            xmlCompopser.endCompose();
            if (destFile != null && destFile.canWrite()) {
                Utils.writeToFile(xmlCompopser.getXmlInfo(), path + File.separator
                        + Constants.RECORD_XML);
            }
        }

        private List<String> checkMD5(List<File> result) {
            // TODO Auto-generated method stub
            SharedPreferences preferences = mContext.getSharedPreferences("md5_info",
                    Context.MODE_PRIVATE);
            List<String> newDetectList = new ArrayList<String>();
            List<File> removeList = new ArrayList<File>();
            for (File file : result) {
                if (file == null) {
                    MyLogger.logE(CLASS_TAG, "[checkMD5] file is null ,continue!");
                    continue;
                }
                String md5 = FileUtils.getFileMD5(file.getAbsolutePath());
                int md5_flag = preferences.getInt(md5, Constants.BackupScanType.MD5_NOT_EXIST);
                if (md5_flag == Constants.BackupScanType.MD5_NOT_EXIST) {
                    newDetectList.add(md5);
                } else if (md5_flag == Constants.BackupScanType.MD5_EXIST) {
                    removeList.add(file);
                }
            }
            result.removeAll(removeList);
            return newDetectList;
        }

        private void initDataTransferBackupForList() {
            HashMap<String, List<BackupFilePreview>> result = getDataTransferBackups();
            synchronized (object) {
                if (!isCanceled && mHandler != null) {
                    Message msg = mHandler.obtainMessage(MessageID.SCANNER_FINISH, result);
                    mHandler.sendMessage(msg);
                }
            }
            mScanThread = null;
        }
        
        private HashMap<String, List<BackupFilePreview>> getDataTransferBackups() {
            File[] files = scanPersonalBackupFiles();
            HashMap<String, List<BackupFilePreview>> result = new HashMap<String, List<BackupFilePreview>>();
            List<BackupFilePreview> backupItems = generateBackupFileItems(files);
            result.put(Constants.SCAN_RESULT_KEY_PERSONAL_DATA, backupItems);
            String path = SDCardUtils.getAppsBackupPath(mContext);
            if (path == null || path.trim().equals("")) {
                return null;
            }
            File appFolderFile = new File(path);
            backupItems = new ArrayList<BackupFilePreview>();
            BackupFilePreview appBackupFile = null;
            if (appFolderFile.exists()) {
                appBackupFile = new BackupFilePreview(appFolderFile);
                backupItems.add(appBackupFile);
            }
            result.put(Constants.SCAN_RESULT_KEY_APP_DATA, backupItems);
            return result;
        }

        private List<BackupFilePreview> generateBackupFileItems(File[] files) {
            if (files == null || isCanceled) {
                return null;
            }
            List<BackupFilePreview> list = new ArrayList<BackupFilePreview>();
            for (File file : files) {
                if (isCanceled) {
                    break;
                }
                BackupFilePreview backupFile = new BackupFilePreview(file);
                if (backupFile != null) {
                    if (backupFile.getFileSize() > 1024 * 2) {
                        list.add(backupFile);
                    } else {
                        if (backupFile.getBackupModules(mContext) > 0) {
                            list.add(backupFile);
                        }
                    }
                }
            }
            if (!isCanceled) {
                sort(list);
                return list;
            } else {
                return null;
            }
        }

        private void sort(List<BackupFilePreview> list) {
            Collections.sort(list, new Comparator<BackupFilePreview>() {
                public int compare(BackupFilePreview object1, BackupFilePreview object2) {
                    String dateLeft = object1.getBackupTime();
                    String dateRight = object2.getBackupTime();
                    if (dateLeft != null && dateRight != null) {
                        return dateRight.compareTo(dateLeft);
                    } else {
                        return 0;
                    }
                }
            });
        }
    }

}
