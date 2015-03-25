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

package com.mediatek.bluetooth.prx.reporter;

public interface PrxrConstants {

	// Level ( need to sync with ble_prxm_arrays.xml )
	public static final byte PRXR_ALERT_LEVEL_NULL = -1;
	public static final byte PRXR_ALERT_LEVEL_NO = 0;
	public static final byte PRXR_ALERT_LEVEL_MILD = 1;
	public static final byte PRXR_ALERT_LEVEL_HIGH = 2;

	// Action used to issue link-loss and path-loss Intent
	public static final String ACTION_LINK_LOSS = "com.mediatek.bluetooth.prx.action.LINK_LOSS";
	public static final String ACTION_PATH_LOSS = "com.mediatek.bluetooth.prx.action.PATH_LOSS";

	public static final String EXTRA_DEVICE_NAME = "com.mediatek.bluetooth.prx.extra.DEVICE_NAME";
	public static final String EXTRA_ALERT_LEVEL = "com.mediatek.bluetooth.prx.extra.ALERT_LEVEL";

	// State
	public static final byte PRXR_STATE_NEW = 0;
	public static final byte PRXR_STATE_REGISTERING = 1;
	public static final byte PRXR_STATE_UNREGISTERING = 2;
	public static final byte PRXR_STATE_CONNECTABLE = 3;
	public static final byte PRXR_STATE_CONNECTING = 4;
	public static final byte PRXR_STATE_DISCONNECTING = 5;
	public static final byte PRXR_STATE_CONNECTED = 6;

	// Config: define the initial/maximum size for concurrent connections
	public static final byte PRXR_INITIAL_CONNECTION_COUNT = 1;
	public static final byte PRXR_MAXIMUM_CONNECTION_COUNT = 1;
}
