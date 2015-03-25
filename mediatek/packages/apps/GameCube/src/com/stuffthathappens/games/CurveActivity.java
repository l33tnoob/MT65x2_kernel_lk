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
import android.view.SurfaceHolder.Callback;

public class CurveActivity extends Activity implements Callback{
	public final static String TAG = "Game";
	private SurfaceView mSurface;
	private SurfaceHolder mHolder;
	private Paint mBackPaint;
	private Paint mPaintX;
	private Paint mPaintY;
	private Paint mPaintZ;
	private Paint mPaintLine;
	private SensorManager mSensorManager = null; 
	private Sensor mSensor = null;	
	private int mRate = SensorManager.SENSOR_DELAY_GAME;
	public final Object LOCK = new Object();
	private final int mSampleMax = 400;
	private int mSampleNum = 0;
	private int mSampleIdx = 0;
	private int BALL_RADIUS = 1;
	private float mLastValue[];
	private float mSampleX[];
	private float mSampleY[];
	private float mSampleZ[];
	private GameLoop mGameLoop;
	private boolean mShowLog = false;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
    	
    	setContentView(R.layout.curve);
    	
    	mSurface = (SurfaceView) findViewById(R.id.curve_surface);
    	mHolder = mSurface.getHolder();
    	mSurface.getHolder().addCallback(this);
    	
    	mBackPaint = new Paint();
    	mBackPaint.setColor(Color.BLACK);
		mPaintX = new Paint();
		mPaintX.setColor(Color.RED);
		mPaintY = new Paint();
		mPaintY.setColor(Color.GREEN);
		mPaintZ = new Paint();
		mPaintZ.setColor(Color.BLUE);
		mPaintLine = new Paint();
		
    	mSampleX = new float[mSampleMax];
    	mSampleY = new float[mSampleMax];
    	mSampleZ = new float[mSampleMax];
    	mLastValue = new float[3];
		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	    	
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
		float ballX, prevX = 0;
		float ballY[] = new float[3];
		float prevY[] = new float[3];
		int offset = 1;
		/*each axis occpies height/3, and each axis needs 4g resolution*/
		float unit = height/120.0f; 
		
		c.drawRect(0, 0, width, height, mBackPaint);
		//mPaintLine.setColor(Color.GRAY);
		//c.drawRect(0, 0, width/3, height/3, mPaintLine);
		//c.drawRect(width/3, height/3, 2*width/3, 2*height/3, mPaintLine);		
		//c.drawRect(2*width/3, 2*height/3, width, height, mPaintLine);
		mPaintLine.setColor(Color.WHITE);
		c.drawLine(0, height/6, width, height/6, mPaintLine);
		c.drawLine(0, height/2, width, height/2, mPaintLine);
		c.drawLine(0, 5*height/6, width, 5*height/6, mPaintLine);	
		mPaintLine.setColor(Color.GRAY);
		c.drawLine(0, height/3, width, height/3, mPaintLine);
		c.drawLine(0, 2*height/3, width, 2*height/3, mPaintLine);
		mPaintLine.setTextSize(12);
		c.drawText("X-g", 2, 12, mPaintLine);
		c.drawText("Y-g", 2, (height/3)+ 12, mPaintLine);
		c.drawText("Z-g", 2, (2*height/3) + 12, mPaintLine);		
		
		synchronized (LOCK){
			mSampleX[mSampleIdx] = mLastValue[0];
			mSampleY[mSampleIdx] = mLastValue[1];
			mSampleZ[mSampleIdx] = mLastValue[2];
			mSampleIdx = (mSampleIdx+1)%mSampleMax;
			if (mSampleNum < mSampleMax)
				mSampleNum++;									
		}

		int num = Math.min(width/offset, mSampleNum);		
		int fst = mSampleIdx-num;
		int pos;

		if (fst < 0)
			fst += mSampleMax;
		
		for (idx = 0; idx < num; idx+=offset)
		{
			pos = (fst+idx)%mSampleMax;
			ballX = idx;
			
			ballY[0] = (height/6) + (-1)*mSampleX[pos]*unit;
			ballY[1] = (height/2) + (-1)*mSampleY[pos]*unit;
			ballY[2] = (5*height/6) + (-1)*mSampleZ[pos]*unit;
			//c.drawCircle(ballX, ballY, BALL_RADIUS, mBallPaint);
			c.drawCircle(ballX, ballY[0], BALL_RADIUS, mPaintX);
			c.drawCircle(ballX, ballY[1], BALL_RADIUS, mPaintY);
			c.drawCircle(ballX, ballY[2], BALL_RADIUS, mPaintZ);			
			if (idx != 0) 
			{
				c.drawLine(prevX, prevY[0], ballX, ballY[0], mPaintX);
				c.drawLine(prevX, prevY[1], ballX, ballY[1], mPaintY);
				c.drawLine(prevX, prevY[2], ballX, ballY[2], mPaintZ);
			}					
			prevX = ballX;
			prevY[0] = ballY[0];
			prevY[1] = ballY[1];
			prevY[2] = ballY[2];
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
				if (lastUpdate == -1 || (curTime - lastUpdate) > 40) {
					lastUpdate = curTime;				
					synchronized (LOCK){
						mLastValue[0] = event.values[0];
						mLastValue[1] = event.values[1];
						mLastValue[2] = event.values[2];
					}
				}
			}
		}

		// from the android.hardware.SensorListener interface		
		public void onAccuracyChanged(Sensor sensor, int accuracy) {

		}
		
	};		
}
