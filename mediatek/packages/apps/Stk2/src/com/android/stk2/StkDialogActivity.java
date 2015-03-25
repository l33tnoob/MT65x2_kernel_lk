/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.stk2;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.TextMessage;

/**
 * AlretDialog used for DISPLAY TEXT commands.
 */
public class StkDialogActivity extends Activity implements View.OnClickListener {
    // members
    TextMessage mTextMsg;
    private static final int MIN_LENGTH = 6;
    private static final int MIN_WIDTH = 170;
    private static final String LOGTAG = "Stk2-DA ";

    private boolean mbSendResp = false;

    StkAppService appService = StkAppService.getInstance();

    private final IntentFilter mSIMStateChangeFilter = new IntentFilter(
            TelephonyIntents.ACTION_SIM_STATE_CHANGED);

    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                String simState = intent.getStringExtra(IccCard.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(
                        com.android.internal.telephony.Phone.GEMINI_SIM_ID_KEY, -1);

                CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - simId[" + simId + "]  state["
                        + simState + "]");

                if ((simId == com.android.internal.telephony.Phone.GEMINI_SIM_2) &&
                        (IccCard.INTENT_VALUE_ICC_NOT_READY.equals(simState))) {
                    StkDialogActivity.this.cancelTimeOut();
                    sendResponse(StkAppService.RES_ID_CONFIRM, false);
                    StkDialogActivity.this.finish();
                }
            }
        }
    };

    Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ID_TIMEOUT:
                                sendResponse(StkAppService.RES_ID_TIMEOUT);
                    finish();
                    break;
            }
        }
    };

    // keys) for saving the state of the dialog in the icicle
    private static final String TEXT = "text";

    // message id for time out
    private static final int MSG_ID_TIMEOUT = 1;

    // buttons id
    public static final int OK_BUTTON = R.id.button_ok;
    public static final int CANCEL_BUTTON = R.id.button_cancel;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        CatLog.d(LOGTAG, "onCreate");

        CatLog.d(LOGTAG, "onCreate - mbSendResp[" + mbSendResp + "]");

        initFromIntent(getIntent());
        if (mTextMsg == null) {
            finish();
            return;
        }

        requestWindowFeature(Window.FEATURE_LEFT_ICON);
        Window window = getWindow();

        setContentView(R.layout.stk_msg_dialog);
        TextView mMessageView = (TextView) window
                .findViewById(R.id.dialog_message);

        Button okButton = (Button) findViewById(R.id.button_ok);
        Button cancelButton = (Button) findViewById(R.id.button_cancel);

        okButton.setOnClickListener(this);
        // okButton.setHighFocusPriority(true);
        cancelButton.setOnClickListener(this);

        setTitle(mTextMsg.title);
        if (!(mTextMsg.iconSelfExplanatory && mTextMsg.icon != null)) {
            if ((mTextMsg.text == null) || (mTextMsg.text.length() < MIN_LENGTH)) {
                mMessageView.setMinWidth(MIN_WIDTH);
            }
            mMessageView.setText(mTextMsg.text);
        }

        if (mTextMsg.icon == null) {
            CatLog.d(LOGTAG, "onCreate icon is null");
            window.setFeatureDrawableResource(Window.FEATURE_LEFT_ICON,
                    com.android.internal.R.drawable.stat_notify_sim_toolkit);
        } else {
            CatLog.d(LOGTAG, "onCreate icon is not null");
            window.setFeatureDrawable(Window.FEATURE_LEFT_ICON,
                    new BitmapDrawable(mTextMsg.icon));
        }

        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
    }

    private void init() {
        CatLog.d(LOGTAG, "init");
        Window window = getWindow();
        TextView mMessageView = (TextView) window
                .findViewById(R.id.dialog_message);

        Button okButton = (Button) findViewById(R.id.button_ok);
        Button cancelButton = (Button) findViewById(R.id.button_cancel);

        okButton.setOnClickListener(this);
        cancelButton.setOnClickListener(this);

        setTitle(mTextMsg.title);
        if (!(mTextMsg.iconSelfExplanatory && mTextMsg.icon != null)) {
            if ((mTextMsg.text == null) || (mTextMsg.text.length() < MIN_LENGTH)) {
                mMessageView.setMinWidth(MIN_WIDTH);
            }
            mMessageView.setText(mTextMsg.text);
        }

        if (mTextMsg.icon == null) {
            window.setFeatureDrawableResource(Window.FEATURE_LEFT_ICON,
                    com.android.internal.R.drawable.stat_notify_sim_toolkit);
        } else {
            window.setFeatureDrawable(Window.FEATURE_LEFT_ICON,
                    new BitmapDrawable(mTextMsg.icon));
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        CatLog.d(LOGTAG, "onNewIntent");
        CatLog.d(LOGTAG, "onNewIntent - mbSendResp[" + mbSendResp + "]");

        initFromIntent(intent);
        if (mTextMsg == null) {
            finish();
            return;
        }
        init();
    }

    public void onClick(View v) {
        String input = null;

        switch (v.getId()) {
            case OK_BUTTON:
                CatLog.d(LOGTAG, "OK Clicked! isCurCmdSetupCall[" + appService.isCurCmdSetupCall()
                        + "]");
                if (appService.isCurCmdSetupCall()) {
                    CatLog.d(LOGTAG, "stk call sendBroadcast(STKCALL_REGISTER_SPEECH_INFO)");
                    Intent intent = new Intent("com.android.stk.STKCALL_REGISTER_SPEECH_INFO");
                    sendBroadcast(intent);
                }
                sendResponse(StkAppService.RES_ID_CONFIRM, true);
                finish();
                break;
            case CANCEL_BUTTON:
                CatLog.d(LOGTAG, "Cancel Clicked!");
                sendResponse(StkAppService.RES_ID_CONFIRM, false);
                finish();
                break;
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                CatLog.d(LOGTAG, "onKeyDown - KEYCODE_BACK");
                sendResponse(StkAppService.RES_ID_BACKWARD);
                finish();
                break;
        }
        return false;
    }

    @Override
    public void onResume() {
        super.onResume();

        CatLog.d(LOGTAG, "onResume - mbSendResp[" + mbSendResp + "]");
        appService.indicateDialogVisibility(true);

        startTimeOut();
    }

    @Override
    public void onPause() {
        super.onPause();

        CatLog.d(LOGTAG, "onPause");

        appService.indicateDialogVisibility(false);
        cancelTimeOut();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        CatLog.d(LOGTAG, "onDestroy - before Send CONFIRM false mbSendResp[" + mbSendResp + "]");
        if (!mbSendResp) {
            CatLog.d(LOGTAG, "onDestroy - Send CONFIRM false");
            sendResponse(StkAppService.RES_ID_CONFIRM, false);
        }

        appService.indicateDialogVisibility(false);
        unregisterReceiver(mSIMStateChangeReceiver);
        CatLog.d(LOGTAG, "onDestroy-");
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        CatLog.d(LOGTAG, "onSaveInstanceState");

        super.onSaveInstanceState(outState);
        outState.putParcelable(TEXT, mTextMsg);
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

        mTextMsg = savedInstanceState.getParcelable(TEXT);
        CatLog.d(LOGTAG, "onRestoreInstanceState - [" + mTextMsg + "]");
    }

    private void sendResponse(int resId, boolean confirmed) {
        if (StkAppService.getInstance().haveEndSession()) {
            // ignore current command
            CatLog.d(LOGTAG, "Ignore response, id is " + resId);
            return;
        }
        CatLog.d(LOGTAG, "sendResponse resID[" + resId + "] confirmed[" + confirmed + "]");

        mbSendResp = true;
        Bundle args = new Bundle();
        args.putInt(StkAppService.OPCODE, StkAppService.OP_RESPONSE);
        args.putInt(StkAppService.RES_ID, resId);
        args.putBoolean(StkAppService.CONFIRMATION, confirmed);
        /*
         * startService(new Intent(this, StkAppService.class).putExtras(args));
         */
        StkAppService service = StkAppService.getInstance();
        if (service != null) {
            service.sendMessageToServiceHandler(
                    StkAppService.OP_RESPONSE, args);
        }
    }

    private void sendResponse(int resId) {
        sendResponse(resId, true);
    }

    private void initFromIntent(Intent intent) {

        if (intent != null) {
            mTextMsg = intent.getParcelableExtra("TEXT");
        } else {
            finish();
        }

        CatLog.d(LOGTAG, "initFromIntent - [" + mTextMsg + "]");
    }

    private void cancelTimeOut() {
        mTimeoutHandler.removeMessages(MSG_ID_TIMEOUT);
    }

    private void startTimeOut() {
        // Reset timeout.
        cancelTimeOut();
        int dialogDuration = StkApp.calculateDurationInMilis(mTextMsg.duration);
        // case 1 userClear = true & responseNeeded = false,
        // Dialog always exists.
        if (mTextMsg.userClear == true && mTextMsg.responseNeeded == false) {
            return;
        } else {
            // userClear = false. will dissapear after a while.
            if (dialogDuration == 0) {
                dialogDuration = StkApp.DIALOG_DEFAULT_TIMEOUT;
            }
            mTimeoutHandler.sendMessageDelayed(mTimeoutHandler
                    .obtainMessage(MSG_ID_TIMEOUT), dialogDuration);
        }
    }
}
