/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.videotelephone;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class VideoTelephony extends PreferenceActivity
        implements
        Preference.OnPreferenceChangeListener {
    public static final String TAG = "EM/VideoTelephony";
    public static final String ENGINEER_MODE_PREFERENCE =
        "engineermode_vt_preferences";
    public static final String WORKING_MODE = "working_mode";
    public static final String WORKING_MODE_DETAIL = "working_mode_detail";
    public static final String CONFIG_AUDIO_CHANNEL_ADAPT =
        "config_audio_channel_adapt";
    public static final String CONFIG_VIDEO_CHANNEL_ADAPT =
        "config_video_channel_adapt";
    public static final String CONFIG_VIDEO_CHANNEL_REVERSE =
        "config_video_channel_reverse";
    public static final String CONFIG_MULTIPLEX_LEVEL =
        "config_multiplex_level";
    public static final String CONFIG_VIDEO_CODEC_PREFERENCE =
        "config_video_codec_preference";
    public static final String CONFIG_USE_WNSRP = "config_use_wnsrp";
    public static final String CONFIG_TERMINAL_TYPE = "config_terminal_type";
    public static final String AUTO_ANSWER = "auto_answer";
    public static final String AUTO_ANSWER_TIME = "auto_answer_time";
    public static final String PEER_AUDIO_RECODER_SERVICE =
        "peer_audio_recoder_service";
    public static final String PEER_AUDIO_RECODER_FORMAT =
        "peer_audio_recoder_format";
    public static final String PEER_VIDEO_RECODER_SERVICE =
        "peer_video_recoder_service";
    public static final String PEER_VIDEO_RECODER_FORMAT =
        "peer_video_recoder_format";
    public static final String DEBUG_MESSAGE = "debug_message";
    public static final String H223_RAW_DATA = "h223_raw_data";
    public static final String LOG_TO_FILE = "log_to_file";
    public static final String H263_ONLY = "h263_only";
    public static final String GET_RAW_DATA = "get_raw_data";
    public static final String SDCARD_FLAG = "sdcard_flag";
    public static final String SET_WORKING_MODE = "set_working_mode";
    public static final String MY3G324M_CONFIG = "my3g324m_configuration";
    public static final String LOG_FILTER = "log_filter";
    public static final String PEER_AUDIO_RECODER = "peer_audio_recoder";
    public static final String PEER_VIDEO_RECODER = "peer_video_recoder";
    protected static final String[] LOG_FILTER_TAGKEY =
        { "log_filter_tag_0", "log_filter_tag_1", "log_filter_tag_2",
            "log_filter_tag_3", /*"log_filter_tag_4", "log_filter_tag_5",
            "log_filter_tag_6"*/ };
    protected static final String[] LOG_FILTER_TAGVALUE =
        { "log_filter_tag_0_value", "log_filter_tag_1_value",
            "log_filter_tag_2_value", "log_filter_tag_3_value",
            /*"log_filter_tag_4_value", "log_filter_tag_5_value",
            "log_filter_tag_6_value"*/ };

    private Preference mPeerAudioRecorder;
    private Preference mPeerVideoRecorder;
    private CheckBoxPreference mDebugPref;
    private CheckBoxPreference mRawDataPref;
    private CheckBoxPreference mLogToFilePref;
    private CheckBoxPreference mH263OnlyPref;
    private CheckBoxPreference mGetRawDataPref;
    private boolean mSDCardFlag = false;
    private boolean mH223RawDataFlag = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.video_telephony);

        PreferenceScreen preferenceScreen = this.getPreferenceScreen();

        mDebugPref = (CheckBoxPreference) findPreference(DEBUG_MESSAGE);
        // mRawDataPref = (CheckBoxPreference) findPreference(H223_RAW_DATA);
        mLogToFilePref = (CheckBoxPreference) findPreference(LOG_TO_FILE);
        mH263OnlyPref = (CheckBoxPreference) findPreference(H263_ONLY);
        mGetRawDataPref = (CheckBoxPreference) findPreference(GET_RAW_DATA);

        mDebugPref.setOnPreferenceChangeListener(this);
        // mRawDataPref.setOnPreferenceChangeListener(this);
        mLogToFilePref.setOnPreferenceChangeListener(this);
        mH263OnlyPref.setOnPreferenceChangeListener(this);
        mGetRawDataPref.setOnPreferenceChangeListener(this);

        initStatus();

        /*if (mSDCardFlag) {
            mRawDataPref = new CheckBoxPreference(this);
            mRawDataPref.setKey(H223_RAW_DATA);
            mRawDataPref.setTitle(R.string.h223_raw_data);
            mRawDataPref.setPersistent(false);
            mRawDataPref.setOnPreferenceChangeListener(this);
            mRawDataPref.setChecked(mH223RawDataFlag);
            preferenceScreen.addPreference(mRawDataPref);

            mPeerAudioRecorder = new Preference(this);
            mPeerAudioRecorder.setKey(PEER_AUDIO_RECODER);
            mPeerAudioRecorder.setTitle(R.string.peer_audio_recorder);
            mPeerAudioRecorder.setPersistent(false);
            mPeerAudioRecorder.setOnPreferenceClickListener(preferenceScreen
                .getOnPreferenceClickListener());
            preferenceScreen.addPreference(mPeerAudioRecorder);

            mPeerVideoRecorder = new Preference(this);
            mPeerVideoRecorder.setKey(PEER_VIDEO_RECODER);
            mPeerVideoRecorder.setTitle(R.string.peer_video_recorder);
            mPeerVideoRecorder.setPersistent(false);
            mPeerVideoRecorder.setOnPreferenceClickListener(preferenceScreen
                .getOnPreferenceClickListener());
            preferenceScreen.addPreference(mPeerVideoRecorder);

        }*/
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
        Preference preference) {
        super.onPreferenceTreeClick(preferenceScreen, preference);

        if (preference.getKey().equals(SET_WORKING_MODE)) {

            startActivity(new Intent(this, WorkingMode.class));

        } else if (preference.getKey().equals(MY3G324M_CONFIG)) {

            startActivity(new Intent(this, Configuration.class));

        } else if (preference.getKey().equals(AUTO_ANSWER)) {

            startActivity(new Intent(this, VTAutoAnswer.class));

        } else if (preference.getKey().equals(PEER_AUDIO_RECODER)) {
            Xlog.v(TAG, "PEER_AUDIO_RECODER");

            startActivity(new Intent(this, PeerAudioRecorder.class));

        } else if (preference.getKey().equals(PEER_VIDEO_RECODER)) {
            Xlog.v(TAG, "PEER_VIDEO_RECODER");
            startActivity(new Intent(this, PeerVideoRecorder.class));

        } else if (preference.getKey().equals(LOG_FILTER)) {
            Xlog.v(TAG, "log_filter");
            startActivity(new Intent(this, LogFilter.class));

        }
        Xlog.v(TAG, "onPreferenceTreeClick");
        return true;
    }

    /**
     * click the preference action
     * 
     * @param preference
     *            : which pref to be clicked
     * @param newValue
     *            : selected value
     * @return whether change preference success
     */
    public boolean onPreferenceChange(Preference preference, Object newValue) {

        if (null == newValue || null == preference) {
            return false;
        }

        final String key = preference.getKey();
        Xlog.v(TAG, "enter onPreferenceChange key is:" + key);
        if (null == key) {
            Xlog.v(TAG, "The preference key is null.");
            return false;
        }

        SharedPreferences preferences =
            getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
        Editor edit = preferences.edit();
        String strPutKey = "";
        if (key.endsWith(DEBUG_MESSAGE)) {
            strPutKey = DEBUG_MESSAGE;
        } else if (key.endsWith(H223_RAW_DATA)) {
            strPutKey = H223_RAW_DATA;
        } else if (key.endsWith(LOG_TO_FILE)) {
            strPutKey = LOG_TO_FILE;
        } else if (key.endsWith(H263_ONLY)) {
            strPutKey = H263_ONLY;
        } else if (key.endsWith(GET_RAW_DATA)) {
            strPutKey = GET_RAW_DATA;
        }
        Xlog.v(TAG, "The newValue is:" + newValue);
        edit.putBoolean(strPutKey, (Boolean) newValue);
        edit.commit();

        return true;
    }

    private void initStatus() {
        SharedPreferences preferences =
            getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);
        mDebugPref.setChecked(preferences.getBoolean(DEBUG_MESSAGE, false));

        mH223RawDataFlag = preferences.getBoolean(H223_RAW_DATA, false);
        if (null != mRawDataPref) {
            mRawDataPref.setChecked(mH223RawDataFlag);
        }

        mLogToFilePref.setChecked(preferences.getBoolean(LOG_TO_FILE, false));
        mH263OnlyPref.setChecked(preferences.getBoolean(H263_ONLY, false));
        mGetRawDataPref.setChecked(preferences.getBoolean(GET_RAW_DATA, false));
        mSDCardFlag = preferences.getBoolean(SDCARD_FLAG, false);
        Xlog.v(TAG, "mSDCardFlag = " + mSDCardFlag);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater menuInflater = this.getMenuInflater();
        menuInflater.inflate(R.menu.vt_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
        case R.id.vt_set_to_default:
            boolean result = setToDefault();
            if (result) {
                initStatus();
                Toast.makeText(this,
                    getResources().getString(R.string.set_default_success),
                    Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this,
                    getResources().getString(R.string.set_default_failed),
                    Toast.LENGTH_SHORT).show();
            }
            break;
        default:
            break;
        }
        return true;
    }

    private boolean setToDefault() {

        SharedPreferences preferences =
            getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);

        Editor edit = preferences.edit();
        edit.clear();
        edit.putString(VideoTelephony.WORKING_MODE, "0");
        edit.putString(VideoTelephony.WORKING_MODE_DETAIL, "0");
        edit.putString(VideoTelephony.CONFIG_AUDIO_CHANNEL_ADAPT, "0");
        edit.putString(VideoTelephony.CONFIG_VIDEO_CHANNEL_ADAPT, "0");
        edit.putString(VideoTelephony.CONFIG_VIDEO_CHANNEL_REVERSE, "0");
        edit.putString(VideoTelephony.CONFIG_MULTIPLEX_LEVEL, "4");
        edit.putString(VideoTelephony.CONFIG_VIDEO_CODEC_PREFERENCE, "1");
        edit.putString(VideoTelephony.CONFIG_USE_WNSRP, "2");
        edit.putString(VideoTelephony.CONFIG_TERMINAL_TYPE, "1");
        edit.putBoolean(VideoTelephony.AUTO_ANSWER, false);
        edit.putString(VideoTelephony.AUTO_ANSWER_TIME, "0");
        edit.putBoolean(VideoTelephony.PEER_AUDIO_RECODER_SERVICE, false);
        edit.putString(VideoTelephony.PEER_AUDIO_RECODER_FORMAT, "0");
        edit.putBoolean(VideoTelephony.PEER_VIDEO_RECODER_SERVICE, false);
        edit.putString(VideoTelephony.PEER_VIDEO_RECODER_FORMAT, "0");
        edit.putBoolean(VideoTelephony.DEBUG_MESSAGE, false);
        edit.putBoolean(VideoTelephony.H223_RAW_DATA, false);
        edit.putBoolean(VideoTelephony.LOG_TO_FILE, false);
        edit.putBoolean(VideoTelephony.H263_ONLY, false);
        edit.putBoolean(VideoTelephony.GET_RAW_DATA, false);
        edit.putBoolean(VideoTelephony.SDCARD_FLAG, false);

        return edit.commit();
    }

}
