package android.bluetooth;
import android.bluetooth.BluetoothDevice;

interface IBluetoothMap {
	BluetoothDevice[] getConnectedDevices();
	boolean isConnected(in BluetoothDevice device);
	boolean disconnect(in BluetoothDevice device);
	int getState(in BluetoothDevice device);
}

