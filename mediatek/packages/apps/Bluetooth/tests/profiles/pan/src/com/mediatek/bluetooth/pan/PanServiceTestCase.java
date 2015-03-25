/******************************************************************************
**
**  The file is used to test BluetoothPanService
**  Which implent ICleanUpInterface, which has only one method, which used to 
**  clean java filed value
**
**  This file is only for unit test.
**
******************************************************************************/

package com.mediatek.bluetooth.pan;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.IBluetoothPan;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.ServiceTestCase;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothUnitTestJni;
import com.mediatek.bluetooth.ICleanUpInterface;
import com.mediatek.bluetooth.IServiceCheckInterface;
import com.mediatek.bluetooth.Reflector;
import com.mediatek.bluetooth.Reflector.Parameter;

import junit.framework.Assert;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class PanServiceTestCase extends ServiceTestCase<BluetoothPanService> 
            implements ICleanUpInterface {

    private static final String TAG = "[BT][PAN-UT][PanServiceTestCase]";

    private static final boolean DEBUG = true;

    private boolean mIsBluetoothOn = false; // check the bluetooth weather is on

    private BluetoothUnitTestJni mJniManager = null;

    private IBluetoothPan mTestServicePan; // IBluetoothPan

    private Intent mStartIntent = new Intent("android.bluetooth.IBluetoothPan");

    private Intent mBindIntent = new Intent(IBluetoothPan.class.getName());

    private BluetoothPanService mPanService = null;

    private static final String SERVICE_FULL_NAME = "com.mediatek.bluetooth.pan.BluetoothPanService";

    private static final String SERVICE_STATE_FIELD_NAME = "sServerState";
    
    private static final String SERVICE_PAN_DEVICES_NAME = "mPanDevices";

    private static final String BLUETOOTH_PAN_DEVICE_FULL_NAME =
                "com.mediatek.bluetooth.pan.BluetoothPanService$BluetoothPanDevice";

    private static final String PAN_DEVICE_STATE_FIELD_NAME = "mState";

    private static final String PAN_DEVICE_LOCAL_ROLE_FIELD_NAME = "mLocalRole";
    
    private static final int ERROR_INT = Integer.MAX_VALUE; 
    
    private static final String DEVICE_ADDRESS = "00:15:83:52:83:17";

    /**
     * Override this method to get the test Context. And check wether the bluetooth is on 
     * Init the jni class when you run each test method,setUp will be called
     */
    @Override
    protected void setUp() throws Exception {
        super.setUp(); // shoule be the first statement
        printDebugLog("setUp +++++++");

        // check the bluetooth is on
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter.isEnabled()) {
            mIsBluetoothOn = true;
        }
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_PAN);
        BluetoothPanService.sUtState = true; // set BluetotohPanService to choose extpan_ut library
    }

    /**
     * Do some clear work when you finished each test method,tearDown will be called
     */
    @Override
    protected void tearDown() throws Exception {
        printDebugLog("tearDown---------");
        super.tearDown(); // should be the last statement
    }

    /**
     * PanServiceTestCase constructor Should call
     */
    public PanServiceTestCase() {
        super(BluetoothPanService.class);
    }

    /**
     * Enable and disable normally
     * Case description:
     *      1 start PanService,
     *      2 stop PanService,
     *      3 callback_pan_handle_activate_cnf,callback_pan_handle_deactivate_cnf parameter are both true
     *      4 close socket in normal way.
     */
    public void test001EnableDisable1() {

        printDebugLog("[API:test01EnableDisable1] : close socket normally");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };
        //set expected result.
        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test01EnableDisable1] set expected result success");
        } else {
            printDebugLog("[API:test01EnableDisable1] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            sleep(1500);
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printErrorLog("[API:test01EnableDisable1] mIsBluetoothOn is false");
        }
    }

    /**
     * Case description:
     *      1 start PanService,
     *      2 stop PanService,
     *      3 callback_pan_handle_activate_cnf and callback_pan_handle_deactivate_cnf parameter are both false
     *      4 close socket normally.
     */
    public void test002EnableDisable2() {

        printDebugLog("[API: test02EnableDisable2] : activate_cnf and deactivate_cnf are both false.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;0",
                "Cb;1;false;callback_pan_handle_deactivate_cnf;void;1;0",
                "Req;0;false;cleanServiceNative;void;0"
        };
        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test02EnableDisable2] set expected result success");
        } else {
            printDebugLog("[API:test02EnableDisable2] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            sleep(2500);

        } else {
            printErrorLog("[API:test02EnableDisable2] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * Force to stop 
     * Case description:
     *      1 start PanService,
     *      2 stop PanService,
     *      3 not received callback_pan_handle_deactivate_cnf,
     *      4 force to close socket.
     */
    public void test003EnableDisable3() {

        printDebugLog("[API:test03EnableDisable3] : force to close socket.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Req;0;true;forceClearServerNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test03EnableDisable3] set expected result success");
        } else {
            printDebugLog("[API:test03EnableDisable3] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            sleep(1500);
        } else {
            printErrorLog("[API:test03EnableDisable3] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This Function is to reject to authorize.
     * Notify user, but not accept the authorize
     * The mTetheringOn is true
     * The Pan role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind PanService
     *      3.stop PanService
     *      4.unbind PanService
     *      5.receive callback_pan_handler_connection_authorize_ind
     *      6.stop the socket normally
     */
    public void test004RejectingToAuthorize1() {

        printDebugLog("[API:test04RejectingToAuthorize1] reject to Authorize and mTetheringOn is true.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0"); 
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test04RejectingToAuthorize1] set expected result success");
        } else {
            printDebugLog("[API:test04RejectingToAuthorize1] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);
            sleep(2500);
        } else {
            printErrorLog("[API:test04RejectingToAuthorize1] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This Function is to reject to authorize.
     * Notify user, but not accept the authorize
     * BluetoothDevice is not null
     * The mTetheringOn is true
     * The Pan role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind PanService
     *      3.stop PanService
     *      4.unbind PanService
     *      5.receive callback_pan_handler_connection_authorize_ind
     *      6.stop the socket normally
     */
    public void test005RejectingToAuthorize2() {

        printDebugLog("[API:test05RejectingToAuthorize2] reject to Authorize and mTetheringOn is true.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:2"); 
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test05RejectingToAuthorize2] set expected result success");
        } else {
            printDebugLog("[API:test05RejectingToAuthorize2] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            BluetoothDevice bd = mockBluetoothDevice("00:15:83:52:83:17");
            try {
                mTestServicePan.connect(bd);
            } catch (RemoteException e) {
                printErrorLog("[API:test05RejectingToAuthorize2] RemoteException");
            }
            this.setBluetoothTethering(true);
            sleep(2500);
        } else {
            printErrorLog("[API:test05RejectingToAuthorize2] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This Function is to reject to authorize. Notify user.
     * But not accept the authorize
     * The mTetheringOn is false
     * The Pan role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind PanService
     *      3.stop PanService
     *      4.unbind PanService
     *      5.receive callback_pan_handle_connection_authorize_ind
     *      6.stop the socket normally
     */
    public void test006RejectingToAuthorize3() {

        printDebugLog("[API:test06RejectingToAuthorize3]: reject to Authorize and mTetheringOn is false");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;false;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test06RejectingToAuthorize3] set expected result success");
        } else {
            printDebugLog("[API:test06RejectingToAuthorize3] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(false);
            sleep(2500);
        } else {
            printErrorLog("[API:test06RejectingToAuthorize3] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect indication
     * And the mTetheringOn is false
     * BluetoothDevice is null
     * Test mPan isTetheringOn()
     * The Pan role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind PanService
     *      3.stop PanService
     *      4.unbind PanService
     *      5.receive callback_pan_handle_connect_ind
     *      6.stop the socket normally
     */
    public void test007ConnectInd1() {
        printDebugLog("[API:test07ConnectInd1]: the mTetheringOn is true.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;false;callback_pan_handle_connect_ind;void;3;0;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test07ConnectInd1] set expected result success");
        } else {
            printDebugLog("[API:test07ConnectInd1] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(false);
            sleep(2500);
        } else {
            printErrorLog("[API:test07ConnectInd1] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect indication
     * And the mTetheringOn is true
     * BluetoothDevice is null
     * Test mPan isTetheringOn()
     * The Pan role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind PanService
     *      3.stop PanService
     *      4.unbind PanService
     *      5.receive callback_pan_handle_connect_ind
     *      6.stop the socket normally
     */
    public void test008ConnectInd2() {
        printDebugLog("[API:test08ConnectInd2]: the mTetheringOn is true.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connect_ind;void;3;0;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("2:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test08ConnectInd2] set expected result success");
        } else {
            printDebugLog("[API:test08ConnectInd2] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);
            sleep(2500);
        } else {
            printErrorLog("[API:test08ConnectInd2] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect indication
     * And the mTetheringOn is true
     * BluetoothDevice is not null
     * Test mPan isTetheringOn()
     * The Pan role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind PanService
     *      3.stop PanService
     *      4.unbind PanService
     *      5.receive callback_pan_handle_connect_ind
     *      6.stop the socket normally
     */
    public void test009ConnectInd3() {
        printDebugLog("[API:test09ConnectInd3]: the mTetheringOn is true.");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_connect_ind;void;3;0;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0");
        checkLog.add("2:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test09ConnectInd3] set expected result success");
        } else {
            printDebugLog("[API:test09ConnectInd3] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.setCleanUpInterface(this);
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);

            this.setBluetoothTethering(true);
            sleep(3500);
        } else {
            printErrorLog("[API:test09ConnectInd3] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect cnf and return directly
     * BluetoothDevice is null
     * Case description:
     *      1.start PanService
     *      2.stop PanService
     *      3.receive callback_pan_handle_connect_cnf
     *      4.stop the socket normally
     */
    public void test010ConnectCnf1() {

        printDebugLog("[API:test10ConnectCnf1]: run test09ConnectCnf1, which remote device is null");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;false;callback_pan_handle_connect_cnf;void;3;0;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test10ConnectCnf1] set expected result success");
        } else {
            printDebugLog("[API:test10ConnectCnf1] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());

        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            sleep(2500);
        } else {
            printErrorLog("[API:test10ConnectCnf1] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect cnf
     * BluetoothDevice is not null by setting authorize_ind
     * connect_cnf first param is false
     * mTetheringOn is true
     * The PAN role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind service
     *      3.unbind service
     *      4.stop PanService
     *      5.receive callback_pan_handle_connect_cnf
     *      6.stop the socket normally
     */
    public void test011ConnectCnf2() {
        printDebugLog("[API:test11ConnectCnf2]: run testConnectCnf2, which remote device is ok and first para is false");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_connect_cnf;void;3;0;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0");
        checkLog.add("0:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test11ConnectCnf2] set expected result success");
        } else {
            printDebugLog("[API:test11ConnectCnf2] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test11ConnectCnf2] mIsBluetoothOn is false");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect cnf
     * BluetoothDevice is not null by setting authorize_ind
     * The first param is true
     * mTetheringOn is true
     * The PAN role is NAP
     * Case description:
     *      1.start PanService
     *      2.bind service
     *      3.unbind service
     *      4.stop PanService
     *      5.receive callback_pan_handle_connect_cnf
     *      6.stop the socket normally
     */
    public void test012ConnectCnf3() {
        printDebugLog("[API:test12ConnectCnf3]: run testConnectCnf3, which remote device is ok and first para is true");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_connect_cnf;void;3;1;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0");
        checkLog.add("2:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test12ConnectCnf3] set expected result success");
        } else {
            printDebugLog("[API:test12ConnectCnf3] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test12ConnectCnf3] The Bluetooth is off");
            Assert.assertTrue(mIsBluetoothOn);
        }

    }

    /**
     * This test function is to test connect cnf
     * BluetoothDevice is not null by setting authorize_ind
     * first param is false
     * mTetheringOn is true
     * The PAN role is GN
     * Case description:
     *      1.start PanService
     *      2.bind service
     *      3.unbind service
     *      4.stop PanService
     *      5.receive callback_pan_handle_connect_cnf
     *      6.stop the socket normally
     */
    public void test013ConnectCnf4() {
        printDebugLog("[API:test13ConnectCnf4]: which remote device is ok and first para is false");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;1;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_connect_cnf;void;3;0;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:1");
        checkLog.add("0:1");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test13ConnectCnf4] set expected result success");
        } else {
            printDebugLog("[API:test13ConnectCnf4] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test13ConnectCnf4] The Bluetooth is off");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * This test function is to test connect cnf
     * BluetoothDevice is not null by setting authorize_ind The first para is true
     * mTetheringOn is true
     * The PAN role is GN
     * Case description:
     *      1.start PanService
     *      2.bind service
     *      3.unbind service 4.stop PanService
     *      5.receive callback_pan_handle_connect_cnf
     *      6.stop the socket normally
     */
    public void test014ConnectCnf5() {
        printDebugLog("[API:test14ConnectCnf5]: which remote device is ok and first para is false");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;1;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_connect_cnf;void;3;1;00:15:83:52:83:17;1",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:1");
        checkLog.add("2:1");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test14ConnectCnf5] set expected result success");
        } else {
            printDebugLog("[API:test14ConnectCnf5] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test14ConnectCnf5] The Bluetooth is off");
            Assert.assertTrue(mIsBluetoothOn);
        }
    }

    /**
     * Test disconnect_ind callback.
     * The BluetoothDevice is null
     */
    public void test015DisconnectInd1() {
        printDebugLog("[API:test15DisconnectInd1]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;true;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;false;callback_pan_handle_disconnect_ind;void;1;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test15DisconnectInd1] set expected result success");
        } else {
            printDebugLog("[API:test15DisconnectInd1] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            sleep(2500);
        } else {
            printErrorLog("[API:test15DisconnectInd1] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_ind callback.
     * The BluetoothDevice is not null
     * The PAN role is NAP
     */
    public void test016DisconnectInd2() {
        printDebugLog("[API:test16DisconnectInd2]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;true;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Cb;1;false;callback_pan_handle_disconnect_ind;void;1;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test16DisconnectInd2] set expected result success");
        } else {
            printDebugLog("[API:test16DisconnectInd2] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test16DisconnectInd2] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_ind callback.
     * The BluetoothDevice is not null
     * The PAN role is GN
     */
    public void test017DisconnectInd3() {
        printDebugLog("[API:test17DisconnectInd3]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;true;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;1;00:15:83:52:83:17",
                "Cb;1;false;callback_pan_handle_disconnect_ind;void;1;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:1");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test17DisconnectInd3] set expected result success");
        } else {
            printDebugLog("[API:test17DisconnectInd3] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test17DisconnectInd3] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_cnf callback.
     * The BluetoothDevice is null
     * The disconnect_cnf first param is false
     */
    public void test018DisconnectCnf1() {
        printDebugLog("[API:test18DisconnectCnf1]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;false;callback_pan_handle_disconnect_cnf;void;2;0;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test18DisconnectCnf1] set expected result success");
        } else {
            printDebugLog("[API:test18DisconnectCnf1] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(2500);
        } else {
            printErrorLog("[API:test18DisconnectCnf1] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_cnf callback.
     * The disconnect_cnf first param is true
     * BluetoothDevice is not null
     * The PAN role is NAP
     */
    public void test019DisconnectCnf2() {
        printDebugLog("[API:test19DisconnectCnf2]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Cb;1;false;callback_pan_handle_disconnect_cnf;void;2;1;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test19DisconnectCnf2] set expected result success");
        } else {
            printDebugLog("[API:test19DisconnectCnf2] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test19DisconnectCnf2] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_cnf callback.
     * The disconnect_cnf first param is false
     * The BluetoothDevice is not null
     * The PAN role is NAP
     */
    public void test020DisconnectCnf3() {
        printDebugLog("[API:test20DisconnectCnf3]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;0;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_disconnect_cnf;void;2;0;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:0");
        checkLog.add("2:0");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test20DisconnectCnf3] set expected result success");
        } else {
            printDebugLog("[API:test20DisconnectCnf3] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test20DisconnectCnf3] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_cnf callback.
     * The disconnect_cnf first param is true
     * BluetoothDevice is not null
     */
    public void test021DisconnectCnf4() {
        printDebugLog("[API:test21DisconnectCnf4]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;1;00:15:83:52:83:17",
                "Cb;1;false;callback_pan_handle_disconnect_cnf;void;2;1;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:1");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test21DisconnectCnf4] set expected result success");
        } else {
            printDebugLog("[API:test21DisconnectCnf4] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);

            sleep(3500);
        } else {
            printErrorLog("[API:test21DisconnectCnf4] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * Test disconnect_cnf callback.
     * The disconnect_cnf first param is false
     * The BluetoothDevice is not null
     */
    public void test022DisconnectCnf5() {
        printDebugLog("[API:test22DisconnectCnf5]: run testDisconnectInd");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;1;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;true;callback_pan_handle_connection_authorize_ind;void;2;1;00:15:83:52:83:17",
                "Cb;1;true;callback_pan_handle_disconnect_cnf;void;2;0;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("1:1");
        checkLog.add("2:1");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test22DisconnectCnf5] set expected result success");
        } else {
            printDebugLog("[API:test22DisconnectCnf5] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new AuthorizeStateCheck());
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            mPanService = this.getService();
            Assert.assertNotNull(mPanService);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            this.setBluetoothTethering(true);
            sleep(3500);
        } else {
            printErrorLog("[API:test22DisconnectCnf5] The Bluetooth is off");
            Assert.assertNotNull(null);
        }
    }

    /**
     * The IBluetoothPan stub functions. First should start and bind service.
     * Call stub functions
     */
    public void test023BluetoothPanStub() {

        printDebugLog("[API:test23BluetoothPanStub] test21BluetoothPanStub enter");
        String jniLogs[] = {
                "Req;0;false;initServiceNative;true;0",
                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_activate_cnf;void;1;1",
                "Cb;1;false;callback_pan_handle_disconnect_ind;void;1;00:15:83:52:83:17",
                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;1;true;callback_pan_handle_deactivate_cnf;void;1;1",
                "Req;0;false;cleanServiceNative;void;0"
        };

        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add("true");
        checkLog.add("false");

        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCleanUpInterface(this);
        if (mJniManager.setCaseExpectedResults(checkLog)) {
            printDebugLog("[API:test23BluetoothPanStub] set expected result success");
        } else {
            printDebugLog("[API:test23BluetoothPanStub] set expected result failed");
            Assert.assertTrue(false);
        }
        mJniManager.addServiceCheckInterface(new EnableDisableStateCheck());
        sleep(500);

        if (mIsBluetoothOn) {
            this.startService(mStartIntent);
            IBinder binder = this.bindService(mBindIntent);
            while (binder == null) {
                printDebugLog("binder is null");
            }
            mTestServicePan = IBluetoothPan.Stub.asInterface(binder);
            Assert.assertNotNull(mTestServicePan);
            BluetoothDevice bd = mockBluetoothDevice("00:15:83:52:83:17");

            try {
                mTestServicePan.connect(bd);
                sleep(100);
                mTestServicePan.getConnectedDevices();
                mTestServicePan.getState(bd);
                mTestServicePan.disconnect(bd);
                mTestServicePan.isTetheringOn();
            } catch (RemoteException ex) {
                printErrorLog(ex.toString());
            }
            this.sleep(2500);
        }
    }

    /**
     * Assert boolean field, check fiela value is equal result or not
     * @param booleanFieldName boolean field name which in BluetoothPanService
     * @param result boolean field value which shoule be
     *
     */
    @SuppressWarnings("finally")
    private void assertBooleanState(String booleanFieldName, String result) {
        if (booleanFieldName == null) {
            printErrorLog("[API:assertBooleanState] boolean field name is null");
            return;
        }
        if (mPanService != null) {
            boolean b = getBooleanFieldValue(SERVICE_FULL_NAME, booleanFieldName, mPanService);
            printDebugLog("[API:assertBooleanState] expectedResult is " + result + ", real check result is : " + b);
            Assert.assertEquals(result, String.valueOf(b));
        } else {
            printErrorLog("[API:assertBooleanState] mPanService is null");
        }
    }

    /**
     * Assert field value is equal result or not
     * @param field field name is BluetoothPanService
     * @param result excepted result which the field should be
     *
     */
    private void assertPanDeviceState(String field, String result) {
        if (mPanService != null) {
            printDebugLog("assertPanDeviceState enter");
            int i = getPanDevicesIntegerFieldValue(field);
            printDebugLog("[API:assertPanDeviceState] expected result is : " + result + ", real value is : " + i);
            Assert.assertEquals(String.valueOf(i), result);
        } else {
            printErrorLog("[API:assertPanDeviceState] mPanService is null");
        }
    }

    /**
     * This Class is to check mServerState field
     * @author mtk54453
     *
     */
    private class EnableDisableStateCheck implements IServiceCheckInterface {

        @Override
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 1:
                case 2:
                    assertBooleanState(SERVICE_STATE_FIELD_NAME, expectedResult);
                    break;

                default:
                    printErrorLog("[API:EnableDisableStateCheck-checkState] unknown id");
                    break;
            }
        }

    }

    /**
     * This Class is to check BluetothPanDevice field<br>
     * mState and mLocalRole
     * @author mtk54453
     *
     */
    private class AuthorizeStateCheck implements IServiceCheckInterface {

        @Override
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 3:
                case 4:
                case 5:
                case 7:
                    String[] ss = expectedResult.split(":");
                    assertPanDeviceState(PAN_DEVICE_STATE_FIELD_NAME, ss[0]);
                    assertPanDeviceState(PAN_DEVICE_LOCAL_ROLE_FIELD_NAME, ss[1]);
                    break;

                default:
                    printErrorLog("[API:AuthorizeStateCheck-checkState] unknown id");
                    break;
            }
        }

    }

    /**
     * Get boolean field value by reflecting.
     * @param classFullName the class full name you want to search the field
     * @param fieldName the field name you want to search
     * @param obj search in which object
     * @return the boolean value
     */
    @SuppressWarnings("unchecked")
    private boolean getBooleanFieldValue(String classFullName, String fieldName, Object obj) {
        boolean retValue = false;
        if (classFullName == null || fieldName == null || obj == null) {
            printErrorLog("[API:getBooleanFieldValue] The param is wrong.");
            return false;
        }
        try {
            Class cls = Class.forName(classFullName);
            Field field = cls.getDeclaredField(fieldName);
            printDebugLog("[API:getBooleanFieldValue] field is " + field);
            field.setAccessible(true);
            retValue = field.getBoolean(obj);
            printDebugLog("[API:getBooleanFieldValue] return value is " + retValue);
        } catch (ClassNotFoundException e) {
            printErrorLog("[API:getBooleanFieldValue] class not found exception");
        } catch (SecurityException e) {
            printErrorLog("[API:getBooleanFieldValue] SecurityException for getDeclaredField");
        } catch (NoSuchFieldException e) {
            printErrorLog("[API:getBooleanFieldValue] field not found exception");
        } catch (IllegalArgumentException e) {
            printErrorLog("[API:getBooleanFieldValue] IllegalArgumentException for getBoolean");
        } catch (IllegalAccessException e) {
            printErrorLog("[API:getBooleanFieldValue] IllegalAccessException for getBooleean");
        }
        return retValue;
    }

    /**
     * Get inner class field value.<br>
     * The inner class object is the filed of the outer class<br>
     * 
     * @param outerClassName String<br> outer class full name
     * @param innerClassName String<br> inner class full name
     * @param outerFieldName String<br> outer class field name which is the inner class object
     * @param innerFieldName String<br> inner class field name
     * @param outerObj Object<br> outer class object
     * @return return the inner class field value
     */
    @SuppressWarnings("unchecked")
/*    private int getInnerClassIntegerFieldValue(String outerClassName, String innerClassName, 
            String outerFieldName, String innerFieldName, Object outerObj) {

        printDebugLog("[API:getInnerClassIntegerFieldValue] getInnerClassIntegerFieldValue enter");
        int retValue = -1;
        if (outerClassName == null || innerClassName == null || outerFieldName == null
                || innerFieldName == null || outerObj == null) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] The param is wrong.");
            return -1;
        }
        try {
            Class outerClass = Class.forName(outerClassName);
            Class innerClass = Class.forName(innerClassName);
            printDebugLog("[API:getInnerClassIntegerFieldValue] outer class is : " + outerClass.toString());
            printDebugLog("[API:getInnerClassIntegerFieldValue] inner class is : " + innerClass.toString());
            Field outerField = outerClass.getDeclaredField(outerFieldName);
            Field innerField = innerClass.getDeclaredField(innerFieldName);
            printDebugLog("[API:getInnerClassIntegerFieldValue] outer class field is : " + outerField.toString());
            printDebugLog("[API:getInnerClassIntegerFieldValue] inner class field is : " + innerField.toString());
            outerField.setAccessible(true);
            innerField.setAccessible(true);
            Object o = outerField.get(outerObj);
            retValue = innerField.getInt(o);
        } catch (ClassNotFoundException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] class not found exception");
        } catch (SecurityException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] SecurityException");
        } catch (NoSuchFieldException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] field not found exception");
        } catch (IllegalArgumentException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] IllegalArgumentException");
        } catch (IllegalAccessException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] IllegalAccessException");
        }
        return retValue;
    }
*/

    /**
     * Get BluetoothPanDevice class integer filed value
     * @param fieldName String<br>
     *          field name which in BluetoothPanDevice
     */
    private int getPanDevicesIntegerFieldValue(String fieldName) {
        printDebugLog("[API:getPanDevicesIntegerFieldValue] getPanDevicesIntegerFieldValue enter");
        if (fieldName == null) {
            printErrorLog("[API:getPanDevicesIntegerFieldValue] param is wrong");
            return ERROR_INT;
        }

        int retValue = ERROR_INT;
        try {
            //get service class
            Class cls = Class.forName(SERVICE_FULL_NAME);
            printDebugLog("[API:getInnerClassIntegerFieldValue] class is : " + cls.toString());
            //get mPanDevices
            Field field = cls.getDeclaredField(SERVICE_PAN_DEVICES_NAME);
            printDebugLog("[API:getInnerClassIntegerFieldValue] pan devices hashmap is : " + field.toString());
            field.setAccessible(true);
            //get BluetoothPanDevices class
            Class deviceClass = Class.forName(BLUETOOTH_PAN_DEVICE_FULL_NAME);
            printDebugLog("[API:getInnerClassIntegerFieldValue] pan device class is : " + deviceClass.toString());
            //get mState or mLocalRole according fieldName
            Field ff = deviceClass.getDeclaredField(fieldName);
            printDebugLog("[API:getInnerClassIntegerFieldValue] get BluetoothPanDevices integer field is : "
                    + ff.toString());
            ff.setAccessible(true);
            Object o = field.get(mPanService);
            //cast to HashMap
            HashMap<BluetoothDevice, Object> ss = (HashMap<BluetoothDevice, Object>) o;
            Iterator it = ss.keySet().iterator();
            Object oo = null;
            //iterator the hashmap and get the BluetoothPanDevice object
            while (it.hasNext()) {
                BluetoothDevice entry = (BluetoothDevice)it.next();
                //BluetoothDevice b = (BluetoothDevice)entry.getKey();
                oo = ss.get(entry);
                if (entry.getAddress().equals(DEVICE_ADDRESS)) {
                    if (oo != null) {
                        retValue = ff.getInt(oo);
                        break;
                    }
                }
            }
        } catch (ClassNotFoundException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] class not found exception");
        } catch (SecurityException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] SecurityException");
        } catch (NoSuchFieldException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] field not found exception");
        } catch (IllegalArgumentException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] IllegalArgumentException");
        } catch (IllegalAccessException e) {
            printErrorLog("[API:getInnerClassIntegerFieldValue] IllegalAccessException");
        }
        return retValue;
    }

    /**
     * The method is to mock a BluetoothDevice.
     * 
     * @param btAddress the bluetoothdevice address
     * @return BluetoothDevice
     */
    @SuppressWarnings("unchecked")
    public BluetoothDevice mockBluetoothDevice(String btAddress) {
        try {
            Class cls = Class.forName("android.bluetooth.BluetoothDevice");
            printDebugLog("[API:mockBluetoothDevice] the class is " + cls.toString());
            if (cls == null) {
                printErrorLog("[API:mockBluetoothDevice] Can't find android.bluetooth.BluetoothDevice class");
                return null;
            }
            Constructor con = cls.getDeclaredConstructor(String.class);
            printDebugLog("[API:mockBluetoothDevice] BluetoothDevice constructor is " + con.toString());
            con.setAccessible(true);
            BluetoothDevice bd = (BluetoothDevice) con.newInstance(btAddress);
            printDebugLog("[API:mockBluetoothDevice] BluetoothDevice address is" + bd.getAddress().toString());
            return bd;
        } catch (ClassNotFoundException ex) {
            printDebugLog("[API:mockBluetoothDevice] Exception: " + ex.toString());
            return null;
        } catch (NoSuchMethodException ex) {
            printDebugLog("[API:mockBluetoothDevice] Exception: " + ex.toString());
            return null;
        } catch (InvocationTargetException ex) {
            printDebugLog("[API:mockBluetoothDevice] Exception: " + ex.toString());
            return null;
        } catch (IllegalAccessException ex) {
            printDebugLog("[API:mockBluetoothDevice] Exception: " + ex.toString());
            return null;
        } catch (InstantiationException ex) {
            printDebugLog("[API:mockBluetoothDevice] Exception: " + ex.toString());
            return null;
        }
    }

    /**
     * call BluetoothPanService mPan setBluetoothTethering to set mTetheringOn
     */
    private void setBluetoothTethering(boolean value) {
        if (mTestServicePan != null) {
            try {
                mTestServicePan.setBluetoothTethering(value);
            } catch (RemoteException e) {
                printErrorLog("[API:setBluetoothTethering] " + e.toString());
            }
        } else {
            printErrorLog("[API:setBluetoothTethering] mTestServicePan is null.");
            Assert.assertNotNull(mTestServicePan);
        }
    }

    /**
     * sleep some seconds
     * @param mSeconds
     */
    private void sleep(int mSeconds) {
        try {
            Thread.sleep(mSeconds);
        } catch (InterruptedException ex) {
            printErrorLog("[API:sleep] InterruptedException :" + ex.toString());
        }
    }

    @Override
    public void cleanUp() {
        printDebugLog("[API:cleanUp] cleanUp enter");
        if (mJniManager != null) {
            mJniManager = null;
        }
        mIsBluetoothOn = false;
        BluetoothPanService.sUtState = false;
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
    private static void printErrorLog(String msg) {
        Log.e(TAG, msg);
    }

}
