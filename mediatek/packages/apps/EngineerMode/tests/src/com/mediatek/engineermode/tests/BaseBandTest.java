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
import android.widget.EditText;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.baseband.Baseband;

public class BaseBandTest extends ActivityInstrumentationTestCase2<Baseband> {
    private static final String ADDRESS = "0xC0000010";
    private static final String RIGHT_ADDRESS = "C0000010";
    private static final String NUM_FORMAT = "11111111111";
    private static final String LEN = "12";
    private static final String LENIS0 = "0";
    private static final String VALUE = "10";
    private static final String MAX_VALUE = "1025";
    private Instrumentation mInst;
    private Solo msolo;
    private Context mContext;
    private Activity mActivity;

    public BaseBandTest() {
    super("com.mediatek.engineermode", Baseband.class);
    }

    protected void setUp() throws Exception {
    super.setUp();
    mInst = getInstrumentation();
    mContext = mInst.getTargetContext();
    mActivity = getActivity();
    msolo = new Solo(mInst, mActivity);
    }

    public void test01_Precondition() {
    assertNotNull(mInst);
    assertNotNull(mContext);
    assertNotNull(mActivity);
    assertNotNull(msolo);
    }

    public void test02_ReadAddr() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr), "");
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len), LEN);
    msolo.clickOnText(mActivity.getResources()
            .getString(R.string.Baseband_Read));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test03_ReadAddrMaxValue() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len),
            MAX_VALUE);
    msolo.clickOnText(mActivity.getResources()
            .getString(R.string.Baseband_Read));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test04_ReadLen() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len),
            LENIS0);
    msolo.clickOnText(mActivity.getResources()
            .getString(R.string.Baseband_Read));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test05_ReadAddrLen() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len), LEN);
    msolo.clickOnText(mActivity.getResources()
            .getString(R.string.Baseband_Read));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test06_WriteAddr() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len), LEN);
    msolo
            .enterText((EditText) mActivity.findViewById(R.id.Baseband_Val),
                    VALUE);
    msolo.clickOnText(mActivity.getResources().getString(
            R.string.Baseband_Write));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test07_testReadBtn() {
    Baseband activity = new Baseband();
    activity.functionCall(0, "C0100001", "12", "");
    }

    public void test08_testWriteBtn() {
    Baseband activity = new Baseband();
    activity.functionCall(1, "C0100011", "12", "10");
    }

    public void test09_WriteAddrMaxValue() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len),
            MAX_VALUE);
    msolo
            .enterText((EditText) mActivity.findViewById(R.id.Baseband_Val),
                    VALUE);
    msolo.clickOnText(mActivity.getResources().getString(
            R.string.Baseband_Write));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test10_checkValNull() {
    Baseband activity = new Baseband();
    activity.checkValue(null, null, null);
    }

    public void test11_checkVal0() {
    Baseband activity = new Baseband();
    activity.checkValue(ADDRESS, LENIS0, VALUE);
    }

    public void test12_checkValMax() {
    Baseband activity = new Baseband();
    activity.checkValue(ADDRESS, MAX_VALUE, VALUE);
    }

    public void test13_CheckNumberFormat() {
    Baseband activity = new Baseband();
    activity.checkValue(ADDRESS, NUM_FORMAT, VALUE);
    }

    public void test14_CheckNumberFormat1() {
    Baseband activity = new Baseband();
    activity.checkValue(ADDRESS, LEN, NUM_FORMAT);
    }

    public void test15_rightRead() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            RIGHT_ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len), LEN);
    msolo.clickOnText(mActivity.getResources()
            .getString(R.string.Baseband_Read));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test16_rightWrite() {
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Addr),
            RIGHT_ADDRESS);
    msolo.enterText((EditText) mActivity.findViewById(R.id.Baseband_Len), LEN);
    msolo
            .enterText((EditText) mActivity.findViewById(R.id.Baseband_Val),
                    VALUE);
    msolo.clickOnText(mActivity.getResources().getString(
            R.string.Baseband_Write));
    msolo.sleep(EmOperate.TIME_SHORT);
    }

    protected void tearDown() throws Exception {
    super.tearDown();
    }

}
