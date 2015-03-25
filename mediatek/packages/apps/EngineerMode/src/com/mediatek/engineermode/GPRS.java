/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.common.featureoption.FeatureOption;

public class GPRS extends Activity implements OnClickListener {

    String mContextCmdStringArray[] = {
            "3,128,128,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,256,256,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,256,256,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,256,256,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,128,128,0,0,1,1500,\"1e6\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e6\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e6\",\"1e5\",0,0,0",
            "3,128,128,0,0,1,1500,\"1e4\",\"1e5\",0,0,0",
            "3,256,256,0,0,1,1500,\"1e3\",\"4e3\",1,0,0",
            "3,512,512,0,0,1,1500,\"1e4\",\"1e5\",0,0,0" };

    private static final int EVENT_GPRS_ATTACHED = 1;
    private static final int EVENT_GPRS_DETACHED = 2;
    private static final int EVENT_PDP_ACTIVATE = 3;
    private static final int EVENT_PDP_DEACTIVATE = 4;
    private static final int EVENT_SEND_DATA = 5;
    private static final int EVENT_GPRS_INTERNAL_AT = 6;
    private static final int EVENT_WRITE_IMEI = 7;
    private static final int EVENT_GPRS_FD = 8;
    private static final int EVENT_GPRS_ATTACH_TYPE = 9;

    private static final int SCRI_DEFAULT_TIMEOUT = 20;

    private static final String PROPERTY_PDN_REUSE = "ril.pdn.reuse";

    private boolean mFlag = true;

    // private static final String[] ActivateAT = null;

    private Phone mPhone = null;

    static final String LOG_TAG = "GPRS EN";

    private Button mBtnSim1;
    private Button mBtnSim2;
    private Button mBtnImei;
    private TextView mTextDefSIMSelect;
    private RadioGroup mRaGpDefSIMSelect;
    private RadioButton mRaBtnSIM1Enabled;
    private RadioButton mRaBtnSIM2Enabled;
    private Button mBtnAttached;
    private Button mBtnDetached;
    private Button mBtnAttachedContinue;
    private Button mBtnDetachedContinue;
    private Button mBtnNotSpecify;
    private RadioGroup mRaGpPDPSelect;
    private RadioGroup mRaGpUsageSelect;
    private Spinner mSPinnerPDPContext;
    private Button mBtnActivate;
    private Button mBtnDeactivate;
    private EditText mEditDataLen;
    private Button mBtnSendData;
    private RadioGroup mGprstAttachSelect;
    private Button mBtnPdnReuseOn;
    private Button mBtnPdnReuseOff;
    //csd
    private Button mBtnDialUpCsd;

    private EditText mEditImeiValue;
    //csd
    private EditText mCsdNum;
    private EditText mCsdSlot;

    private int mPDPSelect = 0;
    private int mUsageSelect = 0;
    private int mPDPContextIndex = 0;

    private ArrayAdapter<String> mSpinnerAdapter;

    private boolean mAlive = false;

    private AlertDialog mAlertDialog;

    public static final String PREFERENCE_GPRS = "com.mtk.GPRS";
    public static final String PREF_ATTACH_MODE = "ATTACH_MODE";
    public static final String PREF_ATTACH_MODE_SIM = "ATTACH_MODE_SIM";
    public static final int ATTACH_MODE_ALWAYS = 1;
    public static final int ATTACH_MODE_WHEN_NEEDED = 0;
    public static final int ATTACH_MODE_NOT_SPECIFY = -1;

    private static final int PDP_CONTEXT_MAX = 15;

    @Override
    public void onDestroy() {
        mAlive = false;
        super.onDestroy();
        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            mAlertDialog.dismiss();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gprs);

        Log.v(LOG_TAG, "onCreate");
        mAlive = true;

        mPhone = PhoneFactory.getDefaultPhone();

        // create ArrayAdapter for Spinner
        mSpinnerAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mSpinnerAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 1; i < PDP_CONTEXT_MAX; i++) {
            mSpinnerAdapter.add("PDP Context " + String.valueOf(i));
        }
        mSpinnerAdapter.add("PDP Context 30");
        mSpinnerAdapter.add("PDP Context 31");

        // get the object of the controls
        mBtnSim1 = (Button) findViewById(R.id.Sim1);
        mBtnSim2 = (Button) findViewById(R.id.Sim2);
        mBtnSim1.setOnClickListener(this);
        mBtnSim2.setOnClickListener(this);
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            mBtnSim1.setVisibility(View.GONE);
            mBtnSim2.setVisibility(View.GONE);
        }

        String imei = TelephonyManager.getDefault().getDeviceId();
        // TextUtils.empty
        Log.v(LOG_TAG, "Default IMEI:" + imei);
        mEditImeiValue = (EditText) findViewById(R.id.IMEI_VALUE);
        mEditImeiValue.setText(imei);

        mBtnImei = (Button) findViewById(R.id.IMEI);
        mBtnImei.setOnClickListener(this);

        // remove IMEI item
        mEditImeiValue.setEnabled(false);
        mBtnImei.setVisibility(View.INVISIBLE);

        if (FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_BSP_PACKAGE) {
            mRaGpDefSIMSelect = (RadioGroup) findViewById(R.id.DefSIM);
            mTextDefSIMSelect = (TextView) findViewById(R.id.DefSIMText);
            mRaBtnSIM1Enabled = (RadioButton) findViewById(R.id.SIM1Enabled);
            mRaBtnSIM2Enabled = (RadioButton) findViewById(R.id.SIM2Enabled);

            mTextDefSIMSelect.setVisibility(View.VISIBLE);
            mRaGpDefSIMSelect.setVisibility(View.VISIBLE);
            mRaBtnSIM1Enabled.setVisibility(View.VISIBLE);
            mRaBtnSIM2Enabled.setVisibility(View.VISIBLE);

            int defaultSim = SystemProperties.getInt(
                    PhoneConstants.GEMINI_DEFAULT_SIM_PROP, -1);
            if (defaultSim == -1) {
                defaultSim = PhoneConstants.GEMINI_SIM_1;
                mRaBtnSIM1Enabled.toggle();
            } else if (defaultSim == PhoneConstants.GEMINI_SIM_2) {
                mRaBtnSIM2Enabled.toggle();
            } else {
                mRaBtnSIM1Enabled.toggle();
            }

            mRaGpDefSIMSelect
                    .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                        public void onCheckedChanged(RadioGroup arg0, int arg1) {
                            try {
                                ITelephony iTelephony = ITelephony.Stub
                                        .asInterface(ServiceManager
                                                .getService("phone"));
                                if (arg0.getCheckedRadioButtonId() == R.id.SIM1Enabled) {
                                    iTelephony
                                            .setDefaultPhone(PhoneConstants.GEMINI_SIM_1);
                                    SystemProperties
                                            .set(PhoneConstants.GEMINI_DEFAULT_SIM_PROP,
                                                    String
                                                            .valueOf(PhoneConstants.GEMINI_SIM_1));
                                }
                                if (arg0.getCheckedRadioButtonId() == R.id.SIM2Enabled) {
                                    iTelephony
                                            .setDefaultPhone(PhoneConstants.GEMINI_SIM_2);
                                    SystemProperties
                                            .set(PhoneConstants.GEMINI_DEFAULT_SIM_PROP,
                                                    String
                                                            .valueOf(PhoneConstants.GEMINI_SIM_2));
                                }
                                rebootAlert();
                            } catch (RemoteException e) {
                                Log
                                        .e(LOG_TAG,
                                                "RemoteException in ITelephony.Stub.asInterface");
                            }
                        }
                    });
        }

        mBtnAttached = (Button) findViewById(R.id.Attached);
        mBtnDetached = (Button) findViewById(R.id.Detached);
        mBtnAttachedContinue = (Button) findViewById(R.id.always_mode_continue);
        mBtnDetachedContinue = (Button) findViewById(R.id.when_needed_continue);
        mBtnNotSpecify = (Button) findViewById(R.id.not_specify);

        mRaGpPDPSelect = (RadioGroup) findViewById(R.id.PDPSelect);
        mRaGpUsageSelect = (RadioGroup) findViewById(R.id.UsageSelect);
        mSPinnerPDPContext = (Spinner) findViewById(R.id.ContextNumber);
        mBtnActivate = (Button) findViewById(R.id.Activate);
        mBtnDeactivate = (Button) findViewById(R.id.Deactivate);
        mEditDataLen = (EditText) findViewById(R.id.DataLength);
        mBtnSendData = (Button) findViewById(R.id.SendData);
        //csd
        mBtnDialUpCsd = (Button) findViewById(R.id.DialUpCsd );

        mGprstAttachSelect = (RadioGroup) findViewById(R.id.GprsAttachType);

        mBtnPdnReuseOn = (Button) findViewById(R.id.pdn_reuse_on);
        mBtnPdnReuseOff = (Button) findViewById(R.id.pdn_reuse_off);

        // setOnClickListener for the controls
        mBtnAttached.setOnClickListener(this);
        mBtnDetached.setOnClickListener(this);
        mBtnAttachedContinue.setOnClickListener(this);
        mBtnDetachedContinue.setOnClickListener(this);
        mBtnNotSpecify.setOnClickListener(this);
        mSPinnerPDPContext.setAdapter(mSpinnerAdapter);
        mBtnActivate.setOnClickListener(this);
        mBtnDeactivate.setOnClickListener(this);
        mBtnSendData.setOnClickListener(this);
        mBtnPdnReuseOn.setOnClickListener(this);
        mBtnPdnReuseOff.setOnClickListener(this);
        mBtnDialUpCsd.setOnClickListener(this);

        //csd
        mCsdNum = (EditText) findViewById(R.id.CSD_NUM);
        //mCsdNum.setText("117");
        mCsdSlot = (EditText) findViewById(R.id.CSD_SLOT);
        mCsdSlot.setText("0");        


        mSPinnerPDPContext
                .setOnItemSelectedListener(new OnItemSelectedListener() {

                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        // TODO Auto-generated method stub
                        mPDPContextIndex = arg2;
                    }

                    public void onNothingSelected(AdapterView<?> arg0) {
                        // TODO Auto-generated method stub

                    }

                });

        mGprstAttachSelect
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    public void onCheckedChanged(RadioGroup arg0, int arg1) {
                        // TODO Auto-generated method stub
                        if (arg0.getCheckedRadioButtonId() == R.id.GprsAlwaysAttach) {

                            SystemProperties.set(
                                    "persist.radio.gprs.attach.type", "1");
                            String cmdStr[] = { "AT+EGTYPE=1", "" };
                            mPhone
                                    .invokeOemRilRequestStrings(
                                            cmdStr,
                                            mResponseHander
                                                    .obtainMessage(EVENT_GPRS_ATTACH_TYPE));
                        }
                        if (arg0.getCheckedRadioButtonId() == R.id.GprsWhenNeeded) {
                            SystemProperties.set(
                                    "persist.radio.gprs.attach.type", "0");
                            String cmdStr[] = { "AT+EGTYPE=0", "" };
                            mPhone
                                    .invokeOemRilRequestStrings(
                                            cmdStr,
                                            mResponseHander
                                                    .obtainMessage(EVENT_GPRS_ATTACH_TYPE));
                        }
                    }
                });

        mRaGpPDPSelect
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    public void onCheckedChanged(RadioGroup arg0, int arg1) {
                        if (arg0.getCheckedRadioButtonId() == R.id.FirstPDP) {
                            mPDPSelect = 0;
                            mRaGpUsageSelect.clearCheck();
                            mRaGpUsageSelect.getChildAt(0).setEnabled(false);
                            mRaGpUsageSelect.getChildAt(1).setEnabled(false);
                        }
                        if (arg0.getCheckedRadioButtonId() == R.id.SecondPDP) {
                            mPDPSelect = 1;
                            mRaGpUsageSelect.check(R.id.Primary);
                            mRaGpUsageSelect.getChildAt(0).setEnabled(true);
                            mRaGpUsageSelect.getChildAt(1).setEnabled(true);
                        }
                    }
                });

        mRaGpUsageSelect
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    public void onCheckedChanged(RadioGroup arg0, int arg1) {
                        if (arg0.getCheckedRadioButtonId() == R.id.Primary) {
                            mUsageSelect = 0;
                        }
                        if (arg0.getCheckedRadioButtonId() == R.id.Secondary) {
                            mUsageSelect = 1;
                        }
                    }
                });

        // set initial value for PDP select
        mRaGpPDPSelect.check(R.id.FirstPDP);

        int gprsAttachType = SystemProperties.getInt(
                "persist.radio.gprs.attach.type", 1);

        if (gprsAttachType == 1) {
            mGprstAttachSelect.check(R.id.GprsAlwaysAttach);
        } else {
            mGprstAttachSelect.check(R.id.GprsWhenNeeded);
        }

        showDefaultSim();

        AlertDialog.Builder builder = new AlertDialog.Builder(GPRS.this);
        mAlertDialog = builder.create();
        mAlertDialog.setButton(DialogInterface.BUTTON_POSITIVE, "OK" , (DialogInterface.OnClickListener)null);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateAttachModeMMI();

        if ("0".equals(SystemProperties.get(PROPERTY_PDN_REUSE, "1"))) {
            mBtnPdnReuseOn.setEnabled(true);
            mBtnPdnReuseOff.setEnabled(false);
        } else {
            mBtnPdnReuseOn.setEnabled(false);
            mBtnPdnReuseOff.setEnabled(true);
        }
    }

    @Override
    public void onClick(View arg0) {
        Log.v(LOG_TAG, "onClick:" + arg0.getId());

        if (arg0.getId() == mBtnImei.getId()) {
            String imeiString[] = { "AT+EGMR=1,", "" };
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                int simId = mPhone.getMySimId();
                if (simId == PhoneConstants.GEMINI_SIM_1) {
                    imeiString[0] = "AT+EGMR=1,7,\"" + mEditImeiValue.getText()
                            + "\"";
                } else if (simId == PhoneConstants.GEMINI_SIM_2) {
                    imeiString[0] = "AT+EGMR=1,10,\""
                            + mEditImeiValue.getText() + "\"";
                }
            } else {
                imeiString[0] = "AT+EGMR=1,7,\"" + mEditImeiValue.getText()
                        + "\"";
            }

            Log.v(LOG_TAG, "IMEI String:" + imeiString[0]);
            mPhone.invokeOemRilRequestStrings(imeiString, mResponseHander
                    .obtainMessage(EVENT_WRITE_IMEI));
        }

        if (arg0 == mBtnSim1) {
            try {
                ITelephony iTelephony = ITelephony.Stub
                        .asInterface(ServiceManager.getService("phone"));
                if (iTelephony == null) {
                    Log.e(LOG_TAG, "clocwork worked...");
                    // not return and let exception happened.
                }

                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    iTelephony.setDefaultPhone(PhoneConstants.GEMINI_SIM_1);
                }

                mPhone = PhoneFactory.getDefaultPhone();
                Log.v(LOG_TAG, "SIM 1");
                showDefaultSim();
                String imei = mPhone.getDeviceId();
                mEditImeiValue.setText(imei);
                Log.e(LOG_TAG, "IMEI 1: " + imei);
            } catch (RemoteException e) {
                Log
                        .e(LOG_TAG,
                                "RemoteException in ITelephony.Stub.asInterface");
            }
        }

        if (arg0 == mBtnSim2) {
            try {
                ITelephony iTelephony = ITelephony.Stub
                        .asInterface(ServiceManager.getService("phone"));
                if (iTelephony == null) {
                    Log.e(LOG_TAG, "clocwork worked...");
                    // not return and let exception happened.
                }
                iTelephony.setDefaultPhone(PhoneConstants.GEMINI_SIM_2);
                mPhone = PhoneFactory.getDefaultPhone();
                Log.v(LOG_TAG, "SIM 2");
                showDefaultSim();
                String imei = mPhone.getDeviceId();
                mEditImeiValue.setText(imei);
                Log.e(LOG_TAG, "IMEI 2: " + imei);

            } catch (RemoteException e) {
                Log
                        .e(LOG_TAG,
                                "RemoteException in ITelephony.Stub.asInterface");
            }
        }

        if (arg0.getId() == mBtnAttached.getId()) {
            String attachedAT[] = { "AT+CGATT=1", "" };
            mPhone.invokeOemRilRequestStrings(attachedAT, mResponseHander
                    .obtainMessage(EVENT_GPRS_ATTACHED));
        }

        if (arg0.getId() == mBtnDetached.getId()) {
            String detachedAT[] = { "AT+CGATT=0", "" };
            mPhone.invokeOemRilRequestStrings(detachedAT, mResponseHander
                    .obtainMessage(EVENT_GPRS_DETACHED));
        }

        SharedPreferences preference = getSharedPreferences(PREFERENCE_GPRS, 0);
        SharedPreferences.Editor editor = preference.edit();

        if (arg0 == mBtnAttachedContinue) {
            SystemProperties.set("persist.radio.gprs.attach.type", "1");
            String cmdStr[] = { "AT+EGTYPE=1,1", "" };
            mPhone.invokeOemRilRequestStrings(cmdStr, mResponseHander
                    .obtainMessage(EVENT_GPRS_ATTACH_TYPE));

            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_ALWAYS);
        } else if (arg0 == mBtnDetachedContinue) {
            SystemProperties.set("persist.radio.gprs.attach.type", "0");
            String cmdStr[] = { "AT+EGTYPE=0,1", "" };
            mPhone.invokeOemRilRequestStrings(cmdStr, mResponseHander
                    .obtainMessage(EVENT_GPRS_ATTACH_TYPE));

            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_WHEN_NEEDED);
        } else if (arg0 == mBtnNotSpecify) {
            editor.putInt(PREF_ATTACH_MODE, ATTACH_MODE_NOT_SPECIFY);
        }

        editor.putInt(PREF_ATTACH_MODE_SIM, mPhone.getMySimId());
        editor.commit();
        updateAttachModeMMI();

        if (arg0.getId() == mBtnActivate.getId()) {
            mFlag = true;

            if (0 == mPDPSelect) {
                String activateAT[] = { "AT+CGQMIN=1", "" };
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+CGQREQ=1";
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+CGDCONT=1,\"IP\",\"internet\",\"192.168.1.1\",0,0";
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+CGEQREQ=1,"
                        + mContextCmdStringArray[mPDPContextIndex];
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+ACTTEST=1,1";
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_PDP_ACTIVATE));
            }
            if (1 == mPDPSelect) {
                String activateAT[] = { "AT+CGQMIN=2", "" };
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+CGQREQ=2";
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                if (0 == mUsageSelect) {
                    activateAT[0] = "AT+CGDCONT=2,\"IP\",\"internet\",\"192.168.1.1\",0,0";
                }
                if (1 == mUsageSelect) {
                    activateAT[0] = "AT+CGDSCONT=2,1,0,0";
                }
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+CGEQREQ=2,"
                        + mContextCmdStringArray[mPDPContextIndex];
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_GPRS_INTERNAL_AT));
                activateAT[0] = "AT+ACTTEST=1,2";
                activateAT[1] = "";
                mPhone.invokeOemRilRequestStrings(activateAT, mResponseHander
                        .obtainMessage(EVENT_PDP_ACTIVATE));

            }
        }

        if (arg0.getId() == mBtnDeactivate.getId()) {
            mFlag = true;

            String deactivateAT[] = new String[2];
            if (0 == mPDPSelect) {
                deactivateAT[0] = "AT+ACTTEST=0,1";
            }
            if (1 == mPDPSelect) {
                deactivateAT[0] = "AT+ACTTEST=0,2";
            }
            deactivateAT[1] = "";
            mPhone.invokeOemRilRequestStrings(deactivateAT, mResponseHander
                    .obtainMessage(EVENT_PDP_DEACTIVATE));

        }

        if (arg0.getId() == mBtnSendData.getId()) {
            String strDataLength = mEditDataLen.getText().toString();
            String sendDataAT[] = new String[2];
            if (0 == mPDPSelect) {
                sendDataAT[0] = "AT+CGSDATA=" + strDataLength + ",1";
            }
            if (1 == mPDPSelect) {
                sendDataAT[0] = "AT+CGSDATA=" + strDataLength + ",2";
            }
            sendDataAT[1] = "";
            mPhone.invokeOemRilRequestStrings(sendDataAT, mResponseHander
                    .obtainMessage(EVENT_SEND_DATA));
        }

        if (arg0.getId() == mBtnPdnReuseOn.getId()) {
            SystemProperties.set(PROPERTY_PDN_REUSE, "1");
            mBtnPdnReuseOn.setEnabled(false);
            mBtnPdnReuseOff.setEnabled(true);
        } else if (arg0.getId() == mBtnPdnReuseOff.getId()) {
            SystemProperties.set(PROPERTY_PDN_REUSE, "0");
            mBtnPdnReuseOn.setEnabled(true);
            mBtnPdnReuseOff.setEnabled(false);
        }

        if (arg0.getId() == mBtnDialUpCsd.getId()){
            Log.v(LOG_TAG, "DialCsd:");            
            String num = mCsdNum.getText().toString();
            int slot = 0;
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                slot = Integer.parseInt(mCsdSlot.getText().toString());
                if (slot < 0 || slot > 3) slot = 0;                
            } 
            try {
                ITelephony iTelephony = ITelephony.Stub
                        .asInterface(ServiceManager.getService("phone"));
                
                iTelephony.dialUpCsd(slot,num);
                
             } catch(RemoteException e) {
                    e.printStackTrace();
             }                 
        }
        
    }

    private Handler mResponseHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            if (!mAlive) {
                return;
            }

            if (mAlertDialog.isShowing()) {
                mAlertDialog.dismiss();
            }

            AsyncResult ar;
            switch (msg.what) {
                case EVENT_GPRS_ATTACHED:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("GPRS Attached");
                        mAlertDialog.setMessage("GPRS Attached succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("GPRS Attached");
                        mAlertDialog.setMessage("GPRS Attache failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_GPRS_DETACHED:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("GPRS Detached");
                        mAlertDialog.setMessage("GPRS Detached succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("GPRS Detached");
                        mAlertDialog.setMessage("GPRS Detached failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_GPRS_FD:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("GPRS Fast Dormancy");
                        mAlertDialog.setMessage("GPRS Fast Dormancy command succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("GPRS Fast Dormancy");
                        mAlertDialog.setMessage("GPRS Fast Dormancy command failed.");
                        mAlertDialog.show();
                    }                
                    break;
                case EVENT_PDP_ACTIVATE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null && mFlag) {
                        mAlertDialog.setTitle("PDP Activate");
                        mAlertDialog.setMessage("PDP Activate succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("PDP Activate");
                        mAlertDialog.setMessage("PDP Activate failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_PDP_DEACTIVATE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("PDP Deactivate");
                        mAlertDialog.setMessage("PDP Deactivate succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("PDP Deactivate");
                        mAlertDialog.setMessage("PDP Deactivate failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_SEND_DATA:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("Send Data");
                        mAlertDialog.setMessage("Send Data succeeded.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("Send Data");
                        mAlertDialog.setMessage("Send Data failed.");
                        mAlertDialog.show();
                    }
                    break;
                case EVENT_GPRS_INTERNAL_AT:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        mFlag = false;
                    }
                    break;
                case EVENT_WRITE_IMEI:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mAlertDialog.setTitle("IMEI WRITE");
                        mAlertDialog.setMessage("The IMEI is writen successfully.");
                        mAlertDialog.show();
                    } else {
                        mAlertDialog.setTitle("IMEI WRITE");
                        mAlertDialog.setMessage("Fail to write IMEI due to radio unavailable or something else.");
                        mAlertDialog.show();
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private void showDefaultSim() {
        mPhone = PhoneFactory.getDefaultPhone();
        int simId = mPhone.getMySimId();

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (simId == PhoneConstants.GEMINI_SIM_1) {
                mBtnSim1.setEnabled(false);
                mBtnSim2.setEnabled(true);
            } else if (simId == PhoneConstants.GEMINI_SIM_2) {
                mBtnSim1.setEnabled(true);
                mBtnSim2.setEnabled(false);
            }
        }

    }

    private void rebootAlert() {
        DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                    pm.reboot("");
                    Toast.makeText(getApplicationContext(),
                            "Restart Device...", Toast.LENGTH_LONG).show();
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    break;
                default:
                    break;
                }
            }
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Reboot?").setPositiveButton("Yes",
                dialogClickListener).setNegativeButton("No",
                dialogClickListener).setTitle("Warning").show();
    }

    private void updateAttachModeMMI() {
        SharedPreferences preference = getSharedPreferences(PREFERENCE_GPRS, 0);
        int attachMode = preference.getInt(PREF_ATTACH_MODE,
                ATTACH_MODE_NOT_SPECIFY);

        switch (attachMode) {
        case ATTACH_MODE_ALWAYS:
            mBtnAttachedContinue.setEnabled(false);
            mBtnDetachedContinue.setEnabled(true);
            mBtnNotSpecify.setEnabled(true);
            break;
        case ATTACH_MODE_WHEN_NEEDED:
            mBtnAttachedContinue.setEnabled(true);
            mBtnDetachedContinue.setEnabled(false);
            mBtnNotSpecify.setEnabled(true);
            break;
        case ATTACH_MODE_NOT_SPECIFY:
            mBtnAttachedContinue.setEnabled(true);
            mBtnDetachedContinue.setEnabled(true);
            mBtnNotSpecify.setEnabled(false);
            break;
        default:
            break;
        }
    }
}
