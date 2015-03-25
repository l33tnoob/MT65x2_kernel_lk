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
package mediatek.net.wfd;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplay;
import android.hardware.display.WifiDisplayStatus;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiMonitor;
import android.net.wifi.p2p.*;
import android.provider.Settings;
import android.test.AndroidTestCase;
import android.util.Log;
import android.os.Messenger;
import android.os.Message;
import android.os.RemoteException;

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.Map;

  

public class WifidisplayFwkTest extends AndroidTestCase {
	private static final String TAG = "WifidisplayFwkTest";
	
    private static class MySync {
        int expectedState = STATE_NULL;
    }

    private static class MySyncWfd {
        int expectedWfdState = STATE_NULL;
    }

    private WifiP2pManager mWifiP2pManager;
    private WifiManager mWifiManager;
    //for wifi enable/disable only
    private static MySync mMySync;
    private static MySyncWfd mMySyncWfd;

    private IntentFilter mIntentFilter;
    private WifiP2pManager.Channel mChannel;
    private Messenger messenger;
    private WifidisplayFwkTest mWmtThis;
    
    private DisplayManager mDisplayManager;
    private WifiDisplayStatus mWifiDisplayStatus;

    // state enum
    private static final int STATE_NULL = 0;
    private static final int STATE_WIFIP2P_CHANGING = 1;
    private static final int STATE_WIFIP2P_CHANGED = 2;
    private static final int STATE_DISCOVERING = 3;
    private static final int STATE_DISCOVER_PEERS_AVAILABLE = 4;
    private static final int STATE_NOT_GOT_PEERS_INFO = 5;
    private static final int STATE_GOT_PEERS_INFO = 6;
    private static final int STATE_STOPPING_DISCOVER = 7;
    private static final int STATE_DISCOVER_STOPPED = 8;
    
    private static final int STATE_CONNECT_NULL = 9;
    private static final int STATE_CONNECTED = 10;
    private static final int STATE_GOT_CONNECYEDINFO = 11;
    private static final int STATE_GROUPINFO = 12;
    private static final int STATE_THIS_CHANGED = 13;
    private static final int STATE_INVITE_SUCCESS = 14;
    private static final int STATE_CANCEL_CONNECT_SUCCESS = 15;
    
    private static final int STATE_CREATING_GROUP = 16;
    private static final int STATE_CREATE_GROUP_OK = 17;
    private static final int STATE_REMOVING_GROUP = 18;
    private static final int STATE_REMOVE_GROUP_OK = 19;
    
    private static final int STATE_CLEARING_ALL_SERVICE = 20;
    private static final int STATE_CLEAR_ALL_SERVICE_OK = 21;
    private static final int STATE_SET_SERVICE_LISTENER_OK = 22;
    private static final int STATE_SET_DNS_LISTENER_OK = 23;
    private static final int STATE_SET_UPNP_LISTENER_OK = 24;
    
    private static final int STATE_REQUESTING = 25;
    private static final int STATE_REQUEST_PPER_OK = 26;
    private static final int STATE_REQUEST_CONNECTION_INFO_OK = 27;
    private static final int STATE_REQUEST_GROUP_INFO_OK = 28;
    
    private static final int STATE_SETTING_DEV_NAME = 29;
    private static final int STATE_SET_DEV_NAME_OK = 30;

    
    private static final int TIMEOUT_MSEC = 5000;
    private static final int WAIT_MSEC = 60;
    private static final int DURATION = 2000;
    
    private static final String mWfdPeerString = "P2P-DEVICE-FOUND 02:0c:43:26:60:40 p2p_dev_addr=02:0c:43:26:60:40 pri_dev_type=8-0050F204-2 name='WFD_DONGLE_266040' config_methods=0x188 dev_capab=0x24 group_capab=0x18 wfd_dev_info=0x0000060111022a0064";
    private static final String mWfdGroupCreatedString = "P2P-GROUP-STARTED p2p0 client ssid=\"DIRECT-BwWFD_DONGLE_266040\" freq=2437 psk=8781123cf06b534bb1ebebd386bc0bea0256e1532048d15ca07f276588353e2b go_dev_addr=02:0c:43:26:60:40";
    //private static final String mGroupCreatedString = "P2P-GROUP-STARTED p2p0 GO ssid=\"DIRECT-vj-B\" freq=2462 passphrase=\"OsCn2cFd\" go_dev_addr=96:db:c9:7c:f5:ee [PERSISTENT]";
    //private static final String mApStaConnectedString = "AP-STA-DISCONNECTED 96:db:c9:7c:f1:e8";

    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (action.equals(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED)) {
                Log.d(TAG, "onReceive(): ACTION_WIFI_DISPLAY_STATUS_CHANGED");

                synchronized (mMySyncWfd){
                    //Log.d(TAG, "DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED");
                    WifiDisplayStatus status = (WifiDisplayStatus)intent.getParcelableExtra(
                            DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                    mWifiDisplayStatus = status;

                    final WifiDisplay[] displays = mWifiDisplayStatus.getDisplays();
                    
                    for (WifiDisplay d : displays) {
                        //scan then found!
                        if (d.isAvailable()) {
                            Log.d(TAG, "available device is " + d.getFriendlyDisplayName());
                            if (d.getDeviceAddress().equals("02:0c:43:26:60:40")) {
                                if (STATE_DISCOVERING == mMySyncWfd.expectedWfdState) {
                                    mMySyncWfd.expectedWfdState = STATE_DISCOVER_PEERS_AVAILABLE;
                                    mMySyncWfd.notify();
                                }
                            }
                        }

                        //do connect then connected
                        if (d.isRemembered()){
                            Log.d(TAG, "paired device is " + d.getFriendlyDisplayName());
                            if (d.getDeviceAddress().equals("02:0c:43:26:60:40")) {
                                mMySyncWfd.expectedWfdState = STATE_CONNECTED;
                                mMySyncWfd.notify();                        
                            }
                        }
                        
                    }                   

                }

            }// end of DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED
        }
    };
    
    @Override
    protected void setUp() throws Exception {
        Log.d(TAG, "setUp()");
    	super.setUp();
    	mWmtThis = this;
    	mMySync = new MySync();
    	mMySyncWfd = new MySyncWfd();
        mIntentFilter = new IntentFilter();
        
        mIntentFilter.addAction(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
        
        mContext.registerReceiver(mReceiver, mIntentFilter);
    	mWifiP2pManager = (WifiP2pManager)getContext().getSystemService(Context.WIFI_P2P_SERVICE);
    	mWifiManager = (WifiManager) getContext().getSystemService(Context.WIFI_SERVICE);
    	mDisplayManager = (DisplayManager) getContext().getSystemService(Context.DISPLAY_SERVICE);

    	messenger = mWifiP2pManager.getMessenger();
    	assertNotNull("Failed to get WiFi P2P service!",mWifiP2pManager);
    	assertNotNull("Failed to get WiFi service!",mWifiP2pManager);
    	mChannel = mWifiP2pManager.initialize(mContext, mContext.getMainLooper(), null);
    	assertNotNull("Failed to get wifi p2p channel!",mChannel);
    	if (!mWifiManager.isWifiEnabled()) {
    		setWifiEnabled(true);
    		Thread.sleep(DURATION);
    	}
        assertTrue(mWifiManager.isWifiEnabled());
    }
    
    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "tearDown()");
        mContext.unregisterReceiver(mReceiver);
        if (mWifiManager.isWifiEnabled()) {
            setWifiEnabled(false);
            Thread.sleep(DURATION);
        } 
        super.tearDown();
    }
    
    
    private void setWifiEnabled(boolean enable) throws Exception {
    	if (enable == true){
        	synchronized (mMySync){
        		mMySync.expectedState = STATE_WIFIP2P_CHANGING;
        		assertTrue(mWifiManager.setWifiEnabled(enable));
        		long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
                while (System.currentTimeMillis() < timeout
                        && mMySync.expectedState == STATE_WIFIP2P_CHANGING)
                    mMySync.wait(WAIT_MSEC);
        	}
    	}
    	else{
        	synchronized (mMySync){
        		mMySync.expectedState = STATE_WIFIP2P_CHANGING;
        		assertTrue(mWifiManager.setWifiEnabled(enable));
        		long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
                while (System.currentTimeMillis() < timeout
                        && mMySync.expectedState == STATE_WIFIP2P_CHANGING)
                    mMySync.wait(WAIT_MSEC);
        	}
    	}

    }
    
    private void sendMessage(int what, Object obj) {
        Message msg = Message.obtain();
        msg.what = what;
        msg.obj = obj;
        assertNotNull("Failed to Messenger!",messenger);
        try {
            messenger.send(msg);      
        } catch (RemoteException e) {
            Log.e(TAG, "Send message error:" + e);
        }

    }
    
    private void startWfdSearch()throws Exception {
        Log.d(TAG, "startWfdSearch()");
    	synchronized (mMySyncWfd){
    		mMySyncWfd.expectedWfdState = STATE_DISCOVERING;
            mDisplayManager.startWifiDisplayScan();

    		createPeer(mWfdPeerString);
    		long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySyncWfd.expectedWfdState == STATE_DISCOVERING)
                mMySyncWfd.wait(WAIT_MSEC);
    	}
    }
    
    private void createPeer(String peerStr) {
    	WifiP2pDevice peer = new WifiP2pDevice(peerStr);
    	sendMessage(WifiMonitor.P2P_DEVICE_FOUND_EVENT, peer);
    	
    }
    
    private void connectWfdPeer()throws Exception {
        Log.d(TAG, "connectWfdPeer()");
        synchronized (mMySyncWfd){
            mMySyncWfd.expectedWfdState = STATE_CONNECT_NULL;

            mDisplayManager.connectWifiDisplay("02:0c:43:26:60:40");
            Log.d(TAG, "dongle MAC is " + "02:0c:43:26:60:40");

            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySyncWfd.expectedWfdState == STATE_CONNECT_NULL) {
                mMySyncWfd.wait(DURATION);
                //do fake Connected
                Log.e(TAG, "do fake Connected");
                mMySyncWfd.expectedWfdState = STATE_CONNECTED;
            }
        }
    }

    private void connectWfdPeerDone()throws Exception {
        //disconnect when this case is done
        mDisplayManager.disconnectWifiDisplay();
    }

    private void disConnectWfdPeer()throws Exception {
        Log.d(TAG, "disConnectWfdPeer()");
        synchronized (mMySyncWfd){
            mMySyncWfd.expectedWfdState = STATE_CONNECTED;

            mDisplayManager.disconnectWifiDisplay();

            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySyncWfd.expectedWfdState == STATE_CONNECTED) {
                mMySyncWfd.wait(DURATION);
                //do fake DisConnected
                Log.e(TAG, "do fake DisConnected");
                mMySyncWfd.expectedWfdState = STATE_CONNECT_NULL;
            }
        }

    }

//=======================================================
//==========TEST CASE START==============================
//=======================================================

    public void test001Enable() throws Exception {
        Log.d(TAG, "test001Enable()");
    	mMySyncWfd.expectedWfdState = STATE_NULL;
    	
    	//enable wfd
    	if (WifiDisplayStatus.FEATURE_STATE_ON != mDisplayManager.getWifiDisplayStatus().getFeatureState()) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.WIFI_DISPLAY_ON, 1);
    	    Thread.sleep(DURATION);
    	}
        assertTrue("Enable failed", 
            WifiDisplayStatus.FEATURE_STATE_ON==mDisplayManager.getWifiDisplayStatus().getFeatureState());
    }

    public void test002Search() throws Exception {
        Log.d(TAG, "test002Search()");
    	mMySyncWfd.expectedWfdState = STATE_NULL;
        
        //enable wfd
        if (WifiDisplayStatus.FEATURE_STATE_ON != mDisplayManager.getWifiDisplayStatus().getFeatureState()) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.WIFI_DISPLAY_ON, 1);
            Thread.sleep(DURATION);
        }
        
    	//search and get peers
    	assertNotNull("mDisplayManager is null!", mDisplayManager);
    	startWfdSearch();

    	assertTrue("Search failed", mMySyncWfd.expectedWfdState==STATE_DISCOVER_PEERS_AVAILABLE);
    }

    public void test003Connect() throws Exception {
        Log.d(TAG, "test003Connect()");
    	mMySyncWfd.expectedWfdState = STATE_NULL;
        
        //enable wfd
        if (WifiDisplayStatus.FEATURE_STATE_ON != mDisplayManager.getWifiDisplayStatus().getFeatureState()) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.WIFI_DISPLAY_ON, 1);
            Thread.sleep(DURATION);
        }
        
        //search and get peers
    	//assertNotNull("mDisplayManager is null!", mDisplayManager);
    	//startWfdSearch();
        //Log.d(TAG, "mMySyncWfd.expectedWfdState:" + mMySyncWfd.expectedWfdState);
    	
        //connect peer
    	connectWfdPeer();
        Log.d(TAG, "expectedWfdState is " + mMySyncWfd.expectedWfdState);
        
        assertTrue("Timeout, failed to connect peers", mMySyncWfd.expectedWfdState==STATE_CONNECTED);
        connectWfdPeerDone();
    }

    public void test004DisConnect() throws Exception {
        Log.d(TAG, "test004DisConnect()");
        mMySyncWfd.expectedWfdState = STATE_NULL;
        
        //enable wfd
        if (WifiDisplayStatus.FEATURE_STATE_ON != mDisplayManager.getWifiDisplayStatus().getFeatureState()) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.WIFI_DISPLAY_ON, 1);
            Thread.sleep(DURATION);
        }
        
        //search and get peers
        //assertNotNull("mDisplayManager is null!", mDisplayManager);
        //startWfdSearch();

        //connect peer
        connectWfdPeer();
        Log.d(TAG, "after connectWfdPeer(), expectedWfdState is " + mMySyncWfd.expectedWfdState);    
        assertTrue("Timeout, failed to connect peers", mMySyncWfd.expectedWfdState==STATE_CONNECTED); 

        //disconnect peer
        disConnectWfdPeer();
        Log.d(TAG, "after disConnectWfdPeer(), expectedWfdState is " + mMySyncWfd.expectedWfdState);    
        assertTrue("Timeout, failed to disconnect peers", mMySyncWfd.expectedWfdState==STATE_CONNECT_NULL); 
    }

    public void test005Disable() throws Exception {
        Log.d(TAG, "test005Disable()");
        mMySyncWfd.expectedWfdState = STATE_NULL;

        //enable wfd
        if (WifiDisplayStatus.FEATURE_STATE_ON != mDisplayManager.getWifiDisplayStatus().getFeatureState()) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.WIFI_DISPLAY_ON, 1);
            Thread.sleep(DURATION);
        }
        
        //disable wfd
        if (WifiDisplayStatus.FEATURE_STATE_ON == mDisplayManager.getWifiDisplayStatus().getFeatureState()) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.WIFI_DISPLAY_ON, 0);
            Thread.sleep(DURATION);
        }
        assertTrue("Disable failed", 
            WifiDisplayStatus.FEATURE_STATE_ON!=mDisplayManager.getWifiDisplayStatus().getFeatureState());
    }


}//end of class

