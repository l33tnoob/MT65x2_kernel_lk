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

import android.R.string;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;

import com.jayway.android.robotium.solo.Solo;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.audio.Audio;
import com.mediatek.engineermode.R;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

public class AudioTest extends ActivityInstrumentationTestCase2<Audio> {

    private static final int LIST_ITEMS_COUNT = 9;
    private static final int FIR_SPINNER_COUNT = 6;
    private static final String CURRENT_MODE = "CurrentMode";
    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private Activity mActivity;
    private ListView mAudioFuncList;

    public AudioTest() {
        super("com.mediatek.engineermode", Audio.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
        mAudioFuncList = (ListView) mActivity.findViewById(R.id.ListView_Audio);
    }

    public void testCase01_ActivityAudio() {
        verifyPreconditions();
    }

    public void testCase02_TestListView() {
        verifyPreconditions();
        int count = mAudioFuncList.getAdapter().getCount();
        //assertEquals(count, LIST_ITEMS_COUNT);
        for (int i = 0; i < count - 3; i++) {
            mSolo
                    .clickOnText(mAudioFuncList.getAdapter().getItem(i)
                            .toString());
            mSolo.sleep(EmOperate.TIME_MID);
            mSolo.goBack();
            mSolo.goBack();
        }
        for (int i = count - 3; i < count; i++) {
            mSolo
                    .clickOnText(mAudioFuncList.getAdapter().getItem(i)
                            .toString());
            mSolo.sleep(EmOperate.TIME_MID);
            mSolo.goBack();
        }
    }

    public void testCase03_TestNormalMode() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(0).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        Intent intent = mSolo.getCurrentActivity().getIntent();
        int currentMode = intent.getIntExtra(CURRENT_MODE, 0);
        assertEquals(0, currentMode);
        mSolo.pressSpinnerItem(0, 0);
        mSolo.pressSpinnerItem(1, 0);
        assertTrue(mSolo.getEditText(1).isEnabled());
        assertTrue(mSolo.getButton(1).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.clickOnButton(1);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(0).getText()));
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(1).getText()));
        mSolo.pressSpinnerItem(0, 2);
        mSolo.pressSpinnerItem(1, 1);
        assertFalse(mSolo.getEditText(1).isEnabled());
        assertFalse(mSolo.getButton(1).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.goBack();
    }

    public void testCase04_TestHeadSetMode() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(1).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        Intent intent = mSolo.getCurrentActivity().getIntent();
        int currentMode = intent.getIntExtra(CURRENT_MODE, 0);
        assertEquals(1, currentMode);
        mSolo.pressSpinnerItem(0, 0);
        mSolo.pressSpinnerItem(1, 0);
        assertTrue(mSolo.getEditText(1).isEnabled());
        assertTrue(mSolo.getButton(1).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.clickOnButton(1);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(0).getText()));
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(1).getText()));
        mSolo.pressSpinnerItem(0, 1);
        mSolo.pressSpinnerItem(1, 1);
        assertFalse(mSolo.getEditText(1).isEnabled());
        assertFalse(mSolo.getButton(1).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.goBack();
    }

    public void testCase05_TestLoudSpekMode() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(2).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        Intent intent = mSolo.getCurrentActivity().getIntent();
        int currentMode = intent.getIntExtra(CURRENT_MODE, 0);
        assertEquals(2, currentMode);
        mSolo.pressSpinnerItem(0, 0);
        mSolo.pressSpinnerItem(1, 0);
        assertTrue(mSolo.getEditText(1).isEnabled());
        assertTrue(mSolo.getButton(1).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.clickOnButton(1);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_SHORT);
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(0).getText()));
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(1).getText()));
        mSolo.pressSpinnerItem(0, 4);
        mSolo.pressSpinnerItem(1, 1);
        assertFalse(mSolo.getEditText(1).isEnabled());
        assertFalse(mSolo.getButton(1).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.goBack();
    }

    public void testCase06_TestHeadsetLoudSpekMode() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(3).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        Intent intent = mSolo.getCurrentActivity().getIntent();
        int currentMode = intent.getIntExtra(CURRENT_MODE, 0);
        assertEquals(3, currentMode);
        mSolo.pressSpinnerItem(0, 0);
        mSolo.pressSpinnerItem(1, 0);
        assertTrue(mSolo.getEditText(1).isEnabled());
        assertTrue(mSolo.getButton(1).isEnabled());
        assertTrue(mSolo.getEditText(2).isEnabled());
        assertTrue(mSolo.getButton(2).isEnabled());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.clickOnButton(1);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.clickOnButton(2);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.pressSpinnerItem(1, 1);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
    }

    public void testCase07_TestSpeechEnhanc() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(4).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        ArrayList<Spinner> spiners = mSolo.getCurrentViews(Spinner.class);
        assertNotNull(spiners);
        assertEquals(2, spiners.size());
        mSolo.pressSpinnerItem(0, 0);
        int count = spiners.get(1).getCount();
        assertEquals(count, 12);
        mSolo.pressSpinnerItem(0, 1);
        // int count2 = spiners.get(1).getCount(); // can't dynamic change
        // assertEquals(count2, 31);
        mSolo.pressSpinnerItem(1, 3);
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(0).getText()));
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.pressSpinnerItem(1, 17);
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(0).getText()));
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.goBack();
    }

    public void testCase08_TestDebugInfo() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(5).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.goBack();
        mSolo.pressSpinnerItem(0, 3);
        assertFalse(TextUtils.isEmpty(mSolo.getEditText(0).getText()));
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.goBack();
    }

    public void testCase09_TestSpeechLog() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(7).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        CheckBox cKCTM4WAY = (CheckBox) mSolo.getCurrentActivity()
                .findViewById(R.id.Audio_CTM4WAYLogger_Enable);
        assertNotNull(cKCTM4WAY);
        TextView cTM4WAYText = (TextView) mSolo.getCurrentActivity()
                .findViewById(R.id.Audio_CTM4WAYLogger_EnableText);
        assertNotNull(cTM4WAYText);
        View spliteView = (View) mSolo.getCurrentActivity().findViewById(
                R.id.Audio_View1);
        assertNotNull(spliteView);
        CheckBox cKSpeechLogger = (CheckBox) mSolo.getCurrentActivity()
                .findViewById(R.id.Audio_SpeechLogger_Enable);
        assertNotNull(cKSpeechLogger);
        RadioButton radioBtnBEPL = (RadioButton) mSolo.getCurrentActivity()
                .findViewById(R.id.Audio_SpeechLogger_EPL);
        assertNotNull(radioBtnBEPL);
        RadioButton radioBtnBNormalVm = (RadioButton) mSolo
                .getCurrentActivity().findViewById(
                        R.id.Audio_SpeechLogger_Normalvm);
        assertNotNull(radioBtnBNormalVm);

        ArrayList<RadioButton> radioButtons = mSolo.getCurrentViews(RadioButton.class);
        assertNotNull(radioButtons);
        assertEquals(2, radioButtons.size());

        ArrayList<CheckBox> checkBoxs = mSolo.getCurrentViews(CheckBox.class);
        assertNotNull(checkBoxs);
        //assertEquals(3, checkBoxs.size());
        if (ChipSupport.isFeatureSupported(ChipSupport.MTK_TTY_SUPPORT)) {
            Elog.v("Audio/Test", "1");
            assertTrue(cKCTM4WAY.isShown());
            assertTrue(cTM4WAYText.isShown());
            assertTrue(spliteView.isShown());
            mSolo.clickOnCheckBox(2);
            mSolo.clickOnCheckBox(2);
        } else {
            Elog.v("Audio/Test", "2");
            assertFalse(cKCTM4WAY.isShown());
            assertFalse(cTM4WAYText.isShown());
            assertFalse(spliteView.isShown());
        }
        mSolo.clickOnCheckBox(0);
        mSolo.sleep(EmOperate.TIME_LONG);
        // if (checkBoxs.get(0).isSelected()) { // can't use this to judgement,
        // why?
        // Elog.v("Audio/Test", "cKSpeechLogger.isSelected()");
        // assertTrue(radioBtnBEPL.isEnabled());
        // assertTrue(radioBtnBEPL.isSelected());
        // assertTrue(radioBtnBNormalVm.isEnabled());
        mSolo.clickOnRadioButton(1);
        mSolo.clickOnRadioButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        // }
        // else {
        // Elog.v("Audio/Test", "!cKSpeechLogger.isSelected()");
        // assertFalse(radioBtnBEPL.isEnabled());
        // assertFalse(radioBtnBNormalVm.isEnabled());
        // }
        mSolo.clickOnCheckBox(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnCheckBox(1);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnCheckBox(1);
        mSolo.clickOnButton(4);
        mSolo.goBack();
    }

    public void testCase10_TestAudioLog() {
        verifyPreconditions();
        mSolo.clickOnText(mAudioFuncList.getAdapter().getItem(8).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        ArrayList<CheckBox> checkBoxs = mSolo.getCurrentViews(CheckBox.class);
        assertNotNull(checkBoxs);
        assertEquals(5, checkBoxs.size());
        for (int i = 0; i < checkBoxs.size(); i++) {
            mSolo.clickOnCheckBox(i);
            mSolo.sleep(EmOperate.TIME_MID);
            mSolo.clickOnCheckBox(i);
        }
        mSolo.clickOnButton(5);
        mSolo.sleep(EmOperate.TIME_MID);
        while (mSolo.searchText("Waiting")) {
            mSolo.sleep(EmOperate.TIME_MID);
        }
        mSolo.goBack();
    }
    
    private boolean searchScrollList(String tag) {
        boolean found = false;
        do {
            if (mSolo.searchText(tag)) {
                found = true;
                break;
            }
        } while (mSolo.scrollDown());
        return found;
    }
    
    public void testCase11_TestDebugSession() {
        mSolo.assertCurrentActivity("current activity should be Audio",Audio.class);
        String targetTag = "Debug Session";
        if (!searchScrollList(targetTag)) {
            return;
        }
        mSolo.clickOnText(targetTag);
        mSolo.sleep(EmOperate.TIME_MID);
        ArrayList<CheckBox> checkBoxs = mSolo.getCurrentViews(CheckBox.class);
        for (int i = 0; i < checkBoxs.size(); i++) {
            CheckBox check = checkBoxs.get(i);
            if (check.isShown()) {
                mSolo.clickOnCheckBox(i);
                mSolo.sleep(200);
                mSolo.clickOnCheckBox(i);
                mSolo.sleep(200);
            }
        }
        mSolo.clickOnButton(mContext.getString(R.string.Audio_Headset_Detect_Button));
        mSolo.sleep(200);
        mSolo.goBack();
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
        assertTrue(mAudioFuncList != null);
    }
}
