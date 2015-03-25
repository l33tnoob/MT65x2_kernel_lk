
package android.server;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHealth;
import android.bluetooth.BluetoothHealthAppConfiguration;
import android.util.Log;

import junit.framework.Assert;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

public class PrivateHdpTestCase {

    private static final String TAG = "[BT][PrivateHdpTestCase]";

    private static final int HEALTH_PROFILE_SOURCE_DATA_TYPE = 0x1007;

    private static final String PATH = "1MTKDEV8C:B8:64:0B:82:D2";

    private static final String DEVICEPATH = "MTKBT/dev_7C_ED_8D_68_48_E8";

    private static final String DEVICEPATH2 = "MTKBT/dev_8C_B8_64_0B_82_D2";

    private static final String CHANNELPATH = "1MTKDEV8C:B8:64:0B:82:D2";

    private static final String CHANNELPATH2 = "/";

    private static final boolean DEBUG = true;

    private static BluetoothHealthAppConfiguration sHealthAppConfig;

    private static final int MESSAGE_REGISTER_APPLICATION = 0;

    private static final int MESSAGE_UNREGISTER_APPLICATION = 1;

    private static final int MESSAGE_CONNECT_CHANNEL = 2;

    private Object mReturnValue;

    private final BluetoothHealthProfileHandler mHdpInstance = BluetoothHealthProfileHandler
            .getHealthUtInstance();

    private final BluetoothDevice mDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(
            "7C:ED:8D:68:48:E8");

    private final BluetoothDevice mDevice2 = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(
            "8C:B8:64:0B:82:D2");

    public void callPrivateMethod(int id) {
        switch (id) {
            case 0:
                printDebugLog("[API:callPrivateMethod] the id is 0, call findChannelByPath");
                // call findChannelByPath method in
                // BluetoothHealthProfileHandler
                // just through structural parameter to test return value is
                // null
                mReturnValue = Reflector.invoke(mHdpInstance, "findChannelByPath",
                        new Reflector.Parameter(BluetoothDevice.class, mDevice),
                        new Reflector.Parameter(String.class, PATH));
                assertMethodReturnValue(String.valueOf(mReturnValue), "null");
                break;
            // call onHealthDevicePropertyChanged in
            // BluetoothHealthProfileHandler
            case 1:
                printDebugLog("[API:callPrivateMethod] the id is 1, call onHealthDevicePropertyChanged");
                Reflector.invoke(mHdpInstance, "onHealthDevicePropertyChanged",
                        new Reflector.Parameter(String.class, DEVICEPATH), new Reflector.Parameter(
                                String.class, CHANNELPATH));

                break;
            // call findHealthApplication in BluetoothHealthProfileHandler
            // just through structural parameter to test return value is null
            // make local variable chan null
            case 2:
                printDebugLog("[API:callPrivateMethod] the id is 2, call findHealthApplication");
                mReturnValue = Reflector.invoke(mHdpInstance, "findHealthApplication",
                        new Reflector.Parameter(BluetoothDevice.class, mDevice),
                        new Reflector.Parameter(String.class, CHANNELPATH));
                assertMethodReturnValue(String.valueOf(mReturnValue), "null");
                break;
            // call findHealthApplication in BluetoothHealthProfileHandler
            // just through structural parameter to test return value is null
            // make local variable chan not null
            case 3:
                printDebugLog("[API:callPrivateMethod] the id is 3, call findHealthApplication");
                mReturnValue = Reflector.invoke(mHdpInstance, "findHealthApplication",
                        new Reflector.Parameter(BluetoothDevice.class, mDevice2),
                        new Reflector.Parameter(String.class, CHANNELPATH));
                assertMethodReturnValue(String.valueOf(mReturnValue), "null");
                break;
            // call onHealthDeviceChannelChanged in
            // BluetoothHealthProfileHandler
            // set parameter exist to ture
            // make local variable channel null
            case 4:
                printDebugLog("[API:callPrivateMethod] the id is 4, call onHealthDeviceChannelChanged");

                Reflector.invoke(mHdpInstance, "onHealthDeviceChannelChanged",
                        new Reflector.Parameter(String.class, DEVICEPATH), new Reflector.Parameter(
                                String.class, CHANNELPATH), new Reflector.Parameter(boolean.class,
                                true));
                break;
            // call onHealthDeviceChannelChanged in
            // BluetoothHealthProfileHandler
            // set parameter exist to false
            // make local variable channel null
            case 5:
                printDebugLog("[API:callPrivateMethod] the id is 5, call onHealthDeviceChannelChanged");
                Reflector.invoke(mHdpInstance, "onHealthDeviceChannelChanged",
                        new Reflector.Parameter(String.class, DEVICEPATH), new Reflector.Parameter(
                                String.class, CHANNELPATH), new Reflector.Parameter(boolean.class,
                                false));
                break;
            // call onHealthDeviceChannelChanged in
            // BluetoothHealthProfileHandler
            // set parameter exist to false
            // make local variable channel not null
            case 6:
                printDebugLog("[API:callPrivateMethod] the id is 6, call onHealthDeviceChannelChanged");
                Reflector.invoke(mHdpInstance, "onHealthDeviceChannelChanged",
                        new Reflector.Parameter(String.class, DEVICEPATH2),
                        new Reflector.Parameter(String.class, CHANNELPATH),
                        new Reflector.Parameter(boolean.class, false));
                break;
            // call broadcastHealthDeviceStateChange in
            // BluetoothHealthProfileHandler
            // {@link BluetoothHealth#STATE_CONNECTING} to {@link
            // BluetoothHealth#STATE_CONNECTED}

            case 7:
                printDebugLog("[API:callPrivateMethod] the id is 7, call broadcastHealthDeviceStateChange");
                Reflector
                        .invoke(mHdpInstance, "broadcastHealthDeviceStateChange",
                                new Reflector.Parameter(BluetoothDevice.class, mDevice),
                                new Reflector.Parameter(int.class,
                                        BluetoothHealth.STATE_CHANNEL_CONNECTING),
                                new Reflector.Parameter(int.class,
                                        BluetoothHealth.STATE_CHANNEL_CONNECTED));
                break;
            // call broadcastHealthDeviceStateChange in
            // BluetoothHealthProfileHandler
            // @link BluetoothHealth#STATE_CONNECTED} to {@link
            // BluetoothHealth#STATE_DISCONNECTING
            case 8:
                printDebugLog("[API:callPrivateMethod] the id is 8, call broadcastHealthDeviceStateChange");
                Reflector
                        .invoke(mHdpInstance, "broadcastHealthDeviceStateChange",
                                new Reflector.Parameter(BluetoothDevice.class, mDevice),
                                new Reflector.Parameter(int.class,
                                        BluetoothHealth.STATE_CHANNEL_CONNECTED),
                                new Reflector.Parameter(int.class,
                                        BluetoothHealth.STATE_CHANNEL_DISCONNECTING));
                break;
            // call findConnectingChannel in BluetoothHealthProfileHandler

            case 9:
                printDebugLog("[API:callPrivateMethod] the id is 9, call findConnectingChannel");
                constructHealthAppConfiguration();
                Reflector.invoke(mHdpInstance, "findConnectingChannel", new Reflector.Parameter(
                        BluetoothDevice.class, mDevice), new Reflector.Parameter(
                        BluetoothHealthAppConfiguration.class, sHealthAppConfig));
                break;
            // call onHealthDevicePropertyChanged in
            // BluetoothHealthProfileHandler
            // set parameter CHANNELPATH equal "/"
            // This means that the main channel is being destroyed.
            case 10:
                printDebugLog("[API:callPrivateMethod] the id is 10, call onHealthDevicePropertyChanged");
                Reflector.invoke(mHdpInstance, "onHealthDevicePropertyChanged",
                        new Reflector.Parameter(String.class, DEVICEPATH), new Reflector.Parameter(
                                String.class, CHANNELPATH2));
                break;
            // call convertState in BluetoothHealthProfileHandler
            // set parameter invalid
            case 11:
                printDebugLog("[API:callPrivateMethod] the id is 11, call convertState");

                mReturnValue = Reflector.invoke(mHdpInstance, "convertState",
                        new Reflector.Parameter(int.class, 5));
                assertMethodReturnValue(String.valueOf(mReturnValue), "-1");
                break;

            default:
                printDebugLog("[API:callPrivateMethod] the id is wrong");
                break;
        }
    }

    /**
     * @param methodResult call method to generate result
     * @param expResult expect result which to compare return value through call
     *            method
     */
    private void assertMethodReturnValue(String methodResult, String expResult) {
        printDebugLog("[API:assertMethodReturnValue] methodResult is : " + methodResult
                + ", extResult is : " + expResult);
        Assert.assertEquals(methodResult, expResult);
    }

    private void constructHealthAppConfiguration() {
        try {
            Class[] params = {
                    String.class, int.class
            };
            Constructor con = BluetoothHealthAppConfiguration.class.getDeclaredConstructor(params);
            printDebugLog("constructBluetoothDevice:" + con);
            con.setAccessible(true);
            sHealthAppConfig = (BluetoothHealthAppConfiguration) con.newInstance(TAG,
                    HEALTH_PROFILE_SOURCE_DATA_TYPE);
            printDebugLog("BluetoothHealthAppConfiguration:" + sHealthAppConfig);
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

    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
