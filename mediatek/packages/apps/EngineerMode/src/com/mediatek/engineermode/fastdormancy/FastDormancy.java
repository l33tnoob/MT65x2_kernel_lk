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

package com.mediatek.engineermode.fastdormancy;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Description: A helper tool to test fast dormancy
 * 
 * @author mtk54043
 * 
 */
public class FastDormancy extends Activity implements OnClickListener {

    private static final String TAG = "EM/FD";
    private static final int COUNT = 4;
    private static final int INDEX_OFF_LE = 0;
    private static final int INDEX_ON_LE = 1;
    private static final int INDEX_OFF_R8 = 2;
    private static final int INDEX_ON_R8 = 3;
    private static final int MSG_SET_TIME = 101;
    private static final int MSG_SEND_FD = 102;
    private static final int DIALOG_SET_FAILED = 201;
    private static final int DIALOG_SEND_FD = 202;

    private EditText[] mFastDormancyEdit;
    private Phone mPhone = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fastdormancy);
        findViewById(R.id.fd_btn_set).setOnClickListener(this);
        findViewById(R.id.fd_btn_send).setOnClickListener(this);
        findViewById(R.id.fd_btn_config).setOnClickListener(this);

        mFastDormancyEdit = new EditText[COUNT];
        mFastDormancyEdit[INDEX_OFF_LE] = (EditText) findViewById(R.id.fd_edit_screen_off_legacy);
        mFastDormancyEdit[INDEX_ON_LE] = (EditText) findViewById(R.id.fd_edit_screen_on_legacy);
        mFastDormancyEdit[INDEX_OFF_R8] = (EditText) findViewById(R.id.fd_edit_screen_off_r8fd);
        mFastDormancyEdit[INDEX_ON_R8] = (EditText) findViewById(R.id.fd_edit_screen_on_r8fd);
        mPhone = PhoneFactory.getDefaultPhone();

    }

    @Override
    protected void onResume() {
        super.onResume();
        String[] fastDTimerValue = mPhone.getFDTimerValue();
        if (fastDTimerValue == null) {
            Elog.e(TAG, "getFDTimerValue() == null");
            Toast.makeText(FastDormancy.this, "Query FD parameter failed!", Toast.LENGTH_LONG).show();
        } else if (fastDTimerValue.length < COUNT) {
            Elog.e(TAG, "getFDTimerValue().length < 4 ");
            Toast.makeText(FastDormancy.this, "Query FD parameter failed!", Toast.LENGTH_LONG).show();
        } else if (fastDTimerValue.length == COUNT) {
            for (int i = 0; i < COUNT; i++) {
                mFastDormancyEdit[i].setText(fastDTimerValue[i]);
                Elog.i(TAG, "fastDTimerValue[" + i + "] = " + fastDTimerValue[i]);
            }
        } else {
            Elog.e(TAG, "getFDTimerValue().length == " + fastDTimerValue.length);
            for (int i = 0; i < fastDTimerValue.length; i++) {
                Elog.i(TAG, "fastDTimerValue[" + i + "] = " + fastDTimerValue[i]);
            }
        }
    }

    @Override
    protected void onDestroy() { 
        Elog.i(TAG, "onDestroy(),removeMessages");
        mResponseHander.removeMessages(MSG_SET_TIME);
        mResponseHander.removeMessages(MSG_SEND_FD);
        super.onDestroy();
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
        case R.id.fd_btn_set:
            String[] fastDTimerValue = new String[COUNT];
            for (int i = 0; i < COUNT; i++) {
                fastDTimerValue[i] = mFastDormancyEdit[i].getText().toString().trim();
                try {
                    Integer.valueOf(fastDTimerValue[i]);
                } catch (NumberFormatException e) {
                    Elog.e(TAG, "NumberFormatException");
                    Toast.makeText(FastDormancy.this, "Your input number must be a int type!", Toast.LENGTH_SHORT).show();
                    return;
                }
//                if ("".equals(fastDTimerValue[i])) {
//                    Toast.makeText(FastDormancy.this, "Please check your input number.", Toast.LENGTH_SHORT).show();
//                    return;
//                }
            }
            mPhone.setFDTimerValue(fastDTimerValue, mResponseHander.obtainMessage(MSG_SET_TIME));
            break;
        case R.id.fd_btn_send:
            String fastDormancyAT[] = { "AT+ESCRI", "" };
            mPhone.invokeOemRilRequestStrings(fastDormancyAT, mResponseHander.obtainMessage(MSG_SEND_FD));
            break;
        case R.id.fd_btn_config:
            startActivity(new Intent(this, ConfigFD.class));
            break;
        default:
            break;
        }
    }

    private Handler mResponseHander = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "Receive msg from modem");
            AsyncResult ar;
            if (msg.what == MSG_SET_TIME) {
                Elog.i(TAG, "Receive MSG_SET_TIME");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(FastDormancy.this, "Success!", Toast.LENGTH_LONG).show();
                } else {
                    showDialog(DIALOG_SET_FAILED);
                }
            } else if (msg.what == MSG_SEND_FD) {
                Elog.i(TAG, "Receive MSG_SEND_FD");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(FastDormancy.this, "Success!", Toast.LENGTH_LONG).show();
                } else {
                    showDialog(DIALOG_SET_FAILED);
                }
            }
        }
    };

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        if (id == DIALOG_SET_FAILED) {
            return builder.setTitle("Warnning!").setMessage("Failed to set FD parameter.").setPositiveButton("OK", null)
                    .create();
        } else if (id == DIALOG_SEND_FD) {
            return builder.setTitle("Warnning!").setMessage("Failed to send FD.").setPositiveButton("OK", null).create();
        }
        // else {
        // return builder.setTitle("Warnning!").setMessage("Query failed.").setPositiveButton("OK", null).create();
        // }
        return null;
    }
}
