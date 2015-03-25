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
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.TextView;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * Show bluetooth chip infomation.
 * 
 * @author mtk54040
 * 
 */
public class BtChipInfoActivity extends Activity {
    private static final String TAG = "EM/BT/ChipInfo";

    // flags
    // private boolean mbIsDialogShowed = false; // flag style
    // private boolean mbIsBLEFeatureDetected = false;
    // dialog ID and MSG ID
    private static final int CHECK_BT_STATE = 1;
    private static final int GET_INFO = 2;
    // private static final int CHECK_BLE_FINISHED = 3;

    private static final int RESULT_SUCCESS = 0;

    private BluetoothAdapter mBtAdapter = null;
    // private Handler mUiHandler = null;

    private BtTest mBtTest = null;

    // string
    private String mChipId = "";
    private String mChipEco = "";
    private String mChipPatchId = "";
    private String mChipPatchLen = "";
    // TextView
    private TextView mChipIdTextView = null;
    private TextView mEcoVerTextView = null;
    private TextView mPatchSizeView = null;
    private TextView mPatchDateView = null;

    @Override
    public void onCreate(Bundle onSavedInstanceState) {
//        Xlog.v(TAG, "onCreate"); // event log
        super.onCreate(onSavedInstanceState);
        setContentView(R.layout.bt_chip_info);
        
        // Initialize UI component
        mChipIdTextView = (TextView) findViewById(R.id.chipId);
        mEcoVerTextView = (TextView) findViewById(R.id.ecoVersion);
        mPatchSizeView = (TextView) findViewById(R.id.patchSize);
        mPatchDateView = (TextView) findViewById(R.id.patchDate);

        if (mBtAdapter == null) {
            mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        // Get chip info when bt is close
        if (mBtAdapter.getState() == BluetoothAdapter.STATE_OFF) {
            showDialog(GET_INFO);
            FunctionTask functionTask = new FunctionTask();
            functionTask.execute();
        } else {
            showDialog(CHECK_BT_STATE);
        }
        
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == GET_INFO) {
            ProgressDialog dialog = new ProgressDialog(BtChipInfoActivity.this);
            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            // dialog.show();
            Xlog.i(TAG, "new ProgressDialog succeed");
            return dialog;
        } else if (id == CHECK_BT_STATE) {
            AlertDialog dialog = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.Error)
                .setMessage(R.string.BT_turn_bt_off) // put in strings.xml
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

    public class FunctionTask extends AsyncTask<Void, Void, Integer> {

        @Override
        protected Integer doInBackground(Void... params) {
            mBtTest = new BtTest();
            // Message msg = mUiHandler.obtainMessage(what);
            // check if BLE is supported or not
            if (mBtTest != null) {
                if (mBtTest.init() == RESULT_SUCCESS) { // get infomation from
                    // onCreate
                    // try {
                    mChipId = "";
                    String[] chipList = getResources().getStringArray(
                            R.array.bt_chip_id);
                    // Xlog.v(TAG, "chipList.length" + "___" + chipList.length);
                    // Xlog.v(TAG, "mBT.GetChipIdInt()" + "__" +
                    // mBT.GetChipIdInt());
                    int tmpId = mBtTest.getChipIdInt();
                    if (tmpId <= chipList.length) {
                        mChipId = chipList[tmpId];
                    }
                    Xlog.v(TAG, "chipId@" + "___" + mChipId);

                    mChipEco = "";
                    String[] ecoList = getResources().getStringArray(
                            R.array.bt_chip_eco);
                    // Xlog.v(TAG, "ecoList.length" + "___" + ecoList.length);
                    // Xlog.v(TAG, "mBT.GetChipEcoNum()" + "__" +
                    // mBT.GetChipEcoNum());
                    int ecoIndex = mBtTest.getChipEcoNum();
                    if (ecoIndex <= ecoList.length) {
                        mChipEco = ecoList[ecoIndex];
                    }

                    Xlog.v(TAG, "chipEco = " + mChipEco);
                    char[] patchIdArray = mBtTest.getPatchId();

                    mChipPatchId = new String(patchIdArray);
                    Xlog.v(TAG, "chipPatchId@" + mChipPatchId.length() + "___"
                            + mChipPatchId);
                    mChipPatchLen = "" + mBtTest.getPatchLen(); // remove ""
                    Xlog.v(TAG, "GetPatchLen=" + mChipPatchLen);

                    mBtTest.unInit();
                } else {
                    Xlog.i(TAG, "new BtTest failed");
                }
                // mbIsBLEFeatureDetected = true;
                // removeDialog(CHECK_BLE);
            }

            return RESULT_SUCCESS;
        }
        
        /**
         * Display the bt chip information
         * 
         */
        @Override
        protected void onPostExecute(Integer result) {
            mChipIdTextView.setText(mChipId);
            mEcoVerTextView.setText(mChipEco);
            mPatchSizeView.setText(mChipPatchLen);
            mPatchDateView.setText(mChipPatchId);

            // mbIsBLEFeatureDetected = true;
            removeDialog(GET_INFO);
//            super.onPostExecute(result);
        }

    }

}
