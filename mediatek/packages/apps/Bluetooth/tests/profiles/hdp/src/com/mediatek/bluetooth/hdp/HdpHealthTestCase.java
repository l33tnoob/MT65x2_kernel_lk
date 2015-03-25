
package com.mediatek.bluetooth.hdp;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHealth;
import android.bluetooth.BluetoothHealthAppConfiguration;
import android.bluetooth.BluetoothHealthCallback;
import android.bluetooth.BluetoothProfile;

import android.content.Context;
import android.content.Intent;
import android.os.ParcelFileDescriptor;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothUnitTestJni;
import com.mediatek.bluetooth.Reflector;

import junit.framework.Assert;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

public class HdpHealthTestCase extends InstrumentationTestCase {

    private static final String TAG = "[BT][HDPUT][HdpHealthTestCase]";

    // Use the appropriate IEEE 11073 data types based on the devices used.
    // Below are some examples. Refer to relevant Bluetooth HDP specifications
    // for detail.
    // 0x1007 - blood pressure meter
    private static final int HEALTH_PROFILE_SOURCE_DATA_TYPE = 0x1007;

    private static final boolean DEBUG = true;

    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;

    private static final String ACTION_DUMP_COVERAGE = "action.dumpcoverage";

    private static final String REMOTE_SERVICE_NAME = "Bluetooth.BTSimulatorReceiver.A2DP";

    private boolean mIsBluetoothOn = false; // check the bluetooth is on whether

    // or not
    private boolean mIsGetProfileProxy = false;

    private BluetoothUnitTestJni mJniManager = null;

    private Context mTestContext; // the test Context

    private static int sChannelId;

    private BluetoothAdapter mAdapter;

    private static BluetoothHealth sBluetoothHealth;

    private static BluetoothDevice sDevice;

    private static BluetoothHealthAppConfiguration sHealthAppConfig;

    /**
     * Callback to handle application registration and unregistration events.
     * The service,passes the status back to the UI client.
     */
    private final BluetoothHealthCallback mHealthCallback = new BluetoothHealthCallback() {
        public void onHealthAppConfigurationStatusChange(BluetoothHealthAppConfiguration config,
                int status) {
            printDebugLog("onHealthAppConfigurationStatusChange");
            if (status == BluetoothHealth.APP_CONFIG_REGISTRATION_FAILURE) {
                sHealthAppConfig = null;
                printDebugLog("register fail");
            } else if (status == BluetoothHealth.APP_CONFIG_REGISTRATION_SUCCESS) {
                sHealthAppConfig = config;
                printDebugLog("register success");
            } else if (status == BluetoothHealth.APP_CONFIG_UNREGISTRATION_FAILURE) {
                printDebugLog("unregister fail");
            } else if (status == BluetoothHealth.APP_CONFIG_UNREGISTRATION_SUCCESS) {
                printDebugLog("unregister success");
            }
        }

        // Callback to handle channel connection state changes.
        // Note that the logic of the state machine may need to be modified
        // based on the HDP device.
        // When the HDP device is connected, the received file descriptor is
        // passed to the
        // ReadThread to read the content.
        public void onHealthChannelStateChange(BluetoothHealthAppConfiguration config,
                BluetoothDevice device, int prevState, int newState, ParcelFileDescriptor fd,
                int channelId) {
            printDebugLog("onHealthChannelStateChange" + "device is:" + device + "prevState is:"
                    + prevState + "newState is:" + newState + "channelId is:" + channelId);
            printDebugLog(String.format("prevState\t%d ----------> newState\t%d", prevState,
                    newState));

            if (prevState == BluetoothHealth.STATE_CHANNEL_DISCONNECTED
                    && newState == BluetoothHealth.STATE_CHANNEL_CONNECTED) {
                if (config.equals(sHealthAppConfig)) {
                    sChannelId = channelId;
                    printDebugLog("create channel ok");
                } else {

                    printDebugLog("create channel fail");
                }
            } else if (prevState == BluetoothHealth.STATE_CHANNEL_CONNECTING
                    && newState == BluetoothHealth.STATE_CHANNEL_DISCONNECTED) {
                printDebugLog("destroy channel ok");
            } else if (newState == BluetoothHealth.STATE_CHANNEL_DISCONNECTED) {
                if (config.equals(sHealthAppConfig)) {
                    printDebugLog("destroy channel fail");
                } else {
                    printDebugLog("destroy channel ok");
                }
            }
        }
    };

    /**
     * Callbacks to handle connection set up and disconnection clean up.
     */
    private final BluetoothProfile.ServiceListener mBluetoothServiceListener = new BluetoothProfile.ServiceListener() {
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            if (profile == BluetoothProfile.HEALTH) {
                sBluetoothHealth = (BluetoothHealth) proxy;
                printDebugLog("onServiceConnected to profile: " + profile + " proxy " + proxy);
            }
        }

        public void onServiceDisconnected(int profile) {
            if (profile == BluetoothProfile.HEALTH) {
                sBluetoothHealth = null;
                printDebugLog("onServiceDisconnected to profile: " + profile);
            }
        }
    };

    public HdpHealthTestCase() {
        super();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp(); // shoule be the first statement
        printDebugLog("setUp +++++++");
        // get the test context
        mTestContext = this.getInstrumentation().getContext();
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_HDP);

        Assert.assertNotNull(mTestContext);

    }

    /**
     * Make sure Bluetooth and health profile are available on the Android
     * device, get a reference to the BluetoothHealth proxy object, and
     * instantiate BluetoothDevice
     */
    public void test001testCondition() throws Exception {
        printDebugLog("test001testCondition");
        enableBt();
        constructBluetoothDevice();
        sleep(1000);
        if (mAdapter.getProfileProxy(mTestContext, mBluetoothServiceListener,
                BluetoothProfile.HEALTH)) {
            mIsGetProfileProxy = true;
        }
        assertEquals(true, mAdapter.isEnabled());
        assertEquals(true, mIsGetProfileProxy);
    }

    /**
     * Create a BluetoothHealth callback andregister an application
     * configuration that acts as a Health SOURCE.
     */
    public void test002registerAppCofiguration() {
        printDebugLog("test002registerAppCofiguration " + " sBluetoothHealth: " + sBluetoothHealth
                + " mHealthCallback: " + mHealthCallback);

        sBluetoothHealth
                .registerAppConfiguration(TAG, HEALTH_PROFILE_SOURCE_DATA_TYPE,
                        BluetoothHealth.SOURCE_ROLE, BluetoothHealth.CHANNEL_TYPE_RELIABLE,
                        mHealthCallback);
    }

    /**
     * Create a BluetoothHealth callback andregister an application
     * configuration that acts as a Health SINK.
     */
    public void test003registerSinkAppConfiguration() {
        printDebugLog("test003registerSinkAppConfiguration");
        sBluetoothHealth.registerSinkAppConfiguration(TAG, HEALTH_PROFILE_SOURCE_DATA_TYPE,
                mHealthCallback);
    }

    /**
     * Establish connection to a health device
     */
    public void test004connectChannelToSource() {
        printDebugLog("test004connectChannelToSource" + " sHealthAppConfig: " + sHealthAppConfig);
        sBluetoothHealth.connectChannelToSource(sDevice, sHealthAppConfig);
    }

    public void test005connectChannelToSource2() {
        printDebugLog("test005connectChannelToSource2");
        sBluetoothHealth.connectChannelToSource(sDevice, null);
    }

    public void test006connectChannelToSink() {
        printDebugLog("test006connectChannelToSink");
        sBluetoothHealth.connectChannelToSink(sDevice, sHealthAppConfig,
                BluetoothHealth.CHANNEL_TYPE_RELIABLE);
    }

    public void test007connectChannelToSink2() {
        printDebugLog("test007connectChannelToSink2");
        sBluetoothHealth.connectChannelToSink(sDevice, null, BluetoothHealth.CHANNEL_TYPE_RELIABLE);
    }

    /**
     * Get the file descriptor of the main channel associated with the remote
     * device and application configuration.
     * 
     * @param device The remote Bluetooth health device
     * @param config The application configuration
     * @return null on failure, ParcelFileDescriptor on success.
     */
    public void test008getMainChannelFd() {
        printDebugLog("test008getMainChannelFd");
        sBluetoothHealth.getMainChannelFd(sDevice, sHealthAppConfig);
    }

    public void test009getMainChannelFd2() {
        printDebugLog("test009getMainChannelFd2");
        sBluetoothHealth.getMainChannelFd(sDevice, null);
    }

    public void test010getConnectedDevices() {
        printDebugLog("test010getConnectedDevices");
        sBluetoothHealth.getConnectedDevices();
    }

    /**
     * Get the current connection state of the profile
     * 
     * @param device Remote bluetooth device.
     * @return State of the profile connection. One of {@link #STATE_CONNECTED},
     *         {@link #STATE_CONNECTING}, {@link #STATE_DISCONNECTED},
     *         {@link #STATE_DISCONNECTING}
     */
    public void test011getConnectionState() {
        printDebugLog("test011getConnectionState");
        sBluetoothHealth.getConnectionState(sDevice);
    }

    public void test012getConnectionState2() {
        printDebugLog("test012getConnectionState2");
        sBluetoothHealth.getConnectionState(null);
    }

    /**
     * Get a list of devices that match any of the given connection states.
     * 
     * @param states Array of states. States can be one of
     *            {@link #STATE_CONNECTED}, {@link #STATE_CONNECTING},
     *            {@link #STATE_DISCONNECTED}, {@link #STATE_DISCONNECTING},
     * @return List of devices. The list will be empty on error.
     */
    public void test013getDevicesMatchingConnectionStates() {
        printDebugLog("test013getDevicesMatchingConnectionStates");
        int[] states = {
                0, 1, 2, 3
        };
        sBluetoothHealth.getDevicesMatchingConnectionStates(states);
    }

    /**
     * When done, close the health channel and unregister the application. The
     * channel will also close when there is extended inactivity.
     */
    public void test014unregisterAppConfiguration() {
        printDebugLog("test14unregisterAppConfiguration");
        sBluetoothHealth.unregisterAppConfiguration(sHealthAppConfig);
    }

    public void test015unregisterAppConfiguration2() {
        printDebugLog("test15unregisterAppConfiguration2");
        sBluetoothHealth.unregisterAppConfiguration(null);
    }

    /**
     * Disconnect channel through the Bluetooth Health API.
     */
    public void test016disconnectChannel() {
        printDebugLog("test16disconnectChannel" + "channelId is:" + sChannelId);
        sBluetoothHealth.disconnectChannel(sDevice, sHealthAppConfig, sChannelId);
    }

    public void test017disconnectChannel2() {
        printDebugLog("test17disconnectChannel2");
        sBluetoothHealth.disconnectChannel(sDevice, null, sChannelId);
    }

    /**
     * just for test service is null
     * 
     * @throws Exception
     */
    public void test018setService2NullregisterAppConfiguration() throws Exception {
        printDebugLog("test018setService2Null_registerAppConfiguration" + " mBluetoothHealth: "
                + sBluetoothHealth);
        try {
            Reflector.set(sBluetoothHealth, "mService", null);
        } catch (NoSuchFieldException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        sBluetoothHealth
                .registerAppConfiguration(TAG, HEALTH_PROFILE_SOURCE_DATA_TYPE,
                        BluetoothHealth.SOURCE_ROLE, BluetoothHealth.CHANNEL_TYPE_RELIABLE,
                        mHealthCallback);
    }

    public void test019setService2NullgetConnectedDevices() throws Exception {
        sBluetoothHealth.getConnectedDevices();
    }

    public void test020setService2NullgetDevicesMatchingConnectionStates() throws Exception {
        int[] states = {
                0, 1, 2, 3
        };
        sBluetoothHealth.getDevicesMatchingConnectionStates(states);
    }

    public void test021callClose() throws Exception {
        Reflector.invoke(sBluetoothHealth, "close", (Reflector.Parameter[]) null);

    }

    public void test022resumeService() {
        if (mAdapter.getProfileProxy(mTestContext, mBluetoothServiceListener,
                BluetoothProfile.HEALTH)) {
            mIsGetProfileProxy = true;
        }
    }

    /**
     * call some private method in BluetoothHealthProfileHandler
     */
    public void test023callPrivateMethod() {
        mJniManager.callPrivateMethod(1, 0, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 1, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 2, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 3, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 4, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 5, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 6, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 7, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 8, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 9, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 10, REMOTE_SERVICE_NAME);
        mJniManager.callPrivateMethod(1, 11, REMOTE_SERVICE_NAME);

    }

    public void test024disableBt() throws Exception {
        disableBt();
        Reflector.invoke(sBluetoothHealth, "isEnabled", (Reflector.Parameter[]) null);
    }

    public void test025enableBt() throws Exception {
        enableBt();
    }

    public void test100EndHdpUT() {
        // send the intent to dump hdp emma coverage
        Intent intent = new Intent(ACTION_DUMP_COVERAGE);
        mTestContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mJniManager.dumpEmmaReport("Bluetooth.BTSimulatorReceiver.A2DP");
    }

    @SuppressWarnings("unchecked")
    private void constructBluetoothDevice() throws Exception {
        try {
            Constructor con = BluetoothDevice.class.getDeclaredConstructor(String.class);
            printDebugLog("constructBluetoothDevice:" + con);
            con.setAccessible(true);
            sDevice = (BluetoothDevice) con.newInstance("7C:ED:8D:68:48:E8");
            printDebugLog("BluetoothDevice:" + sDevice);
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

    /*
     * Do some clear work when you finished each test method,tearDown will be
     * called
     */
    private void enableBt() {
        if (!mAdapter.isEnabled()) {
            mAdapter.enable();
        }
        mIsBluetoothOn = true;
        sleep(4000);
    }

    private void disableBt() {
        if (mAdapter.isEnabled()) {
            mAdapter.disable();
        }
        mIsBluetoothOn = false;
        sleep(4000);
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub

        printDebugLog("tearDown---------");
        mTestContext = null;
        mJniManager = null;
        super.tearDown(); // shouled be the last statement
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

    /*
     * print the debug log
     */
    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
