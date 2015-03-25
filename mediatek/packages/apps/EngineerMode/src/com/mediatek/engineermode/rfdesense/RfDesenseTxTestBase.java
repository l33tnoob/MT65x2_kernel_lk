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

package com.mediatek.engineermode.rfdesense;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class RfDesenseTxTestBase extends Activity {
    private static final String TAG = "TxTestBase";

    protected static final int START = 1;
    protected static final int PAUSE = 2;
    protected static final int REBOOT = 3;
    protected static final int UPDATE_BUTTON = 4;

    protected static final int UPDATE_DELAY = 1000;

    protected Spinner mBand;
//    protected RadioGroup mModulation;
    protected Editor mChannel = new Editor();
    protected Editor mPower = new Editor();
    protected Editor mAfc = new Editor();
    protected Editor mTsc = new Editor();
    protected Spinner mPattern;
    protected Button mButtonStart;
    protected Button mButtonPause;
    protected Button mButtonStop;

    // Some help methods to make code line shorter
    protected static class Editor {
        public EditText editor;
        public String defaultValue = null;
        public int min;
        public int max;
        public int min2;
        public int max2;
        public int step = 1;

        public void set(String arg0, String arg1, String arg2, String arg3, String arg4) {
            defaultValue = arg0;
            min = Integer.parseInt(arg1);
            max = Integer.parseInt(arg2);
            min2 = Integer.parseInt(arg3);
            max2 = Integer.parseInt(arg4);
        }

        public void set(String arg0, String arg1, String arg2) {
            defaultValue = arg0;
            min = Integer.parseInt(arg1);
            max = Integer.parseInt(arg2);
            min2 = Integer.parseInt(arg1);
            max2 = Integer.parseInt(arg2);
        }

        public void setText(String text) {
            editor.setText(text);
        }

        public String getText() {
            return editor.getText().toString();
        }

        public void setToDefault() {
            editor.setText(defaultValue);
        }
        
        public boolean check() {
            try {
                int value = Integer.parseInt(getText());
    
                Elog.i(TAG, "value = " + value);
                Elog.i(TAG, "limits = [" + min + "," + max + "],[" + min2 + "," + max2 + "]");
                if ((value < min || value > max) && (value < min2 || value > max2)) {
                    return false;
                }
                if (step != 1 && (value - min) % step != 0) {
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }

            return true;
        }

        public String getValidRange() {
            String text = "[" + min + "," + max + "]";
            if (min != min2) {
                text += ", [" + min2 + "," + max2 + "]";
            }
            if (step != 1) {
                text += ", step " + step;
            }
            return text;
        }
    }

    protected static final int CHANNEL_DEFAULT = 0;
    protected static final int CHANNEL_MIN = 1;
    protected static final int CHANNEL_MAX = 2;
    protected static final int CHANNEL_MIN2 = 3;
    protected static final int CHANNEL_MAX2 = 4;
    protected static final int POWER_DEFAULT = 5;
    protected static final int POWER_MIN = 6;
    protected static final int POWER_MAX = 7;

    protected static final int STATE_NONE = 0;
    protected static final int STATE_STARTED = 1;
    protected static final int STATE_PAUSED = 2;
    protected static final int STATE_STOPPED = 3;

    protected Phone mPhone;
    protected GeminiPhone mGeminiPhone;
    protected int mState = STATE_NONE;

    protected int mCurrentBand = -1;

    protected final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            String command = "";
            switch (msg.what) {
            case START:
                command = "START Command";
                break;
            case PAUSE:
                command = "PAUSE Command";
                break;
            case REBOOT:
                command = "REBOOT Command";
                break;
            default:
            }

            switch (msg.what) {
            case START:
            case PAUSE:
            case REBOOT:
                AsyncResult ar = (AsyncResult) msg.obj;
                String text;
                if (ar.exception == null) {
                    text = command + " success.";
                } else {
                    text = command + " failed.";
                }

                Toast.makeText(RfDesenseTxTestBase.this, text, Toast.LENGTH_SHORT).show();
                break;

            case UPDATE_BUTTON:
                updateButtons();

            default:
                break;
            }
        }
    };

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test_gsm);

        mBand = (Spinner) findViewById(R.id.band_spinner);
//        mModulation = (RadioGroup)  findViewById(R.id.modulation_radio_group);
        mChannel.editor = (EditText) findViewById(R.id.channel_editor);
        mPower.editor = (EditText) findViewById(R.id.power_editor);
        mAfc.editor = (EditText) findViewById(R.id.afc_editor);
        mTsc.editor = (EditText) findViewById(R.id.tsc_editor);
        mPattern = (Spinner) findViewById(R.id.pattern_spinner);
        mButtonStart = (Button) findViewById(R.id.button_start);
        mButtonPause = (Button) findViewById(R.id.button_pause);
        mButtonStop = (Button) findViewById(R.id.button_stop);

        Button.OnClickListener listener = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (v.getId()) {
                case R.id.button_start:
                    if (!checkValues()) {
                        disableAllButtons();
                        mHandler.sendMessageDelayed(Message.obtain(mHandler, UPDATE_BUTTON), 1000);
                        break;
                    }

                    String band = getResources().getStringArray(R.array.rf_desense_tx_test_gsm_band_values)[mBand.getSelectedItemPosition()];
//                    long modulation = mModulation.getCheckedRadioButtonId();
                    String channel = mChannel.getText();
                    String power = mPower.getText();
                    String afc = mAfc.getText();
                    String tsc = mTsc.getText();
                    String pattern = getResources().getStringArray(R.array.rf_desense_tx_test_gsm_pattern_values)[mPattern.getSelectedItemPosition()];

                    String command = "AT+ERFTX=2,1," + channel + "," + afc + "," + band + "," +
                                     tsc + "," + power + "," + pattern;// + "," +
//                                     (modulation == R.id.modulation_gmsk ? "0" : "1");
                    sendAtCommand(command, "", START);

                    disableAllButtons();
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, UPDATE_BUTTON), 1000);
                    mState = STATE_STARTED;
                    break;

                case R.id.button_pause:
                    sendAtCommand("AT+ERFTX=2,0", "", PAUSE);

                    disableAllButtons();
                    mHandler.sendMessageDelayed(Message.obtain(mHandler, UPDATE_BUTTON), 1000);
                    mState = STATE_PAUSED;
                    break;

                case R.id.button_stop:
                    showDialog(REBOOT);
                    break;

                default:
                    break;
                }
            }
        };

        mButtonStart.setOnClickListener(listener);
        mButtonPause.setOnClickListener(listener);
        mButtonStop.setOnClickListener(listener);
/*
        RadioGroup.OnCheckedChangeListener radioListener = new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                updateLimits();
                setDefaultValues();
            }
        };

        mModulation.setOnCheckedChangeListener(radioListener);
*/
        AdapterView.OnItemSelectedListener l = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentBand != mBand.getSelectedItemPosition()) {
                    updateLimits();
                    setDefaultValues();
                    mCurrentBand = mBand.getSelectedItemPosition();
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };

        mBand.setOnItemSelectedListener(l);
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateButtons();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == REBOOT) {
            OnClickListener listener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (which == DialogInterface.BUTTON_POSITIVE) {
                        sendAtCommand("AT+CFUN=1,1", "", REBOOT);
                        disableAllButtons();
                        mState = STATE_STOPPED;
                        finish();
                    }
                    dialog.dismiss();
                }
            };

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            return builder.setTitle("Reboot")
                          .setMessage("Reboot modem?")
                          .setPositiveButton("Reboot", listener)
                          .setNegativeButton("Cancel", listener)
                          .create();
        }
        return super.onCreateDialog(id);
    }

    protected void sendAtCommand(String str1, String str2, int what) {
        String cmd[] = new String[2];
        cmd[0] = str1;
        cmd[1] = str2;
        Elog.i(TAG, "send: "+ cmd[0]);
        // TODO: just for unit test, remove it
//        Toast.makeText(this, cmd[0], Toast.LENGTH_SHORT).show();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mGeminiPhone.invokeOemRilRequestStringsGemini(cmd, mHandler.obtainMessage(what), PhoneConstants.GEMINI_SIM_1);
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
            mPhone.invokeOemRilRequestStrings(cmd, mHandler.obtainMessage(what));
        }
    }

    protected void updateLimits() {
    }

    protected boolean checkValues() {
        Editor[] editors = new Editor[] {mChannel, mPower, mAfc, mTsc};
        String[] toast = new String[] {"Channel", "TX Power", "AFC", "TSC"};

        for (int i = 0; i < 4; i++) {
            Editor editor = editors[i];
            if (editor.editor.getVisibility() == View.VISIBLE && !editor.check()) {
                String text = "Invalid " + toast[i] + ". Valid range: " + editor.getValidRange();
                Toast.makeText(this, text, Toast.LENGTH_SHORT).show();
                return false;
            }
        }

        return true;
    }

    protected void setDefaultValues() {
        updateLimits();
        mChannel.setToDefault();
        mPower.setToDefault();
        mAfc.setToDefault();
        mTsc.setToDefault();
    }

    protected void disableAllButtons() {
        mButtonStart.setEnabled(false);
        mButtonPause.setEnabled(false);
        mButtonStop.setEnabled(false);
    }

    protected void updateButtons() {
        mButtonStart.setEnabled(mState == STATE_NONE || mState == STATE_PAUSED);
        mButtonPause.setEnabled(mState == STATE_STARTED);
        mButtonStop.setEnabled(mState != STATE_NONE);
    }
}
