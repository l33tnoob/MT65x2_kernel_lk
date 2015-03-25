/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.cwtest;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Resources.NotFoundException;
import android.content.DialogInterface;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.GpsStatus.NmeaListener;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.StrictMode;
import android.os.SystemProperties;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import android.widget.TextView;
import android.widget.Toast;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

public class CWTest extends Activity {

    private static final String TAG = "CWTest/Activity";
    private static final String TAG_BG = "CWTest/BG";
    private static final String COMMAND_END = "*";
    private static final String COMMAND_START = "$";
    private static final int LOCATION_MAX_LENGTH = 12;
    private static final String INTENT_ACTION_SCREEN_OFF = "android.intent.action.SCREEN_OFF";
    private static final String FIRST_TIME = "first.time";

    private static final int INPUT_VALUE_MAX = 999;
    private static final int INPUT_VALUE_MIN = 0;
    private static final int ONE_SECOND = 3000;

    private static final int HANDLE_START_TEST = 1001;
    private static final int HANDLE_STOP_TEST = 1002;
    private static final int HANDLE_UPDATE_RESULT = 1003;

    private static final int RESPONSE_ARRAY_LENGTH = 4;
    
    private static final int DIALOG_GPS_ERROR = 0;
    
    private static final String MNL_PROP_NAME = "persist.radio.mnl.prop";
    private static final String DEFAULT_MNL_PROP = "0001100";
    

    private boolean mShowLoc = false;
    private boolean mStartNmeaRecord = false;

    private ClientSocket mSocketClient = null;

    private LocationManager mLocationManager = null;
    private YgpsWakeLock mYgpsWakeLock = null;
    private Location mLastLocation = null;
    private Button mBtnStart = null;
    private Button mBtnStop = null;
    
    private TextView mCnrTv = null;
    private TextView mDriftTv = null;
    
    private EditText mEtTimes = null;
    private EditText mEtInterval = null;
    
    private TextView mCurrentTimesTv = null;
    
    private TextView mMaxCnrTv = null;
    private TextView mMinCnrTv = null;
    
    private int mMaxCnr = 0;
    private int mMinCnr = 0;

    private String mProvider = "";
    private String mStatus = "";

    // added end
    // added to receive PowerKey pressed
    private IntentFilter mPowerKeyFilter = null;
    private BroadcastReceiver mPowerKeyReceiver = null;
    // added end
    
    private int mTotalTimes = 0;
    private int mInterval = 0;
    private int mCurrentTimes = 0;

    private boolean mDebugFile = false;
    /**
     * Convert Integer array to string with specified length
     * 
     * @param array
     *            Integer array
     * @param count
     *            Specified length
     * @return Integer array numbers string
     */
    private String toString(int[] array, int count) {
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("(");
        for (int idx = 0; idx < count; idx++) {
            strBuilder.append(Integer.toString(array[idx]));
            strBuilder.append(",");
        }
        strBuilder.append(")");
        return strBuilder.toString();
    }

    /**
     * Convert Float array to string with specified length
     * 
     * @param array
     *            Float array
     * @param count
     *            Specified length
     * @return Float array numbers string
     */
    private String toString(float[] array, int count) {
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("(");
        for (int idx = 0; idx < count; idx++) {
            strBuilder.append(Float.toString(array[idx]));
            strBuilder.append(",");
        }
        strBuilder.append(")");
        return strBuilder.toString();
    }


    private  String getDebug2FileProp(String defaultValue) {
        String result = defaultValue;
        String prop = SystemProperties.get(MNL_PROP_NAME);

        int index = 2;
        Xlog.v(TAG, "getMnlProp: " + prop);
        if (null == prop || prop.isEmpty()) {
            result = defaultValue;
        } else {
            char c = prop.charAt(index);
            result = String.valueOf(c);
        }
        Xlog.v(TAG, "getMnlProp result: " + result);
        return result;
    }
    
    private void setDebug2FileMnlProp(String value) {
        Xlog.v(TAG, "setMnlProp: "+ value);
        String prop = SystemProperties.get(MNL_PROP_NAME);

        int index = 2;

        if (null == prop || prop.isEmpty()) {
            prop = DEFAULT_MNL_PROP;
        }
        if (prop.length() > index) {
            char[] charArray = prop.toCharArray();
            charArray[index] = value.charAt(0);
            String newProp = String.valueOf(charArray);
            SystemProperties.set(MNL_PROP_NAME, newProp);
            Xlog.v(TAG, "setMnlProp newProp: " + newProp);
        }

    }
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Xlog.v(TAG, "Enter onCreate  function of Main Activity");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cw_test);
        
        mBtnStart = (Button)findViewById(R.id.cw_test_start_btn);
        mBtnStop = (Button)findViewById(R.id.cw_test_stop_btn);
        mCnrTv = (TextView)findViewById(R.id.cw_test_cnr_content);
        mDriftTv = (TextView)findViewById(R.id.cw_test_clock_drift_content);
        mCnrTv.setText("0");
        mDriftTv.setText("0");
        
        mEtTimes = (EditText)findViewById(R.id.cw_test_times_content);
        mEtInterval = (EditText)findViewById(R.id.cw_test_interval_content);
        
        mCurrentTimesTv = (TextView)findViewById(R.id.cw_test_current_times_content);
        mCurrentTimesTv.setText("0");
        
        mMaxCnrTv = (TextView)findViewById(R.id.cw_test_max_cnr_content);
        mMinCnrTv = (TextView)findViewById(R.id.cw_test_min_cnr_content);
        
        mMaxCnrTv.setText("0");
        mMinCnrTv.setText("0");
        
        mBtnStart.setOnClickListener(mBtnClickListener);
        mBtnStop.setOnClickListener(mBtnClickListener);
        mBtnStart.setEnabled(false);
        mBtnStop.setEnabled(false);
        
        // open debug file
        String ss = getDebug2FileProp("0");
        
        if(ss.equals("0")) {
            mDebugFile = false;
            setDebug2FileMnlProp("1");
        } else {
            mDebugFile = true;
        }

        StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
                .detectNetwork() // or .detectAll() for all detectable problems
                .build());
        mYgpsWakeLock = new YgpsWakeLock();
        mYgpsWakeLock.acquireScreenWakeLock(this);

        try {
            mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
            if (mLocationManager != null) {
                mLocationManager.requestLocationUpdates(
                        LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
                mLocationManager.addGpsStatusListener(mGpsListener);

                if (mLocationManager
                        .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                   Xlog.w(TAG, "provider enabled");
                   
                } else {
                    Xlog.w(TAG, "provider disabled");
                    showDialog(DIALOG_GPS_ERROR);
                }
            } else {
                Xlog.w(TAG, "new mLocationManager failed");
            }
        } catch (SecurityException e) {
            Toast.makeText(this, "security exception", Toast.LENGTH_LONG)
                    .show();
            Xlog.w(TAG, "Exception: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            Xlog.w(TAG, "Exception: " + e.getMessage());
        }

        mPowerKeyFilter = new IntentFilter(INTENT_ACTION_SCREEN_OFF);
        mPowerKeyReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                Xlog.v(TAG, "onReceive, receive SCREEN_OFF event");
                // finish();
            }
        };
        registerReceiver(mPowerKeyReceiver, mPowerKeyFilter);
        Xlog.v(TAG, "registerReceiver powerKeyReceiver");
        mSocketClient = new ClientSocket(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        Xlog.v(TAG, "Enter onPause function");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Xlog.v(TAG, "Enter onResume function");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Xlog.v(TAG, "Enter onStop function");

        mYgpsWakeLock.release();

    }

    @Override
    protected void onRestart() {
        Xlog.v(TAG, "Enter onRestart function");
 
        if (null != mYgpsWakeLock) {
            mYgpsWakeLock.acquireScreenWakeLock(this);
        } else {
            Xlog.d(TAG, "mYGPSWakeLock is null");
        }
        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "enter onDestroy function");
        mLocationManager.removeUpdates(mLocListener);
        mLocationManager.removeGpsStatusListener(mGpsListener);

       if(mDebugFile == false) {
            setDebug2FileMnlProp("0");
        }
        mHandler.removeMessages(HANDLE_START_TEST);
        unregisterReceiver(mPowerKeyReceiver);
        Xlog.v(TAG, "unregisterReceiver powerKeyReceiver");
        mSocketClient.endClient();
        super.onDestroy();
    }

    public final LocationListener mLocListener = new LocationListener() {

        // @Override
        public void onLocationChanged(Location location) {
            Xlog.v(TAG, "Enter onLocationChanged function");
        }

        // @Override
        public void onProviderDisabled(String provider) {
            Xlog.v(TAG, "Enter onProviderDisabled function");
        }

        // @Override
        public void onProviderEnabled(String provider) {
            Xlog.v(TAG, "Enter onProviderEnabled function");
        }

        // @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            Xlog.v(TAG, "Enter onStatusChanged function");
        }
    };

    public final GpsStatus.Listener mGpsListener = new GpsStatus.Listener() {
        private void onFirstFix(int ttff) {
            Xlog.v(TAG, "Enter onFirstFix function: ttff = " + ttff);
        }
        
        private void onPreFix(int ttff) {
            Xlog.v(TAG, "Enter onPreFix function: ttff = " + ttff);
        }
        
        private boolean isLocationFixed(Iterable<GpsSatellite> list) {
            boolean fixed = false;
            synchronized (this) {
                for (GpsSatellite sate : list) {
                    if (sate.usedInFix()) {
                        fixed = true;
                        break;
                    }
                }
            }
            return fixed;
        }

        public void onGpsStatusChanged(int event) {
            Xlog.v(TAG, "Enter onGpsStatusChanged function");
            GpsStatus status = mLocationManager.getGpsStatus(null);
            switch (event) {
            case GpsStatus.GPS_EVENT_STARTED:
                mBtnStart.setEnabled(true);
                mStatus = "gps status started";
                break;
            case GpsStatus.GPS_EVENT_STOPPED:
                mStatus = "gps status stopped";
                break;
            case GpsStatus.GPS_EVENT_FIRST_FIX:
                onFirstFix(status.getTimeToFirstFix());
                mStatus = "gps status first fix";
                break;
            case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                if (!isLocationFixed(status.getSatellites())) {
                    Xlog.v(TAG, "gps status unavailable");
                } else {
                    Xlog.v(TAG, "gps status available");
                }
                break;
            default:
                break;
            }

            Xlog.v(TAG, "onGpsStatusChanged:" + event + " Status:" + mStatus);
        }
    };
    

    public final OnClickListener mBtnClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Bundle extras = new Bundle();
            if (v == (View) mBtnStart) {
                Xlog.v(TAG, "Start button is pressed");
                startCWTest();
            }else if(v == (View) mBtnStop) {
                Xlog.v(TAG, "Stop button is pressed");
                stopCWTest();
            }else {
                return;
            }
        }
    };

    private void startCWTest() {
        
        try {
                mTotalTimes = Integer.parseInt(mEtTimes.getText().toString());
                mInterval = Integer.parseInt(mEtInterval.getText().toString());
                mCurrentTimes = 0;
                if(mTotalTimes <= 0 || mInterval < 3) {
                    Toast.makeText(this, "please input right number, times > 0 and interval >=3s",
                        Toast.LENGTH_SHORT).show();
                    return;
                }
                mCurrentTimesTv.setText("0");
                mCnrTv.setText("0");
                mDriftTv.setText("0");
                mMaxCnrTv.setText("0");
                mMinCnrTv.setText("0");
                mMaxCnr = 0;
                mMinCnr = 0;
                mBtnStart.setEnabled(false);
                mBtnStop.setEnabled(true);
        } catch (NumberFormatException e) {
                Toast.makeText(this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
        }
        mHandler.sendEmptyMessage(HANDLE_START_TEST);
    }
    
    private void stopCWTest() {
        mBtnStart.setEnabled(true);
        mBtnStop.setEnabled(false);
        mHandler.removeMessages(HANDLE_START_TEST);
        mHandler.sendEmptyMessage(HANDLE_STOP_TEST);
    }
    
    /**
     * Send command to MNL server
     * 
     * @param command
     *            PMTK command to be send
     */
    private void sendCommand(String command) {
        Xlog.v(TAG, "GPS Command is " + command);

        int index1 = command.indexOf(COMMAND_START);
        int index2 = command.indexOf(COMMAND_END);
        String com = command;
        if (index1 != -1 && index2 != -1) {

            com = com.substring(index1 + 1, index2);
        } else if (index1 != -1) {
            com = com.substring(index1 + 1);
        } else if (index2 != -1) {
            com = com.substring(0, index2);
        }
        mSocketClient.sendCommand(com.trim());
    }

    /**
     * Invoked when get GPS server respond
     * 
     * @param res
     *            Response message
     */
    public void onResponse(String ss) {
        
        Xlog.v(TAG, "Enter getResponse: " + ss);
        if (null == ss || ss.isEmpty()) {
            return;
        }
        
        if(ss.contains("PMTK817") == false) {
            return;   
        }
        int startIndex = ss.indexOf("$PMTK817");

        String response = ss.substring(startIndex);
        Xlog.v(TAG, "start string " + response);
        
        int endIndex = response.indexOf("*");
        endIndex = endIndex + 3;
        
        String res = response.substring(0, endIndex);
        Xlog.v(TAG, "last string " + res);
        
        if (res.startsWith("$PMTK817")) {
    
            Message m = mHandler.obtainMessage(HANDLE_UPDATE_RESULT);
            m.obj = res;
            mHandler.sendMessage(m);
        }else {
            Xlog.v(TAG, "result is not proper");
        }


    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case HANDLE_START_TEST:
                mCurrentTimes ++;
                //mCurrentTimesTv.setText(Integer.toString(mCurrentTimes));
                Xlog.v(TAG, "[--CMD--]send start command, times = " + Integer.toString(mCurrentTimes));
                sendCommand("$PMTK817,1");
                if(mCurrentTimes >= mTotalTimes) {
                    Xlog.v(TAG, "Test done");
                    break;
                }
                sendEmptyMessageDelayed(HANDLE_START_TEST, mInterval*1000);
                break;
                
            case HANDLE_STOP_TEST:
                sendCommand("$PMTK817,0");
                mCurrentTimesTv.setText("0");
                mCnrTv.setText("0");
                mDriftTv.setText("0");
                break;
            case HANDLE_UPDATE_RESULT:  /* $PMTK817,2,0037,-5.614*23 */
                String res = msg.obj.toString();     
                String[] strA = res.split(",");
                mCurrentTimesTv.setText(Integer.toString(mCurrentTimes));
                if (strA.length >= RESPONSE_ARRAY_LENGTH) {
                    Xlog.v(TAG, "[--CMD--]receive command, times = " + Integer.toString(mCurrentTimes));
                    String strCNR = strA[RESPONSE_ARRAY_LENGTH - 2];
                    int cnr = Integer.parseInt(strCNR);
                    if(mCurrentTimes == 1){
                        mMaxCnr = cnr;
                        mMinCnr = cnr;
                    } else {
                        if(mMaxCnr < cnr){
                            mMaxCnr = cnr;
                        }
                        if(mMinCnr > cnr){
                            mMinCnr = cnr;
                        }
                    }
                    mMaxCnrTv.setText(Integer.toString(mMaxCnr));
                    mMinCnrTv.setText(Integer.toString(mMinCnr));
                    
                    mCnrTv.setText(Integer.toString(cnr));
                    char strB[] = strA[RESPONSE_ARRAY_LENGTH - 1].toCharArray();
                    int index = 0;
                    while(index < strB.length) {
                        if(strB[index] == '*') {
                            break;
                        }
                        index++;
                    }
                    
                    String strDrift = new String(strB, 0, index);
                    mDriftTv.setText(strDrift);
    
                } 
                if(mCurrentTimes >= mTotalTimes) {
                    Xlog.v(TAG, "Test done, Reset button");
                    mBtnStart.setEnabled(true);
                    mBtnStop.setEnabled(false);
                    break;
                }
                break;
            default:
                break;
            }
            super.handleMessage(msg);
        }
    };
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_GPS_ERROR:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.cw_test_dialog_error);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.cw_test_dialog_error_message));
            builder.setPositiveButton(R.string.wifi_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        default:
            Xlog.d(TAG, "error dialog ID");
            break;
        }
        return dialog;
    }
    class YgpsWakeLock {
        private PowerManager.WakeLock mScreenWakeLock = null;
        private PowerManager.WakeLock mCpuWakeLock = null;

        /**
         * Acquire CPU wake lock
         * 
         * @param context
         *            Getting lock context
         */
        void acquireCpuWakeLock(Context context) {
            Xlog.v(TAG, "Acquiring cpu wake lock");
            if (mCpuWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mCpuWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mCpuWakeLock.acquire();
        }

        /**
         * Acquire screen wake lock
         * 
         * @param context
         *            Getting lock context
         */
        void acquireScreenWakeLock(Context context) {
            Xlog.v(TAG, "Acquiring screen wake lock");
            if (mScreenWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mScreenWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mScreenWakeLock.acquire();
        }

        /**
         * Release wake locks
         */
        void release() {
            Xlog.v(TAG, "Releasing wake lock");
            if (mCpuWakeLock != null) {
                mCpuWakeLock.release();
                mCpuWakeLock = null;
            }
            if (mScreenWakeLock != null) {
                mScreenWakeLock.release();
                mScreenWakeLock = null;
            }
        }
    }

}
