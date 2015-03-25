/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.mediatek.rcse.ipcall;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.H264Config;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.profiles.H264Profile1_2;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.profiles.H264Profile1_3;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.profiles.H264Profile1b;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;

/**
 * Video settings display
 *
 * @author hlxn7157
 */
public class VideoSettings extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    /**
     * Codecs size
     */
    public static final int CODECS_SIZE = 4;

    /**
     * Default codecs
     * 3G -> level 1.B: PAYLOAD=96, profile-level-id=42900b, frame_rate=12, frame_size=QCIF, bit_rate=96k
     * 3G+ -> level 1.2: profile-level-id=42800c, frame_rate=10, frame_size=QVGA, bit_rate=176k
     * 3G+ -> level 1.2: profile-level-id=42800c, frame_rate=10, frame_size=CIF, bit_rate=176k
     * WIFI -> level 1.3:profile-level-id=42800d, frame_rate=15, frame_size=CIF, bit_rate=384k
     */
    public static final MediaCodec[] CODECS = {
        new VideoCodec(H264Config.CODEC_NAME, 99,
                H264Config.CLOCK_RATE, H264Config.CODEC_PARAM_PROFILEID + "="
                        + H264Profile1b.BASELINE_PROFILE_ID + ";"
                        + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=1", 12, 96000,
                H264Config.QCIF_WIDTH, H264Config.QCIF_HEIGHT).getMediaCodec(),
        new VideoCodec(H264Config.CODEC_NAME, 98,
                H264Config.CLOCK_RATE, H264Config.CODEC_PARAM_PROFILEID + "="
                        + H264Profile1_2.BASELINE_PROFILE_ID + ";"
                        + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=1", 15, 384000,
                H264Config.QVGA_WIDTH, H264Config.QVGA_HEIGHT).getMediaCodec(),
        new VideoCodec(H264Config.CODEC_NAME, 97,
                H264Config.CLOCK_RATE, H264Config.CODEC_PARAM_PROFILEID + "="
                        + H264Profile1_2.BASELINE_PROFILE_ID + ";"
                        + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=1", 15, 384000,
                H264Config.CIF_WIDTH, H264Config.CIF_HEIGHT).getMediaCodec(),
        new VideoCodec(H264Config.CODEC_NAME, 96,
                H264Config.CLOCK_RATE, H264Config.CODEC_PARAM_PROFILEID + "="
                        + H264Profile1_3.BASELINE_PROFILE_ID + ";"
                        + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=1", 15, 384000,
                H264Config.CIF_WIDTH, H264Config.CIF_HEIGHT).getMediaCodec()
    };

    /**
     * CheckBoxPreference "managed_by_stack" 
     */
    private CheckBoxPreference managed;

    /**
     * Codecs CheckBoxPreference 
     */
    private CheckBoxPreference codec0;
    private CheckBoxPreference codec1;
    private CheckBoxPreference codec2;
    private CheckBoxPreference codec3;
    private CheckBoxPreference codec4;
    private ListPreference customLevel;
    private ListPreference customSize;
    private ListPreference customFramerate;
    private ListPreference customBitrate;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.video_settings);
        setTitle(R.string.menu_video_settings);

        managed = (CheckBoxPreference)findPreference("managed_by_stack");
        managed.setOnPreferenceChangeListener(this);

        codec0 = (CheckBoxPreference)findPreference("codec0");
        codec0.setEnabled(!managed.isChecked());
        codec0.setOnPreferenceChangeListener(this);
        codec1 = (CheckBoxPreference)findPreference("codec1");
        codec1.setEnabled(!managed.isChecked());
        codec2 = (CheckBoxPreference)findPreference("codec2");
        codec2.setEnabled(!managed.isChecked());
        codec3 = (CheckBoxPreference)findPreference("codec3");
        codec3.setEnabled(!managed.isChecked());
        codec4 = (CheckBoxPreference)findPreference("codec4");
        codec4.setEnabled(!managed.isChecked());
        codec4.setOnPreferenceChangeListener(this);

        customLevel = (ListPreference)findPreference("custom_level");
        customLevel.setEnabled(codec4.isChecked());
        customSize = (ListPreference)findPreference("custom_size");
        customSize.setEnabled(codec4.isChecked());
        customFramerate = (ListPreference)findPreference("custom_framerate");
        customFramerate.setEnabled(codec4.isChecked());
        customBitrate = (ListPreference)findPreference("custom_bitrate");
        customBitrate.setEnabled(codec4.isChecked());
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
        if (preference.getKey().equals("managed_by_stack")) {
            Boolean state = (Boolean)objValue;
            codec0.setEnabled(!state);
            codec1.setEnabled(!state);
            codec2.setEnabled(!state);
            codec3.setEnabled(!state);
            codec4.setEnabled(!state);
            return true;
        } else if (preference.getKey().equals("codec0")) {
            // Codec 0 must always be enabled
            return false;
        } else if (preference.getKey().equals("codec4")) {
            Boolean state = (Boolean)objValue;
            customLevel.setEnabled(state);
            customSize.setEnabled(state);
            customFramerate.setEnabled(state);
            customBitrate.setEnabled(state);
            return true;
        }
        return false;
    }

    /**
     * isCodecsManagedByStack
     *
     * @param context the application context
     * @return value of the preference
     */
    public static boolean isCodecsManagedByStack(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return prefs.getBoolean("managed_by_stack", true);
    }

    /**
     * getCodecsList
     *
     * @param context the application context
     * @return value of the preference
     */
    public static boolean[] getCodecsList(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return new boolean[] { 
            prefs.getBoolean("codec0", false),
            prefs.getBoolean("codec1", false),
            prefs.getBoolean("codec2", false),
            prefs.getBoolean("codec3", false),
            prefs.getBoolean("codec4", false) };
    }

    /**
     * getCustomCodec
     *
     * @param context the application context
     * @return the custom codec
     */
    public static MediaCodec getCustomCodec(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        int framerate = Integer.parseInt(prefs.getString("custom_framerate","10"));
        int bitrate = 1000 * Integer.parseInt(prefs.getString("custom_bitrate", "128"));
        String level = "";
        switch (Integer.parseInt(prefs.getString("custom_level", "0"))) {
            case 0 :
                level = H264Profile1b.BASELINE_PROFILE_ID;
                break;
            case 1 :
                level = H264Profile1_2.BASELINE_PROFILE_ID;
                break;
            case 2 :
                level = H264Profile1_3.BASELINE_PROFILE_ID;
                break;
        }
        int width = 0;
        int height = 0;
        switch (Integer.parseInt(prefs.getString("custom_size", "0"))) {
            case 0 :
                width = 176;
                height = 144;
                break;
            case 1 :
                width = 320;
                height = 240;
                break;
            case 2 :
                width = 352;
                height = 288;
                break;
            case 3 :
                width = 0;
                height = 0;
                break;
        }

        return new VideoCodec(H264Config.CODEC_NAME,
                100,
                H264Config.CLOCK_RATE,
                H264Config.CODEC_PARAM_PROFILEID + "=" + level + ";" + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=1",
                framerate,
                bitrate,
                width,
                height).getMediaCodec();
    }

}

