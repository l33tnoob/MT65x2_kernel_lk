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

package com.mediatek.engineermode.videotelephone;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class TagsSelectActivity extends PreferenceActivity
        implements
        Preference.OnPreferenceChangeListener {

    private String mTagName;
    private String mTagNameKey;
    private String mTagValueKey;
    private CheckBoxPreference mVerbosePref;
    private CheckBoxPreference mDebugPref;
    private CheckBoxPreference mInfoPref;
    private CheckBoxPreference mWarningPref;
    private CheckBoxPreference mErrorPref;
    private CheckBoxPreference mGroup1Pref;
    private CheckBoxPreference mGroup2Pref;
    private CheckBoxPreference mGroup3Pref;
    private CheckBoxPreference mGroup4Pref;
    private CheckBoxPreference mGroup5Pref;
    private CheckBoxPreference mGroup6Pref;
    private CheckBoxPreference mGroup7Pref;
    private CheckBoxPreference mGroup8Pref;
    private CheckBoxPreference mGroup9Pref;
    private CheckBoxPreference mGroup10Pref;
    private static final String[] PREFERENCE_KEYS =
        { "log_filter_tag_0", "log_filter_tag_1", "log_filter_tag_2",
            "log_filter_tag_3", "log_filter_tag_4", "log_filter_tag_5",
            "log_filter_tag_6", "log_filter_tag_7", "log_filter_tag_8",
            "log_filter_tag_9", "log_filter_tag_10", "log_filter_tag_11",
            "log_filter_tag_12", "log_filter_tag_13", "log_filter_tag_14" };
    private static final int NUMER_3 = 3;
    private static final int NUMER_4 = 4;
    private static final int NUMER_5 = 5;
    private static final int NUMER_6 = 6;
    private static final int NUMER_7 = 7;
    private static final int NUMER_8 = 8;
    private static final int NUMER_9 = 9;
    private static final int NUMER_10 = 10;
    private static final int NUMER_11 = 11;
    private static final int NUMER_12 = 12;
    private static final int NUMER_13 = 13;
    private static final int NUMER_14 = 14;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.tag_select);

        mVerbosePref = (CheckBoxPreference) findPreference(PREFERENCE_KEYS[0]);
        mDebugPref = (CheckBoxPreference) findPreference(PREFERENCE_KEYS[1]);
        mInfoPref = (CheckBoxPreference) findPreference(PREFERENCE_KEYS[2]);
        mWarningPref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_3]);
        mErrorPref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_4]);
        mGroup1Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_5]);
        mGroup2Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_6]);
        mGroup3Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_7]);
        mGroup4Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_8]);
        mGroup5Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_9]);
        mGroup6Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_10]);
        mGroup7Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_11]);
        mGroup8Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_12]);
        mGroup9Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_13]);
        mGroup10Pref =
            (CheckBoxPreference) findPreference(PREFERENCE_KEYS[NUMER_14]);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Intent intent = this.getIntent();
        mTagNameKey = intent.getStringExtra(LogFilter.TAR_NAME_KEY);
        mTagName = intent.getStringExtra(LogFilter.TAR_NAME);
        mTagValueKey = intent.getStringExtra(LogFilter.TAR_VALUE_KEY);
        this.setTitle(mTagName);
        
        if (0 != mTagNameKey.compareTo("log_filter_tag_3")) {
            while (getPreferenceScreen().getPreferenceCount() > 5)
                getPreferenceScreen().removePreference((CheckBoxPreference)getPreferenceScreen().getPreference(5));
        }
        initStatus();
    }

    private void initStatus() {
        Xlog.v(VideoTelephony.TAG, "initStatus()");
        try {
            SharedPreferences preferences =
                getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                    WorkingMode.MODE_WORLD_READABLE);
            int value = preferences.getInt(mTagValueKey, -1);
            if (-1 == value) {
                // Hide the info log by default,2012/08/29
                // mInfoPref.setChecked(true);
                mWarningPref.setChecked(true);
                mErrorPref.setChecked(true);
                return;
            }
            CheckBoxPreference[] checkBoxs =
                { mVerbosePref, mDebugPref, mInfoPref, mWarningPref,
                    mErrorPref, mGroup1Pref, mGroup2Pref, mGroup3Pref,
                    mGroup4Pref, mGroup5Pref, mGroup6Pref, mGroup7Pref,
                    mGroup8Pref, mGroup9Pref, mGroup10Pref };
            for (int boxNum = 0; boxNum < checkBoxs.length; boxNum++) {
                if ((value & (1 << boxNum)) != 0) {
                    checkBoxs[boxNum].setChecked(true);
                }
            }
        } catch (ClassCastException e) {
            Xlog.e(VideoTelephony.TAG,
                "TagsSelectActivity get string from pref exception");
        }
    }

    private void saveStatus() {
        Xlog.v(VideoTelephony.TAG, "saveStatus()");
        int value = 0;
        CheckBoxPreference[] checkBoxs =
            { mVerbosePref, mDebugPref, mInfoPref, mWarningPref, mErrorPref,
                mGroup1Pref, mGroup2Pref, mGroup3Pref, mGroup4Pref,
                mGroup5Pref, mGroup6Pref, mGroup7Pref, mGroup8Pref,
                mGroup9Pref, mGroup10Pref };
        for (int boxNum = 0; boxNum < checkBoxs.length; boxNum++) {
            if (checkBoxs[boxNum].isChecked()) {
                value = value + (1 << boxNum);
            }
        }
        Xlog.v(VideoTelephony.TAG, "TagsSelectActivity saveStatus() value = "
            + value);
        SharedPreferences preferences =
            getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
        Editor edit = preferences.edit();
        edit.putString(mTagNameKey, mTagName);
        edit.putInt(mTagValueKey, value);
        edit.commit();
        Xlog.v(VideoTelephony.TAG, "mTagNameKey = " + mTagNameKey
            + " mTagName = " + mTagName);
        Xlog.v(VideoTelephony.TAG, "mTagValueKey = " + mTagValueKey
            + " tagValue = " + value);
    }

    @Override
    protected void onPause() {
        super.onPause();
        saveStatus();
    }

    /**
     * click the preference action
     * 
     * @param preference
     *            : which pref to be clicked
     * @param newValue
     *            : selected value
     * @return whether change preference success
     */
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return false;
    }

}
