package com.mediatek.systemupdate.otatest;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class WriteTargetTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(WriteTargetTests.class);

        return suite;
    }

}
