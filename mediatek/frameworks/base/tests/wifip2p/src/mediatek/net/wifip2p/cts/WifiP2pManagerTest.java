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
package mediatek.net.wifip2p.cts;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration.Status;
import android.net.wifi.WifiManager.WifiLock;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.WifiMonitor;
import android.net.wifi.p2p.*;
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




public class WifiP2pManagerTest extends AndroidTestCase {
	private static final String TAG = "WifiP2pManagerTest";
	
    private static class MySync {
        int expectedState = STATE_NULL;
 
    }
    private static class MySyncConnect {
        int expectedConnectState = STATE_CONNECT_NULL;
    }
    
    private WifiP2pManager mWifiP2pManager;
    private WifiManager mWifiManager;
    private static MySync mMySync;
    private static MySyncConnect mMySyncConnect;
    private IntentFilter mIntentFilter;
    private WifiP2pManager.Channel mChannel;
    private Messenger messenger;
    private WifiP2pDeviceList mPeers = new WifiP2pDeviceList();
    private WifiP2pManagerTest mWmtThis;
    private WifiP2pDeviceList mConnectPeers = new WifiP2pDeviceList();
    private WifiP2pConfig config;
    
    private static final int FIRST_PEER_NO = 0;
    private static final int CONNECT_PEER_COUNT = 2;
    
    private static final int INVITE_FIRST_PEER = 1;
    private static final int INVITE_SECOND_PEER = 2;
    
    private int wifiP2pState = WifiP2pManager.WIFI_P2P_STATE_DISABLED;
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
    
    private static final int TIMEOUT_MSEC = 6000;
    private static final int WAIT_MSEC = 60;
    private static final int DURATION = 10000;
    private static final int WAITRESULT = 1000;
    
    private static final String mPeerString = "P2P-DEVICE-FOUND 96:db:c9:7c:f1:e8 p2p_dev_addr=96:db:c9:7c:f1:e8 pri_dev_type=10-0050F204-5 name='A' config_methods=0x188 dev_capab=0x27 group_capab=0x0 go_iface=00:00:00:00:00:00 wfd_en=1 wfd_info=0x3 wfd_ctrl_port=0 wfd_tp=0 wfd_assoc_mac=00:00:00:00:00:00 wfd_sink_status=0x0 wfd_sink_mac=00:00:00:00:00:00 wfd_extend_capa=0x0 wfd_flags=0x1";
    private static final String mProvisionString = "P2P-PROV-DISC-PBC-RESP 96:db:c9:7c:f1:e8";
    private static final String mGroupCreatedString = "P2P-GROUP-STARTED p2p0 GO ssid=\"DIRECT-vj-B\" freq=2462 passphrase=\"OsCn2cFd\" go_dev_addr=96:db:c9:7c:f5:ee [PERSISTENT]";
    private static final String mApStaConnectedString = "AP-STA-DISCONNECTED 96:db:c9:7c:f1:e8";
    
    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (action.equals(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION)) {
                synchronized (mMySync) {
                    mMySync.expectedState = STATE_WIFIP2P_CHANGED;
                    mMySync.notify();
                }
                    
            } else if (action.equals(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION)) {
                synchronized (mMySyncConnect){
                    mMySyncConnect.expectedConnectState = STATE_THIS_CHANGED;
                    mMySyncConnect.notify();
                }
            }  
        }
    };
    
    @Override
    protected void setUp() throws Exception {
    	super.setUp();
    	mWmtThis = this;
    	mMySync = new MySync();
    	mMySyncConnect = new MySyncConnect();
    	config = new WifiP2pConfig();
        mIntentFilter = new IntentFilter();
        
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        
        mContext.registerReceiver(mReceiver, mIntentFilter);
    	mWifiP2pManager = (WifiP2pManager)getContext().getSystemService(Context.WIFI_P2P_SERVICE);
    	mWifiManager = (WifiManager) getContext().getSystemService(Context.WIFI_SERVICE);
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
    
    private void startSearch()throws Exception {
    	synchronized (mMySync){
    		mMySync.expectedState = STATE_DISCOVERING;		
    		mWifiP2pManager.discoverPeers(mChannel, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                	Log.d(TAG, " discover success");
                	mMySync.expectedState = STATE_DISCOVER_PEERS_AVAILABLE;
                }
                public void onFailure(int reason) {
                    Log.d(TAG, " discover fail " + reason);
                }
            });
    		createPeer(mPeerString);
    		long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_DISCOVERING)
                mMySync.wait(WAIT_MSEC);
    	}
    }
    
    private void createPeer(String peerStr) {
    	WifiP2pDevice peer = new WifiP2pDevice(peerStr);
    	sendMessage(WifiMonitor.P2P_DEVICE_FOUND_EVENT, peer);
    	
    }
    
    private void stopSearch()throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_STOPPING_DISCOVER;
            mWifiP2pManager.stopPeerDiscovery(mChannel, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                    Log.d(TAG, "Stop discover success");
                    mMySync.expectedState = STATE_DISCOVER_STOPPED;
                }
                public void onFailure(int reason) {
                    Log.d(TAG, "Stop discover fail " + reason);
                }
            });
            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_STOPPING_DISCOVER)
                mMySync.wait(WAIT_MSEC);
        }
    }
    
    private void connectPeer()throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_CONNECT_NULL;
            WifiP2pConfig config = new WifiP2pConfig();
            config.deviceAddress = "96:db:c9:7c:f1:e8";
            mWifiP2pManager.connect(mChannel, config, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                    Log.d(TAG, "Connect success");
                    mMySync.expectedState = STATE_CONNECTED;
                    Log.d(TAG, "expectedState1 is " + mMySync.expectedState);
                }
                public void onFailure(int reason) {
                    Log.d(TAG, "Connect failed " + reason);
                }
            });
            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_CONNECT_NULL)
                mMySync.wait(WAIT_MSEC);
        }
        
        Log.d(TAG, "config is " + config.toString());
    }
    
    private void createGroup()throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_CREATING_GROUP;
            mWifiP2pManager.createGroup(mChannel, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                    Log.d(TAG, "Create group success");
                    mMySync.expectedState = STATE_CREATE_GROUP_OK;
                }
                public void onFailure(int reason) {
                    Log.d(TAG, "Create Group failed " + reason);
                }
            });
            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_CREATING_GROUP)
                mMySync.wait(WAIT_MSEC);
        }
    }

    private void removeGroup() throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_REMOVING_GROUP;
            mWifiP2pManager.removeGroup(mChannel, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                    Log.d(TAG, "remove group success");
                    mMySync.expectedState = STATE_REMOVE_GROUP_OK;
                }
                public void onFailure(int reason) {
                    Log.d(TAG, "remove Group failed " + reason);
                }
            });
            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_REMOVING_GROUP)
                mMySync.wait(WAIT_MSEC);
        }
    }
    
    private void clearLocalServices() throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_CLEARING_ALL_SERVICE;
            mWifiP2pManager.clearLocalServices(mChannel, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                    Log.d(TAG, "clear all service success");
                    mMySync.expectedState = STATE_CLEAR_ALL_SERVICE_OK;
                }
                public void onFailure(int reason) {
                    Log.d(TAG, "clear all service failed " + reason);
                }
            });
            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_CLEARING_ALL_SERVICE)
                mMySync.wait(WAIT_MSEC);
        }
    }
    
    private void setServiceResponseListener()throws Exception {
        synchronized (mMySync){   
            mWifiP2pManager.setServiceResponseListener(mChannel, new WifiP2pManager.ServiceResponseListener() {
                public void onServiceAvailable(int protocolType,
                        byte[] responseData, WifiP2pDevice srcDevice) {
                    Log.d(TAG, "set Service Response Listener OK");
                }
            });
            mMySync.expectedState = STATE_SET_SERVICE_LISTENER_OK;
        }
    }
    
    private void setDnsSdResponseListeners()throws Exception {
        synchronized (mMySync){   
            mWifiP2pManager.setDnsSdResponseListeners(mChannel, new WifiP2pManager.DnsSdServiceResponseListener() {
                public void onDnsSdServiceAvailable(String instanceName,
                        String registrationType, WifiP2pDevice srcDevice) {
                    Log.d(TAG, "set DNS Service Response Listener OK1");
                }
            }, new WifiP2pManager.DnsSdTxtRecordListener() {
                public void onDnsSdTxtRecordAvailable(String fullDomainName,
                        Map<String, String> txtRecordMap,
                        WifiP2pDevice srcDevice) {
                    Log.d(TAG, "set DNS Service Response Listener OK2");
                }
            });
            mMySync.expectedState = STATE_SET_DNS_LISTENER_OK;
        }
    }
     
    private void setUpnpServiceResponseListener() throws Exception {
        synchronized (mMySync){   
            mWifiP2pManager.setUpnpServiceResponseListener(mChannel, new WifiP2pManager.UpnpServiceResponseListener() {
                public void onUpnpServiceAvailable(List<String> uniqueServiceNames,
                        WifiP2pDevice srcDevice) {
                    Log.d(TAG, "set UPNP Service Response Listener OK");
                }
            });
            mMySync.expectedState = STATE_SET_UPNP_LISTENER_OK;
        }
    }
    
    private void requestPeers() throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_REQUESTING;
            mWifiP2pManager.requestPeers(mChannel, new WifiP2pManager.PeerListListener() {
                public void onPeersAvailable(WifiP2pDeviceList peers) {
                    Log.d(TAG, "request peer info OK is " + peers);
                    mMySync.expectedState = STATE_REQUEST_PPER_OK;
                }
            });            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_REQUESTING)
                mMySync.wait(WAIT_MSEC);
        }
    }
    
    private void requestConnectionInfo() throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_REQUESTING;
            mWifiP2pManager.requestConnectionInfo(mChannel, new WifiP2pManager.ConnectionInfoListener() {
                public void onConnectionInfoAvailable(WifiP2pInfo info) {
                    Log.d(TAG, "request connect info OK is " + info);
                    mMySync.expectedState = STATE_REQUEST_CONNECTION_INFO_OK;
                }
            });            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_REQUESTING)
                mMySync.wait(WAIT_MSEC);
        }
    }
    
    private void requestGroupInfo() throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_REQUESTING;
            mWifiP2pManager.requestGroupInfo(mChannel, new WifiP2pManager.GroupInfoListener() {
                public void onGroupInfoAvailable(WifiP2pGroup group) {
                    Log.d(TAG, "request group info OK is " + group);
                    mMySync.expectedState = STATE_REQUEST_GROUP_INFO_OK;
                }
            });            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_REQUESTING)
                mMySync.wait(WAIT_MSEC);
        }
    }
    
    public void setDeviceName() throws Exception {
        synchronized (mMySync){
            mMySync.expectedState = STATE_SETTING_DEV_NAME;
            mWifiP2pManager.setDeviceName(mChannel, "AA AA", new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                    Log.d(TAG, "Set device name success");
                    mMySync.expectedState = STATE_SET_DEV_NAME_OK;
                }
                public void onFailure(int reason) {
                    Log.d(TAG, "Set device name  failed " + reason);
                }
            });
            
            long timeout = System.currentTimeMillis() + TIMEOUT_MSEC;
            while (System.currentTimeMillis() < timeout
                    && mMySync.expectedState == STATE_SETTING_DEV_NAME)
                mMySync.wait(WAIT_MSEC);
        }
    }
 
    private String toString(WifiP2pDevice d) {
        StringBuffer sbuf = new StringBuffer();
        int count = 0;
        if ((d.deviceName != null) || (d.deviceAddress != null)
          || (d.primaryDeviceType != null) ||(d.secondaryDeviceType != null)
          || (d.wpsConfigMethodsSupported < 0)
        	|| (d.groupCapability < 0) || (d.deviceCapability < 0)
        	|| (d.status < 0)) {
        	sbuf.append("Device: ").append(d.deviceName);
        	sbuf.append("\n deviceAddress: ").append(d.deviceAddress);
        	sbuf.append("\n primary type: ").append(d.primaryDeviceType);
        	sbuf.append("\n secondary type: ").append(d.secondaryDeviceType);
            sbuf.append("\n wps: ").append(d.wpsConfigMethodsSupported);
            sbuf.append("\n grpcapab: ").append(d.groupCapability);
            sbuf.append("\n devcapab: ").append(d.deviceCapability);
            sbuf.append("\n status: ").append(d.status);
            count += 1;
        }
        if (count == 0) {
        	return null;
        }
               
        return sbuf.toString();
    }
   
    
    public void searchAndGetPeers() throws Exception {	
    	//test search peers
    	assertNotNull("mWifiP2pManager is null before start search peers!",mWifiP2pManager);
    	startSearch();
    	assertTrue("Timeout, failed to disvoverpeers",mMySync.expectedState == STATE_DISCOVER_PEERS_AVAILABLE);
       	assertNotNull("mWifiP2pManager is null  before get peers Info!",mWifiP2pManager);   
        
        //test request peers
        mMySync.expectedState = STATE_NOT_GOT_PEERS_INFO;
	    mWifiP2pManager.requestPeers(mChannel, new WifiP2pManager.PeerListListener() {
	    	public void onPeersAvailable(WifiP2pDeviceList peers) {
                mMySync.expectedState = STATE_GOT_PEERS_INFO;
	    	}	
	    }); 
	    
	    Thread.sleep(WAITRESULT);
	    assertTrue("Failed to get searched peers Info",mMySync.expectedState == STATE_GOT_PEERS_INFO); 
    }
   
    public void test001SearchAndStopSearch() throws Exception {
    	mMySync.expectedState = STATE_NULL;
    	
    	//search and get peers
    	searchAndGetPeers();
    	assertNotNull("mWifiP2pManager is null  before connect peers!",mWifiP2pManager);
    	
    	stopSearch();
	    assertTrue("Timeout, failed to stop discover peers",mMySync.expectedState == STATE_DISCOVER_STOPPED); 
    }
    
    public void test002Connect() throws Exception {
        searchAndGetPeers();
        assertNotNull("mWifiP2pManager is null  before connect peers!",mWifiP2pManager);
        connectPeer();
        Log.d(TAG, "expectedState3 is " + mMySync.expectedState);
        assertTrue("Timeout, failed to connect peers",mMySync.expectedState == STATE_CONNECTED); 
    }
    
    public void test003CreateGroup() throws Exception {
        createGroup();
        assertTrue("Timeout, failed to create group",mMySync.expectedState == STATE_CREATE_GROUP_OK);    
    }
    
    public void test004ClearLocalServices() throws Exception {
        clearLocalServices();
        assertTrue("Timeout, failed to clear all local service",mMySync.expectedState == STATE_CLEAR_ALL_SERVICE_OK);    
    }
    
    private void test005SetServiceListener() throws Exception {
        setServiceResponseListener();
        assertTrue("Set service listener failed",mMySync.expectedState == STATE_SET_SERVICE_LISTENER_OK);    
    }
    
    public void test006SetDnsSdResponseListeners() throws Exception {
        setDnsSdResponseListeners();
        assertTrue("Set DNS service listener failed",mMySync.expectedState == STATE_SET_DNS_LISTENER_OK); 
    }
    
    public void test007SetUpnpServiceResponseListener() throws Exception {
        setUpnpServiceResponseListener();
        assertTrue("Set DNS service listener failed",mMySync.expectedState == STATE_SET_UPNP_LISTENER_OK); 
    } 
    
    public void test008RequestInfo() throws Exception {
        requestPeers();
        assertTrue("request peers failed",mMySync.expectedState == STATE_REQUEST_PPER_OK); 
        
        requestConnectionInfo();
        assertTrue("request connection info failed",mMySync.expectedState == STATE_REQUEST_CONNECTION_INFO_OK); 
        
        requestGroupInfo();
        assertTrue("request group info failed",mMySync.expectedState == STATE_REQUEST_GROUP_INFO_OK); 
    }
    
    public void test009SetDeviceName() throws Exception {
        setDeviceName();
        assertTrue("Set dev name failed",mMySync.expectedState == STATE_SET_DEV_NAME_OK); 
    }
    
    public void test010OtherTest() throws Exception {
        /*start test wifip2pdevice*/
        WifiP2pDevice device1 = new WifiP2pDevice();
        WifiP2pDevice device2 = new WifiP2pDevice();
        
        boolean result = false;
        result = device1.equals(device2);
        assertTrue("Device1 equals device2 ",result == true); 
        result = false;
        
        result = device1.isGroupOwner();
        assertTrue("Device1 is GO ",result == false); 
        result = false;
        
        result = device1.isServiceDiscoveryCapable();
        assertTrue("Device1 has service discovery capability  ",result == false); 
        result = false;
        
        result = device1.wpsPbcSupported();
        assertTrue("Device1 support pbc ",result == false); 
        result = false;
        
        result = device1.wpsKeypadSupported();
        assertTrue("Device1 support keypad ",result == false); 
        result = false;
        
        result = device1.wpsDisplaySupported();
        assertTrue("Device1 support display ",result == false); 
        result = false;
        
        Log.d(TAG, "device1: " + device1);
        /*end test wifip2pdevice*/
        
        /*start to test wifip2pdevicelist*/
        WifiP2pDeviceList  list = new WifiP2pDeviceList();
        list.getDeviceList();
        Log.d(TAG, "list : " + list);
        /*end test wifip2pdeivcelist*/
        
        WifiP2pGroup group = new WifiP2pGroup();
        group.getClientList();
        group.getInterface();
        group.getNetworkName();
        group.getOwner();
        group.getPassphrase();
        group.isGroupOwner();
        Log.d(TAG, "group : " + group);
        
    } 
}//end of class




























