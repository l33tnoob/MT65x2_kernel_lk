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

import android.os.ResultReceiver;

import com.mediatek.bluetooth.psm.PsmConnection;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;

public class PrxmPsmConn extends PsmConnection {

	/**
	 * device info (remote device)
	 */
	private BluetoothPrxmDevice deviceInfo;

	/**
	 * link loss level - pending (will assign to deviceInfo.linkLossLevel after confirmation from reporter)
	 */
	private byte linkLossLevelPending;

	/**
	 * keep the state of path-loss monitoring (start/stop)
	 */
	private boolean isMonitoring = false;

	/**
	 * monitor update delay
	 */
	private int monitorDelay = PrxmConstants.PRXM_PATH_LOSS_MONITOR_DELAY;
	
	/**
	 * result callback list
	 */
	private ResultReceiver callback;

	/**
	 * add check flag for SET_PATH_LOSS_REQ (only send out next req when the previous req was confirmed)
	 */
	public boolean isWaitingForSetPathLossCnf = false;
	public boolean isLastPathLossLevelChanged = false;
	public byte lastPathLossLevel = PrxmConstants.PRXM_ALERT_LEVEL_NULL;

	/**
	 * Constructor
	 * 
	 * @param bdaddr
	 */
	protected PrxmPsmConn( BluetoothPrxmDevice deviceInfo ){

		super( PrxmConstants.PRXM_STATE_NEW );

		this.deviceInfo = deviceInfo;
	}

	@Override
	public void setCurrentState( int currentState ){

		super.setCurrentState(currentState);
		this.deviceInfo.setCurrentState( (byte)currentState );
	}

	public BluetoothPrxmDevice getDeviceInfo(){
		return this.deviceInfo;
	}
	public ResultReceiver getCallback(){
		return callback;
	}
	public void setCallback( ResultReceiver callback ){
		this.callback = callback;
	}
	public byte getLinkLossLevelPending(){
		return linkLossLevelPending;
	}
	public void setLinkLossLevelPending( byte linkLossLevelPending ){
		this.linkLossLevelPending = linkLossLevelPending;
	}
	public boolean isMonitoring(){
		return isMonitoring;
	}
	public void setMonitoring( boolean isMonitoring ){
		this.isMonitoring = isMonitoring;
	}
	public int getMonitorDelay(){
		return monitorDelay;
	}
	public void setMonitorDelay(int monitorDelay) {
		this.monitorDelay = monitorDelay;
	}
}
