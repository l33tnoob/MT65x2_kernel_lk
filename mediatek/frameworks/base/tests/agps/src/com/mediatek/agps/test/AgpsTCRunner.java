package com.mediatek.agps.test;

import junit.framework.TestSuite;
import android.test.suitebuilder.annotation.LargeTest;
import android.test.AndroidTestCase;
import android.test.InstrumentationTestRunner;
import android.test.InstrumentationTestSuite;
import android.util.Log;
import android.app.Activity;
import android.os.Bundle;

public class AgpsTCRunner extends InstrumentationTestRunner {
    
    private final String TAG = "agps_test";

    @Override
    public TestSuite getAllTests() {
        log("getAllTests");
        TestSuite suite = new InstrumentationTestSuite(this);
        suite.addTestSuite(AgpsTC.class);
        return suite;
    }
    
    @Override
    public ClassLoader getLoader() {
        log("getLoader");
        return AgpsTCRunner.class.getClassLoader();
    }
    

    @Override
    public void onCreate(Bundle arguments) {
        log("onCreate arg=" + arguments);
        super.onCreate(arguments);
    }
    
    private void log(String msg) {
        Log.d(TAG, msg);
    }
    
    
}