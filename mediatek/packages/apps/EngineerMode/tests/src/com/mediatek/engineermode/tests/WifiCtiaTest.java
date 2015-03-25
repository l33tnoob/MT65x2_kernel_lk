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
import android.net.wifi.WifiManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
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
import com.mediatek.engineermode.wifi.WifiTestSetting;

public class WifiCtiaTest extends
        ActivityInstrumentationTestCase2<WifiTestSetting> {

    private static final String TAG = "EMTest/wifitest";
    private static final int ITEM_COUNT = 3;
    private static final int SLEEP_TIME = 1000;
    private Solo mSolo = null;
    private PreferenceActivity mActivity = null;
    private Context mContext = null;
    private Instrumentation mInst = null;

    public WifiCtiaTest() {
        super("com.mediatek.engineermode", WifiTestSetting.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
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
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test02_CtiaTest() {
        mSolo.clickOnText("Setting");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton("GET");
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.clickOnButton("SET");
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.clickOnButton("Cancel");
        WifiManager wifiManager = (WifiManager) mContext
                .getSystemService(Context.WIFI_SERVICE);
        assertNotNull(wifiManager);
        wifiManager.setWifiEnabled(true);
        int i = 0;
        int state = wifiManager.getWifiState();
        while (WifiManager.WIFI_STATE_ENABLED != wifiManager.getWifiState()) {
            if (i > 10) {
                break;
            }
            EmOperate.waitSomeTime(EmOperate.TIME_LONG);
            i++;
        }
        mSolo.clickOnText("Setting");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton("GET");
        mSolo.sleep(EmOperate.TIME_SHORT);
        mSolo.clickOnButton("SET");
        mSolo.sleep(EmOperate.TIME_SHORT);
        for (i = 0; i < 3; i++) {
            mSolo.clickOnCheckBox(i);
        }
        mSolo.clickOnButton("OK");
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnText("Setting");
        mSolo.sleep(EmOperate.TIME_SHORT);
        for (i = 0; i < 3; i++) {
            mSolo.clickOnCheckBox(i);
        }
        wifiManager.setWifiEnabled(false);
    }
}
