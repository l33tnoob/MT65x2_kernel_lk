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

import android.os.Handler;
import android.os.Message;
import android.os.Handler.Callback;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.psm.Psm;
import com.mediatek.bluetooth.psm.PsmConnection;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmService;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * 					NEW	CONNECTING	DISCONNECTING	CONNECTED	BUSY(x)
 *	PRXM_CONNECT_REQ		O	X		X		X		X
 *	PRXM_CONNECT_CNF		X	O		X		X		X
 *	PRXM_DISCONNECT_REQ		X	X		X		O		O
 *	PRXM_DISCONNECT_IND		X	O		O		O		O
 *	PRXM_GET_CAPABILITY_REQ		X	X		X		O		X
 *	PRXM_GET_CAPABILITY_CNF		X	O(C)		X		O(B)		O
 *	PRXM_GET_REMOTE_TXPOWER_REQ	X	X		X		O		X
 *	PRXM_GET_REMOTE_TXPOWER_CNF	X	O(C)		X		O(B)		O
 *	PRXM_SET_PATHLOSS_REQ		X	X		X		O		X
 *	PRXM_SET_PATHLOSS_CNF		X	X		X		O(B)		O
 *	PRXM_SET_LINKLOSS_REQ		X	X		X		O		X
 *	PRXM_SET_LINKLOSS_CNF		X	X		X		O(B)		O
 *	PRXM_GET_RSSI_REQ		X	X		X		O		O
 *	PRXM_GET_RSSI_CNF		X	X		X		O(B)		O
 *
 */
public class PrxmPsm extends Psm implements Callback {

	private Handler handler;
	
	/**
	 * Constructor
	 * 
	 * @param psmService
	 */
	public PrxmPsm( PsmService psmService ){

		super( BluetoothProfile.ID_PRXM,
			psmService,
			PrxmMsg.MSG_ID_BT_PRXM_GROUP_START,
			PrxmMsg.MSG_ID_BT_PRXM_GROUP_END,
			PrxmConstants.PRXM_INITIAL_CONNECTION_COUNT,
			PrxmConstants.PRXM_MAXIMUM_CONNECTION_COUNT );
	}

	@Override
	protected PsmServiceBinder createServiceBinder(){

		// delay handler initialization util service-bind
		this.handler = new Handler( this.getHandlerLooper(), this );

		return new PrxmService(this);
	}

	/**
	 * handle message for PSM framework
	 */
	@Override
	public int handleMessage( PsmMessage message ){

		// check parameter: message / connection existing (no registration)
		PrxmPsmConn connection;
		if( message == null || (connection = (PrxmPsmConn)this.getConnection( message.getIndex() ) ) == null ){

			if( message == null ){
				BtLog.w( "PrxmPsm.handleMessage(): null message" );
			}
			else {
				BtLog.w( "PrxmPsm.handleMessage(): can't find connection for index[" + message.getIndex() + "]" );
			}
			return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.NO_CONNECTION );
		}

		int state = connection.getCurrentState();
		switch( state ){
			case PrxmConstants.PRXM_STATE_NEW:
				return this.stateNew( connection, message );
			case PrxmConstants.PRXM_STATE_CONNECTING:
				return this.stateConnecting( connection, message );
			case PrxmConstants.PRXM_STATE_CONNECTED:
				return this.stateConnected( connection, message );
			case PrxmConstants.PRXM_STATE_DISCONNECTING:
				return this.stateDisconnecting( connection, message );
			default:
				// undefined state
				BtLog.e( "PrxmPsm.handleMessage() - undefined state: " + state );
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.UNDEFINED_STATE );
		}
	}

	private int stateNew( PrxmPsmConn connection, PsmMessage message ){

		BtLog.d( "stateNew()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxmMsg.MSG_ID_BT_PRXM_CONNECT_REQ:
				// change state and send out request
				connection.setCurrentState( PrxmConstants.PRXM_STATE_CONNECTING );
				this.sendMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			default:
				// invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateConnecting( PrxmPsmConn connection, PsmMessage message ){

		BtLog.d( "stateConnecting()[+]" );

		int rspcode;
		int messageId = message.getId();
		switch( messageId ){
			case PrxmMsg.MSG_ID_BT_PRXM_CONNECT_CNF:
				rspcode = message.getByte( PrxmMsg.PRXM_CONNECT_CNF_B_RSPCODE );
				if( rspcode == ResponseCode.SUCCESS ){
					// set remote device link loss level
					PsmMessage pm = new PsmMessage( PrxmMsg.PRXM_SET_LINKLOSS_REQ, connection.getConnId() );
					pm.setByte( PrxmMsg.PRXM_SET_LINKLOSS_REQ_B_LEVEL, connection.getDeviceInfo().getLinkLossLevel() );
					this.sendMessage( pm );
					return ResultCode.create( ResultCode.STATUS_PENDING );
				}
				else {
					// connect request fail
					connection.setCurrentState( PrxmConstants.PRXM_STATE_NEW );
					this.recvMessage( message );
					return ResultCode.create( ResultCode.STATUS_SUCCESS );
				}
			case PrxmMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_CNF:
				rspcode = message.getByte( PrxmMsg.PRXM_SET_LINKLOSS_CNF_B_RSPCODE );
				if( rspcode != ResponseCode.DISCONNECT ){
					// send request to get capability
					this.sendMessage( new PsmMessage( PrxmMsg.PRXM_GET_CAPABILITY_REQ, connection.getConnId() ) );
					return ResultCode.create( ResultCode.STATUS_PENDING );
				}
				else {
					// command is canceled when the previous command is failed
					return ResultCode.create( ResultCode.STATUS_SUCCESS );
				}
			case PrxmMsg.MSG_ID_BT_PRXM_GET_CAPABILITY_CNF:
				rspcode = message.getByte( PrxmMsg.PRXM_GET_CAPABILITY_CNF_B_RSPCODE );
				if( rspcode != ResponseCode.DISCONNECT ){
					// save capability into connection
					connection.getDeviceInfo().setCapability( (byte)message.getInt( PrxmMsg.PRXM_GET_CAPABILITY_CNF_I_CAPABILITY ) );
					// send request to get txpower
					this.sendMessage( new PsmMessage( PrxmMsg.PRXM_GET_REMOTE_TXPOWER_REQ, connection.getConnId() ) );
					return ResultCode.create( ResultCode.STATUS_PENDING );
				}
				else {
					// command is canceled when the previous command is failed
					return ResultCode.create( ResultCode.STATUS_SUCCESS );
				}
			case PrxmMsg.MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_CNF:
				rspcode = message.getByte( PrxmMsg.PRXM_GET_REMOTE_TXPOWER_CNF_B_RSPCODE );
				if( rspcode != ResponseCode.DISCONNECT ){
					// update state
					connection.setCurrentState( PrxmConstants.PRXM_STATE_CONNECTED );
					// save txpower into connection
					connection.getDeviceInfo().setRemoteTxPower( message.getByte( PrxmMsg.PRXM_GET_REMOTE_TXPOWER_CNF_B_TXPOWER ) );
					// connected => start monitoring
					this.startMonitor( connection );
					// response connect success
					PsmMessage conCnf = new PsmMessage( PrxmMsg.PRXM_CONNECT_CNF, message.getIndex() );
					conCnf.setByte( PrxmMsg.PRXM_CONNECT_CNF_B_RSPCODE, (byte)ResponseCode.SUCCESS );
					this.recvMessage( conCnf );
					// return success
					return ResultCode.create( ResultCode.STATUS_SUCCESS );
				}
				else {
					// command is canceled when the previous command is failed
					return ResultCode.create( ResultCode.STATUS_SUCCESS );
				}
			case PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_IND:
				return this.handleDisconnect( connection, message );
			default:
				// invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateDisconnecting( PrxmPsmConn connection, PsmMessage message ){

		BtLog.d( "stateDisconnecting()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_IND:
				return this.handleDisconnect( connection, message );
			default:
				// report as invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int stateConnected( PrxmPsmConn connection, PsmMessage message ){

		//BtLog.d( "stateConnected()[+]" );

		int messageId = message.getId();
		switch( messageId ){
			case PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_IND:
				return this.handleDisconnect( connection, message );
			case PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_REQ:
			case PrxmMsg.MSG_ID_BT_PRXM_GET_CAPABILITY_REQ:
			case PrxmMsg.MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_REQ:
			case PrxmMsg.MSG_ID_BT_PRXM_GET_RSSI_REQ:
				// send out request
				this.sendMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxmMsg.MSG_ID_BT_PRXM_SET_PATHLOSS_REQ:
				// no pending request -> send out new message
				if( !connection.isWaitingForSetPathLossCnf ){
					connection.isWaitingForSetPathLossCnf = true;
					connection.lastPathLossLevel = message.getByte( PrxmMsg.PRXM_SET_PATHLOSS_REQ_B_LEVEL );
					this.sendMessage( message );
					return ResultCode.create( ResultCode.STATUS_SUCCESS );
				}
				else {
					byte newPathLossLevel = message.getByte( PrxmMsg.PRXM_SET_PATHLOSS_REQ_B_LEVEL );
					if( connection.lastPathLossLevel != newPathLossLevel ){

						connection.isLastPathLossLevelChanged = true;
						connection.lastPathLossLevel = newPathLossLevel;
					}
					return ResultCode.create( ResultCode.STATUS_PENDING );
				}
			case PrxmMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_REQ:
				// keep new loss level as pending (will set back to deviceInfo when CNF is received)
				connection.setLinkLossLevelPending( message.getByte( PrxmMsg.PRXM_SET_LINKLOSS_REQ_B_LEVEL ) );
				this.sendMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxmMsg.MSG_ID_BT_PRXM_GET_CAPABILITY_CNF:
				connection.getDeviceInfo().setCapability( (byte)message.getInt( PrxmMsg.PRXM_GET_CAPABILITY_CNF_I_CAPABILITY ) );
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxmMsg.MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_CNF:
				connection.getDeviceInfo().setRemoteTxPower( message.getByte( PrxmMsg.PRXM_GET_REMOTE_TXPOWER_CNF_B_TXPOWER ) );
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxmMsg.MSG_ID_BT_PRXM_GET_RSSI_CNF:
				connection.getDeviceInfo().setCurrentRssi( message.getByte( PrxmMsg.PRXM_GET_RSSI_CNF_B_RSSI ) );
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxmMsg.MSG_ID_BT_PRXM_SET_PATHLOSS_CNF:
				// check rspcode and determine submit or rollback pending value
				int splRsp = message.getByte( PrxmMsg.PRXM_SET_PATHLOSS_CNF_B_RSPCODE );
				if( splRsp == ResponseCode.SUCCESS && !connection.isLastPathLossLevelChanged ){

					// clear pending request
					connection.lastPathLossLevel = PrxmConstants.PRXM_ALERT_LEVEL_NULL;
					connection.isWaitingForSetPathLossCnf = false;
					this.recvMessage( message );
				}
				else {
					// retry the last request
					PsmMessage retry = new PsmMessage( PrxmMsg.PRXM_SET_PATHLOSS_REQ, connection.getConnId() );
					retry.setByte( PrxmMsg.PRXM_SET_PATHLOSS_REQ_B_LEVEL, connection.lastPathLossLevel );
					connection.isLastPathLossLevelChanged = false;
					this.sendMessage( retry );
				}
				return ResultCode.create( ResultCode.STATUS_SUCCESS );
			case PrxmMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_CNF:
				// check rspcode and determine submit or rollback pending value
				int sllRsp = message.getByte( PrxmMsg.PRXM_SET_LINKLOSS_CNF_B_RSPCODE );
				if( sllRsp == ResponseCode.SUCCESS ){

					// submit pending value
					connection.getDeviceInfo().setLinkLossLevel( connection.getLinkLossLevelPending() );
				}
				connection.setLinkLossLevelPending( PrxmConstants.PRXM_ALERT_LEVEL_NULL );
				this.recvMessage( message );
				return ResultCode.create( ResultCode.STATUS_SUCCESS );				
			default:
				// invalid state
				return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	private int handleDisconnect( PsmConnection conn, PsmMessage message ){

		// update state
		conn.setCurrentState( PrxmConstants.PRXM_STATE_NEW );

		// prepare result and callback to mmi layer
		this.recvMessage( message );

		// disconnected => stop monitoring
		this.stopMonitor( (PrxmPsmConn)conn );

		// return success
		return ResultCode.create( ResultCode.STATUS_SUCCESS );
	}

	/**
	 * start to monitor the path loss status for specific connection
	 * 
	 * @param conn
	 */
	protected void startMonitor( PrxmPsmConn conn ){

		BtLog.d( "startMonitor()[+]" );
		
		// check reporter capability
		if( conn.getDeviceInfo().getCapability() == PrxmConstants.PRXM_CAP_NONE ){

			BtLog.d( "proximity reporter[" + conn.getDeviceInfo().getName() + "] doesn't support txpower service" );
			// doesn't need to monitor reporter's rssi (reporter doesn't support)
			return;
		}

		// update flag in connection
		conn.setMonitoring( true );

		// start monitor for this device
		this.handler.sendMessage( this.handler.obtainMessage( 0, conn ) );
	}

	/**
	 * stop path loss status monitoring
	 * 
	 * @param conn
	 */
	protected void stopMonitor( final PrxmPsmConn conn ){

		conn.setMonitoring( false );
	}

	/**
	 * implement path-loss status monitoring
	 */
	public boolean handleMessage( Message msg ){

		BtLog.d( "handleMessage()[+]: handle proximity monitor update message" );
		
		// get current connection object
		PrxmPsmConn conn = (PrxmPsmConn)msg.obj;

		// stopped if current connection is stopped
		if( !conn.isMonitoring() )	return true;

		// check path loss
		BluetoothPrxmDevice deviceInfo = conn.getDeviceInfo();

		if( Options.LL_DEBUG ){

			BtLog.d( "connId[" + conn.getConnId() + "]: path-loss[" + PrxmUtils.getPathLoss( deviceInfo.getCurrentRssi(), deviceInfo.getRemoteTxPower() ) + "], threshold[" + deviceInfo.getPathLossThreshold() + "]" );
		}

		// check update status
		if( deviceInfo.isUpdateDone() ){

			// check path-loss and get level
			boolean oldPathLost = deviceInfo.isPathLost();
			boolean newPathLost = deviceInfo.checkPathLoss( PrxmUtils.getPathLoss( deviceInfo.getCurrentRssi(), deviceInfo.getRemoteTxPower() ) );
			byte pathLossLevel = ( newPathLost ? deviceInfo.getPathLossLevel() : PrxmConstants.PRXM_ALERT_LEVEL_NO );

			// update remote attribute (need to update when true/true because the level maybe changed )
			if( oldPathLost || newPathLost ){

				PsmMessage req = new PsmMessage( PrxmMsg.PRXM_SET_PATHLOSS_REQ, conn.getConnId() );
				req.setByte( PrxmMsg.PRXM_SET_PATHLOSS_REQ_B_LEVEL, pathLossLevel );
				this.handleMessage( req );
			}

			// reset state
			deviceInfo.resetUpdateState();
		}

		// update txpower & rssi ( send out new request )
		//this.handleMessage( new PsmMessage( PrxmMsg.PRXM_GET_REMOTE_TXPOWER_REQ, conn.getConnId() ) );
		this.handleMessage( new PsmMessage( PrxmMsg.PRXM_GET_RSSI_REQ, conn.getConnId() ) );

		// schedule next run with delay
		this.handler.sendMessageDelayed( this.handler.obtainMessage( 0, conn ), conn.getMonitorDelay() );
		return true;
	}
}
