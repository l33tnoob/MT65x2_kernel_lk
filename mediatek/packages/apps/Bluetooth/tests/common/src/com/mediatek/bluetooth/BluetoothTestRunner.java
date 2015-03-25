
package com.mediatek.bluetooth;

//import android.bluetooth.BluetoothAdapter;
import android.test.InstrumentationTestSuite;
import android.util.Log;

import com.mediatek.bluetooth.a2dp.A2dpServiceTestCase;
import com.mediatek.bluetooth.avrcp.AvrcpReceiverTestCase;
import com.mediatek.bluetooth.avrcp.AvrcpServiceTestCase;

import com.mediatek.bluetooth.hdp.HdpHealthTestCase;
import com.mediatek.bluetooth.hid.HidActivityTestCase;
import com.mediatek.bluetooth.hid.HidAlertActivityTestCase;
import com.mediatek.bluetooth.hid.HidReceiverTestCase;
import com.mediatek.bluetooth.hid.HidServiceTestCase;
import com.mediatek.bluetooth.hid.HidServiceTestCase2;

import com.mediatek.bluetooth.pan.PanAlertActivityTestCase;
import com.mediatek.bluetooth.pan.PanReceiverTestCase;
import com.mediatek.bluetooth.pan.PanServiceTestCase;

import com.mediatek.bluetooth.pbap.PbapBluetoothAuthenticatingTestCase;
import com.mediatek.bluetooth.pbap.PbapServerAuthorizeTestCase;
import com.mediatek.bluetooth.pbap.PbapServiceTestCase;

import com.zutubi.android.junitreport.JUnitReportTestRunner;


public class BluetoothTestRunner extends JUnitReportTestRunner {

    private static final String TAG = "[BT][UT][BluetoothTestRunner]";

//    private BluetoothAdapter mAdapter = null;
//
//    private boolean mBluetoothEnabledBeforeRun = false;

    @Override
    public junit.framework.TestSuite getAllTests() {
        InstrumentationTestSuite suite = new InstrumentationTestSuite(this);
        suite.addTestSuite(PanServiceTestCase.class);
        suite.addTestSuite(PanAlertActivityTestCase.class);
        suite.addTestSuite(PanReceiverTestCase.class);
        suite.addTestSuite(AvrcpServiceTestCase.class);
        suite.addTestSuite(AvrcpReceiverTestCase.class);
        suite.addTestSuite(HidAlertActivityTestCase.class);
        suite.addTestSuite(HidActivityTestCase.class);
        suite.addTestSuite(HidReceiverTestCase.class);
        suite.addTestSuite(HidServiceTestCase.class);
        suite.addTestSuite(HidServiceTestCase2.class);
        suite.addTestSuite(A2dpServiceTestCase.class);
        suite.addTestSuite(PbapBluetoothAuthenticatingTestCase.class);
        suite.addTestSuite(PbapServerAuthorizeTestCase.class);
        suite.addTestSuite(PbapServiceTestCase.class);
        suite.addTestSuite(HdpHealthTestCase.class);
        return suite;
    }

    @Override
    public ClassLoader getLoader() {
        return BluetoothTestRunner.class.getClassLoader();
    }

    /**
     * Turn the bluetooth on. Check the bluetooth is enabled or disabled
     * If the bluetooth is disabled,enable it.
     * @return If bluetooth is on ,return true, else return false
     */
//    private boolean turnBluetoothOn() {
//        Log.d(TAG, "[API:turnBluetoothOn] turnBluetoothOn enter");
//        int num = 0;
//        mAdapter = BluetoothAdapter.getDefaultAdapter();
//        if (mAdapter == null) {
//            Log.d(TAG, "The Device has no bluetooth");
//            this.finish(REPORT_VALUE_RESULT_ERROR, null); // if the device has no bluetooth,finish the test runner.
//            return false;
//        }
//        if (!mAdapter.isEnabled()) {
//            mAdapter.enable();
//            Log.d(TAG, "[API:turnBluetoothOn] The Bluetooth before run is disabled");
//        } else {
//            Log.d(TAG, "[API:turnBluetoothOn] The Bluetooth before run is enabled");
//            mBluetoothEnabledBeforeRun = true;
//            return true;
//        }
//        // do check in while circle to make sure bluetooth is enabled.
//        while (!mAdapter.isEnabled()) {
//            Log.d(TAG, "[API:turnBluetoothOn] current num is " + num);
//            num++; // count the num to calculate time-out
//            sleep(100); // free the CPU
//            if (mAdapter.isEnabled()) {
//                break;
//            }
//            // calculate the time-out
//            if (num > 100) {
//                Log.d(TAG, "[API:turnBluetoothOn] Turn bluetooth on time-out");
//                return false;
//            }
//        }
//        Log.d(TAG, "[API:turnBluetoothOn] turnBluetoothOn out");
//        return true;
//    }

//    private void turnBluetoothOff() {
//        if (!mBluetoothEnabledBeforeRun) {
//            if (mAdapter != null) {
//                if (mAdapter.isEnabled()) {
//                    mAdapter.disable();
//                    sleep(4000);
//                }
//            }
//        }
//    }

    /**
     * sleep some seconds
     * @param mSeconds
     */
    private void sleep(int mSeconds) {
        try {
            Thread.sleep(mSeconds);
        } catch (InterruptedException ex) {
            Log.e(TAG, "[API:sleep] InterruptedException :" + ex.toString());
        }
    }
}
