/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.common.audioprofile;


import java.util.List;
import android.media.RingtoneManager;

public interface IAudioProfileManager {
    /**
     * The scenario that an audio profile is designed for.
     */
    public enum Scenario {
        GENERAL, SILENT, MEETING, OUTDOOR, CUSTOM
    }
    
    /**
     * Type that refers to sounds that are used for the phone ringer.
     *
     * @see #getRingtoneUri(Context, int)
     * @see #setRingtoneUri(Context, int, Uri)
     */
    public static final int TYPE_RINGTONE = RingtoneManager.TYPE_RINGTONE;

    /**
     * Type that refers to sounds that are used for notifications.
     * 
     * @see #getRingtoneUri(Context, int)
     * @see #setRingtoneUri(Context, int, Uri)
     */
    public static final int TYPE_NOTIFICATION = RingtoneManager.TYPE_NOTIFICATION;
    
    /**
     * Type that refers to sounds that are used for the video call.
     *
     * @see #getRingtoneUri(Context, int)
     * @see #setRingtoneUri(Context, int, Uri)
     */
    public static final int TYPE_VIDEO_CALL = 8;
    
    /** The prefixe of audio profile keys. */
    public static final String PROFILE_PREFIX = "mtk_audioprofile_";
    
    /** The key used to store the active audio profile. */
    public static final String KEY_ACTIVE_PROFILE = "mtk_audioprofile_active";

    /** The key used to store the default ringtone of voice call. */
    public static final String KEY_DEFAULT_RINGTONE = "mtk_audioprofile_default_ringtone";

    /** The key used to store the default notification sound. */
    public static final String KEY_DEFAULT_NOTIFICATION = "mtk_audioprofile_default_notification";

    /** The key used to store the default ringtone of video call. */
    public static final String KEY_DEFAULT_VIDEO_CALL = "mtk_audioprofile_default_video_call";
}
