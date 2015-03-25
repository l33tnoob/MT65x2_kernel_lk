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

import android.bluetooth.BluetoothAdapter;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.psm.Psm;
import com.mediatek.bluetooth.psm.PsmConnection;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmService;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.ConvertUtils;

/**
 * @author Jerry Hsu
 * 
 * Proximity Reporter PSM (Profile State Machine) Implementation
 * 
 * 1. quickly reg + unreg ? (queue request until state changed)
 * 2. response by return v.s mock cnf
 * 
 * 					NEW	REGISTERING	UNREGISTERING	CONNECTABLE
 * 	PRXR_REGISTER_REQ		O	X		X		X
 * 	PRXR_REGISTER_CNF		X	O		X		X
 * 	PRXR_DEREGISTER_REQ		X	O		X		O
 * 	PRXR_DEREGISTER_CNF		X	X		O		X
 * 	PRXR_AUTHORIZE_IND		X	X		X		O
 * 	PRXR_AUTHORIZE_RSP		X	X		X		X
 * 	PRXR_CONNECT_IND		X	X		X		X
 * 	PRXR_DISCONNECT_REQ		X	X		X		X
 * 	PRXR_DISCONNECT_IND		X	X		O		X
 * 	PRXR_PATHLOSS_IND		X	X		X		X
 * 	PRXR_LINKLOSS_IND		X	X		X		X
 * 	PRXR_UPDATE_TXPOWER_REQ		X	X		X		X
 * 	PRXR_UPDATE_TXPOWER_CNF		X	X		X		X
 * 
 * 					CONNECTING	DISCONNECTING	CONNECTED
 * 	PRXR_REGISTER_REQ		X		X		X
 * 	PRXR_REGISTER_CNF		X		X		X
 * 	PRXR_DEREGISTER_REQ		O		O		O
 * 	PRXR_DEREGISTER_CNF		X		X		X
 * 	PRXR_AUTHORIZE_IND		X		X		X
 * 	PRXR_AUTHORIZE_RSP		O		X		X
 * 	PRXR_CONNECT_IND		O		X		X
 * 	PRXR_DISCONNECT_REQ		X		X		O
 * 	PRXR_DISCONNECT_IND		O?		O		O
 * 	PRXR_PATHLOSS_IND		X		O?		O
 * 	PRXR_LINKLOSS_IND		X		O?		O
 * 	PRXR_UPDATE_TXPOWER_REQ		X		X		O
 * 	PRXR_UPDATE_TXPOWER_CNF		X		O?		O
 */
public class PrxrPsm extends Psm {

	/**
	 * Constructor
	 * 
	 * @param psmService
	 */
	public PrxrPsm( PsmService psmService ){

		super( BluetoothProfile.ID_PRXR,
			psmService,
			PrxrMsg.MSG_ID_BT_PRXR_GROUP_START,
			PrxrMsg.MSG_ID_BT_PRXR_GROUP_END,
			PrxrConstants.PRXR_INITIAL_CONNECTION_COUNT,
			PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT );
	}

	@Override
	protected PsmServiceBinder createServiceBinder(){

		return new PrxrService(this);
	}

	@Override
	protected void onServiceCreate(){

		super.onServiceCreate();
		this.onServiceBind();
	}

	@Override
	public int handleMessage( PsmMessage message ){

		// check parameter: message / connection existing (no registration)
		PsmConnection connection;
		if( message == null || (connection = this.getConnection( message.getIndex() ) ) == null ){

			if( message == null ){
				BtLog.w( "PrxrPsm.handleMessage(): null message" );
			}
			else {
				BtLog.w( "PrxrPsm.handleMessage(): can't find connection for index[" + message.getIndex() + "]" );
			}
			return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.NO_CONNECTION );
		}

		int state = connection.getCurrentState();
		switch( state ){
			case PrxrConstants.PRXR_STATE_NEW:
				return this.stateNew( connection, message );
			case PrxrConstants.PRXR_STATE_REGISTERING:
				return this.stateRegistering( connection, message );
			case PrxrConstants.PRXR_STATE_UNREGISTERING:
				return this.stateUnregistering( connection, message );
			case PrxrConstants.PRXR_STATE_CONNECTABLE:
				return this.stateConnectable( connection, message );
			case PrxrConstants.PRXR_STATE_CONNECTING:
				return this.stateConnecting( connection, message );
			case PrxrConstants.PRXR_STATE_DISCONNECTING:
				return this.stateDisconnecting( connection, message );
			case PrxrConstants.PRXR_STATE_CONNECTED:
				return this.stateConnected( connection, message );
			default:
				// undefined state
				BtLog.e( "PrxrPsm.handleMessage() - undefined state: " + state );
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.UNDEFINED_STATE );
		}
	}

	private int stateNew( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateNew()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_REGISTER_REQ:
				// change state and send out request
				connection.setCurrentState( PrxrConstants.PRXR_STATE_REGISTERING );
				this.sendMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			default:
				// invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateRegistering( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateRegistering()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_REGISTER_CNF:
				// change state according to rspcode
				byte regRsp = message.getByte( PrxrMsg.PRXR_REGISTER_CNF_B_RSPCODE );
				// ResponseCode.IN_USE means stack is registered (disconnect indication will be received if connections exist) 
				if( regRsp == ResponseCode.SUCCESS || regRsp == ResponseCode.IN_USE ){

					message.setByte( PrxrMsg.PRXR_REGISTER_CNF_B_RSPCODE, ResponseCode.SUCCESS );
					connection.setCurrentState( PrxrConstants.PRXR_STATE_CONNECTABLE );
				}
				else {
					connection.setCurrentState( PrxrConstants.PRXR_STATE_NEW );
				}
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_REQ:
				// make sure unregister can always be executed
				return this.handleUnregisterReq( connection, message );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateUnregistering( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateUnregistering()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_DISCONNECT_IND:
				// report to PrxrService first and it can access state and peer-device info
				this.recvMessage( message );
				// reset peer device
				((PrxrPsmConn)connection).setPeerDevice( null );
				// send out unregister-req
				this.sendMessage( new PsmMessage( PrxrMsg.PRXR_DEREGISTER_REQ, message.getIndex() ) );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_CNF:
				// change state and deliver message
				connection.setCurrentState( PrxrConstants.PRXR_STATE_NEW );
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateConnectable( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateConnectable()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_AUTHORIZE_IND:
				// change state and deliver message
				connection.setCurrentState( PrxrConstants.PRXR_STATE_CONNECTING );
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxrMsg.MSG_ID_BT_PRXR_CONNECT_IND:
				// can skip authorize_ind and enter connected directly
				return this.handleConnectInd( connection, message );
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_REQ:
				return this.handleUnregisterReq( connection, message );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateConnecting( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateConnecting()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_AUTHORIZE_RSP:
				// change state according to rspcode
				byte rspcode = message.getByte( PrxrMsg.PRXR_AUTHORIZE_RSP_B_RSPCODE );
				if( rspcode != ResponseCode.SUCCESS ){
					connection.setCurrentState( PrxrConstants.PRXR_STATE_CONNECTABLE );	
				}
				this.sendMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxrMsg.MSG_ID_BT_PRXR_CONNECT_IND:
				return this.handleConnectInd( connection, message );
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_REQ:
				return this.handleUnregisterReq( connection, message );
			case PrxrMsg.MSG_ID_BT_PRXR_DISCONNECT_IND:
				// TODO [L3] maybe it will not happen at all (according to stack's definition)
				return this.handleDisconnectInd( connection, message );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateDisconnecting( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateDisconnecting()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_DISCONNECT_IND:
				return this.handleDisconnectInd( connection, message );
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_REQ:
				return this.handleUnregisterReq( connection, message);
			case PrxrMsg.MSG_ID_BT_PRXR_PATHLOSS_IND:
			case PrxrMsg.MSG_ID_BT_PRXR_LINKLOSS_IND:
			case PrxrMsg.MSG_ID_BT_PRXR_UPDATE_TXPOWER_CNF:
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateConnected( PsmConnection connection, PsmMessage message ){

		BtLog.d( "stateConnected()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_DISCONNECT_REQ:
				connection.setCurrentState( PrxrConstants.PRXR_STATE_DISCONNECTING );
				this.sendMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxrMsg.MSG_ID_BT_PRXR_DISCONNECT_IND:
				return this.handleDisconnectInd( connection, message );
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_REQ:
				return this.handleUnregisterReq( connection, message );
			case PrxrMsg.MSG_ID_BT_PRXR_PATHLOSS_IND:
			case PrxrMsg.MSG_ID_BT_PRXR_LINKLOSS_IND:
			case PrxrMsg.MSG_ID_BT_PRXR_UPDATE_TXPOWER_CNF:
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}


	/**
	 * handle unregister request
	 * 
	 * @param conn
	 * @param message
	 * @return
	 */
	private int handleUnregisterReq( PsmConnection conn, PsmMessage message ){

		// change state and send message
		conn.setCurrentState( PrxrConstants.PRXR_STATE_UNREGISTERING );

		// enter disconnect state first
		this.sendMessage( new PsmMessage( PrxrMsg.PRXR_DISCONNECT_REQ, message.getIndex() ) );

		// unregister-req message will be sent after disconnect_ind received under PRXR_STATE_UNREGISTERING

		// message sent out indication
		return ResultCode.create( ResultCode.STATUS_SUCCESS );
	}

	/**
	 * handle connect indication 
	 * 
	 * @param conn
	 * @param message
	 * @return
	 */
	private int handleConnectInd( PsmConnection conn, PsmMessage message ){
		
		conn.setCurrentState( PrxrConstants.PRXR_STATE_CONNECTED );
		// set peer device when connected
		PrxrPsmConn ppc = (PrxrPsmConn)conn;
		String peerAddr = ConvertUtils.convertBdAddr( message.getByteArray( PrxrMsg.PRXR_CONNECT_IND_BA_ADDR, PrxrMsg.PRXR_CONNECT_IND_BL_ADDR ) );
		ppc.setPeerDevice( BluetoothAdapter.getDefaultAdapter().getRemoteDevice(peerAddr) );
		// deliver message
		this.recvMessage( message );
		return ResultCode.create( ResultCode.STATUS_SUCCESS );
	}
	
	/**
	 * handle disconnect indication
	 * 
	 * @param conn
	 * @param message
	 * @return
	 */
	private int handleDisconnectInd( PsmConnection conn, PsmMessage message ){

		// update state
		conn.setCurrentState( PrxrConstants.PRXR_STATE_CONNECTABLE );
		// report to PrxrService first and it can access state and peer-device info
		this.recvMessage( message );
		// reset peer device
		((PrxrPsmConn)conn).setPeerDevice( null );
		return ResultCode.create( ResultCode.STATUS_SUCCESS );
	}
}
