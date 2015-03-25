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
public class AccRawActivity extends Activity implements OnClickListener {
	private final static String TAG = "Game";
	private SensorManager sensorMgr;
	private TextView accuracyLabel;
	private TextView xLabel, yLabel, zLabel;
	private Button calibrateButton;
	private Sensor mSensor; 
	private float x, y, z;	
	// deltas for calibration
	private float cx, cy, cz;	
	private long lastUpdate = -1;
	private boolean mShowLog = false;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.accel);
        accuracyLabel = (TextView) findViewById(R.id.accuracy_label);
        xLabel = (TextView) findViewById(R.id.x_label);
        yLabel = (TextView) findViewById(R.id.y_label);
        zLabel = (TextView) findViewById(R.id.z_label);
        //calibrateButton = (Button) findViewById(R.id.calibrate_button);
        //calibrateButton.setOnClickListener(this);
		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
    }

	@Override
	protected void onPause() {
		super.onPause();
		
		sensorMgr.unregisterListener(mSensorEventListener);
		sensorMgr = null;
		
		cx = 0;
		cy = 0;
		cz = 0;
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		sensorMgr = (SensorManager) getSystemService(SENSOR_SERVICE);
		mSensor = sensorMgr.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		boolean accelSupported = sensorMgr.registerListener(mSensorEventListener, 
				mSensor, SENSOR_DELAY_UI);
		
		if (!accelSupported) {
			// on accelerometer on this device
			sensorMgr.unregisterListener(mSensorEventListener);
			accuracyLabel.setText(R.string.no_accelerometer);
		}
		Log.d(TAG,"accelSupported : " + accelSupported);		
	}

	public void onClick(View v) {
		if (v == calibrateButton) {
			cx = -x;
			cy = -y;
			cz = -z;
		}
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
			if (sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
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
		
		/***************************************************************************
		/*  Copied from SensorTest.Java
		/**************************************************************************/
        private final float[] mScale = new float[] { 2, 2.5f, 0.5f };   // accel
        private float[] mPrevData = new float[3];		
        private long mLastTime;
		public void simpleEventHandler(SensorEvent event){
            boolean show = false;
            float[] diff = new float[3];
            float[] values = event.values;

            
            for (int i = 0; i < 3; i++) {
                diff[i] = Math.round(mScale[i] * (values[i] - mPrevData[i]) * 0.45f);
                if (Math.abs(diff[i]) > 0) {
                    //show = true;
                }
                mPrevData[i] = values[i];
            }
            
            if (show) {
                // only shows if we think the delta is big enough, in an attempt
                // to detect "serious" moves left/right or up/down
                Log.e(TAG, "sensorChanged " + event.sensor.getType() + " (" + values[0] + ", " + values[1] + ", " + values[2] + ")"
                                   + " diff(" + diff[0] + " " + diff[1] + " " + diff[2] + ")");
            }
            
            long now = android.os.SystemClock.uptimeMillis();
            if (now - mLastTime > 1000) {
            	mLastTime = 0;
                
                float x = diff[0];
                float y = diff[1];
                boolean gestX = Math.abs(x) > 3;
                boolean gestY = Math.abs(y) > 3;

                if ((gestX || gestY) && !(gestX && gestY)) {
                    if (gestX) {
                        if (x < 0) {
                            Log.w(TAG, "<<<<<<<< LEFT <<<<<<<<<<<<");
                        } else {
                            Log.w(TAG, ">>>>>>>>> RITE >>>>>>>>>>>");
                        }
                    } else {
                        if (y < -2) {
                            Log.w(TAG, "<<<<<<<< UP <<<<<<<<<<<<");
                        } else {
                            Log.w(TAG, ">>>>>>>>> DOWN >>>>>>>>>>>");
                        }
                    }
                } 
            }		
		}


		// from the android.hardware.SensorEventListener interface
		private String mBuffer;
		public void onSensorChanged(SensorEvent event) {
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_ACCELEROMETER) {
				long curTime = System.currentTimeMillis();
				// only allow one update every 100ms, otherwise updates
				// come way too fast and the phone gets bogged down
				// with garbage collection
				if (mShowLog)
				{
					//Log.d(TAG, "onSensorChanged("+type+")"+":"
					//	  +String.format("%+2.3f, %+2.3f, %+2.3f", 
					//	  (event.values[DATA_X]), 
					//	  (event.values[DATA_Y]), 
					//	  (event.values[DATA_Z])));  
					Log.d(TAG, "("+type+")" + event.values[DATA_X]+", "+
					      event.values[DATA_Y]+", "+event.values[DATA_Z]);
				}
				if (lastUpdate == -1 || (curTime - lastUpdate) > 100) {
					lastUpdate = curTime;
					//mBuffer = String.format("X: %+2.3f (%+2.3f)\nY: %+2.3f (%+2.3f)\nZ: %+2.3f (%+2.3f)",
					//		  (event.values[DATA_X]+cx), cx, (event.values[DATA_Y]+cy), cy,
					//		  (event.values[DATA_Z]+cz), cz);
					//mBuffer = "X: " + (event.values[DATA_X]+cx) + " ("+cx+")\n" + 
					//		  "Y: " + (event.values[DATA_Y]+cy) + " ("+cy+")\n" +
					//		  "Z: " + (event.values[DATA_Z]+cz) + " ("+cz+")\n";
					mBuffer = "X: " + (event.values[DATA_X]+cx) + "\n" + 
							  "Y: " + (event.values[DATA_Y]+cy) + "\n" +
							  "Z: " + (event.values[DATA_Z]+cz) + "\n";
					xLabel.setText(mBuffer);
					//xLabel.setText(String.format("X: %+2.5f (%+2.5f)", (event.values[DATA_X]+cx), cx));
					//yLabel.setText(String.format("Y: %+2.5f (%+2.5f)", (event.values[DATA_Y]+cy), cy));
					//zLabel.setText(String.format("Z: %+2.5f (%+2.5f)", (event.values[DATA_Z]+cz), cz));
				}
				simpleEventHandler(event);
			}
		}
	};
}