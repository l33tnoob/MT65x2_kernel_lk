/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.matv;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;

import com.mediatek.atv.AtvService;
import com.mediatek.xlog.Xlog;

/** Set Matv parameters. */
public class Matv extends PreferenceActivity implements
		Preference.OnPreferenceChangeListener {

	private static final String SHARED_PREFERENE_NAME = "em_matv";
	private static final String KEY_FRAME_RATE = "frame_rate";
	private static final String KEY_NR_LEVEL = "nr_level";
	private static final String KEY_AUDIO_PATH = "audio_path";
	private static final String SUM_FRAME_RATE = "Frame rate ";
	private static final String SUM_NR_LEVEL = "NR Level ";
	private static final String DEFAULT_SUMMARY = "x";

	private ListPreference mListPrefFrameRate;
	private ListPreference mListPrefHrLevel;
	private ListPreference mListPreAudioPath;
	private static final String TAG = "EM/matv";

	private AtvService mAtvService = new AtvService(null);

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.layout.matv);

		mListPrefFrameRate = (ListPreference) findPreference(KEY_FRAME_RATE);
		mListPrefHrLevel = (ListPreference) findPreference(KEY_NR_LEVEL);
		mListPreAudioPath = (ListPreference) findPreference(KEY_AUDIO_PATH);
		mListPrefFrameRate.setOnPreferenceChangeListener(this);
		mListPrefHrLevel.setOnPreferenceChangeListener(this);
		mListPreAudioPath.setOnPreferenceChangeListener(this);
		mListPrefFrameRate.setSummary(SUM_FRAME_RATE
				+ getPreferenceScreen().getSharedPreferences().getString(
						KEY_FRAME_RATE, DEFAULT_SUMMARY));
		mListPrefHrLevel.setSummary(SUM_NR_LEVEL
				+ "0x"
				+ getPreferenceScreen().getSharedPreferences().getString(
						KEY_NR_LEVEL, DEFAULT_SUMMARY));
		try {
			if (mListPreAudioPath.getValue() != null) {
				mListPreAudioPath.setSummary(getResources().getStringArray(
						R.array.matv_audio_path_entries)[Integer
						.parseInt(mListPreAudioPath.getValue())]);
			}
		} catch (NumberFormatException e) {
			Xlog.e(TAG, e.getMessage());
		}
		// mListPreAudioPath.setValueIndex(0);
		// putAudioPathToPrefer(0);
	}

	public boolean onPreferenceChange(Preference preference, Object objValue) {

		final String key = preference.getKey();

		if (KEY_FRAME_RATE.equals(key)) {
			try {
				final int value = Integer.parseInt((String) objValue);
				Xlog.v(TAG, KEY_FRAME_RATE + "value is " + value);
				mAtvService.setChipDep(180, value);
				mListPrefFrameRate.setSummary(SUM_FRAME_RATE
						+ (String) objValue);
			} catch (NumberFormatException e) {
				Xlog.e(TAG, "set frame rate exception. ");
			}

		} else if (KEY_NR_LEVEL.equals(key)) {
			try {
				final int value = Integer.parseInt((String) objValue, 16);
				Xlog.v(TAG, KEY_NR_LEVEL + "value is. " + value);
				mAtvService.setChipDep(185, value);
				mListPrefHrLevel.setSummary(SUM_NR_LEVEL
						+ String.format("0x%X", value));
			} catch (NumberFormatException e) {
				Xlog.e(TAG, "set NR level exception. ");
			}
		} else {

			final int value = Integer.parseInt((String) objValue);
			Xlog.v(TAG, KEY_AUDIO_PATH + "value is. " + value);
			mListPreAudioPath.setSummary(getResources().getStringArray(
					R.array.matv_audio_path_entries)[value]);
			putAudioPathToPrefer(value);
		}
		return true;
	}

	private void putAudioPathToPrefer(int value) {
		SharedPreferences preferences = getSharedPreferences(
				SHARED_PREFERENE_NAME, Matv.MODE_WORLD_READABLE);
		Editor edit = preferences.edit();
		edit.putInt(KEY_AUDIO_PATH, value);
		edit.commit();
	}

}
