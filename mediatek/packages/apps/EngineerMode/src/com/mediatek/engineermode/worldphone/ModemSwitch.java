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

package com.mediatek.engineermode.worldphone;

import android.app.Activity;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.worldphone.ModemSwitchHandler;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.android.internal.telephony.gemini.MTKPhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.gsm.GSMPhone;
import com.android.internal.telephony.PhoneProxy;

public class ModemSwitch extends Activity implements View.OnClickListener {
    private final static String TAG = "EM/ModemSwitch";

    private RadioButton mRadioFdd;
    private RadioButton mRadioTdd;
    private RadioButton mRadioAuto;
    private TextView mText;
    private EditText mTimer;
    private Button mButtonSet;
    private Button mButtonSetTimer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.modem_switch);

        mRadioFdd = (RadioButton) findViewById(R.id.modem_switch_fdd);
        mRadioTdd = (RadioButton) findViewById(R.id.modem_switch_tdd);
        mRadioAuto = (RadioButton) findViewById(R.id.modem_switch_auto);
        mText = (TextView) findViewById(R.id.modem_switch_current_value);
        mTimer = (EditText) findViewById(R.id.modem_switch_timer);
        mButtonSet = (Button) findViewById(R.id.modem_switch_set);
        mButtonSet.setOnClickListener(this);
        mButtonSetTimer = (Button) findViewById(R.id.modem_switch_set_timer);
        mButtonSetTimer.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        Xlog.d(TAG, "onResume()");
        super.onResume();

        int modemType = ModemSwitchHandler.getModem();
        Xlog.d(TAG, "Get modem type: " + modemType);

        if (modemType == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
            mText.setText(R.string.modem_switch_is_fdd);
            mRadioFdd.setChecked(true);
        } else if (modemType == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
            mText.setText(R.string.modem_switch_is_tdd);
            mRadioTdd.setChecked(true);
        } else {
            mText.setText(R.string.modem_switch_current_value);
            Toast.makeText(this, "Query Modem type failed: " + modemType, Toast.LENGTH_SHORT).show();
        }

        if (Settings.Global.getInt(getContentResolver(),
                Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 1) == 1) {
            mRadioFdd.setChecked(false);
            mRadioTdd.setChecked(false);
            mRadioAuto.setChecked(true);
        }

        int timer = Settings.Global.getInt(getContentResolver(),
                Settings.Global.WORLD_PHONE_FDD_MODEM_TIMER, 0);
        mTimer.setText(String.valueOf(timer));
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonSetTimer) {
            int timer = 0;
            try {
                timer = Integer.parseInt(mTimer.getText().toString());
            } catch (NumberFormatException e) {
                Xlog.w(TAG, "Invalid format: " + mTimer.getText());
                timer = 0;
            }
            Settings.Global.putInt(getContentResolver(),
                    Settings.Global.WORLD_PHONE_FDD_MODEM_TIMER, timer);
            Toast.makeText(ModemSwitch.this, "Set timer succeed.", Toast.LENGTH_SHORT).show();
            return;
        }

        boolean result = false;
        if (mRadioFdd.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                (((GeminiPhone)(MTKPhoneFactory.getDefaultPhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL);
            } else {
                PhoneProxy proxyPhone = (PhoneProxy)(MTKPhoneFactory.getDefaultPhone());
                (((GSMPhone)(proxyPhone.getActivePhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL);
            }
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
            }
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
                result = true;
            }
        } else if (mRadioTdd.isChecked()) {
            Xlog.d(TAG, "Set modem type: " + ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                (((GeminiPhone)(MTKPhoneFactory.getDefaultPhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL);
            } else {
                PhoneProxy proxyPhone = (PhoneProxy)(MTKPhoneFactory.getDefaultPhone());
                (((GSMPhone)(proxyPhone.getActivePhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL);
            }
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
                ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
            }
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                result = true;
            }
        } else if (mRadioAuto.isChecked()) {
            Xlog.d(TAG, "Set modem type: auto");
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                (((GeminiPhone)(MTKPhoneFactory.getDefaultPhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_AUTO);
            } else {
                PhoneProxy proxyPhone = (PhoneProxy)(MTKPhoneFactory.getDefaultPhone());
                (((GSMPhone)(proxyPhone.getActivePhone())).mWorldPhone).setNetworkSelectionMode(IWorldPhone.SELECTION_MODE_AUTO);
            }
            result = true;
        } else {
            return;
        }

        Xlog.d(TAG, "Set modem type result: " + result);
        if (result) {
            Toast.makeText(ModemSwitch.this, "Switch succeed.", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(ModemSwitch.this, "Switch failed.", Toast.LENGTH_SHORT).show();
        }

        int modemType = ModemSwitchHandler.getModem();
        Xlog.d(TAG, "Get modem type: " + modemType);
        if (modemType == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
            mText.setText(R.string.modem_switch_is_fdd);
        } else if (modemType == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
            mText.setText(R.string.modem_switch_is_tdd);
        }
    }
}
