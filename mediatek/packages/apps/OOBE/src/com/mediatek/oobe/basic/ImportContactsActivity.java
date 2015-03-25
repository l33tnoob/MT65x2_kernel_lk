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

package com.mediatek.oobe.basic;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;

import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.CellConnService.CellConnMgr;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.oobe.ext.IOobeMiscExt;
import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.OOBEStepPreferenceActivity;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;

public class ImportContactsActivity extends OOBEStepPreferenceActivity 
            implements Button.OnClickListener, ServiceConnection {
    private static final String TAG = "OOBE.contacts";

    public static final int ID_COPYING = 0;
    public static final int ID_COPY_ONE_END = 1;
    public static final int ID_COPY_CANCEL = 2;
    public static final int ID_COPY_ALL_END = 3;

    private Button mImportBtn;
    private TextView mTextCopyNote;
    private boolean mImporting;

    private ITelephony mITelephony;
    private ITelephonyEx mITelephonyEx;
    private Handler mCopyHandler;
    private CellConnMgr mCellMgr;
    private ImportContactsService mService;
    private SimInfoPreference mPreference;
    private HashMap<Long, SimInfoPreference> mSimInfoPreMap;
    private IOobeMiscExt mExt;

    BroadcastReceiver mSimStateListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction(); 
            Xlog.d(TAG, "receive action:" + action);
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)
                || TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED.equals(action)) {
                updateSimPreference();
            } else if (TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)) {
                updateSimInfo();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.import_contacts_layout);
        addPreferencesFromResource(R.xml.oobe_preference_import_contacts);

        mITelephony = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
        mITelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
        mCopyHandler = new CopyHandler();
        mCellMgr = new CellConnMgr( new Runnable() {
                @Override
                public void run() {
                }
            });
        mCellMgr.register(this);
        mSimInfoPreMap = new HashMap<Long, SimInfoPreference>();
        mImporting = false;

        mExt = Utils.getOobeMiscPlugin(this);
        setLayout();

        //bind imoport contacts service
        Intent intent = new Intent("com.mediatek.oobe.basic.ImportContactsService");
        bindService(intent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onResume() {
        super.onResume();
        Xlog.d(TAG, "onResume");

        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        registerReceiver(mSimStateListener, intentFilter);
    }

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mSimStateListener);
    }

    @Override
    public void onDestroy() {
        mCellMgr.unregister();
        if (mService != null) {
            mService.setHandler(null);
        }
        unbindService(this);
        super.onDestroy();
    }

    /*
    * init layout of activity
    */
    private void setLayout() {
        initLayout();
        TextView title = (TextView) findViewById(R.id.settings_title);
        title.setText(mExt.replaceSimToSimUim(getString(R.string.oobe_title_import_contacts)));
        mImportBtn = (Button) findViewById(R.id.button_import_contacts);
        mImportBtn.setOnClickListener(this);
        mTextCopyNote = (TextView) findViewById(R.id.textView_note_import_contacts);
        mNextBtn.setText(R.string.oobe_btn_text_skip);
        addSimInfoPreference();
    }

    /*
    * add sim info preference to the screen
    */
    private void addSimInfoPreference() {
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(this);
        mImportBtn.setVisibility(simList.size() > 0 ? View.VISIBLE : View.INVISIBLE);

        PreferenceScreen screen = getPreferenceScreen();
        if (screen == null || simList.size() == 0) {
            Xlog.d(TAG,"do not add preference");
            return;
        }

        screen.removeAll();
        mSimInfoPreMap.clear();

        Collections.sort(simList, new Utils.SimInfoComparable());
        for (SimInfoRecord siminfo : simList) {
            if (siminfo == null) {
                continue;
            }
            int status = -1;
            try {
                if (PhoneConstants.GEMINI_SIM_NUM == 1) {
                    status = mITelephony.getSimIndicatorState();
                } else {
                    status = mITelephonyEx.getSimIndicatorState(siminfo.mSimSlotId);
                }
            } catch (RemoteException e) {
                Xlog.e(TAG, "RemoteException " + e);
            }
            SimInfoPreference simInfoPref = new SimInfoPreference(this, siminfo, status);
            screen.addPreference(simInfoPref);
            mSimInfoPreMap.put(siminfo.mSimInfoId, simInfoPref);
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mImportBtn) {
            if (mService == null) {
                Xlog.e(TAG, "mService is null");
                return;
            }
            String importText = getString(R.string.oobe_btn_start_import_contacts);
            if (importText.equals(mImportBtn.getText())) {
                startImport();
            } else {
                mImportBtn.setEnabled(false);
                //cancel import contacts
                mService.handleCancelRequest();
            }
        } else {
            super.onClick(v);
        }
    }

    private void startImport() {
        // whether the checked sim card is locked
        SimInfoPreference p;
        boolean isChecked = false;
        for (Long id : mSimInfoPreMap.keySet()) {
            p = mSimInfoPreMap.get(id);
            if (p != null && p.isChecked()) {
                if (isSimLocked(p.getSlotIndex())) {
                    return;
                }
                isChecked = true;
            }
        }
        if (!isChecked) {
            return;
        }
        mImportBtn.setEnabled(false);
        mService.setSimInfoList(mSimInfoPreMap);
        //start to import contacts from sim card
        Intent intent = new Intent(this, ImportContactsService.class);
        startService(intent);
    }

    public void onServiceConnected(final ComponentName name, final IBinder service) {
        Xlog.d(TAG,"onServiceConnected()");
        mService = ((ImportContactsService.MyBinder) service).getService();
        mService.setHandler(mCopyHandler);
    }
    
    public void onServiceDisconnected(final ComponentName name) {
        Xlog.d(TAG,"onServiceDisconnected()");
    }

    private class CopyHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            final int msgId = msg.what;
            switch (msgId) {
            case ID_COPYING:
                mPreference = getPreferenceBySimId(-1);
                if (mPreference != null) {
                    mPreference.initProgressBar(msg.arg2);
                    mPreference.updateProgressBar(msg.arg1);
                }
                if (!mImporting) {
                    mImporting = true;
                    mImportBtn.setEnabled(true);
                    mImportBtn.setText(R.string.oobe_btn_cancel_import_contacts);
                    mTextCopyNote.setVisibility(View.VISIBLE);
                    mTextCopyNote.setText(R.string.oobe_note_import_contacts_going);
                    mNextBtn.setText(R.string.oobe_btn_text_next);
                }
                break;
            case ID_COPY_ONE_END:
                Xlog.d(TAG, "ID_COPY_ONE_END");
                mPreference = getPreferenceBySimId(msg.arg2);
                if (mPreference != null) {
                    mPreference.finishProgressBar();
                }
                break;
            case ID_COPY_CANCEL:
                Xlog.d(TAG, "ID_CANCEL_COPY");
                mImportBtn.setText(R.string.oobe_btn_start_import_contacts);
                mNextBtn.setText(R.string.oobe_btn_text_skip);
                mTextCopyNote.setVisibility(View.VISIBLE);
                mTextCopyNote.setText(R.string.oobe_note_import_contacts_cancel);
                mPreference = getPreferenceBySimId(msg.arg2);
                if (mPreference != null) {
                    mPreference.dealWithCancel();
                }
                mImportBtn.setEnabled(true);
                mImporting = false;
                break;
            case ID_COPY_ALL_END :
                Xlog.d(TAG, "ID_COPY_ALL_END");
                if (isAllFinished()) {
                    mImportBtn.setVisibility(View.INVISIBLE);
                } else {
                    mImportBtn.setText(R.string.oobe_btn_start_import_contacts);
                }
                mImportBtn.setEnabled(true);
                mImporting = false;
                mTextCopyNote.setVisibility(View.VISIBLE);
                mTextCopyNote.setText(R.string.oobe_note_import_contacts_finish);
                mNextBtn.setText(R.string.oobe_btn_text_next);
                break;
            default:
                break;
            }
        }
    }

    /*
    * Get preference by simId
    * @param preference whether sim id of preference equals the sim id in  service
    */
    private boolean isAllFinished() {
        SimInfoPreference p;
        for (Long id : mSimInfoPreMap.keySet()) {
            p = mSimInfoPreMap.get(id);
            if (p != null && !p.isFinishImporting()) {
                return false;
            }
        }
        return true;
    }

    /*
    * Get preference by simId
    * @param preference whether sim id of preference equals the sim id in  service
    */
    private SimInfoPreference getPreferenceBySimId(int id) {
        long simId = (long)id;
        if (id == -1 && mService != null) {
            simId = mService.getSimId();
        }
        return mSimInfoPreMap.get(simId);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference instanceof SimInfoPreference) {
            String importText = getString(R.string.oobe_btn_start_import_contacts);
            if (importText.equals(mImportBtn.getText())) {
                SimInfoPreference simPreference = (SimInfoPreference)preference;
                simPreference.setCheck(!simPreference.isChecked());
            }
        }
        return true;
    }

    /*
    * update indicator state of sim card in the screen
    */
    private void updateSimPreference() {
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(this);
        SimInfoPreference p;
        if (simList != null && mSimInfoPreMap.size() > 0) {
            for (SimInfoRecord simInfo : simList) {
                p = mSimInfoPreMap.get(simInfo.mSimInfoId);
                if (simInfo.mSimInfoId == p.getSimId()) {
                    p.setStatus(getSimState(simInfo.mSimSlotId));
                }
            }
        }

    }

    /*
    * update sim preference for sim hotswap
    */
    private void updateSimInfo() {
        HashMap<Long, SimInfoPreference> tmpMap = new HashMap<Long, SimInfoPreference>();
        tmpMap.putAll(mSimInfoPreMap);
        mSimInfoPreMap.clear();
        PreferenceScreen screen = getPreferenceScreen();
        screen.removeAll();
        
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(this);
        if (simList != null && tmpMap.size() > 0) {
            for (SimInfoRecord simInfo : simList) {
                SimInfoPreference p = tmpMap.get(simInfo.mSimInfoId);
                if (p != null) {
                    screen.addPreference(p);
                    mSimInfoPreMap.put(simInfo.mSimInfoId, p);
                }
            }
        }
    }
    /*
    * get indicator state of the sim card by slot
    * @param slot the slot of sim card
    */
    private int getSimState(int slot) {
        int status = -1;
        try {
            if (PhoneConstants.GEMINI_SIM_NUM > 1) {
                status = mITelephonyEx.getSimIndicatorState(slot);
            } else {
                status = mITelephony.getSimIndicatorState();
            }
        } catch (RemoteException exception) {
            Xlog.e(TAG, "RemoteException " + exception.getMessage());
        }
        Xlog.d(TAG, "getSimState: " + status);
        return status;
    }

    /*
    * get indicator state of the sim card by slot
    * @param slot the slot of sim card
    */
    private boolean isSimLocked(int slot) {
        boolean locked = false;
        int simState = getSimState(slot);
        Xlog.d(TAG, "isSimLocked(), sim state: " + simState);
        if (simState == PhoneConstants.SIM_INDICATOR_LOCKED
                || simState == PhoneConstants.SIM_INDICATOR_RADIOOFF) {
            mCellMgr.handleCellConn(slot, CellConnMgr.REQUEST_TYPE_SIMLOCK);
            locked = true;
        }
        return locked;
    }

}
