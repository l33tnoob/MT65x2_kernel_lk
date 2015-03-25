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
import android.content.res.Resources;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class VTAutoAnswer extends Activity {
    private static final String TAG = "EM/VTAutoAnswer";

    private EditText mVtAutoAnswerTime;
    private Button mButEableAutoAnswer;

    private boolean mHasAutoAnswer = true;// false: disable true: enable
    private Resources mRes;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vt_auto_answer);

        mRes = getResources();

        mVtAutoAnswerTime = (EditText) findViewById(R.id.VTAutoAnswer_Edit);
        mButEableAutoAnswer = (Button) findViewById(R.id.VTAutoAnswer_Set_Btn);
        getStatus();
        mButEableAutoAnswer.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                final SharedPreferences preferences =
                    getSharedPreferences(
                        VideoTelephony.ENGINEER_MODE_PREFERENCE,
                        WorkingMode.MODE_WORLD_READABLE);
                Editor edit = preferences.edit();

                if (mHasAutoAnswer) {
                    mButEableAutoAnswer
                        .setText(mRes.getString(R.string.Disable));
                    mVtAutoAnswerTime.setEnabled(false);

                    String time = "1000";
                    final String inputTime =
                        mVtAutoAnswerTime.getText().toString().trim();
                    if (null != inputTime && inputTime.length() > 0) {
                        time = inputTime;
                    }

                    edit.putBoolean(VideoTelephony.AUTO_ANSWER, true);
                    edit.putString(VideoTelephony.AUTO_ANSWER_TIME, time);
                    edit.commit();
                } else {
                    mButEableAutoAnswer.setText(mRes.getString(R.string.Enable));
                    mVtAutoAnswerTime.setEnabled(true);

                    edit.putBoolean(VideoTelephony.AUTO_ANSWER, false);
                    edit.commit();
                }
                mHasAutoAnswer = !mHasAutoAnswer;
            }
        });
    }

    private void getStatus() {
        boolean status = false;
        SharedPreferences preferences =
            getSharedPreferences(VideoTelephony.ENGINEER_MODE_PREFERENCE,
                WorkingMode.MODE_WORLD_READABLE);

        status = preferences.getBoolean(VideoTelephony.AUTO_ANSWER, false);
        Xlog.v(TAG, "status = " + status);
        if (status) {
            mButEableAutoAnswer.setText(mRes.getString(R.string.Disable));
            mVtAutoAnswerTime.setEnabled(false);
            mHasAutoAnswer = !mHasAutoAnswer;
        }
        String time =
            preferences.getString(VideoTelephony.AUTO_ANSWER_TIME, "");
        mVtAutoAnswerTime.setText(time);
        Xlog.v(TAG, "time = " + time);
    }
}
