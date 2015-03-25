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

package com.mediatek.engineermode.cpustress;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.cpustress.CpuStressTestService.ICpuStressTestComplete;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.IllegalFormatException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ClockSwitch extends Activity implements OnCheckedChangeListener,
        OnClickListener, ServiceConnection, ICpuStressTestComplete {

    private static final String TAG = "EM/CpuStress_ClockSwitch";
    private static final String CPU_SS_MODE = "/proc/cpu_ss/cpu_ss_mode";
    private static final String CPU_SS_PERIOD = "/proc/cpu_ss/cpu_ss_period";
    private static final String CPU_SS_PERIOD_MODE = "/proc/cpu_ss/cpu_ss_period_mode";
    private static final String CPU_SS_DEBUG_MODE = "/proc/cpu_ss/cpu_ss_debug_mode";
    private static final String CPU_MAX_FREQ_CMD = "cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq";
    private static final String COMMAND_GET_ALL_PLL = "cat /proc/clkmgr/pll_fsel";
    private static final String REGEX_PLL_GROUP = "\\[[\\s\\S]*?\\]";
    private static final String[] FILES = new String[] { CPU_SS_MODE,
            CPU_SS_PERIOD, CPU_SS_PERIOD_MODE, CPU_SS_DEBUG_MODE };
    private static final String SET_COMMAND_FORMAT = "echo %1$s > %2$s";
    private static final String GET_COMMAND_FORMAT = "cat %1$s";
    private static final String CPU_MAX_FREQ_FORMAT = "%1.1f";
    private static final String CPU_MAX_FREQ_ORI = "1";
    private static final int INDEX_SET_MODE = 0;
    private static final int INDEX_SET_PERIOD = 1;
    private static final int INDEX_SET_PERIOD_MODE = 2;
    private static final int INDEX_SET_DEBUG_MODE = 3;
    private static final int INDEX_QUERY_MODE = 10;
    private static final int INDEX_QUERY_PERIOD = 11;
    private static final int INDEX_QUERY_PERIOD_MODE = 12;
    private static final int INDEX_QUERY_DEBUG_MODE = 13;
    private static final int INDEX_UPDATE_PLL = 18;
    private static final int INDEX_SET_QUERY_DELTA = 10;
    private static final String INDEX_SET_MODE_VALUE_0 = "0";
    private static final String INDEX_SET_MODE_VALUE_1 = "1";
    private static final String INDEX_SET_PERIOD_MODE_VALUE_E = "enable";
    private static final String INDEX_SET_PERIOD_MODE_VALUE_D = "disable";
    private static final String INDEX_SET_DEBUG_MODE_VALUE_E = "enable";
    private static final String INDEX_SET_DEBUG_MODE_VALUE_D = "disable";
    private static final int DIALOG_WAIT = 1;
    private static final int QUERY_ALL_RECORD_MASK = 0xF;
    private static final String DEFAULT_SECOND = "1";
    private static final String DEFAULT_NSECOND = "0";
    private static final double MHZ = 1000000.0;
    private static final String PLL_NAME_ARM = "ARMPLL";
    private static final String PLL_VALUE_PATTERN = "^[0-9a-fA-F]{1,16}$";
    private static final String PLL_VALUE_PRE = "-1";
    private int mQueryRecordMask = 0x0;
    private CpuStressTestService mBoundService = null;
    private CheckBox mCbDebugMsgEnable = null;
    private TextView mTvDebugMsgEnable = null;
    private Button mBtnStart = null;
    private Button mBtnSwitchM = null;
    private Button mBtnSwitchG = null;
    private EditText mEtSecond = null;
    private EditText mEtNSecond = null;
    private EditText mEtArmPllValue = null;
    private Button mBtnSet = null;
    private int mArmPllId = -1;

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            Xlog.v(TAG, "mHandler receive message: " + msg.what);
            Xlog.v(TAG, "msg.what: " + msg.what + " msg.obj: "
                    + msg.obj.toString());
            switch (msg.what) {
            case INDEX_SET_MODE:
            case INDEX_SET_PERIOD:
            case INDEX_SET_PERIOD_MODE:
            case INDEX_SET_DEBUG_MODE:
                Toast.makeText(
                        ClockSwitch.this,
                        FILES[msg.what % INDEX_SET_QUERY_DELTA] + ":"
                                + msg.obj.toString(), Toast.LENGTH_SHORT)
                        .show();
                break;
            case INDEX_QUERY_MODE:
                mQueryRecordMask |= 1 << INDEX_QUERY_MODE
                        % INDEX_SET_QUERY_DELTA;
                // updateSwitchView(msg.obj.toString().trim().equals(
                // INDEX_SET_MODE_VALUE_0));
                break;
            case INDEX_QUERY_PERIOD:
                mQueryRecordMask |= 1 << INDEX_QUERY_PERIOD
                        % INDEX_SET_QUERY_DELTA;
                updatePeriodView(msg.obj.toString().trim());
                break;
            case INDEX_QUERY_PERIOD_MODE:
                mQueryRecordMask |= 1 << INDEX_QUERY_PERIOD_MODE
                        % INDEX_SET_QUERY_DELTA;
                updateAutoTestView(msg.obj.toString().trim().equals(
                        INDEX_SET_PERIOD_MODE_VALUE_E));
                break;
            case INDEX_QUERY_DEBUG_MODE:
                mQueryRecordMask |= 1 << INDEX_QUERY_DEBUG_MODE
                        % INDEX_SET_QUERY_DELTA;
                mCbDebugMsgEnable.setEnabled(false);
                mCbDebugMsgEnable.setChecked(msg.obj.toString().trim().equals(
                        INDEX_SET_DEBUG_MODE_VALUE_E));
                mCbDebugMsgEnable.setEnabled(true);
                break;
            case INDEX_UPDATE_PLL:
                mEtArmPllValue.setText(msg.obj.toString());
                mBtnSet.setEnabled(msg.arg1 == 1);
                break;
            default:
                break;
            }
            if (QUERY_ALL_RECORD_MASK == mQueryRecordMask) {
                removeDialog(DIALOG_WAIT);
            }
            super.handleMessage(msg);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_cpustress_clockswitch);
        mCbDebugMsgEnable = (CheckBox) findViewById(R.id.clockswitch_debug_enable);
        mTvDebugMsgEnable = (TextView) findViewById(R.id.clockswitch_debug_view);
        mBtnStart = (Button) findViewById(R.id.clockswitch_btn_start);
        mBtnSwitchM = (Button) findViewById(R.id.clockswitch_btn_switch26);
        mBtnSwitchG = (Button) findViewById(R.id.clockswitch_btn_switch1g);
        mEtSecond = (EditText) findViewById(R.id.clockswitch_timeout_s);
        mEtNSecond = (EditText) findViewById(R.id.clockswitch_timeout_ns);
        mEtArmPllValue = (EditText) findViewById(R.id.clockswitch_edt_speed);
        mBtnSet = (Button) findViewById(R.id.clockswitch_btn_set);
        mCbDebugMsgEnable.setOnCheckedChangeListener(this);
        mTvDebugMsgEnable.setOnClickListener(this);
        mBtnStart.setOnClickListener(this);
        mBtnSwitchM.setOnClickListener(this);
        mBtnSwitchG.setOnClickListener(this);
        mBtnSet.setOnClickListener(this);
        if (ChipSupport.MTK_6589_SUPPORT <= ChipSupport.getChip()) {
            try {
                if (ShellExe.RESULT_SUCCESS == ShellExe
                        .execCommand(CPU_MAX_FREQ_CMD)) {
                    int freq = Integer.parseInt(ShellExe.getOutput().trim());
                    String value = String.format(CPU_MAX_FREQ_FORMAT, freq
                            / MHZ);
                    mBtnSwitchG.setText(mBtnSwitchG.getText().toString()
                            .replaceAll(CPU_MAX_FREQ_ORI, value));
                }
            } catch (IOException e) {
                Xlog.w(TAG, "Get max freq IOException: " + e.getMessage());
            }
        } else {
            ((TextView) findViewById(R.id.clockswitch_tv_speed))
                    .setVisibility(View.GONE);
            mEtArmPllValue.setVisibility(View.GONE);
            mBtnSet.setVisibility(View.GONE);
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        showDialog(DIALOG_WAIT);
        mQueryRecordMask = 0;
        new Thread(new Runnable() {

            public void run() {
                updateArmPll();
                getCurrentStatus(INDEX_QUERY_MODE);
                getCurrentStatus(INDEX_QUERY_PERIOD);
                getCurrentStatus(INDEX_QUERY_PERIOD_MODE);
                getCurrentStatus(INDEX_QUERY_DEBUG_MODE);
            }
        }).start();
        bindService(new Intent(this, CpuStressTestService.class), this,
                Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        // mBoundService.testObject = null;
        unbindService(this);
        super.onStop();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        ProgressDialog dialog = null;
        if (id == DIALOG_WAIT) {
            dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.hqa_cpustress_dialog_waiting_title);
            dialog.setMessage(getString(R.string.hqa_cpustress_dialog_waiting_message));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
        } else {
            dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.hqa_cpustress_dialog_error_title);
            dialog.setMessage(getString(R.string.hqa_cpustress_dialog_error_message));
        }
        return dialog;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    /**
     * Update clock switch period data to UI
     * 
     * @param period
     *            Period string
     */
    private void updatePeriodView(String period) {
        Xlog.v(TAG, "Enter updatePeriodView: " + period);
        int begin;
        int end;
        begin = period.indexOf('(');
        if (-1 != begin) {
            mEtSecond.setText(period.substring(0, begin).trim());
        }
        begin = period.indexOf(')');
        end = period.lastIndexOf('(');
        if (-1 != begin && -1 != end && begin < end) {
            mEtNSecond.setText(period.substring(begin + 1, end).trim());
        }
        if (null != mBoundService) {
            if (!mBoundService.isClockSwitchRun()) {
                mEtSecond.setText(DEFAULT_SECOND);
                mEtNSecond.setText(DEFAULT_NSECOND);
            }
        }
        mEtSecond.setSelection(mEtSecond.getText().length());
        mEtNSecond.setSelection(mEtNSecond.getText().length());
    }

    /**
     * Update auto test UI
     * 
     * @param bRun
     *            Run auto test or not
     */
    private void updateAutoTestView(boolean bRun) {
        if (bRun) {
            mBtnStart.setText(R.string.hqa_cpustress_clockswitch_stop);
            mEtSecond.setEnabled(false);
            mEtNSecond.setEnabled(false);
            mBtnSwitchM.setEnabled(false);
            mBtnSwitchG.setEnabled(false);
        } else {
            mBtnStart.setText(R.string.hqa_cpustress_clockswitch_start);
            mEtSecond.setEnabled(true);
            mEtNSecond.setEnabled(true);
            mBtnSwitchM.setEnabled(true);
            mBtnSwitchG.setEnabled(true);
        }
    }

    /**
     * Update clock switch buttons status
     * 
     * @param bM
     *            True when set to MHz, false when set to GHz
     */
    private void updateSwitchView(boolean bM) {
        if (bM) {
            mBtnSwitchM.setEnabled(false);
            mBtnSwitchG.setEnabled(true);
        } else {
            mBtnSwitchM.setEnabled(true);
            mBtnSwitchG.setEnabled(false);
        }
    }

    /**
     * Handle set event
     * 
     * @param value
     *            Value to set
     * @param index
     *            Set index
     */
    private void handleEvent(final String value, final int index) {
        Xlog.v(TAG, "handleEvent: " + value + " " + index);
        new Thread(new Runnable() {

            public void run() {
                setCommand(value, index);
                getResponse(index);
            }
        }).start();
    }

    /**
     * Get response after set status
     * 
     * @param index
     *            Set index
     */
    private void getResponse(int index) {
        switch (index) {
        case INDEX_SET_MODE:
        case INDEX_SET_PERIOD:
        case INDEX_SET_PERIOD_MODE:
        case INDEX_SET_DEBUG_MODE:
            getStatus(index);
            break;
        default:
            Xlog.d(TAG, "getResponse: index is error, " + index);
            break;
        }
    }

    /**
     * Set command to clock switch file
     * 
     * @param value
     *            Value to set
     * @param index
     *            Set index
     */
    private void setCommand(String value, int index) {
        String command = null;
        boolean bError = false;
        try {
            command = String.format(SET_COMMAND_FORMAT, value, FILES[index
                    % INDEX_SET_QUERY_DELTA]);
            Xlog.v(TAG, "setCommand: " + command);
        } catch (NullPointerException e) {
            bError = true;
            Xlog.d(TAG, "Command format NullPointerException: "
                    + e.getMessage());
        } catch (IllegalFormatException e) {
            bError = true;
            Xlog.d(TAG, "Command format IllegalFormatException: "
                    + e.getMessage());
        }
        if (bError) {
            Toast.makeText(this,
                    R.string.hqa_cpustress_clockswitch_command_error,
                    Toast.LENGTH_LONG).show();
        } else {
            synchronized (this) {
                try {
                    ShellExe.execCommand(command);
                } catch (IOException e) {
                    Xlog.d(TAG, "Exec command IOException: " + e.getMessage());
                }
            }
        }
    }

    /**
     * Get current status
     * 
     * @param index
     *            Get index
     */
    private void getCurrentStatus(int index) {
        switch (index) {
        case INDEX_QUERY_MODE:
        case INDEX_QUERY_PERIOD:
        case INDEX_QUERY_PERIOD_MODE:
        case INDEX_QUERY_DEBUG_MODE:
            getStatus(index);
            break;
        default:
            Xlog.d(TAG, "getCurrentStatus: index is error, " + index);
            break;
        }
    }

    /**
     * Get status
     * 
     * @param index
     *            Get index
     */
    private void getStatus(int index) {
        Xlog.v(TAG, "Enter getStatus: " + index);
        boolean bError = false;
        String command = null;
        try {
            command = String.format(GET_COMMAND_FORMAT, FILES[index
                    % INDEX_SET_QUERY_DELTA]);
            Xlog.v(TAG, "getCommand: " + command);
        } catch (NullPointerException e) {
            bError = true;
            Xlog.d(TAG, "Command format NullPointerException: "
                    + e.getMessage());
        } catch (IllegalFormatException e) {
            bError = true;
            Xlog.d(TAG, "Command format IllegalFormatException: "
                    + e.getMessage());
        }
        if (bError) {
            Toast.makeText(this, R.string.hqa_cpustress_clockswitch_command_error,
                    Toast.LENGTH_LONG).show();
        } else {
            synchronized (this) {
                try {
                    ShellExe.execCommand(command);
                    Message m = mHandler.obtainMessage(index);
                    m.obj = ShellExe.getOutput();
                    mHandler.sendMessage(m);
                } catch (IOException e) {
                    Xlog.d(TAG, "Exec command IOException: " + e.getMessage());
                }
            }
        }
    }

    private void updateArmPll() {
        String pllInfo = null;
        boolean bSuccess = false;
        String value = "";
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe
                    .execCommand(COMMAND_GET_ALL_PLL)) {
                pllInfo = ShellExe.getOutput();
                Pattern pat = Pattern.compile(REGEX_PLL_GROUP);
                Matcher matcher = pat.matcher(pllInfo);
                int index = 0;
                String name = null;
                String content = null;
                while (matcher.find()) {
                    content = matcher.group();
                    if (null == content) {
                        continue;
                    }
                    content = content.substring(1, content.length() - 1).trim();
                    if (0 == index) {
                        mArmPllId = Integer.parseInt(content);
                        index++;
                    } else if (1 == index) {
                        name = content;
                        index++;
                    } else {
                        value = content;
                        index = 0;
                        if (name.contains(PLL_NAME_ARM)) {
                            Xlog.v(TAG, "find ARMPLL: id " + mArmPllId
                                    + " value " + value);
                            break;
                        }
                    }
                }
                if (null != value) {
                    bSuccess = true;
                    if (value.contains(PLL_VALUE_PRE)) {
                        value = PLL_VALUE_PRE;
                    } else {
                        value = value.substring(2, value.length());
                    }
                }
            }
        } catch (IOException e) {
            Xlog.w(TAG, "updateArmPll IOException: " + e.getMessage());
        }
        Message msg = mHandler.obtainMessage(INDEX_UPDATE_PLL);
        msg.arg1 = bSuccess ? 1 : 0;
        msg.obj = value;
        mHandler.sendMessage(msg);
    }

    private void setArmPll() {
        String editValue = mEtArmPllValue.getText().toString();
        Pattern pattern = Pattern.compile(PLL_VALUE_PATTERN);
        Matcher m = pattern.matcher(editValue);
        if (m.find()) {
            String cmd = "echo " + mArmPllId + " " + editValue
                    + " > /proc/clkmgr/pll_fsel";
            boolean result = false;
            try {
                if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(cmd)) {
                    result = true;
                }
            } catch (IOException e) {
                Xlog.w(TAG, "setArmPll IOException: " + e.getMessage());
            }
            Toast.makeText(this,
                    "Set ARMPLL value " + (result ? "success" : "fail"),
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, "Input ARMPLL error", Toast.LENGTH_SHORT)
                    .show();
        }
        updateArmPll();
    }

    @Override
    public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
        if (arg0.isEnabled()) {
            if (mCbDebugMsgEnable.getId() == arg0.getId()) {
                Xlog.v(TAG, "CheckBox is checked: " + arg1);
                handleEvent(arg1 ? INDEX_SET_DEBUG_MODE_VALUE_E
                        : INDEX_SET_DEBUG_MODE_VALUE_D, INDEX_SET_DEBUG_MODE);
            } else {
                Xlog.v(TAG, "Unknown event");
            }
        }
    }

    @Override
    public void onClick(View arg0) {
        if (mTvDebugMsgEnable.getId() == arg0.getId()) {
            Xlog.v(TAG, "TextView is clicked");
            mCbDebugMsgEnable.performClick();
        } else if (mBtnStart.getId() == arg0.getId()) {
            Xlog.v(TAG, mBtnStart.getText() + " is clicked");
            if (mBtnStart.getText().toString().equals(
                    getResources().getString(
                            R.string.hqa_cpustress_clockswitch_start))) {
                int second = 0;
                long nsecond = 0;
                try {
                    second = Integer.valueOf(mEtSecond.getText().toString());
                    nsecond = Long.valueOf(mEtNSecond.getText().toString());
                } catch (NumberFormatException nfe) {
                    Xlog.d(TAG, nfe.getMessage());
                    Toast.makeText(ClockSwitch.this, "Time period value error",
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                handleEvent(second + " " + nsecond, INDEX_SET_PERIOD);
                handleEvent(INDEX_SET_PERIOD_MODE_VALUE_E,
                        INDEX_SET_PERIOD_MODE);
                updateAutoTestView(true);
                if (null != mBoundService) {
                    mBoundService.startTest(new Bundle());
                }
            } else {
                handleEvent(INDEX_SET_PERIOD_MODE_VALUE_D,
                        INDEX_SET_PERIOD_MODE);
                updateAutoTestView(false);
                if (null != mBoundService) {
                    mBoundService.stopTest();
                    mBoundService.updateWakeLock();
                }
            }
        } else if (mBtnSwitchM.getId() == arg0.getId()) {
            Xlog.v(TAG, mBtnSwitchM.getText() + " is clicked");
            handleEvent(INDEX_SET_MODE_VALUE_0, INDEX_SET_MODE);
        } else if (mBtnSwitchG.getId() == arg0.getId()) {
            Xlog.v(TAG, mBtnSwitchG.getText() + " is clicked");
            handleEvent(INDEX_SET_MODE_VALUE_1, INDEX_SET_MODE);
        } else if (mBtnSet.getId() == arg0.getId()) {
            Xlog.v(TAG, "Set ARM pll is clicked");
            setArmPll();
        } else {
            Xlog.v(TAG, "Unknown event");
        }
    }

    @Override
    public void onServiceConnected(ComponentName name, IBinder service) {
        Xlog.v(TAG, "Enter onServiceConnected");
        mBoundService = ((CpuStressTestService.StressTestBinder) service)
                .getService();
        mBoundService.mTestClass = this;
        // this.removeDialog(DIALOG_WAIT);
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
        Xlog.v(TAG, "Enter onServiceDisconnected");
        // mBoundService.testObject = null;
        mBoundService = null;
    }

    @Override
    public void onUpdateTestResult() {
        Xlog.v(TAG, "Enter onupdateTestResult");
    }

}
