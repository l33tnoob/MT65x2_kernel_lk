package com.mediatek.appguide.plugin.tests;  

import com.jayway.android.robotium.solo.Solo;

import android.app.Activity;
import android.app.Instrumentation;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;

@SuppressWarnings("rawtypes")
public class ApplicationGuideBase extends ActivityInstrumentationTestCase2 {
    protected Solo solo;
    protected Instrumentation mInst; 
    protected Activity mActivity;        
    protected static final String LOG_TAG="ApplicationGuildBase";
    
    protected void sleep(int time) {
        try {
            Thread.sleep(time);
            } catch(Exception e) {
            //ignore;
            }
        }
    
    @SuppressWarnings("unchecked")
    public ApplicationGuideBase(String benchmarkPackageIdString,Class activityClass) {
        super(activityClass);
    }
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.i(LOG_TAG, "super.setUp() done. Base");

        setActivityInitialTouchMode(false);
        Log.i(LOG_TAG, "setActivityInitialTouchMode done. Base");

        mInst = getInstrumentation();
        Log.i(LOG_TAG, "getInstrumentation done. Base");

        mActivity = getActivity();
        Log.i(LOG_TAG, "getActivity done. Base");

        solo = new Solo(mInst, mActivity);

        Log.i(LOG_TAG, "new Solo(mInst, mActivity) done. Base");
    }

    @Override
    protected void tearDown() throws Exception {
        try {
            // Robotium will finish all the activities that have been opened
            solo.finishOpenedActivities();
        } catch (Throwable e) {
            // ignored.
            e.printStackTrace();
        }
        super.tearDown();
    }
}
