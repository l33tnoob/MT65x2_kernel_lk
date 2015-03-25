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

import com.mediatek.common.telephony.ISimInfoUpdate;

import static android.Manifest.permission.READ_PHONE_STATE;
import android.app.ActivityManagerNative;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.util.Log;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.mediatek.common.featureoption.FeatureOption;

import java.util.List;
//
import android.os.AsyncResult;
import android.os.Message;
import android.content.BroadcastReceiver;
import com.android.internal.telephony.IccCardConstants;
import android.content.IntentFilter;
import com.mediatek.telephony.DefaultSimSettings;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.android.internal.telephony.gemini.GeminiPhone;

/**
 *@hide
 */
public class SimInfoUpdate {
    private static final String LOG_TAG = "PHONE";
    private static int[] mInsertSimState;

    private static Context mContext = null;
    // [ALPS00396046][mtk02772] start
    // if simme locked, wait ACTION_SHOW_NEW_SIM_DETECTED broadcast by sim me lock service
    // if sim is not simme lock, broadcast new sim inserted dialog directly after receive sim ready/locked
    private static boolean mShowNewSimDetectedPending;
    private static boolean[] mAllowShowNewSim;    
    private static int mSimCount;
    private static int mNewSimStatus;
    private static String mDetectType;
    
    /* SIM inserted status constants */
    private static final int STATUS_NO_SIM_INSERTED = 0x00;
    private static final int STATUS_SIM1_INSERTED = 0x01;    
    private static final int STATUS_SIM2_INSERTED = 0x02;
    private static final int STATUS_SIM3_INSERTED = 0x04;
    private static final int STATUS_SIM4_INSERTED = 0x08; 

    private static final int STATUS_DUAL_SIM_INSERTED = STATUS_SIM1_INSERTED | STATUS_SIM2_INSERTED;
    
    static final int SIM_NOT_CHANGE = 0;
    static final int SIM_CHANGED = -1;
    static final int SIM_NEW = -2;
    static final int SIM_REPOSITION = -3;
    static final int SIM_NOT_INSERT = -99;

    public static final String INTENT_KEY_DETECT_STATUS = "simDetectStatus";
    public static final String INTENT_KEY_SIM_COUNT = "simCount";
    public static final String INTENT_KEY_NEW_SIM_SLOT = "newSIMSlot";
    public static final String INTENT_KEY_NEW_SIM_STATUS = "newSIMStatus";
    public static final String EXTRA_VALUE_NEW_SIM = "NEW";
    public static final String EXTRA_VALUE_REMOVE_SIM = "REMOVE";
    public static final String EXTRA_VALUE_REPOSITION_SIM = "SWAP";

    static private String[] DEFAULTSIMSETTING_PROPERTY_ICC_OPERATOR_DEFAULT_NAME = {
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME_2,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME_3,
        TelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME_4
    };

    public SimInfoUpdate(boolean constructor) {
        logd("Empty constructor called");
        constructor = false;
    }

    public SimInfoUpdate(Context context) {
        logd("SimInfoUpdate constructor called");
        mContext = context;
        mSimCount = 0;
        mNewSimStatus = 0;
        mDetectType = "";
        /*
         *  int[] mInsertSimState maintains all slots' SIM inserted status currently, 
         *  it may contain 4 kinds of values:
         *    SIM_NOT_INSERT : no SIM inserted in slot i now
         *    SIM_CHANGED    : a valid SIM insert in slot i and is different SIM from last time
         *                     it will later become SIM_NEW or SIM_REPOSITION during update procedure
         *    SIM_NOT_CHANGE : a valid SIM insert in slot i and is the same SIM as last time
         *    SIM_NEW        : a valid SIM insert in slot i and is a new SIM
         *    SIM_REPOSITION    : a valid SIM insert in slot i and is inserted in different slot last time
         *    positive integer #: index to distinguish SIM cards with the same IccId
         */
        mInsertSimState = new int[PhoneConstants.GEMINI_SIM_NUM];
        mAllowShowNewSim = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mAllowShowNewSim[i] = false;
        }
        mShowNewSimDetectedPending = false;
        // [ALPS00396046][mtk02772] start
        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_SHOW_NEW_SIM_DETECTED);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction("action_pin_dismiss");
        intentFilter.addAction(GeminiPhone.EVENT_INITIALIZATION_FRAMEWORK_DONE);
        mContext.registerReceiver(mReceiver, intentFilter);
        // [ALPS00396046][mtk02772] end
    }

    private static void showSimDialog() {
        logd("[showSimDialog] mShowNewSimDetectedPending = " + mShowNewSimDetectedPending);
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            logd("[showSimDialog] mAllowShowNewSim[" + i + "]=" + mAllowShowNewSim[i]);
        }
        if (mShowNewSimDetectedPending && isAllowedToShowSimDialog()) {
            // After ME/PIN/PUK/PIN dismiss, no need to listen those intents after first sim dialog pop out
            // SIM plug-in/out is handled by updateSimInfoByIccId directly
            // Register many kinds of intent is used to fit about power-on sequence,
            // after first sim dialog, only needs to care about power off intent,
            // it will be informed if sim is plug-out/plug-in
            mContext.unregisterReceiver(mReceiver);
            IntentFilter intentFilter = new IntentFilter(GeminiPhone.EVENT_INITIALIZATION_FRAMEWORK_DONE);     
            mContext.registerReceiver(mReceiver, intentFilter);
            logd("[showSimDialog] mDetectType: " + mDetectType);
            if (EXTRA_VALUE_NEW_SIM.equals(mDetectType)) {
                broadcastNewSimDetected();
            } else if (EXTRA_VALUE_REPOSITION_SIM.equals(mDetectType)) {
                broadcastSimRepositioned();
            } else if (EXTRA_VALUE_REMOVE_SIM.equals(mDetectType)) {
                broadcastSimRemoved();
            }
            mShowNewSimDetectedPending = false;
        }
    }

    // [ALPS00396046][mtk02772] start
    private static final BroadcastReceiver mReceiver = new  BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            logd("[BroadcastReceiver][onReceiver]+");
            String action = intent.getAction();
            int slotId;
            logd("[BroadcastReceiver][onReceiver][action] : " + action );
            if (action.equals(TelephonyIntents.ACTION_SHOW_NEW_SIM_DETECTED)) {
                slotId = intent.getIntExtra("simslot", -1);
                logd("[BroadcastReceiver][onReceiver] slotId: " + slotId);
                if (slotId >= 0) {
                    mAllowShowNewSim[slotId] = true;
                }
                showSimDialog();
            } else if (action.equals("action_pin_dismiss")) {
                slotId = intent.getIntExtra("simslot", -1);
                logd("[BroadcastReceiver][onReceiver] slotId: " + slotId);
                if (slotId >= 0) {
                    mAllowShowNewSim[slotId] = true;
                }
                showSimDialog();
            } else if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                logd("[BroadcastReceiver][onReceiver] slotId: " + slotId + " simStatus: " + simStatus);
                if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simStatus)) {
                    mAllowShowNewSim[slotId] = true;
                }
                showSimDialog();
            } else if (GeminiPhone.EVENT_INITIALIZATION_FRAMEWORK_DONE.equals(action)) {
                int airplaneMode = Settings.System.getInt(
                    mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0);
                int dualSimMode = Settings.System.getInt(
                        mContext.getContentResolver(),
                        Settings.System.DUAL_SIM_MODE_SETTING,
                        Settings.System.DUAL_SIM_MODE_SETTING_DEFAULT);
                logd("[BroadcastReceiver][onReceiver] EVENT_INITIALIZATION_FRAMEWORK_DONE airplaneMode:"
                            + airplaneMode + " dualSimMode:" + dualSimMode);
                if (airplaneMode == 1) {
                    if (mAllowShowNewSim == null) {
                        logd("mAllowShowNewSim not allocated");
                    } else {
                        logd("Flight mode on, all mAllowShowNewSim[i] set to true");
                        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                            mAllowShowNewSim[i] = true;
                        }
                    }
                    showSimDialog();
                } else {
                    if (mAllowShowNewSim == null) {
                        logd("mAllowShowNewSim not allocated");
                    } else {
                        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                            if ((dualSimMode & (1 << i)) == 0) {
                                logd("Slot" + i + " radio off");
                                mAllowShowNewSim[i] = true;
                                showSimDialog();
                            }
                        }
                    }
                }
            }
            logd("[BroadcastReceiver][onReceiver]-");
        }
    };
    // [ALPS00396046][mtk02772] end
    
    public static void updateSimInfoByIccId(Context context, String iccId1, String iccId2,
            String iccId3, String iccId4, boolean is3GSwitched) {
        logd("[updateSimInfoByIccId] Single Card Project");
        mContext = context;
        mSimCount = 0;
        mNewSimStatus = 0;
        mDetectType = "";
        mInsertSimState = new int[PhoneConstants.GEMINI_SIM_NUM];
        mAllowShowNewSim = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        mAllowShowNewSim[0] = false;
        mShowNewSimDetectedPending = false;
        updateSimInfoByIccId(iccId1, iccId2, iccId3, iccId4, is3GSwitched);
    }
    
    synchronized public static void updateSimInfoByIccId(String iccId1, String iccId2, String iccId3,
            String iccId4, boolean is3GSwitched) {
        iccId1 = iccId1 == null ? "" : iccId1;
        iccId2 = iccId2 == null ? "" : iccId2;
        iccId3 = iccId3 == null ? "" : iccId3;
        iccId4 = iccId4 == null ? "" : iccId4;
        String[] iccId = {iccId1, iccId2, iccId3, iccId4};
        logd("[updateSimInfoByIccId] Start");
        if (mContext == null) {
            logd("[updateSimInfoByIccId] SimInfoUpdate contructor not called!!!");
            return;
        }
        boolean[] isSimInserted = new boolean[PhoneConstants.GEMINI_SIM_NUM];
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            iccId[i] = (iccId[i] == null) ? "" : iccId[i];
            isSimInserted[i] = !("".equals(iccId[i]));
        }
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mInsertSimState[i] = SIM_NOT_CHANGE;
        }
        // not insert and identical IccId states are indicated
        int index = 0;
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (!isSimInserted[i]) {
                mInsertSimState[i] = SIM_NOT_INSERT;
                continue;
            }
            index = 2;
            for (int j=i+1; j<PhoneConstants.GEMINI_SIM_NUM; j++) {
                if (mInsertSimState[j] == SIM_NOT_CHANGE && iccId[i].equals(iccId[j])) {
                    // SIM i adn SIM j has equal IccId, they are invalid SIMs
                    mInsertSimState[i] = 1;
                    mInsertSimState[j] = index;
                    index++;
                }
            }
        }
        ContentResolver contentResolver = mContext.getContentResolver();
        String[] oldIccIdInSlot = new String[PhoneConstants.GEMINI_SIM_NUM];
        for(int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            SimInfoRecord oldSimInfo = SimInfoManager.getSimInfoBySlot(mContext, i);
            if (oldSimInfo != null) {
                oldIccIdInSlot[i] = oldSimInfo.mIccId;
                logd("[updateSimInfoByIccId] Old IccId in slot" + i + " is " + oldIccIdInSlot[i] + " oldSimId:" + oldSimInfo.mSimInfoId); 
                // slot_i has different SIM states is indicated
                if (mInsertSimState[i] == SIM_NOT_CHANGE && !iccId[i].equals(oldIccIdInSlot[i])) {
                    mInsertSimState[i] = SIM_CHANGED;
                }
                if(mInsertSimState[i] != SIM_NOT_CHANGE) {
                    ContentValues value = new ContentValues(1);
                    value.put(SimInfoManager.SLOT, -1);
                    contentResolver.update(ContentUris.withAppendedId(SimInfoManager.CONTENT_URI,
                            oldSimInfo.mSimInfoId), value, null, null);
                    logd("[updateSimInfoByIccId] Reset slot" + i + " to -1, iccId[" + i + "]= " + iccId[i]); 
                }
            } else {
                if (mInsertSimState[i] == SIM_NOT_CHANGE) {
                    // no SIM inserted last time, but there is one SIM inserted now
                    mInsertSimState[i] = SIM_CHANGED;
                }
                oldIccIdInSlot[i] = "";
                logd("[updateSimInfoByIccId] No SIM in slot " + i + " for last time"); 
            }
        }

        //check if the Inserted SIM is new
        int nNewCardCount = 0;
        int nNewSimStatus = 0;
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (mInsertSimState[i] == SIM_NOT_INSERT) {
                logd("[updateSimInfoByIccId] No SIM inserted in slot " + i + " this time");
            } else {
                logd("[updateSimInfoByIccId] iccId[" + i + "] : " + iccId[i] + ", oldIccIdInSlot[" + i + "] : " + oldIccIdInSlot[i]);
                if (mInsertSimState[i] > 0) {
                    //some special SIM cards have no invalid ICCID. so we add a suffix to prove that we can show two SIM cards 
                    //even if two SIMs of that kind are inserted at the same time.  So this kind of SIM will be always treated as a new SIM.
                    SimInfoManager.addSimInfoRecord(mContext, iccId[i] + Integer.toString(mInsertSimState[i]), i); 
                    logd("[updateSimInfoByIccId] Special SIM with invalid IccId is inserted in slot" + i );
                } else if (mInsertSimState[i] == SIM_CHANGED) {
                    SimInfoManager.addSimInfoRecord(mContext, iccId[i], i); 
                }
                if (isNewInsertedSim(iccId[i], oldIccIdInSlot, PhoneConstants.GEMINI_SIM_NUM)) {
                    //one new card inserted into slot1
                    nNewCardCount++;
                    switch (i) {
                        case PhoneConstants.GEMINI_SIM_1:
                            nNewSimStatus |= STATUS_SIM1_INSERTED;
                            break;
                        case PhoneConstants.GEMINI_SIM_2:
                            nNewSimStatus |= STATUS_SIM2_INSERTED;
                            break;
                        case PhoneConstants.GEMINI_SIM_3:
                            nNewSimStatus |= STATUS_SIM3_INSERTED;
                            break;
                        case PhoneConstants.GEMINI_SIM_4:
                            nNewSimStatus |= STATUS_SIM4_INSERTED;
                            break;
                    }
                    // new SIM card now assign to be SIM_NEW state
                    mInsertSimState[i] = SIM_NEW;
                }
            }
        }
        
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (mInsertSimState[i] == SIM_CHANGED) {
                mInsertSimState[i] = SIM_REPOSITION;
            }
        }

        long[] simIdInSlot = {-3, -3, -3, -3};
        List<SimInfoRecord> simInfos = SimInfoManager.getInsertedSimInfoList(mContext);
        int nSimCount = (simInfos == null) ? 0 : simInfos.size();
        logd("[updateSimInfoByIccId] nSimCount = " + nSimCount);
        for (int i=0; i<nSimCount; i++) {
            SimInfoRecord temp = simInfos.get(i);
            simIdInSlot[temp.mSimSlotId] = temp.mSimInfoId;
            logd("[updateSimInfoByIccId] simIdInSlot[" + temp.mSimSlotId + "] = " + temp.mSimInfoId);
        }

        if (nNewCardCount > 0) {   
            logd("[updateSimInfoByIccId] New SIM detected"); 
            setColorForNewSim(simInfos);
            int airplaneMode = Settings.Global.getInt(contentResolver, Settings.Global.AIRPLANE_MODE_ON, 0);
            if (airplaneMode > 0) {
                setDefaultNameForAllNewSim(simInfos);
            } else {
                setDefaultNameIfImsiReadyOrLocked(simInfos);
            }
        }
        // true if any slot has no SIM this time, but has SIM last time
        boolean hasSimRemoved = false;
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (!isSimInserted[i] && !oldIccIdInSlot[i].equals("")) {
                hasSimRemoved = true;
                break;
            }
        }
        if(PhoneConstants.GEMINI_SIM_NUM == 1) {
            logd("[updateSimInfoByIccId] oldIccId: " + oldIccIdInSlot[0] + "; newIccId: " + iccId[0] 
                    + "; newCardCount: " + nNewCardCount + "; hasSimRemoved: " + hasSimRemoved + "; nSimCount: " + nSimCount);
        } else if (PhoneConstants.GEMINI_SIM_NUM == 2) {
            logd("[updateSimInfoByIccId] oldIccId: " + oldIccIdInSlot[0] + ", " + oldIccIdInSlot[1] 
                    + "; newIccId: " + iccId[0] + ", " + iccId[1] 
                    + "; newCardCount: " + nNewCardCount + "; hasSimRemoved: " + hasSimRemoved + "; nSimCount: " + nSimCount);
        } else if (PhoneConstants.GEMINI_SIM_NUM == 3) {
            logd("[updateSimInfoByIccId] oldIccId: " + oldIccIdInSlot[0] + ", " + oldIccIdInSlot[1] + ", " + oldIccIdInSlot[2]
                    + "; newIccId: "+ iccId[0] + ", " + iccId[1] + ", " + iccId[2] 
                    + "; newCardCount: " + nNewCardCount + "; hasSimRemoved: " + hasSimRemoved + "; nSimCount: " + nSimCount);
        } else if (PhoneConstants.GEMINI_SIM_NUM == 4) {
            logd("[updateSimInfoByIccId] oldIccId: " + oldIccIdInSlot[0] + ", " + oldIccIdInSlot[1] + ", " + oldIccIdInSlot[2] + ", " + oldIccIdInSlot[3]
                    + "; newIccId: "+ iccId[0] + ", " + iccId[1] + ", " + iccId[2] + ", " + iccId[3] 
                    + "; newCardCount: " + nNewCardCount + "; hasSimRemoved: " + hasSimRemoved + "; nSimCount: " + nSimCount);
        }
        // set all default SIMs
        DefaultSimSettings.setAllDefaultSim(mContext, contentResolver, simInfos, simIdInSlot, isSimInserted, nNewCardCount,
                nSimCount, is3GSwitched, mInsertSimState, hasSimRemoved);
        if (!FeatureOption.MTK_BSP_PACKAGE) {
            if (nNewCardCount == 0) {
                int i;
                if (hasSimRemoved) {
                    // no new SIM, at least one SIM is removed, check if any SIM is repositioned first
                    for (i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                        if (mInsertSimState[i] == SIM_REPOSITION) {
                            logd("[updateSimInfoByIccId] No new SIM detected and SIM repositioned");
                            setUpdatedData(EXTRA_VALUE_REPOSITION_SIM, nSimCount, nNewSimStatus);
                            break;
                        }
                    }
                    if (i == PhoneConstants.GEMINI_SIM_NUM) {
                        // no new SIM, no SIM is repositioned => at least one SIM is removed
                        logd("[updateSimInfoByIccId] No new SIM detected and SIM removed");
                        setUpdatedData(EXTRA_VALUE_REMOVE_SIM, nSimCount, nNewSimStatus);
                    }
                } else {
                    // no SIM is removed, no new SIM, just check if any SIM is repositioned
                    for (i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                        if (mInsertSimState[i] == SIM_REPOSITION) {
                            logd("[updateSimInfoByIccId] No new SIM detected and SIM repositioned");
                            setUpdatedData(EXTRA_VALUE_REPOSITION_SIM, nSimCount, nNewSimStatus);
                            break;
                        }
                    }
                    if (i == PhoneConstants.GEMINI_SIM_NUM) {
                        // all status remain unchanged
                        logd("[updateSimInfoByIccId] All SIM inserted into the same slot");
                    }
                }
            } else {
                logd("[updateSimInfoByIccId] New SIM detected");
                setUpdatedData(EXTRA_VALUE_NEW_SIM, nSimCount, nNewSimStatus);
            }
        }

        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            logd("[updateSimInfoByIccId] mInsertSimState[" + i + "] = " + mInsertSimState[i]);
        }
        
        SystemProperties.set(TelephonyProperties.PROPERTY_SIM_INFO_READY, "true");
        logd("[updateSimInfoByIccId] updateSimInfoByIccId PROPERTY_SIM_INFO_READY after set is " 
                + SystemProperties.get(TelephonyProperties.PROPERTY_SIM_INFO_READY, null));
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        logd("[updateSimInfoByIccId] broadcast intent ACTION_SIM_INFO_UPDATE");
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
    }

    private static boolean isNewInsertedSim(String iccId, String[] oldIccId, int simNum) {
        boolean isNewSim = true;
        for(int i=0; i<simNum; i++) {
            if(iccId.equals(oldIccId[i])) {
                isNewSim = false;
                break;
            }
        }
        logd("[isNewInsertedSim]:" + isNewSim);
        return isNewSim;
    }

    private static void setUpdatedData(String detectedType, int simCount, int newSimStatus) {
        mDetectType = detectedType;
        mSimCount = simCount;
        mNewSimStatus = newSimStatus;
        logd("[setUpdatedData] mShowNewSimDetectedPending=" + mShowNewSimDetectedPending);
        if (isAllowedToShowSimDialog()) {
            if (mDetectType.equals(EXTRA_VALUE_NEW_SIM)) {
                broadcastNewSimDetected();
            } else if (mDetectType.equals(EXTRA_VALUE_REPOSITION_SIM)) {
                broadcastSimRepositioned();
            } else if (mDetectType.equals(EXTRA_VALUE_REMOVE_SIM)) {
                broadcastSimRemoved();
            }
        } else {
            logd("[setUpdatedData] Update complete, wait for AllowShowNewSim[], mShowNewSimDetectedPending=true");
            mShowNewSimDetectedPending = true;    
        }
    }

    public static void broadcastNewSimDetected() {
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_DETECTED);
        intent.putExtra(INTENT_KEY_DETECT_STATUS, EXTRA_VALUE_NEW_SIM);
        intent.putExtra(INTENT_KEY_SIM_COUNT, mSimCount);
        intent.putExtra(INTENT_KEY_NEW_SIM_SLOT, mNewSimStatus);
        logd("broadcast intent ACTION_SIM_DETECTED [" + EXTRA_VALUE_NEW_SIM + ", " +  mSimCount + ", " + mNewSimStatus + "]");
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
    }

    public static void broadcastSimRemoved() {
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_DETECTED);
        intent.putExtra(INTENT_KEY_DETECT_STATUS, EXTRA_VALUE_REMOVE_SIM);
        intent.putExtra(INTENT_KEY_SIM_COUNT, mSimCount);
        logd("broadcast intent ACTION_SIM_DETECTED [" + EXTRA_VALUE_REMOVE_SIM + ", " +  mSimCount + "]");
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
    }
 
    public static void broadcastSimRepositioned() {
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_DETECTED);
        intent.putExtra(INTENT_KEY_DETECT_STATUS, EXTRA_VALUE_REPOSITION_SIM);
        intent.putExtra(INTENT_KEY_SIM_COUNT, mSimCount);
        logd("broadcast intent ACTION_SIM_DETECTED [" + EXTRA_VALUE_REPOSITION_SIM + ", " +  mSimCount + "]");
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
    }

    private static boolean isAllowedToShowSimDialog() {
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            if (!mAllowShowNewSim[i]) {
                logd("[isAllowedToShowSimDialog] slot" + i + " is not allowed");
                return false;
            }
        }
        logd("[isAllowedToShowSimDialog] all slots allowed");
        return true;
    }

    // called when flight mode boot up
    private static void setDefaultNameForAllNewSim(List<SimInfoRecord> simInfos) {
        int nSimCount = (simInfos == null ? 0 : simInfos.size());
        logd("[setDefaultNameForAllNewSim] nSimCount is " + nSimCount);
        for (int i=0; i<nSimCount; i++) {
            SimInfoRecord temp = simInfos.get(i);
            if (temp.mDisplayName == null) {
                logd("[setDefaultNameForAllNewSim] set default name for slot" + temp.mSimSlotId);
                SimInfoManager.setDefaultName(mContext, temp.mSimInfoId, null);
            }       
        }       
    }

    private static void setDefaultNameIfImsiReadyOrLocked(List<SimInfoRecord> simInfos) {
        int nSimCount = (simInfos == null ? 0 : simInfos.size());
		long nameSource = SimInfoManager.SIM_SOURCE;
        logd("[setDefaultNameIfImsiReadyOrLocked] nSimCount is " + nSimCount);
        String operatorName = null;
        for (int i=0; i<nSimCount; i++) {
            SimInfoRecord temp = simInfos.get(i);
            if (temp.mDisplayName == null) {
                logd("[setDefaultNameIfImsiReadyOrLocked] the " + i + "th mDisplayName is null");
                operatorName = SystemProperties.get(DEFAULTSIMSETTING_PROPERTY_ICC_OPERATOR_DEFAULT_NAME[temp.mSimSlotId]);
                logd("[setDefaultNameIfImsiReadyOrLocked] operatorName is " + operatorName);
                if (operatorName != null && !operatorName.equals("")) {                          
                    SimInfoManager.setDefaultNameEx(mContext, temp.mSimInfoId, operatorName, nameSource);
                }
            }       
        }       
    }

    // get from SIMRecords, last argument means slot(APP use mSimId to represent slot)
    public static void setDefaultNameForNewSim(Context context, String strName, int slot) {
        long nameSource = SimInfoManager.SIM_SOURCE;
        // the source is from default name if strName is null
        if (strName == null) {
            nameSource = SimInfoManager.DEFAULT_SOURCE;
        }
        SimInfoRecord simInfo = SimInfoManager.getSimInfoBySlot(context, slot);
        if (simInfo != null) {
            // ALPS00384376
            // overwrite sim display name if the name stored in db is not input by user
            long oriNameSource = simInfo.mNameSource;
            String simDisplayName = simInfo.mDisplayName;
            logd("[setDefaultNameForNewSim] SimInfo simId is " + simInfo.mSimInfoId + " simDisplayName is " + simDisplayName 
                    + " newName is " + strName + " oriNameSource = " + oriNameSource + "NewNameSource = " + nameSource);
            if (simDisplayName == null || 
                (oriNameSource == SimInfoManager.DEFAULT_SOURCE && strName != null) ||
                (oriNameSource == SimInfoManager.SIM_SOURCE && strName != null && !strName.equals(simDisplayName))) {
                SimInfoManager.setDefaultNameEx(context, simInfo.mSimInfoId, strName, nameSource);
                broadcastSetDefaultNameDone(slot);
            }
        }
    }

    // get from SIMRecords
    public static void broadcastSetDefaultNameDone(int slot) {
        Intent intent = new Intent("android.intent.action.SIM_NAME_UPDATE");
        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, slot);        
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
        logd("broadcast intent ACTION_SIM_NAME_UPDATE for sim " + slot);
    }

    private static void setColorForNewSim(List<SimInfoRecord> simInfos) {
        int nSimInsert = (simInfos == null ? 0 : simInfos.size());
        boolean isNeedChangeColor = false;
        int pivotSimColor = -1;
        int totalColorNum = PhoneConstants.TOTAL_SIM_COLOR_COUNT;
        logd("[setColorForNewSim] SIM num = " + nSimInsert); 
        for (int i=0; i<nSimInsert; i++) {
            SimInfoRecord pivotSimInfo = simInfos.get(i);
            if (pivotSimInfo != null) {
                do {
                    isNeedChangeColor = false;
                    logd("[setColorForNewSim] i = " + i + " slot" + pivotSimInfo.mSimSlotId 
                            + " simId " + pivotSimInfo.mSimInfoId + " needChange:" + isNeedChangeColor); 
                    // set valid SIM color to pivot SIM, temporally set to blue if not valid
                    if (!(0 <= pivotSimInfo.mColor 
                                && pivotSimInfo.mColor < totalColorNum)) {
                        pivotSimColor = (int)(pivotSimInfo.mSimInfoId-1) % totalColorNum;
                    } else {
                        pivotSimColor = pivotSimInfo.mColor;
                    }
                    // make sure the color will be different with others for consistent UI
                    for(int j=0; j<i; j++) {
                        SimInfoRecord tmpSimInfo = simInfos.get(j);
                        if(tmpSimInfo != null && 0 <= tmpSimInfo.mColor 
                                && tmpSimInfo.mColor < totalColorNum && pivotSimColor == tmpSimInfo.mColor) {
                            pivotSimColor = (pivotSimColor+1) % totalColorNum;
                            pivotSimInfo.mColor = pivotSimColor;
                            isNeedChangeColor = true;
                            logd("[setColorForNewSim] conflict slot" + tmpSimInfo.mSimSlotId + " change slot" 
                                    + pivotSimInfo.mSimSlotId + " to color " + pivotSimColor); 
                            break;
                        }
                    }
                } while (isNeedChangeColor == true);
                ContentValues valueColor = new ContentValues(1);
                valueColor.put(SimInfoManager.COLOR, pivotSimColor);
                mContext.getContentResolver().update(ContentUris.withAppendedId(SimInfoManager.CONTENT_URI, pivotSimInfo.mSimInfoId), 
                        valueColor, null, null);   
                logd("[setColorForNewSim] set slot" + pivotSimInfo.mSimSlotId + " SimInfoId:" + pivotSimInfo.mSimInfoId + " color=" + pivotSimColor); 
            }
        }
    }

    public static void broadcastSimInsertedStatus(int nSimInsertStatus) {
        if (mAllowShowNewSim != null) {
            int simSlot = 1;
            for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                if ((nSimInsertStatus&simSlot) == 0) {
                    mAllowShowNewSim[i] = true;
                    logd("[broadcastSimInsertedStatus] slot" + i + " not inserted, mAllowShowNewSim[" + i + "] = " + mAllowShowNewSim[i]);
                    //showSimDialog();
                }
                simSlot = simSlot << 1;
            }
        } else {
            logd("[broadcastSimInsertedStatus] mAllowShowNewSim not allocated");
        }
        Intent intent = new Intent(TelephonyIntents.ACTION_SIM_INSERTED_STATUS);
        intent.putExtra(INTENT_KEY_SIM_COUNT, nSimInsertStatus);
        logd("broadcast intent ACTION_SIM_INSERTED_STATUS " +  nSimInsertStatus);
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
    }
    
    public static void disposeReceiver() {
        // [ALPS00396046][mtk02772] start
        logd("[disposeReceiver]");
        mContext.unregisterReceiver(mReceiver);
        // [ALPS00396046][mtk02772] end
    }
    private static void logd(String message) {
        Log.d(LOG_TAG, "[SimInfoUpdate]" + message);
    }
}

