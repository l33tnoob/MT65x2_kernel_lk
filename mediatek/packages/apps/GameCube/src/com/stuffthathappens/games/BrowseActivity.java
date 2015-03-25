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

import android.app.Activity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

public class BrowseActivity extends Activity{
	public final static String TAG = "Game";
	private TextView mStatusText;
	private SensorManager mSensorManager = null; 
	private Sensor mSensor = null;
	private Toast mPrompt = null;
	/*
	 * Rate: SENSOR_DELAY_FASTEST  	(0ms) 
	 * 		 SENSOR_DELAY_GAME	 	(20ms)
	 * 		 SENSOR_DELAY_UI		(60ms)
	 *       SENSOR_DELAY_NORMAL	(200ms)
	 */
	private int mRate = SensorManager.SENSOR_DELAY_UI;
	
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.browse);
        mStatusText = (TextView)findViewById(R.id.status);            
    }

	@Override
	protected void onPause() {
		super.onPause();	
		mSensorManager.unregisterListener(mSensorEventListener);
		if (mPrompt != null)
			mPrompt.cancel();		
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

		if (mSensor != null)	
			mSensorManager.registerListener(mSensorEventListener, mSensor, mRate);		
		
	}	
	
	public final SensorEventListener mSensorEventListener = new SensorEventListener() {
		private long lastUpdate = -1;		

		/***************************************************************************
		/*  Copied from BrowserActivity.java
		/**************************************************************************/				
        private float[] mPrev = new float[3];
        private float[] mPrevDiff = new float[3];
        private float[] mDiff = new float[3];
        private float[] mRevertDiff = new float[3];
        private String mStatus = "";
		private void browserHandler(SensorEvent event)
		{
            boolean show = false;
            float[] diff = new float[3];

            for (int i = 0; i < 3; i++) {
                diff[i] = event.values[i] - mPrev[i];
                if (Math.abs(diff[i]) > 1) {
                    //show = true;
                }
                if ((diff[i] > 1.0 && mDiff[i] < 0.2)
                        || (diff[i] < -1.0 && mDiff[i] > -0.2)) {
                    // start track when there is a big move, or revert
                    mRevertDiff[i] = mDiff[i];
                    mDiff[i] = 0;
                } else if (diff[i] > -0.2 && diff[i] < 0.2) {
                    // reset when it is flat
                    mDiff[i] = mRevertDiff[i]  = 0;
                }
                mDiff[i] += diff[i];
                mPrevDiff[i] = diff[i];
                mPrev[i] = event.values[i];
            }

            if (show) {
                // only shows if we think the delta is big enough, in an attempt
                // to detect "serious" moves left/right or up/down
                Log.d("BrowserSensorHack", "sensorChanged " + event.sensor.getType() + " ("
                        + event.values[0] + ", " + event.values[1] + ", " + event.values[2] + ")"
                        + " diff(" + diff[0] + " " + diff[1] + " " + diff[2]
                        + ")");
                Log.d("BrowserSensorHack", "      mDiff(" + mDiff[0] + " "
                        + mDiff[1] + " " + mDiff[2] + ")" + " mRevertDiff("
                        + mRevertDiff[0] + " " + mRevertDiff[1] + " "
                        + mRevertDiff[2] + ")");
            }


            float y = mDiff[1];
            float z = mDiff[2];
            float ay = Math.abs(y);
            float az = Math.abs(z);
            float ry = mRevertDiff[1];
            float rz = mRevertDiff[2];
            float ary = Math.abs(ry);
            float arz = Math.abs(rz);
            boolean gestY = ay > 2.5f && ary > 1.0f && ay > ary;
            boolean gestZ = az > 3.5f && arz > 1.0f && az > arz;

            if ((gestY || gestZ) && !(gestY && gestZ)) {

                if (gestZ) {
                    if (z < 0) {
                        mStatus = "ZoomOut";
                    } else {
                        mStatus = "ZoomIn";
                    }
                } else {
                    mStatus = "flingScroll:" + Math.round(y * 100);
                }
                Log.w(TAG, mStatus);
    	        if (mPrompt == null)	        	
    	        	mPrompt = Toast.makeText(BrowseActivity.this, mStatus, Toast.LENGTH_SHORT);
    	        else
    	        	mPrompt.setText(mStatus);
    	        mPrompt.show();
                
            } else {
            	//mStatus = "NONE";
            	mStatusText.setText("NONE");
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
				//Log.d(TAG, "onSensorChanged("+type+")");			
				if (lastUpdate == -1 || (curTime - lastUpdate) > 100) {
					lastUpdate = curTime;				
					browserHandler(event);
				}
			}
		}

		// from the android.hardware.SensorListener interface		
		public void onAccuracyChanged(Sensor sensor, int accuracy) {

		}
		
	};	
}
