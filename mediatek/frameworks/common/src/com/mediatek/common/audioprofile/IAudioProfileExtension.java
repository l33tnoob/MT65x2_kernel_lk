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

import java.util.HashMap;

import android.content.Context;

/**
 * Interface that extends to customize IAudioProfile
 */
public interface IAudioProfileExtension {
	/**
	 * Assist structure filled with information to help do the active profile change logic.
	 * 
	 * @see IAudioProfileExtension#getActiveProfileChangeInfo(boolean, String, String, boolean)
	 *
	 */
	public interface IActiveProfileChangeInfo {
		// Constant to indicate that no ringer mode change should happen
		public final int RINGER_MODE_UPDATE_NONE = -999;
		
		/**
		 * Get the ringer mode to update
		 * @return The ringer mode
		 */
		public int getRingerModeToUpdate();
		
		/**
		 * Get whether the last active key should be set
		 * @return True if the last active key should be set
		 */
		public boolean shouldSetLastActiveKey();
		
		/**
		 * Get whether we should override ringtone and volume to system
		 * @return True if we should override ringtone and volume to system
		 */
		public boolean shouldOverrideSystem();
		
		/**
		 * Get whether we should sync ringtone and volume to system
		 * @return True if we should
		 */
		public boolean shouldSyncToSystem();		
	}
	
	/**
	 * Interface to get default settings of four default profiles.
	 */
	public interface IDefaultProfileStatesGetter {
	    /**
	     * Get default settings of four default profiles.
	     * Please reference AudioProfileService.getDefaultState(String profileKey) to implement this
	     * 
	     * @return a HashMap filled with (position, AudioProfileState) pairs
	     */
		public <AudioProfileState> HashMap<Integer, AudioProfileState> getDefaultProfileStates();
	}
	
	/**
	 * Initialize an extension instance
	 * note: this must be called before any other method invocation
	 * 
	 * @param service The IAudioProfileService instance to be customized.
	 * @param context The Context in which the extension is running.
	 */
	public void init(IAudioProfileService service, Context context);
	
	/**
	 * Called when Notification changed by other application
	 * 
	 * @param selfChange True if this is a self-change notification.
	 * @return True if the extension has done with its customized logic, and false if it expects the IAudioProfileService to process the default logic.
	 * 
	 * @see android.database.ContentObserver#onChange(boolean selfChange)
	 */
	public boolean onNotificationChange(boolean selfChange);

	/**
	 * Called when Ringtone changed by other application
	 * 
	 * @param selfChange True if this is a self-change notification.
	 * @return True if the extension has done with its customized logic, and false if it expects the IAudioProfileService to process the default logic.
	 * 
	 * @see android.database.ContentObserver#onChange(boolean selfChange)
	 */
	public boolean onRingtoneChange(boolean selfChange);

	/**
	 * Called when ringer mode changed by other application
	 * 
	 * @param newRingerMode the new ringer mode.
	 * @return True if the extension has done with its customized logic, and false if it expects the IAudioProfileService to process the default logic.
	 */
	public boolean onRingerModeChanged(int newRingerMode);

	/**
	 * Called when system volume changed
	 * 
	 * @param oldVolume the old system volume.
	 * @param newVolume the new system volume.
	 * @param extra Extra string.
	 * @return True if the extension has done with its customized logic, and false if it expects the IAudioProfileService to process the default logic.
	 * 
	 * @see AudioProfileListener#onRingerVolumeChanged(int oldVolume, int newVolume, String extra)
	 */
	public boolean onRingerVolumeChanged(int oldVolume, int newVolume, String extra);
	
	/**
	 * Call back when active profile changed
	 * 
	 * @param shouldSetRingerMode the old system volume.
	 * @param oldProfileKey the old system volume.
	 * @param newProfileKey Extra string.
	 * @return True if the extension has done with its customized logic, and false if it expects the IAudioProfileService to process the default logic.
	 *
	 * @see IActiveProfileChangeInfo
	 */
	public boolean onActiveProfileChange(boolean shouldSetRingerMode,
			String oldProfileKey, String newProfileKey);

	/**
	 * Call back when active profile changed
	 * 
	 * @param shouldSetRingerMode Whether need to set ringer mode, false if ringer mode was changed by other app.
	 * @param oldProfileKey Old active profile key.
	 * @param newProfileKey New active profile key.
	 * @param customActiveProfileDeleted Indicate whether the last active custom profile has been deleted
	 * @return the {@link IActiveProfileChangeInfo} instance to help do the active profile change logic; null if it expects the IAudioProfileService to process the default logic.
	 */
	public IActiveProfileChangeInfo getActiveProfileChangeInfo(boolean shouldSetRingerMode,
			String oldProfileKey, String newProfileKey, boolean customActiveProfileDeleted);

    /**
     * Persist the active profile volume to system to make it effective immediately 
     * with given type
     * 
     * @param type The stream type 
     */
	public boolean persistStreamVolumeToSystem(int streamType);

	/**
	 * Whether we should check the four default profiles' settings to determine whether match it's type.
	 * 
	 * @return True if we should check.
	 */
	public boolean shouldCheckDefaultProfiles();

    /**
     * Whether we should sync ringtone of General profile to Outdoor.
     * 
     * @return True if we should sync.
     */
	public boolean shouldSyncGeneralRingtoneToOutdoor();
}
