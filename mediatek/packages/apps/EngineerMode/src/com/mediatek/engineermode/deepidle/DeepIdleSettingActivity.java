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

package com.mediatek.engineermode.deepidle;

import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.Toast;
import android.widget.CompoundButton.OnCheckedChangeListener;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

public class DeepIdleSettingActivity extends Activity implements OnClickListener,
        OnCheckedChangeListener{

	private static final String TAG = "DeepIdle";
	private static final String FS_DPIDLE_MODE = "/proc/spm_fs/dpidle_mode";
	private static final String FS_DPIDLE_LEVEL = "/proc/spm_fs/dpidle_level";
	private static final String FS_DPIDLE_TIMERVAL = "/proc/spm_fs/dpidle_timer";
	private static final String FS_DPIDLE = "/proc/spm_fs/dpidle";
	private static final String CAT = "cat ";
	private static final String CMD_CPU_PDN = 
		    "echo \"%1$d cpu_pdn\" > " + FS_DPIDLE;
	private static final String CMD_POWER_LEVEL = 
		    "echo \"%1$d pwrlevel\" > " + FS_DPIDLE;
	private static final String CMD_TIMER_VAL = 
		    "echo \"%1$s timer_val_ms\" > " + FS_DPIDLE;
	
	private RadioButton mRBDisableDpIdl;
	private RadioButton mRBLegacySleep;
	private RadioButton mRBDormantMode;
	private RadioButton mRBPowerLevel0;
	private RadioButton mRBPowerLevel1;
	private RadioButton mRBDisableTimer;
	private RadioButton mRBTimerValSet;
	private RadioButton[] mRBModes = new RadioButton[3];
	private RadioButton[] mRBLevels = new RadioButton[2];
	private RadioButton[] mRBTimerVals = new RadioButton[2];
	
	private EditText mEditTimerVal;
	private Button mBtnStartTimer;
	private Button mBtnGetSetting;
	private boolean mInitDone;
	private LinearLayout mLevelControler;
	private LinearLayout mSetTimerControler;
	@Override
	 protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.deep_idle_setting);
	    
		mRBDisableDpIdl = (RadioButton)findViewById(R.id.deep_idle_disable);
		mRBDisableDpIdl.setOnCheckedChangeListener(this);
		mRBLegacySleep = (RadioButton)findViewById(R.id.deep_idle_legacy_sleep);
		mRBLegacySleep.setOnCheckedChangeListener(this);
		mRBDormantMode = (RadioButton)findViewById(R.id.deep_idle_dormant_mode);
		mRBDormantMode.setOnCheckedChangeListener(this);
		mRBModes[0] = mRBDisableDpIdl;
		mRBModes[1] = mRBLegacySleep;
		mRBModes[2] = mRBDormantMode;
		
		mRBPowerLevel0 = (RadioButton)findViewById(R.id.deep_idle_power_level0);
		mRBPowerLevel0.setOnCheckedChangeListener(this);
		mRBPowerLevel1 = (RadioButton)findViewById(R.id.deep_idle_power_level1);
		mRBPowerLevel1.setOnCheckedChangeListener(this);
		mRBLevels[0] = mRBPowerLevel0;
		mRBLevels[1] = mRBPowerLevel1;
		
		mRBDisableTimer = (RadioButton)findViewById(R.id.deep_idle_timer_disable);
		mRBDisableTimer.setOnCheckedChangeListener(this);
		mRBTimerValSet = (RadioButton)findViewById(R.id.mcdi_timer_val_set);
		mRBTimerValSet.setOnCheckedChangeListener(this);
		mRBTimerVals[0] = mRBDisableTimer;
		mRBTimerVals[1] = mRBTimerValSet;
		
		mEditTimerVal = (EditText)findViewById(R.id.deep_idle_timer_val);
		mBtnStartTimer = (Button)findViewById(R.id.deep_idle_start_timer);
		mBtnStartTimer.setOnClickListener(this);
		mBtnGetSetting = (Button)findViewById(R.id.deep_idle_get_setting);
		mBtnGetSetting.setOnClickListener(this);
		mLevelControler = (LinearLayout)findViewById(R.id.deep_idle_power_level_contrl);
		mSetTimerControler = (LinearLayout)findViewById(R.id.deep_idle_wake_timer_contrl);
		
		mInitDone = false;
		initUIByData();
		mInitDone = true;
	}
	
	private void initUIByData() {
		String cmd;
		String output;
		
		cmd = CAT + FS_DPIDLE_MODE;
		output = execCommand(cmd);
		//output = "1 \n";
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
		output = output.trim();
		int modeIdx = -1;
		try {
			modeIdx = Integer.parseInt(output);
		} catch(NumberFormatException e) {
			Xlog.e(TAG, "NumberFormatException invalid output:" + output);
		}
		try {
			mRBModes[modeIdx].setChecked(true);
		} catch (IndexOutOfBoundsException e) {
			Xlog.e(TAG, "Fail to set Default Mode; IndexOutOfBoundsException: " + e.getMessage());
		}
		
		cmd = CAT + FS_DPIDLE_LEVEL;
		output = execCommand(cmd);
		//output = "0 ";
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
		output = output.trim();
		int levelIdx = -1;
		try {
			levelIdx = Integer.parseInt(output);
		} catch(NumberFormatException e) {
			Xlog.e(TAG, "NumberFormatException invalid output:" + output);
		}
		try {
			mRBLevels[levelIdx].setChecked(true);
		} catch (IndexOutOfBoundsException e) {
			Xlog.e(TAG, "Fail to set Default Level; IndexOutOfBoundsException: " + e.getMessage());
		}
		
		cmd = CAT + FS_DPIDLE_TIMERVAL;
		output = execCommand(cmd);
		//output = " 100 ";
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
		output = output.trim();
		Xlog.d(TAG, "timer val output: " + output);
		int timerVal = -1;
		try {
			timerVal = Integer.parseInt(output);
		} catch(NumberFormatException e) {
			Xlog.e(TAG, "NumberFormatException invalid output:" + output);
		}
		if (timerVal == 0) {
			mRBDisableTimer.setChecked(true);
		} else if (timerVal > 15 && timerVal < 1000000) {
			mRBTimerValSet.setChecked(true);
			mEditTimerVal.setText(output);
		} else {
			Xlog.e(TAG, "Invalid Timer Value:" + timerVal);
		}
	}
	
	private void enableTimerValUI(boolean enabled) {
		mEditTimerVal.setEnabled(enabled);
		mBtnStartTimer.setEnabled(enabled);
	}
	
	private void setCpuPdn(int input) {
		String cmd;
		cmd = String.format(CMD_CPU_PDN, input);
		execCommand(cmd);
	}
	
	private void setPowerLevel(int input) {
		String cmd;
		cmd = String.format(CMD_POWER_LEVEL, input);
		execCommand(cmd);
	}
	
	private void setTimerVal(String input) {
		String cmd;
		cmd = String.format(CMD_TIMER_VAL, input);
		execCommand(cmd);
	}
	
	private String execCommand(String cmd) {
		 int ret = -1;
		 Xlog.d(TAG, "[cmd]:" + cmd);
		 //Toast.makeText(this, cmd, Toast.LENGTH_SHORT).show();
		 try {
			 ret = ShellExe.execCommand(cmd);
		 } catch (IOException e) {
			 Xlog.e(TAG, "IOException: " + e.getMessage());
		 }
		 if (ret == 0) {
			 String outStr = ShellExe.getOutput();
			 Xlog.d(TAG, "[output]: " + outStr);
			 return outStr;
		 } 
		 return null;
	 }
	
	private void showDialog(String title, String msg) {
		AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
               false).setTitle(title).setMessage(msg).setCancelable(true).
               setPositiveButton(android.R.string.ok, null).create();
		dialog.show();
	}
	
	private void checkOneRadio(RadioButton[] array, RadioButton target, boolean checked) {
		 for (int i = 0; i < array.length; i++) {
			 if (target == array[i]) {
				 array[i].setChecked(checked);
			 } else {
				 array[i].setChecked(!checked);
			 }
		 }
	 }
	
//	private void enableLevelSetTimerUI(boolean enabled) {
//		int i;
//		for (i = 0; i < mRBLevels.length; i++) {
//			mRBLevels[i].setEnabled(enabled);
//		}
//		for (i = 0; i < mRBTimerVals.length; i++) {
//			mRBTimerVals[i].setEnabled(enabled);
//		}
//	}
	
	private void visibleLevelSetTimerUI(boolean visible) {
		int visibility;
		if (visible) {
			visibility = View.VISIBLE;
		} else {
			visibility = View.GONE;
		}
		mLevelControler.setVisibility(visibility);
		mSetTimerControler.setVisibility(visibility);
		mBtnStartTimer.setVisibility(visibility);
	}
	
	private boolean validateInputData() {
		String msg;
		if (TextUtils.isEmpty(mEditTimerVal.getText())) {
			msg = getString(R.string.deep_idle_invalid_timer_val);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return false;
		}
		int timerVal = -1;
		String timerValStr = mEditTimerVal.getText().toString();
		try {
			timerVal = Integer.parseInt(timerValStr);
		} catch (NumberFormatException e) {
			msg = getString(R.string.deep_idle_invalid_timer_val);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return false;
		}
		if (timerVal < 15 || timerVal > 1000000) {
			msg = getString(R.string.deep_idle_invalid_timer_val);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return false;
		}
		return true;
	}
	
	@Override
	public void onClick(View view) {
		int id = view.getId();
		String cmd;
		String output;
		switch(id) {
		case R.id.deep_idle_start_timer:
			if (validateInputData()) {
				setTimerVal(mEditTimerVal.getText().toString());
			}
		    break;
		case R.id.deep_idle_get_setting:
			cmd = CAT + FS_DPIDLE;
			output = execCommand(cmd);
			showDialog(getString(R.string.deep_idle_setting), output);
			break;
		default:
			Xlog.w(TAG, "unknown view id: " + id);
			break;
		}
	}
	
	@Override
	public void onCheckedChanged(CompoundButton btnView, boolean isChecked) {
		int id = btnView.getId();
		switch(id) {
		case R.id.deep_idle_disable:
			if (isChecked) {
				//enableLevelSetTimerUI(false);
				visibleLevelSetTimerUI(false);
			    setCpuPdn(0);
			}
		    break;
		case R.id.deep_idle_legacy_sleep:
			if (isChecked) {
				//enableLevelSetTimerUI(true);
				visibleLevelSetTimerUI(true);
			    setCpuPdn(1);
			}
			break;
		case R.id.deep_idle_dormant_mode:
			if (isChecked) {
				//enableLevelSetTimerUI(true);
				visibleLevelSetTimerUI(true);
			    setCpuPdn(2);
			}
			break;
		case R.id.deep_idle_power_level0:
			if (isChecked) {
			    setPowerLevel(0);
			}
			break;
		case R.id.deep_idle_power_level1:
			if (isChecked) {
			    setPowerLevel(1);
			}
			break;
		case R.id.deep_idle_timer_disable:
			if (isChecked) {
				Xlog.d(TAG, "[debug]onCheckedChanged: deepIdle timer disable");
			    enableTimerValUI(false);
			    checkOneRadio(mRBTimerVals, mRBDisableTimer, true);
			    setTimerVal("0");
			}
			break;
		case R.id.mcdi_timer_val_set:
			if (isChecked) {
			    enableTimerValUI(true);
			    checkOneRadio(mRBTimerVals, mRBTimerValSet, true);
			}
			break;
		default:
			Xlog.w(TAG, "Unknown CompoundButton id: " + id);
			break;
		}
	}
}
