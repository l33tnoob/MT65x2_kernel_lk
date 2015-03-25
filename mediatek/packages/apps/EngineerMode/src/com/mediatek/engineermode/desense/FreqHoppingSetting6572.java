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

package com.mediatek.engineermode.desense;

import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;
import android.widget.AdapterView.OnItemSelectedListener;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

public class FreqHoppingSetting6572 extends Activity implements OnClickListener,
        OnItemSelectedListener{

	private static final String TAG = "FreqHoppingSet";
	private static final String FS_DBG = "/proc/fhctl/dbg";
	private static final String FS_DUMPREG = "/proc/fhctl/dumpregs";
	private static final String FS_HELP = "/proc/fhctl/help";
	private static final String CAT = "cat ";
	private static final int IDX_SSC_EN = 0;
	private static final int IDX_HOP_EN = 1;
	private static final int IDX_DDS = 2;
	private static final int IDX_DDS_MON = 3;
	private static final int IDX_DF = 4;
	private static final int IDX_DT = 5;
	private static final int IDX_UP = 6;
	private static final int IDX_DN = 7;
	private static final int PLL_VAL_NUM = 8;
	private static final String CMD_SET_HOP = 
		    "echo 1 1 %1$s %2$s 0 0 0 0 > " + FS_DBG;
	private static final String CMD_TRIGGER_HOP = 
		    "echo 2 1 %1$d %2$s %3$s 0 0 0 > " + FS_DBG;
	private static final String CMD_DISABLE_HOP = 
		    "echo 2 0 %1$d %2$s 0 0 0 0 > " + FS_DBG;
	private static final String CMD_ENABLE_SSC =
		    "echo 3 1 %1$d %2$s %3$s %4$s %5$s %6$s > " + FS_DBG;
	private static final String CMD_DISABLE_SSC = 
		    "echo 3 0 %1$d %2$s 0 0 0 0 > " + FS_DBG;
	
	private EditText mEditHopDeltaFreq;
	private EditText mEditHopDeltaTime;
	private EditText mEditDds;
	private EditText mEditSscDeltaFreq;
	private EditText mEditSscDeltaTime;
	private EditText mEditSscUpLmt;
	private EditText mEditSscDnLmt;
	private EditText mEditHopDds;
	
	private Button mBtnHopSet;
	private Button mBtnEnableSsc;
	private Button mBtnDisableSsc;
	private Button mBtnTriggerHop;
	private Button mBtnDisableHop;
	private Button mBtnDumpAll;
	private Button mBtnHelp;
	
	private Spinner mSpnPll;
	private String[] mPllsTag;
	private String[] mPllValEntrys = new String[PLL_VAL_NUM];
	
	@Override
    protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.desense_freqhopping_set_6572);
		
		mEditHopDeltaFreq = (EditText)findViewById(R.id.desense_freqhopping_hop_deltafreq_edit);
		mEditHopDeltaTime = (EditText)findViewById(R.id.desense_freqhopping_hop_deltatime_edit);
		mEditDds = (EditText)findViewById(R.id.desense_freqhopping_ssc_dds_edit);
		mEditSscDeltaFreq = (EditText)findViewById(R.id.desense_freqhopping_ssc_deltafreq_edit);
		mEditSscDeltaTime = (EditText)findViewById(R.id.desense_freqhopping_ssc_deltatime_edit);
		mEditSscUpLmt = (EditText)findViewById(R.id.desense_freqhopping_ssc_upperlimit_edit);
		mEditSscDnLmt = (EditText)findViewById(R.id.desense_freqhopping_ssc_downlimit_edit);
		mEditHopDds = (EditText)findViewById(R.id.desense_freqhopping_hop_dds_edit);
		
		mBtnHopSet = (Button)findViewById(R.id.desense_freqhopping_hop_set_btn);
		mBtnHopSet.setOnClickListener(this);
		mBtnEnableSsc = (Button)findViewById(R.id.desense_freqhopping_ssc_enable_btn);
		mBtnEnableSsc.setOnClickListener(this);
		mBtnDisableSsc = (Button)findViewById(R.id.desense_freqhopping_ssc_disable_btn);
		mBtnDisableSsc.setOnClickListener(this);
		mBtnTriggerHop = (Button)findViewById(R.id.desense_freqhopping_trigger_hop_btn);
		mBtnTriggerHop.setOnClickListener(this);
		mBtnDisableHop = (Button)findViewById(R.id.desense_freqhopping_disable_hop_btn);
		mBtnDisableHop.setOnClickListener(this);
		mBtnDumpAll = (Button)findViewById(R.id.desense_freqhopping_dump_all_btn);
		mBtnDumpAll.setOnClickListener(this);
		mBtnHelp = (Button)findViewById(R.id.desense_freqhopping_help_btn);
		mBtnHelp.setOnClickListener(this);
		
		mSpnPll = (Spinner)findViewById(R.id.desense_freqhopping_plls);
		mSpnPll.setOnItemSelectedListener(this);
		mPllsTag = getResources().getStringArray(R.array.desense_freqhopping_plls_6572);
		
	}
	
	private void handleClickHopSet(Button btn) {
		String hopDeltaFreq;
		String hopDeltaTime;
		String cmd;
		if (TextUtils.isEmpty(mEditHopDeltaFreq.getText()) || 
				TextUtils.isEmpty(mEditHopDeltaTime.getText())) {
			Toast.makeText(this, getString(R.string.desense_freqhopping_invalid_input), 
					Toast.LENGTH_SHORT).show();
			return;
		}
		hopDeltaFreq = mEditHopDeltaFreq.getText().toString();
		hopDeltaTime = mEditHopDeltaTime.getText().toString();
		Long freq = parseHexStr(hopDeltaFreq);
		Long time = parseHexStr(hopDeltaTime);
		if (freq >= 0 && freq <= 0x1fffff && time >= 0 && time <= 0xff) {
			cmd = String.format(CMD_SET_HOP, hopDeltaFreq, hopDeltaTime);
			execCommand(cmd);
			String msg = btn.getText().toString() + " " + getString(R.string.desense_freqhopping_operate_success);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
		} else {
			Toast.makeText(this, getString(R.string.desense_freqhopping_invalid_input), 
					Toast.LENGTH_SHORT).show();
		}
	}
	
	private void handleClickEnableSsc(Button btn) {
		if (TextUtils.isEmpty(mEditSscDeltaFreq.getText()) || 
				TextUtils.isEmpty(mEditSscDeltaTime.getText()) || 
				TextUtils.isEmpty(mEditSscUpLmt.getText()) ||
				TextUtils.isEmpty(mEditSscDnLmt.getText())) {
			Toast.makeText(this, getString(R.string.desense_freqhopping_invalid_input), 
					Toast.LENGTH_SHORT).show();
			return;
		}
		String sscDF;
		String sscDT;
		String sscUpLmt;
		String sscDnLmt;
		long sscDFVal;
		long sscDTVal;
		int sscUpLmtVal;
		int sscDnLmtVal;
		sscDF = mEditSscDeltaFreq.getText().toString();
		sscDT = mEditSscDeltaTime.getText().toString();
		sscUpLmt = mEditSscUpLmt.getText().toString();
		sscDnLmt = mEditSscDnLmt.getText().toString();
		sscDFVal = parseHexStr(sscDF);
		sscDTVal = parseHexStr(sscDT);
		sscUpLmtVal = parseDecStr(sscUpLmt);
		sscDnLmtVal = parseDecStr(sscDnLmt);
		if (sscDFVal < 0 || sscDFVal > 0xf ||
				sscDTVal < 0 || sscDTVal > 0xf ||
				sscUpLmtVal < 0 || sscUpLmtVal > 99 ||
				sscDnLmtVal < 0 || sscDnLmtVal > 99) {
			Toast.makeText(this, getString(R.string.desense_freqhopping_invalid_input), 
					Toast.LENGTH_SHORT).show();
			return;
		}
		int pllIdx = mSpnPll.getSelectedItemPosition();
		String cmd;
		cmd = String.format(CMD_ENABLE_SSC, pllIdx, mPllValEntrys[IDX_DDS], 
				sscDF, sscDT, sscUpLmt, sscDnLmt);
		execCommand(cmd);
		String msg = btn.getText().toString() + " " + getString(R.string.desense_freqhopping_operate_success);
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
		mBtnEnableSsc.setEnabled(false);
		mBtnDisableSsc.setEnabled(true);
	}
	
	private void handleClickDisableSsc(Button btn) {
		int pllIdx = mSpnPll.getSelectedItemPosition();
		String cmd;
		cmd = String.format(CMD_DISABLE_SSC, pllIdx, mPllValEntrys[IDX_DDS]);
		execCommand(cmd);
		String msg = btn.getText().toString() + " " + getString(R.string.desense_freqhopping_operate_success);
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
		mBtnEnableSsc.setEnabled(true);
		mBtnDisableSsc.setEnabled(false);
	}
	
	private void handleClickTriggerHop(Button btn) {
		String hopDds;
		if (TextUtils.isEmpty(mEditHopDds.getText())) {
			Toast.makeText(this, getString(R.string.desense_freqhopping_invalid_input), 
					Toast.LENGTH_SHORT).show();
			return;
		}
		hopDds = mEditHopDds.getText().toString();
		long hopDdsVal = parseHexStr(hopDds);
		if (hopDdsVal < 0 || hopDdsVal > 0x1fffff) {
			Toast.makeText(this, getString(R.string.desense_freqhopping_invalid_input), 
					Toast.LENGTH_SHORT).show();
			return;
		}
		int pllIdx = mSpnPll.getSelectedItemPosition();
		String cmd;
		cmd = String.format(CMD_TRIGGER_HOP, pllIdx, mPllValEntrys[IDX_DDS], hopDds);
		execCommand(cmd);
		String msg = btn.getText().toString() + " " + getString(R.string.desense_freqhopping_operate_success);
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
	}
	
	private void handleClickDisableHop(Button btn) {
		int pllIdx = mSpnPll.getSelectedItemPosition();
		String cmd;
		cmd = String.format(CMD_DISABLE_HOP, pllIdx, mPllValEntrys[IDX_DDS]);
		execCommand(cmd);
		String msg = btn.getText().toString() + " " + getString(R.string.desense_freqhopping_operate_success);
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
	}

	@Override
	public void onClick(View view) {
		String cmd;
		String output;
		switch(view.getId()) {
		case R.id.desense_freqhopping_hop_set_btn:
			handleClickHopSet((Button)view);
			break;
		case R.id.desense_freqhopping_ssc_enable_btn:
			handleClickEnableSsc((Button)view);
			break;
		case R.id.desense_freqhopping_ssc_disable_btn:
			handleClickDisableSsc((Button)view);
			break;
		case R.id.desense_freqhopping_trigger_hop_btn:
			handleClickTriggerHop((Button)view);
			break;
		case R.id.desense_freqhopping_disable_hop_btn:
			handleClickDisableHop((Button)view);
			break;
		case R.id.desense_freqhopping_dump_all_btn:
			cmd = CAT + FS_DUMPREG;
			output = execCommand(cmd);
			showDialog(getString(R.string.desense_freqhopping_dump_all), output);
			break;
		case R.id.desense_freqhopping_help_btn:
			cmd = CAT + FS_HELP;
			output = execCommand(cmd);
			showDialog("Help", output);
			break;
		default:
			Xlog.w(TAG, "onClick() unknown view id:" + view.getId());
		    break;	
		}
	}
	
	private void handleSelectPll(int index) {
		// clear Edit
		mEditHopDds.setText("");
		
		String cmd;
		String output;
		cmd = CAT + FS_DBG;
		output = execCommand(cmd);
		//output = "ARMPLL : ssc_en =0, hop_en=0, dds=0x0BA000, dds_mon=0x000000, df=0x0, dt=0x0, up= 0, dn= 0 \nMAINPLL : ssc_en =0, hop_en=0, dds=0x456700, dds_mon=0x000000, df=0x0, dt=0x0, up= 0, dn= 0";
        if (output == null) {
            Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
		showDialog(mPllsTag[index], output);
		output = output.trim();
		resolvePllDataById(output, index);
		if ("0".equals(mPllValEntrys[IDX_SSC_EN])) {
			mBtnEnableSsc.setEnabled(true);
			mBtnDisableSsc.setEnabled(false);
		} else {
			mBtnEnableSsc.setEnabled(false);
			mBtnDisableSsc.setEnabled(true);
		}
		mEditDds.setText(mPllValEntrys[IDX_DDS]);
		mEditSscDeltaFreq.setText(mPllValEntrys[IDX_DF]);
		mEditSscDeltaTime.setText(mPllValEntrys[IDX_DT]);
		mEditSscUpLmt.setText(mPllValEntrys[IDX_UP]);
		mEditSscDnLmt.setText(mPllValEntrys[IDX_DN]);
	}
	
	private long parseHexStr(String hexStr) {
		long hexVal = -1;
		try {
			hexVal = Long.parseLong(hexStr, 16);
		} catch(NumberFormatException e) {
			return -1;
		} 
		return hexVal;
	}
	
	private int parseDecStr(String decStr) {
		int decVal = -1;
		try {
			decVal = Integer.parseInt(decStr);
		} catch(NumberFormatException e) {
			return -1;
		} 
		return decVal;
	}
	
	private void resolvePllDataById(String outStr, int index) {
		String output;
		output = outStr;
		String[] pllInfos = output.split(" *\n *");
		if (pllInfos.length != 2) {
			Xlog.e(TAG, "resolve PLL Data fail");
			return;
		}
		pllInfos[0] = pllInfos[0].trim();
		pllInfos[0] = pllInfos[0].substring(pllInfos[0].indexOf("ssc_en"));
		pllInfos[1] = pllInfos[1].trim();
		pllInfos[1] = pllInfos[1].substring(pllInfos[1].indexOf("ssc_en"));
		switch(index) {
		case 0:
			resolvePllData(pllInfos[0]);
			break;
		case 1:
			resolvePllData(pllInfos[1]);
			break;
		default:
			Xlog.w(TAG, "resolvePllDataById() Unknown index: " + index);
			break;
		}
	}
	
	/**
	 * infoStr format:
	 * "ssc_en =??, hop_en=??, dds=0x??, dds_mon=0x??, df=0x??, dt=0x??, up=??, dn=??"
	 */
	private boolean resolvePllData(String infoStr) {
		String[] pairs;
		pairs = infoStr.split(" *, *");
		if (pairs.length != PLL_VAL_NUM) {
			Xlog.e(TAG, "resolvePllData() resolve fail, unknown format");
			return false;
		}
		String[] entry;
		for (int i = 0; i < PLL_VAL_NUM; i++) {
			entry = pairs[i].split(" *= *");
			mPllValEntrys[i] = entry[1];
			if (mPllValEntrys[i].indexOf("0x") == 0 || mPllValEntrys[i].indexOf("0X") == 0) {
	        	mPllValEntrys[i] = mPllValEntrys[i].substring(2);
			} 
		}

		return true;
	}
	
	private String execCommand(String cmd) {
		 int ret = -1;
		 Xlog.d(TAG, "[cmd]:" + cmd);
		 //Toast.makeText(this, cmd, Toast.LENGTH_LONG).show();
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
                false).setTitle(title).setMessage(msg).
                setPositiveButton(android.R.string.ok, null).create();
		dialog.show();
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		
		handleSelectPll(position);
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		Xlog.d(TAG, "onNothingSelected() + " + parent.toString());
		
	}
}
