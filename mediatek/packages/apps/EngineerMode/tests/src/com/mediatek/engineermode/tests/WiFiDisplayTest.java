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

package com.mediatek.engineermode.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;
import com.mediatek.engineermode.R;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.wfdsettings.WfdList;

import java.util.ArrayList;

public class WiFiDisplayTest extends ActivityInstrumentationTestCase2<WfdList> {

    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private Activity mActivity;
    
    public WiFiDisplayTest() {
        super("com.mediatek.engineermode", WfdList.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
        
    }
    public void testCase01_Quality() {

        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Quality Enhancement");      

        mSolo.clearEditText(0);
        mSolo.enterText(0, "30");
        
        mSolo.clearEditText(1);
        mSolo.enterText(1, "120");
        
        mSolo.clickOnCheckBox(0);
        mSolo.clickOnCheckBox(1);
        mSolo.clickOnCheckBox(2);
        
        mSolo.clickOnButton("Done");
    }

    public void testCase02_VideoCapability() {
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Video Capability");
        
        mSolo.clickOnRadioButton(0);
        mSolo.clickOnRadioButton(1);
        mSolo.clickOnText("Enable resolution setting menu");
        mSolo.clickOnView(mSolo.getView(R.id.Wfd_Vdo_1080p));
        mSolo.clickOnText("Enable resolution setting menu");
        
        mSolo.clickOnButton("Done");
    }

    public void testCase03_PowerSaving() {
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Power Saving for Playing Video");
        
        mSolo.clickOnRadioButton(0);
        mSolo.clickOnRadioButton(1);
        mSolo.clickOnRadioButton(2);
        
        mSolo.clearEditText(0);
        mSolo.enterText(0, "30");
        
        mSolo.clickOnButton("Done");
    }

    public void testCase04_Security() {
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Security");
        
        mSolo.clickOnRadioButton(0);
        mSolo.clickOnRadioButton(1);
        
        mSolo.clickOnButton("Done");
    }

    public void testCase05_Profiling() {
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Latency Profiling");
        
        mSolo.clickOnButton(0);
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
    }
}
