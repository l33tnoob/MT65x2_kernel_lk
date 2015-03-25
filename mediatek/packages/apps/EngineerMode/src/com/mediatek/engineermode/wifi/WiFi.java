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

package com.mediatek.engineermode.wifi;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.FeatureHelpPage;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

public class WiFi extends Activity implements OnItemClickListener {

    private static final String TAG = "EM/WiFi";
    private static final String KEY_PROP_WPAWPA2 = "persist.radio.wifi.wpa2wpaalone";
    private static final String KEY_PROP_CTIA = "mediatek.wlan.ctia";
    private static final String KEY_PROP_WPS2_SUPPORT = "persist.radio.wifi.wps2support";
    private static final String KEY_PROP_WPS_TEST_MODE = "persist.radio.hotspot.probe.rq";
    private static final String VALUE_TRUE = "true";
    private static final String VALUE_FALSE = "false";
    private static final String VALUE_0 = "0";
    private static final String VALUE_1 = "1";
    private static final int HANDLER_EVENT_INIT = 0x011;
    private static final int DIALOG_WIFI_INIT = 0;
    private static final int DIALOG_WIFI_WARN = 1;
    private static final int DIALOG_WIFI_FAIL = 2;
    private static final int DIALOG_WIFI_ERROR = 3;
    protected static final String KEY_CHIP_ID = "WiFiChipID";
    private static final int ITEM_INDEX_0 = 0;
    private static final int ITEM_INDEX_1 = 1;
    private static final int ITEM_INDEX_2 = 2;
    private static final int ITEM_INDEX_3 = 3;
    private static final int ITEM_INDEX_4 = 4;
    private static final long FUNC_INDEX_VERSION = 47;
    private static final long MASK_HIGH_16_BIT = 0xFFFF0000;
    private static final long MASK_HIGH_8_BIT = 0xFF00;
    private static final long MASK_8_BIT = 0xFF;
    private static final int BIT_16 = 16;
    private static final int BIT_8 = 8;
    private WifiManager mWifiManager = null;
    private int mChipID = 0x00;
    private WiFiStateManager mWiFiStateManager = null;
    private ListView mListTestItem = null;
    private CheckBox mCbWfa = null;
    private CheckBox mCbCtia = null;
    private CheckBox mCbOpenWps2 = null;
    private CheckBox mCbWps = null;
    
    private final Handler mHandler = new Handler() {

        public void handleMessage(Message msg) {

            if (HANDLER_EVENT_INIT == msg.what) {
                removeDialog(DIALOG_WIFI_INIT);
                checkWiFiChipStatus();
                Xlog.d(TAG,
                        "The Handle event is : HANDLER_EVENT_INIT, miChipID = "
                                + mChipID);
                ArrayList<String> items = new ArrayList<String>();
                items.add(getString(R.string.wifi_item_tx));
                items.add(getString(R.string.wifi_item_rx));
                items.add(getString(R.string.wifi_item_mcr));
                items.add(getString(R.string.wifi_item_nvram));
                items.add(getString(R.string.help));
                ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                        WiFi.this, android.R.layout.simple_list_item_1, items);
                mListTestItem.setAdapter(adapter);
                showVersion();
                ChannelInfo.getSupportChannels();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi);
        mCbWfa = (CheckBox) findViewById(R.id.wifi_wfa_switcher);
        mCbWfa.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbWfa.isChecked();
                SystemProperties.set(KEY_PROP_WPAWPA2, newState ? VALUE_TRUE
                        : VALUE_FALSE);
            }
        });
        mCbCtia = (CheckBox) findViewById(R.id.wifi_ctia_switcher);
        mCbCtia.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbCtia.isChecked();
                SystemProperties.set(KEY_PROP_CTIA, newState ? VALUE_1
                        : VALUE_0);
            }
        });
        mCbOpenWps2 = (CheckBox) findViewById(R.id.wifi_ap_wps2_support);
        mCbOpenWps2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbOpenWps2.isChecked();
                SystemProperties.set(KEY_PROP_WPS2_SUPPORT,
                        newState ? VALUE_TRUE : VALUE_FALSE);
            }
        });
        mCbWps = (CheckBox) findViewById(R.id.wifi_wps_test_mode);
        mCbWps.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbWps.isChecked();
                SystemProperties.set(KEY_PROP_WPS_TEST_MODE,
                        newState ? VALUE_TRUE : VALUE_FALSE);
                mWifiManager.setApProbeRequestEnabled(newState);
            }
        });
        mWifiManager = (WifiManager) this
                .getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager == null) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        } else {
            if (WifiManager.WIFI_STATE_DISABLED != mWifiManager.getWifiState()) {
                showDialog(DIALOG_WIFI_WARN);
                return;
            }
        }
        mListTestItem = (ListView) findViewById(R.id.ListView_WiFi);
        if (null == mListTestItem) {
            Xlog.w(TAG, "findViewById(R.id.ListView_WiFi) failed");
        } else {
            mListTestItem.setOnItemClickListener(this);
        }
        showDialog(DIALOG_WIFI_INIT);
        new Thread() {
            public void run() {
                if (mWiFiStateManager == null) {
                    mWiFiStateManager = new WiFiStateManager(WiFi.this);
                }
                mChipID = mWiFiStateManager.checkState(WiFi.this);
                mHandler.sendEmptyMessage(HANDLER_EVENT_INIT);
            }
        }.start();
        startService(new Intent(this, WifiStateListener.class));
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_WIFI_INIT:
            ProgressDialog innerDialog = new ProgressDialog(this);
            innerDialog.setTitle(R.string.wifi_dialog_init);
            innerDialog
                    .setMessage(getString(R.string.wifi_dialog_init_message));
            innerDialog.setCancelable(false);
            innerDialog.setIndeterminate(true);
            dialog = innerDialog;
            break;
        case DIALOG_WIFI_WARN:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_warn);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_warn_message));
            builder.setPositiveButton(R.string.wifi_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        case DIALOG_WIFI_FAIL:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_fail);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_fail_message));
            builder.setPositiveButton(R.string.wifi_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        case DIALOG_WIFI_ERROR:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_error);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_error_message));
            builder.setPositiveButton(R.string.wifi_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        default:
            Xlog.d(TAG, "error dialog ID");
            break;
        }
        return dialog;
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mCbWfa.setChecked(VALUE_TRUE.equals(SystemProperties.get(
                KEY_PROP_WPAWPA2, VALUE_FALSE)));
        mCbCtia.setChecked(1 == SystemProperties.getInt(KEY_PROP_CTIA, 0));
        mCbOpenWps2.setChecked(VALUE_TRUE.equals(SystemProperties.get(
                KEY_PROP_WPS2_SUPPORT, VALUE_TRUE)));
        mCbWps.setChecked(VALUE_TRUE.equals(SystemProperties.get(
                           KEY_PROP_WPS_TEST_MODE, VALUE_FALSE)));
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Xlog.v(TAG, "-->onItemClick, item index = " + arg2);
        Intent intent = null;
        if (WiFiStateManager.CHIP_ID_6620 == mChipID) {
            switch (arg2) {
            case ITEM_INDEX_0: // Tx test
                intent = new Intent(this, WiFiTx6620.class);
                break;
            case ITEM_INDEX_1: // Rx test
                intent = new Intent(this, WiFiRx6620.class);
                break;
            case ITEM_INDEX_2: // MCR read and write
                intent = new Intent(this, WiFiMcr.class);
                break;
            case ITEM_INDEX_3: // EEPROM read and write
                intent = new Intent(this, WiFiEeprom.class);
                break;
            case ITEM_INDEX_4: // Help
                intent = new Intent(this, FeatureHelpPage.class);
                intent.putExtra(FeatureHelpPage.HELP_TITLE_KEY, R.string.help);
                intent.putExtra(FeatureHelpPage.HELP_MESSAGE_KEY, R.string.wifi_help_msg);
                break;
            default:
                break;
            }
        } else {
            Xlog.d(TAG, "unknown chip ID: " + mChipID);
        }
        if (null == intent) {
            Toast.makeText(this, R.string.wifi_toast_select_error,
                    Toast.LENGTH_LONG).show();
            Xlog.d(TAG, "select test item error");
        } else {
            intent.putExtra(KEY_CHIP_ID, mChipID);
            this.startActivity(intent);
        }
    }

    /**
     * Show WiFi firmware version
     */
    private void showVersion() {
        TextView mVersion = (TextView) findViewById(R.id.wifi_version);
        if (EMWifi.sIsInitialed) {
            StringBuilder stringBuild = new StringBuilder();
            stringBuild.append("VERSION: CHIP = MT");
            long[] version = new long[1];
            int result = EMWifi.getATParam(FUNC_INDEX_VERSION, version);
            if (0 == result) {
                Xlog.v(TAG, "version number is: 0x"
                        + Long.toHexString(version[0]));
                stringBuild.append(Long.toHexString((version[0] & MASK_HIGH_16_BIT) >> BIT_16));
                stringBuild.append("  FW VER = v");
                stringBuild.append(Long.toHexString((version[0] & MASK_HIGH_8_BIT) >> BIT_8));
                stringBuild.append(".");
                stringBuild.append(Long.toHexString(version[0] & MASK_8_BIT));
                mVersion.setText(stringBuild.toString());
            } else {
                mVersion.setText("VERSION: Get fail");
            }
        } else {
            mVersion.setText("VERSION: UNKNOWN");
        }
        Xlog.v(TAG, "Wifi Chip Version is: " + mVersion.getText());
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mWiFiStateManager = null;
        stopService(new Intent(this, WifiStateListener.class));
        if (EMWifi.sIsInitialed) {
            EMWifi.setNormalMode();
            EMWifi.unInitial();
            EMWifi.sIsInitialed = false;
            mWifiManager.setWifiEnabled(false);
        }
        super.onDestroy();
    }

    /**
     * Check WiFi chip status
     */
    private void checkWiFiChipStatus() {
        switch (mChipID) {
        case WiFiStateManager.ENABLE_WIFI_FAIL:
            showDialog(DIALOG_WIFI_FAIL);
            break;
        case WiFiStateManager.CHIP_READY:
        case WiFiStateManager.INVALID_CHIP_ID:
        case WiFiStateManager.SET_TEST_MODE_FAIL:
            showDialog(DIALOG_WIFI_ERROR);
            break;
        case WiFiStateManager.CHIP_ID_6620:
        case WiFiStateManager.CHIP_ID_5921:
            break;
        default:
            break;
        }
    }

}
