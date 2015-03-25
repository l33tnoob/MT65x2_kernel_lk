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
import com.mediatek.engineermode.settingsfontsize.SettingsFontSize;

public class SettingsFontSizeTest extends ActivityInstrumentationTestCase2<SettingsFontSize> {

    private static final String SMALL = "0.85";
    private static final String LARGE = "1.15";
    private static final String EXTRALARGE = "1.30";

    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private Activity mActivity;

    private EditText mSmallEdit;
    private EditText mLargeEdit;
    private EditText mExtLargeEdit;

    public SettingsFontSizeTest() {
        super("com.mediatek.engineermode", SettingsFontSize.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);

        mSmallEdit = (EditText) mActivity.findViewById(R.id.settings_fs_small_edit);
        mLargeEdit = (EditText) mActivity.findViewById(R.id.settings_fs_large_edit);
        mExtLargeEdit = (EditText) mActivity.findViewById(R.id.settings_fs_extralarge_edit);
        clearAllFont();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
    }

    public void testCase01_VerifyPreconditions() {
        verifyPreconditions();
    }

    public void testCase02_SetNoFontScale() {
        mSolo.clickOnText(mActivity.getResources().getString(R.string.settings_fs_ok));
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void testCase03_SetSmallFontScale() {
        mSolo.enterText((EditText) mActivity.findViewById(R.id.settings_fs_small_edit), SMALL);
        mSolo.clickOnText(mActivity.getResources().getString(R.string.settings_fs_ok));
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void testCase04_SetLargeFontScale() {
        mSolo.enterText((EditText) mActivity.findViewById(R.id.settings_fs_large_edit), LARGE);
        mSolo.clickOnText(mActivity.getResources().getString(R.string.settings_fs_ok));
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void testCase05_SetExLargeFontScale() {
        mSolo.enterText((EditText) mActivity.findViewById(R.id.settings_fs_extralarge_edit), EXTRALARGE);
        mSolo.clickOnText(mActivity.getResources().getString(R.string.settings_fs_ok));
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void testCase06_SetAllFontScale() {

        mSolo.enterText(mSmallEdit, SMALL);
        mSolo.enterText(mLargeEdit, LARGE);
        mSolo.enterText(mExtLargeEdit, EXTRALARGE);

        mSolo.clickOnText(mActivity.getResources().getString(R.string.settings_fs_ok));
        mSolo.sleep(EmOperate.TIME_MID);
    }

    private void clearAllFont() {
        mSolo.clearEditText(mSmallEdit);
        mSolo.clearEditText(mLargeEdit);
        mSolo.clearEditText(mExtLargeEdit);
    }
}
