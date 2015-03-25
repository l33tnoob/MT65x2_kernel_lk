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
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.io.MsdcDrivSet;
import com.mediatek.engineermode.io.MsdcHopSet;
import com.mediatek.engineermode.io.MsdcSd3Test;
import com.mediatek.engineermode.io.MsdcSelect;
import com.mediatek.xlog.Xlog;

public class MsdcTest extends ActivityInstrumentationTestCase2<MsdcSelect> {
    private static final String TAG = "Msdc Test";
    private static final int EXPECTED_ITEM_COUNT = 2;
    private static final int EXPECTED_HOST_COUNT = 5;
    private static final int EXPECTED_COMMAN_COUNT = 8;
    private static final int EXPECTED_SDMODE_COUNT = 6;
    private static final int EXPECTED_SDMAXCURRENT_COUNT = 4;
    private static final int EXPECTED_SDDRIVE_COUNT = 4;
    private static final int EXPECTED_SDPOWERCONTROL_COUNT = 2;
    private static final int DRIVING_SET_INDEX = 0;
    private static final int SD30_TEST_INDEX = 1;
    private static Instrumentation mInst = null;
    private static Solo mSolo = null;
    private static Activity mActivity = null;
    private static Context mContext = null;
    private static ListView mListView = null;

    public MsdcTest() {
        super("com.mediatek.engineermode", MsdcSelect.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        mListView = (ListView) mActivity.findViewById(R.id.ListView_msdcSelect);

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

    public void test03_DrivSet() {
        Log.v("MsdcTest", "mListView 0"
                + mListView.getAdapter().getItem(DRIVING_SET_INDEX).toString());
        mSolo.clickOnText(mListView.getAdapter().getItem(DRIVING_SET_INDEX)
                .toString());
        mSolo.waitForActivity(MsdcDrivSet.class.getSimpleName());
        Activity MsdcDrivSetActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not MsdcDrivSet Class", MsdcDrivSet.class);
        mSolo.goBack();
        final Spinner hostSpinnner = (Spinner) MsdcDrivSetActivity
                .findViewById(R.id.NEW_MSDC_HOST_sppiner);
        Spinner clkPuSpinner = (Spinner) MsdcDrivSetActivity
                .findViewById(R.id.MSDC_Clk_pu_spinner);
        assertNotNull(hostSpinnner);
        assertNotNull(clkPuSpinner);
//        assertEquals(EXPECTED_HOST_COUNT, hostSpinnner.getAdapter().getCount());
//        assertEquals(EXPECTED_COMMAN_COUNT, clkPuSpinner.getAdapter()
//                .getCount());
        // mSolo.pressSpinnerItem(DRIVING_SET_INDEX, 2);
        EmOperate.runOnUiThread(mInst, MsdcDrivSetActivity, new Runnable() {

            public void run() {
                hostSpinnner.setSelection(1);
            }

        });
        mSolo.sleep(200);
        assertEquals("Host Number 1", hostSpinnner.getSelectedItem().toString());
    }

    public void test04_SdTest() {
        mSolo.clickOnText(mListView.getAdapter().getItem(SD30_TEST_INDEX)
                .toString());
        mSolo.waitForActivity(MsdcSd3Test.class.getSimpleName());
        Activity msdcHopSetActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not MsdcSd3Test Class", MsdcSd3Test.class);
        mSolo.goBack();
        Spinner spinnerHost = (Spinner) msdcHopSetActivity
                .findViewById(R.id.msdc_sd3test_host_spinner);
        Spinner spinnerMode = (Spinner) msdcHopSetActivity
                .findViewById(R.id.msdc_sd3test_mode_spinner);
        Spinner spinnerMaxCurrent = (Spinner) msdcHopSetActivity
                .findViewById(R.id.msdc_sd3test_max_current_spinner);
        Spinner spinnerDrive = (Spinner) msdcHopSetActivity
                .findViewById(R.id.msdc_sd3test_drive_spinner);
        Spinner spinnerPowerControl = (Spinner) msdcHopSetActivity
                .findViewById(R.id.msdc_sd3test_power_control_spinner);
        assertNotNull(spinnerHost);
        assertNotNull(spinnerMode);
        assertNotNull(spinnerMaxCurrent);
        assertNotNull(spinnerDrive);
        assertNotNull(spinnerPowerControl);
        assertEquals(EXPECTED_HOST_COUNT-1, spinnerHost.getAdapter().getCount());
        assertEquals(EXPECTED_SDMODE_COUNT, spinnerMode.getAdapter().getCount());
        assertEquals(EXPECTED_SDMAXCURRENT_COUNT, spinnerMaxCurrent
                .getAdapter().getCount());
        assertEquals(EXPECTED_SDDRIVE_COUNT, spinnerDrive.getAdapter()
                .getCount());
        assertEquals(EXPECTED_SDPOWERCONTROL_COUNT, spinnerPowerControl
                .getAdapter().getCount());
    }

    public void test05_DrivGetSet() {
        mSolo.clickOnText(mListView.getAdapter().getItem(DRIVING_SET_INDEX)
                .toString());
        mSolo.waitForActivity(MsdcDrivSet.class.getSimpleName());
        Activity MsdcDrivSetActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not MsdcDrivSet Class", MsdcDrivSet.class);

        Button getBtn = (Button) MsdcDrivSetActivity
                .findViewById(R.id.NEW_MSDC_Get);
        Button setBtn = (Button) MsdcDrivSetActivity
                .findViewById(R.id.NEW_MSDC_Set);
        Spinner hostSpinnner = (Spinner) MsdcDrivSetActivity
                .findViewById(R.id.NEW_MSDC_HOST_sppiner);
        final Spinner clkPuSpinner = (Spinner) MsdcDrivSetActivity
                .findViewById(R.id.MSDC_Clk_pu_spinner);
        assertNotNull(getBtn);
        assertNotNull(setBtn);
        assertNotNull(hostSpinnner);
        assertNotNull(clkPuSpinner);

        // back up origin value
        mSolo.clickOnButton(getBtn.getText().toString());
        mSolo.sleep(200);
        assertEquals(false, mSolo.searchText(MsdcDrivSetActivity.getString(R.string.msdc_get_fail_message)));
        final int backVal = clkPuSpinner.getSelectedItemPosition();
        EmOperate.runOnUiThread(mInst, MsdcDrivSetActivity, new Runnable() {

            public void run() {
                clkPuSpinner.setSelection(3);
            }

        });
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.clickOnButton(setBtn.getText().toString());
        mSolo.sleep(EmOperate.TIME_SHORT);
        assertEquals(true, mSolo.searchText(MsdcDrivSetActivity.getString(R.string.msdc_set_ok_message)));
        mSolo.clickOnButton(0);
        Log.v("MsdcTest", "getBtn " + getBtn.getText().toString());
        mSolo.searchButton("Get");
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.clickOnButton(getBtn.getText().toString());
        mSolo.sleep(EmOperate.TIME_SHORT);
        int clkindex = clkPuSpinner.getSelectedItemPosition();
        assertEquals(3, clkindex);
        
        // restore backup value
        EmOperate.runOnUiThread(mInst, MsdcDrivSetActivity, new Runnable() {

            public void run() {
                clkPuSpinner.setSelection(backVal);
            }

        });
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.clickOnButton(setBtn.getText().toString());
        mSolo.sleep(200);
        mSolo.clickOnButton(0);
        mSolo.goBack();
        Xlog.d(TAG, "[FOR_NATA_MSDC_GET_SET_PASS]");
    }

    public void test06_SDTestGet() {
        mSolo.clickOnText(mListView.getAdapter().getItem(SD30_TEST_INDEX)
                .toString());
        mSolo.waitForActivity(MsdcSd3Test.class.getSimpleName());
        Activity msdcSdTestActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not MsdcSd3Test class", MsdcSd3Test.class);
        mSolo.goBack();
        mSolo.searchButton("Set");
        mSolo.sleep(EmOperate.TIME_SHORT);
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

}
