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
 */
package com.mediatek.calendarimporter;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.calendarimporter.BindServiceHelper.ServiceConnectedOperation;
import com.mediatek.calendarimporter.service.PreviewProcessor;
import com.mediatek.calendarimporter.service.ProcessorMsgType;
import com.mediatek.calendarimporter.service.VCalService;
import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.calendarimporter.utils.Utils;
import android.provider.CalendarContract;

public class ShowPreviewActivity extends Activity implements ServiceConnectedOperation {
    private static final String TAG = "ShowPreviewActivity";

    private BindServiceHelper mServiceHelper;
    private VCalService mService;
    private Uri mUri;
    private Intent mIntent;

    private PreviewProcessor mProcessor;
    private TextView mPreviewText;
    private Button mImportButton;
    private ImageView mImportErrorIcon;
    private TextView mTitleTextView;
    private Button mCancelButton;
    private Button mErrorCertainButton;
    private View mLoadingView;
    private View mMainPreviewView;

    static final int DURATION = 5000;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtils.d(TAG, "onCreate.");
        setContentView(R.layout.view_calander);
        mIntent = getIntent();
        mUri = mIntent.getData();
        mMainPreviewView = findViewById(R.id.preview_activity);

        mServiceHelper = new BindServiceHelper(this);
        mServiceHelper.onBindService();

        mTitleTextView = (TextView) findViewById(R.id.preview_title);
        mPreviewText = (TextView) findViewById(R.id.calendar_value);

        int color = Utils.getThemeMainColor(this, Utils.DEFAULT_COLOR);
        if (color != Utils.DEFAULT_COLOR) {
            mTitleTextView.setTextColor(color);
            findViewById(R.id.preview_divide_line).setBackgroundColor(color);
        }

        mLoadingView = findViewById(R.id.preview_loading);
        mImportErrorIcon = (ImageView) findViewById(R.id.import_error_icon);

        mErrorCertainButton = (Button) findViewById(R.id.import_error_certain);
        mErrorCertainButton.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                finish();
            }
        });

        mImportButton = (Button) findViewById(R.id.button_ok);
        mImportButton.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                if (Utils.hasExchangeOrGoogleAccount(ShowPreviewActivity.this)) {
                    LogUtils.d(TAG, "onResume,show SelectActivity... ");
                    showSelectActivity();
                    finish();
                } else {
                    Toast.makeText(ShowPreviewActivity.this, R.string.no_access_toast, DURATION).show();
                    Intent intent = new Intent(Settings.ACTION_ADD_ACCOUNT);
                    intent.putExtra(Settings.EXTRA_AUTHORITIES, new String[] { CalendarContract.AUTHORITY });
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    LogUtils.d(TAG, "onResume,Show Settings... ");
                    startActivity(intent);
                }
            }
        });
        mCancelButton = (Button) findViewById(R.id.button_cancel);
        mCancelButton.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                finish();
            }
        });
    }

    private void showSelectActivity() {
        Intent intent = getIntent();
        intent.setClass(this, HandleProgressActivity.class);
        startActivity(intent);
    }

    @Override
    protected void onDestroy() {
        LogUtils.d(TAG, "onDestroy() + mService:" + mService);
        if (mService != null) {
            mService.tryCancelProcessor(mProcessor);
        }
        mServiceHelper.unBindService();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        LogUtils.d(TAG, "onBackPressed.");
        if (mProcessor != null && mService != null) {
            mService.tryCancelProcessor(mProcessor);
        }
        super.onBackPressed();
    }

    @Override
    public void serviceConnected(VCalService service) {
        LogUtils.d(TAG, "serviceConnected.");
        mService = service;

        Handler handler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case ProcessorMsgType.PROCESSOR_FINISH:
                    LogUtils.d(TAG, "serviceConnected,handlerMessage : " + msg.arg1 + "/" + msg.arg2 + " " + msg.obj);
                    // Only to handle single vEvent files
                    if (msg.arg2 > 1) {
                        setImportErrorView();
                        return;
                    }
                    mLoadingView.setVisibility(View.GONE);
                    mPreviewText.setText((String) msg.obj);
                    mPreviewText.setVisibility(View.VISIBLE);
                    mImportButton.setVisibility(View.VISIBLE);
                    mMainPreviewView.setVisibility(View.VISIBLE);
                    break;

                case ProcessorMsgType.PROCESSOR_EXCEPTION:
                    setImportErrorView();
                    break;

                default:
                    break;
                }
            }
        };

        mProcessor = new PreviewProcessor(this, mUri, handler);
        mService.tryExecuteProcessor(mProcessor);
    }

    @Override
    public void serviceUnConnected() {
        LogUtils.d(TAG, "serviceUnConnected.");
        mService = null;
    }

    private void setImportErrorView() {
        mPreviewText.setVisibility(View.VISIBLE);
        mCancelButton.setVisibility(View.GONE);
        mImportButton.setVisibility(View.GONE);
        mTitleTextView.setText(android.R.string.dialog_alert_title);
        mPreviewText.setText(R.string.not_support_multi_events);
        mErrorCertainButton.setVisibility(View.VISIBLE);
        mImportErrorIcon.setVisibility(View.VISIBLE);
        mLoadingView.setVisibility(View.GONE);
        mMainPreviewView.setVisibility(View.VISIBLE);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        mServiceHelper.unBindService();
        mIntent = intent;
        mUri = intent.getData();
        mServiceHelper.onBindService();
    }

}
