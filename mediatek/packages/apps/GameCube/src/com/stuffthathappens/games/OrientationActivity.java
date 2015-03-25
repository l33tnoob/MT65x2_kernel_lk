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
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;


public class OrientationActivity extends Activity{
	public final static String TAG = "Game";
	private TextView mOrientation;
	private TextView mAngle;
	private SensorManager mSensorManager = null; 
	private Sensor mSensor = null;
	/*
	 * Rate: SENSOR_DELAY_FASTEST  	(0ms) 
	 * 		 SENSOR_DELAY_GAME	 	(20ms)
	 * 		 SENSOR_DELAY_UI		(60ms)
	 *       SENSOR_DELAY_NORMAL	(200ms)
	 */
	private int mRate = SensorManager.SENSOR_DELAY_UI;
	private boolean mShowLog = false;
	private Toast mPrompt = null;
	
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.orient);
        mOrientation = (TextView)findViewById(R.id.orientation);
        mAngle = (TextView)findViewById(R.id.angle) ;
		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	        
    }

	@Override
	protected void onPause() {
		super.onPause();	
		mSensorManager.unregisterListener(mSensorEventListener);
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

		if (mSensor != null)	
			mSensorManager.registerListener(mSensorEventListener, mSensor, mRate);		
		
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
	
	private void showMessage(String str) {
        if (mPrompt == null)	        	
        	mPrompt = Toast.makeText(OrientationActivity.this, str, Toast.LENGTH_SHORT);
        else
        	mPrompt.setText(str);
        mPrompt.show();	
	}
	
	public final SensorEventListener mSensorEventListener = new SensorEventListener() {
		private long lastUpdate = -1;		

		/***************************************************************************
		/*  Copied from WindowOrientation.java
		/**************************************************************************/		
        private static final int _DATA_X = 0;
        private static final int _DATA_Y = 1;
        private static final int _DATA_Z = 2;
        // Angle around x-axis thats considered almost perfect vertical to hold
        // the device
        private static final int PIVOT = 20;
        // Angle around x-asis that's considered almost too vertical. Beyond
        // this angle will not result in any orientation changes. f phone faces uses,
        // the device is leaning backward.
        private static final int PIVOT_UPPER = 65;
        // Angle about x-axis that's considered negative vertical. Beyond this
        // angle will not result in any orientation changes. If phone faces uses,
        // the device is leaning forward.
        private static final int PIVOT_LOWER = -10;
        // Upper threshold limit for switching from portrait to landscape
        private static final int PL_UPPER = 295;
        // Lower threshold limit for switching from landscape to portrait
        private static final int LP_LOWER = 320;
        // Lower threshold limt for switching from portrait to landscape
        private static final int PL_LOWER = 270;
        // Upper threshold limit for switching from landscape to portrait
        private static final int LP_UPPER = 359;
        // Minimum angle which is considered landscape
        private static final int LANDSCAPE_LOWER = 235;
        // Minimum angle which is considered portrait
        private static final int PORTRAIT_LOWER = 60;
        
        // Internal value used for calculating linear variant
        private static final float PL_LF_UPPER =
            ((float)(PL_UPPER-PL_LOWER))/((float)(PIVOT_UPPER-PIVOT));
        private static final float PL_LF_LOWER =
            ((float)(PL_UPPER-PL_LOWER))/((float)(PIVOT-PIVOT_LOWER));
        //  Internal value used for calculating linear variant
        private static final float LP_LF_UPPER =
            ((float)(LP_UPPER - LP_LOWER))/((float)(PIVOT_UPPER-PIVOT));
        private static final float LP_LF_LOWER =
            ((float)(LP_UPPER - LP_LOWER))/((float)(PIVOT-PIVOT_LOWER));
        
        private final int ROTATION_000 = 0;
        private final int ROTATION_090 = 1;
        private int mSensorRotation = -1;        
        
		private void orientationHandler(SensorEvent event) {
			final boolean VERBOSE = true;
            float[] values = event.values;
            float X = values[_DATA_X];
            float Y = values[_DATA_Y];
            float Z = values[_DATA_Z];
            float OneEightyOverPi = 57.29577957855f;
            float gravity = (float) Math.sqrt(X*X+Y*Y+Z*Z);
            float zyangle = (float)Math.asin(Z/gravity)*OneEightyOverPi;
            int rotation = -1;
            float angle = (float)Math.atan2(Y, -X) * OneEightyOverPi;
            int orientation = 90 - (int)Math.round(angle);
            while (orientation >= 360) {
                orientation -= 360;
            } 
            while (orientation < 0) {
                orientation += 360;
            }
            mOrientation.setText(String.format("Orientation: %d", orientation));
            mAngle.setText(String.format("Z-Angle    : %2.4f", zyangle));            
            if ((zyangle <= PIVOT_UPPER) && (zyangle >= PIVOT_LOWER)) {
                // Check orientation only if the phone is flat enough
                // Don't trust the angle if the magnitude is small compared to the y value
            	/*
                float angle = (float)Math.atan2(Y, -X) * OneEightyOverPi;
                int orientation = 90 - (int)Math.round(angle);
                 normalize to 0 - 359 range
                while (orientation >= 360) {
                    orientation -= 360;
                } 
                while (orientation < 0) {
                    orientation += 360;
                }
                mOrientation.setText(String.format("Orientation: %d", orientation));
                */                
                // Orientation values between  LANDSCAPE_LOWER and PL_LOWER
                // are considered landscape.
                // Ignore orientation values between 0 and LANDSCAPE_LOWER
                // For orientation values between LP_UPPER and PL_LOWER,
                // the threshold gets set linearly around PIVOT.
                if ((orientation >= PL_LOWER) && (orientation <= LP_UPPER)) {
                    float threshold;
                    float delta = zyangle - PIVOT;
                    if (mSensorRotation == ROTATION_090) {
                        if (delta < 0) {
                            // Delta is negative
                            threshold = LP_LOWER - (LP_LF_LOWER * delta);
                        } else {
                            threshold = LP_LOWER + (LP_LF_UPPER * delta);
                        }
                        rotation = (orientation >= threshold) ? ROTATION_000 : ROTATION_090;
                        if (VERBOSE) Log.v(TAG, String.format("CASE1. %2.4f %d %2.4f %d", delta, orientation, threshold, rotation));
                    } else {
                        if (delta < 0) {
                            // Delta is negative
                            threshold = PL_UPPER+(PL_LF_LOWER * delta);
                        } else {
                            threshold = PL_UPPER-(PL_LF_UPPER * delta);
                        }
                        rotation = (orientation <= threshold) ? ROTATION_090: ROTATION_000;
                        if (VERBOSE) Log.v(TAG, String.format("CASE2. %2.4f %d %2.4f %d", delta, orientation, threshold, rotation));
                    }
                } else if ((orientation >= LANDSCAPE_LOWER) && (orientation < LP_LOWER)) {
                    rotation = ROTATION_090;
                    if (VERBOSE) Log.v(TAG, String.format("CASE3. 90 (%d)", orientation));
                } else if ((orientation >= PL_UPPER) || (orientation <= PORTRAIT_LOWER)) {
                    rotation = ROTATION_000;
                    if (VERBOSE) Log.v(TAG, String.format("CASE4. 00 (%d)", orientation));                    
                } else {
                	if (VERBOSE) Log.v(TAG, "CASE5. "+orientation);
                }
                if ((rotation != -1) && (rotation != mSensorRotation)) {
                    mSensorRotation = rotation;
                    if (mSensorRotation == ROTATION_000) {
                		//setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                    	Log.w(TAG, "Rotation: 00");
                    	showMessage("Rotate 00: Portrait");
                    }
                    else if (mSensorRotation == ROTATION_090) 
                    {
                		//setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);                    	
                    	Log.w(TAG, "Rotation: 90");
                    	showMessage("Rotate 90: LandScape");
                    }                    
                }
            } else {
            	//Log.v(TAG, String.format("Invalid Z-Angle: %2.4f (%d %d)", zyangle, PIVOT_LOWER, PIVOT_UPPER));
            }
		}
		
		// from the android.hardware.SensorEventListener interface
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
					orientationHandler(event);
				}
			}
		}

		// from the android.hardware.SensorListener interface		
		public void onAccuracyChanged(Sensor sensor, int accuracy) {

		}
		
	};	
}
