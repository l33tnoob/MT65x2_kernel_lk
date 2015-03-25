package android.atci.test;

import android.content.Intent;
import android.util.Log;
import android.test.AndroidTestCase;
import com.mediatek.atci.service.AtciIntentReceiver;

public class AtciIntentReceiverTestCase extends AndroidTestCase {
	  private AtciIntentReceiver mReceiver = null;
    public static final int TIME_LONG = 2000;
    static final String LOG_TAG = "AtciIntentReceiverTestCase";
    static final String BRINGUP = "com.mediatek.atci.service.bringup";
    static final String BOOTUP = "android.intent.action.BOOT_COMPLETED";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mReceiver = new AtciIntentReceiver();
    }
    
    @Override
    public void tearDown() throws Exception {
        super.tearDown();        
    }
    
    public void test01_Receive() {
        Intent intent1 = new Intent();
        intent1.setAction(BOOTUP);
        Log.d(LOG_TAG, "reveive intent1 test.");
        mReceiver.onReceive(getContext(), intent1);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void test02_Receive() {
        Intent intent2 = new Intent();
        intent2.setAction(BRINGUP);
        Log.d(LOG_TAG, "reveive intent2 test.");
        mReceiver.onReceive(getContext(), intent2);
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
}
