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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.CheckBox;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.networkinfo.Content;
import com.mediatek.engineermode.networkinfo.NetworkInfo;

public class NetworkInfoTest extends
        ActivityInstrumentationTestCase2<NetworkInfo> {
    private static final int TWO_SECONDS = 2000;
    private Instrumentation mInst;
    private Context mContext;
    private Solo mSolo;
    private Activity mActivity;

    public NetworkInfoTest() {
    super("com.mediatek.engineermode", NetworkInfo.class);
    }

    protected void setUp() throws Exception {
    super.setUp();
    mInst = getInstrumentation();
    mContext = mInst.getTargetContext();
    mActivity = getActivity();
    mSolo = new Solo(mInst, mActivity);
    }

    public void test01_Precondition() {
    assertNotNull(mInst);
    assertNotNull(mContext);
    assertNotNull(mActivity);
    assertNotNull(mSolo);
    }

    public void test02_CheckItem() {
    CheckBox chk1 = (CheckBox) mActivity.findViewById(R.id.NetworkInfo_Cell);
    CheckBox chk2 = (CheckBox) mActivity.findViewById(R.id.NetworkInfo_Ch);
    assertNotNull(chk1);
    assertNotNull(chk2);
    mSolo.clickOnCheckBox(0);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.clickOnCheckBox(1);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    mSolo.sleep(TWO_SECONDS);
    String advanced = mActivity.getString(R.string.networkinfo_check);
    if (mSolo.searchText(advanced)) {
        mSolo.clickOnText(advanced);
        mSolo.sleep(TWO_SECONDS);
    }

    }

    public void test03_RR() {
    mSolo.clickOnCheckBox(4);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.clickOnCheckBox(5);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    mSolo.sleep(TWO_SECONDS);
    String advanced = mActivity.getString(R.string.networkinfo_check);
    if (mSolo.searchText(advanced)) {
        mSolo.clickOnText(advanced);
        mSolo.sleep(TWO_SECONDS);
    }
    }

    public void test04_RRMi() {
    mSolo.clickOnCheckBox(6);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.clickOnCheckBox(7);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    mSolo.sleep(TWO_SECONDS);
    String advanced = mActivity.getString(R.string.networkinfo_check);
    if (mSolo.searchText(advanced)) {
        mSolo.clickOnText(advanced);
        mSolo.sleep(TWO_SECONDS);
    }
    }

    public void test05_3GNeigh() {
    mSolo.clickOnCheckBox(17);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.clickOnCheckBox(18);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    mSolo.sleep(TWO_SECONDS);
    String advanced = mActivity.getString(R.string.networkinfo_check);
    if (mSolo.searchText(advanced)) {
        mSolo.clickOnText(advanced);
        mSolo.sleep(TWO_SECONDS);
    }
    }

    public void test06_3GMulti() {
    mSolo.clickOnCheckBox(20);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.clickOnCheckBox(21);
    EmOperate.waitSomeTime(EmOperate.TIME_MID);
    mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    mSolo.sleep(TWO_SECONDS);
    String advanced = mActivity.getString(R.string.networkinfo_check);
    if (mSolo.searchText(advanced)) {
        mSolo.clickOnText(advanced);
        mSolo.sleep(TWO_SECONDS);
    }
    }

    protected void tearDown() throws Exception {
    super.tearDown();
    }

}
