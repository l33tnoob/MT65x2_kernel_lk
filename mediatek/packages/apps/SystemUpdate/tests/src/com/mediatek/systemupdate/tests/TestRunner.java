package com.mediatek.systemupdate.tests;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class TestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(UpdateOptionTests.class);
        suite.addTestSuite(SdPkgInstallActivityTests.class);
        suite.addTestSuite(MainEntryTests.class);
        suite.addTestSuite(OtaPkgManagerActivityTests.class);

        return suite;
    }

}
