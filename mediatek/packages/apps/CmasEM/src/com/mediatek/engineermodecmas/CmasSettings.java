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

package com.mediatek.engineermodecmas;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.widget.Toast;

import com.mediatek.engineermodecmas.R;
import com.mediatek.xlog.Xlog;

public class CmasSettings extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {

    private static final String TAG = "EM/CmasSettings";

    private static final String PACKAGE_NAME = "com.mediatek.cellbroadcastreceiver";
    private static final String PREF_NAME = "com.mediatek.cellbroadcastreceiver_preferences";

    public static final String CMAS_RMT_KEY = "enable_cmas_rmt_support";
    public static final String CMAS_EXERCISE_KEY = "enable_cmas_exercise_support";

    private Context mCmasContext;

    private CheckBoxPreference mCheckBoxRmt;
    private CheckBoxPreference mCheckBoxExercise;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.i(TAG, "onCreate()");
        addPreferencesFromResource(R.xml.cmas_setting);

        mCheckBoxRmt = (CheckBoxPreference) findPreference(CMAS_RMT_KEY);
        mCheckBoxExercise = (CheckBoxPreference) findPreference(CMAS_EXERCISE_KEY);

        try {
            mCmasContext = createPackageContext(PACKAGE_NAME, 0);
        } catch (PackageManager.NameNotFoundException e) {
            Xlog.i(TAG, "CMAS App not installed");
            Toast.makeText(this, "CMAS App not installed.", Toast.LENGTH_SHORT).show();
            getPreferenceScreen().setEnabled(false);
            return;
        }

        mCheckBoxRmt.setOnPreferenceChangeListener(this);
        mCheckBoxExercise.setOnPreferenceChangeListener(this);
        Xlog.i(TAG, "onCreate() End");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Xlog.i(TAG, "onResume()");

        if (mCmasContext == null) {
            return;
        }

        SharedPreferences prefs =
                mCmasContext.getSharedPreferences(PREF_NAME, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE);

        boolean rmtValue = prefs.getBoolean(CMAS_RMT_KEY, false);
        boolean exerciseValue = prefs.getBoolean(CMAS_EXERCISE_KEY, false);

        mCheckBoxRmt.setChecked(rmtValue);
        mCheckBoxExercise.setChecked(exerciseValue);
        Xlog.i(TAG, "onResume()" + rmtValue + exerciseValue);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        SharedPreferences.Editor editor =
                mCmasContext.getSharedPreferences(PREF_NAME, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE)
                            .edit();

        if (preference.getKey().equals(CMAS_RMT_KEY)) {
            Xlog.i(TAG, "onPreferenceChange(): CMAS RMT " + newValue);
            editor.putBoolean(CMAS_RMT_KEY, (Boolean) newValue);
        } else if (preference.getKey().equals(CMAS_EXERCISE_KEY)) {
            Xlog.i(TAG, "onPreferenceChange(): CMAS Exercise" + newValue);
            editor.putBoolean(CMAS_EXERCISE_KEY, (Boolean) newValue);
        }
        editor.commit();
        return true;
    }

}
