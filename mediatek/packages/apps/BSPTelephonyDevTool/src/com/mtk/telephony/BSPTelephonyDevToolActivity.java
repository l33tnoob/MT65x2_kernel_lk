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

package com.mtk.telephony;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.ITelephony;
import com.mediatek.common.telephony.ITelephonyEx;
import android.os.SystemProperties;
import android.os.ServiceManager;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.gemini.GeminiPhone;

public class BSPTelephonyDevToolActivity extends Activity {
    private static final String LOG_TAG = "BSP_Telephony_Dev";

    private Button[] mDefaultSim = new Button[PhoneConstants.GEMINI_SIM_NUM];
    private TextView[] mOperatorSim = new TextView[PhoneConstants.GEMINI_SIM_NUM];
    private Button mStatusbarNotification;
    private Button[] mDefaultDataSim = new Button[PhoneConstants.GEMINI_SIM_NUM];
    private Button mDefaultDataOff;

    private TextView[] mNetworkTypeSim = new TextView[PhoneConstants.GEMINI_SIM_NUM];
    private ProgressBar[] mSignalSim = new ProgressBar[PhoneConstants.GEMINI_SIM_NUM];

    private TextView[] mDataConnectionTypeSim = new TextView[PhoneConstants.GEMINI_SIM_NUM];
    private TextView[] mDataActivitySim = new TextView[PhoneConstants.GEMINI_SIM_NUM];

    private Button[] m3GSim = new Button[PhoneConstants.GEMINI_SIM_NUM];
    private Button m3GSimOff;

    private Button mFlightModeTestStart;
    private Button mFlightModeTestStop;
    private boolean mFlightModeTestEnable = false;
    private TextView mFlightModeTestCounterText;
    private TextView mFlightModeTestTimerText;
    private int mFlightModeTestCounter = 0;
    private int mFlightModeTestTimer = 0;
    private Handler mFlightModeHandler = new Handler();  
    private TelephonyManager mTelephonyManager;
    private TelephonyManagerEx mTelephonyManagerEx;
    private ITelephony mTelephony;
    private ITelephonyEx mTelephonyEx;
    private Switch3GHandler mSwitch3GHandler;

    private int mDefaultSimId;
    private boolean mIsStatusBarNotificationEnabled;
    private int mDefaultDataSimId = -1;
    // for hot plug tool
    private static final String AT_CMD_SIM_PLUG_OUT = "AT+ESIMTEST=17";
    private static final String AT_CMD_SIM_PLUG_IN = "AT+ESIMTEST=18";
    private static final String AT_CMD_SIM_PLUG_IN_ALL = "AT+ESIMTEST=19";
    private static final String AT_CMD_SIM_MISSING = "AT+ESIMTEST=65";
    private static final String AT_CMD_SIM_RECOVERY = "AT+ESIMTEST=66";
    private static final int MAX_GEMINI_SIM_NUM = 4;
    private Phone mPhone;
    private static Button[] mPlugOutSim = new Button[MAX_GEMINI_SIM_NUM];
    private static Button[] mPlugInSim = new Button[MAX_GEMINI_SIM_NUM];
    private static Button[] mMissingSim = new Button[MAX_GEMINI_SIM_NUM];
    private static Button[] mRecoverySim = new Button[MAX_GEMINI_SIM_NUM];
    private Button mPlugOutAllSims;
    private Button mPlugInAllSims;

    private GeminiPhoneStateListener[] mPhoneStateListener = new GeminiPhoneStateListener[PhoneConstants.GEMINI_SIM_NUM];

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd("received intent: " + action);
            if (action.equals(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED)) {
                long simInfoId = intent.getLongExtra(PhoneConstants.MULTI_SIM_ID_KEY, Settings.System.DEFAULT_SIM_NOT_SET);
                SimInfoRecord simInfo = SimInfoManager.getSimInfoById(BSPTelephonyDevToolActivity.this, simInfoId);
                if (simInfo == null) {
                    mDefaultDataSimId = -1;
                } else {
                    mDefaultDataSimId = simInfo.mSimSlotId;
                }
                logd( "Receive ACTION_DATA_DEFAULT_SIM_CHANGED, data sim: " + mDefaultDataSimId);
                updateUI();
            } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                //for Flight Mode Test
                ServiceState ss = ServiceState.newFromBundle(intent.getExtras());
                logd("get ServiceState = " + ss.getState() + ", mFlightModeTestEnable = " + mFlightModeTestEnable);
                if(mFlightModeTestEnable) {
                    if(ss.getState() != ServiceState.STATE_POWER_OFF) {
                        FlightModeTestEnable(true);
                    }
                }                     
            }
        }
    };

    private OnClickListener mStatusbarNotificationOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            mIsStatusBarNotificationEnabled = !mIsStatusBarNotificationEnabled;
            updateUI();
            if (mIsStatusBarNotificationEnabled) {
                startService(new Intent(BSPTelephonyDevToolActivity.this, BSPTelephonyDevToolService.class));
            } else {
                stopService(new Intent(BSPTelephonyDevToolActivity.this, BSPTelephonyDevToolService.class));
            }
        }
    };

    private OnClickListener mDefaultSimOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.btn_default_sim_sim1:
                    mDefaultSimId = PhoneConstants.GEMINI_SIM_1;
                    break;
                case R.id.btn_default_sim_sim2:
                    mDefaultSimId = PhoneConstants.GEMINI_SIM_2;
                    break;
                case R.id.btn_default_sim_sim3:
                    mDefaultSimId = PhoneConstants.GEMINI_SIM_3;
                    break;
                case R.id.btn_default_sim_sim4:
                    mDefaultSimId = PhoneConstants.GEMINI_SIM_4;
                    break;
            }
            try {
                mTelephony.setDefaultPhone(mDefaultSimId);
                SystemProperties.set(PhoneConstants.GEMINI_DEFAULT_SIM_PROP, String.valueOf(mDefaultSimId));
            } catch(RemoteException e) {
                e.printStackTrace();
            }
            updateUI();
            rebootAlert();
        }
    };

    private OnClickListener mDefaultDataSimOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            Intent intent = new Intent(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
            switch (v.getId()) {
                case R.id.btn_default_data_sim1:
                    mDefaultDataSimId = PhoneConstants.GEMINI_SIM_1;
                    break;
                case R.id.btn_default_data_sim2:
                    mDefaultDataSimId = PhoneConstants.GEMINI_SIM_2;
                    break;
                case R.id.btn_default_data_sim3:
                    mDefaultDataSimId = PhoneConstants.GEMINI_SIM_3;
                    break;
                case R.id.btn_default_data_sim4:
                    mDefaultDataSimId = PhoneConstants.GEMINI_SIM_4;
                    break;
                case R.id.btn_default_data_off:
                    mDefaultDataSimId = -1;
            }
            SimInfoRecord simInfo = SimInfoManager.getSimInfoBySlot(BSPTelephonyDevToolActivity.this, mDefaultDataSimId);
            if (simInfo != null) {
                intent.putExtra(PhoneConstants.MULTI_SIM_ID_KEY, simInfo.mSimInfoId);
            }
            sendBroadcast(intent);
            updateUI();
        }
    };

    private OnClickListener m3GSimOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.btn_3g_sim1:
                    mSwitch3GHandler.set3GCapabilitySIM(PhoneConstants.GEMINI_SIM_1);
                    break;
                case R.id.btn_3g_sim2:
                    mSwitch3GHandler.set3GCapabilitySIM(PhoneConstants.GEMINI_SIM_2);
                    break;
                case R.id.btn_3g_sim3:
                    mSwitch3GHandler.set3GCapabilitySIM(PhoneConstants.GEMINI_SIM_3);
                    break;
                case R.id.btn_3g_sim4:
                    mSwitch3GHandler.set3GCapabilitySIM(PhoneConstants.GEMINI_SIM_4);
                    break;
                case R.id.btn_3g_off:
                    mSwitch3GHandler.set3GCapabilitySIM(-1);
            }
            updateUI();
        }
    };

    private OnClickListener mFlightModeOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.flight_mode_start:
                    FlightModeTestEnable(true);
                    break;
                case R.id.flight_mode_stop:
                    FlightModeTestEnable(false);
                    break;
            }
        }
    };

    private OnClickListener mHotPlugOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if (v == mPlugOutSim[0]) {
                logd("Plug out SIM1");
                String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugOutSim[1]) {
                logd("Plug out SIM2");
                String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_2);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugOutSim[2]) {
                logd("Plug out SIM3");
                String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_3);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugOutSim[3]) {
                logd("Plug out SIM4");
                String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_4);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugOutAllSims) {
                logd("Plug out all SIMs");
                String cmdStr[] = {AT_CMD_SIM_PLUG_OUT, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    ((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, 0);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugInSim[0]) {
                logd("Plug in SIM1");
                String cmdStr[] = {AT_CMD_SIM_PLUG_IN, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugInSim[1]) {
                logd("Plug in SIM2");
                String cmdStr[] = {AT_CMD_SIM_PLUG_IN, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_2);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            }  else if (v == mPlugInSim[2]) {
                logd("Plug in SIM3");
                String cmdStr[] = {AT_CMD_SIM_PLUG_IN, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_3);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            }  else if (v == mPlugInSim[3]) {
                logd("Plug in SIM4");
                String cmdStr[] = {AT_CMD_SIM_PLUG_IN, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_4);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mPlugInAllSims) {
                logd("Plug in all SIMs");
                String cmdStr[] = {AT_CMD_SIM_PLUG_IN_ALL, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    ((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, 0);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mMissingSim[0]) {
                logd("SIM1 missing");
                String cmdStr[] = {AT_CMD_SIM_MISSING, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mMissingSim[1]) {
                logd("SIM2 missing");
                String cmdStr[] = {AT_CMD_SIM_MISSING, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_2);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mRecoverySim[0]) {
                logd("SIM1 recovering");
                String cmdStr[] = {AT_CMD_SIM_RECOVERY, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            } else if (v == mRecoverySim[1]) {
                logd("SIM2 recovering");
                String cmdStr[] = {AT_CMD_SIM_RECOVERY, ""};
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
					((GeminiPhone)mPhone).invokeOemRilRequestStringsGemini(cmdStr, null, PhoneConstants.GEMINI_SIM_2);
                } else {
                    mPhone.invokeOemRilRequestStrings(cmdStr, null);
                }
            }
        }
    };
    
    private class GeminiPhoneStateListener extends PhoneStateListener {
        private int mSimId;
        private String mSimString;

        public GeminiPhoneStateListener(int simId) {
            mSimId = simId;
            mSimString = "SIM" + (mSimId+1);
        }

        @Override
        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            int gsmSignalStrength = signalStrength.getGsmSignalStrength();
            logd(mSimString + " signal strength: " + gsmSignalStrength);
            updateSignalStrengthUI(mSignalSim[mSimId], gsmSignalStrength);
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            logd(mSimString + " service state changed: " + serviceState);
            if (serviceState.getState() == ServiceState.STATE_IN_SERVICE) {
                mOperatorSim[mSimId].setText(serviceState.getOperatorAlphaShort());
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mNetworkTypeSim[mSimId].setText(Utility.getNetworkTypeString(mTelephonyManagerEx.getNetworkType(mSimId)));
                } else {
                    mNetworkTypeSim[mSimId].setText(Utility.getNetworkTypeString(mTelephonyManager.getNetworkType()));
                }
            } else {
                mOperatorSim[mSimId].setText(Utility.getServiceStateString(serviceState.getState()));
                mNetworkTypeSim[mSimId].setText("");
            }
        }

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            logd(mSimString + " data state: " + state + ", " + networkType);
            if (state > 0) {
                if (state == TelephonyManager.DATA_CONNECTED) {
                    mDataConnectionTypeSim[mSimId].setText(Utility.getNetworkTypeString(networkType));
                } else {
                    mDataConnectionTypeSim[mSimId].setText(Utility.getDataStateString(state));
                    mDataActivitySim[mSimId].setText("");
                }
            } else {
                mDataConnectionTypeSim[mSimId].setText("");
                mDataActivitySim[mSimId].setText("");
            }

            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                    if (i != mSimId) {
                        int peerSimState = mTelephonyManagerEx.getDataState(i);
                        if (peerSimState > 0) {
                            if (peerSimState == TelephonyManager.DATA_CONNECTED) {
                                mDataConnectionTypeSim[i].setText(Utility.getNetworkTypeString(mTelephonyManagerEx.getNetworkType(i)));
                            } else {
                                mDataConnectionTypeSim[i].setText(Utility.getDataStateString(peerSimState));
                                mDataActivitySim[i].setText("");
                            }
                        } else {
                            mDataConnectionTypeSim[i].setText("");
                            mDataActivitySim[i].setText("");
                        }
                    }
                }
            }
        }

        @Override
        public void onDataActivity(int direction) {
            mDataActivitySim[mSimId].setText(Utility.getDataDirectionString(direction));
        }
    };

    private Runnable mUpdateUIRunnable = new Runnable() {
        @Override
        public void run() {
            updateUI();
        }
    };
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            logd("!!!!!NOT GEMINI LOAD!!!!!");
        }
        // hot plug tool
        mPlugOutSim[0] = (Button) findViewById(R.id.btn_plug_out_sim1);
        mPlugOutSim[1] = (Button) findViewById(R.id.btn_plug_out_sim2);
        mPlugOutSim[2] = (Button) findViewById(R.id.btn_plug_out_sim3);
        mPlugOutSim[3] = (Button) findViewById(R.id.btn_plug_out_sim4);
        
        mPlugInSim[0] = (Button) findViewById(R.id.btn_plug_in_sim1);
        mPlugInSim[1] = (Button) findViewById(R.id.btn_plug_in_sim2);
        mPlugInSim[2] = (Button) findViewById(R.id.btn_plug_in_sim3);
        mPlugInSim[3] = (Button) findViewById(R.id.btn_plug_in_sim4);
        
        mMissingSim[0] = (Button) findViewById(R.id.btn_missing_sim1);
        mMissingSim[1] = (Button) findViewById(R.id.btn_missing_sim2);
        
        mRecoverySim[0] = (Button) findViewById(R.id.btn_recovery_sim1);
        mRecoverySim[1] = (Button) findViewById(R.id.btn_recovery_sim2);

        mPlugOutAllSims = (Button) findViewById(R.id.btn_plug_out_all_sims);
        mPlugInAllSims = (Button) findViewById(R.id.btn_plug_in_all_sims);

        for (int i=0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            logd("OnClickListener plug out sim" + i);
            logd("OnClickListener plug in sim" + i);
            mPlugOutSim[i].setOnClickListener(mHotPlugOnClickListener);
            mPlugInSim[i].setOnClickListener(mHotPlugOnClickListener);
            // SIM missing & recovery is not supported is gemini+
            if (i < 2) {
                logd("OnClickListener missing sim" + i);
                logd("OnClickListener recover sim" + i);
                mMissingSim[i].setOnClickListener(mHotPlugOnClickListener);
                mRecoverySim[i].setOnClickListener(mHotPlugOnClickListener);
            }
        }
        mPlugOutAllSims.setOnClickListener(mHotPlugOnClickListener);
        mPlugInAllSims.setOnClickListener(mHotPlugOnClickListener);
        
        if (!FeatureOption.MTK_SIM_HOT_SWAP_COMMON_SLOT) {
            mPlugOutAllSims.setVisibility(View.GONE);
            mPlugInAllSims.setVisibility(View.GONE);
            for (int i=PhoneConstants.GEMINI_SIM_NUM; i < MAX_GEMINI_SIM_NUM; i++) {
                logd("View GONE plug out sim" + i);
                mPlugOutSim[i].setVisibility(View.GONE);
            }
        } else {
            for (int i=0; i < MAX_GEMINI_SIM_NUM; i++) {
                logd("View GONE plug out sim" + i);
                mPlugOutSim[i].setVisibility(View.GONE);
            }
        }
        
        for (int i=PhoneConstants.GEMINI_SIM_NUM; i < MAX_GEMINI_SIM_NUM; i++) {
            logd("View GONE plug in sim" + i);
            mPlugInSim[i].setVisibility(View.GONE);
            if (i < 2) {
                logd("View GONE missing sim" + i);
                logd("View GONE recover sim" + i);
                mMissingSim[i].setVisibility(View.GONE);
                mRecoverySim[i].setVisibility(View.GONE);
            }
        }
        mPhone = PhoneFactory.getDefaultPhone();
        
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        mTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        mTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"));
        mSwitch3GHandler = new Switch3GHandler(this, mTelephonyEx, mUpdateUIRunnable);

        mDefaultSim[PhoneConstants.GEMINI_SIM_1] = (Button) findViewById(R.id.btn_default_sim_sim1);
        mOperatorSim[PhoneConstants.GEMINI_SIM_1] = (TextView) findViewById(R.id.operator_sim1);
        mStatusbarNotification = (Button) findViewById(R.id.btn_status_bar_notification);
        mDefaultDataSim[PhoneConstants.GEMINI_SIM_1] = (Button) findViewById(R.id.btn_default_data_sim1);
        mDefaultDataOff = (Button) findViewById(R.id.btn_default_data_off);
        mNetworkTypeSim[PhoneConstants.GEMINI_SIM_1] = (TextView) findViewById(R.id.network_type_sim1);
        mSignalSim[PhoneConstants.GEMINI_SIM_1] = (ProgressBar) findViewById(R.id.progress_signal_sim1);
        mDataConnectionTypeSim[PhoneConstants.GEMINI_SIM_1] = (TextView) findViewById(R.id.data_connection_type_sim1);
        mDataActivitySim[PhoneConstants.GEMINI_SIM_1] = (TextView) findViewById(R.id.data_activity_sim1);
        m3GSim[PhoneConstants.GEMINI_SIM_1] = (Button) findViewById(R.id.btn_3g_sim1);
        m3GSimOff = (Button) findViewById(R.id.btn_3g_off);
        mFlightModeTestStart= (Button) findViewById(R.id.flight_mode_start);
        mFlightModeTestStop= (Button) findViewById(R.id.flight_mode_stop);
        mFlightModeTestStart.setEnabled(true);
        mFlightModeTestStop.setEnabled(false);
        mFlightModeTestCounterText = (TextView) findViewById(R.id.flight_mode_counter);
        mFlightModeTestTimerText = (TextView) findViewById(R.id.flight_mode_timer);
        
        String text = String.format(getResources().getString(R.string.flight_mode_counter), 0);
        mFlightModeTestCounterText.setText(text);
        text = String.format(getResources().getString(R.string.flight_mode_timer), 0);
        mFlightModeTestTimerText.setText(text);

        if (!FeatureOption.MTK_GEMINI_3G_SWITCH) {
            findViewById(R.id.switch3g).setVisibility(View.GONE);
        }

        if (PhoneConstants.GEMINI_SIM_NUM > 1) {
            mDefaultSim[PhoneConstants.GEMINI_SIM_2] = (Button) findViewById(R.id.btn_default_sim_sim2);
            mOperatorSim[PhoneConstants.GEMINI_SIM_2] = (TextView) findViewById(R.id.operator_sim2);
            mDefaultDataSim[PhoneConstants.GEMINI_SIM_2] = (Button) findViewById(R.id.btn_default_data_sim2);
            mNetworkTypeSim[PhoneConstants.GEMINI_SIM_2] = (TextView) findViewById(R.id.network_type_sim2);
            mSignalSim[PhoneConstants.GEMINI_SIM_2] = (ProgressBar) findViewById(R.id.progress_signal_sim2);
            mDataConnectionTypeSim[PhoneConstants.GEMINI_SIM_2] = (TextView) findViewById(R.id.data_connection_type_sim2);
            mDataActivitySim[PhoneConstants.GEMINI_SIM_2] = (TextView) findViewById(R.id.data_activity_sim2);
            m3GSim[PhoneConstants.GEMINI_SIM_2] = (Button) findViewById(R.id.btn_3g_sim2);
        } else {
            findViewById(R.id.btn_default_sim_sim2).setVisibility(View.GONE);
            findViewById(R.id.operator_sim2).setVisibility(View.GONE);
            findViewById(R.id.btn_default_data_sim2).setVisibility(View.GONE);
            findViewById(R.id.network_type_sim2).setVisibility(View.GONE);
            findViewById(R.id.network_type_text_sim2).setVisibility(View.GONE);
            findViewById(R.id.progress_signal_sim2).setVisibility(View.GONE);
            findViewById(R.id.data_connection_type_sim2).setVisibility(View.GONE);
            findViewById(R.id.data_connection_type_text_sim2).setVisibility(View.GONE);
            findViewById(R.id.data_activity_sim2).setVisibility(View.GONE);
            findViewById(R.id.btn_3g_sim2).setVisibility(View.GONE);
        }

        if (PhoneConstants.GEMINI_SIM_NUM > 2) {
            mDefaultSim[PhoneConstants.GEMINI_SIM_3] = (Button) findViewById(R.id.btn_default_sim_sim3);
            mOperatorSim[PhoneConstants.GEMINI_SIM_3] = (TextView) findViewById(R.id.operator_sim3);
            mDefaultDataSim[PhoneConstants.GEMINI_SIM_3] = (Button) findViewById(R.id.btn_default_data_sim3);
            mNetworkTypeSim[PhoneConstants.GEMINI_SIM_3] = (TextView) findViewById(R.id.network_type_sim3);
            mSignalSim[PhoneConstants.GEMINI_SIM_3] = (ProgressBar) findViewById(R.id.progress_signal_sim3);
            mDataConnectionTypeSim[PhoneConstants.GEMINI_SIM_3] = (TextView) findViewById(R.id.data_connection_type_sim3);
            mDataActivitySim[PhoneConstants.GEMINI_SIM_3] = (TextView) findViewById(R.id.data_activity_sim3);
            m3GSim[PhoneConstants.GEMINI_SIM_3] = (Button) findViewById(R.id.btn_3g_sim3);
        } else {
            findViewById(R.id.btn_default_sim_sim3).setVisibility(View.GONE);
            findViewById(R.id.operator_sim3).setVisibility(View.GONE);
            findViewById(R.id.btn_default_data_sim3).setVisibility(View.GONE);
            findViewById(R.id.network_type_sim3).setVisibility(View.GONE);
            findViewById(R.id.network_type_text_sim3).setVisibility(View.GONE);
            findViewById(R.id.progress_signal_sim3).setVisibility(View.GONE);
            findViewById(R.id.data_connection_type_sim3).setVisibility(View.GONE);
            findViewById(R.id.data_connection_type_text_sim3).setVisibility(View.GONE);
            findViewById(R.id.data_activity_sim3).setVisibility(View.GONE);
            findViewById(R.id.btn_3g_sim3).setVisibility(View.GONE);
        }

        if (PhoneConstants.GEMINI_SIM_NUM > 3) {
            mDefaultSim[PhoneConstants.GEMINI_SIM_4] = (Button) findViewById(R.id.btn_default_sim_sim4);
            mOperatorSim[PhoneConstants.GEMINI_SIM_4] = (TextView) findViewById(R.id.operator_sim4);
            mDefaultDataSim[PhoneConstants.GEMINI_SIM_4] = (Button) findViewById(R.id.btn_default_data_sim4);
            mNetworkTypeSim[PhoneConstants.GEMINI_SIM_4] = (TextView) findViewById(R.id.network_type_sim4);
            mSignalSim[PhoneConstants.GEMINI_SIM_4] = (ProgressBar) findViewById(R.id.progress_signal_sim4);
            mDataConnectionTypeSim[PhoneConstants.GEMINI_SIM_4] = (TextView) findViewById(R.id.data_connection_type_sim4);
            mDataActivitySim[PhoneConstants.GEMINI_SIM_4] = (TextView) findViewById(R.id.data_activity_sim4);
            m3GSim[PhoneConstants.GEMINI_SIM_4] = (Button) findViewById(R.id.btn_3g_sim4);
        } else {
            findViewById(R.id.btn_default_sim_sim4).setVisibility(View.GONE);
            findViewById(R.id.operator_sim4).setVisibility(View.GONE);
            findViewById(R.id.btn_default_data_sim4).setVisibility(View.GONE);
            findViewById(R.id.network_type_sim4).setVisibility(View.GONE);
            findViewById(R.id.network_type_text_sim4).setVisibility(View.GONE);
            findViewById(R.id.progress_signal_sim4).setVisibility(View.GONE);
            findViewById(R.id.data_connection_type_sim4).setVisibility(View.GONE);
            findViewById(R.id.data_connection_type_text_sim4).setVisibility(View.GONE);
            findViewById(R.id.data_activity_sim4).setVisibility(View.GONE);
            findViewById(R.id.btn_3g_sim4).setVisibility(View.GONE);
        }

        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mDefaultSim[i].setOnClickListener(mDefaultSimOnClickListener);
            mDefaultDataSim[i].setOnClickListener(mDefaultDataSimOnClickListener);
            m3GSim[i].setOnClickListener(m3GSimOnClickListener);

            mPhoneStateListener[i] = new GeminiPhoneStateListener(i);
            mTelephonyManagerEx.listen(mPhoneStateListener[i],
                PhoneStateListener.LISTEN_SIGNAL_STRENGTHS |
                PhoneStateListener.LISTEN_SERVICE_STATE |
                PhoneStateListener.LISTEN_DATA_CONNECTION_STATE |
                PhoneStateListener.LISTEN_DATA_ACTIVITY, i);
        }

        mStatusbarNotification.setOnClickListener(mStatusbarNotificationOnClickListener);
        mDefaultDataOff.setOnClickListener(mDefaultDataSimOnClickListener);
        m3GSimOff.setOnClickListener(m3GSimOnClickListener);
        mFlightModeTestStart.setOnClickListener(mFlightModeOnClickListener);
        mFlightModeTestStop.setOnClickListener(mFlightModeOnClickListener);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_DATA_DEFAULT_SIM_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        registerReceiver(mBroadcastReceiver, intentFilter);

        mIsStatusBarNotificationEnabled = BSPTelephonyDevToolService.isRunning();
        mDefaultSimId = SystemProperties.getInt(PhoneConstants.GEMINI_DEFAULT_SIM_PROP, PhoneConstants.GEMINI_SIM_1);

        long dataSiminfoId = Settings.System.getLong(getContentResolver(),
                Settings.System.GPRS_CONNECTION_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
        SimInfoRecord simInfo = SimInfoManager.getSimInfoById(BSPTelephonyDevToolActivity.this, dataSiminfoId);
        if (simInfo == null) {
            mDefaultDataSimId = -1;
        } else {
            mDefaultDataSimId = simInfo.mSimSlotId;
        }
        updateUI();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mTelephonyManagerEx.listen(mPhoneStateListener[i], PhoneStateListener.LISTEN_NONE, i);
        }
        unregisterReceiver(mBroadcastReceiver);
    }

    private void updateUI() {
        int sim3G = mSwitch3GHandler.get3GCapabilitySIM();
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (mDefaultSimId == i) {
                mDefaultSim[i].setEnabled(false);
            } else {
                mDefaultSim[i].setEnabled(true);
            }
            if (mDefaultDataSimId == i) {
                mDefaultDataSim[i].setEnabled(false);
            } else {
                mDefaultDataSim[i].setEnabled(true);
            }
            if (sim3G == i) {
                m3GSim[i].setEnabled(false);
            } else {
                m3GSim[i].setEnabled(true);
            }
        }

        if (mDefaultDataSimId == -1) {
            mDefaultDataOff.setEnabled(false);
        } else {
            mDefaultDataOff.setEnabled(true);
        }
        if (sim3G == -1) {
            m3GSimOff.setEnabled(false);
        } else {
            m3GSimOff.setEnabled(true);
        }
        if (mIsStatusBarNotificationEnabled) {
            mStatusbarNotification.setText(R.string.stop_statusbar_notification);
        } else {
            mStatusbarNotification.setText(R.string.start_status_bar_notification);
        }
    }

    private void updateSignalStrengthUI(ProgressBar progressBar, int signalStrength) {
        progressBar.setMax(31);
        if (signalStrength == 99) {
            progressBar.setProgress(0);
            progressBar.setEnabled(false);
        } else {
            progressBar.setProgress(signalStrength);
            progressBar.setEnabled(true);
        }
    }

    private void rebootAlert() {
        DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case DialogInterface.BUTTON_POSITIVE:
                        PowerManager pm = (PowerManager)getSystemService(Context.POWER_SERVICE);
                        pm.reboot("");
                        Toast.makeText(getApplicationContext(), R.string.restarting_device, Toast.LENGTH_LONG).show();
                        break;
                    case DialogInterface.BUTTON_NEGATIVE:
                        break;
                }
            }
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(R.string.reboot_confirm_notice).setPositiveButton(R.string.yes, dialogClickListener)
            .setNegativeButton(R.string.no, dialogClickListener).show();
    }

    private void FlightModeTestEnable(boolean enable){

        logd("enter FlightModeTestEnable, enable = " + enable);
        mFlightModeTestEnable = enable;
		
        if(enable) {
            mFlightModeTestStart.setEnabled(false);
            mFlightModeTestStop.setEnabled(true);

            // send enter airplane mode intent
            Settings.Global.putInt(getApplicationContext().getContentResolver(),Settings.Global.AIRPLANE_MODE_ON, 1);
            Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
            intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
            intent.putExtra("state", true);
            getApplicationContext().sendBroadcastAsUser(intent, UserHandle.ALL);			

            // sleep a second, and then leave airplane mode
            Handler handler = new Handler(); 
            handler.postDelayed(new Runnable() { 
            public void run() { 
                // send leave airplane mode intent
                Settings.Global.putInt(getApplicationContext().getContentResolver(),Settings.Global.AIRPLANE_MODE_ON, 0);
                Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
                intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                intent.putExtra("state", false);
                getApplicationContext().sendBroadcastAsUser(intent, UserHandle.ALL);
                }
            }, 3000); 

            mFlightModeTestTimer = 0;
            String text = String.format(getResources().getString(R.string.flight_mode_timer), mFlightModeTestTimer);
            mFlightModeTestTimerText.setText(text);
            mFlightModeHandler.removeCallbacksAndMessages(null);
            mFlightModeHandler.postDelayed(new Runnable() {
                public void run() {  
                    if(mFlightModeTestEnable){
                        mFlightModeTestTimer++;
                        String text = String.format(getResources().getString(R.string.flight_mode_timer), mFlightModeTestTimer);
                        mFlightModeTestTimerText.setText(text);
                        mFlightModeHandler.postDelayed(this, 1000);
                    }
                }
            }, 1000);

            mFlightModeTestCounter++;
            text = String.format(getResources().getString(R.string.flight_mode_counter), mFlightModeTestCounter);
            mFlightModeTestCounterText.setText(text);


        } else {
            mFlightModeTestStart.setEnabled(true);
            mFlightModeTestStop.setEnabled(false);

            mFlightModeHandler.removeCallbacksAndMessages(null);
            mFlightModeTestCounter = 0;
            mFlightModeTestTimer = 0;
            String text = String.format(getResources().getString(R.string.flight_mode_counter), mFlightModeTestCounter);
            mFlightModeTestCounterText.setText(text);
            text = String.format(getResources().getString(R.string.flight_mode_timer), mFlightModeTestTimer);
            mFlightModeTestTimerText.setText(text);
        }				
    }

    private static void logd(String msg) {
        Log.d(LOG_TAG, "[BSPTelDevTool]" + msg);
    }
}
