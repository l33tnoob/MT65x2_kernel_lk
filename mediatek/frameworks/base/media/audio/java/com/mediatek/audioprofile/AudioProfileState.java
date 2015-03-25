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

package com.mediatek.audioprofile;

import android.net.Uri;

public class AudioProfileState {
    /** Profile key. */
    public String mProfileKey;

    /** Voice call ringtone uri. */
    public Uri mRingerStream;

    /** Notification ringtone uri. */
    public Uri mNotificationStream;

    /** Video call ringtone uri. */
    public Uri mVideoCallStream;
    
    /** SIP call ringtone uri. */
    public Uri mSIPCallStream;

    /** Voice call ringtone volume. */
    public int mRingerVolume;

    /** Notification volume. */
    public int mNotificationVolume;

    /** Alarm volume. */
    public int mAlarmVolume;

    /** Whether the phone should vibrate for incoming calls. */
    public boolean mVibrationEnabled;

    /** Whether sound should be played when using dial pad. */
    public boolean mDtmfToneEnabled;

    /** Whether sound should be played when making screen selection. */
    public boolean mSoundEffectEnbled;

    /** Whether to play sounds when the keyguard is shown and dismissed. */
    public boolean mLockScreenSoundEnabled;

    /**
     * Whether the phone should vibrate when pressing soft keys and on certain UI interactions.
     */
    public boolean mHapticFeedbackEnabled;

    /** Whether the notification use the volume of ring. */
    public boolean mNoficationUseRingVolume;
    
    /** SIM card id */
    public long mSimId;

    public AudioProfileState() {
    }

    public static class Builder {
        // must initial
        private final String mProfileKey;
        // If not init, use default values
        private Uri mRingerStream = AudioProfileManager.DEFAULT_RINGER_STREAM_URI;
        private Uri mNotificationStream = AudioProfileManager.DEFAULT_NOTIFICATION_STREAM_URI;
        private Uri mVideoCallStream = AudioProfileManager.DEFAULT_VIDEO_STREAM_URI;
        private Uri mSIPCallStream = AudioProfileManager.DEFAULT_SIP_STREAM_URI;
        private int mRingerVolume = 0;
        private int mNotificationVolume = 0;
        private int mAlarmVolume = 0;
        private boolean mVibrationEnabled = false;
        private boolean mDtmfToneEnabled = false;
        private boolean mSoundEffectEnbled = false;
        private boolean mLockScreenSoundEnabled = false;
        private boolean mHapticFeedbackEnabled = false;
        private boolean mNoficationUseRingVolume = true;
        private long mSimId = -1;
        
        /**
         * AudioProfileState builder
         * 
         * @param profileKey
         *            profile key
         */
        public Builder(String profileKey) {
            this.mProfileKey = profileKey;
        }

        /**
         * Set the {@link Uri} voice call, notification and video call ringtone uri.
         * 
         * @param voiceCallUri
         *            Ringer stream ringtone uri, see as {@link #mRingerStream}
         * @param notificationUri
         *            Notification stream ringtone uri, see as {@link #mNotificationStream}
         * @param videoCallUri
         *            video call stream ringtone uri, see as {@link #mVideoCallStream}
         * @return The builder
         */
        public Builder ringtone(Uri voiceCallUri, Uri notificationUri, Uri videoCallUri) {
            this.mRingerStream = voiceCallUri;
            this.mNotificationStream = notificationUri;
            this.mVideoCallStream = videoCallUri;
            return this;
        }
        
        public Builder ringtone(Uri voiceCallUri, Uri notificationUri, Uri videoCallUri, Uri sipCallUri) {
            this.mRingerStream = voiceCallUri;
            this.mNotificationStream = notificationUri;
            this.mVideoCallStream = videoCallUri;
            this.mSIPCallStream = sipCallUri;
            return this;
        }

        /**
         * Set the ringer, notification and alarm volume.
         * 
         * @param ringerVolume
         *            ringer volume, see as {@link #mRingerVolume}
         * @param notificationVolume
         *            Notification volume, see as {@link #mNotificationVolume}
         * @param alarmVolume
         *            alarm volume, see as {@link #mAlarmVolume}
         * @return The builder
         */
        public Builder volume(int ringerVolume, int notificationVolume, int alarmVolume) {
            this.mRingerVolume = ringerVolume;
            this.mNotificationVolume = notificationVolume;
            this.mAlarmVolume = alarmVolume;
            return this;
        }

        /**
         * Set the vibrate enabled
         * 
         * @param enable
         *            Enable vibration
         * @return the builder
         */
        public Builder vibration(boolean enable) {
            this.mVibrationEnabled = enable;
            return this;
        }

        /**
         * Set the dtmfTone enabled
         * 
         * @param enable
         *            Enable dtmfTone
         * @return the builder
         */
        public Builder dtmfTone(boolean enable) {
            this.mDtmfToneEnabled = enable;
            return this;
        }

        /**
         * Set the soundEffect enabled
         * 
         * @param enable
         *            Enable soundEffect
         * @return the builder
         */
        public Builder soundEffect(boolean enable) {
            this.mSoundEffectEnbled = enable;
            return this;
        }

        /**
         * Set the lockScreenSound enabled
         * 
         * @param enable
         *            Enable lockScreenSound
         * @return the builder
         */
        public Builder lockScreenSound(boolean enable) {
            this.mLockScreenSoundEnabled = enable;
            return this;
        }

        /**
         * Set the hapticFeedback enabled
         * 
         * @param enable
         *            Enable hapticFeedback
         * @return the builder
         */
        public Builder hapticFeedback(boolean enable) {
            this.mHapticFeedbackEnabled = enable;
            return this;
        }

        /**
         * Set the SIM card id
         * 
         * @param simId
         *            SIM card id
         * @return the builder
         */
        public Builder simId(long simId) {
            this.mSimId = simId;
            return this;
        }

        /**
         * Build a AudioProfileState instance
         * 
         * @return A new AudioProfileState instance
         */
        public AudioProfileState build() {
            return new AudioProfileState(this);
        }
    }

    /**
     * Init AudioProfleState with builder
     * 
     * @param builder
     *            The builder
     */
    private AudioProfileState(Builder builder) {
        mProfileKey = builder.mProfileKey;

        mRingerStream = builder.mRingerStream;
        mNotificationStream = builder.mNotificationStream;
        mVideoCallStream = builder.mVideoCallStream;
        mSIPCallStream = builder.mSIPCallStream;
        
        mRingerVolume = builder.mRingerVolume;
        mAlarmVolume = builder.mAlarmVolume;
        mNotificationVolume = builder.mNotificationVolume;

        mVibrationEnabled = builder.mVibrationEnabled;
        mSoundEffectEnbled = builder.mSoundEffectEnbled;
        mDtmfToneEnabled = builder.mDtmfToneEnabled;
        mHapticFeedbackEnabled = builder.mHapticFeedbackEnabled;
        mLockScreenSoundEnabled = builder.mLockScreenSoundEnabled;
        mNoficationUseRingVolume = builder.mNoficationUseRingVolume;
        mSimId = builder.mSimId;
    }

    @Override
    public String toString() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("volume_ringtone = ").append(this.mRingerVolume).append(",");
        buffer.append("volume_notification = ").append(this.mNotificationVolume).append(",");
        buffer.append("volume_alarm = ").append(this.mAlarmVolume).append(",");

        buffer.append("vibrate_on = ").append(this.mVibrationEnabled).append(",");
        buffer.append("dtmf_tone = ").append(this.mDtmfToneEnabled).append(",");
        buffer.append("sound_effects = ").append(this.mSoundEffectEnbled).append(",");
        buffer.append("lockscreen_sounds = ").append(this.mLockScreenSoundEnabled).append(",");
        buffer.append("haptic_feedback = ").append(this.mHapticFeedbackEnabled).append(",");

        buffer.append("ringtone = ").append(this.mRingerStream).append(",");
        buffer.append("notification_sound = ").append(this.mNotificationStream).append(",");
        buffer.append("video_call = ").append(this.mVideoCallStream).append(",");
        buffer.append("sip_call = ").append(this.mSIPCallStream).append(",");
        buffer.append("sim id = ").append(this.mSimId);
        return buffer.toString();
    }
}
