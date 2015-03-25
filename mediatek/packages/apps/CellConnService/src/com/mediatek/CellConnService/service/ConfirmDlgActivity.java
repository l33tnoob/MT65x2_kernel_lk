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

package com.mediatek.CellConnService;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import android.telephony.TelephonyManager;
import android.view.KeyEvent;
import android.widget.Toast;
import android.view.WindowManager;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;

import android.provider.Settings;
import android.provider.Telephony.SimInfo;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;

public class ConfirmDlgActivity extends Activity implements OnClickListener {

    private static final String LOGTAG = "ConfirmDlgActivity";

    //Todo: Use PhoneWindowManager directly
    private static final String SYSTEM_DIALOG_REASON_KEY = "reason";
    private static final String SYSTEM_DIALOG_REASON_RECENT_APPS = "recentapps";
    private static final String SYSTEM_DIALOG_REASON_HOME_KEY = "homekey";

    // buttons id
    public static final int OK_BUTTON = R.id.button_ok;
    public static final int CANCEL_BUTTON = R.id.button_cancel;

    private String mTitle;
    private String mText;
    private String mButtonText;
    private String mRButtonText;

    private int mConfirmType;
    private int mSlot;
    private AlertDialog mAlertDlg;
    private boolean mResultSent;
    private int mPreferSlot;
    private boolean mNegativeExit;
    private boolean mBReceiverIsUnregistered = false;
    private int mPhoneState = 0;
    private boolean mOperator09 = false;

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(LOGTAG, "BroadcastReceiver onReceive");
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(intent.getAction())) {
                Log.d(LOGTAG, "BroadcastReceiver AIRPLANE_MODE_CHANGED");
                boolean airplaneModeON = intent.getBooleanExtra("state", false);
                Log.d(LOGTAG, "AIRPLANE_MODE_CHANGED ,airplaneModeON = " + airplaneModeON);

                if (airplaneModeON) {
                    // All of radio off
                    mNegativeExit = true;
                    sendConfirmResult(mConfirmType, false);
                } else if (PhoneStatesMgrService.CONFIRM_TYPE_RADIO == mConfirmType) {
                    mNegativeExit = true;
                    sendConfirmResult(mConfirmType, false);
                }
                return;

            } else if (Intent.ACTION_DUAL_SIM_MODE_CHANGED.equals(intent.getAction())) {
                Log.d(LOGTAG, "BroadcastReceiver ACTION_DUAL_SIM_MODE_CHANGED");

                int dualSimMode = intent.getIntExtra(Intent.EXTRA_DUAL_SIM_MODE, 0);
                Log.d(LOGTAG, "BroadcastReceiver duslSimMode = " + dualSimMode);                 

                if( ((dualSimMode >> mSlot) & 1) == 1) {
                    if (PhoneStatesMgrService.CONFIRM_TYPE_RADIO == mConfirmType) {
                        sendConfirmResult(mConfirmType, true);
                    }
                } else {
                    mNegativeExit = true;
                    sendConfirmResult(mConfirmType, false);
                }
            } else if (Intent.ACTION_CLOSE_SYSTEM_DIALOGS.equals(intent.getAction())) {
                //ALPS00612404: handle Recent_KEY
                Log.d(LOGTAG, "BroadcastReceiver ACTION_CLOSE_SYSTEM_DIALOGS");
                String reason = intent.getStringExtra(SYSTEM_DIALOG_REASON_KEY);
                if (SYSTEM_DIALOG_REASON_RECENT_APPS.equals(reason)
                    || SYSTEM_DIALOG_REASON_HOME_KEY.equals(reason)) {
                    Log.d(LOGTAG, "Dismiss dialog");
                    mNegativeExit = true;
                    sendConfirmResult(mConfirmType, false);
                }
            } else if (intent.getAction().equals("action_pin_dismiss")) {
                    //ALPS00693776: We have to dismiss confirm dialog when  pin or sim me
                    //unlock screen of the requested slot is been dismiss.
                    int slot = intent.getIntExtra("simslot", PhoneConstants.GEMINI_SIM_1);
                    Log.d(LOGTAG, "BroadcastReceiver action_pin_dismiss" + slot);
                    if (slot == mSlot){
                        mNegativeExit = true;
                        sendConfirmResult(mConfirmType, false);
                   }
            } else if(intent.getAction().equals("action_melock_dismiss")) {
                    int slot = intent.getIntExtra("simslot", PhoneConstants.GEMINI_SIM_1);
                    Log.d(LOGTAG, "BroadcastReceiver action_melock_dismiss" + slot);
                    if (slot == mSlot){
                        mNegativeExit = true;
                        sendConfirmResult(mConfirmType, false);
                   }
            } else if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                Log.d(LOGTAG, intent.toString() + intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE));
                String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

                int slot = 0;
                if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
                    slot = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY,PhoneConstants.GEMINI_SIM_1);
                }

                if (slot >= PhoneConstants.GEMINI_SIM_NUM) {
                    Log.e(LOGTAG, "BroadcastReceiver SIM State changed slot is invalid");
                    return;
                }

                Log.d(LOGTAG, "Slot = " + slot + " ,request slot = " + mSlot);
                if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(stateExtra)) {
                    if(slot == mSlot && PhoneStatesMgrService.CONFIRM_TYPE_RADIO == mConfirmType) {
                        //ALPS00565287: We have to dismiss confirm dialog when the SIM 
                        //state of request slot is ready.
                        mNegativeExit = true;
                        sendConfirmResult(mConfirmType, false);
                    }
                } else if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(stateExtra)) {
                    if(slot == mSlot && (PhoneStatesMgrService.CONFIRM_TYPE_RADIO == mConfirmType
                        || PhoneStatesMgrService.CONFIRM_TYPE_PIN == mConfirmType
                        || PhoneStatesMgrService.CONFIRM_TYPE_SIMMELOCK == mConfirmType)) {
                        Log.d(LOGTAG, "ICC card absent");
                        mNegativeExit = true;
                        sendConfirmResult(mConfirmType, false);
                    }
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        Log.d(LOGTAG, "onCreate");
        IntentFilter itFilter = new IntentFilter();
        if (null == itFilter) {
            Log.e(LOGTAG, "onCreate new intent failed");
            return;
        }

        String optr = SystemProperties.get("ro.operator.optr");
        if (null != optr && optr.equals("OP09")) {
            Log.d(LOGTAG, "optr = " + optr);
            mOperator09 = true;
        }
        
        itFilter.addAction(Intent.ACTION_DUAL_SIM_MODE_CHANGED);
        itFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        itFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        itFilter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        itFilter.addAction("action_pin_dismiss");
        itFilter.addAction("action_melock_dismiss");
        registerReceiver(mIntentReceiver, itFilter);
        mPreferSlot = 0;
        mNegativeExit = false;
        this.onNewIntent(getIntent());

        //ALPS00749400: we need to know whether confirm dialog is created or not.
        Intent intent = new Intent(PhoneStatesMgrService.CONFIRM_DIALOG_START);
        sendBroadcast(intent);
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        Log.d(LOGTAG, "onDestroy");
        if (!mBReceiverIsUnregistered) {
            mBReceiverIsUnregistered = true;
            unregisterReceiver(mIntentReceiver);
        }
        super.onDestroy();
        if(FeatureOption.MTK_SMARTBOOK_SUPPORT) {
            dismissAlertDialog();
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        // TODO Auto-generated method stub
        Log.d(LOGTAG, "onNewIntent");
        initFromIntent(getIntent());
        super.onNewIntent(intent);
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        Log.d(LOGTAG, "onPause");
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        Log.d(LOGTAG, "onStop");
        if (!mResultSent) {
            if (PhoneStatesMgrService.CONFIRM_TYPE_ROAMING == mConfirmType) {
                mNegativeExit = true;
            }
            Log.d(LOGTAG, "Cancel confirm dialog");
            sendConfirmResult(mConfirmType, PhoneStatesMgrService.CONFIRM_RESULT_CANCEL);
        }
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        Log.d(LOGTAG, "onResume");

        //ALPS00741192: We need to check the request SIM status, if it is ready in that time, close the dialog. 
        if(isNeedConfirmDialog() == false){
            Log.d(LOGTAG, "The request slot is ready, close dialog.");
            mNegativeExit = true;
            sendConfirmResult(mConfirmType, false);
            return;
        }

        if (PhoneStatesMgrService.CONFIRM_TYPE_FDN == mConfirmType 
                || PhoneStatesMgrService.CONFIRM_TYPE_SIMLOCKED == mConfirmType
                || PhoneStatesMgrService.CONFIRM_TYPE_SLOTLOCKED == mConfirmType) {
            mAlertDlg = new AlertDialog.Builder(this).setMessage(mText).setIcon(
                    android.R.drawable.ic_dialog_alert).setNegativeButton(
                    android.R.string.ok, this).setCancelable(true)
                    .setOnCancelListener(
                            new DialogInterface.OnCancelListener() {
                                public void onCancel(DialogInterface dialog) {
                                    Log.d(LOGTAG, "onClick is cancel");
                                    mNegativeExit = true;
                                    sendConfirmResult(mConfirmType, false);
                                }
                            }).show();
        } else if (PhoneStatesMgrService.CONFIRM_TYPE_ROAMING == mConfirmType) {
            mAlertDlg = new AlertDialog.Builder(this).setMessage(mText).setTitle(mTitle)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setPositiveButton(mButtonText, this).setNegativeButton(
                            mRButtonText, this).setCancelable(true)
                    .setOnCancelListener(
                            new DialogInterface.OnCancelListener() {
                                public void onCancel(DialogInterface dialog) {
                                    Log.d(LOGTAG, "onClick is cancel");
                                    mNegativeExit = true;
                                    sendConfirmResult(mConfirmType, false);
                                }
                            }).show();
        } else {
            mAlertDlg = new AlertDialog.Builder(this).setMessage(mText)
                    .setTitle(mTitle).setIcon(android.R.drawable.ic_dialog_alert)
                    .setPositiveButton(mButtonText, this).setNegativeButton(
                            android.R.string.cancel, this).setCancelable(true)
                    .setOnCancelListener(
                            new DialogInterface.OnCancelListener() {
                                public void onCancel(DialogInterface dialog) {
                                    Log.d(LOGTAG, "onClick is cancel");
                                    mNegativeExit = true;
                                    sendConfirmResult(mConfirmType, false);
                                }
                            }).show();
        }

        mAlertDlg.getWindow().addFlags(WindowManager.LayoutParams.FLAG_HOMEKEY_DISPATCHED);
        mAlertDlg.setOnKeyListener(new DialogInterface.OnKeyListener(){
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
            {
                if (event.isDown() && keyCode == KeyEvent.KEYCODE_HOME)
                {
                    Log.d(LOGTAG,"home key is down, cancel request");
                    mNegativeExit = true;
                    sendConfirmResult(mConfirmType, false);
                    return true;
                }
                return false;
            }
        }
        );

        mResultSent = false;
    }

    private void initFromIntent(Intent intent) {

        Log.d(LOGTAG, "initFromIntent ++ ");
        mConfirmType = 0;
        if (intent != null) {
            mSlot = intent.getIntExtra(PhoneStatesMgrService.CONFIRM_SLOT, 0);
            Log.d(LOGTAG, "initFromIntent mSlot = " + mSlot);
            mPreferSlot = mSlot;
            mPhoneState = intent.getIntExtra(PhoneStatesMgrService.CONFIRM_PHONE_STATE, 0);

            mConfirmType = intent.getIntExtra(
                    PhoneStatesMgrService.CONFIRM_TYPE, 0);
            Log.d(LOGTAG, "initFromIntent confirmType = "
                    + PhoneStatesMgrService.confirmTypeToString(mConfirmType));
                    
            if (PhoneStatesMgrService.CONFIRM_TYPE_RADIO == mConfirmType) {
                int airplaneMode = Settings.System.getInt(getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0);
                getResourceTypeRadio(intent);
                if (airplaneMode == 1)  {
                    if (isRadioOffBySimManagement(mSlot)) {
                        mText = getResources().getString(R.string.confirm_radio_flight_mode_on_and_msg) + " ";
                        if(mOperator09 == true) {
                            String strOperator = getOperatorBySlot(mSlot);
                            if(strOperator == null) {
                                mText = mText + getResources().getString(
                                        R.string.confirm_radio_flight_mode_on_sim_off_msg_card, intent.getStringExtra(
                                        PhoneStatesMgrService.CONFIRM_CARDNAME));
                            } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                                mText = mText + getResources().getString(
                                        R.string.confirm_radio_flight_mode_on_sim_off_msg_uim, intent.getStringExtra(
                                        PhoneStatesMgrService.CONFIRM_CARDNAME));
                            } else {
                                mText = mText + getResources().getString(
                                        R.string.confirm_radio_flight_mode_on_sim_off_msg, intent.getStringExtra(
                                        PhoneStatesMgrService.CONFIRM_CARDNAME));
                            }
                        } else {
                            mText = mText + getResources().getString(
                                    R.string.confirm_radio_flight_mode_on_sim_off_msg, intent.getStringExtra(
                                    PhoneStatesMgrService.CONFIRM_CARDNAME));
                        }
                    } else {
                        mText = getResources().getString(R.string.confirm_radio_flight_mode_on_sim_on_msg);
                    }
                }
                mButtonText = getResources().getString(android.R.string.ok);
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_PIN == mConfirmType) {
                getResourceTypePin();
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_PUK == mConfirmType) {
                mTitle = getResources().getString(R.string.confirm_pin_title);
                mText = getResources().getString(R.string.confirm_pin_msg);
                mButtonText = getResources().getString(R.string.confirm_unlock_lbutton);
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_SIMMELOCK == mConfirmType) {
                getResourceTypeSimMe();
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_FDN == mConfirmType) {
                mTitle = getResources().getString(R.string.confirm_fdn_title);
                mText = getResources().getString(R.string.confirm_fdn_msg);
                mButtonText = getResources().getString(android.R.string.ok);
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_ROAMING == mConfirmType) {
                getResourceTypeRoaning(intent);
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_SIMLOCKED == mConfirmType) {
                getResourceTypeSimLocked();
            } else if (PhoneStatesMgrService.CONFIRM_TYPE_SLOTLOCKED == mConfirmType) {
                mText = getResources().getString(R.string.confirm_slot_locked_message);
                mButtonText = getResources().getString(android.R.string.ok);
            }
        } else {
            finish();
        }

        Log.i(LOGTAG, "initFromIntent - [" + mText + "]");
    }

    private void getResourceTypeRadio(Intent intent) {
        String strOperator = getOperatorBySlot(mSlot);

        mTitle = getResources().getString(R.string.confirm_radio_title);
        if(mOperator09 == true) {
            if(strOperator == null) {
                mTitle = getResources().getString(R.string.confirm_radio_title_card);
            } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                mTitle = getResources().getString(R.string.confirm_radio_title_uim);
            }
        }

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mText = getResources().getString(R.string.confirm_radio_msg,
                   intent.getStringExtra(PhoneStatesMgrService.CONFIRM_CARDNAME));
            if(mOperator09 == true) {
                if(strOperator == null) {
                    mText = getResources().getString(R.string.confirm_radio_msg_card,
                            intent.getStringExtra(PhoneStatesMgrService.CONFIRM_CARDNAME));
                } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                    mText = getResources().getString(R.string.confirm_radio_msg_uim,
                            intent.getStringExtra(PhoneStatesMgrService.CONFIRM_CARDNAME));
                }
            }
        } else {
            mText = getResources().getString(R.string.confirm_radio_msg_single);
        }
    }

	private void getResourceTypePin() {

        mTitle = getResources().getString(R.string.confirm_pin_title);
        mText = getResources().getString(R.string.confirm_pin_msg);                
        mButtonText = getResources().getString(R.string.confirm_unlock_lbutton);

        if(mOperator09 == true) {
            String strOperator = getOperatorBySlot(mSlot);
            if(strOperator == null) {
                mTitle = getResources().getString(R.string.confirm_pin_title_card);
                mText = getResources().getString(R.string.confirm_pin_msg_card);
            } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                mTitle = getResources().getString(R.string.confirm_pin_title_uim);
                mText = getResources().getString(R.string.confirm_pin_msg_uim);
            }
        }
    }

	private void getResourceTypeSimMe() {

        mText = getResources().getString(R.string.confirm_simmelock_msg);
        mTitle = getResources().getString(R.string.confirm_simmelock_title);
        mButtonText = getResources().getString(R.string.confirm_unlock_lbutton);

        if(mOperator09 == true) {
            String strOperator = getOperatorBySlot(mSlot);
            if(strOperator == null) {
                mText = getResources().getString(R.string.confirm_simmelock_msg_card);
            } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                mText = getResources().getString(R.string.confirm_simmelock_msg_uim);
            }
        }
    }

	private void getResourceTypeRoaning(Intent intent) {

        /* For solving ALPS00471018, user can select "Yes" to use roaming sim card, 
                 or "No" for cancelling the action */
        mTitle = getResources().getString(R.string.confirm_roaming_title);
        mText = getResources().getString(R.string.confirm_roaming_for3Sims_msg);
        mButtonText = getResources().getString(R.string.button_yes);
        mRButtonText = getResources().getString(R.string.button_no);

        if(mOperator09 == true) {
            String strOperator = getOperatorBySlot(mSlot);
            if(strOperator == null) {
                mText = getResources().getString(R.string.confirm_roaming_for3Sims_msg_card);
            } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                mText = getResources().getString(R.string.confirm_roaming_for3Sims_msg_uim);
            }
        }
    }

	private void getResourceTypeSimLocked() {
        mText = getResources().getString(R.string.confirm_sim_locked_message);
        mButtonText = getResources().getString(android.R.string.ok);
        if(mOperator09 == true) {
            String strOperator = getOperatorBySlot(mSlot);
            if(strOperator == null) {
                mText = getResources().getString(R.string.confirm_sim_locked_message_card);
            }else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                mText = getResources().getString(R.string.confirm_sim_locked_message_uim);
            }
        }
    }
    
    private boolean isRadioOffBySimManagement(int simId) {
        boolean result = true;
        try {
            Context otherAppsContext = createPackageContext("com.android.phone", Context.CONTEXT_IGNORE_SECURITY);
            SharedPreferences mIccidPreference = otherAppsContext.getSharedPreferences("RADIO_STATUS", 0);
            SimInfoRecord simInfo = SimInfoManager.getSimInfoBySlot(this, simId);    
            if ((simInfo != null) && (mIccidPreference != null)) {
               Log.d(LOGTAG, "[isRadioOffBySimManagement]SharedPreferences: " + mIccidPreference.getAll().size() + ", IccId: " + simInfo.mIccId);
               result = mIccidPreference.contains(simInfo.mIccId);      
            }
            Log.d(LOGTAG, "[isRadioOffBySimManagement]result: " + result);
        } catch (NameNotFoundException e) {
            Log.w(LOGTAG, "Fail to create com.android.phone createPackageContext");
        }
        return result;
    }     

    public void onClick(DialogInterface dialog, int which) {
        // TODO Auto-generated method stub
        if (which == DialogInterface.BUTTON_POSITIVE) {
            Log.d(LOGTAG, "onClick is true");
            sendConfirmResult(mConfirmType, true);
        } else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Log.d(LOGTAG, "onClick is false");
            sendConfirmResult(mConfirmType, false);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // TODO Auto-generated method stub
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            Log.d(LOGTAG, "onKeyDown back confirm result is false");
            mNegativeExit = true;
            sendConfirmResult(mConfirmType, false);
            return false;

        default:
            break;
        }

        return super.onKeyDown(keyCode, event);
    }
    
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO Auto-generated method stub
        Log.d(LOGTAG, "onConfigurationChanged");
        super.onConfigurationChanged(newConfig);
    }

    private void dismissAlertDialog(){
        try {
            if ((mAlertDlg != null) && (mAlertDlg.isShowing()) && (!this.isFinishing())) {
                Log.d(LOGTAG, "dismissAlertDialog");
                mAlertDlg.dismiss();
            }
        } catch (IllegalArgumentException e) {
            Log.w(LOGTAG, "Trying to dismiss a dialog not connected to the current UI");
        }
    }

    private boolean isNeedConfirmDialog() {
        int state = TelephonyManagerEx.getDefault().getSimState(mSlot);
        Log.d(LOGTAG, "SIM" + mSlot + " State = " + state);
        Log.d(LOGTAG, "confirm type = " + mConfirmType + ", PhoneState = " + mPhoneState);

        //ALPS00753649 & ALPS00759667: We check SIM states only those situation.
        if(mConfirmType == PhoneStatesMgrService.CONFIRM_TYPE_RADIO || 
                mConfirmType == PhoneStatesMgrService.CONFIRM_TYPE_PIN || 
                mConfirmType == PhoneStatesMgrService.CONFIRM_TYPE_SIMMELOCK || 
                mConfirmType == PhoneStatesMgrService.CONFIRM_TYPE_PUK) {
            if(TelephonyManager.SIM_STATE_READY == state && 
                    mPhoneState == PhoneStatesMgrService.PHONE_STATE_NORMAL) {
                return false;
            }
        }
        return true;
    }
    
    private void sendConfirmResult(int confirmType, int nRet) {
        Log.d(LOGTAG, "sendConfirmResult confirmType = "
                + PhoneStatesMgrService.confirmTypeToString(confirmType)
                + " nRet = " + PhoneStatesMgrService.confirmResultToString(nRet));

        Intent retIntent = new Intent(getBaseContext(),
                PhoneStatesMgrService.class).putExtra(
                PhoneStatesMgrService.START_TYPE,
                PhoneStatesMgrService.START_TYPE_RSP);
        if (null == retIntent) {
            Log.e(LOGTAG, "sendConfirmResult new retIntent failed");
            return;
        }
        retIntent.putExtra(PhoneStatesMgrService.CONFIRM_TYPE, confirmType);
        retIntent.putExtra(PhoneStatesMgrService.CONFIRM_RESULT_PREFERSLOT, mPreferSlot);
        retIntent.putExtra(PhoneStatesMgrService.CONFIRM_RESULT, nRet);

        if (!mBReceiverIsUnregistered) {
            mBReceiverIsUnregistered = true;
            unregisterReceiver(mIntentReceiver);
        }

        startService(retIntent);
        dismissAlertDialog();
        finish();
    }

    private String getOperatorBySlot(int slot) {

        Log.d(LOGTAG,"[getOperatorBySlot]+ simSlotId:" + slot);
        if (slot < 0) {
            Log.d(LOGTAG,"[getOperatorBySlot]- simSlotId < 0");
            return null;
        }

        SimInfoRecord simInfo = SimInfoManager.getSimInfoBySlot(getBaseContext(), slot);
        if(simInfo != null) {
            Log.d(LOGTAG,"[getOperatorBySlot]- operator:" + simInfo.mOperator);
            return simInfo.mOperator;
        } else {
            Log.d(LOGTAG,"[getOperatorBySlot]- null info, return");
            return null;
        }
    }

    private void sendConfirmResult(int confirmType, boolean bRet) {
        Log.d(LOGTAG, "sendConfirmResult confirmType = "
                + PhoneStatesMgrService.confirmTypeToString(confirmType)
                + " bRet = " + bRet);

        mResultSent = true;


        if (!bRet) {
            switch (mConfirmType) {
            case PhoneStatesMgrService.CONFIRM_TYPE_RADIO:
                Toast.makeText(this, R.string.confirm_turnon_radio_fail, Toast.LENGTH_SHORT).show();
                break;
            case PhoneStatesMgrService.CONFIRM_TYPE_PIN:
            case PhoneStatesMgrService.CONFIRM_TYPE_PUK:
            case PhoneStatesMgrService.CONFIRM_TYPE_SIMMELOCK:
                Toast.makeText(this, R.string.confirm_unlock_fail, Toast.LENGTH_SHORT).show();
                break;
            case PhoneStatesMgrService.CONFIRM_TYPE_ROAMING:
                if(mOperator09 == true) {
                    String strOperator = getOperatorBySlot(mSlot);
                    if(strOperator == null) {
                        Toast.makeText(this, R.string.confirm_permit_roaming_fail_card
                                ,Toast.LENGTH_SHORT).show();
                    } else if(strOperator.equals(SimInfo.OPERATOR_OP09)) {
                        Toast.makeText(this, R.string.confirm_permit_roaming_fail_uim
                                ,Toast.LENGTH_SHORT).show();
                    } else {
                        Toast.makeText(this, R.string.confirm_permit_roaming_fail
                                ,Toast.LENGTH_SHORT).show();
                    }
                } else {
                    Toast.makeText(this, R.string.confirm_permit_roaming_fail
                            ,Toast.LENGTH_SHORT).show();
                }
            default:
                break;
            }
        }

        if (bRet) {
            sendConfirmResult(confirmType, PhoneStatesMgrService.CONFIRM_RESULT_OK);
        } else {
            sendConfirmResult(confirmType, PhoneStatesMgrService.CONFIRM_RESULT_CANCEL);
        }
    }

}

