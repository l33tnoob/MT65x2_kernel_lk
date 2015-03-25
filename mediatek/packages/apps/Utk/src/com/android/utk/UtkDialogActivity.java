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

package com.android.utk;

import com.android.internal.telephony.cdma.utk.TextMessage;

import android.app.Activity;
import android.content.Intent;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ImageView;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.telephony.cdma.utk.UtkLog;
/**
 * AlretDialog used for DISPLAY TEXT commands.
 *
 */
public class UtkDialogActivity extends Activity implements View.OnClickListener {
    // members
    TextMessage mTextMsg;

    private static final int MIN_LENGTH = 6;
    private static final int MIN_WIDTH = 170;

    UtkAppService appService = UtkAppService.getInstance();

    Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case MSG_ID_TIMEOUT:
                if (!mTextMsg.userClear) {
                    UtkLog.d(this, "handleMessage user clear false");
                    sendResponse(UtkAppService.RES_ID_CONFIRM);
                }
                finish();
                break;
            }
        }
    };

    //keys) for saving the state of the dialog in the icicle
    private static final String TEXT = "text";

    // message id for time out
    private static final int MSG_ID_TIMEOUT = 1;

    // buttons id
    public static final int OK_BUTTON = R.id.button_ok;
    public static final int CANCEL_BUTTON = R.id.button_cancel;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        initFromIntent(getIntent());
        if (mTextMsg == null) {
            finish();
            return;
        }

        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        Window window = getWindow();

        setContentView(R.layout.utk_msg_dialog);
        window.setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.utk_msg_dialog_title);
        TextView titleTv = (TextView)findViewById(R.id.title);
        TextView mMessageView = (TextView) window
                .findViewById(R.id.dialog_message);

        Button okButton = (Button) findViewById(R.id.button_ok);
        Button cancelButton = (Button) findViewById(R.id.button_cancel);

        okButton.setOnClickListener(this);
        cancelButton.setOnClickListener(this);

        titleTv.setText(mTextMsg.title);
        if (!(mTextMsg.iconSelfExplanatory && mTextMsg.icon != null)) {
            if ((mTextMsg.text==null) || (mTextMsg.text.length() < MIN_LENGTH) ) {
            	mMessageView.setMinWidth(MIN_WIDTH);
            }
            mMessageView.setText(mTextMsg.text);
        }

        ImageView icon = (ImageView)findViewById(R.id.icon);
        if (mTextMsg.icon == null) {
            icon.setImageResource(com.android.internal.R.drawable.stat_notify_sim_toolkit);
        } else {
            icon.setImageDrawable(new BitmapDrawable(mTextMsg.icon));
        }
    }

    public void onClick(View v) {
        String input = null;

        switch (v.getId()) {
        case OK_BUTTON:
            UtkLog.d(this, "OK Clicked! isCurCmdSetupCall[" + appService.isCurCmdSetupCall() + "]");
            if (appService.isCurCmdSetupCall()) {
                UtkLog.d(this, "stk call sendBroadcast(STKCALL_REGISTER_SPEECH_INFO)");
                Intent intent = new Intent("com.android.stk.STKCALL_REGISTER_SPEECH_INFO");
                sendBroadcast(intent);
            }

            sendResponse(UtkAppService.RES_ID_CONFIRM, true);
            finish();
            break;
        case CANCEL_BUTTON:
            sendResponse(UtkAppService.RES_ID_CONFIRM, false);
            finish();
            break;
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            sendResponse(UtkAppService.RES_ID_BACKWARD);
            finish();
            break;
        }
        return false;
    }

    @Override
    public void onResume() {
        super.onResume();
        startTimeOut();
    }

    @Override
    public void onPause() {
        super.onPause();

        cancelTimeOut();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        cancelTimeOut();
    }
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putParcelable(TEXT, mTextMsg);
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

        mTextMsg = savedInstanceState.getParcelable(TEXT);
    }

    private void sendResponse(int resId, boolean confirmed) {
        Bundle args = new Bundle();
        args.putInt(UtkAppService.OPCODE, UtkAppService.OP_RESPONSE);
        args.putInt(UtkAppService.RES_ID, resId);
        args.putBoolean(UtkAppService.CONFIRMATION, confirmed);
        startService(new Intent(this, UtkAppService.class).putExtras(args));
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
    }

    private void cancelTimeOut() {
        mTimeoutHandler.removeMessages(MSG_ID_TIMEOUT);
    }

    private void startTimeOut() {
        if(!mTextMsg.userClear)
        {
            // Reset timeout.
            cancelTimeOut();
            int dialogDuration = UtkApp.calculateDurationInMilis(mTextMsg.duration);
            UtkLog.d(this, "==========>   dialogDuration = " + dialogDuration);
            if (dialogDuration == 0) {
                dialogDuration = UtkApp.DEFAULT_DURATION_TIMEOUT;
            }
            mTimeoutHandler.sendMessageDelayed(mTimeoutHandler
                    .obtainMessage(MSG_ID_TIMEOUT), dialogDuration);
        }
    }
}
