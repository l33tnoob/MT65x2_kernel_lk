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
import android.view.View;
import android.widget.TextView;
import android.widget.Button;

public class PSensorCollection extends Activity {
	public static final String TAG = "PSensorCollectionLog";
    private static final int COUNT = 22;
    private static final float data[] = {0.5f, 1f, 1.5f, 2f, 2.5f, 3f, 3.5f, 4f, 4.5f, 5f,
                        5.5f, 6f, 6.5f, 7f, 7.5f, 8f, 8.5f, 9f, 9.5f, 10f, 10.5f, 11f};//new float[COUNT]; 
                        
    private int dis[] = new int[COUNT];             
	private CollectionCurveView mCurveView = null;
	private TextView mText[] = new TextView[COUNT];
	private TextView mDataText[] = new TextView[COUNT];
	
	private Button mClearButton = null;
	private Button mGetButton = null;
	private int mCurrentIndex = -1;

	private SensorManager mSensorManager = null;
	private Sensor mSensor = null;
	private SensorEventListener mSensorEventListener = new SensorEventListener() {
		@Override
		public void onSensorChanged(SensorEvent event) {
			// Do nothing
		}

		@Override
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
			// Do nothing
		}
	};
   
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.sensor_ps_collection);
        mCurveView = (CollectionCurveView) findViewById(R.id.curve_view);
        mClearButton = (Button) findViewById(R.id.btn_clear);
        mGetButton = (Button) findViewById(R.id.btn_get);
        mClearButton.setOnClickListener(mClickListener);
        mGetButton.setOnClickListener(mClickListener);
        
        int tx_id[] = {R.id.tx_1, R.id.tx_2, R.id.tx_3, R.id.tx_4, R.id.tx_5, R.id.tx_6, R.id.tx_7, R.id.tx_8, R.id.tx_9,
                        R.id.tx_10, R.id.tx_11, R.id.tx_12, R.id.tx_13, R.id.tx_14, R.id.tx_15, R.id.tx_16, R.id.tx_17,
                    R.id.tx_18, R.id.tx_19, R.id.tx_20, R.id.tx_21, R.id.tx_22};
        int data_tx_id[] = {R.id.data_tx_1, R.id.data_tx_2, R.id.data_tx_3, R.id.data_tx_4, R.id.data_tx_5, R.id.data_tx_6,
                            R.id.data_tx_7, R.id.data_tx_8, R.id.data_tx_9, R.id.data_tx_10, R.id.data_tx_11, R.id.data_tx_12,
                            R.id.data_tx_13, R.id.data_tx_14, R.id.data_tx_15, R.id.data_tx_16, R.id.data_tx_17, R.id.data_tx_18,
                        R.id.data_tx_19, R.id.data_tx_20, R.id.data_tx_21, R.id.data_tx_22};
        for(int i = 0; i < COUNT; i++) {
            mText[i] = (TextView) findViewById(tx_id[i]);
            mDataText[i] = (TextView) findViewById(data_tx_id[i]);
        }

        for(int i = 0; i < COUNT; i++) {
            mText[i].setText(Float.toString(data[i])+" ");
        }
        
        for(int i = 0; i < COUNT; i++) {
            mDataText[i].setText("    ");
        }
		mCurveView.setDistance(null);
		mClearButton.setEnabled(false);
	}
    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(TAG, "onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mClearButton)) {
                clearData();
                if(mCurrentIndex == -1){
                    mClearButton.setEnabled(false);
                }
            } else if (arg0.equals(mGetButton)) {
                getData();
                mClearButton.setEnabled(true);
            } 
        }
    };
    
    private void clearData() {
        Elog.v(TAG, "Clear psensor data: ");
        if(mCurrentIndex == -1) {
            return;
        }

        mDataText[mCurrentIndex].setText(" ");
        mCurrentIndex--;
        
        mCurveView.setDistance(null);
        mCurveView.postInvalidate();
    }
    
    private void getData() {
        Elog.v(TAG, "Get psensor data: ");
        
        if(mCurrentIndex == COUNT - 1) {
            return;
        }
        mCurrentIndex ++;
        dis[mCurrentIndex] = EmSensor.getPsensorData();
        Elog.v(TAG, "Get "+mCurrentIndex+" data :"+Integer.toString(dis[mCurrentIndex]));
   
        mDataText[mCurrentIndex].setText(Integer.toString(dis[mCurrentIndex])+" ");

        if(mCurrentIndex == COUNT - 1) { // Update curve when data is full.
            mCurveView.setDistance(dis);
            mCurveView.postInvalidate();
        }
    }
	@Override
	protected void onPause() {
		mSensorManager.unregisterListener(mSensorEventListener);
		mSensorManager = null;
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
		mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
		mSensorManager.registerListener(mSensorEventListener, mSensor, SensorManager.SENSOR_DELAY_UI);
	}

}
