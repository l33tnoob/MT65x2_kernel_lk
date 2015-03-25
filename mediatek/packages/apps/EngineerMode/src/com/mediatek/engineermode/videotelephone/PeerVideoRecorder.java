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

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Spinner;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class PeerVideoRecorder extends Activity {

    private Spinner mSupportFormatSpinner;
    private CheckBox mCheckBoxVideoRecService;
    private String mFormat = "0";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.peer_video_recorder);

        Button confirmButton =
            (Button) findViewById(R.id.peer_video_recorder_btn);
        confirmButton.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                SharedPreferences preferences =
                    getSharedPreferences(
                        VideoTelephony.ENGINEER_MODE_PREFERENCE,
                        WorkingMode.MODE_WORLD_READABLE);
                Editor edit = preferences.edit();
                edit.putBoolean(VideoTelephony.PEER_VIDEO_RECODER_SERVICE,
                    mCheckBoxVideoRecService.isChecked());
                edit.putString(VideoTelephony.PEER_VIDEO_RECODER_FORMAT,
                    mFormat);
                edit.commit();
                PeerVideoRecorder.this.finish();
            }
        });

        ArrayAdapter<String> adapter =
            new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        adapter
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        adapter.add(getResources().getString(R.string.no_record));
        adapter.add(getResources().getString(R.string.three_gp));

        mSupportFormatSpinner =
            (Spinner) findViewById(R.id.video_mal_supported_format_spinner);
        mSupportFormatSpinner.setAdapter(adapter);
        mSupportFormatSpinner
            .setOnItemSelectedListener(new OnItemSelectedListener() {

                public void onItemSelected(AdapterView<?> parent, View view,
                    int position, long id) {
                    mFormat = String.valueOf(position);
                }

                public void onNothingSelected(AdapterView<?> parent) {
                    Xlog.e(VideoTelephony.TAG, "noting selected.");
                }
            });

        mCheckBoxVideoRecService =
            (CheckBox) findViewById(R.id.peer_video_recorder_service_checkbox);
        initStatus();
    }

    private void initStatus() {
        try {
            SharedPreferences preferences =
                getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                    WorkingMode.MODE_WORLD_READABLE);
            mCheckBoxVideoRecService.setChecked(preferences.getBoolean(
                VideoTelephony.PEER_VIDEO_RECODER_SERVICE, false));

            final String format =
                preferences.getString(VideoTelephony.PEER_VIDEO_RECODER_FORMAT,
                    "0");

            mSupportFormatSpinner.setSelection(Integer.valueOf(format)
                .intValue());

        } catch (ClassCastException e) {
            Xlog.e(VideoTelephony.TAG, "init status failed");
        }
    }

}
