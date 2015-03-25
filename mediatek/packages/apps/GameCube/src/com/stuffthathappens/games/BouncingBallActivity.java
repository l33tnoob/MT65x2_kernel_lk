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
import static android.hardware.SensorManager.SENSOR_DELAY_GAME;

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
import android.os.Vibrator;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.SurfaceHolder.Callback;

/**
 * This activity shows a ball that bounces around. The phone's 
 * accelerometer acts as gravity on the ball. When the ball hits
 * the edge, it bounces back and triggers the phone vibrator.
 */
public class BouncingBallActivity extends Activity implements Callback{
	private final static String TAG = "Game";
	private static final int BALL_RADIUS = 15;
	private SurfaceView surface;
	private SurfaceHolder holder;
	private final BouncingBallModel model = new BouncingBallModel(BALL_RADIUS);
	private GameLoop gameLoop;
	private Paint backgroundPaint;
	private Paint ballPaint;
	private SensorManager sensorMgr;
	private long lastSensorUpdate = -1;
	private Sensor mSensor;
	private boolean mShowLog = false;	

	@Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
    	
    	setContentView(R.layout.bouncing_ball);
    	
    	surface = (SurfaceView) findViewById(R.id.bouncing_ball_surface);
    	holder = surface.getHolder();
    	surface.getHolder().addCallback(this);
    	
    	backgroundPaint = new Paint();
		backgroundPaint.setColor(Color.WHITE);

		ballPaint = new Paint();
		ballPaint.setColor(Color.BLUE);
		ballPaint.setAntiAlias(true);
		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
    }
    
	@Override
	protected void onPause() {
		super.onPause();
		
		model.setVibrator(null);
		
		sensorMgr.unregisterListener(mSensorEventListener);
		sensorMgr = null;
		
		model.setAccel(0, 0);
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		sensorMgr = (SensorManager) getSystemService(SENSOR_SERVICE);
		mSensor = sensorMgr.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		boolean accelSupported = sensorMgr.registerListener(
				mSensorEventListener, mSensor, SENSOR_DELAY_GAME);
		
		if (!accelSupported) {
			// on accelerometer on this device
			sensorMgr.unregisterListener(mSensorEventListener);
			// TODO show an error
		}
		
		// NOTE 1: you cannot get system services before onCreate()
		// NOTE 2: AndroidManifest.xml must contain this line:
		// <uses-permission android:name="android.permission.VIBRATE"/>
		Vibrator vibrator = (Vibrator) getSystemService(Activity.VIBRATOR_SERVICE);
		model.setVibrator(vibrator);
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
			int height) {
		
		model.setSize(width, height);
	}

	public void surfaceCreated(SurfaceHolder holder) {
		gameLoop = new GameLoop();
		gameLoop.start();
	}
	
	private void draw() {
		// TODO thread safety - the SurfaceView could go away while we are drawing
		
		Canvas c = null;
		try {
			// NOTE: in the LunarLander they don't have any synchronization here,
			// so I guess this is OK. It will return null if the holder is not ready
			c = holder.lockCanvas();
			
			// TODO this needs to synchronize on something
			if (c != null) {
				doDraw(c);
			}
		} finally {
			if (c != null) {
				holder.unlockCanvasAndPost(c);
			}
		}
	}
	
	private void doDraw(Canvas c) {
		int width = c.getWidth();
		int height = c.getHeight();
		c.drawRect(0, 0, width, height, backgroundPaint);
		
		float ballX, ballY;
		boolean bounced;
		synchronized (model.LOCK) {
			ballX = model.ballPixelX;
			ballY = model.ballPixelY;
			bounced = model.bounced;
		}
		if (bounced) {
			ballPaint.setColor(Color.GREEN);
			c.drawCircle(ballX, ballY, BALL_RADIUS, ballPaint);
		} else {
			ballPaint.setColor(Color.BLUE);
			c.drawCircle(ballX, ballY, BALL_RADIUS, ballPaint);
		}
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		try {
			model.setSize(0,0);
			gameLoop.safeStop();
		} finally {
			gameLoop = null;
		}
	}
    
	private class GameLoop extends Thread {
		private volatile boolean running = true;
		
		public void run() {
			while (running) {
				try {
					// TODO don't like this hardcoding
					TimeUnit.MILLISECONDS.sleep(5);
					
					draw();
					model.updatePhysics();

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
		public void onAccuracyChanged(Sensor sensor, int accuracy) {		
		}
	
		public void onSensorChanged(SensorEvent event){
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_ACCELEROMETER) {
				long curTime = System.currentTimeMillis();
				// only allow one update every 50ms, otherwise updates
				// come way too fast
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
				if (lastSensorUpdate == -1 || (curTime - lastSensorUpdate) > 50) {
					lastSensorUpdate = curTime;
					
					model.setAccel(event.values[DATA_X], event.values[DATA_Y]);
				}
			}
		}
	};
}
