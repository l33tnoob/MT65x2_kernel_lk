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

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.telephony.ISwitch3GPolicy;
import android.content.Context;
import android.util.Log;

/**
 * 3G Switch Policy (MTK Default)
 * 
 * @hide
 */
public class Switch3GPolicyDefault implements ISwitch3GPolicy {

    private static final String LOG_TAG = "Switch3GPolicyDefault";
    
    @Override
    public int select3GCapability(Context ctx, String iccId3GSim, int id3GSim, String iccId1, String iccId2, String iccId3, String iccId4) {
        
        int selected = ISwitch3GPolicy.CAPABILITY_3G_INVALID;
        
        Log.i(LOG_TAG, "select3GCapability (iccId3GSim="+iccId3GSim+", id3GSim="+id3GSim+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        /* if user did set 3G switeched SIM */
        if (iccId3GSim != null) {
            
            // LAST 3G SIM FOUND (at last position)   
            if ((id3GSim == PhoneConstants.GEMINI_SIM_1) && (iccId3GSim.equals(iccId1))) {
                Log.i(LOG_TAG, "Found 3G capability SIM at same slot (SIM1), stays.");
                selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
            } else if ((id3GSim == PhoneConstants.GEMINI_SIM_2) && (iccId3GSim.equals(iccId2))) {
                Log.i(LOG_TAG, "Found 3G capability SIM at same slot (SIM2), stays.");
                selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
            } else if ((id3GSim == PhoneConstants.GEMINI_SIM_3) && (iccId3GSim.equals(iccId3))) {
                Log.i(LOG_TAG, "Found 3G capability SIM at same slot (SIM3), stays.");
                selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
            } else if ((id3GSim == PhoneConstants.GEMINI_SIM_4) && (iccId3GSim.equals(iccId4))) {
                Log.i(LOG_TAG, "Found 3G capability SIM at same slot (SIM4), stays.");
                selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
            }
            
            if (selected == ISwitch3GPolicy.CAPABILITY_3G_INVALID) { // check if not decided yet.
                
                /*  scenario 1: check if it's still inserted in the orginal slot. 
                 * If it was moved to other sim slot, we automaitcially switch 3G to that slot. */
                if (iccId3GSim.equals(iccId1) && id3GSim != PhoneConstants.GEMINI_SIM_1) {
                    Log.i(LOG_TAG, "scenario 1: Need to switch 3G capability to SIM1");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM1;
                } else if (iccId3GSim.equals(iccId2) && id3GSim != PhoneConstants.GEMINI_SIM_2) {
                    Log.i(LOG_TAG, "scenario 1: Need to switch 3G capability to SIM2");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM2;
                } else if (iccId3GSim.equals(iccId3) && id3GSim != PhoneConstants.GEMINI_SIM_3) {
                    Log.i(LOG_TAG, "scenario 1: Need to switch 3G capability to SIM3");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM3;
                } else if (iccId3GSim.equals(iccId4) && id3GSim != PhoneConstants.GEMINI_SIM_4) {
                    Log.i(LOG_TAG, "scenario 1: Need to switch 3G capability to SIM4");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM4;
                }
            }
            
            if (selected == ISwitch3GPolicy.CAPABILITY_3G_INVALID) { // check if not decided yet.
                
                /* scenario 2: if ever set 3G switched SIM, 
                 * but the recorded 3G SIM is not matched (ex: not inserted), 
                 * we switch 3G to other inserted SIM card automatically  */
                if ((id3GSim == PhoneConstants.GEMINI_SIM_1 && iccId1 == null) ||
                    (id3GSim == PhoneConstants.GEMINI_SIM_2 && iccId2 == null) || 
                    (id3GSim == PhoneConstants.GEMINI_SIM_3 && iccId3 == null) ||
                    (id3GSim == PhoneConstants.GEMINI_SIM_4 && iccId4 == null))
                {
                    // SCAN NON-EMPTY FROM SLOT1
                    if (iccId1 != null) {
                        Log.i(LOG_TAG, "scenario 2: Need to switch 3G capability to the other inserted SIM1");
                        selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM1;
                    } else if (iccId2 != null) { 
                        Log.i(LOG_TAG, "scenario 2: Need to switch 3G capability to the other inserted SIM2");
                        selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM2;
                    } else if (iccId3 != null) { 
                        Log.i(LOG_TAG, "scenario 2: Need to switch 3G capability to the other inserted SIM3");
                        selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM3;
                    } else if (iccId4 != null) { 
                        Log.i(LOG_TAG, "scenario 2: Need to switch 3G capability to the other inserted SIM4");
                        selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM4;
                    }
                }
            }
            
        } else {
            
            /* scenario 3: If user not set 3G switched SIM (i.e. 3G SIM is default SIM1). 
             * But SIM1 not inserted , then check if there's other SIM card inserted, 
             * we switch 3G to other inserted SIM card automatically */
            if (iccId1 == null) {
                if (iccId2 != null) {
                    Log.i(LOG_TAG, "scenario 3: Need to switch 3G capability to the inserted SIM2");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM2;
                } else if (iccId3 != null) { 
                    Log.i(LOG_TAG, "scenario 3: Need to switch 3G capability to the inserted SIM3");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM3;
                } else if (iccId4 != null) { 
                    Log.i(LOG_TAG, "scenario 3: Need to switch 3G capability to the inserted SIM4");
                    selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM4;
                }
            }
        }
        
        return selected;
    }

    @Override
    public boolean isManualModeEnabled(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        return true;
    }

    @Override
    public boolean isManualModeChange3GAllowed(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        return true;
    }

    @Override
    public int getAllowedSwitch3GSlots(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        int nRet = 0;
        if (iccId1!=null)
            nRet |= 1;
        if (iccId2!=null)
            nRet |= 2;
        if (iccId3!=null)
            nRet |= 4;
        if (iccId4!=null)
            nRet |= 8;
        Log.i(LOG_TAG, "getAllowedSwitch3GSlots nRet="+nRet);
        return nRet;
    }

}
