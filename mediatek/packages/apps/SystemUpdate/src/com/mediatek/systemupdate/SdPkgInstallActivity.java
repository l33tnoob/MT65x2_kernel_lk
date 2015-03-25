package com.mediatek.systemupdate;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.zip.ZipFile;

public class SdPkgInstallActivity extends PkgManagerBaseActivity implements
        DialogInterface.OnCancelListener, DialogInterface.OnClickListener,
        DialogInterface.OnDismissListener {

    private static final String TAG = "SystemUpdate/SdPkgInstall";

    public static final String KEY_ANDROID_NUM = "key_android_num";
    public static final String KEY_VERSION = "key_version";
    public static final String KEY_PATH = "key_path";
    public static final String KEY_NOTES = "key_notes";
    public static final String KEY_ORDER = "key_order";
    private static final String UPDATE_PACKAGE_NAME = Util.PathName.ENTRY_PACKAGE_NAME;
    private static final String DES_PACKAGE = Util.PathName.OTA_PKG_FOLDER
            + Util.PathName.PACKAGE_NAME;

    private static final int DIALOG_INSTALLWARNING = 0;
    private static final int DIALOG_NOENOUGHSPACE = 1;
    private static final int DIALOG_NOSDCARD = 2;
    private static final int DIALOG_OTARESULT = 4;
    private static final int DIALOG_UNZIPPING = 5;
    private static final int DIALOG_UNKNOWN_ERROR = 6;
    private static final int DIALOG_SDCARDMOUNTED = 7;
    private static final int DIALOG_INVALIDATEPACKAGE = 8;

    private static final int MENU_ID_UPGRADE = Menu.FIRST;
    private static final int MENU_ID_REFRESH = Menu.FIRST + 1;

    private String mPath;
    private String mUpdatePath;
    private String mVersion;
    private int mActivityOrder;
    private int mOTADialogTitleResId;
    private int mOTADialogMessageResId;
    private int mCurrentDialog;
    private ProgressDialog mUnzipProgressDialog;
    private static boolean sIsUnzip;
    private boolean mGoToMainEntry;
    private NotifyManager mNotifyManager;

    private DownloadInfo mDownloadInfo;

    private static SdPkgInstallActivity sInstance;
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            Xlog.i(TAG, "handleMessage msg.what = " + msg.what);

            switch (msg.what) {
            case SystemUpdateService.MSG_SDCARDUNKNOWNERROR:
                if (mUnzipProgressDialog != null) {
                    sIsUnzip = false;
                    mUnzipProgressDialog.dismiss();
                }

                showDialog(DIALOG_NOSDCARD);
                break;
            case SystemUpdateService.MSG_SDCARDINSUFFICENT:
                if (mUnzipProgressDialog != null) {
                    sIsUnzip = false;
                    mUnzipProgressDialog.dismiss();
                }
                showDialog(DIALOG_NOENOUGHSPACE);
                break;
            case SystemUpdateService.MSG_SDCARDPACKAGEINVALIDATE:
                showDialog(DIALOG_INVALIDATEPACKAGE);
                break;
            case SystemUpdateService.MSG_UNKNOWERROR:
                showDialog(DIALOG_UNKNOWN_ERROR);
                break;
            case SystemUpdateService.MSG_UNZIP_LODING:
                showDialog(DIALOG_UNZIPPING);
                break;
            case SystemUpdateService.MSG_CKSUM_ERROR:
            case SystemUpdateService.MSG_UNZIP_ERROR:
                if (mUnzipProgressDialog != null) {
                    sIsUnzip = false;
                    mUnzipProgressDialog.dismiss();
                }
                mOTADialogTitleResId = R.string.package_unzip_error;
                mOTADialogMessageResId = R.string.package_error_message_invalid;
                showDialog(DIALOG_OTARESULT);
                break;
            default:
                super.handleMessage(msg);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.i(TAG, "onCreate");

        sInstance = this;

        setContentView(R.layout.sd_package_ready);
        this.getActionBar().setDisplayHomeAsUpEnabled(true);

        Intent intent = getIntent();
        updateUi(intent);
        mDownloadInfo = DownloadInfo.getInstance(getApplicationContext());
        mNotifyManager = new NotifyManager(this);
        IntentFilter filter = new IntentFilter();
        filter.addAction(Util.Action.ACTION_MEDIA_MOUNT_UPDATEUI);
        filter.addAction(Util.Action.ACTION_MEDIA_UNMOUNT_UPDATEUI);
        registerReceiver(mReceiver, filter);
    }

    private void updateUi(Intent intent) {
        String strAndroidNum = intent.getStringExtra(KEY_ANDROID_NUM);
        mVersion = intent.getStringExtra(KEY_VERSION);
        mPath = intent.getStringExtra(KEY_PATH);
        mActivityOrder = intent.getIntExtra(KEY_ORDER, 0);
        String strNotes = intent.getStringExtra(KEY_NOTES);

        if (strAndroidNum != null && mVersion != null && mPath != null) {
            fillPkgInfo(strAndroidNum, mVersion, -1L, mPath);
        }
        if (strNotes != null) {
            List<String> listNotes = Arrays.asList(strNotes.split(File.separator));
            fillReleaseNotes(listNotes);
        }

        String packagePath = Util.getPackagePathName(this);
        if (packagePath != null) {
            mUpdatePath = packagePath + Util.PathName.PACKAGE_NAME;
        }

        if (sIsUnzip && mHandler != null) {
            mHandler.sendEmptyMessage(SystemUpdateService.MSG_UNZIP_LODING);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Xlog.i(TAG, "onStart");
        OtaPkgManagerActivity.stopSelf();
        mGoToMainEntry = false;
        mNotifyManager.clearNotification(NotifyManager.NOTIFY_NEW_VERSION);
    }

    @Override
    protected void onStop() {
        Xlog.i(TAG, "onStop");
        if (!mGoToMainEntry && !mDownloadInfo.getIfNeedRefresh()
                && mDownloadInfo.getActivityID() < 0
                && (MainEntry.getInstance() == null || !MainEntry.getInstance().isStarted())) {
            Xlog.v(TAG, "background, show new version notification");
            mNotifyManager.showNewVersionNotification();
        }

        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Xlog.i(TAG, "onDestroy");
        unregisterReceiver(mReceiver);

        sInstance = null;
        super.onDestroy();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Xlog.d(TAG, "onNewIntent " + mPath);
        if (mPath == null || !mPath.equals(intent.getStringExtra(KEY_PATH))) {
            sIsUnzip = false;
            if (mUnzipProgressDialog != null) {
                mUnzipProgressDialog.dismiss();
            }
            if (mCurrentDialog >= 0) {
                this.dismissDialog(mCurrentDialog);
            }
            updateUi(intent);
        }
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
                    SdPkgInstallActivity.this.startActivity(new Intent(this, MainEntry.class));
                }
            }
            break;
        default:
            break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        Xlog.i(TAG, "onCreateDialog id, dialog id = " + id);
        Dialog dialog = null;
        switch (id) {
        case DIALOG_INSTALLWARNING:// warning user before install
            dialog = new AlertDialog.Builder(this).setTitle(R.string.install_sd_title)
                    .setMessage(getString(R.string.install_sd_message))
                    .setPositiveButton(R.string.btn_install, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int whichButton) {
                            invalidateOptionsMenu();
                            mDownloadInfo.setActivityID(mActivityOrder);
                            mDownloadInfo.setTargetVer(mVersion);
                            mDownloadInfo.setUpgradeStartedState(true);
                            installPackage(DES_PACKAGE, mVersion);
                        }
                    })
                    .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Xlog.v(TAG, "Dialog_Install_warning, cancel,finish activity");
                            if (!mDownloadInfo.getIfNeedRefreshMenu()) {

                                if (mDownloadInfo.getActivityID() < 0) {
                                    mGoToMainEntry = true;
                                    SdPkgInstallActivity.this.startActivity(new Intent(
                                            SdPkgInstallActivity.this, MainEntry.class));
                                }
                            }
                            SdPkgInstallActivity.this.finish();
                        }
                    }).create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        case DIALOG_NOSDCARD: // error occur before unzip update.zip to correct SD card
            dialog = new AlertDialog.Builder(this).setTitle(R.string.error_sdcard)
                    .setMessage(getString(R.string.sdcard_crash_or_unmount))
                    .setOnCancelListener(this).setPositiveButton(android.R.string.ok, this)
                    .create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        case DIALOG_NOENOUGHSPACE:// error occur before unzip update.zip to correct SD card
            dialog = new AlertDialog.Builder(this).setTitle(R.string.insufficient_space_title)
                    .setMessage(getString(R.string.install_sd_info))
                    .setOnCancelListener(new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                            mDownloadInfo.setUpgradeStartedState(false);
                        }
                    })
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            SdPkgInstallActivity.this.finish();
                            Xlog.v(TAG, "Dialog_No_enough_space, OK,finish activity");
                        }
                    }).create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        case DIALOG_INVALIDATEPACKAGE:
            dialog = new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.package_error_title))
                    .setMessage(getString(R.string.package_error_message_invalid))
                    .setOnCancelListener(this).setPositiveButton(android.R.string.yes, this)
                    .create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        case DIALOG_UNZIPPING:
            mUnzipProgressDialog = new ProgressDialog(this);
            mUnzipProgressDialog.setIndeterminate(true);
            mUnzipProgressDialog.setCancelable(false);
            mUnzipProgressDialog.setProgressStyle(android.R.attr.progressBarStyleSmall);
            mUnzipProgressDialog.setMessage(getString(R.string.installing_message));
            mUnzipProgressDialog.show();
            sIsUnzip = true;
            mUnzipProgressDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                    if (keyCode == KeyEvent.KEYCODE_BACK) {
                        SdPkgInstallActivity.this.finish();
                        Xlog.v(TAG, "DIALOG_UNZIPPING,Keycode_back,finish activity");
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
        case DIALOG_UNKNOWN_ERROR:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.unknown_error)
                    .setMessage(getString(R.string.unknown_error_content))
                    .setOnCancelListener(new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                            mDownloadInfo.setUpgradeStartedState(false);
                        }
                    })
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Xlog.v(TAG, "Dialog_Unknown_error, ok,finish activity");
                            Util.deleteFile(mUpdatePath);
                            SdPkgInstallActivity.this.finish();
                        }
                    }).create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        case DIALOG_OTARESULT:
            dialog = new AlertDialog.Builder(this).setTitle(mOTADialogTitleResId)
                    .setMessage(getString(mOTADialogMessageResId)).setOnCancelListener(this)
                    .setPositiveButton(android.R.string.ok, this).create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        case DIALOG_SDCARDMOUNTED:
            dialog = new AlertDialog.Builder(this).setTitle(getString(R.string.error_sdcard))
                    .setMessage(getString(R.string.sdcard_inserted))
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {

                            mGoToMainEntry = true;
                            requeryPackages();
                        }
                    }).setNegativeButton(android.R.string.no, null).create();
            mCurrentDialog = id;
            dialog.setOnDismissListener(this);
            break;
        default:
            break;
        }
        return dialog;
    }

    private void finishInstallProcess() {
        Util.deleteFile(mUpdatePath);
        mDownloadInfo.resetDownloadInfo();
        mDownloadInfo.setUpgradeStartedState(false);
        SdPkgInstallActivity.this.finish();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        Xlog.v(TAG, "Dialog_OTA,OK, finishInstallProcess");
        finishInstallProcess();
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        Xlog.v(TAG, "Dialog canceled, finishInstallProcess");
        finishInstallProcess();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        Xlog.v(TAG, "Dialog onDismiss");
        mCurrentDialog = -1;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, MENU_ID_UPGRADE, 0, R.string.btn_install).setShowAsAction(
                MenuItem.SHOW_AS_ACTION_ALWAYS);

        if (mDownloadInfo.getIfNeedRefreshMenu() && mDownloadInfo.getActivityID() < 0) {
            menu.add(0, MENU_ID_REFRESH, 0, R.string.menu_stats_refresh)
                    .setIcon(R.drawable.ic_menu_refresh_holo_dark)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER);
        }

        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_ID_UPGRADE:
            showDialog(DIALOG_INSTALLWARNING);
            return true;
        case android.R.id.home:

            Xlog.v(TAG, "Navigation key,finish activity");
            if (!mDownloadInfo.getIfNeedRefreshMenu()) {

                if (mDownloadInfo.getActivityID() < 0) {
                    mGoToMainEntry = true;
                    SdPkgInstallActivity.this.startActivity(new Intent(this, MainEntry.class));
                }
            }
            SdPkgInstallActivity.this.finish();
            break;
        case MENU_ID_REFRESH:
            mGoToMainEntry = true;
            requeryPackages();
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected boolean checkUpgradePackage() {
        if (mHandler != null) {
            mHandler.sendEmptyMessage(SystemUpdateService.MSG_UNZIP_LODING);
        }

        if (!checkSdCard()) {
            Xlog.w(TAG, "[checkUpgradePackage], checkSdCardOk fail");
            return false;
        }

        if (!unzipInstallFile()) {
            Xlog.w(TAG, "[checkUpgradePackage], unzipInstallFile fail");
            return false;
        }

        return true;
    }

    private boolean checkSdCard() {
        // check sdcard space
        if (Util.isSdcardAvailable(this)) {
            File f = new File(mPath);
            if (f.exists()) {
                long insufficientSpace = Util.getExtraSpaceNeeded(this, (long) (1.5 * f.length()));
                if (insufficientSpace < 0) {
                    if (mHandler != null) {
                        mHandler.sendEmptyMessage(SystemUpdateService.MSG_SDCARDINSUFFICENT);
                    }
                    return false;
                }
            } else {
                if (mHandler != null) {
                    mHandler.sendEmptyMessage(SystemUpdateService.MSG_SDCARDPACKAGEINVALIDATE);
                }
            }
        } else {
            if (mHandler != null) {
                mHandler.sendEmptyMessage(SystemUpdateService.MSG_SDCARDUNKNOWNERROR);
            } else {
                mDownloadInfo.resetDownloadInfo();
            }
            return false;
        }

        return true;
    }

    private boolean unzipInstallFile() {
        int result = -1;

        String updateZip = Util.getTempPath(this) + Util.PathName.PACKAGE_NAME;
        try {
            ZipFile updatePackage = new ZipFile(mPath);
            result = Util.unzipFileElement(updatePackage, UPDATE_PACKAGE_NAME, mUpdatePath);
            updatePackage.close();
            Xlog.d(TAG, "[unzipInstallFile], unzip install.zip to googleota folder");
        } catch (IOException e) {
            Xlog.e(TAG, "[unzipInstallFile], unzip file fail");
            e.printStackTrace();
        }

        Util.deleteFile(updateZip);

        if (result != Util.OTAresult.OTA_FILE_UNZIP_OK) {
            if (mHandler != null) {
                mHandler.sendEmptyMessage(SystemUpdateService.MSG_UNZIP_ERROR);
            }
            return false;
        }

        return true;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Util.Action.ACTION_MEDIA_MOUNT_UPDATEUI.equals(action)) {
                // M: Add by mtk80800, sd card mounted, reminder user to refresh
                showDialog(DIALOG_SDCARDMOUNTED);

            } else if (Util.Action.ACTION_MEDIA_UNMOUNT_UPDATEUI.equals(action)) {
                // M: Add by mtk80800, sdcard unmount, reset download info and finish this activity
                String path = (String) intent.getExtra("storagePath");
                if (mPath.contains(path)) {
                    Xlog.w(TAG, "SDcard unmount, finish activity");
                    if (mDownloadInfo.getActivityID() < 0) {
                        Toast.makeText(context, R.string.sdcard_unmount, Toast.LENGTH_LONG).show();
                    }
                    SdPkgInstallActivity.this.finish();
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
