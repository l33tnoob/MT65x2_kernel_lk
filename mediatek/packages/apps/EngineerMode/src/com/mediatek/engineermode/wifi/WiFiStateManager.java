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

package com.mediatek.engineermode.wifi;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.os.SystemClock;

import com.mediatek.xlog.Xlog;

public class WiFiStateManager {
    
    private static final String TAG = "EM/WiFi_WiFiStateManager";
    private final WifiManager mWifiManager;
    private int mChipID = 0x0;
    public static final int ENABLE_WIFI_FAIL = -1;
    public static final int INVALID_CHIP_ID = -2;
    public static final int SET_TEST_MODE_FAIL = -3;
    public static final int CHIP_READY = -4;
    protected static final int CHIP_ID_6620 = 0x6620;
    protected static final int CHIP_ID_5921 = 0x5920;
    private static final long DEFAULT_WAIT_TIME = 100;

    /**
     * Constructor
     * 
     * @param activityContext
     *            Context of the activity
     */
    public WiFiStateManager(Context activityContext) {
        mWifiManager = (WifiManager) activityContext
                .getSystemService(Context.WIFI_SERVICE);
    }

    /**
     * Check WiFi chip status
     * 
     * @param activityContext
     *            Context of activity
     * @return WiFi chip status
     */
    public int checkState(Context activityContext) {
        if (mWifiManager != null) {
            if (mWifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                if (mWifiManager.setWifiEnabled(true)) {
                    Xlog.d(TAG, "After enable wifi, state is : "
                            + mWifiManager.getWifiState());
                    while (mWifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                        SystemClock.sleep(DEFAULT_WAIT_TIME);
                    }
                } else {
                    Xlog.w(TAG, "enable wifi power failed");
                    return ENABLE_WIFI_FAIL;
                }
            }
            if (mWifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                Xlog.w(TAG, "enable wifi power failed");
                return ENABLE_WIFI_FAIL;
            }
            if (EMWifi.sIsInitialed) {
                return CHIP_READY;
            } else {
                mChipID = EMWifi.initial();
                if (mChipID == CHIP_ID_6620 || mChipID == CHIP_ID_5921) {
                    Xlog.d(TAG, "Initialize succeed");
                    long result = -1;
                    result = EMWifi.setTestMode();
                    if (result == 0) {
                        synchronized (this) {
                            EMWifi.sIsInitialed = true;
                        }
                        Xlog.i(TAG, "setTestMode succeed");
                    } else {
                        Xlog.w(TAG, "setTestMode failed, ERROR_CODE = "
                                + result);
                        return SET_TEST_MODE_FAIL;
                    }
                } else {
                    return INVALID_CHIP_ID;
                }
            }
        }
        return mChipID;
    }

    /**
     * Disable WiFi
     */
    public void disableWiFi() {
        if (mWifiManager != null) {
            if (mWifiManager.setWifiEnabled(false)) {
                Xlog.d(TAG, "disable wifi power succeed");
            } else {
                Xlog.w(TAG, "disable wifi power failed");
            }
        }
    }
}
