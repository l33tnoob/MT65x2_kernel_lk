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

package com.mediatek.bluetooth.avrcp;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * Listener the sysetm intent or other to report itself
 */
public class BTAvrcpSystemListener {
    public static final String TAG = "MIS_AVRCP";

    private BTAvrcpMusicAdapter mAdapter;

    private boolean mIsRegBattery;

    private boolean mIsRegSystem;

    private int mBatteryStatus;

    private int mSystemStatus;

    public BTAvrcpSystemListener(BTAvrcpMusicAdapter adapter) {
        // for test
        mAdapter = adapter;
        mIsRegBattery = false;
        mIsRegSystem = false;
        mBatteryStatus = 0;
        mSystemStatus = 0;

        // register the global intent to receive it
        onBatteryStatusChange(0);
        onSystemStatusChange(1);
    }

    public synchronized void startListener(BTAvrcpMusicAdapter adapter) {
        // VOLUME_MUSIC
        mAdapter = adapter;
    }

    public synchronized void stopListener() {
        mAdapter = null;
    }

    public void onReceive(Context context, Intent intent) {
        Log.v("AVRCP", "[BT][AVRCP] onReceive :" + intent.toString());
    }

    public synchronized void onBatteryStatusChange(int status) {
        if (null != mAdapter && mIsRegBattery) {
            mIsRegBattery = false;
            mAdapter.notificationBatteryStatusChanged((byte) 0, (byte) 0, (byte) status);
        }
    }

    public synchronized void onSystemStatusChange(int status) {
        if (null != mAdapter && mIsRegSystem) {
            mIsRegSystem = false;
            mAdapter.notificationSystemStatusChanged((byte) 0, (byte) 0, (byte) status);
        }
    }

    public boolean regNotificationEvent(byte eventId, int interval) {
        if (null == mAdapter) {
            return false;
        }
        if (BTAvrcpProfile.EVENT_BATT_STATUS_CHANGED == eventId) {
            // optional: get the current battery status
            mAdapter.notificationBatteryStatusChanged((byte) 0, (byte) 1, (byte) 1);
            return true;
        }
        if (BTAvrcpProfile.EVENT_SYSTEM_STATUS_CHANGED == eventId) {
            // optional: get the current system status
            mAdapter.notificationSystemStatusChanged((byte) 0, (byte) 1, (byte) 1);
            return true;
        }
        if (BTAvrcpProfile.EVENT_VOLUME_CHANGED == eventId) {
            // optional: get the current system status
            mAdapter.notificationVolumeChanged((byte) 0, (byte) 1, (byte) 1);
            return true;
        }
        return false;
    }

    public synchronized void onVolumeStatusChange(int volume) {
        byte absVolume;
        if (null != mAdapter && mIsRegSystem) {
            mIsRegSystem = false;
            absVolume = 0;
            mAdapter.notificationVolumeChanged((byte) 0, (byte) 0, (byte) absVolume);
        }
    }
}
