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

package com.mediatek.engineermode.sensor;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;

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

public class MSensorCurrentActivity extends Activity implements Callback {
	public final static String TAG = "MSensorCurrentActivity";
	private SurfaceView mSurface;
	private SurfaceHolder mHolder;
	private Paint mBackPaint;
	private Paint mPaintAls;
	private Paint mPaintPs;
	private Paint mPaintLine;
	private SensorManager mSensorManager = null;
	private Sensor mMagSensor = null;
	private int mRate = SensorManager.SENSOR_DELAY_UI;
	public final Object LOCK = new Object();
	private final int MAX_NUM = 800;
	private final int DATA_MAG = 0;
	private final int DATA_ACC = 1;
	private int mSampleNum = 0;
	private int mSampleIdx = 0;
	private int BALL_RADIUS = 1;
	private float mLastValue[];
	private float mSampleData[];
	private float mSampleAcc[];
	private GameLoop mGameLoop;
	private boolean mShowLog = false;
	private boolean bReady = false;
	private boolean bDataReady = false;
	private boolean bAccReady = false;
	private TextView mRawOutput;
	private int mColorData = Color.RED;
	private int mColorAcc = Color.GREEN;
    private int mSampleMax = MAX_NUM;
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.sensor_mag_curve);

		mSurface = (SurfaceView) findViewById(R.id.mag_curve_surface);
		mHolder = mSurface.getHolder();
		mSurface.getHolder().addCallback(this);

		mBackPaint = new Paint();
		mBackPaint.setColor(Color.BLACK);
		mPaintAls = new Paint();
		mPaintAls.setColor(mColorData);
		mPaintPs = new Paint();
		mPaintPs.setColor(mColorAcc);
		mPaintLine = new Paint();

		mSampleData = new float[mSampleMax];
		mSampleAcc = new float[mSampleMax];
		mLastValue = new float[] { 0.0f, 0.0f, 0.0f };

		mRawOutput = (TextView) findViewById(R.id.label_mag_curve);
		// always portrait
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
		/*
		 * TYPE_MAGNETIC-FIELD : describe a magnetic filed sensor type
		 */
		mMagSensor = mSensorManager
				.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        /*
         * SENSOR_DELAY_UI : rate suitable for the user interface
         */
		if (mMagSensor != null) {
			mSensorManager.registerListener(mSensorEventListener, mMagSensor,
					mRate);
			bReady = true;
		} else {
			bReady = false;
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		if (bReady == true) {
			mGameLoop = new GameLoop();
			mGameLoop.start();
		} else {
			Toast.makeText(this, "Sensor Not found", Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// gameLoop = new GameLoop();
		// gameLoop.start();
	}

	private void draw() {
		// TODO thread safety - the SurfaceView could go away while we are
		// drawing
		Canvas c = null;
		try {
			// NOTE: in the LunarLander they don't have any synchronization
			// here,
			// so I guess this is OK. It will return null if the holder is not
			// ready
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
		float unit_ps = (height - base) / 4.0f;
		float unit_als = (height - base) / 1088.0f;
		int font_size = 12;
        
        if(width < mSampleMax) {
            mSampleMax = width - 10;
        }
		c.drawRect(0, 0, width, height, mBackPaint);
		mPaintLine.setColor(Color.WHITE);
		c.drawLine(0, height - base, width, height - base, mPaintLine);
		mPaintLine.setTextSize(font_size);
		mPaintLine.setColor(mColorData);
		c.drawText("DATA", 2, (height - base) + font_size, mPaintLine);
		mPaintLine.setColor(mColorAcc);
		c.drawText("ACC", 40, (height - base) + font_size, mPaintLine);

		// Log.d(TAG, "dimension: "+top+","+width+","+height+","+base);

		synchronized (LOCK) {
			mSampleData[mSampleIdx] = mLastValue[DATA_MAG];
			mSampleAcc[mSampleIdx] = mLastValue[DATA_ACC];
			mSampleIdx = (mSampleIdx + 1) % mSampleMax;
			if (bAccReady && bDataReady && (mSampleNum < mSampleMax))
				mSampleNum++;
		}

		int num = Math.min(width / offset, mSampleNum);
		int fst = mSampleIdx - num;
		int pos;

		if (fst < 0)
			fst += mSampleMax;

		for (idx = 0; idx < num; idx += offset) {
			pos = (fst + idx) % mSampleMax;
			ballX = idx;

			ballY[DATA_MAG] = (height - base - mSampleData[pos] * unit_als);
			ballY[DATA_ACC] = (height - base - mSampleAcc[pos] * unit_ps);
            if(ballY[DATA_MAG] < 0) {
                ballY[DATA_MAG] = 0;
                Elog.w(TAG, " Data value is larger than 1088");
            }
			c.drawCircle(ballX, ballY[DATA_MAG], BALL_RADIUS, mPaintAls);
			c.drawCircle(ballX, ballY[DATA_ACC], BALL_RADIUS, mPaintPs);
			if (idx != 0) {
				c.drawLine(prevX, prevY[DATA_MAG], ballX, ballY[DATA_MAG],
						mPaintAls);
				c.drawLine(prevX, prevY[DATA_ACC], ballX, ballY[DATA_ACC],
						mPaintPs);
			}
			prevX = ballX;
			prevY[DATA_MAG] = ballY[DATA_MAG];
			prevY[DATA_ACC] = ballY[DATA_ACC];
		}
	}

	@Override
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

		@Override
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
		@Override
		public void onSensorChanged(SensorEvent event) {
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_MAGNETIC_FIELD) {
				if (mShowLog)
					Log.d(TAG, "(" + type + ")" + event.values[0]);
				synchronized (LOCK) {
					mLastValue[DATA_ACC] = event.accuracy;
					bAccReady = true;

					float x = event.values[0];
					float y = event.values[1];
					float z = event.values[2];
					float result = (float) Math
							.sqrt((double) (x * x + y * y + z * z));
					mLastValue[DATA_MAG] = result;
					bDataReady = true;
				}
			}
			String notifyStr = "MSENSOR DATA IS GOOD!";
			if(event.accuracy != 3) {
			    notifyStr = "MSENSOR DATA IS NOT GOOD!";
			}
			mBuffer = "MSensor Data: " + mLastValue[DATA_MAG] + "\n" + "MSensor Accuracy: "
					+ mLastValue[DATA_ACC] + "\n" + notifyStr;
			mRawOutput.setText(mBuffer);
		}

		// from the android.hardware.SensorListener interface
		@Override
		public void onAccuracyChanged(Sensor sensor, int accuracy) {

		}

	};
}
