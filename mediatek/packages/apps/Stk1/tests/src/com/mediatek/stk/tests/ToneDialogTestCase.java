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


import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.app.Activity;
import android.content.Intent;
import android.view.KeyEvent;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;

import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.ToneSettings;
import com.android.internal.telephony.cat.Tone;
import com.android.internal.telephony.cat.Duration;

import com.android.stk.ToneDialog;

import com.android.stk.R;

public class ToneDialogTestCase extends ActivityInstrumentationTestCase2<ToneDialog> {
    private Instrumentation mInstrumentation = null;
    private ToneDialog mToneDialog = null;
    
    private static final int TIME_LONG = 2000;
    private static final String LOG_TAG = "ToneDialogTestCase";

    public ToneDialogTestCase() {
        super("com.android.stk", ToneDialog.class);
    }

    public ToneDialogTestCase(String pkg, Class<ToneDialog> activityClass) {
        super("com.android.stk", ToneDialog.class);
    }


    @Override
    protected void setUp() {
        try {
            super.setUp();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        Log.i(LOG_TAG, ">>>set up");
        setActivityInitialTouchMode(false);
        
        mInstrumentation = this.getInstrumentation();
        assertNotNull(mInstrumentation);
   
        Log.i(LOG_TAG, "<<<set up");

    }
    
    @Override
    protected void tearDown() {
        Log.i(LOG_TAG, ">>>tear down");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        try {
            super.tearDown();
        } catch (Exception e) {
            e.printStackTrace();
        }
        Log.i(LOG_TAG, "<<<tear down");
    }

    public void testCase01_playTone() {
        Log.d(LOG_TAG, ">>>testCase01_playTone");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent();
        newIntent.setClassName("com.android.stk", "com.android.stk.ToneDialog");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | Intent.FLAG_ACTIVITY_NO_USER_ACTION);
                
        TextMessage textMsg = TextMessage.getInstance();
        assertNotNull(textMsg);
        textMsg.text = "test";
        
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND); //SECOND 1000
        ToneSettings toneSettings = new ToneSettings(duration, Tone.DIAL, true);
        assertNotNull(toneSettings);
        
        newIntent.putExtra("TEXT", textMsg);
        newIntent.putExtra("TONE", toneSettings);
        mToneDialog = (ToneDialog) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mToneDialog);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
         //will timeout and finish this activity
    }
    
    public void testCase02_sendBSKey() {
        Log.d(LOG_TAG, ">>>testCase02_sendBSKey");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent();
        newIntent.setClassName("com.android.stk", "com.android.stk.ToneDialog");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | 0);
                
        TextMessage textMsg = TextMessage.getInstance();
        assertNotNull(textMsg);
        textMsg.text = "test";
        
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND); //SECOND 1000
        ToneSettings toneSettings = new ToneSettings(duration, Tone.DIAL, true); //0x01 is Dial
        assertNotNull(toneSettings);
        
        newIntent.putExtra("TEXT", textMsg);
        newIntent.putExtra("TONE", toneSettings);
        mToneDialog = (ToneDialog) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mToneDialog);
        
        Log.d(LOG_TAG, "send Back Space Key");
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
         //will timeout and finish this activity
         Log.d(LOG_TAG, "<<< testCase02_sendBSKey");
    }
}
