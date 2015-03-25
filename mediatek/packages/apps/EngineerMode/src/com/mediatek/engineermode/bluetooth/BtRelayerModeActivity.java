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
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.Spinner;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * Set uart info and test relayer mode.
 * 
 * @author mtk54040
 * 
 */
public class BtRelayerModeActivity extends Activity implements OnClickListener {
    private static final String TAG = "EM/BT/RelayerMode";
    // UI 
    private Spinner mBauSpinner;
    private Spinner mUartSpinner;
    private Button mStartBtn;
    
//    private ArrayAdapter<String[]> mBaudrateAdpt;
//    private ArrayAdapter<String[]> mUartAdapter;
    private BtTest mBtTest = null; // null

    // dialog ID and MSG ID
    private static final int START_TEST = 1; 
    private static final int END_TEST = 2; 
    
    private static final int RENTURN_SUCCESS = 0;
//    private static final int RENTURN_FAIL = -1;
    
    private static final int EXIT_SUCCESS = 10;
    private static final int PARA_INDEX = 0;
    
    private boolean mStartFlag = false;
    private int mBaudrate = 9600;
    private int mPortNumber = 3;
    

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {

//        Xlog.v(TAG, "-->onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bt_relayer_mode);
        // Init UI component
        mBauSpinner = (Spinner) findViewById(R.id.spinner1);
        mUartSpinner = (Spinner) findViewById(R.id.spinner2);
        mUartSpinner
                .setOnItemSelectedListener(new OnItemSelectedListener() {
                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        if(arg2 == 4) {  // select usb
                            mBauSpinner.setEnabled(false);
                        } else {
                            mBauSpinner.setEnabled(true);
                        }
                    }
                    public void onNothingSelected(AdapterView<?> arg0) {
                        Xlog.v(TAG, "onNothingSelected");
                    }
                });
        mStartBtn = (Button) findViewById(R.id.button1);
/*
        // Fill baudrate content
        ArrayAdapter<String[]> mBaudrateAdpt = new ArrayAdapter<String[]>(this,
                android.R.layout.simple_spinner_item);
        mBaudrateAdpt
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBaudrateAdpt
                .add(getResources().getStringArray(R.array.bt_baudrate));

        // Fill uart content
        ArrayAdapter<String[]> mUartAdapter = new ArrayAdapter<String[]>(this,
                android.R.layout.simple_spinner_item);
        mUartAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        mUartAdapter.add(getResources().getStringArray(R.array.bt_uart));
*/
        mStartBtn.setOnClickListener(this);

    }

    public void onClick(View v) {
        Xlog.v(TAG, "-->onClick");
        Xlog.v(TAG, "mStartFlag--" + mStartFlag);
        if (v.getId() == mStartBtn.getId()) {

            Xlog.i(TAG, "mBtRelayerModeSpinner.getSelectedItem()--"
                    + mBauSpinner.getSelectedItem());
            try {
                mBaudrate = Integer.parseInt(mBauSpinner
                        .getSelectedItem().toString().trim());
                Xlog.v(TAG, "mBaudrate--" + mBaudrate);

            } catch (NumberFormatException e) { // detail info
                Xlog.v(TAG, e.getMessage());
            }

            // Xlog.i(TAG, "mSerialPortSpinner()--"
            // + mSerialPortSpinner.getSelectedItem());
            // Xlog.i(TAG, "id--" + mSerialPortSpinner.getSelectedItemId());
            // mPortNumber = (int) mSerialPortSpinner.getSelectedItemId(); //
            // use method to convert int
            Long tmpLong = mUartSpinner.getSelectedItemId();
            mPortNumber = tmpLong.intValue();
            Xlog.i(TAG, "mPortNumber--" + mPortNumber);
            FunctionTask functionTask = new FunctionTask();
            
            // Disable button to avoid multiple click  
            mStartBtn.setEnabled(false);
            if (mStartFlag) {
//                mStartFlag = false; 
                functionTask.execute(END_TEST);
                mStartBtn.setText("Start");           
            } else {
//                mStartFlag = true; // add violate
                showDialog(START_TEST);
                functionTask.execute(START_TEST);
            }
        }

    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Xlog.v(TAG, "-->onCreateDialog");
        if (id == START_TEST) {
            ProgressDialog dialog = new ProgressDialog(
                    BtRelayerModeActivity.this);
            dialog.setMessage(getString(R.string.BT_relayer_start));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);

            return dialog;
        }
        return null;
    }

    /**
     * Deal with function request.
     * 
     * @author mtk54040
     * 
     */
    public class FunctionTask extends AsyncTask<Integer, Void, Integer> {

        @Override
        protected Integer doInBackground(Integer... params) {
            int result = RENTURN_SUCCESS;
            mBtTest = new BtTest();
            int paraValue = params[PARA_INDEX];
            if (paraValue == START_TEST) {
                result = mBtTest.relayerStart(mPortNumber, mBaudrate);
//                mStartFlag = true;
                Xlog.v(TAG, "-->relayerStart-" + mBaudrate + " uart "
                        + mPortNumber + "result 0 success,-1 fail: result= "
                        + result);
            } else if (paraValue == END_TEST) {
                mBtTest.relayerExit();
                mStartFlag = false;
                result = EXIT_SUCCESS;
            }
            return result;
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (result == RENTURN_SUCCESS) {       // START TEST OK
                mStartBtn.setText("END Test");
                mStartFlag = true;
            } 
            // remove dialog
            removeDialog(START_TEST);

            // Enable next click operation
            mStartBtn.setEnabled(true);
//            super.onPostExecute(result);
        }        
       
    }
}
