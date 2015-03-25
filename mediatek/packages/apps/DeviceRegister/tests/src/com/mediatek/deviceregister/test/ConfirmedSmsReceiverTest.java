package com.mediatek.deviceregister.test;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.test.AndroidTestCase;

import com.mediatek.deviceregister.ConfirmedSmsReceiver;
import com.mediatek.deviceregister.Const;

public class ConfirmedSmsReceiverTest extends AndroidTestCase {

    private ConfirmedSmsReceiver mReceiver;
    MockReceiverContext mMockContext;
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mReceiver = new ConfirmedSmsReceiver();
        mMockContext = new MockReceiverContext(mContext);
    }
    
    public void testOnReceive() {
        Intent confirmedIntent = new Intent(Const.ACTION_CT_CONFIRMED_MESSAGE);
        mReceiver.onReceive(mMockContext, confirmedIntent);
        Intent receivedIntent = mMockContext.getReceivedIntent();
        assertNotNull(receivedIntent);  
        String action = receivedIntent.getAction();
        assertEquals(Const.ACTION_CT_CONFIRMED_MESSAGE, action);
    }
    
    public void testOnReceiveWithIntentNull(){
        mReceiver.onReceive(mMockContext, null);
        Intent receivedIntent = mMockContext.getReceivedIntent();
        assertNull(receivedIntent);
    }
    
    public void testOnReceiveWithInvalidAction(){
        Intent testIntent = new Intent("com.deviceregister.test.action.TEST");
        mReceiver.onReceive(mMockContext, testIntent);      
        Intent receivedIntent = mMockContext.getReceivedIntent();
        assertNull(receivedIntent);
    }

    @Override
    protected void tearDown() throws Exception {
        mReceiver = null;
        mMockContext = null;
        super.tearDown();
    }
        
}
