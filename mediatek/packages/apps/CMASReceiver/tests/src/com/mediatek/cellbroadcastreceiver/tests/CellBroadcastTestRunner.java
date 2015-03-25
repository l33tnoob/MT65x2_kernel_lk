
package com.mediatek.cellbroadcastreceiver.tests;

import android.test.InstrumentationTestSuite;
import android.util.Log;

import com.mediatek.cellbroadcastreceiver.tests.SendCMASTestCase;
import com.mediatek.cellbroadcastreceiver.tests.SendETWSTestCase;


import com.zutubi.android.junitreport.JUnitReportTestRunner;

public class CellBroadcastTestRunner extends JUnitReportTestRunner {

    private static final String TAG = "[CellBroadcast][UT][CellBroadcastTestRunner]";


    @Override
    public junit.framework.TestSuite getAllTests() {
        InstrumentationTestSuite suite = new InstrumentationTestSuite(this);
        // sendCMASTestCase cmasTestCase = new sendCMASTestCase();
        // Log.d(TAG, "getAllTests , created cmasTestCase " + cmasTestCase);
        suite.addTestSuite(SendCMASTestCase.class);
		suite.addTestSuite(SendETWSTestCase.class);
        Log.d(TAG, "getAllTests , sendCMASTestCase.class " + SendCMASTestCase.class);
        return suite;
    }

    @Override
    public ClassLoader getLoader() {
        return CellBroadcastTestRunner.class.getClassLoader();
    }

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
