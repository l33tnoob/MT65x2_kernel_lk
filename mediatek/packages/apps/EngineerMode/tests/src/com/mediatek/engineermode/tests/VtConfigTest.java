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
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.test.ActivityUnitTestCase;

import com.mediatek.engineermode.videotelephone.Configuration;
import com.mediatek.engineermode.videotelephone.VideoTelephony;
import com.mediatek.engineermode.videotelephone.WorkingMode;

public class VtConfigTest extends ActivityUnitTestCase<Configuration> {

    private static final String AUDIO_CHANNEL_ADAPTATION =
        "audio_channel_adaptation";
    private static final String VIDEO_CHANNEL_ADAPTATION =
        "video_channel_adaptation";
    private static final String VIDEO_CHANNEL_REV =
        "video_channel_reverse_data_type";
    private static final String MULTIPLE_LEVEL = "multiplex_level";
    private static final String VIDEO_CODEC = "video_codec_preference";
    private static final String USE_WNSRP = "use_wnsrp";
    private static final String TERMINAL_TYPE = "terminal_type";
    private Context mContext;
    private Instrumentation mInstrumentation;
    private Intent mIntent;
    private PreferenceActivity mActivity;
    private PreferenceScreen preferenceScreen;

    public VtConfigTest() {
        super(Configuration.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mIntent = new Intent(Intent.ACTION_MAIN);
        mIntent.setComponent(new ComponentName(mContext, Configuration.class
            .getName()));
        mActivity = startActivity(mIntent, null, null);
        preferenceScreen = mActivity.getPreferenceScreen();
    }

    public void test01_Precondition() {
        testConditions();
    }

    public void test02_TestListPreferences() {
        SharedPreferences preferences =
            mActivity.getSharedPreferences(
                VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
        assertNotNull(preferences);
        ListPreference audioChanneAdaptPref =
            (ListPreference) mActivity.findPreference(AUDIO_CHANNEL_ADAPTATION);
        assertNotNull(audioChanneAdaptPref);
        audioChanneAdaptPref.getOnPreferenceChangeListener()
            .onPreferenceChange(audioChanneAdaptPref, "3");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_AUDIO_CHANNEL_ADAPT, "0"), "3");
        
        ListPreference videoChanneAdaptlPref =
            (ListPreference) mActivity.findPreference(VIDEO_CHANNEL_ADAPTATION);
        assertNotNull(videoChanneAdaptlPref);
        videoChanneAdaptlPref.getOnPreferenceChangeListener()
            .onPreferenceChange(videoChanneAdaptlPref, "3");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_VIDEO_CHANNEL_ADAPT, "0"), "3");
        
        ListPreference videoChanneRevPref =
            (ListPreference) mActivity.findPreference(VIDEO_CHANNEL_REV);
        assertNotNull(videoChanneRevPref);
        videoChanneRevPref.getOnPreferenceChangeListener()
            .onPreferenceChange(videoChanneRevPref, "1");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_VIDEO_CHANNEL_REVERSE, "0"), "1"); 
       
        ListPreference multiPref =
            (ListPreference) mActivity.findPreference(MULTIPLE_LEVEL);
        assertNotNull(multiPref);
        multiPref.getOnPreferenceChangeListener()
            .onPreferenceChange(multiPref, "3");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_MULTIPLEX_LEVEL, "0"), "3"); 
       
        ListPreference videoCdecPref =
            (ListPreference) mActivity
                .findPreference(VIDEO_CODEC);
        assertNotNull(videoCdecPref);
        videoCdecPref.getOnPreferenceChangeListener()
            .onPreferenceChange(videoCdecPref, "1");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_VIDEO_CODEC_PREFERENCE, "0"), "1"); 
       
        ListPreference useWnsrpPref =
            (ListPreference) mActivity.findPreference(USE_WNSRP);
        assertNotNull(useWnsrpPref);
        useWnsrpPref.getOnPreferenceChangeListener()
            .onPreferenceChange(useWnsrpPref, "1");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_USE_WNSRP, "0"), "1"); 
        
        ListPreference terminalTypePref =
            (ListPreference) mActivity.findPreference(TERMINAL_TYPE);
        assertNotNull(terminalTypePref);
        terminalTypePref.getOnPreferenceChangeListener()
            .onPreferenceChange(terminalTypePref, "1");
        assertEquals(preferences.getString(
            VideoTelephony.CONFIG_TERMINAL_TYPE, "0"), "1"); 
    }

    public void testConditions() {
        assertNotNull(mInstrumentation);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(preferenceScreen);
    }

}