/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.sensor;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.Dialog;
import android.content.Intent;
import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.Locale;

public class PSensorCalibration extends Activity implements OnClickListener {
    private static final String TAG = "PSensorCalibration";
    private static final int UPDATE_INTERVAL = 50;

    private static final int MSG_CALCULATE_MIN_VALUE = 0;
    private static final int MSG_CALCULATE_MAX_VALUE = 1;
    private static final int MSG_DO_CALIBRARION = 2;
    private static final int MSG_CLEAR_CALIBRARION = 3;
    private static final int MSG_GET_DATA = 4;
    private static final int MSG_UPDATE_DATA = 5;

    private static final int MSG_CALCULATE_SUCCESS = 6;
    private static final int MSG_CALCULATE_FAILURE = 7;
    private static final int MSG_GET_DATA_SUCCESS = 8;
    private static final int MSG_GET_DATA_FAILURE = 9;
    private static final int MSG_CALIBRARION_SUCCESS = 10;
    private static final int MSG_CALIBRARION_FAILURE = 11;
    private static final int MSG_CLEAR_SUCCESS = 12;
    private static final int MSG_CLEAR_FAILURE = 13;

    private Button mGetMin;
    private Button mGetMax;
    private Button mDoCalibration;
    private Button mClearCalibration;
    private TextView mCurrentData;
    private TextView mMinValue;
    private TextView mMaxValue;
    private TextView mResult;
    private Toast mToast;

    private int mData = 0;
    private int mHigh = 0;
    private int mLow = 0;
    private int mMin = 0;
    private int mMax = 0;

    private final HandlerThread mHandlerThread = new HandlerThread("async_handler");
    private Handler mHandler;
    private Handler mUiHandler;

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
        setContentView(R.layout.sensor_ps_calibration);

        mGetMin = (Button) findViewById(R.id.button_sensor_ps_calibration_get_min);
        mGetMax = (Button) findViewById(R.id.button_sensor_ps_calibration_get_max);
        mDoCalibration = (Button) findViewById(R.id.button_sensor_ps_calibration_do_cali);
        mClearCalibration = (Button) findViewById(R.id.button_sensor_ps_calibration_clear_cali);

        mGetMin.setOnClickListener(this);
        mGetMax.setOnClickListener(this);
        mDoCalibration.setOnClickListener(this);
        mClearCalibration.setOnClickListener(this);

        mCurrentData = (TextView) findViewById(R.id.text_sensor_ps_calibration_current_data);
        mMinValue = (TextView) findViewById(R.id.text_sensor_ps_calibration_min_value);
        mMaxValue = (TextView) findViewById(R.id.text_sensor_ps_calibration_max_value);
        mResult = (TextView) findViewById(R.id.text_sensor_ps_calibration_result);

        mUiHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_CALCULATE_SUCCESS:
                    Xlog.d(TAG, "set success");
                    enableButtons(true);
                    mMinValue.setText(String.valueOf(mMin));
                    mMaxValue.setText(String.valueOf(mMax));
                    showToast("Calculate succeed");
                    break;
                case MSG_CALCULATE_FAILURE:
                    Xlog.d(TAG, "set fail");
                    enableButtons(true);
                    showToast("Calculate failed");
                    break;
                case MSG_GET_DATA_SUCCESS:
                    enableButtons(true);
                    mCurrentData.setText(String.format(Locale.ENGLISH,
                            getString(R.string.sensor_ps_calibration_current_data_format),
                            mData, mHigh, mLow));
                    mMinValue.setText(String.valueOf(mMin));
                    mMaxValue.setText(String.valueOf(mMax));
                    break;
                case MSG_GET_DATA_FAILURE:
                    Xlog.d(TAG, "get fail");
                    enableButtons(true);
                    showToast("Get PS data failed");
                    break;
                case MSG_CALIBRARION_FAILURE:
                    Xlog.d(TAG, "cali fail");
                    enableButtons(true);
                    showToast("Calibration failed");
                    mResult.setText("FAIL");
                    break;
                case MSG_CALIBRARION_SUCCESS:
                    Xlog.d(TAG, "cali success");
                    enableButtons(true);
                    showToast("Calibration succeed");
                    mResult.setText("PASS");
                    break;
                case MSG_CLEAR_FAILURE:
                    Xlog.d(TAG, "clear fail");
                    enableButtons(true);
                    showToast("Clear failed");
                    mResult.setText("");
                    break;
                case MSG_CLEAR_SUCCESS:
                    Xlog.d(TAG, "clear success");
                    enableButtons(true);
                    showToast("Clear succeed");
                    mResult.setText("");
                    break;
                default:
                }
            }
        };

        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {
            public void handleMessage(Message msg) {
                calibration(msg.what);
            }
        };
    }

    @Override
    protected void onResume() {
        super.onResume();
        Xlog.d(TAG, "onResume()");
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        mSensorManager.registerListener(mSensorEventListener, mSensor, SensorManager.SENSOR_DELAY_UI);

        mHandler.sendEmptyMessage(MSG_GET_DATA);
    }

    @Override
    protected void onPause() {
        Xlog.d(TAG, "onPause()");
        mSensorManager.unregisterListener(mSensorEventListener);
        mSensorManager = null;

        mHandler.removeMessages(MSG_UPDATE_DATA);
        super.onPause();
    }

    @Override
    public void onClick(View arg0) {
        if (arg0.getId() == mGetMin.getId()) {
            Xlog.d(TAG, "get min");
            mHandler.sendEmptyMessage(MSG_CALCULATE_MIN_VALUE);
        } else if (arg0.getId() == mGetMax.getId()) {
            Xlog.d(TAG, "get max");
            mHandler.sendEmptyMessage(MSG_CALCULATE_MAX_VALUE);
        } else if (arg0.getId() == mDoCalibration.getId()) {
            Xlog.d(TAG, "do calibration");
            mHandler.sendEmptyMessage(MSG_DO_CALIBRARION);
        } else if (arg0.getId() == mClearCalibration.getId()) {
            Xlog.d(TAG, "clear calibration");
            mHandler.sendEmptyMessage(MSG_CLEAR_CALIBRARION);
        }
        enableButtons(false);
    }

    private void getCurrentData() {
        Xlog.d(TAG, "getCurrentData()");
        mData = EmSensor.getPsensorData();
        Xlog.d(TAG, String.format("getPsensorData(), ret %d", mData));

        mMin = EmSensor.getPsensorMinValue();
        Xlog.d(TAG, String.format("getPsensorMinValue(), ret %d", mMin));

        mMax = EmSensor.getPsensorMaxValue();
        Xlog.d(TAG, String.format("getPsensorMaxValue(), ret %d", mMax));

        int[] result = new int[2];
        EmSensor.getPsensorThreshold(result);
        Xlog.d(TAG, String.format("getPsensorThreshold(), ret %d, %d", result[0], result[1]));
        mHigh = result[0];
        mLow = result[1];

        mUiHandler.sendEmptyMessage(MSG_GET_DATA_SUCCESS);
    }

    private void calibration(int what) {
        int result = 0;
        if (what != MSG_UPDATE_DATA) {
            Xlog.d(TAG, String.format("calibration(), operation %d", what));
        }

        switch (what) {
        case MSG_GET_DATA:
            getCurrentData();
            mHandler.sendEmptyMessageDelayed(MSG_UPDATE_DATA, UPDATE_INTERVAL);
            break;
        case MSG_UPDATE_DATA:
            mData = EmSensor.getPsensorData();
            mUiHandler.sendEmptyMessage(MSG_GET_DATA_SUCCESS);
            mHandler.sendEmptyMessageDelayed(MSG_UPDATE_DATA, UPDATE_INTERVAL);
            break;
        case MSG_DO_CALIBRARION:
            result = EmSensor.doPsensorCalibration(mMin, mMax);
            if (result == EmSensor.RET_SUCCESS) {
                getCurrentData();
                mUiHandler.sendEmptyMessage(MSG_CALIBRARION_SUCCESS);
            } else {
                mUiHandler.sendEmptyMessage(MSG_CALIBRARION_FAILURE);
            }
            break;
        case MSG_CLEAR_CALIBRARION:
            result = EmSensor.clearPsensorCalibration();
            if (result == EmSensor.RET_SUCCESS) {
                getCurrentData();
                mUiHandler.sendEmptyMessage(MSG_CLEAR_SUCCESS);
            } else {
                mUiHandler.sendEmptyMessage(MSG_CLEAR_FAILURE);
            }
            break;
        case MSG_CALCULATE_MIN_VALUE:
            result = EmSensor.calculatePsensorMinValue();
            if (result == EmSensor.RET_SUCCESS) {
                mMin = EmSensor.getPsensorMinValue();
                Xlog.d(TAG, String.format("getPsensorMinValue(), ret %d", mMin));
                mUiHandler.sendEmptyMessage(MSG_CALCULATE_SUCCESS);
            } else {
                mUiHandler.sendEmptyMessage(MSG_CALCULATE_FAILURE);
            }
            break;
        case MSG_CALCULATE_MAX_VALUE:
            result = EmSensor.calculatePsensorMaxValue();
            if (result == EmSensor.RET_SUCCESS) {
                mMax = EmSensor.getPsensorMaxValue();
                Xlog.d(TAG, String.format("getPsensorMaxValue(), ret %d", mMax));
                mUiHandler.sendEmptyMessage(MSG_CALCULATE_SUCCESS);
            } else {
                mUiHandler.sendEmptyMessage(MSG_CALCULATE_FAILURE);
            }
            break;
        default:
            break;
        }
        if (what != MSG_UPDATE_DATA) {
            Xlog.d(TAG, String.format("calibration(), ret %d", result));
        }
    }

    private void enableButtons(boolean enable) {
        mGetMin.setClickable(enable);
        mGetMax.setClickable(enable);
        mDoCalibration.setClickable(enable);
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

