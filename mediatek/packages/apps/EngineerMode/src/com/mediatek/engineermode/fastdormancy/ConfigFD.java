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

package com.mediatek.engineermode.fastdormancy;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * 
 * 
 *
 * 
 */
public class ConfigFD extends Activity {

    private RadioGroup mRadioGroup;

    /** The data from modem */
    private String[] mReturnData = new String[2];
    private Phone mPhone = null;
    private int mFdValue = -1;

    private static final int EVENT_FD_QUERY = 0;
    private static final int EVENT_FD_SET = 1;
    private static final int SET_FAILED = 0;
    private static final int FD_ON = 0x3FFFFF;
    private static final int FD_OFF = 0x800000;
    private static final int FD_LEGACY_OFF = 0x400000;
    private static final String QUERY_FD[] = { "AT+EPCT?", "+EPCT" };
    private static final String FORE_CMD = "+EPCT:";

    private static final String TAG = "EM_FD";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fastdormancy_config);
        mRadioGroup = (RadioGroup) findViewById(R.id.fd_radio_group);
        Button setButton = (Button) findViewById(R.id.fd_set_button);
        setButton.setOnClickListener(new ButtonClickListener());
        init();
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateUI();
    }

    private void init() {
        mPhone = PhoneFactory.getDefaultPhone();
        mPhone.invokeOemRilRequestStrings(QUERY_FD, mResponseHander.obtainMessage(EVENT_FD_QUERY));
    }

    /**
     * Parse data from modem
     * 
     * @param data
     * @return the date after parse
     */
    private String[] parseData(String[] data) {
        Elog.i(TAG, "parseData() content[0]: " + data[0]);
        if (data[0] != null && data[0].startsWith(FORE_CMD)) {
            data = data[0].substring(FORE_CMD.length()).split(",");
            Elog.d(TAG, "parseData " + data[0]);
            Elog.d(TAG, "parseData " + data[1]);
            data[0].trim();
            data[1].trim();
            return data;
        }
        return data;
    }

    private void updateUI() {
        if (null == mReturnData || null == mReturnData[1]) {
            Elog.w(TAG, "returnData is null ");
            return;
        }

        boolean isOff = ((mFdValue & FD_OFF) == FD_OFF) ? true : false;
        boolean isLegacyOff = ((mFdValue & FD_LEGACY_OFF) == FD_LEGACY_OFF) ? true : false;
        Elog.i(TAG, "value = " + mFdValue);
        if (isOff) {
            mRadioGroup.check(R.id.fd_off_radio);
            Elog.i(TAG, "check off");
        } else if (isLegacyOff) {
            mRadioGroup.check(R.id.fd_legacy_off_radio);
            Elog.i(TAG, "check legacy off");
        } else {
            mRadioGroup.check(R.id.fd_on_radio);
            Elog.i(TAG, "check on");
        }
    }

    public class ButtonClickListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            if (v.getId() == R.id.fd_set_button) {
                int checkedId = mRadioGroup.getCheckedRadioButtonId();
                String cmdStr[] = new String[2];
                if (checkedId == R.id.fd_on_radio) {
                    if (null != mReturnData && null != mReturnData[0] && mFdValue != -1) {
                        mFdValue = mFdValue & FD_ON;
                        Elog.i(TAG, "To Modem :" + mFdValue);
                        cmdStr[0] = "AT+EPCT=" + mReturnData[0] + "," + mFdValue;
                        cmdStr[1] = "";
                        mPhone.invokeOemRilRequestStrings(cmdStr, mResponseHander.obtainMessage(EVENT_FD_SET));
                        Elog.i(TAG, "invoke cmdStr :" + cmdStr[0]);
                    } else {
                        Toast.makeText(ConfigFD.this, "Get FD data fail!", Toast.LENGTH_SHORT).show();
                        Elog.w(TAG, "returnData is null");
                        finish();
                    }
                } else if (checkedId == R.id.fd_off_radio) {
                    if (null != mReturnData && null != mReturnData[0] && mFdValue != -1) {
                        mFdValue = (mFdValue & FD_ON) | FD_OFF;
                        Elog.i(TAG, "To Modem :" + mFdValue);
                        cmdStr[0] = "AT+EPCT=" + mReturnData[0] + "," + mFdValue;
                        cmdStr[1] = "";
                        mPhone.invokeOemRilRequestStrings(cmdStr, mResponseHander.obtainMessage(EVENT_FD_SET));
                        Elog.i(TAG, "invoke cmdStr :" + cmdStr[0]);
                    } else {
                        Toast.makeText(ConfigFD.this, "Get FD data fail!", Toast.LENGTH_SHORT).show();
                        Elog.w(TAG, "returnData is null");
                        finish();
                    }
                } else if (checkedId == R.id.fd_legacy_off_radio) {
                    if (null != mReturnData && null != mReturnData[0] && mFdValue != -1) {
                        mFdValue = (mFdValue & FD_ON) | FD_LEGACY_OFF;
                        Elog.i(TAG, "To Modem :" + mFdValue);
                        cmdStr[0] = "AT+EPCT=" + mReturnData[0] + "," + mFdValue;
                        cmdStr[1] = "";
                        mPhone.invokeOemRilRequestStrings(cmdStr, mResponseHander.obtainMessage(EVENT_FD_SET));
                        Elog.i(TAG, "invoke cmdStr :" + cmdStr[0]);
                    } else {
                        Toast.makeText(ConfigFD.this, "Get FD data fail!", Toast.LENGTH_SHORT).show();
                        Elog.w(TAG, "returnData is null");
                        finish();
                    }
                }
            }
        }
    }

    private Handler mResponseHander = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "Receive msg from modem");
            AsyncResult ar;
            switch (msg.what) {
            case EVENT_FD_QUERY:
                Elog.i(TAG, "Receive EVENT_FD_QUERY_SIM1:");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    mReturnData = (String[]) ar.result;
                    mReturnData = parseData(mReturnData);
                    if (null != mReturnData && mReturnData.length > 0) {
                        for (int i = 0; i < mReturnData.length; i++) {
                            Elog.i(TAG, "mReturnData[" + i + "]: " + mReturnData[i] + "\n");
                        }
                        if (mReturnData.length != 0 && null != mReturnData[0]) {
                            if (mReturnData.length == 1) {
                                mFdValue = 0;
                            } else {
                                try {
                                    mFdValue = Integer.parseInt(mReturnData[1]);
                                } catch (NumberFormatException e) {
                                    Elog.w(TAG, "Invalid mReturnData format: " + mReturnData[1]);
                                    mFdValue = 0;
                                }
                            }
                            updateUI();
                        }
                    } else {
                        Elog.i(TAG, "Received data is null");
                    }
                } else {
                    Elog.i(TAG, "Receive EVENT_FD_QUERY: exception" + ar.exception);
                }
                break;
            case EVENT_FD_SET:
                Elog.i(TAG, "Receive EVENT_FD_SET:");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ConfigFD.this, "success!", Toast.LENGTH_SHORT).show();
                } else {
                    showDialog(SET_FAILED);
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        switch (id) {
        case SET_FAILED:
            builder.setTitle("SCRI/FD Set");
            builder.setMessage("SCRI/FD Set failed.");
            builder.setPositiveButton("OK", null);
            return builder.create();
        default:
            return null;
        }
    }
}
