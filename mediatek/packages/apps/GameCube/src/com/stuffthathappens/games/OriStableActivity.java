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

class Rank {
	public int value;
	public int count;
	public void reset(){
		value = -1;
		count = 0;
	}
	Rank() {
		reset();
	}
}

public class OriStableActivity extends Activity implements OnClickListener{
	private final static String TAG = "Game";
	private SensorManager sensorMgr;
	private TextView mLabelCount, mAccuracy;
	private TextView[][] mLabels = new TextView[10][3];
	private int[][] mCounter = new int[600][3];
	private Rank[][] mRank = new Rank[10][3];
	private int mTotalCount = 0;
	private Button mBtnReset;
	private Sensor mSensor; 
	private long lastUpdate = -1;
	private boolean mShowLog = false;
	private Object mUpdateLock = new Object();
	private final static int IGNORE_COUNT=0;
	
	public void onClick(View arg0) {
		// TODO Auto-generated method stub
		resetAll();
	}	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.stable);
        mLabelCount = (TextView) findViewById(R.id.totalcount);
        mAccuracy = (TextView) findViewById(R.id.accuracy_label);
        mLabels[0][0] = (TextView) findViewById(R.id.table00);
        mLabels[0][1] = (TextView) findViewById(R.id.table01);
        mLabels[0][2] = (TextView) findViewById(R.id.table02);
        mLabels[1][0] = (TextView) findViewById(R.id.table10);
        mLabels[1][1] = (TextView) findViewById(R.id.table11);
        mLabels[1][2] = (TextView) findViewById(R.id.table12);
        mLabels[2][0] = (TextView) findViewById(R.id.table20);
        mLabels[2][1] = (TextView) findViewById(R.id.table21);
        mLabels[2][2] = (TextView) findViewById(R.id.table22);
        mLabels[3][0] = (TextView) findViewById(R.id.table30);
        mLabels[3][1] = (TextView) findViewById(R.id.table31);
        mLabels[3][2] = (TextView) findViewById(R.id.table32);
        mLabels[4][0] = (TextView) findViewById(R.id.table40);
        mLabels[4][1] = (TextView) findViewById(R.id.table41);
        mLabels[4][2] = (TextView) findViewById(R.id.table42);
        mLabels[5][0] = (TextView) findViewById(R.id.table50);
        mLabels[5][1] = (TextView) findViewById(R.id.table51);
        mLabels[5][2] = (TextView) findViewById(R.id.table52);
        mLabels[6][0] = (TextView) findViewById(R.id.table60);
        mLabels[6][1] = (TextView) findViewById(R.id.table61);
        mLabels[6][2] = (TextView) findViewById(R.id.table62);
        mLabels[7][0] = (TextView) findViewById(R.id.table70);
        mLabels[7][1] = (TextView) findViewById(R.id.table71);
        mLabels[7][2] = (TextView) findViewById(R.id.table72);
        mLabels[8][0] = (TextView) findViewById(R.id.table80);
        mLabels[8][1] = (TextView) findViewById(R.id.table81);
        mLabels[8][2] = (TextView) findViewById(R.id.table82);
        mLabels[9][0] = (TextView) findViewById(R.id.table90);
        mLabels[9][1] = (TextView) findViewById(R.id.table91);
        mLabels[9][2] = (TextView) findViewById(R.id.table92);
        
        mBtnReset = (Button) findViewById(R.id.btn_reset);
        mBtnReset.setOnClickListener(this);
        for (int x = 0; x < mRank.length; x++)
        	for (int y = 0; y < mRank[x].length; y++)
        		mRank[x][y] = new Rank();
		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
    }    

    protected void onDestroy() {
    	super.onDestroy();
		mLabelCount = null;
        for (int x = 0; x < mLabels.length; x++)
        	for (int y = 0; y < mLabels[x].length; y++)
        		mLabels[x][y] = null;		
		mLabels = null;
		mCounter = null;
		mBtnReset = null;
		mSensor = null; 	
		
        for (int x = 0; x < mRank.length; x++)
        	for (int y = 0; y < mRank[x].length; y++)
        		mRank[x][y] = null;		
		mRank = null;
    }
    
	@Override
	protected void onPause() {
		super.onPause();
		
		sensorMgr.unregisterListener(mSensorEventListener);
		sensorMgr = null;	
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		sensorMgr = (SensorManager) getSystemService(SENSOR_SERVICE);
		mSensor = sensorMgr.getDefaultSensor(Sensor.TYPE_ORIENTATION);
		boolean bSupported = sensorMgr.registerListener(mSensorEventListener, 
				mSensor, SENSOR_DELAY_UI);
		
		if (!bSupported) {
			// on accelerometer on this device
			sensorMgr.unregisterListener(mSensorEventListener);
		}
		resetAll();
		Log.d(TAG,"Supported : " + bSupported);		
	}

	private void resetAll() {
		synchronized (mUpdateLock) {
			mTotalCount = 0;
	        for (int x = 0; x < mCounter.length; x++)
	        	for (int y = 0; y < mCounter[x].length; y++)
	        		mCounter[x][y] = 0;
	        for (int x = 0; x < mRank.length; x++)
	        	for (int y = 0; y < mRank[x].length; y++)
	        		mRank[x][y].reset();        
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
			if (sensor.getType() == Sensor.TYPE_ORIENTATION) {
				switch (accuracy) {
				case SENSOR_STATUS_UNRELIABLE:
					mAccuracy.setText(R.string.accuracy_unreliable);
					break;
				case SENSOR_STATUS_ACCURACY_LOW:
					mAccuracy.setText(R.string.accuracy_low);
					break;
				case SENSOR_STATUS_ACCURACY_MEDIUM:
					mAccuracy.setText(R.string.accuracy_medium);
					break;
				case SENSOR_STATUS_ACCURACY_HIGH:
					mAccuracy.setText(R.string.accuracy_high);
					break;
				}
			}
		}
		
		private void removeRankByValue(int value, int type) {
			int pos, p;
			for (pos = 0; pos < mRank.length; pos++) {
				if (mRank[pos][type].value == value)
					break;
			}
			if (pos != mRank.length) {
				for (p = pos; p < mRank.length-1 ; p++) {					
					mRank[p][type].value = mRank[p+1][type].value;
					mRank[p][type].count = mRank[p+1][type].count;
				}
				mRank[mRank.length-1][type].reset();
			}				
		}
		
		private void addRankByValue(int value, int count, int type) {
			int pos, p;
			for (pos = 0; pos < mRank.length; pos++) {
				if ((mRank[pos][type].count < count))
					break;
			}
			if (pos != mRank.length) {
				for (p = mRank.length-2; p >= pos ; p--) {					
					mRank[p+1][type].value = mRank[p][type].value;
					mRank[p+1][type].count = mRank[p][type].count;
				}
				mRank[pos][type].value = value;
				mRank[pos][type].count = count;
			}			
		}
		// from the android.hardware.SensorEventListener interface
		private String mBuffer;
		
		public void onSensorChanged(SensorEvent event) {
			int OFFSET = 200;
			int type = event.sensor.getType();
			int x, y, z, pos, p, idx;
			StringBuffer buf;
			if (type == Sensor.TYPE_ORIENTATION) {
				x = Math.round(event.values[DATA_X])+OFFSET;	/*convert to positive value*/
				y = Math.round(event.values[DATA_Y])+OFFSET;	/*convert to positive value*/
				z = Math.round(event.values[DATA_Z])+OFFSET;	/*convert to positive value*/
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
				synchronized(mUpdateLock) {
					mTotalCount++;
					buf = new StringBuffer();
					buf.append(mTotalCount);
					mLabelCount.setText(buf);
					if (mTotalCount < IGNORE_COUNT) return;
					if (x < 0 || x >= mCounter.length) Log.e(TAG, "invalid x:"+x);
					if (y < 0 || y >= mCounter.length) Log.e(TAG, "invalid y:"+y);
					if (z < 0 || z >= mCounter.length) Log.e(TAG, "invalid z:"+z);
					mCounter[x][DATA_X]++;
					mCounter[y][DATA_Y]++;
					mCounter[z][DATA_Z]++;
					//Log.d(TAG, x +":"+mCounter[x][DATA_X]);
					removeRankByValue(x, DATA_X);
					addRankByValue(x, mCounter[x][DATA_X], DATA_X);	
					//Log.d(TAG, ""+mRank[0][DATA_X].value+","+mRank[1][DATA_X].value+","+mRank[2][DATA_X].value+","+mRank[3][DATA_X].value+","+mRank[4][DATA_X].value+",");
					removeRankByValue(y, DATA_Y);
					addRankByValue(y, mCounter[y][DATA_Y], DATA_Y);	
					removeRankByValue(z, DATA_Z);
					addRankByValue(z, mCounter[z][DATA_Z], DATA_Z);	
					
					for (idx = 0; idx < mLabels.length; idx++) {
						if (mRank[idx][DATA_X].value != -1) {
							buf = new StringBuffer();						
							buf.append(mRank[idx][DATA_X].value-OFFSET).append("(").append(mRank[idx][DATA_X].count).append(")");						
							mLabels[idx][DATA_X].setText(buf);
						} else {
							mLabels[idx][DATA_X].setText("");
						}
						if (mRank[idx][DATA_Y].value != -1) {
							buf = new StringBuffer();						
							buf.append(mRank[idx][DATA_Y].value-OFFSET).append("(").append(mRank[idx][DATA_Y].count).append(")");						
							mLabels[idx][DATA_Y].setText(buf);
						} else {
							mLabels[idx][DATA_Y].setText("");
						}

						if (mRank[idx][DATA_Z].value != -1) {
							buf = new StringBuffer();						
							buf.append(mRank[idx][DATA_Z].value-OFFSET).append("(").append(mRank[idx][DATA_Z].count).append(")");						
							mLabels[idx][DATA_Z].setText(buf);
						} else {
							mLabels[idx][DATA_Z].setText("");
						}
						
					}
				}
			}
		}
	};


}