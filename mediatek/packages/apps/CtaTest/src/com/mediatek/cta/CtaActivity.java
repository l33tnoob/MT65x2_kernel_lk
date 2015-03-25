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

package com.mediatek.cta;

import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.app.Activity;
import android.app.ActivityThread;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.net.ConnectivityManager;
import android.net.http.AndroidHttpClient;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.provider.Telephony;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.android.internal.telephony.ISms;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

import com.mediatek.cta.camera.Camera;
import com.mediatek.cta.R;

import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;

import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.params.ConnRouteParams;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.HttpHost;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.StatusLine;

public class CtaActivity extends Activity implements OnClickListener {
    private static final String TAG = "CTA";

    private static final boolean FOR_CUSTOMER = false;
    private static final int TEST_01_PHONE_CALL = 0;
    private static final int TEST_01_PHONE_CALL_INTENT = 1;
    private static final int TEST_01_PHONE_CALL_REFLECT = 2;
    private static final int TEST_01_PHONE_CALL_END = 3;
    private static final int TEST_02_SEND_SMS = 4;
    private static final int TEST_02_SEND_SMS2 = 5;
    private static final int TEST_02_SEND_SMS_REFLECT1 = 6;
    private static final int TEST_02_SEND_SMS_REFLECT2 = 7;
    private static final int TEST_02_SEND_SMS_REFLECT3 = 8;
    private static final int TEST_02_SEND_MMS = 9;
    private static final int TEST_02_SEND_MMS_INTENT = 10;
    private static final int TEST_03_DATA_CONN = 11;
    private static final int TEST_03_DATA_CONN_REFLECT = 12;
    private static final int TEST_03_DATA_CONN_INTENT = 13;
    private static final int TEST_04_WIFI = 14;
    private static final int TEST_05_LOC_INTENT = 15;
    private static final int TEST_05_LOC_INTENT2 = 16;
    private static final int TEST_05_LOC_REQ = 17;
    private static final int TEST_05_LOC_GET = 18;
    private static final int TEST_06_RECORD_PHONE = 19;
    private static final int TEST_06_RECORD_PHONE2 = 20;
    private static final int TEST_07_RECORD = 21;
    private static final int TEST_07_RECORD2 = 22;
    private static final int TEST_08_CAMERA = 23;
    private static final int TEST_08_CAMERA_RECORD = 24;
    private static final int TEST_09_READ_SMS = 25;
    private static final int TEST_09_READ_MMS = 26;
    private static final int TEST_09_READ_CONTACT = 27;
    private static final int TEST_09_READ_CALLLOG = 28;
    private static final int TEST_10_BT = 29;
    private static final int TEST_10_NFC = 30;
    private static final int TEST_11_IMEI = 31;
    private static final int TOTAL_TEST_CASE = 32;

    // Message IDs
    private static final int MSG_START_TEST = 1;
    private static final int MSG_TEST_FINISH = 2;
    private static final int MSG_TEST_PRECONDITION_FAIL = 3;
    private static final int MSG_TEST_FAILED = 4;

    // Dialog IDs
    private static final int DIALOG_WAIT = 0;
    private static final int DIALOG_DISABLE_NETWORK = 1;
    private static final int DIALOG_DISABLE_WIFI = 2;
    private static final int DIALOG_DISABLE_BT = 3;
    private static final int DIALOG_IMEI = 4;
    private static final int DIALOG_LOC = 5;
    private static final int DIALOG_PHONE_NUMBER = 6;
    private static final int DIALOG_SMS_NUMBER = 7;

    private static final String KEY_TEST_ID ="TestId";
    private static final String KEY_LOC = "Loc";
    private static final String KEY_IMEI1 = "IMEI1";
    private static final String KEY_IMEI2 = "IMEI2";
    private static final String PREF = "pref";
    private static final String KEY_PHONE_NUMBER = "call_phone_number";
    private static final String KEY_SMS_NUMBER = "sms_phone_number";
    private static final String DEFAULT_NUMBER = "10086";

    private Button mButtons[] = new Button[TOTAL_TEST_CASE];
    private ProgressDialog mDialog = null;
    private Toast mToast = null;

    private MediaRecorder mRecorder;
    private LocationManager mLocationManager;
    private AudioRecord mAudioRecord;
    private String[] mStringArray = null;
    private boolean mIsRecordingPhone = false;  
    private boolean mIsRecordingAudio = false;  

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.what == MSG_START_TEST) {
                doTest(msg.arg1);
            } else {
                showResult(msg.what, msg.arg1);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.cta_test);
        mStringArray = getResources().getStringArray(R.array.test_case_array);

        // Add buttons
        ViewGroup root = (ViewGroup) findViewById(R.id.layout_root);
        for (int i = 0; i < TOTAL_TEST_CASE; i++) {
            mButtons[i] = new Button(this);
            mButtons[i].setId(i);
            mButtons[i].setText(mStringArray[i]);
            mButtons[i].setOnClickListener(this);
            // Phase out some test cases...
            if (i == TEST_01_PHONE_CALL ||
                    i == TEST_02_SEND_SMS ||
                    i == TEST_03_DATA_CONN) {
                continue;
            }
            // hide for customer
            if (FOR_CUSTOMER && (i == TEST_01_PHONE_CALL_END ||
                    i == TEST_02_SEND_MMS_INTENT ||
                    i == TEST_03_DATA_CONN_INTENT ||
                    i == TEST_05_LOC_INTENT ||
                    i == TEST_05_LOC_INTENT2 ||
                    i == TEST_10_NFC ||
                    i == TEST_11_IMEI)) {
                continue;
            }
            root.addView(mButtons[i]);
        }
    }

    @Override
    public void onDestroy() {
        if (mButtons[TEST_05_LOC_REQ].getText().equals(getString(R.string.cta_test_stop_request))){
            Log.i(TAG, "onDestroy removeUpdates()");
            mLocationManager.removeUpdates(mLocListener);
        }
        if (mButtons[TEST_06_RECORD_PHONE].getText().equals(getString(R.string.cta_test_stop_record)) ||
                mButtons[TEST_07_RECORD].getText().equals(getString(R.string.cta_test_stop_record))) {
            Log.i(TAG, "onDestroy MediaRecorder.stop()");
            try {
                mRecorder.stop();
            } catch (IllegalStateException e) {
                Log.i(TAG, "onDestroy MediaRecorder.stop() IllegalStateException");
            }
        }
        mIsRecordingPhone = false;
        mIsRecordingAudio = false;
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        int testId = v.getId();
        switch (testId) {
        case TEST_01_PHONE_CALL:
        case TEST_01_PHONE_CALL_INTENT:
        case TEST_01_PHONE_CALL_REFLECT:
        {
            Bundle bundle = new Bundle();
            bundle.putInt(KEY_TEST_ID, testId);
            removeDialog(DIALOG_PHONE_NUMBER);
            showDialog(DIALOG_PHONE_NUMBER, bundle);
            return;
        }
        case TEST_02_SEND_SMS:
        case TEST_02_SEND_SMS2:
        case TEST_02_SEND_SMS_REFLECT1:
        case TEST_02_SEND_SMS_REFLECT2:
        case TEST_02_SEND_SMS_REFLECT3:
        case TEST_02_SEND_MMS:
        case TEST_02_SEND_MMS_INTENT:
        {
            Bundle bundle = new Bundle();
            bundle.putInt(KEY_TEST_ID, testId);
            removeDialog(DIALOG_SMS_NUMBER);
            showDialog(DIALOG_SMS_NUMBER, bundle);
            return;
        }
        case TEST_03_DATA_CONN:
        case TEST_04_WIFI:
        case TEST_10_BT:
            showDialog(DIALOG_WAIT);
            break;
        case TEST_08_CAMERA_RECORD:
        {
            SurfaceView view = new SurfaceView(this);
            view.getHolder().addCallback(new SurfaceHolder.Callback() {
                @Override
                public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
                }

                @Override
                public void surfaceCreated(SurfaceHolder holder) {
                    mHolder = holder;
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, TEST_08_CAMERA_RECORD, 0));
                }

                @Override
                public void surfaceDestroyed(SurfaceHolder arg0) {
                }
            });
            setContentView(view);
            showDialog(DIALOG_WAIT);
            return;
        }
        default:
            break;
        }
        enableButtons(false);
        mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, testId, 0));
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
        case DIALOG_WAIT:
            mDialog = new ProgressDialog(this);
            mDialog.setMessage(getString(R.string.cta_test_testing));
            mDialog.setCancelable(false);
            return mDialog;
        case DIALOG_DISABLE_NETWORK:
            return new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.cta_test_error))
                    .setMessage(getString(R.string.cta_test_disable_data_conn))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_DISABLE_WIFI:
            return new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.cta_test_error))
                    .setMessage(getString(R.string.cta_test_disable_wifi))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_DISABLE_BT:
            return new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.cta_test_error))
                    .setMessage(getString(R.string.cta_test_disable_bt))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_IMEI:
            return new AlertDialog.Builder(this)
                    .setTitle("IMEI")
                    .setMessage("IMEI1: " + args.getString(KEY_IMEI1) +
                            "\nIMEI2: " + args.getString(KEY_IMEI2))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_LOC:
            return new AlertDialog.Builder(this)
                    .setTitle("Location")
                    .setMessage("getLastLocation() return:\n" + args.getString(KEY_LOC))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_PHONE_NUMBER:
        {
            final int testId = args.getInt(KEY_TEST_ID, TEST_01_PHONE_CALL);
            final EditText input = new EditText(this);
            final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
            input.setInputType(InputType.TYPE_CLASS_NUMBER);
            input.setText(preferences.getString(KEY_PHONE_NUMBER, DEFAULT_NUMBER));

            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (input.getText() != null && (!input.getText().toString().equals(""))) {
                        String number = input.getText().toString();
                        final SharedPreferences preferences = CtaActivity.this.getSharedPreferences(
                                PREF, Context.MODE_PRIVATE);
                        preferences.edit().putString(KEY_PHONE_NUMBER, number).commit();
                        mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, testId, 0));
                    }
                }
            };

            return new AlertDialog.Builder(this)
                    .setTitle("Phone number:")
                    .setView(input)
                    .setPositiveButton("OK", listener)
                    .setNegativeButton("Cancel", null).create();
        }
        case DIALOG_SMS_NUMBER:
        {
            final int testId = args.getInt(KEY_TEST_ID, TEST_02_SEND_SMS);
            final EditText input = new EditText(this);
            final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
            input.setInputType(InputType.TYPE_CLASS_NUMBER);
            input.setText(preferences.getString(KEY_SMS_NUMBER, DEFAULT_NUMBER));

            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    if (input.getText() != null && (!input.getText().toString().equals(""))) {
                        String number = input.getText().toString();
                        final SharedPreferences preferences = CtaActivity.this.getSharedPreferences(
                                PREF, Context.MODE_PRIVATE);
                        preferences.edit().putString(KEY_SMS_NUMBER, number).commit();
                        mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, testId, 0));
                        showDialog(DIALOG_WAIT);
                    }
                }
            };

            return new AlertDialog.Builder(this)
                    .setTitle("Phone number:")
                    .setView(input)
                    .setPositiveButton("OK", listener)
                    .setNegativeButton("Cancel", null).create();
        }
        default:
            return super.onCreateDialog(id);
        }
    }

    private void doTest(int index) {
        boolean ret = true;
        switch (index) {
        case TEST_01_PHONE_CALL:
            ret = test01_phoneCall();
            break;
        case TEST_01_PHONE_CALL_REFLECT:
            ret = test01_phoneCallReflect();
            break;
        case TEST_01_PHONE_CALL_INTENT:
            test01_phoneCallIntent();
            break;
        case TEST_01_PHONE_CALL_END:
            ret = test01_phoneCallEnd();
            break;
        case TEST_02_SEND_SMS:
            ret = test02_sendSms();
            break;
        case TEST_02_SEND_SMS2:
            ret = test02_sendSms2();
            break;
        case TEST_02_SEND_SMS_REFLECT1:
            ret = test02_sendSmsReflect1();
            break;
        case TEST_02_SEND_SMS_REFLECT2:
            ret = test02_sendSmsReflect2();
            break;
        case TEST_02_SEND_SMS_REFLECT3:
            ret = test02_sendSmsReflect3();
            break;
        case TEST_02_SEND_MMS:
            test02_sendMms();
            break;
        case TEST_02_SEND_MMS_INTENT:
            test02_sendMmsIntent();
            break;
        case TEST_03_DATA_CONN:
            if (test03_dataConnectPrecondition()) {
                test03_dataConnect();
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_03_DATA_CONN_REFLECT:
            if (test03_dataConnectPrecondition()) {
                ret = test03_dataConnectReflect();
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_03_DATA_CONN_INTENT:
            test03_dataConnectIntent();
            break;
        case TEST_04_WIFI:
            if (test04_enableWifiPrecondition()) {
                test04_enableWifi();
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_05_LOC_INTENT:
            test05_requestLocationIntent();
            break;
        case TEST_05_LOC_INTENT2:
            test05_requestLocationIntent2();
            break;
        case TEST_05_LOC_REQ:
            test05_requestLocation();
            break;
        case TEST_05_LOC_GET:
            test05_getLastLocation();
            break;
        case TEST_06_RECORD_PHONE:
            test06_recordPhone();
            break;
        case TEST_06_RECORD_PHONE2:
            test06_recordPhone2();
            break;
        case TEST_07_RECORD:
            test07_recordAudio();
            break;
        case TEST_07_RECORD2:
            test07_recordAudio2();
            break;
        case TEST_08_CAMERA:
            test08_camera();
            break;
        case TEST_08_CAMERA_RECORD:
            test08_camera_record();
            return;
        case TEST_09_READ_CONTACT:
            test09_read_contacts();
            break;
        case TEST_09_READ_CALLLOG:
            test09_read_calllog();
            break;
        case TEST_09_READ_SMS:
            test09_read_sms();
            break;
        case TEST_09_READ_MMS:
            test09_read_mms();
            break;
        case TEST_10_BT:
            if (test10_enableBluetoothPrecondition()) {
                test10_enableBluetooth();
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_10_NFC:
            test10_enableNfc();
            break;
        case TEST_11_IMEI:
            test11_getImei();
            break;
        }
        if (ret) {
            mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_FINISH, index, 700));
        } else {
            mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_FAILED, index, 700));
        }
    }

    private void showResult(int result, int index) {
        enableButtons(true);
        if (mDialog != null) {
            mDialog.dismiss();
        }

        if (result == MSG_TEST_FINISH) {
            switch (index) {
            case TEST_06_RECORD_PHONE:
            case TEST_06_RECORD_PHONE2:
            case TEST_07_RECORD:
            case TEST_07_RECORD2:
                if (mButtons[index].getText().equals(getString(R.string.cta_test_stop_record))) {
                    mButtons[index].setText(mStringArray[index]);
                } else {
                    mButtons[index].setText(R.string.cta_test_stop_record);
                }
                break;
            case TEST_05_LOC_REQ:
                if (mButtons[TEST_05_LOC_REQ].getText().equals(getString(R.string.cta_test_stop_request))) {
                    mButtons[TEST_05_LOC_REQ].setText(R.string.cta_test_5_location_request);
                } else {
                    mButtons[TEST_05_LOC_REQ].setText(R.string.cta_test_stop_request);
                }
                break;
            case TEST_08_CAMERA_RECORD:
                finish();
                startActivity(new Intent(this, CtaActivity.class));
                return;
            }
        } else if (result == MSG_TEST_PRECONDITION_FAIL) {
            switch (index) {
            case TEST_03_DATA_CONN:
            case TEST_03_DATA_CONN_REFLECT:
                showDialog(DIALOG_DISABLE_NETWORK);
                break;
            case TEST_04_WIFI:
                showDialog(DIALOG_DISABLE_WIFI);
                break;
            case TEST_10_BT:
                showDialog(DIALOG_DISABLE_BT);
                break;
            }
        } else if (result == MSG_TEST_FAILED) {
            showToast("Failed!");
        }
    }

    private String phoneNumber() {
        final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
        return preferences.getString(KEY_PHONE_NUMBER, DEFAULT_NUMBER);
    }

    private String smsNumber() {
        final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
        return preferences.getString(KEY_SMS_NUMBER, DEFAULT_NUMBER);
    }

    private boolean test01_phoneCall() {
        try {
            String packageName = ActivityThread.currentPackageName();
            ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService(TELEPHONY_SERVICE));
            Log.i(TAG, "test01_phoneCall " + phoneNumber());
            iTel.call(packageName, phoneNumber());
            return true;
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private void test01_phoneCallIntent() {
        Log.i(TAG, "test01_phoneCallIntent " + phoneNumber());
        Intent phoneIntent = new Intent("android.intent.action.CALL",
                Uri.parse("tel:" + phoneNumber()));
        phoneIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(phoneIntent);
    }

    private boolean test01_phoneCallReflect() {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test01_phoneCallReflect " + packageName + ", " + phoneNumber());
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {TELEPHONY_SERVICE});
            ITelephony phone = (ITelephony) ITelephony.Stub.asInterface(binder);
            phone.call(packageName, phoneNumber());
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private boolean test01_phoneCallEnd() {
        try {
            Log.i(TAG, "test01_phoneCallEnd");
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {TELEPHONY_SERVICE});
            ITelephony phone = (ITelephony) ITelephony.Stub.asInterface(binder);
            phone.endCall();
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private boolean test02_sendSms() {
        try {
            String packageName = ActivityThread.currentPackageName();
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService("isms"));
            Log.i(TAG, "test02_sendSms " + smsNumber());
            iccISms.sendData(packageName, smsNumber(), null, 0, "Hi".getBytes(), null, null);
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private boolean test02_sendSms2() {
        try {
            SmsManager smsManager = SmsManager.getDefault();
            Log.i(TAG, "test02_sendSms2 " + smsNumber());
            smsManager.sendTextMessage(smsNumber(), null, "SMS message", null, null);
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private boolean test02_sendSmsReflect1() {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test02_sendSmsReflect1 " + packageName + ", " + smsNumber());
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {"isms"});
            ISms simISms = (ISms) ISms.Stub.asInterface(binder);
            simISms.sendText(packageName, smsNumber(), null, "SMS message (Reflect 1)", null, null);
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private boolean test02_sendSmsReflect2() {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test02_sendSmsReflect2 " + packageName + ", " + smsNumber());
            Method method = Class.forName("android.os.ServiceManager")
              .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {"isms"});
            ISms simISms = (ISms) ISms.Stub.asInterface(binder);
            byte[] bytes = "SMS message (Reflect 2)".getBytes("GBK");
            simISms.sendData(packageName, smsNumber(), null, 0, bytes, null, null);
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private boolean test02_sendSmsReflect3() {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test02_sendSmsReflect3 " + packageName + ", " + smsNumber());
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {"isms"});
            ISms simISms = (ISms) ISms.Stub.asInterface(binder);
            List<String> parts = new ArrayList<String>();
            parts.add("SMS message (Reflect 3)");
            List<PendingIntent> intents = new ArrayList<PendingIntent>();
            intents.add(null);
            simISms.sendMultipartText(packageName, smsNumber(), null, parts, intents, null);
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private void test02_sendMmsIntent() {
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("subject", "CTA_TEST");
        intent.putExtra("address", smsNumber());
        intent.putExtra("sms_body", "This is CTA MMS Test");
        intent.putExtra(Intent.EXTRA_TEXT, "it's EXTRA_TEXT");
        intent.setType("image/*");
        intent.setClassName("com.android.mms","com.android.mms.ui.ComposeMessageActivity");
        startActivity(intent);
    }

    private void createHttpClient2() {
        /**
         * All pdu header fields.
         */
        final int MESSAGE_ID                      = 0x8B;
        final int MESSAGE_TYPE                    = 0x8C;
        /**
         * X-Mms-Message-Type field types.
         */
        final int MESSAGE_TYPE_SEND_REQ           = 0x80;
        final int MESSAGE_TYPE_SEND_CONF          = 0x81;
        final int MESSAGE_TYPE_NOTIFICATION_IND   = 0x82;
        final int MESSAGE_TYPE_NOTIFYRESP_IND     = 0x83;

        AndroidHttpClient httpClient = AndroidHttpClient.newInstance("MMS 1.0");
        String url = "http://www.baidu.com";
        byte[] pdu;

        try {
            URI hostUrl = new URI(url);
            HttpHost target = new HttpHost(hostUrl.getHost(), hostUrl.getPort(),HttpHost.DEFAULT_SCHEME_NAME);
            HttpPost post = new HttpPost(url);
            HttpRequest req = null;

            pdu = new byte[2];
            pdu[0] = (byte) MESSAGE_TYPE;
            pdu[1] = (byte) MESSAGE_TYPE_SEND_REQ;

            ByteArrayEntity entity = new ByteArrayEntity(pdu);
            entity.setContentType("application/vnd.wap.mms-message");
            post.setEntity(entity);
            req = post;

            final HttpResponse execute = httpClient.execute(target, req);
        } catch (Exception e){
            e.printStackTrace();
        }
    }

    private void test02_sendMms() {
        Log.i(TAG, "test02_sendMms()");
        createHttpClient2();
    }

    private boolean test03_dataConnectPrecondition() {
        ConnectivityManager connService = ConnectivityManager.from(this);
        return !connService.getMobileDataEnabled();
    }

    private void test03_dataConnect() {
        ConnectivityManager connService = ConnectivityManager.from(this);
        Log.i(TAG, "test03_dataConnect call setMobileDataEnabled()");
        connService.setMobileDataEnabled(true);
    }

    private Object invokeBooleanArgMethod(String methodName, boolean value) throws Exception {
        ConnectivityManager mConnectivityManager =
                (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        Class ownerClass = mConnectivityManager.getClass();
        Class[] argsClass = new Class[1];
        argsClass[0] = boolean.class;
        Method method = ownerClass.getMethod(methodName, argsClass);
        return method.invoke(mConnectivityManager, value);
    }

    private boolean test03_dataConnectReflect() {
        Log.i(TAG, "test03_dataConnectReflect");
        try {
            invokeBooleanArgMethod("setMobileDataEnabled", true);
            return true;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return false;
        }
    }

    private void test03_dataConnectIntent() {
        Log.i(TAG, "test03_dataConnectIntent send Settings.ACTION_WIRELESS_SETTINGS");
        Intent intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private boolean test04_enableWifiPrecondition() {
        WifiManager wifiManager = (WifiManager) this.getSystemService(WIFI_SERVICE);
        return wifiManager.getWifiState() == WifiManager.WIFI_STATE_DISABLED;
    }

    private void test04_enableWifi() {
        WifiManager wifiManager = (WifiManager) this.getSystemService(WIFI_SERVICE);
        Log.i(TAG, "test04_enableWifi call setWifiEnabled()");
        wifiManager.setWifiEnabled(true);
    }
    
    LocationListener mLocListener = new LocationListener() {
        public void onLocationChanged(Location location) {
            showToast("onLocationChanged() return: " +
                    " getLatitude: " + location.getLatitude() +
                    " getLongitude: " + location.getLongitude() +
                    " getAltitude: " + location.getAltitude() +
                    " getAccuracy: " + location.getAccuracy() + 
                    " getBearing: " + location.getBearing() + 
                    " getSpeed: " + location.getSpeed());
        }
    
        public void onStatusChanged(String provider, int status, Bundle extras) {
            // Do nothing
        }
    
        public void onProviderEnabled(String provider) {
            // Do nothing
        }
    
        public void onProviderDisabled(String provider) {
            // Do nothing
        }
    };

    private void test05_requestLocation() {
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);

        if (mButtons[TEST_05_LOC_REQ].getText().equals(getString(R.string.cta_test_stop_request))){
            Log.i(TAG, "test05_requestLocation call removeUpdates()");
            mLocationManager.removeUpdates(mLocListener);
        } else {
            Log.i(TAG, "test05_requestLocation call requestLocationUpdates()");
            LocationRequest request = LocationRequest.create();
            request.setQuality(LocationRequest.POWER_HIGH);
            request.setInterval(1000);
            mLocationManager.requestLocationUpdates(request, mLocListener, null);
        }
    }

    private void test05_getLastLocation() {
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        Log.i(TAG, "test05_getLastLocation call getLastLocation()");
        Location location = mLocationManager.getLastLocation();
        Bundle bundle = new Bundle();
        if (location != null) {
            Log.i(TAG, "getLastLocation() return not null");
            bundle.putString(KEY_LOC, "getLatitude: " + location.getLatitude() +
                    " getLongitude: " + location.getLongitude() +
                    " getAltitude: " + location.getAltitude() +
                    " getAccuracy: " + location.getAccuracy() + 
                    " getBearing: " + location.getBearing() + 
                    " getSpeed: " + location.getSpeed());
        } else {
            Log.i(TAG, "getLastLocation() return null");
        }
        removeDialog(DIALOG_LOC);
        showDialog(DIALOG_LOC, bundle);
    }

    private void test05_requestLocationIntent() {
        Log.i(TAG, "test05_requestLocationIntent() send ACTION_LOCATION_SOURCE_SETTINGS");
        Intent intent = new Intent(android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private void test05_requestLocationIntent2() {
        Intent GPSIntent = new Intent();  
        GPSIntent.setClassName("com.android.settings",
                "com.android.settings.widget.SettingsAppWidgetProvider");  
        GPSIntent.addCategory("android.intent.category.ALTERNATIVE");  
        GPSIntent.setData(Uri.parse("custom:3"));  
        sendBroadcast(GPSIntent);
    }

    private void test06_recordPhone() {
        try {
            if (mButtons[TEST_06_RECORD_PHONE].getText().equals(getString(R.string.cta_test_stop_record))){
                Log.i(TAG, "test06_recordPhone call MediaRecorder.stop()");
                mRecorder.stop();
            } else {
                Log.i(TAG, "test06_recordPhone call MediaRecorder.start()");
                mRecorder = new MediaRecorder();
                mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
                mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
                mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
                mRecorder.setOutputFile("/sdcard/cta_test_phone_record.3gpp");
                mRecorder.prepare();
                mRecorder.start();
            }
        } catch (IllegalStateException e) {
            Log.i(TAG, "test06_recordPhone IllegalStateException");
        } catch (IOException e) {
            Log.i(TAG, "test06_recordPhone IOException");
        }
    }

    private void test06_recordPhone2() {
        if (mButtons[TEST_06_RECORD_PHONE2].getText().equals(getString(R.string.cta_test_stop_record))){
            Log.i(TAG, "test06_recordPhone2 stop");
            mIsRecordingPhone = false;
        } else {
            mIsRecordingPhone = true;
            Log.i(TAG, "test06_recordPhone2 start");
            new Thread(new Runnable() {
                public void run() {
                    try {
                        DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream("/sdcard/cta_test_phone_record.raw")));
                        int bufferSize = AudioRecord.getMinBufferSize(44100, AudioFormat.CHANNEL_IN_STEREO,
                                AudioFormat.ENCODING_PCM_16BIT) * 10;
                        short[] buffer = new short[bufferSize];  
                        AudioRecord audioRecord = new AudioRecord(
                                MediaRecorder.AudioSource.MIC, 44100, AudioFormat.CHANNEL_IN_STEREO,
                                AudioFormat.ENCODING_PCM_16BIT, bufferSize);
                        Log.i(TAG, "test06_recordPhone2 startRecording()");
                        audioRecord.startRecording();  

                        while (mIsRecordingPhone) {
                            Log.i(TAG, "test06_recordPhone2 read()");
                            int bufferReadResult = audioRecord.read(buffer, 0, buffer.length);  
                            for (int i = 0; i < bufferReadResult; i++){  
                                dos.writeShort(buffer[i]);
                            }
                        }
                        Log.i(TAG, "test06_recordPhone2 stop()");
                        audioRecord.stop();  
                        dos.close();  
                    } catch (Exception e) {  
                        Log.e(TAG, e.toString());
                    }
                }
            }).start();
        }
    }

    private void test07_recordAudio() {
        try {
            if (mButtons[TEST_07_RECORD].getText().equals(getString(R.string.cta_test_stop_record))){
                Log.i(TAG, "test07_recordAudio call MediaRecorder.stop()");
                mRecorder.stop();
            } else {
                Log.i(TAG, "test07_recordAudio call MediaRecorder.start()");
                mRecorder = new MediaRecorder();
                mRecorder.setAudioSource(MediaRecorder.AudioSource.DEFAULT);
                mRecorder.setOutputFormat(MediaRecorder.OutputFormat.DEFAULT);
                mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
                mRecorder.setOutputFile("/sdcard/cta_test_audio_record.3gpp");
                mRecorder.prepare();
                mRecorder.start();
            }
        } catch (IllegalStateException e) {
            Log.i(TAG, "test07_recordAudio IllegalStateException");
        } catch (IOException e) {
            Log.i(TAG, "test07_recordAudio IOException");
        }
    }

    private void test07_recordAudio2() {
        if (mButtons[TEST_07_RECORD2].getText().equals(getString(R.string.cta_test_stop_record))){
            Log.i(TAG, "test07_recordAudio2 stop");
            mIsRecordingAudio = false;
        } else {
            mIsRecordingAudio = true;
            Log.i(TAG, "test07_recordAudio2 start");
            new Thread(new Runnable() {
                public void run() {
                    try {
                        DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream("/sdcard/cta_test_audio_record.raw")));
                        int bufferSize = AudioRecord.getMinBufferSize(44100, AudioFormat.CHANNEL_IN_STEREO,
                                AudioFormat.ENCODING_PCM_16BIT) * 10;
                        short[] buffer = new short[bufferSize];  
                        AudioRecord audioRecord = new AudioRecord(
                                MediaRecorder.AudioSource.MIC, 44100, AudioFormat.CHANNEL_IN_STEREO,
                                AudioFormat.ENCODING_PCM_16BIT, bufferSize);
                        Log.i(TAG, "test07_recordAudio2 startRecording()");
                        audioRecord.startRecording();  

                        while (mIsRecordingAudio) {
                            Log.i(TAG, "test07_recordAudio2 read()");
                            int bufferReadResult = audioRecord.read(buffer, 0, buffer.length);  
                            for (int i = 0; i < bufferReadResult; i++){  
                                dos.writeShort(buffer[i]);
                            }
                        }
                        Log.i(TAG, "test07_recordAudio2 stop()");
                        audioRecord.stop();  
                        dos.close();  
                    } catch (Exception e) {  
                        Log.e(TAG, e.toString());
                    }
                }
            }).start();
        }
    }

    private void test08_camera() {
        Intent captureIntent = new Intent();
        captureIntent.setClass(this, Camera.class);
        this.startActivity(captureIntent);
    }

    SurfaceHolder mHolder;
    private void test08_camera_record() {
        new Thread(new Runnable() {
            public void run() {
                try {
                    MediaRecorder recorder = new MediaRecorder();
                    recorder.setAudioSource(MediaRecorder.AudioSource.MIC);
                    recorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
                    recorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
                    recorder.setVideoEncoder(MediaRecorder.VideoEncoder.H263);
                    recorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
                    recorder.setVideoSize(176, 144);
                    recorder.setVideoFrameRate(30);
                    recorder.setPreviewDisplay(mHolder.getSurface());
                    recorder.setOutputFile("/sdcard/Record.mp4");
                    recorder.prepare();
                    recorder.start();
                    try {
                        Thread.sleep(10*1000);
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                    recorder.stop();
                    recorder.reset();
                    recorder.release();
                } catch (IllegalStateException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                    return;
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                    return;
                } finally {
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_FINISH, TEST_08_CAMERA_RECORD, 0));
                }
            }
        }).start();
    }

    private void test09_read_contacts() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.ID, CtaListActivity.ID_CONTACTS);
        this.startActivity(intent);
    }

    private void test09_read_calllog() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.ID, CtaListActivity.ID_CALL_LOG);
        this.startActivity(intent);
    }

    private void test09_read_sms() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.ID, CtaListActivity.ID_SMS);
        this.startActivity(intent);
    }

    private void test09_read_mms() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.ID, CtaListActivity.ID_MMS);
        this.startActivity(intent);
    }

    private boolean test10_enableBluetoothPrecondition() {
        BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        return mBtAdapter.getState() == BluetoothAdapter.STATE_OFF;
    }

    private void test10_enableBluetooth() {
        BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        Log.i(TAG, "test10_enableBluetooth call BluetoothAdapter.enable()");
        mBtAdapter.enable();
    }

    private void test10_enableNfc() {
        Intent intent = new Intent(Settings.ACTION_NFCSHARING_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private void test11_getImei() {
        Log.i(TAG, "test11_getImei call getDeviceIdGemini()");
        String imei1 = null;
        String imei2 = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            TelephonyManagerEx tm = TelephonyManagerEx.getDefault();
            if (tm != null) {
                imei1 = tm.getDeviceId(PhoneConstants.GEMINI_SIM_1);
                imei2 = tm.getDeviceId(PhoneConstants.GEMINI_SIM_2);
            }
        } else {
            TelephonyManager tm = (TelephonyManager) getSystemService("phone");
            if (tm != null) {
                imei1 = tm.getDeviceId();
            }
        }
        Log.i(TAG, "test11_getImei call getDeviceIdGemini()" + imei1 + imei2);

        Bundle bundle = new Bundle();
        bundle.putString(KEY_IMEI1, imei1);
        bundle.putString(KEY_IMEI2, imei2);
        removeDialog(DIALOG_IMEI);
        showDialog(DIALOG_IMEI, bundle);
    }

    private void enableButtons(boolean enable) {
        for (int i = 0; i < TOTAL_TEST_CASE; i++) {
            mButtons[i].setClickable(enable);
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
