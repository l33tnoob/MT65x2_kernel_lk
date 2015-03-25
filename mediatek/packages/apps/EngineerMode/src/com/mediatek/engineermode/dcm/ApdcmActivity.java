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

package com.mediatek.engineermode.dcm;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;
import com.mediatek.engineermode.ShellExe;

import java.io.File;
import java.io.IOException;
import java.util.List;

public class ApdcmActivity extends Activity implements OnClickListener {

	private static final String TAG = "APDCM";
	private static final int APDCM_NUM = 7;
	private static final String FS_DBG = "/proc/dcm/dbg";
	private static final String FS_DUMPREGS = "/proc/dcm/dumpregs";
	private static final String FS_HELP = "/proc/dcm/help";
    private static final String CAT = "cat ";
    private static final String CMD_SET_DCM = 
    	    "echo 1 0 %1$d %2$s > " + FS_DBG;
	
	private EditText mEditArmDcm;
	private EditText mEditEmiDcm;
	private EditText mEditInfraDcm;
	private EditText mEditPeriDcm;
	private EditText mEditMiscDcm;
	private EditText mEditMmDcm;
	private EditText mEditMfgDcm;
	
	private Button mBtnArmRead;
	private Button mBtnArmSet;
	private Button mBtnEmiRead;
	private Button mBtnEmiSet;
	private Button mBtnInfraRead;
	private Button mBtnInfraSet;
	private Button mBtnPeriRead;
	private Button mBtnPeriSet;
	private Button mBtnMiscRead;
	private Button mBtnMiscSet;
	private Button mBtnMmRead;
	private Button mBtnMmSet;
	private Button mBtnMfgRead;
	private Button mBtnMfgSet;
	private Button mBtnDumpRegs;
	private Button mBtnHelp;
	private EditText[] mApdcmEdits = new EditText[APDCM_NUM];
	private String[] mApdcmTags = new String[APDCM_NUM];
	private String mDcmStr;
	@Override
    protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.dcm_apdcm);
		
		mEditArmDcm = (EditText)findViewById(R.id.dcm_apdcm_arm_edit);
		mEditEmiDcm = (EditText)findViewById(R.id.dcm_apdcm_emi_edit);
		mEditInfraDcm = (EditText)findViewById(R.id.dcm_apdcm_infra_edit);
		mEditPeriDcm = (EditText)findViewById(R.id.dcm_apdcm_peri_edit);
		mEditMiscDcm = (EditText)findViewById(R.id.dcm_apdcm_misc_edit);
		mEditMmDcm = (EditText)findViewById(R.id.dcm_apdcm_mm_edit);
		mEditMfgDcm = (EditText)findViewById(R.id.dcm_apdcm_mfg_edit);
		mApdcmEdits[0] = mEditArmDcm;
		mApdcmEdits[1] = mEditEmiDcm;
		mApdcmEdits[2] = mEditInfraDcm;
		mApdcmEdits[3] = mEditPeriDcm;
		mApdcmEdits[4] = mEditMiscDcm;
		mApdcmEdits[5] = mEditMmDcm;
		mApdcmEdits[6] = mEditMfgDcm;
		
		mBtnArmRead = (Button)findViewById(R.id.dcm_apdcm_arm_read_btn);
		mBtnArmRead.setOnClickListener(this);
		mBtnEmiRead = (Button)findViewById(R.id.dcm_apdcm_emi_read_btn);
		mBtnEmiRead.setOnClickListener(this);		
		mBtnInfraRead = (Button)findViewById(R.id.dcm_apdcm_infra_read_btn);
		mBtnInfraRead.setOnClickListener(this);		
		mBtnPeriRead = (Button)findViewById(R.id.dcm_apdcm_peri_read_btn);
		mBtnPeriRead.setOnClickListener(this);
		mBtnMiscRead = (Button)findViewById(R.id.dcm_apdcm_misc_read_btn);
		mBtnMiscRead.setOnClickListener(this);		
		mBtnMmRead = (Button)findViewById(R.id.dcm_apdcm_mm_read_btn);
		mBtnMmRead.setOnClickListener(this);		
		mBtnMfgRead = (Button)findViewById(R.id.dcm_apdcm_mfg_read_btn);
		mBtnMfgRead.setOnClickListener(this);	
		
		mBtnArmSet = (Button)findViewById(R.id.dcm_apdcm_arm_set_btn);
		mBtnArmSet.setOnClickListener(this);
		mBtnEmiSet = (Button)findViewById(R.id.dcm_apdcm_emi_set_btn);
		mBtnEmiSet.setOnClickListener(this);
		mBtnInfraSet = (Button)findViewById(R.id.dcm_apdcm_infra_set_btn);
		mBtnInfraSet.setOnClickListener(this);
		mBtnPeriSet = (Button)findViewById(R.id.dcm_apdcm_peri_set_btn);
		mBtnPeriSet.setOnClickListener(this);
		mBtnMiscSet = (Button)findViewById(R.id.dcm_apdcm_misc_set_btn);
		mBtnMiscSet.setOnClickListener(this);
		mBtnMmSet = (Button)findViewById(R.id.dcm_apdcm_mm_set_btn);
		mBtnMmSet.setOnClickListener(this);
		mBtnMfgSet = (Button)findViewById(R.id.dcm_apdcm_mfg_set_btn);
		mBtnMfgSet.setOnClickListener(this);
		
		mBtnDumpRegs = (Button)findViewById(R.id.dcm_apdcm_dump_regs_btn);
		mBtnDumpRegs.setOnClickListener(this);
		mBtnHelp = (Button)findViewById(R.id.dcm_apdcm_help_btn);
		mBtnHelp.setOnClickListener(this);
		
		mApdcmTags[0] = getString(R.string.dcm_text_arm);
		mApdcmTags[1] = getString(R.string.dcm_text_emi);
		mApdcmTags[2] = getString(R.string.dcm_text_infra);
		mApdcmTags[3] = getString(R.string.dcm_text_peri);
		mApdcmTags[4] = getString(R.string.dcm_text_misc);
		mApdcmTags[5] = getString(R.string.dcm_text_mm);
		mApdcmTags[6] = getString(R.string.dcm_text_mfg);
		// init edit hint info
		mDcmStr = getString(R.string.dcm_label);
		for (int i = 0; i < APDCM_NUM; i++) {
			mApdcmEdits[i].setHint(mApdcmTags[i] + " " + mDcmStr);
		}
		setUiByData(APDCM_NUM, true);
	}
	
	private void setUiByData(int apdcmIdx, boolean showDlg) {
		String cmd;
		String output;
		cmd = CAT + FS_DBG;
	    output = execCommand(cmd);
	    // temp test code
		//output = "ARM_DCM=0x7410, \nEMI_DCM=0xdc71,\nINFRA_DCM=0xab43, \nPERI_DCM=0xef56,\nMISC_DCM=0x1569,\nMM_DCM=0x4321 , \nMFG_DCM=0x1234 , \n ";
	    if (output == null) {
	        Toast.makeText(this, "Feature Fail or Don't Support!", Toast.LENGTH_SHORT).show();
	        finish();
	        return;
	    }
	    output = output.trim();
		if (showDlg) {
	        showDialog(mDcmStr, output);
	    }
	    resolveFillData(output, apdcmIdx);
	}
	
	private boolean resolveFillData(String outStr, int fillIdx) {
		String[] entries = outStr.split(" *, *\n *");
		if (entries.length != APDCM_NUM) {
			Xlog.e(TAG, "resolveFillData() Resolve outStr fail, Invalid DCM Number");
			return false;
		}
		String[] keyValPair;
		String val;
		for (int i = 0; i < APDCM_NUM; i++) {
			if (fillIdx == APDCM_NUM || i == fillIdx) {
			    keyValPair = entries[i].split(" *= *");
			    val = keyValPair[1];
			    if (val.contains(",")) {
			        val = val.substring(0, val.indexOf(","));
			        val = val.trim();
			    }
			    if (val.indexOf("0x") == 0 || val.indexOf("0X") == 0) {
			    	val = val.substring(2);
			    }
			    mApdcmEdits[i].setText(val);
			}
		}
		return true;
	}
	
	private void handleClickReadBtn(int apdcmIdx) {
		String msg;
		setUiByData(apdcmIdx, false);
		msg = mApdcmTags[apdcmIdx] + " " +  getString(R.string.dcm_text_read) + " " +
		        getString(R.string.dcm_operate_success);
		Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
	}
	
	private long parseLongStr(String longStr, int radix) {
		long longVal = -1;
		try {
			longVal = Long.parseLong(longStr, radix);
		} catch(NumberFormatException e) {
			return -1;
		} 
		return longVal;
	}
	
	private void handleClickSetBtn(int apdcmIdx) {
		String msg;
		String dcmStr;
		if (TextUtils.isEmpty(mApdcmEdits[apdcmIdx].getText())) {
			msg = getString(R.string.dcm_invalid_dcm) ;
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return;
		}
		
		dcmStr = mApdcmEdits[apdcmIdx].getText().toString();
		long dcmVal = parseLongStr(dcmStr, 16);
		if (dcmVal < 0 || dcmVal > parseLongStr("ffffffff", 16)) {
			msg = getString(R.string.dcm_invalid_dcm) + " " + mApdcmTags[apdcmIdx] + " DCM:" + dcmStr;
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return;
		}
		String cmd;
		cmd = String.format(CMD_SET_DCM, apdcmIdx, dcmStr);
		execCommand(cmd);
		msg = mApdcmTags[apdcmIdx] + " " +  getString(R.string.dcm_text_set) + " " +
		        getString(R.string.dcm_operate_success);
		Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
	}
	
	@Override
	public void onClick(View view) {
		String cmd;
		String output;
		String title;
		int idx = view.getId();
		switch(idx) {
		case R.id.dcm_apdcm_dump_regs_btn:
			cmd = CAT + FS_DUMPREGS;
			output = execCommand(cmd);
			title = getString(R.string.dcm_text_dump_regs);
			showDialog(title, output);
			break;
		case R.id.dcm_apdcm_help_btn:
			cmd = CAT + FS_HELP;
			output = execCommand(cmd);
			title = getString(R.string.dcm_text_help);
			showDialog(title, output);
			break;
	    // below handle read button
		case R.id.dcm_apdcm_arm_read_btn:
			handleClickReadBtn(0);
		    break;
		case R.id.dcm_apdcm_emi_read_btn:
			handleClickReadBtn(1);
		    break;
		case R.id.dcm_apdcm_infra_read_btn:
			handleClickReadBtn(2);
		    break;
		case R.id.dcm_apdcm_peri_read_btn:
			handleClickReadBtn(3);
		    break;
		case R.id.dcm_apdcm_misc_read_btn:
			handleClickReadBtn(4);
		    break;
		case R.id.dcm_apdcm_mm_read_btn:
			handleClickReadBtn(5);
		    break;
		case R.id.dcm_apdcm_mfg_read_btn:
			handleClickReadBtn(6);
		    break;
		// below handle set button
		case R.id.dcm_apdcm_arm_set_btn:
			handleClickSetBtn(0);
			break;
		case R.id.dcm_apdcm_emi_set_btn:
			handleClickSetBtn(1);
			break;
		case R.id.dcm_apdcm_infra_set_btn:
			handleClickSetBtn(2);
			break;
		case R.id.dcm_apdcm_peri_set_btn:
			handleClickSetBtn(3);
			break;
		case R.id.dcm_apdcm_misc_set_btn:
			handleClickSetBtn(4);
			break;
		case R.id.dcm_apdcm_mm_set_btn:
			handleClickSetBtn(5);
			break;
		case R.id.dcm_apdcm_mfg_set_btn:
			handleClickSetBtn(6);
			break;
		default:
			Xlog.w(TAG, "onClick() Unknown view id: " + idx);
			break;
		}
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
	
}
