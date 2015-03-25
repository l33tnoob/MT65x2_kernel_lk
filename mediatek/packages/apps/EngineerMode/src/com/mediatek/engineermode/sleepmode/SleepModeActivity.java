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

package com.mediatek.engineermode.sleepmode;

import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.PowerManager;
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

public class SleepModeActivity extends Activity implements OnClickListener, 
        OnCheckedChangeListener  {

	private static final String TAG = "EM/SleepMode";
	private static final String FS_SUSPEND_MODE = "/proc/spm_fs/suspend_mode";
	private static final String FS_SUSPEND = "/proc/spm_fs/suspend";
	private static final String FS_SUSPEND_TIMER = "/proc/spm_fs/suspend_timer";
	private static final String FS_WAKE_LOCK = "/sys/power/wake_lock";
	private static final String CAT = "cat ";
	private static final String CMD_CPU_PDN = 
		    "echo \"%1$d cpu_pdn\" > " + FS_SUSPEND;
	private static final String CMD_INFRA_PDN = 
		    "echo \"%1$d infra_pdn\" > " + FS_SUSPEND;
	private static final String CMD_FGAUGE = 
		    "echo \"%1$d fgauge\"  > " + FS_SUSPEND;
	private static final String CMD_TIMER_VAL = 
		    "echo \"%1$s timer_val_ms\"  > " + FS_SUSPEND;
	private static final String CMD_WAKE_LOCK = 
		    "echo nosleep > " + FS_WAKE_LOCK;
	private static final String CMD_WAKE_UNLOCK = 
		    "echo nosleep > /sys/power/wake_unlock";
	private static final String TAG_NOSLEEP = "nosleep";
	private static final int SLEEP_MODE_DISABLE = 0;
	private static final int SLEEP_MODE_LEGACY_SLEEP = 1;
	private static final int SLEEP_MODE_SHUT_DOWN = 2;
	
	private RadioButton mRBModeDisable;
	private RadioButton mRBLagacySleep;
	private RadioButton mRBShutDown;
	private RadioButton mRBTimerDisable;
	private RadioButton mRBFuelGauge;
	private RadioButton mRBTimerValSet;
	private RadioButton[] mRBSleepModes;
	private RadioButton[] mRBWakeupTimers;
	
	private Button mBtnStartTimer;
	private Button mBtnGetSetting;
	private EditText mEditTimerVal;
	private LinearLayout mLVTimerControler;
	private PowerManager mPowerManager;
	private PowerManager.WakeLock mWakeLock;
	
	private int mSleepMode = -1;
	
	@Override
    protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.sleep_mode_6572);
		mRBModeDisable = (RadioButton)findViewById(R.id.sleep_mode_disable);
		mRBModeDisable.setOnCheckedChangeListener(this);
		mRBLagacySleep = (RadioButton)findViewById(R.id.sleep_mode_legacy_sleep);
		mRBLagacySleep.setOnCheckedChangeListener(this);
		mRBShutDown = (RadioButton)findViewById(R.id.sleep_mode_shutdown);
		mRBShutDown.setOnCheckedChangeListener(this);
		mRBSleepModes = new RadioButton[3];
		mRBSleepModes[0] = mRBModeDisable;
		mRBSleepModes[1] = mRBLagacySleep;
		mRBSleepModes[2] = mRBShutDown;
		
		mRBTimerDisable = (RadioButton)findViewById(R.id.sleep_mode_timer_disable);
		mRBTimerDisable.setOnCheckedChangeListener(this);
		mRBFuelGauge = (RadioButton)findViewById(R.id.sleep_mode_timer_fuel_gauge);
		mRBFuelGauge.setOnCheckedChangeListener(this);
		mRBTimerValSet = (RadioButton)findViewById(R.id.sleep_mode_timer_val_set);
		mRBTimerValSet.setOnCheckedChangeListener(this);
		mRBWakeupTimers = new RadioButton[3];
		mRBWakeupTimers[0] = mRBTimerDisable;
		mRBWakeupTimers[1] = mRBFuelGauge;
		mRBWakeupTimers[2] = mRBTimerValSet;
		
		mBtnStartTimer = (Button)findViewById(R.id.sleep_mode_start_timer);
		mBtnStartTimer.setOnClickListener(this);
		mBtnGetSetting = (Button)findViewById(R.id.sleep_mode_get_setting);
		mBtnGetSetting.setOnClickListener(this);
		mEditTimerVal = (EditText)findViewById(R.id.sleep_mode_timer_val);
		mLVTimerControler = (LinearLayout)findViewById(R.id.sleep_mode_wake_timer_contrl);
		mPowerManager = (PowerManager)getSystemService(Context.POWER_SERVICE);
		mWakeLock = mPowerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK, TAG);
		initUIByData();
	}
	
	private void initUIByData() {
		String cmd;
		String output;
		// init sleep mode ui
		cmd = CAT + FS_WAKE_LOCK;
        output = execCommand(cmd);
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        if (output.contains(TAG_NOSLEEP)) {
        	mRBModeDisable.setChecked(true);
        	mSleepMode = SLEEP_MODE_DISABLE;
        } else {
		    cmd = CAT + FS_SUSPEND_MODE;
		    output = execCommand(cmd);
		    //output = " 1";
	        if (output == null) {
	            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
	            finish();
	            return;
	        }
		    output = output.trim();
		    if ("0".equals(output)) {
			    mRBLagacySleep.setChecked(true);
			    mSleepMode = SLEEP_MODE_LEGACY_SLEEP;
		    } else if ("1".equals(output)) {
			    mRBShutDown.setChecked(true);
			    mSleepMode = SLEEP_MODE_SHUT_DOWN;
		    } else {
		    	mSleepMode = -1;
			    Xlog.d(TAG, "cat suspend_mode:" + output);
		    }
        }
		
		// init wakeup Timer ui
		cmd = CAT + FS_SUSPEND_TIMER;
		output = execCommand(cmd);
		//output = "0  0";
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
		output = output.trim();
		String[] vals = output.split(" +");
		if ("1".equals(vals[0])) {
			checkOneRadio(mRBWakeupTimers, mRBFuelGauge, true);
		} else if ("0".equals(vals[1])) {
			checkOneRadio(mRBWakeupTimers, mRBTimerDisable, true);
		} else {
			checkOneRadio(mRBWakeupTimers, mRBTimerValSet, true);
			mEditTimerVal.setText(vals[1]);
		}
	}
	
	private void enableInputTimerUI(boolean enabled) {
		mBtnStartTimer.setEnabled(enabled);
		mEditTimerVal.setEnabled(enabled);	
	}
	
	private boolean setCpuPdn(int input) {
		String cmd;
		cmd = String.format(CMD_CPU_PDN, input);
		return null != execCommand(cmd);
	}
	
	private boolean setInfraPdn(int input) {
		String cmd;
		cmd = String.format(CMD_INFRA_PDN, input);
		return null != execCommand(cmd);
	}
	
	private boolean setFgauge(int input) {
		String cmd;
		cmd = String.format(CMD_FGAUGE, input);
		return null != execCommand(cmd);
	}
	
	private boolean setTimerVal(String timerVal) {
		String cmd;
		cmd = String.format(CMD_TIMER_VAL, timerVal);
		return null != execCommand(cmd);
	}
	
	private boolean acquireWakeLock() {
		String cmd;
		cmd = CMD_WAKE_LOCK;
		return null != execCommand(cmd);
	}
	
	private boolean releaseWakeLock() {
		String cmd;
		cmd = CMD_WAKE_UNLOCK;
		return null != execCommand(cmd);
	}
	
	@Override
	public void onCheckedChanged(CompoundButton btnView, boolean isChecked) {
		int id = btnView.getId();
		switch(id) {
		case R.id.sleep_mode_disable:
			if (isChecked) {
				mSleepMode = SLEEP_MODE_DISABLE;
				checkOneRadio(mRBSleepModes, mRBModeDisable, true);
				mLVTimerControler.setVisibility(View.GONE);
				acquireWakeLock();
			}
			break;
		case R.id.sleep_mode_legacy_sleep:
			if (isChecked) {
				checkOneRadio(mRBSleepModes, mRBLagacySleep, true);
				mLVTimerControler.setVisibility(View.VISIBLE);
				if (mSleepMode == SLEEP_MODE_DISABLE) {
					releaseWakeLock();
				}
				mSleepMode = SLEEP_MODE_LEGACY_SLEEP;
				setCpuPdn(0);
				setInfraPdn(0);
			}
			break;
		case R.id.sleep_mode_shutdown:
			if (isChecked) {
				checkOneRadio(mRBSleepModes, mRBShutDown, true);
				mLVTimerControler.setVisibility(View.VISIBLE);
				if (mSleepMode == SLEEP_MODE_DISABLE) {
					releaseWakeLock();
				}
				mSleepMode = SLEEP_MODE_SHUT_DOWN;
				setCpuPdn(1);
				setInfraPdn(1);
			}
			break;
		case R.id.sleep_mode_timer_disable:
			if (isChecked) {
				checkOneRadio(mRBWakeupTimers, mRBTimerDisable, true);
				setFgauge(0);
				setTimerVal("0");
				enableInputTimerUI(false);
			}
			break;
		case R.id.sleep_mode_timer_fuel_gauge:
			if (isChecked) {
				checkOneRadio(mRBWakeupTimers, mRBFuelGauge, true);
				setFgauge(1);
				enableInputTimerUI(false);
			}
			break;
		case R.id.sleep_mode_timer_val_set:
			if (isChecked) {
				checkOneRadio(mRBWakeupTimers, mRBTimerValSet, true);
				enableInputTimerUI(true);
			}
			break;
		default:
	    	Xlog.w(TAG, "unknown view id: " + id);
	    	break;
		}
	}
	
	@Override
	protected void onDestroy() {
	    if (mWakeLock.isHeld()) {
	    	mWakeLock.release();
	    }
	    mWakeLock = null;
	    super.onDestroy();
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
	
	private boolean validateInput() {
		String timerValStr;
		String msg;
		if (TextUtils.isEmpty(mEditTimerVal.getText())) {
			msg = getString(R.string.sleep_mode_invalid_timer_val);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return false;
		}
		timerValStr = mEditTimerVal.getText().toString();
		int timerVal;
		try {
			timerVal = Integer.parseInt(timerValStr);
		} catch (NumberFormatException e) {
			Xlog.d(TAG, "NumberFormatException: parse timerVal fail");
			msg = getString(R.string.sleep_mode_invalid_timer_val);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return false;
		}
		if (timerVal < 15 || timerVal > 1000000) {
			msg = getString(R.string.sleep_mode_invalid_timer_val);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return false;
		}
		return true;
	}

	@Override
	public void onClick(View view) {
		String cmd;
		String output;
		int id = view.getId();
		switch(id) {
		case R.id.sleep_mode_start_timer:
			if (validateInput()) {
				setFgauge(0);
				setTimerVal(mEditTimerVal.getText().toString());
				String msg;
				msg = getString(R.string.sleep_mode_operate_success);
				Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			}
			break;
		case R.id.sleep_mode_get_setting:
			cmd = CAT + FS_SUSPEND;
			output = execCommand(cmd);
			showDialog(getString(R.string.sleep_mode_setting), output);
			break;
		default:
	    	Xlog.w(TAG, "unknown view id: " + id);
	    	break;
		}
	}
}
