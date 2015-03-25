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
import android.content.SharedPreferences;
import android.test.ActivityUnitTestCase;
import android.view.View;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.mediatek.engineermode.videotelephone.VideoTelephony;
import com.mediatek.engineermode.videotelephone.WorkingMode;
import com.mediatek.engineermode.R;

public class VtWorkModeTest extends ActivityUnitTestCase<WorkingMode> {

    private static final String TAG = "EM/VideoTelephony";
    private Context mContext;
    private Instrumentation mInstrumentation;
    private Intent mIntent;
    private Activity mActivity;
    private SharedPreferences preferences;

    public VtWorkModeTest() {
        super(WorkingMode.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mIntent = new Intent(Intent.ACTION_MAIN);
        mIntent.setComponent(new ComponentName(mContext, WorkingMode.class
            .getName()));
        mActivity = startActivity(mIntent, null, null);
        preferences =
            mActivity.getSharedPreferences(
                VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
    }

    public void test01_Precondition() {
        testConditions();
    }

    public void test02_TestItems() {
        testConditions();
        final RadioButton workingModeNormalRadio =
            (RadioButton) mActivity.findViewById(R.id.working_mode_normal);
        assertNotNull(workingModeNormalRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                workingModeNormalRadio.performClick();
            }
        });
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "1"),
            "0");
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE_DETAIL,
            "1"), "2");

        final RadioButton mediaLoopRadio =
            (RadioButton) mActivity
                .findViewById(R.id.working_mode_media_loopback);
        assertNotNull(mediaLoopRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                mediaLoopRadio.performClick();
            }
        });

        final RadioButton mediaLoopStackRadio =
            (RadioButton) mActivity.findViewById(R.id.media_loopback_stack);
        assertNotNull(mediaLoopStackRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                mediaLoopStackRadio.performClick();
            }
        });
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "0"),
            "1");
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE_DETAIL,
            "1"), "0");
        
        final RadioButton mediaLoopTransceiverRadio =
            (RadioButton) mActivity.findViewById(R.id.media_loopback_transceiver);
        assertNotNull(mediaLoopTransceiverRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                mediaLoopTransceiverRadio.performClick();
            }
        });
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "0"),
            "1");
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE_DETAIL,
            "0"), "1");
        
        final RadioButton workingModeNetworkLoopRadio =
            (RadioButton) mActivity.findViewById(R.id.working_mode_network_loopback);
        assertNotNull(workingModeNetworkLoopRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                workingModeNetworkLoopRadio.performClick();
            }
        });
        
        final RadioButton networkLoopStackRadio =
            (RadioButton) mActivity.findViewById(R.id.network_loopback_stack);
        assertNotNull(networkLoopStackRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                networkLoopStackRadio.performClick();
            }
        });
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "0"),
            "2");
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE_DETAIL,
            "1"), "0");
        
        final RadioButton networkLoopServiceRadio =
            (RadioButton) mActivity.findViewById(R.id.network_loopback_service);
        assertNotNull(networkLoopServiceRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                networkLoopServiceRadio.performClick();
            }
        });
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "0"),
            "2");
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE_DETAIL,
            "0"), "1");
        
        final RadioButton workingModeTestFileRadio =
            (RadioButton) mActivity.findViewById(R.id.working_mode_test_file);
        assertNotNull(workingModeTestFileRadio);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                workingModeTestFileRadio.performClick();
            }
        });
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "0"),
            "3");
    }

    public void testConditions() {
        assertNotNull(mInstrumentation);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(preferences);
    }
}