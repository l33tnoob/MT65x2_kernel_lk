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
import android.util.Log;

import android.test.AndroidTestCase;
import com.android.stk.EventReceiver;

public class EventReceiverTestCase extends AndroidTestCase {
    
    private EventReceiver mReceiver = null;
    public static final int TIME_LONG = 2000;
    static final String LOG_TAG = "EventReceiverTestCase";
    static final String ED_1 = "android.intent.action.stk.USER_ACTIVITY";
     //ED2:  Intent.ACTION_LOCALE_CHANGED
    static final String ED_2 = "android.intent.action.stk.BROWSER_TERMINATION";
    static final String ED_3 = "android.intent.action.stk.IDLE_SCREEN_AVAILABLE";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mReceiver = new EventReceiver();
    }
    
    @Override
    public void tearDown() throws Exception {
        super.tearDown();        
    }
    
    public void test01_Receive() {
        Intent intent1 = new Intent();
        intent1.setAction(ED_1);
        Log.d(LOG_TAG, "reveive intent1 USER_ACTIVITY test.");
        mReceiver.onReceive(getContext(), intent1);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    
    public void test02_Receive() {
        Intent intent2 = new Intent();
        intent2.setAction(Intent.ACTION_LOCALE_CHANGED);
        Log.d(LOG_TAG, "reveive intent2 ACTION_LOCALE_CHANGED test.");
        mReceiver.onReceive(getContext(), intent2);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void test03_Receive() {
        Intent intent3 = new Intent();
        intent3.setAction(ED_2);
        Log.d(LOG_TAG, "reveive intent3 ACTION_LOCALE_CHANGED test.");
        mReceiver.onReceive(getContext(), intent3);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void test04_Receive() {
        Intent intent4 = new Intent();
        intent4.setAction(ED_3);
        Log.d(LOG_TAG, "reveive intent4 ACTION_LOCALE_CHANGED test.");
        mReceiver.onReceive(getContext(), intent4);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
}