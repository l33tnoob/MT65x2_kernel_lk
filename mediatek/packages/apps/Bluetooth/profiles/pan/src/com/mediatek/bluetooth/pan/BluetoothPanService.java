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

package com.mediatek.bluetooth.pan;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothPan;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.IBluetooth;
import android.bluetooth.IBluetoothPan;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.InterfaceConfiguration;
import android.net.LinkAddress;
import android.net.NetworkUtils;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.R;

import java.io.IOException;
import java.net.InetAddress;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentHashMap;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class BluetoothPanService extends Service {

    /* Native data */
    private int mNativeData;

    /* Native functions */
    private native void cleanServiceNative();

    private native void forceClearServerNative();

    private native boolean initServiceNative();

    private native boolean listentoSocketNative();

    private native void wakeupListenerNative();

    private native void stopListentoSocketNative();

    /* Native functions for PAN Server */
    private static native void serverAuthorizeRspNative(String btAddr, boolean result);

    private native void serverActivateReqNative();

    private native void serverDeactivateReqNative();

    private native void serverConnectReqNative(int service, String btAddr);

    private static native void serverDisconnectReqNative(String btAddr);

    private static final String TAG = "[BT][PAN][BluetoothPANService]";

    private static final String MESSAGE_DISPLAY = "show toast";

    private static final int MESSAGE_DISPLAY_ID = 0;

    /* Start of HID ID space */
    private static final int PAN_ID_START = BluetoothProfile.getProfileStart(BluetoothProfile.ID_PAN);

    /* HID Server Notification IDs */
    private static int sPanAuthorizeNotify = PAN_ID_START + 1;

    private Intent mUpdateGNStateIntent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);

    private Intent mUpdateNAPStateIntent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);

    private static boolean sServerState = false;

    private SocketListenerThread mSocketListener = null;

    /* Notify */
    Map mNotifyMap = new HashMap();

    /* Notification manager service */
    private NotificationManager mNM = null;

    private Notification mNoti = null;

    int mNotifyId = 0;

    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;

    private BroadcastReceiver mTetheringReceiver = null;

    private boolean mTetheringOn;

    private Context mContext;

    private BluetoothAdapter mAdapter;

    private IBluetooth mBluetoothService;

    private ConcurrentMap<BluetoothDevice, BluetoothPanDevice> mPanDevices;

    private boolean mHasInitiated = false;

    private static final boolean DEBUG = true;

    private static final String BLUETOOTH_IFACE_ADDR_START = "192.168.44.1";

    private static final int BLUETOOTH_PREFIX_LENGTH = 24;

    private INetworkManagementService mNetworkManagementService;

    private String[] mDhcpRange;

    private static final String[] DHCP_DEFAULT_RANGE = {
            "192.168.44.2", "192.168.44.254", "192.168.45.2", "192.168.45.254"
    };

    private String[] mDnsServers;

    private static final String[] DNS_DEFAULT_SERVER = {
            "8.8.8.8", "8.8.4.4"
    };

    public static boolean sUtState = false;

    private void printLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    private void updateSettingsState(int prevState, int state, BluetoothDevice device, int role) {
        printLog("updateSettingsState(" + state + ")");

        Intent intent = new Intent(BluetoothPan.ACTION_CONNECTION_STATE_CHANGED);
        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, device);
        intent.putExtra(BluetoothPan.EXTRA_PREVIOUS_STATE, prevState);
        intent.putExtra(BluetoothPan.EXTRA_STATE, state);
        intent.putExtra(BluetoothPan.EXTRA_LOCAL_ROLE, role);
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);

        printLog("Pan Device state : device: " + device + " State:" + prevState + "->" + state);
        try {
            mBluetoothService.sendConnectionStateChange(device, BluetoothPan.PAN, state, prevState);
        } catch (RemoteException e) {
            Log.e(TAG, "sendConnectionStateChange Exception: " + e);
        }
    }

    private void updateProfileState(int state) {
        /*
         * update_GN_state_intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE,
         * BluetoothProfileManager.Profile.Bluetooth_PAN_GN);
         * update_GN_state_intent
         * .putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);
         * sendBroadcast(update_GN_state_intent);
         * update_NAP_state_intent.putExtra
         * (BluetoothProfileManager.EXTRA_PROFILE,
         * BluetoothProfileManager.Profile.Bluetooth_PAN_NAP);
         * update_NAP_state_intent
         * .putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);
         * sendBroadcast(update_NAP_state_intent);
         */
    }

    private boolean startNetworkConfig() {
        try {
            Log.d(TAG, "start Tethering mDhcp Range");
            mNetworkManagementService.startTethering(mDhcpRange);
        } catch (RemoteException e) {
            Log.e(TAG, "startNetworkConfig error when startTethering");
            e.printStackTrace();
            return false;
        }
        try {
            Log.d(TAG, "set Dns Forwarders");
            mNetworkManagementService.setDnsForwarders(mDnsServers);
        } catch (RemoteException e) {
            Log.e(TAG, "startNetworkConfig error when setDnsForwarders");
            e.printStackTrace();
            return false;
        }
        return true;
    }

    private boolean stopNetworkConfig() {
        try {
            Log.d(TAG, "stopNetworkConfig");
            mNetworkManagementService.stopTethering();
        } catch (RemoteException e) {
            Log.e(TAG, "error when stopTethering");
            e.printStackTrace();
            return false;
        }
        return true;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(BluetoothDevice.ACTION_NAME_CHANGED)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (device == null) {
                    if (DEBUG) {
                        Log.e(TAG, "ERROR: device null");
                    }
                    return;
                }

                BluetoothPanDevice panDevice = mPanDevices.get(device);
                if (panDevice == null) {
                    return;
                }

                String deviceAddr = device.getAddress();
                // String deviceName = device.getName();
                String notifyS = null;
                if (mNotifyMap.containsKey(deviceAddr)) {
                    notifyS = mNotifyMap.get(deviceAddr).toString();
                }
                if (notifyS == null) {
                    if (DEBUG) {
                        Log.e(TAG, "ERROR: notify_s null");
                    }
                    return;
                }

                int notifyID = 0;
                Notification noti = null;
                notifyID = Integer.parseInt(notifyS);
                noti = genPanNotification(notifyID, device, false);
                mNM.notify(notifyID, noti);
            } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                if (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR)
                        == BluetoothAdapter.STATE_TURNING_OFF) {
                    if (mHasInitiated) {
                        unregisterReceiver(mReceiver);
                        localClearService();
                        mHasInitiated = false;
                    }
                }

            }
        }
    };

    private class SocketListenerThread extends Thread {
        public boolean stopped;

        @Override
        public void run() {
            while (!stopped) {
                if (!listentoSocketNative()) {
                    stopped = true;
                }
            }
            if (DEBUG) {
                Log.d(TAG, "SocketListener stopped.");
            }
        }

        public void shutdown() {
            // TODO Auto-generated method stub
            stopped = true;
            wakeupListenerNative();

        }
    }

    public void authorizeRsp(String btAddr, boolean result) {
        if (btAddr == null) {
            return;
        }

        BluetoothDevice device = mAdapter.getRemoteDevice(btAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);

        if (panDevice == null) {
            return;
        }

        if ( panDevice.mState == BluetoothPan.STATE_DISCONNECTING || 
            panDevice.mState == BluetoothPan.STATE_DISCONNECTED) {
            return;
        }

        serverAuthorizeRspNative(btAddr, result);
    }

    public void connectPanDevice(String btAddr, int service) {
        BluetoothDevice device = mAdapter.getRemoteDevice(btAddr);
        if (mPanDevices.containsKey(device)) {
            return;
        }

        BluetoothPanDevice panDevice = new BluetoothPanDevice(BluetoothPan.STATE_CONNECTING,
                service);
        mPanDevices.put(device, panDevice);

        updateSettingsState(BluetoothPan.STATE_DISCONNECTED, BluetoothPan.STATE_CONNECTING, device,
                service);
        serverConnectReqNative(service, btAddr);
    }

    public void disconnectPanDevice(String btAddr) {
        BluetoothDevice device = mAdapter.getRemoteDevice(btAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);

        if (panDevice == null) {
            if (DEBUG) {
                Log.e(TAG, "unknown device");
            }
            return;
        }

        int prevState = panDevice.mState;
        panDevice.mState = BluetoothPan.STATE_DISCONNECTING;
        updateSettingsState(prevState, panDevice.mState, device, panDevice.mLocalRole);
        serverDisconnectReqNative(btAddr);
    }

    private void disconnectPanServerDevices() {
        if (DEBUG) {
            Log.d(TAG, "disconnect all remote panu devices");
        }

        for (BluetoothDevice device : mPanDevices.keySet()) {
            BluetoothPanDevice panDevice = mPanDevices.get(device);
            if (panDevice == null) {
                Log.e(TAG, "disconnectPanServerDevices invalid device!");
                break;
            }
            String deviceAddr = device.getAddress();
            if (mNotifyMap.containsKey(deviceAddr)) {
                mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                mNM.cancel(mNotifyId);
            }

            int state = panDevice.mState;
            if (state == BluetoothPan.STATE_CONNECTED
                    && panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                disconnectPanDevice(device.getAddress());
            }
        }
    }

    private final IBluetoothPanAction.Stub mPanAction = new IBluetoothPanAction.Stub() {
        @Override
        public void disconnectPanDeviceAction(String btAddr) {
            if (DEBUG) {
                Log.d(TAG, "disconnectPanDeviceAction: ");
            }
            disconnectPanDevice(btAddr);
        }

        @Override
        public void authorizeRspAction(String btAddr, boolean result) {
            if (DEBUG) {
                Log.d(TAG, "authorizeRspAction");
            }
            authorizeRsp(btAddr, result);
        }
    };

    /* [In] unit: It is the btn network device index. (e.g. btn0, btn1, ...) */
    private String createNewTetheringAddressLocked(int unit) {
        String address = BLUETOOTH_IFACE_ADDR_START;
        String[] addr = address.split("\\.");
        Integer newIp = Integer.parseInt(addr[2]) + unit;
        address = address.replace(addr[2], newIp.toString());
        return address;
    }

    // configured when we start tethering
    private String enableTethering(int unit, int role) throws IOException {
        printLog("updateTetherState:" + unit);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        INetworkManagementService service = INetworkManagementService.Stub.asInterface(b);
        ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        String[] bluetoothRegexs = cm.getTetherableBluetoothRegexs();
        String iface = bluetoothRegexs[0].replace("\\d", Integer.toString(unit));

        Log.d(TAG, "enableTethering interface name: " + iface);

        // bring toggle the interfaces
        String[] currentIfaces = new String[0];
        try {
            currentIfaces = service.listInterfaces();
        } catch (RemoteException e) {
            Log.e(TAG, "Error listing Interfaces :" + e);
            return null;
        }

        boolean found = false;
        for (String currIface : currentIfaces) {
            if (currIface.equals(iface)) {
                found = true;
                break;
            }
        }

        if (!found) {
            return null;
        }

        String address = createNewTetheringAddressLocked(unit);
        if (address == null) {
            return null;
        }

        InterfaceConfiguration ifcg = null;
        try {
            ifcg = service.getInterfaceConfig(iface);
            if (ifcg != null) {
                InetAddress addr = null;
                final LinkAddress linkAddr = ifcg.getLinkAddress();
                addr = linkAddr.getAddress();
                if (linkAddr == null
                        || addr == null
                        ||
                        // if (ifcg.addr == null || (addr =
                        // ifcg.addr.getAddress()) == null ||
                        addr.equals(NetworkUtils.numericToInetAddress("0.0.0.0"))
                        || addr.equals(NetworkUtils.numericToInetAddress("::0"))) {
                    addr = NetworkUtils.numericToInetAddress(address);
                }
                ifcg.setInterfaceUp();
                ifcg.clearFlag("running");
                ifcg.setLinkAddress(new LinkAddress(addr, BLUETOOTH_PREFIX_LENGTH));
                // ifcg.interfaceFlags = ifcg.interfaceFlags.replace("down",
                // "up");
                // ifcg.addr = new LinkAddress(addr, BLUETOOTH_PREFIX_LENGTH);
                // ifcg.interfaceFlags = ifcg.interfaceFlags.replace("running",
                // "");
                // ifcg.interfaceFlags = ifcg.interfaceFlags.replace("  "," ");
                try {
                    service.setInterfaceConfig(iface, ifcg);
                } catch (Exception e) {
                    Log.e(TAG, "Error set interface config" + iface + ", :" + e);
                    return null;
                }
                if (role == BluetoothPan.LOCAL_NAP_ROLE) {
                    if (cm.tether(iface) != ConnectivityManager.TETHER_ERROR_NO_ERROR) {
                        Log.e(TAG, "Error tethering " + iface);
                    }
                } else if (role == BluetoothPan.LOCAL_GN_ROLE) {
                    startNetworkConfig();
                }
            }
        } catch (RemoteException e) {
            Log.e(TAG, "Error configuring interface " + iface + ", :" + e);
            return null;
        }
        return address;
    }

    private class BluetoothPanDevice {
        private int mState;

        private int mLocalRole; // Which local role is this PAN device bound to

        BluetoothPanDevice(int state, int localRole) {
            mState = state;
            mLocalRole = localRole;
        }
    }

    private final IBluetoothPan.Stub mPan = new IBluetoothPan.Stub() {

        @Override
        public void connect(BluetoothDevice device) {
            // TODO Auto-generated method stub
            String btAddr = device.getAddress();
            if (DEBUG) {
                Log.d(TAG, "BluetoothPanServer Connect");
            }

            // TODO Here only indicate local role, the remote role shall be
            // indicated as well
            connectPanDevice(btAddr, BluetoothPan.LOCAL_PANU_ROLE);
        }

        @Override
        public void disconnect(BluetoothDevice device) {
            // TODO Auto-generated method stub
            if (DEBUG) {
                Log.d(TAG, "BluetoothPanServer Disconnect");
            }
            String btAddr = device.getAddress();

            disconnectPanDevice(btAddr);
        }

        @Override
        public int getState(BluetoothDevice device) {
            // TODO Auto-generated method stub
            BluetoothPanDevice panDevice = mPanDevices.get(device);
            if (panDevice == null) {
                return BluetoothPan.STATE_DISCONNECTED;
            }
            return panDevice.mState;
        }

        @Override
        public List<BluetoothDevice> getConnectedDevices() {
            List<BluetoothDevice> devices = new ArrayList<BluetoothDevice>();

            for (BluetoothDevice device : mPanDevices.keySet()) {
                BluetoothPanDevice panDevice = mPanDevices.get(device);
                if (panDevice == null) {
                    break;
                }
                if (panDevice.mState == BluetoothPan.STATE_CONNECTED) {
                    devices.add(device);
                }
            }
            return devices;
        }

        @Override
        public List<BluetoothDevice> getDevicesMatchingConnectionStates(int[] states) {
            List<BluetoothDevice> devices = new ArrayList<BluetoothDevice>();

            for (BluetoothDevice device : mPanDevices.keySet()) {
                BluetoothPanDevice panDevice = mPanDevices.get(device);
                if (panDevice == null) {
                    break;
                }

                for (int state : states) {
                    if (state == panDevice.mState) {
                        devices.add(device);
                        break;
                    }
                }
            }
            return devices;
        }

        /*
         * It is used for Settings application. Not update the value util BT is
         * on. 1. Open the Settings application. 2. Touch Wireless & networks >
         * Tethering & portable hotspot. 3. Check Bluetooth tethering.
         */
        @Override
        public void setBluetoothTethering(boolean value) {
            if (!value) {
                disconnectPanServerDevices();
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

        @Override
        public boolean isTetheringOn() {
            return mTetheringOn;
        }

    };

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        String action = intent.getAction();
        if (DEBUG) {
            Log.i(TAG, "Enter onBind(): " + action);
        }

        if (IBluetoothPan.class.getName().equals(action)) {
            return mPan;

        } else {
            return mPanAction;
        }
    }

    @Override
    public void onCreate() {
        if (DEBUG) {
            Log.d(TAG, "Enter onCreate()");
        }
        mContext = getApplicationContext();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mPanDevices = new ConcurrentHashMap<BluetoothDevice, BluetoothPanDevice>();
        mTetheringOn = false;
	if (DEBUG) {
            Log.d(TAG, "[API:onCreate] sUtState is " + sUtState);
	}
	if (!sUtState) {
	    Log.d(TAG, "[API:onCreate] library is extpan_jni.so");
	    System.loadLibrary("extpan_jni");
	} else {
	    Log.d(TAG, "[API:onCreate] library is expan_ut.so");
	    System.loadLibrary("extpan_ut");
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        if (DEBUG) {
            Log.d(TAG, "Enter onStartCommand()");
        }

        if (!mHasInitiated) {

            IBinder b = ServiceManager.getService(BluetoothAdapter.BLUETOOTH_SERVICE);
            mBluetoothService = IBluetooth.Stub.asInterface(b);

            // TODO Auto-generated method stub
            mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (mNM == null) {
                if (DEBUG) {
                    Log.e(TAG, "Get Notification-Manager failed. Stop PAN service.");
                }
                stopSelf();
            }

            IntentFilter filter = new IntentFilter();
            filter.addAction(BluetoothDevice.ACTION_NAME_CHANGED);
            filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            registerReceiver(mReceiver, filter);

            mDnsServers = this.getResources().getStringArray(R.array.config_bluetooth_tether_dns_server);
            if (mDnsServers.length == 0) {
                mDnsServers = DNS_DEFAULT_SERVER;
            }

            mDhcpRange = this.getResources().getStringArray(R.array.config_bluetooth_tether_dhcp_range);
            if ((mDhcpRange.length == 0) || (mDhcpRange.length % 2 == 1)) {
                mDhcpRange = DHCP_DEFAULT_RANGE;
            }

            IBinder nmb = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
            mNetworkManagementService = INetworkManagementService.Stub.asInterface(nmb);

            if (mNetworkManagementService == null) {
                Log.e(TAG, "Error get INetworkManagementService");
                this.stopSelf();
            }

            localCreateService();
            super.onCreate();

            mHasInitiated = true;
        } else {
            if (DEBUG) {
                Log.d(TAG, "Already started, just return!");
            }
            return START_STICKY;
        }
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub

        super.onDestroy();
        if (DEBUG) {
            Log.d(TAG, "onDestroy()");
        }
        if (mHasInitiated) {
            unregisterReceiver(mReceiver);
            localClearService();
            mHasInitiated = false;
        }
    }

    void localClearService() {
        boolean timeout = false;
        int cnt = 0;

        if (sServerState) {

            serverDeactivateReqNative();
            for (BluetoothDevice device : mPanDevices.keySet()) {
                BluetoothPanDevice panDevice = mPanDevices.get(device);
                if (panDevice != null) {
                    int prevState = panDevice.mState;
                    if (prevState != BluetoothPan.STATE_DISCONNECTED) {
                        String deviceAddr = device.getAddress();
                        if (mNotifyMap.containsKey(deviceAddr)) {
                            mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                            mNM.cancel(mNotifyId);
                        }
                        updateSettingsState(prevState, BluetoothPan.STATE_DISCONNECTED, device,
                                panDevice.mLocalRole);
                    }
                }
            }

            while (sServerState) {
                if (cnt >= 5000) {
                    timeout = true;
                    break;
                }

                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    if (DEBUG) {
                        Log.e(TAG, "Waiting for server deregister-cnf was interrupted.");
                    }
                }
                cnt += 100;
            }
        }

        if (timeout) {
            /*
             * WARNNING: If we are here, BT task may be crashed or too busy. So
             * we skip waiting DEREGISTER_SERVER_CNF and just clear server
             * context.
             */
            if (DEBUG) {
                Log.w(TAG, "Waiting DEREGISTER_SERVER_CNF time-out. Force clear server context.");
            }
            sServerState = false;
            forceClearServerNative();
        }
        if (mSocketListener != null) {
            try {
                printLog("mSocketListener close.");
                mSocketListener.shutdown();
                mSocketListener.join(1000);
                mSocketListener = null;
                printLog("mSocketListener close OK.");
            } catch (InterruptedException e) {
                if (DEBUG) {
                    Log.e(TAG, "mSocketListener close error.");
                }
            }
        }
        mPanDevices.clear();

        stopListentoSocketNative();
        cleanServiceNative();
    }

    void localCreateService() {
        if (!sServerState) {
            updateProfileState(BluetoothProfileManager.STATE_ENABLING);
            if (initServiceNative()) {
                /* Default values for Pan server settings */
                serverActivateReqNative();

                printLog("Succeed to init BluetoothPanService.");
                if (mSocketListener == null) {
                    mSocketListener = new SocketListenerThread();
                    mSocketListener.setName("BTPanSocketListener");
                    mSocketListener.stopped = false;
                    mSocketListener.start();
                    printLog("SocketListener started.");
                }

                if (DEBUG) {
                    Log.d(TAG, "Pre-enable PAN Server");
                }
            } else {
                if (DEBUG) {
                    Log.d(TAG, "Failed to init BluetoothPanService.");
                }
            }
        }
    }

    /* Utility function: sendServiceMsg */
    private void sendServiceMsg(int what, String message) {
        Message msg = Message.obtain();

        if (DEBUG) {
            Log.i(TAG, "sendServiceMsg status=" + what + "message=" + message);
        }
        msg.what = what;

        Bundle data = new Bundle();
        data.putString(MESSAGE_DISPLAY, message);
        msg.setData(data);

        mServiceHandler.sendMessage(msg);
    }

    private void callback_pan_handle_activate_cnf(boolean result) {
        if (result) {
            sServerState = true;
            updateProfileState(BluetoothProfileManager.STATE_ENABLED);
        } else {
            updateProfileState(BluetoothProfileManager.STATE_ABNORMAL);
        }
    }

    private void callback_pan_handle_deactivate_cnf(boolean result) {
        sServerState = false;
        if (result) {
            updateProfileState(BluetoothProfileManager.STATE_DISABLED);
        } else {
            updateProfileState(BluetoothProfileManager.STATE_ABNORMAL);
        }
    }

    /*
     * [IN] service: It shall be referred to bluetooth_pan_struct.h
     * "bt_pan_service_enum" service > BluetoothPan: LOCAL_NAP_ROLE,
     * LOCAL_GN_ROLE, LOCAL_PANU_ROLE
     */
    private void callback_pan_handle_connection_authorize_ind(int service, String deviceAddr) {
        if ((service == BluetoothPan.LOCAL_NAP_ROLE) && !mTetheringOn) {
            String toastMsg = getString(R.string.bluetooth_pan_NAP_unavailable, mAdapter.getRemoteDevice(deviceAddr)
                    .getName());
            sendServiceMsg(MESSAGE_DISPLAY_ID, toastMsg);
            authorizeRsp(deviceAddr, false);
            return;
        }

        int deviceState = BluetoothPan.STATE_CONNECTING;
        BluetoothDevice device = mAdapter.getRemoteDevice(deviceAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);

        if (panDevice == null) {
            panDevice = new BluetoothPanDevice(deviceState, service);
            mPanDevices.put(device, panDevice);
        } else {
            if (panDevice.mState != BluetoothPan.STATE_DISCONNECTED) {
                authorizeRsp(deviceAddr, false);
                return;
            }
        }

        updateSettingsState(BluetoothPan.STATE_DISCONNECTED, deviceState, device, service);

        if (!mNotifyMap.containsKey(deviceAddr)) {
            mNotifyMap.put(deviceAddr, sPanAuthorizeNotify);
            sPanAuthorizeNotify++;
        }

        mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
        mNoti = genPanNotification(mNotifyId, device, true);
        mNM.notify(mNotifyId, mNoti);
    }

    private void callback_pan_handle_connect_ind(int service, String deviceAddr, int unit) {
        if ((service == BluetoothPan.LOCAL_NAP_ROLE) && !mTetheringOn) {
            disconnectPanDevice(deviceAddr);
            return;
        }

        int prevState = BluetoothPan.STATE_DISCONNECTED;
        int deviceState = BluetoothPan.STATE_CONNECTED;
        BluetoothDevice device = mAdapter.getRemoteDevice(deviceAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);

        if (panDevice == null) {
            panDevice = new BluetoothPanDevice(deviceState, service);
            mPanDevices.put(device, panDevice);
        } else {
            prevState = panDevice.mState;
            panDevice.mState = deviceState;
            panDevice.mLocalRole = service;
        }

        try {
            enableTethering(unit, service);
        } catch (IOException e) {
            Log.e(TAG, "Error enableTethering :" + e);
            return;
        }

        updateSettingsState(prevState, deviceState, device, service);

        if (!mNotifyMap.containsKey(deviceAddr)) {
            mNotifyMap.put(deviceAddr, sPanAuthorizeNotify);
            sPanAuthorizeNotify++;
        }

        mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
        mNoti = genPanNotification(mNotifyId, device, false);
        mNM.notify(mNotifyId, mNoti);
    }

    private void callback_pan_handle_connect_cnf(boolean result, String deviceAddr, int unit) {
        String toastMsg = new String();
        BluetoothDevice device = mAdapter.getRemoteDevice(deviceAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);
        String deviceName = device.getName();

        if (panDevice == null) {
            if (DEBUG) {
                Log.e(TAG, "unknow device when callback_pan_handle_connect_cnf:" + deviceAddr);
            }
            return;
        }

        int prevState = panDevice.mState;
        if (result) {
            if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_NAP_connect_ok, deviceName);
            } else if (panDevice.mLocalRole == BluetoothPan.LOCAL_GN_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_GN_connect_ok, deviceName);
            }

            try {
                enableTethering(unit, panDevice.mLocalRole);
            } catch (IOException e) {
                Log.e(TAG, "Error enableTethering :" + e);
                return;
            }

            panDevice.mState = BluetoothPan.STATE_CONNECTED;

            if (!mNotifyMap.containsKey(deviceAddr)) {
                mNotifyMap.put(deviceAddr, sPanAuthorizeNotify);
                sPanAuthorizeNotify++;
            }

            mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
            mNoti = genPanNotification(mNotifyId, device, false);
            mNM.notify(mNotifyId, mNoti);
        } else {
            if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_NAP_connect_fail, deviceName);
            } else {
                toastMsg = getString(R.string.bluetooth_pan_GN_connect_fail, deviceName);
            }

            panDevice.mState = BluetoothPan.STATE_DISCONNECTED;

            if (mNotifyMap.containsKey(deviceAddr)) {
                mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                mNM.cancel(mNotifyId);
            }
        }

        sendServiceMsg(MESSAGE_DISPLAY_ID, toastMsg);
        updateSettingsState(prevState, panDevice.mState, device, panDevice.mLocalRole);

    }

    private void callback_pan_handle_disconnect_ind(String deviceAddr) {
        String toastMsg = new String();
        BluetoothDevice device = mAdapter.getRemoteDevice(deviceAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);
        String deviceName = device.getName();

        if (panDevice == null) {
            if (DEBUG) {
                Log.e(TAG, "unknow device when callback_pan_handle_disconnect_ind:" + deviceAddr);
            }
            return;
        }

        if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
            toastMsg = getString(R.string.bluetooth_pan_NAP_disconnect_ind, deviceName);
        } else if (panDevice.mLocalRole == BluetoothPan.LOCAL_GN_ROLE) {
            stopNetworkConfig();
            toastMsg = getString(R.string.bluetooth_pan_GN_disconnect_ind, deviceName);
        }

        sendServiceMsg(MESSAGE_DISPLAY_ID, toastMsg);

        mPanDevices.remove(device);

        if (mNotifyMap.containsKey(deviceAddr)) {
            mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
            mNM.cancel(mNotifyId);
        }
        updateSettingsState(panDevice.mState, BluetoothPan.STATE_DISCONNECTED, device,
                panDevice.mLocalRole);
    }

    private void callback_pan_handle_disconnect_cnf(boolean result, String deviceAddr) {
        String toastMsg = new String();
        BluetoothDevice device = mAdapter.getRemoteDevice(deviceAddr);
        BluetoothPanDevice panDevice = mPanDevices.get(device);
        String deviceName = device.getName();
        int state = BluetoothPan.STATE_DISCONNECTED;

        if (panDevice == null) {
            if (DEBUG) {
                Log.e(TAG, "unknow device when callback_pan_handle_disconnect_cnf:" + deviceAddr);
            }
            return;
        }

        int prevState = panDevice.mState;

        if (result) {
            if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_NAP_disconnect_ok, deviceName);
            } else if (panDevice.mLocalRole == BluetoothPan.LOCAL_GN_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_GN_disconnect_ok, deviceName);
                stopNetworkConfig();
            }

            mPanDevices.remove(device);

            if (mNotifyMap.containsKey(deviceAddr)) {
                mNotifyId = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                mNM.cancel(mNotifyId);
            }
        } else {
            if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_GN_disconnect_fail, deviceName);
            } else if (panDevice.mLocalRole == BluetoothPan.LOCAL_GN_ROLE) {
                toastMsg = getString(R.string.bluetooth_pan_NAP_disconnect_fail, deviceName);
            }
            panDevice.mState = BluetoothPan.STATE_CONNECTED;
            state = panDevice.mState;
        }

        sendServiceMsg(MESSAGE_DISPLAY_ID, toastMsg);
        updateSettingsState(prevState, state, device, panDevice.mLocalRole);
    }

    /* Utility function: genPanNotification */
    private Notification genPanNotification(int type, BluetoothDevice device, boolean needSound) {
        BluetoothPanDevice panDevice = mPanDevices.get(device);
        if (panDevice == null) {
            Log.e(TAG, "genPanNotification invalid device!");
            return null;
        }

        String deviceName = device.getName();

        Intent tmpIntent = new Intent();
        Notification tmpNoti = null;
        PendingIntent tmpContentIntent = null;
        int iconID = -1;
        String clazz = null;
        String ticker = null;
        String title = null;
        String action = null;
        if (DEBUG) {
            Log.i(TAG, "genPanNotification " + device.getAddress() + ";device_state " + panDevice.mState);
        }

        iconID = android.R.drawable.stat_sys_data_bluetooth;
        clazz = BluetoothPanAlert.class.getName();
        tmpIntent.setClassName(getPackageName(), clazz).putExtra(BluetoothPan.DEVICE_ADDR, device.getAddress());
        String notifyMsg = new String();
        int notifyFlags = 0;

        if (panDevice.mState == BluetoothPan.STATE_CONNECTED) {
            notifyFlags = Notification.FLAG_ONGOING_EVENT;

            if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                notifyMsg = getString(R.string.bluetooth_pan_NAP_connected_notify_message, deviceName);
                ticker = getString(R.string.bluetooth_pan_NAP_connected_notify_ticker);
                title = getString(R.string.bluetooth_pan_NAP_connected_notify_title);
                action = BluetoothPan.BT_PAN_NAP_DEVICE_CONNECTED;
            } else if (panDevice.mLocalRole == BluetoothPan.LOCAL_GN_ROLE) {
                notifyMsg = getString(R.string.bluetooth_pan_GN_connected_notify_message, deviceName);
                ticker = getString(R.string.bluetooth_pan_GN_connected_notify_ticker);
                title = getString(R.string.bluetooth_pan_GN_connected_notify_title);
                action = BluetoothPan.BT_PAN_GN_DEVICE_CONNECTED;
            }
        } else if (panDevice.mState == BluetoothPan.STATE_CONNECTING) {
            notifyFlags = Notification.FLAG_ONLY_ALERT_ONCE;

            if (panDevice.mLocalRole == BluetoothPan.LOCAL_NAP_ROLE) {
                notifyMsg = getString(R.string.bluetooth_pan_NAP_authorize_notify_message, deviceName);
                ticker = getString(R.string.bluetooth_pan_NAP_authorize_notify_ticker);
                title = getString(R.string.bluetooth_pan_NAP_authorize_notify_title);
                action = BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE;
            } else if (panDevice.mLocalRole == BluetoothPan.LOCAL_GN_ROLE) {
                notifyMsg = getString(R.string.bluetooth_pan_GN_authorize_notify_message, deviceName);
                ticker = getString(R.string.bluetooth_pan_GN_authorize_notify_ticker);
                title = getString(R.string.bluetooth_pan_GN_authorize_notify_title);
                action = BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE;
            }
        }

        tmpNoti = new Notification(iconID, ticker, System.currentTimeMillis());
        tmpNoti.flags = notifyFlags;
        if (needSound) {
            tmpNoti.defaults |= Notification.DEFAULT_SOUND;
            tmpNoti.defaults |= Notification.DEFAULT_VIBRATE;
        }

        tmpIntent.putExtra(BluetoothPan.ACTION, action);
        tmpContentIntent = PendingIntent.getActivity(getApplicationContext(), type, tmpIntent,
                PendingIntent.FLAG_CANCEL_CURRENT);
        tmpNoti.setLatestEventInfo(mContext, title, notifyMsg, tmpContentIntent);

        return tmpNoti;
    }

    /* Handler associated with the main thread */
    private Handler mServiceHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            if (DEBUG) {
                Log.i(TAG, "handleMessage(): " + msg.what);
            }
            Bundle data = msg.getData();
            String toastMsg = (data != null) ? data.getString(MESSAGE_DISPLAY) : null;
            switch (msg.what) {
                case MESSAGE_DISPLAY_ID:
                    Toast.makeText(mContext, toastMsg, Toast.LENGTH_LONG).show();
                    break;
                default:
                    return;
            }
        }
    };
}
