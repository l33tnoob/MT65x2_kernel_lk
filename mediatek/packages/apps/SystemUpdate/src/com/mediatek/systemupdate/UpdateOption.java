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

package com.mediatek.systemupdate;

import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.text.TextUtils;

import com.mediatek.systemupdate.Util.PathName;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;

public class UpdateOption extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {

    private static final String KEY_UPDATE_OPTIONS = "update_options";
    private static final String ENTRY_OTA = "OTA update";
    private static final String ENTRY_SD = "SD Card update";
    private static final String ENTRY_OTA_SD = "OTA and SD Card update";
    private ListPreference mListPrefer;
    private static final String TAG = "SystemUpdate/EM";

    /**
     * Called when the activity is first created.
     * 
     * @param savedInstanceState
     *            : the bundle
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.update_options);

        mListPrefer = (ListPreference) findPreference(KEY_UPDATE_OPTIONS);

        mListPrefer.setOnPreferenceChangeListener(this);
        final String type = getUpdateTypeFromEM();
        setPrefSummary(type);
        if (!TextUtils.isEmpty(type)) {
            mListPrefer.setValue(type);
        }

    }

    private void setPrefSummary(String type) {
        String summary = "";
        if (type.equals(Util.UPDATE_TYPES.OTA_UPDATE_ONLY.toString())) {
            summary = ENTRY_OTA;
        } else if (type.equals(Util.UPDATE_TYPES.SDCARD_UPDATE_ONLY.toString())) {
            summary = ENTRY_SD;
        } else if (type.equals(Util.UPDATE_TYPES.OTA_SDCARD_UPDATE.toString())) {
            summary = ENTRY_OTA_SD;
        }
        mListPrefer.setSummary(summary);
    }

    public boolean onPreferenceChange(Preference preference, Object objValue) {

        final String key = preference.getKey();

        if (KEY_UPDATE_OPTIONS.equals(key)) {
            setUpdateTypeFromEM((String) objValue);
            final String type = getUpdateTypeFromEM();
            setPrefSummary(type);
            if (!TextUtils.isEmpty(type)) {
                mListPrefer.setValue(type);
            }
        }
        return false;
    }

    private void setUpdateTypeFromEM(String type) {

        File f = new File(PathName.UPDATE_TYPE_IN_DATA);
        try {
            FileOutputStream outputStream = new FileOutputStream(f);
            outputStream.write(type.getBytes());
            outputStream.getFD().sync();
            outputStream.close();
        } catch (java.io.FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return;
    }

    static String getUpdateTypeFromEM() {

        String type = "";
        File f = new File(PathName.UPDATE_TYPE_IN_DATA);
        if (!f.exists()) {
            Xlog.v(TAG, PathName.UPDATE_TYPE_IN_DATA + "doesn's exist");
            return "";
        }

        try {
            FileReader reader = new FileReader(PathName.UPDATE_TYPE_IN_DATA);
            BufferedReader bfReader = new BufferedReader(reader);

            type = bfReader.readLine();

            Xlog.v(TAG, "Get Type from EM is " + type);

            reader.close();
            bfReader.close();

        } catch (IOException e) {
            e.printStackTrace();
        }

        return type;
    }
}
