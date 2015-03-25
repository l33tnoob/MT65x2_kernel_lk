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

package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

/**
 * 
 * <p>
 * Description: Show current battery's information and record battery log.
 * 
 */
public class BatteryLog extends Activity implements OnClickListener {

    private int mLogRecordInterval;
    private boolean mIsRecording;

    private File mLogFile;
    private File mBatteryLogFile;

    private TextView mStatus;
    private TextView mLevel;
    private TextView mScale;
    private TextView mHealth;
    private TextView mVoltage;
    private TextView mTemperature;
    private TextView mTechnology;
    private TextView mUptime;
    private EditText mIntervalEdit;
    private Button mLogRecord;

    private static final int EVENT_TICK = 1;
    private static final int EVENT_LOG_RECORD = 2;

    private static final int DELAY_TIME = 1000;
    private static final int MAX_NUMBER = 100;
    private static final int MAX_NUMBER_LENGTH = 3;
    private static final int FORMART_UPTIME = 1000;
    private static final int DEFAULT_INTERVAL = 10000;

    private static final int MAGIC_NUMBER_TEN = 10;

    // battery attr
    private static final String PLUGGED = "plugged";
    private static final String LEVEL = "level";
    private static final String SCALE = "scale";
    private static final String VOLTAGE = "voltage";
    private static final String TEMPERATURE = "temperature";
    private static final String STATUS = "status";
    private static final String HEALTH = "health";

    private static final String TAG = "EM-BatteryLog";

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == EVENT_TICK) {
                updateBatteryStats();
                sendEmptyMessageDelayed(EVENT_TICK, DELAY_TIME);
            }
        }

        private void updateBatteryStats() {
            final long uptime = SystemClock.elapsedRealtime();
            mUptime.setText(DateUtils.formatElapsedTime(uptime / FORMART_UPTIME));
        }
    };

    /**
     * Format a number of tenths-units as a decimal string without using a conversion to float. E.g. 347 -> "34.7"
     * 
     * @return float
     */
    private String tenthsToFixedString(int x) {
        int tens = x / MAGIC_NUMBER_TEN;
        return new String("" + tens + "." + (x - MAGIC_NUMBER_TEN * tens));
    }

    /**
     *Listens for intent broadcasts
     */
    private IntentFilter mIntentFilter;
    private IntentFilter mIntentFilterSDCard;

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context arg0, Intent arg1) {
            String action = arg1.getAction();
            if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                int plugType = arg1.getIntExtra(PLUGGED, 0);

                mLevel.setText("" + arg1.getIntExtra(LEVEL, 0));
                mScale.setText("" + arg1.getIntExtra(SCALE, 0));
                mVoltage.setText("" + arg1.getIntExtra(VOLTAGE, 0) + " " + getString(R.string.battery_info_voltage_units));
                mTemperature.setText("" + tenthsToFixedString(arg1.getIntExtra(TEMPERATURE, 0))
                        + getString(R.string.battery_info_temperature_units));
                mTechnology.setText("" + arg1.getStringExtra("technology"));

                int status = arg1.getIntExtra(STATUS, BatteryManager.BATTERY_STATUS_UNKNOWN);
                String statusString = null;
                if (status == BatteryManager.BATTERY_STATUS_CHARGING) {
                    statusString = getString(R.string.battery_info_status_charging);
                    if (plugType > 0) {
                        statusString = statusString
                                + " "
                                + getString((plugType == BatteryManager.BATTERY_PLUGGED_AC) 
                                        ? R.string.battery_info_status_charging_ac
                                        : R.string.battery_info_status_charging_usb);
                    }
                } else if (status == BatteryManager.BATTERY_STATUS_DISCHARGING) {
                    statusString = getString(R.string.battery_info_status_discharging);
                } else if (status == BatteryManager.BATTERY_STATUS_NOT_CHARGING) {
                    statusString = getString(R.string.battery_info_status_not_charging);
                } else if (status == BatteryManager.BATTERY_STATUS_FULL) {
                    statusString = getString(R.string.battery_info_status_full);
                } else {
                    statusString = getString(R.string.battery_info_status_unknown);
                }
                mStatus.setText(statusString);

                int health = arg1.getIntExtra(HEALTH, BatteryManager.BATTERY_HEALTH_UNKNOWN);
                String healthString;
                if (health == BatteryManager.BATTERY_HEALTH_GOOD) {
                    healthString = getString(R.string.battery_info_health_good);
                } else if (health == BatteryManager.BATTERY_HEALTH_OVERHEAT) {
                    healthString = getString(R.string.battery_info_health_overheat);
                } else if (health == BatteryManager.BATTERY_HEALTH_DEAD) {
                    healthString = getString(R.string.battery_info_health_dead);
                } else if (health == BatteryManager.BATTERY_HEALTH_OVER_VOLTAGE) {
                    healthString = getString(R.string.battery_info_health_over_voltage);
                } else if (health == BatteryManager.BATTERY_HEALTH_UNSPECIFIED_FAILURE) {
                    healthString = getString(R.string.battery_info_health_unspecified_failure);
                } else {
                    healthString = getString(R.string.battery_info_health_unknown);
                }
                mHealth.setText(healthString);
            }
        }
    };
    private BroadcastReceiver mIntentReceiverSDCard = new BroadcastReceiver() {

        @Override
        public void onReceive(Context arg0, Intent arg1) {
            String action = arg1.getAction();
            if (action.equals(Intent.ACTION_MEDIA_BAD_REMOVAL) || action.equals(Intent.ACTION_MEDIA_REMOVED)
                    || action.equals(Intent.ACTION_MEDIA_EJECT)) {
                if (!mIsRecording) {
                    return;
                }
                mIsRecording = false;
                mLogHandler.removeMessages(EVENT_LOG_RECORD);
                mLogRecord.setText(R.string.battery_info_Log_Start);
                AlertDialog.Builder builder = new AlertDialog.Builder(BatteryLog.this);
                builder.setTitle("SD card error");
                builder.setMessage("SD card has been removed.");
                builder.setPositiveButton("OK", null);
                builder.create().show();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.battery_info);

        mLogRecord = (Button) findViewById(R.id.Log_Record);
        if (mLogRecord == null) {
            Log.e(TAG, "clocwork worked...");
            // not return and let exception happened.
        }
        mLogRecord.setOnClickListener(this);

        // create the IntentFilter that will be used to listen
        // to battery status broadcasts
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Intent.ACTION_BATTERY_CHANGED);

        mIntentFilterSDCard = new IntentFilter();
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_REMOVED);
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_EJECT);
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        mIntentFilterSDCard.addDataScheme("file");

        // check whether the sdcard exists, if yes, set up batterylog directory,
        // and if not, notify user to plug in it
        mLogRecordInterval = DEFAULT_INTERVAL;
        File sdcard = null;
        if (!Environment.getExternalStorageState().equals(Environment.MEDIA_REMOVED)) {
            sdcard = Environment.getExternalStorageDirectory();
            mBatteryLogFile = new File(sdcard.getParent() + "/" + sdcard.getName() + "/batterylog/");
            Log.e(TAG, sdcard.getParent() + "/" + sdcard.getName() + "/batterylog/");
            if (!mBatteryLogFile.isDirectory()) {
                mBatteryLogFile.mkdirs();
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        mStatus = (TextView) findViewById(R.id.status);
        mLevel = (TextView) findViewById(R.id.level);
        mScale = (TextView) findViewById(R.id.scale);
        mHealth = (TextView) findViewById(R.id.health);
        mTechnology = (TextView) findViewById(R.id.technology);
        mVoltage = (TextView) findViewById(R.id.voltage);
        mTemperature = (TextView) findViewById(R.id.temperature);
        mUptime = (TextView) findViewById(R.id.uptime);
        mIntervalEdit = (EditText) findViewById(R.id.Log_Record_Interval);
        if (mIntervalEdit == null) {
            Log.e(TAG, "clocwork worked...");
            // not return and let exception happened.
        }

        mHandler.sendEmptyMessageDelayed(EVENT_TICK, DELAY_TIME);

        registerReceiver(mIntentReceiver, mIntentFilter);
        registerReceiver(mIntentReceiverSDCard, mIntentFilterSDCard);
    }

    @Override
    public void onPause() {
        super.onPause();
        mHandler.removeMessages(EVENT_TICK);

        // we are no longer on the screen stop the observers
        unregisterReceiver(mIntentReceiver);
        unregisterReceiver(mIntentReceiverSDCard);
    }

    /**
     * @param arg0
     *            view
     */
    public void onClick(View arg0) {
        if (arg0.getId() == mLogRecord.getId()) {
            if (!mIsRecording) {
                if (Environment.getExternalStorageState().equals(Environment.MEDIA_REMOVED)
                        || Environment.getExternalStorageState().equals(Environment.MEDIA_BAD_REMOVAL)
                        || Environment.getExternalStorageState().equals(Environment.MEDIA_UNMOUNTED)) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("SD Card not available");
                    builder.setMessage("Please insert an SD Card.");
                    builder.setPositiveButton("OK", null);
                    builder.create().show();
                    return;
                }

                String state = Environment.getExternalStorageState();
                Log.i(TAG, "Environment.getExternalStorageState() is : " + state);

                if (Environment.getExternalStorageState().equals(Environment.MEDIA_SHARED)) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("sdcard is busy");
                    builder.setMessage("Sorry, your SD card is busy.");
                    builder.setPositiveButton("OK", null);
                    builder.create().show();
                    return;
                }
                // check if the EditText control has no content, if not, check
                // the content whether is right

                if (MAX_NUMBER_LENGTH < mIntervalEdit.getText().toString().length()
                        || 0 == mIntervalEdit.getText().toString().length()) {
                    Toast.makeText(this, "The input is not correct. Please input the number between 1 and 100.",
                            Toast.LENGTH_LONG).show();
                    return;
                }
                if (Integer.valueOf(mIntervalEdit.getText().toString()) > MAX_NUMBER
                        || Integer.valueOf(mIntervalEdit.getText().toString()) < 1) {
                    Toast.makeText(this, "The input is not correct. Please input the number between 1 and 100.",
                            Toast.LENGTH_LONG).show();
                    return;
                }
                mLogRecordInterval = Integer.valueOf(mIntervalEdit.getText().toString()) * FORMART_UPTIME;
                Log.i(TAG, String.valueOf(mLogRecordInterval));

                mLogRecord.setText(R.string.battery_info_Log_End);

                // Create a new file under the "/sdcard/batterylog" path
                Calendar rightNow = Calendar.getInstance();
                SimpleDateFormat fmt = new SimpleDateFormat("yyyyMMddhhmmss");
                String sysDateTime = fmt.format(rightNow.getTime());
                String fileName = "";
                fileName = fileName + sysDateTime;
                fileName = fileName + ".txt";
                Log.i(TAG, fileName);

                // mLogFile = new File("/sdcard/batterylog/" + fileName);
                mLogFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator
                        + "batterylog" + File.separator + fileName);
                try {
                    mLogFile.createNewFile();
                    String batteryInfoLable = "Battery status, level, scale, health, voltage, "
                            + "temperature, technology, time since boot:\n";
                    FileWriter fileWriter = new FileWriter(mLogFile);
                    fileWriter.write(batteryInfoLable);
                    fileWriter.flush();
                    fileWriter.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }

                mLogHandler.sendEmptyMessageDelayed(EVENT_LOG_RECORD, DELAY_TIME);

                mIsRecording = true;
            } else {
                mLogRecord.setText(R.string.battery_info_Log_Start);
                mLogHandler.removeMessages(EVENT_LOG_RECORD);
                mIsRecording = false;
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("BatteryLog Saved");
                builder.setMessage("BatteryLog has been saved under" + " /sdcard/batterylog.");
                builder.setPositiveButton("OK", null);
                builder.create().show();
            }
        }
    }

    public Handler mLogHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == EVENT_LOG_RECORD) {
                Log.i(TAG, "Record one time");
                writeCurrentBatteryInfo();
                sendEmptyMessageDelayed(EVENT_LOG_RECORD, mLogRecordInterval);
            }
        }

        private void writeCurrentBatteryInfo() {
            String logContent = "";
            logContent = logContent + mStatus.getText() + ", " + mLevel.getText() + ", " + mScale.getText() + ", "
                    + mHealth.getText() + ", " + mVoltage.getText() + ", " + mTemperature.getText() + ", "
                    + mTechnology.getText() + ", " + mUptime.getText() + "\n";
            FileWriter fileWriter = null;
            try {
                fileWriter = new FileWriter(mLogFile, true);
                fileWriter.write(logContent);
                fileWriter.flush();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (null != fileWriter) {
                    try {
                        fileWriter.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    };
}
