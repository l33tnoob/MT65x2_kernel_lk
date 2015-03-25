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

import android.util.Log;
import android.content.Context;
import android.content.ContentResolver;
import android.provider.Settings;
import android.net.sip.SipManager;
import com.android.internal.telephony.PhoneConstants;
import java.util.List;

import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

/**
 *@hide
 */
public class DefaultVoiceCallSimSettings {
    private static final String LOG_TAG = "PHONE";

    public static void setVoiceCallDefaultSim(Context context, ContentResolver contentResolver,
            List<SimInfoRecord> simInfos, long[] simIdForSlot, int nSimCount) {

        if (!FeatureOption.MTK_BSP_PACKAGE) {
            long defSIM = Settings.System.DEFAULT_SIM_NOT_SET;
            long oldVoiceCallDefaultSIM = Settings.System.getLong(contentResolver,
                    Settings.System.VOICE_CALL_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
            logd("nSimCount = : " + nSimCount);
            if (nSimCount > 1) {
                defSIM = Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK;
                if (isVoiceCallDefaultSIM(oldVoiceCallDefaultSIM)) {
                    logd("setVoiceCallDefaultSim -- To : " + Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK);
                    Settings.System.putLong(contentResolver,
                            Settings.System.VOICE_CALL_SIM_SETTING, Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK);
                }
            } else if (nSimCount == 1) {
                defSIM = simInfos.get(0).mSimInfoId;
                if (!(SipManager.isVoipSupported(context) && isVoipEnabled(contentResolver))
                        || (isVoiceCallDefaultSIM(oldVoiceCallDefaultSIM))) {
                    logd("setVoiceCallDefaultSim -- To : " + defSIM);
                    Settings.System.putLong(contentResolver,
                            Settings.System.VOICE_CALL_SIM_SETTING, defSIM);
                }
            }

            if (isSimRemoved(oldVoiceCallDefaultSIM, simIdForSlot, PhoneConstants.GEMINI_SIM_NUM)) {
                logd("setVoiceCallDefaultSim -- To : " + defSIM);
                Settings.System.putLong(contentResolver,
                        Settings.System.VOICE_CALL_SIM_SETTING, defSIM);
            }
        }
    }

    private static boolean isSimRemoved(long defSimId, long[] curSim, int numSim) {
        // there is no default sim if defSIMId is less than zero
        if (defSimId <= 0) {
            return false;
        }

        boolean isDefaultSimRemoved = true;
        for (int i = 0; i < numSim; i++) {
            if (defSimId == curSim[i]) {
                isDefaultSimRemoved = false;
                break;
            }
        }
        return isDefaultSimRemoved;
    }

    private static boolean isVoiceCallDefaultSIM (long voiceCallSIM) {
        return voiceCallSIM == Settings.System.DEFAULT_SIM_NOT_SET;
    }

    private static boolean isVoipEnabled (ContentResolver contentResolver) {
        return Settings.System.getInt(contentResolver,
                Settings.System.ENABLE_INTERNET_CALL, 0) == 1;
    }

    private static void logd (String message) {
        Log.d(LOG_TAG, "[DefaultVoiceCallSimSetting] " + message);
    }
}
