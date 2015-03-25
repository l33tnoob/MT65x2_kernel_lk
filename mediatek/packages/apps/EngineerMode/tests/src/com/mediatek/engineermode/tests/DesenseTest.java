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
import android.test.UiThreadTest;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.desense.DesenseActivity;
import com.mediatek.engineermode.desense.DesensePllsActivity;
import com.mediatek.engineermode.desense.FreqHoppingSet;
import com.mediatek.engineermode.desense.MemPllSet;
import com.mediatek.engineermode.desense.PllDetailActivity;

public class DesenseTest extends
        ActivityInstrumentationTestCase2<DesenseActivity> {

    private static final String TAG = "EMTest/desense";
    private static final int ITEM_COUNT = 3;
    private static final int SLEEP_TIME = 1000;
    private Solo mSolo = null;
    private Activity mActivity = null;
    private Context mContext = null;
    private Instrumentation mInst = null;
    private ListView mListView = null;

    public DesenseTest() {
        super("com.mediatek.engineermode", DesenseActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        mListView = (ListView) mActivity.findViewById(R.id.desense_listview);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_Precondition() {
        assertNotNull(mInst);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
        assertNotNull(mListView);
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test02_CheckItems() {
        int actual = mListView.getAdapter().getCount();
        assertEquals(ITEM_COUNT, actual);
    }

    public void test03_TestPlls() {
        mSolo.clickOnText(mListView.getAdapter().getItem(0).toString());
        mSolo.waitForActivity(DesensePllsActivity.class.getSimpleName());
        Activity pllsActivity = mSolo.getCurrentActivity();
        ListView pllMenuListView = (ListView) pllsActivity
                .findViewById(R.id.pll_menu_listview);
        assertEquals(8, pllMenuListView.getAdapter().getCount());
        mSolo.clickOnText(pllMenuListView.getAdapter().getItem(0).toString());
        mSolo.waitForActivity(PllDetailActivity.class.getSimpleName());
        mSolo.clickOnButton(0);
        final EditText valueEdt = mSolo.getEditText(0);
        EmOperate.runOnUiThread(mInst, pllsActivity, new Runnable() {

            public void run() {
                valueEdt.append("z");
            }

        });
        mInst.waitForIdleSync();
        mSolo.clickOnButton(0);
        mSolo.goBack();
        mSolo.goBack();
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test04_TestFreHopping() {
        mSolo.clickOnText(mListView.getAdapter().getItem(1).toString());
        mSolo.waitForActivity(FreqHoppingSet.class.getSimpleName());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.pressSpinnerItem(0, 4);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.clickOnButton("Enable");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton("Disable");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton("Read All");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.enterText(0, "1");
        mSolo.clickOnButton("Enable");
        for (int i = 1; i < 5; i++) {
            mSolo.enterText(i, "1");
        }
        mSolo.clickOnButton("Enable");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton("Disable");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test05_MempllSet() {
        mSolo.clickOnText(mListView.getAdapter().getItem(2).toString());
        mSolo.waitForActivity(MemPllSet.class.getSimpleName());
        Activity mempllActivity = mSolo.getCurrentActivity();
        Button btn200to266 = (Button) mempllActivity
                .findViewById(R.id.desense_mempll_btn_convert266);
        Button btn266to200 = (Button) mempllActivity
                .findViewById(R.id.desense_mempll_btn_convert200);
        if (btn266to200.isEnabled()) {
            assertFalse(btn200to266.isEnabled());
            mSolo.clickOnButton(1);
            mSolo.sleep(EmOperate.TIME_MID);
            assertFalse(btn266to200.isEnabled());
            assertTrue(btn200to266.isEnabled());
            mSolo.clickOnButton(0);
            mSolo.sleep(EmOperate.TIME_MID);
        } else {
            assertFalse(btn266to200.isEnabled());
            mSolo.clickOnButton(0);
            mSolo.sleep(EmOperate.TIME_MID);
            assertFalse(btn200to266.isEnabled());
            assertTrue(btn266to200.isEnabled());
            mSolo.clickOnButton(1);
            mSolo.sleep(EmOperate.TIME_MID);
        }
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        mSolo.sleep(EmOperate.TIME_MID);
    }
}
