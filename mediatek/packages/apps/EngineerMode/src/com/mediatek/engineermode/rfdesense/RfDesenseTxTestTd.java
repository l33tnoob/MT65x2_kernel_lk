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

package com.mediatek.engineermode.rfdesense;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.rfdesense.RfDesenseTxTestBase;

public class RfDesenseTxTestTd extends RfDesenseTxTestBase {
    private static final String TAG = "TxTestTd";

    public static final String KEY_BAND = "Band_3G";
    public static final String KEY_MODULATION = "Bodulation_3G";
    public static final String KEY_CHANNEL = "Channel_3G";
    public static final String KEY_POWER = "Power_3G";
    public static final String KEY_AFC = "AFC_3G";
    public static final String KEY_TSC = "TSC_3G";
    public static final String KEY_PATTERN = "Pattern_3G";
    public static final String KEY_STATE = "Started_3G";

    @Override
    protected void onPause() {
        super.onPause();
        SharedPreferences.Editor pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE).edit();
        if (mState != STATE_STOPPED) {
            pref.putInt(KEY_BAND, mBand.getSelectedItemPosition());
            pref.putString(KEY_CHANNEL, mChannel.getText());
            pref.putString(KEY_POWER, mPower.getText());
            pref.putInt(KEY_STATE, mState);
        } else {
            pref.clear();
        }
        pref.commit();
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        findViewById(R.id.afc).setVisibility(View.GONE);
        findViewById(R.id.tsc).setVisibility(View.GONE);
        findViewById(R.id.pattern).setVisibility(View.GONE);
        mAfc.editor.setVisibility(View.GONE);
        mTsc.editor.setVisibility(View.GONE);
        mPattern.setVisibility(View.GONE);

        TextView channelLabel = (TextView)findViewById(R.id.channel);
        channelLabel.setText(R.string.rf_desense_channel_3g);

        Button.OnClickListener listener = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (v.getId()) {
                case R.id.button_start:
                    if (!checkValues()) {
                        disableAllButtons();
                        mHandler.sendMessageDelayed(Message.obtain(mHandler, UPDATE_BUTTON), 1000);
                        break;
                    }

                    String band;
                    if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
                        band = getResources().getStringArray(R.array.rf_desense_tx_test_td_band_values)[mBand.getSelectedItemPosition()];
                    } else {
                        band = getResources().getStringArray(R.array.rf_desense_tx_test_fd_band_values)[mBand.getSelectedItemPosition()];
                    }
                    String channel = mChannel.getText();
                    String power = mPower.getText();

                    String command = "AT+ERFTX=0,0," + band + "," + channel + "," + power;
                    sendAtCommand(command, "", START);

                    disableAllButtons();
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, UPDATE_BUTTON), 1000);
                    mState = STATE_STARTED;
                    break;

                case R.id.button_pause:
                    sendAtCommand("AT+ERFTX=0,1", "", PAUSE);

                    disableAllButtons();
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, UPDATE_BUTTON), 1000);
                    mState = STATE_PAUSED;
                    break;

                case R.id.button_stop:
                    showDialog(REBOOT);
                    break;

                default:
                    break;
                }
            }
        };

        mButtonStart.setOnClickListener(listener);
        mButtonPause.setOnClickListener(listener);
        mButtonStop.setOnClickListener(listener);

        ArrayAdapter<CharSequence> adapter;
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_td_band,
                android.R.layout.simple_spinner_item);
        } else {
            adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_fd_band,
                android.R.layout.simple_spinner_item);
        }
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter);

        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE);

        mBand.setSelection(pref.getInt(KEY_BAND, 0));
        if (mCurrentBand != mBand.getSelectedItemPosition()) {
            updateLimits();
            mCurrentBand = mBand.getSelectedItemPosition();
        }
        mChannel.setText(pref.getString(KEY_CHANNEL, mChannel.defaultValue));
        mPower.setText(pref.getString(KEY_POWER, mPower.defaultValue));
        mState = pref.getInt(KEY_STATE, STATE_NONE);
    }

    @Override
    protected void updateLimits() {
        int band = mBand.getSelectedItemPosition();
        String[] limits;

        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            limits = getResources().getStringArray(R.array.rf_desense_tx_test_td_limits)[band].split(",");
        } else {
            limits = getResources().getStringArray(R.array.rf_desense_tx_test_fd_limits)[band].split(",");
        }

        mChannel.set(limits[CHANNEL_DEFAULT], limits[CHANNEL_MIN], limits[CHANNEL_MAX], limits[CHANNEL_MIN2], limits[CHANNEL_MAX2]);
        mPower.set(limits[POWER_DEFAULT], limits[POWER_MIN], limits[POWER_MAX]);
    }
}
