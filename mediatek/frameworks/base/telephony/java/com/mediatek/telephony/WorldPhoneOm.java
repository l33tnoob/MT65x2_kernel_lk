/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.telephony;

import static android.Manifest.permission.READ_PHONE_STATE;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.util.Log;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.gsm.NetworkInfoWithAcT;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.worldphone.ModemSwitchHandler;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.telephony.WorldPhoneUtil;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 *@hide
 */
public class WorldPhoneOm extends Handler implements IWorldPhone {
    private static final int EVENT_GET_PLMN_PREFER_LIST_1 = 1000;
    private static final int EVENT_GET_PLMN_PREFER_LIST_2 = 1001;
    private static Object mLock = new Object();
    private static Context mContext;
    private static Phone[] mPhone;
    private static String mOperatorSpec;
    private static String mPlmnSs;
    private static String[] mImsi;
    private static String[] mGsmPlmnStrings;
    private static int mRilRadioTechnology;
    private static int mRegState;
    private static int mState;
    private static int mUserType;
    private static int mRegion;
    private static int mDenyReason;
    private static int mSuspendId;
    private static int m3gSimSlot;
    private static boolean mWaitForDesignateService;
    private static boolean sNeedQueryModemType;
    private static boolean sAllowSwitchModem;
    private static boolean[] sNeedGetPlmnPreferList;
    private static boolean[] mSuspendWaitImsi;
    private static boolean[] mFirstSelect;
    private static CommandsInterface[] mCM;
    private static UiccController[] mUiccController;
    private static IccRecords[] mIccRecordsInstance;
    private static ModemSwitchHandler mModemSwitchHandler;
    private static final String[] MCCMNC_TABLE_TYPE1 = {
        "46000", "46002", "46007"
    };
    private static final String[] MCCMNC_TABLE_TYPE3 = {
        "46001", "46006", "45407", "46003", "46005", "45502"
    };
    private static final String[] MCC_TABLE_DOMESTIC = {
        "460"
    };
    private static int sTddStandByCounter;
    private static int sFddStandByCounter;
    private static boolean sWaitInTdd;
    private static boolean sWaitInFdd;
    private static final int[] FDD_STANDBY_TIMER = {
        -1
    };
    private static final int[] TDD_STANDBY_TIMER = {
        300, 360, 420, 480, 540, 600
    };
    
    public WorldPhoneOm(Context context) {
        logd("Constructor invoked");
        mContext = context;
        sNeedQueryModemType = true;
        mCM = new CommandsInterface[PhoneConstants.GEMINI_SIM_NUM];
        mPhone = new Phone[PhoneConstants.GEMINI_SIM_NUM];
        mUiccController = new UiccController[PhoneConstants.GEMINI_SIM_NUM];
        mIccRecordsInstance = new IccRecords[PhoneConstants.GEMINI_SIM_NUM];
        mImsi = new String[PhoneConstants.GEMINI_SIM_NUM];
        sNeedGetPlmnPreferList = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        mSuspendWaitImsi = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        mFirstSelect = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        mOperatorSpec = SystemProperties.get("ro.operator.optr", NO_OP);
        logd("Operator Spec:" + mOperatorSpec);
        logd(WorldPhoneUtil.modemToString(ModemSwitchHandler.getModem()));
        resetAllProperties();
        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
            logd("3G Switch Supported");
            intentFilter.addAction(TelephonyIntents.EVENT_3G_SWITCH_DONE);
        } else {
            logd("3G Switch Not Supported");
        }
        mContext.registerReceiver(mReceiver, intentFilter);
        sTddStandByCounter = 0;
        sFddStandByCounter = 0;
        sWaitInTdd = false;
        sWaitInFdd = false;
        if (Settings.Global.getInt(context.getContentResolver(), Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 1) == 0) {
            logd("Auto select disable");
            m3gSimSlot = AUTO_SELECT_DISABLE;
        } else {
            logd("Auto select enable");
        }
    }
    
    public WorldPhoneOm(int op, Context context, Phone[] phone) {
        this(context);
        logd("Gemini Project");
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mPhone[i] = phone[i];
            mCM[i] = ((PhoneBase)((PhoneProxy)phone[i]).getActivePhone()).mCi;
            mCM[i].setOnPlmnChangeNotification(this, EVENT_GSM_PLMN_CHANGED_1 + i, null);
            mCM[i].setOnGSMSuspended(this, EVENT_GSM_SUSPENDED_1 + i, null);
            mCM[i].registerForOn(this, EVENT_RADIO_ON_1 + i, null);
        }
        mModemSwitchHandler = new ModemSwitchHandler(mCM[0]);
    }
    
    public WorldPhoneOm(int op, Context context, Phone phone) {
        this(context);
        logd("Single Card Project");
        mPhone[0] = phone;
        mCM[0] = ((PhoneBase)phone).mCi;
        mCM[0].setOnPlmnChangeNotification(this, EVENT_GSM_PLMN_CHANGED_1, null);
        mCM[0].setOnGSMSuspended(this, EVENT_GSM_SUSPENDED_1, null);
        mCM[0].registerForOn(this, EVENT_RADIO_ON_1, null);
        mModemSwitchHandler = new ModemSwitchHandler(mCM[0]);
    }

    private boolean isAllowCampOn(String plmnString, int slotId) {
        logd("[isAllowCampOn]+ " + plmnString);
        logd("Slot" + slotId + " is 3G SIM");
        logd("User type:" + mUserType);
        mRegion = getRegion(plmnString);
        if (mUserType == sType1User) {
            mWaitForDesignateService = true;
            if (mRegion == REGION_DOMESTIC) {
                if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                    mWaitForDesignateService = false;
                    mDenyReason = CAMP_ON_NOT_DENIED;
                    removeModemStandByTimer();
                    logd("In TDD modem -> stay in TDD modem");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                } else {
                    mDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD;
                    logd("In FDD modem -> switch to TDD modem");
                    logd("[isAllowCampOn]- Camp on REJECT");
                    return false;
                }
            } else if (mRegion == REGION_FOREIGN) {
                if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                    mDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                    logd("In TDD modem -> switch to FDD modem");
                    logd("[isAllowCampOn]- Camp on REJECT");
                    return false;
                } else {
                    mDenyReason = CAMP_ON_NOT_DENIED;
                    removeModemStandByTimer();
                    logd("In FDD modem -> stay in FDD modem");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                }
            }
        } else if (mUserType == sType2User) {
            mWaitForDesignateService = false;
            if (mRegion == REGION_FOREIGN) {
                if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                    mDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                    logd("In TDD modem -> switch to FDD modem");
                    logd("[isAllowCampOn]- Camp on REJECT");
                    return false;
                } else {
                    mDenyReason = CAMP_ON_NOT_DENIED;
                    logd("In FDD modem -> stay in FDD modem");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                }
            }
            if (!sAllowSwitchModem) {
                mDenyReason = CAMP_ON_NOT_DENIED;
                logd("Not allow to switch modem");
                logd("[isAllowCampOn]- Camp on OK");
                return true;
            }
            int nwType = 0;
            plmnString = plmnString.substring(0, 5);
            for (String mccmnc : MCCMNC_TABLE_TYPE1) {
                if (plmnString.equals(mccmnc)) {
                    nwType = sType1User;
                    break;
                }    
            }
            if (nwType == sType1User) {
                logd("In TD network");
                if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                    mDenyReason = CAMP_ON_NOT_DENIED;
                    logd("In TDD modem -> stay in TDD modem");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                } else {
                    mDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD;
                    logd("In FDD modem -> switch to TDD modem");
                    logd("[isAllowCampOn]- Camp on REJECT");
                    return false;
                }
            } else {
                logd("Not in TD network");
                if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                    mDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                    logd("In TDD modem -> switch to FDD modem");
                    logd("[isAllowCampOn]- Camp on REJECT");
                    return false;
                } else {
                    mDenyReason = CAMP_ON_NOT_DENIED;
                    logd("In FDD modem -> stay in FDD modem");
                    logd("[isAllowCampOn]- Camp on OK");
                    return true;
                }
            }
        } else if (mUserType == sType3User) {  
            mWaitForDesignateService = false;
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                mDenyReason = DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD;
                logd("In TDD modem -> switch to FDD modem");
                logd("[isAllowCampOn]- Camp on REJECT");
                return false;
            } else {
                mDenyReason = CAMP_ON_NOT_DENIED;
                logd("In FDD modem -> stay in FDD modem");
                logd("[isAllowCampOn]- Camp on OK");
                return true;
            }
        } else {
            logd("Unknown user type");
        }
        mDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
        logd("[isAllowCampOn]- Camp on REJECT");
        return false;
    }

    private void handleNoService() {
        logd("[handleNoService]+ Can not find service");
        logd("Type" + mUserType + " user");
        logd(WorldPhoneUtil.regionToString(mRegion));
        if (mUserType == sType1User) {
            mWaitForDesignateService = true;
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                logd("TDD modem, expecting TD service");
                if (TDD_STANDBY_TIMER[sTddStandByCounter] >= 0) {
                    if (!sWaitInTdd) {
                        sWaitInTdd = true;
                        logd("Wait " + TDD_STANDBY_TIMER[sTddStandByCounter] + "s. Timer index = " + sTddStandByCounter);
                        postDelayed(mTddStandByTimerRunnable, TDD_STANDBY_TIMER[sTddStandByCounter]*1000);
                    } else {
                        logd("Timer already set:" + TDD_STANDBY_TIMER[sTddStandByCounter] + "s");
                    }
                } else  {
                    logd("Standby in TDD modem");
                }
            } else {
                logd("FDD modem, expecting TD service");
                if (FDD_STANDBY_TIMER[sFddStandByCounter] >= 0) {
                    if (!sWaitInFdd) {
                        sWaitInFdd = true;
                        logd("Wait " + FDD_STANDBY_TIMER[sFddStandByCounter] + "s. Timer index = " + sFddStandByCounter);
                        postDelayed(mFddStandByTimerRunnable, FDD_STANDBY_TIMER[sFddStandByCounter]*1000);
                    } else {
                        logd("Timer already set:" + FDD_STANDBY_TIMER[sFddStandByCounter] + "s");
                    }
                } else {
                    logd("Standby in FDD modem");
                }
            }
        } else if (mUserType == sType2User || mUserType == sType3User) {
            mWaitForDesignateService = false;
            if (ModemSwitchHandler.getModem() == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                logd("TDD modem -> switch to FDD modem");
                handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
            } else {
                logd("FDD modem -> standby in FDD modem");
            }
        } else {
            logd("Unknow user type");
        }
        logd("[handleNoService]-");
        return;
    }

    private Runnable mTddStandByTimerRunnable = new Runnable() {
        public void run() {
            logd("TDD time out!");
            sTddStandByCounter++;
            if (sTddStandByCounter >= TDD_STANDBY_TIMER.length) {
                sTddStandByCounter = TDD_STANDBY_TIMER.length-1;
            }
            handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
        }
    };

    private Runnable mFddStandByTimerRunnable = new Runnable() {
        public void run() {
            logd("FDD time out!");
            sFddStandByCounter++;
            if (sFddStandByCounter >= FDD_STANDBY_TIMER.length) {
                sFddStandByCounter = FDD_STANDBY_TIMER.length-1;
            }
            handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
        }
    };

    private void removeModemStandByTimer() {
        if (sWaitInTdd) {
            logd("Remove TDD wait timer. Set sWaitInTdd = false");
            sWaitInTdd = false;
            removeCallbacks(mTddStandByTimerRunnable);
        }
        if (sWaitInFdd) {
            logd("Remove FDD wait timer. Set sWaitInFdd = false");
            sWaitInFdd = false;
            removeCallbacks(mFddStandByTimerRunnable);
        }
    }

    private void searchForDesignateService(String strPlmn) {
        logd("[searchForDesignateService]+ Search for TD srvice");
        if (strPlmn == null) {
            logd("[searchForDesignateService]- null source");
            return;
        }
        strPlmn = strPlmn.substring(0, 5);
        for (String mccmnc : MCCMNC_TABLE_TYPE1) {
            if (strPlmn.equals(mccmnc)) {
                logd("mUserType:" + mUserType + " mRegion:" + mRegion);
                logd(WorldPhoneUtil.modemToString(ModemSwitchHandler.getModem()));
                logd("Find TD service");
                handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
                break;
            }    
        }
        logd("[searchForDesignateService]-");
        return;
    }
    
    public void handleMessage(Message msg) {
        AsyncResult ar = (AsyncResult)msg.obj;
        switch (msg.what) {
            case EVENT_GSM_PLMN_CHANGED_1:
                logd("handleMessage : <EVENT_GSM_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_GSM_SUSPENDED_1:
                logd("handleMessage : <EVENT_GSM_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_RADIO_ON_1:
                logd("handleMessage : <EVENT_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_GSM_PLMN_CHANGED_2:
                logd("handleMessage : <EVENT_GSM_PLMN_CHANGED>");
                handlePlmnChange(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_GSM_SUSPENDED_2:
                logd("handleMessage : <EVENT_GSM_SUSPENDED>");
                handlePlmnSuspend(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_RADIO_ON_2:
                logd("handleMessage : <EVENT_RADIO_ON>");
                handleRadioOn(PhoneConstants.GEMINI_SIM_2);
                break;
            case EVENT_QUERY_MODEM_TYPE:
                logd("handleMessage : <EVENT_QUERY_MODEM_TYPE>");
                handleQueryModemType(ar);
                break;
            case EVENT_SET_RAT_GSM_ONLY:
                logd("handleMessage : <EVENT_SET_RAT_GSM_ONLY>");
                if (ar.exception == null) {
                    logd("Set RAT=2g ok");
                } else {
                    logd("Set RAT=2g fail " + ar.exception);
                }
                break;
            case EVENT_SET_RAT_WCDMA_PREF:
                logd("handleMessage : <EVENT_SET_RAT_WCDMA_PREF>");
                if (ar.exception == null) {
                    logd("Set RAT=auto ok");
                } else {
                    logd("Set RAT=auto fail " + ar.exception);
                }
                break;
            case EVENT_STORE_MODEM_TYPE:
                logd("handleMessage : <EVENT_STORE_MODEM_TYPE>");
                if (ar.exception == null) {
                    logd("Store modem type success");
                } else {
                    logd("Store modem type fail " + ar.exception);
                }
                break;
            case EVENT_GET_PLMN_PREFER_LIST_1:
                logd("handleMessage : <EVENT_GET_PLMN_PREFER_LIST>");
                handleGetPlmnPreferList(ar, PhoneConstants.GEMINI_SIM_1);
                break;
            case EVENT_GET_PLMN_PREFER_LIST_2:
                logd("handleMessage : <EVENT_GET_PLMN_PREFER_LIST>");
                handleGetPlmnPreferList(ar, PhoneConstants.GEMINI_SIM_2);
                break;
            default:
                logd("Unknown msg:" + msg.what);
        }
    }

    private void handleRadioOn(int slotId) {
        logd("Slot" + slotId);
        if (sNeedQueryModemType) {
            sNeedQueryModemType = false;
            logd("Query modem type");
            mCM[0].queryModemType(obtainMessage(EVENT_QUERY_MODEM_TYPE));
        }
        if (m3gSimSlot == UNKNOWN_3G_SLOT) {
            m3gSimSlot = get3gCapabilitySim();
        }
    }

    private void handleQueryModemType(AsyncResult ar) {
        if (ar.exception == null && ar.result != null) {
            int modemType = ((int[]) ar.result)[0];
            logd("modemType:" + modemType);
            ModemSwitchHandler.setModem(modemType);
            mCM[0].storeModemType(modemType, obtainMessage(EVENT_STORE_MODEM_TYPE));
        } else {
            logd("Query modem type fail");
        }
    }

    private void handleGetPlmnPreferList(AsyncResult ar, int slotId) {
        logd("Slot" + slotId);
        if (ar.exception == null) {
            ArrayList<NetworkInfoWithAcT> list = (ArrayList<NetworkInfoWithAcT>)ar.result;
            Collections.sort(list, new NetworkCompare());
            for (NetworkInfoWithAcT network : list) {
                logd(network.getOperatorAlphaName() + " [" + network.getPriority() + "]");
            }
            logd("<PLMN Prefer List End>");
        } else {
            logd("exception = " + ar.exception);
        }
    }
    
    private void handlePlmnChange(AsyncResult ar, int slotId) {
        logd("Slot" + slotId);
        if (m3gSimSlot == UNKNOWN_3G_SLOT) {
            m3gSimSlot = get3gCapabilitySim();
        }
        if (ar.exception == null && ar.result != null) {
            mGsmPlmnStrings = (String[])ar.result;
            for (int i=0; i<mGsmPlmnStrings.length; i++) {
                logd("mGsmPlmnStrings[" + i + "]=" + mGsmPlmnStrings[i]);
            }
            if (m3gSimSlot == slotId && mWaitForDesignateService
                    && mDenyReason != DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD) {
                searchForDesignateService(mGsmPlmnStrings[0]);
            }
            // To speed up performance in foreign countries, once get PLMN(no matter which slot)
            // determine region right away and switch modem type if needed
            mRegion = getRegion(mGsmPlmnStrings[0]);
            if (m3gSimSlot != AUTO_SELECT_DISABLE && m3gSimSlot != NO_3G_CAPABILITY && 
                    mRegion == REGION_FOREIGN) {
                handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handlePlmnSuspend(AsyncResult ar, int slotId) {
        logd("Slot" + slotId);
        if (ar.exception == null && ar.result != null) {
            mSuspendId = ((int[]) ar.result)[0];
            logd("Suspending with Id=" + mSuspendId);
            if (m3gSimSlot == slotId) {
                if (mUserType != sUnknownUser) {
                    resumeCampingProcedure(slotId);
                } else {
                    mSuspendWaitImsi[slotId] = true;
                    logd("User type unknown, wait for IMSI");
                }
            } else {
                logd("Not 3G slot, camp on OK");
                if (mCM[slotId].getRadioState().isOn()) {
                    mCM[slotId].setResumeRegistration(mSuspendId, null);
                } else {
                    logd("Radio off or unavailable, can not send EMSR");
                }
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }
    
    private final BroadcastReceiver mReceiver = new  BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            logd("[BroadcastReceiver]+");
            String action = intent.getAction();
            logd("Action: " + action);
            int slotId;
            if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                logd("slotId: " + slotId + " simStatus: " + simStatus);
                if (simStatus.equals(IccCardConstants.INTENT_VALUE_ICC_IMSI)) {
                    if (m3gSimSlot == UNKNOWN_3G_SLOT) {
                        m3gSimSlot = get3gCapabilitySim();
                    }
                    //mIccRecordsInstance[slotId] = ((PhoneBase)mCM[m3gSimSlot]).mIccRecords.get();
                    mUiccController[slotId] = UiccController.getInstance(slotId);
                    mIccRecordsInstance[slotId] = mUiccController[slotId].getIccRecords(UiccController.APP_FAM_3GPP);
                    mImsi[slotId] = mIccRecordsInstance[slotId].getIMSI();
                    logd("mImsi[" + slotId + "]:" + mImsi[slotId]);
                    if (sNeedGetPlmnPreferList[slotId]) {
                        sNeedGetPlmnPreferList[slotId] = false;
                        mCM[slotId].getCurrentPOLList(obtainMessage(EVENT_GET_PLMN_PREFER_LIST_1 + slotId));
                    }
                    if (slotId == m3gSimSlot) {
                        logd("3G slot");
                        mUserType = getUserType(mImsi[slotId]);
                        if (mFirstSelect[slotId]) {
                            mFirstSelect[slotId] = false;
                            if (mUserType == sType1User) {
                                mWaitForDesignateService = true;
                                if (mRegion == REGION_DOMESTIC) {
                                    handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
                                }
                            } else if (mUserType == sType2User) {
                                mWaitForDesignateService = false;
                            } else if (mUserType == sType3User) {
                                mWaitForDesignateService = false;
                                handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
                            }
                        }
                        if (mSuspendWaitImsi[slotId]) {
                            mSuspendWaitImsi[slotId] = false;
                            logd("IMSI fot slot" + slotId + " now ready, resuming PLMN:" 
                                    + mGsmPlmnStrings[0] + " with ID:" + mSuspendId);
                            resumeCampingProcedure(slotId);
                        }
                    } else {
                        // not 3G slot, do not store into mUserType
                        getUserType(mImsi[slotId]);
                        logd("Not 3G slot");
                    }
                } else if (simStatus.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
                    mImsi[slotId] = "";
                    mFirstSelect[slotId] = true;
                    mSuspendWaitImsi[slotId] = false;
                    sNeedGetPlmnPreferList[slotId] = true;
                    if (slotId == m3gSimSlot) {
                        logd("3G Sim removed, no world phone service");
                        mUserType = sUnknownUser;
                        mDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
                        mWaitForDesignateService = false;
                        sAllowSwitchModem = true;
                        if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
                            m3gSimSlot = UNKNOWN_3G_SLOT;
                        } else {
                            m3gSimSlot = DEFAULT_3G_SLOT;
                        }
                    } else {
                        logd("Slot" + slotId + " is not 3G slot");
                    }
                }
            } else if (action.equals(TelephonyIntents.EVENT_3G_SWITCH_DONE)) {
                if (m3gSimSlot != AUTO_SELECT_DISABLE) {
                    m3gSimSlot = intent.getIntExtra(TelephonyIntents.EXTRA_3G_SIM, 0);
                }
                handle3gSwitched();
            } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
                if (serviceState != null) {
                    slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                    mPlmnSs = serviceState.getOperatorNumeric();
                    mRilRadioTechnology = serviceState.getRilVoiceRadioTechnology();
                    mRegState = serviceState.getRegState();
                    mState = serviceState.getState();
                    logd("m3gSimSlot=" + m3gSimSlot);
                    logd(WorldPhoneUtil.modemToString(ModemSwitchHandler.getModem()));
                    logd("slotId: " + slotId + " isRoaming: " + serviceState.getRoaming()
                            + " isEmergencyOnly: " + serviceState.isEmergencyOnly());
                    logd("mPlmnSs: " + mPlmnSs);
                    logd("mState: " + WorldPhoneUtil.stateToString(mState));
                    logd("mRegState: " + WorldPhoneUtil.regStateToString(mRegState));
                    logd("mRilRadioTechnology: " + serviceState.rilRadioTechnologyToString(mRilRadioTechnology));
                    if (slotId == m3gSimSlot) {
                        if (mState == ServiceState.STATE_IN_SERVICE) {
                            sAllowSwitchModem = false;
                        } else {
                            sAllowSwitchModem = true;
                        }
                        logd("sAllowSwitchModem: " + sAllowSwitchModem);
                    }
                    if ((mRegState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING
                            && mState == ServiceState.STATE_OUT_OF_SERVICE)) {
                        if (slotId == m3gSimSlot) {
                            handleNoService();
                        }
                    }
                }
            }
            logd("[BroadcastReceiver]-");
        }
    };

    private void handle3gSwitched() {
        if (m3gSimSlot == NO_3G_CAPABILITY) {
            logd("3G capability turned off");
            removeModemStandByTimer();
            mUserType = sUnknownUser;
        } else if (m3gSimSlot == AUTO_SELECT_DISABLE) {
            logd("Auto Network Selection Disabled");
            removeModemStandByTimer();
        } else {
            logd("3G capability in slot" + m3gSimSlot);
            if (mImsi[m3gSimSlot].equals("")) {
                // may caused by receive 3g switched intent when boot up 
                logd("3G slot IMSI not ready");
                mUserType = sUnknownUser;
                return;
            }
            mUserType = getUserType(mImsi[m3gSimSlot]);
            if (mUserType == sType1User) {
                mWaitForDesignateService = true;
            } else if (mUserType == sType2User || mUserType == sType3User) {
                mWaitForDesignateService = false;
            } else {
                logd("Unknown user type");
            }
            if (mGsmPlmnStrings != null) {
                mRegion = getRegion(mGsmPlmnStrings[0]);
            }
            if (mRegion != REGION_UNKNOWN && mUserType != sUnknownUser) {
                mFirstSelect[m3gSimSlot] = false;
                if (mUserType == sType2User) {
                    if (mRegion == REGION_FOREIGN) {
                        handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
                    }
                } else if (mUserType == sType1User && mRegion == REGION_DOMESTIC) {
                    handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
                } else {
                    handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
                }
            }
        }
    }

    public void setNetworkSelectionMode(int mode) {
        if (mode == SELECTION_MODE_AUTO) {
            logd("Network Selection <AUTO>");
            Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 1);
            m3gSimSlot = get3gCapabilitySim();
        } else {
            logd("Network Selection <MANUAL>");
            Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.WORLD_PHONE_AUTO_SELECT_MODE, 0);
            m3gSimSlot = AUTO_SELECT_DISABLE;
        }
        handle3gSwitched();
    }

    private void handleSwitchModem(int toModem) {
        if (toModem == ModemSwitchHandler.getModem()) {
            if (toModem == ModemSwitchHandler.MODEM_SWITCH_MODE_TDD) {
                logd("Already in TDD modem");
            } else {
                logd("Already in FDD modem");
            }
            return;
        } else {
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                if (mPhone[i].getState() != PhoneConstants.State.IDLE) {
                    logd("Phone" + i + " is not idle, modem switch not allowed");
                    return;
                }
            }
            removeModemStandByTimer();
            if (toModem == ModemSwitchHandler.MODEM_SWITCH_MODE_FDD) {
                logd("Switching to FDD modem");
                ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
            } else {
                logd("Switching to TDD modem");
                ModemSwitchHandler.switchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
            }
            resetNetworkProperties();
        }
    }

    private void resumeCampingProcedure(int slotId) {
        logd("Resume camping slot" + slotId + " mSuspendId:" + mSuspendId);
        if (isAllowCampOn(mGsmPlmnStrings[0], slotId)) {
            if (mCM[slotId].getRadioState().isOn()) {
                mCM[slotId].setResumeRegistration(mSuspendId, null);
            } else {
                logd("Radio off or unavailable, can not send EMSR");
            }
        } else {
            logd("Because: " + WorldPhoneUtil.denyReasonToString(mDenyReason));
            if (mDenyReason == DENY_CAMP_ON_REASON_NEED_SWITCH_TO_FDD) {
                handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_FDD);
            } else if (mDenyReason == DENY_CAMP_ON_REASON_NEED_SWITCH_TO_TDD) {
                handleSwitchModem(ModemSwitchHandler.MODEM_SWITCH_MODE_TDD);
            }
        }
    }

    /* Might return -1 if 3G is off */
    private int get3gCapabilitySim() {
        int slot3g;
        int capability = SystemProperties.getInt("gsm.baseband.capability", 3);
        int capability2 = SystemProperties.getInt("gsm.baseband.capability2", 3);
        if (capability > 3 || capability2 > 3) {
            slot3g = mPhone[0].get3GCapabilitySIM();
            logd("m3gSimSlot=" + slot3g);
            return slot3g;
        } else {
            logd("3G turn off");
            return -1;
        }
    }
        
    private void setRatMode(int ratMode, int slotId) {
        if (ratMode == SET_RAT_TO_AUTO) {
            logd("[setRatMode] Setting slot" + slotId + " RAT=auto");
            mCM[slotId].setPreferredNetworkType(Phone.NT_MODE_WCDMA_PREF,
                obtainMessage(EVENT_SET_RAT_WCDMA_PREF));       
        } else {
            logd("[setRatMode] Setting slot" + slotId + " RAT=2G");
            mCM[slotId].setPreferredNetworkType(Phone.NT_MODE_GSM_ONLY,
                obtainMessage(EVENT_SET_RAT_GSM_ONLY));
        }
    }

    private void resetAllProperties() {
        logd("Reseting all properties");
        mGsmPlmnStrings = null;
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mFirstSelect[i] = true;
            sNeedGetPlmnPreferList[i] = true;
        }
        sAllowSwitchModem = true;
        mWaitForDesignateService = false;
        mDenyReason = DENY_CAMP_ON_REASON_UNKNOWN;
        resetSimProperties();
        resetNetworkProperties();
    }
    
    private void resetNetworkProperties() {
        logd("[resetNetworkProperties]");
        synchronized (mLock) {        
            mRegion = REGION_UNKNOWN;
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                mSuspendWaitImsi[i] = false;
            }
        }
    }

    private void resetSimProperties() {
        logd("[resetSimProperties]");
        synchronized (mLock) {        
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                mImsi[i] = "";
            }
            mUserType = sUnknownUser;
            if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
                m3gSimSlot = UNKNOWN_3G_SLOT;
            } else {
                m3gSimSlot = DEFAULT_3G_SLOT;
            }
        }
    }

    private void restartSelection(String reason) {
        // clean all state, properties
        logd("[restartSelection] Restarting from TDD modem");
        logd("Reason:" + reason);
        resetAllProperties();
    }

    private int getUserType(String simImsi) {
        if (simImsi != null && !simImsi.equals("")) {
            simImsi = simImsi.substring(0, 5);
            logd("[getUserType] simPlmn:" + simImsi);
            for (String mccmnc : MCCMNC_TABLE_TYPE1) {
                if (simImsi.equals(mccmnc)) {
                    logd("[getUserType] Type1 user");
                    return sType1User;
                }    
            }
            for (String mccmnc : MCCMNC_TABLE_TYPE3) {
                if (simImsi.equals(mccmnc)) {
                    logd("[getUserType] Type3 user");
                    return sType3User;
                }    
            }            
            logd("[getUserType] Type2 user");
            return sType2User;
        } else {
            logd("[getUserType] null simImsi");
            return sUnknownUser;
        }
    }

    private int getRegion(String srcMccOrPlmn) {
        if (srcMccOrPlmn == null) {
            logd("[getRegion] null source");
            return REGION_UNKNOWN;
        }
        String currentMcc = srcMccOrPlmn.substring(0, 3);
        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (currentMcc.equals(mcc)) {
                logd("[getRegion] REGION_DOMESTIC");
                return REGION_DOMESTIC;
            }    
        }
        logd("[getRegion] REGION_FOREIGN");
        return REGION_FOREIGN;
    }

    public void disposeWorldPhone() {
        mContext.unregisterReceiver(mReceiver);
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mCM[i].unSetOnPlmnChangeNotification(this);
            mCM[i].unSetOnGSMSuspended(this);
            mCM[i].unregisterForOn(this);
        }
    }

    class NetworkCompare implements Comparator<NetworkInfoWithAcT> {
        public int compare(NetworkInfoWithAcT object1, NetworkInfoWithAcT object2) {
            return (object1.getPriority() - object2.getPriority());
        }
    }
    
    private static void logd(String msg) {
        Log.d(LOG_TAG, "[WPOM]" + msg);
    }
}
