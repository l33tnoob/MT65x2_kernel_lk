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

package com.stuffthathappens.games;

import static android.hardware.SensorManager.DATA_X;
import static android.hardware.SensorManager.DATA_Y;
import static android.hardware.SensorManager.DATA_Z;
import static android.hardware.SensorManager.SENSOR_DELAY_UI;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_HIGH;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_LOW;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM;
import static android.hardware.SensorManager.SENSOR_STATUS_UNRELIABLE;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.util.Log;

/**
 * Displays values from the accelerometer sensor.
 * 
 * @author Eric M. Burke
 */
public class AlsPsRawActivity extends Activity{
	private final static String TAG = "Game";
	private SensorManager sensorMgr;
	private TextView accuracyLabel;
	private TextView alsLabel, psLabel;
	private Button calibrateButton;
	private Sensor mProximitySensor;
	private Sensor mLightSensor;
	private long lastUpdate = -1;
	private boolean mShowLog = false;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.alsps);
        accuracyLabel = (TextView) findViewById(R.id.accuracy_label);
        alsLabel = (TextView) findViewById(R.id.als_label);
        psLabel = (TextView) findViewById(R.id.ps_label);        
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
    }

	@Override
	protected void onPause() {
		super.onPause();		
		sensorMgr.unregisterListener(mSensorEventListener);
		sensorMgr = null;
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		sensorMgr = (SensorManager) getSystemService(SENSOR_SERVICE);
		mProximitySensor = sensorMgr.getDefaultSensor(Sensor.TYPE_PROXIMITY);
		mLightSensor = sensorMgr.getDefaultSensor(Sensor.TYPE_LIGHT);
		boolean alsSupported = sensorMgr.registerListener(mSensorEventListener, 
				mLightSensor, SENSOR_DELAY_UI);
		boolean psSupported = sensorMgr.registerListener(mSensorEventListener, 
				mProximitySensor, SENSOR_DELAY_UI);
		
		if (!alsSupported || !psSupported) {
			// on accelerometer on this device
			sensorMgr.unregisterListener(mSensorEventListener);
			if (!alsSupported && psSupported)
				accuracyLabel.setText(R.string.no_als);
			else if (alsSupported && !psSupported)
				accuracyLabel.setText(R.string.no_ps);
			else 
				accuracyLabel.setText(R.string.no_alsps);
		}
		Log.d(TAG,"alsSupported: " + alsSupported + "; psSupported: "+psSupported);		
	}
	
	@Override 
	public boolean onCreateOptionsMenu(Menu menu) { 
		boolean supRetVal = super.onCreateOptionsMenu(menu);
		if (mShowLog)
			menu.add(0, 0, 0, getString(R.string.hide_log));
		else
			menu.add(0, 0, 0, getString(R.string.show_log));
		return supRetVal; 
	} 

	@Override 
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) { 
		case 0: 
			if (mShowLog) {
				mShowLog = false;
				item.setTitle(R.string.show_log);
			} else {
				mShowLog = true;
				item.setTitle(R.string.hide_log);
			}
			return true;			
		}
		return false; 
	}	

	public final SensorEventListener mSensorEventListener = new SensorEventListener() {
		// from the android.hardware.SensorListener interface
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
			// this method is called very rarely, so we don't have to
			// limit our updates as we do in onSensorChanged(...)
			//Log.d(TAG, "onAccuracyChanged("+sensor+","+accuracy+")");
			int type = sensor.getType();
			if (type == Sensor.TYPE_PROXIMITY || type == Sensor.TYPE_LIGHT) {
				switch (accuracy) {
				case SENSOR_STATUS_UNRELIABLE:
					accuracyLabel.setText(R.string.accuracy_unreliable);
					break;
				case SENSOR_STATUS_ACCURACY_LOW:
					accuracyLabel.setText(R.string.accuracy_low);
					break;
				case SENSOR_STATUS_ACCURACY_MEDIUM:
					accuracyLabel.setText(R.string.accuracy_medium);
					break;
				case SENSOR_STATUS_ACCURACY_HIGH:
					accuracyLabel.setText(R.string.accuracy_high);
					break;
				}
			}
		}
		


		// from the android.hardware.SensorEventListener interface
		private String mBuffer;
		public void onSensorChanged(SensorEvent event) {
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_PROXIMITY) {

				if (mShowLog)
				{
					Log.d(TAG, "("+type+")" + event.values[0]);
				}
				mBuffer = "PS:  " + event.values[0];
				psLabel.setText(mBuffer);
				mBuffer = null;
			} else if (type == Sensor.TYPE_LIGHT) {
				if (mShowLog)
				{
					Log.d(TAG, "("+type+")" + event.values[0]);
				}
				mBuffer = "ALS: " + event.values[0];
				alsLabel.setText(mBuffer);
				mBuffer = null;				
			}
		}
	};
}