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

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class WiFiRx6620 extends WiFiTestActivity implements OnClickListener {

    private static final String TAG = "EM/WiFi_Rx";
    private static final int HANDLER_EVENT_RX = 2;
    protected static final long HANDLER_RX_DELAY_TIME = 1000;
    private static final long PERCENT = 100;
    private static final String TEXT_ZERO = "0";
    private static final int BANDWIDTH_INDEX_40 = 1;
    private static final int WAIT_COUNT = 10;
    private final String[] mBandwidth = { "20MHz", "40MHz", "U20MHz", "L20MHz", };
    private TextView mTvFcs = null;
    private TextView mTvRx = null;
    private TextView mTvPer = null;
    private Button mBtnGo = null;
    private Button mBtnStop = null;
    private Spinner mChannelSpinner = null;
    private Spinner mBandwidthSpinner = null;
    private int mBandwidthIndex = 0;
    // private ArrayAdapter<String> mBandwidthAdapter = null;
    private ArrayAdapter<String> mChannelAdapter = null;
    private WiFiStateManager mWiFiStateManager = null;
    private ChannelInfo mChannel = null;
    private long[] mInitData = null;
    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (!EMWifi.sIsInitialed) {
                showDialog(DIALOG_WIFI_ERROR);
                return;
            }
            if (HANDLER_EVENT_RX == msg.what) {
                long[] i4Rx = new long[2];
                long i4RxCntOk = -1;
                long i4RxCntFcsErr = -1;
                long i4RxPer = -1;
                Xlog.i(TAG, "The Handle event is : HANDLER_EVENT_RX");
                try {
                    i4RxPer = Long.parseLong(mTvPer.getText().toString());
                } catch (NumberFormatException e) {
                    Xlog.d(TAG, "Long.parseLong NumberFormatException: "
                            + e.getMessage());
                }
                EMWifi.getPacketRxStatus(i4Rx, 2);
                Xlog.d(TAG, "after rx test: rx ok = "
                                + String.valueOf(i4Rx[0]));
                Xlog.d(TAG, "after rx test: fcs error = "
                        + String.valueOf(i4Rx[1]));
                i4RxCntOk = i4Rx[0]/* - i4Init[0] */;
                i4RxCntFcsErr = i4Rx[1]/* - i4Init[1] */;
                if (i4RxCntFcsErr + i4RxCntOk != 0) {
                    i4RxPer = i4RxCntFcsErr * PERCENT
                            / (i4RxCntFcsErr + i4RxCntOk);
                }
                mTvFcs.setText(String.valueOf(i4RxCntFcsErr));
                mTvRx.setText(String.valueOf(i4RxCntOk));
                mTvPer.setText(String.valueOf(i4RxPer));
            }
            mHandler.sendEmptyMessageDelayed(HANDLER_EVENT_RX,
                    HANDLER_RX_DELAY_TIME);
        }
    };
    private final OnItemSelectedListener mSpinnerListener = new OnItemSelectedListener() {

        public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
                long arg3) {
            Xlog.e(TAG, "arg0.getId(): " + arg1.getId() + " " +  R.id.WiFi_RX_Channel_Spinner + " " + R.id.WiFi_Bandwidth_Spinner);
            if (arg0.getId() == R.id.WiFi_RX_Channel_Spinner) {
                if (EMWifi.sIsInitialed) {
                    mChannel.mChannelSelect = mChannelAdapter.getItem(arg2);
                    EMWifi.setChannel(mChannel.getChannelFreq());
                } else {
                    showDialog(DIALOG_WIFI_ERROR);
                }
            } else if (arg0.getId() == R.id.WiFi_Bandwidth_Spinner) {
                mBandwidthIndex = arg2 < mBandwidth.length ? arg2
                        : mBandwidthIndex;
                if (BANDWIDTH_INDEX_40 == mBandwidthIndex) {
                    mChannelAdapter
                            .remove(mChannel.mFullChannelName[CHANNEL_0]);
                    mChannelAdapter
                            .remove(mChannel.mFullChannelName[CHANNEL_1]);
                    mChannelAdapter
                            .remove(mChannel.mFullChannelName[CHANNEL_11]);
                    mChannelAdapter
                            .remove(mChannel.mFullChannelName[CHANNEL_12]);
                    updateWifiChannel(mChannel, mChannelAdapter,
                            mChannelSpinner);
                } else {
                    boolean bUpdate = false;
                    if (-1 == mChannelAdapter
                            .getPosition(mChannel.mFullChannelName[CHANNEL_0])) {
                        mChannelAdapter
                                .insert(mChannel.mFullChannelName[CHANNEL_0],
                                        CHANNEL_0);
                        bUpdate = true;
                    }
                    if (-1 == mChannelAdapter
                            .getPosition(mChannel.mFullChannelName[CHANNEL_1])) {
                        mChannelAdapter
                                .insert(mChannel.mFullChannelName[CHANNEL_1],
                                        CHANNEL_1);
                        bUpdate = true;
                    }
                    if (mChannel.isContains(CHANNEL_12)
                            && -1 == mChannelAdapter
                                    .getPosition(mChannel.mFullChannelName[CHANNEL_11])) {
                        mChannelAdapter.insert(
                                mChannel.mFullChannelName[CHANNEL_11],
                                CHANNEL_11);
                        bUpdate = true;
                    }
                    if (mChannel.isContains(CHANNEL_13)
                            && -1 == mChannelAdapter
                                    .getPosition(mChannel.mFullChannelName[CHANNEL_12])) {
                        mChannelAdapter.insert(
                                mChannel.mFullChannelName[CHANNEL_12],
                                CHANNEL_12);
                        bUpdate = true;
                    }
                    if (bUpdate) {
                        updateWifiChannel(mChannel, mChannelAdapter,
                                mChannelSpinner);
                    }
                }
            }
        }

        public void onNothingSelected(AdapterView<?> arg0) {
            Xlog.d(TAG, "onNothingSelected");
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi_rx_6620);
        mTvFcs = (TextView) findViewById(R.id.WiFi_FCS_Content);
        mTvRx = (TextView) findViewById(R.id.WiFi_Rx_Content);
        mTvPer = (TextView) findViewById(R.id.WiFi_PER_Content);
        mTvFcs.setText(R.string.wifi_empty);
        mTvRx.setText(R.string.wifi_empty);
        mTvPer.setText(R.string.wifi_empty);
        mBtnGo = (Button) findViewById(R.id.WiFi_Go_Rx);
        mBtnStop = (Button) findViewById(R.id.WiFi_Stop_Rx);
        mBtnGo.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);
        mInitData = new long[2];
        mChannel = new ChannelInfo();
        mChannelSpinner = (Spinner) findViewById(R.id.WiFi_RX_Channel_Spinner);
        mChannelAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mChannelAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mChannelAdapter.clear();
        for (int i = 1; i <= ChannelInfo.sChannels[0]; i++) {
            for (String s : mChannel.mFullChannelName) {
                if (s.startsWith("Channel " + ChannelInfo.sChannels[i] + " ")) {
                    mChannelAdapter.add(s);
                    break;
                }
            }
        }
        mChannelSpinner.setAdapter(mChannelAdapter);
        mChannelSpinner.setOnItemSelectedListener(mSpinnerListener);

        mBandwidthSpinner = (Spinner) findViewById(R.id.WiFi_Bandwidth_Spinner);
        // Bandwidth setings
        ArrayAdapter<String> bwAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        bwAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mBandwidth.length; i++) {
            bwAdapter.add(mBandwidth[i]);
        }
        mBandwidthSpinner.setAdapter(bwAdapter);
        mBandwidthSpinner.setOnItemSelectedListener(mSpinnerListener);
        setViewEnabled(true);
    }

    @Override
    public void onClick(View arg0) {
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }
        if (arg0.getId() == mBtnGo.getId()) {
            onClickBtnRxGo();
        } else if (arg0.getId() == mBtnStop.getId()) {
            onClickBtnRxStop();
        }
    }

    @Override
    protected void onDestroy() {
        mHandler.removeMessages(HANDLER_EVENT_RX);
        if (EMWifi.sIsInitialed) {
            EMWifi.setATParam(1, 0);
        }
        super.onDestroy();
    }

    /**
     * Invoked when "Go" button clicked
     */
    private void onClickBtnRxGo() {
        int i = -1;
        int len = 2;
        EMWifi.getPacketRxStatus(mInitData, 2);
        Xlog.d(TAG, "before rx test: rx ok = " + String.valueOf(mInitData[0]));
        Xlog.d(TAG, "before rx test: fcs error = "
                + String.valueOf(mInitData[1]));
        // if (mALCCheck.isChecked() == false) {
        i = 0;
        // } else {
        // i = 1;
        // }
        // temperature conpensation
        EMWifi.setATParam(ATPARAM_INDEX_TEMP_COMPENSATION, i);

        // Bandwidth setting
        EMWifi.setATParam(ATPARAM_INDEX_BANDWIDTH, mBandwidthIndex);
        // start Rx
        EMWifi.setATParam(ATPARAM_INDEX_COMMAND, 2);
        mHandler.sendEmptyMessage(HANDLER_EVENT_RX);
        mTvFcs.setText(TEXT_ZERO);
        mTvRx.setText(TEXT_ZERO);
        mTvPer.setText(TEXT_ZERO);
        setViewEnabled(false);
    }

    /**
     * Invoked when "Stop" button clicked
     */
    private void onClickBtnRxStop() {
        // long i4RxCntOk = -1;
        // long i4RxCntFcsErr = -1;
        // long i4RxPer = -1;
        // long[] i4Rx = new long[2];
        long[] u4Value = new long[1];
        mHandler.removeMessages(HANDLER_EVENT_RX);
        for (int i = 0; i < WAIT_COUNT; i++) {
            u4Value[0] = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, 0);
            if (u4Value[0] == 0) {
                break;
            } else {
                SystemClock.sleep(WAIT_COUNT);
                Xlog.w(TAG, "stop Rx test failed at the " + i + "times try");
            }
        }
        setViewEnabled(true);
    }

    /**
     * Set views status
     * 
     * @param state
     *            True if view need to set enabled
     */
    private void setViewEnabled(boolean state) {
        mBtnGo.setEnabled(state);
        mBtnStop.setEnabled(!state);
        mChannelSpinner.setEnabled(state);
        mBandwidthSpinner.setEnabled(state);
    }
}
