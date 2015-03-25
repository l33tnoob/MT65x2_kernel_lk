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
import android.database.Cursor;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
//import com.android.internal.telephony.dataconnection.DcFailCause;
import com.mediatek.common.telephony.IGsmDCTExt;
import com.mediatek.common.featureoption.FeatureOption;

public class GsmDCTExt implements IGsmDCTExt{
    static final String TAG = "GsmDCTExt";

    public GsmDCTExt() {
    }

    public GsmDCTExt(Context context) {
    }

    public Cursor getOptPreferredApn(String imsi, String operator, int simID) {
        return null;
    }

    public boolean isDomesticRoamingEnabled() {
        return false;
    }

    public boolean isDataAllowedAsOff(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DEFAULT)) {
            return false;
        }
        return true;
    }

    public boolean getFDForceFlag(boolean force_flag) {
        //Only for operator (not CMCC) have the chance to set forceFlag as true when SCREEN is ON
        //Force to send SCRI msg to NW if MTK_FD_FORCE_REL_SUPPORT is true
        //ALPS00071650 for FET NW issue
        if(FeatureOption.MTK_FD_FORCE_REL_SUPPORT){
            return true;
        }
        return force_flag;    	
    }

    public int getPsAttachSimWhenRadioOn() {
        return -1;
    }

    public boolean isPsDetachWhenAllPdpDeactivated() {
        return true;
    }

    public boolean hasSmCauseRetry(int cause){        
        return false;
    }

    public boolean needRetry(int count){
        return false;
    }

    public int getDelayTime() {
        return 20000;
    }
    public boolean needDelayedRetry(int cause) {
        return false;
    }

    public boolean needSmRetry(Object cause){        
        log("default: no needSmRetry  " );
        return false;
    }
    public boolean doSmRetry(Object cause, Object obj1, Object obj2){
        //obj1 = RetryManager, obj2 = etc...
        log("default: no op sm retry");
        return false;
    }    
    public boolean setSmRetryConfig(Object retryManager){
        return false;
    }
    public boolean needRacUpdate(){
        return false;        
    }
    public boolean onRacUpdate(Object in){
        return false;        
    }

    public void log(String text) {
        Log.d(TAG, text);
    }
}
