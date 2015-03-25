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

package com.mediatek.bluetooth.prx.monitor;

public interface PrxmConstants {

	// Level ( need to sync with ble_prxm_arrays.xml )
	public static final byte PRXM_ALERT_LEVEL_NULL = -1;
	public static final byte PRXM_ALERT_LEVEL_NO = 0;
	public static final byte PRXM_ALERT_LEVEL_MILD = 1;
	public static final byte PRXM_ALERT_LEVEL_HIGH = 2;
	
	// State
	public static final byte PRXM_STATE_NEW = 0;
	public static final byte PRXM_STATE_CONNECTING = 1;
	public static final byte PRXM_STATE_DISCONNECTING = 2;
	public static final byte PRXM_STATE_CONNECTED = 3;

	// Capability
	public static final byte PRXM_CAP_NONE = 0x00;
	public static final byte PRXM_CAP_IMMEDIATE_ALERT = 0x01;
	public static final byte PRXM_CAP_TX_POWER = 0x02;

	// Config
	public static final byte PRXM_DEFAULT_LINK_LOSS_LEVEL = PRXM_ALERT_LEVEL_HIGH;
	public static final byte PRXM_DEFAULT_PATH_LOSS_LEVEL = PRXM_ALERT_LEVEL_MILD;
	public static final byte PRXM_DEFAULT_PATH_LOSS_THRESHOLD = 50;

	// Config: define the initial/maximum size for concurrent connections
	public static final byte PRXM_INITIAL_CONNECTION_COUNT = 2;
	public static final byte PRXM_MAXIMUM_CONNECTION_COUNT = 6;

	public static final int PRXM_PATH_LOSS_MONITOR_DELAY = 1000;
	public static final byte PRXM_PATH_LOSS_GRANULARITY = 100;
	public static final byte PRXM_PATH_LOSS_MAX_RSSI = -20;
	public static final byte PRXM_PATH_LOSS_MIN_RSSI = -110;
}
