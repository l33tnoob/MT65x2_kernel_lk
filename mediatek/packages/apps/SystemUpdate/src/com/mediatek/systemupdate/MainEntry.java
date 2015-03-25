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

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;
import com.mediatek.common.featureoption.FeatureOption;

import java.util.ArrayList;
import java.util.List;

/**
 * MainEntry supplies the interface with users, and it queries OTA packages, searches SD card update
 * packages.
 * 
 * @author mtk80800
 * 
 */
public class MainEntry extends PreferenceActivity implements OnDismissListener {

    private static final String TAG = "SystemUpdate/MainEntry";

    private static final int MENU_ID_REFRESH = Menu.FIRST;

    private static final int DIALOG_SDCARDMOUNTED = 7;

    private boolean mQueryDone;

    private View mLoadingContainer;
    private View mListContainer;
    private PreferenceScreen mParentPreference;
    private PreferenceCategory mOtaCategory;
    private PreferenceCategory mSdCategory;
    private List<UpdatePackageInfo> mUpdateInfoList = new ArrayList<UpdatePackageInfo>();;

    private List<Integer> mUiLoopList = new ArrayList<Integer>();
    private boolean mIsWaitingForUpdateUI;
    private DownloadInfo mDownloadInfo;
    private SystemUpdateService mService;
    private StorageBroadcastReceiver mReceiver;

    private int mDialogId;

    private boolean mIsStarted;
    private boolean mIsStopping;
    private boolean mIsQuerying;
    private boolean mIsTurnToDetail;
    private boolean mIsFoundNewVersion;

    private NotifyManager mNotification;

    private static MainEntry sInstance;

    static MainEntry getInstance() {
        return sInstance;
    }

    public boolean isStarted() {
        return mIsStarted;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Xlog.i(TAG, "[onCreate] enter, thread name = " + Thread.currentThread().getName());
        super.onCreate(savedInstanceState);
        sInstance = this;
        this.getActionBar().setDisplayHomeAsUpEnabled(true);

        mNotification = new NotifyManager(this);
        mDownloadInfo = DownloadInfo.getInstance(getApplicationContext());

        Xlog.i(TAG, "[onCreate] prepare UI");

        setContentView(R.layout.list);
        mLoadingContainer = findViewById(R.id.loading_container);
        mListContainer = findViewById(R.id.list_container);
        

        TextView empty = (TextView) findViewById(android.R.id.empty);
        
        //This part is just for CTA test, could remove it after the CTA test passed.
        //The same case in PkgManagerBaseActivity's fillReleaseNotes() method;
        //if(FeatureOption.MTK_MOBILE_MANAGEMENT){
        	//empty.setText(getString(R.string.no_new_version)+"\n\n"+getString(R.string.cta_warning));
        //}

        mLoadingContainer.setVisibility(View.VISIBLE);
        mListContainer.setVisibility(View.INVISIBLE);

        addPreferencesFromResource(R.xml.entry);
        mParentPreference = getPreferenceScreen();
        mParentPreference.setOrderingAsAdded(true);

        IntentFilter filter = new IntentFilter();
        filter.addAction(Util.Action.ACTION_MEDIA_MOUNT_UPDATEUI);
        filter.addAction(Util.Action.ACTION_MEDIA_UNMOUNT_UPDATEUI);
        mReceiver = new StorageBroadcastReceiver();
        registerReceiver(mReceiver, filter);

    }

    @Override
    protected void onDestroy() {
        Xlog.i(TAG, "onDestroy");

        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
        }

        sInstance = null;
        super.onDestroy();
    }

    @Override
    protected void onStart() {
        super.onStart();
        mIsStarted = true;

        mIsStopping = false;
        mIsQuerying = false;
        mIsTurnToDetail = false;
        mIsFoundNewVersion = false;

        mNotification.clearNotification(NotifyManager.NOTIFY_NEW_VERSION);
        int activityId = mDownloadInfo.getActivityID();
        Xlog.i(TAG, "[onStart] mActivityOrder = " + activityId);

        if (activityId >= 0) {
            Intent intent = null;
            if (activityId == Integer.MAX_VALUE) {
                SdPkgInstallActivity.stopSelf();
                intent = this.getInfoIntent(null);
            } else {
                PackageInfoReader geter = new PackageInfoReader(this,
                        Util.PathName.PKG_INFO_IN_DATA);
                UpdatePackageInfo info = geter.getInfo(activityId);
                if (info != null) {
                    OtaPkgManagerActivity.stopSelf();
                    intent = this.getInfoIntent(info);
                }
            }

            if (intent != null) {
                mIsTurnToDetail = true;
                this.startActivity(intent);
                finish();
                return;
            } else {
                Xlog.w(TAG, "[onStart] get updatePackageInfo error, reset activityId");
                activityId = -1;
                mDownloadInfo.setActivityID(activityId);
            }
        }

        if (activityId < 0) {
            SdPkgInstallActivity.stopSelf();
            OtaPkgManagerActivity.stopSelf();

            Intent serviceIntent = new Intent(this, SystemUpdateService.class);
            bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        }

        Xlog.i(TAG, "[onStart], windows focus = " + this.hasWindowFocus());
    }

    @Override
    protected void onStop() {
        Xlog.i(TAG, "onStop");
        mIsStarted = false;
        if (mDialogId > 0) {
            dismissDialog(mDialogId);
        }

        if (!mIsTurnToDetail && mIsFoundNewVersion && !mDownloadInfo.getIfNeedRefresh()) {
            Xlog.v(TAG, "[onStop] is stopping, show notification instead");
            mNotification.showNewVersionNotification();
        }
        if (mService != null) {
            mService.resetHandler(mUiHandler);
            unbindService(mConnection);
            mService = null;
        }

        super.onStop();
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Xlog.d(TAG, "[onCreateContextMenu] show fresh menu " + mQueryDone);
        if (mQueryDone) {
            menu.add(0, MENU_ID_REFRESH, 0, R.string.menu_stats_refresh)
                    .setIcon(R.drawable.ic_menu_refresh_holo_dark)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);

        }
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_ID_REFRESH:
            Xlog.d(TAG, "--[onOptionsItemSelected], refresh --");
            requeryPackages();
            return true;
        case android.R.id.home:
            Xlog.d(TAG, "[onOptionsItemSelected], android.R.id.home");
            mIsStopping = true;
            finish();
            return true;
        default:
            return false;
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        mIsTurnToDetail = true;
        MainEntry.this.finish();
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        Xlog.i(TAG, "onCreateDialog id, dialog id = " + id);
        Dialog dialog = null;
        switch (id) {
        case DIALOG_SDCARDMOUNTED:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.error_sdcard)
                    .setMessage(R.string.sdcard_inserted)
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Xlog.i(TAG, "Dialog SDCARDMOUNTED: query again");
                            requeryPackages();
                        }
                    }).setNegativeButton(android.R.string.no, null).create();
            dialog.setOnDismissListener(this);
            break;
        default:
            break;
        }
        return dialog;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Xlog.w(TAG, "event.getFlages()" + event.getFlags());
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            Xlog.w(TAG, "onKeyDown, keycode is KEYCODE_BACK");
            mIsStopping = true;
            break;
        default:
            break;
        }
        return super.onKeyDown(keyCode, event);
    }

    private Handler mUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Xlog.d(TAG,
                    "[mUiHandler] handlerMessage " + msg.what + ", Thread = "
                            + Thread.currentThread());
            switch (msg.what) {
            case SystemUpdateService.MSG_LARGEPKG:
            case SystemUpdateService.MSG_DELTADELETED:
            case SystemUpdateService.MSG_NOTIFY_QUERY_DONE:

                processOtaBehavior();

                break;
            case SystemUpdateService.MSG_SDCARDPACKAGESDETECTED:

                mUpdateInfoList = (List<UpdatePackageInfo>) msg.obj;
                Xlog.v(TAG, "[mUiHandler] mUpdateInfoList size " + mUpdateInfoList.size());
                processSdBehavior();

                break;
            case SystemUpdateService.MSG_UNKNOWERROR:

                processOtaBehavior();
                Toast.makeText(MainEntry.this, R.string.unknown_error_content, Toast.LENGTH_LONG)
                        .show();

                break;
            default:
                break;
            }
        }
    };

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Xlog.i(TAG, "[onServiceConnected], thread name = " + Thread.currentThread().getName());

            mService = ((SystemUpdateService.ServiceBinder) service).getService();
            Xlog.i(TAG, "[onServiceConnected], mService = " + mService);
            if (mService != null) {
                mService.setHandler(mUiHandler);
            }

            boolean needRescan = mDownloadInfo.getIfNeedRefresh();
            if (needRescan
                    || (!loadHistoryPackage() && DownloadInfo.STATE_NEWVERSION_READY != mDownloadInfo
                            .getDLSessionStatus())) {
                Xlog.d(TAG, "[onServiceConnected], need query packages");
                queryPackagesInternal();
            } else {
                Xlog.d(TAG, "[onServiceConnected], DON'T need query, load from file");
                refreshUI();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Xlog.i(TAG, "[onServiceDisconnected]");
            if (mService != null) {
                mService.setHandler(null);
                mService = null;
            }
        }

    };

    private void requeryPackages() {
        Xlog.i(TAG, "[queryPackages]");
        mDownloadInfo.setIfNeedRefresh(true);
        mIsFoundNewVersion = false;

        if (mService != null) {
            queryPackagesInternal();
        } else {
            Intent serviceIntent = new Intent(this, SystemUpdateService.class);
            bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        }
    }

    private void queryPackagesInternal() {
        Xlog.i(TAG, "[queryPackagesInternal]");
        showLoadingContainer();
        mUpdateInfoList.clear();
        mIsQuerying = true;
        if (mService != null) {
            mService.queryPackages();
        }
    }

    private boolean loadHistoryPackage() {
        showLoadingContainer();
        mUpdateInfoList.clear();
        mUiLoopList.clear();
        if (mService != null) {
            mUpdateInfoList = mService.loadPackages();
        }
        boolean result = mUpdateInfoList != null && !mUpdateInfoList.isEmpty();
        Xlog.v(TAG, "loadHistoryPackage result = " + result);
        return result;
    }

    private void processOtaBehavior() {
        Xlog.v(TAG, "processOtaBehavior");
        if (Util.getUpdateType() == Util.UPDATE_TYPES.OTA_UPDATE_ONLY) {
            refreshUI();
        }
    }

    private void processSdBehavior() {
        Xlog.v(TAG, "processSdBehavior");
        refreshUI();
    }

    private void refreshUI() {
        Xlog.v(TAG, "[refreshUI]");
        int size = (mUpdateInfoList == null) ? 0 : mUpdateInfoList.size();
        mParentPreference.removeAll();

        boolean isOtaExist = isOtaPackageExist();

        if (isOtaExist || size > 0) {
            mIsFoundNewVersion = true;

            if (mIsStopping && mIsQuerying) {
                Xlog.v(TAG, "[refreshUI] is stopping, show notification instead");
                mNotification.showNewVersionNotification();
                mIsFoundNewVersion = false;
                return;
            }

            mIsQuerying = false;

        }

        if (isOtaExist) {
            if (size == 0) {
                Xlog.v(TAG, "[refreshUI] Only OTA package exists, start OTA detail");
                Intent intent = getInfoIntent(null);
                mIsTurnToDetail = true;
                startActivity(intent);
                finish();
                return;

            } else {
                Xlog.v(TAG, "[refreshUI] show packages list, need devider");

                mOtaCategory = new PreferenceCategory(this);
                mSdCategory = new PreferenceCategory(this);
                mOtaCategory.setTitle(R.string.ota_category_title);
                mSdCategory.setTitle(R.string.sd_category_title);
                mParentPreference.addPreference(mOtaCategory);
                mParentPreference.addPreference(mSdCategory);

                addPreference(null, mOtaCategory);

                for (UpdatePackageInfo info : mUpdateInfoList) {
                    addPreference(info, mSdCategory);
                }

            }
        } else {
            if (size == 0) {
                Xlog.v(TAG, "[refreshUI] no update packages, show empty view");
            } else if (size == 1) {
                mIsTurnToDetail = true;
                Xlog.v(TAG, "[refreshUI] Only SD package exists, start SD detail");

                Intent intent = getInfoIntent(mUpdateInfoList.get(0));
                startActivity(intent);
                finish();
                return;

            } else {
                Xlog.v(TAG, "[refreshUI] All SD packages, show packages list, NO need devider");

                for (UpdatePackageInfo info : mUpdateInfoList) {
                    addPreference(info, mParentPreference);
                }

            }
        }

        showListContainer();
    }

    private boolean isOtaPackageExist() {
        int dlSession = mDownloadInfo.getDLSessionStatus();
        Xlog.v(TAG, "mDownloadInfo.getDLSessionStatus = " + dlSession);
        return dlSession == DownloadInfo.STATE_NEWVERSION_READY;
    }

    private Intent getInfoIntent(UpdatePackageInfo info) {
        Intent intent = new Intent();
        if (info == null) {
            intent.setClass(this, OtaPkgManagerActivity.class);
        } else {
            intent.setClass(this, SdPkgInstallActivity.class);
            intent.putExtra(SdPkgInstallActivity.KEY_VERSION, info.version);
            intent.putExtra(SdPkgInstallActivity.KEY_PATH, info.path);
            intent.putExtra(SdPkgInstallActivity.KEY_ANDROID_NUM, info.androidNumber);
            intent.putExtra(SdPkgInstallActivity.KEY_NOTES, info.notes);
            Xlog.v(TAG,
                    "[getInfoIntent], the order of the packageInfo is "
                            + mUpdateInfoList.indexOf(info));
            intent.putExtra(SdPkgInstallActivity.KEY_ORDER, mUpdateInfoList.indexOf(info));
        }
        return intent;
    }

    private void addPreference(UpdatePackageInfo info, PreferenceGroup parentPreference) {
        Preference preference = new Preference(this);
        String title = (info == null) ? mDownloadInfo.getAndroidNum() + " Version "
                + mDownloadInfo.getVerNum() : info.androidNumber + " Version " + info.version;
        preference.setTitle(title);

        String summary = info == null ? getString(R.string.ota_preference_summary) : info.path;
        preference.setSummary(summary);

        Intent intent = getInfoIntent(info);
        preference.setIntent(intent);
        parentPreference.addPreference(preference);
    }

    private void showListContainer() {
        Xlog.v(TAG, "showListContainer");
        if (View.VISIBLE == mLoadingContainer.getVisibility()) {
            mLoadingContainer.startAnimation(AnimationUtils.loadAnimation(MainEntry.this,
                    android.R.anim.fade_out));
            mListContainer.startAnimation(AnimationUtils.loadAnimation(MainEntry.this,
                    android.R.anim.fade_in));
        }
        mListContainer.setVisibility(View.VISIBLE);
        mLoadingContainer.setVisibility(View.INVISIBLE);
        mQueryDone = true;
        invalidateOptionsMenu();
    }

    private void showLoadingContainer() {
        if (View.INVISIBLE == mLoadingContainer.getVisibility()) {
            mLoadingContainer.setVisibility(View.VISIBLE);
            mListContainer.setVisibility(View.INVISIBLE);
        }
        mQueryDone = false;
        invalidateOptionsMenu();
    }

    private class StorageBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Util.Action.ACTION_MEDIA_MOUNT_UPDATEUI.equals(action)) {
                Xlog.v(TAG, "Received ACTION_MEDIA_MOUNT_UPDATEUI");
                // M: Add by mtk80800, sd card mounted, reminder user to refresh
                if (mIsStarted) {
                    Xlog.v(TAG, "MainEntry.this.isResumed(), show mounted dialog");
                    mDialogId = DIALOG_SDCARDMOUNTED;
                    showDialog(DIALOG_SDCARDMOUNTED);
                }
            } else if (Util.Action.ACTION_MEDIA_UNMOUNT_UPDATEUI.equals(action)) {
                // M: Add by mtk80800, sdcard unmount, reset download info and finish this activity
                Xlog.w(TAG, "Receive ACTION_MEDIA_UNMOUNT_UPDATEUI, finish");
                Toast.makeText(MainEntry.this, R.string.sdcard_unmount, Toast.LENGTH_LONG).show();
                MainEntry.this.finish();
            }
        }
    }

    @Override
    public void onDismiss(DialogInterface arg0) {
        mDialogId = -1;
    };
}
