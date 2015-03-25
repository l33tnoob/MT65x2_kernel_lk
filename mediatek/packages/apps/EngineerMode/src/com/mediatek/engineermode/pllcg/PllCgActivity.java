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

package com.mediatek.engineermode.pllcg;

import java.io.File;
import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

public class PllCgActivity extends Activity implements OnClickListener{
	private static final String TAG = "PllCg";
	private static final String FS_CLK_TEST = "/proc/clkmgr/clk_test";
	private static final String FS_PLL_TEST = "/proc/clkmgr/pll_test";
	private static final String CMD_CLKMUX = 
		    "echo clkmux %1$s %2$s EM > " + FS_CLK_TEST;
	private static final String CMD_CGID_ENABLE = 
		    "echo enable %1$s EM > " + FS_CLK_TEST;
	private static final String CMD_CGID_DISABLE = 
		    "echo disable %1$s EM > " + FS_CLK_TEST;
	private static final String CAT = "cat ";
	private Button mBtnClkmux;
	private Button mBtnEnable;
	private Button mBtnDisable;
	private Button mBtnReadAll;
	private EditText mEditClkmuxId;
	private EditText mEditCgId;
	@Override
	 protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.pll_cg);
		mBtnClkmux = (Button)findViewById(R.id.pllcg_clkmux_btn);
		mBtnClkmux.setOnClickListener(this);
		mBtnEnable = (Button)findViewById(R.id.pllcg_enable_btn);
		mBtnEnable.setOnClickListener(this);
		mBtnDisable = (Button)findViewById(R.id.pllcg_disable_btn);
		mBtnDisable.setOnClickListener(this);
		mBtnReadAll = (Button)findViewById(R.id.pllcg_read_all_btn);
		mBtnReadAll.setOnClickListener(this);
		mEditClkmuxId = (EditText)findViewById(R.id.pllcg_clkmux_id_edit);
		mEditCgId = (EditText)findViewById(R.id.pllcg_cg_id_edit);
		if (!(new File(FS_CLK_TEST).exists())) {
		    Toast.makeText(this, "Don't Support This Feature!", Toast.LENGTH_SHORT).show();
		    finish();
		    return;
		}
	}
	@Override
	public void onClick(View view) {
		String cmd;
		StringBuilder output = new StringBuilder();
		switch(view.getId()) {
		case R.id.pllcg_clkmux_btn:
			setClkmux();
			break;
		case R.id.pllcg_enable_btn:
			controlCgId(true);
			break;
		case R.id.pllcg_disable_btn:
			controlCgId(false);
			break;
		case R.id.pllcg_read_all_btn:
			cmd = CAT + FS_CLK_TEST;
			output.append(execCommand(cmd));
			cmd = CAT + FS_PLL_TEST;
			output.append(execCommand(cmd));
			showDialog(getString(R.string.pllcg_all_info),output.toString());
			break;
		default:
			Xlog.w(TAG, "Unknown view id: " + view.getId());
			break;
		}
	}
	
	private void setClkmux() {
		String clkmuxId;
		String cgId;
		String msg;
		String cmd;
		
		if (TextUtils.isEmpty(mEditClkmuxId.getText()) || 
				TextUtils.isEmpty(mEditCgId.getText())) {
			msg = getString(R.string.pllcg_no_enough_input);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return;
		}
		clkmuxId = mEditClkmuxId.getText().toString();
		cgId = mEditCgId.getText().toString();
		cmd = String.format(CMD_CLKMUX, clkmuxId, cgId);
		execCommand(cmd);
	}
	
	private void controlCgId(boolean enabled) {
		String clkmuxId;
		String cgId;
		String msg;
		String cmd;
		if (TextUtils.isEmpty(mEditCgId.getText())) {
			msg = getString(R.string.pllcg_no_enough_input);
			Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
			return;
		}
		cgId = mEditCgId.getText().toString();
		if (enabled) {
		    cmd = String.format(CMD_CGID_ENABLE, cgId);
		} else {
			cmd = String.format(CMD_CGID_DISABLE, cgId);
		}
		execCommand(cmd);
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
	
	private void showDialog(String title, String msg) {
		AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                false).setTitle(title).setMessage(msg).setCancelable(true).
                setPositiveButton(android.R.string.ok, null).create();
		dialog.show();
	}

}
