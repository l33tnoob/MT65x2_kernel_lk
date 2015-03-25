package com.mediatek.cts.window;

import com.mediatek.cts.window.stub.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;
import dalvik.annotation.ToBeFixed;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.Surface;
import android.view.Window;


@TestTargetClass(Window.class)
public class WindowLandscapeTest extends ActivityInstrumentationTestCase2<LandscapeStubActivity> {
    private final static String TAG = "WindowLandscapeTest";
    private LandscapeStubActivity mActivity;
    private Instrumentation mInstrumentation;
    static final int STRESS_TEST_TIMES = 50;

    public WindowLandscapeTest() {
        super("com.android.cts.stub", LandscapeStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
        mInstrumentation = getInstrumentation();
        Log.d(TAG, "setup");
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Log.d(TAG, "tearDown");
    }
    
    private void startFinishActivity(Class c) {
        final Context targetContext = mInstrumentation.getTargetContext();
        final Intent intent = new Intent(targetContext, c);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        LandscapeStubActivity land = (LandscapeStubActivity)mInstrumentation.startActivitySync(intent);
        
        mInstrumentation.waitForIdleSync();

        //assertTrue(land.isLandscape());
        
        land.finish();
        
        mInstrumentation.waitForIdleSync();
    }

    public void testIsLandscape() throws Exception {
        Log.d(TAG, "isLandscape = " + mActivity.isLandscape());
        assertTrue(mActivity.isLandscape());
        mActivity.finish();
        
        mInstrumentation.waitForIdleSync();

        // Run the stress test over STRESS_TEST_TIMES
        for (int i = STRESS_TEST_TIMES; i > 0 ; i--) {
             startFinishActivity(LandscapeStubActivity.class);	
        }   
    }
}
