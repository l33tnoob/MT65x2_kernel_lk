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

import java.util.concurrent.TimeUnit;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.SurfaceHolder.Callback;
import android.widget.TextView;

class SampleData {
	public float x;
	public float y;
	public float z;
	public float angle;
	public float threshold;
	public float orientation;
	public int rotation;
	
	public SampleData() 
	{
		x = y = z = angle = threshold = orientation = 0.0f;
		rotation = 0;
	}
};
class AccOrientation {
	public float angle;
	public float orientation;
	public float threshold;
	public int   rotation;
	
	public AccOrientation()
	{
		angle = orientation = threshold = 0.0f;
		rotation = 0;
	}
};

public class AccCurveAngleActivity extends Activity implements Callback{
	public final static String TAG = "Game";
	private SurfaceView mSurface;
	private SurfaceHolder mHolder;
	private Paint mBackPaint;
	private Paint mPaintLine;
	private SensorManager mSensorManager = null; 
	private Sensor mSensor = null;	
	private int mRate = SensorManager.SENSOR_DELAY_GAME;
	public final Object LOCK = new Object();
	private final int mSampleMax = 400;
	private int mSampleNum = 0;
	private int mSampleIdx = 0;
	private int BALL_RADIUS = 1;
	private SampleData mLastSample;
	private SampleData mSamples[];
	private GameLoop mGameLoop;
	private boolean mShowLog = false;
	private AccOrientation mLastAccOrientation;
	private final static int DATA_X = 0;
	private final static int DATA_Y = 1;
	private final static int DATA_Z = 2;
	private final static int DATA_ANGLE = 3;
	private final static int DATA_THRESHOLD = 4;
	private final static int DATA_ORIENTATION = 5;
	private Paint mPaintData[];
    private final static int ROTATION_000 = 0;
    private final static int ROTATION_090 = 1;	
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
    	
    	setContentView(R.layout.acc_curve_angle);
    	
    	mSurface = (SurfaceView) findViewById(R.id.curve_surface);
    	mHolder = mSurface.getHolder();
    	mSurface.getHolder().addCallback(this);
    	
    	mBackPaint = new Paint();
    	mBackPaint.setColor(Color.BLACK);
		mPaintLine = new Paint();
		mPaintData = new Paint[6];
		for (int idx = 0; idx < mPaintData.length; idx++)
			mPaintData[idx] = new Paint();
		mPaintData[DATA_X].setColor(Color.RED);
		mPaintData[DATA_Y].setColor(Color.GREEN);
		mPaintData[DATA_Z].setColor(Color.BLUE);
		mPaintData[DATA_ANGLE].setColor(Color.YELLOW);
		mPaintData[DATA_ORIENTATION].setColor(Color.CYAN);
		mPaintData[DATA_THRESHOLD].setColor(Color.MAGENTA);
    	mSamples = new SampleData[mSampleMax];
    	for (int idx = 0; idx < mSamples.length; idx++)
    		mSamples[idx] = new SampleData();
    	mLastSample = new SampleData();
		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	    	
    }
	
	@Override
	protected void onStop() {
		super.onStop();
		for (int idx = 0; idx < mPaintData.length; idx++)
			mPaintData[idx] = null;
		mPaintData = null;
    	for (int idx = 0; idx < mSamples.length; idx++)
    		mSamples[idx] = null;
    	mSamples = null;
    	mLastSample = null;
    	mBackPaint = null;
    	mPaintLine = null;    
	}
    
	@Override
	protected void onPause() {
		super.onPause();
				
		mSensorManager.unregisterListener(mSensorEventListener);
		mSensorManager = null;
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
	
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) 
	{
		mGameLoop = new GameLoop();
		mGameLoop.start();		
	}

	public void surfaceCreated(SurfaceHolder holder) {
		//gameLoop = new GameLoop();
		//gameLoop.start();
	}
	
	private void draw() {
		// TODO thread safety - the SurfaceView could go away while we are drawing		
		Canvas c = null;
		try {
			// NOTE: in the LunarLander they don't have any synchronization here,
			// so I guess this is OK. It will return null if the holder is not ready
			c = mHolder.lockCanvas();
			
			// TODO this needs to synchronize on something
			if (c != null) {
				doDraw(c);
			}
		} finally {
			if (c != null) {
				mHolder.unlockCanvasAndPost(c);
			}
		}
	}
	
	private void doDraw(Canvas c) {
		int width = c.getWidth();
		int height = c.getHeight();
		int idx;
		float curX, preX = 0;
		float curY[] = new float[6];
		float preY[] = new float[6];
		int offset = 1;
		/*each axis occupies height/3, and each axis needs 4g resolution*/
		float unit[] = new float[6]; 
		
		c.drawRect(0, 0, width, height, mBackPaint);
		mPaintLine.setColor(Color.GRAY);
		c.drawLine(0, height/4, width, height/4, mPaintLine);
		c.drawLine(0, 3*height/4, width, 3*height/4, mPaintLine);	
		mPaintLine.setColor(Color.GRAY);
		c.drawLine(0, height/2, width, height/2, mPaintLine);
		mPaintLine.setTextSize(12);
		
		c.drawText("angle", 2, 12,  mPaintData[DATA_ANGLE]);
		c.drawText("orientation", 40, 12, mPaintData[DATA_ORIENTATION]);
		c.drawText("theshold", 100, 12, mPaintData[DATA_THRESHOLD]);

		c.drawText("x", 2, (height/2)+ 12, mPaintData[DATA_X]);
		c.drawText("y", 20, (height/2)+ 12, mPaintData[DATA_Y]);
		c.drawText("z", 40, (height/2)+ 12, mPaintData[DATA_Z]);
		
		unit[DATA_X] = unit[DATA_Y] = unit[DATA_Z] = height/80.0f;
		unit[DATA_ANGLE] = unit[DATA_THRESHOLD] = unit[DATA_ORIENTATION] = height/720.0f;
		synchronized (LOCK){
			mSamples[mSampleIdx].x = mLastSample.x;
			mSamples[mSampleIdx].y = mLastSample.y;
			mSamples[mSampleIdx].z = mLastSample.z;
			mSamples[mSampleIdx].angle = mLastSample.angle;
			mSamples[mSampleIdx].threshold = mLastSample.threshold;
			mSamples[mSampleIdx].orientation = mLastSample.orientation;
			
			mSampleIdx = (mSampleIdx+1)%mSampleMax;
			if (mSampleNum < mSampleMax)
				mSampleNum++;									
		}

		int num = Math.min(width/offset, mSampleNum);		
		int fst = mSampleIdx-num;
		int pos, pt;

		if (fst < 0)
			fst += mSampleMax;
		
		for (idx = 0; idx < num; idx+=offset)
		{
			pos = (fst+idx)%mSampleMax;
			curX = idx;
			
			curY[DATA_X] = (3*height/4) + (-1)*mSamples[pos].x*unit[DATA_X];
			curY[DATA_Y] = (3*height/4) + (-1)*mSamples[pos].y*unit[DATA_Y];
			curY[DATA_Z] = (3*height/4) + (-1)*mSamples[pos].z*unit[DATA_Z];
			curY[DATA_ANGLE] = (1*height/2) + (-1)*mSamples[pos].angle*unit[DATA_ANGLE];
			curY[DATA_THRESHOLD] = (1*height/2) + (-1)*mSamples[pos].threshold*unit[DATA_THRESHOLD];
			curY[DATA_ORIENTATION] = (1*height/2) + (-1)*mSamples[pos].orientation*unit[DATA_ORIENTATION];
						
			for (pt = 0; pt < curY.length; pt++)
				c.drawCircle(curX, curY[pt], BALL_RADIUS, mPaintData[pt]);
			if (idx != 0) 
			{
				for (pt = 0; pt < curY.length; pt++)
					c.drawLine(preX, preY[pt], curX, curY[pt], mPaintData[pt]);
			}					
			preX = curX;
			for (pt = 0; pt < curY.length; pt++)
				preY[pt] = curY[pt];		
		}		
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		try {
			mGameLoop.safeStop();
		} finally {
			mGameLoop = null;
		}
	}	
	
	private class GameLoop extends Thread {
		private volatile boolean running = true;
		
		public void run() {
			while (running) {
				try {
					TimeUnit.MILLISECONDS.sleep(100);
					
					draw();
				} catch (InterruptedException ie) {
					running = false;
				}
			}
		}
		
		public void safeStop() {
			running = false;
			interrupt();
		}
	}	
	
	private String mBuffer;
	public final SensorEventListener mSensorEventListener = new SensorEventListener() {
		private long lastUpdate = -1;		

		
		// from the android.hardware.SensorEventListener interface
		public void onSensorChanged(SensorEvent event) {
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_ACCELEROMETER) {
				long curTime = System.currentTimeMillis();
				// only allow one update every 100ms, otherwise updates
				// come way too fast and the phone gets bogged down
				// with garbage collection
				//Log.d(TAG, "onSensorChanged("+type+")");				
				if (lastUpdate == -1 || (curTime - lastUpdate) > 40) {
					lastUpdate = curTime;				
					synchronized (LOCK){
						mLastAccOrientation = calculateAngle(event);
						mLastSample.x = event.values[0];
						mLastSample.y = event.values[1];
						mLastSample.z = event.values[2];
						mLastSample.angle = mLastAccOrientation.angle;		
						//mLastSample.orientation = mLastAccOrientation.orientation;
						mLastSample.orientation = mLastAccOrientation.rotation*90;
						mLastSample.threshold = mLastAccOrientation.threshold;
						mBuffer = mLastSample.x+","+
								  mLastSample.y+","+
								  mLastSample.z+","+
								  mLastSample.angle+","+
								  mLastSample.orientation+","+
								  mLastSample.threshold;
						if (mShowLog)
							Log.d(TAG, mBuffer);
					}
				}
			}
		}

		// from the android.hardware.SensorListener interface		
		public void onAccuracyChanged(Sensor sensor, int accuracy) {

		}
		
		public AccOrientation calculateAngle(SensorEvent event) {
			/***************************************************************************
			/*  Copied from WindowOrientation.java
			/**************************************************************************/		
	        final int _DATA_X = 0;
	        final int _DATA_Y = 1;
	        final int _DATA_Z = 2;
	        // Angle around x-axis thats considered almost perfect vertical to hold
	        // the device
	        final int PIVOT = 20;
	        // Angle around x-asis that's considered almost too vertical. Beyond
	        // this angle will not result in any orientation changes. f phone faces uses,
	        // the device is leaning backward.
	        final int PIVOT_UPPER = 65;
	        // Angle about x-axis that's considered negative vertical. Beyond this
	        // angle will not result in any orientation changes. If phone faces uses,
	        // the device is leaning forward.
	        final int PIVOT_LOWER = -10;
	        // Upper threshold limit for switching from portrait to landscape
	        final int PL_UPPER = 295;
	        // Lower threshold limit for switching from landscape to portrait
	        final int LP_LOWER = 320;
	        // Lower threshold limt for switching from portrait to landscape
	        final int PL_LOWER = 270;
	        // Upper threshold limit for switching from landscape to portrait
	        final int LP_UPPER = 359;
	        // Minimum angle which is considered landscape
	        final int LANDSCAPE_LOWER = 235;
	        // Minimum angle which is considered portrait
	        final int PORTRAIT_LOWER = 60;
	        
	        // Internal value used for calculating linear variant
	        final float PL_LF_UPPER =
	            ((float)(PL_UPPER-PL_LOWER))/((float)(PIVOT_UPPER-PIVOT));
	        final float PL_LF_LOWER =
	            ((float)(PL_UPPER-PL_LOWER))/((float)(PIVOT-PIVOT_LOWER));
	        //  Internal value used for calculating linear variant
	        final float LP_LF_UPPER =
	            ((float)(LP_UPPER - LP_LOWER))/((float)(PIVOT_UPPER-PIVOT));
	        final float LP_LF_LOWER =
	            ((float)(LP_UPPER - LP_LOWER))/((float)(PIVOT-PIVOT_LOWER));
	        
	        int mSensorRotation = -1;        
	        
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
            AccOrientation result = new AccOrientation();
            
            while (orientation >= 360) {
                orientation -= 360;
            } 
            while (orientation < 0) {
                orientation += 360;
            }
            result.orientation = orientation;
            result.angle = zyangle;  
            result.threshold = 0;
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
                        if (mShowLog) 
                        	Log.v(TAG, String.format("CASE1. %2.4f %d %2.4f %d", delta, orientation, threshold, rotation));
                    } else {
                        if (delta < 0) {
                            // Delta is negative
                            threshold = PL_UPPER+(PL_LF_LOWER * delta);
                        } else {
                            threshold = PL_UPPER-(PL_LF_UPPER * delta);
                        }
                        rotation = (orientation <= threshold) ? ROTATION_090: ROTATION_000;
                        if (mShowLog)
                        	Log.v(TAG, String.format("CASE2. %2.4f %d %2.4f %d", delta, orientation, threshold, rotation));
                    }
                    result.threshold = threshold;
                } else if ((orientation >= LANDSCAPE_LOWER) && (orientation < LP_LOWER)) {
                    rotation = ROTATION_090;
                    if (mShowLog)
                    	Log.v(TAG, String.format("CASE3. 90 (%d)", orientation));
                } else if ((orientation >= PL_UPPER) || (orientation <= PORTRAIT_LOWER)) {
                    rotation = ROTATION_000;
                    if (mShowLog)
                    	Log.v(TAG, String.format("CASE4. 00 (%d)", orientation));                    
                } else {
                	if (mShowLog)
                		Log.v(TAG, "CASE5. "+orientation);
                }
                if ((rotation != -1) && (rotation != mSensorRotation)) {
                    mSensorRotation = rotation;
                    if (mSensorRotation == ROTATION_000) {
                		//setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                    	if (mShowLog)
                    		Log.w(TAG, "Rotation: 00");
                    }
                    else if (mSensorRotation == ROTATION_090) 
                    {
                		//setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                    	if (mShowLog)
                    		Log.w(TAG, "Rotation: 90");
                    }                    
                }
                result.rotation = rotation;
            } else {
            	//Log.v(TAG, String.format("Invalid Z-Angle: %2.4f (%d %d)", zyangle, PIVOT_LOWER, PIVOT_UPPER));
            }	
            return result;
		}
		
	};		
}
