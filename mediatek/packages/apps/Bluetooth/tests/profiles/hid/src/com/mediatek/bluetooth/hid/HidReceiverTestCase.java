package com.mediatek.bluetooth.hid;
import android.app.Instrumentation;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.test.InstrumentationTestCase;
import android.util.Log;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

public class HidReceiverTestCase extends InstrumentationTestCase {

    private static final String TAG = "[BT][HIDUT][HidReceiverTestCase]";

    private Instrumentation mInstrumentation;

    private Context mTestContext;

    private BluetoothDevice mDevice;

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        printLog("setUp ++++++++");
        mInstrumentation = getInstrumentation();
        mTestContext = mInstrumentation.getTargetContext();
        BluetoothHidService.sUtState = true;
        printLog("setUp --------");
    }

    public void test01DeviceSelected()throws Exception {
        printLog("test01_DeviceSelected");
        Intent intent = new Intent();
        intent.setAction(BluetoothDevicePicker.ACTION_DEVICE_SELECTED);
        Bundle bundle = new Bundle();
        constructBluetoothDevice("7C:ED:8D:68:48:E8");
        bundle.putParcelable(BluetoothDevice.EXTRA_DEVICE, mDevice);
        intent.putExtras(bundle);
        BluetoothHidReceiver mReceiver = new BluetoothHidReceiver();
        mReceiver.onReceive(mTestContext, intent);
        constructBluetoothDevice("9F:ED:8D:68:48:E8");
        mReceiver.onReceive(mTestContext, intent);

    }

    public void test02SecretCodeReceive() {
        printLog("test02_SecretCodeReceive");
        Intent intent = new Intent(android.provider.Telephony.Intents.SECRET_CODE_ACTION);
        BluetoothHidReceiver mReceiver = new BluetoothHidReceiver();
        mReceiver.onReceive(mTestContext, intent);
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        printLog("tearDown ++++");
        super.tearDown();
    }

    @SuppressWarnings("unchecked")
    private void constructBluetoothDevice(String address) throws Exception {
        try {
            Constructor con = BluetoothDevice.class.getDeclaredConstructor(String.class);
            Log.i(TAG, "Constructor:" + con);
            con.setAccessible(true);
            mDevice = (BluetoothDevice) con.newInstance(address);
            Log.i(TAG, "BluetoothDevice:" + mDevice);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }

    }

    private void printLog(String str) {
        if (true) {
            Log.d(TAG, str);
        }
    }
}
