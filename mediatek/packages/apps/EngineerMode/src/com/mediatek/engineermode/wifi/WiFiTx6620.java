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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERfETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.Arrays;
import java.util.Locale;

public class WiFiTx6620 extends WiFiTestActivity implements OnClickListener {
    private static final String TAG = "EM/WiFi_Tx";
    private static final long MAX_VALUE = 0xFFFFFFFF;
    private static final int MIN_VALUE = 0x0;
    private static final int HANDLER_EVENT_GO = 1;
    private static final int HANDLER_EVENT_STOP = 2;
    private static final int HANDLER_EVENT_TIMER = 3;
    private static final int HANDLER_EVENT_FINISH = 4;
    private static final int MAX_LOWER_RATE_NUMBER = 12;
    private static final int MAX_HIGH_RATE_NUMBER = 21;
    private static final int CCK_RATE_NUMBER = 4;
    private static final int DEFAULT_PKT_CNT = 3000;
    private static final int DEFAULT_PKT_LEN = 1024;
    private static final int DEFAULT_TX_GAIN = 0;
    private static final int ONE_SENCOND = 1000;
    private static final int BIT_8_MASK = 0xFF;
    private static final int LENGTH_3 = 3;
    private static final int BANDWIDTH_40MHZ_MASK = 0x8000;
    private static final int BANDWIDTH_INDEX_40 = 1;
    private static final int COMMAND_INDEX_OUTPUTPOWER = 4;
    private static final int COMMAND_INDEX_STOPTEST = 0;
    private static final int COMMAND_INDEX_STARTTX = 1;
    private static final int COMMAND_INDEX_LOCALFREQ = 5;
    private static final int COMMAND_INDEX_CARRIER = 6;
    private static final int COMMAND_INDEX_CARRIER_NEW = 10;
    private static final int RATE_MODE_MASK = 31;
    private static final int RATE_MCS_INDEX = 0x20;
    private static final int RATE_NOT_MCS_INDEX = 0x07;
    private static final int CWMODE_CCKPI = 5;
    private static final int CWMODE_OFDMLTF = 2;
    private static final int TXOP_LIMIT_VALUE = 0x00020000;
    private static final int TEST_MODE_TX = 0;
    private static final int TEST_MODE_DUTY = 1;
    private static final int TEST_MODE_CARRIER = 2;
    private static final int TEST_MODE_LEAKAGE = 3;
    private static final int TEST_MODE_POWEROFF = 4;
    private static final long[] PACKCONTENT_BUFFER = { 0xff220004, 0x33440006,
            0x55660008, 0x55550019, 0xaaaa001b, 0xbbbb001d };
    private boolean mHighRateSelected = false;
    private boolean mCCKRateSelected = true;
    private int mLastRateGroup = 0;
    private int mLastBandwidth = 0;
    private Spinner mChannelSpinner = null;
    private Spinner mGuardIntervalSpinner = null;
    private Spinner mBandwidthSpinner = null;
    private Spinner mPreambleSpinner = null;
    private EditText mEtPkt = null;
    private EditText mEtPktCnt = null;
    private EditText mEtTxGain = null;
    private Spinner mRateSpinner = null;
    private Spinner mModeSpinner = null;
    // private EditText mXTEdit = null;
    // private Button mWriteBtn = null;
    // private Button mReadBtn = null;
    // private CheckBox mALCCheck = null;
    private Button mBtnGo = null;
    private Button mBtnStop = null;
    private ArrayAdapter<String> mChannelAdapter = null;
    // private ArrayAdapter<String> mRateAdapter = null;
    private ArrayAdapter<String> mModeAdapter = null;
    private ArrayAdapter<String> mPreambleAdapter = null;
    // private ArrayAdapter<String> mGuardIntervalAdapter = null;
    // private ArrayAdapter<String> mBandwidthAdapter = null;
    private int mModeIndex = 0;
    private int mPreambleIndex = 0;
    private int mBandwidthIndex = 0;
    private int mGuardIntervalIndex = 0;
    private static final int ANTENNA = 0;

    private RateInfo mRate = null;
    private ChannelInfo mChannel = null;
    private long mPktLenNum = DEFAULT_PKT_LEN;
    private long mCntNum = DEFAULT_PKT_CNT;
    private long mTxGainVal = DEFAULT_TX_GAIN;
    private boolean mTestInPorcess = false;
    private HandlerThread mTestThread = null;
    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (HANDLER_EVENT_FINISH == msg.what) {
                Xlog.v(TAG, "receive HANDLER_EVENT_FINISH");
                setViewEnabled(true);
            }
        }
    };
    private Handler mEventHandler = null;

    String[] mMode = { "continuous packet tx", "100% duty cycle",
            "carrier suppression", "local leakage", "enter power off" };
    String[] mPreamble = { "Normal", "CCK short", "802.11n mixed mode",
            "802.11n green field", };
    String[] mBandwidth = { "20MHz", "40MHz", "U20MHz", "L20MHz", };
    String[] mGuardInterval = { "800ns", "400ns", };

    static class RateInfo {
        private static final short EEPROM_RATE_GROUP_CCK = 0;
        private static final short EEPROM_RATE_GROUP_OFDM_6_9M = 1;
        private static final short EEPROM_RATE_GROUP_OFDM_12_18M = 2;
        private static final short EEPROM_RATE_GROUP_OFDM_24_36M = 3;
        private static final short EEPROM_RATE_GROUP_OFDM_48_54M = 4;
        private static final short EEPROM_RATE_GROUP_OFDM_MCS0_32 = 5;

        int mRateIndex = 0;

        int mOFDMStartIndex = 4; 

        private final short[] mUcRateGroupEep = { EEPROM_RATE_GROUP_CCK,
                EEPROM_RATE_GROUP_CCK, EEPROM_RATE_GROUP_CCK,
                EEPROM_RATE_GROUP_CCK, EEPROM_RATE_GROUP_OFDM_6_9M,
                EEPROM_RATE_GROUP_OFDM_6_9M, EEPROM_RATE_GROUP_OFDM_12_18M,
                EEPROM_RATE_GROUP_OFDM_12_18M, EEPROM_RATE_GROUP_OFDM_24_36M,
                EEPROM_RATE_GROUP_OFDM_24_36M, EEPROM_RATE_GROUP_OFDM_48_54M,
                EEPROM_RATE_GROUP_OFDM_48_54M,
                /* for future use */
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, };
        private final String[] mPszRate = { "1M", "2M", "5.5M", "11M", "6M",
                "9M", "12M", "18M", "24M", "36M", "48M", "54M",
                /* for future use */
                "MCS0", "MCS1", "MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7",
                "MCS32",

        };

        private final int[] mRateCfg = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72,
                96, 108,
                /* here we need to add cfg data for MCS*** */
                22, 12, 18, 24, 36, 48, 72, 96, 108 };

        /**
         * Get total rate number
         * 
         * @return The total rate number
         */
        int getRateNumber() {
            return mPszRate.length;
        }

        /**
         * Get rate string by {@link #mRateIndex}
         * 
         * @return Rate string
         */
        String getRate() {
            return mPszRate[mRateIndex];
        }

        /**
         * Get rate configured data
         * 
         * @return Rate data
         */
        int getRateCfg() {
            return mRateCfg[mRateIndex];
        }

        /**
         * Get group the rate belong to
         * 
         * @return Group ID
         */
        int getUcRateGroupEep() {
            return mUcRateGroupEep[mRateIndex];
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi_tx_6620);

        mChannelSpinner = (Spinner) findViewById(R.id.WiFi_Channel_Spinner);
        mPreambleSpinner = (Spinner) findViewById(R.id.WiFi_Preamble_Spinner);
        mEtPkt = (EditText) findViewById(R.id.WiFi_Pkt_Edit);
        mEtPktCnt = (EditText) findViewById(R.id.WiFi_Pktcnt_Edit);
        mEtTxGain = (EditText) findViewById(R.id.WiFi_Tx_Gain_Edit);// Tx gain
        mRateSpinner = (Spinner) findViewById(R.id.WiFi_Rate_Spinner);
        mModeSpinner = (Spinner) findViewById(R.id.WiFi_Mode_Spinner);
        // mXTEdit = (EditText) findViewById(R.id.WiFi_XtalTrim_Edit);
        // mWriteBtn = (Button) findViewById(R.id.WiFi_XtalTrim_Write);
        // mReadBtn = (Button) findViewById(R.id.WiFi_XtalTrim_Read);
        // mALCCheck = (CheckBox) findViewById(R.id.WiFi_ALC);
        mBtnGo = (Button) findViewById(R.id.WiFi_Go);
        mBtnStop = (Button) findViewById(R.id.WiFi_Stop);
        mBandwidthSpinner = (Spinner) findViewById(R.id.WiFi_Bandwidth_Spinner);
        mGuardIntervalSpinner = (Spinner) findViewById(R.id.WiFi_Guard_Interval_Spinner);

        mTestThread = new HandlerThread("Wifi Tx Test");
        mTestThread.start();
        mEventHandler = new EventHandler(mTestThread.getLooper());
        mBtnGo.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);
        mChannel = new ChannelInfo();
        mRate = new RateInfo();
        // mEtPktCnt.setOnKeyListener(new View.OnKeyListener() {
        // public boolean onKey(View v, int keyCode, KeyEvent event) {
        // CharSequence inputVal = mEtPktCnt.getText();
        // if (TextUtils.equals(inputVal, "0")) {
        // Toast.makeText(WiFiTx6620.this,
        // R.string.wifi_toast_packet_error,
        // Toast.LENGTH_SHORT).show();
        // mEtPktCnt.setText(String.valueOf(DEFAULT_PKT_CNT));
        // }
        // return false;
        // }
        // });
        mChannelAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mChannelAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 1; i <= ChannelInfo.sChannels[0]; i++) {
            if (ChannelInfo.sChannels[i] > ChannelInfo.CHANNEL_NUMBER_14) {
                break;
            }
            for (String s : mChannel.mFullChannelName) {
                if (s.startsWith("Channel " + ChannelInfo.sChannels[i] + " ")) {
                    mChannelAdapter.add(s);
                    break;
                }
            }
        }
        mChannelSpinner.setAdapter(mChannelAdapter);
        mChannelSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                if (EMWifi.sIsInitialed) {
                    mChannel.mChannelSelect = mChannelAdapter.getItem(arg2);
                    EMWifi.setChannel(mChannel.getChannelFreq());
                    uiUpdateTxPower();
                } else {
                    showDialog(DIALOG_WIFI_ERROR);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Xlog.v(TAG, "onNothingSelected");
            }
        });
        ArrayAdapter<String> rateAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        rateAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mRate.getRateNumber(); i++) {
            rateAdapter.add(mRate.mPszRate[i]);
        }
        mRateSpinner.setAdapter(rateAdapter);
        mRateSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                if (!EMWifi.sIsInitialed) {
                    showDialog(DIALOG_WIFI_ERROR);
                    return;
                }
                mRate.mRateIndex = arg2;

                // set Tx Rate
                Xlog.i(TAG, "The mRateIndex is : " + arg2);
                // judge if high rate item selected MCS0~MCS7 MCS32
                mHighRateSelected = arg2 >= MAX_LOWER_RATE_NUMBER ? true
                        : false;
                int delta = mHighRateSelected ? 2 : 0;
                mPreambleAdapter.clear();
                mPreambleAdapter.add(mPreamble[0 + delta]);
                mPreambleAdapter.add(mPreamble[1 + delta]);
                mPreambleIndex = delta;
                uiUpdateTxPower();

                if (arg2 >= CCK_RATE_NUMBER) {
                    if (mCCKRateSelected) {
                        mCCKRateSelected = false;
                        mModeAdapter.remove(mMode[2]);
                        mModeSpinner.setSelection(0);
                    }
                } else {
                    if (!mCCKRateSelected) {
                        mCCKRateSelected = true;
                        mModeAdapter.insert(mMode[2], 2);
                        mModeSpinner.setSelection(0);
                    }
                }
                updateChannels();
                mLastRateGroup = mRate.getUcRateGroupEep();
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Xlog.v(TAG, "onNothingSelected");
            }
        });
        mRateSpinner.setSelection(mRate.mOFDMStartIndex); //show 5G channel default.
        mModeAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mModeAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mMode.length; i++) {
            mModeAdapter.add(mMode[i]);
        }
        mModeSpinner.setAdapter(mModeAdapter);
        mModeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                mModeIndex = arg2;
                Xlog.i(TAG, "The mModeIndex is : " + arg2);
                if (!mCCKRateSelected) {
                    if (arg2 >= 2) {
                        mModeIndex++;
                    }
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Xlog.v(TAG, "onNothingSelected");
            }
        });
        // 802.11n select seetings
        mPreambleAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mPreambleAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mPreamble.length; i++) {
            mPreambleAdapter.add(mPreamble[i]);
        }
        mPreambleSpinner.setAdapter(mPreambleAdapter);
        mPreambleSpinner
                .setOnItemSelectedListener(new OnItemSelectedListener() {

                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        mPreambleIndex = arg2 + (mHighRateSelected ? 2 : 0);

                    }

                    public void onNothingSelected(AdapterView<?> arg0) {
                        Xlog.v(TAG, "onNothingSelected");
                    }
                });
        // Bandwidth seetings
        ArrayAdapter<String> bandwidthAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        bandwidthAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mBandwidth.length; i++) {
            bandwidthAdapter.add(mBandwidth[i]);
        }
        mBandwidthSpinner.setAdapter(bandwidthAdapter);
        mBandwidthSpinner
                .setOnItemSelectedListener(new OnItemSelectedListener() {

                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        // max Bandwidth setting value is 4
                        mBandwidthIndex = arg2 < mBandwidth.length ? arg2
                                : mBandwidthIndex;
                        updateChannels();
                        mLastBandwidth = mBandwidthIndex;
                    }

                    public void onNothingSelected(AdapterView<?> arg0) {
                        Xlog.v(TAG, "onNothingSelected");
                    }
                });
        // Guard Interval seetings
        ArrayAdapter<String> guardIntervalAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        guardIntervalAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mGuardInterval.length; i++) {
            guardIntervalAdapter.add(mGuardInterval[i]);
        }
        mGuardIntervalSpinner.setAdapter(guardIntervalAdapter);
        mGuardIntervalSpinner
                .setOnItemSelectedListener(new OnItemSelectedListener() {
                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        mGuardIntervalIndex = arg2 < 2 ? arg2
                                : mGuardIntervalIndex;
                    }

                    public void onNothingSelected(AdapterView<?> arg0) {
                        Xlog.v(TAG, "onNothingSelected");
                    }
                });
        setViewEnabled(true);
    }


    @Override
    public void onClick(View view) {
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }
        Xlog.d(TAG, "view_id = " + view.getId());
        if (view.getId() == mBtnGo.getId()) {
            onClickBtnTxGo();
        } else if (view.getId() == mBtnStop.getId()) {
            onClickBtnTxStop();
        }
    }

    @Override
    protected void onDestroy() {
        if (mEventHandler != null) {
            mEventHandler.removeMessages(HANDLER_EVENT_TIMER);
            if (mTestInPorcess) {
                if (EMWifi.sIsInitialed) {
                    EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                            COMMAND_INDEX_STOPTEST);
                }
                mTestInPorcess = false;
            }
        }
        mTestThread.quit();
        super.onDestroy();
    }

    /**
     * Update Tx power
     */
    private void uiUpdateTxPower() {
        short ucGain = 0;
        long i4TxPwrGain = 0;
        // long i4OutputPower = 0;
        // long i4targetAlc = 0;
        long[] gain = new long[LENGTH_3];
        int comboChannelIndex = mChannel.getChannelIndex();
        // 40MHz 0x8000 | mChannel.mChannelIndex else mChannel.mChannelIndex
        comboChannelIndex |= ((mBandwidthIndex == BANDWIDTH_INDEX_40) ? BANDWIDTH_40MHZ_MASK
                : 0);
        // may change to array[3];
        Xlog.w(TAG, "channelIdx " + comboChannelIndex + " rateIdx "
                + mRate.mRateIndex + " gain " + Arrays.toString(gain) + " Len "
                + LENGTH_3);
        if (0 == EMWifi.readTxPowerFromEEPromEx(comboChannelIndex,
                mRate.mRateIndex, gain, LENGTH_3)) {
            i4TxPwrGain = gain[0];
            // i4OutputPower = gain[1];
            // i4targetAlc = gain[2];
            Xlog.i(TAG, "i4TxPwrGain from uiUpdateTxPower is " + i4TxPwrGain);
            ucGain = (short) (i4TxPwrGain & BIT_8_MASK);
        }
        /*
         * if (ucGain == 0x00 || ucGain == 0xFF) { if (mRate.getUcRateGroupEep()
         * <= mRate.EEPROM_RATE_GROUP_CCK) { mTxGainEdit.setText("20"); } else {
         * mTxGainEdit.setText("22"); } } else { // long val = ucGain;
         * mTxGainEdit.setText(Long.toHexString(ucGain)); }
         */
        mEtTxGain.setText(String.format(Locale.ENGLISH,
                getString(R.string.wifi_tx_gain_format), ucGain / 2.0));
        // mTxGainEdit.setText(Long.toHexString(ucGain));
    }

    /**
     * Update channels
     */
    private void updateChannels() {
        boolean bUpdateWifiChannel = false;
        if (ChannelInfo.sHas14Ch) {
            if (mLastRateGroup != mRate.getUcRateGroupEep()) {
                if (RateInfo.EEPROM_RATE_GROUP_CCK == mRate.getUcRateGroupEep()) {
                    if (ChannelInfo.sHasUpper14Ch) {
                        int index = mChannelAdapter
                                .getPosition(mChannel.mFullChannelName[CHANNEL_14]);
                        if (-1 == index) {
                            mChannelAdapter
                                    .add(mChannel.mFullChannelName[CHANNEL_13]);
                            bUpdateWifiChannel = true;
                        } else {
                            mChannelAdapter.insert(
                                    mChannel.mFullChannelName[CHANNEL_13],
                                    index);
                            bUpdateWifiChannel = true;
                        }
                    } else {
                        mChannelAdapter
                                .add(mChannel.mFullChannelName[CHANNEL_13]);
                        bUpdateWifiChannel = true;
                    }
                } else if (RateInfo.EEPROM_RATE_GROUP_CCK == mLastRateGroup) {
                    mChannelAdapter
                            .remove(mChannel.mFullChannelName[CHANNEL_13]);
                    bUpdateWifiChannel = true;
                }
            }
        }
        if (ChannelInfo.sHasUpper14Ch) {
            if (mLastRateGroup != mRate.getUcRateGroupEep()) {
                if (RateInfo.EEPROM_RATE_GROUP_CCK == mLastRateGroup) {
                    for (int i = 1; i <= ChannelInfo.sChannels[0]; i++) {
                        if (ChannelInfo.sChannels[i] > ChannelInfo.CHANNEL_NUMBER_14) {
                            for (String s : mChannel.mFullChannelName) {
                                if (s.startsWith("Channel "
                                        + ChannelInfo.sChannels[i] + " ")) {
                                    mChannelAdapter.add(s);
                                    bUpdateWifiChannel = true;
                                    break;
                                }
                            }
                        }
                    }
                } else if (RateInfo.EEPROM_RATE_GROUP_CCK == mRate
                        .getUcRateGroupEep()) {
                    for (int i = ChannelInfo.CHANNEL_NUMBER_14; 
                    i < mChannel.mFullChannelName.length; i++) {
                        mChannelAdapter.remove(mChannel.mFullChannelName[i]);
                        bUpdateWifiChannel = true;
                    }
                }
            }
        }
        if (mLastBandwidth != mBandwidthIndex) {
            if (mBandwidthIndex == BANDWIDTH_INDEX_40) {
                mChannelAdapter.remove(mChannel.mFullChannelName[CHANNEL_0]);
                mChannelAdapter.remove(mChannel.mFullChannelName[CHANNEL_1]);
                mChannelAdapter.remove(mChannel.mFullChannelName[CHANNEL_11]);
                mChannelAdapter.remove(mChannel.mFullChannelName[CHANNEL_12]);
                bUpdateWifiChannel = true;
            }
            if (mLastBandwidth == BANDWIDTH_INDEX_40) {
                mChannelAdapter.insert(mChannel.mFullChannelName[CHANNEL_0],
                        CHANNEL_0);
                mChannelAdapter.insert(mChannel.mFullChannelName[CHANNEL_1],
                        CHANNEL_1);
                if (mChannel.isContains(CHANNEL_12)) {
                    mChannelAdapter.insert(
                            mChannel.mFullChannelName[CHANNEL_11], CHANNEL_11);
                }
                if (mChannel.isContains(CHANNEL_13)) {
                    mChannelAdapter.insert(
                            mChannel.mFullChannelName[CHANNEL_12], CHANNEL_12);
                }
                bUpdateWifiChannel = true;
            }
        }
        if (bUpdateWifiChannel) {
            updateWifiChannel(mChannel, mChannelAdapter, mChannelSpinner);
            uiUpdateTxPower();
        }
    }


    /*
     * private void onClickBtnXtalTrimRead() { long[] val = new long[1];
     * EMWifi.getXtalTrimToCr(val); Log.d(TAG, "VAL=" + val[0]);
     * mXTEdit.setText(String.valueOf(val[0])); }
     * 
     * private void onClickBtnXtaltrimWrite() { long u4XtalTrim = 0; try {
     * u4XtalTrim = Long.parseLong(mXTEdit.getText().toString()); } catch
     * (NumberFormatException e) { Toast.makeText(WiFi_Tx_6620.this,
     * "invalid input value", Toast.LENGTH_SHORT).show(); return; }
     * 
     * Log.d(TAG, "u4XtalTrim =" + u4XtalTrim);
     * EMWifi.setXtalTrimToCr(u4XtalTrim); }
     */
    private void onClickBtnTxGo() {
        long u4TxGainVal = 0;
        int i = 0;
        long pktNum;
        long cntNum;
        CharSequence inputVal;
        try {
            float pwrVal = Float.parseFloat(mEtTxGain.getText().toString());
            u4TxGainVal = (long) (pwrVal * 2);
            mEtTxGain
                    .setText(String.format(Locale.ENGLISH,
                            getString(R.string.wifi_tx_gain_format),
                            u4TxGainVal / 2.0));
            // u4TxGainVal = Long.parseLong(mTxGainEdit.getText().toString(),
            // 16);
        } catch (NumberFormatException e) {
            Toast.makeText(WiFiTx6620.this, "invalid input value",
                    Toast.LENGTH_SHORT).show();
            return;
        }
        mTxGainVal = u4TxGainVal;
        mTxGainVal = mTxGainVal > BIT_8_MASK ? BIT_8_MASK : mTxGainVal;
        mTxGainVal = mTxGainVal < 0 ? 0 : mTxGainVal;
        Xlog.i(TAG, "Wifi Tx Test : " + mMode[mModeIndex]);
        switch (mModeIndex) {
        case TEST_MODE_TX:
            try {
                pktNum = Long.parseLong(mEtPkt.getText().toString());
                cntNum = Long.parseLong(mEtPktCnt.getText().toString());
            } catch (NumberFormatException e) {
                Toast.makeText(WiFiTx6620.this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            mPktLenNum = pktNum;
            mCntNum = cntNum;
            break;
        case TEST_MODE_DUTY:
            // EMWifi.setOutputPower(mRate.getRateCfg(), u4TxGainVal,
            // u4Antenna);//for mt5921
            // set output power
            // setp 1:set rate
            // setp 2:set Tx gain
            // setp 3:set Antenna
            // setp 4:start output power test
            break;

        case TEST_MODE_CARRIER:
            /*
             * int i4ModeType; if (mRate.getRateCfg() <=
             * mRate.EEPROM_RATE_GROUP_CCK) { i4ModeType = 0; } else {
             * i4ModeType = 1; }
             * 
             * //EMWifi.setCarrierSuppression(i4ModeType, u4TxGainVal,
             * u4Antenna);//for mt5921
             */
            // setp 1:set EEPROMRate Info
            // setp 2:set Tx gain
            // setp 3:set Antenna
            // step 4:start RF Carriar Suppression Test
            break;
        case TEST_MODE_LEAKAGE:
            // EMWifi.setLocalFrequecy(u4TxGainVal, u4Antenna);//for mt5921
            // setp 1:set Tx gain
            // setp 2:set Antenna
            // step 3:start Local Frequency Test

            break;
        case TEST_MODE_POWEROFF:
            // EMWifi.setNormalMode();
            // EMWifi.setOutputPin(20, 0);
            // EMWifi.setPnpPower(4);
            break;
        default:
            break;
        }
        if (mEventHandler == null) {
            Xlog.w(TAG, "eventHandler = null");
        } else {
            mEventHandler.sendEmptyMessage(HANDLER_EVENT_GO);
            // mGoBtn.setEnabled(false);
            setViewEnabled(false);
        }
    }

    private void setViewEnabled(boolean state) {
        mChannelSpinner.setEnabled(state);
        mGuardIntervalSpinner.setEnabled(state);
        mBandwidthSpinner.setEnabled(state);
        mPreambleSpinner.setEnabled(state);
        mEtPkt.setEnabled(state);
        mEtPktCnt.setEnabled(state);
        mEtTxGain.setEnabled(state);
        mRateSpinner.setEnabled(state);
        mModeSpinner.setEnabled(state);
        // mXTEdit.setEnabled(state);
        // mWriteBtn.setEnabled(state);
        // mReadBtn.setEnabled(state);
        // mALCCheck.setEnabled(state);
        mBtnGo.setEnabled(state);
        mBtnStop.setEnabled(!state);
    }

    private void onClickBtnTxStop() {
        if (mEventHandler == null) {
            Xlog.w(TAG, "eventHandler = null");
        } else {
            mEventHandler.sendEmptyMessage(HANDLER_EVENT_STOP);
        }
        switch (mModeIndex) {
        case TEST_MODE_TX:
            break;
        case TEST_MODE_POWEROFF:
            EMWifi.setPnpPower(1);
            EMWifi.setTestMode();
            EMWifi.setChannel(mChannel.getChannelFreq());
            uiUpdateTxPower();
            // mGoBtn.setEnabled(true);
            break;
        default:
            EMWifi.setStandBy();
            // mGoBtn.setEnabled(true);
            break;
        }
    }

    class EventHandler extends Handler {

        /**
         * Constructor
         * 
         * @param looper
         *            Use the provided queue instead of the default one
         */
        public EventHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            if (!EMWifi.sIsInitialed) {
                showDialog(DIALOG_WIFI_ERROR);
                return;
            }
            Xlog.d(TAG, "new msg");
            // long i = 0;
            int rateIndex;
            long[] u4Value = new long[1];
            switch (msg.what) {
            case HANDLER_EVENT_GO:
                switch (mModeIndex) {
                case TEST_MODE_TX:
                    // EMWifi.setChannel(mChannel.getChannelFreq());
                    // set Tx gain of RF
                    EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);
                    Xlog.i(TAG, "The mPreambleIndex is : " + mPreambleIndex);
                    EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, mPreambleIndex);
                    // u4Antenna = 0, never be changed since first
                    // valued
                    EMWifi.setATParam(ATPARAM_INDEX_ANTENNA, ANTENNA);
                    // set package length, is there a maximum packet
                    // length? mtk80758-2010-11-2
                    EMWifi.setATParam(ATPARAM_INDEX_PACKLENGTH, mPktLenNum);
                    // set package length, is there a maximum packet
                    // length? mtk80758-2010-11-2
                    // if cntNum = 0, send continious unless stop button
                    // is pressed
                    EMWifi.setATParam(ATPARAM_INDEX_PACKCOUNT, mCntNum);
                    // package interval in unit of us, no need to allow
                    // user to set this value
                    EMWifi.setATParam(ATPARAM_INDEX_PACKINTERVAL, 0);
                    // if (mALCCheck.isChecked() == false) {
                    // i = 0;
                    // } else {
                    // i = 1;
                    // }
                    // 9, means temperature conpensation
                    EMWifi.setATParam(ATPARAM_INDEX_TEMP_COMPENSATION, 0);
                    // TX enable enable ? what does this mean
                    EMWifi.setATParam(ATPARAM_INDEX_TXOP_LIMIT,
                            TXOP_LIMIT_VALUE);
                    // set Tx content
                    for (int i = 0; i < PACKCONTENT_BUFFER.length; i++) {
                        EMWifi.setATParam(ATPARAM_INDEX_PACKCONTENT,
                                PACKCONTENT_BUFFER[i]);
                    }
                    // packet retry limit
                    EMWifi.setATParam(ATPARAM_INDEX_RETRY_LIMIT, 1);
                    // QoS queue -AC2
                    EMWifi.setATParam(ATPARAM_INDEX_QOS_QUEUE, 2);
                    Xlog.i(TAG, "The mGuardIntervalIndex is : "
                            + mGuardIntervalIndex);
                    // GuardInterval setting
                    EMWifi.setATParam(ATPARAM_INDEX_GI, mGuardIntervalIndex);
                    Xlog.i(TAG, "The mBandwidthIndex is : " + mBandwidthIndex);
                    // Bandwidth setting
                    EMWifi.setATParam(ATPARAM_INDEX_BANDWIDTH, mBandwidthIndex);
                    rateIndex = mRate.mRateIndex;
                    if (mHighRateSelected) {
                        rateIndex -= MAX_LOWER_RATE_NUMBER;
                        if (rateIndex > RATE_NOT_MCS_INDEX) {
                            rateIndex = RATE_MCS_INDEX; // for MCS32
                        }
                        rateIndex |= (1 << RATE_MODE_MASK);
                    }
                    // rateIndex |= (1 << 31);
                    Xlog.i(TAG, String.format("TXX rate index = 0x%08x",
                            rateIndex));
                    EMWifi.setATParam(ATPARAM_INDEX_RATE, rateIndex);
                    int number = mChannel.getChannelFreq();
                    EMWifi.setChannel(number);
                    Xlog.i(TAG, "target channel freq ="
                            + mChannel.getChannelFreq());
                    // start tx test
                    if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                            COMMAND_INDEX_STARTTX)) {
                        mTestInPorcess = true;
                    }
                    sendEmptyMessageDelayed(HANDLER_EVENT_TIMER, ONE_SENCOND);
                    break;
                case TEST_MODE_DUTY:
                    // EMWifi.setOutputPower(mRate.getRateCfg(),
                    // u4TxGainVal, u4Antenna);//for mt5921
                    // set output power
                    // setp 1:set rate
                    // setp 2:set Tx gain
                    // setp 3:set Antenna
                    // setp 4:start output power test
                    rateIndex = mRate.mRateIndex;
                    if (mHighRateSelected) {
                        rateIndex -= MAX_LOWER_RATE_NUMBER;
                        if (rateIndex > RATE_NOT_MCS_INDEX) {
                            rateIndex = RATE_MCS_INDEX; // for MCS32
                        }
                        rateIndex |= (1 << RATE_MODE_MASK);
                    }
                    Xlog.i(TAG, String.format("Tx rate index = 0x%08x",
                            rateIndex));
                    EMWifi.setATParam(ATPARAM_INDEX_RATE, rateIndex);
                    EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);
                    Xlog.i(TAG, "The mPreambleIndex is : " + mPreambleIndex);
                    EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, mPreambleIndex);
                    EMWifi.setATParam(ATPARAM_INDEX_ANTENNA, ANTENNA);
                    // start output power test
                    if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                            COMMAND_INDEX_OUTPUTPOWER)) {
                        mTestInPorcess = true;
                    }
                    break;
                case TEST_MODE_CARRIER:
                    // setp 1:set EEPROMRate Info
                    // setp 2:set Tx gain
                    // setp 3:set Antenna
                    // step 4:start RF Carriar Suppression Test
                    EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);
                    EMWifi.setATParam(ATPARAM_INDEX_ANTENNA, ANTENNA);
                    // start carriar suppression test
                    if (ChipSupport.getChip() == ChipSupport.MTK_6573_SUPPORT) {
                        if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                                COMMAND_INDEX_CARRIER)) {
                            mTestInPorcess = true;
                        }
                    } else {
                        if (mCCKRateSelected) {
                            EMWifi.setATParam(ATPARAM_INDEX_CWMODE,
                                    CWMODE_CCKPI);
                        } else {
                            EMWifi.setATParam(ATPARAM_INDEX_CWMODE,
                                    CWMODE_OFDMLTF);
                        }
                        if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                                COMMAND_INDEX_CARRIER_NEW)) {
                            mTestInPorcess = true;
                        }
                    }
                    break;
                case TEST_MODE_LEAKAGE:
                    // Wifi.setLocalFrequecy(u4TxGainVal, u4Antenna);
                    // setp 1:set Tx gain
                    // setp 2:set Antenna
                    // step 3:start Local Frequency Test
                    EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);
                    EMWifi.setATParam(ATPARAM_INDEX_ANTENNA, ANTENNA);
                    // start carriar suppression test
                    if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                            COMMAND_INDEX_LOCALFREQ)) {
                        mTestInPorcess = true;
                    }
                    break;
                case TEST_MODE_POWEROFF:
                    // Wifi.setNormalMode();
                    // Wifi.setOutputPin(20, 0);
                    // Wifi.setPnpPower(4);
                    break;
                default:
                    break;
                }
                break;
            case HANDLER_EVENT_STOP:
                Xlog.i(TAG, "The Handle event is : HANDLER_EVENT_STOP");
                if (mTestInPorcess) {
                    u4Value[0] = EMWifi.setATParam(ATPARAM_INDEX_COMMAND,
                            COMMAND_INDEX_STOPTEST);
                    mTestInPorcess = false;
                }
                // driver does not support query operation on
                // functionIndex = 1 , we can only judge whether this
                // operation is processed successfully through the
                // return value
                if (mEventHandler != null) {
                    mEventHandler.removeMessages(HANDLER_EVENT_TIMER);
                }
                mHandler.sendEmptyMessage(HANDLER_EVENT_FINISH);
                break;
            case HANDLER_EVENT_TIMER:
                u4Value[0] = 0;
                long pktCnt = 0;
                Xlog.i(TAG, "The Handle event is : HANDLER_EVENT_TIMER");
                try {
                    pktCnt = Long.parseLong(mEtPktCnt.getText().toString());
                } catch (NumberFormatException e) {
                    Toast.makeText(WiFiTx6620.this, "invalid input value",
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                // here we need to judge whether target number packet is
                // finished sent or not
                if (0 == EMWifi
                        .getATParam(ATPARAM_INDEX_TRANSMITCOUNT, u4Value)) {
                    Xlog.d(TAG,
                            "query Transmitted packet count succeed, count = "
                                    + u4Value[0] + " target count = " + pktCnt);
                    if (u4Value[0] == pktCnt) {
                        removeMessages(HANDLER_EVENT_TIMER);
                        mHandler.sendEmptyMessage(HANDLER_EVENT_FINISH);
                        break;
                    }
                } else {
                    Xlog.w(TAG, "query Transmitted packet count failed");
                }
                sendEmptyMessageDelayed(HANDLER_EVENT_TIMER, ONE_SENCOND);
                break;
            default:
                break;
            }
        }
    }
}
