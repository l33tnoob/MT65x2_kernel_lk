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

package com.mediatek.bluetooth.hid;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHid;
import android.bluetooth.BluetoothInputDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.IBluetooth;
import android.bluetooth.IBluetoothHid;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.bluetooth.R;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * @author MTK80684
 */
public class BluetoothHidService extends Service {
    private int mNativeData;

    /*
     * static { System.loadLibrary("exthid_jni"); }
     */
    /* Native functions */
    private native void cleanServiceNative();

    private native void forceClearServerNative();

    private native boolean initServiceNative();

    private native boolean listentoSocketNative();

    private native void wakeupListenerNative();

    private native void stopListentoSocketNative();

    /* Native functions for HID Server */
    private native void serverAuthorizeReqNative(String btAddr, boolean result);

    private native void serverActivateReqNative();

    private native void serverDeactivateReqNative();

    private native void serverConnectReqNative(String btAddr);

    private native void serverDisconnectReqNative(String btAddr);

    private native void serverUnplugReqNative(String btAddr);

    private native void serverSendReportReqNative(String btAddr, String report);

    private native void serverSetReportReqNative(String btAddr, int reportType, String report);

    private native void serverGetReportReqNative(String btAddr, int reportType, int reportId);

    private native void serverSetProtocolReqNative(String btAddr, int protocolMode);

    private native void serverGetProtocolReqNative(String btAddr);

    private native void serverSetIdleReqNative(String btAddr);

    private native void serverGetIdleReqNative(String btAddr);

    private static final String TAG = "[BT][HID][BluetoothHidService]";

    private static final String BT_HID_SETTING_INFO = "BT_HID_SETTING_INFO";

    private static final String BT_HID_NOT_FOUNT = "BT_HID_NOT_FOUNT";

    private static final int BT_HID_DEBUG_LOG = 1;

    private static final int BT_HID_ERROR_LOG = 2;

    private static final int BT_HID_INFO_LOG = 3;

    private static final int BT_HID_VERB_LOG = 4;

    private static final int BT_HID_WARN_LOG = 5;

    Context mCx = this;

    /* Notification manager service */
    private NotificationManager mNM = null;

    /* Flag for debug messages */
    private static final boolean DEBUG = true;

    /* Start of HID ID space */
    private static final int HID_ID_START = 10; // BluetoothProfile.getProfileStart(BluetoothProfile.ID_HID);

    public static final String FINISH_ACTION = "com.mediatek.bluetooth.hid.finish";

    /* HID Server Notification IDs */
    private static int sHidConnectNotify = HID_ID_START + 1;

    /* Server state */
    Map<String, String> mStateMap = new HashMap<String, String>();

    Map mNotifyMap = new HashMap();

    private int mServerState;

    Preference mPreference = null;

    PreferenceCategory mPC;

    private static boolean sServiceDisable;

    private IBluetooth mBluetoothService;

    Intent mUpdateStateIntent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);

    public static boolean sUtState = false;

    private static BluetoothHidService sUtInstance;

    public static synchronized BluetoothHidService getHidServiceUtInstance() {
        return sUtInstance;
    }

    private class SocketListenerThread extends Thread {
        public boolean mStopped;

        @Override
        public void run() {
            while (!mStopped) {
                if (!listentoSocketNative()) {
                    mStopped = true;
                }
            }

            printLog("SocketListener stopped.", BT_HID_DEBUG_LOG);
        }

        public void shutdown() {
            mStopped = true;
            wakeupListenerNative();

        }
    }

    private class ActionTimeoutThread extends Thread {
        public String mBTAddr;

        public String mState;

        private boolean mStoped = false;

        @Override
        public void run() {
            actionTimeout(mBTAddr, mState);
        }

        public void shutdown() {
            mStoped = true;
        }

        private void actionTimeout(String btAddr, String state) {
            boolean timeout = false;
            int cnt = 0;
            String curState = getDeviceState(btAddr);
            /* if (!mStateMap.containsKey(btAddr)) { */
            if (curState == null) {
                printLog("ERROR: stateMap not contain " + btAddr, BT_HID_ERROR_LOG);
                return;
            }

            while (!curState.equals(state) && !mStoped) {
                if (cnt >= 60000) {
                    timeout = true;
                    break;
                }

                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    printLog("Waiting for action was interrupted.", BT_HID_ERROR_LOG);
                }
                cnt += 100;
            }

            if (timeout) {
                printLog("Waiting action time-out. Force return.", BT_HID_WARN_LOG);
                if (state.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverDisconnectReqNative(btAddr);
                    sendServiceMsg(BluetoothHid.MBTEVT_HID_HOST_CONNECT_FAIL, btAddr);
                } else if (state.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECT)) {
                    sendServiceMsg(BluetoothHid.MBTEVT_HID_HOST_DISCONNECT_FAIL, btAddr);
                }
            }
        }

    }

    /* A thread that keep listening to the socket for incoming ILM */
    private SocketListenerThread mSocketListener = null;

    private ActionTimeoutThread mConnectTimeout = null;

    private ActionTimeoutThread mDisconnectTimeout = null;

    private void connectHidDevice(String btAddr) {
        if (mConnectTimeout == null) {
            mConnectTimeout = new ActionTimeoutThread();
        }
        mConnectTimeout.setName("hidConnectTimeoutThread");
        mConnectTimeout.mBTAddr = btAddr;
        mConnectTimeout.mState = BluetoothHid.BT_HID_DEVICE_CONNECT;
        mServerState = BluetoothHid.BT_HID_STATE_CONNECTING;
        // stateMap.remove(BT_Addr);
        // stateMap.put(BT_Addr, BluetoothHid.BT_HID_DEVICE_CONNECTING);
        /*
         * pc=BluetoothHidActivity.getDeviceList(); if(pc!=null){
         * mPreference=pc.findPreference(BT_Addr); if(mPreference!=null){
         * mPreference.setSummary(R.string.bluetooth_hid_summary_connecting);
         * mPreference.setEnabled(false); } }
         */
        printLog("connectHidDevice::updateActivityUI:hid_summary_connecting", BT_HID_INFO_LOG);
        updateActivityUI(btAddr, R.string.bluetooth_hid_summary_connecting, false);
        updateSettingsState(BluetoothHid.BT_HID_DEVICE_CONNECTING, getBluetoothDevice(btAddr));
        updateDeviceState(btAddr, BluetoothHid.BT_HID_DEVICE_CONNECTING);
        serverConnectReqNative(btAddr);
        if (!mConnectTimeout.isAlive()) {
            mConnectTimeout.start();
        }
    }

    private void disconnectHidDevice(String btAddr) {
        if (mStateMap.containsKey(btAddr)) {
            if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                if (mDisconnectTimeout == null) {
                    mDisconnectTimeout = new ActionTimeoutThread();
                }
                mDisconnectTimeout.setName("hidDisconnectTimeoutThread");
                mDisconnectTimeout.mBTAddr = btAddr;
                mDisconnectTimeout.mState = BluetoothHid.BT_HID_DEVICE_DISCONNECT;
                mServerState = BluetoothHid.BT_HID_STATE_DISCONNECTING;
                // stateMap.remove(BT_Addr);
                // stateMap.put(BT_Addr,
                // BluetoothHid.BT_HID_DEVICE_DISCONNECTING);
                /*
                 * pc=BluetoothHidActivity.getDeviceList(); if(pc!=null){
                 * mPreference=pc.findPreference(BT_Addr);
                 * if(mPreference!=null){mPreference.setSummary(R.string.
                 * bluetooth_hid_summary_disconnecting);
                 * mPreference.setEnabled(false); } }
                 */

                updateActivityUI(btAddr, R.string.bluetooth_hid_summary_disconnecting, false);
                updateSettingsState(BluetoothHid.BT_HID_DEVICE_DISCONNECTING,
                        getBluetoothDevice(btAddr));
                updateDeviceState(btAddr, BluetoothHid.BT_HID_DEVICE_DISCONNECTING);
                serverDisconnectReqNative(btAddr);
                if (!mDisconnectTimeout.isAlive()) {
                    mDisconnectTimeout.start();
                }
            } else {
                printLog("error state to disconnect", BT_HID_ERROR_LOG);
            }
        }

    }

    /********************************************************************************************
     * Binder Interface Objects Definitionas and Binder Callbacks
     ********************************************************************************************/

    /*
     * The binder object for launching HID client and requesting connection
     * status.
     */
    private final IBluetoothHid.Stub mHid = new IBluetoothHid.Stub() {
        public void connect(BluetoothDevice device) throws RemoteException {
            String btAddr = device.getAddress();
            printLog("BluetoothHidServer Connect", BT_HID_DEBUG_LOG);

            String state = getDeviceState(btAddr);
            if (state != null) {
                if (!state.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)
                        && !state.equals(BluetoothHid.BT_HID_DEVICE_CONNECTING)) {
                    connectHidDevice(btAddr);
                } else {
                    printLog("already connected", BT_HID_DEBUG_LOG);
                }
            } else {
                connectHidDevice(btAddr);
            }
        }

        public void disconnect(BluetoothDevice device) throws RemoteException {
            printLog("BluetoothHidServer Disconnect", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            disconnectHidDevice(btAddr);
        }

        public void setReport(BluetoothDevice device, byte reportType, String report)
                throws RemoteException {
            // TODO Auto-generated method stub
            printLog("BluetoothHidServer setReport", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            if (mStateMap.containsKey(btAddr)) {
                if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverSetReportReqNative(btAddr, reportType, report);
                } else {
                    printLog("error state to setReport", BT_HID_ERROR_LOG);
                }
            }
        }

        public void getReport(BluetoothDevice device, byte reportType, byte reportId, int bufferSize)
                throws RemoteException {
            // TODO Auto-generated method stub
            printLog("BluetoothHidServer getReport", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            if (mStateMap.containsKey(btAddr)) {
                if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverGetReportReqNative(btAddr, reportType, reportId);
                } else {
                    printLog("error state to getReport", BT_HID_ERROR_LOG);
                }
            }
        }

        public void sendData(BluetoothDevice device, String report) throws RemoteException {
            // TODO Auto-generated method stub
            printLog("BluetoothHidServer sendData", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            if (mStateMap.containsKey(btAddr)) {
                if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverSendReportReqNative(btAddr, report);
                } else {
                    printLog("error state to sendData", BT_HID_ERROR_LOG);
                }
            }
        }

        public void getProtocolMode(BluetoothDevice device) throws RemoteException {
            // TODO Auto-generated method stub
            printLog("BluetoothHidServer getProtocolMode", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            if (mStateMap.containsKey(btAddr)) {
                if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverGetProtocolReqNative(btAddr);
                } else {
                    printLog("error state to getProtocolMode", BT_HID_ERROR_LOG);
                }
            }
        }

        public void setProtocolMode(BluetoothDevice device, int protocolMode)
                throws RemoteException {
            // TODO Auto-generated method stub
            printLog("BluetoothHidServer setProtocolMode", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            if (mStateMap.containsKey(btAddr)) {
                if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverSetProtocolReqNative(btAddr, protocolMode);
                } else {
                    printLog("error state to setProtocolMode", BT_HID_ERROR_LOG);
                }
            }
        }

        public void virtualUnplug(BluetoothDevice device) throws RemoteException {
            // TODO Auto-generated method stub
            printLog("BluetoothHidServer virtualUnplug", BT_HID_DEBUG_LOG);
            String btAddr = device.getAddress();

            if (mStateMap.containsKey(btAddr)) {
                if (mStateMap.get(btAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    serverUnplugReqNative(btAddr);
                } else {
                    printLog("error state to virtualUnplug", BT_HID_ERROR_LOG);
                }
            }
        }

        public int getState(BluetoothDevice device) throws RemoteException {
            if (mStateMap.isEmpty()) {
                // return BluetoothProfileManager.STATE_DISCONNECTED;
                return BluetoothInputDevice.STATE_DISCONNECTED;
            }
            if (mStateMap.containsKey(device.getAddress())) {
                String tmpStr = mStateMap.get(device.getAddress()).toString();

                if (tmpStr.equals(BluetoothHid.BT_HID_DEVICE_CONNECTING)
                        || tmpStr.equals(BluetoothHid.BT_HID_DEVICE_AUTHORIZE)) {
                    // return BluetoothProfileManager.STATE_CONNECTING;
                    return BluetoothInputDevice.STATE_CONNECTING;
                } else if (tmpStr.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    // return BluetoothProfileManager.STATE_CONNECTED;
                    return BluetoothInputDevice.STATE_CONNECTED;
                } else if (tmpStr.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECT)) {
                    // return BluetoothProfileManager.STATE_DISCONNECTED;
                    return BluetoothInputDevice.STATE_DISCONNECTED;
                } else if (tmpStr.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECTING)
                        || tmpStr.equals(BluetoothHid.BT_HID_DEVICE_UNPLUG)) {
                    // return BluetoothProfileManager.STATE_DISCONNECTING;
                    return BluetoothInputDevice.STATE_DISCONNECTING;
                }
            }

            // return BluetoothProfileManager.STATE_DISCONNECTED;
            return BluetoothInputDevice.STATE_DISCONNECTED;

        }

        public BluetoothDevice[] getCurrentDevices() throws RemoteException {
            printLog("getCurrentDevices", BT_HID_DEBUG_LOG);
            // BluetoothDevice[] deviceList=new BluetoothDevice[5];
            Set<BluetoothDevice> deviceList = new HashSet<BluetoothDevice>();
            Set<BluetoothDevice> pairedDevices = BluetoothAdapter.getDefaultAdapter()
                    .getBondedDevices();

            if (pairedDevices != null) {
                for (BluetoothDevice tmpDevice : pairedDevices) {
                    if (mStateMap.containsKey(tmpDevice.getAddress())) {
                        if (mStateMap.get(tmpDevice.getAddress()).equals(
                                BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                            // deviceList[deviceIndex++]=tmpDevice;
                            deviceList.add(tmpDevice);
                        }
                    }
                }
            }

            // return deviceList;
            printLog("getCurrentDevices:deviceList.size=" + deviceList.size(), BT_HID_VERB_LOG);
            return deviceList.toArray(new BluetoothDevice[deviceList.size()]);
        }

    };

    /********************************************************************************************
     * Callback Functions for HID Profile Manager
     ********************************************************************************************/

    /* AIDL callback to Hid Profile Manager in Bluetooth Settings */
    private void updateSettingsState(String state, BluetoothDevice device) {
        int preState = convertStatusToInt(mStateMap.get(device.getAddress()));
        int curState = convertStatusToInt(state);

        Intent tmpInt = new Intent(BluetoothInputDevice.ACTION_CONNECTION_STATE_CHANGED);
        tmpInt.putExtra(BluetoothDevice.EXTRA_DEVICE, device);
        tmpInt.putExtra(BluetoothInputDevice.EXTRA_PREVIOUS_STATE, preState);
        tmpInt.putExtra(BluetoothInputDevice.EXTRA_STATE, curState);

        // if ((!state.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECT))
        // || (state.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECT) &&
        // !hasOtherConnectedHidDevice(device))) {
        // sendBroadcast(tmpInt);
        // }

        try {
            mBluetoothService.sendConnectionStateChange(device, BluetoothProfile.INPUT_DEVICE,
                    curState, preState);
        } catch (RemoteException e) {
            Log.e(TAG, "sendConnectionStateChange Exception: " + e);
        }
        //M:Support multi-user
        mCx.sendBroadcastAsUser(tmpInt,UserHandle.ALL);
        printLog("updateSettingsState", BT_HID_VERB_LOG);
    }

    private int convertStatusToInt(String oriState) {
        if (null == oriState) {
            return BluetoothInputDevice.STATE_DISCONNECTED;
        }

        if (oriState.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
            return BluetoothInputDevice.STATE_CONNECTED;
        } else if (oriState.equals(BluetoothHid.BT_HID_DEVICE_CONNECTING)
                || oriState.equals(BluetoothHid.BT_HID_DEVICE_AUTHORIZE)) {
            return BluetoothInputDevice.STATE_CONNECTING;
        } else if (oriState.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECT)) {
            return BluetoothInputDevice.STATE_DISCONNECTED;
        } else if (oriState.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECTING)
                || oriState.equals(BluetoothHid.BT_HID_DEVICE_UNPLUG)) {
            return BluetoothInputDevice.STATE_DISCONNECTING;
        }

        return BluetoothInputDevice.STATE_DISCONNECTED;
    }

    // private boolean hasOtherConnectedHidDevice(BluetoothDevice exceptDevice)
    // {
    // boolean bResult = false;
    // Set<String> c = mStateMap.keySet();
    // Iterator it = c.iterator();
    //
    // while (it.hasNext()) {
    // String tmp = (String) it.next();
    // if (mStateMap.get(tmp) == null) {
    // continue;
    // }
    // if (mStateMap.get(tmp).equals(BluetoothHid.BT_HID_DEVICE_CONNECT)
    // && !tmp.equals(exceptDevice.getAddress())) {
    // bResult = true;
    // break;
    // }
    // }
    //
    // return bResult;
    // }

    private void updateActivityUI(String btAddr, int summary, boolean enable) {
        printLog("updateActivityUI", BT_HID_VERB_LOG);
        Intent tmpInt = new Intent(BluetoothHidActivity.ACTION_SUMMARY_CHANGED);

        tmpInt.putExtra(BluetoothHidActivity.EXTRA_DEVICE, btAddr);
        tmpInt.putExtra(BluetoothHidActivity.EXTRA_SUMMARY, summary);
        tmpInt.putExtra(BluetoothHidActivity.EXTRA_ENABLE, enable);
        printLog("updateActivityUI:sendBroadcast ", BT_HID_INFO_LOG);
        //M:Support multi-user
        //sendBroadcast(tmpInt);
        mCx.sendBroadcastAsUser(tmpInt,UserHandle.ALL);
    }

    private String getDeviceState(String btAddr) {
        if (mStateMap.isEmpty()) {
            return null;
        }
        if (mStateMap.containsKey(btAddr)) {
            return mStateMap.get(btAddr).toString();
        } else {
            return null;
        }
    }

    private final IBluetoothHidServerNotify.Stub mHidServerNotify = new IBluetoothHidServerNotify.Stub() {

        public void activateReq() {
            printLog("BluetoothHidActivity Activate: ", BT_HID_DEBUG_LOG);
            serverActivateReqNative();
        }

        public void deactivateReq() {
            printLog("BluetoothHidActivity DeactivateReq", BT_HID_DEBUG_LOG);
            serverDeactivateReqNative();
        }

        public void connectReq(String btAddr) {
            printLog("BluetoothHidActivity Connect", BT_HID_DEBUG_LOG);
            String state = getDeviceState(btAddr);
            if (state != null) {
                if (!state.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)
                        && !state.equals(BluetoothHid.BT_HID_DEVICE_CONNECTING)) {
                    connectHidDevice(btAddr);
                } else {
                    printLog("already connected", BT_HID_DEBUG_LOG);
                }
            } else {
                connectHidDevice(btAddr);
            }
        }

        public void disconnectReq(String btAddr) {
            printLog("BluetoothHidActivity Disconnect", BT_HID_DEBUG_LOG);

            disconnectHidDevice(btAddr);
        }

        public void unplugReq(String btAddr) {
            printLog("BluetoothHidActivity unplug", BT_HID_DEBUG_LOG);
            /*
             * pc=BluetoothHidActivity.getDeviceList(); if(pc!=null){
             * mPreference=pc.findPreference(BT_Addr); if(mPreference!=null){
             * mPreference
             * .setSummary(R.string.bluetooth_hid_summary_disconnecting);
             * mPreference.setEnabled(false); } }
             */
            updateActivityUI(btAddr, R.string.bluetooth_hid_summary_disconnecting, false);
            updateSettingsState(BluetoothHid.BT_HID_DEVICE_DISCONNECTING,
                    getBluetoothDevice(btAddr));
            serverUnplugReqNative(btAddr);
        }

        public void sendReportReq(String btAddr, boolean len) {
            printLog("BluetoothHidActivity sendReport", BT_HID_DEBUG_LOG);
	    String report_short = "BlueAngel";
	    String report_long  = "BlueAngel HID PTS Test send report, the string should have a length larger then the MTU which is 48 in our solution";
	    if(len)
            	serverSendReportReqNative(btAddr, report_long);
	    else		
		serverSendReportReqNative(btAddr, report_short);
        }

        public void setReportReq(String btAddr) {
            printLog("BluetoothHidActivity setReport", BT_HID_DEBUG_LOG);
            String report = "BlueAngel HID PTS Test set report";
            serverSetReportReqNative(btAddr, BluetoothInputDevice.REPORT_TYPE_OUTPUT, report);
        }

        public void getReportReq(String btAddr) {
            printLog("BluetoothHidActivity getReport", BT_HID_DEBUG_LOG);
            serverGetReportReqNative(btAddr, BluetoothInputDevice.REPORT_TYPE_INPUT, 1);
        }

        public void setProtocolReq(String btAddr) {
            printLog("BluetoothHidActivity setProtocol", BT_HID_DEBUG_LOG);
            serverSetProtocolReqNative(btAddr, BluetoothInputDevice.PROTOCOL_REPORT_MODE);
        }

        public void getProtocolReq(String btAddr) {
            printLog("BluetoothHidActivity getProtocol", BT_HID_DEBUG_LOG);
            serverGetProtocolReqNative(btAddr);
        }

        public void setIdleReq(String btAddr) {
            printLog("BluetoothHidActivity setIdle", BT_HID_DEBUG_LOG);
            serverSetIdleReqNative(btAddr);
        }

        public void getIdleReq(String btAddr) {
            printLog("BluetoothHidActivity getIdle", BT_HID_DEBUG_LOG);
            serverGetIdleReqNative(btAddr);
        }

        public String getStateByAddr(String btAddr) throws RemoteException {
            return getDeviceState(btAddr);
        }

        public void clearService() throws RemoteException {
            printLog("BluetoothHidActivity clearService", BT_HID_DEBUG_LOG);
            localClearService();
        }

        public void authorizeReq(String btAddr, boolean result) throws RemoteException {
            printLog("BluetoothHidActivity authorizeReq", BT_HID_DEBUG_LOG);
            serverAuthorizeReqNative(btAddr, result);
        }

        public void finishActionReq() throws RemoteException {
            printLog("BluetoothHidActivity finishActionReq", BT_HID_DEBUG_LOG);
            Intent intent = new Intent(FINISH_ACTION);
             //M:Support multi-user
            //sendBroadcast(intent);
            mCx.sendBroadcastAsUser(intent,UserHandle.ALL);
        }
    };

    @Override
    public void onCreate() {
        printLog("Enter onCreate()", BT_HID_DEBUG_LOG);
        /* Request system services */
        mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        if (!sUtState) {
            System.loadLibrary("exthid_jni");
        } else {
            System.loadLibrary("exthid_ut");
        }
        if (mNM == null) {
            printLog("Get Notification-Manager failed. Stop HID service.", BT_HID_DEBUG_LOG);
            stopSelf();
        }
        sUtInstance = this;
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_NAME_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
      //M:Support multi-user
        mCx.registerReceiverAsUser(mReceiver,UserHandle.ALL,filter, null,null);

        localCreateService();

        IBinder b = ServiceManager.getService(BluetoothAdapter.BLUETOOTH_SERVICE);
        if (null == b) {
            throw new RuntimeException("Bluetooth service not available");
        }
        mBluetoothService = IBluetooth.Stub.asInterface(b);

        super.onCreate();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        printLog("onDestroy()", BT_HID_DEBUG_LOG);
        unregisterReceiver(mReceiver);
        localClearService();
    }

    void localClearService() {
        boolean timeout = false;
        int cnt = 0;

        if (mServerState != BluetoothHid.BT_HID_STATE_DISACTIVE) {
            sServiceDisable = true;
            serverDeactivateReqNative();

            while (mServerState != BluetoothHid.BT_HID_STATE_DISACTIVE) {
                if (cnt >= 5000) {
                    timeout = true;
                    break;
                }

                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    printLog("Waiting for server deregister-cnf was interrupted.", BT_HID_ERROR_LOG);
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
            printLog("Waiting DEREGISTER_SERVER_CNF time-out. Force clear server context.",
                    BT_HID_WARN_LOG);
            mServerState = BluetoothHid.BT_HID_STATE_DISACTIVE;
            forceClearServerNative();
            sendServiceMsg(BluetoothHid.MBTEVT_HID_HOST_DISABLE_FAIL, null);
        }
        if (mSocketListener != null) {
            try {
                printLog("mSocketListener close.", BT_HID_DEBUG_LOG);
                mSocketListener.shutdown();
                mSocketListener.join();
                mSocketListener = null;
                printLog("mSocketListener close OK.", BT_HID_DEBUG_LOG);
            } catch (InterruptedException e) {
                printLog("mSocketListener close error.", BT_HID_ERROR_LOG);
            }
        }

        if (mConnectTimeout != null) {
            try {
                printLog("mConnectTimeout close.", BT_HID_INFO_LOG);
                mConnectTimeout.shutdown();
                mConnectTimeout.join();
                mConnectTimeout = null;
                printLog("mConnectTimeout close OK.", BT_HID_INFO_LOG);
            } catch (InterruptedException e) {
                printLog("mConnectTimeout close error.", BT_HID_ERROR_LOG);
            }
        }

        if (mDisconnectTimeout != null) {
            try {
                printLog("mDisconnectTimeout close.", BT_HID_INFO_LOG);
                mDisconnectTimeout.shutdown();
                mDisconnectTimeout.join();
                mDisconnectTimeout = null;
                printLog("mDisconnectTimeout close OK.", BT_HID_INFO_LOG);
            } catch (InterruptedException e) {
                printLog("mDisconnectTimeout close error.", BT_HID_ERROR_LOG);
            }
        }

        stopListentoSocketNative();
        cleanServiceNative();
    }

    void localCreateService() {
        if (mServerState != BluetoothHid.BT_HID_STATE_ACTIVE) {
            mUpdateStateIntent.putExtra(BluetoothProfileManager.EXTRA_PROFILE,
                    BluetoothProfileManager.Profile.Bluetooth_HID);
            mUpdateStateIntent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                    BluetoothProfileManager.STATE_ENABLING);
            //M:Support multi-user
            //sendBroadcast(mUpdateStateIntent);
            mCx.sendBroadcastAsUser(mUpdateStateIntent,UserHandle.ALL);
            if (initServiceNative()) {
                printLog("Succeed to init BluetoothHidService.", BT_HID_DEBUG_LOG);
                if (mSocketListener == null) {
                    mSocketListener = new SocketListenerThread();
                    mSocketListener.setName("BTHidSocketListener");
                    mSocketListener.mStopped = false;
                    mSocketListener.start();
                    printLog("SocketListener started.", BT_HID_DEBUG_LOG);
                }
                sServiceDisable = false;
                /* Default values for HID server settings */
                mServerState = BluetoothHid.BT_HID_STATE_DISACTIVE;
                serverActivateReqNative();
                printLog("Pre-enable HID Server", BT_HID_DEBUG_LOG);
            } else {
                printLog("Failed to init BluetoothHidService.", BT_HID_DEBUG_LOG);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        String action = intent.getAction();
        printLog("Enter onBind(): " + action, BT_HID_INFO_LOG);
        if (IBluetoothHid.class.getName().equals(action)) {
            return mHid;

        } else {
            return mHidServerNotify;
        }
    }

    /* Utility function: printLog */
    private void printLog(String msg, int type) {
        if (DEBUG) {
            switch (type) {
                case BT_HID_DEBUG_LOG:
                    Xlog.d(TAG, msg);
                    break;
                case BT_HID_ERROR_LOG:
                    Xlog.e(TAG, msg);
                    break;
                case BT_HID_INFO_LOG:
                    Xlog.i(TAG, msg);
                    break;
                case BT_HID_VERB_LOG:
                    Xlog.v(TAG, msg);
                    break;
                case BT_HID_WARN_LOG:
                    Xlog.w(TAG, msg);
                    break;
                default:
                    break;
            }
        }
    }

    private void updateDeviceState(String deviceAddr, String state) {
        /*
         * if (mStateMap.containsKey(deviceAddr)) {
         * mStateMap.remove(deviceAddr); }
         */
        mStateMap.put(deviceAddr, state);
    }

    /* Handler associated with the main thread */
    private Handler mServiceHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            printLog("handleMessage(): " + msg.what, BT_HID_INFO_LOG);
            int notifyID = 0;
            Bundle data = msg.getData();
            String deviceAddr = (data != null) ? data.getString(BluetoothHid.DEVICE_ADDR) : null;
            String deviceName = new String();

            Notification noti = null;
            if (deviceAddr != null) {
                mPC = BluetoothHidActivity.getDeviceList();
                if (mPC != null) {
                    mPreference = mPC.findPreference(deviceAddr);
                }

                BluetoothDevice mBD = getBluetoothDevice(deviceAddr);
                if (mBD != null) {
                    deviceName = mBD.getName();
                    if (deviceName == null) {
                        deviceName = getDeviceName(deviceAddr);
                    }
                } else {
                    deviceName = getDeviceName(deviceAddr);
                    if (deviceName == null && mPreference != null) {
                        deviceName = mPreference.getTitle().toString();
                    }
                }

            }

            switch (msg.what) {
                case BluetoothHid.MBTEVT_HID_HOST_ENABLE_SUCCESS:
                    mServerState = BluetoothHid.BT_HID_STATE_ACTIVE;
                    mUpdateStateIntent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                            BluetoothProfileManager.STATE_ENABLED);
                    //M:Support multi-user
                    //sendBroadcast(mUpdateStateIntent);
                    mCx.sendBroadcastAsUser(mUpdateStateIntent,UserHandle.ALL);
                    Intent bindIntent = new Intent(BluetoothInputDevice.ACTION_BIND_SERVICE);
                    //sendBroadcast(bindIntent);
                    mCx.sendBroadcastAsUser(bindIntent,UserHandle.ALL);
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_ENABLE_FAIL:
                    mUpdateStateIntent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                            BluetoothProfileManager.STATE_ABNORMAL);
                    //sendBroadcast(mUpdateStateIntent);
                    mCx.sendBroadcastAsUser(mUpdateStateIntent,UserHandle.ALL);
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_DISABLE_SUCCESS:
                    mServerState = BluetoothHid.BT_HID_STATE_DISACTIVE;
                    mUpdateStateIntent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                            BluetoothProfileManager.STATE_DISABLED);
                    //sendBroadcast(mUpdateStateIntent);
                    mCx.sendBroadcastAsUser(mUpdateStateIntent,UserHandle.ALL);
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_DISABLE_FAIL:
                    mUpdateStateIntent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                            BluetoothProfileManager.STATE_ABNORMAL);
                   // sendBroadcast(mUpdateStateIntent);
                    mCx.sendBroadcastAsUser(mUpdateStateIntent,UserHandle.ALL);
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_CONNECT_SUCCESS:
                    Toast.makeText(mCx, getString(R.string.bluetooth_hid_connect_ok, deviceName),
                            Toast.LENGTH_LONG).show();
                    mServerState = BluetoothHid.BT_HID_STATE_CONNECTED;
                    updateSettingsState(BluetoothHid.BT_HID_DEVICE_CONNECT,
                            getBluetoothDevice(deviceAddr));
                    updateDeviceState(deviceAddr, BluetoothHid.BT_HID_DEVICE_CONNECT);
                    /*
                     * if(mPreference!=null){ mPreference.setSummary(R.string
                     * .bluetooth_hid_summary_connected);
                     * mPreference.setEnabled(true); }
                     */
                    updateActivityUI(deviceAddr, R.string.bluetooth_hid_summary_connected, true);
                    if (!mNotifyMap.containsKey(deviceAddr)) {
                        mNotifyMap.put(deviceAddr, sHidConnectNotify);
                        sHidConnectNotify++;
                    }

                    notifyID = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                    noti = genHidNotification(notifyID, deviceName, deviceAddr,
                            BluetoothHid.BT_HID_DEVICE_CONNECT, false);
                  //M:Support multi-user
                   // mNM.notify(notifyID, noti);
                    mNM.notifyAsUser(null,notifyID, noti,UserHandle.ALL);

                    if (mConnectTimeout != null) {
                        try {
                            printLog("mConnectTimeout close.", BT_HID_INFO_LOG);
                            mConnectTimeout.shutdown();
                            mConnectTimeout.join();
                            mConnectTimeout = null;
                            printLog("mConnectTimeout close OK.", BT_HID_INFO_LOG);
                        } catch (InterruptedException e) {
                            printLog("mConnectTimeout close error.", BT_HID_ERROR_LOG);
                        }
                    }

                    break;

                case BluetoothHid.MBTEVT_HID_HOST_CONNECT_FAIL:
                    if (mStateMap.containsKey(deviceAddr)) {
                        if (mStateMap.get(deviceAddr).equals(BluetoothHid.BT_HID_DEVICE_CONNECTING)
                                || mStateMap.get(deviceAddr).equals(
                                        BluetoothHid.BT_HID_DEVICE_AUTHORIZE)) {
                            Toast.makeText(mCx,
                                    getString(R.string.bluetooth_hid_connect_fail, deviceName),
                                    Toast.LENGTH_LONG).show();
                            updateSettingsState(BluetoothHid.BT_HID_DEVICE_DISCONNECT,
                                    getBluetoothDevice(deviceAddr));
                            updateDeviceState(deviceAddr, BluetoothHid.BT_HID_DEVICE_DISCONNECT);
                            /*
                             * if(mPreference!=null){ mPreference.setSummary(
                             * R.string.bluetooth_hid_summary_not_connected );
                             * mPreference.setEnabled(true); }
                             */
                            updateActivityUI(deviceAddr,
                                    R.string.bluetooth_hid_summary_not_connected, true);
                        }
                    }

                    if (mConnectTimeout != null) {
                        try {
                            printLog("mConnectTimeout close.", BT_HID_INFO_LOG);
                            mConnectTimeout.shutdown();
                            mConnectTimeout.join();
                            mConnectTimeout = null;
                            printLog("mConnectTimeout close OK.", BT_HID_INFO_LOG);
                        } catch (InterruptedException e) {
                            printLog("mConnectTimeout close error.", BT_HID_ERROR_LOG);
                        }
                    }
                    if (mNotifyMap.containsKey(deviceAddr)) {
                        notifyID = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                    }
                    //M:Support multi-user
                    //mNM.cancel(notifyID);
                    mNM.cancelAsUser(null,notifyID,UserHandle.ALL);

                    break;

                case BluetoothHid.MBTEVT_HID_HOST_DISCONNECT_SUCCESS:
                    if (!sServiceDisable) {
                        Toast.makeText(mCx,
                                getString(R.string.bluetooth_hid_disconnect_ok, deviceName),
                                Toast.LENGTH_LONG).show();
                    }
                    mServerState = BluetoothHid.BT_HID_STATE_DISCONNECTED;

                    if (mStateMap.containsKey(deviceAddr)) {
                        if (mStateMap.get(deviceAddr).equals(BluetoothHid.BT_HID_DEVICE_UNPLUG)) {
                            updateSettingsState(BluetoothHid.BT_HID_DEVICE_DISCONNECT,
                                    getBluetoothDevice(deviceAddr));
                            /*
                             * mStateMap.remove(deviceAddr);
                             * mStateMap.put(deviceAddr,
                             * BluetoothHid.BT_HID_DEVICE_UNPLUG_DISCONNECT);
                             */
                            updateDeviceState(deviceAddr,
                                    BluetoothHid.BT_HID_DEVICE_UNPLUG_DISCONNECT);
                            if (mPreference != null) {
                                mPC.removePreference(mPreference);
                            }
                        } else {
                            updateSettingsState(BluetoothHid.BT_HID_DEVICE_DISCONNECT,
                                    getBluetoothDevice(deviceAddr));
                            /*
                             * mStateMap.remove(deviceAddr);
                             * mStateMap.put(deviceAddr,
                             * BluetoothHid.BT_HID_DEVICE_DISCONNECT);
                             */
                            updateDeviceState(deviceAddr, BluetoothHid.BT_HID_DEVICE_DISCONNECT);
                            if (mNotifyMap.containsKey(deviceAddr)) {
                                notifyID = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                                //mNM.cancel(notifyID);
                                mNM.cancelAsUser(null,notifyID,UserHandle.ALL);
                            }
                            /*
                             * if(mPreference!=null){ mPreference.setSummary(
                             * R.string.bluetooth_hid_summary_not_connected );
                             * mPreference.setEnabled(true); }
                             */
                            updateActivityUI(deviceAddr,
                                    R.string.bluetooth_hid_summary_not_connected, true);
                        }
                    }

                    if (mDisconnectTimeout != null) {
                        try {
                            printLog("mDisconnectTimeout close.", BT_HID_INFO_LOG);
                            mDisconnectTimeout.shutdown();
                            mDisconnectTimeout.join();
                            mDisconnectTimeout = null;
                            printLog("mDisconnectTimeout close OK.", BT_HID_INFO_LOG);
                        } catch (InterruptedException e) {
                            printLog("mDisconnectTimeout close error.", BT_HID_ERROR_LOG);
                        }
                    }

                    break;

                case BluetoothHid.MBTEVT_HID_HOST_DISCONNECT_FAIL:
                    if (mStateMap.containsKey(deviceAddr)) {
                        if (mStateMap.get(deviceAddr).equals(
                                BluetoothHid.BT_HID_DEVICE_DISCONNECTING)
                                || mStateMap.get(deviceAddr).equals(
                                        BluetoothHid.BT_HID_DEVICE_UNPLUG)) {
                            if (!sServiceDisable) {
                                Toast.makeText(
                                        mCx,
                                        getString(R.string.bluetooth_hid_disconnect_fail,
                                                deviceName), Toast.LENGTH_LONG).show();
                            }

                            updateSettingsState(BluetoothHid.BT_HID_DEVICE_CONNECT,
                                    getBluetoothDevice(deviceAddr));
                            updateDeviceState(deviceAddr, BluetoothHid.BT_HID_DEVICE_CONNECT);
                            /*
                             * if(mPreference!=null){ mPreference.setSummary(
                             * R.string.bluetooth_hid_summary_connected);
                             * mPreference.setEnabled(true); }
                             */
                            updateActivityUI(deviceAddr, R.string.bluetooth_hid_summary_connected,
                                    true);
                        }
                    }

                    if (mDisconnectTimeout != null) {
                        try {
                            printLog("mDisconnectTimeout close.", BT_HID_INFO_LOG);
                            mDisconnectTimeout.shutdown();
                            mDisconnectTimeout.join();
                            mDisconnectTimeout = null;
                            printLog("mDisconnectTimeout close OK.", BT_HID_INFO_LOG);
                        } catch (InterruptedException e) {
                            printLog("mDisconnectTimeout close error.", BT_HID_ERROR_LOG);
                        }
                    }
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_SEND_CONTROL_SUCCESS:
                    Toast.makeText(mCx, getString(R.string.bluetooth_hid_unplug_ok, deviceName),
                            Toast.LENGTH_LONG).show();
                    mServerState = BluetoothHid.BT_HID_STATE_DISCONNECTED;
                    updateDeviceState(deviceAddr, BluetoothHid.BT_HID_DEVICE_UNPLUG);

                    if (mNotifyMap.containsKey(deviceAddr)) {
                        notifyID = Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                        //mNM.cancel(notifyID);
                        mNM.cancelAsUser(null,notifyID,UserHandle.ALL);
                    }
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_SEND_CONTROL_FAIL:
                    Toast.makeText(mCx, getString(R.string.bluetooth_hid_unplug_fail, deviceName),
                            Toast.LENGTH_LONG).show();
                    break;

                case BluetoothHid.MBTEVT_HID_HOST_RECEIVE_AUTHORIZE:
                    if (!mNotifyMap.containsKey(deviceAddr)) {
                        mNotifyMap.put(deviceAddr, sHidConnectNotify);
                        sHidConnectNotify++;
                    }

                    updateSettingsState(BluetoothHid.BT_HID_DEVICE_CONNECTING,
                            getBluetoothDevice(deviceAddr));
                    updateDeviceState(deviceAddr, BluetoothHid.BT_HID_DEVICE_AUTHORIZE);
                    /*
                     * if(mPreference!=null){ mPreference.setSummary(R.string
                     * .bluetooth_hid_summary_connecting);
                     * mPreference.setEnabled(false); }
                     */

                    updateActivityUI(deviceAddr, R.string.bluetooth_hid_summary_connecting, false);
                    /*
                     * notifyID =
                     * Integer.parseInt(mNotifyMap.get(deviceAddr).toString());
                     * noti = genHidNotification(notifyID, deviceName,
                     * deviceAddr, BluetoothHid.BT_HID_DEVICE_AUTHORIZE, true);
                     * mNM.notify(notifyID, noti);
                     */
                    serverAuthorizeReqNative(deviceAddr, true);
                    break;

                default:
                    break;
            }

            if (BluetoothHid.MBTEVT_HID_HOST_CONNECT_SUCCESS == msg.what
                    || BluetoothHid.MBTEVT_HID_HOST_CONNECT_FAIL == msg.what
                    || BluetoothHid.MBTEVT_HID_HOST_DISCONNECT_SUCCESS == msg.what
                    || BluetoothHid.MBTEVT_HID_HOST_DISCONNECT_FAIL == msg.what
                    || BluetoothHid.MBTEVT_HID_HOST_SEND_CONTROL_SUCCESS == msg.what
                    || BluetoothHid.MBTEVT_HID_HOST_SEND_CONTROL_FAIL == msg.what) {
                Intent intent = new Intent(FINISH_ACTION);
                //sendBroadcast(intent);
                mCx.sendBroadcastAsUser(intent,UserHandle.ALL);
            }

        }
    };

    /* Utility function: sendServiceMsg */
    private void sendServiceMsg(int what, String addr) {
        Message msg = Message.obtain();

        printLog("sendServiceMsg status=" + what + "address=" + addr, BT_HID_INFO_LOG);
        if (what == BluetoothHid.MBTEVT_HID_HOST_DISABLE_SUCCESS
                || what == BluetoothHid.MBTEVT_HID_HOST_DISABLE_FAIL) {
            mServerState = BluetoothHid.BT_HID_STATE_DISACTIVE;
        }

        if (what == BluetoothHid.MBTEVT_HID_HOST_ENABLE_SUCCESS) {
            mServerState = BluetoothHid.BT_HID_STATE_ACTIVE;
        }

        msg.what = what;

        Bundle data = new Bundle();
        data.putString(BluetoothHid.DEVICE_ADDR, addr);
        msg.setData(data);

        mServiceHandler.sendMessage(msg);
    }

    /* Utility function: genHidNotification */
    private Notification genHidNotification(int type, String deviceName, String deviceaddr,
            String action, boolean needSound) {

        Context context = getApplicationContext();
        Intent tmpIntent = new Intent();
        Notification tmpNoti = null;
        PendingIntent tmpContentIntent = null;
        int iconID = -1;
        String clazz = null;
        String ticker = null;
        String title = null;
        printLog("genHidNotification " + deviceaddr, BT_HID_INFO_LOG);

        iconID = R.drawable.bthid_ic_notify_wireless_keyboard;
        clazz = BluetoothHidAlert.class.getName();
        tmpIntent.setClassName(getPackageName(), clazz).putExtra(BluetoothHid.DEVICE_ADDR,
                deviceaddr);

        if (action.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
            ticker = getString(R.string.bluetooth_hid_connected_notify_ticker);
            title = getString(R.string.bluetooth_hid_connected_notify_title);
            tmpNoti = new Notification(iconID, ticker, System.currentTimeMillis());
            tmpNoti.flags = Notification.FLAG_ONGOING_EVENT;

            tmpIntent.putExtra(BluetoothHid.ACTION, BluetoothHid.BT_HID_DEVICE_DISCONNECT);
            tmpContentIntent = PendingIntent.getActivity(getApplicationContext(), type, tmpIntent,
                    PendingIntent.FLAG_CANCEL_CURRENT);
            tmpNoti.setLatestEventInfo(context, title, getString(
                    R.string.bluetooth_hid_connected_notify_message, deviceName), tmpContentIntent);
        } else if (action.equals(BluetoothHid.BT_HID_DEVICE_AUTHORIZE)) {
            ticker = getString(R.string.bluetooth_hid_connect_request_notify_ticker);
            title = getString(R.string.bluetooth_hid_connect_request_notify_title);
            tmpNoti = new Notification(iconID, ticker, System.currentTimeMillis());
            tmpNoti.flags = Notification.FLAG_ONLY_ALERT_ONCE;
            if (needSound) {
                tmpNoti.defaults |= Notification.DEFAULT_SOUND;
                tmpNoti.defaults |= Notification.DEFAULT_VIBRATE;
            }

            tmpIntent.putExtra(BluetoothHid.ACTION, BluetoothHid.BT_HID_DEVICE_AUTHORIZE);
            tmpContentIntent = PendingIntent.getActivity(getApplicationContext(), type, tmpIntent,
                    PendingIntent.FLAG_CANCEL_CURRENT);
            tmpNoti.setLatestEventInfo(context, title, getString(
                    R.string.bluetooth_hid_connect_request_notify_message, deviceName),
                    tmpContentIntent);
        }

        return tmpNoti;
    }

    private BluetoothDevice getBluetoothDevice(String btAddr) {

        return BluetoothAdapter.getDefaultAdapter().getRemoteDevice(btAddr);

    }

    private String getDeviceName(String btAddr) {
        SharedPreferences settings = getSharedPreferences(BT_HID_SETTING_INFO, 0);

        int preIndex = 0;
        int preferenceCount = settings.getInt("preferenceCount", 0);

        for (preIndex = 0; preIndex < preferenceCount; preIndex++) {
            String tmpAddr = settings.getString("deviceAddr" + Integer.toString(preIndex),
                    BT_HID_NOT_FOUNT);
            if (tmpAddr.equals(btAddr)) {
                return settings.getString("deviceName" + Integer.toString(preIndex),
                        BT_HID_NOT_FOUNT);
            }
        }
        return null;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int notifyID = 0;
            Notification noti = null;
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            if (device == null) {
                printLog("ERROR: device null", BT_HID_ERROR_LOG);
                return;
            }

            String deviceAddr = device.getAddress();
            String deviceName = device.getName();
            String state = getDeviceState(deviceAddr);
            String notifyStr = null;
            if (mNotifyMap.containsKey(deviceAddr)) {
                notifyStr = mNotifyMap.get(deviceAddr).toString();
            }
            if (notifyStr == null) {
                printLog("ERROR: notify_s null", BT_HID_ERROR_LOG);
                return;
            }
            if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(intent.getAction())) {
                int bondedState = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                        BluetoothDevice.ERROR);
                if (bondedState == BluetoothDevice.BOND_NONE) {
                    if (deviceAddr != null) {
                        mPC = BluetoothHidActivity.getDeviceList();
                        if (mPC != null) {
                            mPreference = mPC.findPreference(deviceAddr);
                        }

                        if (mPreference != null) {
                            mPC.removePreference(mPreference);
                        }
                    }
                }

            }
            if (state != null && BluetoothDevice.ACTION_NAME_CHANGED.equals(intent.getAction())) {
                if (state.equals(BluetoothHid.BT_HID_DEVICE_CONNECT)) {
                    notifyID = Integer.parseInt(notifyStr);
                    noti = genHidNotification(notifyID, deviceName, deviceAddr,
                            BluetoothHid.BT_HID_DEVICE_CONNECT, false);
                    //mNM.notify(notifyID, noti);
                    mNM.notifyAsUser(null,notifyID, noti,UserHandle.ALL);
                } else if (state.equals(BluetoothHid.BT_HID_DEVICE_AUTHORIZE)) {
                    notifyID = Integer.parseInt(notifyStr);
                    noti = genHidNotification(notifyID, deviceName, deviceAddr,
                            BluetoothHid.BT_HID_DEVICE_AUTHORIZE, false);
                    //mNM.notify(notifyID, noti);
                    mNM.notifyAsUser(null,notifyID, noti,UserHandle.ALL);
                }

            }
        }
    };
}
