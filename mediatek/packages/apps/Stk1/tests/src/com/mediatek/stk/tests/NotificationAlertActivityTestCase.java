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
import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;

import com.android.stk.NotificationAlertActivity;
import com.android.stk.R;

public class NotificationAlertActivityTestCase extends ActivityInstrumentationTestCase2<NotificationAlertActivity> {
    private Instrumentation mInstrumentation = null;
    private NotificationAlertActivity mNotificationActivity = null;
    
    private Button mButtonOK = null;
    private Button mButtonCancel = null;
    
    private static final int TIMEOUT = 2000;
    private static final String TAG = "NotificationAlertActivityTestCase";

    public NotificationAlertActivityTestCase() {
        super(NotificationAlertActivity.class);
    }

    @Override
    protected void setUp() {
        try {
            super.setUp();
        } catch (Exception e) {
            e.printStackTrace();
        }
        Log.i(TAG, ">>>set up");
        setActivityInitialTouchMode(false);
        
        mInstrumentation = this.getInstrumentation();
        assertNotNull(mInstrumentation);
   
        Log.i(TAG, "<<<set up");

    }

    @Override
    protected void tearDown() {
        Log.i(TAG, ">>>tear down");
        
        try {
            Thread.sleep(TIMEOUT);
        } catch (InterruptedException e) {
            Log.d(TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        try {
            super.tearDown();
        } catch (Exception e) {
            e.printStackTrace();
        }
        Log.i(TAG, "<<<tear down");
    }
    
    // click OK button.
    public void testCase01_OK() {
        Log.d(TAG, ">>>click mButtonOK");
        
        try {
            Thread.sleep(TIMEOUT);
        } catch (InterruptedException e) {
            Log.d(TAG, "wait time InterruptedException: " + e.getMessage());
        }
        mNotificationActivity = getActivity();
        assertNotNull(mNotificationActivity);
        
        Log.d(TAG, ">>> get buttone 1");
        mButtonOK = (Button) mNotificationActivity.findViewById(R.id.button_ok);
        mButtonCancel = (Button) mNotificationActivity.findViewById(R.id.button_cancel);
        
        Log.d(TAG, ">>> click buttone 1");
        mInstrumentation.waitForIdleSync();
        clickView(mButtonOK);
        Log.d(TAG, "<<<click mButtonOK");
    }

    // click speaker/earphone menu item.
    public void testCase02_Cancel() {
        Log.d(TAG, ">>>click mButtonCancel");
        
        try {
            Thread.sleep(TIMEOUT);
        } catch (InterruptedException e) {
            Log.d(TAG, "wait time InterruptedException: " + e.getMessage());
        }
        
        mNotificationActivity = getActivity();
        assertNotNull(mNotificationActivity);
        
        Log.d(TAG, ">>> get buttone 2");
        mButtonOK = (Button) mNotificationActivity.findViewById(R.id.button_ok);
        mButtonCancel = (Button) mNotificationActivity.findViewById(R.id.button_cancel);
        
        Log.d(TAG, ">>> click buttone 2");
        mInstrumentation.waitForIdleSync();
        clickView(mButtonCancel);
        Log.d(TAG, "<<<click mButtonCancel");
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
}
