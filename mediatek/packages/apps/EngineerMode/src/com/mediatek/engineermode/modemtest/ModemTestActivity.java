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

package com.mediatek.engineermode.modemtest;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class ModemTestActivity extends Activity {

    public static final String TAG = "ModemTest";

    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    private static final int MODEM_NONE = 0;
    private static final int MODEM_CTA = 1;
    private static final int MODEM_FTA = 2;
    private static final int MODEM_IOT = 3;
    private static final int MODEM_QUERY = 4;
    private static final int MODEM_OPERATOR = 5;
    private static final int MODEM_FACTORY = 6;

    private static final int EVENT_QUERY_PREFERRED_TYPE_DONE = 1000;
    private static final int EVENT_SET_PREFERRED_TYPE_DONE = 1001;
    private static final int REBOOT_DIALOG = 2000;
    private static final int NETWORK_TYPE = 3;

    private static final int CMD_LENGTH = 6;
    private static final int MODE_LENGTH = 3;

    private static final String PREFERENCE_GPRS = "com.mtk.GPRS";
    private static final String PREF_ATTACH_MODE = "ATTACH_MODE";
    private static final String PREF_ATTACH_MODE_SIM = "ATTACH_MODE_SIM";
    private static final int ATTACH_MODE_ALWAYS = 1;
    private static final int ATTACH_MODE_NOT_SPECIFY = -1;
    private static final int DOCOMO_OPTION = 1 << 7;
    private static final int SOFTBANK_OPTION = 1 << 8;

    private static final int IPO_ENABLE = 1;
    private static final int IPO_DISABLE = 0;

    private static final int PCH_DATA_PREFER = 0;
    private static final int PCH_CALL_PREFER = 1;

    private int mCtaOption = 0;
    private int mIotOption = 0;
    private int mFtaOption = 0;
    private int mOperatorOption = 0;
    private int mFactoryOption = 0;

    private Button mNoneBtn;
    private Button mCtaBtn;
    private Button mFtaBtn;
    private Button mIotBtn;
    private Button mOperatorBtn;
    private Button mFactoryBtn;
    private TextView mTextView;
    private boolean mModemFlag;
    private String[] mCtaOptionsArray;
    private String[] mFtaOptionsArray;
    private String[] mIotOptionsArray;
    private String[] mOperatorOptionsArray;
    private int mCurrentMode = 0;

    private final Handler mATCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            boolean rebootFlag = false;
            switch (msg.what) {
            case MODEM_NONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_NONE AT cmd success.", Toast.LENGTH_LONG)
                            .show();
                    rebootFlag = true;
                } else {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_NONE AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case MODEM_CTA:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_INTEGRITY_OFF AT cmd success.", Toast.LENGTH_LONG)
                            .show();
                    rebootFlag = true;
                } else {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_INTEGRITY_OFF AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case MODEM_FTA:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_FTA AT cmd success.", Toast.LENGTH_LONG)
                            .show();
                    rebootFlag = true;
                } else {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_FTA AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case MODEM_IOT:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_IOT AT cmd success.", Toast.LENGTH_LONG)
                            .show();
                    rebootFlag = true;
                } else {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_IOT AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case MODEM_QUERY:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.i(TAG, "Query success.");
                    String data[] = (String[]) ar.result;
                    handleQuery(data);
                } else {
                    Toast.makeText(ModemTestActivity.this, "Query failed.",
                            Toast.LENGTH_LONG).show();
                }
                break;
            case MODEM_OPERATOR:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ModemTestActivity.this,
                                    "MODEM_OPERATOR AT cmd success.",
                                    Toast.LENGTH_LONG).show();
                    rebootFlag = true;
                } else {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_OPERATOR AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case MODEM_FACTORY:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_FACTORY AT cmd success.", Toast.LENGTH_LONG)
                            .show();
                    rebootFlag = true;
                } else {
                    Toast.makeText(ModemTestActivity.this,
                            "MODEM_FACTORY AT cmd failed.", Toast.LENGTH_LONG)
                            .show();
                }
                break;
            case EVENT_QUERY_PREFERRED_TYPE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int type = ((int[]) ar.result)[0];
                    Elog.i(TAG, "Get Preferred Type " + type);
                    if (type == 0) {
                        mModemFlag = true;
                    } else {
                        mModemFlag = false;
                    }
                }
                break;
            case EVENT_SET_PREFERRED_TYPE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    // Toast.makeText(ModemTestActivity.this,
                    // "Turn off WCDMA Preferred Fail",
                    // Toast.LENGTH_LONG).show();
                    Elog.e(TAG, "Turn off WCDMA Preferred Fail");
                }
                break;
            default:
                break;
            }
            if (rebootFlag) {
                Elog.i(TAG, "disableAllButton.");
                disableAllButton();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.modem_test_activity_6589);

        View.OnClickListener listener = new View.OnClickListener() {

            public void onClick(View v) {
                switch (v.getId()) {
                case R.id.modem_test_none_btn:
                    sendATCommad("0", MODEM_NONE);
                    if (mCurrentMode == MODEM_FTA) {
                        setGprsTransferType(PCH_CALL_PREFER);
                    }
                    break;
                case R.id.modem_test_cta_btn:
                    if (mModemFlag) {
                        writePreferred(NETWORK_TYPE);
                        mPhone.setPreferredNetworkType(
                                        NETWORK_TYPE,
                                        mATCmdHander
                                                .obtainMessage(EVENT_SET_PREFERRED_TYPE_DONE));
                    }
                    sendATCommad("1,0", MODEM_CTA);                        
                    if (mCurrentMode == MODEM_FTA) {
                        setGprsTransferType(PCH_CALL_PREFER);
                    }
                    break;
                case R.id.modem_test_fta_btn:
                    if (mModemFlag) {
                        writePreferred(NETWORK_TYPE);
                        mPhone.setPreferredNetworkType(
                                        NETWORK_TYPE,
                                        mATCmdHander
                                                .obtainMessage(EVENT_SET_PREFERRED_TYPE_DONE));
                    }
                    showDialog(MODEM_FTA);
                    break;
                case R.id.modem_test_iot_btn:
                    showDialog(MODEM_IOT);
                    break;
                case R.id.modem_test_operator_btn:
                    if (mModemFlag) {
                        writePreferred(NETWORK_TYPE);
                        mPhone
                                .setPreferredNetworkType(
                                        NETWORK_TYPE,
                                        mATCmdHander
                                                .obtainMessage(EVENT_SET_PREFERRED_TYPE_DONE));
                    }
                    showDialog(MODEM_OPERATOR);
                    break;
                case R.id.modem_test_factory_btn:
                    if (mModemFlag) {
                        writePreferred(NETWORK_TYPE);
                        mPhone
                                .setPreferredNetworkType(
                                        NETWORK_TYPE,
                                        mATCmdHander
                                                .obtainMessage(EVENT_SET_PREFERRED_TYPE_DONE));
                    }
                    sendATCommad("5", MODEM_FACTORY);
                    if (mCurrentMode == MODEM_FTA) {
                        setGprsTransferType(PCH_CALL_PREFER);
                    }
                    break;
                default:
                    break;
                }
            }
        };
        mTextView = (TextView) findViewById(R.id.modem_test_textview);
        mNoneBtn = (Button) findViewById(R.id.modem_test_none_btn);
        mFactoryBtn = (Button) findViewById(R.id.modem_test_factory_btn);
        mFactoryBtn.setOnClickListener(listener);
        mCtaBtn = (Button) findViewById(R.id.modem_test_cta_btn);
        mCtaBtn.setOnClickListener(listener);

        mFtaBtn = (Button) findViewById(R.id.modem_test_fta_btn);
        mIotBtn = (Button) findViewById(R.id.modem_test_iot_btn);
        mOperatorBtn = (Button) findViewById(R.id.modem_test_operator_btn);
        mNoneBtn.setOnClickListener(listener);
        mFtaBtn.setOnClickListener(listener);
        mIotBtn.setOnClickListener(listener);
        mOperatorBtn.setOnClickListener(listener);
        mTextView.setText("The current mode is unknown");
        mCtaOptionsArray = getResources().getStringArray(
                R.array.modem_test_cta_options);
        mFtaOptionsArray = getResources().getStringArray(
                R.array.modem_test_fta_options);
        mIotOptionsArray = getResources().getStringArray(
                R.array.modem_test_iot_options_6589);
        mOperatorOptionsArray = getResources().getStringArray(
                R.array.modem_test_operator_options_6589);
        // send AT Cmd and register the event
        mPhone = PhoneFactory.getDefaultPhone();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        }
        // mPhone.registerForNetworkInfo(mResponseHander, EVENT_NW_INFO, null);

        String cmd[] = new String[2];
        cmd[0] = "AT+EPCT?";
        cmd[1] = "+EPCT:";
        mPhone.invokeOemRilRequestStrings(cmd, mATCmdHander
                .obtainMessage(MODEM_QUERY));
    }

    @Override
    protected void onResume() {
        super.onResume();
        mCtaOption = 0;
        mIotOption = 0;
        mFtaOption = 0;
        mOperatorOption = 0;
        mFactoryOption = 0;
        checkNetworkType();
    }

    private void checkNetworkType() {
        Elog.i(TAG, "TcheckNetworkType");
        mPhone.getPreferredNetworkType(mATCmdHander
                .obtainMessage(EVENT_QUERY_PREFERRED_TYPE_DONE));
    }

    private void sendATCommad(String str, int message) {
        String cmd[] = new String[2];
        cmd[0] = "AT+EPCT=" + str;
        cmd[1] = "";
        mPhone.invokeOemRilRequestStrings(cmd, mATCmdHander
                .obtainMessage(message));
    }

    private void handleQuery(String[] data) {
        if (null == data) {
            Toast.makeText(ModemTestActivity.this,
                    "The returned data is wrong.", Toast.LENGTH_SHORT)
                    .show();
            return;
        } else {
            Elog.i(TAG, "data length is " + data.length);
            int i = 0;
            for (String str : data) {
                if (str != null) {
                    Elog.i(TAG, "data[" + i + "] is : " + str);
                }
                i++;
            }
        }
        if (data[0].length() > CMD_LENGTH) {
            String mode = data[0].substring(CMD_LENGTH + 1, data[0].length());
            Elog.i(TAG, "mode is " + mode);
            if (mode.length() >= MODE_LENGTH) {
                String subMode = mode.substring(0, 1);
                String subCtaMode = mode.substring(2, mode.length());
                Elog.i(TAG, "subMode is " + subMode);
                Elog.i(TAG, "subCtaMode is " + subCtaMode);
                mCurrentMode = Integer.parseInt(subMode);
                if ("0".equals(subMode)) {
                    mTextView.setText("The current mode is none");
                } else if ("1".equals(subMode)) {
                    mTextView.setText("The current mode is Integrity Off");
                } else if ("2".equals(subMode)) {
                    mTextView.setText("The current mode is FTA:");
                    try {
                        int ftaLength = mFtaOptionsArray.length;
                        Elog.i(TAG, "ftaLength is " + ftaLength);
                        int val = Integer.valueOf(subCtaMode).intValue();
                        Elog.i(TAG, "val is " + val);
                        String text = "The current mode is FTA: ";
                        for (int j = 0; j < ftaLength; j++) {
                            Elog.i(TAG, "j ==== " + j);
                            Elog.i(TAG, "(val & (1 << j)) is "
                                    + (val & (1 << j)));
                            if ((val & (1 << j)) != 0) {
                                text = text + mFtaOptionsArray[j] + ",";
                            }
                        }
                        // Drop the last ","
                        text = text.substring(0, text.length() - 1);
                        mTextView.setText(text);
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                        Elog.i(TAG, "Exception when transfer subFtaMode");
                    }
                } else if ("3".equals(subMode)) {
                    mTextView.setText("The current mode is IOT:");
                    try {
                        int iotLength = mIotOptionsArray.length;
                        Elog.i(TAG, "iotLength is " + iotLength);
                        int val = Integer.valueOf(subCtaMode).intValue();
                        Elog.i(TAG, "val is " + val);
                        String text = "The current mode is IOT: ";
                        for (int j = 0; j < iotLength - 1; j++) {
                            Elog.i(TAG, "j ==== " + j);
                            Elog.i(TAG, "(val & (1 << j)) is "
                                    + (val & (1 << j)));
                            if ((val & (1 << j)) != 0) {
                                text = text + mIotOptionsArray[j + 1] + ",";
                            }
                        }
                        // Drop the last ","
                        text = text.substring(0, text.length() - 1);
                        mTextView.setText(text);
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                        Elog.i(TAG, "Exception when transfer subIotMode");
                    }
                } else if ("4".equals(subMode)) {
                    mTextView.setText("The current mode is Operator.");
                    try {
                        int operatorLength = mOperatorOptionsArray.length;
                        Elog.i(TAG, "operatorLength is " + operatorLength);
                        int val = Integer.valueOf(subCtaMode).intValue();
                        Elog.i(TAG, "val is " + val);
                        String text = "The current mode is Operator: ";
                        for (int j = 0; j < operatorLength; j++) {
                            Elog.i(TAG, "j ==== " + j);
                            Elog.i(TAG, "(val & (1 << j)) is "
                                    + (val & (1 << j)));
                            if ((val & (1 << j)) != 0) {
                                text = text + mOperatorOptionsArray[j]
                                        + ",";
                            }
                        }
                        // Drop the last ","
                        text = text.substring(0, text.length() - 1);
                        mTextView.setText(text);
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                        Elog.i(TAG, "Exception when transfer subFtaMode");
                    }
                } else if ("5".equals(subMode)) {
                    mTextView.setText("The current mode is Factory.");
                }
            } else {
                Elog.i(TAG, "mode len is " + mode.length());
            }
        } else {
            Elog.i(TAG, "The data returned is not right.");
        }
    }

    private void disableAllButton() {
        mNoneBtn.setEnabled(false);
        mFactoryBtn.setEnabled(false);
        mCtaBtn.setEnabled(false);
        mFtaBtn.setEnabled(false);
        mIotBtn.setEnabled(false);
        mOperatorBtn.setEnabled(false);
        showDialog(REBOOT_DIALOG);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case REBOOT_DIALOG:
            return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                    "MODEM TEST").setMessage("Please reboot the phone!")
                    .setPositiveButton("OK", null).create();
        case MODEM_CTA:
            return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                    "MODEM TEST").setMultiChoiceItems(
                    R.array.modem_test_cta_options,
                    new boolean[] { false, false, false, false, false, false,
                            false, false, false },
                    new DialogInterface.OnMultiChoiceClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton, boolean isChecked) {

                            /* User clicked on a check box do some stuff */
                            Elog.v(TAG, "whichButton = " + whichButton);
                            Elog.v(TAG, "isChecked = " + isChecked);
                            if (isChecked) {
                                mCtaOption = mCtaOption + (1 << whichButton);
                            } else {
                                mCtaOption = mCtaOption - (1 << whichButton);
                            }
                            Elog.v(TAG, "mCtaOption = " + mCtaOption);
                        }
                    }).setPositiveButton("Send",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {

                            /* User clicked Yes so do some stuff */
                            sendATCommad("1," + String.valueOf(mCtaOption),
                                    MODEM_CTA);
                            if (mCurrentMode == MODEM_FTA) {
                                setGprsTransferType(PCH_CALL_PREFER);
                            }
                        }
                    }).setNegativeButton("Cancel",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {

                            /* User clicked No so do some stuff */
                            mCtaOption = 0;
                        }
                    }).create();
        case MODEM_FTA:
            return new AlertDialog.Builder(ModemTestActivity.this)
                    .setTitle("MODEM TEST").setMultiChoiceItems(
                            R.array.modem_test_fta_options,
                            new boolean[] { false, false, false, false, false,
                                    false, false, false, false },
                            new DialogInterface.OnMultiChoiceClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int whichButton, boolean isChecked) {

                                    /* User clicked on a check box do some stuff */
                                    Elog.v(TAG, "whichButton = " + whichButton);
                                    Elog.v(TAG, "isChecked = " + isChecked);
                                    if (isChecked) {
                                        mFtaOption = mFtaOption
                                                + (1 << whichButton);
                                    } else {
                                        mFtaOption = mFtaOption
                                                - (1 << whichButton);
                                    }
                                    Elog.v(TAG, "mFtaOption = " + mFtaOption);
                                }
                            }).setPositiveButton("Send",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int whichButton) {

                                    /* User clicked Yes so do some stuff */
                                    sendATCommad("2,"
                                            + String.valueOf(mFtaOption),
                                            MODEM_FTA);
                                    enableIPO(false);
                                    setGprsTransferType(PCH_DATA_PREFER);
                                }
                            }).setNegativeButton("Cancel",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int whichButton) {

                                    /* User clicked No so do some stuff */
                                    mFtaOption = 0;
                                }
                            }).create();
        case MODEM_IOT:
            return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                    "MODEM TEST").setMultiChoiceItems(
                    R.array.modem_test_iot_options_6589,
                    new boolean[] { false, false, false, false, false, false,
                            false, false, false },
                    new DialogInterface.OnMultiChoiceClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton, boolean isChecked) {

                            /* User clicked on a check box do some stuff */
                            Elog.v(TAG, "whichButton = " + whichButton);
                            Elog.v(TAG, "isChecked = " + isChecked);
                            if (whichButton > 0) {
                                if (isChecked) {
                                    mIotOption = mIotOption + (1 << (whichButton - 1));
                                } else {
                                    mIotOption = mIotOption - (1 << (whichButton - 1));
                                }
                            }
                            Elog.v(TAG, "mIotOption = " + mIotOption);
                        }
                    }).setPositiveButton("Send",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {

                            /* User clicked Yes so do some stuff */
                            sendATCommad("3," + String.valueOf(mIotOption),
                                    MODEM_IOT);
                            if (mCurrentMode == MODEM_FTA) {
                                setGprsTransferType(PCH_CALL_PREFER);
                            }
                        }
                    }).setNegativeButton("Cancel",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {

                            /* User clicked No so do some stuff */
                            mIotOption = 0;
                        }
                    }).create();
        case MODEM_OPERATOR:
            return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                    "MODEM TEST").setMultiChoiceItems(
                    R.array.modem_test_operator_options_6589,
                    new boolean[] { false, false, false, false, false, false,
                            false, false, false, false, false, false, false },
                    new DialogInterface.OnMultiChoiceClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton, boolean isChecked) {

                            /* User clicked on a check box do some stuff */
                            Elog.v(TAG, "whichButton = " + whichButton);
                            Elog.v(TAG, "isChecked = " + isChecked);
                            if (isChecked) {
                                mOperatorOption = mOperatorOption
                                        + (1 << whichButton);
                            } else {
                                mOperatorOption = mOperatorOption
                                        - (1 << whichButton);
                            }
                            Elog.v(TAG, "mOperatorOption = " + mOperatorOption);
                        }
                    }).setPositiveButton("Send",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {

                            /* User clicked Yes so do some stuff */
                            attachOrDetachGprs();
                            sendATCommad(
                                    "4," + String.valueOf(mOperatorOption),
                                    MODEM_OPERATOR);
                            if (mCurrentMode == MODEM_FTA) {
                                setGprsTransferType(PCH_CALL_PREFER);
                            }
                        }
                    }).setNegativeButton("Cancel",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {

                            /* User clicked No so do some stuff */
                            mOperatorOption = 0;
                        }
                    }).create();
        default:
            break;
        }
        return null;
    }

    private void writePreferred(int type) {
        SharedPreferences sh = this.getSharedPreferences("RATMode",
                MODE_WORLD_READABLE);
        SharedPreferences.Editor editor = sh.edit();
        editor.putInt("ModeType", type);
        editor.commit();
    }

    private void attachOrDetachGprs() {
        if ((mOperatorOption & DOCOMO_OPTION) != 0 || (mOperatorOption & SOFTBANK_OPTION) != 0) {
            Elog.v(TAG, "Attach GPRS for DoCoMo/Softband");
            SystemProperties.set("persist.radio.gprs.attach.type", "1");

            String cmdStr[] = { "AT+EGTYPE=1,1", "" };
            mPhone.invokeOemRilRequestStrings(cmdStr, null);

            SharedPreferences preference = getSharedPreferences(PREFERENCE_GPRS, 0);
            SharedPreferences.Editor editor = preference.edit();
            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_ALWAYS);
        } else {
            Elog.v(TAG, "Dettach GPRS for DoCoMo/Softband");
            SystemProperties.set("persist.radio.gprs.attach.type", "0");

            String cmdStr[] = { "AT+EGTYPE=0,1", "" };
            mPhone.invokeOemRilRequestStrings(cmdStr, null);

            SharedPreferences preference = getSharedPreferences(PREFERENCE_GPRS, 0);
            SharedPreferences.Editor editor = preference.edit();
            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_NOT_SPECIFY);
        }
    }

    private void enableIPO(boolean value) {
        Elog.v(TAG, value ? "enableIOP(true)" : "enableIPO(false)");
        Settings.System.putInt(getContentResolver(),
                Settings.System.IPO_SETTING, value ? IPO_ENABLE : IPO_DISABLE);
    }

    private void setGprsTransferType(int type) {
        try {
            ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
            Settings.System.putInt(getContentResolver(), Settings.System.GPRS_TRANSFER_SETTING, type);
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                if (mGeminiPhone != null) {
                    mGeminiPhone.getPhonebyId(PhoneConstants.GEMINI_SIM_1).setGprsTransferType(type, null);
                    mGeminiPhone.getPhonebyId(PhoneConstants.GEMINI_SIM_2).setGprsTransferType(type, null);
                }
            } else {
                if (telephony != null) {
                    telephony.setGprsTransferType(type);
                }
            }
        } catch (RemoteException e) {
            Elog.v(TAG, e.getMessage());
        }
    }
}
