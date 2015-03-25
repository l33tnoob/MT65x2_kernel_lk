package com.hissage.tests;

import android.test.InstrumentationTestRunner;
import junit.framework.TestSuite;

public class PluginTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(AcNotificationReceiverTest.class);
        suite.addTestSuite(ActivitiesManagerExtTest.class);
        //suite.addTestSuite(ChatManagerExtTest.class);
        //suite.addTestSuite(ContactManagerExtTest.class);
        suite.addTestSuite(GroupManagerExtTest.class);
        //suite.addTestSuite(IpMessagePluginExtTest.class);
        //suite.addTestSuite(MessageManagerExtTest.class);
        //suite.addTestSuite(NotificationsManagerExtTest.class);
        //suite.addTestSuite(ResourceManagerExtTest.class);
        //suite.addTestSuite(ServiceManagerExtTest.class);
        //suite.addTestSuite(SettingManagerExtTest.class);
        return suite;
    }
}
