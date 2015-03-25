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

package com.mediatek.bluetooth.settings;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

import java.util.ArrayList;

import com.mediatek.activity.AssembledPreferenceActivity;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.ClassUtils;
///M: import medietek profile package
import android.bluetooth.ConfigHelper;
import android.bluetooth.ProfileConfig;

/**
 * @author Jerry Hsu
 *
 * This activity will be started by Intent from [/packages/apps/Settings/res/xml/bluetooth_settings.xml]
 */
public class BluetoothAdvancedSettingsActivity extends AssembledPreferenceActivity {

    private static ArrayList<AssemblyPreference> registeredPreferences = new ArrayList<AssemblyPreference>(4);
    static {
        // *** Note: profile implementation MUST provide constructor without parameter
        if(ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_FTP)) {
        	registerProfile( "com.mediatek.bluetooth.ftp.BluetoothFtpServerAdvSettings" );    // FTP
        }
		if(ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_SIMAP)) {
        	registerProfile( "com.mediatek.bluetooth.simap.BluetoothSimapSettings" );    // SIMAP
		}
        registerProfile( "com.mediatek.bluetooth.pan.BluetoothPanSettings" );    // PAN
        if(ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_PRXR)) {
        	registerProfile( "com.mediatek.bluetooth.prx.reporter.PrxrBluetoothSettings" );    // PRXR
        }
		if(ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_MAPS)) {
        	registerProfile( "com.mediatek.bluetooth.map.BluetoothMapServerSettings" );    // MAP
		}
    }

    private static void registerProfile( String className ){

        AssemblyPreference pref = (AssemblyPreference)ClassUtils.newObject( className, null, null );
        if( pref != null ){
            // register profile if it's not null
            registeredPreferences.add( pref );
        }
        else {
            // skip profile if it can't be loaded
            BtLog.e( "BluetoothAdvancedSettingsActivity: fail to register profile[" + className + "]" );
        }
    }

    public BluetoothAdvancedSettingsActivity(){

        super( registeredPreferences );
    }

    private IntentFilter filter = new IntentFilter( BluetoothAdapter.ACTION_STATE_CHANGED );

    private BroadcastReceiver receiver = new BroadcastReceiver(){
        @Override
        public void onReceive( Context context, Intent intent ){
            int btState = intent.getIntExtra( BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR );

            BtLog.d( "onReceive: action[" + intent.getAction() + "] btState[" + btState + "]" );

            if( btState == BluetoothAdapter.STATE_TURNING_OFF ){
                finish();
            }
        }
    };

    @Override
    protected void onCreate( Bundle savedInstanceState ){

        super.onCreate( savedInstanceState );

        registerReceiver( receiver, filter );
    }

    @Override
    protected void onDestroy(){

        try
        {
            unregisterReceiver( receiver );
        }
        catch( Exception ex ){
            // just ignore
        }

        super.onDestroy();
    }
}
