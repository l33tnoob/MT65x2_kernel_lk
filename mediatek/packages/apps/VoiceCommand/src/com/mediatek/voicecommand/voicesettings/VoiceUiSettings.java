/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.voicecommand.voicesettings;

import android.app.ActionBar;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
//import android.widget.Switch;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;

import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class VoiceUiSettings extends PreferenceActivity implements
        OnSharedPreferenceChangeListener {

    private static final String TAG = "VoiceUiSettings";
    private static final String KEY_VOICE_UI_LANGUAGE = "language_settings";
    private static final String KEY_VOICE_UI_FOR_APP_CATEGORY = "voice_ui_app";
    public static final String KEY_VOICE_UI_FOR_PLAY_COMMAND = "voice_ui_key";
    public static final String KEY_VOICE_UI_SETTINGS = "voice_ui_settings";

    // private static final String VOICE_CONTROL_ENABLED =
    // "voice_control_enabled";

    // default to English
    // private static final String VOICE_UI_SUPPORT_LANGUAGES =
    // "voice_ui_support_languages";
    private String[] mSupportLangs;

    // private Switch mEnabledSwitch;
    private PreferenceCategory mVoiceUiAppCategory;
    private Preference mLanguagePref;
    // data get from framework
    private List<String> mFeatureList;
    private List<SwitchPreference> mFeaturePrefs = new ArrayList<SwitchPreference>();
    private ConfigurationManager mVoiceConfigMgr;
    private boolean isSystemLanguage = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.voice_ui_settings);

        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        if (mVoiceConfigMgr == null) {
            Xlog.e(TAG, "ConfigurationManager is null");
        }
        isSystemLanguage = mVoiceConfigMgr.getIsSystemLanguage();
        Xlog.d(TAG, "VoiceUiSettings isSytemLanguage is " + isSystemLanguage);

        if (isSystemLanguage) {
            Preference voiceUiSettings = findPreference(KEY_VOICE_UI_SETTINGS);
            Xlog.d(TAG, "findpreference : " + voiceUiSettings);
            getPreferenceScreen().removePreference(voiceUiSettings);
        } else {
            mSupportLangs = mVoiceConfigMgr.getLanguageList();
            mLanguagePref = findPreference(KEY_VOICE_UI_LANGUAGE);
        }

        mVoiceUiAppCategory = (PreferenceCategory) findPreference(KEY_VOICE_UI_FOR_APP_CATEGORY);
        getActionBar().setTitle(R.string.voice_ui_title);
        mFeatureList = Arrays.asList(mVoiceConfigMgr.getFeatureNameList());

        createPreferenceHierarchy(mFeatureList);

        getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM);

        mVoiceUiAppCategory.setEnabled(true);
    }

    private void createPreferenceHierarchy(List<String> featureList) {
        for (int i = 0; i < featureList.size(); i++) {
            SwitchPreference appPref = new SwitchPreference(this) {
                @Override
                protected void onClick() {
                    // TODO Auto-generated method stub
                    // super.onClick();
                }
            };
            if (appPref != null) {
                String featureName = featureList.get(i);
                int processID = mVoiceConfigMgr.getProcessID(featureName);
                int TitleId = VoiceUiResUtil
                        .getProcessTitleResourceId(processID);
                if (TitleId != 0) {
                    appPref.setTitle(TitleId);
                } else {
                    appPref.setTitle("error");
                }
                // appPref.setSummary(fetchSummary(featureName));
                int IconId = VoiceUiResUtil.getIconId(processID);
                if (IconId != 0) {
                    appPref.setIcon(IconId);
                }
                appPref.setPersistent(true);
                appPref.setKey(featureName);
                mVoiceUiAppCategory.addPreference(appPref);
                mFeaturePrefs.add(appPref);
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Xlog.d(TAG, "onResume called");

        // boolean isEnable = mVoiceConfigMgr.getVoiceControlEnable();

        Xlog.d(TAG, "feature enabled array = "
                + mVoiceConfigMgr.getFeatureEnableArray().toString());
        // update summary
        for (int i = 0; i < mFeaturePrefs.size(); i++) {
            SwitchPreference appPref = mFeaturePrefs.get(i);

            boolean isEnabled = mVoiceConfigMgr.isProcessEnable(appPref
                    .getKey());
            appPref.setChecked(isEnabled);

        }

        if (!isSystemLanguage) {
            int languageIndex = mVoiceConfigMgr.getCurrentLanguage();
            Xlog.d(TAG, "Default language is " + mSupportLangs[languageIndex]);
            mLanguagePref.setSummary(mSupportLangs[languageIndex]);
        }
        PreferenceManager.getDefaultSharedPreferences(this)
                .registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        PreferenceManager.getDefaultSharedPreferences(this)
                .unregisterOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
            String key) {
        // TODO Auto-generated method stub
        if (mFeatureList.contains(key)) {
            final String processName = key;
            boolean enable = sharedPreferences.getBoolean(key, false);
            Xlog.d(TAG, "onSharedPreferenceChanged set enable " + processName
                    + " " + enable);
            mVoiceConfigMgr.updateFeatureEnable(processName, enable);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {

        if (KEY_VOICE_UI_LANGUAGE.equals(preference.getKey())) {
            Intent intent = new Intent(
                    "com.mediatek.voicecommand.VOICE_CONTROL_LANGUAGE");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            return true;
        } else if (mFeatureList.contains(preference.getKey())) {
            final String processName = preference.getKey();
            Intent intent = new Intent(
                    "com.mediatek.voicecommand.VOICE_UI_COMMAND_PLAY");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(KEY_VOICE_UI_FOR_PLAY_COMMAND, processName);
            startActivity(intent);
            // mVoiceConfigMgr.updateFeatureEnable(processName, enable);
        } else {
            Xlog.d(TAG, "onPreferenceClick not support click this preference "
                    + preference.getKey());
        }

        // mEnabledSwitch.setChecked(mVoiceConfigMgr.getVoiceControlEnable());
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

}
