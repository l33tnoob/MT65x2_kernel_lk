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

package android.bluetooth;

import android.util.Log;


public class ProfileConfig {
	private static final String TAG = "ProfileConfig";
	
    public static final String PROFILE_ID_SIMAP = "profile_supported_simap";
	/// M: Advanced Profile: Proximity Monitor Role ID
	public static final String PROFILE_ID_PRXM = "profile_supported_prxm";
	/// M: Advanced Profile: Proximity Reporter Role ID
	public static final String PROFILE_ID_PRXR  = "profile_supported_prxr";
	/// M: Advanced Profile: Ftp Profile ID
	public static final String PROFILE_ID_FTP = "profile_supported_ftp";
	/// M: Advanced Profile: Printer Profile ID
	public static final String PROFILE_ID_BPP = "profile_supported_bpp";
	/// M: Advanced Profile: BIP Profile ID
	public static final String PROFILE_ID_BIP = "profile_supported_bip";
	/// M: Advanced Profile: DUN Profile ID
	public static final String PROFILE_ID_DUN = "profile_supported_dun";
	/// M: Advanced Profile: MAP Server Role ID
	public static final String PROFILE_ID_MAPS = "profile_supported_maps";
	/// M: Advanced Profile: MAP Client Role ID
	public static final String PROFILE_ID_MAPC = "profile_supported_mapc";
	/// M: Advanced Profile: TIME Server Role ID
	public static final String PROFILE_ID_TIMES = "profile_supported_times";
	/// M: Advanced Profile: TIME Client Role ID
	public static final String PROFILE_ID_TIMEC = "profile_supported_timec";

	private String profileID;

	private boolean profileEnabled;

	public ProfileConfig(){
	}

	public void setProfileID(String ID)
	{
		profileID = ID;
	}

	public String getProfileID()
	{
		return profileID;
	}

	public void setProfileEnabled(boolean enabled)
	{
		profileEnabled = enabled;
	}

	public boolean getProfileEnabled()
	{
		return profileEnabled;
	}
}

