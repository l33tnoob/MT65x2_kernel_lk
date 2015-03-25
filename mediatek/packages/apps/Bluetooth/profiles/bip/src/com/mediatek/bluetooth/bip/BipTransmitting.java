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
import android.os.Handler;
import android.os.Environment;


import android.view.View;
import android.widget.TextView;
import android.widget.ProgressBar;
import android.widget.Button;

import android.app.NotificationManager;
import android.net.Uri;
import android.util.Log;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import java.io.File;


import com.mediatek.bluetooth.util.SystemUtils;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;


public class BipTransmitting extends AlertActivity
                                 implements DialogInterface.OnClickListener {

    private static final String TAG = "BipTransmitting";

    private int mOriDialogType;
    private int mDialogType;

    private Context mContext;

    private TextView mPercentText;
    private TextView mDescriptionView;
    private ProgressBar mSendingProgress;
    private AlertController.AlertParams mPara;
    private View mView = null;

    private int mPercentage = 0;
    private String mFileName = null;
    private int mNotificationId = BipService.NOTIFICATION_ID_BIPI; 

    public static final String
        ACTION_TRANSMISSION_UPDATE = "com.mediatek.bluetooth.bipitransmitting.action.TRANSMISSION_UPDATE",
        ACTION_RECEIVING_UPDATE = "com.mediatek.bluetooth.bipitransmitting.action.RECEIVING_UPDATE",
        EXTRA_DIALOG_TYPE = "com.mediatek.bluetooth.bipitransmitting.extra.DIALOG_TYPE",
        EXTRA_NOTIFICATION_ID = "com.mediatek.bluetooth.bipitransmitting.extra.NOTIFICATION_ID",
        EXTRA_PERCENTAGE = "com.mediatek.bluetooth.bipitransmitting.extra.PERCENTAGE",
        EXTRA_FILE_NAME = "com.mediatek.bluetooth.bipitransmitting.extra.FILE_NAME";

    public static final int
        DIALOG_TRANSMISSION_PROCESSING = 11,
        DIALOG_TRANSMISSION_SUCCESS = 12,
        DIALOG_TRANSMISSION_FAIL = 13,
        DIALOG_PENDING_JOB = 20,
        DIALOG_RECEIVE_PROCESSING = 21,
        DIALOG_RECEIVE_SUCCESS = 22,
        DIALOG_RECEIVE_FAIL = 23;



    public static final int RESULT_HIDE = RESULT_FIRST_USER + 1;
    public static final int RESULT_CANCEL = RESULT_FIRST_USER + 2; 
    public static final int RESULT_BACK = RESULT_FIRST_USER + 3; 

    private IntentFilter mFilter = new IntentFilter();
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "onReceive");

            String action = intent.getAction();

            if ( (action.equals(ACTION_TRANSMISSION_UPDATE) && mOriDialogType == DIALOG_TRANSMISSION_PROCESSING) ||
                 (action.equals(ACTION_RECEIVING_UPDATE) && mOriDialogType == DIALOG_RECEIVE_PROCESSING) ) {
                mDialogType = intent.getIntExtra(EXTRA_DIALOG_TYPE, 1);
                mPercentage = intent.getIntExtra(EXTRA_PERCENTAGE, 0);
                mFileName = intent.getStringExtra(EXTRA_FILE_NAME);
                

                Xlog.v(TAG, "mDialogType:" + mDialogType + "\tmNotificaitonId:" + mNotificationId +
                          "\tmPercentage:" + mPercentage + "\tmFileName:"+  mFileName);
   
                setViewContent();
                updateProgressbar();
                updateButton();
            }
            else if( (action.equals(ACTION_TRANSMISSION_UPDATE) && mOriDialogType == DIALOG_PENDING_JOB) ) {
                if ( mNotificationId == intent.getIntExtra(EXTRA_NOTIFICATION_ID, 0) ) {
                    finish();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Xlog.v(TAG, "OnCreate");

        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String action = intent.getAction();

        mContext = getApplicationContext();

        mOriDialogType = intent.getIntExtra(EXTRA_DIALOG_TYPE, 1);
        mDialogType = mOriDialogType;
        mPercentage = intent.getIntExtra(EXTRA_PERCENTAGE, 0);
        mNotificationId = intent.getIntExtra(EXTRA_NOTIFICATION_ID, mNotificationId);
        mFileName = intent.getStringExtra(EXTRA_FILE_NAME);    
        if (mFileName == null)
            mFileName = "FileName";

        
        Xlog.v(TAG, "mDialogType:" + mDialogType +  "\tmNotificaitonId:" + mNotificationId +
                   "\tmPercentage:" + mPercentage + "\tmFileName:"+  mFileName);

        setUpDialog();

        if( mOriDialogType == DIALOG_TRANSMISSION_PROCESSING ||
            mOriDialogType == DIALOG_RECEIVE_PROCESSING ||
            mOriDialogType == DIALOG_PENDING_JOB ) {

            mFilter.addAction(ACTION_TRANSMISSION_UPDATE);
            mFilter.addAction(ACTION_RECEIVING_UPDATE);
            registerReceiver(mReceiver, mFilter);
            //registerReceiver(mReceiver, new IntentFilter(ACTION_TRANSMISSION_UPDATE));
        }
    }

    @Override
    protected void onStart() {
        Xlog.v(TAG, "onStart");

        super.onStart();     
        updateProgressbar();
        updateButton();
    }


    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "onDestroy()");

        if( mOriDialogType == DIALOG_TRANSMISSION_PROCESSING || mOriDialogType == DIALOG_RECEIVE_PROCESSING ) {
            unregisterReceiver(mReceiver);
        }
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        Xlog.v(TAG, "onStop()");
        
        super.onStop();
    }

   
//temporay release end
    private void setUpDialog() {
        Xlog.v(TAG, "setUpDialog");

        //final AlertController.AlertParams p = mAlertParams;
        mPara = mAlertParams;
        mPara.mIconId = android.R.drawable.ic_dialog_info;
        mPara.mTitle = getString(R.string.bt_bip_app_name);

        if ( mDialogType == DIALOG_TRANSMISSION_PROCESSING || mDialogType == DIALOG_RECEIVE_PROCESSING ) {
            mPara.mPositiveButtonText = getString(R.string.bt_bip_transmitting_dialog_hide);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.bt_bip_transmitting_dialog_cancel);
            mPara.mNegativeButtonListener = this;
        } 
        else if ( mDialogType == DIALOG_TRANSMISSION_SUCCESS || mDialogType == DIALOG_RECEIVE_SUCCESS ) {
            mPara.mPositiveButtonText = getString(R.string.bt_bip_transmitting_dialog_ok);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.bt_bip_transmitting_dialog_cancel);
            mPara.mNegativeButtonListener = this;
        }
        else if (mDialogType == DIALOG_TRANSMISSION_FAIL || mDialogType == DIALOG_RECEIVE_FAIL) {
            mPara.mIconId = android.R.drawable.ic_dialog_alert;
            mPara.mPositiveButtonText = getString(R.string.bt_bip_transmitting_dialog_ok);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.bt_bip_transmitting_dialog_cancel);
            mPara.mNegativeButtonListener = this;
        }
        else if (mDialogType == DIALOG_PENDING_JOB) {
            mPara.mPositiveButtonText = getString(R.string.bt_bip_transmitting_dialog_yes);
            mPara.mPositiveButtonListener = this;
            mPara.mNegativeButtonText = getString(R.string.bt_bip_transmitting_dialog_no);
            mPara.mNegativeButtonListener = this;
        }

        mPara.mView = createView();
        setupAlert();
    }


    private View createView() {
        Xlog.v(TAG, "createView");

        mView = getLayoutInflater().inflate(R.layout.bt_bip_transmitting_dialog, null);
        mSendingProgress = (ProgressBar)mView.findViewById(R.id.progress_transfer);
        mPercentText = (TextView)mView.findViewById(R.id.progress_percent);
        mDescriptionView = (TextView)mView.findViewById(R.id.description_view);

        if ( null == mSendingProgress ||  null == mPercentText || null == mDescriptionView ) {
            Xlog.e(TAG, "visual component is null");
        }
        else {
            setViewContent();
        }

        return mView;
    }



    private void setViewContent() {
        Xlog.v(TAG, "setViewContent");

        if ( mDialogType == DIALOG_TRANSMISSION_PROCESSING ) {
            mDescriptionView.setText(getString(R.string.bt_bip_transmitting, mFileName));
        }
        else if ( mDialogType == DIALOG_RECEIVE_PROCESSING ) {
            mDescriptionView.setText(getString(R.string.bt_bip_receiving, mFileName));
        }  
        else if ( mDialogType == DIALOG_TRANSMISSION_SUCCESS ) {
            mDescriptionView.setText(getString(R.string.bt_bip_transmitting_successful, mFileName));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
        }
        else if ( mDialogType == DIALOG_RECEIVE_SUCCESS ) {
            mDescriptionView.setText(getString(R.string.bt_bip_receiving_successful, mFileName));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_TRANSMISSION_FAIL) {
            mDescriptionView.setText(getString(R.string.bt_bip_transmitting_fail, mFileName));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_RECEIVE_FAIL) {
            mDescriptionView.setText(getString(R.string.bt_bip_receiving_fail, mFileName));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_PENDING_JOB) {
            mDescriptionView.setText(getString(R.string.bt_bip_cancel_pending_job));
            mSendingProgress.setVisibility(View.GONE);
            mPercentText.setVisibility(View.GONE);
        }
    }


    private void updateProgressbar() {
        Xlog.v(TAG, "updateProgressbar");

        mSendingProgress.setMax(100);
        mSendingProgress.setProgress(mPercentage);
        mPercentText.setText(Integer.toString(mPercentage).toString() + "%");
    }


    private void updateButton() {

        if (mDialogType == DIALOG_TRANSMISSION_PROCESSING || mDialogType == DIALOG_RECEIVE_PROCESSING) {
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setText(getString(R.string.bt_bip_transmitting_dialog_hide));
            mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setText(getString(R.string.bt_bip_transmitting_dialog_cancel));
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setVisibility(View.VISIBLE);            
            mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setVisibility(View.VISIBLE);            
        }
        //else if (mDialogType == DIALOG_TRANSMISSION_SUCCESS || mDialogType == DIALOG_RECEIVE_SUCCESS ) {
        else if (mDialogType == DIALOG_TRANSMISSION_SUCCESS ) {
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setText(getString(R.string.bt_bip_transmitting_dialog_ok));
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setVisibility(View.VISIBLE);
            mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setVisibility(View.GONE);
        }
        else if (mDialogType == DIALOG_TRANSMISSION_FAIL || mDialogType == DIALOG_RECEIVE_FAIL ) {
            mAlert.setIcon(android.R.drawable.ic_dialog_alert);
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setText(getString(R.string.bt_bip_transmitting_dialog_ok));
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setVisibility(View.VISIBLE);
            mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setVisibility(View.GONE);
        }
        //else if (mDialogType == DIALOG_PENDING_JOB) {
        else if (mDialogType == DIALOG_PENDING_JOB | mDialogType == DIALOG_RECEIVE_SUCCESS) {
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setText(getString(R.string.bt_bip_transmitting_dialog_yes));
            mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setText(getString(R.string.bt_bip_transmitting_dialog_no));
            mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setVisibility(View.VISIBLE);
            mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setVisibility(View.VISIBLE);
        }
    }


    public void onClick(DialogInterface dialog, int which) {
        Xlog.v(TAG, "onClick");

        if (which ==  DialogInterface.BUTTON_POSITIVE) {
            Xlog.v(TAG, "positive button");

            if (mDialogType == DIALOG_TRANSMISSION_PROCESSING || mDialogType == DIALOG_RECEIVE_PROCESSING) {
                setResult(RESULT_HIDE);
            }
            else if (mDialogType == DIALOG_TRANSMISSION_FAIL || mDialogType == DIALOG_TRANSMISSION_SUCCESS ||
                     mDialogType == DIALOG_RECEIVE_FAIL || mDialogType == DIALOG_RECEIVE_SUCCESS ) {
                ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);
     
                if ( mDialogType == DIALOG_RECEIVE_SUCCESS ) {
                    Intent intent = new Intent( Intent.ACTION_VIEW );

                    intent.setDataAndType( Uri.fromFile(new File(SystemUtils.getReceivedFilePath(mContext)+ "/" + mFileName )), "image/*" );
                    //intent.setDataAndType( Uri.fromFile(new File(Environment.getExternalStorageDirectory().getPath()+ "/" + mFileName )), "image/*" );
                    //intent.setDataAndType( Uri.fromFile(new File( "sdcard/" + mFileName )), "image/*" );
                    intent.setFlags( Intent.FLAG_ACTIVITY_NEW_TASK );
                    startActivity(intent);
                } 

                setResult(RESULT_OK);
            }
            else if (mDialogType == DIALOG_PENDING_JOB) {
                ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);
                //TODO: Send intent to BipiService
                Intent intent = new Intent(this, BipService.class);
                intent.putExtra("action", BipService.ACTION_CANCEL_PENDING);
                intent.putExtra(EXTRA_NOTIFICATION_ID, mNotificationId);
                startService(intent);
            }
            else {
                Xlog.v(TAG, "exception case");
            }
        }
        else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Xlog.v(TAG, "negative button");

            if (mDialogType == DIALOG_TRANSMISSION_PROCESSING) {
                ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);

                Intent intent = new Intent(this, BipService.class);
                intent.putExtra("action", BipService.ACTION_BIPI_CANCEL);
                startService(intent);
            }
            else if (mDialogType == DIALOG_RECEIVE_PROCESSING) {
                ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);

                Intent intent = new Intent(this, BipService.class);
                intent.putExtra("action", BipService.ACTION_BIPR_CANCEL);
                startService(intent);
            }
            else if (mDialogType == DIALOG_RECEIVE_SUCCESS) {
                ((NotificationManager)getSystemService(NOTIFICATION_SERVICE)).cancel(mNotificationId);
            }
            setResult(RESULT_CANCEL);
        }
        finish();
    }
}
