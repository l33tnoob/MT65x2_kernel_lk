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
import android.content.DialogInterface;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class WiFiTestActivity extends Activity {

    private static final String TAG = "EM/WiFiTest";
    static final int DIALOG_WIFI_FAIL = 2;
    static final int DIALOG_WIFI_ERROR = 3;
    static final int CHANNEL_0 = 0;
    static final int CHANNEL_1 = 1;
    static final int CHANNEL_11 = 11;
    static final int CHANNEL_12 = 12;
    static final int CHANNEL_13 = 13;
    static final int CHANNEL_14 = 14;
    static final int ATPARAM_INDEX_COMMAND = 1;
    static final int ATPARAM_INDEX_POWER = 2;
    static final int ATPARAM_INDEX_RATE = 3;
    static final int ATPARAM_INDEX_PREAMBLE = 4;
    static final int ATPARAM_INDEX_ANTENNA = 5;
    static final int ATPARAM_INDEX_PACKLENGTH = 6;
    static final int ATPARAM_INDEX_PACKCOUNT = 7;
    static final int ATPARAM_INDEX_PACKINTERVAL = 8;
    static final int ATPARAM_INDEX_TEMP_COMPENSATION = 9;
    static final int ATPARAM_INDEX_TXOP_LIMIT = 10;
    static final int ATPARAM_INDEX_PACKCONTENT = 12;
    static final int ATPARAM_INDEX_RETRY_LIMIT = 13;
    static final int ATPARAM_INDEX_QOS_QUEUE = 14;
    static final int ATPARAM_INDEX_BANDWIDTH = 15;
    static final int ATPARAM_INDEX_GI = 16;
    static final int ATPARAM_INDEX_CWMODE = 65;
    static final int ATPARAM_INDEX_TRANSMITCOUNT = 32;
    private WiFiStateManager mWiFiStateManager;

    @Override
    protected void onStart() {
        super.onStart();
        checkWiFiChipState();
    }
    
    /**
     * Update WiFi channels
     * 
     * @param channelAdapter 
     * @param channel 
     * @param channelSpinner 
     */
    protected void updateWifiChannel(ChannelInfo channel,
            ArrayAdapter<String> channelAdapter, Spinner channelSpinner) {
        Xlog.e(TAG, "enter updateWifiChannel: " + channel.getChannelIndex());
        if (EMWifi.sIsInitialed) {
            if (0 != channel.getChannelIndex()) {
                channelSpinner.setSelection(0);
            }
            channel.mChannelSelect = channelAdapter.getItem(0);
            int number = channel.getChannelFreq();
            EMWifi.setChannel(number);
            Xlog.i(TAG, "The channel freq =" + number);
        } else {
            Xlog.w(TAG, "Wifi is not initialized");
            showDialog(DIALOG_WIFI_ERROR);
        }
    }
    
    /**
     * Check WiFi chip status
     */
    protected void checkWiFiChipState() {
        int result = 0x0;
        if (mWiFiStateManager == null) {
            mWiFiStateManager = new WiFiStateManager(this);
        }
        result = mWiFiStateManager.checkState(this);
        switch (result) {
        case WiFiStateManager.ENABLE_WIFI_FAIL:
            showDialog(DIALOG_WIFI_FAIL);
            break;
        case WiFiStateManager.INVALID_CHIP_ID:
        case WiFiStateManager.SET_TEST_MODE_FAIL:
            showDialog(DIALOG_WIFI_ERROR);
            break;
        case WiFiStateManager.CHIP_READY:
        case WiFiStateManager.CHIP_ID_6620:
        case WiFiStateManager.CHIP_ID_5921:
            break;
        default:
            break;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_WIFI_FAIL:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_fail);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_fail_message));
            builder.setPositiveButton(R.string.wifi_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            mWiFiStateManager.disableWiFi();
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
                            mWiFiStateManager.disableWiFi();
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
}
