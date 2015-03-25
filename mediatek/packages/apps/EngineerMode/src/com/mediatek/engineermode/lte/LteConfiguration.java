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

package com.mediatek.engineermode.lte;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.InputType;
import android.text.InputFilter;
import android.text.InputFilter.LengthFilter;
import android.text.Spanned;
import android.widget.Button;
import android.widget.CheckBox;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class LteConfiguration extends Activity implements View.OnClickListener, CheckBox.OnCheckedChangeListener{
    private static final String TAG = "LteConfiguration";
    private static final int MSG_SET_COMMAND = 0;
    private static final int MSG_GET_PARAMETER = 1;
    private static final int MSG_GET_SEARCH_TIMER = 2;

    private int[] mThresholdMax;
    private List<EditText> mThresholdEditors;
    private List<EditText> mTimerEditors;
    private List<EditText> mSearchTimerEditors;
    private Spinner mSpinnerRsrp;
    private Spinner mSpinnerRsrq;
    private Spinner mSpinnerRelation;
    private CheckBox mCheckBoxPsdm;
    private Toast mToast = null;
    private ProgressDialog mDialog = null;
    private Phone mPhone;
    private GeminiPhone mGeminiPhone;
    private int mSlot = PhoneConstants.GEMINI_SIM_1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lte_configuration);
        initComponents();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
        sendATCommand("AT+EMPC?", "+EMPC:", MSG_GET_PARAMETER);
    }

    @Override
    public void onClick(View v) {
        String cmd = null;
        switch (v.getId()) {
        case R.id.lte_set_threshold:
            if (checkThreshold()) {
                cmd = "AT+EMPC=1";
                for (EditText editor : mThresholdEditors) {
                    cmd += "," + editor.getText().toString();
                }
                sendATCommand(cmd, MSG_SET_COMMAND);
            } else {
                showToast(R.string.lte_error_set_invalid_value);
            }
            break;
    	case R.id.lte_set_timer:
            if (checkTimer(mTimerEditors)) {
                cmd = "AT+EMPC=2";
                for (EditText editor : mTimerEditors) {
                    cmd += "," + editor.getText().toString();
                }
                sendATCommand(cmd, MSG_SET_COMMAND);
            } else {
                showToast(R.string.lte_error_set_invalid_timer);
            }
            break;
        case R.id.lte_instant_search:
            sendATCommand("AT+EBGS", MSG_SET_COMMAND);
            break;
        case R.id.lte_set_threshold_operation:
            cmd = "AT+EMPC=0," + mSpinnerRsrp.getSelectedItemPosition() + ","
                    + mSpinnerRsrq.getSelectedItemPosition() + ","
                    + mSpinnerRelation.getSelectedItemPosition();
            sendATCommand(cmd, MSG_SET_COMMAND);
            break;
        case R.id.lte_set_search_timer:
            if (checkSearchTimer(mSearchTimerEditors)) {
                cmd = "AT+ERSCFG=";
                for (EditText editor : mSearchTimerEditors) {
                    cmd += editor.getText().toString() + ",";
                }
                cmd = cmd.substring(0, cmd.length() - 1);
                sendATCommand(cmd, MSG_SET_COMMAND);
            } else {
                showToast(R.string.lte_error_set_invalid_search_timer);
            }
            break;
        case R.id.lte_get_search_timer:
            sendATCommand("AT+ERSCFG?", "+ERSCFG", MSG_GET_SEARCH_TIMER);
            break;
    	default:
    	    break;
    	}
    }

    @Override
    public void onCheckedChanged(CompoundButton v, boolean isChecked) {
        if (v.getId() == R.id.lte_enable_psdm) {
            String cmd = "AT+EMPC=3," + (isChecked ? "1" : "0");
            sendATCommand(cmd, MSG_SET_COMMAND);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case MSG_SET_COMMAND:
            mDialog = new ProgressDialog(this);
            mDialog.setMessage(getString(R.string.lte_wait_set));
            mDialog.setCancelable(false);
            return mDialog;
        case MSG_GET_PARAMETER:
            mDialog = new ProgressDialog(this);
            mDialog.setMessage(getString(R.string.lte_wait_get));
            mDialog.setCancelable(false);
            return mDialog;
        case MSG_GET_SEARCH_TIMER:
            mDialog = new ProgressDialog(this);
            mDialog.setMessage(getString(R.string.lte_wait_get));
            mDialog.setCancelable(false);
            return mDialog;
        default:
            break;
        }
        return super.onCreateDialog(id);
    }

    private final Handler mAtCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.d(TAG, "handleMessage() " + msg.what);
            if (mDialog != null) {
                mDialog.dismiss();
            }

            AsyncResult ar;
            switch (msg.what) {
            case MSG_SET_COMMAND:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
//                    showToast(R.string.lte_command_succeed);
                } else {
                    Elog.e(TAG, "Exception: " + ar.exception);
                    showToast(R.string.lte_command_failed);
                }
                break;
            case MSG_GET_PARAMETER:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    String[] data = (String[]) ar.result;
                    if (parseParameters(data)) {
//                        showToast(R.string.lte_command_succeed);
                    } else {
                        showToast(R.string.lte_error_get_invalid_value);
                    }
                } else {
                    Elog.e(TAG, "Exception: " + ar.exception);
                    showToast(R.string.lte_command_failed);
                }
                break;
            case MSG_GET_SEARCH_TIMER:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    String[] data = (String[]) ar.result;
                    if (parseSearchTimer(data)) {
//                        showToast(R.string.lte_command_succeed);
                    } else {
                        showToast(R.string.lte_error_get_invalid_value);
                    }
                } else {
                    Elog.e(TAG, "Exception: " + ar.exception);
                    showToast(R.string.lte_command_failed);
                }
                break;
            default:
                break;
            }
        }
    };

    private void initComponents() {
        mSpinnerRsrp = (Spinner) findViewById(R.id.lte_4g_rsrp);
        mSpinnerRsrq = (Spinner) findViewById(R.id.lte_4g_rsrq);
        mSpinnerRelation = (Spinner) findViewById(R.id.lte_4g_relation);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.lte_threshold_operation_on_off, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinnerRsrp.setAdapter(adapter);
        mSpinnerRsrq.setAdapter(adapter);

        ArrayAdapter<CharSequence> adapter2 = ArrayAdapter.createFromResource(this,
                R.array.lte_threshold_operation_and_or, android.R.layout.simple_spinner_item);
        adapter2.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinnerRelation.setAdapter(adapter2);

        mSpinnerRsrp.setSelection(0);
        mSpinnerRsrq.setSelection(0);
        mSpinnerRelation.setSelection(0);

        mCheckBoxPsdm = (CheckBox) findViewById(R.id.lte_enable_psdm);
        mCheckBoxPsdm.setOnCheckedChangeListener(this);

        Button button = (Button) findViewById(R.id.lte_set_threshold);
        button.setOnClickListener(this);
    	button = (Button) findViewById(R.id.lte_set_timer);
        button.setOnClickListener(this);
        button = (Button) findViewById(R.id.lte_instant_search);
        button.setOnClickListener(this);
        button = (Button) findViewById(R.id.lte_set_threshold_operation);
        button.setOnClickListener(this);
        button = (Button) findViewById(R.id.lte_set_search_timer);
        button.setOnClickListener(this);
        button = (Button) findViewById(R.id.lte_get_search_timer);
        button.setOnClickListener(this);

        mThresholdEditors = initEditorTable(R.id.lte_threshold, R.array.lte_threshold);
        mThresholdMax = getResources().getIntArray(R.array.lte_threshold_max);
        mTimerEditors = initEditorTable(R.id.lte_timer, R.array.lte_timer);
        mSearchTimerEditors = initEditorTable(R.id.lte_search_timer_1, R.array.lte_search_timer_1);
        mSearchTimerEditors.addAll(initEditorTable(R.id.lte_search_timer_2, R.array.lte_search_timer_2));
    }

    private List<EditText> initEditorTable(int layoutResId, int arrayResId) {
        TableLayout root = (TableLayout) findViewById(layoutResId);
        String[] labels = getResources().getStringArray(arrayResId);
        List<EditText> list = new ArrayList<EditText>(labels.length);
        LayoutParams para = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 1);

        for (int i = 0; i < labels.length; i++) {
            EditText editor = new EditText(this);
            editor.setInputType(InputType.TYPE_CLASS_NUMBER);
            list.add(editor);
        }

        int rows = labels.length;
        for (int i = 0; i < rows; i++) {
            TableRow row = new TableRow(this);
            for (int j = i; j < labels.length; j += rows) {
                TextView label = new TextView(this);
                label.setText(labels[j]);
                row.addView(label);
                row.addView(list.get(j));
            }
            root.addView(row);
        }
        return list;
    }

    private boolean checkThreshold() {
        for (int i = 0; i < mThresholdEditors.size(); i++) {
            try {
                int value = Integer.parseInt(mThresholdEditors.get(i).getText().toString().trim());
                if (value >= 0 && value <= mThresholdMax[i]) {
                    continue;
                }
            } catch (NumberFormatException e) {
            }
            mThresholdEditors.get(i).requestFocus();
            return false;
        }
        return true;
    }

    private boolean checkTimer(List<EditText> editors) {
        for (EditText edit : editors) {
            try {
                if (Long.parseLong(edit.getText().toString().trim()) >= 100) {
                    continue;
                }
            } catch (NumberFormatException e) {
            }
            edit.requestFocus();
            return false;
        }
        return true;
    }

    private boolean checkSearchTimer(List<EditText> editors) {
        for (EditText edit : editors) {
            try {
                if (Long.parseLong(edit.getText().toString().trim()) > 0) {
                    continue;
                }
            } catch (NumberFormatException e) {
            }
            edit.requestFocus();
            return false;
        }
        return true;
    }

    private boolean parseParameters(String[] data) {
        // Format of data[0]:
        // +EMPC:<4G_rsrp>,<4G_rsrq>,<4G_relation>,<good_rssi>,<bad_rssi>,<good_rscp>,<bad_rscp>,
        //         <good_rsrp>,<bad_rsrp>,<good_rsrq>,<bad_rsrq>,<T1>,<T2>,<T3>,<T4>,<enable>

        int len = "+EMPC:".length();
        if (data == null || data[0].length() <= len) {
            Elog.e(TAG, "Got invalid threshold values");
            return false;
        }

        String str = data[0].substring(len, data[0].length());
        Elog.i(TAG, "Got threshold values: " + str);
        String[] rawValues = str.split(",");
        if (rawValues.length < mThresholdEditors.size() + mTimerEditors.size() + 4) {
            return false;
        }

        List<Long> values = new ArrayList<Long>(rawValues.length);
        try {
            for (int i = 0; i < rawValues.length; i++) {
                long value = Long.parseLong(rawValues[i].trim());
                if (i < 3 && value != 0 && value != 1) {
                    return false;
                }
                values.add(value);
            }
        } catch (NumberFormatException e) {
            return false;
        }

        mSpinnerRsrp.setSelection(values.remove(0) == 1 ? 1 : 0);
        mSpinnerRsrq.setSelection(values.remove(0) == 1 ? 1 : 0);
        mSpinnerRelation.setSelection(values.remove(0) == 1 ? 1 : 0);
        for (EditText edit : mThresholdEditors) {
            edit.setText(values.remove(0).toString());
        }
        for (EditText edit : mTimerEditors) {
            edit.setText(values.remove(0).toString());
        }
        mCheckBoxPsdm.setOnCheckedChangeListener(null);
        mCheckBoxPsdm.setChecked(1 == values.remove(0));
        mCheckBoxPsdm.setOnCheckedChangeListener(this);
        return true;
    }

    private boolean parseSearchTimer(String[] data) {
        // Format of data[0]:
        // +ERSCFG:<T1>,<T2>,<T3>,...,<T19>
        int len = "+ERSCFG:".length();
        if (data == null || data[0].length() <= len) {
            Elog.i(TAG, "Got invalid timer values");
            return false;
        }

        String rawValue = data[0].substring(len, data[0].length());
        Elog.i(TAG, "Got timer values: " + rawValue);
        String[] values = rawValue.split(",");
        if (values.length < mSearchTimerEditors.size()) {
            return false;
        }

        for (int i = 0; i < mSearchTimerEditors.size(); i++) {
            mSearchTimerEditors.get(i).setText(values[i].trim());
        }
        return true;
    }

    private void sendATCommand(String str, int message) {
        sendATCommand(str, "", message);
    }

    private void sendATCommand(String str, String str2, int message) {
        Elog.d(TAG, "sendATCommand() " + str + ", " + str2);
        String cmd[] = new String[2];
        cmd[0] = str;
        cmd[1] = str2;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone.invokeOemRilRequestStringsGemini(cmd, mAtCmdHander.obtainMessage(message), mSlot);
        } else {
            mPhone.invokeOemRilRequestStrings(cmd, mAtCmdHander.obtainMessage(message));
        }
        removeDialog(message);
        showDialog(message);
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    private void showToast(int msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

