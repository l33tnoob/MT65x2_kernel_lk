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

import android.app.Activity;
import android.app.ActivityManagerNative;
import android.app.IActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Bundle;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.internal.app.LocalePicker;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.List;
import java.util.Locale;

public class LanguageSettingsWizard extends Activity 
            implements View.OnClickListener, AdapterView.OnItemSelectedListener {
    private static final String TAG = "OOBE.language";
    private static final String ACTION_EMERGENCY_CALL = "com.android.phone.EmergencyDialer.DIAL";
    private ArrayAdapter<LocalePicker.LocaleInfo> mAdapter;
    private Spinner mSpinner = null;
    private Button mEmergencybtn;
    private Button mBackbtn;
    private Button mNextbtn;

    private Locale mCurrentLocale;
    private IntentFilter mSimStateIntentFilter;
    private boolean mFirstRun = false;

    /**
     *  Handle sim info update action to update the state of emergency call button
     */
    private BroadcastReceiver mSimStateChangedListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)) {
                updateEmergencycallButton();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.d(TAG, "OnCreate()");
        setContentView(R.layout.language_layout);

        mCurrentLocale = getLanguage();
        Xlog.d(TAG, "current language is: " + mCurrentLocale);

        mSpinner = (Spinner) findViewById(R.id.language_spinner);
        mSpinner.setVisibility(View.VISIBLE);

        initLayout();

        mSimStateIntentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        mFirstRun = Settings.System.getInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 0) == 0;
    }

    /**
     * init layout of language
     */
    private void initLayout() {
        //init emergency button
        mEmergencybtn = (Button) findViewById(R.id.emergcy_call_button);
        updateEmergencycallButton();

        //set title
        TextView title = (TextView) findViewById(R.id.settings_title);
        title.setText(R.string.oobe_title_language_setting);

        //init progress bar
        LinearLayout progressBar = (LinearLayout) findViewById(R.id.progressbar_layout);
        int totalStep = getIntent().getIntExtra(Utils.OOBE_BASIC_STEP_TOTAL, 1);
        int stepIndex = getIntent().getIntExtra(Utils.OOBE_BASIC_STEP_INDEX, 0);
        for (int i = 0; i < totalStep; i++) {
            ImageView child = (ImageView) progressBar.getChildAt(i);
            if (i == stepIndex - 1) {
                child.setImageResource(R.drawable.progress_radio_on);
            }
            child.setVisibility(View.VISIBLE);
        }

        //init back and next button
        mBackbtn = (Button) findViewById(R.id.panel_button_back);
        mBackbtn.setOnClickListener(this);
        if (stepIndex == 1) {
            mBackbtn.setVisibility(View.INVISIBLE);
        }
        mNextbtn = (Button) findViewById(R.id.panel_button_next);
        mNextbtn.setOnClickListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        
        mAdapter = LocalePicker.constructAdapter(this);
        mSpinner.setAdapter(mAdapter);
        for (int i = 0; i < mAdapter.getCount(); i++) {
            LocalePicker.LocaleInfo locale = mAdapter.getItem(i);
            if (mCurrentLocale != null && locale != null
                        && mCurrentLocale.equals(locale.getLocale())) {
                Xlog.d(TAG, "set language is: " + locale.toString());
                mSpinner.setSelection(i, true);
            }
        }
        mSpinner.setOnItemSelectedListener(this);
        
        registerReceiver(mSimStateChangedListener, mSimStateIntentFilter);
    }


    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mSimStateChangedListener);
    }

    @Override
    public void onDestroy() {
        Xlog.v(TAG, "LanguageSetupActivity onDestroy");
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        if (v == mEmergencybtn) {
            Intent intent = new Intent(ACTION_EMERGENCY_CALL);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            startActivity(intent);
        } else if (v == mBackbtn) {
            nextStep(false);
        } else if (v == mNextbtn) {
            nextStep(true);
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        LocalePicker.LocaleInfo locale = mAdapter.getItem(position);
        if (locale != null && locale.getLocale() != null 
                    && !locale.getLocale().equals(mCurrentLocale)) {
            Xlog.d(TAG, "onItemSelected,select language is: " + locale.toString());
            LocalePicker.updateLocale(locale.getLocale());
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
    }

    private Locale getLanguage() {
        try {
            IActivityManager am = ActivityManagerNative.getDefault();
            Configuration config = am.getConfiguration();
            return config.locale;
        } catch (android.os.RemoteException e) {
            Xlog.e(TAG, "get language Excetpion");
            e.printStackTrace();
        }
        return null;
    }

    /**
     * update emergency call button status
     */
    private void updateEmergencycallButton() {
        boolean emergency = !isSimExist() && !Utils.isWifiOnly(this) && Utils.isVoiceSupport(this);
        if (emergency) {
            mEmergencybtn.setVisibility(View.VISIBLE);
            mEmergencybtn.setOnClickListener(this);
        } else {
            mEmergencybtn.setVisibility(View.GONE);
            mEmergencybtn.setOnClickListener(null);
        }

    }

    /**
     * Is sim inserted
     * @return true if sim card is exist, false if sim card is not exist
     */
    private boolean isSimExist() {
        boolean simExist = false;
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(this);
        if (simList != null) {
            simExist = simList.size() > 0;
        }
        return simExist;
    }

    /**
     * Go to next step
     * @param isNext true to start next step, false to start last step
     */
    private void nextStep(boolean isNext) {
        int result = isNext ? Utils.RESULT_CODE_NEXT : Utils.RESULT_CODE_BACK;
        finishActivityByResult(result);
    }

    /**
     * Set result code and finish
     * @param resultCode true to start next step, false to start last step
     */
    private void finishActivityByResult(int resultCode) {
        Xlog.d(TAG, "finishActivityByResult, resultCode: " + resultCode);
        Intent intent = new Intent();
        setResult(resultCode, intent);
        finish();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            // do not handle back key if it's first run
            if (mFirstRun) {
                return true;
            }
            finishActivityByResult(Utils.RESULT_CODE_BACK);
        }
        return super.onKeyDown(keyCode, event);
    }
}
