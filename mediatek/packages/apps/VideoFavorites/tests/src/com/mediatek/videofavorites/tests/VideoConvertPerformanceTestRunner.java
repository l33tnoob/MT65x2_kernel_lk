package com.mediatek.videofavorites.tests;


import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class VideoConvertPerformanceTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getAllTests() {
        return new TestSuite(VideoConvertPerformanceTest.class);
    }

}