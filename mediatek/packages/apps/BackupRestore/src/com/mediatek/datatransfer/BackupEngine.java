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

package com.mediatek.datatransfer;

import android.content.Context;
import android.util.Log;

import com.mediatek.datatransfer.modules.AppBackupComposer;
import com.mediatek.datatransfer.modules.BookmarkBackupComposer;
import com.mediatek.datatransfer.modules.CalendarBackupComposer;
import com.mediatek.datatransfer.modules.Composer;
import com.mediatek.datatransfer.modules.ContactBackupComposer;
import com.mediatek.datatransfer.modules.MessageBackupComposer;
import com.mediatek.datatransfer.modules.MmsBackupComposer;
import com.mediatek.datatransfer.modules.MusicBackupComposer;
import com.mediatek.datatransfer.modules.NoteBookBackupComposer;
import com.mediatek.datatransfer.modules.PictureBackupComposer;
import com.mediatek.datatransfer.modules.SmsBackupComposer;
import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.FileUtils;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.Constants.LogTag;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class BackupEngine {

    public interface OnBackupDoneListner {
        public void onFinishBackup(BackupResultType result);
    }

    public enum BackupResultType {
        Success, Fail, Error, Cancel
    }

    private Context mContext;
    private ProgressReporter mProgressReporter;
    private List<Composer> mComposerList;
    private OnBackupDoneListner mBackupDoneListner;
    private boolean mIsRunning = false;
    private boolean mIsPause = false;
    private boolean mIsCancel = false;
    private Object mLock = new Object();
    private String mBackupFolder;
    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();

    private static BackupEngine mSelfInstance;

    public static BackupEngine getInstance(final Context context, final ProgressReporter reporter) {
        if (mSelfInstance == null) {
            new BackupEngine(context, reporter);
        } else {
            mSelfInstance.updateInfo(context, reporter);
        }

        return mSelfInstance;
    }

    public BackupEngine(final Context context, final ProgressReporter reporter) {
        mContext = context;
        mProgressReporter = reporter;
        mComposerList = new ArrayList<Composer>();
        mSelfInstance = this;
    }

    ArrayList<Integer> mModuleList;

    public void setBackupModelList(ArrayList<Integer> moduleList) {
        reset();
        mModuleList = moduleList;
    }

    public void setBackupItemParam(int itemType, ArrayList<String> paraList) {
        mParasMap.put(itemType, paraList);
    }

    public boolean startBackup(final String folderName) {
        boolean startSuccess = true;
        mBackupFolder = folderName;
        Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "startBackup():" + folderName);

        if (setupComposer(mModuleList)) {
            Utils.isBackingUp = mIsRunning = true;
            new BackupThread().start();
        } else {
            startSuccess = false;
        }
        return startSuccess;
    }

    public final boolean isRunning() {
        return mIsRunning;
    }

    private final void updateInfo(final Context context, final ProgressReporter reporter) {
        mContext = context;
        mProgressReporter = reporter;
    }

    public final void pause() {
        mIsPause = true;
    }

    public final boolean isPaused() {
        return mIsPause;
    }

    public final void continueBackup() {
        if (mIsPause) {
            synchronized (mLock) {
                mIsPause = false;
                mLock.notify();
            }
        }
    }

    public final void cancel() {
        if (mComposerList != null && mComposerList.size() > 0) {
            for (Composer composer : mComposerList) {
                composer.setCancel(true);
            }
            mIsCancel = true;
            continueBackup();
        }
    }

    public final void setOnBackupDoneListner(final OnBackupDoneListner listner) {
        mBackupDoneListner = listner;
    }

    private void addComposer(final Composer composer) {
        if (composer != null) {
            int type = composer.getModuleType();
            ArrayList<String> params = mParasMap.get(type);
            if (params != null) {
                composer.setParams(params);
            }
            composer.setReporter(mProgressReporter);
            composer.setParentFolderPath(mBackupFolder);
            mComposerList.add(composer);
        }
    }

    private void reset() {
        if (mComposerList != null) {
            mComposerList.clear();
        }

        if (mParasMap != null) {
            mParasMap.clear();
        }

        mIsPause = false;
        mIsCancel = false;
    }

    private boolean setupComposer(final ArrayList<Integer> list) {
        Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "setupComposer begin...");

        boolean result = true;
        File path = new File(mBackupFolder);
        if (!path.exists()) {
            result = path.mkdirs();
        }

        if (result) {
            Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "create folder " + mBackupFolder
                    + " success");

            for (int type : list) {
                switch (type) {
                case ModuleType.TYPE_CONTACT:
                    addComposer(new ContactBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_CALENDAR:
                    addComposer(new CalendarBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_SMS:
                    addComposer(new SmsBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_MMS:
                    addComposer(new MmsBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_MESSAGE:
                    addComposer(new MessageBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_APP:
                    addComposer(new AppBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_PICTURE:
                    addComposer(new PictureBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_MUSIC:
                    addComposer(new MusicBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_NOTEBOOK:
                    addComposer(new NoteBookBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_BOOKMARK:
                    addComposer(new BookmarkBackupComposer(mContext));
                    break;

                default:
                    result = false;
                    break;
                }
            }

            Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "setupComposer finish");
        } else {
            Log.e(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "setupComposer failed");
            result = false;
        }
        return result;
    }

    private class BackupThread extends Thread {
        @Override
        public void run() {
            BackupResultType result = BackupResultType.Fail;

            Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "BackupThread begin...");
            for (Composer composer : mComposerList) {
                Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "BackupThread->composer:"
                        + composer.getModuleType() + " start...");
                if (!composer.isCancel()) {
                    composer.init();
                    composer.onStart();
                    Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "BackupThread-> composer:"
                            + composer.getModuleType() + " init finish");
                    while (!composer.isAfterLast() && !composer.isCancel()) {
                        if (mIsPause) {
                            synchronized (mLock) {
                                try {
                                    Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG
                                            + "BackupThread wait...");
                                    mLock.wait();
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                        if (!composer.isCancel()) {
                            composer.composeOneEntity();
                            Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG
                                    + "BackupThread->composer:"
                                    + composer.getModuleType() + " compose one entiry");
                        }
                    }
                }

                try {
                    sleep(Constants.TIME_SLEEP_WHEN_COMPOSE_ONE);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                composer.onEnd();

                // generateModleXmlInfo(composer);

                Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "BackupThread-> composer:"
                        + composer.getModuleType() + " finish");
            }

            Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "BackupThread run finish, result:"
                    + result);
            Utils.isBackingUp = mIsRunning = false;
            if (mIsCancel) {
                result = BackupResultType.Cancel;
                if (!mModuleList.contains(ModuleType.TYPE_APP)) {
                    deleteFolder(new File(mBackupFolder));
                }
            } else {
                result = BackupResultType.Success;
            }

            if (mBackupDoneListner != null) {
                if (mIsPause) {
                    synchronized (mLock) {
                        try {
                            Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG
                                    + "BackupThread wait before end...");
                            mLock.wait();
                            if (mIsCancel) {
                                result = BackupResultType.Cancel;
                            }
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
                mBackupDoneListner.onFinishBackup(result);
            }
        }
    }

    // public void deleteFile() {
    // File file = new File(mBackupFolder);
    // FileUtils.deleteFileOrFolder(file);
    // Log.d(LogTag.LOG_TAG, LogTag.BACKUP_ENGINE_TAG + "deleteFile "
    // + mBackupFolder);
    // }

    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                file.delete();
            } else if (file.isDirectory()) {
                File files[] = file.listFiles();
                for (int i = 0; i < files.length; ++i) {
                    this.deleteFolder(files[i]);
                }
            }

            file.delete();
        }
    }

}
