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


package com.mediatek.bluetooth.simap;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.BluetoothProfile;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.content.BroadcastReceiver;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.bluetooth.BluetoothServerSocket;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.provider.ContactsContract.RawContacts;
import android.telephony.TelephonyManager;
import com.mediatek.telephony.TelephonyManagerEx;
import com.android.internal.telephony.PhoneConstants;
import android.text.TextUtils;
import android.util.Log;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.bluetooth.BluetoothSimap;
import android.bluetooth.IBluetoothSimap;
import android.bluetooth.IBluetoothSimapCallback;
import android.bluetooth.BluetoothProfileManager;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;

import com.mediatek.common.featureoption.FeatureOption;

public class BluetoothSimapService extends Service {
    private static final String TAG = "BluetoothSimapService";

    public static final boolean DEBUG = true;

    public static final boolean VERBOSE = true;


    /**
     * Intent indicating incoming connection request which is sent to
     * BluetoothSimapActivity
     */
    public static final String ACCESS_REQUEST_ACTION = "com.mediatek.bluetooth.simap.accessrequest";

    /**
     * Intent indicating incoming connection request accepted by user which is
     * sent from BluetoothSimapActivity
     */
    public static final String ACCESS_ALLOWED_ACTION = "com.mediatek.bluetooth.simap.accessallowed";

    /**
     * Intent indicating incoming connection request denied by user which is
     * sent from BluetoothSimapActivity
     */
    public static final String ACCESS_DISALLOWED_ACTION =
            "com.mediatek.bluetooth.simap.accessdisallowed";


    /**
     * Intent indicating timeout for user confirmation, which is sent to
     * BluetoothSimapActivity
     */
    public static final String USER_CONFIRM_TIMEOUT_ACTION =
            "com.mediatek.bluetooth.simap.userconfirmtimeout";

    /**
     * Intent indicating incoming connection request which is sent to
     * BluetoothSimapActivity
     */
    public static final String CONNECTED_NOTIFY_ACTION = "com.mediatek.bluetooth.simap.connectednotify";

    /**
     * Intent Extra name indicating always allowed which is sent from
     * BluetoothSimapActivity
     */
    public static final String EXTRA_ALWAYS_ALLOWED = "com.mediatek.bluetooth.simap.alwaysallowed";

    public static final String THIS_PACKAGE_NAME = "com.mediatek.bluetooth";

    public static final int SIMAP_AUTHORIZE_IND = 105;
    public static final int SIMAP_CONNECTED     = 106;
    public static final int SIMAP_DISCONNECTED  = 107;


    public static final String BTSIMAP_CONNECTED =
        "com.mediatek.bluetooth.simap.intent.action.BTSIMAP_CONNECTED";

    public static final String BTSIMAP_DISCONNECTED =
        "com.mediatek.bluetooth.simap.intent.action.BTSIMAP_DISCONNECTED";

	
	private final static int MAX_SIM_NUM = PhoneConstants.GEMINI_SIM_NUM;

    /**
     * the intent that gets sent when deleting the notification 
     */
    public static final String ACTION_CLEAR_AUTH_NOTIFICATION = "com.mediatek.bluetooth.simap.intent.action.CLEAR_AUTH";
    public static final String ACTION_CLEAR_CONN_NOTIFICATION = "com.mediatek.bluetooth.simap.intent.action.CLEAR_CONN";

    public static final String SEND_SIMUNAVALIBLE_IND =
        "com.mediatek.bluetooth.simap.intent.action.SEND_SIMUNAVALIBLE_IND";


    public static final int RESULT_FAILURE = 0;
    public static final int RESULT_SUCCESS = 1;
    /** Connection canceled before completion. */
    public static final int RESULT_CANCELED = 2;


    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;

    private static final String BLUETOOTH_ADMIN_PERM = android.Manifest.permission.BLUETOOTH_ADMIN;

    private static final int START_LISTENER = 1;

    private static final int USER_TIMEOUT = 2;

    private static final int USER_CONFIRM_TIMEOUT_VALUE = 100000;

    //private static final int TIME_TO_WAIT_VALUE = 6000;

    /* Start of SIMAP ID space */
    private static final int SIMAP_ID_START = BluetoothProfile.getProfileStart(BluetoothProfile.ID_SIMAP);

    private static final int NOTIFICATION_ID_ACCESS = SIMAP_ID_START + 1;
    private static final int NOTIFICATION_ID_CONNECTED = SIMAP_ID_START + 2;

    private PowerManager.WakeLock mWakeLock = null;

    private BluetoothAdapter mAdapter;

    private BluetoothDevice mRemoteDevice = null;

    private static String sRemoteDeviceName = null;

    private static boolean mHasStarted = false;

    private volatile boolean mInterrupted;

    private boolean mServiceInitiated = false;

	private boolean savedEnableState = false;

	private boolean enableAction = false;

    private int mState;

    private int mStartId = -1;

	private int mSIMIndex = 1;


    /* Native data */
    private int mNativeData;

    /* SharedPreferences for storing SIMAP settings: enable and SIM selection. */
    private SharedPreferences mPreferences;

    private static final String KEY_SIMAP_SETTINGS = "simap_server_settings";
    private static final String KEY_SIMAP_ENABLE = "simap_server_enable";
    private static final String KEY_SIMAP_SIM_INDEX = "simap_server_sim_index";

    /* Package only, for braodcast */
    static final String DISCONNECT_REQUEST = "simap_disconnect_request";

	//private int mDisconnectMode;
	private int GRACEFUL_DISC_MODE = 0;
	private int IMMEDIATE_DISC_MODE = 1;


    @Override
    public void onCreate() {
    	Log.i(TAG, "SIMAP: onCreate...");

		if (mServiceInitiated)
		{
			Log.i(TAG, "Already initiated, just return!");
		}
		
        mInterrupted = false;
        mState = BluetoothSimap.STATE_IDLE;
        mAdapter = BluetoothAdapter.getDefaultAdapter();

		mServiceInitiated = initServiceNative();
		Log.v(TAG, "Service initiated: " + mServiceInitiated);

        IntentFilter filter = new IntentFilter(
                BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothSimapService.ACCESS_DISALLOWED_ACTION);
        filter.addAction(BluetoothSimapService.ACCESS_ALLOWED_ACTION);
        filter.addAction(BluetoothSimapService.DISCONNECT_REQUEST);
        filter.addAction(BluetoothSimapService.SEND_SIMUNAVALIBLE_IND);

        filter.addAction(BluetoothDevice.ACTION_NAME_CHANGED);
		//filter.addAction(BluetoothProfileManager.ACTION_DISABLE_PROFILES);
		
		registerReceiver(mBluetoothReceiver, filter);

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.v(TAG, "onStartCommand [+]");

		if (mHasStarted)
		{
			Log.i(TAG, "Already started, just return!");
			return START_STICKY;
		}

		if (mServiceInitiated) {
			
			if (mSocketListener == null) {
			mSocketListener = new SimapSocketListener();
			mSocketListener.setName("SimapSocketListener");
			mSocketListener.start();
			Log.i(TAG, "SimapSocketListener started.");
			}	
			
		    /* Check if need to eanble server in advance */
		    mPreferences = getSharedPreferences(KEY_SIMAP_SETTINGS, 0);
			savedEnableState = mPreferences.getBoolean(KEY_SIMAP_ENABLE, false);
		    if (savedEnableState) {
				log("Pre-enable SIMAP Server...");
				enable();
		    }

			if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
				mSIMIndex= mPreferences.getInt(KEY_SIMAP_SIM_INDEX,
							BluetoothSimap.BT_SIMAP_CARD1);
				Log.i(TAG, "SIM card: " + mSIMIndex);
				if(!isSimExist(mSIMIndex))
				{
					mSIMIndex = getAvailSimId();
					storeSettings(savedEnableState);
				}
				selectSIMNative(mSIMIndex);
				
			}
		
			mStartId = startId;

			mHasStarted = true;
			
		} else {
			log("Failed to init BluetoothSimapService. Stop SIMAP service.");
			stopSelf();
		}
		
        Log.v(TAG, "onStartCommand [-]");
	        
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        if (VERBOSE) Log.v(TAG, "onDestroy...");

		//disable the service
		disable();

        if (mWakeLock != null) {
            mWakeLock.release();
            mWakeLock = null;
        }

        closeSimapService();

		unregisterReceiver(mBluetoothReceiver);
		
		mSimapCallback.kill();

        mHasStarted = false;
		mStartId = -1;
		mServiceInitiated = false;
    }

    @Override
    public IBinder onBind(Intent intent) {
		Log.i(TAG, "Enter onBind(): mBinder=" + mBinder);
        
        return mBinder;
    }
	
    private final void closeSimapService() {
        if (VERBOSE) Log.v(TAG, "Simap Service closeSimapService");
        
        if (stopSelfResult(mStartId)) {
            if (VERBOSE) Log.v(TAG, "successfully stopped simap service");
        }

		stopListenNative();

		if (mSocketListener != null) {
		    try {
				mSocketListener.join();
				mSocketListener = null;
		    } catch (InterruptedException e) {
				Log.e(TAG, "mSocketListener close error.");
		    }
		}
		
		cleanupNativeDataNative();		
    }

	private boolean isSimExist(int slotId){
		TelephonyManagerEx tm = TelephonyManagerEx.getDefault();
		int simState = tm.getSimState(slotId);
		
		if(simState == TelephonyManager.SIM_STATE_ABSENT)
			return false;
		else
			return true;
	}
	
	private int getAvailSimId()
	{
		int i= 0;
		
		for(i = 0; i < MAX_SIM_NUM; i++){
			if(isSimExist(i) == true)
				return (i+1);
		}
		return 1;
	}

    private final Handler mSessionStatusHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
        
	        Intent intent;

			if (VERBOSE) Log.v(TAG, "Handler(): got msg = " + msg.what);
			
            switch (msg.what) {
					
                // SimapServer inform authorization indication
                case SIMAP_AUTHORIZE_IND:
                    Log.i(TAG, "= SIMAP_AUTHORIZE_IND =");

					
                    mRemoteDevice = (BluetoothDevice)msg.obj;
                    if (mRemoteDevice == null) {
                        Log.i(TAG, "mRemoteDevice = null");
                    }
                    sRemoteDeviceName = mRemoteDevice.getName();
                    // In case getRemoteName failed and return null
                    if (TextUtils.isEmpty(sRemoteDeviceName)) {
                        Log.i(TAG, "mRemoteDevice.getName()return empty, use the default name");
                        sRemoteDeviceName = getString(R.string.simap_defaultname);
                    }
                    boolean trust = mRemoteDevice.getTrustState();
                    if (VERBOSE) Log.v(TAG, "GetTrustState() = " + trust);

                    if (trust) {
                        
                        if (VERBOSE) Log.v(TAG, "incomming connection accepted from: "
                    	        + sRemoteDeviceName + " automatically as trusted device");

                        authorizeRsp(true);
							
                    } else {
                    
                        createSimapAuthNotification(true);
						
                        if (VERBOSE) Log.v(TAG, "incomming connection accepted from: "
                                + sRemoteDeviceName);

                        // In case car kit time out and try to use SIM
                        // access, while UI still there waiting for user to
                        // confirm
                        mSessionStatusHandler.sendMessageDelayed(mSessionStatusHandler
                                .obtainMessage(USER_TIMEOUT), USER_CONFIRM_TIMEOUT_VALUE);
                    }
                    break;
					
				case USER_TIMEOUT:
					
					Log.i(TAG, "= USER_TIMEOUT =");
					authorizeRsp(false);
					intent = new Intent(USER_CONFIRM_TIMEOUT_ACTION);
					sendBroadcast(intent);
				
					removeSimapAuthNotification(NOTIFICATION_ID_ACCESS);

					break;

				case SIMAP_CONNECTED:
					
					Log.i(TAG, "=== SIMAP_CONNECTED ===");
					createSimapConnNotification();
					
					intent = new Intent(BTSIMAP_CONNECTED);
					sendBroadcast(intent);
					
					break;
					
				case SIMAP_DISCONNECTED:
					Log.i(TAG, "=== SIMAP_DISCONNECTED ===");
				
					/* dismiss the dialog or clear the notification */
					intent = new Intent(BTSIMAP_DISCONNECTED);
					sendBroadcast(intent);
				
					//Cancel access request notification
					removeSimapAuthNotification(NOTIFICATION_ID_ACCESS);
					mSessionStatusHandler.removeMessages(USER_TIMEOUT);
				
					removeSimapConnNotification(NOTIFICATION_ID_CONNECTED);

					break;

					
                default:
                    break;
            }
        }
    };

    private void setState(int state) {
		if (DEBUG) Log.d(TAG, "setState(state): Simap state " + mState + " -> " + state);
        setState(state, RESULT_SUCCESS);
    }

    private synchronized void setState(int state, int result) {
        if (state != mState) {

			if (DEBUG) Log.d(TAG, "Simap state " + mState + " -> " + state + ", result = "
					+ result);

			if (state == BluetoothSimap.STATE_ENABLING
				|| mState == BluetoothSimap.STATE_ENABLING
				|| mState == BluetoothSimap.STATE_DISABLING
				)
			{
				Intent bpm_intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
				bpm_intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, BluetoothProfileManager.Profile.Bluetooth_SIMAP);
				int bpm_state;

				if (state == BluetoothSimap.STATE_ENABLING) {
					
					bpm_state = BluetoothProfileManager.STATE_ENABLING;					
					
				} else if (mState == BluetoothSimap.STATE_ENABLING) {
				    if (state == BluetoothSimap.STATE_ENABLED) {
				    	bpm_state = BluetoothProfileManager.STATE_ENABLED;
				    }
					else {
				    	bpm_state = BluetoothProfileManager.STATE_ABNORMAL;
					}
				} else {
				    if (state == BluetoothSimap.STATE_IDLE) {
				    	bpm_state = BluetoothProfileManager.STATE_DISABLED;
				    }
					else {
				    	bpm_state = BluetoothProfileManager.STATE_ABNORMAL;
					}
				}
				bpm_intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, bpm_state);
				
				Log.i(TAG, "send broadcast: simap Enabling/Disabling status: bpm_state = " + bpm_state);
				sendBroadcast(bpm_intent, BLUETOOTH_PERM);
			}

			if (state == BluetoothSimap.STATE_CONNECTED 
				|| state == BluetoothSimap.STATE_DISCONNECTED
				|| state == BluetoothSimap.STATE_DISCONNECTING)
			{
            Intent intent = new Intent(BluetoothSimap.SIMAP_STATE_CHANGED_ACTION);
            intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, BluetoothProfileManager.Profile.Bluetooth_SIMAP);
            intent.putExtra(BluetoothSimap.SIMAP_PREVIOUS_STATE, mState);
            mState = state;
            intent.putExtra(BluetoothSimap.SIMAP_STATE, mState);
            intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mRemoteDevice);
			Log.i(TAG, "send broadcast: simap state changed");
            sendBroadcast(intent, BLUETOOTH_PERM);
        }
			else
			{
				mState = state;
			}
        }
    }

    private void createSimapAuthNotification(boolean createNew) {
        Context context = getApplicationContext();

        NotificationManager nm = (NotificationManager)context
                .getSystemService(Context.NOTIFICATION_SERVICE);

        // Create an intent triggered by clicking on the status icon.
        Intent clickIntent = new Intent();
        clickIntent.setClass(context, BluetoothSimapActivity.class);
        clickIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        clickIntent.setAction(ACCESS_REQUEST_ACTION);

        // Create an intent triggered by clicking on the
        // "Clear All Notifications" button
        Intent deleteIntent = new Intent(ACTION_CLEAR_AUTH_NOTIFICATION);
        deleteIntent.setClass(context, BluetoothSimapReceiver.class);

        Notification notification = null;
        String name = getRemoteDeviceName();

            notification = new Notification(android.R.drawable.stat_sys_data_bluetooth, context
                    .getString(R.string.simap_remote_request), System.currentTimeMillis());
            notification.setLatestEventInfo(context, context.getString(R.string.simap_remote_request),
                    context.getString(R.string.simap_request_notif_message, name), PendingIntent
                            .getActivity(context, 0, clickIntent, 0));

            //notification.flags |= Notification.FLAG_AUTO_CANCEL;
			notification.flags |= Notification.FLAG_NO_CLEAR;
			if (createNew)
			{
                notification.flags |= Notification.FLAG_ONLY_ALERT_ONCE;
                notification.defaults = Notification.DEFAULT_SOUND;
            }
            notification.deleteIntent = PendingIntent.getBroadcast(context, 0, deleteIntent, 0);
            nm.notify(NOTIFICATION_ID_ACCESS, notification);
        }

    private void removeSimapAuthNotification(int id) {
        Context context = getApplicationContext();
        NotificationManager nm = (NotificationManager)context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(id);
    }

    private void createSimapConnNotification() {
        Context context = getApplicationContext();
        String title;

		Log.i(TAG, "createSimapConnNotification...");
		
        NotificationManager nm = (NotificationManager)context
                .getSystemService(Context.NOTIFICATION_SERVICE);

        // Create an intent triggered by clicking on the status icon.
        Intent clickIntent = new Intent();
        clickIntent.setClass(context, BluetoothSimapConnNotification.class);
        clickIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        clickIntent.setAction(CONNECTED_NOTIFY_ACTION);

        // Create an intent triggered by clicking on the
        // "Clear All Notifications" button
        Intent deleteIntent = new Intent(ACTION_CLEAR_CONN_NOTIFICATION);
        deleteIntent.setClass(context, BluetoothSimapReceiver.class);

        Notification notification = null;
        String name = getRemoteDeviceName();

		title = context.getString(R.string.simap_connected_notify_title, name);

        notification = new Notification(android.R.drawable.stat_sys_data_bluetooth, context
                .getString(R.string.simap_connected_notify_ticker), System.currentTimeMillis());
        notification.setLatestEventInfo(context, title,
                context.getString(R.string.simap_connected_notify_message, name), PendingIntent
                        .getActivity(context, 0, clickIntent, 0));

        //notification.flags |= Notification.FLAG_ONLY_ALERT_ONCE;
        //notification.defaults = Notification.DEFAULT_SOUND;
        notification.flags |= Notification.FLAG_NO_CLEAR;
        notification.deleteIntent = PendingIntent.getBroadcast(context, 0, deleteIntent, 0);
        nm.notify(NOTIFICATION_ID_CONNECTED, notification);
    }

    private void removeSimapConnNotification(int id) {
        Context context = getApplicationContext();
        NotificationManager nm = (NotificationManager)context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(id);
    }


    public static String getRemoteDeviceName() {
        return sRemoteDeviceName;
    }

	private void storeSettings(boolean enabled) {
		/* Store server settings into shared preferences. */
    	Log.i(TAG, "storeSettings: enabled=" + enabled +", mSIMIndex=" + mSIMIndex);
		if (mPreferences != null)
		{
			mPreferences.edit()
				.putBoolean(KEY_SIMAP_ENABLE, enabled)
				.commit();	
			
			if (FeatureOption.MTK_GEMINI_SUPPORT == true) { 	
				mPreferences.edit()
					.putInt(KEY_SIMAP_SIM_INDEX, mSIMIndex)
					.commit();					
			}
		}
	}
	
	public boolean enable ()
	{
        Log.i(TAG, "enable, mState= " + String.valueOf(mState));
		/*
		if (mState != BluetoothSimap.STATE_IDLE)
		{
			Log.e(TAG, "enable: mState is NOTE STATE_IDLE, just return!");
			return false; 
		}
		*/
		
		setState(BluetoothSimap.STATE_ENABLING);
        if(!enableNative())
        {
			setState(BluetoothSimap.STATE_IDLE);
            return false;
        }

        return true;
	}


    public void disable()
    {
        Log.i(TAG, "disable, mState= " + String.valueOf(mState));

		if (mState == BluetoothSimap.STATE_IDLE)
		{
			Log.e(TAG, "disable: mState is already STATE_IDLE, just return!");
			return; 
		}

		// if authorizing, reject, remove notification
		if (mState == BluetoothSimap.STATE_AUTHORIZING) {

			authorizeRsp(false);
			
			removeSimapAuthNotification(NOTIFICATION_ID_ACCESS);
			
		}
		else if (mState == BluetoothSimap.STATE_CONNECTED)
		{
			// disconnectClient the connection
			disconnectClient(IMMEDIATE_DISC_MODE);
		}
	
		setState(BluetoothSimap.STATE_DISABLING);
        disableNative();
    }

	private void authorizeRsp(boolean accept)
	{
		Log.i(TAG, "authorizeRsp: accept=" + accept + ", mState="+mState);

		mSessionStatusHandler.removeMessages(USER_TIMEOUT); 

		if (mState != BluetoothSimap.STATE_AUTHORIZING)
		{
			Log.i(TAG, "mState!=STATE_AUTHORZING, ignore the authorizeRsp request");
			return;
		}

		setState(BluetoothSimap.STATE_ENABLED);

		authorizeRspNative(accept);
	}

	private void disconnectClient(int discMode)
	{
		Log.i(TAG, "disconnectClient...discMode=" + discMode);
		setState(BluetoothSimap.STATE_DISCONNECTING);
		disconnectNative(discMode);
	}

	private void sendSIMUnaccessibleInd() 
	{	
		Log.i(TAG, "sendSIMUnaccessibleInd...mState="+mState);
		if (mState != BluetoothSimap.STATE_CONNECTED)
		{
			Log.i(TAG, "mState != BluetoothSimap.STATE_CONNECTED, just return");
			return;
		}
		
		sendSIMUnaccessibleIndNative();		
	}


    /**
     * Handlers for incoming service calls
     */
    private final IBluetoothSimap.Stub mBinder = new IBluetoothSimap.Stub() {
	
        public boolean enableService() {
            if (DEBUG) Log.d(TAG, "turn On SIMAP... ");
			enableAction = true;
            enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");

			enable();

			return true;
        }

        public void disableService() {
            if (DEBUG) Log.d(TAG, "turn Off SIMAP... ");
			enableAction = true;
            enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
			
			disable();
        }

        public int getState() {
            if (DEBUG) Log.d(TAG, "getState " + mState);

            enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
            return mState;
        }

        public BluetoothDevice getClient() {
            if (DEBUG) Log.d(TAG, "getClient" + mRemoteDevice);

            enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
            if (mState == BluetoothSimap.STATE_ENABLED) {
                return null;
            }
            return mRemoteDevice;
        }

        public boolean isConnected(BluetoothDevice device) {
            enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
            return mState == BluetoothSimap.STATE_CONNECTED && mRemoteDevice.equals(device);
        }

        public boolean connect(BluetoothDevice device) {
            enforceCallingOrSelfPermission(BLUETOOTH_ADMIN_PERM,
                    "Need BLUETOOTH_ADMIN permission");
            return false;
        }

        public void disconnect() {
            if (DEBUG) Log.d(TAG, "disconnectClient");

            enforceCallingOrSelfPermission(BLUETOOTH_ADMIN_PERM,
                    "Need BLUETOOTH_ADMIN permission");
            synchronized (BluetoothSimapService.this) {
                switch (mState) {
                    case BluetoothSimap.STATE_CONNECTED:
                        // send disconnectClient request
                        disconnectClient(IMMEDIATE_DISC_MODE);
                        break;
                    default:
                        break;
                }
            }
        }

		public boolean selectSIM(int simIndex) {
			if (DEBUG) Log.d(TAG, "select SIM:  " + simIndex);
            enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");

			selectSIMNative(simIndex);

			mSIMIndex = simIndex;

			return true;
		}

	public int getSIMIndex() {
		if (DEBUG) Log.d(TAG, "getSIMIndex return:	" + mSIMIndex);

		return mSIMIndex;
		}


	public void registerCallback(IBluetoothSimapCallback cb) {
		if (DEBUG) Log.d(TAG, "+registerCallback: " + cb);
	    if (cb != null) 
			mSimapCallback.register(cb);
	}

	public void unregisterCallback(IBluetoothSimapCallback cb) {
		if (DEBUG) Log.d(TAG, "-unregisterCallback: " + cb);
	    if (cb != null) 
			mSimapCallback.unregister(cb);
	}

	public boolean isServiceStarted() {
		if (DEBUG) Log.d(TAG, "isServiceStarted return:	" + mHasStarted);
		return mHasStarted;
	}

	public void startSimapService() {
		if (DEBUG) Log.d(TAG, "startSimapService ...");
        Context context = getApplicationContext();
		context.startService(new Intent(context, BluetoothSimapService.class));		
	}
		
    };

    private final RemoteCallbackList<IBluetoothSimapCallback> mSimapCallback
	= new RemoteCallbackList<IBluetoothSimapCallback>();


    private class SimapSocketListener extends Thread {
	private boolean init_ok = false;

	public SimapSocketListener() {
	    init_ok = prepareListentoSocketNative();
	    Log.d(TAG, "After preparing, init_ok: " + init_ok);
	}
		
	@Override
	public void run() {
	    boolean job_done = false;
	
		if (init_ok) {
			job_done = startListenNative();
	    }
	    Log.d(TAG, "SocketListener exited. job_done: " + job_done);
	}

	public void shutdown(){
	    Log.d(TAG, "SimapSocketListener shutdown.");
		
		stopListenNative();
	}
	
    }

    /** For disabling all cabilities of SIMAP service. This function may be called 
     *  when BT is turning off. Under this situation, SIMAP server and client should 
     *  disconnectClient its connection, and change back to IDLE state. SocketListener 
     *  should also be shut down here.
     */
    private void clearService() {
	Log.d(TAG, "clearService(), mState = " + String.valueOf(mState));

	//boolean serverEnabled = (mState != BluetoothSimap.STATE_IDLE);
	boolean timeout = false;
	int cnt = 0;

	mHasStarted = false;

	/* Disable SIMAP server. */
	if (mState != BluetoothSimap.STATE_IDLE) {

		//disable the server 
	    disable();

		//wait the disable cnf
	    while (mState != BluetoothSimap.STATE_IDLE) {
		if (cnt >= 2000) {
		    timeout = true;
		    break;
		}

		try {
		    Thread.sleep(100);
		} catch (Exception e) {
		    Log.e(TAG, "Waiting for server deregister-cnf was interrupted.");
		}
		cnt += 100;
	    }
	}

	if (timeout) {
	    /* WARNNING: 
	     *     If we are here, BT task may be crashed or too busy. So we skip waiting
	     *  DEREGISTER_SERVER_CNF and just clear server context.
	     */
	    Log.w(TAG, "Waiting DEREGISTER_SERVER_CNF time-out. Force clear server context.");
	    mState = BluetoothSimap.STATE_IDLE;
	}

	/* Shut SocketListener down */
	if (mSocketListener != null) {
	    try {
		mSocketListener.shutdown();
		mSocketListener.join();
		mSocketListener = null;
		Log.d(TAG, "mSocketListener finish shutdown!");
	    } catch (InterruptedException e) {
		Log.e(TAG, "mSocketListener close error.");
	    }
	}

	/* Store server settings into shared preferences. */
	Log.i(TAG, "clearService call storeSettings() ");
	storeSettings(savedEnableState);

    }
	

    /* BroadcastReceiver for receiving BT Power-off */
    private BroadcastReceiver mBluetoothReceiver = new BroadcastReceiver() {
    
	public void onReceive(Context context, Intent intent) {

		String action = intent.getAction();
		BluetoothDevice device =
				intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
		log("mBluetoothReceiver:[Intent] action="+action+", state="+String.valueOf(mState));
		
		if ((mState == BluetoothSimap.STATE_CONNECTED) &&
				action.equals(BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED) &&
				device != null && mRemoteDevice != null &&
				device.equals(mRemoteDevice)) {
			try {
				mBinder.disconnect();
			} catch (RemoteException e) {}
		} else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {

			switch (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
									   BluetoothAdapter.ERROR)) {
			case BluetoothAdapter.STATE_TURNING_OFF:
				clearService();
				break;
			}
		} else if (action.equals(BluetoothSimapService.ACCESS_ALLOWED_ACTION))
		{
			Log.i(TAG, "ACCESS_ALLOWED_ACTION");
            if (intent.getBooleanExtra(EXTRA_ALWAYS_ALLOWED, false)) {
                boolean result = mRemoteDevice.setTrust(true);
                if (VERBOSE) Log.v(TAG, "setTrust() result=" + result);
            }
			
			removeSimapAuthNotification(NOTIFICATION_ID_ACCESS);			
            authorizeRsp(true);

		} else if (action.equals(BluetoothSimapService.ACCESS_DISALLOWED_ACTION))
		{
			removeSimapAuthNotification(NOTIFICATION_ID_ACCESS);			
            authorizeRsp(false);
		} else if (action.equals(BluetoothSimapService.DISCONNECT_REQUEST))
		{
			/* disconnectClient request from user */
			disconnectClient(GRACEFUL_DISC_MODE);
		} else if (action.equals(BluetoothSimapService.SEND_SIMUNAVALIBLE_IND))
		{
			/* send SIM not avalible IND  */
			sendSIMUnaccessibleInd();
		} else if (action.equals(BluetoothDevice.ACTION_NAME_CHANGED)) {
			if (mRemoteDevice != null && device != null && mRemoteDevice.equals(device))
			{
				Log.i(TAG, "mRemoteDevice name changed!");
				updateDeviceName();
			}
		}
		
	}
    };

	private void updateDeviceName()
	{
		sRemoteDeviceName = mRemoteDevice.getName();
		// In case getRemoteName failed and return null
		if (TextUtils.isEmpty(sRemoteDeviceName)) {
			Log.i(TAG, "mRemoteDevice.getName()return empty, use the default name");
			sRemoteDeviceName = getString(R.string.simap_defaultname);
		}
		Log.i(TAG, "updateDeviceName: sRemoteDeviceName = " + sRemoteDeviceName + ", mState= " + mState);

		if (mState == BluetoothSimap.STATE_AUTHORIZING) {
			//update authorizing notification
			createSimapAuthNotification(false); 
		} else if (mState == BluetoothSimap.STATE_CONNECTED) {
			//update connected notification
			createSimapConnNotification();			
		}
	}
	
    /* A thread that keep listening to the socket for incoming ILM */
    private SimapSocketListener mSocketListener = null;

    static {
	System.loadLibrary("extsimap_jni");
    }

    private native static boolean classInitNative();
    private native boolean initServiceNative();
    private native void cleanupNativeDataNative();
    private native boolean enableNative();
    private native void disableNative();
    private native void disconnectNative(int discMode);
    private native void authorizeRspNative(boolean accept);
    private native boolean selectSIMNative(int simIndex);

    private native boolean startListenNative();
    private native void stopListenNative();
    private native boolean prepareListentoSocketNative();
		
	private native void sendSIMUnaccessibleIndNative();

    /* Callback for enable result */
    private void onEnableCnf(boolean succ) {
    	Log.i(TAG, "Enter onEnableCnf: " + succ);
		int msgid;

		//if (succ){
			msgid = BluetoothSimap.BT_SIMAPUI_READY;
			setState(BluetoothSimap.STATE_ENABLED);
			Log.i(TAG, "enableAction: " + enableAction);

			if(enableAction == true){
				savedEnableState = true;
				enableAction = false;
			}

			storeSettings(true);
		/*
		}
		else {
			msgid = BluetoothSimap.BT_SIMAPUI_DISABLED;
			setState(BluetoothSimap.STATE_IDLE);
		}
		*/

		if (mSimapCallback!= null) {
		    final int N = mSimapCallback.beginBroadcast();
		    log("Start broadcasting to callback. N=" + N);

		    for (int i=0; i<N; i++) {
			try {
			    mSimapCallback.getBroadcastItem(i).postEvent(
				    msgid, null);
			} catch (RemoteException e) {
			    // do nothing.
			}
		    }

		    mSimapCallback.finishBroadcast();
		    log("End broadcasting to callback.");

		} else {
		    log("For non-ui server enabling.");
		}
    }

    /* Callback for disable result */
    private void onDisableCnf(boolean succ) {
    	Log.i(TAG, "Enter onDisableCnf: " + succ);
		int msgid;
		
		//if (succ){
			setState(BluetoothSimap.STATE_IDLE);
			msgid = BluetoothSimap.BT_SIMAPUI_DISABLED;

			Log.i(TAG, "onDisableCnf call storeSettings(false) enableAction = "+enableAction);
			
			if(enableAction == true){
				savedEnableState = false;
				enableAction = false;
			}
			storeSettings(false);
		/*		
		}
		else {
			setState(BluetoothSimap.STATE_ENABLED);
			msgid = BluetoothSimap.BT_SIMAPUI_READY;
		}
		*/

		if (mSimapCallback!= null) {
		    final int N = mSimapCallback.beginBroadcast();
		    log("Start broadcasting to callback. N=" + N);

		    for (int i=0; i<N; i++) {
			try {
			    mSimapCallback.getBroadcastItem(i).postEvent(
				    msgid, null);
			} catch (RemoteException e) {
			    // do nothing.
			}
		    }

		    mSimapCallback.finishBroadcast();
		    log("End broadcasting to callback.");

		} else {
		    log("For non-ui server enabling.");
		}
    }

    /* Callback for authorize indication */
    private boolean onAuthorizeInd(String addr) {
		Log.i(TAG, "Enter onAuthorizeInd(): " + addr);
		Object obj = null;

		
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
		if (adapter == null){
			Log.e(TAG, "adapter == null");
		    authorizeRspNative(false);
			return false;
		}
		
        obj = adapter.getRemoteDevice(addr);
		if (obj == null){
			Log.e(TAG, "obj == null");
		    authorizeRspNative(false);
			return false;
		}

		setState(BluetoothSimap.STATE_AUTHORIZING);
		
		sendServiceMsg(SIMAP_AUTHORIZE_IND, obj);

		return true;

    }

    /* Callback for connected indication */
    private void onConnectedInd(String deviceName) {
		Log.i(TAG, "Enter onConnectedInd()");

		setState(BluetoothSimap.STATE_CONNECTED);

		sendServiceMsg(SIMAP_CONNECTED, null);
    }

    /* Callback for disconnect indication */
    private void onDisconnectedInd() {
		Log.i(TAG, "Enter onDisconnectedInd()");

		setState(BluetoothSimap.STATE_ENABLED);

		sendServiceMsg(SIMAP_DISCONNECTED, null);
    }

    /* Callback for disconnect indication */
    private void onDisconnectCnf(boolean succ) {
		Log.i(TAG, "Enter onDisconnectCnf(): " + succ);
		//if (succ){
			setState(BluetoothSimap.STATE_ENABLED);
			sendServiceMsg(SIMAP_DISCONNECTED, null);
		//}
		//else {
		//	setState(BluetoothSimap.STATE_CONNECTED);
		//}
	}

	private void onBtResetInd() {
		Log.i(TAG, "Enter onBtResetInd() [+]");
		
		//check the state
		if (mState == BluetoothSimap.STATE_IDLE)
		{
			Log.e(TAG, "disable: mState is already STATE_IDLE, just return!");
			return; 
		}
		
		if (mState == BluetoothSimap.STATE_AUTHORIZING) {			
			// if authorizing, remove notification
			removeSimapAuthNotification(NOTIFICATION_ID_ACCESS);			
			mSessionStatusHandler.removeMessages(USER_TIMEOUT);
		}
		else if (mState == BluetoothSimap.STATE_CONNECTED)
		{
			setState(BluetoothSimap.STATE_DISCONNECTED);
			//Cancel access connected notification
			removeSimapConnNotification(NOTIFICATION_ID_CONNECTED);
		}
		
		setState(BluetoothSimap.STATE_IDLE);
		
		int msgid;
		msgid = BluetoothSimap.BT_SIMAPUI_DISABLED;
		if (mSimapCallback!= null) {
		    final int N = mSimapCallback.beginBroadcast();
		    log("Start broadcasting to callback. N=" + N);

		    for (int i=0; i<N; i++) {
			try {
			    mSimapCallback.getBroadcastItem(i).postEvent(
				    msgid, null);
			} catch (RemoteException e) {
			    // do nothing.
		}
    }

		    mSimapCallback.finishBroadcast();
		    log("End broadcasting to callback.");
		}
		Log.i(TAG, "onBtResetInd() [-] ");
	}

    /* Callback for BT SIM-AP Command/Request from Native Layer */
    private AfAdapterResult onBtSimapCmd(int fnCode, int iarg, String sarg) {
        AfAdapterResult result = new AfAdapterResult();
        result.onCommand(fnCode, iarg, sarg);
        return result;
    }


    /* Utility function: sendServiceMsg */
    private void sendServiceMsg(int what, Object obj) {
    	Log.i(TAG, "sendServiceMsg: " + what + ", "+ obj);
		Message msg = Message.obtain();
		msg.what = what;
		msg.obj = obj;

		mSessionStatusHandler.sendMessage(msg);
    }

	private static void log(String msg) {
		Log.d(TAG, msg);
	}

	
}


