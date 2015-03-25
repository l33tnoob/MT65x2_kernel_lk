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

package com.android.simmelock;

import android.app.Activity;
import android.app.TabActivity;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

public class TabLockList extends TabActivity implements OnTabChangeListener {
    private static final String TAG = "Gemini_Simme Lock";
    private static final boolean DBG = true;

    private static final int INTENT_SIM1_INT_EXTRA = 0;
    private static final int INTENT_SIM2_INT_EXTRA = 1;
    private static final int INTENT_SIM3_INT_EXTRA = 2;
    private static final int INTENT_SIM4_INT_EXTRA = 3;
    private static final int TAB_SIM_1 = 0;
    private static final int TAB_SIM_2 = 1;

    private TabHost mTabHost;
    private GeminiPhone mGeminiPhone;
    private boolean Sim1State = false;
    private boolean Sim2State = false;
    private boolean Sim3State = false;
    private boolean Sim4State = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();

        if (mGeminiPhone != null && telephonyManagerEx != null) {
            Sim1State = telephonyManagerEx.hasIccCard(PhoneConstants.GEMINI_SIM_1) & mGeminiPhone.isRadioOnGemini(PhoneConstants.GEMINI_SIM_1);
            Sim2State = telephonyManagerEx.hasIccCard(PhoneConstants.GEMINI_SIM_2) & mGeminiPhone.isRadioOnGemini(PhoneConstants.GEMINI_SIM_2);
            if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                Sim3State = telephonyManagerEx.hasIccCard(PhoneConstants.GEMINI_SIM_3) & mGeminiPhone.isRadioOnGemini(PhoneConstants.GEMINI_SIM_3);
            }
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                Sim4State = telephonyManagerEx.hasIccCard(PhoneConstants.GEMINI_SIM_4) & mGeminiPhone.isRadioOnGemini(PhoneConstants.GEMINI_SIM_4);
            }
        }
        mTabHost = getTabHost();
        mTabHost.setOnTabChangedListener(this);

        SetupSIM1Tab();
        SetupSIM2Tab();
        if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
            setupSIM3Tab();
        }
        if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
            setupSIM4Tab();
        }
        SetCurrentTab();
    }

    private void SetCurrentTab() {
        // Gemini+ set current tab only sim1
        if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
            mTabHost.setCurrentTab(TAB_SIM_1);
            return;
        }
        
        if (!Sim1State && Sim2State) {
            mTabHost.setCurrentTab(TAB_SIM_2);
        } else {
            mTabHost.setCurrentTab(TAB_SIM_1);
        }
    }

    private void SetupSIM1Tab() {
        Intent intent = new Intent("android.intent.action.MAIN");
        intent.setClass(this, LockList.class);
        intent.putExtra("Setting SIM Number", INTENT_SIM1_INT_EXTRA);
        mTabHost.addTab((mTabHost.newTabSpec("SIM1")).setIndicator("SIM1",
                getResources().getDrawable(R.drawable.tab_manage_sim1)).setContent(intent));
    }

    private void SetupSIM2Tab() {
        Intent intent = new Intent("android.intent.action.MAIN");
        intent.setClass(this, LockList.class);
        intent.putExtra("Setting SIM Number", INTENT_SIM2_INT_EXTRA);
        mTabHost.addTab((mTabHost.newTabSpec("SIM2")).setIndicator("SIM2",
                getResources().getDrawable(R.drawable.tab_manage_sim2)).setContent(intent));
    }

    private void setupSIM3Tab() {
        Intent intent = new Intent("android.intent.action.MAIN");
        intent.setClass(this, LockList.class);
        intent.putExtra("Setting SIM Number", INTENT_SIM3_INT_EXTRA);
        mTabHost.addTab((mTabHost.newTabSpec("SIM3")).setIndicator("SIM3",
                getResources().getDrawable(R.drawable.tab_manage_sim2)).setContent(intent));
    }

    private void setupSIM4Tab() {
        Intent intent = new Intent("android.intent.action.MAIN");
        intent.setClass(this, LockList.class);
        intent.putExtra("Setting SIM Number", INTENT_SIM4_INT_EXTRA);
        mTabHost.addTab((mTabHost.newTabSpec("SIM4")).setIndicator("SIM3",
                getResources().getDrawable(R.drawable.tab_manage_sim2)).setContent(intent));
    }

    @Override
    public void onTabChanged(String tabId) {
        Activity activity = getLocalActivityManager().getActivity(tabId);
        activity.onWindowFocusChanged(true);
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }
}
