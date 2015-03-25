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
import android.widget.Button;
import android.widget.Spinner;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.videotelephone.PeerAudioRecorder;
import com.mediatek.engineermode.videotelephone.VideoTelephony;
import com.mediatek.engineermode.videotelephone.WorkingMode;

public class VtPeerAudioRecTest extends ActivityUnitTestCase<PeerAudioRecorder> {

    private Context mContext;
    private Instrumentation mInstrumentation;
    private Intent mIntent;
    private Activity mActivity;
    private SharedPreferences preferences;

    public VtPeerAudioRecTest() {
        super(PeerAudioRecorder.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mIntent = new Intent(Intent.ACTION_MAIN);
        mIntent.setComponent(new ComponentName(mContext,
            PeerAudioRecorder.class.getName()));
        mActivity = startActivity(mIntent, null, null);
        preferences =
            mActivity.getSharedPreferences(
                VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
    }

    public void test01_Precondition() {
        testConditions();
    }

    public void test02_TestSPinner() {
        testConditions();
        final Spinner audioSuptFormat =
            (Spinner) mActivity
                .findViewById(R.id.audio_mal_supported_format_spinner);
        assertNotNull(audioSuptFormat);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                audioSuptFormat.setSelection(2);
            }
        });
    }

    public void test03_TestButton() {
        testConditions();
        final Spinner audioSuptFormat =
            (Spinner) mActivity
                .findViewById(R.id.audio_mal_supported_format_spinner);
        assertNotNull(audioSuptFormat);
        audioSuptFormat.getOnItemSelectedListener().onItemSelected(
            null, audioSuptFormat, 2,
            audioSuptFormat.getItemIdAtPosition(2));
        final Button confirmButton =
            (Button) mActivity.findViewById(R.id.peer_audio_recorder_btn);
        assertNotNull(confirmButton);
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                confirmButton.performClick();
            }
        });
        assertEquals(preferences.getString(
            VideoTelephony.PEER_AUDIO_RECODER_FORMAT, "0"), "2");
    }

    public void testConditions() {
        assertNotNull(mInstrumentation);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mIntent);
        assertNotNull(preferences);
    }
}