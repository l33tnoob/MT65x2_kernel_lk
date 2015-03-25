package com.mediatek.bluetooth.service;

import android.os.ResultReceiver;
import android.bluetooth.BluetoothDevice;

interface IBluetoothTimec
{
	void registerCallback(in ResultReceiver callback);
	boolean unregisterCallback(in ResultReceiver callback);

	int connect(in BluetoothDevice device);

	int disconnect();

	int getServerTime();

	int getAutoConfig();

	int getDstInfo();

	int requestServerUpdate();

	int cancelServerUpdate();

	int getUpdateStatus();
}
