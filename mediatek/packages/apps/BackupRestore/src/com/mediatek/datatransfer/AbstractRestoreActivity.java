package com.mediatek.datatransfer;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.Service;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;

import com.mediatek.datatransfer.CheckedListActivity.OnCheckedCountChangedListener;
import com.mediatek.datatransfer.RestoreService.OnRestoreStatusListener;
import com.mediatek.datatransfer.RestoreService.RestoreBinder;
import com.mediatek.datatransfer.ResultDialog.ResultEntity;
import com.mediatek.datatransfer.SDCardReceiver.OnSDCardStatusChangedListener;
import com.mediatek.datatransfer.modules.Composer;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.DialogID;
import com.mediatek.datatransfer.utils.Constants.State;
import com.mediatek.datatransfer.utils.Utils;

import java.io.IOException;
import java.util.ArrayList;

public abstract class AbstractRestoreActivity extends CheckedListActivity implements
        OnCheckedCountChangedListener { 

    private final String CLASS_TAG = MyLogger.LOG_TAG + "/AbstractRestoreActivity";
    protected ArrayList<String> mUnCheckedList = new ArrayList<String>();
    protected Handler mHandler;
    BaseAdapter mAdapter;
    private Button mBtRestore = null;
    private CheckBox mChboxSelect = null;
    private View mDivider ;
    private ProgressDialog mProgressDialog;
    protected RestoreBinder mRestoreService;
    OnRestoreStatusListener mRestoreListener;
    OnSDCardStatusChangedListener mSDCardListener;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        super.onCreate(savedInstanceState);
        setProgressBarIndeterminateVisibility(false);
        setContentView(R.layout.restore);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        init();

        /*
         * bind Restore Service when activity onCreate, and unBind Service when
         * activity onDestroy
         */
        this.bindService();
        MyLogger.logI(CLASS_TAG, " onCreate");
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            // app icon in action bar clicked; go home
            Intent intent = new Intent(this, MainActivity.class);
            intent.putExtra(MainActivity.ACTION_WHERE, MainActivity.ACTION_RESTORE);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(intent);
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        MyLogger.logI(CLASS_TAG, " onDestroy");
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
        unRegisteSDCardListener();

        // when startService when to Restore and stopService when onDestroy if
        // the service in IDLE
        if (mRestoreService != null && mRestoreService.getState() == State.INIT) {
            this.stopService();
        }

        // set listener to null avoid some special case when restart after
        // configure changed
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(null);
        }
        this.unBindService();
        mHandler = null;
    }

    @Override
    public void onPause() {
        super.onPause();
        MyLogger.logI(CLASS_TAG, " onPasue");
    }

    @Override
    public void onStop() {
        super.onStop();
        MyLogger.logI(CLASS_TAG, " onStop");
        if (confirmDialog != null && confirmDialog.isShowing()) {
            confirmDialog.dismiss();
        }
    }

    @Override
    protected void onStart() {
        MyLogger.logI(CLASS_TAG, " onStart");
        super.onStart();
    }

    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        MyLogger.logI(CLASS_TAG, " onConfigurationChanged");
    }

    @Override
    public void onCheckedCountChanged() {
        mAdapter.notifyDataSetChanged();
        updateButtonState();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU && event.isLongPress()) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void init() {
        registerOnCheckedCountChangedListener(this);
        registerSDCardListener();
        initButton();
        initHandler();
        initLoadingView();
        initDetailList();
        createProgressDlg();
    }

    LinearLayout loadingContent = null;

    private void initLoadingView() {
        loadingContent = (LinearLayout) findViewById(R.id.loading_container);
    }

    protected void showLoadingContent(boolean show) {
        findViewById(R.id.loading_container).setVisibility(show ? View.VISIBLE : View.GONE);
        findViewById(R.id.restore_content).setVisibility(!show ? View.VISIBLE : View.GONE);
    }

    protected Dialog onCreateDialog(int id) {
        return onCreateDialog(id, null);
    }

    Dialog confirmDialog = null;
    protected Dialog onCreateDialog(int id, Bundle args) {
        Dialog dialog = null;
        MyLogger.logI(CLASS_TAG, " oncreateDialog, id = " + id);
        switch (id) {

        case DialogID.DLG_RESTORE_CONFIRM:
            confirmDialog = dialog = new AlertDialog.Builder(this).setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.notice).setMessage(R.string.restore_confirm_notice)
                    .setNegativeButton(android.R.string.cancel, null)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            MyLogger.logI(CLASS_TAG, " to Restore");
                            startRestore();
                        }
                    }).setCancelable(false).create();
            break;

        case DialogID.DLG_SDCARD_REMOVED:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.error)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setMessage(R.string.sdcard_removed)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            if (mRestoreService != null) {
                                mRestoreService.reset();
                            }
                            stopService();
                        }
                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_RUNNING:
            dialog = new AlertDialog.Builder(AbstractRestoreActivity.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.warning)
                    .setMessage(R.string.state_running)
                    .setPositiveButton(android.R.string.ok, null).create();
            break;
        case DialogID.DLG_SDCARD_FULL:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.error)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setMessage(R.string.sdcard_is_full)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            if (mRestoreService != null) {
                                mRestoreService.cancelRestore();
                            }
                        }
                    }).setCancelable(false).create();
            break;

        default:
            break;
        }
        return dialog;
    }

    private void unRegisteSDCardListener() {
        if (mSDCardListener != null) {
            SDCardReceiver receiver = SDCardReceiver.getInstance();
            receiver.unRegisterOnSDCardChangedListener(mSDCardListener);
        }
    }

    private void registerSDCardListener() {
        mSDCardListener = new OnSDCardStatusChangedListener() {
            @Override
            public void onSDCardStatusChanged(boolean mount) {
                if (!mount) {

                    AbstractRestoreActivity.this.finish();
                }
            }
        };

        SDCardReceiver receiver = SDCardReceiver.getInstance();
        receiver.registerOnSDCardChangedListener(mSDCardListener);
    }

    private void initHandler() {
        mHandler = new Handler();
    }

    private void initButton() {
        mDivider = findViewById(R.id.restore_divider);
        mDivider.setBackground(getListView().getDivider());
        mBtRestore = (Button) findViewById(R.id.restore_bt_restore);
        mBtRestore.setOnClickListener(new Button.OnClickListener() {

            public void onClick(View v) {
                if (getCheckedCount() > 0) {
                    if (Utils.getWorkingInfo() > 0) {
                        showDialog(DialogID.DLG_RUNNING);
                        return;
                    }
                    showDialog(DialogID.DLG_RESTORE_CONFIRM);
                }
            }
        });
        mChboxSelect = (CheckBox) findViewById(R.id.restore_checkbox_select);
        mChboxSelect.setVisibility(View.INVISIBLE);
        mChboxSelect.setOnClickListener(new CheckBox.OnClickListener() {

            public void onClick(View v) {
                if (isAllChecked(true)) {
                    setAllChecked(false);
                } else {
                    setAllChecked(true);
                }
            }
        });
    }

    protected void updateButtonState() {
        if (getCount() == 0) {
            setButtonsEnable(false);
            mChboxSelect.setVisibility(View.GONE);
            return;
        }
        mDivider.setVisibility(View.VISIBLE);
        if (isAllChecked(false)) {
            mBtRestore.setEnabled(false);
            mChboxSelect.setChecked(false);
        } else {
            mBtRestore.setEnabled(true);
            mChboxSelect.setChecked(isAllChecked(true));
            mChboxSelect.setVisibility(View.VISIBLE);
            mChboxSelect.setText(getApplication().getResources().getString(R.string.selectall));
        }
    }

    protected void setButtonsEnable(boolean enabled) {
        if (mChboxSelect != null) {
            mChboxSelect.setEnabled(enabled);
        }
        if (mBtRestore != null) {
            mBtRestore.setEnabled(enabled);
        }
    }

    private void initDetailList() {
        mAdapter = initAdapter();
        setListAdapter(mAdapter);
        this.updateButtonState();
    }

    abstract protected BaseAdapter initAdapter();

    protected void notifyListItemCheckedChanged() {
        if (mAdapter != null) {
            mAdapter.notifyDataSetChanged();
        }
        updateButtonState();
    }

    protected ProgressDialog createProgressDlg() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setMessage(getString(R.string.restoring));
            mProgressDialog.setCancelable(false);
        }
        return mProgressDialog;
    }

    protected void showProgressDialog() {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.show();
    }

    protected void setProgressDialogMax(int max) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setMax(max);
    }

    protected void setProgressDialogProgress(int value) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setProgress(value);
    }

    protected void setProgressDialogMessage(CharSequence message) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setMessage(message);
    }

    protected boolean isProgressDialogShowing() {
        return mProgressDialog.isShowing();
    }

    protected void dismissProgressDialog() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }

    protected boolean isCanStartRestore() {
        if (mRestoreService == null) {
            MyLogger.logE(CLASS_TAG, " isCanStartRestore : mRestoreService is null");
            return false;
        }

        if (mRestoreService.getState() != State.INIT) {
            MyLogger.logE(CLASS_TAG,
                    " isCanStartRestore :Can not to start Restore. Restore Service is ruuning");
            return false;
        }
        return true;
    }

    protected boolean errChecked() {
        boolean ret = false;
        String path = SDCardUtils.getStoragePath(this);
        if (path == null) {
            // no sdcard
            ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        showDialog(DialogID.DLG_SDCARD_REMOVED, null);
                    }
                });
            }
        } else if (SDCardUtils.getAvailableSize(path) <= 512) {
            // no space
            ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        showDialog(DialogID.DLG_SDCARD_FULL, null);
                    }
                });
            }
        }
        return ret;
    }

    public void setOnRestoreStatusListener(OnRestoreStatusListener listener) {
        mRestoreListener = listener;
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(mRestoreListener);
        }
    }

    protected abstract void afterServiceConnected();

    protected abstract void startRestore();

    private void bindService() {
        getApplicationContext().bindService(new Intent(this, RestoreService.class), mServiceCon,
                Service.BIND_AUTO_CREATE);
    }

    private void unBindService() {
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(null);
        }
        getApplicationContext().unbindService(mServiceCon);
    }

    protected void startService() {
        startService(new Intent(this, RestoreService.class));
    }

    protected void stopService() {
        if (mRestoreService != null) {
            mRestoreService.reset();
        }
        stopService(new Intent(this, RestoreService.class));
    }

    ServiceConnection mServiceCon = new ServiceConnection() {

        public void onServiceConnected(ComponentName name, IBinder service) {
            mRestoreService = (RestoreBinder) service;
            if (mRestoreService != null) {
                mRestoreService.setOnRestoreChangedListner(mRestoreListener);
                afterServiceConnected();
            }
            MyLogger.logI(CLASS_TAG, " onServiceConnected");
        }

        public void onServiceDisconnected(ComponentName name) {
            mRestoreService = null;
            MyLogger.logI(CLASS_TAG, " onServiceDisconnected");
        }
    };

    public class NormalRestoreStatusListener implements OnRestoreStatusListener {

        public void onComposerChanged(final int type, final int max) {
            Log.i(CLASS_TAG, "onComposerChanged");
        }

        public void onProgressChanged(Composer composer, final int progress) {
            Log.i(CLASS_TAG, "onProgressChange, p = " + progress);
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                        }
                    }
                });
            }
        }

        public void onRestoreEnd(boolean bSuccess, final ArrayList<ResultEntity> resultRecord) {
            Log.i(CLASS_TAG, "onRestoreEnd");
        }

        public void onRestoreErr(IOException e) {
            Log.i(CLASS_TAG, "onRestoreErr");
            if (errChecked()) {
                if (mRestoreService != null && mRestoreService.getState() != State.INIT
                        && mRestoreService.getState() != State.FINISH) {
                    mRestoreService.pauseRestore();
                }
            }
        }
    }
}
