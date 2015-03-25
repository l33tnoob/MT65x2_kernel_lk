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

package com.mediatek.bluetooth.time.server;

import android.os.Bundle;
import android.os.IBinder;
import android.os.ResultReceiver;

import com.mediatek.bluetooth.psm.PsmConnection;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.service.IBluetoothTimes;

import java.util.Calendar;
import java.util.GregorianCalendar;

public class TimeServerService extends IBluetoothTimes.Stub implements PsmServiceBinder {
	private static final String TAG = "TimeServerService";

	// Service states, internal use.
	private static final int STATE_NEW			= TimeServerConstants.TIMES_STATE_NEW,
							 STATE_ENABLING		= TimeServerConstants.TIMES_STATE_REGISTERING,
							 STATE_DISABLING	= TimeServerConstants.TIMES_STATE_UNREGISTERING,
							 STATE_ENABLED		= TimeServerConstants.TIMES_STATE_CONNECTABLE;

	private int mServiceState;

	// Profile (Finite) State Machine of Time Server
	private TimeServerPsm mPsm;

	// Connection IDs
	private int[] mConnID;

	// Connection state, used to track the return values of register().
	private int[] connState;

	// UI callback
	private ResultReceiver registeredCallback = null;

	public TimeServerService(TimeServerPsm psm) {
		final int max_conn = TimeServerConstants.TIMES_MAX_CONNECTION_COUNT;

		mPsm = psm;
		mServiceState = STATE_NEW;
		mConnID = new int[max_conn];
		connState = new int[max_conn];

		for (int i = 0; i < max_conn; i++) {
			mConnID[i] = -1;
		}
	}

	// For implementing PsmServiceBinder ver.1110_cb
	public void onServiceBind() {
		// TODO: Check saved prference value to auto-enable the server.
		// enableServiceImple();
	}

	// For implementing PsmServiceBinder ver.1110_cb
	public void onServiceDestroy() {
		if (mServiceState == STATE_ENABLED) {
			resetService();
		}
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
			case TimeServerMsg.MSG_ID_BT_TIMES_REGISTER_CNF:
				handleRegisterCnf(msg);
				break;

			case TimeServerMsg.MSG_ID_BT_TIMES_DEREGISTER_CNF:
				handleUnregisterCnf(msg);
				break;

			case TimeServerMsg.MSG_ID_BT_TIMES_AUTHORIZE_IND:
				handleAuthorizeInd(msg);
				break;

			case TimeServerMsg.MSG_ID_BT_TIMES_DISCONNECT_IND:
				handleDisconnectInd(msg);
				break;

			case TimeServerMsg.MSG_ID_BT_TIMES_GET_CTTIME_IND:
				handleGetCtTimeInd(msg);
				break;

			default:
				break;
		}
	}

/**************************************************************************************************
 * Handler functions
 **************************************************************************************************/
	private void handleRegisterCnf(PsmMessage msg) {
		if (mServiceState != STATE_ENABLING) {
			TimeServerLog.e(TAG, "Invalid state for TIMES_REGISTER_CNF: " + mServiceState);
			return;
		}

		int cid = msg.getIndex();
		byte rsp = msg.getByte(TimeServerMsg.TIMES_REGISTER_CNF_B_RSPCODE);
		connState[cid] = (rsp == ResponseCode.SUCCESS) ? 1 : -1;

		boolean bSuccess = true;
		for (int i = 0; i < mConnID.length; i ++) {
			if (connState[i] == 0) {
				// Wait for other confirmations coming back.
				return;
			} else if (connState[i] == -1) {
				bSuccess = false;
			}
		}

		if (bSuccess) {
			mServiceState = STATE_ENABLED;
		} else {
			TimeServerLog.e(TAG, "Failed to enable time server.");
			resetService();
		}

		Bundle result = new Bundle();
		result.putByte("rspcode", rsp);
		forwardResult(msg.getId(), result);
	}

	private void handleUnregisterCnf(PsmMessage msg) {
		if (mServiceState != STATE_DISABLING) {
			TimeServerLog.e(TAG, "Invalidstate for TIMES_DEREGISTER_CNF: " + mServiceState);
			return;
		}

		int cid = msg.getIndex();
		mPsm.unregisterConnection(cid);
		mConnID[cid] = -1;

		for (int i = 0; i < mConnID.length; i++) {
			if (mConnID[i] > -1) return;
		}

		mServiceState = STATE_NEW;
		forwardResult(msg.getId(), null);
	}

	private void handleAuthorizeInd(PsmMessage msg) {
		// For now, accept automatically.
		PsmMessage rsp = new PsmMessage(TimeServerMsg.TIMES_AUTHORIZE_RSP, msg.getIndex());
		rsp.setByte(TimeServerMsg.TIMES_AUTHORIZE_RSP_B_RSPCODE, ResponseCode.SUCCESS);
		mPsm.handleMessage(rsp);
	}

	private void handleDisconnectInd(PsmMessage msg) {

		if (mServiceState == STATE_DISABLING) {
			int idx = msg.getIndex();
			int res = unregister(idx);
			if (ResultCode.status(res) != ResultCode.STATUS_SUCCESS) {
				// Release the connection object directly.
				mPsm.unregisterConnection(mConnID[idx]);
				mConnID[idx] = -1;
			}
		}

		// TODO: Need to broadcast the DISCONNECT event.
	}

	private void handleGetCtTimeInd(PsmMessage msg) {
		TimeServerLog.d(TAG, "handleGetCtTimeInd()");

		GregorianCalendar calendar = new GregorianCalendar();
		PsmMessage rsp = new PsmMessage(TimeServerMsg.TIMES_GET_CTTIME_RSP, msg.getIndex());
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_RSPCODE, ResponseCode.SUCCESS);

		rsp.setShort(TimeServerMsg.TIMES_GET_CTTIME_RSP_S_YEAR, (short) calendar.get(Calendar.YEAR));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_MONTH, (byte) calendar.get(Calendar.MONTH));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_DAY, (byte) calendar.get(Calendar.DAY_OF_MONTH));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_HOURS, (byte) calendar.get(Calendar.HOUR_OF_DAY));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_MINUTES, (byte) calendar.get(Calendar.MINUTE));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_SECONDS, (byte) calendar.get(Calendar.SECOND));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_DAY_OF_WEEK, (byte) calendar.get(Calendar.DAY_OF_WEEK));
		rsp.setByte(TimeServerMsg.TIMES_GET_CTTIME_RSP_B_FRAC256, (byte) 0);

		mPsm.handleMessage(rsp);
	}

	private void forwardResult(int msg_id, Bundle result) {
		if (registeredCallback != null) {
			registeredCallback.send(msg_id, result);
		} else {
			TimeServerLog.e(TAG, "UI callback is null.");
		}
	}

/**************************************************************************************************
 * Service functions
 **************************************************************************************************/
	// For extending IBluetoothTimes.Stub
    public void registerCallback(ResultReceiver callback) {
		registeredCallback = callback;
	}

	// For extending IBluetoothTimes.Stub
	public boolean unregisterCallback(ResultReceiver callback) {
		registeredCallback = null;
		return true;
	}

	// For extending IBluetoothTimes.Stub
	public synchronized int enableService() {
		return enableServiceImple();
	}

	// For extending IBluetoothTimes.Stub
	public synchronized int disableService() {
		TimeServerLog.d(TAG, "disableService()");

		if (mServiceState == STATE_ENABLED) {
			resetService();
			return ResultCode.create(ResultCode.STATUS_SUCCESS);
		} else {
			return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// For extending IBluetoothTimes.Stub
	public int getServiceState() {
		return mServiceState;
	}

	// For extending IBluetoothTimes.Stub
	public int responseAuthorizeInd(int connID, byte rspcode) {
		return -1;
	}

	// For extending IBluetoothTimes.Stub
	public int disconnect(int connID) {
		return mPsm.handleMessage(new PsmMessage(TimeServerMsg.TIMES_DISCONNECT_REQ, connID));
	}

	private synchronized int enableServiceImple() {
		TimeServerLog.d(TAG, "enableServiceImple()");
		final int max_conn = TimeServerConstants.TIMES_MAX_CONNECTION_COUNT;

		if (mServiceState == STATE_NEW) {
			mServiceState = STATE_ENABLING;

			for (int i = 0; i < max_conn; i++) {
				connState[i] = 0;
			}

			for (int i = 0; i < max_conn; i++) {
				PsmConnection conn = new TimeServerPsmConn();
				try {
					mConnID[i] = mPsm.registerConnection(conn);
					int res = register(mConnID[i]);
					if (ResultCode.status(res) != ResultCode.STATUS_SUCCESS) {
						// Handle the failed case of registering server
						TimeServerLog.e(TAG, "Register connection failed: " + i + ", res: " + res);
						resetService();
						return res;
					}

				} catch (Exception ex) {
					TimeServerLog.e(TAG, "Register connection failed: " + i);
					return ResultCode.create(ResultCode.STATUS_FAILED);
				}
			}
			return ResultCode.create(ResultCode.STATUS_SUCCESS);

		} else {
			return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// No server state check, forcefully reset.
	private void resetService() {
		TimeServerLog.d(TAG, "resetService()");
		final int max_conn = TimeServerConstants.TIMES_MAX_CONNECTION_COUNT;
		mServiceState = STATE_DISABLING;
		int res;

		boolean isDone = true;
		for (int i = 0; i < max_conn; i++) {
			if (mConnID[i] == -1) continue;

			// Try to disconnect first
			res = disconnect(mConnID[i]);
			if (ResultCode.status(res) == ResultCode.STATUS_SUCCESS) {
				// Need to wait for DISCONNECT_IND back
				isDone = false;
				continue;
			} 

			// Not in CONNECTED state, try to unregister the connection
			res = unregister(mConnID[i]);
			if (ResultCode.status(res) == ResultCode.STATUS_SUCCESS) {
				// Need to wait for UNREGISTER_CNF back
				isDone = false;

			} else {
				// No need to wait for _IND or _CNF, release the connection
				mPsm.unregisterConnection(mConnID[i]);
				mConnID[i] = -1;
			}
		}

		if (isDone) {
			mServiceState = STATE_NEW;
		}
	}

	private int register(int conn_id) {
		return mPsm.handleMessage(new PsmMessage(TimeServerMsg.TIMES_REGISTER_REQ, conn_id));
	}

	private int unregister(int conn_id) {
		return mPsm.handleMessage(new PsmMessage(TimeServerMsg.TIMES_DEREGISTER_REQ, conn_id));
	}

}
