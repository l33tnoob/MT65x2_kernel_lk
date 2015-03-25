package com.mediatek.bluetooth.service;

import android.bluetooth.BluetoothDevice;
import android.os.ResultReceiver;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;

interface IBluetoothPrxm
{
// **************************************************************************
// Sync API - don't need responses from remote device
// **************************************************************************

	// get registered device list
	BluetoothPrxmDevice[] getRegisteredDevices();
	BluetoothDevice[] getConnectedDevices();

	// register / unregister bluetooth device 
	boolean registerDevice( in BluetoothDevice device );
	void unregisterDevice( in BluetoothDevice device );

	// register / unregister device callback (ResultReceiver)
	boolean registerDeviceCallback( String bdaddr, in ResultReceiver callback );
	void unregisterDeviceCallback( String bdaddr );

	// get state
	BluetoothPrxmDevice getDeviceInfo( String bdaddr );
	int getProfileManagerState( String bdaddr );

	// config local attribute
	int configPathLossLevel( String bdaddr, byte level );
	int configPathLossThreshold( String bdaddr, byte threshold );

// **************************************************************************
// Async API - need responses from remote device
// **************************************************************************

	int connectByProfileManager( in BluetoothDevice device );

	// MSG_ID_BT_PRXM_CONNECT_REQ / MSG_ID_BT_PRXM_CONNECT_CNF
	// @return response code
	int connect( String bdaddr );

	// MSG_ID_BT_PRXM_DISCONNECT_REQ / MSG_ID_BT_PRXM_DISCONNECT_IND
	// @return response code
	int disconnect( String bdaddr );

	// MSG_ID_BT_PRXM_GET_CAPABILITY_REQ / MSG_ID_BT_PRXM_GET_CAPABILITY_CNF
	// @return capabilities
	int getRemoteCapability( String bdaddr );

	// MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_REQ / MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_CNF
	// @return tx power
	int getRemoteTxPower( String bdaddr );

	// MSG_ID_BT_PRXM_SET_PATHLOSS_REQ / MSG_ID_BT_PRXM_SET_PATHLOSS_CNF
	// @return response code
	int setPathLoss( String bdaddr, byte level );

	// MSG_ID_BT_PRXM_SET_LINKLOSS_REQ / MSG_ID_BT_PRXM_SET_LINKLOSS_CNF
	// @return response code
	int setLinkLoss( String bdaddr, byte level );
}