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
import android.widget.TextView;
import android.widget.Toast;

public class AlsPsCurveActivity extends Activity implements Callback{
	public final static String TAG = "Game";
	private SurfaceView mSurface;
	private SurfaceHolder mHolder;
	private Paint mBackPaint;
	private Paint mPaintAls;
	private Paint mPaintPs;
	private Paint mPaintLine;
	private SensorManager mSensorManager = null; 
	private Sensor mProximitySensor = null;
	private Sensor mLightSensor = null;
	private int mRate = SensorManager.SENSOR_DELAY_UI;
	public final Object LOCK = new Object();
	private final int mSampleMax = 400;
	private final int DATA_ALS = 0;
	private final int DATA_PS = 1;
	private int mSampleNum = 0;
	private int mSampleIdx = 0;
	private int BALL_RADIUS = 1;
	private float mLastValue[];
	private float mSampleAls[];
	private float mSamplePs[];
	private GameLoop mGameLoop;
	private boolean mShowLog = false;
	private boolean bReady = false;
	private boolean bALSReady = false;
	private boolean bPSReady = false;
	private TextView mRawOutput;
	private int mColorALS = Color.RED;
	private int mColorPS  = Color.GREEN;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
    	
    	setContentView(R.layout.alsps_curve);
    	
    	mSurface = (SurfaceView) findViewById(R.id.curve_surface);
    	mHolder = mSurface.getHolder();
    	mSurface.getHolder().addCallback(this);
    	
    	mBackPaint = new Paint();
    	mBackPaint.setColor(Color.BLACK);
		mPaintAls = new Paint();
		mPaintAls.setColor(mColorALS);
		mPaintPs = new Paint();
		mPaintPs.setColor(mColorPS);
		mPaintLine = new Paint();
		
    	mSampleAls = new float[mSampleMax];
    	mSamplePs = new float[mSampleMax];
    	mLastValue = new float[] {0.0f, 0.0f, 0.0f};
    	
    	mRawOutput = (TextView) findViewById(R.id.label_alsps);
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
		mProximitySensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
		mLightSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
	
		if (mProximitySensor != null || mLightSensor != null) {
			mSensorManager.registerListener(mSensorEventListener, mProximitySensor, mRate);
			mSensorManager.registerListener(mSensorEventListener, mLightSensor, mRate);
			bReady = true;
		} else {
			bReady = false;
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
	
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) 
	{
		if (bReady == true) {
			mGameLoop = new GameLoop();
			mGameLoop.start();	
		} else {
            Toast.makeText(this, "Sensor Not found", Toast.LENGTH_SHORT).show();
		}
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
	
	private String mBuffer;
	private void doDraw(Canvas c) {
		int top = mSurface.getTop();
		int width = mSurface.getWidth();
		int height = mSurface.getHeight();
		int idx;
		float ballX, prevX = 0;
		float ballY[] = new float[3];
		float prevY[] = new float[3];
		int offset = 1;
		int base = 20;
		float unit_ps = (height-base)/4.0f; 
		float unit_als = (height-base)/10880.0f;
		int font_size = 12;
		
		c.drawRect(0, 0, width, height, mBackPaint);
		mPaintLine.setColor(Color.WHITE);
		c.drawLine(0, height-base, width, height-base, mPaintLine);		
		mPaintLine.setTextSize(font_size);
		mPaintLine.setColor(mColorALS);
		c.drawText("als", 2, (height-base)+font_size, mPaintLine);
		mPaintLine.setColor(mColorPS);
		c.drawText("ps", 22, (height-base)+font_size, mPaintLine);
		
		//Log.d(TAG, "dimension: "+top+","+width+","+height+","+base);
		
		synchronized (LOCK){			
			mSampleAls[mSampleIdx] = mLastValue[DATA_ALS];
			mSamplePs[mSampleIdx] = mLastValue[DATA_PS];
			mSampleIdx = (mSampleIdx+1)%mSampleMax;
			if (bPSReady && bALSReady && (mSampleNum < mSampleMax))
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
			
			ballY[DATA_ALS] = (height-base-mSampleAls[pos]*unit_als);
			ballY[DATA_PS] = (height-base-mSamplePs[pos]*unit_ps);

			c.drawCircle(ballX, ballY[DATA_ALS], BALL_RADIUS, mPaintAls);
			c.drawCircle(ballX, ballY[DATA_PS], BALL_RADIUS, mPaintPs);			
			if (idx != 0) 
			{
				c.drawLine(prevX, prevY[DATA_ALS], ballX, ballY[DATA_ALS], mPaintAls);
				c.drawLine(prevX, prevY[DATA_PS], ballX, ballY[DATA_PS], mPaintPs);
			}					
			prevX = ballX;
			prevY[DATA_ALS] = ballY[DATA_ALS];
			prevY[DATA_PS] = ballY[DATA_PS];
		}		
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		try {
			if (mGameLoop != null)
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
		
		// from the android.hardware.SensorEventListener interface
		public void onSensorChanged(SensorEvent event) {
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_PROXIMITY) {
				if (mShowLog)
					Log.d(TAG, "("+type+")" + event.values[0]);
				synchronized(LOCK) {
					mLastValue[DATA_PS] = event.values[0];
					bPSReady = true;
				}				
			} else if (type == Sensor.TYPE_LIGHT) {
				if (mShowLog)
					Log.d(TAG, "("+type+")" + event.values[0]);
				synchronized(LOCK) {
					mLastValue[DATA_ALS] = event.values[0];
					bALSReady = true;
				}				
			}
			mBuffer = "ALS: "+mLastValue[DATA_ALS]+"\t"+"PS: "+mLastValue[DATA_PS];
			mRawOutput.setText(mBuffer);				
		}

		// from the android.hardware.SensorListener interface		
		public void onAccuracyChanged(Sensor sensor, int accuracy) {

		}
		
	};		
}
