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

package com.mediatek.op.telephony;

import android.util.Log;
import android.content.Context;
import android.telephony.ServiceState;
import com.mediatek.common.telephony.IServiceStateExt;
import android.telephony.SignalStrength;
import com.mediatek.common.featureoption.FeatureOption;
import android.content.res.Resources;

import java.util.Map;

public class ServiceStateExt implements IServiceStateExt {
    static final String TAG = "GSM";

    public ServiceStateExt() {
    }

    public ServiceStateExt(Context context) {
    }

    public void onPollStateDone(ServiceState oldSS, ServiceState newSS, int oldGprsState, int newGprsState) {
    }

    public String onUpdateSpnDisplay(String plmn, int radioTechnology) {
        /* ALPS00362903 */
        if(FeatureOption.MTK_NETWORK_TYPE_ALWAYS_ON == true){
            // for LTE
            if (radioTechnology > 13 && plmn != Resources.getSystem().getText(com.android.internal.R.string.
                    lockscreen_carrier_default).toString()){
                plmn = plmn + " 4G";		
            }            
            /* ALPS00492303 */
            //if (radioTechnology > 2 && plmn != null){
            else if (radioTechnology > 2 && plmn != Resources.getSystem().getText(com.android.internal.R.string.
                    lockscreen_carrier_default).toString()){
                plmn = plmn + " 3G";		
            }				
        }

        return plmn;
    }

    public boolean isRegCodeRoaming(boolean originalIsRoaming, int mccmnc, String numeric) {
        return originalIsRoaming;
    }

    public boolean isImeiLocked(){
        return false;
    }		

    public boolean isBroadcastEmmrrsPsResume(int value) {
        return false;
    }

    public boolean needEMMRRS() {
        return false;
    }

    public boolean needSpnRuleShowPlmnOnly() {
        return false;
    }
    
    public boolean needBrodcastACMT(int error_type,int error_cause) {
        return false;
    }
	
    public boolean needRejectCauseNotification(int cause){
        return false;    
    }

    public boolean needIgnoredState(int state,int new_state,int cause){
        if((state == ServiceState.STATE_IN_SERVICE) && (new_state == 2)){
            /* Don't update for searching state, there shall be final registered state update later */						
            Log.i(TAG,"set dontUpdateNetworkStateFlag for searching state");                  
            return true;
        }	   

        /* -1 means modem didn't provide <cause> information. */
        if(cause != -1){
            if((new_state == 3) && (cause != 0)){
                /* This is likely temporarily network failure, don't update for better UX */			
                Log.i(TAG,"set dontUpdateNetworkStateFlag for REG_DENIED with cause");                  
                return true;
            }
        }	
		
        Log.i(TAG,"clear dontUpdateNetworkStateFlag");       	   

        return false;
    }	

    public boolean ignoreDomesticRoaming(){
        return false;
    }

    public int mapGsmSignalLevel(int asu,int GsmRscpQdbm){
        int level;
        // [ALPS01055164] -- START , for 3G network
        if (GsmRscpQdbm < 0) {
            // 3G network
            if (asu <= 5 || asu == 99) level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
            else if (asu >= 15) level = SignalStrength.SIGNAL_STRENGTH_GREAT;
            else if (asu >= 12)  level = SignalStrength.SIGNAL_STRENGTH_GOOD;
            else if (asu >= 9)  level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
            else level = SignalStrength.SIGNAL_STRENGTH_POOR;
        // [ALPS01055164] -- END
        } else {
            // 2G network 
            if (asu <= 2 || asu == 99) level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
            else if (asu >= 12) level = SignalStrength.SIGNAL_STRENGTH_GREAT;
            else if (asu >= 8)  level = SignalStrength.SIGNAL_STRENGTH_GOOD;
            else if (asu >= 5)  level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
            else level = SignalStrength.SIGNAL_STRENGTH_POOR;
        }
        return level;
    }

    public int mapGsmSignalDbm(int GsmRscpQdbm,int asu){
        int dBm;
        Log.d(TAG,"mapGsmSignalDbm() GsmRscpQdbm=" + GsmRscpQdbm + " asu=" + asu);
        if (GsmRscpQdbm < 0) {
            dBm = GsmRscpQdbm / 4; //Return raw value for 3G Network
        } else {
            dBm = -113 + (2 * asu);
        }
        return dBm;
    }

    public int setEmergencyCallsOnly(int state,int cid){
        if((cid == -1) || (state == 4)){
            /* state(4) is 'unknown'  and cid(-1) means cid was not provided in +creg URC */                
            Log.i(TAG,"No valid info to distinguish limited service and no service");                                        
            return -1;	
        }		
        else if(((state ==0)||(state == 3)) && ((cid & 0xffff)!=0)){
            return 1;
        }				
        else{
            return 0;
        }		
    }		
		
    public void log(String text) {
        Log.d(TAG, text);
    }

    public Map<String, String> loadSpnOverrides() {
        return null;
    }

    public boolean allowSpnDisplayed() {
        return true;
    }

    public boolean supportEccForEachSIM() {
        return false;
    }

    public void updateOplmn(Context context, Object ci) {
    }

    public String getEccPlmnValue() {
        return Resources.getSystem().getText(com.android.internal.R.string.emergency_calls_only).toString();
    }
}
