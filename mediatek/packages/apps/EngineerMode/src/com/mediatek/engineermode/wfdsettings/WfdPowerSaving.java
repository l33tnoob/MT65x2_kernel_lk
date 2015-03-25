/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERfETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

package com.mediatek.engineermode.wfdsettings;

import android.app.Activity;
import android.content.SharedPreferences;
import android.view.View;
import android.view.View.OnClickListener;
import android.os.Bundle;
import android.widget.Button;

import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.provider.Settings;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;
import java.util.Arrays;

public class WfdPowerSaving extends Activity implements OnClickListener {
    private static final String TAG = "EM/WFD_POWER_SAV";
    private static final int SETTING_BASE_NORMAL = 0;
    private static final int SETTING_BASE_EXT = 10;
    private static final String POWER_SAVING_PREF_NAME = "em_wfd_power_saving";
    private static final String PREF_KEY_ENABLE_EXT_DISPLAY = "enable_ext_display";

    private Button  mBtDone = null;
    private RadioGroup mRgPower = null;
    private EditText mEtDisplayTime = null;
    
    private int mState = 0;
    private int mDisplayTime = 0;
    private int mSettingBase = SETTING_BASE_NORMAL;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfd_power_saving);
        
        mRgPower = (RadioGroup) findViewById(R.id.Wfd_Power_Saving_Style);
        mEtDisplayTime = (EditText) findViewById(R.id.Wfd_Power_Delay_Time_Edit);
        /* remove dim and show image , rollback in future*/
        RadioButton tempButton = (RadioButton) findViewById(R.id.Wfd_Power_Style_2);
        tempButton.setVisibility(View.GONE);
        
        mBtDone = (Button) findViewById(R.id.Wfd_Done);
        mBtDone.setOnClickListener(this);
        
        mState = Settings.Global.getInt(getContentResolver(), Settings.Global.WIFI_DISPLAY_POWER_SAVING_OPTION, 0);
        //initFromPreference();
        setInitState(mState);
        mDisplayTime =  Settings.Global.getInt(getContentResolver(), Settings.Global.WIFI_DISPLAY_POWER_SAVING_DELAY, 0);
        mEtDisplayTime.setText(String.valueOf(mDisplayTime));
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6582_SUPPORT)) {
            findViewById(R.id.wfd_power_saving_ext_display).setVisibility(View.GONE);
        }
    }
    
    private void saveToPreference() {
        SharedPreferences pref = getSharedPreferences(POWER_SAVING_PREF_NAME, MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        CheckBox checkExtDisplay = (CheckBox)findViewById(R.id.wfd_power_saving_ext_display);
        boolean checked = checkExtDisplay.isChecked();
        editor.putBoolean(PREF_KEY_ENABLE_EXT_DISPLAY, checked);
        if (!checked) {
            mSettingBase = SETTING_BASE_NORMAL;
        } else {
            mSettingBase = SETTING_BASE_EXT;
        }
        editor.commit();
    }
    
    private void initFromPreference() {
        SharedPreferences pref = getSharedPreferences(POWER_SAVING_PREF_NAME, MODE_PRIVATE);
        boolean enableExtDisplay = pref.getBoolean(PREF_KEY_ENABLE_EXT_DISPLAY, false);
        if (!enableExtDisplay) {
            mSettingBase = SETTING_BASE_NORMAL;
        } else {
            mSettingBase = SETTING_BASE_EXT;
        }
        CheckBox checkExtDisplay = (CheckBox)findViewById(R.id.wfd_power_saving_ext_display);
        checkExtDisplay.setChecked(enableExtDisplay);
    }

    @Override
    public void onClick(View view) {
        Elog.d(TAG, "view_id = " + view.getId());
        if (view.getId() == mBtDone.getId()) {
            onClickBtnDone();
            finish();
        } 
    }
    private void setInitState(int state) {
        if (state >= SETTING_BASE_EXT) {
            mSettingBase = SETTING_BASE_EXT;
            CheckBox checkExtDisplay = (CheckBox)findViewById(R.id.wfd_power_saving_ext_display);
            checkExtDisplay.setChecked(true);
        } else {
            mSettingBase = SETTING_BASE_NORMAL;
        }
        Elog.v(TAG, "setInitState = " + state + " mSettingBase:" + mSettingBase);

        if (state >= mSettingBase) {
            state = state - mSettingBase;
        } else {
            Elog.d(TAG, "Invalid state:" + state + " mSettingBase:" + mSettingBase);
        }
        if(state == 0) {
            mRgPower.check(R.id.Wfd_Power_Style_0);
        } else if(state == 1) {
            mRgPower.check(R.id.Wfd_Power_Style_1);
        } else if(state == 2) {
        //   mRgPower.check(R.id.Wfd_Power_Style_2);
        //} else if(state == 3) {
            mRgPower.check(R.id.Wfd_Power_Style_3);
        } else {
            Elog.w(TAG, "Wrong input resolution info");
        }
    }
    
    private int getState(){
        CheckBox checkExtDisplay = (CheckBox)findViewById(R.id.wfd_power_saving_ext_display);
        boolean checked = checkExtDisplay.isChecked();
        if (!checked) {
            mSettingBase = SETTING_BASE_NORMAL;
        } else {
            mSettingBase = SETTING_BASE_EXT;
        }
        int state = 0;
        if(mRgPower.getCheckedRadioButtonId() == R.id.Wfd_Power_Style_0) {
            state = 0 + mSettingBase;
        } else if(mRgPower.getCheckedRadioButtonId() == R.id.Wfd_Power_Style_1) {
            state = 1 + mSettingBase;
        } else if(mRgPower.getCheckedRadioButtonId() == R.id.Wfd_Power_Style_2) {
            state = 2 + mSettingBase;
        } else if(mRgPower.getCheckedRadioButtonId() == R.id.Wfd_Power_Style_3) {
            state = 2 + mSettingBase;  // temp revise 3 -> 2
        }
        return state;
    }
    
    private void onClickBtnDone() {
        //saveToPreference();
        mState = getState();
        Settings.Global.putInt(getContentResolver(), Settings.Global.WIFI_DISPLAY_POWER_SAVING_OPTION, mState);
        
        mDisplayTime = Integer.valueOf(mEtDisplayTime.getText().toString());
        Settings.Global.putInt(getContentResolver(), Settings.Global.WIFI_DISPLAY_POWER_SAVING_DELAY, mDisplayTime);
        Elog.v(TAG, "Last solution = " + mState);
        Elog.v(TAG, "Display Time = " + mDisplayTime);
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

}
