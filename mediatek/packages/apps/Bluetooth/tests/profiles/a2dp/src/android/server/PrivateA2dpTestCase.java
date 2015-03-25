
/******************************************************************************
**
**  This class is used to call private method in BluetoothA2dpService.
**  Only one method callPrivateMethod, this method is called by native callback.
**  The callback methods are triggered by BTSimulatorReceiver.
**
******************************************************************************/

package android.server;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.util.Log;

import junit.framework.Assert;

import java.util.HashMap;

public class PrivateA2dpTestCase {

    private static final String TAG = "[BT][PrivateA2dpTestCase]";

    private static final boolean DEBUG = true;

    //get BluetoothA2dpService instance, through public static method getA2dpServiceInstance
    private final BluetoothA2dpService mA2dpInstance = BluetoothA2dpService.getA2dpServiceInstance();

    //get BluetoothDevice instance through BluetoothAdapter
    private final BluetoothDevice mDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice("00:0D:FD:4B:57:E3");

    //get HashMap which to reflect mAudioDevices in BluetoothA2dpService
    private HashMap<BluetoothDevice, Integer> mAudioDeviceList = new HashMap<BluetoothDevice, Integer>();

    //get return value which by calling private method, translate this object to string
    //use the string to compare with the expected result.
    private Object mReturnValue;

    /**
    * this method is called by native callback, callStaticMethod
    * @param id goto which switch case, to call different private method
    */
    public void callPrivateMethod(int id) {

        switch (id) {

            case 0:
                printDebugLog("[API:callPrivateMethod] the id is 0, call public method");
                //call connectSinkInternal goto getDevicesMatchingConnectionStates block
                setAudioDevicesList(0);
                mReturnValue = mA2dpInstance.connectSinkInternal(mDevice);
                assertMethodReturnValue(String.valueOf(mReturnValue), "true");

                //call connectSinkInternal goto getDevicesMatchingConnectionStates else block
                setAudioDevicesList(2);
                mReturnValue = mA2dpInstance.connectSinkInternal(mDevice);
                assertMethodReturnValue(String.valueOf(mReturnValue), "false");

                //call disconnectSinkInternal goto switch block
                mAudioDeviceList.clear();
                mReturnValue = mA2dpInstance.disconnectSinkInternal(mDevice);
                assertMethodReturnValue(String.valueOf(mReturnValue), "false");

                //call disconnectSinkInternal goto switch else blcok
                setAudioDevicesList(1);
                mReturnValue = mA2dpInstance.disconnectSinkInternal(mDevice);
                assertMethodReturnValue(String.valueOf(mReturnValue), "true");

                //call getState 
                mAudioDeviceList.clear();
                int i = mA2dpInstance.getState();
                setPrivateFieldValue("mFmOverBtOn", true);
                mA2dpInstance.disconnect(mDevice);

                break;

            case 1:
                printDebugLog("[API:callPrivateMethod] the id is 1, call isPhoneDock");
                mReturnValue = invokePrivateMethodWithParam("isPhoneDocked",
                        new Reflector.Parameter(BluetoothDevice.class, mDevice));
                assertMethodReturnValue(String.valueOf(mReturnValue), "false");
                break;

            case 2:
                printDebugLog("[API:callPrivateMethod] the id is 2, call handleSinkPlayingStateChange");
                invokePrivateMethodWithParam("handleSinkPlayingStateChange",
                                new Reflector.Parameter(BluetoothDevice.class, mDevice),
                                new Reflector.Parameter(int.class, 1), new Reflector.Parameter(int.class, 0));
                break;

            case 3:
                printDebugLog("[API:callPrivateMethod] the id is 3, call onConnectSinkResult");
                invokePrivateMethodWithParam("onConnectSinkResult",
                                new Reflector.Parameter(String.class, "00:0D:FD:4B:57:E3"),
                                new Reflector.Parameter(boolean.class, false));
                invokePrivateMethodWithParam("onConnectSinkResult",
                                new Reflector.Parameter(String.class, "MTKBT/dev_00_0D_FD_4B_57_E3"),
                                new Reflector.Parameter(boolean.class, false));
                break;

            case 4:
                printDebugLog("[API:callPrivateMethod] the id is 4, call adjustOtherSinkPriorities");
                invokePrivateMethodWithParam("adjustOtherSinkPriorities",
                        new Reflector.Parameter(BluetoothDevice.class, mDevice));
                break;

            case 5:
                printDebugLog("[API:callPrivateMethod] the id is 5, call handleFmSinkPlayingStateChange");
                //call handleFmSinkPlayingStateChange and state is FM_START_FAILED(33)
                setPrivateFieldValue("mFmStartReq", true);
                invokePrivateMethodWithParam("handleFmSinkPlayingStateChange", new Reflector.Parameter(int.class, 33));
                assertFieldValue("mFmResult", "1");
                assertFieldValue("mFmOverBtOn", "false");
                assertFieldValue("mFmStartReq", "false");

                //call handleFmSinkPlayingStateChange and state is STATE_PLAYING(10)
                setPrivateFieldValue("mFmStartReq", true);
                invokePrivateMethodWithParam("handleFmSinkPlayingStateChange", new Reflector.Parameter(int.class, 10));
                assertFieldValue("mFmResult", "0");
                assertFieldValue("mFmOverBtOn", "true");
                assertFieldValue("mFmStartReq", "false");
                break;

            case 6:
                printDebugLog("[API:callPrivateMethod] the id is 6, call private method without params");
                //call fmOverBtViaController and mA2dpState < BluetoothA2dp.STATE_CONNECTED
                invokePrivateMethodWithoutParam("fmOverBtViaController");
//                assertFieldValue("mFmResult", "1");
//                assertFieldValue("mFmOverBtOn", "false");

                //call fmOverBtViaController and mA2dpState == BluetoothA2dp.STATE_CONNECTED
                setPrivateFieldValue("mA2dpState", 2);
                invokePrivateMethodWithoutParam("fmOverBtViaController");
                assertFieldValue("mFmStartReq", "true");

                //call fmOverBtViaController and mA2dpState > BluetoothA2dp.STATE_CONNECTED
                setPrivateFieldValue("mA2dpState", 3);
                invokePrivateMethodWithoutParam("fmOverBtViaController");
                assertFieldValue("mFmStartReq", "true");

                //call stopFm and mA2dpState is STATE_PLAYING(10)
                setPrivateFieldValue("mA2dpState", 10);
                invokePrivateMethodWithoutParam("stopFm");

                //call stopFm and mA2dpState is 11, goto STATE_PLAYING else block
                setPrivateFieldValue("mA2dpState", 11);
                invokePrivateMethodWithoutParam("stopFm");

                //call stopFm and mA2dpState is 0, goto STATE_DISCONNECTED block
                setPrivateFieldValue("mA2dpState", 0);
                invokePrivateMethodWithoutParam("stopFm");

                //call fmOverBtViaHost
                invokePrivateMethodWithoutParam("fmOverBtViaHost");

                //call fmThroughPath, return 0
                mReturnValue = invokePrivateMethodWithoutParam("fmThroughPath");
                assertMethodReturnValue(String.valueOf(mReturnValue), "0");

                //call fmSendIntent
                invokePrivateMethodWithoutParam("fmSendIntent");

                //call decA2dpThroughput4WifiOn
                invokePrivateMethodWithoutParam("decA2dpThroughput4WifiOn");

                //call incA2dpThroughput4WifiOff
                invokePrivateMethodWithoutParam("incA2dpThroughput4WifiOff");

                //call onBluetoothDisable and mAudioDevices is not empty, mFmOverBtOn is true
                //device state is STATE_CONNECTING(1)
                setAudioDevicesList(1);
                setPrivateFieldValue("mFmOverBtOn", true);
                invokePrivateMethodWithoutParam("onBluetoothDisable");

                //call onBluetoothDisable and mAudioDevices is not empty, mFmOverBtOn is true
                //device state is STATE_DISCONNECTING(3)
                setAudioDevicesList(3);
                invokePrivateMethodWithoutParam("onBluetoothDisable");
                break;

            case 7:
                printDebugLog("[API:callPrivateMethod] the id is 7");
                //state == BluetoothA2dp.STATE_PLAYING and mFmStartReq is true
                setPrivateFieldValue("mFmStartReq", true);
                mReturnValue = invokePrivateMethodWithParam("handleFMSinkStateChange",
                        new Reflector.Parameter(int.class, 10));
                assertMethodReturnValue(String.valueOf(mReturnValue), "10");
                assertFieldValue("mFmOverBtOn", "true");
                assertFieldValue("mFmStartReq", "false");
                assertFieldValue("mFmResult", "0");

                //state == BluetoothA2dpService.FM_START_FAILED and mFmStartReq is true
                setPrivateFieldValue("mFmStartReq", true);
                mReturnValue = invokePrivateMethodWithParam("handleFMSinkStateChange",
                        new Reflector.Parameter(int.class, 33));
                assertMethodReturnValue(String.valueOf(mReturnValue), "0");
                assertFieldValue("mFmOverBtOn", "false");
                assertFieldValue("mFmStartReq", "false");
                assertFieldValue("mFmResult", "1");

                //call convertBluezSinkStringToState which string is connected
                mReturnValue = invokePrivateMethodWithParam("convertBluezSinkStringToState",
                            new Reflector.Parameter(String.class, "connected"));
                assertMethodReturnValue(String.valueOf(mReturnValue), "2");

                //call convertBluezSinkStringToState which string is bt_test, means goto default
                mReturnValue = invokePrivateMethodWithParam("convertBluezSinkStringToState",
                            new Reflector.Parameter(String.class, "bt_test"));
                assertMethodReturnValue(String.valueOf(mReturnValue), "-1");

                //call switchStatusOnSinkStateChange
                setPrivateFieldValue("mFmOverBtOn", true);
                setPrivateFieldValue("mFmStartReq", true);
                invokePrivateMethodWithParam("switchStatusOnSinkStateChange", new Reflector.Parameter(int.class, 2));
                assertFieldValue("mFmOverBtOn", "false");
                assertFieldValue("mFmStartReq", "false");
                break;

            case 8:
                printDebugLog("[API:callPrivateMethod] id is 8, and call handleFMandWifiAction");
                //call handleFMandWifiAction, send intent is FM_POWERUP
                setPrivateFieldValue("mA2dpDisconnecting", 1);
                mReturnValue = invokePrivateMethodWithParam("handleFMandWifiAction", new Reflector.Parameter(Intent.class, 
                                new Intent("com.mediatek.FMRadio.FMRadioService.ACTION_TOA2DP_FM_POWERUP")));
                assertFieldValue("mFmResult", "1");

                //call handleFMandWifiAction, send intent is FM_POWERDOWN, and mFmOverBtOn, mFmOverBtMode are true
                setPrivateFieldValue("mFmOverBtOn", true);
                setPrivateFieldValue("mFmOverBtMode", true);
                invokePrivateMethodWithParam("handleFMandWifiAction", new Reflector.Parameter(Intent.class, 
                                new Intent("com.mediatek.FMRadio.FMRadioService.ACTION_TOA2DP_FM_POWERDOWN")));
                assertFieldValue("mFmOverBtOn", "false");

                //call handleFMandWifiAction and mFmOverBtOn is false, goto else block
                setPrivateFieldValue("mFmOverBtOn", false);
                invokePrivateMethodWithParam("handleFMandWifiAction", new Reflector.Parameter(Intent.class, 
                                new Intent("com.mediatek.FMRadio.FMRadioService.ACTION_TOA2DP_FM_POWERDOWN")));

                //call handleFMandWifiAction
                invokePrivateMethodWithParam("handleFMandWifiAction", new Reflector.Parameter(Intent.class, 
                                new Intent("android.net.wifi.WIFI_STATE_CHANGED").putExtra("wifi_state", 3)));

                //call handleFMandWifiAction
                invokePrivateMethodWithParam("handleFMandWifiAction", new Reflector.Parameter(Intent.class, 
                                new Intent("android.net.wifi.WIFI_STATE_CHANGED").putExtra("wifi_state", 1)));
                break;

            case 9:
                printDebugLog("[API:callPrivateMethod] id is 9, and call checkSinkSuspendState");
                //call checkSinkSuspendState, mPlayingA2dpDevice is not null, mTargetA2dpState is STATE_CONNECTED
                setPrivateFieldValue("mPlayingA2dpDevice", mDevice);
                setPrivateFieldValue("mTargetA2dpState", 2);
                mReturnValue = invokePrivateMethodWithParam("checkSinkSuspendState", new Reflector.Parameter(int.class, 2));
                assertMethodReturnValue(String.valueOf(mReturnValue), "true");

                //call checkSinkSuspendState and mTargetA2dpState is STATE_PLAYING
                setPrivateFieldValue("mTargetA2dpState", 10);
                invokePrivateMethodWithParam("checkSinkSuspendState", new Reflector.Parameter(int.class, 2));
                assertMethodReturnValue(String.valueOf(mReturnValue), "true");
                break;

            case 10:
                printDebugLog("[API:callPrivateMethod] id is 10, and call onSinkPropertyChanged");
                String[] propValues = new String[2];
                propValues[0] = "State";
                propValues[1] = "disconnected";
                //if (mAudioDevices.get(device) == null) block
                invokePrivateMethodWithParam("onSinkPropertyChanged", 
                                new Reflector.Parameter(String.class, "MTKBT/dev_00_0D_FD_4B_57_E3"),
                                new Reflector.Parameter(String[].class, propValues));

                //if (address == null) block
                invokePrivateMethodWithParam("onSinkPropertyChanged",
                                new Reflector.Parameter(String.class, "00:0D:FD:4B:57:E3"),
                                new Reflector.Parameter(String[].class, propValues));

                //if (mAudioDevices.get(device) != null) and state is connected
                setPrivateFieldValue("mPlayingA2dpDevice", mDevice);
                propValues[1] = "connected";
                invokePrivateMethodWithParam("onSinkPropertyChanged",
                                new Reflector.Parameter(String.class, "MTKBT/dev_00_0D_FD_4B_57_E3"),
                                new Reflector.Parameter(String[].class, propValues));

                //if (mAudioDevices.get(device) != null), mAudioDevices is not null ,and preState is STATE_DISCONNECTED
                // and state is STATE_CONNECTED
                propValues[1] = "connected";
                setAudioDevicesList(0);
                invokePrivateMethodWithParam("onSinkPropertyChanged",
                                new Reflector.Parameter(String.class, "MTKBT/dev_00_0D_FD_4B_57_E3"),
                                new Reflector.Parameter(String[].class, propValues));
                
                //call isA2dpDisconnect method
                propValues[1] = "fmstartfailed";
                invokePrivateMethodWithParam("isA2dpDisconnect",
                                new Reflector.Parameter(String[].class, propValues));
                break;

            case 11:
                printDebugLog("[API:callPrivateMethod] id is 11, and call suspendSink and resumeSink");

                //call suspendSink, goto device == null block
                mA2dpInstance.suspendSink(null);

                //call suspendSink, goto device != null and state == null block
                setAudioDevicesList(1);
                mA2dpInstance.suspendSink(mDevice);

                //call suspendSink, goto device != null and state == CONNECTING block
                setAudioDevicesList(2);
                mA2dpInstance.suspendSink(mDevice);

                //call resumeSink, goto device == null block
                mA2dpInstance.resumeSink(null);

                //call resumeSink, goto else block
                setAudioDevicesList(2);
                mA2dpInstance.resumeSink(mDevice);
                break;

            case 12:
                printDebugLog("[API:callPrivateMethod] id is 12, and call isDisconnectSinkFeasible");
                setAudioDevicesList(0);
                invokePrivateMethodWithParam("isDisconnectSinkFeasible", 
                                new Reflector.Parameter(BluetoothDevice.class, mDevice));
                break;

            default:
                printDebugLog("[API:callPrivateMethod] the id is wrong");
                break;
        }
    }

    /**
    * @param fieldName field name in A2dpService
    * @param expResult expected result which to compare filed real value
    */
    private void assertFieldValue(String fieldName, String expResult) {
        printDebugLog("[API:assertFieldValue] fieldName is : " + fieldName + ", extResult is : " + expResult);
        String str = getPrivateFieldValue(fieldName).toString();
        printDebugLog("[API:assertFieldValue] the current field value is : " + str);
        Assert.assertEquals(str, expResult);
    }

    /**
    * @param methodResult call method to generate result
    * @param expResult expect result which to compare return value through call method
    */
    private void assertMethodReturnValue(String methodResult, String expResult) {
        printDebugLog("[API:assertMethodReturnValue] methodResult is : " + methodResult + ", extResult is : " + expResult);
        Assert.assertEquals(methodResult, expResult);
    }

    /**
    * set mAudioDevices
    * the BluetoothDevice is mDevice
    */
    private void setAudioDevicesList(int state) {
        mAudioDeviceList.clear();
        mAudioDeviceList.put(mDevice, state);
        setPrivateFieldValue("mAudioDevices", mAudioDeviceList);
    }

    /**
    * get private field value
    * @param object the field in object
    * @param fieldName field name in the object
    */
    private Object getPrivateFieldValue(String fieldName) {
        Object returnObject;
        try {
            returnObject = Reflector.get(mA2dpInstance, fieldName);
            return returnObject;
        } catch (NoSuchFieldException ex) {
            printDebugLog("[API:getPrivateFieldValue] NoSuchFieldException occured");
            return null;
        }
    }

    /**
    * set private field value
    * @param object object which contains field
    * @param fieldName field name
    * @param value the value you want to set to field
    */
    private void setPrivateFieldValue(String fieldName, Object value) {
        try {
            Reflector.set(mA2dpInstance, fieldName, value);
        } catch (NoSuchFieldException ex) {
            printDebugLog("[API:setPrivateFieldValue] NoSuchFieldException occured");
        }
    }

    /**
    * invoke private method which without parameters through Reflector
    * @param methodName method name which in A2dpService
    * @return return the result which generate by invoke the method
    */
    private Object invokePrivateMethodWithoutParam(String methodName) {
        Object retObject = Reflector.invoke(mA2dpInstance, methodName, (Reflector.Parameter[])null);
        return retObject;
    }

    /**
    * invoke private method which with parameters through Reflector
    * @param methodName method name in A2dpService
    * @return return the result which generate by invoke the method
    */
    @SuppressWarnings("unchecked")
    private Object invokePrivateMethodWithParam(String methodName, Reflector.Parameter... params) {
        Object retObject = Reflector.invoke(mA2dpInstance, methodName, params);
        return retObject;
    }

    /**
    * Print Debug Log
    * @param msg the message you want to record
    */
    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
