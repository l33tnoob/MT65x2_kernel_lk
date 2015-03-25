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
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.Button;
import android.widget.ListView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.camera.Camera;
import com.mediatek.engineermode.camera.CameraPreview;

public class CameraTest extends ActivityInstrumentationTestCase2<Camera> {

    private static final String AF_MODE = "AFMode";
    private static final String AF_STEP = "AFStep";
    private static final String RAW_CAPTURE_MODE = "RawCaptureMode";
    private static final String RAW_TYPE = "RawType";
    private static final String ANTI_FLICKER = "AntiFlicker";
    private static final String ISO_STR = "ISO";
    private static final String AUTO_STR = "AUTO";
    public static final int TIME_SUPER_LONG = 10000;
    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private Activity mActivity;
    private ListView mCameraSetList;

    public CameraTest() {
        super(Camera.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
        mCameraSetList = (ListView) mActivity.findViewById(R.id.ListView_Camera);
    }

    public void testCase01_ActivityAudio() {
        verifyPreconditions();
    }

    public void testCase02_TestListview() {
        verifyPreconditions();
        int count = mCameraSetList.getAdapter().getCount();
        assertEquals(count, 7);
        for (int i = 0; i < count; i++) {
            mSolo.clickOnText(mCameraSetList.getAdapter().getItem(i).toString());
            mSolo.sleep(EmOperate.TIME_MID);
            mSolo.goBack();
        }
    }

    public void testCase03_SetAfMode() {
        verifyPreconditions();
        mSolo.clickOnText(mCameraSetList.getAdapter().getItem(0).toString());
        mSolo.clickOnButton(0);
        mSolo.clickOnText(mCameraSetList.getAdapter().getItem(5).toString());
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);
        mSolo.goBack();
        // mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void testCase04_TestCaptureMode0() {
        verifyPreconditions();
        Intent intent = new Intent();
        intent.setClass(mContext, CameraPreview.class);
        intent.putExtra(AF_MODE, 0);
        intent.putExtra(AF_STEP, 0);
        intent.putExtra(RAW_CAPTURE_MODE, 1);
        intent.putExtra(RAW_TYPE, 0);
        intent.putExtra(ANTI_FLICKER, "50");
        intent.putExtra(ISO_STR, AUTO_STR);
        mActivity.startActivity(intent);
      //  mSolo.sleep(EmOperate.TIME_LONG);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);
        mSolo.goBack();
        // mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void testCase05_TestCaptureMode1() {
        verifyPreconditions();
        Intent intent = new Intent();
        intent.setClass(mContext, CameraPreview.class);
        intent.putExtra(AF_MODE, 1);
        intent.putExtra(AF_STEP, 2);
        intent.putExtra(RAW_CAPTURE_MODE, 2);
        intent.putExtra(RAW_TYPE, 1);
        intent.putExtra(ANTI_FLICKER, "50");
        intent.putExtra(ISO_STR, "100");
        mActivity.startActivity(intent);
       // mSolo.sleep(EmOperate.TIME_LONG);
        CameraPreview.sCanBack = false;
        mSolo.clickOnButton(0);
        Elog.v("Test/Camera", "Test : clickOnButton");
        while (!CameraPreview.sCanBack) {
            Elog.v("Test/Camera", "Test : !CameraPreview.sCanBack");
            mSolo.sleep(20);
        }
        Elog.v("Test/Camera", "Test : goBack");
        mSolo.goBack();
        // mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void testCase06_TestCaptureMode2() {
        verifyPreconditions();
        Intent intent = new Intent();
        intent.setClass(mContext, CameraPreview.class);
        intent.putExtra(AF_MODE, 2);
        intent.putExtra(AF_STEP, 0);
        intent.putExtra(RAW_CAPTURE_MODE, 3);
        intent.putExtra(RAW_TYPE, 1);
        intent.putExtra(ANTI_FLICKER, "60");
        intent.putExtra(ISO_STR, "200");
        mActivity.startActivity(intent);
      //  mSolo.sleep(EmOperate.TIME_MID);
        // CameraPreview.sCanBack = false;
        // mSolo.clickOnButton(0);
        // while (!CameraPreview.sCanBack) {
        // mSolo.sleep(20);
        // }
        mSolo.goBack();
        // mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void testCase07_TestCaptureMode3() {
        verifyPreconditions();
        Intent intent = new Intent();
        intent.setClass(mContext, CameraPreview.class);
        intent.putExtra(AF_MODE, 3);
        intent.putExtra(AF_STEP, 0);
        intent.putExtra(RAW_CAPTURE_MODE, 3);
        intent.putExtra(RAW_TYPE, 0);
        intent.putExtra(ANTI_FLICKER, "50");
        intent.putExtra(ISO_STR, "300");
        mActivity.startActivity(intent);
       // mSolo.sleep(EmOperate.TIME_MID);
        // CameraPreview.sCanBack = false;
        // mSolo.clickOnButton(0);
        // while (!CameraPreview.sCanBack) {
        // mSolo.sleep(20);
        // }
        mSolo.goBack();
        // mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void testCase08_TestCaptureMode4() {
        verifyPreconditions();
        Intent intent = new Intent();
        intent.setClass(mContext, CameraPreview.class);
        intent.putExtra(AF_MODE, 4);
        intent.putExtra(AF_STEP, 0);
        intent.putExtra(RAW_CAPTURE_MODE, 3);
        intent.putExtra(RAW_TYPE, 1);
        intent.putExtra(ANTI_FLICKER, "60");
        intent.putExtra(ISO_STR, "150");
        mActivity.startActivity(intent);
      //  mSolo.sleep(EmOperate.TIME_LONG);
        CameraPreview.sCanBack = false;
        mSolo.clickOnButton(0);
        while (!CameraPreview.sCanBack) {
            mSolo.sleep(20);
        }
        mSolo.goBack();
        // mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void testCase09_TestCaptureMode5() {
        verifyPreconditions();
        Intent intent = new Intent();
        intent.setClass(mContext, CameraPreview.class);
        intent.putExtra(AF_MODE, 5);
        intent.putExtra(AF_STEP, 0);
        intent.putExtra(RAW_CAPTURE_MODE, 1);
        intent.putExtra(RAW_TYPE, 1);
        intent.putExtra(ANTI_FLICKER, "60");
        intent.putExtra(ISO_STR, "1600");
        mActivity.startActivity(intent);
      //  mSolo.sleep(EmOperate.TIME_LONG);
        CameraPreview.sCanBack = false;
        mSolo.clickOnButton(0);
        while (!CameraPreview.sCanBack) {
            mSolo.sleep(20);
        }
        mSolo.goBack();
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
        assertTrue(mCameraSetList != null);
    }
}
