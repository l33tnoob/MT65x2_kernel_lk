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
import static android.hardware.SensorManager.SENSOR_DELAY_GAME;
import static android.hardware.SensorManager.SENSOR_DELAY_NORMAL;
import static android.hardware.SensorManager.SENSOR_DELAY_FASTEST;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_HIGH;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_LOW;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM;
import static android.hardware.SensorManager.SENSOR_STATUS_UNRELIABLE;

import java.util.List;

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
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;

class SensorShow {
	private long mLastTime;
	private TextView mAccuracy;
	private TextView mDelay;
	private TextView[] mValues;
	private View mSep;
	private TextView mTitle;
	SensorShow() {
		mAccuracy = null;
		mValues = new TextView[3];
		mDelay = null;
		mLastTime = 0;
	}
	public void initResource(TextView accuracy, TextView delay, TextView value0, TextView value1, TextView value2) {
		mAccuracy = accuracy;
		mDelay = delay;
		mValues[0] = value0;
		mValues[1] = value1;
		mValues[2] = value2;
		if (mAccuracy != null) mAccuracy.setVisibility(View.INVISIBLE);
		if (mDelay != null) mDelay.setVisibility(View.INVISIBLE);
		if (mValues[0] != null) mValues[0].setVisibility(View.INVISIBLE);
		if (mValues[1] != null) mValues[1].setVisibility(View.INVISIBLE);
		if (mValues[2] != null) mValues[2].setVisibility(View.INVISIBLE);
	}
	
	public void setAccuracy(int accuracy) {
		boolean valid = true;
		if (this.mAccuracy == null)
			return;
		switch (accuracy){
		case SENSOR_STATUS_UNRELIABLE:
			this.mAccuracy.setText(R.string.accuracy_unreliable);
			break;
		case SENSOR_STATUS_ACCURACY_LOW:
			this.mAccuracy.setText(R.string.accuracy_low);
			break;
		case SENSOR_STATUS_ACCURACY_MEDIUM:
			this.mAccuracy.setText(R.string.accuracy_medium);
			break;
		case SENSOR_STATUS_ACCURACY_HIGH:
			this.mAccuracy.setText(R.string.accuracy_high);
			break;
		default:
			valid = false;
			break;
		}
		if (valid) {
			if (mAccuracy != null) mAccuracy.setVisibility(View.VISIBLE);
			if (mDelay != null) mDelay.setVisibility(View.VISIBLE);
			if (mValues[0] != null) mValues[0].setVisibility(View.VISIBLE);
			if (mValues[1] != null) mValues[1].setVisibility(View.VISIBLE);
			if (mValues[2] != null) mValues[2].setVisibility(View.VISIBLE);			
		}
	}
	
	public void setValues(float values[], int num) {
		for (int idx = 0; (idx < num) && (this.mValues[idx] != null); idx++)		
			this.mValues[idx].setText(Float.toString(values[idx]));
	}
	
	public void setDelay(long delay) {
		if (this.mDelay == null)
			return;
		this.mDelay.setText(Long.toString(delay));
	}
	
	public long getLastTime() {
		return mLastTime;
	}
	
	public void setLastTime(long time) {
		mLastTime = time;
	}

}
public class SensorRaw extends Activity implements OnClickListener{
	private final static String TAG = "Game";
	private SensorManager mSensorManager; 	
	private boolean mShowLog = false;
	private boolean mUpdateUI = true;
	private List<Sensor> mSensorList;
	private SensorShow[] mSensorShow;
	private final static int MAX_SENSOR_NUM = 16;
	private Button btn_fast, btn_game, btn_ui, btn_norm;
	private int mDelay = SENSOR_DELAY_GAME;
 
	private int registerAll(int delay) {
		int num = 0;
		for (Sensor sensor: mSensorList) {
			if (sensor.getType() < 1)
				continue;
			if((mSensorEventListener != null) && (mSensorManager != null))
			{
				mSensorManager.registerListener(mSensorEventListener, sensor, delay);
				num++;
			}
		}		
		return num;
	}
	private void deregisterAll() {
		if (mSensorManager != null)
			mSensorManager.unregisterListener(mSensorEventListener);		
	}
	
	public void onClick(View v) {
		int delay = SENSOR_DELAY_GAME;
		if (v == btn_fast) {
			delay = SENSOR_DELAY_FASTEST;
		} else if (v == btn_game) {
			delay = SENSOR_DELAY_GAME;
		} else if (v == btn_ui) {
			delay = SENSOR_DELAY_UI;
		} else if (v == btn_norm) {
			delay = SENSOR_DELAY_NORMAL;
		}
		if (delay != mDelay) {
			deregisterAll();
			registerAll(delay);
			mDelay = delay;
		}
	}
		
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.allraw);
        mSensorShow = new SensorShow[MAX_SENSOR_NUM];
        for (int idx = 0; idx < mSensorShow.length; idx++)
        	mSensorShow[idx] = new SensorShow();
        mSensorShow[Sensor.TYPE_ACCELEROMETER-1].initResource((TextView) findViewById(R.id.acc_accuracy),
        													  (TextView) findViewById(R.id.acc_delay),
        													  (TextView) findViewById(R.id.acc_x),
        													  (TextView) findViewById(R.id.acc_y),
        													  (TextView) findViewById(R.id.acc_z));
        mSensorShow[Sensor.TYPE_MAGNETIC_FIELD-1].initResource((TextView) findViewById(R.id.mag_accuracy),
															  (TextView) findViewById(R.id.mag_delay),
															  (TextView) findViewById(R.id.mag_x),
															  (TextView) findViewById(R.id.mag_y),
															  (TextView) findViewById(R.id.mag_z));
        mSensorShow[Sensor.TYPE_ORIENTATION-1].initResource((TextView) findViewById(R.id.ori_accuracy),
															  (TextView) findViewById(R.id.ori_delay),
															  (TextView) findViewById(R.id.ori_azimuth),
															  (TextView) findViewById(R.id.ori_pitch),
															  (TextView) findViewById(R.id.ori_roll));
        mSensorShow[Sensor.TYPE_PROXIMITY-1].initResource((TextView) findViewById(R.id.ps_accuracy),
															  (TextView) findViewById(R.id.ps_delay),
															  (TextView) findViewById(R.id.ps_dis),
															  null,
															  null);
        mSensorShow[Sensor.TYPE_LIGHT-1].initResource((TextView) findViewById(R.id.als_accuracy),
															  (TextView) findViewById(R.id.als_delay),
															  (TextView) findViewById(R.id.als_lux),
															  null,
															  null);
        
        btn_fast = (Button) findViewById(R.id.btn_fast);
        btn_fast.setOnClickListener(this);
        btn_game = (Button) findViewById(R.id.btn_game);
        btn_game.setOnClickListener(this);
        btn_ui = (Button) findViewById(R.id.btn_ui);
        btn_ui.setOnClickListener(this);
        btn_norm = (Button) findViewById(R.id.btn_norm);
        btn_norm.setOnClickListener(this);

		//always portrait
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
    }
    
    @Override
    protected void onDestroy() {
    	super.onDestroy();
    	for (int idx = 0; idx < mSensorShow.length; idx++)
    		mSensorShow[idx] = null;
    	mSensorShow = null;
    }

	@Override
	protected void onPause() {
		super.onPause();
		
		deregisterAll();
		mSensorManager = null;
	}

	@Override
	protected void onResume() {		
		super.onResume();
		
		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);

		/*the size of mSensorList will be one if no sensor available, the sensor type is 0*/
		mSensorList = mSensorManager.getSensorList(Sensor.TYPE_ALL);
		
		int num = registerAll(mDelay);
		if (num == 0)
			Toast.makeText(this, "No available Sensors", Toast.LENGTH_SHORT).show();			
	}


	@Override 
	public boolean onCreateOptionsMenu(Menu menu) { 
		boolean supRetVal = super.onCreateOptionsMenu(menu);
		if (mShowLog)
			menu.add(0, 0, 0, getString(R.string.hide_log));
		else
			menu.add(0, 0, 0, getString(R.string.show_log));
		if (mUpdateUI)
			menu.add(0, 1, 0, getString(R.string.update_off));
		else
			menu.add(0, 1, 0, getString(R.string.update_on));
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
		case 1:
			if (mUpdateUI) {
				mUpdateUI = false;
				item.setTitle(R.string.update_on);
			} else {
				mUpdateUI = true;
				item.setTitle(R.string.update_off);
			}		
			return true;			
		}
		return false; 
	}	

	public final SensorEventListener mSensorEventListener = new SensorEventListener() {
		// from the android.hardware.SensorListener interface
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
			int idx = sensor.getType()-1;
			if(mSensorShow != null && mSensorShow[idx]!= null)
			{
			mSensorShow[idx].setAccuracy(accuracy);
		}
		}
		
		// from the android.hardware.SensorEventListener interface
		private String mBuffer;
		public void onSensorChanged(SensorEvent event) {
			long diff, curTime = System.currentTimeMillis();
			int type = event.sensor.getType();
			int idx = type-1;
			SensorShow show = mSensorShow[idx];
			if (mUpdateUI == false) /*by-pass UI update*/
				return;			
			switch (type) {
			case Sensor.TYPE_ACCELEROMETER:
			case Sensor.TYPE_MAGNETIC_FIELD:
			case Sensor.TYPE_ORIENTATION:
				diff = curTime - show.getLastTime();
				show.setValues(event.values, 3);
				show.setLastTime(curTime);
				show.setDelay(diff);
				break;
			case Sensor.TYPE_LIGHT:
			case Sensor.TYPE_PROXIMITY:	
				diff = curTime - show.getLastTime();
				show.setValues(event.values, 1);
				show.setLastTime(curTime);
				show.setDelay(diff);
				break;
			default:
				break;
			}
		}
	};
}