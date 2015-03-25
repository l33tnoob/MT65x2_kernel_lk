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

import android.view.View;
import android.widget.Button;
import android.view.KeyEvent;
import android.app.Activity;
import android.content.Intent;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;

import com.android.stk.StkDialogActivity;
import com.android.internal.telephony.cat.Duration;
import com.android.internal.telephony.cat.TextMessage;

import com.android.stk.R;

public class StkDialogActivityTestCase extends ActivityInstrumentationTestCase2<StkDialogActivity> {
    private Instrumentation mInstrumentation = null;
    private StkDialogActivity mDialogActivity = null;
    
    private Button mButtonOK = null;
    private Button mButtonCancel = null;
    
    private static final int TIME_LONG = 2000;
    private static final String LOG_TAG = "StkDialogActivityTestCase";
    

    public StkDialogActivityTestCase() {
        super("com.android.stk1", StkDialogActivity.class);
    }

    public StkDialogActivityTestCase(String pkg, Class<StkDialogActivity> activityClass) {
        super("com.android.stk1", StkDialogActivity.class);
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
            super.tearDown();
        } catch (Exception e) {
            e.printStackTrace();
        }
        Log.i(LOG_TAG, "<<<tear down");
    }

    // click OK button.
    public void testCase01_OK() {
        Log.d(LOG_TAG, ">>>click mButtonOK");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent();
        newIntent.setClassName("com.android.stk", "com.android.stk.StkDialogActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_MULTIPLE_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | 0);
                
        TextMessage msg = TextMessage.getInstance();
        msg.title = "test";
        msg.text = "test";
        msg.icon = null;
        msg.iconSelfExplanatory = false;
        msg.isHighPriority = true;
        msg.responseNeeded = true;
        msg.userClear = false;
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        msg.duration = duration;
        
        newIntent.putExtra("TEXT", msg);
        Log.d(LOG_TAG, ">>>click mButtonOK 00");
        mDialogActivity = (StkDialogActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mDialogActivity);
        
        mInstrumentation.waitForIdleSync();
        Log.d(LOG_TAG, "testCase01_OK, 01"); //
         mButtonOK = (Button) mDialogActivity.findViewById(R.id.button_ok);
        mButtonCancel = (Button) mDialogActivity.findViewById(R.id.button_cancel);
        
        assertNotNull(mButtonOK);
        clickView(mButtonOK); //will finish this activity
        Log.d(LOG_TAG, "<<<click mButtonOK");
    }

    // click speaker/earphone menu item.
    public void testCase02_Cancel() {
        Log.d(LOG_TAG, ">>>click mButtonCancel");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
            
        Intent newIntent = new Intent();
        newIntent.setClassName("com.android.stk", "com.android.stk.StkDialogActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_MULTIPLE_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | 0);
                
        TextMessage msg = TextMessage.getInstance();
        msg.title = "test";
        msg.text = "test";
        msg.icon = null;
        msg.iconSelfExplanatory = false;
        msg.isHighPriority = true;
        msg.responseNeeded = true;
        msg.userClear = false;
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        msg.duration = duration;
        
        newIntent.putExtra("TEXT", msg);
        Log.d(LOG_TAG, ">>>click testCase02_Cancel 00");
        mDialogActivity = (StkDialogActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mDialogActivity);
        
        Log.d(LOG_TAG, "testCase02_Cancel, 01"); //
        mButtonOK = (Button) mDialogActivity.findViewById(R.id.button_ok);
        mButtonCancel = (Button) mDialogActivity.findViewById(R.id.button_cancel);
        
        mInstrumentation.waitForIdleSync();
        Log.d(LOG_TAG, "testCase02_Cancel, 02"); //
        clickView(mButtonCancel); //will finish this activity
        Log.d(LOG_TAG, "<<<click mButtonCancel"); //
    }
    
    private void clickView(final View view) {
        try {
            runTestOnUiThread(new Runnable() {
                @Override
                public void run() {
                    view.performClick();
                    Log.d(LOG_TAG, "performClick"); //
                }
            });
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
    
    public void testCase03_SendBSKey() {
        Log.d(LOG_TAG, ">>>send backspace key event");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent();
        newIntent.setClassName("com.android.stk", "com.android.stk.StkDialogActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_MULTIPLE_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | 0);
                
        TextMessage msg = TextMessage.getInstance();
        msg.title = "test";
        msg.text = "test";
        msg.icon = null;
        msg.iconSelfExplanatory = false;
        msg.isHighPriority = true;
        msg.responseNeeded = true;
        msg.userClear = false;
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        msg.duration = duration;
        
        newIntent.putExtra("TEXT", msg);
        Log.d(LOG_TAG, ">>>click testCase03_SendBSKey 00");
        mDialogActivity = (StkDialogActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mDialogActivity);
        
        Log.d(LOG_TAG, "testCase03_SendBSKey, 01"); //
        mButtonOK = (Button) mDialogActivity.findViewById(R.id.button_ok);
        mButtonCancel = (Button) mDialogActivity.findViewById(R.id.button_cancel);
        
        Log.d(LOG_TAG, "testCase03_SendBSKey, 02"); //
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK); //will finish this activity
  
        Log.d(LOG_TAG, "<<<send backspace key event");
    }
    
    /*
    public void testCase04_SIMStateChange() {
        Log.d(LOG_TAG, ">>>SIMStateChange event");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        
        mDialogActivity = getActivity();
        assertNotNull(mDialogActivity);
        
        mButtonOK = (Button) mDialogActivity.findViewById(R.id.button_ok);
        mButtonCancel = (Button) mDialogActivity.findViewById(R.id.button_cancel);
        
        mInstrumentation.waitForIdleSync();
        //mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK); //will finish this activity
  
        Log.d(LOG_TAG, "<<<SIMStateChange event");
    }
    */
}
