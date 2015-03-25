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
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.engineermode.wifi;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import com.mediatek.engineermode.R;

public class MtkCTIATestDialog extends AlertDialog implements
        DialogInterface.OnClickListener, AdapterView.OnItemSelectedListener {

    private static final String TAG = "EM/CTIATestDialog";

    private static final int POSITIVE_BUTTON = BUTTON1;
    private static final int NEGATIVE_BUTTON = BUTTON2;

    // Preferences
    private static final String CTIA_PREF = "CTIA_PREF";
    private static final String PREF_CTIA_ENABLE = "CTIA_ENABLE";
    private static final String PREF_CTIA_RATE = "CTIA_RATE";
    private static final String PREF_CTIA_POWER = "CTIA_POWER_MODE";

    // add by mtk03034
    private static final String PREF_CTIA_DUMP_BEACON = "CTIA_DUMP_1";
    private static final String PREF_CTIA_DUMP_COUNTER = "CTIA_DUMP_2";
    private static final String PREF_CTIA_DUMP_INTERVAL = "CTIA_DUMP_3";
    
    private static final int NOTIF_ID_ENABLE_CTIA = 10;

    private View mView = null;
    private WifiManager mWm = null;
    private Context mContext = null;

    // Supported rate
    private String[] mRate = { "Automatic", "1M", "2M", "5_5M", "11M", "6M",
            "9M", "12M", "18M", "24M", "36M", "48M", "54M", "20MCS0800",
            "20MCS01800", "20MCS2800", "20MCS3800", "20MCS4800", "20MCS5800",
            "20MCS6800", "20MCS7800", "20MCS0400", "20MCS1400", "20MCS2400",
            "20MCS3400", "20MCS4400", "20MCS5400", "20MCS6400", "20MCS7400",
            "40MCS0800", "40MCS1800", "40MCS2800", "40MCS3800", "40MCS4800",
            "40MCS5800", "40MCS6800", "40MCS7800", "40MCS32800", "40MCS0400",
            "40MCS1400", "40MCS2400", "40MCS3400", "40MCS4400", "40MCS5400",
            "40MCS6400", "40MCS7400", "40MCS32400", };

    // Supported power saving mode
    /*
     * private String[] mPsMode = { "CAM", "Maximum PS", "Fast PS", };
     */

    private CheckBox mCheckbox = null;
    private Spinner mRateSpinner = null;
    // private Spinner mPsSpinner = null;
    private int mRateVal = 0;
    private int mPowerMode = 0;
    private Button mGetBtn = null;
    private Button mSetBtn = null;
    private EditText mIdEditText = null;
    private EditText mValEditText = null;

    // add 20111107
    private EditText mIntervalEditText = null;

    // add 20111107
    private CheckBox mDumpBeaconCheckbox = null;
    private CheckBox mDumpCounterCheckbox = null;

    protected MtkCTIATestDialog(Context context) {
        super(context);
        mContext = context;
        mWm = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
    }

    /**
     * Called after flags are set, the dialog's layout/etc should be set up here
     */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        onLayout();
        restorePrefs();
        super.onCreate(savedInstanceState);
    }

    private void setLayout(int layoutResId) {
        mView = getLayoutInflater().inflate(layoutResId, null);
        setView(mView);
        onReferenceViews(mView);
    }

    private void onLayout() {
        setLayout(R.layout.ctiasetting);
        int positiveButtonResId = R.string.wifi_ok;
        int negativeButtonResId = R.string.wifi_cancel;
        int neutralButtonResId = 0;
        setButtons(positiveButtonResId, negativeButtonResId, neutralButtonResId);
    }

    /** Called when the widgets are in-place waiting to be filled with data */
    private void onFill() {
        // Todo
    }

    private Button.OnClickListener mBtnClickListener = new Button.OnClickListener() {
        public void onClick(View v) {
            int val = 0;
            int id = 0;

            if (v == mSetBtn) {
                try {
                    // Parse and format the ID to hexadecimal
                    id = (int) Long.parseLong(mIdEditText.getText().toString(),
                            16);
                    // Parse and format the Value to hexadecimal
                    val = (int) Long.parseLong(mValEditText.getText()
                            .toString(), 16);
                    // SW Flag set
                    int ret = EMWifi.doCTIATestSet(id, val);
                    if (0 != ret) {
                        mValEditText.setText("ERROR");
                    }
                    Log.v(TAG, "Set ret: " + ret + " ID: " + id + " VAL: "
                            + val);
                } catch (NumberFormatException e) {
                    Log.v(TAG, "set number format error");
                    mValEditText.setText("0");
                }
            } else if (v == mGetBtn) {
                try {
                    // Parse and format the ID to hexadecimal
                    id = (int) Long.parseLong(mIdEditText.getText().toString(),
                            16);
                    long[] value = { 0 };
                    int ret = EMWifi.doCTIATestGet(id, value);
                    Log.v(TAG, "Get ret: " + ret + " ID: " + id + " VAL: "
                            + value[0]);
                    // mValEditText.setText(Long.toString(val, 16));
                    mValEditText.setText(ret == 0 ? Long.toString(value[0], 16)
                            : "UNKNOWN");
                } catch (NumberFormatException e) {
                    Log.v(TAG, "get number format error");
                    mValEditText.setText("0");
                }
            }
        }
    };

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private void savePrefs() {
        // Save user preferences.
        SharedPreferences settings = mContext
                .getSharedPreferences(CTIA_PREF, 0);
        Editor editor = settings.edit();
        editor.putBoolean(PREF_CTIA_ENABLE, mCheckbox.isChecked());
        editor.putInt(PREF_CTIA_RATE, getRateFromSpinner());
        // edit.putInt(PREF_CTIA_POWER,
        // getPsModeFromSpinner());
        editor.putBoolean(PREF_CTIA_DUMP_BEACON, mDumpBeaconCheckbox.isChecked());
        editor.putBoolean(PREF_CTIA_DUMP_COUNTER, mDumpCounterCheckbox
                .isChecked());

        int tmpInterval = 0;
        try {
            tmpInterval = Integer.parseInt(mIntervalEditText.getText()
                    .toString());
        } catch (NumberFormatException e) {
            tmpInterval = 1;
        }
        if (tmpInterval > 255) {
            tmpInterval = 255;
        } else if (tmpInterval < 1) {
            tmpInterval = 1;
        }
        editor.putInt(PREF_CTIA_DUMP_INTERVAL, tmpInterval);
        editor.commit();
    }

    private void restorePrefs() {
        SharedPreferences settings = mContext
                .getSharedPreferences(CTIA_PREF, 0);
        boolean prefEnableCtia = settings.getBoolean(PREF_CTIA_ENABLE, false);
        int prefRate = settings.getInt(PREF_CTIA_RATE, 0);

        boolean prefDumpBeacon = settings.getBoolean(PREF_CTIA_DUMP_BEACON,
                false);
        boolean prefDumpCounter = settings.getBoolean(PREF_CTIA_DUMP_COUNTER,
                false);

        int interval = settings.getInt(PREF_CTIA_DUMP_INTERVAL, 1);

        // int pref_power = settings.getInt(PREF_CTIA_POWER, 0);

        mCheckbox.setChecked(prefEnableCtia);
        mRateSpinner.setSelection(prefRate);
        // mPsSpinner.setSelection(pref_power);

        mDumpBeaconCheckbox.setChecked(prefDumpBeacon);
        mDumpCounterCheckbox.setChecked(prefDumpCounter);
        mIntervalEditText.setText(interval + "");

    }

    /** Called when we need to set our member variables to point to the views. */
    private void onReferenceViews(View view) {
        mRateSpinner = (Spinner) view.findViewById(R.id.rate_spinner);
        // mPsSpinner = (Spinner) view.findViewById(R.id.ps_spinner);
        setSpinnerAdapter(mRateSpinner, mRate);
        // setSpinnerAdapter(mPsSpinner, mPsMode);

        mGetBtn = (Button) view.findViewById(R.id.get_btn);
        mSetBtn = (Button) view.findViewById(R.id.set_btn);

        mSetBtn.setOnClickListener(mBtnClickListener);
        mGetBtn.setOnClickListener(mBtnClickListener);

        mRateSpinner.setOnItemSelectedListener(this);
        // mPsSpinner.setOnItemSelectedListener(this);
        mCheckbox = (CheckBox) view.findViewById(R.id.enable_checkbox);

        mIdEditText = (EditText) view.findViewById(R.id.idedittext);
        mValEditText = (EditText) view.findViewById(R.id.valedittext);
        mIntervalEditText = (EditText) view
                .findViewById(R.id.interval_edittext);

        // Add dump beacon/probe response
        mDumpBeaconCheckbox = (CheckBox) view
                .findViewById(R.id.enable_dump_checkbox);
        mDumpCounterCheckbox = (CheckBox) view
                .findViewById(R.id.enable_dump_counter_checkbox);

    }

    private void setSpinnerAdapter(Spinner spinner, String[] items) {
        if (items != null) {
            ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(
                    getContext(), android.R.layout.simple_spinner_item, items);
            adapter
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(adapter);
        }
    }

    private void setButtons(int positiveResId, int negativeResId,
            int neutralResId) {
        final Context context = getContext();
        if (positiveResId > 0) {
            setButton(context.getString(positiveResId), this);
        }
        if (negativeResId > 0) {
            setButton2(context.getString(negativeResId), this);
        }
        if (neutralResId > 0) {
            setButton3(context.getString(neutralResId), this);
        }
    }

    private void handleRateChange(int rate) {
        // Log.v(TAG, "rate = " + rate);
        mRateVal = rate;
    }

    private void handlePsChange(int psMode) {
        // Log.v(TAG, "ps_mode = " + psMode);
        mPowerMode = psMode;
    }

    private int getRateFromSpinner() {
        int position = mRateSpinner.getSelectedItemPosition();
        return position;
    }

    /*
     * private int getPsModeFromSpinner() { int position =
     * mPsSpinner.getSelectedItemPosition(); return position; }
     */

    public void onItemSelected(AdapterView parent, View view, int position,
            long id) {
        if (parent == mRateSpinner) {
            handleRateChange(getRateFromSpinner());
        }
        /*
         * else if (parent == mPsSpinner) {
         * handlePsChange(getPsModeFromSpinner()); }
         */
    }

    public void onClick(DialogInterface arg0, int arg1) {
        if (arg1 == POSITIVE_BUTTON) {
            savePrefs();
            if (mCheckbox.isChecked()) {
                Log.v(TAG, "doCTIATestOn: " + mWm.doCtiaTestOn());
                Log.v(TAG, "doCTIATestRate: " + mWm.doCtiaTestRate(mRateVal));
                // Log.v(TAG, "doCTIATestPower: " +
                // mWm.doCTIATestPower(mPowerMode));
                // mWm.doCTIATestSet(, val);
                WifiManager wifiManager = (WifiManager) this.getContext().getSystemService(Context.WIFI_SERVICE);
                int state = wifiManager.getWifiState();
                if (state == WifiManager.WIFI_STATE_ENABLED) {
                    notifyCtiaEnabled(getContext());
                }

            } else {
                Log.v(TAG, "doCTIATestOff: " + mWm.doCtiaTestOff());
                Log.v(TAG, "doCTIATestRate: " + mWm.doCtiaTestRate(mRateVal));
                // Log.v(TAG, "doCTIATestPower: " +
                // mWm.doCTIATestPower(mPowerMode));
                dismissCtiaEnabledNotify(getContext());
            }

            int id = 0;
            int val = 0;
            id = (int) Long.parseLong("10020000", 16);
            val = mDumpBeaconCheckbox.isChecked() ? 1 : 0;
            Log.v(TAG, "doCTIATestSet: id: " + id + " val: " + val + " result: "
                    + EMWifi.doCTIATestSet(id, val));
            // add dump case
            int tmpInterval = 0;
            try {
                tmpInterval = Integer.parseInt(mIntervalEditText.getText()
                        .toString());
            } catch (NumberFormatException e) {
                tmpInterval = 1;
                mIntervalEditText.setText("1");
            }
            if (tmpInterval > 255) {
                mIntervalEditText.setText("255");
                tmpInterval = 255;
            } else if (tmpInterval < 1) {
                mIntervalEditText.setText("1");
                tmpInterval = 1;
            }
            // handle the dump counter case
            id = (int) Long.parseLong("10020001", 16);
            val = (int) Long.parseLong("0000"
                    + Integer.toHexString(tmpInterval)
                    + (mDumpCounterCheckbox.isChecked() ? "01" : "00"), 16);
            Log.v(TAG, "doCTIATestSet: id: " + id + " val: " + val + " result: "
                    + EMWifi.doCTIATestSet(id, val));
            this.dismiss();
        } else if (arg1 == NEGATIVE_BUTTON) {
            Log.v(TAG, "cancel");
            this.dismiss();
        }
    }

    public void onNothingSelected(AdapterView<?> arg0) {
    }
    
    public static void initWifiCtiaOnEnabled(Context context) {
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        SharedPreferences preferences = context
                .getSharedPreferences(CTIA_PREF, 0);
        boolean enableCtia = preferences.getBoolean(PREF_CTIA_ENABLE, false);
        int rate = preferences.getInt(PREF_CTIA_RATE, 0);

        boolean enableDumpBeacon = preferences.getBoolean(PREF_CTIA_DUMP_BEACON,
                false);
        boolean enableDumpCounter = preferences.getBoolean(PREF_CTIA_DUMP_COUNTER,
                false);

        int interval = preferences.getInt(PREF_CTIA_DUMP_INTERVAL, 1);

        if (enableCtia) {
            Log.v(TAG, "doCTIATestOn: " + wifiManager.doCtiaTestOn());
            Log.v(TAG, "doCTIATestRate: " + wifiManager.doCtiaTestRate(rate));
        } else {
            Log.v(TAG, "doCTIATestOff: " + wifiManager.doCtiaTestOff());
            Log.v(TAG, "doCTIATestRate: " + wifiManager.doCtiaTestRate(rate));
        }
        int id = 0;
        int val = 0;
        // handle dump beacon
        id = (int) Long.parseLong("10020000", 16);
        val = enableDumpBeacon ? 1 : 0;
        Log.v(TAG, "doCTIATestSet: id: " + id + " val: " + val + " result: "
                + EMWifi.doCTIATestSet(id, val));
        // handle the dump counter case
        id = (int) Long.parseLong("10020001", 16);
        val = (int) Long.parseLong("0000"
                + Integer.toHexString(interval)
                + (enableDumpCounter ? "01" : "00"), 16);
        Log.v(TAG, "doCTIATestSet: id: " + id + " val: " + val + " result: "
                + EMWifi.doCTIATestSet(id, val));
    }
    
    public static boolean isWifiCtiaEnabled(Context context) {
        boolean enabled = false;
        SharedPreferences preferences = context.getSharedPreferences(CTIA_PREF, 0);
        enabled = preferences.getBoolean(PREF_CTIA_ENABLE, false);
        return enabled;
    }
    
    public static void notifyCtiaEnabled(Context context) {
        emitNotif(context, NOTIF_ID_ENABLE_CTIA, "WIFI CTIA is Enabled", 
                "click here to switch CTIA mode", WifiTestSetting.class);
    } 
    
    public static void dismissCtiaEnabledNotify(Context context) {
        NotificationManager nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(NOTIF_ID_ENABLE_CTIA);
    }
    
    static void emitNotif(Context context, int id, String title, String content, Class<? extends Activity> targetClass) {
        
        Notification notif = new Notification(android.R.drawable.ic_dialog_info, 
                title, System.currentTimeMillis());
        notif.flags |= Notification.FLAG_NO_CLEAR;
        Intent intent = new Intent(context, targetClass);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent pi = PendingIntent.getActivity(context, 0, intent, 0);
        notif.setLatestEventInfo(context, title, content, pi);
        NotificationManager nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        nm.notify(id, notif);
    }

}
