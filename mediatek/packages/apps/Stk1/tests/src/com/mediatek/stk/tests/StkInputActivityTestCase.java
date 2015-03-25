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
import android.widget.TextView;
import android.widget.EditText;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.view.KeyEvent;
import android.app.Activity;
import android.content.Intent;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;

import com.android.internal.telephony.cat.Duration;
import com.android.internal.telephony.cat.Input;

import com.android.stk.StkInputActivity;
import com.android.stk.R;

public class StkInputActivityTestCase extends ActivityInstrumentationTestCase2<StkInputActivity> {
    private Instrumentation mInstrumentation = null;
    private StkInputActivity mInputActivity = null;
    
    private Button mButtonOK = null;
    private Button mButtonYes = null;
    private Button mButtonNo = null;
    private EditText mTextIn = null;
    
    private static final int TIME_LONG = 1000;
    private static final String LOG_TAG = "StkInputActivityTestCase";

    public StkInputActivityTestCase() {
        super("com.android.stk2", StkInputActivity.class);
    }

    public StkInputActivityTestCase(String pkg, Class<StkInputActivity> activityClass) {
        super("com.android.stk2", StkInputActivity.class);
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

    // click OK button.
    public void testCase01_OK() {
        Log.d(LOG_TAG, ">>>click mButtonOK");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkInputActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | 0);
        
        Input input = Input.getInstance();
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        input.duration = duration;
        input.text = "test";
        input.defaultText = "1234";
        input.icon = null;
		    input.ucs2 = false;
		    input.packed = false;
		    input.digitOnly = true;
		    input.echo = true;
		    input.yesNo = false;
		    input.helpAvailable = true;
		    input.iconSelfExplanatory = false;
        
        
        newIntent.putExtra("INPUT", input);

        mInputActivity = (StkInputActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mInputActivity);
        
        mButtonOK = (Button) mInputActivity.findViewById(R.id.button_ok);
        mTextIn = (EditText) mInputActivity.findViewById(R.id.in_text);
        Log.i(LOG_TAG, "testCase01_OK,  01");
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_NUMPAD_0);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_NUMPAD_1);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_NUMPAD_0);
        
        Log.i(LOG_TAG, "testCase01_OK,  02");
        mInstrumentation.waitForIdleSync();
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        clickView(mButtonOK); //will finish this activity
        Log.i(LOG_TAG, "<<<tear down");
    }
    
    public void testCase02_Yes() {
        Log.d(LOG_TAG, ">>>click mButtonYES");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkInputActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | 0);
        
        Input input = Input.getInstance();
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        input.duration = duration;
        input.text = "test";
        input.defaultText = "defaultText";
        input.icon = null;
		    input.ucs2 = false;
		    input.packed = false;
		    input.digitOnly = false;
		    input.echo = true;
		    input.yesNo = true;
		    input.helpAvailable = true;
		    input.iconSelfExplanatory = false;
        
        
        newIntent.putExtra("INPUT", input);

        mInputActivity = (StkInputActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mInputActivity);

        Log.i(LOG_TAG, "testCase02_Yes,  01");
        mButtonYes = (Button) mInputActivity.findViewById(R.id.button_yes);
        mButtonNo = (Button) mInputActivity.findViewById(R.id.button_no);
        
        mInstrumentation.waitForIdleSync();
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        clickView(mButtonYes); //will finish this activity
        Log.d(LOG_TAG, "<<<click mButtonYES");
    }
    
    public void testCase03_No() {
        Log.d(LOG_TAG, ">>>click mButtonNO");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkInputActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | 0);
        
        Input input = Input.getInstance();
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        input.duration = duration;
        input.text = "test";
        input.defaultText = "defaultText";
        input.icon = null;
		    input.ucs2 = false;
		    input.packed = false;
		    input.digitOnly = false;
		    input.echo = true;
		    input.yesNo = true;
		    input.helpAvailable = true;
		    input.iconSelfExplanatory = false;
        
        
        newIntent.putExtra("INPUT", input);

        mInputActivity = (StkInputActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mInputActivity);
        
        mButtonYes = (Button) mInputActivity.findViewById(R.id.button_yes);
        mButtonNo = (Button) mInputActivity.findViewById(R.id.button_no);
        
        mInstrumentation.waitForIdleSync();
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        clickView(mButtonNo); //will finish this activity
        Log.d(LOG_TAG, "<<<click mButtonNO");
    }
    
    private void clickView(final View view) {
        try {
            runTestOnUiThread(new Runnable() {
                @Override
                public void run() {
                    view.performClick();
                }
            });
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
    
    public void testCase04_SendBSKey() {
        Log.d(LOG_TAG, ">>>send backspace key event");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkInputActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | 0);
        
        Input input = Input.getInstance();
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        input.duration = duration;
        input.text = "test";
        input.defaultText = "defaultText";
        input.icon = null;
		    input.ucs2 = false;
		    input.packed = false;
		    input.digitOnly = false;
		    input.echo = true;
		    input.yesNo = true;
		    input.helpAvailable = true;
		    input.iconSelfExplanatory = false;
        
        
        newIntent.putExtra("INPUT", input);

        mInputActivity = (StkInputActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mInputActivity);
        
        mInstrumentation.waitForIdleSync();
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK); //will finish this activity
  
        Log.d(LOG_TAG, "<<<send backspace key event");
    }
    
    public void testCase05_SendMenuKey() {
        Log.d(LOG_TAG, ">>>send menu key event");
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkInputActivity");
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | 0);
        
        Input input = Input.getInstance();
        Duration duration = new Duration(5, Duration.TimeUnit.SECOND);
        input.duration = duration;
        input.text = "test";
        input.defaultText = "defaultText";
        input.icon = null;
		    input.ucs2 = false;
		    input.packed = false;
		    input.digitOnly = false;
		    input.echo = true;
		    input.yesNo = true;
		    input.helpAvailable = true;
		    input.iconSelfExplanatory = false;
        
        
        newIntent.putExtra("INPUT", input);

        mInputActivity = (StkInputActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mInputActivity);
        
        mInstrumentation.waitForIdleSync();
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MENU); //will create the option menu
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK); //will finish the option menu
        
        mInstrumentation.waitForIdleSync();
        
        try {
            Thread.sleep(4000);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
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
