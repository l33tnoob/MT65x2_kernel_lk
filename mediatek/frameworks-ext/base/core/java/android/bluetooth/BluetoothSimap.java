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

/*

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

import java.util.HashSet;
import java.util.Set;


/**
 * Bluetooth SIMAP manager .
 */
public class BluetoothSimap implements BluetoothProfileManager.BluetoothProfileBehavior{
    private static final String TAG = "BT SIMAP";
	
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

	/* UI event */
    public final static int BT_SIMAPUI_EVENT_NONE	    = 0;
    public final static int BT_SIMAPUI_SHUTDOWNING	    = 1;  /* deinit Param: none */
    public final static int BT_SIMAPUI_READY		    = 2;  /* server register ok Param: none */
    public final static int BT_SIMAPUI_AUTHORIZING	    = 3;  /* server receive a incoming */
    public final static int BT_SIMAPUI_CONNECTING	    = 5;
    public final static int BT_SIMAPUI_CONNECTED	    = 6;  /* Param: rspcode */
    public final static int BT_SIMAPUI_ABORTED		    = 10;
    public final static int BT_SIMAPUI_DISCONNECTED	    = 11;
    public final static int BT_SIMAPUI_ERROR		    = 12;
    public final static int BT_SIMAPUI_DISABLED		    = 13; /* For Android */

	/* state */
    public static final int STATE_IDLE          = -1;
    public static final int STATE_AUTHORIZING   = BluetoothProfileManager.STATE_CONNECTING;
    public static final int STATE_CONNECTED     = BluetoothProfileManager.STATE_CONNECTED;
    public static final int STATE_DISCONNECTING = BluetoothProfileManager.STATE_DISCONNECTING;
    public static final int STATE_DISCONNECTED  = BluetoothProfileManager.STATE_DISCONNECTED;
    public static final int STATE_ENABLING    = BluetoothProfileManager.STATE_ENABLING;
    public static final int STATE_ENABLED     = STATE_DISCONNECTED; 
    public static final int STATE_DISABLING  = BluetoothProfileManager.STATE_DISABLING;

    /** int extra for SIMAP_STATE_CHANGED_ACTION */
    public static final String SIMAP_STATE =
		BluetoothProfileManager.EXTRA_NEW_STATE;
        //"android.bluetooth.simap.intent.SIMAP_STATE";

    /** int extra for SIMAP_STATE_CHANGED_ACTION */
    public static final String SIMAP_PREVIOUS_STATE =
		BluetoothProfileManager.EXTRA_PREVIOUS_STATE;
        //"android.bluetooth.simap.intent.SIMAP_PREVIOUS_STATE";

    /** Indicates the state of an simap connection state has changed.
     *  This intent will always contain SIMAP_STATE, SIMAP_PREVIOUS_STATE and
     *  BluetoothIntent.ADDRESS extras.
     */
    public static final String SIMAP_STATE_CHANGED_ACTION =
		BluetoothProfileManager.ACTION_STATE_CHANGED;
        //"android.bluetooth.simap.intent.action.SIMAP_STATE_CHANGED";


    /* SIM index */
    public final static int BT_SIMAP_CARD1		    = 1;
    public final static int BT_SIMAP_CARD2		    = 2;

	/**/
    private IBluetoothSimap mService;

	/**/
    private final ServiceListener mServiceListener;

    private Context mContext;


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

	public BluetoothSimap(Context context, ServiceListener l)
	{
		mContext = context;
        mServiceListener = l;

		if (!context.bindService(new Intent(IBluetoothSimap.class.getName()), mConnection, 0)) {
			Log.e(TAG, "Could not bind to Bluetooth Simap Service");
		}
		
	}

	public BluetoothSimap(Context context)
	{
		mContext = context;
        mServiceListener = null;

		if (!context.bindService(new Intent(IBluetoothSimap.class.getName()), mConnection, 0)) {
			Log.e(TAG, "Could not bind to Bluetooth Simap Service");
		}
		
	}

    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

    /**
     * Close the connection to the backing service.
     * Other public functions of BluetoothSimap will return default error
     * results once close() has been called. Multiple invocations of close()
     * are ok.
     */
    public synchronized void close() {
        if (mConnection != null) {
            mContext.unbindService(mConnection);
            mConnection = null;
        }
    }


    /**
     * Get the currently connected remote Bluetooth device (PCE).
     * @return The remote Bluetooth device, or null if not in connected or
     *         connecting state, or if this proxy object is not connected to
     *         the Simap service.
     */
    public BluetoothDevice getConnectedClient() {
        if (DBG) log("getConnectedClient()");
        if (mService != null) {
            try {
                return mService.getClient();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return null;
    }

	public Set<BluetoothDevice> getConnectedDevices() {
        if (DBG) log("getConnectedDevices()");
		HashSet<BluetoothDevice> connSet = new HashSet<BluetoothDevice>();

		BluetoothDevice connDev = getConnectedClient();
		if (connDev != null)
		{
			connSet.add(connDev);
		}
		
        return connSet;
    }

    /**
     * Returns true if the specified Bluetooth device is connected (does not
     * include connecting). Returns false if not connected, or if this proxy
     * object is not currently connected to the Simap service.
     */
    public boolean isConnected(BluetoothDevice device) {
        if (DBG) log("isConnected(" + device + ")");
        if (mService != null) {
            try {
                return mService.isConnected(device);
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return false;
    }

	
    public boolean enableService()
    {
        Log.i(TAG, "enableService");
        if (mService != null) {
            try {
                return mService.enableService();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return false;
    }

    public void disableService()
    {
        Log.i(TAG, "disableService");
        if (mService != null) {
            try {
                 mService.disableService();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return ;
    }

    public boolean selectSIM(int index)
    {
        Log.i(TAG, "selectSIM");
        if (mService != null) {
            try {
                 return mService.selectSIM(index);
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return false;
    }

	public int getSelectedSIMIndex()
	{
        Log.i(TAG, "getSelectedSIMIndex");
        if (mService != null) {
            try {
                 return mService.getSIMIndex();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return 0;
    }

    /**
     * Disconnects the current Simap client . Currently this call blocks,
     * it may soon be made asynchornous. Returns false if this proxy object is
     * not currently connected to the Simap service.
     */
    public boolean disconnectClient() {
        if (DBG) log("disconnectClient()");
        if (mService != null) {
            try {
                mService.disconnect();
                return true;
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }
        return false;
    }

    /**
     * Disconnects the current Simap client
     */
    public boolean disconnect(BluetoothDevice device) {
        if (DBG) log("disconnect(device): " + device);
		if (isConnected(device))
		{				
	        if (mService != null) {
	            try {
	                mService.disconnect();
	                return true;
	            } catch (RemoteException e) {Log.e(TAG, e.toString());}
	        } else {
	            Log.w(TAG, "Proxy not attached to service");
	            if (DBG) log(Log.getStackTraceString(new Throwable()));
	        }
		}
        return false;
    }

    public boolean connect(BluetoothDevice device) {
        if (DBG) log("connect(device), just return false");
		return false;
    }
	
	public int getState() {
		int state = -1;
        if (DBG) log("getState()");
        if (mService != null) {
            try {
                state = mService.getState();
            } catch (RemoteException e) {Log.e(TAG, e.toString());}
        } else {
            Log.w(TAG, "Proxy not attached to service");
            if (DBG) log(Log.getStackTraceString(new Throwable()));
        }

		return state;
	}
		
	public int getState(BluetoothDevice device) {
		int state = -1;
        if (DBG) log("getState(device) " + device);

		if (isConnected(device))
		{				
	        if (mService != null) {
	            try {
	                state = mService.getState();
	            } catch (RemoteException e) {Log.e(TAG, e.toString());}
	        } else {
	            Log.w(TAG, "Proxy not attached to service");
	            if (DBG) log(Log.getStackTraceString(new Throwable()));
	        }
		}

		return state;
	}
		

	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			if (DBG) log("Proxy object connected");
			mService = IBluetoothSimap.Stub.asInterface(service);
			if (mServiceListener != null) {
				mServiceListener.onServiceConnected();
			}
		}
		public void onServiceDisconnected(ComponentName className) {
			if (DBG) log("Proxy object disconnected");
			mService = null;
			if (mServiceListener != null) {
				mServiceListener.onServiceDisconnected();
			}
		}
	};


	private static void log(String msg) {
		Log.d(TAG, msg);
	}
	
};

