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

import android.content.ContentResolver;
import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.common.telephony.ITelephonyEx;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.featureoption.FeatureOption;

import java.util.List;

/**
 *@hide
 */
public class DefaultVtSimSettings {
    private static final String LOG_TAG = "PHONE";

    public static void setVtDefaultSim(ContentResolver contentResolver,
            long[] simIdForSlot, boolean[] isSimInserted) {

        if (!FeatureOption.MTK_BSP_PACKAGE && FeatureOption.MTK_VT3G324M_SUPPORT) {
            long nVTDefSIM = Settings.System.DEFAULT_SIM_NOT_SET;
            int n3gSIMSlot = get3GSimId();

            if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
                logd("MTK_GEMINI_3G_SWITCH is open");
                /// Add for ALPS00776530 Under 3G_SWITCH, in case 3GSwitchManual not Enabled
                //  && 3GSwitchManualChange3G not Allowed, we set default vt sim card to 0,
                //  which means for UI is "off". @{
                ITelephonyEx iTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
                try {
                    if(iTelephonyEx != null) {
                        logd("iTelephonyEx.is3GSwitchManualEnabled() = " + iTelephonyEx.is3GSwitchManualEnabled()
                                + "; iTelephonyEx.is3GSwitchManualChange3GAllowed() = "
                                + iTelephonyEx.is3GSwitchManualChange3GAllowed());
                    }
                    if (iTelephonyEx != null && !iTelephonyEx.is3GSwitchManualEnabled()
                            && !iTelephonyEx.is3GSwitchManualChange3GAllowed()) {
                        nVTDefSIM = 0; // set 0 for "off"
                        logd("setVtDefaultSim set mVTDefSIM to 0");
                    } else if (isSimInserted[n3gSIMSlot]) {
                        nVTDefSIM = simIdForSlot[n3gSIMSlot];
                        logd("nVTDefSIM = " + nVTDefSIM + "; n3gSIMSlot =" + n3gSIMSlot);
                    }
                } catch (RemoteException e) {
                    Log.e(LOG_TAG, "mTelephony exception");
                }
                /// @}
            } else if (isSimInserted[PhoneConstants.GEMINI_SIM_1]) {
                nVTDefSIM = simIdForSlot[PhoneConstants.GEMINI_SIM_1];
            }
            logd("setVtDefaultSim -- nVTDefSIM : " + nVTDefSIM);
            Settings.System.putLong(contentResolver,
                    Settings.System.VIDEO_CALL_SIM_SETTING, nVTDefSIM);
        }
    }

    private static int get3GSimId() {
        if (FeatureOption.MTK_GEMINI_3G_SWITCH) {
            int simId = SystemProperties.getInt("gsm.3gswitch", 0);
            if ((simId > 0) && (simId <= PhoneConstants.GEMINI_SIM_NUM)) {
                //Property value shall be 1~4, convert to PhoneConstants.GEMINI_SIM_x
                return (simId - 1);
            } else {
                logd("get3GSimId() got invalid property value:" + simId);
            }
        }
        return PhoneConstants.GEMINI_SIM_1;
    }

    private static void logd (String message) {
        Log.d(LOG_TAG, "[DefaultVtSimSetting] " + message);
    }
}
