package com.mediatek.systemupdate.otatest;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class OtaSetDataTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();

        suite.addTestSuite(OtaSetDataTests.class);

        return suite;
    }

}
