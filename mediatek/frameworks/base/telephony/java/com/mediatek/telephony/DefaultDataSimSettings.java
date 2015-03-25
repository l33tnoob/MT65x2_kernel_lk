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

/*
 *
 */

package com.mediatek.telephony;

import static android.Manifest.permission.READ_PHONE_STATE;
import android.app.ActivityManagerNative;
import android.util.Log;
import android.content.Context;
import android.content.Intent;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.provider.Settings;
import android.net.ConnectivityManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import java.util.List;
import android.os.UserHandle;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.ITelephonyExt;

import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;


/**
 *@hide
 */
public class DefaultDataSimSettings {
    private static final String LOG_TAG = "PHONE";
    
    public static void setDataDefaultSim(Context context, ContentResolver contentResolver, List<SimInfoRecord> simInfos,
            long[] simIdForSlot, boolean[] isSimInserted, int nNewCardCount, int nSimCount, int[] mInsertSimState, boolean hasSimRemoved) {
        ITelephonyExt telephonyExt = null;
        try {
            telephonyExt = MediatekClassFactory.createInstance(ITelephonyExt.class);
        } catch (Exception e) {
            e.printStackTrace();
        }
        int n3gSIMSlot = get3GSimId(); 
        if (!FeatureOption.MTK_BSP_PACKAGE) {
            //get all default SIM setting
            long oldGprsDefaultSIM = Settings.System.getLong(contentResolver, Settings.System.GPRS_CONNECTION_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);     
            ConnectivityManager connectivityManager = (ConnectivityManager)context.getSystemService(context.CONNECTIVITY_SERVICE);
            logd("oldGprsDefaultSIM: " + oldGprsDefaultSIM + ", nSimCount: " + nSimCount + ", hasSimRemoved: " + hasSimRemoved);
            if (nSimCount > 1) {
                if (oldGprsDefaultSIM == Settings.System.DEFAULT_SIM_NOT_SET) {
                    if (telephonyExt.isDefaultDataOn())
                        connectivityManager.setMobileDataEnabledGemini(n3gSIMSlot);
                    else
                        connectivityManager.setMobileDataEnabled(false);
                }
            } else if (nSimCount == 1) {
                long simId = simInfos.get(0).mSimInfoId;
                // [mtk02772]
                // TODO: Data connection has different behavior between Gemini and non-Gemini,
                //       This part should be extracted to itself from SIM framework
                if (FeatureOption.MTK_GEMINI_SUPPORT == true) {   
                    if (oldGprsDefaultSIM == Settings.System.DEFAULT_SIM_NOT_SET) {
                        if (telephonyExt.isDefaultDataOn()) {
                            connectivityManager.setMobileDataEnabledGemini(simInfos.get(0).mSimSlotId);
                        } else {
                            connectivityManager.setMobileDataEnabled(false);
                        }
                    }
                } else {
                    if (oldGprsDefaultSIM == Settings.System.DEFAULT_SIM_NOT_SET) {
                        boolean enabled = true;
                        connectivityManager.setMobileDataEnabled(enabled);
                        logd("non-gemini, default on: " + enabled + ", for DEFAULT_SIM_NOT_SET");  
                    }   
                }
            }
    
            // [mtk02772]
            // TODO: Data connection has different behavior between Gemini and non-Gemini,
            //       This part should be extracted to itself from SIM framework
            if (FeatureOption.MTK_GEMINI_SUPPORT == true) { 
                if (isSimRemoved(oldGprsDefaultSIM,simIdForSlot,PhoneConstants.GEMINI_SIM_NUM)) {
                    if (telephonyExt.isDefaultDataOn()) {
                        if (nSimCount > 0) {
                            logd("default data SIM removed and default data on, switch to 3G SIM");
                            if (isSimInserted[n3gSIMSlot])
                                connectivityManager.setMobileDataEnabledGemini(n3gSIMSlot);
                            else
                                connectivityManager.setMobileDataEnabledGemini(simInfos.get(0).mSimSlotId);
                        }
                    } else {
                        connectivityManager.setMobileDataEnabled(false);
                    }
                } else if (telephonyExt.isDefaultEnable3GSIMDataWhenNewSIMInserted()) {
                    if (oldGprsDefaultSIM > 0) {
                        if (nNewCardCount > 0) {
                            logd("SIM swapped and data on, default switch to 3G SIM");
                            if (isSimInserted[n3gSIMSlot])
                                connectivityManager.setMobileDataEnabledGemini(n3gSIMSlot);
                            else
                                connectivityManager.setMobileDataEnabledGemini(simInfos.get(0).mSimSlotId);
                        } else {
                            boolean hasSIMRepositioned = false;
                            for (int i=0; i<mInsertSimState.length; i++) {
                                if (mInsertSimState[i] == SimInfoUpdate.SIM_REPOSITION) {
                                    hasSIMRepositioned = true;
                                    break;
                                }
                            }
                            if (!hasSimRemoved && hasSIMRepositioned) {
                                logd("Some SIM is switched slot, default switch to 3G SIM");
                                if (isSimInserted[n3gSIMSlot])
                                    connectivityManager.setMobileDataEnabledGemini(n3gSIMSlot);
                                else
                                    connectivityManager.setMobileDataEnabledGemini(simInfos.get(0).mSimSlotId);
                            }
                        }
                    } else {
                        if (nNewCardCount > 0 && nNewCardCount == nSimCount) {
                            logd("All SIM new, data off and default switch data to 3G SIM");
                            if (isSimInserted[n3gSIMSlot])
                                connectivityManager.setMobileDataEnabledGemini(n3gSIMSlot);
                            else
                                connectivityManager.setMobileDataEnabledGemini(simInfos.get(0).mSimSlotId);
                        }
                    }
                }
            } else { // non-Gemini 
                if (isSimRemoved(oldGprsDefaultSIM,simIdForSlot,PhoneConstants.GEMINI_SIM_NUM)) {
                    Settings.System.putLong(contentResolver, Settings.System.GPRS_CONNECTION_SIM_SETTING, 
                                                              Settings.System.GPRS_CONNECTION_SIM_SETTING_NEVER);  
                }
            }
    
            long gprsDefaultSIM = Settings.System.getLong(contentResolver, Settings.System.GPRS_CONNECTION_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
            if (gprsDefaultSIM != Settings.System.DEFAULT_SIM_NOT_SET && gprsDefaultSIM != Settings.System.GPRS_CONNECTION_SIM_SETTING_NEVER) {
                int slot = getSlotById(context, gprsDefaultSIM);        
                if (slot != SimInfoManager.SLOT_NONE) {
                    //if (FeatureOption.MTK_GEMINI_SUPPORT)
                    //    ((GeminiPhone)phone).setGprsConnType(1, slot);
                    connectivityManager.setMobileDataEnabledGemini(slot);
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        ITelephony iTelephony = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
                        if (iTelephony != null) {
                            try{
                                iTelephony.enableApnTypeGemini(PhoneConstants.APN_TYPE_DEFAULT, slot);
                            }catch (RemoteException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                } else {
                    logd("gprsDefaultSIM does not exist in slot then skip.");
                }
            }
        } else {
            ConnectivityManager connectivityManager = (ConnectivityManager)context.getSystemService(context.CONNECTIVITY_SERVICE);
            int gprsDefaultSlot = Settings.System.getInt(contentResolver, Settings.System.GPRS_CONNECTION_SETTING, Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
                logd("original data settings: " + gprsDefaultSlot);
            if (gprsDefaultSlot == Settings.System.DEFAULT_SIM_NOT_SET_INT) {
                if (telephonyExt.isDefaultDataOn()) {
                    int m3GSimId = get3GSimId();
                    if (isSimInserted[m3GSimId])
                        connectivityManager.setMobileDataEnabledGemini(m3GSimId);                       
    /*                      
                    if (is3GSwitched()) {
                        if (isSimInsert(Phone.GEMINI_SIM_2))
                            connectivityManager.setMobileDataEnabledGemini(Phone.GEMINI_SIM_2);
                        else if (isSimInsert(Phone.GEMINI_SIM_1))
                            connectivityManager.setMobileDataEnabledGemini(Phone.GEMINI_SIM_1);
                } else {
                        if (isSimInsert(Phone.GEMINI_SIM_1))
                            connectivityManager.setMobileDataEnabledGemini(Phone.GEMINI_SIM_1);
                        else if (isSimInsert(Phone.GEMINI_SIM_2))
                            connectivityManager.setMobileDataEnabledGemini(Phone.GEMINI_SIM_2);
                }
    */                        
                } else {
                    connectivityManager.setMobileDataEnabled(false);
                }
            } else if (gprsDefaultSlot >= PhoneConstants.GEMINI_SIM_1 && gprsDefaultSlot < PhoneConstants.GEMINI_SIM_NUM) {
                if (isSimInserted[gprsDefaultSlot]) { //data SIM slot has SIM inserted
                    logd("gprsDefaultSlot is inserted then enable it.");
                    connectivityManager.setMobileDataEnabledGemini(gprsDefaultSlot);
                } else {
                    if (telephonyExt.isDefaultDataOn()) { //data SIM slot has no SIM and default data on
                        for (int i=PhoneConstants.GEMINI_SIM_1; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
                            if (isSimInserted[i]) {
                                logd("enabled data of valid SIM: " + i);
                                connectivityManager.setMobileDataEnabledGemini(i);
                            }
                        }
                    } else { //data SIM slot has no SIM and default data off
                        connectivityManager.setMobileDataEnabled(false);
                    }
                }
            } else {
                connectivityManager.setMobileDataEnabled(false);
            }
    
            gprsDefaultSlot = Settings.System.getInt(contentResolver, Settings.System.GPRS_CONNECTION_SETTING, Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
            logd("final data settings: " + gprsDefaultSlot);
            // if (gprsDefaultSlot == Phone.GEMINI_SIM_1 || gprsDefaultSlot == Phone.GEMINI_SIM_2)
            //    setGprsConnType(GeminiNetworkSubUtil.CONN_TYPE_ALWAYS, gprsDefaultSlot);
        }
    }

    private static boolean isSimRemoved(long defSimId,long[] curSim, int numSim) {     
        // there is no default sim if defSIMId is less than zero
        if (defSimId <= 0) {
            return false;
        }
        
        boolean isDefaultSimRemoved = true;
        for (int i=0; i<numSim; i++)
        {
            if (defSimId == curSim[i]) {
                isDefaultSimRemoved = false;
                break; 
            }
        }
        return isDefaultSimRemoved;
    }

    private static int get3GSimId() {
        if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
            int simId = SystemProperties.getInt("gsm.3gswitch", 0); 
            if ((simId > 0)&& (simId <= PhoneConstants.GEMINI_SIM_NUM)) {
                return (simId -1); //Property value shall be 1~4,  convert to PhoneConstants.GEMINI_SIM_x
            }else {
                logd("get3GSimId() got invalid property value:"+ simId);
            }
        } 
        return PhoneConstants.GEMINI_SIM_1;
    }

    private static void logd(String message) {
        Log.d(LOG_TAG, "[DefaultDataSimSetting] " + message);
    }

    private static int getSlotById(Context context, long simId) {
        int  slotId = SimInfoManager.SLOT_NONE;
        SimInfoRecord simInfo = SimInfoManager.getSimInfoById(context, simId);
        if (null == simInfo) {
            logd("[getSlotById] simInfo is null");
        } else {
            slotId = simInfo.mSimSlotId;
        }
        return slotId;
    }
}
