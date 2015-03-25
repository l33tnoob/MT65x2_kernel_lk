package com.mediatek.systemupdate.otatest;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class TestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Md5InfoTests.class);
        suite.addTestSuite(PackageInfoTests.class);

        return suite;
    }

}
