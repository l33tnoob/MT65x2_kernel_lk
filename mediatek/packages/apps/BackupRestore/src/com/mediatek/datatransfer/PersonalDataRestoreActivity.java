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

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.datatransfer.RestoreService.RestoreProgress;
import com.mediatek.datatransfer.ResultDialog.ResultEntity;
import com.mediatek.datatransfer.modules.NoteBookXmlParser;
import com.mediatek.datatransfer.utils.BackupFilePreview;
import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.FileUtils;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;
import com.mediatek.datatransfer.utils.Utils;
import com.mediatek.datatransfer.utils.Constants.DialogID;
import com.mediatek.datatransfer.utils.Constants.ModulePath;
import com.mediatek.datatransfer.utils.Constants.State;

import java.io.File;
import java.util.ArrayList;

public class PersonalDataRestoreActivity extends AbstractRestoreActivity {
    private String CLASS_TAG = MyLogger.LOG_TAG + "/PersonalDataRestoreActivity";

    private PersonalDataRestoreAdapter mRestoreAdapter;
    private File mFile;
    private boolean mIsDataInitialed;
    private boolean mIsStoped = false;
    private boolean mNeedUpdateResult = false;
    BackupFilePreview mPreview = null;
    private PersonalDataRestoreStatusListener mRestoreStoreStatusListener;
    private boolean mIsCheckedRestoreStatus = false;
    private String mRestoreFolderPath;

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        if (intent == null || intent.getStringExtra(Constants.FILENAME) == null) {
            finish();
            return;
        }
        mFile = new File(intent.getStringExtra(Constants.FILENAME));
        if (!mFile.exists()) {
            Toast.makeText(this, "file don't exist", Toast.LENGTH_LONG);
            finish();
            return;
        }
        Log.i(CLASS_TAG, "onCreate");
        init();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "onResume");
        if (!mFile.exists()) {
            Toast.makeText(this, R.string.file_no_exist_and_update, Toast.LENGTH_LONG);
            finish();
            return;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        // update to avoid files deleted
        if (mFile.exists()) {
            new FilePreviewTask().execute();
            updateResultIfneed();
        } else {
            Toast.makeText(this, R.string.file_no_exist_and_update, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        mIsStoped = false;
    }

    @Override
    public void onStop() {
        super.onStop();
        mIsStoped = true;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mHandler = null;
    }

    private void init() {
        initActionBar();
        updateTitle();
    }

    public void updateTitle() {
        StringBuilder sb = new StringBuilder();
        sb.append(getString(R.string.backup_personal_data));
        int totalCount = getCount();
        int selectCount = getCheckedCount();
        sb.append("(" + selectCount + "/" + totalCount + ")");
        this.setTitle(sb.toString());
    }

    private void initActionBar() {
        ActionBar bar = this.getActionBar();
        bar.setDisplayShowHomeEnabled(false);
        bar.setTitle(mFile.getName());
        StringBuilder builder = new StringBuilder(getString(R.string.backup_data));
        builder.append(" ");
        long size = FileUtils.computeAllFileSizeInFolder(mFile);
        builder.append(FileUtils.getDisplaySize(size, this));
        bar.setSubtitle(builder.toString());
    }

    private void showUpdatingTitle() {
        ActionBar bar = this.getActionBar();
        bar.setTitle(R.string.updating);
        bar.setSubtitle(null);
    }

    @Override
    public void onCheckedCountChanged() {
        super.onCheckedCountChanged();
        updateTitle();
    }

    @Override
    protected void notifyListItemCheckedChanged() {
        super.notifyListItemCheckedChanged();
        updateTitle();
    }

    private ArrayList<Integer> getSelectedItemList() {
        ArrayList<Integer> list = new ArrayList<Integer>();
        int count = getCount();
        for (int position = 0; position < count; position++) {
            PersonalItemData item = (PersonalItemData) getItemByPosition(position);
            if (isItemCheckedByPosition(position)) {
                list.add(item.getType());
            }
        }
        return list;
    }

    @Override
    protected BaseAdapter initAdapter() {
        ArrayList<PersonalItemData> list = new ArrayList<PersonalItemData>();
        mRestoreAdapter = new PersonalDataRestoreAdapter(this, list,
                R.layout.restore_personal_data_item);
        return mRestoreAdapter;
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;

        switch (id) {
            case DialogID.DLG_RESULT:
                dialog = ResultDialog.createResultDlg(this, R.string.restore_result, args,
                        new OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (mRestoreService != null) {
                                    mRestoreService.reset();
                                }
                                stopService();
                                NotifyManager.getInstance(PersonalDataRestoreActivity.this)
                                        .clearNotification();
                            }
                        });
                break;

            default:
                dialog = super.onCreateDialog(id, args);
                break;
        }
        return dialog;
    }

    @Override
    protected void onPrepareDialog(final int id, final Dialog dialog, final Bundle args) {
        switch (id) {
            case DialogID.DLG_RESULT:
                AlertDialog dlg = (AlertDialog) dialog;
                ListView view = (ListView) dlg.getListView();
                if (view != null) {
                    ListAdapter adapter = ResultDialog.createResultAdapter(this, args);
                    view.setAdapter(adapter);
                }
                break;
            default:
                break;
        }
    }

    private void showRestoreResult(ArrayList<ResultEntity> list) {
        dismissProgressDialog();
        Bundle args = new Bundle();
        args.putParcelableArrayList("result", list);
        showDialog(DialogID.DLG_RESULT, args);
    }

    private void updateResultIfneed() {

        if (mRestoreService != null && mNeedUpdateResult) {
            int state = mRestoreService.getState();
            if (state == State.FINISH) {
                MyLogger.logV(CLASS_TAG, "updateResult because of finish when onStop");
                showRestoreResult(mRestoreService.getRestoreResult());
            }
        }
        mNeedUpdateResult = false;
    }

    private String getProgressDlgMessage(int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.restoring));

        builder.append("(").append(ModuleType.getModuleStringFromType(this, type)).append(")");
        return builder.toString();
    }

    @Override
    public void startRestore() {
        if (!isCanStartRestore()) {
            return;
        }
        startService();
        Log.v(CLASS_TAG, "startRestore");
        ArrayList<Integer> list = getSelectedItemList();
        if (list.size() == 0) {
            Toast.makeText(this, getString(R.string.no_item_selected), Toast.LENGTH_SHORT).show();
            return;
        }
        mRestoreService.setRestoreModelList(list);
        mRestoreFolderPath = mFile.getAbsolutePath();
        boolean ret = mRestoreService.startRestore(mRestoreFolderPath);
        if (ret) {
            showProgressDialog();
            String msg = getProgressDlgMessage(list.get(0));
            setProgressDialogMessage(msg);
            setProgressDialogProgress(0);
            setProgress(0);
            if (mPreview != null) {
                int count = mPreview.getItemCount(list.get(0));
                setProgressDialogMax(count);
            }
        } else {
            stopService();
        }
    }

    @Override
    protected void afterServiceConnected() {
        MyLogger.logD(CLASS_TAG, "afterServiceConnected, to checkRestorestate");
        checkRestoreState();
    }

    private void checkRestoreState() {
        if (mIsCheckedRestoreStatus) {
            MyLogger.logD(CLASS_TAG, "can not checkRestoreState, as it has been checked");
            return;
        }
        if (!mIsDataInitialed) {
            MyLogger.logD(CLASS_TAG, "can not checkRestoreState, wait data to initialed");
            return;
        }
        MyLogger.logD(CLASS_TAG, "all ready. to checkRestoreState");
        mIsCheckedRestoreStatus = true;
        if (mRestoreService != null) {
            int state = mRestoreService.getState();
            MyLogger.logD(CLASS_TAG, "checkRestoreState: state = " + state);
            if (state == State.RUNNING || state == State.PAUSE) {

                RestoreProgress p = mRestoreService.getCurRestoreProgress();
                if (p.mType == 0 || p.mMax == 0) {
                    MyLogger.logE(CLASS_TAG, "CurrentProgress is not availableMax = " + p.mMax
                            + " curprogress = " + p.mCurNum);
                    mRestoreService.reset();
                    return;
                }
                MyLogger.logD(CLASS_TAG, "checkRestoreState: Max = " + p.mMax + " curprogress = "
                        + p.mCurNum);
                String msg = getProgressDlgMessage(p.mType);

                if (state == State.RUNNING) {
                    showProgressDialog();
                }
                setProgressDialogMax(p.mMax);
                setProgressDialogProgress(p.mCurNum);
                setProgressDialogMessage(msg);
            } else if (state == State.FINISH) {
                if (mIsStoped) {
                    mNeedUpdateResult = true;
                } else {
                    showRestoreResult(mRestoreService.getRestoreResult());
                }
            } else if (state == State.ERR_HAPPEN) {
                errChecked();
            }
        }
    }

    @Override
    public void onConfigurationChanged(final Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e(CLASS_TAG, "onConfigurationChanged");
    }

    private class PersonalDataRestoreAdapter extends BaseAdapter {

        private ArrayList<PersonalItemData> mList;
        private int mLayoutId;
        private LayoutInflater mInflater;

        public PersonalDataRestoreAdapter(Context context, ArrayList<PersonalItemData> list,
                int resource) {
            mList = list;
            mLayoutId = resource;
            mInflater = LayoutInflater.from(context);
        }

        public void changeData(ArrayList<PersonalItemData> list) {
            mList = list;
        }

        public int getCount() {
            if (mList == null) {
                return 0;
            }
            return mList.size();
        }

        public Object getItem(int position) {
            if (mList == null) {
                return null;
            }
            return mList.get(position);

        }

        public long getItemId(int position) {
            PersonalItemData item = mList.get(position);
            return item.getType();

        }

        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(mLayoutId, parent, false);
            }

            PersonalItemData item = mList.get(position);
            ImageView image = (ImageView) view.findViewById(R.id.item_image);
            TextView textView = (TextView) view.findViewById(R.id.item_text);
            CheckBox chxbox = (CheckBox) view.findViewById(R.id.item_chkbox);
            image.setBackgroundResource(item.getIconId());
            textView.setText(item.getTextId());
            boolean enabled = item.isEnable();
            textView.setEnabled(enabled);
            chxbox.setEnabled(enabled);
            view.setClickable(!enabled);
            if (enabled) {
                chxbox.setChecked(isItemCheckedByPosition(position));
            } else {
                chxbox.setChecked(false);
                view.setEnabled(false);
            }
            return view;
        }
    }

    private class FilePreviewTask extends AsyncTask<Void, Void, Long> {
        private int mModule = 0;

        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            int types[] = new int[] {
                    ModuleType.TYPE_CONTACT, ModuleType.TYPE_MESSAGE,
                    ModuleType.TYPE_PICTURE, ModuleType.TYPE_CALENDAR, ModuleType.TYPE_MUSIC,
                    ModuleType.TYPE_BOOKMARK
            };

            ArrayList<PersonalItemData> list = new ArrayList<PersonalItemData>();
            for (int type : types) {
                if ((mModule & type) != 0) {
                    PersonalItemData item = new PersonalItemData(type, true);
                    list.add(item);
                }
            }
            mRestoreAdapter.changeData(list);
            syncUnCheckedItems();
            setButtonsEnable(true);
            notifyListItemCheckedChanged();
            mIsDataInitialed = true;
            // setProgressBarIndeterminateVisibility(false);
            showLoadingContent(false);
            initActionBar();
            if (mRestoreStoreStatusListener == null) {
                mRestoreStoreStatusListener = new PersonalDataRestoreStatusListener();
            }
            setOnRestoreStatusListener(mRestoreStoreStatusListener);
            MyLogger.logD(CLASS_TAG, "mIsDataInitialed is ok");
            checkRestoreState();
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            setButtonsEnable(false);
            // setProgressBarIndeterminateVisibility(true);
            showLoadingContent(true);
            initActionBar();
            // showUpdatingTitle();

        }

        @Override
        protected Long doInBackground(Void... arg0) {
            mPreview = new BackupFilePreview(mFile);
            if (mPreview != null) {
                mModule = mPreview.getBackupModules(PersonalDataRestoreActivity.this);
            }
            return null;
        }
    }

    private class PersonalDataRestoreStatusListener extends NormalRestoreStatusListener {

        public void onComposerChanged(final int type, final int max) {
            Log.i(CLASS_TAG, "RestoreDetailActivity: onComposerChanged type = " + type);

            if (mHandler != null) {
                mHandler.post(new Runnable() {

                    public void run() {
                        String msg = getProgressDlgMessage(type);
                        setProgressDialogMessage(msg);
                        setProgressDialogMax(max);
                        setProgressDialogProgress(0);
                    }
                });
            }
        }

        public void onRestoreEnd(boolean bSuccess, ArrayList<ResultEntity> resultRecord) {
            final ArrayList<ResultEntity> iResultRecord = resultRecord;
            MyLogger.logD(CLASS_TAG, "onRestoreEnd");
            boolean hasSuccess = false;
            for (ResultEntity result : resultRecord) {
                if (ResultEntity.SUCCESS == result.getResult()) {
                    hasSuccess = true;
                    break;
                }
            }

            if (hasSuccess) {
                String recrodXmlFile = mRestoreFolderPath + File.separator + Constants.RECORD_XML;
                String content = Utils.readFromFile(recrodXmlFile);
                ArrayList<RecordXmlInfo> recordList = new ArrayList<RecordXmlInfo>();
                if (content != null) {
                    recordList = RecordXmlParser.parse(content.toString());
                }
                RecordXmlComposer xmlCompopser = new RecordXmlComposer();
                xmlCompopser.startCompose();

                RecordXmlInfo restoreInfo = new RecordXmlInfo();
                restoreInfo.setRestore(true);
                restoreInfo.setDevice(Utils.getPhoneSearialNumber());
                restoreInfo.setTime(String.valueOf(System.currentTimeMillis()));

                boolean bAdded = false;
                for (RecordXmlInfo record : recordList) {
                    if (record.getDevice().equals(restoreInfo.getDevice())) {
                        xmlCompopser.addOneRecord(restoreInfo);
                        bAdded = true;
                    } else {
                        xmlCompopser.addOneRecord(record);
                    }
                }

                if (!bAdded) {
                    xmlCompopser.addOneRecord(restoreInfo);
                }
                xmlCompopser.endCompose();
                Utils.writeToFile(xmlCompopser.getXmlInfo(), recrodXmlFile);
            }

            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {

                        MyLogger.logD(CLASS_TAG, " Restore show Result Dialog");
                        if (mIsStoped) {
                            mNeedUpdateResult = true;
                        } else {
                            showRestoreResult(iResultRecord);
                        }
                    }
                });
            }
        }
    }
}
