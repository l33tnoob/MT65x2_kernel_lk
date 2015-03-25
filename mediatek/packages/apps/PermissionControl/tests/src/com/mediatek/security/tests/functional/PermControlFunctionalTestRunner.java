package com.mediatek.security.tests.functional;
import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;
import com.mediatek.common.featureoption.FeatureOption;

public class PermControlFunctionalTestRunner extends JUnitInstrumentationTestRunner {
	@Override
	public TestSuite getAllTests(){
	TestSuite tests = new TestSuite();
        tests.addTestSuite(PermControlMainUiTest.class);
        return tests;
    }
}

