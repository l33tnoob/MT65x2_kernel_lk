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

package com.mediatek.common.audioprofile;

import android.net.Uri;
import com.mediatek.common.audioprofile.IAudioProfileListener;

/**
 * {@hide}
 */
interface IAudioProfileService {

    void setActiveProfile(String profileKey);
    String addProfile();
    boolean deleteProfile(String profileKey);
    void reset();
    int getProfileCount();
    List<String> getAllProfileKeys();
    List<String> getPredefinedProfileKeys();
    List<String> getCustomizedProfileKeys();
    boolean isNameExist(String name);
    String getActiveProfileKey();
    String getLastActiveProfileKey();
    
    Uri getRingtoneUri(String profileKey, int type);
    Uri getRingtoneUriWithSIM(String profileKey, int type, long simId);
    int getStreamVolume(String profileKey, int streamType);
    boolean getVibrationEnabled(String profileKey);
    boolean getDtmfToneEnabled(String profileKey);
    boolean getSoundEffectEnabled(String profileKey);
    boolean getLockScreenEnabled(String profileKey);
    boolean getHapticFeedbackEnabled(String profileKey);
    List<String> getProfileStateString(String profileKey);
    String getProfileName(String profileKey);
    
    void setRingtoneUri(String profileKey, int type, long simId,  in Uri ringtoneUri);
    void setStreamVolume(String profileKey, int streamType, int index);
    void setVibrationEnabled(String profileKey, boolean enabled);
    void setDtmfToneEnabled(String profileKey, boolean enabled);
    void setSoundEffectEnabled(String profileKey, boolean enabled);
    void setLockScreenEnabled(String profileKey, boolean enabled);
    void setHapticFeedbackEnabled(String profileKey, boolean enabled);
    void setProfileName(String profileKey, String newName);
    void setUserId (int userId);
    
    boolean isActive(String profileKey);
    boolean isRingtoneExist(in Uri uri);
    int getStreamMaxVolume(int streamType);
    Uri getDefaultRingtone(int type);
    
    oneway void listenAudioProfie(IAudioProfileListener callback, int event);
}
