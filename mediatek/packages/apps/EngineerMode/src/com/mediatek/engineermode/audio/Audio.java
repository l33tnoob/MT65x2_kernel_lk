/*
 *  Copyright Statement:
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
import android.content.Intent;
import android.media.AudioSystem;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/** The Audio activity which is a entry of Audio debug functions. */
public class Audio extends Activity implements OnItemClickListener {
    /**
     * . CURRENT_MODE is in order to distinguish the headset, normal and loud
     * speaker mode
     */
    public static final String CURRENT_MODE = "CurrentMode";
    public static final String ENHANCE_MODE = "is_enhance";
    public static final String AUDIO_VERSION_COMMAND = "GET_AUDIO_VOLUME_VERSION";
    public static final String AUDIO_VERSION_1 = "GET_AUDIO_VOLUME_VERSION=1";
    /** Log Tag. */
    public static final String TAG = "EM/Audio";
    /** Used for start NORMAL MODE activity. */
    private static final int NORMAL_MODE = 0;
    /** Used for start HEADSET MODE activity. */
    private static final int HEADSET_MODE = 1;
    /** Used for start LOUDSPEAKER MODE activity. */
    private static final int LOUDSPEAKER_MODE = 2;
    /** Used for start HEADSET LOUDSPEAKER MODE activity. */
    private static final int HEADSET_LOUDSPEAKER_MODE = 3;
    /** Used for start SPEECH ENHANCE activity. */
    private static final int SPEECH_ENHANCE = 4;
    /** Used for start DEBUG INFO activity. */
    private static final int DEBUG_INFO = 5;
    private static final int DEBUG_SESSION = 6;
    /** Used for start SPEECH LOGGER activity. */
    private static final int SPEECH_LOGGER = 7;
    /** Used for start AUDIO LOGGER activity. */
    private static final int AUDIO_LOGGER = 8;
    private String mVersion;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio);

        ListView mAudioFunctionList = (ListView) findViewById(R.id.ListView_Audio);
        String[] strArr = getResources()
                .getStringArray(R.array.audio_functions);
        ArrayList<String> list = new ArrayList<String>();
        for (String str : strArr) {
            list.add(str);
        }
        mVersion = AudioSystem.getParameters(AUDIO_VERSION_COMMAND);
        Elog.d(TAG, mVersion);
        if (!AUDIO_VERSION_1.equals(mVersion)) {
            list.remove(HEADSET_LOUDSPEAKER_MODE);
        }
        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, list);
        mAudioFunctionList.setAdapter(adapter);
        mAudioFunctionList.setOnItemClickListener(this);
    }

    /**
     * Click List view items to start different activity.
     * 
     * @param arg0
     *            : View adapter.
     * @param arg1
     *            : Selected view.
     * @param arg2
     *            : Selected view's position.
     * @param arg3
     *            : Selected view's id
     */
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {

        final Intent intent = new Intent();
        if (AUDIO_VERSION_1.equals(mVersion)) {
            // New chip: normal mode, headset mode, loudspeaker mode and headset_loudspeaker mode
            if (arg2 <= HEADSET_LOUDSPEAKER_MODE) {
                intent.setClass(this, AudioModeSetting.class);
                intent.putExtra(CURRENT_MODE, arg2);
                intent.putExtra(ENHANCE_MODE, true);
                startActivity(intent);
                return;
            }
        } else {
            // Normal mode, headset mode and loudspeaker mode
            if (arg2 <= LOUDSPEAKER_MODE) {
                intent.setClass(this, AudioModeSetting.class);
                intent.putExtra(CURRENT_MODE, arg2);
                intent.putExtra(ENHANCE_MODE, false);
                startActivity(intent);
                return;
            } else {
                // Tricky. Shift the index because we have removed a list entry in onCreate()
                arg2++;
            }
        }
        switch (arg2) {
        case SPEECH_ENHANCE:
            intent.setClass(this, AudioSpeechEnhancement.class);
            this.startActivity(intent);
            break;
        case DEBUG_INFO:
            intent.setClass(this, AudioDebugInfo.class);
            this.startActivity(intent);
            break;
        case DEBUG_SESSION:
            intent.setClass(this, AudioDebugSession.class);
            this.startActivity(intent);
            break;
        case SPEECH_LOGGER:
            intent.setClass(this, AudioSpeechLoggerX.class);
            this.startActivity(intent);
            break;
        // to make user mode can dump audio data
        case AUDIO_LOGGER:
            intent.setClass(this, AudioAudioLogger.class);
            this.startActivity(intent);
            break;
        default:
            break;
        }
    }

}
