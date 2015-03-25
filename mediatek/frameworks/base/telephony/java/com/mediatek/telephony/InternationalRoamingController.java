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

import android.app.ActivityManagerNative;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.text.TextUtils;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneProxy;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.cdma.CDMAPhone;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.internationalroaming.IInternationalRoamingController;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.common.telephony.internationalroaming.strategy.ICardStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.IDataStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.IGeneralStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.INetworkSelectionStrategy;
import com.mediatek.xlog.Xlog;

/**
 * International romaing controller, manage international roaming related
 * features, mainly focus on network selection part.
 * 
 * @hide
 */
public class InternationalRoamingController extends Handler implements
        IInternationalRoamingController {
    private static final boolean DEBUG = true;
    private static final String LOG_TAG_PHONE = "PHONE";
    private static final String TAG_PREFIX = "[IRC]";

    private static final String ICCID_STRING_FOR_NO_SIM = "N/A";
    private static final String INTENT_ACTION_START_SWITCH_PHONE = "com.mediatek.intent.action.START_RADIO_TECHNOLOGY";
    private static final String CURRENT_NETWORK_MODE = "ct.internationalcard.network.mode";
    private static final String PROP_INTERNATIONAL_CARD_TYPE = "gsm.internationalcard.type";
    private static final String PREF_INTERNATIONAL_SIM_INFO = "mediatek_internationalcard_info";
    private static final String KEY_INTERNATIONAL_SIM_ICCID = "com.mediatek.internationalcard.iccid";
    private static final String KEY_INTERNATIONAL_SIM_TYPE = "com.mediatek.internationalcard.type";
    private static final String KEY_INTERNATIONAL_CDMA_IMSI = "com.mediatek.internationalcard.cdma.imsi";
    private static final String KEY_INTERNATIONAL_GSM_IMSI = "com.mediatek.internationalcard.gsm.imsi";

    // Store the current network mode and mcc information for memo network
    // selection.
    private static final String PREF_INTERNATIONAL_NETWORK_INFO = "international_network_info";
    private static final String KEY_INTERNATIONAL_LAST_PHONE_TYPE = "last.phone.type";
    private static final String KEY_INTERNATIONAL_LAST_NETWORK_MCC = "last.network.mcc";

    // Preference keys used for test, dynamically change home network mcc and
    // switch phone delay time.
    private static final String KEY_TEST_HOME_NETWORK_MCC = "test.home.network.mcc";
    private static final String KEY_TEST_SWITCH_PHONE_DELAY = "test.switch.phone.delay";

    // Timeout duration to delay switching phone when no radio service.
    private static final int RADIO_NO_SERVICE_SCREENON_TIMEOUT = 20 * 1000;
    private static final int RADIO_NO_SERVICE_SCREENOFF_TIMEOUT = 3 * 60 * 1000;

    // SIM card inserted status.
    private static final int SIM_INSERTED_STATUS_UNKNOWN = -1;
    private static final int SIM_INSERTED_STATUS_NONE = 0;
    private static final int SIM_INSERTED_STATUS_OLD = 1;
    private static final int SIM_INSERTED_STATUS_NEW = 2;

    // MCC length in operator numeric.
    private static final int OPERATOR_MCC_LENGTH = 3;

    // Time duration in which we switch phone directly if no radio service,
    // used to speed up network registration when device first booted or back
    // from airplane mode, use 1min currently.
    private static final int SPEED_UP_SWITCH_DURATION = 90 * 1000;

    private static final int EVENT_DUAL_PHONE_AVAILABLE = 1;
    private static final int EVENT_DUAL_PHONE_POWER_ON = 2;
    private static final int EVENT_CDMA_PLMN_CHANGED = 3;
    private static final int EVENT_GSM_SUSPENDED = 4;
    private static final int EVENT_GSM_PLMN_CHANGED = 5;
    private static final int EVENT_SERVICE_STATE_CHANGED = 6;
    private static final int EVENT_RADIO_NO_SERVICE = 7;
    private static final int EVENT_EMPTY_CDMA_AVOID_NETWORK_LIST = 8;

    // SIM switch handler related message start from 100.
    private static final int EVENT_RADIO_OFF_GSM = 100;

    // Card type handler related message start from 200.
    private static final int EVENT_ICC_CHANGED = 200;

    private Context mContext;

    private PhoneProxy mDualModePhone;
    private PhoneProxy mGsmPhone;
    private PhoneBase mDualActivePhone;
    private PhoneBase mGsmActivePhone;
    private CommandsInterface mDualModePhoneCM;

    // GeminiPhone instance, used to obtain message and send to it to notify
    // switch phone started/finished.
    private Handler mGeminiPhoneHandler;
    private Message mStartSwitchPoneMsg;
    private Message mFinishSwitchPoneMsg;

    // International roaming related strategies.
    private ICardStrategy mCardStrategy;
    private IDataStrategy mDataStrategy;
    private IGeneralStrategy mGeneralStrategy;
    private INetworkSelectionStrategy mNetworkSelectionStrategy;

    private final SimSwithHandler mSimSwithHandler;
    private final CardTypeHandler mCardTypeHandler;

    private boolean mHasSearchedOnGsm;
    private boolean mHasSearchedOnCdma;
    private boolean mUnderSimSwitching;
    private String[] mGsmPlmnStrings;

    // Whether to ignore the searched state when resuming roaming CDMA
    // registration, used for user manual select CDMA network, or else
    // the device may not register on roaming CDMA because it didn't search on
    // GSM mode.
    private boolean mIgnoreSearchedState;

    // Whether there is alreay a no service timeout message queued.
    private boolean mNoServiceQueued;

    // Radio reboot time or radio on from airplane mode time.
    private long mRadioRebootTime;

    // Whether the radio on is triggered by SIM switch.
    private boolean mRadioOnFromSimSwitch;

    // Whether the radio on is triggered by turning off airplane mode.
    private boolean mRadioOnFromAirplaneMode;

    // Whether it is the first time to register on network, that means the
    // device is just booted or back from airplane mode.
    private boolean mIsFirstRegistration;

    // Whether there is GSM/CDMA network register suspend, in this case, we need
    // to call resume network register or switch phone to resume modem.
    private boolean mRegisterSuspend;

    // SIM id of the dual mode slot.
    private int mDualSimId;

    // Whether there is SIM inserted in the dual mode slot.
    private boolean mSimInserted;
    private int mSimInsertedStatus = SIM_INSERTED_STATUS_UNKNOWN;

    private IccRecords mIccRecords;
    private UiccController mUiccController;
    private UiccCard mUiccCard;
    private boolean mImsiLoaded;
    private String mCdmaImsi;
    private String mGsmImsi;    
    private int mCardType = InternationalRoamingConstants.CARD_TYPE_UNKNOWN;

    private int mPrevRegState = ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING;
    private String mPrevOpNumeric = "00000";

    public InternationalRoamingController(Context context, Object dualModePhone, Object gsmPhone,
            Handler handler) {
        mContext = context;
        mDualModePhone = (PhoneProxy) dualModePhone;
        mGsmPhone = (PhoneProxy) gsmPhone;
        mGeminiPhoneHandler = handler;
        logd("InternationalRoamingController mContext = " + mContext + ", mDualModePhone = "
                + mDualModePhone + ", mGsmPhone = " + mGsmPhone + ", mGeminiPhoneHandler = "
                + mGeminiPhoneHandler);

        mDualActivePhone = (PhoneBase) (mDualModePhone.getActivePhone());
        mGsmActivePhone = (PhoneBase) (mGsmPhone.getActivePhone());
        mDualSimId = mDualModePhone.getMySimId();
        mRadioRebootTime = System.currentTimeMillis();
        mIsFirstRegistration = true;

        initStrategies();

        // TODO: shall we move it to background thread?
        mSimSwithHandler = new SimSwithHandler(Looper.myLooper());
        mCardTypeHandler = new CardTypeHandler(Looper.myLooper());

        // Command interfaces will not change during switch phone.
        mDualModePhoneCM = ((PhoneBase) (mDualModePhone.getActivePhone())).mCi;
        mDualModePhoneCM.registerForAvailable(this, EVENT_DUAL_PHONE_AVAILABLE, null);
        mDualModePhoneCM.registerForOn(this, EVENT_DUAL_PHONE_POWER_ON, null);
        mDualModePhoneCM.setOnPlmnChangeNotification(this, EVENT_GSM_PLMN_CHANGED, null);
        mDualModePhoneCM.setOnGSMSuspended(this, EVENT_GSM_SUSPENDED, null);

        mUiccController = UiccController.getInstance(mDualSimId);
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mAirplaneModeReceiver, filter);
    }

    @Override
    public int switchPhone(int mode, boolean fromUser) {
        logd("switchPhone: mode = " + mode + ", fromUser = " + fromUser + ", phone type = "
                + mDualModePhone.getPhoneType());

        if (fromUser) {
            mHasSearchedOnGsm = false;
            mHasSearchedOnCdma = false;
            mIgnoreSearchedState = true;
        }

        if (mUnderSimSwitching || !mGsmActivePhone.mCi.getRadioState().isAvailable()) {
            logd("switchPhone failed, mode = " + mode + ",mUnderSimSwitching = "
                    + mUnderSimSwitching);
            return InternationalRoamingConstants.SIM_SWITCH_RESULT_ERROR_BUSY;
        }

        if (!mSimInserted) {
            logd("switchPhone failed because there is no SIM inserted.");
            return InternationalRoamingConstants.SIM_SWITCH_RESULT_ERROR_NO_SIM;
        }

        // TODO: Is there a better way to avoid phone switch if the card type is
        // single CDMA before we know the really card type.
        final int phoneType = mDualModePhone.getPhoneType();
        if (mCardType == InternationalRoamingConstants.CARD_TYPE_SINGLE_CDMA
                && phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            logd("switchPhone failed because card type is single CDMA.");
            return InternationalRoamingConstants.SIM_SWITCH_RESULT_ERROR_CARD_TYPE;
        }

        int targetMode = mode;
        if (mode == InternationalRoamingConstants.SIM_SWITCH_MODE_INVERSE) {
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                targetMode = InternationalRoamingConstants.SIM_SWITCH_MODE_GSM;
            } else {
                targetMode = InternationalRoamingConstants.SIM_SWITCH_MODE_CDMA;
            }
        }

        if ((phoneType == PhoneConstants.PHONE_TYPE_CDMA && targetMode == InternationalRoamingConstants.SIM_SWITCH_MODE_CDMA)
                || (phoneType == PhoneConstants.PHONE_TYPE_GSM && targetMode == InternationalRoamingConstants.SIM_SWITCH_MODE_GSM)) {
            return InternationalRoamingConstants.SIM_SWITCH_RESULT_ERROR_SAMETYPE;
        }

        // Give a chance for different operator to decide whether we need really
        // do the switch phone process.
        int preCheck = mNetworkSelectionStrategy.onPreSwitchPhone();
        if (preCheck != InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS) {
            return preCheck;
        }

        startToSwitchPhone();
        return InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS;
    }

    @Override
    public boolean isUnderSimSwitching() {
        return mUnderSimSwitching;
    }

    @Override
    public int getDualModePhoneCardType() {
        return mCardType;
    }

    @Override
    public boolean hasSearchedOnGsm() {
        return mHasSearchedOnGsm;
    }

    @Override
    public boolean hasSearchedOnCdma() {
        return mHasSearchedOnCdma;
    }

    @Override
    public boolean ignoreSearchedState() {
        return mIgnoreSearchedState;
    }

    @Override
    public boolean isFirstRegistration() {
        return mIsFirstRegistration;
    }

    @Override
    public boolean isHomeNetwork(String mcc) {
        // For home network MCC test.
        SharedPreferences preference = mContext.getSharedPreferences(PREF_INTERNATIONAL_NETWORK_INFO, 0);
        String testMcc = preference.getString(KEY_TEST_HOME_NETWORK_MCC, null);
        if (mcc != null && testMcc != null) {
            logd("isHomeNetwork: testMcc = " + testMcc + ",mcc = " + mcc);
            return mcc.startsWith(testMcc);
        }

        return mGeneralStrategy.isHomeNetwork(mcc);
    }

    @Override
    public boolean needToBootOnGsm(String iccid) {
        final int simInsertedStatus = checkSimInsertedStatus(iccid);
        updateSimCardInfoIfNeeded(simInsertedStatus, iccid);
        logd("needToBootOnGsm: iccid = " + iccid + ",simInsertedStatus = " + simInsertedStatus);

        return mNetworkSelectionStrategy.needToBootOnGsm();
    }

    @Override
    public boolean needToBootOnCDMA(String iccid) {
        final int simInsertedStatus = checkSimInsertedStatus(iccid);
        updateSimCardInfoIfNeeded(simInsertedStatus, iccid);
        logd("needToBootOnCDMA: iccid = " + iccid + ",simInsertedStatus = " + simInsertedStatus);

        return (simInsertedStatus == SIM_INSERTED_STATUS_NEW) 
                || mNetworkSelectionStrategy.needToBootOnCdma();
    }

    @Override
    public void resumeRegistration(int networkMode, int suspendedSession) {
        logd("resumeRegistration: networkMode = " + networkMode + ", suspendedSession = " + suspendedSession
                + ", mRegisterSuspend = " + mRegisterSuspend);
        if (networkMode == InternationalRoamingConstants.RESUME_NW_GSM) {
            mDualModePhoneCM.setResumeRegistration(suspendedSession, null);
        } else {
            mDualActivePhone.resumeCdmaRegister(null);
        }
        mRegisterSuspend = false;
    }

    @Override
    public String getLastNetworkMcc() {
        SharedPreferences preference = mContext.getSharedPreferences(PREF_INTERNATIONAL_NETWORK_INFO, 0);
        return preference.getString(KEY_INTERNATIONAL_LAST_NETWORK_MCC, null);
    }

    @Override
    public int getLastPhoneType() {
        SharedPreferences preference = mContext.getSharedPreferences(PREF_INTERNATIONAL_NETWORK_INFO, 0);
        return preference.getInt(KEY_INTERNATIONAL_LAST_PHONE_TYPE, PhoneConstants.PHONE_TYPE_NONE);
    }

    /**
     * Clear avoid network list, check whether the operation execute succeed in
     * message EVENT_EMPTY_AVOID_CDMA_NETWORK_LIST.
     */
    @Override
    public void clearCdmaAvoidNetworkList() {
        if (mDualModePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            logd("Start to clear CMDA avoid network list.");
            CDMAPhone phone = (CDMAPhone) (((PhoneProxy) mDualModePhone).getActivePhone());
            phone.setAvoidSYS(false, obtainMessage(EVENT_EMPTY_CDMA_AVOID_NETWORK_LIST));
        } else {
            logd("Fail to clear avoid network list because it is GSM phone");
        }
    }

    @Override
    public void dispose() {
        mCardStrategy.dispose();
        mDataStrategy.dispose();
        mGeneralStrategy.dispose();
        mNetworkSelectionStrategy.dispose();

        mDualModePhoneCM.unregisterForAvailable(this);
        mDualModePhoneCM.unregisterForOn(this);
        mDualModePhoneCM.unSetOnPlmnChangeNotification(this);
        mDualModePhoneCM.unSetOnGSMSuspended(this);

        mDualModePhone.unregisterForServiceStateChanged(this);
        
        // Unregister ICC events for there is no SIM card inserted.
        mUiccController.unregisterForIccChanged(mCardTypeHandler);

        mContext.unregisterReceiver(mAirplaneModeReceiver);
    }

    @Override
    public void handleMessage(Message msg) {
        logd("handleMessage: msg.what = " + msg.what);
        AsyncResult ar = (AsyncResult) msg.obj;

        switch (msg.what) {
            case EVENT_DUAL_PHONE_AVAILABLE:
                logd("EVENT_DUAL_PHONE_AVAILABLE: phone type = " + mDualModePhone.getPhoneType());
                removeNoServiceMessage();

                // TODO: discuss with VIA to make it the same as GSM.
                if (mDualModePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                    CDMAPhone phone = (CDMAPhone) (((PhoneProxy) mDualModePhone).getActivePhone());
                    phone.unregisterForMccMncChange(this);
                    phone.registerForMccMncChange(this, EVENT_CDMA_PLMN_CHANGED, null);
                }

                mGeneralStrategy.onDualPhoneRadioAvailable();
                break;

            case EVENT_DUAL_PHONE_POWER_ON:
                removeNoServiceMessage();
                mDualModePhone.registerForServiceStateChanged(this, EVENT_SERVICE_STATE_CHANGED, null);

                int radioOnReason = InternationalRoamingConstants.RADIO_ON_REASON_UNKNOWN;
                // Reset radio reboot time if radio on is triggered by turning
                // off airplane mode, the flag will be reset in onDualPhoneRadioOn.
                if (mRadioOnFromAirplaneMode) {
                    mRadioRebootTime = System.currentTimeMillis();
                    mHasSearchedOnGsm = false;
                    mHasSearchedOnCdma = false;
                    mIgnoreSearchedState = false;
                    mIsFirstRegistration = true;
                    radioOnReason = InternationalRoamingConstants.RADIO_ON_REASON_TURNOFF_AIRPLANE_MODE;
                    mRadioOnFromAirplaneMode = false;
                } else if (mRadioOnFromSimSwitch) {
                    radioOnReason = InternationalRoamingConstants.RADIO_ON_REASON_SWITCH_PHONE;
                    mRadioOnFromSimSwitch = false;
                }
                logd("EVENT_DUAL_PHONE_POWER_ON: radioOnReason = " + radioOnReason);
                mGeneralStrategy.onDualPhoneRadioOn(radioOnReason);
                break;

            case EVENT_CDMA_PLMN_CHANGED:
                // Found CDMA radio signal.
                removeNoServiceMessage();
                mHasSearchedOnCdma = true;
                mRegisterSuspend = true;

                // [WA] VIA CDMA modem will send CDMA plmn changed URC when radio on
                // even if there is no card inserted.
                if (mSimInserted) {
                    String mccMnc = (String) ar.result;
                    logd("EVENT_CDMA_PLMN_CHANGED: mccMnc = " + mccMnc);
                    mNetworkSelectionStrategy.onCdmaPlmnChanged(mccMnc);
                } else {
                    logd("Fake EVENT_CDMA_PLMN_CHANGED, no SIM inserted.");
                    resumeRegistration(InternationalRoamingConstants.RESUME_NW_CDMA, 0);
                }
                break;

            case EVENT_GSM_PLMN_CHANGED:
                logd("EVENT_GSM_PLMN_CHANGED");
                // Found GSM radio signal.
                removeNoServiceMessage();
                mHasSearchedOnGsm = true;
                mRegisterSuspend = true;
                
                if (ar.exception == null && ar.result != null) {
                    mGsmPlmnStrings = (String[]) ar.result;
                    for (int i = 0; i < mGsmPlmnStrings.length; i++) {
                        logd("EVENT_GSM_PLMN_CHANGED: i = " + i + ", mGsmPlmnStrings="
                                + mGsmPlmnStrings[i]);
                    }
                }
                break;

            case EVENT_GSM_SUSPENDED:
                logd("EVENT_GSM_SUSPENDED");
                if (ar.exception == null && ar.result != null) {
                    int suspendedSession = ((int[]) ar.result)[0];
                    logd("EVENT_GSM_SUSPENDED: suspendedSession = " + suspendedSession);
                    mNetworkSelectionStrategy.onGsmSuspend(mGsmPlmnStrings, suspendedSession);
                }
                break;

            case EVENT_SERVICE_STATE_CHANGED:
                ServiceState serviceState = (ServiceState) ar.result;
                final int regState = serviceState.getRegState();
                logd("EVENT_SERVICE_STATE_CHANGED: regState = " + regState);

                if (regState == ServiceState.REGISTRATION_STATE_HOME_NETWORK
                        || regState == ServiceState.REGISTRATION_STATE_ROAMING) {
                    // Running into normal service, reset the flags.
                    mHasSearchedOnGsm = false;
                    mHasSearchedOnCdma = false;
                    mIgnoreSearchedState = false;
                    mIsFirstRegistration = false;

                    if (regState != mPrevRegState
                            || !mPrevOpNumeric.equals(serviceState.getOperatorNumeric())) {
                        if (regState == ServiceState.REGISTRATION_STATE_ROAMING) {
                            mDataStrategy.onRegisterRoamingNetwork(serviceState
                                    .getOperatorNumeric());
                        } else {
                            mDataStrategy.onRegisterHomeNetwork(serviceState.getOperatorNumeric());
                        }
                    }

                    // Registered, save network information and remove no
                    // service message, we need to save network info after data
                    // strategy handle roaming because it will use the pervious
                    // saved network.
                    removeNoServiceMessage();
                    saveCurrentNetworkInfo(serviceState.getOperatorNumeric());
                } else {
                    if (mDualModePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                        mHasSearchedOnGsm = true;
                    } else if (mDualModePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                        mHasSearchedOnCdma = true;
                    }

                    if (regState == ServiceState.REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING
                            && mDualModePhone.getServiceState().getState() != ServiceState.STATE_POWER_OFF
                            && mDualModePhone.getIccCard().getState() == IccCardConstants.State.READY) {
                        logd("No service: mNoServiceQueued = " + mNoServiceQueued);
                        if (!mNoServiceQueued) {
                            sendNoServiceMessage(calcSwitchPhoneDelayedTime());
                        }
                    }
                }

                mPrevRegState = regState;
                mPrevOpNumeric = serviceState.getOperatorNumeric();
                break;

            case EVENT_RADIO_NO_SERVICE:
                logd("EVENT_RADIO_NO_SERVICE: mIgnoreSearchedState = " + mIgnoreSearchedState);
                mIgnoreSearchedState = false;
                mIsFirstRegistration = false;
                mNetworkSelectionStrategy.onNoService(mDualModePhone.getPhoneType());
                break;

            case EVENT_EMPTY_CDMA_AVOID_NETWORK_LIST:
                // Record the result of clearing avoid network list operation.
                logd("Empty cdma network list: exception = " + ar.exception);
                break;

            default:
                logd("Should never run into this case: msg = " + msg);
                break;
        }
    }

    /**
     * Initial concrete strategies base on the current operator through
     * MediatekClassFactory.
     */
    private void initStrategies() {
        mCardStrategy = (ICardStrategy) (MediatekClassFactory.createInstance(ICardStrategy.class,
                this, mContext, mDualModePhone, mGsmPhone));
        mDataStrategy = (IDataStrategy) (MediatekClassFactory.createInstance(IDataStrategy.class,
                this, mContext, mDualModePhone, mGsmPhone));
        mGeneralStrategy = (IGeneralStrategy) (MediatekClassFactory.createInstance(
                IGeneralStrategy.class, this, mContext, mDualModePhone, mGsmPhone));
        mNetworkSelectionStrategy = (INetworkSelectionStrategy) (MediatekClassFactory
                .createInstance(INetworkSelectionStrategy.class, this, mContext, mDualModePhone,
                        mGsmPhone));
    }

    /**
     * Calculate the delayed time duration of switch phone process.
     * 
     * @return
     */
    private int calcSwitchPhoneDelayedTime() {
        final int delayedTime;
        // For switch phone delay time duration test.
        SharedPreferences preference = mContext.getSharedPreferences(PREF_INTERNATIONAL_NETWORK_INFO, 0);
        final int testDelayTime = preference.getInt(KEY_TEST_SWITCH_PHONE_DELAY, -1);
        if (testDelayTime != -1) {
            delayedTime = testDelayTime;
        } else if (System.currentTimeMillis() - mRadioRebootTime < SPEED_UP_SWITCH_DURATION) {
            // Switch phone directly to speed up network register time if device first booted or radio on from airplane mode.
            delayedTime = 0;
        } else {
            PowerManager powerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            // Switching after 3 mins when the device screen is in off state for power saving.
            delayedTime = powerManager.isScreenOn() ? RADIO_NO_SERVICE_SCREENON_TIMEOUT : RADIO_NO_SERVICE_SCREENOFF_TIMEOUT;
        }
        logd("calcSwitchPhoneDelayedTime: testDelayTime = " + testDelayTime + ", delayedTime = " + delayedTime);
        return delayedTime;
    }
    
    /**
     * Send no service message to request switch phone after the given duration.
     * 
     * @param delayedTime
     */
    private void sendNoServiceMessage(int delayedTime) {
        mNoServiceQueued = true;
        sendMessageDelayed(obtainMessage(EVENT_RADIO_NO_SERVICE), delayedTime);
    }

    /**
     * Remove no service message.
     */
    private void removeNoServiceMessage() {
        if (mNoServiceQueued) {
            removeMessages(EVENT_RADIO_NO_SERVICE);
            mNoServiceQueued = false;
        }
    }

    /**
     * Send phone switch broadcast, application like settings can receive this
     * broadcast to do things at this time.
     */
    private void notifySwitchPhoneStarted() {
        Intent intent = new Intent(INTENT_ACTION_START_SWITCH_PHONE);
        intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mDualModePhone.getMySimId());
        ActivityManagerNative.broadcastStickyIntent(intent, null, 0);
    }

    /**
     * Start to switch phone, send broadcast to notify applicaiton and start
     * message to GeminiPhone, then power off the GSM modem to begin phone
     * switching.
     */
    private void startToSwitchPhone() {
        logd("startToSwitchPhone, notify application and set GSM radio off.");
        mUnderSimSwitching = true;
        notifySwitchPhoneStarted();

        // Get message to send to GeminiPhone to notify switching phone start
        // and end.
        mStartSwitchPoneMsg = mGeminiPhoneHandler
                .obtainMessage(InternationalRoamingConstants.EVENT_RADIO_IR_SIM_SWITCH_START);
        mFinishSwitchPoneMsg = mGeminiPhoneHandler
                .obtainMessage(InternationalRoamingConstants.EVENT_RADIO_IR_SIM_SWITCH_DONE);

        if (mStartSwitchPoneMsg != null) {
            mGeminiPhoneHandler.sendMessage(mStartSwitchPoneMsg);
        }

        mSimSwithHandler.removeCallbacksAndMessages(null);
        mGsmActivePhone.mCi
                .setRadioPowerOff(mSimSwithHandler.obtainMessage(EVENT_RADIO_OFF_GSM));
    }

    /**
     * Finished switching phone, send message to GeminiPhone and update active
     * phone in both phone proxies. The finish time is advanced before radio
     * on(after phone instance updated) to make sure GeminiPhone re-register SIM
     * inserted status before radio on. So be careful to send AT request in post
     * switch phone because the radio may be not ready yet.
     */
    private void finishToSwitchPhone() {
        logd("finishToSwitchPhone...");
        mUnderSimSwitching = false;
        if (mFinishSwitchPoneMsg != null) {
            mGeminiPhoneHandler.sendMessageAtFrontOfQueue(mFinishSwitchPoneMsg);
        }

        // Update active phone.
        mDualActivePhone = (PhoneBase) (mDualModePhone.getActivePhone());
        mGsmActivePhone = (PhoneBase) (mGsmPhone.getActivePhone());

        mNetworkSelectionStrategy.onPostSwitchPhone();
    }
    
    /**
     * Check SIM inserted status.
     * 
     * @param iccid
     * @return
     */
    private int checkSimInsertedStatus(String iccid) {
        if (iccid == null || (iccid.equals("")) || (iccid.equals(ICCID_STRING_FOR_NO_SIM))) {
            return SIM_INSERTED_STATUS_NONE;
        } else {
            SharedPreferences preference = mContext.getSharedPreferences(
                    PREF_INTERNATIONAL_SIM_INFO, 0);
            String oldIccid = preference.getString(KEY_INTERNATIONAL_SIM_ICCID, null);
            if (iccid.equals(oldIccid)) {
                return SIM_INSERTED_STATUS_NEW;
            } else {
                return SIM_INSERTED_STATUS_OLD;
            }
        }
    }

    /**
     * Update SIM card information if the SIM inserted status change.
     * 
     * @param status
     * @param iccid
     */
    private void updateSimCardInfoIfNeeded(int status, String iccid) {
        if (mSimInsertedStatus == status) {
            logd("updateSimCardInfoIfNeeded with same status: " + status);
            return;
        }

        logd("updateSimCardInfoIfNeeded: status = " + status + ", iccid = " + iccid
                + ", mSimInsertedStatus = " + mSimInsertedStatus);
        mSimInsertedStatus = status;
        SharedPreferences preference = mContext
                .getSharedPreferences(PREF_INTERNATIONAL_SIM_INFO, 0);
        switch (status) {
            case SIM_INSERTED_STATUS_NONE:
                mSimInserted = false;
                mImsiLoaded = true;

                // Unregister ICC changed events.
                mUiccController.unregisterForIccChanged(mCardTypeHandler);
                mCdmaImsi = null;
                mGsmImsi = null;
                mCardType = InternationalRoamingConstants.CARD_TYPE_UNKNOWN;

                logd("updateSimCardInfoIfNeeded with no sim inserted.");
                break;

            case SIM_INSERTED_STATUS_NEW:
                mSimInserted = true;
                mImsiLoaded = false;

                clearSavedNetworkInfo();
                SharedPreferences.Editor editor = preference.edit();
                editor.putString(KEY_INTERNATIONAL_SIM_ICCID, iccid);
                editor.commit();

                mUiccController.registerForIccChanged(mCardTypeHandler, EVENT_ICC_CHANGED, null);
                mGeneralStrategy.onNewSimInserted(mDualSimId);
                break;

            case SIM_INSERTED_STATUS_OLD:
                mSimInserted = true;

                // Unregister ICC changed events.
                mUiccController.unregisterForIccChanged(mCardTypeHandler);
                mCardType = preference.getInt(KEY_INTERNATIONAL_SIM_TYPE,
                        InternationalRoamingConstants.CARD_TYPE_UNKNOWN);
                mCdmaImsi = preference.getString(KEY_INTERNATIONAL_CDMA_IMSI, null);
                mGsmImsi = preference.getString(KEY_INTERNATIONAL_GSM_IMSI, null);
                logd("updateSimCardInfoIfNeeded: mCardType = " + mCardType + ", mCdmaImsi = "
                        + mCdmaImsi + ", mGsmImsi = " + mGsmImsi);
                if ((mCardType == InternationalRoamingConstants.CARD_TYPE_SINGLE_CDMA && mCdmaImsi != null)
                        || (mCardType == InternationalRoamingConstants.CARD_TYPE_DUAL_MODE
                                && mCdmaImsi != null && mGsmImsi != null)) {
                    mImsiLoaded = true;
                    notifySimImsiLoaded(mCdmaImsi, mGsmImsi);
                } else {
                    mImsiLoaded = false;
                    mUiccController.registerForIccChanged(mCardTypeHandler, EVENT_ICC_CHANGED, null);
                }
                break;

            default:
                logd("Should never run into this case.");
                break;
        }
    }

    /**
     * Load dual phone's IMSI completed, parse card type and store it to
     * system settings.
     * 
     * @param dualSimId
     * @param cdmaImsi
     * @param gsmImsi
     */
    private void loadDualPhoneImsiCompleted(int dualSimId, String cdmaImsi, String gsmImsi) {
        if (mImsiLoaded && TextUtils.equals(mCdmaImsi, cdmaImsi)
                && TextUtils.equals(mGsmImsi, gsmImsi)) {
            logd("IMSI has already loaded: mCardType = " + mCardType + ", mCdmaImsi = " + mCdmaImsi
                    + ", mGsmImsi = " + mGsmImsi);
            return;
        }

        mImsiLoaded = true;
        mCardType = mCardStrategy.parseCardType(cdmaImsi, gsmImsi);
        logd("loadDualPhoneImsiCompleted: mCardType = " + mCardType + ", mCdmaImsi = " + mCdmaImsi
                + ", mGsmImsi = " + mGsmImsi + ",mRegisterSuspend = " + mRegisterSuspend);

        // Store international card information to preference for performance.
        SharedPreferences preference = mContext
                .getSharedPreferences(PREF_INTERNATIONAL_SIM_INFO, 0);
        SharedPreferences.Editor editor = preference.edit();
        editor.putInt(KEY_INTERNATIONAL_SIM_TYPE, mCardType);
        editor.putString(KEY_INTERNATIONAL_CDMA_IMSI, cdmaImsi);
        editor.putString(KEY_INTERNATIONAL_GSM_IMSI, gsmImsi);
        editor.commit();

        // TODO: check where the system setting is used, need to remove?
        Settings.System.putInt(mContext.getContentResolver(), PROP_INTERNATIONAL_CARD_TYPE,
                mCardType);

        notifySimImsiLoaded(cdmaImsi, gsmImsi);
    }

    /**
     * Notify SIM IMSI loaded.
     * 
     * @param cdmaImsi
     * @param gsmImsi
     */
    private void notifySimImsiLoaded(String cdmaImsi, String gsmImsi) {
        // Notify card and data strategies IMSI loaded.
        mCardStrategy.onSimImsiLoaded(mDualSimId, cdmaImsi, gsmImsi);
        mDataStrategy.onSimImsiLoaded(cdmaImsi, gsmImsi);
    }

    /**
     * Save the current network register information.
     * 
     * @param operatorNumeric
     */
    private void saveCurrentNetworkInfo(String operatorNumeric) {
        logd("saveCurrentNetworkInfo: operatorNumeric = " + operatorNumeric);
        SharedPreferences preference = mContext.getSharedPreferences(
                PREF_INTERNATIONAL_NETWORK_INFO, 0);
        SharedPreferences.Editor editor = preference.edit();
        editor.putInt(KEY_INTERNATIONAL_LAST_PHONE_TYPE, mDualModePhone.getPhoneType());
        if (!TextUtils.isEmpty(operatorNumeric) && operatorNumeric.length() >= OPERATOR_MCC_LENGTH) {
            editor.putString(KEY_INTERNATIONAL_LAST_NETWORK_MCC,
                    operatorNumeric.substring(0, OPERATOR_MCC_LENGTH));
        }
        editor.commit();
    }

    /**
     * Clear the saved network register information.
     */
    private void clearSavedNetworkInfo() {
        logd("clearSavedNetworkInfo...");
        SharedPreferences preference = mContext.getSharedPreferences(
                PREF_INTERNATIONAL_NETWORK_INFO, 0);
        SharedPreferences.Editor editor = preference.edit();
        editor.putInt(KEY_INTERNATIONAL_LAST_PHONE_TYPE, PhoneConstants.PHONE_TYPE_NONE);
        editor.putString(KEY_INTERNATIONAL_LAST_NETWORK_MCC, "000");
        editor.commit();
    }

    private void logd(String msg) {
        Xlog.d(LOG_TAG_PHONE, TAG_PREFIX + msg);
    }

    /**
     * Handler used to switch phone to another mode step by step.
     */
    private class SimSwithHandler extends Handler {
        private static final String CURRENT_SIM_MODE = "ct.internationalcard.sim.mode";

        // System properties definition.
        private static final String PROPERTY_TELEPHONY_MODE_SLOT1 = "mtk_telephony_mode_slot1";
        private static final String PROPERTY_TELEPHONY_MODE_SLOT2 = "mtk_telephony_mode_slot2";

        private static final int EVENT_SWITCH_TELEPHONY_MODE = 101;
        private static final int EVENT_SWITCH_RIL_SOCKET = 102;
        private static final int EVENT_SWITCH_PHONE_INSTANCE = 103;

        // Telephony mode.
        private static final int SIM_SWITCH_MODE_CDMA = 0;
        private static final int SIM_SWITCH_MODE_GSM = 1;
        private static final int SIM_SWITCH_MODE_WCDMA = 2;

        private boolean mChangeToGemini;

        public SimSwithHandler(Looper looper) {
            super(looper);
        }

        /*
         * Switching SIM card interface and Phone instance should be step by step. 
         * CDMA->GSM: 
         * 1. Power off GSM and CDMA modem 
         * 2. Set telephony mode, CCCI store the system property and switch SIM interface 
         * 3. Switch socket, the socket name is decided by the system property
         * 4. Update active phone instance in PhoneProxy
         * 5. Power on GSM modem 
         * 
         * GSM-> CDMA: 
         * 1. Power off GSM modem 
         * 2. Set telephony mode, CCCI store the system property and switch SIM interface 
         * 3. Switch socket, the socket name is decided by the system property 
         * 4. Update active phone instance in PhoneProxy
         * 5. Power on GSM and CDMA modem
         */
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RADIO_OFF_GSM:
                    if (mDualActivePhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                        logd("[SSH] CDMA -> GSM, Set CDMA radio off.");
                        mChangeToGemini = true;
                        mDualActivePhone.mCi.setRadioPowerCardSwitch(0,
                                obtainMessage(EVENT_SWITCH_TELEPHONY_MODE));
                    } else {
                        logd("[SSH] GSM -> CDMA, EVENT_SWITCH_TELEPHONY_MODE.");
                        mChangeToGemini = false;
                        obtainMessage(EVENT_SWITCH_TELEPHONY_MODE, null).sendToTarget();
                    }
                    break;

                case EVENT_SWITCH_TELEPHONY_MODE:
                    if (mChangeToGemini) {
                        logd("[SSH] EVENT_SWITCH_SIM_INTERFACE: CDMA -> GSM.");
                        setTelephonyMode(mDualSimId, SIM_SWITCH_MODE_WCDMA,
                                obtainMessage(EVENT_SWITCH_RIL_SOCKET));
                    } else {
                        logd("[SSH] EVENT_SWITCH_SIM_INTERFACE: GSM -> CDMA.");
                        setTelephonyMode(mDualSimId, SIM_SWITCH_MODE_CDMA,
                                obtainMessage(EVENT_SWITCH_RIL_SOCKET));
                    }
                    break;

                case EVENT_SWITCH_RIL_SOCKET:
                    // Register for radio state change, radio will change to
                    // available if socket switch and connected successfully.
                    mDualActivePhone.mCi.registerForRadioStateChanged(this,
                            EVENT_SWITCH_PHONE_INSTANCE, null);
                    if (mChangeToGemini) {
                        logd("[SSH] EVENT_SWITCH_RIL_SOCKET: CDMA -> GSM.");
                        mDualActivePhone.mCi.switchRilSocket(Phone.NT_MODE_WCDMA_PREF, mDualSimId);
                    } else {
                        logd("[SSH] EVENT_SWITCH_RIL_SOCKET: GSM -> CDMA.");
                        mDualActivePhone.mCi.switchRilSocket(Phone.NT_MODE_CDMA, mDualSimId);
                    }
                    break;

                case EVENT_SWITCH_PHONE_INSTANCE:
                    if (!mDualActivePhone.mCi.getSimState().isAvailable()) {
                        logd("[SSH] Dual phone SIM is not available when handing EVENT_SWITCH_PHONE_INSTANCE.");
                        break;
                    }

                    removeMessages(EVENT_SWITCH_PHONE_INSTANCE);
                    mDualActivePhone.mCi.unregisterForRadioStateChanged(this);
                    // We need to unregister service status change before delete
                    // and create new phone.
                    mDualModePhone
                            .unregisterForServiceStateChanged(InternationalRoamingController.this);

                    final int newTech = mChangeToGemini ? ServiceState.RIL_RADIO_TECHNOLOGY_GSM
                            : ServiceState.RIL_RADIO_TECHNOLOGY_1xRTT;
                    mDualModePhone.updatePhoneObjectForSwitchPhone(newTech);

                    logd("[SSH] EVENT_SWITCH_PHONE: newTech = "
                            + ServiceState.rilRadioTechnologyToString(newTech));

                    // Set modem reset mode.
                    SystemProperties.set("mux.report.case", String.valueOf(1));
                    finishToSwitchPhone();

                    // Power on GSM and CDMA if needed, set radio on reason.
                    mRadioOnFromSimSwitch = true;
                    mGsmActivePhone.mCi.setRadioPowerOn(null);
                    if (!mChangeToGemini) {
                        mDualActivePhone.mCi.setRadioPowerCardSwitch(1, null);
                    }
                    break;

                default:
                    logd("[SSH] Should never run into this case: message = " + msg);
                    break;
            }
        }

        /**
         * Set current telephony mode, it calls CCCI to set the system
         * properties represent the mode and switch SIM interface to the
         * appropriate position, either external or internal modem.
         * 
         * @param slotId the slot ID which need to set mode.
         * @param mode the requested mode.
         * @param response
         */
        private void setTelephonyMode(int slotId, int mode, Message response) {
            logd("[SSH] setTelephonyMode: new mode is " + modeToString(mode));
            int slot1Mode = SystemProperties.getInt(PROPERTY_TELEPHONY_MODE_SLOT1, 1);
            int slot2Mode = SystemProperties.getInt(PROPERTY_TELEPHONY_MODE_SLOT2, 1);
            if (slotId == PhoneConstants.GEMINI_SIM_1) {
                slot1Mode = mode;
            } else {
                slot2Mode = mode;
            }

            // Only MTK internal CCCI can handle setTelephonyMode request, so we
            // need to use GsmPhone to send this request.
            mGsmActivePhone.mCi.setTelephonyMode(slot1Mode, slot2Mode, true, response);
        }

        private String modeToString(int mode) {
            switch (mode) {
                case SIM_SWITCH_MODE_CDMA:
                    return "CDMA_MODE";
                case SIM_SWITCH_MODE_GSM:
                    return "GSM_MODE";
                case SIM_SWITCH_MODE_WCDMA:
                    return "WCDMA_MODE";
                default:
                    return "<Unknown mode>";
            }
        }
    }

    /**
     * Handler used to get IMSIs and get SIM card type of dual phone.
     */
    private class CardTypeHandler extends Handler {
        private static final int EVENT_DUAL_IMSI_READY = 201;

        public CardTypeHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            logd("[CTH] handleMessage: msg.what=" + msg.what);

            switch (msg.what) {
                case EVENT_ICC_CHANGED:
                    mUiccCard = mUiccController.getUiccCard();
                    logd("[CTH] EVENT_ICC_CHANGED: mUiccCard = " + mUiccCard);
                    if (mUiccCard != null) {
                        // Remove extra EVENT_ICC_CHANGED messages.
                        removeMessages(EVENT_ICC_CHANGED);
                        mUiccController.unregisterForIccChanged(this);
                        mUiccCard.registerForDualImsiReady(this, EVENT_DUAL_IMSI_READY, null);
                    }
                    break;

                case EVENT_DUAL_IMSI_READY:
                    mUiccCard.unregisterForDualImsiReady(this);
                    mCdmaImsi = mUiccCard.getCdmaImsi();
                    mGsmImsi = mUiccCard.getGsmImsi();
                    logd("[CTH] EVENT_DUAL_IMSI_READY: mCdmaImsi = " + mCdmaImsi + ", mGsmImsi = "
                            + mGsmImsi + ", mUiccCard = " + mUiccCard);
                    loadDualPhoneImsiCompleted(mDualSimId, mCdmaImsi, mGsmImsi);
                    break;

                default:
                    logd("[CTH] Should never run into this case: msg = " + msg);
                    break;
            }
        }
    }

    private final BroadcastReceiver mAirplaneModeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                final boolean enabled = intent.getBooleanExtra("state", false);
                logd("Airplane mode changed to " + enabled);
                mRadioOnFromAirplaneMode = !enabled;
            }
        }
    };
}
