package com.mediatek.bluetooth;

import android.util.Log;

import android.os.IBinder;
import android.os.ServiceManager;
import android.os.RemoteException;

import android.bluetooth.IBluetooth;
import android.bluetooth.BluetoothAdapter;

/**
  * A extension class for BluetoothAdapter
  * 
  * @hide
  *
  */
public class BluetoothAdapterEx{

    private final String TAG = "BluetoothAdapterEx";
    private final IBluetooth mService;
    private static BluetoothAdapterEx sAdapterEx;

    /**
      * use to get the singleton of BluetoothAdapterEx
      *
      * @hide
      * @internal
      *
      * @return an instance of BluetoothAdapterEx
      *
      */     
    public static synchronized BluetoothAdapterEx getDefaultAdapterEx(){
        if (null == sAdapterEx) {
            IBinder b = ServiceManager.getService(BluetoothAdapter.BLUETOOTH_SERVICE);
            if (b != null) {
                IBluetooth service = IBluetooth.Stub.asInterface(b);
                sAdapterEx = new BluetoothAdapterEx(service);
            }
        }
        return sAdapterEx;    
    }

    /**
      * constructor
      *
      * @hide
      * @internal
      *
      * @param an instance of BluetoothService
      *
      */ 
    private BluetoothAdapterEx(IBluetooth service) {
        if (service == null) {
            throw new IllegalArgumentException("service is null");
        }
        mService = service;
    }

    /**
      * get the status of the SSPDebugMode
      *
      * @hide
      * @internal
      *
      * @return true if debug mode is on otherwise debug mode is off
      * 
      */
    public boolean getSSPDebugMode() {
        try {
            return mService.getSSPDebugMode();
        } catch (RemoteException e) {Log.e(TAG, "", e);}
        return false;
    }

    /**
      * set the debug mode
      *
      * @hide
      * @internal
      *
      * @return true if success otherwise failed
      *
      */    
    public boolean setSSPDebugMode(boolean on) {
        try {
            return mService.setSSPDebugMode(on);
        } catch (RemoteException e) {Log.e(TAG, "", e);}
        return false;
    }    
}
