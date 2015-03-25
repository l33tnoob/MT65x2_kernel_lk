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

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.datatransfer.BackupEngine.BackupResultType;
import com.mediatek.datatransfer.BackupService.BackupProgress;
import com.mediatek.datatransfer.BackupService.OnBackupStatusListener;
import com.mediatek.datatransfer.CheckedListActivity.OnCheckedCountChangedListener;
import com.mediatek.datatransfer.ResultDialog.ResultEntity;
import com.mediatek.datatransfer.modules.AppBackupComposer;
import com.mediatek.datatransfer.modules.BookmarkBackupComposer;
import com.mediatek.datatransfer.modules.CalendarBackupComposer;
import com.mediatek.datatransfer.modules.Composer;
import com.mediatek.datatransfer.modules.ContactBackupComposer;
import com.mediatek.datatransfer.modules.MmsBackupComposer;
import com.mediatek.datatransfer.modules.MusicBackupComposer;
import com.mediatek.datatransfer.modules.NoteBookBackupComposer;
import com.mediatek.datatransfer.modules.PictureBackupComposer;
import com.mediatek.datatransfer.modules.SmsBackupComposer;
import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.Constants.ContactType;
import com.mediatek.datatransfer.utils.Constants.DialogID;
import com.mediatek.datatransfer.utils.Constants.State;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Utils;
import com.mediatek.datatransfer.utils.FileUtils;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;

public class PersonalDataBackupActivity extends AbstractBackupActivity implements
        OnCheckedCountChangedListener {
    private static String CLASS_TAG = MyLogger.LOG_TAG + "/PersonalDataBackupActivity";
    private static final float DISABLE_ALPHA = 0.4f;
    private static final float ENABLE_ALPHA = 1.0f;

    private ArrayList<PersonalItemData> mBackupItemDataList = new ArrayList<PersonalItemData>();
    private ArrayList<AlertDialog> dialogs = new ArrayList<AlertDialog>();
    private PersonalDataBackupAdapter mBackupListAdapter;
    InitPersonalDataTask initDataTask = null;
    private OnBackupStatusListener mBackupListener;
    private boolean[] mContactCheckTypes = new boolean[10];
    private boolean[] mMessageCheckTypes = new boolean[2];
    private String CONTACT_TYPE = "contact";
    private String MESSAGE_TYPE = "message";
    private String mBackupFolderPath;

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        super.onCreate(savedInstanceState);
        if (savedInstanceState != null) {
            mContactCheckTypes = savedInstanceState.getBooleanArray(CONTACT_TYPE);
            mMessageCheckTypes = savedInstanceState.getBooleanArray(MESSAGE_TYPE);
        } else {
            getSimInfoList();
            // mContactCheckTypes = new boolean[mSimCount+1];
            for (int index = 0; index < mContactCheckTypes.length; index++) {
                mContactCheckTypes[index] = true;
            }
            mMessageCheckTypes[0] = true;
            mMessageCheckTypes[1] = true;
        }
        Log.i(CLASS_TAG, "onCreate");
        init();
        setProgressBarIndeterminateVisibility(false);
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        Log.i(CLASS_TAG, "onStart");
        if (SDCardUtils.getExternalStoragePath(this) == null) {
            Log.i(CLASS_TAG, "onStart NO SDCARD! cancel initDataTask ");
            finish();
            return;
        }
        initDataTask = new InitPersonalDataTask();
        initDataTask.execute();
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        Log.i(CLASS_TAG, "onStop");
        if (initDataTask != null && initDataTask.getStatus() == AsyncTask.Status.RUNNING) {
            initDataTask.cancel(true);
            mBackupListAdapter.reset();
            initDataTask = null;
        }
    }

    @Override
    protected void onSaveInstanceState(final Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putBooleanArray(CONTACT_TYPE, mContactCheckTypes);
        outState.putBooleanArray(MESSAGE_TYPE, mMessageCheckTypes);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "onResume");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (dialogs != null && !dialogs.isEmpty()) {
            for (AlertDialog dialog : dialogs) {
                dialog.cancel();
            }
        }
        Log.i(CLASS_TAG, "onDestroy");
    }

    private void init() {
        initActionBar();
        updateTitle();
        dialogs.clear();
        registerSIMCardStatusReceiver();
        // getSimInfoList();
    }

    private void registerSIMCardStatusReceiver() {
        // TODO Auto-generated method stub
        SIMCardStatusReceiver simReceiver = new SIMCardStatusReceiver();
        IntentFilter itFilter = new IntentFilter();
        itFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        itFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        itFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        itFilter.addAction(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        itFilter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        this.registerReceiver(simReceiver, itFilter);
    }

    private boolean dissmissDialog = false;
    // Todo: Use PhoneWindowManager directly
    private static final String SYSTEM_DIALOG_REASON_KEY = "reason";
    private static final String SYSTEM_DIALOG_REASON_RECENT_APPS = "recentapps";
    private static final String SYSTEM_DIALOG_REASON_HOME_KEY = "homekey";

    public class SIMCardStatusReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            MyLogger.logD(CLASS_TAG, "BroadcastReceiver new Action " + intent.getAction());
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(intent.getAction())
                    || TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent
                            .getAction())
                    || Intent.ACTION_DUAL_SIM_MODE_CHANGED.equals(intent
                            .getAction())) {
                clearDialogs();
            } else if (Intent.ACTION_CLOSE_SYSTEM_DIALOGS.equals(intent.getAction())) {
                String reason = intent.getStringExtra(SYSTEM_DIALOG_REASON_KEY);
                MyLogger.logD(CLASS_TAG, "BroadcastReceiver new Action " + intent.getAction());
                if (SYSTEM_DIALOG_REASON_RECENT_APPS.equals(reason)
                        || SYSTEM_DIALOG_REASON_HOME_KEY.equals(reason)) {
                    clearDialogs();
                }
            } else if (TelephonyIntents.ACTION_PHB_STATE_CHANGED.equals(intent.getAction())) {
                boolean phbReady = intent.getBooleanExtra("ready", false);
                int slotId = intent.getIntExtra("simId", -10);
                if (phbReady && slotId >= 0) {
                    initDataTask = new InitPersonalDataTask();
                    initDataTask.execute();
                }
            }
        }

    }

    private void clearDialogs() {
        if (dialogs != null && !dialogs.isEmpty()) {
            for (AlertDialog dialog : dialogs) {
                dialog.cancel();
            }
        }
    }

    public void updateTitle() {
        StringBuilder sb = new StringBuilder();
        sb.append(getString(R.string.backup_personal_data));
        int totalNum = getCount();
        int checkedNum = getCheckedCount();
        sb.append("(" + checkedNum + "/" + totalNum + ")");
        // MyLogger.logD(CLASS_TAG,"Title is = "+sb.toString());
        setTitle("");
        setTitle(sb.toString());
    }

    private void initActionBar() {
        ActionBar bar = this.getActionBar();
        bar.setDisplayShowHomeEnabled(false);
    }

    private void startPersonalDataBackup(String folderName) {
        if (folderName == null || folderName.trim().equals("")) {
            return;
        }
        startService();
        if (mBackupService != null) {
            ArrayList<Integer> list = getSelectedItemList();
            mBackupService.setBackupModelList(list);
            if (list.contains(ModuleType.TYPE_CONTACT)) {
                ArrayList<String> params = new ArrayList<String>();
                String[] contactTypes = new String[] {
                        ContactType.ALL, ContactType.PHONE,
                        ContactType.SIM1, ContactType.SIM2, ContactType.SIM3, ContactType.SIM4
                };
                if (mContactCheckTypes[0]) {
                    params.add(ContactType.PHONE);
                }
                if (mSimCount >= 1) {
                    for (int i = 1; i <= mSimCount; i++) {
                        if (mContactCheckTypes[i]) {
                            params.add(mSimInfoList.get(i - 1).mDisplayName);
                            MyLogger.logD(CLASS_TAG, " mDisplayName is "
                                    + mSimInfoList.get(i - 1).mDisplayName);
                        }
                    }
                }
                mBackupService.setBackupItemParam(ModuleType.TYPE_CONTACT, params);
            }
            if (list.contains(ModuleType.TYPE_MESSAGE)) {
                ArrayList<String> params = new ArrayList<String>();
                if (mMessageCheckTypes[0]) {
                    params.add(Constants.ModulePath.NAME_SMS);
                }
                if (mMessageCheckTypes[1]) {
                    params.add(Constants.ModulePath.NAME_MMS);
                }
                mBackupService.setBackupItemParam(ModuleType.TYPE_MESSAGE, params);
            }
            boolean ret = mBackupService.startBackup(folderName);
            if (ret) {
                showProgress();
            } else {
                String path = SDCardUtils.getStoragePath(this);
                if (path == null) {
                    // no sdcard
                    Log.d(CLASS_TAG, "SDCard is removed");
                    ret = true;
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            showDialog(DialogID.DLG_SDCARD_REMOVED);
                        }
                    });
                } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
                    // no space
                    Log.d(CLASS_TAG, "SDCard is full");
                    ret = true;
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            showDialog(DialogID.DLG_SDCARD_FULL);
                        }
                    });
                } else {
                    Log.e(CLASS_TAG, "unkown error");
                    Bundle b = new Bundle();
                    b.putString("name", folderName.substring(folderName.lastIndexOf('/') + 1));
                    showDialog(DialogID.DLG_CREATE_FOLDER_FAILED, b);
                }
                stopService();
            }
        } else {
            stopService();
            MyLogger.logE(CLASS_TAG, "startPersonalDataBackup: error! service is null");
        }
    }

    @Override
    public void startBackup() {
        Log.v(CLASS_TAG, "startBackup");

        if (isSimCardSelected()) {
            if (isAirModeOn(this)) {
                showAirModeOnDialog();
                return;
            }
            if (!checkSimLocked(null)) {
                showDialog(DialogID.DLG_EDIT_FOLDER_NAME);
                return;
            }
        } else {
            showDialog(DialogID.DLG_EDIT_FOLDER_NAME);
        }
    }

    private boolean isSimCardSelected() {
        getSimInfoList();
        if (mSimCount >= 1) {
            for (int i = 1; i <= mSimCount; i++) {
                if (mContactCheckTypes[i]) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isSimCardSelected(int slot) {
        MyLogger.logD(CLASS_TAG, "[isSimCardSelected] slot = " + slot);
        getSimInfoList();
        if (mSimCount >= 1) {
            for (SimInfoRecord simInfo : mSimInfoList) {
                if (simInfo.mSimSlotId == slot) {
                    int index = mSimInfoList.indexOf(simInfo) + 1;
                    if (index > 0 && index < mContactCheckTypes.length) {
                        MyLogger.logD(CLASS_TAG, "[isSimCardSelected] slot = " + slot
                                + (mContactCheckTypes[index] ? " selected" : "not been selected"));
                        return mContactCheckTypes[index];
                    }
                }
            }
            /*
             * for (int i = 0; i < mSimCount; i++) { if (mContactCheckTypes[i]
             * && (slot == i)) { MyLogger.logD(CLASS_TAG,
             * "[isSimCardSelected] slot = " +slot + " selected"); return true;
             * }else{ MyLogger.logD(CLASS_TAG, "[isSimCardSelected] slot = "
             * +slot + "not been selected"); } }
             */
        }
        return false;
    }

    private void showAirModeOnDialog() {
        // TODO Auto-generated method stub
        AlertDialog airModeDialog = new AlertDialog.Builder(this)
                .setTitle(R.string.air_mode_dialog_title)
                .setCancelable(true)
                .setMessage(R.string.air_mode_dialog_message)
                .setPositiveButton(android.R.string.ok, null).create();
        /**
         * On KK version, the 3 part apps don't have permission to change
         * AirplaneMode and open SIM card operation. so when click the [OK],it
         * only show some message and do nothing. so remove the [cancel] button
         * and only remain the [OK] button
         * .setPositiveButton(android.R.string.ok, new
         * DialogInterface.OnClickListener() {
         * 
         * @Override public void onClick(DialogInterface dialog, int which) {
         *           //AIR MODE OFF
         *           setAirplaneMode(PersonalDataBackupActivity.this,true); } })
         *           .setNegativeButton(android.R.string.cancel, new
         *           DialogInterface.OnClickListener() {
         * @Override public void onClick(DialogInterface dialog, int which) {
         *           //continue backup
         *           showDialog(DialogID.DLG_EDIT_FOLDER_NAME); } }).create();
         */
        airModeDialog.show();
        dialogs.add(airModeDialog);
    }

    private boolean checkSimLocked(SimInfoRecord simInfo) {
        // TODO Auto-generated method stub
        if (simInfo != null) {
            showSIMLockedDialog(simInfo);
            return true;
        } else {
            if (mSimInfoList != null) {
                MyLogger.logI(CLASS_TAG, "[mSimInfoList]===>mSimInfoList size = "
                        + mSimInfoList.size());
                for (SimInfoRecord tsimInfo : mSimInfoList) {
                    int simId = tsimInfo.mSimSlotId;
                    if (isSIMLocked(simId) && isSimCardSelected(tsimInfo.mSimSlotId)) {
                        MyLogger.logD(CLASS_TAG, "[checkSimLocked]===> isSIMLocked " + simId);
                        checkSimLocked(tsimInfo);
                        simInfo = null;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private boolean isAirModeOn(Context context) {
        // TODO Auto-generated method stub
        int result = 0;
        try {
            result = Settings.Global.getInt(getContentResolver(), Settings.Global.AIRPLANE_MODE_ON);
        } catch (SettingNotFoundException e) {
            // TODO: handle exception
        }
        return result == 1;
    }

    private void showSIMLockedDialog(final SimInfoRecord simInfo) {
        // TODO Auto-generated method stub
        AlertDialog alertDialog = new AlertDialog.Builder(this)
                .setTitle(R.string.sim_locked_dialog_title)
                .setCancelable(true)
                .setMessage(R.string.sim_locked_dialog_message)
                .setPositiveButton(R.string.sim_locked_dialog_unlock, null).create();

        /**
         * On KK version, the 3 part apps don't have permission to change
         * AirplaneMode and open SIM card operation. so when click the [OK],it
         * only show some message and do nothing. so remove the [cancel] button
         * and only remain the [OK] button
         */
        /*
         * .setPositiveButton(R.string.sim_locked_dialog_unlock, new
         * DialogInterface.OnClickListener() {
         * @Override public void onClick(DialogInterface dialog, int which) { //
         * * startUnlockSim(simInfo.mSimSlotId); } }).create();
         * setNegativeButton(android.R.string.cancel, new
         * DialogInterface.OnClickListener() {
         * @Override public void onClick(DialogInterface dialog, int which) {
         * SimInfoRecord nextSim = getNextSimCard(simInfo);
         * MyLogger.logD(CLASS_TAG, "simInfo = "+ simInfo.mSimInfoId);
         * if(nextSim!=null){ checkSimLocked(nextSim); }else{
         * showDialog(DialogID.DLG_EDIT_FOLDER_NAME); } } }).create();
         */
        alertDialog.show();
        dialogs.add(alertDialog);
    }

    /**
     * Only KK branch don't have the permission to do this. so Remain the code
     * here
     * 
     * @param slot
     */
    private void startUnlockSim(int slot) {
        Intent it = new Intent();
        // it.setAction("com.android.phone.SetupUnlockPINLock");
        // it.putExtra("PhoneConstants.GEMINI_SIM_ID_KEY", slot);
        // it.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
        // Intent.FLAG_ACTIVITY_CLEAR_TASK);
        // startActivity(it);
        MyLogger.logD(CLASS_TAG, "[startUnlockSim] slot = " + slot);
        it.setAction("com.android.phone.ACTION_UNLOCK_SIM_LOCK");
        it.putExtra("com.android.phone.EXTRA_SIM_SLOT", slot);
        it.putExtra("com.android.phone.EXTRA_UNLOCK_TYPE", 501);
        sendBroadcast(it);
    }

    public void setAirplaneMode(Context context, boolean enabling) {
        Settings.Global.putInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0);
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intent.putExtra("state", false);
        context.sendBroadcast(intent);
    }

    private SimInfoRecord getNextSimCard(SimInfoRecord info) {
        if (info == null) {
            return null;
        }
        // getSimInfoList();
        int index = getSimIndex(info);
        MyLogger.logD(CLASS_TAG, "[getNextSimCard] current SIM card index is " + index);
        if ((index + 1) < mSimInfoList.size()) {
            SimInfoRecord nextSIM = mSimInfoList.get(index + 1);
            MyLogger.logD(CLASS_TAG, "[getNextSimCard] current SIM card is " + info.mDisplayName
                    + " Next SIM card is " + nextSIM.mDisplayName);
            return isSimCardSelected(nextSIM.mSimSlotId)
                    && isSIMLocked(nextSIM.mSimSlotId) ? nextSIM : null;
        }

        return null;

    }

    private int getSimIndex(SimInfoRecord info) {
        int index = -1;
        for (SimInfoRecord siminfo : mSimInfoList) {
            if (siminfo.mSimInfoId == info.mSimInfoId) {
                index = mSimInfoList.indexOf(siminfo);
            }
        }
        return index;
    }

    private boolean isSIMLocked(int slot) {
        TelephonyManagerEx mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        int statue = mTelephonyManagerEx.getSimState(slot);
        MyLogger.logD(CLASS_TAG, "SIM card slot " + slot + " It's statue = " + statue);
        // Toast.makeText(getApplicationContext(), "FUCK!!!", 0).show();
        return statue == android.telephony.TelephonyManager.SIM_STATE_PIN_REQUIRED;
    }

    protected void afterServiceConnected() {
        mBackupListener = new PersonalDataBackupStatusListener();
        setOnBackupStatusListener(mBackupListener);
        checkBackupState();
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

    public BaseAdapter initBackupAdapter() {
        mBackupListAdapter = new PersonalDataBackupAdapter(this, mBackupItemDataList,
                R.layout.backup_personal_data_item);
        return mBackupListAdapter;
    }

    @Override
    public void onCheckedCountChanged() {
        super.onCheckedCountChanged();
        updateTitle();
        // MyLogger.logE(CLASS_TAG, "-----------------onCheckedCountChanged");
    }

    private void showBackupResult(final BackupResultType result, final ArrayList<ResultEntity> list) {

        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }

        if (mCancelDlg != null && mCancelDlg.isShowing()) {
            mCancelDlg.dismiss();
        }

        if (result != BackupResultType.Cancel) {
            Bundle args = new Bundle();
            args.putParcelableArrayList(Constants.RESULT_KEY, list);
            showDialog(DialogID.DLG_RESULT, args);
        } else {
            stopService();
        }
    }

    private int getContactTypeNumber() {
        int count = (mSimCount + 1) < mContactCheckTypes.length ? (mSimCount + 1)
                : mContactCheckTypes.length;
        return count;
    }

    private boolean isAllValued(boolean[] array, int count, boolean value) {
        boolean ret = true;
        for (int position = 0; position < count; position++) {
            if (array[position] != value) {
                ret = false;
                break;
            }
        }
        return ret;
    }

    private void showMessageConfigDialog() {
        for (String string : messageEnable) {
            MyLogger.logE(CLASS_TAG, "messageEnable = " + string);
        }
        final String[] select = (String[]) messageEnable.toArray(new String[messageEnable.size()]);
        final boolean[] temp = new boolean[2];
        for (int index = 0; index < 2; index++) {
            temp[index] = mMessageCheckTypes[index];
        }
        AlertDialog alertDialog = new AlertDialog.Builder(this)
                .setTitle(R.string.message_module)
                .setCancelable(true)
                .setMultiChoiceItems(select, temp,
                        new DialogInterface.OnMultiChoiceClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which, boolean isChecked) {
                                MyLogger.logD(CLASS_TAG, "DialogID.DLG_CONTACT_CONFIG: the number "
                                        + which + " is checked(" + isChecked + ")");
                                AlertDialog d = (AlertDialog) dialog;
                                int count = select.length;

                                if (isAllValued(temp, count, false)) {
                                    d.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(false);
                                } else {
                                    d.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(true);
                                }
                            }
                        })
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        for (int index = 0; index < 2; index++) {
                            mMessageCheckTypes[index] = temp[index];
                        }
                    }
                }).setNegativeButton(android.R.string.cancel, null).create();
        alertDialog.show();
    }

    private void showContactConfigDialog() {
        final String[] select;
        final boolean[] temp = new boolean[mSimCount + 1];
        for (int index = 0; index < mSimCount + 1; index++) {
            temp[index] = mContactCheckTypes[index];
        }
        /*
         * if (mSimInfoList != null) { for (SIMInfo simInfo : mSimInfoList) {
         * MyLogger.logD(CLASS_TAG, "sim id  = " + simInfo.mSimId + ", name = "
         * + simInfo.mDisplayName + ", slot = " + simInfo.mSlot); } }
         */
        switch (mSimCount) {
            case 1:
                select = new String[] {
                        getString(R.string.contact_phone),
                        mSimInfoList.get(0).mDisplayName
                };
                break;

            case 2:
                select = new String[] {
                        getString(R.string.contact_phone),
                        mSimInfoList.get(0).mDisplayName, mSimInfoList.get(1).mDisplayName
                };
                break;
            case 3:
                select = new String[] {
                        getString(R.string.contact_phone),
                        mSimInfoList.get(0).mDisplayName, mSimInfoList.get(1).mDisplayName,
                        mSimInfoList.get(2).mDisplayName
                };
                break;
            case 4:
                select = new String[] {
                        getString(R.string.contact_phone),
                        mSimInfoList.get(0).mDisplayName, mSimInfoList.get(1).mDisplayName,
                        mSimInfoList.get(2).mDisplayName,
                        mSimInfoList.get(3).mDisplayName
                };
                break;
            default:
                select = new String[] {
                    getString(R.string.contact_phone)
                };
                break;
        }

        AlertDialog alertDialog = new AlertDialog.Builder(this)
                .setTitle(R.string.contact_module)
                .setCancelable(true)
                .setMultiChoiceItems(select, temp,
                        new DialogInterface.OnMultiChoiceClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which, boolean isChecked) {
                                MyLogger.logD(CLASS_TAG, "DialogID.DLG_CONTACT_CONFIG: the number "
                                        + which + " is checked(" + isChecked + ")");
                                AlertDialog d = (AlertDialog) dialog;
                                int count = mSimCount + 1;

                                if (isAllValued(temp, count, false)) {
                                    d.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(false);
                                } else {
                                    d.getButton(DialogInterface.BUTTON_POSITIVE).setEnabled(true);
                                }
                            }
                        })
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        boolean empty = true;
                        for (int index = 0; index < mSimCount + 1; index++) {
                            mContactCheckTypes[index] = temp[index];
                        }
                        int count = getContactTypeNumber();
                        for (int index = 0; index < count; index++) {
                            if (mContactCheckTypes[index]) {
                                empty = false;
                                break;
                            }
                        }
                        MyLogger.logD(CLASS_TAG, "mContactCheckTypes = "
                                + Arrays.toString(mContactCheckTypes));
                        if (empty) {
                            setItemCheckedByPosition(0, false);
                        } else {
                            setItemCheckedByPosition(0, true);
                        }
                    }
                }).setNegativeButton(android.R.string.cancel, null).create();
        alertDialog.show();
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;
        switch (id) {
            // input backup file name
            case DialogID.DLG_EDIT_FOLDER_NAME:
                dialog = createFolderEditorDialog();
                break;

            case DialogID.DLG_RESULT:
                final DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(final DialogInterface dialog, final int which) {
                        stopService();
                        NotifyManager.getInstance(PersonalDataBackupActivity.this)
                                .clearNotification();
                    }
                };
                dialog = ResultDialog.createResultDlg(this, R.string.backup_result, args, listener);
                break;
            case DialogID.DLG_BACKUP_CONFIRM_OVERWRITE:
                dialog = new AlertDialog.Builder(this).setIcon(android.R.drawable.ic_dialog_alert)
                        .setTitle(R.string.notice).setMessage(
                                R.string.backup_confirm_overwrite_notice)
                        .setNegativeButton(android.R.string.cancel, null)
                        .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                MyLogger.logI(CLASS_TAG, " to backup");
                                File folder = new File(mBackupFolderPath);
                                File[] files = folder.listFiles();
                                if (files != null && files.length > 0) {
                                    DeleteFolderTask task = new DeleteFolderTask();
                                    task.execute(files);
                                } else {
                                    startPersonalDataBackup(mBackupFolderPath);
                                }
                            }
                        })
                        // .setCancelable(false)
                        .create();
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

            case DialogID.DLG_EDIT_FOLDER_NAME:
                EditText editor = (EditText) dialog.findViewById(R.id.edit_folder_name);
                if (editor != null) {
                    SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
                    String dateString = dateFormat.format(new Date(System.currentTimeMillis()));
                    editor.setText(dateString);
                }
                break;
            default:
                super.onPrepareDialog(id, dialog, args);
                break;
        }
    }

    private AlertDialog createFolderEditorDialog() {

        LayoutInflater factory = LayoutInflater.from(this);
        final View view = factory.inflate(R.layout.dialog_edit_folder_name, null);
        EditText editor = (EditText) view.findViewById(R.id.edit_folder_name);
        final AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle(R.string.edit_folder_name).setView(view)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        AlertDialog d = (AlertDialog) dialog;
                        EditText editor = (EditText) d.findViewById(R.id.edit_folder_name);
                        if (editor != null) {
                            CharSequence folderName = editor.getText();
                            String path = SDCardUtils
                                    .getPersonalDataBackupPath(PersonalDataBackupActivity.this);
                            if (path == null) {
                                showDialog(DialogID.DLG_NO_SDCARD);
                                return;
                            }
                            StringBuilder builder = new StringBuilder(path);
                            builder.append(File.separator);
                            builder.append(folderName);
                            mBackupFolderPath = builder.toString();
                            hideKeyboard(editor);
                            editor.setText("");
                            File folder = new File(mBackupFolderPath);
                            File[] files = null;
                            if (folder.exists()) {
                                files = folder.listFiles();
                            }

                            if (files != null && files.length > 0) {
                                showDialog(DialogID.DLG_BACKUP_CONFIRM_OVERWRITE);
                            } else {
                                startPersonalDataBackup(mBackupFolderPath);
                            }
                        } else {
                            MyLogger.logE(CLASS_TAG, " can not get folder name");
                        }
                    }

                    private void hideKeyboard(EditText editor) {
                        // TODO Auto-generated method stub
                        InputMethodManager imm = ((InputMethodManager) getSystemService(INPUT_METHOD_SERVICE));
                        imm.hideSoftInputFromWindow(editor.getWindowToken(), 0);
                    }
                }).setNegativeButton(android.R.string.cancel, null).create();
        editor.addTextChangedListener(new TextWatcher() {

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                if (s.toString().length() <= 0 || s.toString().matches(".*[/\\\\:#*?\"<>|].*")
                        || s.toString().matches(" *\\.+ *") || s.toString().trim().length() == 0) { // characters
                    // not allowed
                    dialog.getButton(Dialog.BUTTON_POSITIVE).setEnabled(false);
                } else {
                    dialog.getButton(Dialog.BUTTON_POSITIVE).setEnabled(true);
                }
            }
        });
        dialogs.add(dialog);
        return dialog;
    }

    protected String getProgressDlgMessage(final int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.backuping));
        builder.append("(");
        builder.append(ModuleType.getModuleStringFromType(this, type));
        builder.append(")");
        return builder.toString();
    }

    @Override
    protected void checkBackupState() {
        if (mBackupService != null) {
            int state = mBackupService.getState();
            switch (state) {
                case State.RUNNING:
                    /* fall through */
                case State.PAUSE:
                    BackupProgress p = mBackupService.getCurBackupProgress();
                    Log.e(CLASS_TAG, "checkBackupState: Max = " + p.mMax + " curprogress = "
                            + p.mCurNum);
                    if (state == State.RUNNING) {
                        mProgressDialog.show();
                    }
                    if (p.mCurNum < p.mMax) {
                        String msg = getProgressDlgMessage(p.mType);
                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                        }
                    }
                    if (mProgressDialog != null) {
                        mProgressDialog.setMax(p.mMax);
                        mProgressDialog.setProgress(p.mCurNum);
                    }
                    break;
                case State.FINISH:
                    showBackupResult(mBackupService.getBackupResultType(),
                            mBackupService.getBackupResult());
                    break;
                default:
                    super.checkBackupState();
                    break;
            }
        }
    }

    @Override
    public void onConfigurationChanged(final Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e(CLASS_TAG, "onConfigurationChanged");
    }

    List<SimInfoRecord> mSimInfoList;
    int mSimCount = 0;

    private void getSimInfoList() {
        mSimInfoList = SimInfoManager.getInsertedSimInfoList(PersonalDataBackupActivity.this);
        if (mSimInfoList != null) {
            for (SimInfoRecord simInfo : mSimInfoList) {
                MyLogger.logD(CLASS_TAG, "sim id  = " + simInfo.mSimInfoId + ", name = "
                        + simInfo.mDisplayName + ", slot = " + simInfo.mSimSlotId);
            }
        } else {
            MyLogger.logD(CLASS_TAG, "No SIM inserted!");
        }
        if (mSimInfoList != null) {
            mSimCount = mSimInfoList.isEmpty() ? 0 : mSimInfoList.size();
        }
        // MyLogger.logD(CLASS_TAG, "SIM count = " + mSimCount);
    }

    private void updateData(ArrayList<PersonalItemData> list) {
        mBackupItemDataList = list;
        mBackupListAdapter.changeData(mBackupItemDataList);
        syncUnCheckedItems();
        mBackupListAdapter.notifyDataSetChanged();
        updateTitle();
        updateButtonState();
        checkBackupState();
    }

    private class DeleteFolderTask extends AsyncTask<File[], String, Long> {
        private ProgressDialog mDeletingDialog;

        public DeleteFolderTask() {
            mDeletingDialog = new ProgressDialog(PersonalDataBackupActivity.this);
            mDeletingDialog.setCancelable(false);
            mDeletingDialog.setMessage(getString(R.string.delete_please_wait));
            mDeletingDialog.setIndeterminate(true);
        }

        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            if (mBackupFolderPath != null) {
                startPersonalDataBackup(mBackupFolderPath);
            }

            if (mDeletingDialog != null) {
                mDeletingDialog.dismiss();
            }
        }

        protected void onPreExecute() {
            if (mDeletingDialog != null) {
                mDeletingDialog.show();
            }
        }

        protected Long doInBackground(File[]... params) {
            File[] files = params[0];
            for (File file : files) {
                FileUtils.deleteFileOrFolder(file);
            }

            return null;
        }
    }

    List<String> messageEnable = new ArrayList<String>();

    private class InitPersonalDataTask extends AsyncTask<Void, Void, Long> {
        private static final String TASK_TAG = "InitPersonalDataTask";
        ArrayList<PersonalItemData> mBackupDataList;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            MyLogger.logD(CLASS_TAG, TASK_TAG + "---onPreExecute");
            // show progress and set title as "updating"
            // setProgressBarIndeterminateVisibility(true);
            showLoadingContent(true);
            setTitle(R.string.backup_personal_data);
            setButtonsEnable(false);
        }

        @Override
        protected void onPostExecute(Long arg0) {
            showLoadingContent(false);
            setButtonsEnable(true);
            updateData(mBackupDataList);
            setOnBackupStatusListener(mBackupListener);
            // setProgressBarIndeterminateVisibility(false);
            MyLogger.logD(CLASS_TAG, "---onPostExecute----getTitle"
                    + PersonalDataBackupActivity.this.getTitle());
            super.onPostExecute(arg0);
        }

        @Override
        protected Long doInBackground(Void... arg0) {
            messageEnable.clear();
            mBackupDataList = new ArrayList<PersonalItemData>();
            int types[] = new int[] {
                    ModuleType.TYPE_CONTACT, ModuleType.TYPE_MESSAGE,
                    ModuleType.TYPE_PICTURE, ModuleType.TYPE_CALENDAR, ModuleType.TYPE_MUSIC,
                    ModuleType.TYPE_BOOKMARK
            };
            int num = types.length;
            boolean skipAppend = false;
            for (int i = 0; i < num; i++) {
                boolean bEnabled = true;
                skipAppend = false;
                int count = 0;
                Composer composer;
                switch (types[i]) {
                    case ModuleType.TYPE_CONTACT:
                        count = getModulesCount(new ContactBackupComposer(
                                PersonalDataBackupActivity.this));
                        break;
                    case ModuleType.TYPE_MESSAGE:
                        int countSMS = 0;
                        int countMMS = 0;
                        composer = new SmsBackupComposer(PersonalDataBackupActivity.this);
                        if (composer.init()) {
                            countSMS = composer.getCount();
                            composer.onEnd();
                        }
                        if (countSMS != 0) {
                            messageEnable.add(getString(R.string.message_sms));
                        }
                        composer = new MmsBackupComposer(PersonalDataBackupActivity.this);
                        if (composer.init()) {
                            countMMS = composer.getCount();
                            composer.onEnd();
                        }
                        count = countSMS + countMMS;
                        MyLogger.logE(CLASS_TAG, "countSMS = " + countSMS + "countMMS" + countMMS);
                        if (countMMS != 0) {
                            messageEnable.add(getString(R.string.message_mms));
                        }
                        break;
                    case ModuleType.TYPE_PICTURE:
                        count = getModulesCount(new PictureBackupComposer(
                                PersonalDataBackupActivity.this));
                        if (SDCardUtils.getInternalStorage() == null
                                || SDCardUtils.getInternalStorage().equals("")) {
                            skipAppend = true;
                        }
                        break;
                    case ModuleType.TYPE_CALENDAR:
                        count = getModulesCount(new CalendarBackupComposer(
                                PersonalDataBackupActivity.this));
                        break;
                    case ModuleType.TYPE_APP:
                        count = getModulesCount(new AppBackupComposer(
                                PersonalDataBackupActivity.this));
                        break;
                    case ModuleType.TYPE_MUSIC:
                        count = getModulesCount(new MusicBackupComposer(
                                PersonalDataBackupActivity.this));
                        if (SDCardUtils.getInternalStorage() == null
                                || SDCardUtils.getInternalStorage().equals("")) {
                            skipAppend = true;
                        }
                        break;
                    case ModuleType.TYPE_NOTEBOOK:
                        count = getModulesCount(new NoteBookBackupComposer(
                                PersonalDataBackupActivity.this));
                        break;
                    case ModuleType.TYPE_BOOKMARK:
                        count = getModulesCount(new BookmarkBackupComposer(
                                PersonalDataBackupActivity.this));
                        break;
                    default:
                        break;
                }
                composer = null;
                bEnabled = !(count == 0);
                PersonalItemData item = new PersonalItemData(types[i], bEnabled);
                if (!skipAppend)
                    mBackupDataList.add(item);
            }
            return null;
        }
    }

    private int getModulesCount(Composer... composers) {
        int count = 0;
        for (Composer composer : composers) {
            if (composer.init()) {
                count += composer.getCount();
                composer.onEnd();
            }
        }
        return count;
    }

    private class PersonalDataBackupAdapter extends BaseAdapter {
        private ArrayList<PersonalItemData> mDataList;
        private int mLayoutId;
        private LayoutInflater mInflater;

        public PersonalDataBackupAdapter(Context context, ArrayList<PersonalItemData> list,
                int resource) {
            mDataList = list;
            mLayoutId = resource;
            mInflater = LayoutInflater.from(context);
        }

        public void changeData(ArrayList<PersonalItemData> list) {
            mDataList = list;
        }

        public void reset() {
            mDataList = null;
        }

        @Override
        public int getCount() {
            return mDataList.size();
        }

        @Override
        public Object getItem(final int position) {
            return mDataList.get(position);
        }

        @Override
        public long getItemId(final int position) {
            return mDataList.get(position).getType();
        }

        @Override
        public View getView(final int position, final View convertView, final ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(mLayoutId, parent, false);
            }

            final PersonalItemData item = mDataList.get(position);
            final View content = view.findViewById(R.id.item_content);
            final View config = view.findViewById(R.id.item_config);
            final ImageView imgView = (ImageView) view.findViewById(R.id.item_image);
            final TextView textView = (TextView) view.findViewById(R.id.item_text);
            final CheckBox chxbox = (CheckBox) view.findViewById(R.id.item_checkbox);

            boolean bEnabled = item.isEnable();
            imgView.setEnabled(bEnabled);
            textView.setEnabled(bEnabled);
            content.setAlpha(bEnabled ? ENABLE_ALPHA : DISABLE_ALPHA);
            chxbox.setEnabled(bEnabled);
            if (item.getType() == ModuleType.TYPE_CONTACT) {
                boolean isChecked = isItemCheckedByPosition(position);
                // MyLogger.logD(CLASS_TAG, "contact config: positon + " +
                // position + " is checked: "
                // + isChecked);

                float alpha = isChecked ? ENABLE_ALPHA : DISABLE_ALPHA;
                config.setEnabled(isChecked);
                config.setAlpha(alpha);
                config.setVisibility(View.VISIBLE);
                config.setOnClickListener(new OnClickListener() {
                    public void onClick(View v) {
                        // contact config click
                        getSimInfoList();
                        showContactConfigDialog();
                    }
                });
            } else if (item.getType() == ModuleType.TYPE_MESSAGE) {
                boolean isChecked = isItemCheckedByPosition(position);
                // MyLogger.logD(CLASS_TAG, "message config: positon + " +
                // position + " is checked: "
                // + isChecked);

                float alpha = isChecked ? ENABLE_ALPHA : DISABLE_ALPHA;
                config.setEnabled(isChecked);
                config.setAlpha(alpha);
                config.setVisibility(View.VISIBLE);
                config.setOnClickListener(new OnClickListener() {
                    public void onClick(View v) {
                        // contact config click
                        showMessageConfigDialog();
                    }
                });
            } else {
                config.setVisibility(View.GONE);
                config.setOnClickListener(null);
            }

            content.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    if (chxbox.isEnabled()) {
                        revertItemCheckedByPosition(position);
                    }
                }
            });

            if (!bEnabled) {
                chxbox.setChecked(false);
            }
            long id = getItemId(position);
            setItemDisabledById(id, !bEnabled);
            imgView.setImageResource(item.getIconId());
            textView.setText(item.getTextId());
            if (isItemCheckedByPosition(position)) {
                if (chxbox.isEnabled()) {
                    chxbox.setChecked(true);
                }
            } else {
                if (chxbox.isEnabled()) {
                    chxbox.setChecked(false);
                }
            }

            return view;
        }
    }

    public class PersonalDataBackupStatusListener extends NomalBackupStatusListener {
        @Override
        public void onBackupEnd(final BackupResultType resultCode,
                final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {

            RecordXmlInfo backupInfo = new RecordXmlInfo();
            backupInfo.setRestore(false);
            backupInfo.setDevice(Utils.getPhoneSearialNumber());
            backupInfo.setTime(String.valueOf(System.currentTimeMillis()));
            RecordXmlComposer xmlCompopser = new RecordXmlComposer();
            xmlCompopser.startCompose();
            xmlCompopser.addOneRecord(backupInfo);
            xmlCompopser.endCompose();
            if (mBackupFolderPath != null && !mBackupFolderPath.isEmpty()) {
                Utils.writeToFile(xmlCompopser.getXmlInfo(), mBackupFolderPath + File.separator
                        + Constants.RECORD_XML);
            }
            final BackupResultType iResultCode = resultCode;
            final ArrayList<ResultEntity> iResultRecord = resultRecord;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        showBackupResult(iResultCode, iResultRecord);
                    }
                });
            }
        }

        @Override
        public void onComposerChanged(final Composer composer) {
            if (composer == null) {
                MyLogger.logE(CLASS_TAG, "onComposerChanged: error[composer is null]");
                return;
            } else {
                MyLogger.logI(CLASS_TAG, "onComposerChanged: type = " + composer.getModuleType()
                        + "Max = " + composer.getCount());
            }
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        String msg = getProgressDlgMessage(composer.getModuleType());
                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                            mProgressDialog.setMax(composer.getCount());
                            mProgressDialog.setProgress(0);
                        }
                    }
                });
            }
        }
    }

}

