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

import android.app.Instrumentation;
import android.content.Context;
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.test.ActivityInstrumentationTestCase2;
import com.jayway.android.robotium.solo.Solo;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.devicemgr.DeviceMgr;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

public class DeviceMgrTest extends ActivityInstrumentationTestCase2<DeviceMgr> {

    private static final String KEY_SMS_AUTO_REG = "sms_auto_reg";
    private static final String TAG = "EMTest/DeviceMgr";
    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private PreferenceActivity mActivity;
    private ListPreference mListPreferSmsAutoReg;

    public DeviceMgrTest() {
        super("com.mediatek.engineermode", DeviceMgr.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
        mListPreferSmsAutoReg =
            (ListPreference) mActivity.findPreference(KEY_SMS_AUTO_REG);
    }

    public void testCase01_ActivityDeviceMgr() {
        verifyPreconditions();
    }

    public void testCase02_CheckInitStatus() {
        verifyPreconditions();
        Method getValueMethod =
            getPrivateMethod(DeviceMgr.class, "getSavedCTA");
        assertTrue(getValueMethod != null);
        try {
            if (getValueMethod.invoke(mActivity, null).equals(1)) {
                assertEquals(mListPreferSmsAutoReg.getSummary(), "Enabled");
                assertEquals(mListPreferSmsAutoReg.getValue(), "1");
            } else {
                assertEquals(mListPreferSmsAutoReg.getSummary(), "Disabled");
                assertEquals(mListPreferSmsAutoReg.getValue(), "0");
            }
        } catch (IllegalAccessException e) {
            Elog.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Elog.e(TAG, e.toString());
        }
    }

    public void testCase03_ListPrefClick() {
        verifyPreconditions();
        Method getValueMethod =
            getPrivateMethod(DeviceMgr.class, "getSavedCTA");
        assertTrue(getValueMethod != null);
        try {
            if (getValueMethod.invoke(mActivity, null).equals(1)) {
                assertEquals(mListPreferSmsAutoReg.getSummary(), "Enabled");
                assertEquals(mListPreferSmsAutoReg.getValue(), "1");
                EmOperate.runOnUiThread(mIns, mActivity, new Runnable() {

                    public void run() {
                        mListPreferSmsAutoReg.getOnPreferenceChangeListener()
                            .onPreferenceChange(mListPreferSmsAutoReg, "0");
                    }
                });
                EmOperate.runOnUiThread(mIns, mActivity, new Runnable() {

                    public void run() {
                        mListPreferSmsAutoReg.getOnPreferenceChangeListener()
                            .onPreferenceChange(mListPreferSmsAutoReg, "1");
                    }
                });
                assertEquals(mListPreferSmsAutoReg.getSummary(), "Enabled");
                assertEquals(mListPreferSmsAutoReg.getValue(), "1");
            } else {
                assertEquals(mListPreferSmsAutoReg.getSummary(), "Disabled");
                assertEquals(mListPreferSmsAutoReg.getValue(), "0");
                EmOperate.runOnUiThread(mIns, mActivity, new Runnable() {

                    public void run() {
                        mListPreferSmsAutoReg.getOnPreferenceChangeListener()
                            .onPreferenceChange(mListPreferSmsAutoReg, "1");
                    }
                });
                EmOperate.runOnUiThread(mIns, mActivity, new Runnable() {

                    public void run() {
                        mListPreferSmsAutoReg.getOnPreferenceChangeListener()
                            .onPreferenceChange(mListPreferSmsAutoReg, "0");
                    }
                });
                assertEquals(mListPreferSmsAutoReg.getSummary(), "Disabled");
                assertEquals(mListPreferSmsAutoReg.getValue(), "0");
            }
        } catch (IllegalAccessException e) {
            Elog.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Elog.e(TAG, e.toString());
        }
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
        assertTrue(mListPreferSmsAutoReg != null);
    }

    private Method getPrivateMethod(Class currentClass, String methodName) {
        try {
            Method methodField =
                currentClass.getDeclaredMethod(methodName, null);
            assertTrue(methodField != null);
            methodField.setAccessible(true);
            return methodField;
        } catch (NoSuchMethodException e) {
            Elog.e(TAG, e.toString());
            return null;
        }
    }
}
