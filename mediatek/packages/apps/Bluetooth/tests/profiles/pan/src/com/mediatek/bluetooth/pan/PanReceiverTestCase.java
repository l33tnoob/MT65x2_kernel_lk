
package com.mediatek.bluetooth.pan;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothPan;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;
import android.util.Log;

import junit.framework.Assert;

import java.lang.reflect.Field;

public class PanReceiverTestCase extends InstrumentationTestCase {

    private static final String TAG = "[BT][PAN-UT][PanReceiverTestCase]";

    private static final boolean DEBUG = true;

    private Context mTestContext;

    /**
    * Which will be called after each test case<br>
    * Do some clean work.
    */
    @Override
    protected void tearDown() throws Exception {
        printDebugLog("tearDown -------");
        super.tearDown();
    }

    /**
    * Which will be called before each test case<br>
    * Get context which to call receiver onReceive
    */
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        printDebugLog("setUp ++++++++");
        mTestContext = this.getInstrumentation().getContext();
        if (mTestContext == null) {
            printErrorLog("[API:setUp] mTestContext is null");
            Assert.assertNotNull(mTestContext);
        }
    }

    /**
    * Intent is Bluetooth state change to Bluetooth_state_on
    */
    public void test01BluetoothStateChangeToOn() {
        Intent intent = new Intent("android.bluetooth.adapter.action.STATE_CHANGED");
        intent.putExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.STATE_ON);
        BluetoothPanReceiver mReceiver = new BluetoothPanReceiver();
        mReceiver.onReceive(mTestContext, intent);
    }

    /**
    * Intent is device change to BOOT_COMPLETED
    */
    public void test02BootCompleted() {
        Intent intent = new Intent(Intent.ACTION_BOOT_COMPLETED);
        BluetoothPanReceiver mReceiver = new BluetoothPanReceiver();
        mReceiver.onReceive(mTestContext, intent);
    }

    /**
    * Call BluetoothPanService private receiver which get it through reflector<br>
    * Intent is BluetoothAdapter.ACTION_STATE_CHANGED 
    */
    @SuppressWarnings("unchecked")
    public void test03PanServiceReceiver1() {
        Intent intent = new Intent(BluetoothDevice.ACTION_NAME_CHANGED);
        try {
            BluetoothPanService ms = new BluetoothPanService();
            Class cls = Class.forName("com.mediatek.bluetooth.pan.BluetoothPanService");
            printDebugLog("[API:test03PanServiceReceiver1] cls is : " + cls.toString());
            Assert.assertNotNull(cls);
            Field field = cls.getDeclaredField("mReceiver");
            printDebugLog("[API:test03PanServiceReceiver1] field is : " + field.toString());
            Assert.assertNotNull(field);
            field.setAccessible(true);
            BroadcastReceiver receiver = (BroadcastReceiver) field.get(ms);
            Assert.assertNotNull(receiver);
            receiver.onReceive(mTestContext, intent);
            intent.setAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            intent.putExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.STATE_TURNING_OFF);
            receiver.onReceive(mTestContext, intent);
        } catch (ClassNotFoundException ex) {
            printErrorLog(ex.toString());
        } catch (NoSuchFieldException ex) {
            printErrorLog(ex.toString());
        } catch (SecurityException ex) {
            printErrorLog(ex.toString());
        } catch (IllegalAccessException ex) {
            printErrorLog(ex.toString());
        }
    }

    /**
    * Call BluetoothPanAlertActivity private receiver which get it through reflector<br>
    * Intent is BluetoothPan.ACTION_CONNECTION_STATE_CHANGED
    */
    @SuppressWarnings("unchecked")
    public void test04PanAlertReceiver1() {
        Intent intent = new Intent(BluetoothPan.ACTION_CONNECTION_STATE_CHANGED);
        try {
            BluetoothPanAlert alert = new BluetoothPanAlert();
            Class cls = Class.forName("com.mediatek.bluetooth.pan.BluetoothPanAlert");
            printDebugLog("[API:test04PanAlertReceiver1] cls is : " + cls.toString());
            Assert.assertNotNull(cls);
            Field field = cls.getDeclaredField("mReceiver");
            printDebugLog("[API:test04PanAlertReceiver1] field is : " + field.toString());
            Assert.assertNotNull(field);
            field.setAccessible(true);
            BroadcastReceiver receiver = (BroadcastReceiver) field.get(alert);
            Assert.assertNotNull(receiver);
            receiver.onReceive(mTestContext, intent);
        } catch (ClassNotFoundException ex) {
            printErrorLog(ex.toString());
        } catch (NoSuchFieldException ex) {
            printErrorLog(ex.toString());
        } catch (SecurityException ex) {
            printErrorLog(ex.toString());
        } catch (IllegalAccessException ex) {
            printErrorLog(ex.toString());
        }
    }

    /**
    * Print debug log
    */
    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    /**
    * Print error log
    */
    private void printErrorLog(String msg) {
        Log.e(TAG, msg);
    }
}
