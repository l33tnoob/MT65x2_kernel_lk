package com.mediatek.bluetooth.service;

import android.os.ResultReceiver;

interface IBluetoothTimes
{
	void registerCallback(in ResultReceiver callback);
	boolean unregisterCallback(in ResultReceiver callback);

	int enableService();
	int disableService(); 

	int getServiceState();

	// MSG_ID_BT_TIMES_CONNECT_IND / MSG_ID_BT_TIMES_CONNECT_RSP
	int responseAuthorizeInd(int connId, byte rspcode);

	// MSG_ID_BT_TIMES_DISCONNECT_REQ / MSG_ID_BT_TIMES_DISCONNECT_IND
	int disconnect(int connId);
}
