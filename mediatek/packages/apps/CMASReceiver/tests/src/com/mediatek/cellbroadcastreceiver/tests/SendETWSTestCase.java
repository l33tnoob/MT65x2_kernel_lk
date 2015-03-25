package com.mediatek.cellbroadcastreceiver.tests;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;
import android.util.Log;

import junit.framework.Assert;

import java.util.List;

public class SendETWSTestCase extends InstrumentationTestCase {

    private static final String TAG = "[CellBroadcast][CMASUT][sendCMASTestCase]";

    private static final boolean DEBUG = true;

    private Context mTestContext; // the test Context

    /*
     * Override this method to get the test Context. And check weather the bluetooth is on Init the jni class when you run
     * each test method,setUp will be called
     */
    @Override
    protected void setUp() throws Exception {
        super.setUp(); // shoule be the first statement
        printDebugLog("setUp +++++++");
        // get the test context
        mTestContext = this.getInstrumentation().getContext();

        Assert.assertNotNull(mTestContext);
    }

    /*
     * Do some clear work when you finished each test method,tearDown will be called
     */
    @Override
    protected void tearDown() throws Exception {

        printDebugLog("tearDown---------");

        mTestContext = null;

        super.tearDown(); // shouled be the last statement
    }

    /*
     * A2dpServiceTestCase constructor Should call
     */
    public SendETWSTestCase() {
        super();
    }

    public void test000TestCase() {

    }

    public void test021Dupulicate() {
		SendETWSMessages.testSendEtwsMessageNormal(mTestContext);
        //SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_001, "22333", 0, 0);
        sleep(12000);
        //SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_001, "22333", 0, 0);
        //sleep(5000);
        //SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_001, "22333", 0, 1);
        //sleep(5000);
        // assert sCmasIdSet.size() == 2
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

    /*
     * print the debug log
     */
    private void printErrorLog(String msg) {
        Log.e(TAG, msg);
    }
}
