
package com.mediatek.bluetooth.avrcp;

import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.os.Message;
import android.test.ServiceTestCase;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothUnitTestJni;
import com.mediatek.bluetooth.ICleanUpInterface;
import com.mediatek.bluetooth.Reflector;
import com.mediatek.bluetooth.Reflector.Parameter;

import junit.framework.Assert;
import java.util.BitSet;
import java.lang.NoSuchFieldException;

public class AvrcpServiceTestCase extends ServiceTestCase<BluetoothAvrcpService>
            implements ICleanUpInterface {

    private static final String TAG = "[BT][AVRCP-UT][AvrcpServiceTestCase]";

    private static final boolean DEBUG = true;

    private BluetoothUnitTestJni mJniManager = null;

    private BluetoothAvrcpService mService;

    private Intent mStartIntent = new Intent("com.mediatek.bluetooth.avrcp.BluetoothAvrcpService");

    private boolean mIsBluetoothOn;

    /**
     * AvrcpServiceTestCase constructor
     */
    public AvrcpServiceTestCase() {
        super(BluetoothAvrcpService.class);
    }

    /**
     * Do some initialization. Check the context is null or not. Check bluetooth is on or not.
     */
    public void setUp() throws Exception {
        super.setUp();
        printDebugLog("setUp +++++++");
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_AVRCP);
        if (BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            mIsBluetoothOn = true;
        }
        BluetoothAvrcpService.sUtState = true;
    }

    /**
     * Do clean work
     */
    public void tearDown() throws Exception {
        printDebugLog("tearDown -------");
        super.tearDown();
    }

/*    public void testRealCoverage() {
        printDebugLog("testRealCoverage enter");
        BluetoothUtCommonClass.sleep(10 * 1000 * 60);
    }
*/
    /**
     * Avrcp service enabled and disable. activateCnf and deactivateCnf are both STATE_ABNORMAL
     */
    public void test001EnableDisable1() {
        String[] jniLogs = {
                "Req;0;false;enableNative;1;0",
                "Cb;1;false;method_onActivateCnf;void;2;0;1",
                "Cb;1;false;method_onDeactivateCnf;void;2;0;1"
        };
        printDebugLog("[API:test01EnableDisable1] test01EnableDisable1 enter");
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mService = this.getService();
            Assert.assertNotNull(mService);
            sleep(1500);
        } else {
            printErrLog("[API:test01EnableDisable1] Bluetooth is disabled");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * Avrcp service enabled and disable. 
     * activateCnf and deactivateCnf are both STATE_ENABLE
     */
    public void test002EnableDisable2() {
        String[] jniLogs = {
                "Req;0;false;enableNative;1;0",
                "Cb;1;false;method_onActivateCnf;void;2;0;0",
                "Cb;1;false;method_onDeactivateCnf;void;2;0;0"
        };
        printDebugLog("[API:test02EnableDisable2] test02EnableDisable2 enter");
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mService = this.getService();
            Assert.assertNotNull(mService);
            sleep(1500);
        } else {
            printErrLog("[API:test02EnableDisable2] Bluetooth is disabled");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * enableNative return false
     */
    public void test003EnableDisable3() {
        String[] jniLogs = {
                "Req;0;false;enableNative;0;0",
                "Cb;1;false;method_onActivateCnf;void;2;0;0",
                "Cb;1;false;method_onDeactivateCnf;void;2;0;0"
        };
        printDebugLog("[API:test03EnableDisable3] test03EnableDisable3 enter");
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        
        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mService = this.getService();
            Assert.assertNotNull(mService);
            sleep(1500);
        } else {
            printErrLog("[API:test03EnableDisable3] Bluetooth is disabled");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * Test Connect and Disconnect.
     */
    public void test004ConnectDisconnect1() {
        String[] jniLogs = {
                "Req;0;false;enableNative;1;0",
                "Cb;1;false;method_onActivateCnf;void;2;0;0",
                "Cb;1;false;method_onConnectInd;void;3;0;mtk;1",
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Cb;1;false;method_onDeactivateCnf;void;2;0;0"
        };
        printDebugLog("[API:test04ConnectDisconnect1] test04ConnectDisconnect1 enter");
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mService = this.getService();
            Assert.assertNotNull(mService);
            sleep(3500);
        } else {
            printErrLog("[API:test04ConnectDisconnect1] Bluetooth is disabled");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * Test Pass through key ind
     */
    public void test005PassThroughInd1() {

        printDebugLog("[API:test05PassThroughInd1] test05PassThroughInd1 enter");
        String[] jniLogs = {
                "Req;0;false;enableNative;1;0",
                "Cb;1;false;method_onActivateCnf;void;2;0;0",
                "Cb;1;false;method_onConnectInd;void;3;0;mtk;1",
                "Cb;1;false;method_passThroughKeyInd;void;2;64;1", // AVRCP_POP_POWER
                "Cb;1;false;method_passThroughKeyInd;void;2;65;1", // AVRCP_POP_VOLUME_UP
                "Cb;1;false;method_passThroughKeyInd;void;2;66;1", // AVRCP_POP_VOLUME_DOWN
                "Cb;1;false;method_passThroughKeyInd;void;2;67;1", // AVRCP_POP_MUTE
                "Cb;1;false;method_passThroughKeyInd;void;2;68;1", // AVRCP_POP_PLAY
                "Cb;1;false;method_passThroughKeyInd;void;2;69;1", // AVRCP_POP_STOP
                "Cb;1;false;method_passThroughKeyInd;void;2;70;1", // AVRCP_POP_PAUSE
                "Cb;1;false;method_passThroughKeyInd;void;2;71;1", // AVRCP_POP_RECORD
                "Cb;1;false;method_passThroughKeyInd;void;2;72;1", // AVRCP_POP_REWIND
                "Cb;1;false;method_passThroughKeyInd;void;2;73;1", // AVRCP_POP_FAST_FORWARD
                "Cb;1;false;method_passThroughKeyInd;void;2;74;1", // AVRCP_POP_EJECT
                "Cb;1;false;method_passThroughKeyInd;void;2;75;1", // AVRCP_POP_FORWARD
                "Cb;1;false;method_passThroughKeyInd;void;2;76;1", // AVRCP_POP_BACKWARD
                "Cb;1;false;method_passThroughKeyInd;void;2;0;1",  // default
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Cb;1;false;method_onDeactivateCnf;void;2;0;0"
        };

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        
        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mService = this.getService();
            Assert.assertNotNull(mService);
            sleep(17500);
        } else {
            printErrLog("[API:test05PassThroughInd1] Bluetooth is disabled");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
    */
    public void test006MusicAdapterHandleKeyMessage() {

        Message msg = new Message();
        BitSet regSet = new BitSet(16);
        this.startService(mStartIntent);
        mService = this.getService();
        Assert.assertNotNull(mService);
        BTAvrcpMusicAdapter adapterObject = null;
        try {
            adapterObject = (BTAvrcpMusicAdapter) Reflector.get(mService, "mAdapter");
        } catch (NoSuchFieldException ex) {
            printErrLog("[API:test006MusicAdapterHandleKeyMessage] get music adapter exception occured");
        }
        if (adapterObject == null) {
            printErrLog("[API:test006MusicAdapterHandleKeyMessage] get music adapter objecet failed");
            Assert.assertNotNull(adapterObject);
        } else {
            printDebugLog("[API:test006MusicAdapterHandleKeyMessage] object is : " + adapterObject);

            msg.what = 0x11;
            Reflector.invoke(adapterObject, "handleKeyMessage", new Parameter(Message.class, msg));
            msg.what = 0x22;
            msg.arg1 = 0x01;
            regSet.clear(0x01);
            setFieldValue(adapterObject, "mRegBit", regSet);
            Reflector.invoke(adapterObject, "handleKeyMessage", new Parameter(Message.class, msg));
            msg.arg1 = 0x02;
            regSet.clear(0x02);
            setFieldValue(adapterObject, "mRegBit", regSet);
            Reflector.invoke(adapterObject, "handleKeyMessage", new Parameter(Message.class, msg));
            msg.arg1 = 0x09;
            regSet.clear(0x09);
            setFieldValue(adapterObject, "mRegBit", regSet);
            Reflector.invoke(adapterObject, "handleKeyMessage", new Parameter(Message.class, msg));
        }
    }

    /**
    * Set field value which is in AvrcpService
    * @param o the object which to set the field
    * @param fieldName field name which to set
    * @param value which set to filed in object
    */
    private void setFieldValue(Object o, String fieldName, Object value) {
        try {
            Reflector.set(o, fieldName, value);
        } catch (NoSuchFieldException ex) {
            printErrLog("[API:setFieldValue] NoSuchFieldException occured");
        }
    }

    /**
    * Callback to clean java field value
    */
    @Override
    public void cleanUp() {
        mJniManager = null;
        mStartIntent = null;
        mIsBluetoothOn = false;
        mService = null;
        BluetoothAvrcpService.sUtState = false;
    }

    /**
     * sleep some seconds
     * @param mSeconds
     */
    private void sleep(int mSeconds) {
        try {
            Thread.sleep(mSeconds);
        } catch (InterruptedException ex) {
            printErrLog("[API:sleep] InterruptedException :" + ex.toString());
        }
    }

    /**
    * Record the debug log
    */
    private void printDebugLog(String debugMsg) {
        if (DEBUG) {
            Log.d(TAG, debugMsg);
        }
    }

    /**
    * Record the error log
    */
    private void printErrLog(String errorMsg) {
        Log.e(TAG, errorMsg);
    }

}
