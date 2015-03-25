/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.bluetooth;

import java.lang.IllegalArgumentException;
import android.app.Application;
import android.util.Log;
import android.app.SearchManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.database.ContentObserver;
import android.os.Handler;

import android.content.ComponentName;
import android.content.pm.PackageManager;
import android.bluetooth.ConfigHelper;
import android.bluetooth.ProfileConfig;

import com.mediatek.bluetooth.util.BtLog;

public class BluetoothApplication extends Application {
    private static final String TAG = "BluetoothApplication";
    private BluetoothReceiver mReceiver;
    private static final boolean DBG = true;
    //For Debugging only
    private static int sRefCount=0;

    private static final String ADVANCED_SERVICE_CLASS = "com.mediatek.bluetooth.AdvancedService";
	
    private static final String PRXM_ENTRY_CLASS = "com.mediatek.bluetooth.prx.monitor.PrxmDeviceMgmtActivity";
	
    static {
        if (DBG) Log.d(TAG,"Loading JNI Library");
    }       

    public BluetoothApplication() {
        super();
        if (DBG) {
            synchronized (BluetoothApplication.class) {
                sRefCount++;
                Log.d(TAG, "REFCOUNT: Constructed "+ this + " Instance Count = " + sRefCount);
            }
        }
    }    
    
    @Override
    public void onCreate() {
        super.onCreate();

        BtLog.d("BluetoothApplication.onCreate");

        mReceiver = new BluetoothReceiver();
        // Register intent receivers
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_MEDIA_EJECT);
        filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        filter.addDataScheme("file");
        registerReceiver(mReceiver, filter);
        
		//start advanced service
		startService((Context)this, ADVANCED_SERVICE_CLASS);
		//enable/disable proximity monitor profile activity entry
		enableProfEntry(ProfileConfig.PROFILE_ID_PRXM);        
    }

    /**
     * There's no guarantee that this function is ever called.
     */
    @Override
    public void onTerminate() {
        super.onTerminate();

        BtLog.d("BluetoothApplication.onTerminate");

        unregisterReceiver(mReceiver);
    }

    @Override
    protected void finalize() {
        if (DBG) {
            synchronized (BluetoothApplication.class) {
                sRefCount--;
                Log.d(TAG, "REFCOUNT: Finalized: " + this +", Instance Count = " + sRefCount);
            }
        }
    }
    
	private void enableProfEntry(String profileId)
	{
		PackageManager pm = getPackageManager();
		ComponentName compName;
		int enableFlag = PackageManager.COMPONENT_ENABLED_STATE_ENABLED;

		if(profileId.equals(ProfileConfig.PROFILE_ID_PRXM))
		{
			try {
				compName = new ComponentName((Context)this, PRXM_ENTRY_CLASS);
				enableFlag = ConfigHelper.checkSupportedProfiles(profileId)?
					         PackageManager.COMPONENT_ENABLED_STATE_ENABLED:
					         PackageManager.COMPONENT_ENABLED_STATE_DISABLED; 
				Log.d(TAG, "Proximity Profile Entry enable: " + enableFlag);
				pm.setComponentEnabledSetting(compName, enableFlag, PackageManager.DONT_KILL_APP);
			}catch (IllegalArgumentException ex) {
				Log.d(TAG, "Proximity Profile Entry Exception: " + ex);
			}
		}
	}

	private void startService(Context context, String serviceClass) {

	    try {

			Log.d(TAG, "start service [" + serviceClass);
	    	context.startService(new Intent(context, Class.forName(serviceClass)));
			//context.startService(new Intent(context, BluetoothAdvancedService.class));
	    } catch (ClassNotFoundException ex) {
	    	Log.d(TAG, "start service for class[" + serviceClass + "] fail:", ex);
	    }
    }
}
