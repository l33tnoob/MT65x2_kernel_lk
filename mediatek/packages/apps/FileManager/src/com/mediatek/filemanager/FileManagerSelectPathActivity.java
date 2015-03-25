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

package com.mediatek.filemanager;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageButton;

import com.mediatek.filemanager.service.FileManagerService;
import com.mediatek.filemanager.service.FileManagerService.OperationEventListener;
import com.mediatek.filemanager.service.ProgressInfo;
import com.mediatek.filemanager.utils.LogUtils;

import java.io.File;


/**
 * The activity is used by other application to select a Path. It will return the selected Folder's
 * path in intent.
 * 
 * <pre>
 * Intent intent = new Intent();
 * intent.putExtra(DOWNLOAD_PATH_KEY, mCurrentDirPath);
 * setResult(RESULT_OK, intent);
 * </pre>
 */
public class FileManagerSelectPathActivity extends AbsBaseActivity {
    private static final String TAG = "FileManagerSelectPathActivity";

    /** the path key for return selected path */
    public static final String DOWNLOAD_PATH_KEY = "download path";

    private Button mBtnSave = null;
    private ImageButton mBtnCreateFolder = null;

    private static final int SHOW_PATH = 1;
    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.what == SHOW_PATH) {
                showDirectoryContent((String) msg.obj);
            }
        };
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void setMainContentView() {
        setContentView(R.layout.select_path_main);

        mBtnSave = (Button) findViewById(R.id.download_btn_save);
        mBtnSave.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                // if (mMountHelper.checkRootPathMount(mCurrentDirPath)) {
                Intent intent = new Intent();
                intent.putExtra(DOWNLOAD_PATH_KEY, mCurrentPath);
                LogUtils.i(TAG, "setMainContentView,OK confirmed,Current Path = " + mCurrentPath);
                setResult(RESULT_OK, intent);
                finish();
            }
        });

        Button btnCancel = (Button) findViewById(R.id.download_btn_cancel);
        btnCancel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                setResult(RESULT_CANCELED, intent);
                finish();

            }
        });

        mBtnCreateFolder = (ImageButton) findViewById(R.id.btn_create_folder);
        mBtnCreateFolder.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                showCreateFolderDialog();
            }
        });
    }

    @Override
    protected void serviceConnected() {
        LogUtils.i(TAG, "serviceConnected...");
        super.serviceConnected();
        //mService.setListType(FileManagerService.FILE_FILTER_TYPE_FOLDER, this.getClass().getName());
        mService.setListType(FileManagerService.FILE_FILTER_TYPE_ALL, this.getClass().getName());
    }

    @Override
    protected void onPause() {
        LogUtils.d(TAG, "onPause...");
        if (mService != null) {
            mService.setListType(FileManagerService.FILE_FILTER_TYPE_DEFAULT, this.getClass()
                    .getName());
        }
        super.onPause();
    }

    @Override
    protected String initCurrentFileInfo() {
        LogUtils.d(TAG, "initCurrentFileInfo...");
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            final String downloadPathKey = extras.getString(DOWNLOAD_PATH_KEY);
            if (downloadPathKey != null) {
                // SD card is mounted?
                if (mMountPointManager.isRootPathMount(downloadPathKey)) {
                    LogUtils.d(TAG, "initCurrentFileInfo,downloadPathKey = " + downloadPathKey);
                    if (mService != null) {
                        mService.createFolder(this.getClass().getName(), downloadPathKey,
                                new OperationEventListener() {
                                    @Override
                                    public void onTaskResult(int result) {
                                        if (result == ERROR_CODE_SUCCESS
                                                || result == ERROR_CODE_FILE_EXIST) {
                                            Message.obtain(mHandler, SHOW_PATH, downloadPathKey)
                                                    .sendToTarget();
                                        } else {
                                            Message.obtain(mHandler, SHOW_PATH,
                                                    mMountPointManager.getRootPath())
                                                    .sendToTarget();
                                        }
                                    }

                                    @Override
                                    public void onTaskProgress(ProgressInfo progressInfo) {
                                        return;
                                    }

                                    @Override
                                    public void onTaskPrepare() {
                                        return;
                                    }
                                });
                        return null;
                    }
                }
            }
        }

        return mMountPointManager.getRootPath();
    }

    @Override
    protected void onPathChanged() {
        LogUtils.d(TAG, "onPathChanged...");
        super.onPathChanged();
        boolean enable = false;
        if (mMountPointManager.isRootPathMount(mCurrentPath)) {
            if (new File(mCurrentPath).canWrite()) {
                enable = true;
            }
        }
        updateButtonsState(enable);
    }

    /**
     * To update states of buttons shown in FileManagerSelectPathActivity.
     * 
     * @param flag to determine weather those button is Enable and Clickable or not.
     */
    private void updateButtonsState(boolean flag) {
        LogUtils.d(TAG, "updateButtonsState flag=" + flag);
        mBtnSave.setEnabled(flag);
        mBtnSave.setClickable(flag);
        mBtnCreateFolder.setEnabled(flag);
        mBtnCreateFolder.setClickable(flag);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (mService != null && mService.isBusy(this.getClass().getName())) {
            LogUtils.d(TAG, "onItemClick,service is busy,return.");
            return;
        }

        FileInfo selecteItemFileInfo = (FileInfo) parent.getItemAtPosition(position);
        if (!selecteItemFileInfo.isDirectory()) {
            LogUtils.d(TAG, "onItemClick,click file,return.");
            return;
        }

        int top = view.getTop();
        LogUtils.v(TAG, "top = " + top);
        addToNavigationList(mCurrentPath, selecteItemFileInfo, top);
        showDirectoryContent(selecteItemFileInfo.getFileAbsolutePath());
    }
}
