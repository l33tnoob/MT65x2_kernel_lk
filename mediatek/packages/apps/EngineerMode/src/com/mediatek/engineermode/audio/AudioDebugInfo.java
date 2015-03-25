/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.media.AudioSystem;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/** Set the audio debug info parameters. */
public class AudioDebugInfo extends Activity
        implements
        OnItemSelectedListener,
        OnClickListener {

    /** parameters value edit text. */
    private EditText mDebugValue;
    /** set button. */
    private Button mBtnSet;
    /** The parameters spinner index that has been select. */
    private int mSpinnerIndex;
    /** volume speech size. */
    private static final int VOLUME_SPEECH_SIZE = 310;
    /** audio data size. */
    private static final int DATA_SIZE = 1444;
    /** audio record preference's key. */
    private static final String AUDIO_RECORD_PREFER = "audio_record";
    /** date byte array. */
    private byte[] mData;
    /** Get data error dialog id. */
    private static final int DIALOG_ID_GET_DATA_ERROR = 1;
    /** set audio debug info parameter value success dialog id. */
    private static final int DIALOG_ID_SET_SUCCESS = 2;
    /** set audio debug info parameter value failed dialog id. */
    private static final int DIALOG_ID_SET_ERROR = 3;
    /** magic number 256. */
    private static final int MAGIC_NUMBER_256 = 256;
    /** magic number 65535. */
    private static final int MAGIC_NUMBER_65535 = 65535;
    private static final int SPINNER_COUNT = 16;
    private static final int LONGEST_NUM_LEN = 5;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_debuginfo);

        Spinner paramSpinner =
            (Spinner) findViewById(R.id.Audio_DebugInfo_Spinner);
        mDebugValue = (EditText) findViewById(R.id.Audio_DebugInfo_EditText);
        mBtnSet = (Button) findViewById(R.id.Audio_DebugInfo_Button);
        mBtnSet.setOnClickListener(this);

        // create ArrayAdapter for Spinner
        final ArrayAdapter<String> mSpinnerAdatper =
            new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mSpinnerAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        Resources resources = getResources();
        for (int i = 0; i < SPINNER_COUNT; i++) {
            mSpinnerAdatper.add(resources.getString(R.string.paramter) + i);
        }
        paramSpinner.setAdapter(mSpinnerAdatper);
        paramSpinner.setOnItemSelectedListener(this);

        // get the current data
        mData = new byte[DATA_SIZE];
        for (int n = 0; n < DATA_SIZE; n++) {
            mData[n] = 0;
        }

        int ret = AudioSystem.getEmParameter(mData, DATA_SIZE);
        if (ret != 0) {
            showDialog(DIALOG_ID_GET_DATA_ERROR);
            Xlog.i(Audio.TAG,
                "Audio_DebugInfo GetEMParameter return value is : " + ret);
        }

        final SharedPreferences preferences =
            this.getSharedPreferences(AUDIO_RECORD_PREFER,
                android.content.Context.MODE_PRIVATE);
        mSpinnerIndex = preferences.getInt("NUM", 0);

        paramSpinner.setSelection(mSpinnerIndex);
        final int initValue =
            mData[VOLUME_SPEECH_SIZE + mSpinnerIndex * 2 + 1]
                * MAGIC_NUMBER_256
                + mData[VOLUME_SPEECH_SIZE + mSpinnerIndex * 2];
        mDebugValue.setText(String.valueOf(initValue));

    }

    /**
     * on spinner selected.
     * 
     * @param arg0
     *            : adapter view on witch select.
     * @param arg1
     *            : selected view.
     * @param arg2
     *            : selected view's position.
     * @param arg3
     *            : selected view's id
     * */
    public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
        long arg3) {
        mSpinnerIndex = arg2;

        final int initValue =
            mData[VOLUME_SPEECH_SIZE + mSpinnerIndex * 2 + 1]
                * MAGIC_NUMBER_256
                + mData[VOLUME_SPEECH_SIZE + mSpinnerIndex * 2];
        mDebugValue.setText(String.valueOf(initValue));

        final SharedPreferences preferences =
            this.getSharedPreferences(AUDIO_RECORD_PREFER,
                android.content.Context.MODE_PRIVATE);

        SharedPreferences.Editor editor = preferences.edit();
        editor.putInt("NUM", mSpinnerIndex);
        editor.commit();

    }

    /**
     * nothing selected.
     * 
     * @param arg0
     *            : adapter view on witch select.
     */
    public void onNothingSelected(AdapterView<?> arg0) {
        Xlog.i(Audio.TAG, "do noting...");
    }

    /**
     * click the set button.
     * 
     * @param arg0
     *            : click which view
     */
    public void onClick(View arg0) {
        if (arg0.getId() == mBtnSet.getId()) {
            if (null == mDebugValue.getText().toString()) {
                Toast
                    .makeText(this, R.string.input_null_tip, Toast.LENGTH_LONG)
                    .show();
                return;
            }
            if (LONGEST_NUM_LEN < mDebugValue.getText().toString().length()
                || 0 == mDebugValue.getText().toString().length()) {
                Toast.makeText(this, R.string.input_length_error,
                    Toast.LENGTH_LONG).show();
                return;
            }

            final long inputValue =
                Long.valueOf(mDebugValue.getText().toString());
            if (inputValue > MAGIC_NUMBER_65535) {
                Toast.makeText(this, R.string.input_length_error,
                    Toast.LENGTH_LONG).show();
                return;
            }
            int high = (int) (inputValue / MAGIC_NUMBER_256);
            int low = (int) (inputValue % MAGIC_NUMBER_256);
            mData[VOLUME_SPEECH_SIZE + mSpinnerIndex * 2] = (byte) low;
            mData[VOLUME_SPEECH_SIZE + mSpinnerIndex * 2 + 1] = (byte) high;

            int result = AudioSystem.setEmParameter(mData, DATA_SIZE);
            if (0 == result) {
                showDialog(DIALOG_ID_SET_SUCCESS);
            } else {
                showDialog(DIALOG_ID_SET_ERROR);
                Xlog.i(Audio.TAG, "SetEMParameter return value is : " + result);
            }
        }

    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_ID_GET_DATA_ERROR:
            return new AlertDialog.Builder(AudioDebugInfo.this).setTitle(
                R.string.get_data_error_title).setMessage(
                R.string.get_data_error_msg).setPositiveButton(
                android.R.string.ok, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        AudioDebugInfo.this.finish();
                    }
                }).setNegativeButton(android.R.string.cancel, null).create();
        case DIALOG_ID_SET_SUCCESS:
            return new AlertDialog.Builder(AudioDebugInfo.this).setTitle(
                R.string.set_success_title).setMessage(
                R.string.set_debuginfo_success).setPositiveButton(
                android.R.string.ok, null).create();
        case DIALOG_ID_SET_ERROR:
            return new AlertDialog.Builder(AudioDebugInfo.this).setTitle(
                R.string.set_error_title).setMessage(
                R.string.set_debuginfo_failed).setPositiveButton(
                android.R.string.ok, null).create();
        default:
            return null;
        }
    }

}
