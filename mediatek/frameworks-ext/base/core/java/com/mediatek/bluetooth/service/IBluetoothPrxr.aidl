package com.mediatek.bluetooth.service;

import android.bluetooth.BluetoothDevice;
import android.os.ResultReceiver;

interface IBluetoothPrxr
{
	void registerCallback( in ResultReceiver callback );
	boolean unregisterCallback( in ResultReceiver callback );

	int enableService();
	int disableService(); 

	BluetoothDevice[] getConnectedDevices();

	int getServiceState();
	int getProfileManagerState( String bdaddr );

	// MSG_ID_BT_PRXR_CONNECT_IND / MSG_ID_BT_PRXR_CONNECT_RSP
	int responseAuthorizeInd( int connId, byte rspcode );

	// MSG_ID_BT_PRXR_DISCONNECT_REQ / MSG_ID_BT_PRXR_DISCONNECT_IND
	int disconnect( int connId );
	int disconnectByAddr( String bdaddr );
}
