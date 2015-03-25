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

package com.mediatek.engineermode.bluetooth;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;


import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * Do BT BLE test mode
 * 
 * @author mtk54040
 * 
 */

public class BleTestMode extends Activity implements OnClickListener {

    private BluetoothAdapter mBtAdapter = null;

    private static final String TAG = "BLETestMode";

    // views in this activity
    // button
    private Button mBtnStart = null;
    private Button mBtnStop = null;

    private TextView mResultText = null;
    private String mResultStr = "R:";

    // Radio Button
    // BLE test mode Tx/Rx RadioGroup
    private RadioButton mRBtnTx = null;
    private RadioButton mRBtnRx = null;
    // BLE test mode Hopping/Single RadioGroup
    private RadioButton mRBtnHopping = null;
    private RadioButton mRBtnSingle = null;
    private static final int CHANNEL_NUM = 40;
    // Checkbox
    private CheckBox mContinune = null;

    // Spinner
    private Spinner mChannelSpn = null;
    private Spinner mPatternSpn = null;

    // spinner value
    private byte mChannelValue = 0x00;
    private byte mPatternValue = 0x00;

    // btn values
    private boolean mTxTest = true;
  //  private boolean mHopping = false;

    private static final int RENTURN_SUCCESS = 0;

    // jni layer object
    private BtTest mBtTest = null;

    // BtTest object init and start test flag
    private boolean mBtInited = false;
    private boolean mTestStared = false;// this flag is not necessary in this
    // module
    private boolean mIniting = false;
    // Dialog ID
    private static final int CHECK_BT_STATE = 1;
    private static final int CHECK_STOP = 2;
    private static final int CHECK_BT_DEVEICE = 3;
    // Message ID
    private static final int TEST_START = 11;
    private static final int TEST_STOP = 12;
    // execuate result
    private static final int TEST_SUCCESS = 13;
    private static final int TEST_FAILED = 14;
    private static final int STOP_FINISH = 15;
    // activity exit message ID
    private static final int ACTIVITY_EXIT = 20;

//    private HandlerThread mWorkThread = null;
    // private volatile Looper mWorkLooper = null;
    private WorkHandler mWorkHandler = null;

    @Override
    protected void onCreate(Bundle onSavedInstanceState) {
//        Xlog.v(TAG, "-->onCreate");
        super.onCreate(onSavedInstanceState);
        setContentView(R.layout.ble_test_mode);
        
        // Initialize UI component
        mBtnStart = (Button) findViewById(R.id.BLEStart);
        mBtnStop = (Button) findViewById(R.id.BLEStop);
        mResultText = (TextView) findViewById(R.id.BLEResult_Text);

        mRBtnTx = (RadioButton) findViewById(R.id.BLETestModeTx);
        mRBtnRx = (RadioButton) findViewById(R.id.BLETestModeRx);

        mRBtnHopping = (RadioButton) findViewById(R.id.BLEHopping);
        mRBtnSingle = (RadioButton) findViewById(R.id.BLESingle);

        mContinune = (CheckBox) findViewById(R.id.BLEContiniousTx);

        mChannelSpn = (Spinner) findViewById(R.id.BLEChannelSpinner);
        mPatternSpn = (Spinner) findViewById(R.id.BLEPatternSpinner);

        mBtnStart.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);

        mRBtnTx.setChecked(true);
        mTxTest = true;

        mRBtnTx.setOnClickListener(this);
        mRBtnRx.setOnClickListener(this);

        mRBtnSingle.setChecked(true);
//        mHopping = false;
        mRBtnSingle.setOnClickListener(this);
        mRBtnHopping.setOnClickListener(this);

        // Fill "channel  spinner" content and action handler set
        ArrayAdapter<String> mSpnChannelAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item);
        mSpnChannelAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        for (int i = 0; i < CHANNEL_NUM; i++) {
            // mSpnChannelAdapter.add("CH " + i);
            mSpnChannelAdapter
                    .add(getString(R.string.BT_ble_test_channnel) + i);
        }

        mChannelSpn.setAdapter(mSpnChannelAdapter);
        mChannelSpn.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                Xlog.i(TAG, "item id = " + arg2);
                mChannelValue = (byte) arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Xlog.i(TAG, "DO Nothing ");
            }
        });

        // Fill "pattern  " content and action handler set
        ArrayAdapter<String[]> mSpnPatternAdapter = new ArrayAdapter<String[]>(
                this, android.R.layout.simple_spinner_item);
        mSpnPatternAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpnPatternAdapter.add(getResources().getStringArray(
                R.array.bt_ble_test_pattern));

        // mSpnPattern.setAdapter(mSpnPatternAdapter);
        mPatternSpn.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                Xlog.i(TAG, "item id = " + arg2);
                mPatternValue = (byte) arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Xlog.i(TAG, "DO Nothing ");
            }
        });

        setViewState(false);

        HandlerThread workThread = new HandlerThread(TAG);
        workThread.start();

        Looper looper = workThread.getLooper();
        mWorkHandler = new WorkHandler(looper);
//        mWorkHandler = new WorkHandler(mWorkThread.getLooper());

    }

    // UI thread's handler
    private Handler mUiHandler = new Handler() {
        // @Override
        public void handleMessage(Message msg) {
            Xlog.i(TAG, "-->main Handler - handleMessage");
            mResultText.setText(mResultStr);
            switch (msg.what) {
            case TEST_SUCCESS:
                mTestStared = true;
                break;
            case TEST_FAILED:
                // here we can give some notification
                mTestStared = false;
                setViewState(false);
                break;
            case STOP_FINISH:
                mTestStared = false;
                setViewState(false);
                removeDialog(CHECK_STOP);
                break;
            default:
                break;
            }
        }
    };
    /**
     * Check bt state when activity is in foreground
     * 
     */
    @Override
    protected void onResume() {
//        Xlog.v(TAG, "-->onResume");
        super.onResume();
        if (mBtAdapter == null) {
            mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        if (mBtAdapter == null) {
            showDialog(CHECK_BT_DEVEICE);
        } else {
            if (mBtAdapter.getState() != BluetoothAdapter.STATE_OFF) {
                showDialog(CHECK_BT_STATE);
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Xlog.d(TAG, "-->onCreateDialog");
        if (id == CHECK_STOP) {
            ProgressDialog dialog = new ProgressDialog(this);

            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);

            return dialog;
        } else if (id == CHECK_BT_STATE) {
            AlertDialog dialog = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.Error)
                .setMessage(getString(R.string.BT_turn_bt_off)) // put in strings.xml
                .setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int which) {
                            finish();
                        }
                    }).create();
            return dialog;

        } else if (id == CHECK_BT_DEVEICE) {
            // "Error").setMessage("Can't find any bluetooth device") // put in
            // strings.xml
            AlertDialog dialog = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.Error)
                .setMessage(getString(R.string.BT_no_dev)) // put in strings.xml
                .setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int which) {
                            finish();
                        }
                    }).create();
            return dialog;
        }

        return null;
    }

    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "-->onDestroy");
        mWorkHandler.sendEmptyMessage(ACTIVITY_EXIT);
        // mWorkThread = null;
        super.onDestroy();

    }

    /**
     * If pressing "Start" button Tx: 01 1E 20 03 XX 25 YY //HCI LE Transmitter
     * Test CMD XX is based on Channel selection YY is based on Pattern
     * selection Rx: 04 0E 04 01 1E 20 00 //HCI Command Complete Event
     */
    private boolean handleTxTestStart() {
        Xlog.v(TAG, "-->handleTxTestStart");

        int cmdLen = 7;
        char[] cmd = new char[cmdLen];
        char[] response = null;
        int i = 0;
        cmd[0] = 0x01;
        cmd[1] = 0x1E;
        cmd[2] = 0x20;
        cmd[3] = 0x03;
        cmd[4] = (char) mChannelValue;
        cmd[5] = 0x25;
        cmd[6] = (char) mPatternValue;

        response = mBtTest.hciCommandRun(cmd, cmdLen);
        if (response != null) {
            String s = null;
            for (i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.v(TAG, s);
            }
        }
//        else {
//            Xlog.i(TAG, "HCICommandRun failed");
//            return false;
//        }
        // here we need to judge whether this operation is succeeded or not
        response = null;
        return true;
    }

    /**
     * If pressing "Stop" button Tx: 01 1F 20 00 //HCI LE Test End CMD For Rx,
     * we have two cases of HCI Command Complete Event Case A) Rx: 04 0E 0A 01
     * 1F 20 00 00 00 00 00 00 00 Case B) Rx: 04 0E 06 01 1F 20 00 00 00
     */
    private void handleTxTestStop() {

        Xlog.v(TAG, "-->handleTxTestStop");
        int cmdLen = 4;
        char[] cmd = new char[cmdLen];
        char[] response = null;
        int i = 0;

        cmd[0] = 0x01;
        cmd[1] = 0x1F;
        cmd[2] = 0x20;
        cmd[3] = 0x00;

        response = mBtTest.hciCommandRun(cmd, cmdLen);
        if (response != null) {
            String s = null;
            for (i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.v(TAG, s);
            }
        }
//        else {
//            Xlog.i(TAG, "HCICommandRun failed");
//            return;
//        }
        response = null;
    }

    /* BLE test mode test */

    /**
     * If pressing "Start" button Tx: 01 1D 20 01 ZZ //HCI LE Receiver Test CMD
     * ZZ is based on Channel selection Rx: 04 0E 04 01 1D 20 00 
     * //HCI Command Complete Event
     * 
     */
    private boolean handleRxTestStart() {
        Xlog.v(TAG, "-->handleRxTestStart");
        char[] cmd = new char[5];
        char[] response = null;
        cmd[0] = 0x01;
        cmd[1] = 0x1D;
        cmd[2] = 0x20;
        cmd[3] = 0x01;
        cmd[4] = (char) mChannelValue;
        response = mBtTest.hciCommandRun(cmd, cmd.length);
        if (response != null) {
            String s = null;
            for (int i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.v(TAG, s);
            }
        }
        return true;
    }

    /**
     * If pressing "Stop" button Tx: 01 1F 20 00 //HCI LE Test End CMD For Rx,
     * we have two cases of HCI Command Complete Event Case A) Rx: 04 0E 0A 01
     * 1F 20 00 BB AA ?? ?? ?? ?? Case B) Rx: 04 0E 06 01 1F 20 00 BB AA ??
     * means do not care Packet Count = 0xAABB
     */
    private void handleRxTestStop() {
        Xlog.v(TAG, "-->handleRxTestStop");
        char[] cmd = {0x01, 0x1F, 0x20, 0x00};
        char[] response = null;
        response = mBtTest.hciCommandRun(cmd, cmd.length);
        if (response != null) {
            String s = null;
            for (int i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.v(TAG, s);
            }
            // Response format: 04 0e 0a/06 01 1f 20 00 BB AA 00 00...
            // packet count = 0xAABB
            mResultStr = String.format("***Packet Count: %d",
                    (long) response[8] * 256 + (long) response[7]);
        } 
    }

    /**
     * Do test after push "start" button
     * 
     * @return 0 success
     */
    private boolean handleStartBtnClick() {
        Xlog.v(TAG, "-->handleStartBtnClick");
        // BLEResult_String = "Response:";
        // judge if Rx or Tx test is selected
        /*
         * Each time before executing, HCI Reset CMD is needed to reset
         * Bluetooth Controller Tx: 01 03 0C 00 //HCI Reset CMD Rx: 04 0E 04 01
         * 03 0C 00 //HCI Command Complete Event
         */
        int cmdLen = 4;
        char[] cmd = new char[cmdLen];
        char[] response = null;
        int i = 0;
        cmd[0] = 0x01;
        cmd[1] = 0x03;
        cmd[2] = 0x0C;
        cmd[3] = 0x00;
        // HCI reset command
        response = mBtTest.hciCommandRun(cmd, cmdLen);
        if (response == null) {
            Xlog.i(TAG, "HCICommandRun failed");
            return false;
        } else {
            String s = null;
            for (i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.v(TAG, s);
            }
        }
        response = null;

        if (mTxTest) {
            return handleTxTestStart();
        } else {
            return handleRxTestStart();
        }
    }

    /**
     * run "HCI Reset" command.
     */
    private void runHCIResetCmd() {
        /*
         * If pressing "HCI Reset" button Tx: 01 03 0C 00 Rx: 04 0E 04 01 03 0C
         * 00 After pressing "HCI Reset" button, all state will also be reset
         */
        Xlog.i(TAG, "-->runHCIResetCmd");
        int cmdLen = 4;
        char[] cmd = new char[cmdLen];
        char[] response = null;
        int i = 0;

        cmd[0] = 0x01;
        cmd[1] = 0x03;
        cmd[2] = 0x0C;
        cmd[3] = 0x00;
        response = mBtTest.hciCommandRun(cmd, cmdLen);
        if (response == null) {
            Xlog.v(TAG, "HCICommandRun failed");
        } else {
            String s = null;
            for (i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.v(TAG, s);
            }
        }
        // here we need to judge whether this operation is succeeded or not
        response = null;
    }

    /**
     * stop test.
     */
    private void handleStopBtnClick() {
        Xlog.v(TAG, "-->handleStopBtnClick");
        if (mTxTest) {
            handleTxTestStop();
        } else {
            handleRxTestStop();
        }
        mTestStared = false;
    }

    /*
     * (non-Javadoc)
     * 
     * @see android.view.View.OnClickListener#onClick(android.view.View)
     */
    public void onClick(View v) {

        Xlog.v(TAG, "-->onClick");
        if (v.equals(mBtnStart)) {
            setViewState(true);
            mWorkHandler.sendEmptyMessage(TEST_START);
            /*
             * if(!handleStartBtnClick()) { setViewState(false); }
             */
        } else if (v.equals(mBtnStop)) {
            // Rx test button is clicked
            mBtnStop.setEnabled(false);
            showDialog(CHECK_STOP);
            mWorkHandler.sendEmptyMessage(TEST_STOP);
            /*
             * handleStopBtnClick(); setViewState(false);
             */

        } else if (v.equals(mRBtnRx)) {
            // Rx test button is clicked
            mTxTest = false;

        } else if (v.equals(mRBtnTx)) {
            // Rx test button is clicked
            mTxTest = true;

        } 
 /*       else if (v.equals(mRBtnHopping)) {
            // Hopping test button is clicked
          //  mHopping = true;

        } else if (v.equals(mRBtnSingle)) {
            // Single test button is clicked
           // mHopping = false;
        } else {
            Xlog.i(TAG, "no view matches current view");
        }
        */
    }

    /**
     * set view to state (true/false --> enable(stop btn pressed) /
     * disable(start btn pressed))
     * 
     * @param state
     */
    private void setViewState(boolean state) {

        mRBtnTx.setEnabled(!state);
        mRBtnRx.setEnabled(!state);
        mRBtnHopping.setEnabled(false);
        mRBtnSingle.setEnabled(!state);
        mContinune.setEnabled(!state);
        mChannelSpn.setEnabled(!state);
        mPatternSpn.setEnabled(!state);

        mBtnStart.setEnabled(!state);
        mBtnStop.setEnabled(state);
    }

    /**
     * Init BtTest -call init function of BtTest.
     * 
     * @return true if success.
     */
    private boolean initBtTestOjbect() {
        Xlog.v(TAG, "-->initBtTestOjbect");
        if (mIniting) {
            return false;
        }
        if (mBtInited) {
            return mBtInited;
        }
        if (mBtTest == null) {
            mBtTest = new BtTest();
        }
        if (mBtTest != null && !mBtInited) {
            mIniting = true;
            if (mBtTest.init() == RENTURN_SUCCESS) {
                runHCIResetCmd();
                mBtInited = true;                
            } else {
                mBtInited = false;
                Xlog.i(TAG, "mBT initialization failed");
            }
        }
        mIniting = false;
        return mBtInited;
    }

    /**
     * Clear BtTest object -call deInit function of BtTest.
     * 
     * @return true if success.
     */
    private boolean uninitBtTestOjbect() {
        Xlog.v(TAG, "-->uninitBtTestOjbect");
        if (mBtTest != null && mBtInited) {
            if (mTestStared) {
                // handleStopBtnClick();
                runHCIResetCmd();
            }
            if (mBtTest.unInit() != RENTURN_SUCCESS) {
                Xlog.i(TAG, "mBT un-initialization failed");
            }
        }
        mBtTest = null;
        mBtInited = false;
        mTestStared = false;
        return true;
    }

    /**
     * Deal with function request.
     * 
     * @author mtk54040
     * 
     */
    private final class WorkHandler extends Handler {

        /**
         * @param looper
         */
        private WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case TEST_START:
                if (initBtTestOjbect() && handleStartBtnClick()) {
                    // if (handleStartBtnClick()) {
                    mUiHandler.sendEmptyMessage(TEST_SUCCESS);
                    // }
                }
                if (!mBtInited) {
                    mUiHandler.sendEmptyMessage(TEST_FAILED);
                }
                break;
            case TEST_STOP:
                if (mBtInited) {
                    handleStopBtnClick();
                }
                mUiHandler.sendEmptyMessage(STOP_FINISH);
                break;
            case ACTIVITY_EXIT:
                uninitBtTestOjbect();
                break;
            default:
                break;
            }
        }
    }
}
