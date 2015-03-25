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

package android.bluetooth;

import android.content.ComponentName;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.IBinder;
import android.util.Log;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.IBluetoothMap;
import java.util.HashSet;
import java.util.Set;
import java.util.Arrays;
import java.util.Collections;




public class BluetoothMap implements BluetoothProfileManager.BluetoothProfileBehavior{
    private static final String TAG = "BluetoothMap";
	
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

	/* state */
  	//service state
	public final static int STATE_ENABLING 				= BluetoothProfileManager.STATE_ENABLING;
    public final static int STATE_ENABLED				    = BluetoothProfileManager.STATE_ENABLED;
	public final static int STATE_DISABLING				= BluetoothProfileManager.STATE_DISABLING;
	public final static int STATE_DISABLED				= BluetoothProfileManager.STATE_DISABLED;
	public final static int STATE_ABNORMAL				= BluetoothProfileManager.STATE_ABNORMAL;

	//device state
	public final static int STATE_CONNECTED				= BluetoothProfileManager.STATE_CONNECTED;
	public final static int STATE_DISCONNECTING			= BluetoothProfileManager.STATE_DISCONNECTING;
	public final static int STATE_DISCONNECTED			= BluetoothProfileManager.STATE_DISCONNECTED;
	public final static int STATE_UNKNOWN					= BluetoothProfileManager.STATE_UNKNOWN;
	//define an extra state for authorizing
	public final static int STATE_AUTHORIZING			= STATE_UNKNOWN + 1;
	public final static int STATE_NONE					= STATE_UNKNOWN + 2;	

	public final static String ACTION_AUTHORIZE_RESULT = 
							"com.mediatek.bluetooth.map.BluetoothMapService.action.AUTHORIZE_RESULT";
	public final static String ACTION_DISCONNECT_DEVICE = 
							"com.mediatek.bluetooth.map.BluetoothMapService.action.DISCONNECT_DEVICE";
	public final static String EXTRA_RESULT = 
							"com.mediatek.bluetooth.map.BluetoothMapService.extra.RESULT";
	


    /* SIM index */
    public final static int BLUETOOTH_SIM_CARD1		    = 1;
    public final static int BLUETOOTH_SIM_CARD2		    = 2;

    private IBluetoothMap mService;

    private Context mContext;
	private ServiceListener mListener;


    /**
     * An interface for notifying Bluetooth PCE IPC clients when they have
     * been connected to the BluetoothSimap service.
     */
    public interface ServiceListener {
        /**
         * Called to notify the client when this proxy object has been
         * connected to the BluetoothSimap service. Clients must wait for
         * this callback before making IPC calls on the BluetoothSimap
         * service.
         */
        public void onServiceConnected();

        /**
         * Called to notify the client that this proxy object has been
         * disconnected from the BluetoothSimap service. Clients must not
         * make IPC calls on the BluetoothSimap service after this callback.
         * This callback will currently only occur if the application hosting
         * the BluetoothSimap service, but may be called more often in future.
         */
        public void onServiceDisconnected();
    }

	public BluetoothMap(Context context, ServiceListener l)
	{
		mContext = context;
        mListener = l;

		if (!context.bindService(new Intent(IBluetoothMap.class.getName()), mConnection, 0)) {
			Log.e(TAG, "Could not bind to Bluetooth Map Service");
		}
		
	}

	public BluetoothMap(Context context)
	{
		mContext = context;
        mListener = null;

		if (!context.bindService(new Intent(IBluetoothMap.class.getName()), mConnection, 0)) {
			Log.e(TAG, "Could not bind to Bluetooth Map Service");
		}
		
	}

    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

   	public synchronized void close() {
        if (mConnection != null) {
            mContext.unbindService(mConnection);
            mConnection = null;
        }
    }

	//APIs below will not exposed to all application
/* 
   public void enableServer()
    {
        log("enableService");
        if (mService != null) {
            try {
				mService.enableServer();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            log(Log.getStackTraceString(new Throwable()));
        }
    }

    public void disableServer()
    {
        Log.i(TAG, "disableService");
        if (mService != null) {
            try {
                 mService.disableServer();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
             log(Log.getStackTraceString(new Throwable()));
        }
    }

	public boolean registerSim(int value){
		 Log.i(TAG, "registerMsgRepository");
        if (mService != null) {
            try {
                 return mService.registerSim(value);
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
             log(Log.getStackTraceString(new Throwable()));
        }
		return true;
	}
	public void unregisterSim(int value) {

		Log.i(TAG, "unregisterMsgRepository");
        if (mService != null) {
            try {
                 mService.unregisterSim(value);
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
             log(Log.getStackTraceString(new Throwable()));
        }
	}
	public boolean replaceAccount(long value) {
		Log.i(TAG, "replaceEmailAccount");
        if (mService != null) {
            try {
               return  mService.replaceAccount(value);
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            log(Log.getStackTraceString(new Throwable()));
        }
		return false;				
	}
*/
    
   	public Set<BluetoothDevice> getConnectedDevices() {
        log("getConnectedDevices()");
		HashSet<BluetoothDevice> connSet = new HashSet<BluetoothDevice>();
		if (mService != null) {
			try {
            	return Collections.unmodifiableSet(new HashSet<BluetoothDevice>(Arrays.asList(mService.getConnectedDevices())));
        	} catch (RemoteException e) {
            	Log.e(TAG, "", e);
            	return null;
        	}
		} else {
			log("Proxy not attached to service");
		}
        return null;
    }

    /**
     * Returns true if the specified Bluetooth device is connected (does not
     * include connecting). Returns false if not connected, or if this proxy
     * object is not currently connected to the Simap service.
     */
    public boolean isConnected(BluetoothDevice device) {
        log("isConnected(" + device + ")");
        if (mService != null) {
            try {
                return mService.isConnected(device);
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
             log(Log.getStackTraceString(new Throwable()));
        }
        return false;
    }   

    /**
     * Disconnects the current Simap client
     */
    public boolean disconnect(BluetoothDevice device) {
        log("disconnect(device): " + device);
		if (isConnected(device))
		{				
	        if (mService != null) {
	            try {
	                mService.disconnect(device);
	                return true;
	            } catch (RemoteException e) {Log.e(TAG, e.toString());}
	        } else {
	            Log.w(TAG, "Proxy not attached to service");
	            log(Log.getStackTraceString(new Throwable()));
	        }
		}
        return false;
    }

    public boolean connect(BluetoothDevice device) {
        log("connect(device), just return false");
		return false;
    }	
	
	public int getState(BluetoothDevice device) {
		int state = -1;
        log("getState(device) " + device);

		if (mService != null) {
	       try {
	            state = mService.getState(device);
	       } catch (RemoteException e) {Log.e(TAG, e.toString());}
	    } else {
	            Log.w(TAG, "Proxy not attached to service");
	            log(Log.getStackTraceString(new Throwable()));
	    }
		

		return state;
	}
		

	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			 log("Proxy object connected");
			mService = IBluetoothMap.Stub.asInterface(service);
			if (mListener != null) {
				mListener.onServiceConnected();
			}
		}
		public void onServiceDisconnected(ComponentName className) {
			 log("Proxy object disconnected");
			mService = null;
			if (mListener != null) {
				mListener.onServiceDisconnected();
			}
		}
	};


	private static void log(String msg) {
		if (DBG) {
			Log.d(TAG, msg);
		}
	}
	
};


