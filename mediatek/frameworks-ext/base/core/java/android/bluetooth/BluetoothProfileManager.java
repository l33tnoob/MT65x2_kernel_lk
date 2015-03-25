/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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


package android.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.server.BluetoothProfileManagerService;
import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.content.Context;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.os.IBinder;
import android.util.Log;

import java.util.Arrays;
import java.util.Collections;
import java.util.Set;
import java.util.HashSet;

/**
 * Public API for controlling the Bluetooth Profile manager Service.
 *
 * BluetoothProfileManager is a proxy object for controlling the Bluetooth profile manager
 * Service via IPC.
 *
 * Creating a BluetoothProfileManager object will initiate a binding with the
 * BluetoothProfileManagerService. Users of this object should call close() when they
 * are finished, so that this proxy object can unbind from the service.
 *
 * Currently the BluetoothProfileManager service runs in the system server and this
 * proxy object will be immediately bound to the service on construction.
 *
 *
 * @hide
 */
public final class BluetoothProfileManager {
    private static final String TAG = "BluetoothProfileManager";
    private static final boolean DBG = true;

	/** when a profile state is changed, Intent is broadcasted by relevant profile service
		to Indicate the state is changed. 
		* This intent will always contain EXTRA_PROFILE,
		* EXTRA_NEW_STATE, EXTRA_PREVIOUS_STATE and BluetoothDevice.EXTRA_DEVICE
		* extras.
		*/

	public static final String ACTION_PROFILE_STATE_UPDATE =
		"android.bluetooth.profilemanager.action.PROFILE_CHANGED";
	public static final String ACTION_DISABLE_PROFILES =
		"android.bluetooth.profilemanager.action.DISABLE_PROFILES";
	public static final String ACTION_ALL_PROFILES_DISABLED =
		"android.bluetooth.profilemanager.action.ALL_PROFILES_DISABLED";
	public static final String ACTION_STATE_CHANGED=
		"android.bluetooth.profilemanager.action.STATE_CHANGED";
	public static final String ACTION_UPDATE_NOTIFICATION=
		"android.bluetooth.profilemanager.action.UPDATE_ NOTIFICATION";
	public static final String ACTION_TIMEOUT=
		"android.bluetooth.profilemanager.action.TIMEOUT";
	public static final String EXTRA_PROFILE =
        "android.bluetooth.profilemanager.extra.PROFILE";
	/** int extra for ACTION_SINK_STATE_CHANGED */
    public static final String EXTRA_NEW_STATE =
        "android.bluetooth.profilemanager.extra.EXTRA_NEW_STATE";
    /** int extra for ACTION_SINK_STATE_CHANGED */
    public static final String EXTRA_PREVIOUS_STATE =
        "android.bluetooth.profilemanager.extra.EXTRA_PREVIOUS_STATE";   

    public static final int STATE_ACTIVE           = 0;
    public static final int STATE_CONNECTED        = 1;
    public static final int STATE_DISCONNECTED     = 2;
    public static final int STATE_CONNECTING       = 3;
    public static final int STATE_DISCONNECTING    = 4;
	public static final int STATE_UNKNOWN          = 5;

	public static final int STATE_ENABLING           = 10;
    public static final int STATE_ENABLED        = 11;
    public static final int STATE_DISABLING     = 12;
    public static final int STATE_DISABLED       = 13;
    public static final int STATE_ABNORMAL    = 14;

    private final IBluetoothProfileManager mService;
    private final Context mContext;

	public enum Profile{
         Bluetooth_HEADSET(0),
         Bluetooth_A2DP(1),
         Bluetooth_HID(2),
         Bluetooth_FTP_Client(3),
         Bluetooth_FTP_Server(4),
         Bluetooth_BIP_Initiator(5),
         Bluetooth_BIP_Responder(6),
         Bluetooth_BPP_Sender(7),
         Bluetooth_SIMAP(8),
         Bluetooth_PBAP(9),
         Bluetooth_OPP_Server(10),
		Bluetooth_OPP_Client(11),
		Bluetooth_DUN(12),
		Bluetooth_AVRCP(13),
		Bluetooth_PRXM(14),
		Bluetooth_PRXR(15),
		 Bluetooth_PAN_NAP(16),
		 Bluetooth_PAN_GN(17),
		 Bluetooth_MAP_Server(18);
		

		 public final int localizedString;
		 private Profile(int localizedString) {
            this.localizedString = localizedString;
        }	
	}

	/** the interface help profile to provide inified API to be called by profile manafer service*/
	public interface BluetoothProfileBehavior{
		 Set<BluetoothDevice> getConnectedDevices();
		 boolean connect(BluetoothDevice device);
		 boolean disconnect(BluetoothDevice device);
		 int getState(BluetoothDevice device);
		 void close();
	}
	
    /**
     * Create a BluetoothProfileManager proxy object for interacting with the local
     * Profile Manager service.
     * @param c Context
     */
    public BluetoothProfileManager(Context c) {
        mContext = c;
      //  mService=null;

		/** open these code when ProfileManagerService is ready*/
        IBinder b = ServiceManager.getService(BluetoothProfileManagerService.BLUETOOTH_PROFILEMANAGER_SERVICE);
        if (b != null) {
            mService = IBluetoothProfileManager.Stub.asInterface(b);
        } else {
            Log.w(TAG, "Bluetooth profile manager service not available!");            
            mService = null;
        }
       
    }

    /** Initiate a connection to an device based on the profile.
     *  Listen for ACTION_STATE_CHANGED to find out when the
     *  connection is completed.
     *  @param profile specific profile.
     *  @param device Remote BT device.
     *  @return false on immediate error, true otherwise
     *  @hide
     */
    public boolean connect(Profile profile, BluetoothDevice device) {
        if (DBG) log("connect(" + device + ")");
        try {
            return mService.connect(profile.name(), device);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /** disconnect with an device based on the profile.
     *  Listen for ACTION_STATE_CHANGED to find out when the
     *  connection is completed.
     *  @param profile specific profile.
     *  @param device Remote BT device.
     *  @return false on immediate error, true otherwise
     *  @hide
     */
    public boolean disconnect(Profile profile, BluetoothDevice device) {
        if (DBG) log("disconnect(" + device + ")");
        try {
            return mService.disconnect(profile.name(), device);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

	public Set<BluetoothDevice> getConnectedDevices(Profile profile) {
        if (DBG) log("getConnectedDevices()"+profile);
        try {
            return Collections.unmodifiableSet(
                    new HashSet<BluetoothDevice>(Arrays.asList(mService.getConnectedDevices(profile.name()))));
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return null;
        }
    }
	
    public int getState(Profile profile, BluetoothDevice device) {
        if (DBG) log("getState(" + device + ")");
        try {
            return mService.getState(profile.name(),device);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return STATE_DISCONNECTED;
        }
    }
 
	/**only for HFP and A2DP*/
	public boolean isPreferred(Profile profile, BluetoothDevice device) {
		if (DBG) log("isPreferred(" + device + ")");
        try {
            return mService.isPreferred(profile.name(),device);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
	}
	
	/**only for HFP and A2DP*/
	public boolean setPreferred(Profile profile, BluetoothDevice device, boolean preferred) {
        if (DBG) log("setPreferred(" + device + ")");
        try {
            return mService.setPreferred(profile.name(),device,preferred);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
	}

/**only for HFP and A2DP*/
	public int getPreferred(Profile profile, BluetoothDevice device) {
        if (DBG) log("getPreferred(" + device + ")");
        try {
            return mService.getPreferred(profile.name(),device);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return -1;
        }
	}
	private static void log(String msg) {
        Log.d(TAG, msg);
    }

	
   
}





