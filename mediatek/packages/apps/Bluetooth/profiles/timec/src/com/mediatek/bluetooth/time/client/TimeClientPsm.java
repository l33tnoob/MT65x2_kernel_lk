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

import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Message;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.psm.Psm;
import com.mediatek.bluetooth.psm.PsmConnection;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.psm.PsmService;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.ResponseCode;

public class TimeClientPsm extends Psm {

	private static final String TAG = "TimeClientPsm";

	private Handler mHandler;

	public TimeClientPsm(PsmService psmService) {
		// Time client can only connect with one server at the same time.
		super(BluetoothProfile.ID_TIMEC, psmService,
				TimeClientMsg.MSG_ID_BT_TIMEC_GROUP_START,
				TimeClientMsg.MSG_ID_BT_TIMEC_GROUP_END,
				1, 1);
	}

	@Override
	protected PsmServiceBinder createServiceBinder() {
		return new TimeClientService(this);
	}

	@Override
	protected void onServiceCreate() {
		super.onServiceCreate();
		onServiceBind();
	}

	@Override
	public int handleMessage(PsmMessage msg) {
		TimeClientLog.d(TAG, "Handle message: " + msg.getId());
		PsmConnection conn = getConnection(msg.getIndex());
		int state = conn.getCurrentState();

		switch (state) {
			case TimeClientConstants.TIMEC_STATE_NEW:
				return stateNew(conn, msg);
			case TimeClientConstants.TIMEC_STATE_CONNECTING:
				return stateConnecting(conn, msg);
			case TimeClientConstants.TIMEC_STATE_DISCONNECTING:
				return stateDisconnecting(conn, msg);
			case TimeClientConstants.TIMEC_STATE_CONNECTED:
				return stateConnected(conn, msg);
			case TimeClientConstants.TIMEC_STATE_PROCESSING:
				return stateProcessing(conn, msg);
			case TimeClientConstants.TIMEC_STATE_REQUESTING:
				return stateRequesting(conn, msg);
			case TimeClientConstants.TIMEC_STATE_CANCELLING:
				return stateCancelling(conn, msg);
			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.UNDEFINED_STATE);
		}
	}

	private int stateNew(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			case TimeClientMsg.MSG_ID_BT_TIMEC_CONNECT_REQ:
				TimeClientLog.d(TAG, "Handle TIMEC_CONNECT_REQ");
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_CONNECTING);
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	private int stateConnecting(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			case TimeClientMsg.MSG_ID_BT_TIMEC_CONNECT_CNF:
				TimeClientLog.d(TAG, "Handle TIMEC_CONNECT_CNF");
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_CONNECTED);
				recvMessage(msg);
			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	private int stateDisconnecting(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			case TimeClientMsg.MSG_ID_BT_TIMEC_DISCONNECT_IND:
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_NEW);
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);
			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	private int stateConnected(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			case TimeClientMsg.MSG_ID_BT_TIMEC_DISCONNECT_REQ:
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_DISCONNECTING);
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeClientMsg.MSG_ID_BT_TIMEC_DISCONNECT_IND:
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_NEW);
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeClientMsg.MSG_ID_BT_TIMEC_GET_CTTIME_REQ:
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_PROCESSING);
				sendMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			case TimeClientMsg.MSG_ID_BT_TIMEC_GET_CTTIME_CNF:
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_CONNECTED);
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	private int stateProcessing(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			case TimeClientMsg.MSG_ID_BT_TIMEC_GET_CTTIME_CNF:
				conn.setCurrentState(TimeClientConstants.TIMEC_STATE_CONNECTED);
				recvMessage(msg);
				return ResultCode.create(ResultCode.STATUS_SUCCESS);

			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	private int stateRequesting(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

	private int stateCancelling(PsmConnection conn, PsmMessage msg) {
		int msg_id = msg.getId();
		switch (msg_id) {
			default:
				return ResultCode.create(ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE);
		}
	}

}
