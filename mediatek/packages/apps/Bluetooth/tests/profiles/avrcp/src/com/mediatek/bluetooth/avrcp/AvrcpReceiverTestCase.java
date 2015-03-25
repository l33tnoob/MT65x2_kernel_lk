
package com.mediatek.bluetooth.avrcp;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;
import android.util.Log;

public class AvrcpReceiverTestCase extends InstrumentationTestCase {

    private static final String TAG = "[BT][AVRCP-UT][AvrcpReceiverTestCase]";

    private static final boolean DEBUG = true;

    private Context mTestContext = null;

    /**
     * setUp method which will be called before each test case
     */
    public void setUp() throws Exception {
        super.setUp();
        printDebugLog("setUp +++++++++++");
        mTestContext = this.getInstrumentation().getContext();
        if (mTestContext == null) {
            printErrLog("[API:setUp] mTestContext is null");
            return;
        }
    }

    /**
     * tearDown will be called after each test case
     */
    public void tearDown() throws Exception {
        printDebugLog("tearDown --------");
        mTestContext = null;
        super.tearDown();
    }

    /**
     * Intent is BluetoothAdapter.ACTION_STATE_CHANGED
     */
    public void test01ReceiveStateChange() {
        Intent intent = new Intent(BluetoothAdapter.ACTION_STATE_CHANGED);
        intent.putExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.STATE_ON);
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        receiver.onReceive(mTestContext, intent);
    }

    /**
     * Intent is android.provider.Telephony.SECRET_CODE
     */
    public void test02ReceiveSecretCode() {
        Intent intent = new Intent("android.provider.Telephony.SECRET_CODE");
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        receiver.onReceive(mTestContext, intent);
    }

    /**
     * Intent is android.mediatek.bluetooth.avrcp.pts
     */
    public void test03ReceivePts() {
        Intent intent = new Intent("android.mediatek.bluetooth.avrcp.pts");
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        receiver.onReceive(mTestContext, intent);
    }

    /**
     * Intent is android.mediatek.bluetooth.avrcp.connect
     */
    public void test04ReceiveConnect1() {
        Intent intent = new Intent("android.mediatek.bluetooth.avrcp.connect");
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        receiver.onReceive(mTestContext, intent);
    }
    
    /**
     * Intent is android.mediatek.bluetooth.avrcp.connect
     */
    public void test05ReceiveConnect2() {
        Intent intent = new Intent("android.mediatek.bluetooth.avrcp.connect");
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        BluetoothAvrcpReceiver.sAvrcpServer = new BluetoothAvrcpService();
        receiver.onReceive(mTestContext, intent);
    }

    /**
     * Intent is android.mediatek.bluetooth.avrcp.disconnect
     * And sAvrcpServer is null
     */
    public void test06ReceiveDisconnect1() {
        Intent intent = new Intent("android.mediatek.bluetooth.avrcp.disconnect");
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        receiver.onReceive(mTestContext, intent);
    }

    /**
     * Intent is android.mediatek.bluetooth.avrcp.disconnect
     * And sAvrcpServer is not null
     */
    public void test07ReceiveDisconnect2() {
        Intent intent = new Intent("android.mediatek.bluetooth.avrcp.disconnect");
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        BluetoothAvrcpReceiver.sAvrcpServer = new BluetoothAvrcpService();
        receiver.onReceive(mTestContext, intent);
    }

    public void test08DestroySelf() {
        BluetoothAvrcpReceiver receiver = new BluetoothAvrcpReceiver();
        BluetoothAvrcpService service = new BluetoothAvrcpService();
        BluetoothAvrcpReceiver.sAvrcpServer = service;
        receiver.destroyMyself(service);
    }

    /**
     * Record the debug log which message is msg
     * @param msg
     */
    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    /**
     * Record the error log which message is msg
     * @param msg
     */
    private void printErrLog(String msg) {
        Log.e(TAG, msg);
    }
}
