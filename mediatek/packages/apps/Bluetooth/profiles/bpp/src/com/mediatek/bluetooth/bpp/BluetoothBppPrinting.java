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

package com.mediatek.bluetooth.bpp;

import com.mediatek.bluetooth.R;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import android.os.Bundle;
import android.os.Handler;

import android.view.View;
import android.widget.TextView;
import android.widget.ProgressBar;
import android.widget.Button;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import android.app.NotificationManager;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

public class BluetoothBppPrinting extends AlertActivity
                                  implements DialogInterface.OnClickListener {

    private static final String TAG = "BluetoothBppPrinting";

    private int mDialogType;
    private ProgressBar mSendingProgress;
    private TextView mPercentText;
    private AlertController.AlertParams mPara;
    private View mView = null;
    private TextView mDescriptionView;
    private TextView mReasonView;
    private int mPercentage = 0;
	private boolean clicked = true;

    private String mFileName = null;
    private String mReason = null;
    private String mOldReason = null;
    private int mNotificationId = BluetoothBppManager.NOTIFICATION_ID_BPP; 


    public static final String ACTION_PRINTING_UPDATE = "com.mediatek.bluetooth.bppprinting.action.PRINTING_UPDATE";
    public static final String EXTRA_PERCENTAGE = "com.mediatek.bluetooth.bppprinting.extra.PERCENTAGE";
    public static final String EXTRA_DIALOG_TYPE = "com.mediatek.bluetooth.bppprinting.extra.DIALOG_TYPE";
    public static final String EXTRA_FILE_NAME = "com.mediatek.bluetooth.bppprinting.extra.FILE_NAME";
    public static final String EXTRA_REASON = "com.mediatek.bluetooth.bppprinting.extra.REASON";
    public static final String EXTRA_NOTIFICATION_ID = "com.mediatek.bluetooth.bppprinting.extra.NOTIFICATION_ID";

    public static final int DIALOG_PRINT_PROCESSING_INIT = 1;
    public static final int DIALOG_PRINT_PROCESSING = 2;
    public static final int DIALOG_PRINT_SUCCESS = 3;
    public static final int DIALOG_PRINT_FAIL = 4;


    public static final int RESULT_HIDE = RESULT_FIRST_USER + 1;
    public static final int RESULT_CANCEL = RESULT_FIRST_USER + 2; 
    public static final int RESULT_BACK = RESULT_FIRST_USER + 3; 

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "onReceive");

            String action = intent.getAction();

            if (action.equals(ACTION_PRINTING_UPDATE)) {
               int temp  = intent.getIntExtra(EXTRA_DIALOG_TYPE, -1);
               if ( -1 != temp ) {
                   mDialogType = temp;
               }

              // mNotificationId = intent.getIntExtra(EXTRA_NOTIFICATION_ID, mNotificationId);
               mPercentage = intent.getIntExtra(EXTRA_PERCENTAGE, 0);
               mFileName = intent.getStringExtra(EXTRA_FILE_NAME);

               mOldReason = mReason;
               mReason = intent.getStringExtra(EXTRA_REASON);
               if (mReason == null) 
                   mReason = "Reason";
               

               if ( 99 == mPercentage &&
                    mOldReason.equals(getString(R.string.reason_nondefine)) == false ) {
                    mReason = mOldReason;
               }

               Xlog.v(TAG, "mDialogType:" + mDialogType +
                          "\tmNotificaitonId:" + mNotificationId + "\tmPercentage:" + mPercentage +
                          "\tmFileName:"+  mFileName + "\tmReason:"+ mReason);
   
               setViewContent();
               updateProgressbar();
               updateButton();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Xlog.v(TAG, "OnCreate");

        super.onCreate(savedInstanceState);
		
		if(savedInstanceState != null){
			clicked = savedInstanceState.getBoolean("clicked", true);
		}

		if(!clicked){
			Xlog.v(TAG, "Get attributes from savedInstanceState");
			mDialogType = savedInstanceState.getInt(EXTRA_DIALOG_TYPE, 1);
			mPercentage = savedInstanceState.getInt(EXTRA_PERCENTAGE, 0);
			mNotificationId = savedInstanceState.getInt(EXTRA_NOTIFICATION_ID, mNotificationId);
			mFileName = savedInstanceState.getString(EXTRA_FILE_NAME);    
	        mReason = savedInstanceState.getString(EXTRA_REASON);
	        if (mFileName == null)
	            mFileName = "FileName";
	        if (mReason == null)
	            mReason = "Reason";
		}else{
	        Intent intent = getIntent();
	        String action = intent.getAction();


	        mDialogType = intent.getIntExtra(EXTRA_DIALOG_TYPE, 1);
	        mPercentage = intent.getIntExtra(EXTRA_PERCENTAGE, 0);
	        mNotificationId = intent.getIntExtra(EXTRA_NOTIFICATION_ID, mNotificationId);
	        mFileName = intent.getStringExtra(EXTRA_FILE_NAME);    
	        mReason = intent.getStringExtra(EXTRA_REASON);
	        if (mFileName == null)
	            mFileName = "FileName";
	        if (mReason == null)
	            mReason = "Reason";
		}
        
        Xlog.v(TAG, "mDialogType:" + mDialogType +
                   "\tmNotificaitonId:" + mNotificationId + "\tmPercentage:" + mPercentage +
                   "\tmFileName:"+  mFileName + "\tmReason:"+ mReason);
		clicked = false;

        setUpDialog();

        registerReceiver(mReceiver, new IntentFilter(ACTION_PRINTING_UPDATE));
    }

    @Override
    protected void onStart() {
        Xlog.v(TAG, "onStart");

        super.onStart();     
        updateProgressbar();
        updateButton();
    }

	@Override
	public void onSaveInstanceState(Bundle outState){
        Xlog.v(TAG, "onSaveInstanceState, clicked="+clicked);
		outState.putBoolean("clicked",clicked);
		outState.putInt(EXTRA_DIALOG_TYPE,mDialogType);
		outState.putInt(EXTRA_PERCENTAGE,mPercentage);
		outState.putInt(EXTRA_NOTIFICATION_ID,mNotificationId);
		outState.putString(EXTRA_FILE_NAME,mFileName);
		outState.putString(EXTRA_REASON,mReason);
		super.onSaveInstanceState(outState);
	}

    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "onDestroy()");

        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    protected void onStop() {
        Xlog.v(TAG, "onStop()");
        
        super.onStop();
    }


    @Override
    public void onBackPressed() {
        Xlog.v(TAG, "onBackPressed");

        if ( DIALOG_PRINT_PROCESSING_INIT == mDialogType ) {
            Intent intent = new Intent(this, BluetoothBppManager.class);
            intent.putExtra("action", BluetoothBppManager.ACTION_CANCEL);
            startService(intent);
            setResult(RESULT_BACK);
        }
        finish();
   } 
   
    private void setUpDialog() {
        Xlog.v(TAG, "setUpDialog");

        mPara = mAlertParams;
        mPara.mIconId = android.R.drawable.ic_dialog_info;
        mPara.mTitle = getString(R.string.app_name);

        if ( mDialogType == DIALOG_PRINT_PROCESSING_INIT || mDialogType == DIALOG_PRINT_PROCESSING) {
            mPara.mPositiveButtonText = getString(R.string.dialog_printing_hide);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.dialog_printing_cancel);
            mPara.mNegativeButtonListener = this;
        } 
        else if (mDialogType == DIALOG_PRINT_SUCCESS) {
            mPara.mPositiveButtonText = getString(R.string.dialog_printing_ok);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.dialog_printing_cancel);
            mPara.mNegativeButtonListener = this;
        }
        else if (mDialogType == DIALOG_PRINT_FAIL) {
            mPara.mIconId = android.R.drawable.ic_dialog_alert;
            mPara.mPositiveButtonText = getString(R.string.dialog_printing_ok);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.dialog_printing_cancel);
            mPara.mNegativeButtonListener = this;
        }
        mPara.mView = createView();
        setupAlert();
    }


    private View createView() {
        Xlog.v(TAG, "createView");

        mView = getLayoutInflater().inflate(R.layout.bt_bpp_printing_dialog, null);
        mSendingProgress = (ProgressBar)mView.findViewById(R.id.progress_transfer);
        mPercentText = (TextView)mView.findViewById(R.id.progress_percent);
        mDescriptionView = (TextView)mView.findViewById(R.id.description_view);
        mReasonView = (TextView)mView.findViewById(R.id.reason_view);

        if ( null == mSendingProgress || null == mPercentText || null == mDescriptionView || null == mReasonView ) {
            Xlog.e(TAG, "visual component is null");
        }
        else {
            setViewContent();
        }

        return mView;
    }



    private void setViewContent() {
        Xlog.v(TAG, "setViewContent");


        if ( mDialogType == DIALOG_PRINT_PROCESSING || mDialogType == DIALOG_PRINT_PROCESSING_INIT ) {
            mDescriptionView.setText(getString(R.string.bluetooth_printing, mFileName));
            mReasonView.setText(mReason); 
            if (mReason.equals(getString(R.string.reason_nondefine)) == true )
                mReasonView.setVisibility(View.GONE);
            else
                mReasonView.setVisibility(View.VISIBLE);
                
        } 
        else if ( mDialogType == DIALOG_PRINT_SUCCESS ) {
            mDescriptionView.setText(getString(R.string.printing_successful, mFileName));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
            mReasonView.setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_PRINT_FAIL) {
            mDescriptionView.setText(getString(R.string.printing_fail, mFileName));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
            mReasonView.setVisibility(View.GONE);
        }
    }


    private void updateProgressbar() {
        Xlog.v(TAG, "updateProgressbar");

        mSendingProgress.setMax(100);
        mSendingProgress.setProgress(mPercentage);
        mPercentText.setText(Integer.toString(mPercentage).toString() + "%");
    }


    private void updateButton() {
        Xlog.v(TAG, "updateButton" );

		Button posButton = mAlert.getButton(DialogInterface.BUTTON_POSITIVE);
		Button negButton = mAlert.getButton(DialogInterface.BUTTON_NEGATIVE);
		if(posButton == null || negButton == null)
		{
			Xlog.v(TAG, "get null button" );
			return;
		}
		
        if (mDialogType == DIALOG_PRINT_PROCESSING_INIT) {
            posButton.setText(getString(R.string.dialog_printing_hide));
            negButton.setText(getString(R.string.dialog_printing_cancel));
            posButton.setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_PRINT_PROCESSING) {
            posButton.setText(getString(R.string.dialog_printing_hide));
            negButton.setText(getString(R.string.dialog_printing_cancel));
            posButton.setVisibility(View.VISIBLE);            
        }
        else if (mDialogType == DIALOG_PRINT_SUCCESS) {
            posButton.setText(getString(R.string.dialog_printing_ok));
            posButton.setVisibility(View.VISIBLE);
            negButton.setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_PRINT_FAIL) {
            mAlert.setIcon(android.R.drawable.ic_dialog_alert);
            posButton.setText(getString(R.string.dialog_printing_ok));
            posButton.setVisibility(View.VISIBLE);
            negButton.setVisibility(View.GONE);
        }
    }


    public void onClick(DialogInterface dialog, int which) {
        Xlog.v(TAG, "onClick");
		clicked = true;
        if (which ==  DialogInterface.BUTTON_POSITIVE) {
            Xlog.v(TAG, "positive button");

            if (mDialogType == DIALOG_PRINT_PROCESSING) {
                setResult(RESULT_HIDE);
            }
            else if (mDialogType == DIALOG_PRINT_FAIL || mDialogType == DIALOG_PRINT_SUCCESS) {
                ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);
                setResult(RESULT_OK);
            }
            else {
                Xlog.v(TAG, "exception case");
            }
        }
        else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Xlog.v(TAG, "negative button");

            ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);
            Intent intent = new Intent(this, BluetoothBppManager.class);
            intent.putExtra("action", BluetoothBppManager.ACTION_CANCEL);
            startService(intent);

            setResult(RESULT_CANCEL);
        }
        finish();
    }
}
