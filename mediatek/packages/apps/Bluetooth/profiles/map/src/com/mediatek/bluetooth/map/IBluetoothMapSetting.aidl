package com.mediatek.bluetooth.map;
import com.mediatek.bluetooth.map.IBluetoothMapSettingCallback;
interface IBluetoothMapSetting {
	void enableServer();
	void disableServer();
	boolean isEnabled();
	boolean registerSim(int value);
	void unregisterSim(int value);
	boolean replaceAccount(long value);
	int[] getSims();
	long getEmailAccount();
	void registerCallback(IBluetoothMapSettingCallback cb);    
	void unregisterCallback(IBluetoothMapSettingCallback cb);
}