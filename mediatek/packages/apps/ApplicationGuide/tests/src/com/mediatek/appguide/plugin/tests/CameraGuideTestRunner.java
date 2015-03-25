package com.mediatek.appguide.plugin.tests;
import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class CameraGuideTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite tests = new TestSuite();
        tests.addTestSuite(CameraGuideTester.class);
        return tests;
    }
}

