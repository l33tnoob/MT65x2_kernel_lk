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

package com.mediatek.FMTransmitter;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

public class FMTransmitterUpgradeReceiver extends BroadcastReceiver {
    static final String TAG = "FMTxUpgradeReceiver";
    static final String PREF_DB_VERSION = "db_version";
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        // We are now running with the system up, but no apps started,
        // so can do whatever cleanup after an upgrade that we want.
        mContext = context.getApplicationContext();
        SharedPreferences prefs = context.getSharedPreferences(TAG, Context.MODE_PRIVATE);
        int prefVersion = prefs.getInt(PREF_DB_VERSION, 0);

            if (prefVersion != FMTransmitterContentProvider.DATABASE_VERSION) {
                FMTxLogUtils.d(TAG,
                        "prefVersion:" + "currentVersion:" + prefVersion + FMTransmitterContentProvider.DATABASE_VERSION);
                // if the preversion is not the same with the currentversion
                // put current version to database
                prefs.edit().putInt(PREF_DB_VERSION, FMTransmitterContentProvider.DATABASE_VERSION).commit();
                // delete all the searched data if prefVersion is not the same as current version
                FMTransmitterStation.cleanSearchedStations(mContext);
                
                // if currentVersion is MTK_50KHZ_SUPPORT,update currentstation to 10000 
                // else if is MTK_100KHZ_SUPPORT,update currentstation to 1000
                FMTransmitterStation.setCurrentStation(mContext, FMTransmitterStation.FIXED_STATION_FREQ);
               
            }  
    }
}
