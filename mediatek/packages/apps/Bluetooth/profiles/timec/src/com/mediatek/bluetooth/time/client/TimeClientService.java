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

package com.mediatek.bluetooth.time.client;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ResultReceiver;

import com.mediatek.bluetooth.psm.Psm;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.service.IBluetoothTimec;
import com.mediatek.bluetooth.time.client.TimeClientPsmConn;
import com.mediatek.bluetooth.util.ConvertUtils;

import java.util.GregorianCalendar;

public class TimeClientService extends IBluetoothTimec.Stub implements PsmServiceBinder {

	private static final String TAG = "TimeClientService";

	// Profile (Finite) State Machine of Time Client
	private Psm mPsm;

	// Time server device
	private BluetoothDevice mDevice;

	// UI callback
	private ResultReceiver mCallback;

	private int mConnID = -1;

	public TimeClientService(TimeClientPsm psm) {
		mPsm = psm;
	}

/**************************************************************************************************
 * Interface implementing functions
 **************************************************************************************************/
 	// For implementing PsmServiceBinder ver.1110_cb
	public void onServiceBind() {
	}

 	// For implementing PsmServiceBinder ver.1110_cb
	public void onServiceDestroy() {
	}

 	// For implementing PsmServiceBinder ver.1048_ble
	public void onBind() {
	}

 	// For implementing PsmServiceBinder ver.1048_ble
	public void onDestroy() {
	}

 	// For implementing PsmServiceBinder
	public IBinder getBinder() {
		return this;
	}

 	// For implementing PsmServiceBinder
	public void onMessageReceived(PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			case TimeClientMsg.MSG_ID_BT_TIMEC_CONNECT_CNF:
				handleConnectCnf(msg);
				break;

			case TimeClientMsg.MSG_ID_BT_TIMEC_DISCONNECT_IND:
				handleDisconnectInd(msg);
				break;

			case TimeClientMsg.MSG_ID_BT_TIMEC_GET_CTTIME_CNF:
				handleGetCtTimeCnf(msg);
				break;

			default:
				break;
		}
	}

/**************************************************************************************************
 * Handler functions
 **************************************************************************************************/
	private void forwardResult(int msg_id, Bundle result) {
		if (mCallback != null) {
			mCallback.send(msg_id, result);
		} else {
			TimeClientLog.w(TAG, "UI callback is null");
		}
	}

	private void handleConnectCnf(PsmMessage msg) {
		byte rsp = msg.getByte(TimeClientMsg.TIMEC_CONNECT_CNF_B_RSPCODE);
		if (rsp != ResponseCode.SUCCESS) {
			mPsm.unregisterConnection(mConnID);
			mConnID = -1;
		}

		Bundle result = new Bundle();
		result.putByte("rspcode", rsp);
		forwardResult(msg.getId(), result);
	}

	private void handleDisconnectInd(PsmMessage msg) {
		if (mConnID != -1) {
			mPsm.unregisterConnection(mConnID);
			mConnID = -1;
		}
		forwardResult(msg.getId(), null);
	}

	private void handleGetCtTimeCnf(PsmMessage msg) {
		byte rsp = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_RSPCODE);
		int year = msg.getShort(TimeClientMsg.TIMEC_GET_CTTIME_CNF_S_YEAR);
		int month = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_MONTH);
		int day = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_DAY);
		int hours = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_HOURS);
		int minutes = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_MINUTES);
		int seconds = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_SECONDS);
		int day_of_week = msg.getByte(TimeClientMsg.TIMEC_GET_CTTIME_CNF_B_DAY_OF_WEEK);

		// For now, print retrieved time informations
		TimeClientLog.d(TAG, "handleGetCtTimeCnf() " +
				" year: " + year +
				" month: " + month +
				" day: " + day +
				" hours: " + hours +
				" minutes: " + minutes +
				" seconds: " + seconds + 
				" day of week: " + day_of_week);

		Bundle result = new Bundle();
		result.putByte("rspcode", rsp);
		forwardResult(msg.getId(), result);
	}

/**************************************************************************************************
 * Service functions
 **************************************************************************************************/
	public void registerCallback(ResultReceiver callback) {
		Utils.logD(TAG, "registerCallback()");
		mCallback = callback;
	}

	public boolean unregisterCallback(ResultReceiver callback) {
		if (callback == mCallback) {
			mCallback = null;
			return true;
		}
		return false;
	}

	public int connect(BluetoothDevice device) {
		if (device != null) {
			mDevice = device;
		}

		String str_addr = device.getAddress();
		byte[] ba_addr = ConvertUtils.convertBdAddr(str_addr);
		TimeClientLog.d(TAG, "Connect to " + str_addr + ", converted: " + ba_addr);

		mConnID = mPsm.registerConnection(new TimeClientPsmConn());
		PsmMessage req = new PsmMessage(TimeClientMsg.TIMEC_CONNECT_REQ, mConnID);
		req.setByteArray(TimeClientMsg.TIMEC_CONNECT_REQ_BA_ADDR, TimeClientMsg.TIMEC_CONNECT_REQ_BL_ADDR, ba_addr);

		return mPsm.handleMessage(req);
	}

	public int disconnect() {
		TimeClientLog.d(TAG, "Disconnect, id: " + mConnID);
		PsmMessage req = new PsmMessage(TimeClientMsg.TIMEC_DISCONNECT_REQ, mConnID);
		return mPsm.handleMessage(req);
	}

	public int getServerTime() {
		TimeClientLog.d(TAG, "getServerTime()");
		PsmMessage req = new PsmMessage(TimeClientMsg.TIMEC_GET_CTTIME_REQ, mConnID);
		return mPsm.handleMessage(req);
	}

	public int getAutoConfig() {
		return -1;
	}

	public int setAutoConfig(boolean enabled) {
		return -1;
	}

	public int getDstInfo() {
		return -1;
	}

	public int requestServerUpdate() {
		return -1;
	}

	public int cancelServerUpdate() {
		return -1;
	}

	public int getUpdateStatus() {
		return -1;
	}

}
