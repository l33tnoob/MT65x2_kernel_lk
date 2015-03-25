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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.IContentProvider;
import android.database.sqlite.SQLiteFullException;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.text.TextUtils;

import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.OptionsUtils;

import java.util.List;

public final class MediaStoreHelper {

    private static final String TAG = "MediaStoreHelper";
    private final Context mContext;
    private BaseAsyncTask mBaseAsyncTask;
    private String mDstFolder;
    private static final int SCAN_FOLDER_NUM = 20;
    /**
     * Constructor of MediaStoreHelper
     * 
     * @param context
     *            the Application context
     */
    public MediaStoreHelper(Context context) {
        mContext = context;
    }

    public MediaStoreHelper(Context context, BaseAsyncTask baseAsyncTask) {
        mContext = context;
        mBaseAsyncTask = baseAsyncTask;
    }

    public void updateInMediaStore(String newPath, String oldPath) {
        LogUtils.d(TAG, "updateInMediaStore,newPath = " + newPath + ",oldPath = " + oldPath);
        if (mContext != null && !TextUtils.isEmpty(newPath) && !TextUtils.isEmpty(newPath)) {
            Uri uri = MediaStore.Files.getMtpObjectsUri("external");
            uri = uri.buildUpon().appendQueryParameter("need_update_media_values", "true").build();

            String where = MediaStore.Files.FileColumns.DATA + "=?";
            String[] whereArgs = new String[] { oldPath };

            ContentResolver cr = mContext.getContentResolver();
            ContentValues values = new ContentValues();
            values.put(MediaStore.Files.FileColumns.DATA, newPath);
            whereArgs = new String[] { oldPath };
            if (!OptionsUtils.isMtkSDSwapSurpported()) {
                try {
                    LogUtils.d(TAG, "updateInMediaStore,update.");
                    cr.update(uri, values, where, whereArgs);

                    // mediaProvicer.update() only update data columns of
                    // database, it is need to other fields of the database, so scan the
                    // new path after update(). see ALPS00416588
                    scanPathforMediaStore(newPath);
                } catch (NullPointerException e) {
                    LogUtils.e(TAG, "Error, NullPointerException:" + e + ",update db may failed!!!");
                } catch (SQLiteFullException e) {
                    LogUtils.e(TAG, "Error, database or disk is full!!!" + e);
                    if (mBaseAsyncTask != null) {
                        mBaseAsyncTask.cancel(true);
                    }
                } catch (UnsupportedOperationException e) {
                    LogUtils.e(TAG, "Error, database is closed!!!");
                }
            } else {
                try {
                    LogUtils.d(TAG, "updateInMediaStore,update.");
                    cr.update(uri, values, where, whereArgs);

                    // mediaProvicer.update() only update data columns of
                    // database, it is need to other fields of the database, so scan the
                    // new path after update(). see ALPS00416588
                    scanPathforMediaStore(newPath);
                } catch (UnsupportedOperationException e) {
                    LogUtils.e(TAG, "Error, database is closed!!!");
                } catch (NullPointerException e) {
                    LogUtils.e(TAG, "Error, NullPointerException:" + e + ",update db may failed!!!");
                } catch (SQLiteFullException e) {
                    LogUtils.e(TAG, "Error, database or disk is full!!!" + e);
                    if (mBaseAsyncTask != null) {
                        mBaseAsyncTask.cancel(true);
                    }
                }
            }
        }
    }

    /**
     * scan Path for new file or folder in MediaStore
     * 
     * @param path
     *            the scan path
     */
    public void scanPathforMediaStore(String path) {
        LogUtils.d(TAG, "scanPathforMediaStore.path =" + path);
        if (mContext != null && !TextUtils.isEmpty(path)) {
            String[] paths = { path };
            LogUtils.d(TAG, "scanPathforMediaStore,scan file .");
            MediaScannerConnection.scanFile(mContext, paths, null, null);
        }
    }

    public void scanPathforMediaStore(List<String> scanPaths) {
        LogUtils.d(TAG, "scanPathforMediaStore,scanPaths.");
        int length = scanPaths.size();
        if (mContext != null && length > 0) {
            String[] paths;
            if (mDstFolder != null && length > SCAN_FOLDER_NUM) {
                paths = new String[] { mDstFolder };
            } else {
                paths = new String[length];
                scanPaths.toArray(paths);
            }

            LogUtils.d(TAG, "scanPathforMediaStore, scanFiles.");
            MediaScannerConnection.scanFile(mContext, paths, null, null);
        }
    }

    /**
     * delete the record in MediaStore
     * 
     * @param paths
     *            the delete file or folder in MediaStore
     */
    public void deleteFileInMediaStore(List<String> paths) {
        LogUtils.d(TAG, "deleteFileInMediaStore.");
        Uri uri = MediaStore.Files.getContentUri("external");
        StringBuilder whereClause = new StringBuilder();
        whereClause.append("?");
        for (int i = 0; i < paths.size() - 1; i++) {
            whereClause.append(",?");
        }
        String where = MediaStore.Files.FileColumns.DATA + " IN(" + whereClause.toString() + ")";
        // notice that there is a blank before "IN(".
        if (mContext != null && !paths.isEmpty()) {
            ContentResolver cr = mContext.getContentResolver();
            String[] whereArgs = new String[paths.size()];
            paths.toArray(whereArgs);
            LogUtils.d(TAG, "deleteFileInMediaStore,delete.");
            try {
                cr.delete(uri, where, whereArgs);
            } catch (SQLiteFullException e) {
                LogUtils.e(TAG, "Error, database or disk is full!!!" + e);
                if (mBaseAsyncTask != null) {
                    mBaseAsyncTask.cancel(true);
                }
            } catch (UnsupportedOperationException e) {
                LogUtils.e(TAG, "Error, database is closed!!!");
                if (mBaseAsyncTask != null) {
                    mBaseAsyncTask.cancel(true);
                }
            }
        }
    }

    /**
     * delete the record in MediaStore
     * 
     * @param path
     *            the delete file or folder in MediaStore
     */
    public void deleteFileInMediaStore(String path) {
        LogUtils.d(TAG, "deleteFileInMediaStore,path =" + path);
        if (TextUtils.isEmpty(path)) {
            return;
        }
        Uri uri = MediaStore.Files.getContentUri("external");
        String where = MediaStore.Files.FileColumns.DATA + "=?";
        String[] whereArgs = new String[] { path };
        if (mContext != null) {
            ContentResolver cr = mContext.getContentResolver();
            LogUtils.d(TAG, "deleteFileInMediaStore,delete.");
            try {
                if (!OptionsUtils.isMtkSDSwapSurpported()) {
                    cr.delete(uri, where, whereArgs);
                } else {
                    try {
                        cr.delete(uri, where, whereArgs);
                    } catch (UnsupportedOperationException e) {
                        LogUtils.e(TAG, "Error, database is closed!!!");
                        if (mBaseAsyncTask != null) {
                            mBaseAsyncTask.cancel(true);
                        }
                    }
                }
            } catch (SQLiteFullException e) {
                LogUtils.e(TAG, "Error, database or disk is full!!!" + e);
                if (mBaseAsyncTask != null) {
                    mBaseAsyncTask.cancel(true);
                }
            }
        }
    }

    /**
     * Set dstfolder so when scan files size more than SCAN_FOLDER_NUM use folder
     * path to make scanner scan this folder directly.
     * 
     * @param dstFolder
     */
    public void setDstFolder(String dstFolder) {
        mDstFolder = dstFolder;
    }
}