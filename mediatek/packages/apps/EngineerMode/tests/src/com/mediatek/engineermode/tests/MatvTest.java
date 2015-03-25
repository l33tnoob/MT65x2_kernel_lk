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
import android.test.ActivityInstrumentationTestCase2;
import android.content.Context;
import android.util.Log;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.EngineerMode;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;
import com.mediatek.common.featureoption.FeatureOption;

public class MatvTest extends ActivityInstrumentationTestCase2<EngineerMode> {
    private static final String TAG = "MatvTest";
    private Instrumentation mInst = null;
    private Activity mActivity = null;
    private Solo mSolo = null;
    private boolean mIsMatvSupported = false;
    
    public MatvTest() {
        super("com.mediatek.engineermode", EngineerMode.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        if (ChipSupport.isFeatureSupported(ChipSupport.HAVE_MATV_FEATURE)) {
            mIsMatvSupported = true;
        }
        Xlog.d(TAG, "HAVE_MATV_FEATURE: " + FeatureOption.HAVE_MATV_FEATURE);
    }
    
    public void testCase01Precondition() {
        assertNotNull(mInst);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
    }
    
    public void testCase02MatvExisted() {
        // switch to Hardware Testing
        mSolo.sendKey(Solo.RIGHT);
        mSolo.sendKey(Solo.RIGHT);
        mSolo.sleep(100);
        if (mIsMatvSupported) {
            assertTrue(mSolo.searchText("MATV"));
        } else {
            assertFalse(mSolo.searchText("MATV"));
            Xlog.d(TAG, "[FOR_NATA_MATV_NO_SUPPORT]");
        }
    }
    
    public void testCase03LaunchMatv() {
        if (mIsMatvSupported) {
            mSolo.sendKey(Solo.RIGHT);
            mSolo.sendKey(Solo.RIGHT);
            mSolo.sleep(100);
            mSolo.clickOnText("MATV");
            mSolo.sleep(500);
            mSolo.goBack();
            Xlog.d(TAG, "[FOR_NATA_MATV_LAUNCH_PASS]");
        }
    }


    protected void tearDown() throws Exception {
        mSolo.goBack();
        super.tearDown();
    }

}
