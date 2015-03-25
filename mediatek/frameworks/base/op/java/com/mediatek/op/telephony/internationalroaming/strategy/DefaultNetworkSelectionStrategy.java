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

package com.mediatek.op.telephony.internationalroaming.strategy;

import android.content.Context;

import com.android.internal.telephony.Phone;

import com.mediatek.common.telephony.internationalroaming.IInternationalRoamingController;
import com.mediatek.common.telephony.internationalroaming.InternationalRoamingConstants;
import com.mediatek.common.telephony.internationalroaming.strategy.INetworkSelectionStrategy;

public class DefaultNetworkSelectionStrategy extends StrategyBase implements
        INetworkSelectionStrategy {
    private static final String TAG = "[DefaultNWStrategy]";

    public DefaultNetworkSelectionStrategy(IInternationalRoamingController controller,
            Context context, Phone dualModePhone, Phone gsmPhone) {
        super(controller, context, dualModePhone, gsmPhone);
    }

    @Override
    public boolean needToBootOnGsm() {
        logd("needToBootOnGsm...");
        
        return false;
    }

    @Override
    public boolean needToBootOnCdma() {
        logd("needToBootOnCdma...");
        
        // Check for IPO or airplane mode reboot.
        return mController.hasSearchedOnCdma();
    }

    @Override
    public int onPreSwitchPhone() {
        logd("onPreSwitchPhone...");
        
        return InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS;
    }

    @Override
    public void onPostSwitchPhone() {
        logd("onPostSwitchPhone...");
    }

    @Override
    public void onGsmSuspend(String[] plmnString, int suspendedSession) {
        logd("onNoService: plmnString = " + plmnString);
        
        mController.resumeRegistration(InternationalRoamingConstants.RESUME_NW_GSM, suspendedSession);
    }

    @Override
    public void onCdmaPlmnChanged(String plmnString) {
        logd("onCdmaPlmnChanged: plmnString = " + plmnString);
        
        mController.resumeRegistration(InternationalRoamingConstants.RESUME_NW_CDMA, 0);
    }

    @Override
    public void onNoService(int phoneType) {
        logd("onNoService: phoneType = " + phoneType);
        
        mController.switchPhone(InternationalRoamingConstants.SIM_SWITCH_MODE_INVERSE, false);
    }

    @Override
    public void dispose() {
        logd("dispose...");
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }
}
