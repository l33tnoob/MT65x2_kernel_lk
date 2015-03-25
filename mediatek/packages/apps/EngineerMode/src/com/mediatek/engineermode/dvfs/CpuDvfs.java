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

package com.mediatek.engineermode.dvfs;

import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

public class CpuDvfs extends Activity implements OnItemClickListener, 
        DialogInterface.OnClickListener, OnClickListener{
	private static final String TAG = "CpuDvfs";
	private static final String FS_GOVERNOR_LIST = 
		    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors";
	private static final String FS_GOVERNOR_DEFAULT =
		    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
	private static final String FS_SET_SPEED = 
		    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed";
//	private static final String FS_MAX_SPEED =
//		    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
//	private static final String FS_MIN_SPEED = 
//		    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq";
	private static final String FS_CUR_FREQ = 
		    "/proc/cpufreq/cpufreq_cur_freq";
	private static final String ECHO = "echo";
	private static final String CAT = "cat ";
	private static final String CMD_PERFORMANCE_GOVERNOR = 
		    "echo performance > " + FS_GOVERNOR_DEFAULT;
	private static final String CMD_SET_CPU_SPEED = 
		    "echo %1$s > " + FS_CUR_FREQ;
	private ListView mLVCpuGovernor;
	private EditText mEditCpuClockVal;
	private Button mBtnSetFreq;
	private CharSequence[] mGovernorItems;
	private long mFreqMin = -1;
	private long mFreqMax = -1;

	@Override
	 protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.cpu_dvfs);
		mLVCpuGovernor = (ListView)findViewById(R.id.dvfs_cpu_governor);
		mLVCpuGovernor.setOnItemClickListener(this);
		mEditCpuClockVal = (EditText)findViewById(R.id.dvfs_cpu_clock);
		mBtnSetFreq = (Button)findViewById(R.id.dvfs_set_cpu_freq);
		mBtnSetFreq.setOnClickListener(this);
		String lblGovernor = getString(R.string.dvfs_cpu_governor);
		String[] arrGovernor = new String[1];
		arrGovernor[0] = lblGovernor;
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, 
				android.R.layout.simple_list_item_1, arrGovernor);
		mLVCpuGovernor.setAdapter(adapter);
		initUIByData();
	}
	
//	private void getMaxMinFreq() {
//		String cmd;
//		String output;
//		cmd = CAT + FS_MIN_SPEED;
//		output = execCommand(cmd);
//		output = output.trim();
//		try {
//			mFreqMin = Long.parseLong(output);
//		} catch(NumberFormatException e) {
//            Xlog.e(TAG, "NumberFormatExceptin: Fail to get min cpu speed; " + e.getMessage());
//		}
//		
//		cmd = CAT + FS_MAX_SPEED;
//		output = execCommand(cmd);
//		output = output.trim();
//		try {
//			mFreqMax = Long.parseLong(output);
//		} catch(NumberFormatException e) {
//            Xlog.e(TAG, "NumberFormatExceptin: Fail to get max cpu speed; " + e.getMessage());
//		}
//
//	}
	
	private boolean validateSetting() {
		long cpuClock = -1;
		String msg;
		try {
			cpuClock = Long.parseLong(mEditCpuClockVal.getText().toString());
		} catch(NumberFormatException e) {
			msg = getString(R.string.dvfs_invalid_clock);
			Toast.makeText(this, msg , Toast.LENGTH_SHORT).show();
			return false;
		}
		
		if (cpuClock < mFreqMin || cpuClock > mFreqMax) {
			msg = getString(R.string.dvfs_invalid_clock_range);
			msg = String.format(msg, mFreqMin, mFreqMax);
			Toast.makeText(this, msg , Toast.LENGTH_SHORT).show();
			return false;
		} 
		return true;
	}
	
	private void initUIByData() {
		String cmd;
		String output;
		cmd = CAT + FS_CUR_FREQ;
		output = execCommand(cmd);
		//output = "Current Freq : 497250\n [0] 1209000 KHZ\n [1] 754000 KHZ\n [2] 497250 KHZ \n ";
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
		output = output.trim();
		String[] freqInfos = output.split(" *\n *");
		String dftFreq = freqInfos[0];
		String[] keyVal = dftFreq.split(" *: *");
		mEditCpuClockVal.setText(keyVal[1]);
		for (int i = 1; i < freqInfos.length; i++) {
			String[] freqParts = freqInfos[i].split(" +");
			long freqVal = -1;
			try {
				freqVal = Long.parseLong(freqParts[1]);
			} catch (NumberFormatException e) {
				Xlog.e(TAG, "NumberFormatExcaption: parse available freq fail: " + freqParts[1]);
			}
			if (mFreqMax == -1) {
				mFreqMax = freqVal;
			} else if (freqVal > mFreqMax) { 
				mFreqMax = freqVal;	
			}
			if (mFreqMin == -1) {
				mFreqMin = freqVal;
			} else if (freqVal < mFreqMin) {
				mFreqMin = freqVal;
			}
		}
	}
	
	private void showSelectDialog(String title, CharSequence[] items, 
			int checkedId, DialogInterface.OnClickListener listener) {
		AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                false).setTitle(title).setSingleChoiceItems(items, checkedId, listener).
                setNegativeButton(android.R.string.cancel, null).create();
		dialog.show();
	}
	
	 private String execCommand(String cmd) {
		 int ret = -1;
		 Xlog.d(TAG, "[cmd]:" + cmd);
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
	 
	 private CharSequence[] getGovernorAvailList() {
		 String cmd = CAT + " " + FS_GOVERNOR_LIST;
		 String output = execCommand(cmd);
		 if (output == null) {
		     return new CharSequence[]{};
		 }
		 output = output.trim();
		 String[] items = output.split(" +");
		 CharSequence[] charItems;
		 if (items.length > 0) {
			 charItems = new CharSequence[items.length];
			 for (int i = 0; i < items.length; i++) {
				 charItems[i] = items[i];
			 }
		 } else {
			 charItems = new CharSequence[0];
		 }
		 return charItems;
	 }
	 
	 private int getGovernorIndex(CharSequence[] items) {
		 if (items == null) {
			 return -1;
		 }
		 String target;
		 String cmd = CAT + " " + FS_GOVERNOR_DEFAULT;
		 target = execCommand(cmd);
		 for (int i = 0; i < items.length; i++) {
			 if (items[i].equals(target)) {
				 return i;
			 }
		 }
		 return -1;
	 }

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		int index;
		switch(position) {
		case 0:
			mGovernorItems = getGovernorAvailList();
			index = getGovernorIndex(mGovernorItems);
			showSelectDialog(getString(R.string.dvfs_governor_choose), mGovernorItems, index, this);
			break;
	    default:
	    	Xlog.w(TAG, "unknown position: " + position);
	    	break;
		}
	}
	
	private void setGovernor(String item) {
		String cmd = ECHO + " " + item + " > " + FS_GOVERNOR_DEFAULT;
		Xlog.d(TAG, "SetGovernor: " + cmd);
		execCommand(cmd);
	}
	
	private void setCpuSpeed(String speed) {
		String cmd;

		cmd = CMD_PERFORMANCE_GOVERNOR;
		execCommand(cmd);
		
		cmd = String.format(CMD_SET_CPU_SPEED, speed);
		execCommand(cmd);
	}

	@Override
	public void onClick(DialogInterface dialog, int which) {
		CharSequence checkItem = mGovernorItems[which];
		setGovernor(checkItem.toString());
		dialog.dismiss();
	}

	@Override
	public void onClick(View view) {
		if (view.getId() == R.id.dvfs_set_cpu_freq) {
			if (validateSetting()) {
				setCpuSpeed(mEditCpuClockVal.getText().toString());
				String msg = getString(R.string.dvfs_success);
				Toast.makeText(this, msg , Toast.LENGTH_SHORT).show();
			}
		}
	}
}
