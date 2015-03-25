package com.mediatek.voicecommand.tests;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class VoiceCommandTestRunner extends InstrumentationTestRunner {
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(VoiceSettingsTest.class);
        suite.addTestSuite(VoiceCommandTest.class);
        suite.addTestSuite(JNIAdapterTest.class);
        suite.addTestSuite(CFManagerTest.class);
        return suite;
    }
}