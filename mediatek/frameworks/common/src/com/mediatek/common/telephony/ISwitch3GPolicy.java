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

package com.mediatek.common.telephony;

import android.content.Context;

/**
 * The 3G switch policy interface to provide 3G SIM selection logic.  
 * 
 */
public interface ISwitch3GPolicy {
    
    public static final String POLICY_DEFAULT = "DEFAULT";
    public static final String POLICY_OP = "OP";
    
    // 3G SIM Capability Settings
    public static final int CAPABILITY_3G_INVALID = -1;
//  public static final int CAPABILITY_3G_DEFAULT = 0;      // reserved.
    public static final int CAPABILITY_3G_GEMINI_SIM1 = 1;
    public static final int CAPABILITY_3G_GEMINI_SIM2 = 2;
    public static final int CAPABILITY_3G_GEMINI_SIM3 = 3;
    public static final int CAPABILITY_3G_GEMINI_SIM4 = 4;
//  public static final int CAPABILITY_3G_DT_NONSWAP = 11;  // reserved.
//  public static final int CAPABILITY_3G_DT_SWAP = 12;     // reserved.
    public static final int CAPABILITY_3G_NOCHANGE = 21;
    
    /**
     * To Select 3G Capable SIM based on custom policy. 
     * 
     * Return values including: 
     *     ISwitch3GPolicy.CAPABILITY_3G_INVALID:       invalid value. (no decision made)  
     *     ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM1:   SIM1 selected.
     *     ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM2:   SIM2 selected.
     *     ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM3:   SIM3 selected.
     *     ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM4:   SIM4 selected.
     *     ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE:      no change.
     * 
     * @param ctx current context
     * @param iccId3GSim the remembered 3G SIM IccId (last record) 
     * @param simId3GSim the remembered 3G SIM Id (last record)
     * @param iccId1 current SIM1 IccId, null if not exist/unknown
     * @param iccId2 current SIM2 IccId, null if not exist/unknown
     * @param iccId3 current SIM3 IccId, null if not exist/unknown
     * @param iccId4 current SIM4 IccId, null if not exist/unknown
     * @return the selected 3G Capable SIM based on custom policy
     */
    public int select3GCapability(Context ctx, String iccId3GSim, int simId3GSim, String iccId1, String iccId2, String iccId3, String iccId4);
    
    /**
     * To Check if 3G Switch Manual Control Mode Enabled. 
     * 
     * @param ctx current context
     * @param iccId1 current SIM1 IccId, null if not exist/unknown
     * @param iccId2 current SIM2 IccId, null if not exist/unknown
     * @param iccId3 current SIM3 IccId, null if not exist/unknown
     * @param iccId4 current SIM4 IccId, null if not exist/unknown
     * @return true if 3G Switch manual control mode is enabled, else false;
     */
    public boolean isManualModeEnabled(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4);
    
    /**
     * Check if 3G Switch allows Changing 3G SIM Slot in Manual Control Mode.  
     * 
     * @param ctx current context
     * @param iccId1 current SIM1 IccId, null if not exist/unknown
     * @param iccId2 current SIM2 IccId, null if not exist/unknown
     * @param iccId3 current SIM3 IccId, null if not exist/unknown
     * @param iccId4 current SIM4 IccId, null if not exist/unknown
     * @return true if 3G Switch allows Changing 3G SIM Slot in manual control mode, else false;
     */
    public boolean isManualModeChange3GAllowed(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4);
    
    /**
     * To Get 3G Switch Allowed 3G SIM Slots.
     * 
     * Returns an integer showing allowed 3G SIM Slots bitmasks. 
     *   Bit0 for SIM1; Bit1 for SIM2.  
     *   0 for disallowed; 1 for allowed. 
     * 
     * Examples as below: 
     *   00000001: SIM1 is allowed. 
     *   00000010: SIM2 is allowed.
     *   00000100: SIM3 is allowed. 
     *   00001000: SIM4 is allowed.
     *   00001111: SIM1, SIM2, SIM3, SIM4 are allowed.
     *   0:           no SIM is allowed. 
     * 
     * @param ctx current context
     * @param iccId1 current SIM1 IccId, null if not exist/unknown
     * @param iccId2 current SIM2 IccId, null if not exist/unknown
     * @param iccId3 current SIM3 IccId, null if not exist/unknown
     * @param iccId4 current SIM4 IccId, null if not exist/unknown
     * @return the allowed 3G SIM Slots bitmasks
     */
    public int getAllowedSwitch3GSlots(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4);
    
}
