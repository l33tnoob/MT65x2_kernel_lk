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

/*
 * Copyright (C) 2007 The Android Open Source Project
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
package com.stuffthathappens.games;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.*;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Config;
import android.util.Log;
import android.view.View;

public class MagCompass extends GraphicsActivity {

    private static final String TAG = "Compass";

	private SensorManager mSensorManager;
	private Sensor mAcc;
	private Sensor mMag;
	private Sensor mOri;
    private SampleView mView;
    private float[] mGData = new float[3];
    private float[] mMData = new float[3];
    private float[] mOData = new float[3];
    private float[] mR = new float[16];
    private float[] mI = new float[16];
    private float[] mOrientation = new float[3]; 
    private int mCount;
    
    private final SensorEventListener mListener = new SensorEventListener() {
    
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
	// some test code which will be used/cleaned up before we ship this.
//	        SensorManager.remapCoordinateSystem(mR,
//	                SensorManager.AXIS_X, SensorManager.AXIS_Z, mR);
//	        SensorManager.remapCoordinateSystem(mR,
//	                SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X, mR);
	        SensorManager.getOrientation(mR, mOrientation);
	        float incl = SensorManager.getInclination(mI);
	        final float rad2deg = (float)(180.0f/Math.PI);
	        mOrientation[0] =  (mOrientation[0]*rad2deg);
	        mOrientation[1] =  (mOrientation[1]*rad2deg);
	        mOrientation[2] = -(mOrientation[2]*rad2deg);
	        	
	        //Log.d("TAG", "MAG (" + mOrientation[0] + ", " + mOrientation[1] + ", " + mOrientation[2] + ")");
	        //Log.d("TAG", "ORI (" + mOData[0] + ", " + mOData[1] + ", " + mOData[2] + ")");
	        
            if (mView != null) {
                mView.invalidate();
            }
	        	
            /*
	            Log.d("Compass", "yaw: " + (int)(mOrientation[0]*rad2deg) +
	                    "  pitch: " + (int)(mOrientation[1]*rad2deg) +
	                    "  roll: " + (int)(mOrientation[2]*rad2deg) +
	                    "  incl: " + (int)(incl*rad2deg)
	                    );
	        */

		}    	
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
			// TODO Auto-generated method stub
        }
    };

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mSensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        mView = new SampleView(this);
        setContentView(mView);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    }

    @Override
    protected void onResume()
    {
        Log.d(TAG, "onResume");
        super.onResume();		
		mAcc = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		mMag = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
		mOri = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
		boolean bAccSupported = mSensorManager.registerListener(mListener, mAcc, SensorManager.SENSOR_DELAY_GAME);
		boolean bMagSupported = mSensorManager.registerListener(mListener, mMag, SensorManager.SENSOR_DELAY_GAME);
		boolean bOriSupported = mSensorManager.registerListener(mListener, mOri, SensorManager.SENSOR_DELAY_GAME);
		Log.d(TAG, "AccSupport: "+bAccSupported+"\t"+ "MagSupport: "+bMagSupported + "\t"+ "OriSupport: "+bOriSupported);
    }
    
    @Override
    protected void onStop()
    {
        Log.d(TAG, "onStop");
        mSensorManager.unregisterListener(mListener);
        super.onStop();
    }

    private class SampleView extends View {
        private Paint   mPaint = new Paint();
        private Path    mPath = new Path();
        private boolean mAnimate;
        private long    mNextTime;

        public SampleView(Context context) {
            super(context);

            // Construct a wedge-shaped path
            mPath.moveTo(0, -50);
            mPath.lineTo(-20, 60);
            mPath.lineTo(0, 50);
            mPath.lineTo(20, 60);
            mPath.close();
        }
    
        @Override protected void onDraw(Canvas canvas) {
            Paint paint = mPaint;

            canvas.drawColor(Color.WHITE);
            
            paint.setAntiAlias(true);
            paint.setColor(Color.BLACK);
            paint.setStyle(Paint.Style.FILL);

            int w = canvas.getWidth();
            int h = canvas.getHeight();
            int cx = w / 2;
            int cy = h / 2;

            canvas.translate(cx, cy);
            if (mOrientation != null) {    
                canvas.rotate(-mOrientation[0]);
            }
            canvas.drawPath(mPath, mPaint);
        }
    
        @Override
        protected void onAttachedToWindow() {
            mAnimate = true;
            super.onAttachedToWindow();
        }
        
        @Override
        protected void onDetachedFromWindow() {
            mAnimate = false;
            super.onDetachedFromWindow();
        }
    }
}

