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
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Toast;

import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccUtils;
import com.android.internal.telephony.PhoneProxy;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.rfdesense.RfDesenseTxTestBase;

public class RfDesenseTxTestGsm extends RfDesenseTxTestBase {
    private static final String TAG = "TxTestGsm";

    public static final String KEY_BAND = "Band_2G";
    public static final String KEY_MODULATION = "Bodulation_2G";
    public static final String KEY_CHANNEL = "Channel_2G";
    public static final String KEY_POWER = "Power_2G";
    public static final String KEY_AFC = "AFC_2G";
    public static final String KEY_TSC = "TSC_2G";
    public static final String KEY_PATTERN = "Pattern_2G";
    public static final String KEY_STATE = "Started_2G";

    @Override
    protected void onPause() {
        super.onPause();
        SharedPreferences.Editor pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE).edit();
        if (mState != STATE_STOPPED) {
            pref.putInt(KEY_BAND, mBand.getSelectedItemPosition());
//            pref.putInt(KEY_MODULATION, mModulation.getCheckedRadioButtonId());
            pref.putString(KEY_CHANNEL, mChannel.getText());
            pref.putString(KEY_POWER, mPower.getText());
            pref.putString(KEY_AFC, mAfc.getText());
            pref.putString(KEY_TSC, mTsc.getText());
            pref.putInt(KEY_PATTERN, mPattern.getSelectedItemPosition());
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

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_gsm_band,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter);

        ArrayAdapter<CharSequence> adapter2 = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_gsm_pattern,
                android.R.layout.simple_spinner_item);
        adapter2.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mPattern.setAdapter(adapter2);

        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE);

        mBand.setSelection(pref.getInt(KEY_BAND, 0));
//        mModulation.check(pref.getInt(KEY_MODULATION, R.id.modulation_gmsk));
        if (mCurrentBand != mBand.getSelectedItemPosition()) {
            updateLimits();
            mCurrentBand = mBand.getSelectedItemPosition();
        }
        mChannel.setText(pref.getString(KEY_CHANNEL, mChannel.defaultValue));
        mPower.setText(pref.getString(KEY_POWER, mPower.defaultValue));
        mAfc.setText(pref.getString(KEY_AFC, mAfc.defaultValue));
        mTsc.setText(pref.getString(KEY_TSC, mTsc.defaultValue));
        mPattern.setSelection(pref.getInt(KEY_PATTERN, 0));
        mState = pref.getInt(KEY_STATE, STATE_NONE);
    }

/*
    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == REBOOT) {
            OnClickListener listener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (which == DialogInterface.BUTTON_POSITIVE) {
                        SharedPreferences.Editor pref = getPreferences(MODE_PRIVATE).edit();
                        pref.clear();
                        pref.commit();
                        sendAtCommand("AT+CFUN=1,1", "", REBOOT);
                        disableAllButtons();
                        mState = STATE_STOPPED;
                        finish();
                    }
                    dialog.dismiss();
                }
            };

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            return builder.setTitle("Reboot")
                          .setMessage("Reboot phone?")
                          .setPositiveButton("Reboot", listener)
                          .setNegativeButton("Cancel", listener)
                          .create();
        }
        return super.onCreateDialog(id);
    }
*/

    protected void updateLimits() {
        int band = mBand.getSelectedItemPosition();
//        long modulation = mModulation.getCheckedRadioButtonId();
        String[] limits;

//        if (modulation == R.id.modulation_gmsk) {
            limits = getResources().getStringArray(R.array.rf_desense_tx_test_gsm_gmsk_limits)[band].split(",");
//        } else {
//            limits = getResources().getStringArray(R.array.rf_desense_tx_test_gsm_edge_limits)[band].split(",");
//        }

        mChannel.set(limits[CHANNEL_DEFAULT], limits[CHANNEL_MIN], limits[CHANNEL_MAX], limits[CHANNEL_MIN2], limits[CHANNEL_MAX2]);
        mPower.set(limits[POWER_DEFAULT], limits[POWER_MIN], limits[POWER_MAX]);
        mPower.step = 2;
        mAfc.set("4100", "0", "8191");
        mTsc.set("0", "0", "7");
    }
}
