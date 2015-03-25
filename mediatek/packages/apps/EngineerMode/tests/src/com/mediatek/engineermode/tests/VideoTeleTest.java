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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.test.ActivityUnitTestCase;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.videotelephone.VideoTelephony;
import com.mediatek.engineermode.videotelephone.WorkingMode;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class VideoTeleTest extends ActivityUnitTestCase<VideoTelephony> {

    private static final String TAG = "EM/VideoTelephony";
    private Context mContext;
    private Instrumentation mInstrumentation;
    private Intent mIntent;
    private PreferenceActivity mActivity;
    private SharedPreferences preferences;
    private PreferenceScreen preferenceScreen;

    public VideoTeleTest() {
        super(VideoTelephony.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mIntent = new Intent(Intent.ACTION_MAIN);
        mIntent.setComponent(new ComponentName(mContext, VideoTelephony.class
            .getName()));
        mActivity = startActivity(mIntent, null, null);
        preferenceScreen = mActivity.getPreferenceScreen();
        preferences =
            mActivity.getSharedPreferences(
                VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
    }

    public void test01_Precondition() {
        testConditions();
    }

    public void test02_DebugMsgTest() {
        testConditions();
        final CheckBoxPreference debugPref =
            (CheckBoxPreference) mActivity
                .findPreference(VideoTelephony.DEBUG_MESSAGE);
        assertNotNull(debugPref);
        debugPref.getOnPreferenceChangeListener().onPreferenceChange(debugPref,
            !debugPref.isChecked());
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                debugPref.setChecked(!debugPref.isChecked());
            }
        });
        assertEquals(preferences
            .getBoolean(VideoTelephony.DEBUG_MESSAGE, false), debugPref
            .isChecked());
        debugPref.getOnPreferenceChangeListener().onPreferenceChange(debugPref,
            !debugPref.isChecked());
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                debugPref.setChecked(!debugPref.isChecked());
            }
        });
        assertEquals(preferences
            .getBoolean(VideoTelephony.DEBUG_MESSAGE, false), debugPref
            .isChecked());
    }

    public void test03_LogToFileTest() {
        testConditions();
        final CheckBoxPreference logToFilePref =
            (CheckBoxPreference) mActivity
                .findPreference(VideoTelephony.LOG_TO_FILE);
        assertNotNull(logToFilePref);
        logToFilePref.getOnPreferenceChangeListener().onPreferenceChange(
            logToFilePref, !logToFilePref.isChecked());
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                logToFilePref.setChecked(!logToFilePref.isChecked());
            }
        });
        assertEquals(preferences.getBoolean(VideoTelephony.LOG_TO_FILE, false),
            logToFilePref.isChecked());
        logToFilePref.getOnPreferenceChangeListener().onPreferenceChange(
            logToFilePref, !logToFilePref.isChecked());
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                logToFilePref.setChecked(!logToFilePref.isChecked());
            }
        });
        assertEquals(preferences.getBoolean(VideoTelephony.LOG_TO_FILE, false),
            logToFilePref.isChecked());
    }

    public void test04_H236OnlyTest() {
        testConditions();
        final CheckBoxPreference h263OnlyPref =
            (CheckBoxPreference) mActivity
                .findPreference(VideoTelephony.H263_ONLY);
        assertNotNull(h263OnlyPref);
        h263OnlyPref.getOnPreferenceChangeListener().onPreferenceChange(
            h263OnlyPref, !h263OnlyPref.isChecked());
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                h263OnlyPref.setChecked(!h263OnlyPref.isChecked());
            }
        });
        assertEquals(preferences.getBoolean(VideoTelephony.H263_ONLY, false),
            h263OnlyPref.isChecked());
        h263OnlyPref.getOnPreferenceChangeListener().onPreferenceChange(
            h263OnlyPref, !h263OnlyPref.isChecked());
        EmOperate.runOnUiThread(mInstrumentation, mActivity, new Runnable() {

            public void run() {
                h263OnlyPref.setChecked(!h263OnlyPref.isChecked());
            }
        });
        assertEquals(preferences.getBoolean(VideoTelephony.H263_ONLY, false),
            h263OnlyPref.isChecked());
    }

    public void test05_TestSetToDefalt() {
        testConditions();
        Method setToDefault =
            getPrivateMethod(VideoTelephony.class, "setToDefault");
        assertTrue(setToDefault != null);
        try {
            assertEquals(setToDefault.invoke(mActivity, null), true);
        } catch (IllegalAccessException e) {
            Elog.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Elog.e(TAG, e.toString());
        }
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE, "1"),
            "0");
        assertEquals(preferences.getString(VideoTelephony.WORKING_MODE_DETAIL,
            "1"), "0");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_AUDIO_CHANNEL_ADAPT, "1"), "0");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_VIDEO_CHANNEL_ADAPT, "1"), "0");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_VIDEO_CHANNEL_REVERSE, "1"), "0");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_MULTIPLEX_LEVEL, "1"), "4");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_VIDEO_CODEC_PREFERENCE, "0"), "1");
        assertEquals(preferences
            .getString(VideoTelephony.CONFIG_USE_WNSRP, "1"), "2");
        assertEquals(preferences
            .getString(VideoTelephony.AUTO_ANSWER_TIME, "1"), "0");
        assertEquals(preferences.getString(VideoTelephony.CONFIG_TERMINAL_TYPE,
            "0"), "1");
        assertEquals(preferences.getString(
            VideoTelephony.PEER_AUDIO_RECODER_FORMAT, "1"), "0");
        assertEquals(preferences.getString(
            VideoTelephony.PEER_VIDEO_RECODER_FORMAT, "1"), "0");
        assertEquals(preferences.getBoolean(VideoTelephony.AUTO_ANSWER, true),
            false);
        assertEquals(preferences.getBoolean(
            VideoTelephony.PEER_AUDIO_RECODER_SERVICE, true), false);
        assertEquals(preferences.getBoolean(
            VideoTelephony.PEER_VIDEO_RECODER_SERVICE, true), false);
        assertEquals(
            preferences.getBoolean(VideoTelephony.DEBUG_MESSAGE, true), false);
        assertEquals(
            preferences.getBoolean(VideoTelephony.H223_RAW_DATA, true), false);
        assertEquals(preferences.getBoolean(VideoTelephony.LOG_TO_FILE, true),
            false);
        assertEquals(preferences.getBoolean(VideoTelephony.H263_ONLY, true),
            false);
        assertEquals(preferences.getBoolean(VideoTelephony.SDCARD_FLAG, true),
            false);
    }

    public void test06_TestWorkMode() {
        testConditions();
        Preference workingMode =
            mActivity.findPreference(VideoTelephony.SET_WORKING_MODE);
        assertNotNull(workingMode);
        mActivity.onPreferenceTreeClick(preferenceScreen, workingMode);
    }

    public void test07_TestConfig() {
        testConditions();
        Preference config =
            mActivity.findPreference(VideoTelephony.MY3G324M_CONFIG);
        assertNotNull(config);
        mActivity.onPreferenceTreeClick(preferenceScreen, config);
    }

    public void test08_TestAutoAnswer() {
        testConditions();
        Preference autoAnswer =
            mActivity.findPreference(VideoTelephony.AUTO_ANSWER);
        assertNotNull(autoAnswer);
        mActivity.onPreferenceTreeClick(preferenceScreen, autoAnswer);
    }

    public void test09_TestLogFilter() {
        testConditions();
        Preference logFilter =
            mActivity.findPreference(VideoTelephony.LOG_FILTER);
        assertNotNull(logFilter);
        mActivity.onPreferenceTreeClick(preferenceScreen, logFilter);
    }

    public void test10_TestSDCardFlag() {
        
        Preference mPeerAudioRecorder = new Preference(mContext);
        mPeerAudioRecorder.setKey(VideoTelephony.PEER_AUDIO_RECODER);
        //mPeerAudioRecorder.setTitle(R.string.peer_audio_recorder);
        mPeerAudioRecorder.setPersistent(false);
        mPeerAudioRecorder.setOnPreferenceClickListener(preferenceScreen
            .getOnPreferenceClickListener());
        assertNotNull(mPeerAudioRecorder);
        preferenceScreen.addPreference(mPeerAudioRecorder);
        mActivity.onPreferenceTreeClick(preferenceScreen, mPeerAudioRecorder);
        
        Preference mPeerVideoRecorder = new Preference(mContext);
        mPeerVideoRecorder.setKey(VideoTelephony.PEER_VIDEO_RECODER);
        //mPeerVideoRecorder.setTitle(R.string.peer_video_recorder);
        mPeerVideoRecorder.setPersistent(false);
        mPeerVideoRecorder.setOnPreferenceClickListener(preferenceScreen
            .getOnPreferenceClickListener());
        assertNotNull(mPeerVideoRecorder);
        preferenceScreen.addPreference(mPeerVideoRecorder);
        mActivity.onPreferenceTreeClick(preferenceScreen, mPeerVideoRecorder);
    }

    public void testConditions() {
        assertNotNull(mInstrumentation);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(preferences);
        assertNotNull(preferenceScreen);
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