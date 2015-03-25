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
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.touchscreen.TsHandWriting;

public class TsHandWritingTest extends ActivityInstrumentationTestCase2<TsHandWriting> {
    private static final String TAG = "TsHandWriting";
    private static Instrumentation mInst = null;
    private static Solo mSolo = null;
    private static Activity mActivity = null;
    private static Context mContext = null;


    public TsHandWritingTest() {
        super(TsHandWriting.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);

    }

    public void test01_Prerequisite() {
        assertNotNull(mInst);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
    }

    public void test02_TouchTest() {
        Activity testActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not TsHandWriting Class", TsHandWriting.class);
        
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);  
        // clear 
        mSolo.pressMenuItem(0);
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);  
        
        mSolo.clickLongOnScreen(3, 3,EmOperate.TIME_SHORT);
        mSolo.clickLongOnScreen(19, 19, EmOperate.TIME_SHORT);
  
        mSolo.clickLongOnScreen(35, 35,EmOperate.TIME_SHORT);
        mSolo.clickLongOnScreen(56, 56, EmOperate.TIME_SHORT);
        
        mSolo.clickLongOnScreen(78, 78,EmOperate.TIME_SHORT);
        mSolo.clickLongOnScreen(90, 90, EmOperate.TIME_SHORT);
        
        mSolo.clickLongOnScreen(68, 168,EmOperate.TIME_SHORT);
        mSolo.clickLongOnScreen(180, 80, EmOperate.TIME_SHORT);
      
        mSolo.pressMenuItem(0);
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);    
        
    
        
        mSolo.goBack();
    }
    
    protected void tearDown() throws Exception {
        super.tearDown();
    }

}
