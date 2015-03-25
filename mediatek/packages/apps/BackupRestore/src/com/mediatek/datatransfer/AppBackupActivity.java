package com.mediatek.datatransfer;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.datatransfer.BackupEngine.BackupResultType;
import com.mediatek.datatransfer.BackupService.BackupProgress;
import com.mediatek.datatransfer.ResultDialog.ResultEntity;
import com.mediatek.datatransfer.modules.AppBackupComposer;
import com.mediatek.datatransfer.modules.Composer;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.DialogID;
import com.mediatek.datatransfer.utils.Constants.State;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class AppBackupActivity extends AbstractBackupActivity {

    private String CLASS_TAG = MyLogger.LOG_TAG + "/AppBackupActivity";
    private List<AppSnippet> mData = new ArrayList<AppSnippet>();
    private AppBackupAdapter mAdapter;
    private InitDataTask mInitDataTask;
    private boolean mIsDataInitialed = false;
    private AppBackupStatusListener mBackupStatusListener = new AppBackupStatusListener();
    private boolean mIsCheckedBackupStatus = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        super.onCreate(savedInstanceState);
        setProgressBarIndeterminateVisibility(false);
        Log.v(CLASS_TAG, "onCreate");
    }

    @Override
    protected void onStart() {
        super.onStart();
        // update
        mInitDataTask = new InitDataTask();
        mInitDataTask.execute();
    }

    @Override
    public BaseAdapter initBackupAdapter() {
        mAdapter = new AppBackupAdapter(this, mData, R.layout.app_item);
        return mAdapter;
    }

    @Override
    public void startBackup() {
        Log.v(CLASS_TAG, "startBackup");

        startService();
        if (mBackupService != null) {
            ArrayList<Integer> backupList = new ArrayList<Integer>();
            backupList.add(ModuleType.TYPE_APP);
            mBackupService.setBackupModelList(backupList);

            ArrayList<String> list = getSelectedPackageNameList();
            mBackupService.setBackupItemParam(ModuleType.TYPE_APP, list);
            String appPath = SDCardUtils.getAppsBackupPath(this);
            MyLogger.logD(CLASS_TAG, "backup path is: " + appPath);
            boolean ret = mBackupService.startBackup(appPath);
            if (ret) {
                showProgress();
                mProgressDialog.setProgress(0);
                mProgressDialog.setMax(list.size());
                String packageName = list.get(0);
                AppSnippet appSnippet = getAppSnippetByPackageName(packageName);
                String msg = formatProgressDialogMsg(appSnippet);
                mProgressDialog.setMessage(msg);
            } else {
                showDialog(DialogID.DLG_SDCARD_FULL);
                stopService();
            }
        }
    }

    protected void afterServiceConnected() {
        MyLogger.logD(CLASS_TAG, "afterServiceConnected, to checkBackupState");
        checkBackupState();
    }

    private ArrayList<String> getSelectedPackageNameList() {
        ArrayList<String> list = new ArrayList<String>();
        int count = mAdapter.getCount();
        for (int position = 0; position < count; position++) {
            AppSnippet item = (AppSnippet) getItemByPosition(position);
            if (isItemCheckedByPosition(position)) {
                list.add(item.getPackageName());
            }
        }
        return list;
    }

    @Override
    public void onCheckedCountChanged() {
        super.onCheckedCountChanged();
        updateTitle();
    }

    private void updateData(ArrayList<AppSnippet> list) {
        if (list == null) {
            MyLogger.logE(CLASS_TAG, "updateData, list is null");
            return;
        }
        mData = list;
        mAdapter.changeData(list);
        syncUnCheckedItems();
        mAdapter.notifyDataSetChanged();
        updateTitle();
        updateButtonState();
        mIsDataInitialed = true;
        MyLogger.logD(CLASS_TAG, "data is initialed, to checkBackupState");
        checkBackupState();
    }

    private AppSnippet getAppSnippetByPackageName(String packageName) {

        AppSnippet result = null;
        for (AppSnippet item : mData) {
            if (item.getPackageName().equalsIgnoreCase(packageName)) {
                result = item;
                break;
            }
        }
        return result;
    }

    private String formatProgressDialogMsg(AppSnippet item) {
        StringBuilder builder = new StringBuilder(getString(R.string.backuping));
        if (item != null) {
            builder.append("(").append(item.getName()).append(")");
        }
        return builder.toString();
    }

    public void updateTitle() {
        StringBuilder sb = new StringBuilder();
        sb.append(getString(R.string.backup_app));
        int totalNum = mAdapter.getCount();
        int selectNum = this.getSelectedPackageNameList().size();
        sb.append("(" + selectNum + "/" + totalNum + ")");
        this.setTitle(sb.toString());
    }

    /** M: Bug Fix for CR ALPS01269048 @{ */
    protected String getProgressDlgMessage(final int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.backuping));
        builder.append("(");
        builder.append(ModuleType.getModuleStringFromType(this, type));
        builder.append(")");
        return builder.toString();
    }
    /** @} */

    @Override
    protected void checkBackupState() {
        if (mIsCheckedBackupStatus) {
            MyLogger.logD(CLASS_TAG, "can not checkBackupState, as it has been checked");
            return;
        }
        if (!mIsDataInitialed) {
            MyLogger.logD(CLASS_TAG, "can not checkBackupState, wait data to initialed");
            return;
        }
        MyLogger.logD(CLASS_TAG, "to checkBackupState");
        mIsCheckedBackupStatus = true;
        if (mBackupService != null) {
            int state = mBackupService.getState();
            MyLogger.logD(CLASS_TAG, "checkBackupState: state = " + state);
            switch (state) {
            case State.RUNNING:
            case State.PAUSE:
                ArrayList<String> params = mBackupService.getBackupItemParam(ModuleType.TYPE_APP);
                BackupProgress p = mBackupService.getCurBackupProgress();
                Log.e(CLASS_TAG, CLASS_TAG + "checkBackupState: Max = " + p.mMax
                        + " curprogress = " + p.mCurNum);

                if (state == State.RUNNING) {
                    mProgressDialog.show();
                }
                    if (p.mCurNum < p.mMax) {
                        /** M: Bug Fix for CR ALPS01269048 @{ */
                        String msg;
                        if (ModuleType.TYPE_APP == p.mType && params != null) {
                            String packageName = params.get(p.mCurNum);
                            msg = formatProgressDialogMsg(getAppSnippetByPackageName(packageName));
                        } else {
                            msg = getProgressDlgMessage(p.mType);
                        }
                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                        }
                        /** @} */
                    }
                if (mProgressDialog != null) {
                    mProgressDialog.setMax(p.mMax);
                    mProgressDialog.setProgress(p.mCurNum);
                }
                break;
            case State.FINISH:
                showBackupResult(mBackupService.getBackupResultType(),
                        mBackupService.getAppBackupResult());
                break;
            default:
                super.checkBackupState();
                break;
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;
        switch (id) {
        case DialogID.DLG_RESULT:
            final DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(final DialogInterface dialog, final int which) {
                    stopService();
                }
            };
            dialog = ResultDialog.createResultDlg(this, R.string.backup_result, args, listener);
            break;

        case DialogID.DLG_LOADING:
            ProgressDialog progressDlg = new ProgressDialog(this);
            progressDlg.setCancelable(false);
            progressDlg.setMessage(getString(R.string.loading_please_wait));
            progressDlg.setIndeterminate(true);
            dialog = progressDlg;
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
                ListAdapter adapter = ResultDialog.createAppResultAdapter(mData, this, args,
                        ResultDialog.RESULT_TYPE_BACKUP);
                view.setAdapter(adapter);
            }
            break;
        default:
            super.onPrepareDialog(id, dialog, args);
            break;
        }
    }

    protected void showBackupResult(final BackupResultType result,
            final ArrayList<ResultEntity> appResultRecord) {

        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }

        if (mCancelDlg != null && mCancelDlg.isShowing()) {
            mCancelDlg.dismiss();
        }

        if (result != BackupResultType.Cancel) {
            Bundle args = new Bundle();
            args.putParcelableArrayList("result", appResultRecord);
            ListAdapter adapter = ResultDialog.createAppResultAdapter(mData, this, args,
                    ResultDialog.RESULT_TYPE_BACKUP);
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(false)
                    .setTitle(R.string.backup_result)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(final DialogInterface dialog, final int which) {
                            if (mBackupService != null) {
                                mBackupService.reset();
                            }
                            stopService();
                            NotifyManager.getInstance(AppBackupActivity.this).clearNotification();
                        }
                    }).setAdapter(adapter, null).create();
            dialog.show();
        } else {
            stopService();
        }
    }

    private class InitDataTask extends AsyncTask<Void, Void, Long> {

        List<ApplicationInfo> mAppInfoList;
        ArrayList<AppSnippet> appDatas;

        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            updateData(appDatas);
            setButtonsEnable(!appDatas.isEmpty());
            updateButtonState();
            setOnBackupStatusListener(mBackupStatusListener);
            // setProgressBarIndeterminateVisibility(false);
            showLoadingContent(false);

        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            // show progress and set title as "updating"
            //setProgressBarIndeterminateVisibility(true);
            showLoadingContent(true);
            setTitle(R.string.backup_app);
            setButtonsEnable(false);
        }

        @Override
        protected Long doInBackground(Void... arg0) {
            mAppInfoList = AppBackupComposer.getUserAppInfoList(AppBackupActivity.this);
            PackageManager pm = getPackageManager();
            appDatas = new ArrayList<AppSnippet>();
            for (ApplicationInfo info : mAppInfoList) {
                Drawable icon = info.loadIcon(pm);
                CharSequence name = info.loadLabel(pm);
                AppSnippet snippet = new AppSnippet(icon, name, info.packageName);
                appDatas.add(snippet);
            }

            Collections.sort(appDatas, new Comparator<AppSnippet>() {
                public int compare(AppSnippet object1, AppSnippet object2) {
                    String left = new StringBuilder(object1.getName()).toString();
                    String right = new StringBuilder(object2.getName()).toString();
                    if (left != null && right != null) {
                        return left.compareTo(right);
                    }
                    return 0;
                }
            });
            return null;
        }
    }

    private class AppBackupAdapter extends BaseAdapter {

        private List<AppSnippet> mList;
        private int mLayoutId;
        private LayoutInflater mInflater;

        public AppBackupAdapter(Context context, List<AppSnippet> list, int resource) {
            mList = list;
            mLayoutId = resource;
            mInflater = LayoutInflater.from(context);
        }

        public void changeData(List<AppSnippet> list) {
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
            return mList.get(position).getPackageName().hashCode();
        }

        public View getView(final int position, View convertView, ViewGroup parent) {
            if (mList == null) {
                return null;
            }
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(mLayoutId, parent, false);
            }
            final AppSnippet item = mList.get(position);
            ImageView imgView = (ImageView) view.findViewById(R.id.item_image);
            TextView textView = (TextView) view.findViewById(R.id.item_text);
            CheckBox checkbox = (CheckBox) view.findViewById(R.id.item_checkbox);
            imgView.setBackgroundDrawable(item.getIcon());
            textView.setText(item.getName());
            checkbox.setChecked(isItemCheckedByPosition(position));
            return view;
        }
    }

    private class AppBackupStatusListener extends NomalBackupStatusListener {

        @Override
        public void onComposerChanged(final Composer composer) {
            if (composer == null) {
                MyLogger.logE( CLASS_TAG,
                       "onComposerChasetProgressBarIndeterminateVisibility(false);nged: error[composer is null]");
                return;
            } else {
                MyLogger.logI(CLASS_TAG, "onComposerChanged: type = " + composer.getModuleType()
                        + "Max = " + composer.getCount());
            }
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        ArrayList<String> params = mBackupService
                                .getBackupItemParam(ModuleType.TYPE_APP);

                        /** M: Bug Fix for CR ALPS01269048 @{ */
                        BackupProgress p = mBackupService.getCurBackupProgress();
                        String msg;
                        if (ModuleType.TYPE_APP == p.mType && params != null) {
                            String packageName = params.get(0);
                            MyLogger.logV(CLASS_TAG, "onComposerChanged, first packageName is "
                                    + packageName);
                            msg = formatProgressDialogMsg(getAppSnippetByPackageName(packageName));
                        } else {
                            msg = getProgressDlgMessage(composer.getModuleType());
                        }
                        /** @} */

                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                            mProgressDialog.setMax(composer.getCount());
                            mProgressDialog.setProgress(0);
                        }
                    }
                });
            }
        }

        @Override
        public void onProgressChanged(final Composer composer, final int progress) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                            if (progress < composer.getCount()) {
                                ArrayList<String> params = mBackupService
                                        .getBackupItemParam(ModuleType.TYPE_APP);

                                /** M: Bug Fix for CR ALPS01269048 @{ */
                                BackupProgress p = mBackupService.getCurBackupProgress();
                                String msg;
                                if (ModuleType.TYPE_APP == p.mType && params != null) {
                                    String packageName = params.get(progress);
                                    MyLogger.logV(CLASS_TAG, "onComposerChanged: the " + progress
                                            + "  packageName is " + packageName);
                                    msg = formatProgressDialogMsg(getAppSnippetByPackageName(packageName));
                                } else {
                                    msg = getProgressDlgMessage(composer.getModuleType());
                                }
                                /** @} */
                                if (mProgressDialog != null) {
                                    mProgressDialog.setMessage(msg);
                                }
                            }
                        }
                    }
                });
            }
        }

        @Override
        public void onBackupEnd(final BackupResultType resultCode,
                final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        showBackupResult(resultCode, appResultRecord);
                    }
                });
            }
        }
    }

}
