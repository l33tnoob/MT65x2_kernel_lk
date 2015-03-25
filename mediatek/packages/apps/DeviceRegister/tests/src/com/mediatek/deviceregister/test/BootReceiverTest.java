package com.mediatek.deviceregister.test;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.test.AndroidTestCase;
import com.mediatek.deviceregister.BootReceiver;
import com.mediatek.deviceregister.Const;

public class BootReceiverTest extends AndroidTestCase {
    private BootReceiver mReceiver;
    private SharedPreferences mSharedPrf;
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mReceiver = new BootReceiver();
        mSharedPrf = mContext.getSharedPreferences(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Context.MODE_PRIVATE);
        mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, true).commit();        
    }
    
    public void testOnReceive(){        
        Intent bootIntent = new Intent(Const.ACTION_BOOTCOMPLETED);
        mReceiver.onReceive(mContext, bootIntent);      
        boolean haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        assertFalse(haveReceived);        
    }
    
    public void testOnReceiveWithIntentNull(){
        mReceiver.onReceive(mContext, null);
        boolean haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        assertTrue(haveReceived);
    }
    
    public void testOnReceiveWithInvalidAction(){
        Intent testIntent = new Intent("com.deviceregister.test.action.TEST");
        mReceiver.onReceive(mContext, testIntent);      
        boolean haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        assertTrue(haveReceived);
    }
    
    @Override
    protected void tearDown() throws Exception {
        mReceiver = null;
        mSharedPrf = null;
        super.tearDown();
    }     
    
}
