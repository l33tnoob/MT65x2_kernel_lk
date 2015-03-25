package com.mediatek.deviceregister.test;

import android.test.InstrumentationTestRunner;
import android.test.InstrumentationTestSuite;

import junit.framework.TestSuite;

public class TestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getAllTests() {
        InstrumentationTestSuite suite = new InstrumentationTestSuite(this);        
        suite.addTestSuite(BootReceiverTest.class);        
        suite.addTestSuite(RegisterFeasibleReceiverTest.class);
        suite.addTestSuite(ConfirmedSmsReceiverTest.class);
        suite.addTestSuite(RegisterServiceTest2.class);
        return suite;
    }
    
    
    
}
