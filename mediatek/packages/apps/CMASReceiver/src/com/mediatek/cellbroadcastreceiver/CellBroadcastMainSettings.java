/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.mediatek.cellbroadcastreceiver;



import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import com.android.internal.telephony.TelephonyIntents;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.provider.Telephony.SIMInfo;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import com.mediatek.cellbroadcastreceiver.CheckBoxAndSettingsPreference.OnSettingChangedListener;
import java.util.List;
/**
 * Settings activity for the cell broadcast receiver.
 */
public class CellBroadcastMainSettings extends PreferenceActivity 
                               implements OnSettingChangedListener{
    private static final String TAG = "[CMAS]CellBroadcastMainSettings";

    public static int sSlotId;
    public static int sReadySlotId = -1;

    private CheckBoxAndSettingsPreference mEnableCBCheckBox;
    private CheckBoxAndSettingsPreference mImminentCheckBox;
    private CheckBoxAndSettingsPreference mAmberCheckBox;
    private CheckBoxAndSettingsPreference mSpeechCheckBox;
    private CheckBoxAndSettingsPreference mEnableAllCheckBox;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.preferences_main);

        mEnableCBCheckBox = (CheckBoxAndSettingsPreference) 
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST);
        mEnableCBCheckBox.setOnSettingChangedListener(this);
        mImminentCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_IMMINENT_ALERTS);
        mAmberCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS);
        mSpeechCheckBox = (CheckBoxAndSettingsPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_ALERT_SPEECH);
        mEnableAllCheckBox = (CheckBoxAndSettingsPreference)
        findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_ALL_ALERT);
        mEnableAllCheckBox.setOnSettingChangedListener(this);
        refreshEnableCheckBox();
        IntentFilter intentFilter = new IntentFilter(CellBroadcastConfigService.ACTION_SIM_STATE_CHANGED);
        registerReceiver(mSimStateChangeReceiver, intentFilter);
    }

    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mSimStateChangeReceiver);
    }

    void refreshEnableCheckBox() {
        boolean canReceiveMessage = false;
        List<SimInfoRecord> listSimInfo = SimInfoManager.getInsertedSimInfoList(this);
        if (listSimInfo == null || (listSimInfo != null && listSimInfo.isEmpty())) {
            Log.d(TAG, "there is no sim card");
            canReceiveMessage = false;
        }
        for (SimInfoRecord simInfo : listSimInfo) {
            // check whether has a SIM that can receive CMAS
            if (TelephonyManagerEx.getDefault().getSimState(simInfo.mSimSlotId) == TelephonyManager.SIM_STATE_READY) {
                canReceiveMessage = true;
            }
        }
        Log.d(TAG, "refreshEnableCheckBox canReceiveMessage " + canReceiveMessage);
        if(canReceiveMessage) {
            mEnableCBCheckBox.setEnabled(true);
            setCheckBoxPreferenceEnable(mEnableCBCheckBox.isChecked());
        } else {
            mEnableCBCheckBox.setEnabled(false);
            setCheckBoxPreferenceEnable(false);
        }
    }

    public void setCheckBoxPreferenceEnable(boolean enabled) {
        mImminentCheckBox.setEnabled(enabled && (!mEnableAllCheckBox.isChecked()));
        mAmberCheckBox.setEnabled(enabled && (!mEnableAllCheckBox.isChecked()));
        mSpeechCheckBox.setEnabled(enabled);
        mEnableAllCheckBox.setEnabled(enabled);
    }

    @Override
    public void onEnableCBChanged() {
        Log.d(TAG, "onEnableCBChanged ");
        if(mEnableCBCheckBox.isChecked()){
            Log.d(TAG, "onEnableCBChanged true ");
            setCheckBoxPreferenceEnable(true);
        } else{
            Log.d(TAG, "onEnableCBChanged false");
            setCheckBoxPreferenceEnable(false);
        }
    }
    
    @Override
    public void onEnableAllChanged() {
        Log.d(TAG, "onEnableAllChanged ");
        if (mEnableAllCheckBox.isChecked()) {
            Log.d(TAG, "onEnableAllChanged true ");
            setAlertPreferenceEnable(true);
        } else {
            Log.d(TAG, "onEnableAllChanged false");
            setAlertPreferenceChecked(true);
            setAlertPreferenceEnable(false);
        }
    }

    public void setAlertPreferenceEnable(boolean enabled) {
        mImminentCheckBox.setEnabled(enabled);
        mAmberCheckBox.setEnabled(enabled);
    }

    private void setAlertPreferenceChecked(boolean checked) {
        mImminentCheckBox.setChecked(checked);
        mAmberCheckBox.setChecked(checked);
    }

    private BroadcastReceiver mSimStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            refreshEnableCheckBox();
        }
    };
}
