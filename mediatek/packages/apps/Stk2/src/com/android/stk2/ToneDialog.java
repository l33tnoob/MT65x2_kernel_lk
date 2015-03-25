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
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Vibrator;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.ToneSettings;

/**
 * Activity used for PLAY TONE command.
 */
public class ToneDialog extends Activity {
    TextMessage toneMsg = null;
    ToneSettings settings = null;
    TonePlayer player = null;

    Vibrator mVibrator = null;

    private static final String LOGTAG = "Stk2-TD ";
    private boolean mbSendResp = false;

    private final IntentFilter mSIMStateChangeFilter = new IntentFilter(
            TelephonyIntents.ACTION_SIM_STATE_CHANGED);

    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {

                String simState = intent.getStringExtra(IccCard.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(
                        com.android.internal.telephony.Phone.GEMINI_SIM_ID_KEY, -1);

                CatLog.d(LOGTAG, "mSIMStateChangeReceiver - simId[" + simId + "]  state["
                        + simState + "]");

                if ((simId == com.android.internal.telephony.Phone.GEMINI_SIM_2) &&
                        (IccCard.INTENT_VALUE_ICC_NOT_READY.equals(simState))) {
                    ToneDialog.this.finish();
                }
            }
        }
    };

    /**
     * Handler used to stop tones from playing when the duration ends.
     */
    Handler mToneStopper = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ID_STOP_TONE:
                                sendResponse(StkAppService.RES_ID_DONE);
                    finish();
                    break;
            }
        }
    };

    // Message id to signal tone duration timeout.
    private static final int MSG_ID_STOP_TONE = 0xda;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        CatLog.d(LOGTAG, "onCreate - mbSendResp[" + mbSendResp + "]");

        initFromIntent(getIntent());

        // remove window title
        View title = findViewById(com.android.internal.R.id.title);
        title.setVisibility(View.GONE);

        if ((null == toneMsg) || (null == toneMsg.text) || (toneMsg.text.equals(""))) {

        } else {
            // set customized content view
            setContentView(R.layout.stk_tone_dialog);

            TextView tv = (TextView) findViewById(R.id.message);
            ImageView iv = (ImageView) findViewById(R.id.icon);

            // set text and icon
            if (null != toneMsg) {
                tv.setText(toneMsg.text);
            } else {
                CatLog.d(LOGTAG, "onCreate - null tone text");
            }

            if (toneMsg.icon == null) {
                iv.setImageResource(com.android.internal.R.drawable.ic_volume);
            } else {
                iv.setImageBitmap(toneMsg.icon);
            }
        }

        // Start playing tone and vibration
        if (null == settings) {
            CatLog.d(LOGTAG, "onCreate - null settings - finish");
            finish();
        }

        player = new TonePlayer();
        player.play(settings.tone);
        int timeout = StkApp.calculateDurationInMilis(settings.duration);
        if (timeout == 0) {
            timeout = StkApp.TONE_DFEAULT_TIMEOUT;
        }

        if (mVibrator == null) {
            mVibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
        }
        mToneStopper.sendEmptyMessageDelayed(MSG_ID_STOP_TONE, timeout);
        if (settings.vibrate && mVibrator != null) {
            mVibrator.vibrate(timeout);
        } else {
            CatLog.d(LOGTAG, "onCreate - settings.vibrate = " + settings.vibrate);
            CatLog.d(LOGTAG, "onCreate - mVibrator is null " + (mVibrator == null));
        }

        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        CatLog.d(LOGTAG, "onDestroy - before Send End Session mbSendResp[" + mbSendResp + "]");
        if (!mbSendResp) {
            CatLog.d(LOGTAG, "onDestroy - Send End Session");
            sendResponse(StkAppService.RES_ID_END_SESSION);
        }

        mToneStopper.removeMessages(MSG_ID_STOP_TONE);
        if (null != player) {
            player.stop();
            player.release();
        }
        if (null != mVibrator) {
            mVibrator.cancel();
        }
        unregisterReceiver(mSIMStateChangeReceiver);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                sendResponse(StkAppService.RES_ID_END_SESSION);
                finish();
                break;
        }
        return false;
    }

    private void initFromIntent(Intent intent) {
        if (intent == null) {
            finish();
        }
        toneMsg = intent.getParcelableExtra("TEXT");
        settings = intent.getParcelableExtra("TONE");
    }

    private void sendResponse(int resId) {
        if (StkAppService.getInstance().haveEndSession()) {
            // ignore current command
            CatLog.d(LOGTAG, "Ignore response, id is " + resId);
            return;
        }

        CatLog.d(LOGTAG, "sendResponse resID[" + resId + "]");
        mbSendResp = true;
        Bundle args = new Bundle();
        args.putInt(StkAppService.OPCODE, StkAppService.OP_RESPONSE);
        args.putInt(StkAppService.RES_ID, resId);
        startService(new Intent(this, StkAppService.class).putExtras(args));
    }
}
