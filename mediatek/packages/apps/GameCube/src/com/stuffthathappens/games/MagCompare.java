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

public class MagCompare extends Activity implements OnClickListener {
	private final static String TAG = "Game";
	private SensorManager mSensorManager;
	private TextView accuracyLabel;
	private TextView yaw_raw, yaw_cal, yaw_max, yaw_min; 
	private TextView pit_raw, pit_cal, pit_max, pit_min;
	private TextView rol_raw, rol_cal, rol_max, rol_min;
	private Button calibrateButton;
	private Sensor mMagSensor;
	private Sensor mAccSensor;
	private Sensor mOriSensor;
    private float[] mGData = new float[3];
    private float[] mMData = new float[3];
    private float[] mOData = new float[3];
    private float[] mR = new float[16];
    private float[] mI = new float[16];
    private float[] mOrientation = new float[3]; 	
	private float x, y, z;	
	// deltas for calibration
	private float cx, cy, cz;	
	private long lastUpdate = -1;
	private boolean mShowLog = false;
	private String mBuffer;	
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.magcmp);
        accuracyLabel = (TextView) findViewById(R.id.accuracy_label);
        yaw_raw = (TextView) findViewById(R.id.yaw_raw);
        yaw_cal = (TextView) findViewById(R.id.yaw_cal);
        yaw_max = (TextView) findViewById(R.id.yaw_max);
        yaw_min = (TextView) findViewById(R.id.yaw_min);
        
        pit_raw = (TextView) findViewById(R.id.pit_raw);
        pit_cal = (TextView) findViewById(R.id.pit_cal);
        pit_max = (TextView) findViewById(R.id.pit_max);        
        pit_min = (TextView) findViewById(R.id.pit_min);

        rol_raw = (TextView) findViewById(R.id.rol_raw);
        rol_cal = (TextView) findViewById(R.id.rol_cal);
        rol_max = (TextView) findViewById(R.id.rol_max);        
        rol_min = (TextView) findViewById(R.id.rol_min);

        //always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
    }

	@Override
	protected void onPause() {
		super.onPause();
		
		mSensorManager.unregisterListener(mSensorEventListener);
		mSensorManager = null;
		
		cx = 0;
		cy = 0;
		cz = 0;
	}

	@Override
	protected void onResume() {
		boolean bAccSupport, bMagSupport, bOriSupport;		
		super.onResume();
		
		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		mAccSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		mMagSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
		mOriSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
		bAccSupport = mSensorManager.registerListener(mSensorEventListener, mAccSensor, SENSOR_DELAY_UI);
		bMagSupport = mSensorManager.registerListener(mSensorEventListener, mMagSensor, SENSOR_DELAY_UI);
		bOriSupport = mSensorManager.registerListener(mSensorEventListener, mOriSensor, SENSOR_DELAY_UI);

		if (!bAccSupport || !bMagSupport || !bOriSupport) {
			// on accelerometer on this device
			String status = "";
			mSensorManager.unregisterListener(mSensorEventListener);
			mSensorManager.unregisterListener(mSensorEventListener);
			mSensorManager.unregisterListener(mSensorEventListener);
			if (bAccSupport)
				status += "Accelerometer: YES\n";
			else
				status += "Accelerometer: NO \n";
			if (bMagSupport)
				status += "Magnetic field: YES\n";
			else
				status += "Magnetic field: NO \n";
			if (bOriSupport)
				status += "Orientation: YES\n";
			else
				status += "Orientation: NO \n";
			accuracyLabel.setText(status.toCharArray(), 0, status.length());
		}
		Log.d(TAG,"Supported : " + bAccSupport+ ", " + bMagSupport+", "+bOriSupport);		
	}

	public void onClick(View v) {
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
        private final float[] mScale = new float[] { 2, 2.5f, 0.5f };  
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
		}

		// from the android.hardware.SensorEventListener interface
		private String mYawStr, mPitchStr, mRollStr;
		private float  diff_yaw, diff_pitch, diff_roll;
		private float[]  diff = new float[3];
		private float[]  max = new float[] {-1000.0f, -1000.0f, -1000.0f};
		private float[]  min = new float[] {+1000.0f, +1000.0f, +1000.0f};
		private String[] status = new String[3];
		public void onSensorChanged(SensorEvent event) {
			int type = event.sensor.getType();
	        if (type == Sensor.TYPE_ACCELEROMETER) {
	            mGData = event.values.clone();
	        } else if (type == Sensor.TYPE_MAGNETIC_FIELD) {
	        	mMData = event.values.clone();
	        } else if (type == Sensor.TYPE_ORIENTATION)	{
	        	mOData = event.values.clone();
	        } else {
	            // we should not be here.
	            return;
	        }
	        
	        SensorManager.getRotationMatrix(mR, mI, mGData, mMData);
	        SensorManager.getOrientation(mR, mOrientation);
	        float incl = SensorManager.getInclination(mI);
	        final float rad2deg = (float)(180.0f/Math.PI);
	        mOrientation[0] =  (mOrientation[0]*rad2deg);
	        if (mOrientation[0] < 0)
	        	mOrientation[0] = 360.0f + mOrientation[0];	        
	        mOrientation[1] =  (mOrientation[1]*rad2deg);
	        mOrientation[2] = -(mOrientation[2]*rad2deg);
	        	
	               
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
				
				for (int idx = 0; idx < 3; idx++) 
				{
					diff[idx] = Math.abs(mOData[idx] - mOrientation[idx]);
					if (diff[idx] > max[idx])
						max[idx] = diff[idx];
					if (diff[idx] < min[idx])
						min[idx] = diff[idx];
					if (diff[idx] > 3.0)
						status[idx] = "   *";
					else
						status[idx] = "";
				}
				mBuffer = mOData[0] +"\n"+ mOrientation[0] + "\n" + diff[0] + status[0] + "\n" + max[0] + "\n" + min[0] +"\n\n" + 
						  mOData[1] +"\n"+ mOrientation[1] + "\n" + diff[1] + status[1] + "\n" + max[1] + "\n" + min[1] +"\n\n" +
						  mOData[2] +"\n"+ mOrientation[2] + "\n" + diff[2] + status[2] + "\n" + max[2] + "\n" + min[2];
				yaw_raw.setText(mBuffer);
				
				/*
				yaw_raw.setText(mOData[0] + "");
				yaw_cal.setText(mOrientation[0] + "");
				yaw_max.setText(max[0] + "");				
				yaw_min.setText(min[0] + "");

				pit_raw.setText(mOData[1] + "");
				pit_cal.setText(mOrientation[1] + "");
				pit_max.setText(max[1] + "");				
				pit_min.setText(min[1] + "");

				rol_raw.setText(mOData[2] + "");
				rol_cal.setText(mOrientation[2] + "");
				rol_max.setText(max[2] + "");				
				rol_min.setText(min[2] + "");
				*/
				
				//mBuffer = String.format("X: %+6.3f (%+6.3f)%s\nY: %+6.3f (%+6.3f)%s\nZ: %+6.3f (%+6.3f)%s",
				//		  mOData[0], mOrientation[0], mYawStr, 
				//		  mOData[1], mOrientation[1], mPitchStr,
				//		  mOData[2], mOrientation[2], mRollStr);
				//mBuffer = "X: " + (event.values[DATA_X]+cx) + " ("+cx+")\n" + 
				//		  "Y: " + (event.values[DATA_Y]+cy) + " ("+cy+")\n" +
				//		  "Z: " + (event.values[DATA_Z]+cz) + " ("+cz+")\n";
				//mBuffer = "Y: " + mOData[0] +" ("+ mOrientation[0] + "," + diff_yaw + ") " + mYawStr+"\n" + 
				//		  "P: " + mOData[1] +" ("+ mOrientation[1] + "," + diff_pitch + ") " + mPitchStr+"\n" +
				//		  "R: " + mOData[2] +" ("+ mOrientation[2] + "," + diff_roll + ") " + mRollStr;
				//yaw_raw.setText(mBuffer);
				//xLabel.setText(String.format("X: %+2.5f (%+2.5f)", (event.values[DATA_X]+cx), cx));
				//yLabel.setText(String.format("Y: %+2.5f (%+2.5f)", (event.values[DATA_Y]+cy), cy));
				//zLabel.setText(String.format("Z: %+2.5f (%+2.5f)", (event.values[DATA_Z]+cz), cz));
			}
			simpleEventHandler(event);
		}
	};
}