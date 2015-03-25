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
import android.text.Html;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.io.IOException;

/**
 * Do BT test mode.
 * 
 * @author mtk54040
 * 
 */
public class TestModeActivity extends Activity implements
        DialogInterface.OnCancelListener {
    private BluetoothAdapter mAdapter;

    private static final String TAG = "TestMode";
    
    private static final String RUN_SU = "su";
    // private static final int HANDLER_TEST_FAILED = 0;

    private static final int CHECK_BT_STATE = 1;
    private static final int BLUETOOTH_INIT = 2;
    private static final int DIALOG_BT_STOP = 3;
//    private static final int CHECK_BT_DEVEICE = 3;

    private CheckBox mChecked;
    private EditText mTestModeEdit;

    private BtTest mBtTest;
    private static final int BT_TEST_1 = 1;
    private static final int BT_TEST_2 = 2;
    private static final int RETURN_FAIL = -1;
    private static final int DEFAULT_INT = 7;
    private static final String DEFAULT_STR = "7";

    private static final int BT_TEST_SUCCESS = 5;
    private static final int BT_TEST_FAIL = 6;
    private static final int BT_TEST_STOP_SUCCESS = 7;

    private static final int OP_BT_TEST_1 = 11;
    private static final int OP_DO_TEST_2 = 12;
    private static final int OP_DO_TEST_STOP = 13;

//    private HandlerThread mWorkThread = null;
    private WorkHandler mWorkHandler = null;
    private HandlerThread mWorkThread = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.test_mode);

        // CharSequence str1 = getString(R.string.strTestMode);
        TextView tv = (TextView) findViewById(R.id.TestModetxv);
        mTestModeEdit = (EditText) findViewById(R.id.BTTestMode_edit);

        // tv.setText(Html.fromHtml(str1.toString()));
        tv.setText(Html.fromHtml(getString(R.string.strTestMode)));

        mChecked = (CheckBox) findViewById(R.id.TestModeCb);
        mChecked.setOnCheckedChangeListener(mCheckedListener);

        if (mAdapter == null) {
            mAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        if (mAdapter.getState() == BluetoothAdapter.STATE_OFF) {
            mWorkThread = new HandlerThread(TAG);
            mWorkThread.start();

            Looper looper = mWorkThread.getLooper();
            mWorkHandler = new WorkHandler(looper);
//            mWorkHandler = new WorkHandler(mWorkThread.getLooper());
        } else {
            showDialog(CHECK_BT_STATE);
        }
    }

    private final CheckBox.OnCheckedChangeListener mCheckedListener = new CheckBox.OnCheckedChangeListener() {
        public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
            boolean ischecked = mChecked.isChecked();
            // is checked
            if (ischecked) {
                showDialog(BLUETOOTH_INIT);
                // String val = mTestModeEdit.getEditableText().toString();
                String val = mTestModeEdit.getText().toString();
                if (val == null || val.length() < 1) {
                    // mTestModeEdit.setText("7");
                    // val = "7";
                    mTestModeEdit.setText(DEFAULT_STR);
                    val = DEFAULT_STR;
                }
                // int v = 7;
                int v = DEFAULT_INT;
                try {
                    v = Integer.valueOf(val);
                } catch (NumberFormatException e) {
                    Xlog.i(TAG, e.getMessage());
                }

                if (v > DEFAULT_INT) {
                    mTestModeEdit.setText(DEFAULT_STR);
                }
                mWorkHandler.sendEmptyMessage(OP_BT_TEST_1);

            } else {
                showDialog(BLUETOOTH_INIT);
                mWorkHandler.sendEmptyMessage(OP_DO_TEST_2);
            }
        }
    };
    
    private final Handler mUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
//            if (msg.what == BT_TEST_SUCCESS) {
//
//            } else if (msg.what == BT_TEST_FAIL) {
            if (msg.what == BT_TEST_FAIL) {
                Toast.makeText(getApplicationContext(),
                        R.string.BT_data_fail,
                        Toast.LENGTH_SHORT).show();
            }
            if(msg.what == BT_TEST_STOP_SUCCESS) {
                finish();
            }else {
                removeDialog(BLUETOOTH_INIT);
                mChecked.setEnabled(true);
            }
        }
    };

    /**
     *  implemented for DialogInterface.OnCancelListener
     * 
     * 
     */
    public void onCancel(DialogInterface dialog) {
        // request that the service stop the query with this callback object.
        Xlog.v(TAG, "onCancel");
        finish();
    }
    
    @Override
    public void onBackPressed() {
        Xlog.v(TAG, "-->onBackPressed ");

        if (mWorkHandler != null && mBtTest != null) {
            showDialog(DIALOG_BT_STOP);
            mWorkHandler.sendEmptyMessage(OP_DO_TEST_STOP);
        }else {
            super.onBackPressed();
        }
    }
    
    @Override
    public void onDestroy() {
        Xlog.v(TAG, "-->onDestroy");
        // super.onDestroy();
        if(mWorkThread != null) {
            mWorkThread.quit();
        }
        // mBtTest.UnInit();
        super.onDestroy();
    }



    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog returnDialog = null;
        if (id == BLUETOOTH_INIT) {
            ProgressDialog dialog = new ProgressDialog(TestModeActivity.this);
            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            dialog.setOnCancelListener(this);
            returnDialog = dialog;
//            return dialog;
        } else if (id == CHECK_BT_STATE) {
            AlertDialog dialog = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.Error)
                .setMessage(R.string.BT_turn_bt_off) 
                .setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int which) {
                            finish();
                        }
                    }).create();
            returnDialog = dialog;
        } else if (id == DIALOG_BT_STOP) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.BT_deinit));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            returnDialog = dialog;
        }

        return returnDialog;
    }

    private final class WorkHandler extends Handler {
        private WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {

            if (msg.what == OP_BT_TEST_1) {
                final Runtime rt = Runtime.getRuntime();
                try {
                    rt.exec(RUN_SU);
                    Xlog.v(TAG, "excute su command.");
                } catch (IOException e) {
                    Xlog.v(TAG, e.getMessage());
                }

                mBtTest = new BtTest();
                // String val = mTestModeEdit.getEditableText().toString();
                String val = mTestModeEdit.getText().toString();
                int tmpValue = 0;
                try {
                    tmpValue = Integer.valueOf(val);
                } catch (NumberFormatException e) {
                    Xlog.i(TAG, e.getMessage());
                }

                mBtTest.setPower(tmpValue);
                Xlog.i(TAG, "power set " + val);
                if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_1)) {
                    // if (-1 == mBtTest.doBtTest(1)) {
                    Xlog.v(TAG, "transmit data failed.");
                    // removeDialog(DIALOG_BLUETOOTH_INIT);
                    mUiHandler.sendEmptyMessage(BT_TEST_FAIL);
                } else {
                    mUiHandler.sendEmptyMessage(BT_TEST_SUCCESS);
                }

                Xlog.i(TAG, "pollingStart");
                mBtTest.pollingStart();

            } else if (msg.what == OP_DO_TEST_2 || msg.what == OP_DO_TEST_STOP) {
                final Runtime rt = Runtime.getRuntime();
                try {
                    rt.exec(RUN_SU);
                    Xlog.i(TAG, "excute su command.");
                } catch (IOException e) {
                    Xlog.v(TAG, e.getMessage());
                }

                if (mBtTest != null) {
                    Xlog.i(TAG, "pollingStop");
                    mBtTest.pollingStop();
                    if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_2)) {
                        // if (-1 == mBtTest.doBtTest(2)) {
                        Xlog.i(TAG, "transmit data failed 1.");
                    }
                    mBtTest = null;
                } 
                if(msg.what == OP_DO_TEST_2) {
                    mUiHandler.sendEmptyMessage(BT_TEST_SUCCESS);
                } else if(msg.what == OP_DO_TEST_STOP) {
                    mUiHandler.sendEmptyMessage(BT_TEST_STOP_SUCCESS); 
                }
            }
        }
    }
}
