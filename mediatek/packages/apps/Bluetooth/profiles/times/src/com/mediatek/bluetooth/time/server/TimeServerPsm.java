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

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.psm.Psm;
import com.mediatek.bluetooth.psm.PsmConnection;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmService;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.ResponseCode;

public class TimeServerPsm extends Psm {
	private static final String TAG = "TimeServerPsm";

	public TimeServerPsm(PsmService psmService) {
		super(BluetoothProfile.ID_TIMES, psmService,
				TimeServerMsg.MSG_ID_BT_TIMES_GROUP_START,
				TimeServerMsg.MSG_ID_BT_TIMES_GROUP_END,
				TimeServerConstants.TIMES_INIT_CONNECTION_COUNT,
				TimeServerConstants.TIMES_MAX_CONNECTION_COUNT);
	}

	@Override
	protected PsmServiceBinder createServiceBinder() {
		return new TimeServerService(this);
	}

	@Override
	protected void onServiceCreate() {
		super.onServiceCreate();
		onServiceBind();
	}

	@Override
	// Dispatch message to relative handler function.
	public int handleMessage(PsmMessage message) {
		TimeServerLog.d(TAG, "Handle message: " + message.getId());
		PsmConnection connection = getConnection(message.getIndex());
		int state = connection.getCurrentState();

		switch (state) {
			case TimeServerConstants.TIMES_STATE_NEW:
				return stateNew(connection, message);
			case TimeServerConstants.TIMES_STATE_REGISTERING:
				return stateRegistering(connection, message);
			case TimeServerConstants.TIMES_STATE_UNREGISTERING:
				return stateUnregistering(connection, message);
			case TimeServerConstants.TIMES_STATE_CONNECTABLE:
				return stateConnectable(connection, message);
			case TimeServerConstants.TIMES_STATE_CONNECTING:
				return stateConnecting(connection, message);
			case TimeServerConstants.TIMES_STATE_DISCONNECTING:
				return stateDisconnecting(connection, message);
			case TimeServerConstants.TIMES_STATE_CONNECTED:
				return stateConnected(connection, message);
			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.UNDEFINED_STATE);
		}
	}

	// Handler function: NEW state
	private int stateNew(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_REGISTER_REQ:
				conn.setCurrentState(TimeServerConstants.TIMES_STATE_REGISTERING);
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: REGISTERING state
	private int stateRegistering(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		byte rsp;

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_REGISTER_CNF:
				TimeServerLog.d(TAG, "Received TIMES_REGISTER_CNF");
				rsp = msg.getByte(TimeServerMsg.TIMES_REGISTER_CNF_B_RSPCODE);

				if (rsp == ResponseCode.SUCCESS) {
					conn.setCurrentState(TimeServerConstants.TIMES_STATE_CONNECTABLE);
				} else {
					conn.setCurrentState(TimeServerConstants.TIMES_STATE_NEW);
				}

				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: UNREGISTERING state
	private int stateUnregistering(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_DEREGISTER_CNF:
				TimeServerLog.d(TAG, "Received TIMES_DEREGISTER_CNF");
				conn.setCurrentState(TimeServerConstants.TIMES_STATE_NEW);
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: CONNECTABLE state
	private int stateConnectable(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_AUTHORIZE_IND:
				conn.setCurrentState(TimeServerConstants.TIMES_STATE_CONNECTING);
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeServerMsg.MSG_ID_BT_TIMES_DEREGISTER_REQ:
				conn.setCurrentState(TimeServerConstants.TIMES_STATE_UNREGISTERING);
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			// To be implemented. The most implementation of Time server.

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: CONNECTING state
	private int stateConnecting(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_AUTHORIZE_RSP:
				byte rsp = msg.getByte(TimeServerMsg.TIMES_AUTHORIZE_RSP_B_RSPCODE);
				if (rsp != ResponseCode.SUCCESS) {
					conn.setCurrentState(TimeServerConstants.TIMES_STATE_CONNECTABLE);
				}
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeServerMsg.MSG_ID_BT_TIMES_CONNECT_IND:
				conn.setCurrentState(TimeServerConstants.TIMES_STATE_CONNECTED);
				// Implementing...
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeServerMsg.MSG_ID_BT_TIMES_DISCONNECT_REQ:
			case TimeServerMsg.MSG_ID_BT_TIMES_DISCONNECT_IND:
				// Implementing...
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: DISCONNECTING state
	private int stateDisconnecting(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_DISCONNECT_IND:
				return handleDisconnectInd(conn, msg);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: CONNECTED state
	private int stateConnected(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();

		switch (msg_id) {
			case TimeServerMsg.MSG_ID_BT_TIMES_DISCONNECT_IND:
				return handleDisconnectInd(conn, msg);

			case TimeServerMsg.MSG_ID_BT_TIMES_DISCONNECT_REQ:
				conn.setCurrentState(TimeServerConstants.TIMES_STATE_DISCONNECTING);
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeServerMsg.MSG_ID_BT_TIMES_GET_CTTIME_IND:
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeServerMsg.MSG_ID_BT_TIMES_GET_CTTIME_RSP:
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	// Handler function: MSG_ID_TIMES_DISCONNECT_IND
	private int handleDisconnectInd(PsmConnection conn, PsmMessage msg) {
		conn.setCurrentState(TimeServerConstants.TIMES_STATE_CONNECTABLE);
		recvMessage(msg);
		return ResultCode.create(ResultCode.STATUS_SUCCESS);
	}

}
