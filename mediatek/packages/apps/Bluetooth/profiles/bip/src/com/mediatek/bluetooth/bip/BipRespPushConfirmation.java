/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

package com.mediatek.bluetooth.bip;

import com.mediatek.bluetooth.R;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;


import android.os.Bundle;

import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.CheckBox;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;
import android.bluetooth.BluetoothAdapter;

public class BipRespPushConfirmation extends AlertActivity
                                  implements DialogInterface.OnClickListener {

    private static final String TAG = "BipResponderPushConfirmation";


    public static final String
        ACTION_CANCEL_BY_PEER = "com.mediatek.bluetooth.biprpushconfirmation.action.CANCEL_BY_PEER",
        EXTRA_DEVICE_NAME = "com.mediatek.bluetooth.biprpushconfirmation.extra.DEVICE_NAME",
        EXTRA_FILE_NAME = "com.mediatek.bluetooth.biprpushconfirmation.extra.FILE_NAME",
        EXTRA_FILE_SIZE = "com.mediatek.bluetooth.biprpushconfirmation.extra.FILE_SIZE";


    private AlertController.AlertParams mPara;
    private View mView = null;
    private TextView mDescriptionView;
    private CheckBox mAlwaysAccept;


    private static boolean mClick = false;
    private static boolean mFromBack = false;

    private static String mRemoteDeviceName = null;
    private static String mFileName = null;
    private static String mFileSize = null;


    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "onReceive");

            String action = intent.getAction();
			
	    if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
	    	int btState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
			
		switch(btState) {
		case BluetoothAdapter.STATE_TURNING_OFF:
				mClick = true;
				finish();
			}
		}

            if ( action.equals(ACTION_CANCEL_BY_PEER) ){
                mClick = true;
                finish();
            }
        }
    };




    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Xlog.v(TAG, "OnCreate");

        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String action = intent.getAction();

        mRemoteDeviceName = intent.getStringExtra(EXTRA_DEVICE_NAME);
        if (mRemoteDeviceName == null)
            mRemoteDeviceName = "DeviceName";
        mFileName = intent.getStringExtra(EXTRA_FILE_NAME);
        if (mFileName == null)
            mFileName = "FileName";
        mFileSize = intent.getStringExtra(EXTRA_FILE_SIZE);
        if (mFileSize == null)
            mFileSize = "unknown size";
        

        mClick = false;

        setUpDialog();
    }

    @Override
    protected void onStart() {
        Xlog.v(TAG, "onStart");

        super.onStart();     
    }


    @Override
    protected void onResume() {
        Xlog.v(TAG, "onResume");
        super.onResume();
        mFromBack = false;
	IntentFilter mFilter = new IntentFilter();
	mFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
	mFilter.addAction(ACTION_CANCEL_BY_PEER);
	registerReceiver(mReceiver, mFilter);	
    }


    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "onDestroy()");

        super.onDestroy();
    }


    @Override
    protected void onPause() {
        Xlog.v(TAG, "onPause()");
        super.onPause();
        unregisterReceiver(mReceiver);
        if (!mClick) {
            finish();
            Intent intent = new Intent(this, BipService.class);
            if ( mFromBack ) {
                intent.putExtra("action", BipService.ACTION_RECEIVE_REJECT);
            }
            else {
                intent.putExtra("action", BipService.ACTION_RECEIVE_RESTORE);
            }
            startService(intent);
        }
    }


    @Override
    protected void onStop() {
        Xlog.v(TAG, "onStop()");
        
        super.onStop();
    }


    @Override
    public void onBackPressed() {
        Xlog.v(TAG, "onBackPressed");

        super.onBackPressed();
        mFromBack = true;
    }


    private void setUpDialog() {
        Xlog.v(TAG, "setUpDialog");

        mPara = mAlertParams;
        mPara.mIconId = android.R.drawable.ic_dialog_info;
        mPara.mTitle = getString(R.string.bt_bipr_accept_title);

        mPara.mPositiveButtonText = getString(R.string.bt_bipr_accept_yes);
        mPara.mPositiveButtonListener = this;
        mPara.mNegativeButtonText = getString(R.string.bt_bipr_accept_no);
        mPara.mNegativeButtonListener = this;

        mPara.mView = createView();
        setupAlert();
    }


    private View createView() {
        Xlog.v(TAG, "createView");

        mView = getLayoutInflater().inflate(R.layout.bt_bipr_push_confirmation_dialog, null);

        mDescriptionView = (TextView)mView.findViewById(R.id.description_view);
	mAlwaysAccept = (CheckBox)mView.findViewById(R.id.always_accept);
        if ( null == mDescriptionView ) {
            Xlog.e(TAG, "mDescriptionView is null");
        }
        else {
           mDescriptionView.setText(getString(R.string.bt_bipr_accept_content, mFileName, mFileSize, mRemoteDeviceName));
        }


        return mView;
    }

    public void onClick(DialogInterface dialog, int which) {
        Xlog.v(TAG, "onClick");
        mClick = true;

        if (which ==  DialogInterface.BUTTON_POSITIVE) {
            Xlog.v(TAG, "positive button");

            Intent intent = new Intent(this, BipService.class);
            intent.putExtra("action", BipService.ACTION_RECEIVE_ACCEPT);
			
	    Xlog.v(TAG, "Always Accept: "+mAlwaysAccept.isChecked());
	    if(mAlwaysAccept.isChecked()){
		intent.putExtra("alwaysAccept", true);
	    }else{
		intent.putExtra("alwaysAccept", false);
	    }
            startService(intent);

        }
        else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Xlog.v(TAG, "negative button");

            Intent intent = new Intent(this, BipService.class);
            intent.putExtra("action", BipService.ACTION_RECEIVE_REJECT);
            startService(intent);

        }
        finish();
    }
}
