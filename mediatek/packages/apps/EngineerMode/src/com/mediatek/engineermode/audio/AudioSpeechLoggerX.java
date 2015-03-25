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

package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;

import java.io.IOException;

public class AudioSpeechLoggerX extends Activity {
    public static final String TAG = "EM/Audio";
    public static final String ENGINEER_MODE_PREFERENCE = "engineermode_audiolog_preferences";
    public static final String EPL_STATUS = "epl_status";

    private static final int DATA_SIZE = 1444;
    private static final int VM_LOG_POS = 1440;

    private static final int SET_SPEECH_VM_ENABLE = 0x60;
    private static final int SET_DUMP_SPEECH_DEBUG_INFO = 0x61;
    private static final int SET_DUMP_AP_SPEECH_EPL = 0xA0;
    private static final int GET_DUMP_AP_SPEECH_EPL = 0xA1;
    private static final int DIALOG_GET_DATA_ERROR = 0;
    private static final int DIALOG_ID_NO_SDCARD = 1;
    private static final int DIALOG_ID_SDCARD_BUSY = 2;
    private static final int CONSTANT_256 = 256;
    private static final int CONSTANT_0XFF = 0xFF;

    private CheckBox mCbSpeechLogger;
    private CheckBox mCbEplDebug;
    private CheckBox mCbVoipLogger;
    private CheckBox mCbCtm4Way;
    private RadioButton mRbEpl;
    private RadioButton mRbNormalVm;

    private byte mData[];
    private int mVmLogState = 0;
    private boolean mForRefresh = false; // Sloved radiobutton can not checked

    private final CheckBox.OnCheckedChangeListener mCheckedListener =
        new CheckBox.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
                SharedPreferences preferences =
                        getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_WORLD_READABLE);
                Editor edit = preferences.edit();
                if (buttonView.equals(mCbSpeechLogger)) {
                    onClickSpeechLogger(edit, checked);
                } else if (buttonView.equals(mCbCtm4Way)) {
                    if (checked) {
                        mData[VM_LOG_POS] |= 0x02;
                        mVmLogState |= 0x02;
                        Elog.d(TAG, "E mVmLogState " + mVmLogState);
                    } else {
                        mData[VM_LOG_POS] &= (~0x02);
                        mVmLogState &= (~0x02);
                        Elog.d(TAG, "D mVmLogState " + mVmLogState);
                    }
                    int index = AudioSystem.setEmParameter(mData, DATA_SIZE);
                    if (index != 0) {
                        Elog.i(TAG, "set CTM4WAY parameter failed");
                        Toast.makeText(AudioSpeechLoggerX.this,
                                R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                    }
                } else if (buttonView.equals(mCbEplDebug)) {
                    if (checked) {
                        Elog.d(TAG, "mCKEPLDebug checked");
                        int ret = AudioSystem.setAudioCommand(SET_DUMP_AP_SPEECH_EPL, 1);
                    } else {
                        Elog.d(TAG, "mCKEPLDebug Unchecked");
                        int ret = AudioSystem.setAudioCommand(SET_DUMP_AP_SPEECH_EPL, 0);
                    }
                } else if (buttonView.equals(mCbVoipLogger)) {
                    if (checked) {
                        Elog.d(TAG, "mCbVoipLogger checked");
                        setVOIP(1);
                    } else {
                        Elog.d(TAG, "mCbVoipLogger Unchecked");
                        setVOIP(0);
                    }
                } else if (buttonView.equals(mRbEpl)) {
                    if (checked) {
                        Elog.d(TAG, "mCKBEPL checked");
                        int ret = AudioSystem.setAudioCommand(SET_SPEECH_VM_ENABLE, 1);
                        AudioSystem.getEmParameter(mData, DATA_SIZE);
                        if (ret == -1) {
                            Elog.i(TAG, "set mCKBEPL parameter failed");
                            Toast.makeText(AudioSpeechLoggerX.this,
                                    R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                        }
                        edit.putInt(EPL_STATUS, 1);
                        edit.commit();
                    } else {
                        Elog.d(TAG, "mCKBEPL unchecked");
                    }
                } else if (buttonView.equals(mRbNormalVm)) {
                    if (checked) {
                        Elog.d(TAG, "mCKBNormalVm checked");
                        if (mForRefresh) {
                            mForRefresh = false;
                        } else {
                            Elog.d(TAG, "mCKBNormalVm checked ok");
                            int ret = AudioSystem.setAudioCommand(SET_SPEECH_VM_ENABLE, 0);
                            AudioSystem.getEmParameter(mData, DATA_SIZE);
                            if (ret == -1) {
                                Elog.i(TAG, "set mCKBNormalVm parameter failed");
                                Toast.makeText(AudioSpeechLoggerX.this,
                                        R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                            }
                            edit.putInt(EPL_STATUS, 0);
                            edit.commit();
                        }
                    } else {
                        Elog.d(TAG, "mCKBNormalVm unchecked");
                    }
                }
            }
        };

    private void onClickSpeechLogger(Editor edit, boolean checked) {
        if (checked) {
            Elog.d(TAG, "mCbSpeechLogger checked");
            if (!checkSDCardIsAvaliable()) {
                Elog.d(TAG, "mCbSpeechLogger checked 111");
                mCbSpeechLogger.setChecked(false);
                mRbEpl.setEnabled(false);
                mRbNormalVm.setEnabled(false);
                return;
            }
            mRbEpl.setEnabled(true);
            mRbNormalVm.setEnabled(true);
            mForRefresh = true;
            mRbNormalVm.setChecked(true);
            mRbEpl.setChecked(true);
            mData[VM_LOG_POS] |= 0x01;
            int index = AudioSystem.setEmParameter(mData, DATA_SIZE);
            if (index != 0) {
                Elog.i(TAG, "set mAutoVM parameter failed");
                Toast.makeText(this, R.string.set_failed_tip, Toast.LENGTH_LONG).show();
            }
        } else {
            Elog.d(TAG, "mCbSpeechLogger unchecked");
            if (mRbEpl.isChecked()) {
                mRbEpl.setChecked(false);
            }
            if (mRbNormalVm.isChecked()) {
                mRbNormalVm.setChecked(false);
            }

            mRbEpl.setEnabled(false);
            mRbNormalVm.setEnabled(false);
            AudioSystem.getEmParameter(mData, DATA_SIZE);
            edit.putInt(EPL_STATUS, 0);
            edit.commit();
            mData[VM_LOG_POS] &= (~0x01);
            int index = AudioSystem.setEmParameter(mData, DATA_SIZE);
            if (index != 0) {
                Elog.i(TAG, "set mAutoVM parameter failed");
                Toast.makeText(this, R.string.set_failed_tip, Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_speechloggerx);

        mCbSpeechLogger = (CheckBox) findViewById(R.id.Audio_SpeechLogger_Enable);
        mCbEplDebug = (CheckBox) findViewById(R.id.Audio_EPLDebug_Enable);
        mCbVoipLogger = (CheckBox) findViewById(R.id.Audio_VOIPLogger_Enable);
        mCbCtm4Way = (CheckBox) findViewById(R.id.Audio_CTM4WAYLogger_Enable);
        mRbEpl = (RadioButton) findViewById(R.id.Audio_SpeechLogger_EPL);
        mRbNormalVm = (RadioButton) findViewById(R.id.Audio_SpeechLogger_Normalvm);
        Button dumpSpeechInfo = (Button) findViewById(R.id.Dump_Speech_DbgInfo);
        View spliteView = (View) this.findViewById(R.id.Audio_View1);
        TextView ctm4WayText = (TextView) findViewById(R.id.Audio_CTM4WAYLogger_EnableText);

        if (!ChipSupport.isFeatureSupported(ChipSupport.MTK_TTY_SUPPORT)) {
            mCbCtm4Way.setVisibility(View.GONE);
            ctm4WayText.setVisibility(View.GONE);
            spliteView.setVisibility(View.GONE);
        }

        checkStatus();

        mCbSpeechLogger.setOnCheckedChangeListener(mCheckedListener);
        mCbEplDebug.setOnCheckedChangeListener(mCheckedListener);
        mCbVoipLogger.setOnCheckedChangeListener(mCheckedListener);
        mCbCtm4Way.setOnCheckedChangeListener(mCheckedListener);
        mRbEpl.setOnCheckedChangeListener(mCheckedListener);
        mRbNormalVm.setOnCheckedChangeListener(mCheckedListener);
        dumpSpeechInfo.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                Elog.d(TAG, "On Click mDumpSpeechInfo button.");
                int ret = AudioSystem.setAudioCommand(SET_DUMP_SPEECH_DEBUG_INFO, 1);
                if (ret == -1) {
                    Elog.i(TAG, "set mDumpSpeechInfo parameter failed");
                    Toast.makeText(AudioSpeechLoggerX.this,
                            R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(AudioSpeechLoggerX.this,
                            R.string.set_success_tip, Toast.LENGTH_LONG).show();
                }
            }
        });
    }

    private void checkStatus() {
        final SharedPreferences preferences =
                getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_WORLD_READABLE);
        final int eplStatus = preferences.getInt(EPL_STATUS, 1);

        mData = new byte[DATA_SIZE];
        int ret = AudioSystem.getEmParameter(mData, DATA_SIZE);
        if (ret != 0) {
            showDialog(DIALOG_GET_DATA_ERROR);
            Elog.i(TAG, "Audio_SpeechLogger GetEMParameter return value is : " + ret);
        }

        mVmLogState = shortToInt(mData[VM_LOG_POS], mData[VM_LOG_POS + 1]);
        Elog.i(TAG, "Audio_SpeechLogger GetEMParameter return value is : " + mVmLogState);

        if ((mVmLogState & 0x01) == 0) {
            mCbSpeechLogger.setChecked(false);
            mRbEpl.setEnabled(false);
            mRbNormalVm.setEnabled(false);
            mRbEpl.setChecked(false);
            mRbNormalVm.setChecked(false);
        } else {
            mCbSpeechLogger.setChecked(true);
            mRbEpl.setEnabled(true);
            mRbNormalVm.setEnabled(true);
            if (eplStatus == 1) {
                mRbEpl.setChecked(true);
            } else {
                mRbNormalVm.setChecked(true);
            }
        }

        if ((mVmLogState & 0x02) == 0) {
            mCbCtm4Way.setChecked(false);
        } else {
            mCbCtm4Way.setChecked(true);
        }

        if (getVOIP() == 0) {
            mCbVoipLogger.setChecked(false);
        } else {
            mCbVoipLogger.setChecked(true);
        }

        int epl = AudioSystem.getAudioCommand(GET_DUMP_AP_SPEECH_EPL);
        Elog.i(TAG, "Get EPL setting: " + epl);
        if (AudioSystem.getAudioCommand(GET_DUMP_AP_SPEECH_EPL) == 1) {
            mCbEplDebug.setChecked(true);
        } else {
            mCbEplDebug.setChecked(false);
        }
    }

    private int getVOIP() {
        final String[] cmdx =
            { "/system/bin/sh", "-c",
                "cat /data/data/com.mediatek.engineermode/sharefile/audio_voip" }; // file
        try {
            if (ShellExe.execCommand(cmdx) != 0) {
                return 0;
            }
        } catch (IOException e) {
            Elog.e(TAG, e.toString());
            return 0;
        }
        return Integer.valueOf(ShellExe.getOutput());
    }

    private boolean setVOIP(int n) {
        final String[] cmd =
            { "/system/bin/sh", "-c",
                "mkdir /data/data/com.mediatek.engineermode/sharefile" }; // file
        try {
            ShellExe.execCommand(cmd);
        } catch (IOException e) {
            Elog.e(TAG, e.toString());
            return false;
        }

        final String[] cmdx =
            {
                "/system/bin/sh",
                "-c",
                "echo "
                    + n
                    + " > /data/data/com.mediatek.engineermode/sharefile/audio_voip " }; // file

        try {
            if (ShellExe.execCommand(cmdx) != 0) {
                return false;
            }
        } catch (IOException e) {
            Elog.e(TAG, e.toString());
            return false;
        }
        return true;
    }

    private Boolean checkSDCardIsAvaliable() {
        final String state = Environment.getExternalStorageState();
        Elog.i(TAG, "Environment.getExternalStorageState() is : " + state);
        if (state.equals(Environment.MEDIA_REMOVED)) {
            showDialog(DIALOG_ID_NO_SDCARD);
            return false;
        }

        if (state.equals(Environment.MEDIA_SHARED)) {
            showDialog(DIALOG_ID_SDCARD_BUSY);
            return false;
        }
        return true;
    }

    private int shortToInt(byte low, byte high) {
        int temp = CONSTANT_0XFF & (high + CONSTANT_256);
        int highByte = CONSTANT_256 * temp;
        int lowByte = CONSTANT_0XFF & (low + CONSTANT_256);
        return highByte + lowByte;
    }

    @Override
    public Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_GET_DATA_ERROR:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.get_data_error_title)
                    .setMessage(R.string.get_data_error_msg)
                    .setPositiveButton(android.R.string.ok,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    AudioSpeechLoggerX.this.finish();
                                }
                            }).create();
        case DIALOG_ID_NO_SDCARD:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.no_sdcard_title)
                    .setMessage(R.string.no_sdcard_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        case DIALOG_ID_SDCARD_BUSY:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.sdcard_busy_title)
                    .setMessage(R.string.sdcard_busy_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        default:
            return super.onCreateDialog(id);
        }
    }
}
