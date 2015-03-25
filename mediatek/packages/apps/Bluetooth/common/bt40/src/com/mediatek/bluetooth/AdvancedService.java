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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*******************************************************************************
 *
 * Filename:
 * ---------
 * AdvancedService.java
 *
 * Project:
 * --------
 *   BT Project
 *
 * Description:
 * ------------
 *   This file is used to provide service for trigger advance profiles enable/disable
 *   actions.
 *
 * Author:
 * -------
 * Dexiang Jiang
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision: 
 * $Modtime:
 * $Log: 
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/
package com.mediatek.bluetooth;

import android.app.Service;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfileManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;
import android.util.Log;
import android.os.IBinder;

import com.mediatek.bluetooth.util.BtLog;
///M: import medietek profile package
import android.bluetooth.ConfigHelper;
import android.bluetooth.ProfileConfig;

public class AdvancedService extends Service {
	
    private static final String TAG = "BluetoothAdvancedService";
    private static final boolean DBG = true;

    // service class
    private static final String FTP_SERVICE_CLASS = "com.mediatek.bluetooth.ftp.BluetoothFtpService";  
    private static final String DUN_SERVICE_CLASS = "com.mediatek.bluetooth.dun.BluetoothDunService";
    private static final String BIP_SERVICE_CLASS = "com.mediatek.bluetooth.bip.BipService";
	private static final String SIMAP_SERVICE_CLASS = "com.mediatek.bluetooth.simap.BluetoothSimapService";
	private static final String MAP_SERVICE_CLASS = "com.mediatek.bluetooth.map.BluetoothMapServerService";
    //For BLE Service
    private static final String PROFILE_SERVICE_CLASS = "com.mediatek.bluetooth.ProfileService";

    private Context mContext;
    private BluetoothAdapter mAdapter;
    private boolean mIsInitiated = false;
	private boolean mIsProfilesStarted = false;

	// JNI reference
	static {
		System.loadLibrary("extadvanced_jni");
	}


	@Override
    public void onCreate() {
    
        if (DBG) log("Advanced Service for advanced profiles is created");
        mContext = getApplicationContext();
        mAdapter = BluetoothAdapter.getDefaultAdapter();

		if (!mIsInitiated) {    
		    if (!initNative())
            {
                if (DBG) log("Could not init Bluetooth Advanced Service");
                return;			
            }			   		
            advancedEnableNative();
            mIsInitiated = true;

            // Start profiles if BT is enabled
			this.preStartProfilesService();
        }
        else {
            if (DBG) log("Already started, just return!");
            return;
        }
    }

	@Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        if (DBG) log("Bluetooth Advanced Service is started");

		// Start profiles if BT is enabled
		this.preStartProfilesService();
		
        return START_STICKY;
    }

    @Override
	public void onDestroy() {
	
        if (DBG) log("Bluetooth Advanced Service is destroyed");
        clearService();
		mIsInitiated = false;
    }

    @Override
    public IBinder onBind(Intent intent) {

        if (DBG) log("unsupported function: BluetoothAdvancedService.onBind()");
        return null;
    }

    private synchronized void onAdvancedEnableCnf(boolean result) {	
        BtLog.i("onAdvancedEnableCnf, result[" + result + "]");
    }
	
    private synchronized void onAdvancedDisableCnf(boolean result) {	
        BtLog.i("onAdvancedDisableCnf, result[" + result + "]");
    }
	
	//Power On	=> start profile services
    private synchronized void onAdvancedStartRequestInd(boolean result) {
    
        int btState;
		btState = mAdapter.getState();
		BtLog.i("onAdvancedStartRequestInd, btState[" + btState + "]," + 
			"mIsProfilesStarted[" + mIsProfilesStarted + "]");

		if (!mIsProfilesStarted &&
			(btState == BluetoothAdapter.STATE_TURNING_ON ||
			btState == BluetoothAdapter.STATE_ON)) {

			mIsProfilesStarted = true;
            this.startProfileServices(mContext);	
        }       
    }

	//Power Off	=> start profile services
    private synchronized void onAdvancedStopRequestInd(boolean result) {
    
        int btState;
		btState = mAdapter.getState();
		BtLog.i("onAdvancedStopRequestInd, btState[" + btState + "]," + 
			"mIsProfilesStarted[" + mIsProfilesStarted + "]");

		if (mIsProfilesStarted &&
			(btState == BluetoothAdapter.STATE_TURNING_OFF ||
			btState == BluetoothAdapter.STATE_OFF)) {

            this.stopProfileServices(mContext);
			mIsProfilesStarted = false;
        } 
    }

    private void clearService() {
		
        if (!mIsInitiated)
        {
        	if (DBG) log("Advanced Service is already initiated");
            return;
        }
		
        advancedDisableNative();
        cleanupNative();
        mIsInitiated = false;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    // Advanced Serviced maybe delayed to start(by received BOOT_COMPLETE intent) in phone 
    // boot case(if last time BT is enabled).Bluedroid will do enable after get BOOT_COMPLETE, but
    // intent delay will cause Advanced Service can't be start and get profile start notify from native 
    // GAP, so check BT state in Advanced Service onCreate to make sure advanced profiles can be 
    // start in time.
    private void preStartProfilesService() {

		BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        int btState = btAdapter.getState();
		BtLog.i("preStartProfilesService, btState[" + btState + "]," + 
			"mIsProfilesStarted[" + mIsProfilesStarted + "]");   

		// BT must be enabled, then profiles can be start
        if (!mIsProfilesStarted && btState == BluetoothAdapter.STATE_ON ) {

            mIsProfilesStarted = true;
            this.startProfileServices(mContext);
		}
	}
	
    private void startProfileServices(Context context) {

        BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        int btState = btAdapter.getState();
		BtLog.i("startProfileServices, btState[" + btState + "]");
		
        if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_FTP) && btAdapter != null && 
			btState != BluetoothAdapter.STATE_OFF && btState != BluetoothAdapter.STATE_TURNING_OFF) {
			if (DBG) log("Advanced Service => start FTP");
            this.startService(context, FTP_SERVICE_CLASS);
        }
        if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_BIP) && btAdapter != null && 
			(btState != BluetoothAdapter.STATE_OFF || btState != BluetoothAdapter.STATE_TURNING_OFF) ) {
			if (DBG) log("Advanced Service => start BIP");
            this.startService(context, BIP_SERVICE_CLASS);
        }
        if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_MAPS) && btAdapter != null &&
			(btState != BluetoothAdapter.STATE_OFF || btState != BluetoothAdapter.STATE_TURNING_OFF) ) {
			if (DBG) log("Advanced Service => start MAP");
            this.startService(context, MAP_SERVICE_CLASS);
        }
        if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_DUN) && btAdapter != null &&
			(btState != BluetoothAdapter.STATE_OFF || btState != BluetoothAdapter.STATE_TURNING_OFF) ) {
			if (DBG) log("Advanced Service => start DUN");
            this.startService(context, DUN_SERVICE_CLASS);
        }	
        if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_SIMAP) && btAdapter != null &&
			(btState != BluetoothAdapter.STATE_OFF || btState != BluetoothAdapter.STATE_TURNING_OFF) ) {
			if (DBG) log("Advanced Service => start SIMAP");
            this.startService(context, SIMAP_SERVICE_CLASS);
        }	

		if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_PRXR) && btAdapter != null &&
			(btState != BluetoothAdapter.STATE_OFF || btState != BluetoothAdapter.STATE_TURNING_OFF) ) {
			if (DBG) log("Advanced Service => start PRXP");
            this.startService(context, PROFILE_SERVICE_CLASS);
        }	
    }

    private void stopProfileServices(Context context) {
		if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_FTP)) {
        	this.stopService(context, FTP_SERVICE_CLASS);
		}
		if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_BIP)) {
        	this.startService(context, BIP_SERVICE_CLASS, "action", "com.mediatek.bluetooth.bipiservice.action.BIP_DISABLE");
        }
		if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_DUN)) {
			this.stopService(context, DUN_SERVICE_CLASS);
		}
        if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_MAPS)) {
            this.stopService(context, MAP_SERVICE_CLASS);
        }
		if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_SIMAP)) {
			this.stopService(context, SIMAP_SERVICE_CLASS);
		}
		if (ConfigHelper.checkSupportedProfiles(ProfileConfig.PROFILE_ID_PRXR)) {
			this.stopService(context, PROFILE_SERVICE_CLASS);
		}
    }

    private void startService(Context context, String serviceClass) {

        try {
            context.startService(new Intent(context, Class.forName(serviceClass)));
        } catch (ClassNotFoundException ex) {
            BtLog.e("start service for class[" + serviceClass + "] fail:", ex);
        }
    }

    private void stopService(Context context, String serviceClass) {

        try {
            context.stopService(new Intent(context, Class.forName(serviceClass)));
        } catch (ClassNotFoundException ex) {
            BtLog.e("stop service for class[" + serviceClass + "] fail:", ex);
        }
    }

    private void startService(Context context, String serviceClass, String extraName, String extraValue) {

        try {
            context.startService(new Intent(context, Class.forName(serviceClass)).putExtra(extraName, extraValue));
        } catch (ClassNotFoundException ex) {
            BtLog.e("start service for class[" + serviceClass + "] fail:", ex);
        }
    }

    private native boolean initNative();
    private native void cleanupNative();
    private native void advancedEnableNative();
    private native void advancedDisableNative();
}
