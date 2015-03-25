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

import java.util.Set;

import com.mediatek.bluetooth.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothMap;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.xlog.Xlog;

public class BluetoothMapActivity extends AlertActivity 
				implements DialogInterface.OnClickListener {
	private final String TAG = "BluetoothMapAlert";
	private int mType;
	private BluetoothDevice mDevice;
	private TextView mContentView;
	
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
        	log("receive intent:"+intent.getAction());
            if (BluetoothMapNotification.ACTION_MAP_DISCONNECTED.equals(intent.getAction())) {
				BluetoothDevice device = intent.getParcelableExtra(BluetoothMapNotification.EXTRA_DEVICE); 
				if (device != null && device.equals(mDevice)) {
					finish();
				}
            } else if(BluetoothAdapter.ACTION_STATE_CHANGED.equals(intent.getAction())) {
            	//force to dismisss if dialog still exist when Bluetooth is off
				int state = BluetoothAdapter.getDefaultAdapter().getState();
				if(state == BluetoothAdapter.STATE_OFF) {
					finish();
				}
            }
        }
    };
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
		Intent intent = getIntent();
		String action = intent.getAction();
		mType = intent.getIntExtra(BluetoothMapNotification.EXTRA_ALERT_TYPE, -1);
		mDevice = intent.getParcelableExtra(BluetoothMapNotification.EXTRA_DEVICE);

		log("onCreate(): mType is "+mType + ",action is "+action);

		if (mDevice == null) {
			log("the device is null");
			return;
		}

	//	if (action.equals(BluetoothMapNotification.))
				
		final AlertController.AlertParams p = mAlertParams;
	    p.mIconId = android.R.drawable.ic_dialog_info;
		switch (mType) {
			case BluetoothMapNotification.ALERT_TYPE_AUTHORIZE:
				p.mTitle = getString(R.string.bluetooth_map_server_authorize_title);
				p.mPositiveButtonText = getString(R.string.bluetooth_map_server_authorize_confirm_allow);
				p.mNegativeButtonText = getString(R.string.bluetooth_map_server_authorize_confirm_reject);
				break;
			case BluetoothMapNotification.ALERT_TYPE_CONNECT:
				p.mTitle = getString(R.string.bluetooth_map_server_disconnect_title);
				p.mPositiveButtonText = getString(R.string.bluetooth_map_server_yes);
				p.mNegativeButtonText = getString(R.string.bluetooth_map_server_no);
				break;
			default:
				log("unexpected alert type");
		}
		
		p.mView = createView(mType);

		

//		p.mPositiveButtonText = getString(R.string.bluetooth_map_yes);
	    p.mPositiveButtonListener = this;
//	    p.mNegativeButtonText = getString(R.string.bluetooth_map_no);
	    p.mNegativeButtonListener = this;
	    
	    setupAlert();	
		
		IntentFilter filter = new IntentFilter();
	    filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
		filter.addAction(BluetoothMapNotification.ACTION_MAP_DISCONNECTED);
        registerReceiver(mReceiver, filter);	 	       
	}
	
	protected void onResume(){
		super. onResume();
		log("onResume()");
		
	}

	protected void onDestroy(){
		log("onDestroy");
		unregisterReceiver(mReceiver);
		super.onDestroy();
	}
	
    private View createView(int type) {
        View view = getLayoutInflater().inflate(R.layout.bluetooth_map_confirm_dialog, null);
        String text = new String();
        mContentView = (TextView)view.findViewById(R.id.content);
		log("createView");
		
        if(mContentView != null){
			switch (mType) {
			case BluetoothMapNotification.ALERT_TYPE_AUTHORIZE:
				text = getString(R.string.bluetooth_map_server_authorize_message,mDevice.getName());
				break;
			case BluetoothMapNotification.ALERT_TYPE_CONNECT:
				text = getString(R.string.bluetooth_map_server_disconnect_message,mDevice.getName());
				break;
			default:
				log("unexpected alert type");
			}		
	        mContentView.setText(text);
        }       

        return view;
    }

    public void onClick(DialogInterface dialog, int which) {
	log("onClick(): which is "+which);
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
            	switch (mType) {
					case BluetoothMapNotification.ALERT_TYPE_AUTHORIZE:
						Authorize(true);			
						break;
					case BluetoothMapNotification.ALERT_TYPE_CONNECT:
						disconncet();
						break;
					default:
						log("unexpected alert type");
            	}
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                switch (mType) {
					case BluetoothMapNotification.ALERT_TYPE_AUTHORIZE:
						Authorize(false);				
						break;
					case BluetoothMapNotification.ALERT_TYPE_CONNECT:
						break;
					default:
						log("unexpected alert type");
            	}
                break;
            default:
            	break;
        }
		//onlyOnce = true;
		finish();	
    }  
	
	
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_HOME) {        	
        	finish();
        }
        return super.onKeyDown(keyCode, event);
    }

	private void Authorize(boolean accept) {
		log("Authorize(), device is " + mDevice.getName()+ ",result is "+accept);
		Intent intent = new Intent(BluetoothMap.ACTION_AUTHORIZE_RESULT);
		intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mDevice);
		intent.putExtra(BluetoothMap.EXTRA_RESULT, accept);
		sendBroadcast(intent);
	}
	private void disconncet() {
		log("disconncet(), device is " + mDevice.getName());
		Intent intent = new Intent(BluetoothMap.ACTION_DISCONNECT_DEVICE);
		intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mDevice);
		sendBroadcast(intent);		
	}

	private void log(String info) {
		if (null != info){
			Xlog.v(TAG, info);
		}

	}
    

}

