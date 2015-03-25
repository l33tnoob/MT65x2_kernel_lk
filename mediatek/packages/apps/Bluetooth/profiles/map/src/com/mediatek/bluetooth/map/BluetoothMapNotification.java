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

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothMap;


import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.HashSet;
import java.util.Set;


import android.util.Log;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.xlog.Xlog;

class BluetoothMapNotification {
	private final String TAG = "BluetoothMapNotification";
	public static final String ACTION_MAP_DISCONNECTED = 
								"com.mediatek.bluetooth.map.BluetoothMapNotification.action.MAP_DISCONNECTED";
//	public static final String EXTRA_DEVICE = BluetoothDevice.EXTRA_DEVICE;
/*	public static final String EXTRA_ALERT_TYPE = 
								"com.mediatek.bluetooth.map.BluetoothMapNotification.extra.DEVICE";
								*/
	public static final String EXTRA_ALERT_TYPE = 
								"com.mediatek.bluetooth.map.BluetoothMapNotification.extra.ALERT_TYPE";
	public static final String EXTRA_DEVICE = 
								"com.mediatek.bluetooth.map.BluetoothMapNotification.extra.DEVICE";
	public static final String ACTION_MAP_CLEAR_NOTIFICATION = 
								"com.mediatek.bluetooth.map.BluetoothMapNotification.action.MAP_CLEAR_NOTIFICATION";
	public static final String ACTION_AUTHORIZE_INDICATION =
								"com.mediatek.bluetooth.map.BluetoothMapNotification.action.AUTHORIZE_INDICATION";
	public static final String ACTION_CONNECT_INDICATION =
								"com.mediatek.bluetooth.map.BluetoothMapNotification.action.CONNECT_INDICATION";
	
	public static final int ALERT_TYPE_AUTHORIZE = 0;
	public static final int ALERT_TYPE_CONNECT   = 1;

	private static final int MAP_ID_START = BluetoothProfile.getProfileStart(BluetoothProfile.ID_MAP);
    private static final int MAP_AUTHORIZE_NOTIFY = MAP_ID_START + 1;
	private static final int MAP_CONNECT_NOTIFY	= MAP_ID_START + 2;

	private Context mContext;
	private HashMap<BluetoothDevice, Integer> mDevices;
	
	BluetoothMapNotification(Context context){
		mContext = context;
		mDevices = new HashMap<BluetoothDevice, Integer>();
	}
	public void createNotification(int alertType, BluetoothDevice device, boolean isNewView)
    {
        NotificationManager nm = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notification = null;

		log("createNotification: alert type = "+alertType+
			",device is "+device.getName()+",isNewView->"+isNewView);

        // Create an intent triggered by clicking on the status icon
        Intent intent = getIntent(alertType, device);
   /*     Intent intent = new Intent();
        intent.setClass(mContext, BluetoothMapActivity.class); 		
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction(ACTION_AUTHORIZE_INDICATION);
        intent.putExtra(EXTRA_DEVICE, device);
	 	intent.putExtra(EXTRA_ALERT_TYPE, alertType);
	 	*/
               
        String name = device.getName();
		String tickerText = getNotificationTickerText(alertType);
		String contentTitle = getNotificationContentTitle(alertType, name);
		String contentMessage = getNotificationContentMessage(alertType, name);
		int notificationId = getNotificationID(alertType);
        notification = new Notification(android.R.drawable.stat_sys_data_bluetooth, tickerText, System.currentTimeMillis());
        notification.setLatestEventInfo(mContext, contentTitle, contentMessage, 
										PendingIntent.getActivity(mContext, 0, intent, 0));
		
 //       notification.flags |= Notification.FLAG_AUTO_CANCEL;
        notification.flags |= Notification.FLAG_ONLY_ALERT_ONCE;
        if (isNewView)
        {
            notification.defaults |= Notification.DEFAULT_SOUND;
            notification.defaults |= Notification.DEFAULT_VIBRATE;
        }
        else
        {
            notification.defaults = 0;
        }

	
		Intent deleteIntent = new Intent(ACTION_MAP_CLEAR_NOTIFICATION);
        notification.deleteIntent = PendingIntent.getBroadcast(mContext, 0, deleteIntent, 0);
		
        nm.notify(notificationId, notification);
		log("notificationId is " + notificationId);
		setDeviceState(alertType,device);
    }

	private Intent getIntent(int type, BluetoothDevice device){
		Intent intent = new Intent();
        intent.setClass(mContext, BluetoothMapActivity.class); 		
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(EXTRA_DEVICE, device);
	 	intent.putExtra(EXTRA_ALERT_TYPE, type);

		if (type == ALERT_TYPE_AUTHORIZE) {
			intent.setAction(ACTION_AUTHORIZE_INDICATION);
		} else if (type == ALERT_TYPE_CONNECT){
			intent.setAction(ACTION_CONNECT_INDICATION);
		} else {

			log("unknown type "+ type);
		}
		
		return intent;
	}

	private String getNotificationTickerText(int type) {
		if (type == ALERT_TYPE_AUTHORIZE) {
			return mContext.getString(R.string.bluetooth_map_server_authorize_request_ticker);
		} else if (type == ALERT_TYPE_CONNECT) {
			return mContext.getString(R.string.bluetooth_map_server_connect_notify_ticker);
		} else {
			log("error, unexpected alert type");
			return null;
		}
	}

	private String getNotificationContentTitle(int type, String name) {
		if (type == ALERT_TYPE_AUTHORIZE) {
			return mContext.getString(R.string.bluetooth_map_server_authorize_request_title);
		} else if (type == ALERT_TYPE_CONNECT) {
			return mContext.getString(R.string.bluetooth_map_server_connect_notify_title, name);
		} else {
			log("error, unexpected alert type");
			return null;
		}
	}

	private String getNotificationContentMessage(int type, String name) {
		if (type == ALERT_TYPE_AUTHORIZE) {
			return mContext.getString(R.string.bluetooth_map_server_authorize_request_message);
		} else if (type == ALERT_TYPE_CONNECT) {
			return mContext.getString(R.string.bluetooth_map_server_connect_notify_message, name);
		} else {
			log("error, unexpected alert type");
			return null;
		}
	}
	private int getNotificationID(int type) {	
		if (type == ALERT_TYPE_AUTHORIZE) {
			return MAP_AUTHORIZE_NOTIFY;
		} else if (type == ALERT_TYPE_CONNECT) {
			return MAP_CONNECT_NOTIFY;
		} else {
			log("error, unexpected alert type");
			return -1;
		}
	}

	//TODO: when two or more device is requesting dialog 
    public void removeNotification(int type, BluetoothDevice device) 
    {
		int id = getNotificationID(type);
		log("remove notification: type is" + type);
		NotificationManager nm = (NotificationManager)mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(id);

		resetDeviceState(type, device);

	//notify Alert dialog to dismiss
	Intent intent = new Intent(BluetoothMapNotification.ACTION_MAP_DISCONNECTED);
        intent.putExtra(BluetoothMapNotification.EXTRA_DEVICE, device);
        mContext.sendBroadcast(intent);
    }

	private void setDeviceState(int type, BluetoothDevice device) {
		if (device == null) {
			log("error, the device is null");
			return;
		}
		if (type == ALERT_TYPE_AUTHORIZE) {
			//replace the old value with the new state
			mDevices.put(device, BluetoothMap.STATE_AUTHORIZING);
		} else if (type == ALERT_TYPE_CONNECT){			
			mDevices.put(device, BluetoothMap.STATE_CONNECTED);
		}

		log("setDeviceState "+ type + ", "+device.getName());
		log("mDevices.size():"+mDevices.size());
		log("mDevices.get(device):"+mDevices.get(device));
		log("getDeviceState:"+getDeviceState(device));
	}
	private void resetDeviceState(int type, BluetoothDevice device) {
		if (device == null) {
			log("error, the device is null");
			return;
		}
		mDevices.remove(device);
		
		log("resetDeviceState "+ type + ", "+device.getName());
		log("mDevices.size():"+mDevices.size());
		log("mDevices.get(device):"+mDevices.get(device));
		log("getDeviceState:"+getDeviceState(device));
	/*	if (type == ALERT_TYPE_AUTHORIZE) {
			//replace the old value with the new statev
			mDevices.remove(device);
		} else if (type == ALERT_TYPE_CONNECT){			
			mDevices.put(device, BluetoothMap.STATE_CONNECTED);
		}
		*/
	}

	public int getDeviceState(BluetoothDevice device) {
		if (device == null) {
			log("error, the device is null");
			return BluetoothMap.STATE_NONE;
		}
		if (mDevices.containsKey(device)) {
			return mDevices.get(device);
		}
		return BluetoothMap.STATE_NONE;
	}

	public HashSet<BluetoothDevice> getAuthoringDevices(){
		HashSet<BluetoothDevice> authorizingDevices = new HashSet<BluetoothDevice>();
		Iterator devices = mDevices.entrySet().iterator();
		while (devices.hasNext()) {
			Map.Entry entry = (Map.Entry)devices.next();
			if (((Integer)entry.getValue()).intValue() == BluetoothMap.STATE_AUTHORIZING){
				authorizingDevices.add((BluetoothDevice)entry.getKey());
			}
			
		}
		return authorizingDevices;
	}

	private void log(String info) {
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}
