
/*******************************************************************************
**
**  This file is used to test BluetoothA2dpService.java
**  which used to call AIDL interface.
**
**  This file is only for unit test
**
*******************************************************************************/


package com.mediatek.bluetooth.a2dp;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.IBluetoothA2dp;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.server.BluetoothA2dpService;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothUnitTestJni;

import junit.framework.Assert;

import java.util.List;

public class A2dpServiceTestCase extends InstrumentationTestCase {

    private static final String TAG = "[BT][A2DPUT][A2DPServiceTestCase]";

    private static final boolean DEBUG = true;

    private boolean mIsBluetoothOn = false; // check the bluetooth weather is on

    private BluetoothUnitTestJni mJniManager = null;

    //BluetoothA2dp instance, which used to call AIDL interface
    private static IBluetoothA2dp sTestServiceA2dp; // IBluetoothA2dp

    private BluetoothAdapter mAdapter;

    private static final String REMOTE_SERVICE_NAME = "Bluetooth.BTSimulatorReceiver.A2DP";
    /*
     * Override this method to get the test Context. And check weather the bluetooth is on Init the jni class when you run
     * each test method,setUp will be called
     */
    @Override
    protected void setUp() throws Exception {
        super.setUp(); // shoule be the first statement
        printDebugLog("setUp +++++++");
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_A2DP);
    }

    /*
     * Do some clear work when you finished each test method,tearDown will be called
     */
    @Override
    protected void tearDown() throws Exception {

        printDebugLog("tearDown---------");
        mIsBluetoothOn = false;
        mJniManager = null;
        super.tearDown(); // shouled be the last statement
    }

    protected void enableBT() {
        if (mAdapter != null) {
            if (!mAdapter.isEnabled()) { // check the bluetooth is off
                mAdapter.enable(); // turn on the bluetooth
                sleep(3000); // wait for 3s,to make sure the bluetooth is on
                if (mAdapter.isEnabled()) { // check again
                    mIsBluetoothOn = true;
                } else {
                    mIsBluetoothOn = false;
                }
            } else { // the bluetooth is on
                mIsBluetoothOn = true;
            }
        } else {
            printErrorLog("[API:setUp] The device has no BluetoothAdapter");
            Assert.assertNotNull(null);
        }
    }

    protected void disableBT() {
        if (mAdapter != null) {
            if (mAdapter.isEnabled()) { // check the bluetooth is off
                mAdapter.disable(); // turn on the bluetooth
                sleep(3000); // wait for 3s,to make sure the bluetooth is on
                if (mAdapter.isEnabled()) { // check again
                    mIsBluetoothOn = true;
                } else {
                    mIsBluetoothOn = false;
                }
            } else { // the bluetooth is on
                mIsBluetoothOn = false;
            }
        } else {
            printErrorLog("[API:setUp] The device has no BluetoothAdapter");
            Assert.assertNotNull(null);
        }
    }

   /*
   * A2dpServiceTestCase constructor
   */
    public A2dpServiceTestCase() {
        super();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    public void test001StartA2dpUT() {
        // lunch BTSimulatorReceiver.A2DP
        //enableBT();
    }

    /**
    * The last test case, which used to generate coverage.ec
    */
    public void test100EndA2dpUT() {
        mJniManager.dumpEmmaReport(REMOTE_SERVICE_NAME);
        //disableBT();
    }

    /**
    * Call AIDL getConnectedDevices interface
    * First get proxy
    */
    public void test002getConnectedDevices() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };
        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            try {
                List<BluetoothDevice> res = sTestServiceA2dp.getConnectedDevices();
                printDebugLog("[API:test04getConnectedDevices] res.size() " + res.size());
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);
        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL getDevicesMatchingConnectionStates interface
    * First get proxy
    */
    public void test003getDevicesMatchingConnectionStates() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };
        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            try {
                List<BluetoothDevice> res = sTestServiceA2dp
                        .getDevicesMatchingConnectionStates(new int[] {
                                BluetoothProfile.STATE_CONNECTED,
                                BluetoothProfile.STATE_CONNECTING,
                                BluetoothProfile.STATE_DISCONNECTING
                        });
                printDebugLog("[API:getDevicesMatchingConnectionStates] res.size() " + res.size());
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);
        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL setPriority interface
    * First get proxy
    */
    public void test004setPriority() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");

            try {
                boolean res = sTestServiceA2dp.setPriority(target, 100);
                printDebugLog("[API:test06setPriority] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);
        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL getPriority interface
    * First get proxy
    */
    public void test005getPriority() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");

            try {
                int res = sTestServiceA2dp.getPriority(target);
                printDebugLog("[API:test07getPriority] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);

        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL isA2dpPlaying interface
    * First get proxy
    */
    public void test006isA2dpPlaying() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };
        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");

            try {
                boolean res = sTestServiceA2dp.isA2dpPlaying(target);
                printDebugLog("[API:test07getPriority] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);

        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL suspendSink interface
    * First get proxy
    */
    public void test007suspendSink() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };
        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");

            try {
                boolean res = sTestServiceA2dp.suspendSink(target);
                printDebugLog("[API:test09suspendSink] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);
        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL resumeSink interface
    * First get proxy
    */
    public void test008resumeSink() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };
        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");

            try {
                boolean res = sTestServiceA2dp.resumeSink(target);
                printDebugLog("[API:test10resumeSink] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }
        sleep(2000);
        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /*
     * wifi state change case description: 1 start A2DP, 2 stop A2DP ,
     */
    public void test009wifiStatusChange() {
        int waitingTime = 3000;

        String jniLogs[] = {
            "Cb;2;false;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;4;",
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        sleep(waitingTime);

        if (mIsBluetoothOn) {
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL connect interface
    * First get proxy
    */
    public void test010connect() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");

            try {
                boolean res = sTestServiceA2dp.connect(target);
                printDebugLog("[API:test01connect] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }

        sleep(2000);

        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call AIDL disconnect interface
    * First get proxy
    */
    public void test011disconnect() {

        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        if (sTestServiceA2dp != null) {
            if (mAdapter.isEnabled()) {
                printDebugLog("mBluetoothService enable !");
            }
            BluetoothDevice target = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");
            // a2dpService runs in the system_server,
            // enable BT to start a2dpService.
            try {
                boolean res = sTestServiceA2dp.disconnect(target);
                printDebugLog("[API:test03disconnect] res " + res);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
            }
        }

        sleep(2000);

        if (mIsBluetoothOn) {
            // Assert.assertTRUE(!mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
    * Call BluetoothA2dpService callback
    * disconnect state change case description: 1 start A2DP, 2 stop A2DP ,
    */
    public void test012disconnctStatusChange() {
        int waitingTime = 3000;

        String jniLogs[] = {
            "Cb;2;false;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;1;",
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        sleep(waitingTime);

        if (mIsBluetoothOn) {
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /*
     * connecting state change case description: 1 start A2DP, 2 stop A2DP ,
     */
    public void test013connectingStatusChange() {
        int waitingTime = 3000;

        String jniLogs[] = {
            "Cb;2;false;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;2;",
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);

        sleep(waitingTime);

        if (mIsBluetoothOn) {
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
     * connected state change case description: 1 start A2DP, 2 stop A2DP ,
     */
    public void test014connectedStatusChange() {
        int waitingTime = 3000;

        String jniLogs[] = {
            "Cb;2;false;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;3;",
        };

        setA2DPService();

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);

        sleep(waitingTime);

        if (mIsBluetoothOn) {
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is true");
        }
    }

    /**
     * FM playing failure state change case description: 1 start A2DP, 2 stop
     * A2DP ,
     */
    public void test015FmPlayingFailedStatusChange() {
        int waitingTime = 3000;

        String jniLogs[] = {
            "Cb;2;false;onSinkPropertyChanged;void;2;MTKBT/dev_00_0D_FD_4B_57_E3;5;",
        };
        setA2DPService();
        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        sleep(waitingTime);
    }

    /**
     * a2dp service allowIncomingConnect, the bluetooth device is correct
     */
    public void test016allowIncomingConnect() {
        String jniLogs[] = {
                "Req;0;false;startNative;void;0", "Req;0;false;stopNative;void;0"
        };

        mJniManager.setCaseLog(jniLogs, ";", REMOTE_SERVICE_NAME);
        setA2DPService();
        BluetoothDevice device = mAdapter.getRemoteDevice("00:0D:FD:4B:57:E3");
        try {
            sTestServiceA2dp.allowIncomingConnect(device, true);
        } catch (RemoteException e) {
            printErrorLog("[API:test016allowIncomingConnect] RemoteException occured");
        }
    }

    /**
     * call private method in BluetoothA2dpService
     */
    public void test017callPrivateMethod() {
        mJniManager.callPrivateMethod(0, 0, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 1, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 2, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 3, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 4, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 5, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 6, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 7, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 8, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 9, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 10, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 11, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(0, 12, REMOTE_SERVICE_NAME);
    }

    /*
     * sleep for a while
     */
    private void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /**
     * print the debug log
     */
    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    /**
     * print the debug log
     */
    private void printErrorLog(String msg) {
        Log.e(TAG, msg);
    }

    /**
     * get a2dp service binder proxy
     */
    private void setA2DPService() {
        if (sTestServiceA2dp == null) {
            IBinder b = ServiceManager.getService(BluetoothA2dpService.BLUETOOTH_A2DP_SERVICE);
            if (b != null) {
                sTestServiceA2dp = IBluetoothA2dp.Stub.asInterface(b);
            } else {
                printErrorLog("failed to get A2DP service");
            }
        }
    }

}

