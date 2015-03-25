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

/*
 * Copyright (C) 2007 The Android Open Source Project
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
 */

package com.android.stk2;

import android.media.AudioManager;
import android.media.ToneGenerator;

import com.android.internal.telephony.cat.Tone;

import java.util.HashMap;

/**
 * Class that implements a tones player for the SIM toolkit application.
 */
public class TonePlayer {
    private static final HashMap<Tone, Integer> mToneMap = new HashMap<Tone, Integer>();

    static {
        // Map STK tone ids to the system tone ids.
        mToneMap.put(Tone.DIAL, ToneGenerator.TONE_SUP_DIAL);
        mToneMap.put(Tone.BUSY, ToneGenerator.TONE_SUP_BUSY);
        mToneMap.put(Tone.CONGESTION, ToneGenerator.TONE_SUP_CONGESTION);
        mToneMap.put(Tone.RADIO_PATH_ACK, ToneGenerator.TONE_SUP_RADIO_ACK);
        mToneMap.put(Tone.RADIO_PATH_NOT_AVAILABLE, ToneGenerator.TONE_SUP_RADIO_NOTAVAIL);
        mToneMap.put(Tone.ERROR_SPECIAL_INFO, ToneGenerator.TONE_SUP_ERROR);
        mToneMap.put(Tone.CALL_WAITING, ToneGenerator.TONE_SUP_CALL_WAITING);
        mToneMap.put(Tone.RINGING, ToneGenerator.TONE_SUP_RINGTONE);
        mToneMap.put(Tone.GENERAL_BEEP, ToneGenerator.TONE_PROP_BEEP);
        mToneMap.put(Tone.POSITIVE_ACK, ToneGenerator.TONE_PROP_ACK);
        mToneMap.put(Tone.NEGATIVE_ACK, ToneGenerator.TONE_PROP_NACK);
    }

    private ToneGenerator mToneGenerator = null;

    TonePlayer() {
        mToneGenerator = new ToneGenerator(AudioManager.STREAM_SYSTEM, 100);
    }

    public void play(Tone tone) {
        int toneId = getToneId(tone);
        if (toneId > 0 && mToneGenerator != null) {
            mToneGenerator.startTone(toneId);
        }
    }

    public void stop() {
        if (mToneGenerator != null) {
            mToneGenerator.stopTone();
        }
    }

    public void release() {
        mToneGenerator.release();
    }

    private int getToneId(Tone tone) {
        int toneId = ToneGenerator.TONE_PROP_BEEP;

        if (tone != null && mToneMap.containsKey(tone)) {
            Integer toneIdInteger = mToneMap.get(tone);
            if (toneIdInteger != null) {
                toneId = toneIdInteger;
            }
        }
        return toneId;
    }
}
