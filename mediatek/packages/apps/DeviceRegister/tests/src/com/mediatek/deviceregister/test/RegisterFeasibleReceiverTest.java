package com.mediatek.deviceregister.test;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.ServiceManager;
import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;
import com.mediatek.deviceregister.Const;
import com.mediatek.deviceregister.RegisterFeasibleReceiver;

public class RegisterFeasibleReceiverTest extends AndroidTestCase {

    private RegisterFeasibleReceiver registerReceiver;
    private MockReceiverContext mockContext;
    private SharedPreferences mSharedPrf;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        registerReceiver = new RegisterFeasibleReceiver();
        mockContext = new MockReceiverContext(mContext);
        mSharedPrf = mockContext.getSharedPreferences(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Context.MODE_PRIVATE);
        openSwitch();
    }

    @Override
    protected void tearDown() throws Exception {
        registerReceiver = null;
        mockContext = null;
        mSharedPrf = null;
        super.tearDown();
    }

    public void testOnReceive() {
        Intent registerFeasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        boolean haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        if (haveReceived) {
            registerReceiver.onReceive(mockContext, registerFeasibleIntent);
            haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                    Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
            assertTrue(haveReceived);
            mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, false).commit();
        }
        registerReceiver.onReceive(mockContext, registerFeasibleIntent);
        haveReceived = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        assertTrue(haveReceived);                      
    }
    
    public void testStartService(){
        mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, false).commit();        
        Intent registerFeasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        registerReceiver.onReceive(mockContext, registerFeasibleIntent);
        assertNotNull(mockContext.getReceivedIntent());         
    }
    
    public void testNotStartService(){
        mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, true).commit();
        Intent registerFeasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        registerReceiver.onReceive(mockContext, registerFeasibleIntent);
        assertNull(mockContext.getReceivedIntent());         
    }
    
    public void testOnReceiveWithIntentNull(){
        mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, false).commit();
        registerReceiver.onReceive(mockContext, null);
        boolean flagAfterOnReceive = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        assertFalse(flagAfterOnReceive);        
        
    }
    
    public void testOnReceiveWithInvalidAction(){
        mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, false).commit();  
        Intent testIntent = new Intent("com.mediatek.deviceregister.action.TEST");
        registerReceiver.onReceive(mockContext, testIntent);
        boolean flagAfterOnReceive = mSharedPrf.getBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST,
                Const.DEFALT_RECEIVED_FEASIBLE_BROADCAST);
        assertFalse(flagAfterOnReceive);             
    }
    
    public void testOnReceiveWithSwitchNotOpen() throws Exception {
        closeSwitch();
        mSharedPrf.edit().putBoolean(Const.KEY_RECEIVED_FEASIBLE_BROADCAST, false).commit();        
        Intent registerFeasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        registerReceiver.onReceive(mockContext, registerFeasibleIntent);
        assertNull(mockContext.getReceivedIntent());
    }
    
    private void closeSwitch()  throws Exception {
        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent mAgent = DmAgent.Stub.asInterface(binder);
        mAgent.setRegisterSwitch("0".getBytes());
    }
    
    private void openSwitch() throws Exception {
        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent mAgent = DmAgent.Stub.asInterface(binder);
        mAgent.setRegisterSwitch("1".getBytes());
    }
    
}
