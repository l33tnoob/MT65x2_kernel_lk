package com.mediatek.bluetooth;

import android.bluetooth.IBluetoothManager;
import android.bluetooth.IBluetoothManagerCallback;
import android.bluetooth.IBluetooth;

/**
  * A extension class for BluetoothAdapter
  * 
  * @hide
  *
  */
public class BluetoothAdapterEx{

    /** Static contants to indicate expected remote device type when discovering */
    /**
     * @hide
     * @internal
     */
    public static final int TYPE_BR_EDR_ONLY    = 0;
    /**
     * @hide
     * @internal
     */
    public static final int TYPE_LE_ONLY        = 1;
    /**
     * @hide
     * @internal
     */
    public static final int TYPE_DUAL_MODE      = 2;
}
