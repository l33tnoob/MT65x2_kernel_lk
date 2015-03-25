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

package com.mediatek.engineermode.lcm;

import android.R.integer;
import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class LCM extends Activity implements OnClickListener {
	private EditText mCycleText ;
	private EditText mMipiText ;
	private ToggleButton mTmToggleButton;
	private Button mSetButton;
	
	private Handler mHandler = null;
	
	// flags
	private boolean mbIsDialogShowed = false;
	private boolean mbIsBTPoweredOff = true;
	private boolean mbIsDataDetected = false;
	// dialog ID and MSG ID
	private static final int DIALOG_CHECK_DATA = 100;
	private static final int CHECK_DATA_FINISHED = 0x10;
	
	private String mTmStatus = "OFF";
	private String mCycleString = "12";
	private String mMipiString = "50";
	
	//0 DBI, 1 DPI, 2 MIPI
	private int mInterfaceType = 0;
	
	private String TAG = "EM/lcm";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
		Xlog.v(TAG, "-->onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_desense_lcm);
        
        mCycleText = (EditText) findViewById(R.id.lcm_cycle_edit);
        mMipiText = (EditText) findViewById(R.id.lcm_mipi_edit);
        
        mTmToggleButton = (ToggleButton)findViewById(R.id.toggleButton1);
        mSetButton = (Button) findViewById(R.id.lcm_set_btn);    
        
		if (mCycleText == null || mMipiText == null
				|| mTmToggleButton == null || mSetButton == null) {
			Xlog.w(TAG, "clocwork worked...");
		}	
        
//		mCycleText.setText("10");
//		mMipiText.setText("t00");
//		mTmToggleButton.setText("OFF");
		
		mTmToggleButton.setOnClickListener(this);
		mSetButton.setOnClickListener(this);
		
		mHandler = new Handler(){
			@Override
			public void handleMessage(Message msg) {
				// TODO Auto-generated method stub
				if (msg.what == CHECK_DATA_FINISHED) {
					if (mInterfaceType == 0) {
					    mCycleText.setText(mCycleString);
					    mMipiText.setEnabled(false);
					} else if (mInterfaceType == 2){
					    mMipiText.setText(mMipiString);
						mCycleText.setEnabled(false);
					}
					mTmToggleButton.setText(mTmStatus);
					if (mTmStatus.equalsIgnoreCase("ON")) {
					    mTmToggleButton.setChecked(true);
					}
					
					mbIsDataDetected = true;
					if (mbIsDialogShowed) {
						removeDialog(DIALOG_CHECK_DATA);
					}
				}
			}
			
		};
    }

	/* (non-Javadoc)
	 * @see android.app.Activity#onResume()
	 */
	@Override
	protected void onResume() {
		Xlog.v(TAG, "-->onResume");
		// TODO Auto-generated method stub
		super.onResume();
		mTmToggleButton.setText("ON");
		mTmToggleButton.setChecked(true);
		if (!mbIsDataDetected) {
			showDialog(DIALOG_CHECK_DATA);
			new Thread() {
				public void run() {
					try {
						/// ///0 DBI, 1 DPI, 2 MIPI
					    mInterfaceType = EmDsenseLcmAssit.LCMGetInterfaceType();
						Xlog.v(TAG, "-->onResume--LCMGetInterfaceType()="
								+ mInterfaceType);
						
						int tmValue = EmDsenseLcmAssit.LCMGetTm();						
						if (tmValue == 1) {
						    mTmStatus = "ON";
						}
						
						Xlog.v(TAG, "-->onResume--tmValue="+tmValue);
						int cycleValue = EmDsenseLcmAssit.LCDWriteCycleGetCurrentVal();
						mCycleString = ""+cycleValue;
						
						Xlog.v(TAG, "-->onResume--cycleValue=" + mCycleString);		
						
						int mipiValue = EmDsenseLcmAssit.LCMGetMipi();
						mMipiString =""+mipiValue;
						Xlog.v(TAG, "-->onResume--LCMGetMipi()="
								+ mMipiString);						
												
						Xlog.v(TAG, "-->onResume--LCDWriteCycleGetMinVal()="
								+EmDsenseLcmAssit.LCDWriteCycleGetMinVal());						

					} catch (Exception e) {
						// TODO: handle exception
						Xlog.d(TAG, "-->onResume--"+e.getMessage());
					}

					Message msg = new Message();
					msg.what = CHECK_DATA_FINISHED;
					mHandler.sendMessage(msg);
				}
			}.start();
			mbIsDialogShowed = true;
		}
	}

	/* (non-Javadoc)
	 * @see android.app.Activity#onStop()
	 */
	@Override
	protected void onStop() {
		Xlog.v(TAG, "-->onStop");
		// TODO Auto-generated method stub
		super.onStop();
	}

	@Override
	protected Dialog onCreateDialog(int id) {
		Xlog.v(TAG, "-->onCreateDialog");
		if (id == DIALOG_CHECK_DATA) {
			ProgressDialog dialog = new ProgressDialog(LCM.this);
			if (dialog != null) {
				dialog.setTitle("Progress");
				dialog.setMessage("Please wait for device to initialize ...");
				dialog.setCancelable(false);
				dialog.setIndeterminate(true);
				dialog.show();
				Xlog.i(TAG, "new ProgressDialog succeed");
			} else {
				Xlog.i(TAG, "new ProgressDialog failed");
			}

			return dialog;
		}

		return null;
	}

	public void onClick(View v) {
		// TODO Auto-generated method stub
		Xlog.v(TAG, "-->onClick");
		if (v.getId() == mTmToggleButton.getId()) {
			
		    mTmStatus = mTmToggleButton.getText().toString();
			Log.d(TAG, "-->onClick--mTmToggleButton--" + mTmStatus);
			if (mTmStatus.equalsIgnoreCase("ON")) {
				EmDsenseLcmAssit.LCMSetTm(1);
			}else if (mTmStatus.equalsIgnoreCase("OFF")) {
				EmDsenseLcmAssit.LCMSetTm(0);
			}
			
		}else if (v.getId() == mSetButton.getId()) {
			Xlog.v(TAG, "-->onClick--mSetButton");
			
			if (mInterfaceType == 0) {
				String tmpCycle = mCycleText.getText().toString();
				Xlog.v(TAG, "mCycleString = " + mCycleString + " new="+tmpCycle);
				if (!tmpCycle.equalsIgnoreCase(mCycleString)) {

					try {
						int cycleValue = Integer.parseInt(tmpCycle);
						if (cycleValue>4 &&cycleValue<56) {
//							mCycleString = ""+EmDsenseLcmAssit.LCDWriteCycleSetVal(cycleValue);
							if (0==(EmDsenseLcmAssit.LCDWriteCycleSetVal(cycleValue))) {
							    mCycleText.setText(""+cycleValue);
								Toast.makeText(getApplicationContext(),
										"set cycle number success:"+cycleValue,
										Toast.LENGTH_SHORT).show();
							}else {
								Toast.makeText(getApplicationContext(),
										"set cycle number return fail:"+cycleValue,
										Toast.LENGTH_SHORT).show();
							}


						} else {
							Xlog.d(TAG, "--cycleValue--is not in 5~55!");
							Toast.makeText(getApplicationContext(),
									"Please input cycle number between 5~55.",
									Toast.LENGTH_SHORT).show();
						}

					} catch (Exception e) {
						// TODO: handle exception
						Xlog.d(TAG, "--setButton--"+e.getMessage());
						Toast.makeText(getApplicationContext(),
								"Please input differnt cycle number between 5~55!",
								Toast.LENGTH_SHORT).show();
					}
					
				}else {
					Toast.makeText(getApplicationContext(),
							"Please input cycle number between 5~55.",
							Toast.LENGTH_SHORT).show();
				}

			}else if (mInterfaceType == 2) {
				String tmpMipi = mMipiText.getText().toString();
				Xlog.v(TAG, "mMipiString = " + mMipiString + "  new="+tmpMipi);
				if (!tmpMipi.equalsIgnoreCase(mMipiString)) {
					try {
						int mipiValue = Integer.parseInt(tmpMipi);
						if (0<mipiValue && mipiValue<1001) {
						    mMipiString = ""+EmDsenseLcmAssit.LCMSetMipi(mipiValue);
							mMipiText.setText(mMipiString);
							Toast.makeText(getApplicationContext(),
									"set mipi clock success:"+mMipiString,
									Toast.LENGTH_SHORT).show();
						}else {
							Xlog.d(TAG, "--mipiValue--is not in 0~1000!");
							Toast.makeText(getApplicationContext(),
									"Please input mipi clock between 0~1000!",
									Toast.LENGTH_SHORT).show();
						}

					} catch (Exception e) {
						// TODO: handle exception
						Xlog.d(TAG, "setButton--"+e.getMessage());
						Toast.makeText(getApplicationContext(),
								"Please input mipi clock between 0~1000!",
								Toast.LENGTH_SHORT).show();
					}

				}else {
					Toast.makeText(getApplicationContext(),
							"Please input differnt mipi clock between 0~1000!",
							Toast.LENGTH_SHORT).show();
				}
			}
		}
	}
	
}