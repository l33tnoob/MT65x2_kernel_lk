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
import android.bluetooth.BluetoothAdapter;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.graphics.Camera;
import android.media.audiofx.BassBoost.Settings;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.ListView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.touchscreen.TouchScreenList;
import com.mediatek.engineermode.touchscreen.TouchScreenSettings;
import com.mediatek.engineermode.touchscreen.TsHandWriting;
import com.mediatek.engineermode.touchscreen.TsMultiTouch;
import com.mediatek.engineermode.touchscreen.TsRateReport;
import com.mediatek.engineermode.touchscreen.TsVerifyList;
import com.mediatek.engineermode.touchscreen.TsVerifyLine;
import com.mediatek.engineermode.touchscreen.TsVerifyShakingPoint;



public class TouchScreenTest extends ActivityInstrumentationTestCase2<TouchScreenList>{
	
    private static final String TAG = "TouchScreenList";
    private static final String EM_BAKC_FAIL_MSG = "Back to TouchScreenList fail";
    private Solo mSolo = null;
    private Activity mActivity = null;
    private Context mContext = null;
    private Instrumentation mInst = null;
    private ContentResolver mCr = null;
    private ListView mListView = null;

	public TouchScreenTest() {
		super(TouchScreenList.class);
	}

	protected void setUp() throws Exception {
		super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        mListView = (ListView) mActivity.findViewById(R.id.ListView_TouchScreen);
	}
	
    public void test01_Precondition() {
        assertNotNull(mInst);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
        assertNotNull(mListView);
        mSolo.sleep(EmOperate.TIME_LONG);
    }

    public void test02_HandWriting() {
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_HandWriting));
        mSolo.waitForActivity(TsHandWriting.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
    }
    public void test03_RateReport() {
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_RateReport));
        mSolo.waitForActivity(TsRateReport.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
    }
    public void test04_MutilTouch() {
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_MultiTouch));
        mSolo.waitForActivity(TsMultiTouch.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
    }
    public void test05_Settings() {
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_Settings));
        mSolo.waitForActivity(TouchScreenSettings.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        EmOperate.backKey(mInst);
        EmOperate.backKey(mInst);
    }
    public void test06_VerificationList() {
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_Verification));
        mSolo.waitForActivity(TsVerifyList.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_VerificationPoint));
        mSolo.waitForActivity(TsVerifyShakingPoint.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
        
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_VerificationLine));
        mSolo.waitForActivity(TsVerifyLine.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
        
        mSolo.clickOnText(mActivity.getString(R.string.TouchScreen_VerificationShaking));
        mSolo.waitForActivity(TsVerifyShakingPoint.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
        
        EmOperate.backKey(mInst);
    }
}
