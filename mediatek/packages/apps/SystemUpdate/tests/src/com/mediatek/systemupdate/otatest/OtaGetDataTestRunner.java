package com.mediatek.systemupdate.otatest;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class OtaGetDataTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();

        suite.addTestSuite(OtaGetDataTests.class);

        return suite;
    }

}
