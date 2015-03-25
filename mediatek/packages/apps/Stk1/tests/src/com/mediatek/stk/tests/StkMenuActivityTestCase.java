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

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.util.Log;

import android.test.ActivityInstrumentationTestCase2;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;

import com.android.stk.StkMenuActivity;

public class StkMenuActivityTestCase extends ActivityInstrumentationTestCase2<StkMenuActivity> {
    private Instrumentation mInstrumentation = null;
    private StkMenuActivity mActivity = null;
    
    private static final int TIME_LONG = 2000;
    private static final String LOG_TAG = "StkMenuActivityTestCase";

    public StkMenuActivityTestCase() {
        super("com.android.stk", StkMenuActivity.class);
    }

    public StkMenuActivityTestCase(String pkg, Class<StkMenuActivity> activityClass) {
        super("com.android.stk", StkMenuActivity.class);
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
    
    public void testCase01__Override() {
        Log.i(LOG_TAG, "<<< test case 01");
        //enter first level menu
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkMenuActivity");
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP;
        
        // We assume this was initiated by the user pressing the tool kit icon
        intentFlags |= 0;  

        newIntent.putExtra("STATE", 1); //1 is StkMenuActivity.STATE_MAIN
        
        newIntent.setFlags(intentFlags);
        
        try {
            mActivity = (StkMenuActivity) mInstrumentation.startActivitySync(newIntent);
            assertNotNull(mActivity);
        } catch (RuntimeException e) {
            Log.i(LOG_TAG, "can not test this activity because of: " + e.getMessage());
        }
        
        /*
        //enter second level menu
        Intent newIntent1 = new Intent(Intent.ACTION_VIEW);
        newIntent1.setClassName("com.android.stk", "com.android.stk.StkMenuActivity");
        int intentFlags1 = Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP;
        intentFlags1 |= Intent.FLAG_ACTIVITY_NO_USER_ACTION;  
        
        newIntent1.putExtra("STATE", 2); //2 is StkMenuActivity.STATE_SECONDARY
        newIntent1.setFlags(intentFlags1);
        StkMenuActivity mActivity2 = (StkMenuActivity) mInstrumentation.startActivitySync(newIntent1);
        assertNotNull(mActivity2);
        
        //finish 2
        Intent newIntent2 = new Intent(Intent.ACTION_VIEW);
        newIntent2.setClassName("com.android.stk", "com.android.stk.StkMenuActivity");
        int intentFlags2 = Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TOP;

        intentFlags2 |= Intent.FLAG_ACTIVITY_NO_USER_ACTION;
        newIntent2.putExtra("STATE", 3); //3 is StkMenuActivity.STATE_END 
        
        newIntent2.setFlags(intentFlags2);
        mInstrumentation.startActivitySync(newIntent2);
      
        //finish 1
        mActivity.finish();
        */
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        Log.i(LOG_TAG, ">>> test case 01");
    }
    
    /*
    public void test02_KeyBS() {
        //enter first level menu
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkMenuActivity");
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP;
        
        // We assume this was initiated by the user pressing the tool kit icon
        intentFlags |= 0;  

        newIntent.putExtra("STATE", 1); // 1 is StkMenuActivity.STATE_MAIN
        
        newIntent.setFlags(intentFlags);
        mActivity = (StkMenuActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mActivity);
        
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK); //will finish this activity
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void test03_OptionMenu() {
        //enter first level menu
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName("com.android.stk", "com.android.stk.StkMenuActivity");
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP;
        
        // We assume this was initiated by the user pressing the tool kit icon
        intentFlags |= 0;  

        newIntent.putExtra("STATE", 1); // 1 is StkMenuActivity.STATE_MAIN
        
        newIntent.setFlags(intentFlags);
        mActivity = (StkMenuActivity) mInstrumentation.startActivitySync(newIntent);
        assertNotNull(mActivity);
        
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MENU); //will finish this activity
        
        mActivity.finish();
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    */
}
