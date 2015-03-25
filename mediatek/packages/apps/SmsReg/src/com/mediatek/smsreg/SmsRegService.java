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

package com.mediatek.smsreg;

import android.R.integer;
import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.common.telephony.ITelephonyEx;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.smsreg.ui.SendMessageAlertActivity;
import com.mediatek.telephony.SmsManagerEx;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

public class SmsRegService extends Service {
    private static final String TAG = "SmsReg/Service";
    private Boolean mIsSendMsg = false;
    private String[] mIMSI = new String[2];
    private String mSimIMSI = null;
    private String mSavedIMSI;
    private Boolean mServiceAlive = true;

    private TelephonyManagerEx mTelephonyManager;
    private InfoPersistentor mInfoPersistentor;
    private SmsBuilder mSmsBuilder;
    private SmsManagerEx mSmsManager;
    private ConfigInfoGenerator mXmlG;

    private final long mSearchNetDelay = 90000; // wait 1.5min for search signal
    private final long mNotifyDialogDelay = 30000; // wait 30s for search signal
    private static final String SMS_SENDING_RESULT_TAG = "SMS_SENDING_RESULT";
    private PendingIntent mSender;
    private SimStateReceiver mSimStateReceiver;
    private SmsReceivedReceiver mSmsReceivedReceiver;
    private Boolean mSlotGemini[] = {false,false};    // whether slot has no card
    private Boolean mReadyGemini[] = {false,false};  // whether sim card is ready
    
    @Override
    public void onCreate() {
        mSmsManager = SmsManagerEx.getDefault();
        Xlog.e(TAG, "SmsRegService onCreate.");
        // create XMLgenerator object
        mXmlG = XMLGenerator.getInstance(SmsRegConst.getConfigPath());
        if (mXmlG == null) {
            Xlog.e(TAG, "Init XMLGenerator error!");
            return;
        }

        mSmsBuilder = new SmsBuilder(this);
        mTelephonyManager = new TelephonyManagerEx(this);
        if (mTelephonyManager == null) {
            Xlog.e(TAG, "TelephonyManager service is not exist!");
        }
        mInfoPersistentor = new InfoPersistentor();

    }

    @Override
    public void onDestroy() {
        Xlog.i(TAG, "SmsRegService destroy");
        mSmsManager = null;
        mTelephonyManager = null;
        mInfoPersistentor = null;
        mSmsBuilder = null;
        mXmlG = null;
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Xlog.e(TAG, "SmsReg service on start");
        super.onStart(intent, startId);

        // if mXmlG is init error the service should be stopped
        if (mXmlG == null) {
            Xlog.e(TAG, "XMLGenerator instance init error!");
            stopSelf();
            return;
        }

        if (intent == null) {
            Xlog.w(TAG, "intent is null!");
            return;
        }

        String action = intent.getAction();
        Xlog.i(TAG, "SmsReg service onStart, action = " + action);
        if (action == null) {
            Xlog.w(TAG, "intent action is null!");
            return;
        }

        if (action.equals("BOOTCOMPLETED")) {
            //Retry to send smsreg message if proc be killed by system.
            String operatorID = mXmlG.getOperatorName();
            Xlog.i(TAG, "the operator Id = " + operatorID);
            if(operatorID.equalsIgnoreCase("cu")) {
                mSimStateReceiver = new SimStateReceiver();
                this.registerReceiver(mSimStateReceiver,
                        new IntentFilter("android.intent.action.SIM_STATE_CHANGED"));

                mSmsReceivedReceiver = new SmsReceivedReceiver();
                this.registerReceiver(mSmsReceivedReceiver,
                        new IntentFilter("android.intent.action.DM_REGISTER_SMS_RECEIVED"));
                Xlog.i(TAG, "add retry send message alarm message ,if smsreg process be killed ");
                Intent retryIntent =new Intent(this, SmsRegReceiver.class);    
                retryIntent.setAction(SmsRegConst.RETRY_SEND_SMSREG);
                mSender =  PendingIntent.getBroadcast(
                        this, 0, retryIntent, android.app.PendingIntent.FLAG_UPDATE_CURRENT);
                AlarmManager alarm = (AlarmManager)getSystemService(ALARM_SERVICE);
                alarm.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + mSearchNetDelay, mSender);
            } else if (operatorID.equalsIgnoreCase("cmcc")) {
                Xlog.i(TAG, " register send alarm message ");
                BlackListUnit.getInstance().isBlackFileReady();
                Xlog.i(TAG, "add retry send message alarm message ,Start confirm dialog");
                Intent retryIntent =new Intent(this, SmsRegReceiver.class);    
                retryIntent.setAction(SmsRegConst.ACTION_PREPARE_CONFIRM_DIALOG);
                mSender =  PendingIntent.getBroadcast(
                        this, 0, retryIntent, android.app.PendingIntent.FLAG_UPDATE_CURRENT);
                AlarmManager alarm = (AlarmManager)getSystemService(ALARM_SERVICE);
                alarm.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + mNotifyDialogDelay, mSender);
            } else {
                Xlog.i(TAG, "Unknown  operator Id = " + operatorID);
            }
            
        } else if (action.equals("SIM_STATE_CHANGED")) {
            String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            int idCurSim = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
            Xlog.i(TAG, "sim[" +  idCurSim + "]state is : " + stateExtra);
            if (!IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {
                Xlog.w(TAG, "sim state is not loaded");
                return;
            }

            // only used for gemini loads
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                ITelephonyEx mTelephony = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
                Xlog.i(TAG,"get slot sim ITelephonyEx");
                if (mTelephony == null) {
                    Xlog.i(TAG, "mTelephony is null");
                } else {
                for (int i = 0; i < SmsRegConst.GEMSIM.length; i++) {
                    try {
                            if (!mTelephony.hasIccCard(i)) {
                            mSlotGemini[i] = true;
                            Xlog.i(TAG, "Slot[" + i + "] has no sim card.");
                        } else {
                            Xlog.i(TAG, "Slot[" + i + "] has sim card.");
                            //if BOOT_COMPLETED message is after All SIM cards SIM_STATE_CHANGED LOADED message
                            //check the IMSI of the SIM card which we missed the SIM_STATE_CHANGED LOADED message
                            if (mTelephonyManager.getSubscriberId(i) != null) {
                                mReadyGemini[i] = true;
                                Xlog.i(TAG, "Slot[" + i + "] can read IMSI.");
                            }
                        }
                    } catch (RemoteException e) {
                        Xlog.e(TAG, "* RemoteException:" + e.getMessage());
                        e.printStackTrace();
                        return;
                    }
                }
                }
                idCurSim = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
                if (idCurSim != -1) {
                    mReadyGemini[idCurSim] = true;
                    Xlog.i(TAG, "Slot[" + idCurSim + "] is ready.");
                } else {
                    // get simId error
                    return;
                }
                // if not both card ready, return
                if ((mSlotGemini[0] || mReadyGemini[0])
                        && (mSlotGemini[1] || mReadyGemini[1])) {
                    Xlog.i(TAG, "-------- Both slot checked finished --------");
                } else {
                    Xlog.i(TAG, "-------- Wait for another sim card ready --------");
                    return;
                }
            }

            Xlog.i(TAG, "sim state is loaded");

            // Only for gemini, check whether device is registered firstly
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                mSavedIMSI = mInfoPersistentor.getSavedIMSI();
                if (mSavedIMSI != null) {
                    for (int i = 0; i < SmsRegConst.GEMSIM.length; i++) {
                        // if slot[i] not empty, check
                        if (!mSlotGemini[i]) {
                            String cheImsi = mTelephonyManager
                                    .getSubscriberId(SmsRegConst.GEMSIM[i]);
                            if (mSavedIMSI.equals(cheImsi)) {
                                Xlog.i(TAG, "-------- Gemini device registered already --------");
                                stopService();
                                return;
                            }
                        }
                    }
                }
            }

            // register the phonestate listener
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                Xlog.i(TAG, "Regist service state listener for sim1.");
                mTelephonyManager.listen(mPhoneStateListener,
                        PhoneStateListener.LISTEN_SERVICE_STATE,
                        PhoneConstants.GEMINI_SIM_1);
                Xlog.i(TAG, "Regist service state listener gemini for sim2.");
                mTelephonyManager.listen(mPhoneStateListenerGemini,
                        PhoneStateListener.LISTEN_SERVICE_STATE,
                        PhoneConstants.GEMINI_SIM_2);
            } else {
                Xlog.i(TAG, "Regist service state listener for sim.");
                mTelephonyManager.listen(mPhoneStateListener,
                        PhoneStateListener.LISTEN_SERVICE_STATE,
                        PhoneConstants.GEMINI_SIM_1);
            }

            if (!mIsSendMsg) {
                // send register message
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    getSimCardMatchCustomizedGemini();
                    int simNum = -1;
                    if (mIMSI[0] != null) {
                        simNum = PhoneConstants.GEMINI_SIM_1;
                    } else if (mIMSI[1] != null) {
                        simNum = PhoneConstants.GEMINI_SIM_2;
                    } else {
                        Xlog.e(TAG, "No sim card or the sim card is not "
                                + "the customized operator");
                        return;
                    }

                    if (isNeedRegisterGemini()) {
                        sendRegisterMessageGemini(simNum);
                    }
                } else {
                    getSimCardMatchCustomized();
                    if (mSimIMSI != null && isNeedRegister()) {
                        sendRegisterMessage();
                    } else {
                        Xlog.e(TAG, "No sim card or the sim card is not"
                                + " the customized operator");
                        return;
                    }
                }

            } else {
                Xlog.w(TAG, "the register message has been sent");
            }

        } else if (action.equals("REGISTER_SMS_RECEIVED")) {
            Xlog.i(TAG, "broadcast REGISTER_SMS_RECEIVED has received");
            int resultCode = intent.getIntExtra(SMS_SENDING_RESULT_TAG, Activity.RESULT_OK);
            if (resultCode == Activity.RESULT_OK) {
                Xlog.w(TAG, "Save the IMSI");
                notifyDMtoResetPermissionFile();
                String imsi = intent.getStringExtra("mIMSI");
                Xlog.w(TAG, "The IMSI to save is  = [" + imsi + "]");
                if (imsi != null && !(imsi.equals(""))) {
                    InfoPersistentor ip = new InfoPersistentor();
                    ip.setSavedIMSI(imsi);
                }
            } else {
                Xlog.e(TAG, "Sms sending failed.");
            }
            stopService();
        } else if(action.equals(SmsRegConst.ACTION_PREPARE_CONFIRM_DIALOG)) {
            if (mSmsReceivedReceiver == null) {
                mSmsReceivedReceiver = new SmsReceivedReceiver();
                this.registerReceiver(mSmsReceivedReceiver,
                        new IntentFilter("android.intent.action.DM_REGISTER_SMS_RECEIVED"));
            }
            //
            //ITelephonyEx mTelephony = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
            //Xlog.i(TAG,"get slot sim ITelephonyEx");
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                getSimCardMatchCustomizedGemini();
                if(isNeedRegisterGemini()) {
                    //
                    int simNum = -1;
                    if(mIMSI[0] == null && mIMSI[1] == null) {
                        Xlog.i(TAG,"The both SIM slot has not right sim card" );
                        stopService();
                        return;
                    }
                    if(mIMSI[0] != null && mIMSI[1] != null) {
                        //sim 1 and sim2 are ok
                        simNum = 2;
                    } else if(mIMSI[0] != null) {
                        //only sim1 is OK
                        simNum = 0;
                    } else if (mIMSI[1] != null){
                        //only sim2 is ok
                        simNum = 1;
                    }
                    Xlog.i(TAG,"the sim num is :" + simNum);
                    int slotId = isNeedSendRegisterMessageGemini(simNum);
                    Xlog.i(TAG,"the slotId is :" + slotId);
                    if(slotId != -1) {
                        startReqNotifyDialog(slotId);
                    } else {
                        stopService();
                    }
                } else {
                    Xlog.i(TAG,"there is no need to register");
                    HashMap<String,String> saveMap = BlackListUnit.getInstance().readObjectFromfile();
                    saveMap.put(BlackListUnit.SLOT_ONE_KEY, "");
                    saveMap.put(BlackListUnit.SLOT_TWO_KEY, "");
                    BlackListUnit.getInstance().writeObjectToFile(saveMap);
                    stopService();
                }
                
            } else {
                getSimCardMatchCustomized();
                if(isNeedRegister()) {
                    if(mSimIMSI == null) {
                        Xlog.i(TAG,"The SIM slot has not right sim card" );
                        stopService();
                        return;
                    } else {
                        boolean isNeedNotify = isNeedSendRegisterMessage();
                        if (isNeedNotify) {
                            int slotId = 0;
                            startReqNotifyDialog(slotId);
                        } else {
                            stopService();
                        }
                    }
                } else {
                    Xlog.i(TAG, "there is no need to register");
                    HashMap<String,String> saveMap = BlackListUnit.getInstance().readObjectFromfile();
                    saveMap.put(BlackListUnit.SLOT_ONE_KEY, "");
                    BlackListUnit.getInstance().writeObjectToFile(saveMap);
                    stopService();
                }
            }    
        } else if (action.equals(SmsRegConst.ACTION_CONFIRM_DIALOG_END)) {
            if (mSmsReceivedReceiver == null) {
                mSmsReceivedReceiver = new SmsReceivedReceiver();
                this.registerReceiver(mSmsReceivedReceiver,
                        new IntentFilter("android.intent.action.DM_REGISTER_SMS_RECEIVED"));
            }
            int slotId = intent.getIntExtra(SmsRegConst.EXTRA_SLOT_ID, -1);
            Xlog.i(TAG, "slotId is" + slotId);
            boolean isNeedSendMsg = intent.getBooleanExtra(SmsRegConst.EXTRA_IS_NEED_SEND_MSG, false);
            Xlog.i(TAG, "isNeedSendMsg is" + isNeedSendMsg);
            HashMap<String,String> saveMap = BlackListUnit.getInstance().readObjectFromfile();
            if (isNeedSendMsg) {
                Xlog.i(TAG, "user agree send msg ");
                if (slotId != -1) {
                    
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {                   
                        if (slotId == 0) {
                            saveMap.put(BlackListUnit.SLOT_ONE_KEY, "");
                            sendRegisterMessageGemini(PhoneConstants.GEMINI_SIM_1);
                        } else if (slotId == 1) {
                            saveMap.put(BlackListUnit.SLOT_TWO_KEY, "");
                            sendRegisterMessageGemini(PhoneConstants.GEMINI_SIM_2);
                        }                   
                        BlackListUnit.getInstance().writeObjectToFile(saveMap);
                    } else {
                        sendRegisterMessage();
                        saveMap.put(BlackListUnit.SLOT_ONE_KEY, "");
                        saveMap.put(BlackListUnit.SLOT_TWO_KEY, "");
                        BlackListUnit.getInstance().writeObjectToFile(saveMap);
                    }
                } else {
                    Xlog.i(TAG, "user agree send msg ,slot id is not right :" + slotId);
                    stopService();
                }
            } else {
                Xlog.i(TAG, "user reject send msg ");
                if (slotId != -1) { 
                    String imsi = mTelephonyManager.getSubscriberId(slotId);
                    Xlog.i(TAG, "the current imsi is " + imsi);
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {                   
                        if (slotId == 0) {
                            saveMap.put(BlackListUnit.SLOT_ONE_KEY, imsi);
                        } else if (slotId == 1) {
                            saveMap.put(BlackListUnit.SLOT_TWO_KEY, imsi);
                        }                   
                        BlackListUnit.getInstance().writeObjectToFile(saveMap);
                        stopService();
                    } else {
                        saveMap.put(BlackListUnit.SLOT_ONE_KEY, imsi);
                        saveMap.put(BlackListUnit.SLOT_TWO_KEY, "");
                        BlackListUnit.getInstance().writeObjectToFile(saveMap);
                        stopService();
                    }
                } else {
                    Xlog.i(TAG, "user agree send msg ,slot id is not right :" + slotId);
                    stopService();
                }
                
            }
        } else if(action.equals("RETRY_SEND_SMSREG")) {
            if (mSmsReceivedReceiver == null) {
                mSmsReceivedReceiver = new SmsReceivedReceiver();
                this.registerReceiver(mSmsReceivedReceiver,
                        new IntentFilter("android.intent.action.DM_REGISTER_SMS_RECEIVED"));
            }
            if (!mIsSendMsg) {
                // send register message
                Xlog.i(TAG, "broadcast RETRY_SEND_SMSREG has received");
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    getSimCardMatchCustomizedGemini();
                    int simNum = -1;
                    if (mIMSI[0] != null) {
                        simNum = PhoneConstants.GEMINI_SIM_1;
                    } else if (mIMSI[1] != null) {
                        simNum = PhoneConstants.GEMINI_SIM_2;
                    } else {
                        Xlog.e(TAG, "No sim card or the sim card is not "
                                + "the customized operator");
                        stopService();
                        return;
                    }

                    if (isNeedRegisterGemini()) {
                        sendRegisterMessageGemini(simNum);
                    }
                } else {
                    getSimCardMatchCustomized();
                    if (mSimIMSI != null && isNeedRegister()) {
                        sendRegisterMessage();
                    } else {
                        Xlog.e(TAG, "No sim card or the sim card is not"
                                + " the customized operator");
                        stopService();
                        return;
                    }
                }

            } else {
                Xlog.w(TAG, "the register message has been sent");
            }
        } else {
            Xlog.e(TAG, "Get the wrong intent");
            mServiceAlive = false;
        }
        if (!mServiceAlive) {
            Xlog.w(TAG, "mServiceAlive is false");
            stopService();
        }
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }


    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            Log.i(TAG, "Service state change sim:" + serviceState.getState());
            if (serviceState.getState() == ServiceState.STATE_IN_SERVICE) {
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    getSimCardMatchCustomizedGemini();
                    if (mIMSI[0] != null && isNeedRegisterGemini()) {
                            sendRegisterMessageGemini(PhoneConstants.GEMINI_SIM_1);
                    } else {
                        Xlog.e(TAG, "Sim card 1 is not the right operator");
                    }
                } else {
                    getSimCardMatchCustomized();
                    if (mSimIMSI != null && isNeedRegister()) {
                        sendRegisterMessage();
                    }
                }
            }
        }
    };

    private PhoneStateListener mPhoneStateListenerGemini =
            new PhoneStateListener() {
        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            Log.i(TAG, "Service state change sim gemini:" + serviceState);
            if (serviceState.getState() == ServiceState.STATE_IN_SERVICE) {
                getSimCardMatchCustomizedGemini();
                if (mIMSI[0] == null && mIMSI[1] != null) {
                    if (isNeedRegisterGemini()) {
                        sendRegisterMessageGemini(PhoneConstants.GEMINI_SIM_2);
                    }
                } else {
                    Xlog.e(TAG, "Sim2 do not need to register or "
                            + "sim2 is not the right operator");
                }
            }
        }
    };

    private void getSimCardMatchCustomized() {
        // the phone is for operator
        // which operator
        String operatorID = mXmlG.getOperatorName();
        String[] operatorNumber = mXmlG.getNetworkNumber();
        Xlog.i(TAG, "the operator Id = " + operatorID
                + ", operatorNumber.length = " + operatorNumber.length);
        // get sim card state
        int simState = mTelephonyManager.getSimState(0);
        if (TelephonyManager.SIM_STATE_READY == simState) {
            String currentSimOperator = mTelephonyManager.getSimOperator(0);
            Xlog.i(TAG, "there is a sim card is ready the operator is "
                    + currentSimOperator);
            if (currentSimOperator == null
                    || currentSimOperator.trim().equals("")) {
                Xlog.i(TAG, "operator is null, do nothing. ");
                return;
            }
            int j = 0;
            for (; j < operatorNumber.length; j++) {
                String configedOperaterNumber = operatorNumber[j];
                if (configedOperaterNumber != null
                        && configedOperaterNumber.equals(currentSimOperator)) {
                    Xlog.i(TAG, "the ready sim card operator is "
                            + operatorNumber[j]);
                    // get ISMI
                    mSimIMSI = mTelephonyManager.getSubscriberId(0);
                    Xlog.i(TAG, "the current imsi is " + mSimIMSI);
                    break;
                }
            }
            if (j >= operatorNumber.length) {
                Xlog.e(TAG, "There is no sim card operator is matched current"
                        + "operator number.");
            }
        } else {
            Xlog.w(TAG, "Sim state is not ready, state = " + simState);
        }
    }

    private void getSimCardMatchCustomizedGemini() {
        String operatorID = mXmlG.getOperatorName();
        String[] operatorNumber = mXmlG.getNetworkNumber();
        Xlog.i(TAG, "the operator Id = " + operatorID
                + ", operatorNumber.length = " + operatorNumber.length);
        for (int i = 0; i < SmsRegConst.GEMSIM.length; i++) {
            // get sim card state
            int simState = mTelephonyManager
                    .getSimState(SmsRegConst.GEMSIM[i]);
            if (TelephonyManager.SIM_STATE_READY == simState) {

                String currentOperator = mTelephonyManager
                        .getSimOperator(SmsRegConst.GEMSIM[i]);
                Xlog.i(TAG, "there is a sim card is ready the operator is "
                        + currentOperator);
                if (currentOperator == null
                        || currentOperator.trim().equals("")) {
                    Xlog.i(TAG, "operator is null, continue next one. ");
                    continue;
                }
                for (int j = 0; j < operatorNumber.length; j++) {
                    String configuredOperator = operatorNumber[j];
                    Xlog.i(TAG, "the phone is for the operator[ "
                            + configuredOperator + "]");
                    if (configuredOperator != null
                            && configuredOperator.equals(currentOperator)) {
                        mIMSI[i] = mTelephonyManager
                                .getSubscriberId(SmsRegConst.GEMSIM[i]);
                        Xlog.i(TAG, "current mIMSI[" + i + "]=" + mIMSI[i]);
                        break;
                    }
                }
            }
        } // for
    }

    private Boolean isNeedRegister() {
        mSavedIMSI = mInfoPersistentor.getSavedIMSI();
        Xlog.i(TAG, "the mSavedIMSI = [" + mSavedIMSI + "]");

        if (mSavedIMSI != null) {
            Xlog.i(TAG, "The saved mIMSI =[" + mSavedIMSI + "]");
            Xlog.i(TAG, "The current mIMSI =[" + mSimIMSI + "]");

            if (mSimIMSI != null && mSavedIMSI.equals(mSimIMSI)) {
                Xlog.w(TAG, "The SIM card and device have rigistered");
                stopService();
                return false;
            }
        }
        Xlog.w(TAG, "The sim card in this phone is "
                + "not registered, need register");
        return true;
    }

    public void startAlertDialog(int simId) {
        Intent intent = new Intent();
        intent.putExtra(SmsRegConst.EXTRA_SLOT_ID, simId);
        intent.setAction(SmsRegConst.ACTION_CONFIRM_DIALOG_START);
        startActivity(intent);
    }

    private Boolean isNeedRegisterGemini() {
        mSavedIMSI = mInfoPersistentor.getSavedIMSI();
        Xlog.i(TAG, "the mSavedIMSI = [" + mSavedIMSI + "]");

        if (mSavedIMSI != null) {
            Xlog.i(TAG, "The saved mIMSI =[" + mSavedIMSI + "]");
            Xlog.i(TAG, "The current mIMSI0 =[" + mIMSI[0] + "]");
            Xlog.i(TAG, "The current mIMSI1 =[" + mIMSI[1] + "]");

            if (mIMSI[0] != null && mIMSI[0].equals(mSavedIMSI)) {
                Xlog.i(TAG, "The SIM1 have registered already.");
                stopService();
                return false;
            }
            if (mIMSI[1] != null && mIMSI[1].equals(mSavedIMSI)) {
                Xlog.i(TAG, "The SIM2 have registered already.");
                stopService();
                return false;
            }
        }
        Xlog.w(TAG, "The sim card in this phone is not"
                + " registered, need register");
        return true;
    }
    
    public boolean isNeedSendRegisterMessage () {
        //check black list,If there is a IMSI in sim Slot ,which is not belong to the black slot IMSI.
        boolean isNeed = false;
        Xlog.i(TAG, "isNeedSendRegisterMessage+++");
        HashMap<String,String> map = BlackListUnit.getInstance().readObjectFromfile();
        if(map != null) {
            String slotBlackIMSI = null;
            slotBlackIMSI = map.get(BlackListUnit.SLOT_ONE_KEY);
            Xlog.i(TAG, "black list slot1 IMSI is " + slotBlackIMSI);
            Xlog.i(TAG, "mSimIMSI is " + mSimIMSI);
            if(mSimIMSI.equalsIgnoreCase(slotBlackIMSI)) {
                //this sim card belongs to balck list;not need to register 
                isNeed = false;
                Xlog.i(TAG, " slot1 sim card belongs to balck,not need to register");
                
            } else {
                Xlog.i(TAG, "there is a new sim card,should clear slot1 black list");
                map.put(BlackListUnit.SLOT_ONE_KEY, "");
                map.put(BlackListUnit.SLOT_TWO_KEY, "");
                BlackListUnit.getInstance().writeObjectToFile(map);
                isNeed = true;
            }
        } else {
            Xlog.e(TAG, "BlackListUnit.readObjectFromfile map is null ");
        }
        Xlog.i(TAG, "isNeedSendRegisterMessage---");
        return isNeed;
    }
    
    public int isNeedSendRegisterMessageGemini (int slotNum) {
        Xlog.i(TAG, "isNeedSendRegisterMessageGemini+++");
        int slotId = -1;
        HashMap<String,String> map = BlackListUnit.getInstance().readObjectFromfile();
        if(map != null) {
            switch(slotNum) {
            case 0 :
                if(!mIMSI[0].equalsIgnoreCase(map.get(BlackListUnit.SLOT_ONE_KEY))&&
                        !mIMSI[0].equalsIgnoreCase(map.get(BlackListUnit.SLOT_TWO_KEY))) {
                    slotId = 0;
                    map.put(BlackListUnit.SLOT_ONE_KEY, "");
                    map.put(BlackListUnit.SLOT_TWO_KEY, "");
                    BlackListUnit.getInstance().writeObjectToFile(map);
                } else {
                    slotId = -1;
                }
                break;
            case 1 :
                if(!mIMSI[1].equalsIgnoreCase(map.get(BlackListUnit.SLOT_ONE_KEY))&&
                        !mIMSI[1].equalsIgnoreCase(map.get(BlackListUnit.SLOT_TWO_KEY))) {
                    slotId = 1;
                    map.put(BlackListUnit.SLOT_ONE_KEY, "");
                    map.put(BlackListUnit.SLOT_TWO_KEY, "");
                    BlackListUnit.getInstance().writeObjectToFile(map);
                } else {
                    slotId = -1;
                }
                break;
            case 2 :
                if(!mIMSI[0].equalsIgnoreCase(map.get(BlackListUnit.SLOT_ONE_KEY))&&
                        !mIMSI[0].equalsIgnoreCase(map.get(BlackListUnit.SLOT_TWO_KEY))) {
                    slotId = 0;
                    map.put(BlackListUnit.SLOT_ONE_KEY, "");
                    BlackListUnit.getInstance().writeObjectToFile(map);
                    return slotId;
                }
                if(!mIMSI[1].equalsIgnoreCase(map.get(BlackListUnit.SLOT_ONE_KEY))&&
                        !mIMSI[1].equalsIgnoreCase(map.get(BlackListUnit.SLOT_TWO_KEY))) {
                    slotId = 1;
                    map.put(BlackListUnit.SLOT_TWO_KEY, "");
                    BlackListUnit.getInstance().writeObjectToFile(map);
                    return slotId;
                }
                slotId = -1;
                break;
            }
        
        } else {
            Xlog.e(TAG, "BlackListUnit.readObjectFromfile map is null ");
        }
        Xlog.i(TAG, "isNeedSendRegisterMessageGemini---");
        return slotId;
    }
    
    public void startReqNotifyDialog( int slotId) {
        Intent intent = new Intent();
        intent.putExtra(SmsRegConst.EXTRA_SLOT_ID, slotId);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setClass(this, SendMessageAlertActivity.class);//(SmsRegConst.ACTION_CONFIRM_DIALOG_START);
        startActivity(intent);
    }
    private void sendRegisterMessage() {
        Xlog.i(TAG, "send register message begin...");
        // check the country if it is roaming international
        String simCountryIso = mTelephonyManager.getSimCountryIso(0);
        String networkIso = mTelephonyManager.getNetworkCountryIso(0);
        Xlog.i(TAG, "simCountryIso = " + simCountryIso);
        Xlog.i(TAG, " networkIso= " + networkIso);
        if (simCountryIso != null && simCountryIso.equals(networkIso)) {
            String smsRegMsg = mSmsBuilder.getSmsContent(mXmlG, 0);
            Xlog.i(TAG, "SmsRegMsg = " + smsRegMsg);
            if (smsRegMsg != null) {
                String optAddr = mXmlG.getSmsNumber();
                Short optPort = mXmlG.getSmsPort();
                Short srcPort = mXmlG.getSrcPort();
                Xlog.i(TAG, "Operator's sms number = " + optAddr);
                Xlog.i(TAG, "Operator's sms port = " + optPort);
                Xlog.i(TAG, "Src port = " + srcPort);
                String operatorID = mXmlG.getOperatorName();
                if (mSmsManager != null) {
                    if (operatorID.equalsIgnoreCase("cu")
                            || operatorID.equalsIgnoreCase("cmcc")) {
                        if (!mIsSendMsg) {
                            PendingIntent mPendingIntent = getSendPendingIntent(0);
                            mSmsManager.sendDataMessage(optAddr, null, optPort,
                                    srcPort, smsRegMsg.getBytes(), mPendingIntent,
                                    null, PhoneConstants.GEMINI_SIM_1);
                            Xlog.i(TAG, "send register message end, "
                                    + "RegMsg is send out!");
                            mIsSendMsg = true;
                        } else {
                            Xlog.w(TAG, "RegMsg has been sent already. ");
                        }
                    } else {
                        Xlog.w(TAG, "RegMsg is not send, "
                                + "it is not the operator cu or cmcc");
                    }
                } else {
                    Xlog.e(TAG, "Send RegMsg failed, mSmsManager is null");
                }
            } else {
                Xlog.e(TAG,
                        "Send RegMsg failed, The Sms Register message is null");
                mServiceAlive = false;
            }
        }
    }

    private void sendRegisterMessageGemini(int simId) {
        Xlog.i(TAG, "send register message gemini begin...");
        String simCountryIso = mTelephonyManager.getSimCountryIso(simId);
        String networkIso = mTelephonyManager.getNetworkCountryIso(simId);
        Xlog.i(TAG, "simCountryIso = " + simCountryIso);
        Xlog.i(TAG, " networkIso= " + networkIso);
        if (simCountryIso != null && simCountryIso.equals(networkIso)) {
            String smsRegMsg = mSmsBuilder.getSmsContent(mXmlG, simId);
            Xlog.i(TAG, "SmsRegMsg = " + smsRegMsg);
            if (smsRegMsg != null) {
                String optAddr = mXmlG.getSmsNumber();
                Short optPort = mXmlG.getSmsPort();
                Short srcPort = mXmlG.getSrcPort();
                Xlog.i(TAG, "Operator's sms number = " + optAddr);
                Xlog.i(TAG, "Operator's sms port = " + optPort);
                Xlog.i(TAG, "Src port = " + srcPort);
                String operatorID = mXmlG.getOperatorName();
                if (mSmsManager != null) {
                    if (operatorID.equalsIgnoreCase("cu")
                            || operatorID.equalsIgnoreCase("cmcc")) {
                        if (!mIsSendMsg) {
                            PendingIntent mPendingIntent = getSendPendingIntent(simId);
                            mSmsManager.sendDataMessage(optAddr, null, optPort,
                                    srcPort, smsRegMsg.getBytes(), mPendingIntent,
                                    null, simId);
                            Xlog.i(TAG, "send register message end, "
                                    + "RegMsg gemini is send out!");
                            mIsSendMsg = true;
                        } else {
                            Xlog.w(TAG, "RegMsg gemini has been sent already. ");
                        }
                    } else {
                        Xlog.w(TAG, "RegMsg is not send, "
                                + "it is not the operator cu or cmcc");
                    }
                } else {
                    Xlog.e(TAG, "Send RegMsg failed, mSmsManager is null");
                }
            } else {
                Xlog.e(TAG,
                        "Send RegMsg failed, The Sms Register message is null");
                mServiceAlive = false;
            }
        } else {
            Xlog.w(TAG,
                    "SimCountryIso is not equals with NetworkCountryIso, do nothing");
        }
    }

    private PendingIntent getSendPendingIntent(int simId) {
        Xlog.i(TAG, "get Pending Intent begin, simId = " + simId);
        String imsi = null;
        Intent mIntent = new Intent();
        mIntent.setAction("android.intent.action.DM_REGISTER_SMS_RECEIVED");
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            Xlog.i(TAG, "put extra SimID, SimID = " + simId);
            mIntent.putExtra("SimID", simId);
            imsi = mTelephonyManager.getSubscriberId(simId);
        } else {
            imsi = mTelephonyManager.getSubscriberId(0);
        }
        Xlog.i(TAG, "put extra mIMSI, mIMSI = " + imsi);
        mIntent.putExtra("mIMSI", imsi);

        PendingIntent mSendPendingIntent = PendingIntent.getBroadcast(this, 0,
                mIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        Xlog.i(TAG, "get Pending Intent end");
        return mSendPendingIntent;
    }

    protected void stopService() {
        Xlog.i(TAG, "stop service.");

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (mPhoneStateListener != null) {
                Xlog.i(TAG, "unRegist service state listener for sim1.");
                mTelephonyManager.listen(mPhoneStateListener,
                        PhoneStateListener.LISTEN_NONE,
                        PhoneConstants.GEMINI_SIM_1);
                mPhoneStateListener = null;
            }

            if (mPhoneStateListenerGemini != null) {
                Xlog.i(TAG, "unRegist service state listener gemini for sim2.");
                mTelephonyManager.listen(mPhoneStateListenerGemini,
                        PhoneStateListener.LISTEN_NONE,
                        PhoneConstants.GEMINI_SIM_2);
                mPhoneStateListenerGemini = null;
            }
        } else {
            if (mPhoneStateListener != null) {
                Xlog.i(TAG, "unRegist service state listener for sim.");
                mTelephonyManager.listen(mPhoneStateListener,
                        PhoneStateListener.LISTEN_NONE,
                        0);
                mPhoneStateListener = null;
            }
        }

        if (mSimStateReceiver != null) {
            Xlog.i(TAG, "unRegist sim state receiver.");
            unregisterReceiver(mSimStateReceiver);
            mSimStateReceiver = null;
        }

        if (mSmsReceivedReceiver != null) {
            Xlog.i(TAG, "unRegist smsReceived receiver.");
            unregisterReceiver(mSmsReceivedReceiver);
            mSmsReceivedReceiver = null;
        }
        if (mSender != null) {
            Xlog.i(TAG, "cancel retry alarm message.");
            AlarmManager alarm = (AlarmManager)getSystemService(ALARM_SERVICE);
            alarm.cancel(mSender);
            mSender = null;
        }
        stopSelf();
    }
    
    public void notifyDMtoResetPermissionFile() {
        Xlog.i(TAG, "send broadcast to notifyDMtoResetPermissionFile.");
        Intent intent = new Intent();
        intent.setAction(SmsRegConst.DM_SMSREG_MESSAGE_NEW);
        sendBroadcast(intent);
    }

    static class SimStateReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.i(TAG, "sim state changed");
            intent.setAction("SIM_STATE_CHANGED");
            intent.setClass(context, SmsRegService.class);
            context.startService(intent);
        }
    }

    static class SmsReceivedReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            int resultCode = getResultCode();
            intent.setAction("REGISTER_SMS_RECEIVED");
            intent.setClass(context, SmsRegService.class);
            intent.putExtra(SMS_SENDING_RESULT_TAG, resultCode);
            context.startService(intent);
        }
    }
}
