package com.mediatek.systemupdate.otatest;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class WriteSourceTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(WriteSourceTests.class);
        return suite;
    }

}
