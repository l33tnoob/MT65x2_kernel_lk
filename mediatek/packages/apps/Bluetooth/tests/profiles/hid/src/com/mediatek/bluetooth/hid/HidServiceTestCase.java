
package com.mediatek.bluetooth.hid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHid;
import android.bluetooth.BluetoothInputDevice;
import android.bluetooth.IBluetoothHid;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.ServiceTestCase;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothUnitTestJni;
import com.mediatek.bluetooth.Reflector;

import junit.framework.Assert;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

public class HidServiceTestCase extends ServiceTestCase<BluetoothHidService> {

    private static final String TAG = "[BT][Hid][HidServiceTestCase]";

    private static final boolean DEBUG = true;

    private boolean mIsBluetoothOn = false;

    private BluetoothUnitTestJni mJniManager = null;

    private Context mTestContext;

    private IBluetoothHid mService;

    private static BluetoothHidService sHidService;

    private static BluetoothDevice sDevice;

    private static Object sUtHidService;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mService = IBluetoothHid.Stub.asInterface(service);
            printLog("onServiceConnected is called");
        }

        public void onServiceDisconnected(ComponentName className) {
            mService = null;
            printLog("onServiceDisconnected is called");
        }
    };

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        BluetoothHidService.sUtState = true;
        BluetoothAdapter ba = BluetoothAdapter.getDefaultAdapter();
        if (ba != null && !ba.isEnabled()) {
            ba.enable();
            sleep(4000);
        }
        if (ba != null && ba.isEnabled()) {
            mIsBluetoothOn = true;
        }
        mTestContext = this.getContext();
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_HID);

    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        mIsBluetoothOn = false;
        super.tearDown();
    }

    public HidServiceTestCase() {
        super(BluetoothHidService.class);
        // TODO Auto-generated constructor stub
    }

    public void test01Enable1() {
        // case description:1 service enable,2 return success ,
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test01Enable1");
        sleep(100);
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn) {
            Intent tmpIntent = new Intent(this.getContext(), BluetoothHidService.class);
            mTestContext.startService(tmpIntent);
            sleep(2000);
            mTestContext.stopService(tmpIntent);
            sleep(2000);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn is false");
        }
    }

    public void test02Enable2() {
        int waitingTime = 4000;

        // case description: 1 service enable, 2 return fail,

        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;1;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test02Enable2");
        sleep(100);
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn) {
            Intent tmpIntent = new Intent(this.getContext(), BluetoothHidService.class);
            mTestContext.startService(tmpIntent);
            sleep(waitingTime);
            mTestContext.stopService(tmpIntent);
            sleep(waitingTime);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn is false");
        }
    }

    public void test03Disable1() {
        /*
         * case description:1 service bond, 2 unbind service , 3 disable success
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test03Disable1");
        sleep(1000);
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test03 bindSerivce Success");
            }
            bindTimeOut();
            mTestContext.unbindService(mConnection);
            printLog("unbind HidService success");
            sleep(2000);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
    }

    public void test04Disable2() {
        /*
         * case description:1 service bond,2 unbind service ,3 disable fail
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;3;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test04Disable2");
        sleep(1000);
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test04 bindSerivce Success");
            }
            bindTimeOut();
            mTestContext.unbindService(mConnection);
            printLog("unbind HidService success");
            sleep(2000);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
    }

    public void test05Connect1() throws Exception {
        /*
         * case description(from settings): 1 turn on BT, 2 connect Hid device ,
         * 3 connect success,
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverConnectReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;4;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test05Connect1");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());

            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test05 bindSerivce Success");
            }
            bindTimeOut();
            try {
                constructBluetoothDevice();
                mService.connect(sDevice);
                sleep(2000);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "connect HidService success");
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
        sleep(2000);
    }

    public void test06Connect2() throws Exception {
        /*
         * case description(from settings): 1 turn on BT, 2 connect Hid device ,
         * 3 connect fail,
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverConnectReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;5;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("betty:run test06Connect2");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());

            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testConnect bindSerivce Success");
            }
            bindTimeOut();
            try {
                constructBluetoothDevice();
                mService.connect(sDevice);
                sleep(2000);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            printLog("connect HidService success");
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
        sleep(2000);
    }

    public void test07Disconnect1() throws Exception {
        // int waitingTime = 2000;
        /*
         * case description: 1 connect Hid device success, 2 disconnect Hid
         * device success ,3 btmtk_hidh_handle_disconnect_cnf,
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverConnectReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;4;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDisconnectReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;6;7C:ED:8D:68:48:E8"
        };
        printLog("run test07Disconnect1");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test07Disconnect1 bindSerivce Success");
            }
            bindTimeOut();
            try {
                constructBluetoothDevice();
                mService.connect(sDevice);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            printLog("connect HidService success");
            sleep(2000);
            try {
                mService.disconnect(sDevice);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            printLog("test07Disconnect1 Disconnect success");
            sleep(2000);
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
        sleep(2000);
    }

    public void test8Disconnect2() throws Exception {
        /*
         * case description: 1 connect Hid device success, 2 disconnect Hid
         * device fail ,3 btmtk_hidh_handle_disconnect_cnf,
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverConnectReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;4;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDisconnectReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;7;7C:ED:8D:68:48:E8"
        };
        printLog("betty:run test8Disconnect2");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());

            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test8Disconnect2 bindSerivce Success");
            }
            bindTimeOut();
            try {
                constructBluetoothDevice();
                mService.connect(sDevice);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            printLog("test8Disconnect2 connect HidService success");
            sleep(2000);
            try {
                mService.disconnect(sDevice);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            printLog("test10Disconnect2 Disconnect HidService fail");
            sleep(2000);
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
        sleep(2000);
    }

    public void test09getStateDevice() throws Exception {
        /*
         * case description(from settings): 1 turn on BT, 2 connect Hid device ,
         * 3 connect success,
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("test09getStateDevice");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());

            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test09getStateDevice bindSerivce Success");
            }
            bindTimeOut();
            sUtHidService = BluetoothHidService.getHidServiceUtInstance();
            constructBluetoothDevice();
            sleep(2000);
            Map<String, String> mUtStateMap1 = new HashMap<String, String>();
            mUtStateMap1.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_CONNECTING);
            Map<String, String> mUtStateMap2 = new HashMap<String, String>();
            mUtStateMap2.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_CONNECT);
            Map<String, String> mUtStateMap3 = new HashMap<String, String>();
            mUtStateMap3.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_DISCONNECT);
            Map<String, String> mUtStateMap4 = new HashMap<String, String>();
            mUtStateMap4.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_DISCONNECTING);
            Map<String, String> mUtStateMap5 = new HashMap<String, String>();
            mUtStateMap5.put(sDevice.getAddress(), "forUt");

            try {

                // mHidService = new BluetoothHidService();

                Assert.assertNotNull(sUtHidService);
                Reflector.set(sUtHidService, "mStateMap", mUtStateMap1);
                mService.getState(sDevice);
                sleep(1000);

                Reflector.set(sUtHidService, "mStateMap", mUtStateMap2);
                mService.getState(sDevice);
                mService.getCurrentDevices();
                sleep(1000);

                Reflector.set(sUtHidService, "mStateMap", mUtStateMap3);
                mService.getState(sDevice);
                sleep(1000);

                Reflector.set(sUtHidService, "mStateMap", mUtStateMap4);
                mService.getState(sDevice);
                sleep(1000);
                Reflector.set(sUtHidService, "mStateMap", mUtStateMap5);
                mService.getState(sDevice);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            sleep(1000);
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
    }

    public void test10reflectReceiver1() throws Exception {
        printLog("test10reflectReceiver1");
        BroadcastReceiver receiver;
        Intent tmpIntent = new Intent(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        try {
            sHidService = new BluetoothHidService();
            receiver = (BroadcastReceiver) Reflector.get(sHidService, "mReceiver");
            receiver.onReceive(mTestContext, tmpIntent);
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        }
    }

    public void test11reflectReceiver2() throws Exception {
        printLog("test11reflectReceiver2");
        BroadcastReceiver receiver;
        constructBluetoothDevice();
        Intent tmpIntent = new Intent(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        try {
            sHidService = new BluetoothHidService();
            receiver = (BroadcastReceiver) Reflector.get(sHidService, "mReceiver");
            receiver.onReceive(mTestContext, tmpIntent);
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        }
    }

    public void test12reflectReceiver3() throws Exception {
        printLog("test12reflectReceiver3");
        String[] jniLogs = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test14reflectReceiver2 bindSerivce Success");
            }

            bindTimeOut();
            constructBluetoothDevice();
            sUtHidService = BluetoothHidService.getHidServiceUtInstance();
            final int notify = 11;
            Map<String, String> mUtStateMap1 = new HashMap<String, String>();
            mUtStateMap1.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_CONNECT);
            Map<String, String> mUtStateMap2 = new HashMap<String, String>();
            mUtStateMap2.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_AUTHORIZE);
            Map mUtNotifyMap = new HashMap();
            mUtNotifyMap.put(sDevice.getAddress(), notify);
            BroadcastReceiver receiver;
            Intent intent1 = new Intent(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            Intent intent2 = new Intent(BluetoothDevice.ACTION_NAME_CHANGED);
            Bundle bundle1 = new Bundle();
            bundle1.putParcelable(BluetoothDevice.EXTRA_DEVICE, sDevice);
            bundle1.putInt(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.BOND_NONE);
            Bundle bundle2 = new Bundle();

            bundle2.putParcelable(BluetoothDevice.EXTRA_DEVICE, sDevice);
            try {
                // mHidService = new BluetoothHidService();
                Reflector.set(sUtHidService, "mStateMap", mUtStateMap1);
                Reflector.set(sUtHidService, "mNotifyMap", mUtNotifyMap);
                receiver = (BroadcastReceiver) Reflector.get(sUtHidService, "mReceiver");
                intent1.putExtras(bundle1);
                receiver.onReceive(mTestContext, intent1);
                intent2.putExtras(bundle2);
                receiver.onReceive(mTestContext, intent2);
                sleep(2000);
                Reflector.set(sUtHidService, "mStateMap", mUtStateMap2);
                receiver.onReceive(mTestContext, intent2);
            } catch (NoSuchFieldException e) {
                e.printStackTrace();
            }
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
    }

    public void test13setAndGetreportAndProtocalMode() throws Exception {
        /*
         * case description(from settings): 1 turn on BT, 2 bind service success
         * , 3 connect success,4,aidl call setAndGet_reportAndProtocalMode
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverSetReportReqNative;void;3;7C:ED:8D:68:48:E8;" +
                "BluetoothInputDevice.REPORT_TYPE_OUTPUT;BlueAngel HID UT set report",
                "Cb;0;false;sendServiceMsg;void;2;12;7C:ED:8D:68:48:E8",
                "Req;0;false;serverGetReportReqNative;void;3;7C:ED:8D:68:48:E8;" +
                "BluetoothInputDevice.REPORT_TYPE_INPUT;1",
                "Cb;0;false;sendServiceMsg;void;2;14;7C:ED:8D:68:48:E8",
                "Req;0;false;serverSendReportReqNative;void;2;7C:ED:8D:68:48:E8;BlueAngel HID UT send report",
                "Cb;0;false;sendServiceMsg;void;2;24;7C:ED:8D:68:48:E8",
                "Req;0;false;serverUnplugReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;10;7C:ED:8D:68:48:E8",
                "Req;0;false;serverSetProtocolReqNative;void;2;7C:ED:8D:68:48:E8;2",
                "Req;0;false;serverGetProtocolReqNative;void;1;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("test13setAndGet_reportAndProtocalMode");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());

            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("test13setAndGet_reportAndProtocalMode bindSerivce Success");
            }
            bindTimeOut();
            constructBluetoothDevice();
            sUtHidService = BluetoothHidService.getHidServiceUtInstance();
            Map<String, String> mUtStateMap = new HashMap<String, String>();
            mUtStateMap.put(sDevice.getAddress(), BluetoothHid.BT_HID_DEVICE_CONNECT);
            final byte reportId = 1;
            try {
                Reflector.set(sUtHidService, "mStateMap", mUtStateMap);
                mService.setReport(sDevice, BluetoothInputDevice.REPORT_TYPE_OUTPUT,
                        "BlueAngel HID UT set report");
                sleep(1000);
                mService.getReport(sDevice, BluetoothInputDevice.REPORT_TYPE_INPUT, reportId, 50);
                sleep(1000);
                mService.sendData(sDevice, "BlueAngel HID UT set report");
                sleep(1000);
                mService.virtualUnplug(sDevice);
                sleep(1000);
                mService.setProtocolMode(sDevice, 2);
                sleep(1000);
                mService.getProtocolMode(sDevice);
                sleep(1000);
                mService.getCurrentDevices();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            sleep(1000);
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
    }

    private void bindTimeOut() {
        int timeout = 0;
        while (mService == null) {
            Log.d(TAG, "mService in while is null");
            if (timeout > 100) {
                Log.d(TAG, "timeout is out of bound,then return");
                return;
            }
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            timeout++;
        }
    }

    @SuppressWarnings("unchecked")
    private void constructBluetoothDevice() throws Exception {
        try {
            Constructor con = BluetoothDevice.class.getDeclaredConstructor(String.class);
            Log.i(TAG, "Constructor:" + con);
            con.setAccessible(true);
            sDevice = (BluetoothDevice) con.newInstance("7C:ED:8D:68:48:E8");
            Log.i(TAG, "BluetoothDevice:" + sDevice);
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

    private void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void printLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
