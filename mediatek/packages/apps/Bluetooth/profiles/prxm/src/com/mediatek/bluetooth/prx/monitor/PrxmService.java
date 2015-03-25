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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfileManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ResultReceiver;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.prx.monitor.PrxmProvider.BluetoothPrxmDeviceMetaData;
import com.mediatek.bluetooth.psm.Psm;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.psm.PsmServiceBinder;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.service.IBluetoothPrxm;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.ConvertUtils;

/**
 * @author Jerry Hsu
 */
public class PrxmService extends IBluetoothPrxm.Stub implements PsmServiceBinder {

	private Psm prxmPsm;				// profile state machine object
	private HashMap<String, Integer> connMap;	// Map<bdaddr, connId>
	private ArrayList<BluetoothPrxmDevice> registeredDevices;	// registered device (reporter)
	private ContentResolver contentResolver;	// content provider for prxm

	/**
	 * Constructor
	 * 
	 * @param prxmPsm
	 */
	public PrxmService( PrxmPsm prxmPsm ){

		BtLog.i( "PrxmService()[+]" );

		// init member attributes
		this.prxmPsm = prxmPsm;
		this.connMap = new HashMap<String, Integer>( PrxmConstants.PRXM_MAXIMUM_CONNECTION_COUNT );
		this.registeredDevices = new ArrayList<BluetoothPrxmDevice>( PrxmConstants.PRXM_MAXIMUM_CONNECTION_COUNT );
		this.contentResolver = this.prxmPsm.getService().getContentResolver();

		// load saved objects
		Cursor c = this.contentResolver.query( BluetoothPrxmDeviceMetaData.CONTENT_URI, null, null, null, null );
		BluetoothPrxmDevice[] dl = PrxmProvider.fetchDevices(c);
		BluetoothAdapter bta = BluetoothAdapter.getDefaultAdapter();
		Set<BluetoothDevice> bondedDevices = bta.getBondedDevices();
		for( BluetoothPrxmDevice d : dl ){

			try {
				// check if stack keep this device (un-paired devices will be removed after BT reboot)
				if( bondedDevices.contains(bta.getRemoteDevice(d.getAddress())) ){

					this.registerDevice( d );	// register device
				}
				else {
					// remote from content provider
					this.contentResolver.delete( 
						BluetoothPrxmDeviceMetaData.CONTENT_URI,
						BluetoothPrxmDeviceMetaData.DEVICE_ADDR + "='" + d.getAddress() + "'", null );
				}
			}
			catch( Exception ex ){

				BtLog.e( "loading saved BluetoothDevice[" + d + "] error: ", ex );
			}
		}
	}

	// implement interface: PsmServiceBinder
	public void onServiceBind(){

		// send broadcast to profile manager
		PrxmUtils.broadcastProfileManagerActivationState( this.prxmPsm.getService(), PrxmUtils.STATE_ENABLED );
	}
	// implement interface: PsmServiceBinder
	public void onServiceDestroy(){

		// send broadcast for connected devices
		for( BluetoothPrxmDevice device : this.registeredDevices ){
			if( device.getCurrentState() == PrxmConstants.PRXM_STATE_CONNECTED ){
				PrxmUtils.broadcastProfileManagerStateChanged( this.prxmPsm.getService(), device.getDevice(), false );
			}
		}
		// send broadcast to profile manager
		PrxmUtils.broadcastProfileManagerActivationState( this.prxmPsm.getService(), PrxmUtils.STATE_DISABLED );
	}
	// implement interface: PsmServiceBinder
	public IBinder getBinder(){
		return this;
	}
	// implement interface: PsmServiceBinder
	public void onMessageReceived( PsmMessage message ){

		// prepare connection for message processing
		PrxmPsmConn conn = (PrxmPsmConn)this.prxmPsm.getConnection( message.getIndex() );
		if( conn == null ){

			BtLog.e( "PrxmService.onMessageReceived() error: can't get PrxmPsmConn for message[" + message.toPrintString() + "]" );
			return;
		}

		// handle message accroding to messageId
		byte rspcode;
		int mid = message.getId();
		switch( mid ){
			case PrxmMsg.MSG_ID_BT_PRXM_CONNECT_CNF:
				rspcode = message.getByte( PrxmMsg.PRXM_CONNECT_CNF_B_RSPCODE );
				// send profile-manager broadcast when state changed
				if( rspcode == ResponseCode.SUCCESS ){
					PrxmUtils.broadcastProfileManagerStateChanged( this.prxmPsm.getService(), conn.getDeviceInfo().getDevice(), true );
				}
				break;
			case PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_IND:
				rspcode = message.getByte( PrxmMsg.PRXM_DISCONNECT_IND_B_RSPCODE );
				// send profile-manager broadcast when state changed
				PrxmUtils.broadcastProfileManagerStateChanged( this.prxmPsm.getService(), conn.getDeviceInfo().getDevice(), false );
				break;
			case PrxmMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_CNF:
				rspcode = message.getByte( PrxmMsg.PRXM_SET_LINKLOSS_CNF_B_RSPCODE );
				break;
			default:
				rspcode = ResponseCode.UNKNOWN;
		}

		// dispatch to callback object if any.
		if( conn.getCallback() == null ){

			BtLog.w( "PrxmService: no callback for message: " + message.toPrintString() );
		}
		else {
			Bundle resultData = new Bundle();
			resultData.putByte( "rspcode", rspcode );
			conn.getCallback().send( mid, resultData );
		}
	}

	/**
	 * get registered devices
	 */
	public synchronized BluetoothPrxmDevice[] getRegisteredDevices() throws RemoteException {

		if( this.registeredDevices.size() > 0 ){

			return this.registeredDevices.toArray( new BluetoothPrxmDevice[this.registeredDevices.size()] );
		}
		else {
			return new BluetoothPrxmDevice[0];
		}
	}

	public BluetoothDevice[] getConnectedDevices() throws RemoteException {

		if( this.registeredDevices.size() > 0 ){

			ArrayList<BluetoothDevice> res = new ArrayList<BluetoothDevice>(this.registeredDevices.size());
			for( BluetoothPrxmDevice device : this.registeredDevices ){

				if( device.getCurrentState() == PrxmConstants.PRXM_STATE_CONNECTED ){

					res.add( device.getDevice() );
				}
			}
			return res.toArray( new BluetoothDevice[res.size()] );
		}
		else {
			return new BluetoothDevice[0];
		}
	}
	
	/**
	 * register device, true: new device / false: existing device
	 * 
	 * @param device
	 * @return
	 * @throws RemoteException
	 */
	public synchronized boolean registerDevice( BluetoothDevice device ) throws RemoteException {

		if( Options.LL_DEBUG ){
			BtLog.d( "registerDevice()[+]: device[" + device + "]"  );
		}

		if( this.connMap.containsKey( device.getAddress() ) ){

			return false;	// existing connection (according to bdaddr)
		}
		else {
			// new object for new device
			BluetoothPrxmDevice deviceInfo = new BluetoothPrxmDevice( device );
			deviceInfo.setLinkLossLevel( PrxmConstants.PRXM_DEFAULT_LINK_LOSS_LEVEL );
			deviceInfo.setPathLossLevel( PrxmConstants.PRXM_DEFAULT_PATH_LOSS_LEVEL );
			deviceInfo.setPathLossThreshold( PrxmConstants.PRXM_DEFAULT_PATH_LOSS_THRESHOLD );
			this.registerDevice( deviceInfo );
			// insert into content provider
			Uri uri = this.contentResolver.insert( BluetoothPrxmDeviceMetaData.CONTENT_URI, PrxmProvider.getContentValues(deviceInfo) );
			if( uri == null ){
				BtLog.e( "PrxmService.registerDevice() fail: can't insert device[" + device + "]" );
			}
			else {
				deviceInfo.setId( Integer.parseInt( uri.getLastPathSegment() ) );
			}
			return true;
		}
	}

	/**
	 * internal use only
	 * 
	 * @param deviceInfo
	 */
	private void registerDevice( BluetoothPrxmDevice deviceInfo ){

		Integer connId = this.prxmPsm.registerConnection( new PrxmPsmConn( deviceInfo ) );
		this.connMap.put( deviceInfo.getAddress(), connId );
		this.registeredDevices.add( deviceInfo );
	}

	/**
	 * unregister device
	 * 
	 * @param device
	 * @throws RemoteException
	 */
	public synchronized void unregisterDevice( BluetoothDevice device ) throws RemoteException {

		String address = device.getAddress();
		BtLog.d( "unregisterDevice()[+]: device[" + address + "]" );

		// disconnect device before unregister it
		try {
			BluetoothPrxmDevice prxr = this.getDeviceInfo( address );
			if( prxr.getCurrentState() == PrxmConstants.PRXM_STATE_CONNECTED ){
				this.disconnect(address);
				PrxmUtils.broadcastProfileManagerStateChanged( this.prxmPsm.getService(), device, false );
			}
		}
		catch( Exception ex ){

			BtLog.e( "disconnect device[" + address + "] error: ", ex );
		}

		// remote from content provider
		this.contentResolver.delete( 
				BluetoothPrxmDeviceMetaData.CONTENT_URI,
				BluetoothPrxmDeviceMetaData.DEVICE_ADDR + "='" + address + "'", null );

		// remove from connection list
		this.registeredDevices.remove( new BluetoothPrxmDevice( device ) );

		// remove from connection map and then unregister connection 
		this.prxmPsm.unregisterConnection( this.connMap.remove( address ) );
	}

	/**
	 * register callback for specific device
	 * 
	 * @param bdaddr
	 * @param callback
	 * @return
	 * @throws RemoteException
	 */
	public synchronized boolean registerDeviceCallback( String bdaddr, ResultReceiver callback ) throws RemoteException {

		BtLog.d( "registerDeviceCallback()[+]"  );

		PrxmPsmConn conn = this.getConnection( bdaddr );
		if( conn.getCallback() != null ){

			BtLog.w( "registerDeviceCallback() will override existing callback for device: " + bdaddr );
		}
		conn.setCallback( callback );
		return true;
	}

	/**
	 * unregister callback for specific device
	 * 
	 * @param bdaddr
	 * @throws RemoteException
	 */
	public synchronized void unregisterDeviceCallback( String bdaddr ) throws RemoteException {

		BtLog.d( "unregisterDeviceCallback()[+]"  );

		this.getConnection( bdaddr ).setCallback( null );
	}

	/**
	 * get PrxmDeviceInfo for specific device(bdaddr) 
	 */
	public BluetoothPrxmDevice getDeviceInfo( String bdaddr ) throws RemoteException {

		return this.getConnection( bdaddr ).getDeviceInfo();
	}

	/**
	 * get device state for profile manager
	 */
	public int getProfileManagerState( String bdaddr ) throws RemoteException {

		// unregistered device
		Integer connId = this.connMap.get( bdaddr );
		if( connId == null ){
			return BluetoothProfileManager.STATE_DISCONNECTED;
		}

		int localState = this.getConnection( bdaddr ).getDeviceInfo().getCurrentState();
		switch( localState ){
			case PrxmConstants.PRXM_STATE_NEW:		return BluetoothProfileManager.STATE_DISCONNECTED;
			case PrxmConstants.PRXM_STATE_CONNECTING:	return BluetoothProfileManager.STATE_CONNECTING;
			case PrxmConstants.PRXM_STATE_DISCONNECTING:	return BluetoothProfileManager.STATE_DISCONNECTING;
			case PrxmConstants.PRXM_STATE_CONNECTED:		return BluetoothProfileManager.STATE_CONNECTED;
			default:					return BluetoothProfileManager.STATE_UNKNOWN;
		}
	}
	
	/**
	 * config path-loss level locally => will be used when path-loss
	 */
	public int configPathLossLevel( String bdaddr, byte level ) throws RemoteException {

		BluetoothPrxmDevice device = this.getDeviceInfo( bdaddr );
		device.setPathLossLevel( level );

		// update content provider
		Uri uri = Uri.withAppendedPath( BluetoothPrxmDeviceMetaData.CONTENT_URI, Integer.toString( device.getId() ) );
		ContentValues cv = new ContentValues(1);
		cv.put( BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL, device.getPathLossLevel() );
		this.contentResolver.update( uri, cv, null, null );

		return ResultCode.rspcode( ResultCode.STATUS_SUCCESS );
	}

	/**
	 * config path-loss level locally => will be used to check path-loss
	 */
	public int configPathLossThreshold( String bdaddr, byte threshold ) throws RemoteException {

		BluetoothPrxmDevice device = this.getDeviceInfo( bdaddr );
		device.setPathLossThreshold( threshold );

		// update content provider
		Uri uri = Uri.withAppendedPath( BluetoothPrxmDeviceMetaData.CONTENT_URI, Integer.toString( device.getId() ) );
		ContentValues cv = new ContentValues(1);
		cv.put( BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD, device.getPathLossThreshold() );
		this.contentResolver.update( uri, cv, null, null );

		return ResultCode.rspcode( ResultCode.STATUS_SUCCESS );
	}

	/**
	 * connect to specified proximity-reporter
	 */
	public int connectByProfileManager( BluetoothDevice device ) throws RemoteException {

		if( Options.LL_DEBUG ){
			BtLog.d( "connectByProfileManager()[+]: device[" + device + "]" );
		}

		// device registered => start device setting activity
		Intent intent = new Intent( PrxmDeviceSettingActivity.ACTION_START );
		intent.setFlags( Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS );
		intent.putExtra( BluetoothDevice.EXTRA_DEVICE, device );
		this.prxmPsm.getService().startActivity( intent );
		//PrxmDeviceMgmtActivity.startConnectActivity( this.prxmFsm.getService(), device );
		return ResultCode.create( ResultCode.STATUS_SUCCESS, ResponseCode.SUCCESS );
	}

	/**
	 * connect to specified proximity-reporter
	 */
	public int connect( String bdaddr ) throws RemoteException {

		if( Options.LL_DEBUG ){
			BtLog.d( "connect()[+]: bdaddr[" + bdaddr + "]" );
		}
		PsmMessage req = new PsmMessage( PrxmMsg.PRXM_CONNECT_REQ, this.getConnId(bdaddr) );
		req.setByteArray( PrxmMsg.PRXM_CONNECT_REQ_BA_ADDR, PrxmMsg.PRXM_CONNECT_REQ_BL_ADDR, ConvertUtils.convertBdAddr(bdaddr) );
		return this.prxmPsm.handleMessage( req );
	}

	/**
	 * disconnect current connected proximity-reporter
	 */
	public int disconnect( String bdaddr ) throws RemoteException {

		if( Options.LL_DEBUG ){
			BtLog.d( "disconnect()[+]: bdaddr[" + bdaddr + "]" );
		}
		return this.prxmPsm.handleMessage( new PsmMessage( PrxmMsg.PRXM_DISCONNECT_REQ, this.getConnId(bdaddr) ) );
	}

	/**
	 * get connected proximity-reporter's capability
	 */
	public int getRemoteCapability( String bdaddr ) throws RemoteException {

		return this.prxmPsm.handleMessage( new PsmMessage( PrxmMsg.PRXM_GET_CAPABILITY_REQ, this.getConnId( bdaddr ) ) );
	}

	/**
	 * get connected proximity-reporter's tx-power
	 */
	public int getRemoteTxPower( String bdaddr ) throws RemoteException {

		return this.prxmPsm.handleMessage( new PsmMessage( PrxmMsg.PRXM_GET_REMOTE_TXPOWER_REQ, this.getConnId( bdaddr ) ) );
	}

	/**
	 * set connected proximity-reporter's link-loss level
	 */
	public int setLinkLoss( String bdaddr, byte level ) throws RemoteException {

		// update content provider
		BluetoothPrxmDevice device = this.getDeviceInfo( bdaddr );
		Uri uri = Uri.withAppendedPath( BluetoothPrxmDeviceMetaData.CONTENT_URI, Integer.toString( device.getId() ) );
		ContentValues cv = new ContentValues(1);
		cv.put( BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL, level );
		this.contentResolver.update( uri, cv, null, null );

		PsmMessage req = new PsmMessage( PrxmMsg.PRXM_SET_LINKLOSS_REQ, this.getConnId( bdaddr ) );
		req.setByte( PrxmMsg.PRXM_SET_LINKLOSS_REQ_B_LEVEL, level );
		return this.prxmPsm.handleMessage( req );
	}

	/**
	 * set connected proximity-reporter's path-loss level
	 */
	public int setPathLoss( String bdaddr, byte level ) throws RemoteException {

		PsmMessage req = new PsmMessage( PrxmMsg.PRXM_SET_PATHLOSS_REQ, this.getConnId( bdaddr ) );
		req.setByte( PrxmMsg.PRXM_SET_PATHLOSS_REQ_B_LEVEL, level );
		return this.prxmPsm.handleMessage( req );
	}

	/**
	 * get connection id by device address
	 * 
	 * @param bdaddr
	 * @return
	 */
	private int getConnId( String bdaddr ){

		Integer connId = this.connMap.get( bdaddr );
		if( connId == null ){

			BtLog.e( "getConnId failed - invalid bdaddr: " + bdaddr );
			throw new IllegalArgumentException( "invalid bdaddr:[" + bdaddr + "]" );
		}
		else {
			return connId;
		}
	}

	/**
	 * get connection by device address
	 * 
	 * @param bdaddr
	 * @return
	 */
	private PrxmPsmConn getConnection( String bdaddr ){

		PrxmPsmConn result = (PrxmPsmConn)this.prxmPsm.getConnection( this.getConnId(bdaddr) );
		if( result == null ){

			BtLog.e( "getConnection failed - bdaddr: " + bdaddr );
			throw new IllegalArgumentException( "invalid bdaddr:[" + bdaddr + "]" );
		}
		return result;
	}
}
