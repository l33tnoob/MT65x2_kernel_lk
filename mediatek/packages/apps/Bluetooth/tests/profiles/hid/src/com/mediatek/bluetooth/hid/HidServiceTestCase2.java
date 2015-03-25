
package com.mediatek.bluetooth.hid;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothInputDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.ServiceTestCase;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothUnitTestJni;

public class HidServiceTestCase2 extends ServiceTestCase<BluetoothHidService> {

    private static final String TAG = "[BT][Hid][HidServiceTestCase2]";

    private static final boolean DEBUG = true;

    private boolean mIsBluetoothOn = false;

    private BluetoothUnitTestJni mJniManager = null;

    private Context mTestContext;

    private IBluetoothHidServerNotify mServerNotify;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mServerNotify = IBluetoothHidServerNotify.Stub.asInterface(service);
            printLog("onServiceConnected is called");
        }

        public void onServiceDisconnected(ComponentName className) {
            mServerNotify = null;
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

    public HidServiceTestCase2() {
        super(BluetoothHidService.class);
        // TODO Auto-generated constructor stub
    }

    public void test01ActivateReq() {
        int waitingTime = 2000;
        /*
         * case description: activity enable HidService
         */
        printLog("test01ActivateReq");
        sleep(1000);
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                bindTimeout();
                try {
                    mServerNotify.activateReq();
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return;
                }
                sleep(waitingTime);
                mTestContext.unbindService(mConnection);
                Log.i(TAG, "unbind HidService success");
                sleep(waitingTime);
            }
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
    }

    public void test02DeactivateReq() {
        int waitingTime = 2000;
        /*
         * case description: activity disable HidService
         */
        printLog("run test02DeactivateReq");
        sleep(1000);
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                bindTimeout();
                try {
                    mServerNotify.deactivateReq();
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return;
                }
                sleep(waitingTime);
                mTestContext.unbindService(mConnection);
                Log.i(TAG, "unbind HidService success");
                sleep(waitingTime);
            }
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
    }

    public void test03connectReq() {
        int waitingTime = 2000;
        /*
         * case description(from Activity): 1 turn on BT, 2 connect Hid device ,
         * 3 connect success,
         */
        printLog("run test03connectReq");
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testConnect bindSerivce Success");
            }
            bindTimeout();
            try {
                mServerNotify.connectReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "connect HidService success");
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
        sleep(waitingTime);
    }

    public void test04disconnectReq() {
        int waitingTime = 2000;
        /*
         * case description: 1 connect Hid device success, 2 activity
         * disconnectReq
         */
        printLog("test04disconnectReq");
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testdisConnect bindSerivce Success");
            }
            bindTimeout();
            try {
                mServerNotify.connectReq("7C:ED:8D:68:48:E8");
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "connect HidService success");
            sleep(waitingTime);
            try {
                mServerNotify.disconnectReq("7C:ED:8D:68:48:E8");
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "disconnect HidService success");
            sleep(waitingTime);
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
        sleep(waitingTime);
    }

    public void test05unplugReq() {
        int waitingTime = 2000;
        /*
         * case description(from settings): 1 turn on BT, 2 Unplug Hid device
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0", "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverUnplugReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;10;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0", "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test05unplugReq");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testConnect bindSerivce Success");
            }
            bindTimeout();
            try {
                mServerNotify.unplugReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "unplug HidService success");
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
        sleep(waitingTime);
    }

    public void test06sendReportReq() {
        int waitingTime = 2000;
        /*
         * case description(from settings): 1 turn on BT, 2 Activity
         * sendReportReq
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0", "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverSendReportReqNative;void;2;7C:ED:8D:68:48:E8;BlueAngel HID UT send report",
                "Cb;0;false;sendServiceMsg;void;2;24;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0", "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test06sendReportReq");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testConnect bindSerivce Success");
            }
            bindTimeout();
            try {
                mServerNotify.sendReportReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "connect HidService success");
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
        sleep(waitingTime);
    }

    public void test07setReportReq() {
        int waitingTime = 2000;
        /*
         * case description: 1 turn on BT 2 activity setReport
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0", "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverSetReportReqNative;void;3;7C:ED:8D:68:48:E8;BluetoothInputDevice.REPORT_TYPE_OUTPUT;BlueAngel HID UT set report",
                "Cb;0;false;sendServiceMsg;void;2;12;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0", "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("run test07setReportReq");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testConnect bindSerivce Success");
            }
            bindTimeout();
            try {
                mServerNotify.setReportReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
            Log.i(TAG, "connect HidService success");
            mTestContext.unbindService(mConnection);
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "isBluetoothOn false");
        }
        sleep(waitingTime);
    }

    public void test08getReportReqandLeft() {
        int waitingTime = 2000;
        /*
         * case description: 1 turn on BT, 2 activity getReport, 3 just block
         * some PTS action
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0", "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverGetReportReqNative;void;3;7C:ED:8D:68:48:E8;BluetoothInputDevice.REPORT_TYPE_INPUT;1",
                "Cb;0;false;sendServiceMsg;void;2;14;7C:ED:8D:68:48:E8",
                "Req;0;false;serverAuthorizeReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;27;7C:ED:8D:68:48:E8",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0", "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        printLog("test08getReportReqandLeft");
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mServerNotify == null) {
            Intent tmpIntent = new Intent(mTestContext, BluetoothHidService.class);
            if (mTestContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {
                printLog("testConnect bindSerivce Success");
                bindTimeout();
            }
            try {
                mServerNotify.getReportReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
                mServerNotify.setProtocolReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
                mServerNotify.getProtocolReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
                mServerNotify.setIdleReq("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
                mServerNotify.getIdleReq("7C:ED:8D:68:48:E8");
                mServerNotify.getStateByAddr("7C:ED:8D:68:48:E8");
                sleep(waitingTime);
                mServerNotify.authorizeReq("7C:ED:8D:68:48:E8", true);
                sleep(waitingTime);
                mServerNotify.finishActionReq();
                mServerNotify.clearService();
            } catch (RemoteException e) {
                e.printStackTrace();
                return;
            }
        } else {
            assertTrue(mIsBluetoothOn);
        }
        sleep(waitingTime);
    }
    private void bindTimeout() {
        int timeout = 0;
        while (mServerNotify == null) {
            Log.d(TAG, "mServerNotify in while is null");
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
