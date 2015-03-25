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

package com.mediatek.op.audioprofile;

import java.util.HashMap;

import android.content.Context;
import com.mediatek.common.audioprofile.IAudioProfileExtension;
import com.mediatek.common.audioprofile.IAudioProfileService;

public class DefaultAudioProfileExtension implements IAudioProfileExtension {
	// IS_SUPPORT_OUTDOOR_EDITABLE is to be a feature option in the future
	protected final boolean IS_SUPPORT_OUTDOOR_EDITABLE = false;
	
	public static class ActiveProfileChangeInfo implements IActiveProfileChangeInfo {
		public boolean mShouldOverrideSystem = true;
		public int mRingerModeToUpdate = IActiveProfileChangeInfo.RINGER_MODE_UPDATE_NONE;
		public boolean mShouldSetLastActiveKey = false;
		public boolean mShouldSyncToSystem = false;

		public void setRingerModeToUpdate(int ringerModeToUpdate) {
		    mRingerModeToUpdate = ringerModeToUpdate;
		}
		
		public int getRingerModeToUpdate() {
			return mRingerModeToUpdate;
		}
		
		public void setShouldSetLastActiveKey(boolean shouldSetLastActiveKey) {
		    mShouldSetLastActiveKey = shouldSetLastActiveKey;
		}
		
		public boolean shouldSetLastActiveKey() {
			return mShouldSetLastActiveKey;
		}
		
		public void setShouldOverrideSystem(boolean shouldOverrideSystem) {
		    mShouldOverrideSystem = shouldOverrideSystem;
		}
		
		public boolean shouldOverrideSystem(){
			return mShouldOverrideSystem;
		}
		
		public void setShouldSyncToSystem(boolean shouldSyncToSystem) {
		    mShouldSyncToSystem = shouldSyncToSystem;
		}
		
		public boolean shouldSyncToSystem(){
			return mShouldSyncToSystem;
		}
	}

	public void init(IAudioProfileService service, Context context) {
	} 

	public boolean onNotificationChange(boolean selfChange) {
		return false;
	}

	public boolean onRingerModeChanged(int newRingerMode) {
		return false;
	}

	public boolean onRingerVolumeChanged(int oldVolume, int newVolume, String extra) {
		return false;
	}

	public boolean onRingtoneChange(boolean selfChange) {
		return false;
	}

	public boolean persistStreamVolumeToSystem(int streamType) {
		return false;
	}

	public boolean shouldCheckDefaultProfiles() {
		return true;
	}

	public boolean shouldSyncGeneralRingtoneToOutdoor() {
		return !IS_SUPPORT_OUTDOOR_EDITABLE;
	}

	public IActiveProfileChangeInfo getActiveProfileChangeInfo(
			boolean shouldSetRingerMode, String oldProfileKey,
			String newProfileKey, boolean customActiveProfileDeleted) {
		return null;
	}

	public boolean onActiveProfileChange(boolean shouldSetRingerMode,
			String oldProfileKey, String newProfileKey) {
		return true;
	}
}

