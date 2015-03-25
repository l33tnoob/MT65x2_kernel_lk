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
 * BluetoothDunService.java
 *
 * Project:
 * --------
 *   BT Project
 *
 * Description:
 * ------------
 *   This file is used to provide DUN service api
 *
 * Author:
 * -------
 * Ting Zheng
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
package com.mediatek.bluetooth.dun;

import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.BluetoothTethering;
import com.mediatek.bluetooth.R;

import android.app.Service;

import android.bluetooth.BluetoothDun;
import android.bluetooth.IBluetoothDun;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.util.Log;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.os.*;
import com.android.internal.telephony.Phone;

import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.ServiceManager;
//import android.net.INetworkManagementEventObserver;


public class BluetoothDunService extends Service 
{
    private static final String TAG = "BluetoothDunService";
    private static final boolean DBG = true;

    /**
     * Intent indicating incoming connection request which is sent to
     * BluetoothSimapActivity
     */
    public static final String ACCESS_REQUEST_ACTION = "com.mediatek.bluetooth.dun.accessrequest";

    /**
     * Intent indicating incoming connection request access action by user which is
     * sent from BluetoothDunActivity
     */
    public static final String ACCESS_RESPONSE_ACTION = "com.mediatek.bluetooth.dun.accessresponse";

    /**
     * Intent indicating resend incoming connection notification action by user which is
     * sent from BluetoothDunActivity
     */
    public static final String RESEND_NOTIFICATION_ACTION = "com.mediatek.bluetooth.dun.resendnotification";

    /**
     * Intent indicating incoming connection request access action result by user which is
     * sent from BluetoothDunActivity
     */
    public static final String EXTRA_ACCESS_RESULT =
            "com.mediatek.bluetooth.dun.accessresult";

    /* Result codes */
    public static final int RESULT_USER_ACCEPT = 1;
    public static final int RESULT_USER_REJECT = 2;

    public static final String EXTRA_DEVICE = 
            "com.mediatek.bluetooth.dun.device";

    /**
     * the intent that gets sent when deleting the notification 
     */
    public static final String ACTION_CLEAR_AUTH_NOTIFICATION = 
            "com.mediatek.bluetooth.dun.intent.action.CLEAR_AUTH";

    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;


    /* Start of DUN ID space */
    private static final int DUN_ID_START = BluetoothProfile.getProfileStart(BluetoothProfile.ID_SPP);

    /* DUN Notification IDs, it shall be unique within one app */
    private static final int DUN_AUTHORIZE_NOTIFY = DUN_ID_START + 1;

    /* connection timeout in milliseconds */
    private static final int DUN_CONN_TIMEOUT = 60000;
    private static final int DUN_TETHER_RETRY = 500;

    /* message */
    private static final int MESSAGE_CONNECT_TIMEOUT = 1;
    private static final int MESSAGE_TETHER_RETRY = 2;

    private Context mContext;
    private BluetoothAdapter mAdapter;
    private boolean mHasInitiated = false;

    private int mDunState = BluetoothDun.STATE_DISCONNECTED;
    private String mDunConnPath;
    //private String[] mDnsServers;
    private static final String DUN_Profile = "BluetoothDun";

    private static final String BLUETOOTH_IFACE_ADDR_START= "192.168.44.1";
	
    //private static BluetoothTethering mBTtethering;

    private boolean mTetheringOn;
    
    private BroadcastReceiver mTetheringReceiver = null;


    private final BroadcastReceiver mReceiver = new BroadcastReceiver()
    {
        public void onReceive(Context context, Intent intent)
        {
            String action = intent.getAction();

            if (DBG) log("Receive intent action = " + action);
            if (action.equals(BluetoothDunService.ACCESS_RESPONSE_ACTION))
            {
                int result = intent.getIntExtra(BluetoothDunService.EXTRA_ACCESS_RESULT, 
					BluetoothDunService.RESULT_USER_REJECT);
                if (result == BluetoothDunService.RESULT_USER_ACCEPT)
                {
                    dunConnectRspNative(mDunConnPath, true);
                }
                else
                {
                    dunConnectRspNative(mDunConnPath, false);
                    //dunSetState(BluetoothDun.STATE_DISCONNECTED);
                }
            }
            else if (action.equals(BluetoothDunService.RESEND_NOTIFICATION_ACTION))
            {
                BluetoothDevice device = mAdapter.getRemoteDevice(mDunConnPath);
                createDunAuthNotification(context, device, true);
            }
            else if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION))
            {
				ConnectivityManager connmgr;

                connmgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                NetworkInfo MobileInfo = connmgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
                NetworkInfo WifiInfo = connmgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

                NetworkInfo.DetailedState MobileState = NetworkInfo.DetailedState.IDLE;
                NetworkInfo.DetailedState WifiState = NetworkInfo.DetailedState.IDLE;

                if(MobileInfo!=null)
                    MobileState=MobileInfo.getDetailedState();
                if(WifiInfo!=null)
                    WifiState=WifiInfo.getDetailedState();
                if (DBG) log("NetworkInfo broadcast, MobileState=" + MobileState + ",WifiState=" + WifiState);	

                /* if get network service via wifi, get interface name by "wifi.interface" system property key 
                *  String interfaceName = SystemProperties.get("wifi.interface", "tiwlan0");
                *  log("wifi interface name=" + interfaceName);
                */
                
                if (MobileState == NetworkInfo.DetailedState.IDLE && WifiState == NetworkInfo.DetailedState.IDLE)
                {
                    if (mDunState == BluetoothDun.STATE_CONNECTED)
                    {
                        dunDisconnectNative();
                    }
                }
            }
            else if(action.equals(BluetoothTethering.BLUETOOTH_INTERFACE_ADDED))
            {
                if (DBG) log("receiver BluetoothTethering.BLUETOOTH_INTERFACE_ADDED");
                if(mDunState == BluetoothDun.STATE_CONNECTED)
                {
                    String iface = intent.getStringExtra(BluetoothTethering.BLUETOOTH_INTERFACE_NAME);
                    ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                    if (cm.tether(iface) != ConnectivityManager.TETHER_ERROR_NO_ERROR) {
                        Log.e(TAG, "Error tethering "+iface+", retry...");
                        
                        Message msg = Message.obtain(mHandler, MESSAGE_TETHER_RETRY);
                        msg.obj = iface;
                        mHandler.sendMessageDelayed(msg, DUN_TETHER_RETRY);
                    }
                }
                else
                {
                    if (DBG) log("DUN does not connected");
                }
            }
            else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) 
            {
                if (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR) 
                        == BluetoothAdapter.STATE_TURNING_OFF) 
                {
                    clearService();
                }
            }
        }
    };

    /* Proxy binder API */
    private final IBluetoothDun.Stub mServer = new IBluetoothDun.Stub() {
        public synchronized void dunDisconnect()
        {
            //check state
            dunDisconnectNative();
        }
        
        public synchronized int dunGetState()
        {
            return mDunState;
        }

        public synchronized BluetoothDevice dunGetConnectedDevice()
        {
            if ((mDunState == BluetoothDun.STATE_CONNECTED) &&
		(mDunConnPath != null))
            {
                BluetoothDevice device = mAdapter.getRemoteDevice(mDunConnPath);
                return device;
            }
            return null;
        }

        /* It is used for Settings application. Not update the value util BT is on.
        */
        public void setBluetoothTethering(boolean value) {
            if (!value) {
                if (mDunState == BluetoothDun.STATE_CONNECTING) {
                    dunConnectRspNative(mDunConnPath, false);					
                } else if (mDunState == BluetoothDun.STATE_CONNECTED) {
                    dunDisconnectNative();
                }
            }
        
            if (mAdapter.getState() != BluetoothAdapter.STATE_ON && value) {
                IntentFilter filter = new IntentFilter();
                filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
                mTetheringReceiver = new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {
                        if (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.STATE_OFF)
                                == BluetoothAdapter.STATE_ON) {
                            mTetheringOn = true;
                            mContext.unregisterReceiver(mTetheringReceiver);
                        }
                    }
                };
                mContext.registerReceiver(mTetheringReceiver, filter);
            } else {
                mTetheringOn = value;
            }
        }
        
        public boolean isTetheringOn() {
            return mTetheringOn;
        }

    };

    static {
	System.loadLibrary("extdun_jni");
    }

    public BluetoothDunService() 
    {    
    }

    public void onCreate()
    {
        if (DBG) log("Bluetooth Dun Service is created");
        mContext = getApplicationContext();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mTetheringOn = false;
    }

    public int onStartCommand(Intent intent, int flags, int startId) {

        if (DBG) log("Bluetooth Dun Service is started");

        if (!mHasInitiated) {
            if (!initNative())
            {
                Log.e(TAG, "Could not init BluetoothDunService");
                notifyProfileState(BluetoothProfileManager.STATE_ABNORMAL);
                return START_STICKY;			
            }

            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            intentFilter.addAction(BluetoothDunService.ACCESS_RESPONSE_ACTION);
            intentFilter.addAction(BluetoothDunService.RESEND_NOTIFICATION_ACTION);
            intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
            intentFilter.addAction(BluetoothTethering.BLUETOOTH_INTERFACE_ADDED);
            mContext.registerReceiver(mReceiver, intentFilter);
    		
            dunEnableNative();
            // broadcast enabling to profilemanager
            notifyProfileState(BluetoothProfileManager.STATE_ENABLING);

            mHasInitiated = true;
        }
        else {
            if (DBG) log("Already started, just return!");
            return START_STICKY;
        }
        
        return START_STICKY;
    }

    public IBinder onBind(Intent intent) {
	Log.i(TAG, "Enter onBind()");
	if (IBluetoothDun.class.getName().equals(intent.getAction())) {
	    return mServer;
	}
	return null;
    }

    public void onDestroy()
    {
        if (DBG) log("Bluetooth Dun Service is destroyed");

        clearService();
    }

    private final Handler mHandler = new Handler() 
    {
        @Override
        public void handleMessage(Message msg) 
        {
            switch (msg.what) 
            {
            case MESSAGE_CONNECT_TIMEOUT:
                if (mDunState == BluetoothDun.STATE_CONNECTING) 
                {
                    dunConnectRspNative(mDunConnPath, false);					
                    removeDunAuthNotification(DUN_AUTHORIZE_NOTIFY);
                    dunSetState(BluetoothDun.STATE_DISCONNECTED);					
                }
                break;
            case MESSAGE_TETHER_RETRY:
                {
                    ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                    String iface = (String)msg.obj;
                    if (cm.tether(iface) != ConnectivityManager.TETHER_ERROR_NO_ERROR) {
                        Log.e(TAG, "Error tethering again"+iface);
                    }
                }
                break;
            }
        }
    };

    private synchronized void onDunConnectReq(String path)
    {
        if (!mTetheringOn)
        {
            dunConnectRspNative(path, false);  
            return;
        }

        // create dun activity to show authorize dialog
        log("dun connect request, device address: " + path);
        BluetoothDevice device = mAdapter.getRemoteDevice(path);

        createDunAuthNotification(mContext, device, false);

        mDunConnPath = path;
        dunSetState(BluetoothDun.STATE_CONNECTING);

        Message msg = Message.obtain(mHandler, MESSAGE_CONNECT_TIMEOUT);
        mHandler.sendMessageDelayed(msg, DUN_CONN_TIMEOUT);
    }

    private synchronized void onDunEnableCnf(boolean result)
    {

        if (result)
        {
            notifyProfileState(BluetoothProfileManager.STATE_ENABLED);
        }
        else
        {
            notifyProfileState(BluetoothProfileManager.STATE_ABNORMAL);
        }

    }
	
    private synchronized void onDunDisableCnf(boolean result)
    {

        if (result)
        {
            notifyProfileState(BluetoothProfileManager.STATE_DISABLED);
        }
        else
        {
            notifyProfileState(BluetoothProfileManager.STATE_ABNORMAL);
        }

    }
	
    private synchronized void onDunStateChanged(String path, String stateValues)
    {
        int state = convertStringtoState(stateValues);
        int prevstate = mDunState;
        BluetoothTethering btTethering;

        log("dun state changed to " + stateValues);
        if (state == BluetoothDun.STATE_CONNECTED)
        {
            //startNetworkService();
            mHandler.removeMessages(MESSAGE_CONNECT_TIMEOUT);
        }
        else if (state == BluetoothDun.STATE_DISCONNECTED)
        {
            btTethering = BluetoothTethering.getBluetoothTetheringInstance();
            btTethering.unregisterBTTether();
            mHandler.removeMessages(MESSAGE_TETHER_RETRY);

            if (prevstate == BluetoothDun.STATE_CONNECTING)
            {
                mHandler.removeMessages(MESSAGE_CONNECT_TIMEOUT);
                removeDunAuthNotification(DUN_AUTHORIZE_NOTIFY);
            }
        }

        dunSetState(state);
    }

    private synchronized void onDunDialupReq()
    {
        ConnectivityManager connmgr;
        String response;
        String[] dnsServers;
        BluetoothTethering btTethering;
		
        dnsServers = new String[2];
        connmgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        
        NetworkInfo MobileInfo = connmgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
		NetworkInfo WifiInfo = connmgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (MobileInfo == null && WifiInfo == null)
        {
            Log.w(TAG, "dialup request, get network info failed");
            response = "unknown";
            dunDialupRspNative(response, BLUETOOTH_IFACE_ADDR_START, dnsServers);
            return;			
        }
		NetworkInfo.State MobileState = NetworkInfo.State.UNKNOWN;
		NetworkInfo.State WifiState = NetworkInfo.State.UNKNOWN;
		
		if(MobileInfo!=null)
			MobileState=MobileInfo.getState();
		if(WifiInfo!=null)
			WifiState=WifiInfo.getState();	
	
        if (MobileState == NetworkInfo.State.CONNECTED || WifiState == NetworkInfo.State.CONNECTED)
        {
        	NetworkInfo.State state = (MobileState == NetworkInfo.State.CONNECTED)?MobileState:WifiState;
			if (DBG) log("startUsingNetworkFeature: ("	+ state + ")");
            response = "active";
            dnsServers[0] = SystemProperties.get("net.dns1");
            dnsServers[1] = SystemProperties.get("net.dns2");
            if (DBG) log("Network connected, DNS1=" + dnsServers[0] + ", " + "DNS2=" + dnsServers[1]);				  
            if (dnsServers[1].isEmpty())
            {
                dnsServers[1] = "8.8.8.8";
            }
            if (DBG) log("Network connected, DNS2=" + dnsServers[1] + ", " + "DNS1=" + dnsServers[0]);				  
            btTethering = BluetoothTethering.getBluetoothTetheringInstance();
            btTethering.registerBTTether(this);
        }
        else if (MobileState == NetworkInfo.State.SUSPENDED || WifiState == NetworkInfo.State.SUSPENDED)
        {
            response = "busy";
        }
        else if (MobileState == NetworkInfo.State.DISCONNECTED || WifiState == NetworkInfo.State.DISCONNECTED)
        {
            response = "unavailable";
        }
        else
        {
            response = "unknown";
        }
        if (DBG) log("dunDialupRspNative response: ("	+ response + ")");
        dunDialupRspNative(response, BLUETOOTH_IFACE_ADDR_START, dnsServers);
    }

    private void dunSetState(int state)
    {
        int prevstate = mDunState;
        BluetoothDevice device = null;
        mDunState = state;
        
        if (mDunConnPath != null)
        {
            device = mAdapter.getRemoteDevice(mDunConnPath);
        }
		
        Intent intent = new Intent(BluetoothDun.STATE_CHANGED_ACTION);
        intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_DUN);
        intent.putExtra(BluetoothDun.EXTRA_STATE, state);
        intent.putExtra(BluetoothDun.EXTRA_PRE_STATE, prevstate);
        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, device);		
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
    }

    private void clearService()
    {
        if (!mHasInitiated)
        {
            return;
        }
        
        if (mDunState != BluetoothDun.STATE_DISCONNECTED)		
        {
            //stopNetworkService();
            mHandler.removeMessages(MESSAGE_TETHER_RETRY);
			
            if (mDunState == BluetoothDun.STATE_CONNECTING)
            {
                mHandler.removeMessages(MESSAGE_CONNECT_TIMEOUT);
                removeDunAuthNotification(DUN_AUTHORIZE_NOTIFY);				
            }
        }
		
        dunDisableNative();
        cleanupNative();
        mHasInitiated = false;

        mContext.unregisterReceiver(mReceiver);
    }

    private void createDunAuthNotification(Context context, BluetoothDevice device, boolean resend)
    {
        NotificationManager nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notification = null;

        // Create an intent triggered by clicking on the status icon
        Intent intent = new Intent();
        intent.setClass(context, BluetoothDunActivity.class); 		
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction(ACCESS_REQUEST_ACTION);
        intent.putExtra(BluetoothDunService.EXTRA_DEVICE, device);

        // Create an intent triggered by clicking on the
        // "Clear All Notifications" button
        Intent deleteIntent = new Intent(ACTION_CLEAR_AUTH_NOTIFICATION);
        deleteIntent.setClass(context, BluetoothDunReceiver.class);
        
        String name = device.getName();
        notification = new Notification(android.R.drawable.stat_sys_data_bluetooth, context
                    .getString(R.string.bluetooth_dun_notification_connect_request_ticker), System.currentTimeMillis());
        notification.setLatestEventInfo(context, context.getString(R.string.bluetooth_dun_notification_connect_request_title),
                    context.getString(R.string.bluetooth_dun_notification_connect_request_message, name), PendingIntent
                    .getActivity(context, 0, intent, 0));
		
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        notification.flags |= Notification.FLAG_ONLY_ALERT_ONCE;
        if (!resend)
        {
            notification.defaults |= Notification.DEFAULT_SOUND;
            notification.defaults |= Notification.DEFAULT_VIBRATE;
        }
        else
        {
            notification.defaults = 0;
        }
        notification.deleteIntent = PendingIntent.getBroadcast(context, 0, deleteIntent, 0);

        nm.notify(DUN_AUTHORIZE_NOTIFY, notification);
    }

    private void removeDunAuthNotification(int id) 
    {
        NotificationManager nm = (NotificationManager)mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(id);
    }
/*
    private boolean startNetworkService()
    {
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        INetworkManagementService service;

        if (b != null)
        {
             service = INetworkManagementService.Stub.asInterface(b);
        }
        else
        {
            Log.w(TAG, "start network service, network management service is not available!");
            return false;  			
        }

        if (service == null)
        {
            Log.w(TAG, "start network service, get network management service failed!");
            return false;			
        }
		
        String intIfname = "btn0";
        String extIfname = "ccmni0";		

        try {		
            service.setIpForwardingEnabled(true);
        } catch (Exception e) {
            Log.w(TAG, "set ip forward enabled error");
        }

        try {
            service.enableNat(intIfname, extIfname);
        } catch (Exception e) {
            Log.w(TAG, "enable nat error");
        }
        return true;
    }
*/
/*
    private void stopNetworkService()
    {
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        INetworkManagementService service;

        if (b != null)
        {
             service = INetworkManagementService.Stub.asInterface(b);
        }
        else
        {
            Log.w(TAG, "stop network service, network management service is not available!");
            return;  			
        }

        if (service == null)
        {
            Log.w(TAG, "stop network service, get network management service failed!");
            return;			
        }

        String intIfname = "btn0";
        String extIfname = "ccmni0";		

        try {		
            service.setIpForwardingEnabled(false);
        } catch (Exception e) {
            Log.w(TAG, "set ip forward disable error");
        }

        try {
            service.disableNat(intIfname, extIfname);
        } catch (Exception e) {
            Log.w(TAG, "disable nat error");
        }
    }
*/
    private void notifyProfileState(int state)
    {
        log("notifyProfileState: " + state);

	Intent intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
	intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_DUN);
	intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);
	mContext.sendBroadcast(intent, BLUETOOTH_PERM);

    }

    private int convertStringtoState(String value) {
        if (value.equalsIgnoreCase("disconnected"))
            return BluetoothDun.STATE_DISCONNECTED;
        if (value.equalsIgnoreCase("connected"))
            return BluetoothDun.STATE_CONNECTED;
        return -1;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private native boolean initNative();
    private native void cleanupNative();
    private synchronized native void dunEnableNative();
    private synchronized native void dunDisableNative();
    private synchronized native void dunDisconnectNative();
    private synchronized native void dunConnectRspNative(String path, boolean accept);
    private synchronized native void dunDialupRspNative(String response, String ipBase, String[] dnsServers);
}
