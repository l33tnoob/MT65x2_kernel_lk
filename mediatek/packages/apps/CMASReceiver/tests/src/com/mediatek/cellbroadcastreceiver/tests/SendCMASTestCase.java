package com.mediatek.cellbroadcastreceiver.tests;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;
import android.util.Log;

import junit.framework.Assert;

import java.util.List;

public class SendCMASTestCase extends InstrumentationTestCase {

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
    public SendCMASTestCase() {
        super();
    }

    public void test000TestCase() {

    }

    public void test021Dupulicate() {
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_001, "22333", 0, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_001, "22333", 0, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_001, "22333", 0, 1);
        sleep(5000);
        // assert sCmasIdSet.size() == 2
    }

    public void test022Dupulicate() {
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_002, "22333", 0, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_002, "22333", 2, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_002, "22333", 0, 4);
        sleep(5000);
        // assert sCmasIdSet.size() == 3
    }

    public void test023Dupulicate() {
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_003, "22333", 0, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_003, "22333", 1, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_003, "22333", 0, 3);
        sleep(5000);
        // assert sCmasIdSet.size() == 2
    }

    public void test024Dupulicate() {
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_004, "22333", 0, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_004, "22333", 1, 0);
        sleep(5000);
        SendCMASMessages.sendCMASMessageWithSpecLoc(mTestContext, SendCMASMessages.CMASMSG_PDU_004, "22333", 0, 3);
        sleep(5000);
        // assert sCmasIdSet.size() == 2
    }

    public void test031ContentTele() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_090);
        sleep(3000);
    }

    public void test032ContentEmail() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_091);
        sleep(3000);
    }

    public void test033ContentSite() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_092);
        sleep(3000);
    }

    public void test034ContentLong() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_093);
        sleep(3000);
    }

    public void test041MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_00);
        sleep(3000);
    }

    public void test042MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_01);
        sleep(3000);
    }

    public void test043MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_02);
        sleep(3000);
    }

    public void test044MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_03);
        sleep(3000);
    }

    public void test045MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_04);
        sleep(3000);
    }

    public void test046MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_05);
        sleep(3000);
    }

    public void test047MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_06);
        sleep(3000);
    }

    public void test048MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_07);
        sleep(3000);
    }

    public void test049MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_08);
        sleep(3000);
    }

    public void test050MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_09);
        sleep(3000);
    }

    public void test051MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_10);
        sleep(3000);
    }

    public void test052MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_11);
        sleep(3000);
    }

    public void test053MsgID() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_12);
        sleep(3000);
    }

    public void test061Update() {
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_101);
        sleep(3000);
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_102);
        sleep(3000);
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_103);
        sleep(3000);
        SendCMASMessages.sendCMASMessage(mTestContext, SendCMASMessages.CMASMSG_PDU_102);
        sleep(6000);
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
