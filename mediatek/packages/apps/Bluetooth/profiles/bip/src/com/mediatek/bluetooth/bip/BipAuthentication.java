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

import android.os.Bundle;
import android.os.Handler;

import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import android.app.NotificationManager;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

public class BipAuthentication extends AlertActivity
                                  implements DialogInterface.OnClickListener {

    private static final String TAG = "BipAuthentication";

  
    public static final String
        EXTRA_FROM = "com.mediatek.bluetooth.bipauthentication.extra.FROM",
        EXTRA_NEED_USERID = "com.mediatek.bluetooth.bipauthentication.extra.NEED_USERID";
 

    private static AlertController.AlertParams mPara;
    private static View mView = null;
    private static EditText mPasswordEdit, mUserIdEdit;
    private static TextView mUserIdText;

    private static String mFrom;
    private static boolean mNeedUserId;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Xlog.v(TAG, "OnCreate");

        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String action = intent.getAction();

        mFrom = intent.getStringExtra(EXTRA_FROM);
        mNeedUserId = intent.getBooleanExtra(EXTRA_NEED_USERID, false);


        setUpDialog();
    }

    @Override
    protected void onStart() {
        Xlog.v(TAG, "onStart");

        super.onStart();     
    }


    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "onDestroy()");

        super.onDestroy();
    }

    @Override
    protected void onStop() {
        Xlog.v(TAG, "onStop()");
        
        super.onStop();
    }


    private void setUpDialog() {
        Xlog.v(TAG, "setUpDialog");

        //final AlertController.AlertParams p = mAlertParams;
        mPara = mAlertParams;
        mPara.mIconId = android.R.drawable.ic_dialog_info;
        mPara.mTitle = getString(R.string.bt_bip_app_name);

        mPara.mPositiveButtonText = getString(R.string.bt_bip_auth_ok);
        mPara.mPositiveButtonListener = this;
        mPara.mNegativeButtonText = getString(R.string.bt_bip_auth_cancel);
        mPara.mNegativeButtonListener = this;

        mPara.mView = createView();
        setupAlert();
    }


    private View createView() {
        Xlog.v(TAG, "createView");

        mView = getLayoutInflater().inflate(R.layout.bt_bip_authentication_dialog, null);

        mPasswordEdit = (EditText)mView.findViewById(R.id.password_edit);

        mUserIdText = (TextView)mView.findViewById(R.id.userid_text);
        mUserIdEdit = (EditText)mView.findViewById(R.id.userid_edit);

        if ( false == mNeedUserId ) {
            if ( null == mUserIdText || null == mUserIdEdit ) {
                Xlog.v(TAG, "UserId text is null or UserId edit is null");             
            }
            else {
                mUserIdText.setVisibility(View.GONE);
                mUserIdEdit.setVisibility(View.GONE);
            }
        }

        return mView;
    }



    public void onClick(DialogInterface dialog, int which) {
        Xlog.v(TAG, "onClick");

        if (which ==  DialogInterface.BUTTON_POSITIVE) {
            Xlog.v(TAG, "positive button");

            Intent intent = new Intent(this, BipService.class);
            if ( mFrom.equals("BIPI") ) {
                intent.putExtra("action", BipService.ACTION_BIPI_AUTH_INFO);
            }
            else {
                intent.putExtra("action", BipService.ACTION_BIPR_AUTH_INFO);
            }

            if ( mNeedUserId ) {
                intent.putExtra(BipService.EXTRA_AUTH_USERID, mUserIdEdit.getText().toString());
            }
            else {
                intent.putExtra(BipService.EXTRA_AUTH_USERID, "UserId");
            }
            intent.putExtra(BipService.EXTRA_AUTH_PASSWD, mPasswordEdit.getText().toString());
            startService(intent);

        }
        else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Xlog.v(TAG, "negative button");

            Intent intent = new Intent(this, BipService.class);
            if ( mFrom.equals("BIPI") ) {
                intent.putExtra("action", BipService.ACTION_BIPI_CANCEL);
            }
            else {
                intent.putExtra("action", BipService.ACTION_BIPR_CANCEL);
            }
            startService(intent);

        }
        finish();
    }
}
