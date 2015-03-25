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

package com.mediatek.stk.tests;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;

import android.util.Log;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import android.test.AndroidTestCase;

import com.android.stk.BootCompletedReceiver;


public class BootCompletedReceiverTestCase extends AndroidTestCase {
    
    private BootCompletedReceiver mReceiver = null;
    public static final int TIME_LONG = 2000;
    static final String LOG_TAG = "BootCompletedReceiverTestCase";
     //1:  Intent.ACTION_BOOT_COMPLETED
     //2:  TelephonyIntents.ACTION_SIM_STATE_CHANGED
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mReceiver = new BootCompletedReceiver();
    }
    
    @Override
    public void tearDown() throws Exception {
        super.tearDown();        
    }
    
    public void test01_Receive() {
        Intent intent1 = new Intent();
        intent1.setAction(Intent.ACTION_BOOT_COMPLETED);
        Log.d(LOG_TAG, "reveive intent1 ACTION_BOOT_COMPLETED test.");
        mReceiver.onReceive(getContext(), intent1);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    //when sim card insert, uninstall stk2
    public void test02_Receive() {
        Intent intent2 = new Intent();
        intent2.setAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intent2.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 1);
        intent2.putExtra(IccCardConstants.INTENT_KEY_ICC_STATE, IccCardConstants.INTENT_VALUE_ICC_NOT_READY);
        Log.d(LOG_TAG, "reveive intent2 ACTION_SIM_STATE_CHANGED test.");
        mReceiver.onReceive(getContext(), intent2);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
     //when sim card insert, install stk2
    public void test03_Receive() {
        Intent intent3 = new Intent();
        intent3.setAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intent3.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 1);
        intent3.putExtra(IccCardConstants.INTENT_KEY_ICC_STATE, IccCardConstants.INTENT_VALUE_ICC_READY);
        Log.d(LOG_TAG, "reveive intent3 ACTION_SIM_STATE_CHANGED test.");
        mReceiver.onReceive(getContext(), intent3);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
}
