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
import android.widget.ListView;
import android.widget.Spinner;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.io.IoList;
import com.mediatek.engineermode.io.Eint;
import com.mediatek.engineermode.io.Gpio;
import com.mediatek.engineermode.io.MsdcDrivSet;
import com.mediatek.engineermode.io.MsdcSelect;
import com.mediatek.xlog.Xlog;


public class IoListTest extends ActivityInstrumentationTestCase2<IoList> {
    private static final String TAG = "Msdc Test";
    private static final int EXPECTED_ITEM_COUNT = 3;

    private static Instrumentation mInst = null;
    private static Solo mSolo = null;
    private static Activity mActivity = null;
    private static Context mContext = null;
    private static ListView mListView = null;

    public IoListTest() {
        super(IoList.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        mListView = (ListView) mActivity.findViewById(R.id.ListView_Io);

    }

    public void test01_Prerequisite() {
        assertNotNull(mInst);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
        assertNotNull(mListView);
    }

    public void test02_CheckItem() {
        int expectedItem = EXPECTED_ITEM_COUNT;
        int actual = mListView.getAdapter().getCount();
        assertEquals(expectedItem, actual);
    }

    public void test03_Gpio() {
        mSolo.clickOnText(mActivity.getString(R.string.GPIO));
        mSolo.waitForActivity(Gpio.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        
        EmOperate.backKey(mInst); // back input 
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Direction_In));
        mSolo.sleep(EmOperate.TIME_MID);
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Direction_Out));
        mSolo.sleep(EmOperate.TIME_MID);
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Data_High));
        mSolo.sleep(EmOperate.TIME_MID);
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Data_Low));    
        mSolo.sleep(EmOperate.TIME_SUPER_LONG); 
        
        // error test
        mSolo.clearEditText(0);
        mSolo.enterText(0, "150");
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Direction_In));
        mSolo.sleep(EmOperate.TIME_MID);
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Direction_Out));
        mSolo.sleep(EmOperate.TIME_MID);
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Data_High));
        mSolo.sleep(EmOperate.TIME_MID);
        
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.GPIO_Data_Low));
        mSolo.sleep(EmOperate.TIME_MID); 
        mSolo.clearEditText(0);
        
        mSolo.sleep(EmOperate.TIME_MID);
        EmOperate.backKey(mInst);
    }

    public void test04_Eint() {
        mSolo.clickOnText(mActivity.getString(R.string.EINT));
        mSolo.waitForActivity(Eint.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst); // back input 
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        
        mSolo.clearEditText(0);
        mSolo.enterText(0, "10");
        mSolo.clickOnButton(mActivity.getResources().getString(
                R.string.EINT_query));
        mSolo.sleep(EmOperate.TIME_MID);
         
        
        EmOperate.backKey(mInst);
    }

    public void test05_Msdc() {
        mSolo.clickOnText(mActivity.getString(R.string.MSDC));
        
        mSolo.waitForActivity(MsdcSelect.class.getSimpleName(),
                    EmOperate.TIME_SUPER_LONG);

        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        mSolo.assertCurrentActivity("Not Found Activity MsdcSelect", MsdcSelect.class);
        EmOperate.backKey(mInst);
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        mSolo.goBack();
        Xlog.d(TAG, "[FOR_NATA_MSDC_LAUNCH_PASS]");
    }


    protected void tearDown() throws Exception {
        super.tearDown();
    }

}
