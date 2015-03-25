package com.mtk.sanitytest.smstest;

import com.android.mms.ui.ComposeMessageActivity;
import com.jayway.android.robotium.solo.Solo; //import android.test.ActivityInstrumentationTestCase2;
import android.test.InstrumentationTestCase;
import android.test.suitebuilder.annotation.Smoke;
import android.content.ContentResolver;
import android.content.Intent;
import android.os.SystemClock;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;
import android.view.View;

import java.util.HashMap;
import java.util.Map;
import java.io.File;
import android.app.Activity;
import android.app.Instrumentation;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import android.widget.TextView;
import com.android.mms.ui.RecipientsEditor;

public class SMSTest extends InstrumentationTestCase {

    private Solo solo;
    private static final String TAG = "SMSTest";
    //private static final String CONFIG_FILE = "/sdcard/sanity_configure.xml";
    private static final String SMS_SENT_TABLE_URI = "content://sms/sent";
    private static final String SMS_INBOX_TABLE_URI = "content://sms/inbox";
    private static String sim1_number = "+886989649973";
    private static String sim2_number = "+886989649973";
    private Context mContext = null;
    private Activity mActivity = null;
    private Instrumentation mInst = null;

    // public SMSTest() {
    // super("com.android.mms",
    // com.android.mms.ui.ComposeMessageActivity.class);
    //
    // }

    @Override
    public void setUp() throws Exception {
        solo = new Solo(getInstrumentation());
        mContext = getInstrumentation().getContext();
        SystemClock.sleep(3000);

        mInst = getInstrumentation();
        Log.i(TAG, "sim1: "+sim1_number+" sim2:"+sim2_number);
    }

    @Override
    public void tearDown() throws Exception {

        try {
            // Robotium will finish all the activities that have been opened
            solo.finalize();
        } catch (Throwable e) {
            e.printStackTrace();
        }
        super.tearDown();
    }

    @Smoke
    public void test_sms1_receive() throws Exception {

        final String TEST_NAME = "test_sim1_receive";
        final String MESSAGE = "[" + TEST_NAME + "] ";
        int mSendCnt = 0;
        int mReceiveCnt = 0;
        final String number = sim1_number;

        // test start
        Log.v(TAG, MESSAGE + "test start");

        receive_sms("SIM1", sim1_number);
        Log.v(TAG, MESSAGE + "test end");
        assertTrue(TEST_NAME, true);
    }

    @Smoke
    public void test_sms2_receive() throws Exception {
        final String TEST_NAME = "test_sim2_receive";
        final String MESSAGE = "[" + TEST_NAME + "] ";
        int mSendCnt = 0;
        int mReceiveCnt = 0;
        final String number = sim2_number;

        // test start
        Log.v(TAG, MESSAGE + "test start");

        receive_sms("SIM2", sim2_number);
        Log.v(TAG, MESSAGE + "test end");
        assertTrue(TEST_NAME, true);
    }

    public void receive_sms(String simName, String number) {

        int sendCnt = 0;
        int receiveCnt = 0;

        // Send sms from sim2 to sim1
        Log.i(TAG, "Send message from simX to " + simName);

        // check current inbox/sent box count
        //mSendCnt = getCnt(Uri.parse(SMS_SENT_TABLE_URI), number);
        receiveCnt = getCnt(Uri.parse(SMS_INBOX_TABLE_URI), number);
        //Log.i(TAG, "mSendCnt: " + mSendCnt + " mReceiveCnt: " + mReceiveCnt);
        Log.i(TAG, "receiveCnt: " + receiveCnt);

        String contentMsg = "this is a test message send to " + number + " (" + simName + ")";
        sendSMS(contentMsg, 1, sim1_number, 1);

        //assertTrue("Send message failed from sim1 to sim2", 
        //           isSuccessfully(mSendCnt, Uri.parse(SMS_SENT_TABLE_URI), sim1_number));

        assertTrue("Receive message failed from simX to sim1", 
                   isSuccessfully(receiveCnt, Uri.parse(SMS_INBOX_TABLE_URI), number));

        SystemClock.sleep(2000);
    }
    
    // Send sms
    public void sendSMS(String message, int textIndex, String simNumber, int ListIndex) {

        // Start SMS Activity
        sendSmsIntent(message, simNumber);
        SystemClock.sleep(3000);
        // Get current Activity
        mActivity = solo.getCurrentActivity();
        SystemClock.sleep(2000);

        // Get send button id
        int Send_id = mActivity.getResources().getIdentifier("send_button_sms",
                "id", "com.android.mms");

        // Get view
        View Send = solo.getView(Send_id);
        Log.i(TAG, "Send button is: " + Send);

        // Click send button
        solo.clickOnView(Send);
        SystemClock.sleep(2000);
        // Click sim button
    }

    public void sendSmsIntent(String message, String number) {

        // Attach text message and file by intent
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent.setClassName("com.android.mms",
                "com.android.mms.ui.ComposeMessageActivity");
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("address", number);
        intent.putExtra("sms_body", message);
        intent.putExtra("compose_mode", false);
        intent.putExtra("exit_on_sent", true);
        mInst.getContext().startActivity(intent);
    }

    // get sent/receive count
    public int getCnt(Uri uri,String telNo) {
        String telNo2 = "";
        
        if (telNo.startsWith("+886")) {
            telNo2 = telNo.substring(4);
        } else {
            telNo2 = "+886"+telNo;
        }
        int cnt = -1;
        ContentResolver cr = mContext.getContentResolver();
        Cursor c = cr.query(uri, null, "address = ? or address = ?", new String[]{telNo, telNo2}, null);
        if (c != null) {
            Log.i(TAG, telNo+" Get cursor successfully");
            cnt = c.getCount();
            Log.i(TAG, telNo+" cnt: " + cnt);
            c.close();
        } else { 
            Log.i(TAG, "cursor is null");
        }
        return cnt;
    }

    public boolean isSuccessfully(int count_old, Uri uri, String telNo) {

        // every 2 seconds to check if receives
        for (int i = 0 ; i < 60 ; i++){
            SystemClock.sleep(2000);
            if ((count_old + 1) <= getCnt(uri, telNo)){
                Log.i(TAG, telNo + " Send/Receive message successfully");
                return true;
            }
        }

        return false;
    }

}
