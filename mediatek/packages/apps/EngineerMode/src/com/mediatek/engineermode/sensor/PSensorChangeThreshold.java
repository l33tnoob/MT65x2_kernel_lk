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
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.Locale;

public class PSensorChangeThreshold extends Activity implements OnClickListener {
    private static final String TAG = "PSensorChangeThreshold";
    private static final int MSG_CHANGE_THRESHOLD = 0;
    private static final int MSG_SET_SUCCESS = 1;
    private static final int MSG_SET_FAILURE = 2;
    private static final int MSG_INVALID_NUMBER = 3;
    private static final int THRESHOLD_MIN = 0;
    private static final int THRESHOLD_MAX = 65535;

    private Button mBtnSet;
    private EditText mEtHigh;
    private EditText mEtLow;
    private Toast mToast;

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
        setContentView(R.layout.sensor_ps_change_threshold);

        mBtnSet = (Button) findViewById(R.id.button_sensor_ps_change_threshold);
        mBtnSet.setOnClickListener(this);

        mEtHigh = (EditText) findViewById(R.id.edit_sensor_ps_threshold_high);
        mEtLow = (EditText) findViewById(R.id.edit_sensor_ps_threshold_low);

        mUiHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_SET_SUCCESS:
                    Xlog.d(TAG, "set success");
                    mBtnSet.setClickable(true);
                    showToast("Set threshold succeed");
                    break;
                case MSG_SET_FAILURE:
                    Xlog.d(TAG, "set fail");
                    mBtnSet.setClickable(true);
                    showToast("Set threshold failed");
                    break;
                case MSG_INVALID_NUMBER:
                    Xlog.d(TAG, "set fail");
                    mBtnSet.setClickable(true);
                    showToast("Invalid value");
                    break;
                default:
                }
            }
        };

        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {
            public void handleMessage(Message msg) {
                if (MSG_CHANGE_THRESHOLD == msg.what) {
                    Xlog.d(TAG, String.format("MSG_CHANGE_THRESHOLD"));
                    int high = 0, low = 0;
                    try {
                        high = Integer.parseInt(mEtHigh.getText().toString());
                        low = Integer.parseInt(mEtLow.getText().toString());
                        if (high < THRESHOLD_MIN || high > THRESHOLD_MAX || low < THRESHOLD_MIN || low > THRESHOLD_MAX) {
                            throw new NumberFormatException("");
                        }
                    } catch (NumberFormatException e) {
                        mUiHandler.sendEmptyMessage(MSG_INVALID_NUMBER);
                        return;
                    }

                    if (EmSensor.RET_SUCCESS == EmSensor.setPsensorThreshold(high, low)) {
                        mUiHandler.sendEmptyMessage(MSG_SET_SUCCESS);
                    } else {
                        mUiHandler.sendEmptyMessage(MSG_SET_FAILURE);
                    }
                }
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
    }

    @Override
    protected void onPause() {
        Xlog.d(TAG, "onPause()");
        mSensorManager.unregisterListener(mSensorEventListener);
        mSensorManager = null;
        super.onPause();
    }

    @Override
    public void onClick(View arg0) {
        if (arg0.getId() == mBtnSet.getId()) {
            Xlog.d(TAG, "change threshold");
            mHandler.sendEmptyMessage(MSG_CHANGE_THRESHOLD);
        }
        mBtnSet.setClickable(false);
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

