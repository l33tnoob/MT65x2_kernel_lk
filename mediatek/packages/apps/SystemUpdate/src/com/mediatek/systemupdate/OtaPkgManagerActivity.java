package com.mediatek.systemupdate;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.Settings;

import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class OtaPkgManagerActivity extends PkgManagerBaseActivity {

    private static enum MenuStatus {
        Menu_None, Menu_Download, Menu_Cancel, Menu_Upgrade,
    };

    private static final String TAG = "SystemUpdate/OtaManager";

    private static final String NOTES_STR_SPLIT = "\r\n";

    private static final String OTA_PKG_PATH = Util.PathName.OTA_PKG_FOLDER + Util.PathName.PACKAGE_NAME;

    private MenuStatus mMenuStatus = MenuStatus.Menu_None;

    private static final int MENU_ID_DOWNLOAD = Menu.FIRST;
    private static final int MENU_ID_CANCEL = Menu.FIRST + 1;
    private static final int MENU_ID_UPGRADE = Menu.FIRST + 2;
    private static final int MENU_ID_REFRESH = Menu.FIRST + 3;

    private static final int DIALOG_CANCELDOWNLOAD = 1;
    private static final int DIALOG_NOENOUGHSPACE = 2;
    private static final int DIALOG_PACKAGE_DELETED = 3;
    private static final int DIALOG_SERVERERROR = 4;
    private static final int DIALOG_NOSDCARD = 5;
    private static final int DIALOG_UNKNOWNERROR = 6;
    private static final int DIALOG_OTARESULT = 7;
    private static final int DIALOG_UNZIPPING = 8;
    private static final int DIALOG_WARN_DATA_COST = 9;
    private static final int DIALOG_CHANGE_NETWORK = 10;
    private static final int DIALOG_NOTIFY_POWEROFF = 12;

    // M: add by mtk80800
    private static final int DIALOG_SDCARDMOUNTED = 11;
    // M:end add

    static final String CLASS_NAME = "com.mediatek.systemupdate.OtaPkgManagerActivity";
    private static final String OTARESULT_DLG_TITLE = "otaresult_dlg_title";
    private static final String OTARESULT_DLG_MSG = "otaresult_dlg_msg";
    private static final String ACTION_WIFI_SETTINGS = "android.settings.WIFI_SETTINGS";
    private static final String ACTION_APN_SETTINGS = "android.settings.APN_SETTINGS";

    private DownloadInfo mDownloadInfo = null;
    private SystemUpdateService mService = null;

    private ProgressDialog mUnzipProgressDialog;
    private AlertDialog mDownloadCancelDialog;
    private AlertDialog mDownloadStorageDialog;
    private AlertDialog mAlertDialog;

    private int mOTADialogTitleResId = 0;
    private int mOTADialogMessageResId = 0;
    private boolean mNeedReset = false;

    private static OtaPkgManagerActivity sInstance;

    /**
     * Whether New version notification need to show when turn background
     */
    private boolean mGoToMainEntry;
    private NotifyManager mNotifyManager;
    private int mOTAresult = Util.OTAresult.CHECK_OK;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        Xlog.i(TAG, "onCreate");

        sInstance = this;

        this.getActionBar().setDisplayHomeAsUpEnabled(true);
        mDownloadInfo = DownloadInfo.getInstance(getApplicationContext());

        mNotifyManager = new NotifyManager(this);
        IntentFilter filter = new IntentFilter();
        filter.addAction(Util.Action.ACTION_MEDIA_MOUNT_UPDATEUI);
        filter.addAction(Util.Action.ACTION_MEDIA_UNMOUNT_UPDATEUI);
        registerReceiver(mReceiver, filter);

    }

    @Override
    protected void onStart() {
        super.onStart();
        mGoToMainEntry = false;
        SdPkgInstallActivity.stopSelf();
        mNotifyManager.clearNotification(NotifyManager.NOTIFY_NEW_VERSION);
        Xlog.i(TAG, "onStart");

        if ((mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_DOWNLOADING)
                || (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_PACKAGEUNZIPPING)
                || (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_PAUSEDOWNLOAD)) {
            Xlog.i(TAG, "onStart, bind service");
            Intent serviceIntent = new Intent(this, SystemUpdateService.class);
            bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        }

    }

    @Override
    protected void onStop() {
        Xlog.i(TAG, "onStop");

        if (!mGoToMainEntry && !mDownloadInfo.getIfNeedRefresh() && mDownloadInfo.getActivityID() < 0
                && (MainEntry.getInstance() == null || !MainEntry.getInstance().isStarted())) {
            Xlog.v(TAG, "background, show new version notification");
            mNotifyManager.showNewVersionNotification();
        }

        if (mService != null) {
            mService.resetHandler(mHandler);
            unbindService(mConnection);
            mService = null;
        }
        doClearAction();
        super.onStop();
    }

    @Override
    protected void onResume() {
        Xlog.i(TAG, "onResume()");

        super.onResume();

        showUILayout(mDownloadInfo.getDLSessionStatus());
    }

    protected void onDestroy() {
        Xlog.i(TAG, "onDestroy");

        unregisterReceiver(mReceiver);

        sInstance = null;
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Xlog.w(TAG, "event.getFlages()" + event.getFlags());
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            Xlog.w(TAG, "onKeyDown, keycode is KEYCODE_BACK");
            if (!mDownloadInfo.getIfNeedRefreshMenu()) {

                if (mDownloadInfo.getActivityID() < 0) {
                    mGoToMainEntry = true;
                    OtaPkgManagerActivity.this.startActivity(new Intent(OtaPkgManagerActivity.this, MainEntry.class));
                }
            }
            break;
        default:
            break;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void doClearAction() {

        removeDialog(DIALOG_CANCELDOWNLOAD);
        removeDialog(DIALOG_NOENOUGHSPACE);
        removeDialog(DIALOG_NOSDCARD);
        removeDialog(DIALOG_UNKNOWNERROR);
        removeDialog(DIALOG_CHANGE_NETWORK);
        removeDialog(DIALOG_UNZIPPING);

        mUnzipProgressDialog = null;
        mDownloadCancelDialog = null;
        mDownloadStorageDialog = null;
        mAlertDialog = null;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub

        if (mMenuStatus == MenuStatus.Menu_Download) {
            int status = mDownloadInfo.getDLSessionStatus();
            if (status == DownloadInfo.STATE_NEWVERSION_READY) {
                menu.add(Menu.NONE, MENU_ID_DOWNLOAD, 0, R.string.btn_download).setShowAsAction(
                        MenuItem.SHOW_AS_ACTION_ALWAYS);
                if (mDownloadInfo.getIfNeedRefreshMenu()) {
                    menu.add(Menu.NONE, MENU_ID_REFRESH, 0, R.string.menu_stats_refresh).setShowAsAction(
                            MenuItem.SHOW_AS_ACTION_NEVER);
                }

            } else {
                menu.add(Menu.NONE, MENU_ID_DOWNLOAD, 0, R.string.btn_resume)
                        .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            }

        } else if (mMenuStatus == MenuStatus.Menu_Upgrade) {
            menu.add(Menu.NONE, MENU_ID_UPGRADE, 0, R.string.btn_install).setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        } else if (mMenuStatus == MenuStatus.Menu_Cancel) {

            menu.add(Menu.NONE, MENU_ID_CANCEL, 0, R.string.btn_cancel).setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        }

        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO Auto-generated method stub

        switch (item.getItemId()) {

        case MENU_ID_DOWNLOAD:
            if (!Util.isSdcardAvailable(this)) {

                Toast.makeText(this, getString(R.string.sdcard_crash_or_unmount), Toast.LENGTH_LONG).show();
                return true;
            }

            /*
             * if (item.getItemId() == MENU_ID_DOWNLOAD) {
             * Util.deleteFile(Util.getPackageFileName(this)); }
             */

            if (!mDownloadInfo.getIfWifiDLOnly()) {
                showDialog(DIALOG_WARN_DATA_COST);
            } else {
                downloadPkg();
            }

            return true;
        case MENU_ID_CANCEL:
            cancelDownloadPkg();
            return true;
        case MENU_ID_UPGRADE:
            item.setEnabled(false);
            installPackage(OTA_PKG_PATH, mDownloadInfo.getVerNum());
            return true;
        case MENU_ID_REFRESH:
            mGoToMainEntry = true;
            requeryPackages();
            return true;
        case android.R.id.home:
            if (!mDownloadInfo.getIfNeedRefreshMenu()) {

                if (mDownloadInfo.getActivityID() < 0) {
                    mGoToMainEntry = true;
                    OtaPkgManagerActivity.this.startActivity(new Intent(OtaPkgManagerActivity.this, MainEntry.class));
                }
            }
            finish();
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void cancelDownloadPkg() {
        Xlog.i(TAG, "cancelDLPkg");

        mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);

        showDialog(DIALOG_CANCELDOWNLOAD);
    }

    private boolean downloadPkg() {

        if (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_NEWVERSION_READY) {
            Util.deleteFile(Util.getPackageFileName(OtaPkgManagerActivity.this));
        }

        if (!Util.isSdcardAvailable(this)) {

            Toast.makeText(this, getString(R.string.sdcard_crash_or_unmount), Toast.LENGTH_LONG).show();
            return true;
        }

        long lPkgSize = Util.getFileSize(Util.getPackageFileName(this));

        long lSizeNeeded = Util.getExtraSpaceNeeded(this, (long) (mDownloadInfo.getUpdateImageSize()
                * (Util.DECOMPRESS_RATIO + 1) - lPkgSize));
        if (lSizeNeeded == -1) {
            Toast.makeText(this, getString(R.string.unmount_sdcard), Toast.LENGTH_LONG).show();
            return true;

        } else if (lSizeNeeded > 0) {
            Toast.makeText(this, getString(R.string.insufficient_space_content, lSizeNeeded), Toast.LENGTH_LONG).show();
            return true;

        }

        if (mService == null) {
            Intent serviceIntent = new Intent(this, SystemUpdateService.class);
            bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        } else {
            showUILayout(DownloadInfo.STATE_DOWNLOADING);
            mService.startDlPkg();
        }

        /*
         * mService.startDlPkg();
         * 
         * showDownloadingPkgUI();
         */
        return true;

    }

    private void removeWarning() {
        View vTextAskForDLOta = findViewById(R.id.textAskForDLOta);

        if (vTextAskForDLOta != null) {
            vTextAskForDLOta.setVisibility(View.GONE);
        }

        View vTextWarningForDLOta = findViewById(R.id.textWarningForDLOta);

        if (vTextWarningForDLOta != null) {
            vTextWarningForDLOta.setVisibility(View.GONE);
        }

        View vCheckWifi = findViewById(R.id.checkWifiOnly);

        if (vCheckWifi != null) {
            vCheckWifi.setVisibility(View.GONE);
        }

        View vSize = findViewById(R.id.textPkgSize);

        if (vSize != null) {
            vSize.setVisibility(View.GONE);
        }

    }

    private void removeProBar() {

        View vProBar = findViewById(R.id.downloaingProBar);

        if (vProBar != null) {
            vProBar.setVisibility(View.GONE);
        }

        View vDLRatio = findViewById(R.id.downloaingRatio);

        if (vDLRatio != null) {
            vDLRatio.setVisibility(View.GONE);
        }
    }

    private void resetDescriptionInfo() {
        Xlog.i(TAG, "resetDescriptionInfo");

        String strPkg = Util.getPackageFileName(this);

        if (strPkg != null) {
            File imgf = new File(strPkg);
            if (imgf.exists()) {
                Xlog.i(TAG, "resetDescriptionInfo, image exist, delete it");
                Util.deleteFile(strPkg);
            }
        }

        mDownloadInfo.resetDownloadInfo();
    }

    private void updateRatio() {

        float fsize = -1;
        float tsize = -1;

        long totalSize = mDownloadInfo.getUpdateImageSize();

        if (totalSize <= 0) {
            return;
        }

        long currSize = Util.getFileSize(Util.getPackageFileName(this));

        Xlog.i(TAG, "updateProBar dlSize:" + currSize + " totalSize:" + totalSize);

        if (mDownloadInfo.getDLSessionRenameState() && mDownloadInfo.getDownLoadPercent() == Util.MAX_PERCENT) {
            currSize = totalSize;
            Xlog.i(TAG, "onDlPkgUpgrade, download complete but upzip terminate by exception");
        }
        int ratio = (int) (((double) currSize / (double) totalSize) * Util.MAX_PERCENT);
        if (ratio > Util.MAX_PERCENT) {
            ratio = Util.MAX_PERCENT;
            currSize = totalSize;
        }
        mDownloadInfo.setDownLoadPercent(ratio);
        ProgressBar dlRatioProgressBar = (ProgressBar) findViewById(R.id.downloaingProBar);

        dlRatioProgressBar.setProgress(ratio);
        /*
         * CharSequence ratioText = Integer.toString(ratio) + "%"; TextView
         * dlRatio = (TextView) findViewById(R.id.downloadingPercent);
         * 
         * dlRatio.setText(ratioText); fsize = (float) ((float) currSize /
         * 1024.0); fsize = (float) (((int) (fsize * 100)) / 100.0); tsize =
         * (float) ((float) totalSize / 1024.0); tsize = (float) (((int) (tsize
         * * 100)) / 100.0); TextView dlProgress = (TextView)
         * findViewById(R.id.downloadingRatio);
         * dlProgress.setText(getString(R.string.download_per_info, fsize,
         * tsize));
         */

        TextView dlRatio = (TextView) findViewById(R.id.downloadingRatio);
        if (totalSize >= Util.M_SIZE) {
            dlRatio.setText(getString(R.string.download_per_info_M, ratio, (double) currSize / Util.M_SIZE,
                    (double) totalSize / Util.M_SIZE));
        } else if (totalSize >= Util.K_SIZE) {
            dlRatio.setText(getString(R.string.download_per_info_K, ratio, (double) currSize / Util.K_SIZE,
                    (double) totalSize / Util.K_SIZE));
        } else {
            dlRatio.setText(getString(R.string.download_per_info_B, ratio, currSize, totalSize));
        }

        /*
         * mMenuStatus = MenuStatus.Menu_Cancel; invalidateOptionsMenu();
         */

    }

    private void initWifiOnlyCheckbox(boolean show, boolean readRecord) {
        CheckBox ckWifiOnly = (CheckBox) findViewById(R.id.checkWifiOnly);

        if (!show) {
            if (ckWifiOnly != null) {
                ckWifiOnly.setVisibility(View.GONE);
            }
        } else {

            if (readRecord) {
                ckWifiOnly.setChecked(mDownloadInfo.getIfWifiDLOnly());

                Xlog.i(TAG, "ckWifiOnly.setChecked() " + mDownloadInfo.getIfWifiDLOnly());
            } else {
                ckWifiOnly.setChecked(true);
                Xlog.i(TAG, "ckWifiOnly.setChecked(true)");
            }

            mDownloadInfo.setIfWifiDLOnly(ckWifiOnly.isChecked());
            ckWifiOnly.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    mDownloadInfo.setIfWifiDLOnly(isChecked);
                }
            });
        }

    }

    private void showDlInterface() {
        setContentView(R.layout.ota_package_download);
        removeWarning();
        updateRatio();
        mMenuStatus = MenuStatus.Menu_Cancel;
        invalidateOptionsMenu();
        initWifiOnlyCheckbox(mDownloadInfo.getOtaAutoDlStatus(), true);
        fillPkgInfo(mDownloadInfo.getAndroidNum(), mDownloadInfo.getVerNum(), mDownloadInfo.getUpdateImageSize(),
                Util.getPackageFileName(this));
    }

    private void showUILayout(int state) {
        Xlog.i(TAG, "showUILayout with " + state);

        switch (state) {

        case DownloadInfo.STATE_QUERYNEWVERSION:
            requeryPackages();
            break;

        case DownloadInfo.STATE_NEWVERSION_READY:
            setContentView(R.layout.ota_package_download);
            removeProBar();
            mMenuStatus = MenuStatus.Menu_Download;
            invalidateOptionsMenu();
            initWifiOnlyCheckbox(true, false);
            fillPkgInfo(mDownloadInfo.getAndroidNum(), mDownloadInfo.getVerNum(), mDownloadInfo.getUpdateImageSize(),
                    Util.getPackageFileName(this));

            break;
        case DownloadInfo.STATE_DOWNLOADING:
            showDlInterface();
            break;
        case DownloadInfo.STATE_PAUSEDOWNLOAD:
            setContentView(R.layout.ota_package_download);
            updateRatio();
            mMenuStatus = MenuStatus.Menu_Download;
            invalidateOptionsMenu();
            initWifiOnlyCheckbox(true, false);
            fillPkgInfo(mDownloadInfo.getAndroidNum(), mDownloadInfo.getVerNum(), mDownloadInfo.getUpdateImageSize(),
                    Util.getPackageFileName(this));
            break;

        case DownloadInfo.STATE_DLPKGCOMPLETE:

            setContentView(R.layout.ota_package_ready);

            mMenuStatus = MenuStatus.Menu_Upgrade;
            invalidateOptionsMenu();
            initWifiOnlyCheckbox(false, false);
            fillPkgInfo(mDownloadInfo.getAndroidNum(), mDownloadInfo.getVerNum(), mDownloadInfo.getUpdateImageSize(), null);

            Intent reminder = new Intent(this, ForegroundDialogService.class);

            boolean stopResult = stopService(reminder);

            Xlog.i(TAG, "stopResult =  " + stopResult);

            break;

        case DownloadInfo.STATE_PACKAGEUNZIPPING:
            showUnzippingInterface();

            break;
        default:
            break;

        }

        fillPkgInfo(mDownloadInfo.getAndroidNum(), mDownloadInfo.getVerNum(), mDownloadInfo.getUpdateImageSize(),
                Util.getPackageFileName(this));

        List<String> listNotes = new ArrayList<String>();

        String strNotes = mDownloadInfo.getVersionNote();

        if (strNotes != null) {
            strNotes.trim();

            String[] arrayNotes = strNotes.split(NOTES_STR_SPLIT);

            for (String str : arrayNotes) {

                if (str != null) {
                    str.trim();
                    if (str.length() > 0) {

                        listNotes.add(str);
                        Xlog.i(TAG, "notes  " + str);
                    }
                }

            }
        }

        if (listNotes.size() == 0) {
            listNotes.add("Bug fix");
        }

        fillReleaseNotes(listNotes);

    }

    private void showUnzippingInterface() {

        if (mService == null) {
            return;

        }
        if (!mService.isDownloading()) {
            Xlog.i(TAG, "onDlPkgUpgrade, download complete but upzip terminate by exception");
            mDownloadInfo.setDLSessionUnzipState(false);
            mDownloadInfo.setDLSessionRenameState(false);
            UpgradePkgManager.resetPkg(this);
            mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);
            showUILayout(DownloadInfo.STATE_PAUSEDOWNLOAD);
        } else {
            showDlInterface();
            removeDialog(DIALOG_UNZIPPING);
            showDialog(DIALOG_UNZIPPING);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        Xlog.i(TAG, "onCreateDialog id, dialog id = " + id);
        switch (id) {

        case DIALOG_UNZIPPING:
            mUnzipProgressDialog = new ProgressDialog(this);
            mUnzipProgressDialog.setIndeterminate(true);
            mUnzipProgressDialog.setCancelable(false);
            mUnzipProgressDialog.setProgressStyle(android.R.attr.progressBarStyleSmall);
            mUnzipProgressDialog.setMessage(getString(R.string.package_unzip));
            mUnzipProgressDialog.show();
            mUnzipProgressDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                    if (keyCode == KeyEvent.KEYCODE_BACK) {
                        OtaPkgManagerActivity.this.finish();
                        return true;
                    }
                    return false;
                }
            });
            mUnzipProgressDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    mUnzipProgressDialog = null;
                }
            });
            return mUnzipProgressDialog;

        case DIALOG_NOENOUGHSPACE:
            mDownloadStorageDialog = new AlertDialog.Builder(this).setTitle(R.string.insufficient_space_title)
                    .setMessage(OtaPkgManagerActivity.this.getString(R.string.insufficient_space))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            // GoogleOtaClient.this.resetDescriptionInfo();
                            OtaPkgManagerActivity.this.finish();
                        }
                    }).create();
            return mDownloadStorageDialog;
        case DIALOG_NOSDCARD:
            mDownloadStorageDialog = new AlertDialog.Builder(this).setTitle(R.string.error_sdcard)
                    .setMessage(OtaPkgManagerActivity.this.getString(R.string.sdcard_crash_or_unmount))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            OtaPkgManagerActivity.this.resetDescriptionInfo();
                            OtaPkgManagerActivity.this.finish();
                        }
                    }).create();
            return mDownloadStorageDialog;

        case DIALOG_UNKNOWNERROR:
            mAlertDialog = new AlertDialog.Builder(this).setTitle(R.string.unknown_error)
                    .setMessage(OtaPkgManagerActivity.this.getString(R.string.unknown_error_content))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            OtaPkgManagerActivity.this.finish();
                        }
                    }).create();
            return mAlertDialog;

        case DIALOG_CANCELDOWNLOAD:
            mDownloadCancelDialog = new AlertDialog.Builder(this).setTitle(R.string.cancel_download_title)
                    .setMessage(OtaPkgManagerActivity.this.getString(R.string.cancel_download_content))
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            mDownloadCancelDialog = null;
                            clearDownloadRecord();
                            OtaPkgManagerActivity.this.finish();
                        }
                    }).setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            continueDownload();
                        }
                    }).setOnCancelListener(new DialogInterface.OnCancelListener() {
                        public void onCancel(DialogInterface dialog) {
                            continueDownload();
                        }
                    }).create();
            return mDownloadCancelDialog;

        case DIALOG_OTARESULT:
            mAlertDialog = new AlertDialog.Builder(this).setTitle(mOTADialogTitleResId)
                    .setMessage(OtaPkgManagerActivity.this.getString(mOTADialogMessageResId))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            if (mNeedReset) {

                                Xlog.i(TAG, "Need reset");
                                mDownloadInfo.resetDownloadInfo();
                                resetDescriptionInfo();
                                NotifyManager notifyMgr = new NotifyManager(OtaPkgManagerActivity.this);
                                notifyMgr.clearNotification(NotifyManager.NOTIFY_DL_COMPLETED);
                            }

                            OtaPkgManagerActivity.this.finish();
                        }
                    }).setOnKeyListener(new DialogInterface.OnKeyListener() {
                        public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                            if (keyCode == KeyEvent.KEYCODE_BACK) {

                                OtaPkgManagerActivity.this.finish();
                                return true;
                            }
                            return false;
                        }
                    }).create();
            return mAlertDialog;

        case DIALOG_WARN_DATA_COST:
            mAlertDialog = new AlertDialog.Builder(this).setIcon(R.drawable.alert_downloading)
                    .setTitle(OtaPkgManagerActivity.this.getString(R.string.warn_data_cost_title))
                    .setMessage(OtaPkgManagerActivity.this.getString(R.string.warn_data_cost_content))
                    .setPositiveButton(R.string.warn_data_cost_yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            downloadPkg();
                        }
                    }).setNegativeButton(R.string.warn_data_cost_no, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            initWifiOnlyCheckbox(true, false);
                        }
                    }).create();
            return mAlertDialog;

        case DIALOG_CHANGE_NETWORK:

            mAlertDialog = new AlertDialog.Builder(this).setTitle(R.string.change_network_title)
                    .setMessage(R.string.change_network_content).setCancelable(false)
                    .setPositiveButton(R.string.change_network_wifi_btn, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            OtaPkgManagerActivity.this.startActivity(new Intent(ACTION_WIFI_SETTINGS));

                        }
                    }).setNeutralButton(R.string.change_network_apn_btn, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            Intent it = new Intent(ACTION_APN_SETTINGS);

                            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                                long simid = Settings.System.getLong(getContentResolver(),
                                        Settings.System.GPRS_CONNECTION_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);

                                //int slotid = SimInfoManager.getSlotById(OtaPkgManagerActivity.this, simid);
                                int slotid = -1;
                                SimInfoRecord simRec = SimInfoManager.getSimInfoById(OtaPkgManagerActivity.this, simid);
                                if(simRec != null) {
                                	slotid = simRec.mSimSlotId; 
                                }
                                it.putExtra("simId", slotid);

                                Xlog.d(TAG, "data connection id = " + slotid);
                            }

                            OtaPkgManagerActivity.this.startActivity(it);

                        }
                    }).setNegativeButton(R.string.change_network_discard_btn, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            clearDownloadRecord();

                            OtaPkgManagerActivity.this.finish();
                        }
                    }).setOnKeyListener(new DialogInterface.OnKeyListener() {
                        public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                            if (keyCode == KeyEvent.KEYCODE_BACK) {
                                OtaPkgManagerActivity.this.finish();
                                return true;
                            }
                            return false;
                        }
                    }).create();
            return mAlertDialog;
        case DIALOG_SDCARDMOUNTED:

            return new AlertDialog.Builder(this).setTitle(getString(R.string.error_sdcard))
                    .setMessage(getString(R.string.sdcard_inserted))
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            requeryPackages();

                        }
                    }).setNegativeButton(android.R.string.no, null).create();

        case DIALOG_PACKAGE_DELETED:
            mAlertDialog = new AlertDialog.Builder(this).setTitle(R.string.btn_resume)
                    .setMessage(R.string.package_deleted_error).setCancelable(false)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            if (mService != null) {
                                mService.cancelDlPkg();
                            }

                            NotifyManager notifyMgr = new NotifyManager(OtaPkgManagerActivity.this);
                            notifyMgr.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);

                            OtaPkgManagerActivity.this.finish();
                        }
                    }).create();
            return mAlertDialog;

        case DIALOG_NOTIFY_POWEROFF:

            ProgressDialog powerOffDialog = new ProgressDialog(this);
            powerOffDialog.setIndeterminate(true);
            powerOffDialog.setProgressStyle(android.R.attr.progressBarStyleSmall);
            powerOffDialog.setCancelable(false);
            powerOffDialog.setMessage(getString(R.string.installing_message));
            powerOffDialog.show();
            return powerOffDialog;

        default:
            break;
        }
        return null;
    }

    private void clearDownloadRecord() {
        Util.deleteFile(Util.getPackageFileName(OtaPkgManagerActivity.this));
        mDownloadInfo.resetDownloadInfo();
        if (mService != null) {
            mService.cancelDlPkg();
        }

        NotifyManager notifyMgr = new NotifyManager(OtaPkgManagerActivity.this);
        notifyMgr.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);

    }

    private void continueDownload() {
        int status = mDownloadInfo.getDLSessionStatus();
        if (status == DownloadInfo.STATE_PAUSEDOWNLOAD) {
            Xlog.i(TAG, "onCreateDialog, DIALOG_NETWORKERROR resume");
            downloadPkg();
        }
        mDownloadCancelDialog = null;
    }

    private void onLayoutErrorInfo() {
        Xlog.i(TAG, "onLayoutErrorInfo");

        mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);

        showUILayout(DownloadInfo.STATE_PAUSEDOWNLOAD);

    }

    protected boolean checkUpgradePackage() {

        CheckPkg checkPkg = new CheckPkg(this, UpgradePkgManager.OTA_PACKAGE, Util.getPackageFileName(this));

        mOTAresult = checkPkg.execForResult();
        Xlog.i(TAG, "check_ota result = " + mOTAresult);
        checkPkg.deleteUnusedFile();
        if (mOTAresult == Util.OTAresult.CHECK_OK) {
            return true;
        } else {
            sendCheckOTAMessage();
            return false;
        }

    }

    protected void notifyUserInstall() {

        Xlog.i(TAG, "notifyUserInstall");
        mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_NOTIFY_POWEROFF));

        return;
    }

    private void deleteOtaPackage() {
        Xlog.i(TAG, "deletePackage");

        String strPkg = Util.getPackageFileName(this);

        if (strPkg == null) {
            return;
        }
        File imgf = new File(strPkg);
        if (imgf == null) {
            Xlog.i(TAG, "deletePackage, new file error");
            return;
        }
        if (imgf.exists()) {
            Xlog.i(TAG, "deletePackage, delte package");
            Util.deleteFile(strPkg);
        }
    }

    private void dismissDialog(Dialog dialog) {
        if (dialog != null) {
            dialog.dismiss();
        }
    }

    private void sendCheckOTAMessage() {
        if (mHandler != null) {
            Xlog.i(TAG, "sendCheckOTAMessage, mOTAresult = " + mOTAresult);
            switch (mOTAresult) {
            case Util.OTAresult.CHECK_OK:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_CLOSECLIENTUI));
                break;

            case Util.OTAresult.ERROR_ONLY_FULL_CHANGE_SIZE:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_NEEDFULLPACKAGE));
                deleteOtaPackage();
                // send message to prompt user delta is proper and delete delta
                break;
            case Util.OTAresult.ERROR_ACCESS_SD:
            case Util.OTAresult.ERROR_SD_WRITE_PROTECTED:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_SDCARDERROR));
                // send message to prompt user sdcard error and need check
                break;
            case Util.OTAresult.ERROR_ACCESS_USERDATA:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_USERDATAERROR));
                // send message to prompt user user data partition error and
                // delete image
                break;
            case Util.OTAresult.ERROR_SD_FREE_SPACE:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_SDCARDINFUFFICENT));
                // send message to prompt user sdcard insufficent and need to
                // delete some file form sdcard
                break;
            case Util.OTAresult.ERROR_USERDATA_FREE_SPACE:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_USERDATAINSUFFICENT));
                // send message to prompt user user data insufficent and need to
                // delete some file form sdcard
                break;
            default:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_OTA_PACKAGEERROR));
                deleteOtaPackage();
                // send message to prompt unknown error and delete delta.
                break;
            }
        }
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Xlog.i(TAG, "handleMessage msg.what = " + msg.what);

            switch (msg.what) {

            case SystemUpdateService.MSG_NOTIFY_POWEROFF:
                showDialog(DIALOG_NOTIFY_POWEROFF);
                break;

            case SystemUpdateService.MSG_DL_STARTED:

                showUILayout(DownloadInfo.STATE_DOWNLOADING);
                break;

            case SystemUpdateService.MSG_NETWORKERROR:
            case SystemUpdateService.MSG_FILE_NOT_EXIST:
                onLayoutErrorInfo();
                break;

            case SystemUpdateService.MSG_LARGEPKG:

                mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);

                showDialog(DIALOG_CHANGE_NETWORK);
                break;

            case SystemUpdateService.MSG_DLPKGCOMPLETE:
                removeDialog(DIALOG_UNZIPPING);
                showUILayout(DownloadInfo.STATE_DLPKGCOMPLETE);
                break;
            case SystemUpdateService.MSG_DLPKGUPGRADE:
                updateRatio();
                break;
            case SystemUpdateService.MSG_DELTADELETED:
                showDialog(DIALOG_PACKAGE_DELETED);
                break;

            /*
             * case SystemUpdateService.MSG_DELTADELETED: onRequeryNeed();
             * break;* case SystemUpdateService.MSG_SDCARDCRASHORUNMOUNT:
             * onNonDialogPrompt(R.string.sdcard_crash_or_unmount); break;
             */

            case SystemUpdateService.MSG_SDCARDUNKNOWNERROR:
                dismissDialog(mUnzipProgressDialog);
                showDialog(DIALOG_NOSDCARD);
                break;
            case SystemUpdateService.MSG_SDCARDINSUFFICENT:
                dismissDialog(mUnzipProgressDialog);
                showDialog(DIALOG_NOENOUGHSPACE);
                break;
            case SystemUpdateService.MSG_UNKNOWERROR:
                showDialog(DIALOG_UNKNOWNERROR);
                break;
            case SystemUpdateService.MSG_OTA_PACKAGEERROR:
                mNeedReset = true;
                mOTADialogTitleResId = R.string.package_error_title;
                mOTADialogMessageResId = R.string.package_error_message_invalid;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_OTA_NEEDFULLPACKAGE:
                mNeedReset = true;
                mOTADialogTitleResId = R.string.package_error_title;
                mOTADialogMessageResId = R.string.package_error_message_full;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_OTA_SDCARDERROR:
                mNeedReset = false;
                mOTADialogTitleResId = R.string.package_error_title;
                mOTADialogMessageResId = R.string.unmount_sdcard;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_OTA_USERDATAERROR:
                mNeedReset = true;
                mOTADialogTitleResId = R.string.package_error_title;
                mOTADialogMessageResId = R.string.package_error_message_crash;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_OTA_SDCARDINFUFFICENT:
                mNeedReset = false;
                mOTADialogTitleResId = R.string.package_error_title;
                mOTADialogMessageResId = R.string.insufficient_space;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_OTA_USERDATAINSUFFICENT:
                mNeedReset = false;
                mOTADialogTitleResId = R.string.package_error_title;
                mOTADialogMessageResId = R.string.package_error_message_insuff;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_UNZIP_LODING:
                showDialog(DIALOG_UNZIPPING);
                break;
            case SystemUpdateService.MSG_CKSUM_ERROR:
            case SystemUpdateService.MSG_UNZIP_ERROR:
                dismissDialog(mUnzipProgressDialog);
                mNeedReset = true;
                mOTADialogTitleResId = R.string.package_unzip_error;
                mOTADialogMessageResId = R.string.package_error_message_invalid;
                showDialog(DIALOG_OTARESULT);
                break;
            case SystemUpdateService.MSG_OTA_CLOSECLIENTUI:
                OtaPkgManagerActivity.this.finish();
                break;
            /*
             * case SystemUpdateService.MSG_LARGEPKG: pauseDownloadPkg(false);
             * showDialog(DIALOG_CHANGE_NETWORK);
             */
            default:
                super.handleMessage(msg);
            }
        }
    };

    private ServiceConnection mConnection = new ServiceConnection() {

        private void clearNotification(Context context, int notifyId) {
            NotifyManager notifyMgr = new NotifyManager(context);
            notifyMgr.clearNotification(notifyId);
        }

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {

            mService = ((SystemUpdateService.ServiceBinder) service).getService();
            Xlog.i(TAG, "onServiceConnected, mService = " + mService);

            mService.setHandler(mHandler);

            int status = mDownloadInfo.getDLSessionStatus();
            Xlog.i(TAG, "onServiceConnected, download status = " + status);
            switch (status) {

            case DownloadInfo.STATE_NEWVERSION_READY:
                // case DownloadInfo.STATE_PAUSEDOWNLOAD:
                // mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_DOWNLOADING);
                clearNotification(OtaPkgManagerActivity.this, NotifyManager.NOTIFY_NEW_VERSION);
                mService.startDlPkg();
                // M: add by mtk80800: enter the downloading & installing
                // process, mark it
                DownloadInfo.getInstance(getApplicationContext()).setActivityID(Integer.MAX_VALUE);

                // M: End add
                showUILayout(DownloadInfo.STATE_DOWNLOADING);

                break;
            case DownloadInfo.STATE_DOWNLOADING:

                if (mService != null) {

                    if (!mService.checkIsDownloading()) {
                        onLayoutErrorInfo();
                    }

                }
                break;

            case DownloadInfo.STATE_PACKAGEUNZIPPING:

                showUnzippingInterface();

                break;

            default:
                break;
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            Xlog.i(TAG, "onServiceDisconnected");
            mService = null;
        }

    };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Util.Action.ACTION_MEDIA_MOUNT_UPDATEUI.equals(action)) {
                // M: Add by mtk80800, sd card mounted, reminder user to refresh
                showDialog(DIALOG_SDCARDMOUNTED);

            } else if (Util.Action.ACTION_MEDIA_UNMOUNT_UPDATEUI.equals(action)) {
                // M: Add by mtk80800, sdcard unmount, reset download info and
                // finish this activity
                String availablePath = Util.getAvailablePath(OtaPkgManagerActivity.this);
                String path = (String) intent.getExtra("storagePath");
                if (availablePath != null && availablePath.contains(path)) {
                    Xlog.w(TAG, "SDcard unmount, finish activity");
                    if (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_NEWVERSION_READY) {
                        Toast.makeText(context, R.string.sdcard_unmount, Toast.LENGTH_LONG).show();
                    }
                    OtaPkgManagerActivity.this.finish();
                }
            }
        }
    };

    public static void stopSelf() {
        if (sInstance != null) {
            Xlog.i(TAG, "stopped Self");
            sInstance.finish();
        }
    }
}
