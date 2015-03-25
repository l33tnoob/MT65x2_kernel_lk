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

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfileManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ResultReceiver;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.service.IBluetoothPrxr;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * 1. API
 * 	- register	(req/cnf)
 * 	- unregister	(req/cnf)
 * 	- connect	(ind/rsp)
 * 	- disconnect	(req/ind)
 * 	- pathloss	(ind)
 * 	- linkloss	(ind)
 * 
 * 2. MMI
 * 	- Proximity-Reporter On/Off Checkbox (Advanced Setting)
 * 
 * 3. State
 * 	- New
 * 	- Registering
 * 	- Connectable (Registered)
 * 	- Connecting
 * 	- Connected
 * 	- Disconnecting
 *
 */
public class PrxrService extends IBluetoothPrxr.Stub implements PsmServiceBinder {

	private static final String PRXR_PREFERENCE_NAME = "prxr_pref";
	private static final String PRXR_PREFERENCE_KEY_STATE = "prxr_pref_state";
	
	private static final int STATE_NEW = 0;
	private static final int STATE_ENABLING = 1;
	private static final int STATE_DISABLING = 2;
	private static final int STATE_ENABLED = 3;

	/**
	 * profile fsm service object
	 */
	private PrxrPsm prxrPsm;

	/**
	 * service state: new / enabling / disabling / enabled (registered, connectable)
	 */
	private int serviceState;

	/**
	 * connection id for all connections
	 */
	private int[] connId;
	private byte[] connState;

	/**
	 * MMI registered callback (e.g. Settings)
	 */
	private List<ResultReceiver> registeredCallbacks;

	/**
	 * Constructor
	 * 
	 * @param prxrPsm
	 */
	public PrxrService( PrxrPsm prxrPsm ){

		BtLog.i( "PrxrService()[+]" );
		this.prxrPsm = prxrPsm;
		this.serviceState = STATE_NEW;
		this.connId = new int[PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT];
		this.connState = new byte[PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT];
		for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){

			this.connId[i] = -1;
		}
		this.registeredCallbacks = new LinkedList<ResultReceiver>();
	}

	// implement interface: PsmServiceBinder
	public void onServiceBind(){

		// restore state from SharedPreferences
		SharedPreferences sp = this.prxrPsm.getService().getSharedPreferences( PRXR_PREFERENCE_NAME, Context.MODE_PRIVATE );
		int state = sp.getInt( PRXR_PREFERENCE_KEY_STATE, STATE_NEW );
		if( state == STATE_ENABLED ){

			this.enableServiceImpl();
		}
	}
	// implement interface: PsmServiceBinder
	public void onServiceDestroy(){

		if( this.serviceState == STATE_ENABLED ){

			// report disconnect
			for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){
				// check connection is initialized
				if( connId[i] < 0 )	continue;
				PrxrPsmConn ppc = (PrxrPsmConn)this.prxrPsm.getConnection(connId[i]);
				if( ppc != null ){
					if( ppc.getCurrentState() == PrxrConstants.PRXR_STATE_CONNECTED ){
						PrxrUtils.broadcastProfileManagerStateChanged( this.prxrPsm.getService(), ppc.getPeerDevice(), false );
					}
				}
			}

			this.resetService();

			// report disabled
			PrxrUtils.broadcastProfileManagerActivationState( this.prxrPsm.getService(), PrxrUtils.STATE_DISABLED );
		}
	}

	// implement interface: PsmServiceBinder
	public IBinder getBinder(){
		return this;
	}
	// implement interface: PsmServiceBinder - handle message from stack
	public void onMessageReceived( PsmMessage message ){

		PrxrPsmConn ppc = (PrxrPsmConn)this.prxrPsm.getConnection( message.getIndex() );
		if( ppc == null ){

			BtLog.e( "PrxrService.onMessageReceived() error: can't find PrxrPsmConn for message[" + message.toPrintString() + "]" );
			return;
		}
		Intent intent;
		int messageId = message.getId();
		switch( messageId ){
			case PrxrMsg.MSG_ID_BT_PRXR_REGISTER_CNF:
				this.handleRegisterCnf( message );
				break;
			case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_CNF:
				this.handleUnregisterCnf( message );
				break;
			case PrxrMsg.MSG_ID_BT_PRXR_AUTHORIZE_IND:
				try {
					// TODO [L3] popup ui for user interaction
					this.responseAuthorizeInd( message.getIndex(), ResponseCode.SUCCESS );
				}
				catch( RemoteException re ){

					BtLog.e( "responseConnectInd() error.", re );
				}
				break;
			case PrxrMsg.MSG_ID_BT_PRXR_CONNECT_IND:
				// notify profile manager connection is established 
				PrxrUtils.broadcastProfileManagerStateChanged( this.prxrPsm.getService(), ppc.getPeerDevice(), true );
				// send broadcast intent to cancel link-loss notification if any
				intent = new Intent( PrxrConstants.ACTION_LINK_LOSS );
				intent.putExtra( PrxrConstants.EXTRA_ALERT_LEVEL, PrxrConstants.PRXR_ALERT_LEVEL_NO );
				intent.putExtra( BluetoothDevice.EXTRA_DEVICE, ppc.getPeerDevice() );
				this.prxrPsm.getService().sendBroadcast( intent );
				break;
			case PrxrMsg.MSG_ID_BT_PRXR_DISCONNECT_IND:
				// notify profile manager connection is established 
				PrxrUtils.broadcastProfileManagerStateChanged( this.prxrPsm.getService(), ppc.getPeerDevice(), false );
				break;
			case PrxrMsg.MSG_ID_BT_PRXR_LINKLOSS_IND:
				if( ppc.getCurrentState() != PrxrConstants.PRXR_STATE_UNREGISTERING ){
					// create and send broadcast intent for link-loss
					intent = new Intent( PrxrConstants.ACTION_LINK_LOSS );
					intent.putExtra( PrxrConstants.EXTRA_ALERT_LEVEL, message.getByte( PrxrMsg.PRXR_LINKLOSS_IND_B_LEVEL ) );
					intent.putExtra( BluetoothDevice.EXTRA_DEVICE, ppc.getPeerDevice() );
					this.prxrPsm.getService().sendBroadcast( intent );
				}
				break;
			case PrxrMsg.MSG_ID_BT_PRXR_PATHLOSS_IND:
				// create and send broadcast intent for path-loss
				intent = new Intent( PrxrConstants.ACTION_PATH_LOSS );
				intent.putExtra( PrxrConstants.EXTRA_ALERT_LEVEL, message.getByte( PrxrMsg.PRXR_PATHLOSS_IND_B_LEVEL ) );
				intent.putExtra( BluetoothDevice.EXTRA_DEVICE, ppc.getPeerDevice() );
				this.prxrPsm.getService().sendBroadcast( intent );
				break;
			default:
				// translate callback result (from PsmMessage to Bundle) if necessary
				this.forwardResult( messageId, null );
		}
	}

	/**
	 * handle register-cnf message. possible situations:
	 * 1. report Success: when all connections are registered (judged by connection state).
	 * 2. waiting: when no fail connection && there's connection under registering state.
	 * 3. report Fail: when any connection is fail. 
	 * 
	 * @param message
	 */
	private void handleRegisterCnf( PsmMessage message ){

		BtLog.d( "handleRegisterCnf()[+]:" );

		// check state
		if( this.serviceState != STATE_ENABLING ){

			BtLog.w( "invalid state for register-cnf message, state:[" + this.serviceState + "]" );
			return;
		}

		// check all connection state
		int cid = message.getIndex();
		byte rspcode = message.getByte( PrxrMsg.PRXR_REGISTER_CNF_B_RSPCODE );
		this.connState[cid] = (byte)(( rspcode == ResponseCode.SUCCESS ) ? 1 : -1);

		boolean hasSuccess = false;
		for( int i=0; i<this.connId.length; i++ ){

			// wait for other connection's register-cnf
			if( connState[i] == 0 )	return;
			if( connState[i] == 1 ){

				hasSuccess = true;
			}
		}

		// report result
		if( hasSuccess ){

			// send broadcast to profile manager
			PrxrUtils.broadcastProfileManagerActivationState( this.prxrPsm.getService(), PrxrUtils.STATE_ENABLED );

			// success: no fail && no registering
			this.serviceState = STATE_ENABLED;
			Bundle result = new Bundle(1);
			result.putByte( "rspcode", ResponseCode.SUCCESS );
			this.forwardResult( message.getId(), result );
		}
		else {
			BtLog.w( "enable service failed" );

			// failed (hasFail == true)
			this.resetService();
			Bundle result = new Bundle(1);
			result.putByte( "rspcode", ResponseCode.FAILED );
			this.forwardResult( message.getId(), result );
		}
	}

	/**
	 * handle unregister-cnf message. 
	 * 1. report Success: when all connections are unregistered (doesn't check the rspcode in the message)
	 * 2. waiting: if any connection is not unregistered.
	 * 
	 * @param message
	 */
	private void handleUnregisterCnf( PsmMessage message ){

		BtLog.d( "handleUnregisterCnf()[+]" );

		// check state
		if( this.serviceState != STATE_DISABLING ){

			BtLog.w( "invalid state for unregister-cnf message, state:[" + this.serviceState + "]" );
			return;
		}

		// release connection resource
		byte cid = message.getIndex();
		this.prxrPsm.unregisterConnection( cid );
		this.connId[cid] = -1;

		// check all connection state and only callback when all connections are unregistered
		boolean hasPending = false;
		for( int i=0; i<connId.length; i++ ){

			// skip unregistered connection
			if( connId[i] < 0 )	continue;

			// has pending connection (unregistering)
			hasPending = true;
		}
		if( !hasPending ){

			// send broadcast to profile manager
			PrxrUtils.broadcastProfileManagerActivationState( this.prxrPsm.getService(), PrxrUtils.STATE_DISABLED );

			// change state when all connections are unregistered
			this.serviceState = STATE_NEW;

			// callback to mmi without rspcode (all Success)
			this.forwardResult( message.getId(), null );
		}
	}

	/**
	 * forward result to registered callback ResultReceiver(s)
	 * 
	 * @param resultCode
	 * @param resultData
	 */
	private void forwardResult( int resultCode, Bundle resultData ){
		
		for( ResultReceiver rr : this.registeredCallbacks ){

			rr.send( resultCode, resultData );
		}
	}

	/**
	 * unregister all connections
	 */
	private void resetService(){

		BtLog.d( "resetService()[+]" );

		// update state for unregister-cnf handling
		this.serviceState = STATE_DISABLING;

		// loop all connections
		boolean isDone = true;
		for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){

			// check connection is initialized
			if( connId[i] < 0 )	continue;

			try {
				// unregister (will do disconnect first at PSM)
				int resultCode = this.unregister( connId[i] );
				if( ResultCode.status( resultCode ) == ResultCode.STATUS_SUCCESS ){

					isDone = false;	// need to wait response
				}
				else {
					// no more cnf and release connection right now
					this.prxrPsm.unregisterConnection( connId[i] );
					this.connId[i] = -1;
				}
			}
			catch( Exception ex ){

				BtLog.e( "unregisterAllConnections.unregister(" + connId[i] + ") error:", ex );
			}
		}

		// update service state
		if( isDone ){

			this.serviceState = STATE_NEW;
		}
	}
	
	/**
	 * register callback object
	 * 
	 * @param callback
	 * @return
	 */
	public void registerCallback( ResultReceiver callback ) throws RemoteException {

		BtLog.d( "PrxrService.registerCallback()[+]" );

		if( callback != null ){

			this.registeredCallbacks.add( callback );
		}
	}

	/**
	 * unregister callback object
	 * 
	 * @param callback
	 * @return
	 */
	public boolean unregisterCallback( ResultReceiver callback ) throws RemoteException {

		BtLog.d( "PrxrService.unregisterCallback()[+]" );

		if( callback != null ){

			return this.registeredCallbacks.remove( callback );
		}
		return false;
	}

	/**
	 * enable service: register all connections and enter connectable state.
	 * 
	 * @return
	 * @throws RemoteException
	 */
	public synchronized int enableService() throws RemoteException {

		// save state into SharedPreference
		SharedPreferences sp = this.prxrPsm.getService().getSharedPreferences( PRXR_PREFERENCE_NAME, Context.MODE_PRIVATE );
		Editor editor = sp.edit();
		editor.putInt( PRXR_PREFERENCE_KEY_STATE, STATE_ENABLED );
		editor.commit();

		// do service enabling
		return this.enableServiceImpl();
	}
	
	private synchronized int enableServiceImpl(){

		BtLog.d( "enableServiceImpl()[+]" );

		if( this.serviceState == STATE_NEW ){

			// update state: new -> enabling
			this.serviceState = STATE_ENABLING;

			// reset conn state
			for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){

				this.connState[i] = 0;
			}
			
			// register all connections
			for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){

				// create new connection
				PrxrPsmConn conn = new PrxrPsmConn();
				try {
					// register connection (may throw runtime exception)
					this.connId[i] = this.prxrPsm.registerConnection( conn );

					// call register
					int resultCode = this.register( this.connId[i] );

					// fail if anyone failed
					if( ResultCode.status( resultCode ) != ResultCode.STATUS_SUCCESS ){

						// fatal error => reset service
						BtLog.e( "PrxrService.enableService() register failed: connId[" + connId[i] +"], resultCode[" + resultCode + "]" );
						this.resetService();

						// return non-success response and let mmi not to wait cnf
						return resultCode;
					}
				}
				catch( Exception ex ){

					BtLog.e( "register connection[" + i + "] failed.", ex );
				}
			}

			// return success and let mmi to wait cnf
			return ResultCode.create( ResultCode.STATUS_SUCCESS );
		}
		else {
			BtLog.e( "enableService() fail: invalid service state[" + this.serviceState + "]" );
			return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	/**
	 * disable service: unregister all connections.
	 * 
	 * @throws RemoteException
	 */
	public synchronized int disableService() throws RemoteException {

		BtLog.d( "disableService()" );

		// save state into SharedPreference
		SharedPreferences sp = this.prxrPsm.getService().getSharedPreferences( PRXR_PREFERENCE_NAME, Context.MODE_PRIVATE );
		Editor editor = sp.edit();
		editor.putInt( PRXR_PREFERENCE_KEY_STATE, STATE_NEW );
		editor.commit();

		// check service state
		if( this.serviceState == STATE_ENABLED ){

			// always success
			this.resetService();
			return ResultCode.create( ResultCode.STATUS_SUCCESS );
		}
		else {
			return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.INVALID_STATE );
		}
	}

	/**
	 * return the overall state of all connections: new / registering / connectable (registered)
	 * 
	 * @return
	 * @throws RemoteException
	 */
	public int getServiceState() throws RemoteException {

		// convert internal state to public prxr state
		switch( this.serviceState ){
			case STATE_NEW:
				return PrxrConstants.PRXR_STATE_NEW;
			case STATE_ENABLING:
				return PrxrConstants.PRXR_STATE_REGISTERING;
			case STATE_DISABLING:
				return PrxrConstants.PRXR_STATE_UNREGISTERING;
			case STATE_ENABLED:
				return PrxrConstants.PRXR_STATE_CONNECTABLE;
			default:
				// shouldn't happen
				return PrxrConstants.PRXR_STATE_NEW;
		}
	}

	/**
	 * get device state for profile manager
	 */
	public int getProfileManagerState( String bdaddr ) throws RemoteException {

		BtLog.d( "getProfileManagerState()[+]:" + bdaddr );

		PrxrPsmConn ppc = this.getConnection(bdaddr);
		if( ppc != null ){

			int state = ppc.getCurrentState();
			BtLog.d( "cuurent connection: state:" + state );
			switch( state ){
				case PrxrConstants.PRXR_STATE_NEW:		return BluetoothProfileManager.STATE_DISABLED;
				case PrxrConstants.PRXR_STATE_CONNECTABLE:	return BluetoothProfileManager.STATE_DISCONNECTED;	// BluetoothProfileManager.STATE_ENABLED;
				case PrxrConstants.PRXR_STATE_CONNECTED:	return BluetoothProfileManager.STATE_CONNECTED;
				case PrxrConstants.PRXR_STATE_CONNECTING:	return BluetoothProfileManager.STATE_CONNECTING;
				case PrxrConstants.PRXR_STATE_DISCONNECTING:	return BluetoothProfileManager.STATE_DISCONNECTING;
				case PrxrConstants.PRXR_STATE_REGISTERING:	return BluetoothProfileManager.STATE_ENABLING;
				case PrxrConstants.PRXR_STATE_UNREGISTERING:	return BluetoothProfileManager.STATE_DISABLING;
				default:					return BluetoothProfileManager.STATE_ABNORMAL;
			}			
		}
		return BluetoothProfileManager.STATE_DISCONNECTED;
		//return BluetoothProfileManager.STATE_ENABLED;
	}

	public BluetoothDevice[] getConnectedDevices() throws RemoteException {

		// report disconnect
		ArrayList<BluetoothDevice> res = new ArrayList<BluetoothDevice>(PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT);
		for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){

			// check connection is initialized
			if( connId[i] < 0 )	continue;

			PrxrPsmConn ppc = (PrxrPsmConn)this.prxrPsm.getConnection(connId[i]);
			if( ppc != null ){
				if( ppc.getCurrentState() == PrxrConstants.PRXR_STATE_CONNECTED ){
					res.add(ppc.getPeerDevice());
				}
			}
		}
		return res.toArray( new BluetoothDevice[res.size()] );
	}

	private PrxrPsmConn getConnection( String bdaddr ){

		if( bdaddr == null )	return null;

		// report disconnect
		for( int i=0; i<PrxrConstants.PRXR_MAXIMUM_CONNECTION_COUNT; i++ ){
			// check connection is initialized
			if( connId[i] < 0 )	continue;
			PrxrPsmConn ppc = (PrxrPsmConn)this.prxrPsm.getConnection(connId[i]);
			if( ppc != null ){
				BluetoothDevice device = ppc.getPeerDevice();
				if( device != null && bdaddr.equals(device.getAddress()) ){
					return ppc;
				}
			}
		}
		return null;
	}
	
	
	private int register( int connId ){

		BtLog.d( "register()[+]" );
		return this.prxrPsm.handleMessage( new PsmMessage( PrxrMsg.PRXR_REGISTER_REQ, connId ) );
	}

	private int unregister( int connId ){

		BtLog.d( "unregister()[+]" );
		return this.prxrPsm.handleMessage( new PsmMessage( PrxrMsg.PRXR_DEREGISTER_REQ, connId ) );
	}

	public int responseAuthorizeInd( int connId, byte rspcode ) throws RemoteException {

		PsmMessage rsp = new PsmMessage( PrxrMsg.PRXR_AUTHORIZE_RSP, connId );
		rsp.setByte( PrxrMsg.PRXR_AUTHORIZE_RSP_B_RSPCODE, rspcode );
		return this.prxrPsm.handleMessage( rsp );
	}
	
	public int disconnect( int connId ) throws RemoteException {

		BtLog.d( "disconnect()[+]" );
		return this.prxrPsm.handleMessage( new PsmMessage( PrxrMsg.PRXR_DISCONNECT_REQ, connId ) );
	}

	/**
	 * disconnect specified device
	 */
	public int disconnectByAddr( String bdaddr ) throws RemoteException {

		if( Options.LL_DEBUG ){
			BtLog.d( "disconnect()[+]: bdaddr[" + bdaddr + "]" );
		}
		PrxrPsmConn ppc = this.getConnection(bdaddr);
		if( ppc != null ){
			return this.disconnect( ppc.getConnId() );
		}
		return ResultCode.create( ResultCode.STATUS_FAILED, ResponseCode.DEVICE_NOT_FOUND );
	}
}
