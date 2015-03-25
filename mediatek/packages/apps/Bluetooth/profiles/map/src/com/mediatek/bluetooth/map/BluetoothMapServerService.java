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

package com.mediatek.bluetooth.map;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemService;
import android.util.Log;
import android.app.Service;

import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import android.bluetooth.BluetoothMap;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.IBluetoothMap;

import android.os.RemoteCallbackList;
import android.os.RemoteException;

import com.mediatek.bluetooth.map.IBluetoothMapSetting;
import com.mediatek.bluetooth.map.IBluetoothMapSettingCallback;
import com.mediatek.bluetooth.map.cache.EventReport;
import com.mediatek.bluetooth.map.cache.BMessageObject;
import com.mediatek.bluetooth.map.cache.FolderListObject;
import com.mediatek.bluetooth.map.cache.MessageListObject;
import com.mediatek.bluetooth.map.cache.*;
import com.mediatek.bluetooth.map.MAP;
import com.mediatek.bluetooth.map.BluetoothMapNotification;
import com.mediatek.bluetooth.map.InstanceManager;
import com.mediatek.bluetooth.map.Instance;
import com.mediatek.bluetooth.map.util.InstanceUtil;
import com.mediatek.bluetooth.map.util.NetworkUtil;

import java.util.HashMap;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import com.mediatek.xlog.Xlog;

public class BluetoothMapServerService extends Service  implements EventChannel{
	private final String TAG = "BluetoothMapServerService";

	private final int MAP_SERVER_ENABLE_CNF         = 0;
	private final int MAP_SERVER_DISABLE_CNF        = 1;
	private final int MAP_SERVER_REGISTER_CNF       = 2;
	private final int MAP_SERVER_DEREGISTER_CNF     = 3;
	private final int MAP_SERVER_CONNECT_IND        = 4;
	private final int MAP_SERVER_DISCONNECT_IND     = 5;
	private final int MAP_SERVER_DISCONNECT_CNF     = 6;
	private final int MAP_SERVER_SET_FOLDER         = 7;
	private final int MAP_SERVER_UPDATE_INBOX       = 8;
	private final int MAP_SERVER_GET_MESSAGE_LIST   = 9;
	private final int MAP_SERVER_GET_FOLDER_LIST    = 10;
	private final int MAP_SERVER_GET_MESSAGE        = 11;
	private final int MAP_SERVER_SET_NOTIFICATION   = 12;
	private final int MAP_SERVER_SEND_REPORT_CNF    = 13;
	private final int MAP_SERVER_AUTHORIZE_IND	  	= 14; 
	private final int MAP_SERVER_PUSH_MESSAGE		= 15;
	private final int MAP_SERVER_SET_MESSAGE_STATUS	= 16;
	private final int MAP_SERVER_MNS_DISCONNCET_CNF = 17;
	private final int MAP_SERVER_MNS_CONNECT_CNF    = 18;
	private final int MAP_REGISTER_SERVER			= 100;	
	//when disabling MAP server and device is connected, we have to disconnect device first and then disable server 
	private final int MAP_DISCONNECT_INSTANCE_TIMEOUT	= 101;

	private final int REGISTER_SERVER_DELAY_DURATION = 1000; //1s
	private final int DISCONNECT_INSTANCE_DELAY_DURATION = 1000; //500ms

	private static final String SERVICE_ENABLE = "map server enable setting";
	/*the service state only contains enabling, enabled,disabling and disabled*/
	/*each instance state info can be obtained from instache manager*/
	private int mState; 
	private boolean mInit = false;
	private boolean mForceToDeinit = false;

	private int mNativeData;

	//TODO: expose multi instance
	private Instance mInstance; 

	private EventReportManager mEventManager;

	private InstanceManager mManager;
	private BluetoothMapNotification mNotification;

	private Context mContext;
	private BluetoothAdapter mAdapter;

	private final int SUCCESS   = 0;
	private final int FAIL		= -1;
	private final int PENDING	= 1;

	private final Handler mHandler = new Handler() {
		private int mRegSrvCnt = 0;
     //   @Override
        public void handleMessage(Message msg) {
        	Instance instance;
			ArrayList<Instance> instances;
        	int result = MAP.RESULT_ERROR;
			String addr;
			BluetoothDevice device = null;
        	log("message received: "+ msg.what);
        	
        	switch(msg.what) {
        		case MAP_SERVER_ENABLE_CNF:
					/*to do: set state*/				
					setState(BluetoothMap.STATE_ENABLED);
					sendMessage(obtainMessage(MAP_REGISTER_SERVER));
					onStateChanged(BluetoothMap.STATE_ENABLED);
					break;
				case MAP_SERVER_DISABLE_CNF:
					/*to do: set state*/
					mManager.removeAllInstances();
					setState(BluetoothMap.STATE_DISABLED);
					onStateChanged(BluetoothMap.STATE_DISABLED);
					deinitServer();
					break;
				case MAP_SERVER_REGISTER_CNF:
					instance = mManager.getInstanceById(msg.arg1);
					if(msg.arg2 >= 0 && instance != null) {
						instance.onInstanceRegistered();
					} else {
						mManager.removeInstance(msg.arg1);
					}
					break;
				case MAP_SERVER_DEREGISTER_CNF:
					instance = mManager.getInstanceById(msg.arg1);
					if(instance != null) {
						instance.onInstanceDeregistered();
					} 
					mManager.removeInstance(msg.arg1);
					break;					
				case MAP_SERVER_CONNECT_IND:
					instance = mManager.getInstanceById(msg.arg1);					
					device = mAdapter.getRemoteDevice((String)msg.obj);
					if(instance != null) {
						instance.onDeviceConnected(device);
					} else {
						log("invalid instance ID is received");
						return;
					}

					mNotification.createNotification(BluetoothMapNotification.ALERT_TYPE_CONNECT,
													device, true);
					break;				
				case MAP_SERVER_DISCONNECT_CNF:
				case MAP_SERVER_DISCONNECT_IND:
					instance = mManager.getInstanceById(msg.arg1);
					device = mAdapter.getRemoteDevice((String)msg.obj);
					if(instance != null) {
						instance.onDeviceDisconnected(device);
					} else {
						log("invalid instance ID is received:"+msg.arg1);
					}
					if (mManager.getState(device) == BluetoothMap.STATE_DISCONNECTED) {
						mNotification.removeNotification(BluetoothMapNotification.ALERT_TYPE_CONNECT, device);
					}
					if (mState == BluetoothMap.STATE_DISABLING) {
						removeMessages(MAP_DISCONNECT_INSTANCE_TIMEOUT, instance);
						disable(false);
					}
					break;					
				case MAP_SERVER_SET_FOLDER:					
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("SetFolderRequest or instance is null");
						return;
					}
					SetFolderRequest setFolderReq = (SetFolderRequest)msg.obj;
					result = instance.setFolder(setFolderReq);
					setFolderResponse(setFolderReq.getAddress(), msg.arg1, result);
					break;
				case MAP_SERVER_UPDATE_INBOX:
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address or instance is null");
						return;
					}
					addr = (String)msg.obj;
					result = instance.updateInbox();
					updateInboxResponse(addr,msg.arg1,result);
					break;	
					
				case MAP_SERVER_GET_MESSAGE_LIST:
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					MessageListRequest msgListReq = (MessageListRequest) msg.obj;
					MessageListObject msgListRsp = instance.getMessagelist(msgListReq);
					addr = msgListReq.getAddress();
					getMessageListResponse(addr, msg.arg1, MAP.RESULT_OK, msgListRsp);					
					break;
					
					
				case MAP_SERVER_GET_FOLDER_LIST:					
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					FolderListObject[] fodlerlistrsp = null;
					FolderListRequest folderListReq = (FolderListRequest) msg.obj;					
					addr = folderListReq.getAddress();
					device = mAdapter.getRemoteDevice(addr);
					if (device != null && device.equals(instance.getDevice())) {
						fodlerlistrsp = instance.getFolderlist(folderListReq);						
					}
					getFolderListResponse(addr, msg.arg1, MAP.RESULT_OK, fodlerlistrsp);					
					break;
				case MAP_SERVER_GET_MESSAGE:
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					MessageRequest messagereq = (MessageRequest) msg.obj;
					BMessageObject messagersp = instance.getMessage(messagereq);
					if (messagersp != null) {
						result = MAP.RESULT_OK;
					}
					addr = messagereq.getAddress();
					getMessageResponse(addr, msg.arg1,result, messagersp);	
				/*	if (messagersp != null) {
					messagersp.reset();
					} */
					break;
				case MAP_SERVER_SET_NOTIFICATION:					
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					result = MAP.RESULT_OK;
					setNotificationRegResponse((String)msg.obj, msg.arg1, result);
					break;
				case MAP_SERVER_SEND_REPORT_CNF:	
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					mEventManager.onEventSentCompleted();
					break;
				case MAP_SERVER_AUTHORIZE_IND:
					//TODO: launch an alert to notify user that a device is trying to access local resource
					if(msg.obj == null) {
						log("address is null");
						return;
					}
					device = mAdapter.getRemoteDevice((String) msg.obj);
					int state = mNotification.getDeviceState(device);
					if (state == BluetoothMap.STATE_NONE) {
						mNotification.createNotification(BluetoothMapNotification.ALERT_TYPE_AUTHORIZE
											,device, true);
					} else if (state == BluetoothMap.STATE_CONNECTED) {
						//already connected
						authorizeResponse(device, true);
					} else if (state == BluetoothMap.STATE_AUTHORIZING) {
						log("the device is authorizing");
						//do nothing
					} else {
						log("unexpected state : "+state);
						authorizeResponse(device, false);						
					}
					break;
				case MAP_SERVER_PUSH_MESSAGE:
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					BMessageObject bMessageobject = instance.getBMessageObject();
					bMessageobject.reset();
					if (parseMessage((String) msg.obj, msg.arg1, bMessageobject) == false){
						result = MAP.RESULT_ERROR;
					}
					else
					{
						if(instance.pushMessage(bMessageobject))
						{
							result = MAP.RESULT_OK;
						}
						else
						{
							result = MAP.RESULT_ERROR;							
						}
					}
					sendMessageResponse((String) msg.obj, msg.arg1 ,result, bMessageobject.getHandle());
					break;
		/*		case MAP_SERVER_DELETE_MESSAGE:
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					//BMessageObject bMessageobject = instance.getBMessageObject();
					//sendMessageResponse((String) msg.obj, msg.arg1 ,bMessageobject);
					if(instance.deleteMessage(msg.arg1, msg.arg2)){
						result = MAP.RESULT_OK;
					} else {
						result = MAP.RESULT_ERROR;
					}
					setMessageStatusResponse((String)msg.obj, msg.arg1, result);
					break;
					*/
				case MAP_SERVER_SET_MESSAGE_STATUS:
					instance = mManager.getInstanceById(msg.arg1);
					if (msg.obj == null || instance == null) {
						log("address is null or instance is null");
						return;
					}
					StatusSwitchRequest req = (StatusSwitchRequest)msg.obj;
					if(instance.setMessageStatus(req)) {
						result = MAP.RESULT_OK;
					} else {
						result = MAP.RESULT_ERROR;
					}
					addr = req.getAddress();
					setMessageStatusResponse(addr, msg.arg1, result);
					break;
				case MAP_SERVER_MNS_DISCONNCET_CNF:
					device = mAdapter.getRemoteDevice((String) msg.obj);
					instances = mManager.getInstanceByDevice(device);
					for(int i = 0; i < instances.size(); i++) {
						instances.get(i).deregisterCallback();
					}
					if (mState == BluetoothMap.STATE_DISABLING) {
						disable(false);
					}
					break;
				case MAP_SERVER_MNS_CONNECT_CNF:
					if (msg.arg1 == MAP.RESULT_ERROR) {
						log("fail to set up mns connection");
					} else {
						device = mAdapter.getRemoteDevice((String) msg.obj);
						instances = mManager.getInstanceByDevice(device);
						for(int i = 0; i < instances.size(); i++) {
							instances.get(i).registerCallback(mEventManager);
						}
					}
				break;	
				case MAP_REGISTER_SERVER:
					if(NetworkUtil.isPhoneRadioReady()) {
						registerServer();
					} else {
						mRegSrvCnt ++;
						if (mRegSrvCnt < 10) {
							sendMessageDelayed(obtainMessage(MAP_REGISTER_SERVER), REGISTER_SERVER_DELAY_DURATION);
						} else {
							log("Network is still not ready after 10s, so phone radio maybe is in error");
						}
					}
					break;
				case MAP_DISCONNECT_INSTANCE_TIMEOUT:
					instance = (Instance)msg.obj;
					if (instance != null) {
						instance.onDeviceDisconnected(instance.getDevice());
						if (mState == BluetoothMap.STATE_DISABLING) {
							disable(false);
						}
					}
					break;
				default:
					log("Unrecognized message");					

			}
		}
	};
	

	private BroadcastReceiver  mReceiver = new BroadcastReceiver(){
		 @Override
        public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			log("receive intent:"+action);
			if (action.equals(BluetoothMap.ACTION_AUTHORIZE_RESULT)) {
				boolean allow = intent.getBooleanExtra(BluetoothMap.EXTRA_RESULT, false);
				BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
				
				authorizeResponse(device, allow);
			} else if (action.equals(BluetoothMap.ACTION_DISCONNECT_DEVICE)) {
				BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
				disconnect(device);
			} else if(action.equals(BluetoothDevice.ACTION_NAME_CHANGED)) {
				BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
				int state = mNotification.getDeviceState(device);
				if (state == BluetoothMap.STATE_AUTHORIZING) {
					mNotification.createNotification(BluetoothMapNotification.ALERT_TYPE_AUTHORIZE, 
													device,false);
				} else if (state == BluetoothMap.STATE_CONNECTED) {
					mNotification.createNotification(BluetoothMapNotification.ALERT_TYPE_CONNECT, 
													device,false);
				}
			} else if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
				int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,BluetoothDevice.BOND_NONE);
				BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
				if (state == BluetoothDevice.BOND_NONE && mManager.isConnected(device)) {
					disconnect(device);
				}				
			} else if (action.equals(BluetoothMapNotification.ACTION_MAP_CLEAR_NOTIFICATION)) {
				clearAuthorizingDevices();
			} else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
				//when proxy binds to the service using BIND_AUTO_CREATE, the onDestroy will not be called
				int state = BluetoothAdapter.getDefaultAdapter().getState();
				if (state == BluetoothAdapter.STATE_TURNING_OFF) {
					clearService();
				} else if (state == BluetoothAdapter.STATE_ON ) {
					initService();
				}

			}
		}

	};

		
	public void onCreate() {
		log("MAP: onCreate...");
	//	mContext = getApplicationContext();
	//	mMessageListContainer = new HashMap<int, MessageList>();
	//	mFolderListContainer  = new HashMap<int, FolderList>();
		
	//	createInstance();	
		mManager = InstanceManager.getDefaultManager(this);
		mNotification = new BluetoothMapNotification(this);
		mEventManager = new EventReportManager(this);
		mAdapter = BluetoothAdapter.getDefaultAdapter();
		IntentFilter filter = new IntentFilter();
		filter.addAction(BluetoothMap.ACTION_AUTHORIZE_RESULT);
		filter.addAction(BluetoothMap.ACTION_DISCONNECT_DEVICE);
		filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
		filter.addAction(BluetoothMapNotification.ACTION_MAP_CLEAR_NOTIFICATION);
		filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
		registerReceiver(mReceiver, filter);
	}

	public int onStartCommand(Intent intent, int flags, int startId) {
		int ret = super.onStartCommand(intent, flags, startId);
		initService();
		return ret;
	}
	@Override
	public IBinder onBind(Intent intent) {
		String action = intent.getAction();
		log("onbind(): action "+action);
		log(action);
		if (action.equals(IBluetoothMap.class.getName())) {
			return mBinder;
		} else if (action.equals(IBluetoothMapSetting.class.getName())){
			return mSettingBinder;
		}
		return null;
	}
	//service seems no onStop
	public void onStop() {
		log("onStop");
		disable();
		return;
	}
	@Override
	public void onDestroy(){
		super.onDestroy();
		unregisterReceiver(mReceiver);
		clearService();
	}
	@Override
	public void onLowMemory(){
		super.onLowMemory();
		log("onLowMemory: sytem may try to tighten belt");
	}
	

	/***proxy method for advanced settings******/
	private synchronized void registerServer() {
		log("registerServer()");
		if (!isEnabled()) {
			log("map service has not been initialized");
			return;
		}
		int index;
		ArrayList<Instance> instances = mManager.generateInstances(this);
		for(index = 0; index < instances.size();index++) {

			registerInstance(instances.get(index));			
		}		
	}
	


	//get messagelist cache to store request to get message list
	private MessageListRequest getMessageListCache(int masId) {	
		Instance instance = mManager.getInstanceById(masId);
		if (instance != null) {
			return instance.getMessageListCache();
		} else {
			return null;
		}
	}

	//get folderlist cache to store request to get folder list
	private FolderListRequest getFolderListCache(int masId) {
		Instance instance = mManager.getInstanceById(masId);
		if (instance != null) {
			return instance.getFolderListReqCache();
		} else {
			return null;
		}
	}

	private SetFolderRequest  getFolderCache(int masId) {

		Instance instance = mManager.getInstanceById(masId);
		if (instance != null) {
			return instance.getFolderReqCache();
		} else {
			return null;
		}
	}
	private MessageRequest getMessageRequestCache(int masId){
		Instance instance = mManager.getInstanceById(masId);
		if (instance != null) {
			return instance.getMessageReqCache();
		} else {
			return null;
		}
	}
 	private StatusSwitchRequest getStatusSwitchCache(){

		log("getStatusSwitchCache()");
		return new StatusSwitchRequest();
		
	}

	///*package*/void registerInstance(int instanceId, String name, int messageType) {
	private synchronized boolean registerInstance(Instance instance) {
		int id = instance.getInstanceId();
		String masName = instance.getName();
		int messageType = instance.getType();		
		String rootPath = instance.getRootPath(); 

		log("registerInstance():id is "+id+", name is "+ masName+", type is "+messageType);
		//it is allowed to set id as zero in MAP profile.
		//but not permitted in externall layer.
		//so we will follow external rule temprorily
		if (id < 0 || id > 255) {
			log("the id is invalid");
			return false;

		}
		if (masName == null || rootPath == null) {
			log("error, the mas name or root path is null");
			return false;
		}
		if (isEnabled()){
			registerInstanceNative(id, masName, messageType, rootPath);
			return true;
		} else {

			log("MAP service has not been initialized");
			return false;
		}
	}

	private synchronized boolean deregisterInstance(Instance instance) {
		int id = instance.getInstanceId();
		boolean ret = false;
		log("deregisterInstance: id="+id);
		if (getState() == BluetoothMap.STATE_DISABLED || id > 255 || id < 0) {
			log("fail to deregister instance");
			return false;
		}

	/*	if (instance.isMasConnected()) {
			disconnectMasNative(id);
		} */		
		deregisterInstanceNative(id);
		return true;
	}
	private synchronized boolean authorizeResponse(BluetoothDevice device, boolean accept) {
		boolean ret = false;
		//check the server state
		log("authorizeResponse: accept="+accept);
		if (getState() == BluetoothMap.STATE_ENABLED ||
			getState() == BluetoothMap.STATE_DISABLING) {			
			int result = (accept == true) ? MAP.RESULT_OK : MAP.RESULT_ERROR;

			mNotification.removeNotification(BluetoothMapNotification.ALERT_TYPE_AUTHORIZE,device);
			
			authorizeResponseNative(device.getAddress(),result);
			ret = true;
		} else {
			log("MAP service has not been initialized");
			ret = false;
		}
		return ret;
	}

	private void clearAuthorizingDevices() {
		HashSet<BluetoothDevice> devices= mNotification.getAuthoringDevices();
		Iterator iterator = devices.iterator();
		if (iterator.hasNext()){
			authorizeResponse((BluetoothDevice)iterator.next(),false);
		}
	}
	
	private synchronized boolean disconnect(BluetoothDevice device) {
		ArrayList<Instance> instances = mManager.getInstanceByDevice(device);
		for (int i = 0; instances != null && i < instances.size(); i++) {
			disconnectMasNative(instances.get(i).getInstanceId());
		}
		return true;		
	}

	/* Description:Disconnect MAS connection if exist*/
	private synchronized int disconnectServer() {
		log("disconnectServer()");
		int index;
		ArrayList<Instance> instances = mManager.getAllInstances();

		clearAuthorizingDevices();
		
		if (instances == null || instances.size() == 0) {
			return SUCCESS;
		}
		for (index = 0; index < instances.size(); index++) {
			if (instances.get(index).isMasConnected()) {
				if(PENDING == disconnectMasSession(instances.get(index))) {	
					mHandler.sendMessageDelayed(mHandler.obtainMessage(MAP_DISCONNECT_INSTANCE_TIMEOUT, instances.get(index)),
										DISCONNECT_INSTANCE_DELAY_DURATION);			
					return PENDING;
				}
			}
		}

		/*wait MNS connection is disconnected*/
		for (index = 0; index < instances.size(); index++) {
			if (instances.get(index).isMnsConnected()) {
				return PENDING;
			}
		}

		return SUCCESS;
	}

	private synchronized int disconnectMasSession(Instance instance) {
		int result = FAIL;
		if (instance == null || !instance.isMasConnected()) 
		{
			return result;
		}

		result = disconnectMasNative(instance.getInstanceId());
		if (result == SUCCESS) {
			instance.onDeviceDisconnecting(instance.getDevice());
			return PENDING;
		} else {
			instance.onDeviceDisconnected(instance.getDevice());
			return SUCCESS;
		}
	}
		
	private synchronized void disconnectMnsSession(BluetoothDevice device){
		String address = device.getAddress();
		log("disconnectMnsSession(): "+ address);
		if(address == null) {
			log("the address is null");
			return;
		}
		disconnectMnsSessionNative(address);
	}
	
	
	public synchronized boolean sendReport(BluetoothDevice device,EventReport report) {
		String address = device.getAddress();
		log("sendReport(): name is "+ device.getName()+",address="+address);

		if (address == null) {
			log("the address is null");
			return false;
		}
		if (isEnabled()) {
			sendReportNative(address, report);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}

	private synchronized boolean updateInboxResponse(String address, int masId,int result) {

		log("updateInboxResponse(): result="+result);
		if (isEnabled()) {
			updateInboxResponseNative(address, masId, result);
			return true;
		} else {
			return false;
		}

	}
	private synchronized boolean setNotificationRegResponse(String address, int masId,int result) {

			log("setNotificationRegResponse(): result="+result);
			if (isEnabled()) {				
				setNotifRegResponseNative(address, masId, result);
				return true;
			} else {
				return false;
			}
	
	}

	private synchronized boolean parseMessage(String address, int masId,BMessageObject bMessage) {
		log("parseMessage: address->"+address+",masId->"+masId);
	
		if (isEnabled()) {
			if(parseMessageNative(address, masId, bMessage) < 0)
			{
				return false;
			}
			else
			{
				return true;
			}
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}
	
	private synchronized boolean sendMessageResponse(String address, int masId,int result, long handle) {
		log("sendMessageResponse: address->"+address+",masId->"+masId);
		
		if (isEnabled()) {
			sendMessageResponseNative(address, masId, result, handle);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}
	private synchronized boolean getFolderListResponse(String address, int masId, int result,FolderListObject[] response) {
		log("getFolderListResponse: address->"+address+", masid ->"+masId+",result->"+result);
		if (response == null) {
			ArrayList<FolderListObject> object = new ArrayList<FolderListObject>();
			response = object.toArray(new FolderListObject[object.size()]);
		}
		log("response size is "+ response.length);
		if (isEnabled()) {
			getFolderListResponseNative(address, masId, result, response);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}
	private synchronized boolean getMessageListResponse(String address, int masId, int result,MessageListObject messageList) {
		log("getMessageListResponse: address->"+address+",masId->"+masId+",result->"+result);
		if (isEnabled()) {
			getMessageListResponseNative(address, masId, result, messageList);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}
	
	private synchronized boolean getMessageResponse(String address, int masId, int result, BMessageObject bMessage) {
		log("getMessageResponse: address->"+address+",masId->"+masId+",result->"+result);
		if (isEnabled()) {
			getMessageResponseNative(address, masId, result, bMessage);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}
	private synchronized boolean setMessageStatusResponse(String address, int masId, int result) {
		log("setMessageStatusResponse: ");
		if (isEnabled()) {
			setMessageStatusResponseNative(address, masId,result);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}

	private synchronized boolean setFolderResponse(String address, int masId, int result) {
		log("setFolderResponse: address->"+address+",masId->"+masId+",result->"+result);
		if (isEnabled() && address != null) {
			setFolderResponseNative(address, masId, result);
			return true;
		} else {
			log("MAP service has not been initialized");
			return false;
		}
	}

	private synchronized boolean setNotificationReg(String address, int masId, int result) {
		log("setNotificationReg: address->"+address+",masId->"+masId+",result->"+result);
		if (mState == BluetoothMap.STATE_ENABLED && address != null) {
			setNotifRegResponseNative(address, masId, result);
			return true;
		} else {
			log("MAP service has not been initialized or device is null");
			return false;
		}
	}
	

	private void onRequestOrIndicationReceived(int what, int arg1, int arg2, Object object) {
		mHandler.obtainMessage(what, arg1, arg2, object).sendToTarget();
	
	}

	private void setState(int state) {
		log("setState: old state is "+mState+",new state is "+state);
		boolean shouldSendIntent = false;
		int oldState = mState;
		int newState = state;
		
		switch (oldState) {
			case BluetoothMap.STATE_ENABLING:
				if (newState == BluetoothMap.STATE_ENABLED) {
					mState = BluetoothMap.STATE_ENABLED;
					shouldSendIntent = true;
				} else {
					log("error");
				}
				break;
			case BluetoothMap.STATE_ENABLED:
				if (newState != BluetoothMap.STATE_ENABLED && 
					newState != BluetoothMap.STATE_ENABLING) {
					mState = newState;
					shouldSendIntent = true;
				}
				break;
			case BluetoothMap.STATE_DISABLING:

				if (newState == BluetoothMap.STATE_DISABLED) {
					mState = newState;
					shouldSendIntent = true;
				}
				break;
			case BluetoothMap.STATE_DISABLED:
				if (newState == BluetoothMap.STATE_ENABLED ||
					newState == BluetoothMap.STATE_ENABLING) {
					mState = newState;
					shouldSendIntent = true;
				}
				break;
			default:
				log("unexpected state");
		}
		if (shouldSendIntent) {
			Intent intent = new Intent();
			intent.setAction(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
			intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_MAP_Server);
			intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,newState);
			intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE,oldState);
			sendBroadcast(intent);
		}

	}

	private void onStateChanged(int state){
		log("onStateChanged()");
		if (mMapServerCallback == null) {
			log("mMapServerCallback is null, no need to call back");
			return;
		}
		final int N = mMapServerCallback.beginBroadcast();
		for (int i = 0; i < N; i++) {
			try {
				mMapServerCallback.getBroadcastItem(i).onStateChanged(state);
			} catch (RemoteException e) {
				// do nothing.
			}
		}
		mMapServerCallback.finishBroadcast();
	}

	
	private final RemoteCallbackList<IBluetoothMapSettingCallback> mMapServerCallback
				= new RemoteCallbackList<IBluetoothMapSettingCallback>();

	private final IBluetoothMapSetting.Stub mSettingBinder = new IBluetoothMapSetting.Stub() {
			
		public void enableServer(){
			log("mSettingBinder:enableServer");
			enable();
			//save the setting
			SharedPreferences.Editor editor = getSharedPreferences(TAG, Context.MODE_PRIVATE).edit();
			editor.putBoolean(SERVICE_ENABLE, true);
			editor.apply();
		}
		public void disableServer(){
			log("mSettingBinder:disableServer");
			disable();	
			SharedPreferences.Editor editor = getSharedPreferences(TAG, Context.MODE_PRIVATE).edit();
			editor.putBoolean(SERVICE_ENABLE, false);
			editor.apply();
		}
		public boolean isEnabled(){
			return BluetoothMapServerService.this.isEnabled();
		}
		public boolean registerSim(int value) {
			log("mSettingBinder:registerSim,"+value);
			return mManager.registerSim(value);
		}
		public void unregisterSim(int value) {
			log("mSettingBinder:unregisterSim, "+value);
			mManager.unregisterSim(value);
		}
		public boolean replaceAccount(long value) {			
			return mManager.replaceAccount(value);
		}
		public int[] getSims(){
			return mManager.getSims();
		}
		public long getEmailAccount(){
			return mManager.getEmailAccount();
		}

		public void registerCallback(IBluetoothMapSettingCallback cb) {
			mMapServerCallback.register(cb);
		}
		public void unregisterCallback(IBluetoothMapSettingCallback cb){
			mMapServerCallback.unregister(cb);
		}
	};

	private final IBluetoothMap.Stub mBinder = new IBluetoothMap.Stub() {
		
		/*proxy interface*/
			//when map proxy in framework check the device state, just query in instance manager
	
			public boolean isConnected(BluetoothDevice device) {
				log("isConnetected");
				if(!isEnabled()) {
					log("error, the service has not been ready ");
				}				
				return mManager.isConnected(device);
			}
		
			public BluetoothDevice[] getConnectedDevices() {
				log("getConnectedDevice");
				if(!isEnabled()) {
					log("error, the service has not been ready ");
				}
				
				return mManager.getConnectedDevices();
			}
		
			public int getState(BluetoothDevice device) {
				log("getState");
				if(!isEnabled()) {
					log("error, the service has not been ready ");
				}			
				
				return mManager.getState(device);
			}
			public boolean disconnect(BluetoothDevice device) {
				log("disconnect");
				if(!isEnabled()) {
					log("error, the service has not been ready ");
				}				
				disconnect(device);
				return true;
			}
			public void connect(BluetoothDevice device) {
				//do nothing
			}
			public void close(BluetoothDevice device) {
				//do nothing
			}
		
	};


	
	

	/*proxy interface done*/
	private void log(String info) {
		if (null != info){
			Xlog.v(TAG, info);
		}
	}

	private void initService(){
		boolean orignalSetting;
		if (mInit) {
			log("[MAP]the service has been initialized");
			return;
		}
		if (initNative()){
			mState = BluetoothMap.STATE_DISABLED;
			
			SharedPreferences sp = getSharedPreferences(TAG, Context.MODE_PRIVATE);
			orignalSetting = sp.getBoolean(SERVICE_ENABLE, false);
			if (orignalSetting) {
				enable();
			}
		} else {
			mState = BluetoothMap.STATE_ABNORMAL;
		}
		mInit = true;
	}

	private void deinitServer() {
		if (!mForceToDeinit) {
			return;
		}
		mForceToDeinit = false;
		if (mInit)
		{
			deinitNative();
			mInit = false;
		}
	}
	private void clearService(){
		mForceToDeinit = true;
		if (isEnabled()) {
			disable();
		} else {
			deinitServer();
		}
	}
	private void enable() {
		log("enableServer()");
		if (mState != BluetoothMap.STATE_DISABLED) {
			log("fail to enable because the service state is "+mState);
			onStateChanged(BluetoothMap.STATE_DISABLED);
			return;
		}
		enableNative();
		setState(BluetoothMap.STATE_ENABLING);	

	}
	private void disable() {
		disable(true);
	}	
	private void disable(boolean checkState) {
		int index = 0;
		Instance instance;
		BluetoothDevice[] devices;
		log("disableServer()");
		if (checkState && 
			(mState == BluetoothMap.STATE_DISABLING || mState == BluetoothMap.STATE_DISABLED)) {
			log("the serive is disabling or disabled");
			onStateChanged(mState);
			return;
		}
		setState(BluetoothMap.STATE_DISABLING);

		if (PENDING == disconnectServer()) {
			return;
		}

		//MAS has to be deregistered before disabling service
		ArrayList<Instance> instances = mManager.getAllInstances();
		for(index = 0; index < instances.size(); index ++) {
			instance = instances.get(index);
		        deregisterInstance(instance);
		}
//		mManager.removeAllInstances();
		InstanceUtil.reset();
		
		disableNative();
	}
	private boolean isEnabled() {
			if (mState == BluetoothMap.STATE_ENABLED) {
				return true;
			}
			return false;
		}
	private int getState() {
		return mState;
	}
	
	static {
		System.loadLibrary("extmap_jni");
		classInitNative();
    }	

	private native static void classInitNative();	
//	private native void classInitNative();	

	private native boolean initNative();	
	private native void deinitNative();	
	private native int enableNative();
	private native int disableNative();
	private native int registerInstanceNative(int masId, String severName, int messageType, String rootPath);
	private native int deregisterInstanceNative(int masId);
	private native int parseMessageNative(String address, int masId, BMessageObject message);
	private native int sendMessageResponseNative(String address, int masId, int result, long handle);
	private native int getFolderListResponseNative(String address, int masId, int result, FolderListObject[] folderlist);
	private native int getMessageResponseNative(String address, int masId, int result, BMessageObject bMessage);
	private native int setMessageStatusResponseNative(String address, int masId, int result);
	private native int updateInboxResponseNative(String address, int masId, int result);
	private native int getMessageListResponseNative(String address, int masId,int result, MessageListObject msglist);
	private native int setFolderResponseNative(String address, int masId, int result);
	private native int authorizeResponseNative(String address, int result);
	private native int sendReportNative(String address, EventReport report);
	private native int setNotifRegResponseNative(String address, int masInstanceId, int result) ;
	private native int disconnectMasNative(int masId);
	private native int disconnectServerNative();
	private native int disconnectMnsSessionNative(String address);

	
}
