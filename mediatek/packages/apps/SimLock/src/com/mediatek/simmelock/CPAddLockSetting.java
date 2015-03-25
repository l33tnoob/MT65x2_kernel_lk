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

package com.android.simmelock;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccUtils;
import com.android.internal.telephony.PhoneProxy;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

public class CPAddLockSetting extends SimLockBaseActivity {
    private static final int DIALOG_MCCMNCLENGTHINCORRECT = 1;
    private static final int DIALOG_ADDLOCKFAIL = 2;
    private static final int DIALOG_ADDLOCKSUCCEED = 3;
    private static final int DIALOG_PASSWORDLENGTHINCORRECT = 4;
    private static final int DIALOG_PASSWORDWRONG = 5;
    private static final int DIALOG_GID1WRONG = 6;
    private static final int DIALOG_GID2WRONG = 7;
    private static final int ADDLOCK_ICC_SML_COMPLETE = 120;
    private static final int EVENT_GET_SIM_GID1 = 36;
    private static final int EVENT_GET_SIM_GID2 = 37;

    private EditText etMccMnc = null;
    private EditText etGid1 = null;
    private EditText etGid2 = null;
    private EditText etPwd = null;
    private EditText etPwdConfirm = null;
    private Spinner s1;
    private Spinner s2;
    private Spinner s3;

    private String mSimMccMnc = null;
    private String mSimGid1 = null;
    private String mSimGid2 = null;
    private boolean mMccMncReadSim = false;
    private boolean mGid1ReadSim = false;
    private boolean mGid2ReadSim = false;
    private boolean mSimGid1Valid = false;
    private boolean mSimGid2Valid = false;
    private boolean clickFlag = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cpaddlocksetting);

        // set the regulation of EditText
        etMccMnc = (EditText) findViewById(R.id.idcpaddlockEditInputMCCMNC);
        etGid1 = (EditText) findViewById(R.id.idcpaddlockEditInputGID1);
        etGid2 = (EditText) findViewById(R.id.idcpaddlockEditInputGID2);
        etPwd = (EditText) findViewById(R.id.idcpaddlockEditInputPassword);
        // Let the user to choose "put in" for just read MCCMNC from the SIM
        // card
        s1 = (Spinner) findViewById(R.id.spinnercp1);
        s2 = (Spinner) findViewById(R.id.spinnercp2);
        s3 = (Spinner) findViewById(R.id.spinnercp3);
        etPwdConfirm = (EditText) findViewById(R.id.idcpaddlockEditInputPasswordAgain);

        etMccMnc.setOnLongClickListener(mOnLongClickListener);
        etGid1.setOnLongClickListener(mOnLongClickListener);
        etGid2.setOnLongClickListener(mOnLongClickListener);
        etPwd.setOnLongClickListener(mOnLongClickListener);
        etPwdConfirm.setOnLongClickListener(mOnLongClickListener);

        // Press the CONFIRM Button
        Button btnConfirm = (Button) findViewById(R.id.idcpaddlockButtonConfirm);
        Button btnCancel = (Button) findViewById(R.id.idcpaddlockButtonCancel);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.Input_mode,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s1.setAdapter(adapter);
        AdapterView.OnItemSelectedListener l = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (s1.getSelectedItem().toString().equals(getString(R.string.strUserInput))) {
                    etMccMnc.setVisibility(View.VISIBLE);
                    // set the regulation of EditText
                    SMLCommonProcess.limitEditText(etMccMnc, 6);
                    mMccMncReadSim = false;
                } else {
                    mMccMncReadSim = true;
                    etMccMnc.setVisibility(View.GONE);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };
        s1.setOnItemSelectedListener(l);

        ArrayAdapter<CharSequence> adapter2 = ArrayAdapter.createFromResource(this, R.array.Input_mode,
                android.R.layout.simple_spinner_item);
        adapter2.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s2.setAdapter(adapter2);
        AdapterView.OnItemSelectedListener l2 = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (s2.getSelectedItem().toString().equals(getString(R.string.strUserInput))) {
                    etGid1.setVisibility(View.VISIBLE);
                    // set the regulation of EditText
                    SMLCommonProcess.limitEditText(etGid1, 3);
                    mGid1ReadSim = false;
                    mSimGid1Valid = false;
                } else {
                    mGid1ReadSim = true;
                    mSimGid1Valid = false;
                    etGid1.setVisibility(View.GONE);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };
        s2.setOnItemSelectedListener(l2);

        ArrayAdapter<CharSequence> adapter3 = ArrayAdapter.createFromResource(this, R.array.Input_mode,
                android.R.layout.simple_spinner_item);
        adapter3.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s3.setAdapter(adapter3);
        AdapterView.OnItemSelectedListener l3 = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (s3.getSelectedItem().toString().equals(getString(R.string.strUserInput))) {
                    etGid2.setVisibility(View.VISIBLE);
                    // set the regulation of EditText
                    SMLCommonProcess.limitEditText(etGid2, 3);
                    mGid2ReadSim = false;
                    mSimGid2Valid = false;
                } else {
                    mGid2ReadSim = true;
                    mSimGid2Valid = false;
                    etGid2.setVisibility(View.GONE);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };
        s3.setOnItemSelectedListener(l3);

        etPwd.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        SMLCommonProcess.limitEditTextPassword(etPwd, 8);

        etPwdConfirm.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        SMLCommonProcess.limitEditTextPassword(etPwdConfirm, 8);

        // Yu for ICS
        btnConfirm.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View view) {
                log("clickFlag: " + clickFlag);
                if (clickFlag) {
                    return;
                } else {
                    clickFlag = true;
                }
                if ((!mMccMncReadSim)
                        && ((5 > etMccMnc.getText().length()) || (6 < etMccMnc.getText().length()))) {
                    showDialog(DIALOG_MCCMNCLENGTHINCORRECT);
                } else if (mMccMncReadSim
                        && ((mSimMccMnc == null) || (5 > mSimMccMnc.length()) || (6 < mSimMccMnc.length()))) {
                    showDialog(DIALOG_MCCMNCLENGTHINCORRECT);
                } else if ((!mGid1ReadSim) && etGid1.getText().length() < 1) {
                    showDialog(DIALOG_GID1WRONG);
                } else if ((!mGid2ReadSim) && etGid2.getText().length() < 1) {
                    showDialog(DIALOG_GID2WRONG);
                } else if ((!mGid1ReadSim) && ((Integer.parseInt(etGid1.getText().toString()) < 0)
                        || (Integer.parseInt(etGid1.getText().toString()) > 254))) {
                    showDialog(DIALOG_GID1WRONG);
                } else if ((!mGid2ReadSim) && ((Integer.parseInt(etGid2.getText().toString()) < 0)
                        || (Integer.parseInt(etGid2.getText().toString()) > 254))) {
                    showDialog(DIALOG_GID2WRONG);
                } else if (mGid1ReadSim && !mSimGid1Valid) {
                    showDialog(DIALOG_GID1WRONG);
                } else if (mGid2ReadSim && !mSimGid2Valid) {
                    showDialog(DIALOG_GID2WRONG);
                } else if ((etPwd.getText().length() < 4) || ((etPwd.getText().length() > 8))) {
                    showDialog(DIALOG_PASSWORDLENGTHINCORRECT);
                } else if (!etPwd.getText().toString().equals(etPwdConfirm.getText().toString())) {
                    showDialog(DIALOG_PASSWORDWRONG);
                } else {
                    Message callback = Message.obtain(mHandler, ADDLOCK_ICC_SML_COMPLETE);
                    setIccNetworkLockEnabled(3, 2, etPwd.getText().toString(),
                            mMccMncReadSim ? mSimMccMnc : etMccMnc.getText().toString(),
                            mGid1ReadSim ? mSimGid1 : etGid1.getText().toString(),
                            mGid2ReadSim ? mSimGid2 : etGid2.getText().toString(), callback);
                }
            }
        });

        // Press the CANCEL Button
        btnCancel.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View view) {
                // do nothing to quit the edit page
                CPAddLockSetting.this.finish();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!isSimReady()) {
            log("Add NP lock fail : SIM not ready!");
            return;
        }

        // To get the MCC+MNC+GID1+GID2 from SIM card
        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            mSimMccMnc = telephonyManager.getSimOperator();

            Phone phone = PhoneFactory.getDefaultPhone();
            IccFileHandler iccFh = ((PhoneProxy) phone).getIccFileHandler();
            iccFh.loadEFTransparent(IccConstants.EF_GID1, mHandler.obtainMessage(EVENT_GET_SIM_GID1));
            iccFh.loadEFTransparent(IccConstants.EF_GID2, mHandler.obtainMessage(EVENT_GET_SIM_GID2));
        } else {
            TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
            mSimMccMnc = telephonyManagerEx.getSimOperator(simNumber);

            GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            IccFileHandler iccFh = ((PhoneProxy)(mGeminiPhone.getPhonebyId(simNumber))).getIccFileHandler();
            iccFh.loadEFTransparent(IccConstants.EF_GID1, mHandler.obtainMessage(EVENT_GET_SIM_GID1));
            iccFh.loadEFTransparent(IccConstants.EF_GID2, mHandler.obtainMessage(EVENT_GET_SIM_GID2));

            // To get the GID2 from SIM card //TEMP
            mSimGid2 = telephonyManagerEx.getSimOperatorName(simNumber);
            if (mSimGid2 == null) {
                log("Fail to read SIM GID2!");
            } else {
                log("[Gemini]Succeed to read SIM GID2. SIM GID2 is " + mSimGid2);
            }
        }

        if (mSimMccMnc == null) {
            log("Fail to read SIM MCC+MNC!");
        } else {
            log("Read SIM MCC+MNC: " + mSimMccMnc);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this)
            .setCancelable(false)
            .setTitle(R.string.strAttention)
            .setIcon(android.R.drawable.ic_dialog_alert)
            .setOnKeyListener(this);

        switch (id) {
        case DIALOG_ADDLOCKFAIL: // Fail
            builder.setMessage(R.string.strAddLockFail).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            etGid1.setText("");
                            etGid2.setText("");
                            etMccMnc.setText("");
                            etPwd.setText("");
                            etPwdConfirm.setText("");
                            dialog.cancel();
                            clickFlag = false;
                        }
                    });
            return builder.create();
        case DIALOG_PASSWORDLENGTHINCORRECT:// Length is incorrect
            builder.setMessage(R.string.strPasswordLengthIncorrect).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            etPwd.setText("");
                            etPwdConfirm.setText("");
                            dialog.cancel();
                            clickFlag = false;
                        }
                    });
            return builder.create();
        case DIALOG_MCCMNCLENGTHINCORRECT:// Length is incorrect
            builder.setMessage(R.string.strMCCMNCLengthIncorrect).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            etMccMnc.setText("");
                            dialog.cancel();
                            clickFlag = false;
                        }
                    });
            return builder.create();
        case DIALOG_ADDLOCKSUCCEED:// Succeed
            builder.setMessage(R.string.strAddLockSucceed).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            dialog.cancel();
                            clickFlag = false;
                            CPAddLockSetting.this.finish();
                        }
                    });
            return builder.create();
        case DIALOG_GID1WRONG:// Wrong GID1
            builder.setMessage(R.string.strGID1WRONG).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            etGid1.setText("");
                            dialog.cancel();
                            clickFlag = false;
                        }
                    });
            return builder.create();
        case DIALOG_GID2WRONG:// Wrong GID2
            builder.setMessage(R.string.strGID2WRONG).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            etGid2.setText("");
                            dialog.cancel();
                            clickFlag = false;
                        }
                    });
            return builder.create();
        case DIALOG_PASSWORDWRONG:// Wrong password
            builder.setMessage(R.string.str_simme_passwords_dont_match).setPositiveButton(R.string.strYes,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            etPwd.setText("");
                            etPwdConfirm.setText("");
                            dialog.cancel();
                            clickFlag = false;
                        }
                    });
            return builder.create();
        default:
            break;
        }
        return super.onCreateDialog(id);
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case ADDLOCK_ICC_SML_COMPLETE:
                if (ar.exception != null) {
                    showDialog(DIALOG_ADDLOCKFAIL);
                } else {
                    showDialog(DIALOG_ADDLOCKSUCCEED);
                }
                break;
            case EVENT_GET_SIM_GID1:
                if (ar.exception != null) {
                    log("fail to get SIM GID1");
                    mSimGid1Valid = false;
                } else {
                    log("succeed to get SIM GID1");
                    byte[] data = (byte[]) (ar.result);
                    log("SIM GID :" + data);
                    String hexSIMGID1 = IccUtils.bytesToHexString(data);
                    if ((data[0] & 0xff) == 0xff) {
                        log("SIM GID1 not initialized");
                        mSimGid1Valid = false;
                    } else {
                        mSimGid1Valid = true;
                        if (hexSIMGID1.length() >= 2) {
                            mSimGid1 = String.valueOf(Integer.parseInt(hexSIMGID1.substring(0, 2)));
                        } else {
                            mSimGid1 = String.valueOf(Integer.parseInt(hexSIMGID1));
                        }
                        log("Normal SIM GID1 :" + mSimGid1);
                    }
                }
                break;
            case EVENT_GET_SIM_GID2:
                if (ar.exception != null) {
                    log("fail to get SIM GID2");
                    mSimGid2Valid = false;
                } else {
                    log("succeed to get SIM GID2");
                    byte[] data = (byte[]) (ar.result);
                    log("SIM GID2 :" + data);
                    String hexSIMGID2 = IccUtils.bytesToHexString(data);
                    if ((data[0] & 0xff) == 0xff) {
                        log("SIM GID2 not initialized");
                        mSimGid2Valid = false;
                    } else {
                        mSimGid2Valid = true;
                        if (hexSIMGID2.length() >= 2) {
                            mSimGid2 = String.valueOf(Integer.parseInt(hexSIMGID2.substring(0, 2)));
                        } else {
                            mSimGid2 = String.valueOf(Integer.parseInt(hexSIMGID2));
                        }
                        log("Normal SIM GID2 :" + mSimGid2);
                    }
                }
                break;
            default:
                break;
            }
        }
    };
}
