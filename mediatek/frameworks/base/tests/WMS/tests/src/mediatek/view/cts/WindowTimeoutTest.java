package com.mediatek.cts.window;

import com.mediatek.cts.window.stub.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;
import dalvik.annotation.ToBeFixed;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Debug;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.Surface;
import android.view.Window;

import java.io.*;
import java.util.Arrays;
import java.util.List;

@TestTargetClass(Window.class)
public class WindowTimeoutTest extends ActivityInstrumentationTestCase2<WindowTimeoutStubActivity> {
    private final static String TAG = "WindowTimeoutTest";
    private WindowTimeoutStubActivity mActivity;
    private ContentResolver mResolver;
    private int APP_TRANSITION_TIMEOUT = 5000;
    private int APP_FREEZE_TIMEOUT = 5000;
    private int BUFFER_TIME = 1000;
    private Object mLock = new Object();
    private Handler mH = new H();


    public WindowTimeoutTest() {
        super("com.android.cts.stub", WindowTimeoutStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
        mResolver = mActivity.getContentResolver();
        // Initialize the orientation to portrait
        Settings.System.putInt(mResolver, Settings.System.USER_ROTATION, Surface.ROTATION_0);
        Log.d(TAG, "setup");
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Log.d(TAG, "tearDown");
    }

    // Test if the debug mechanism for app transition timeout has been checked in
    public void testWindowTimeoutForAppTransition() throws Exception {
        Log.d(TAG, "testWindowTimeoutForAppTransition begin");
        new ProcessBuilder("/system/bin/am", "start", "-n", "com.android.launcher/com.android.launcher2.Launcher").start();
        Thread.sleep(BUFFER_TIME);
        mActivity.mStartTesting = true;
        new ProcessBuilder("/system/bin/am", "start", "-n", "com.mediatek.cts.window.stub/com.mediatek.cts.window.WindowTimeoutStubActivity").start();
        Thread.sleep(APP_TRANSITION_TIMEOUT + BUFFER_TIME);
        String[] pattern = { "*** APP TRANSITION TIMEOUT" };
        assertTrue(canParseLogPattern(pattern));
        mActivity.mStartTesting = false;
        Log.d(TAG, "testAppTransitionTimeout end");
    }

    // Test if the debug mechanism for app freeze timeout has been checked in
    public void testWindowTimeoutForAppFreeze() throws Exception {
        Log.d(TAG, "testAppFreezeTimeout begin");
        mActivity.mStartTesting = true;
        Thread.sleep(BUFFER_TIME);
        // Set the rotation degree and change the ACCELEROMETER_ROTATION settings to trigger rotation
        Settings.System.putInt(mResolver, Settings.System.USER_ROTATION, Surface.ROTATION_90);
        Settings.System.putInt(mResolver, Settings.System.ACCELEROMETER_ROTATION, 0);

        Thread.sleep(APP_FREEZE_TIMEOUT + BUFFER_TIME);
        String[] pattern = { "App freeze timeout expired."};
        assertTrue(canParseLogPattern(pattern));

        // Reset the rotation settings
        Settings.System.putInt(mResolver, Settings.System.USER_ROTATION, Surface.ROTATION_0);
        Settings.System.putInt(mResolver, Settings.System.ACCELEROMETER_ROTATION, 1);
        mActivity.mStartTesting = false;
        Log.d(TAG, "testAppFreezeTimeout end");
    }

    // Check if the solution patch for JB MR1 Google issue (Wrong window order) has been checked in
    // Abandoned test case because Google had fixed this bug
    public void testWindowOrderAdjustment() throws Exception {
        Log.d(TAG, "testWindowOrderAdjustment begin");
        /*synchronized(mLock) {
            new ProcessBuilder("/system/bin/input", "touchscreen", "tap", "265", "139").start();
            mH.sendMessageDelayed(mH.obtainMessage(H.DO_DELAY_NOTIFY), 2000);
            mLock.wait();
            new ProcessBuilder("/system/bin/input", "touchscreen", "tap", "265", "222").start();
            mH.sendMessageDelayed(mH.obtainMessage(H.DO_DELAY_NOTIFY), 2000);
            mLock.wait();
            assertTrue(checkWindowOrderInToken());
        }
        Thread.sleep(BUFFER_TIME);*/
        Log.d(TAG, "testWindowOrderAdjustment end");
    }

    public void testAppOrientationChangePerformance() throws Exception {
        Log.d(TAG, "testAppOrientationChangePerformance begin");
        /*PackageManager pm = mActivity.getPackageManager();
        final Intent mainIntent = new Intent(Intent.ACTION_MAIN, null);
        mainIntent.addCategory(Intent.CATEGORY_LAUNCHER);
        final List<ResolveInfo> pkgAppsList = pm.queryIntentActivities(mainIntent, 0);
        //Enable window orientation log
        new ProcessBuilder("/system/bin/dumpsys", "window", "-d", "enable", "0", "10").start();

        for (int i = 0 ; i < 2 ; i++) {
            InputStream is = null;
            OutputStream os = null;
            try {
                //Start launching the activities and rotate it, then stop it
                for (ResolveInfo ri : pkgAppsList) {
                    //Ignore those application which has prefer orientation
                    if (ri.activityInfo.screenOrientation != ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED) {
                        Log.d(TAG, "Ignore: " + ri.activityInfo.packageName + " orientation = " + ri.activityInfo.screenOrientation);
                        continue;
                    }
                    //Ingore those application not installed in system partition
                    ApplicationInfo ai = pm.getApplicationInfo(ri.activityInfo.packageName, 0);
                    if ((ai.flags & ApplicationInfo.FLAG_SYSTEM) == 0)
                        continue;

                    String target = ri.activityInfo.packageName + "/" + ri.activityInfo.name;
                    Log.d(TAG, "start " + target);
                    mH.sendMessageDelayed(mH.obtainMessage(H.ORIENTATION_TEST_TIMEOUT, ri.activityInfo.packageName), 30000);
                    new ProcessBuilder("/system/bin/am", "start", "-n", target).start();
                    Thread.sleep(5 * BUFFER_TIME);
                    beginCollection(i, ri);
                    toLandscape();
                    Thread.sleep(5 * BUFFER_TIME);
                    toPortrait();
                    Thread.sleep(5 * BUFFER_TIME);
                    endCollection(i, is, os, ri);
                    Thread.sleep(3 * BUFFER_TIME);
                    Log.d(TAG, "force-stop " + ri.activityInfo.packageName);
                    new ProcessBuilder("/system/bin/am", "force-stop", ri.activityInfo.packageName).start();
                    Thread.sleep(5 * BUFFER_TIME);
                    mH.removeMessages(H.ORIENTATION_TEST_TIMEOUT);
                }
            } catch (Exception e) {
                Log.e(TAG, "File I/O error", e);
            } finally {
                if (os != null)
                    os.close();
                if (is != null)
                    is.close();
            }
        }*/
        Log.d(TAG, "testAppOrientationChangePerformance end");
    }

    private void beginCollection(int idx, ResolveInfo target) throws Exception {
        if (idx ==0) {
            //Clear the log
            new ProcessBuilder("/system/bin/logcat", "-c").start();
        } else {
            File dir = mActivity.getDir("trace", Context.MODE_WORLD_WRITEABLE);
            File trace = new File(dir + "/" + target.activityInfo.name+".trace");
            Log.d(TAG, trace.getPath());
            Log.d(TAG, "Profile " + target.activityInfo.packageName);
            new ProcessBuilder("/system/bin/am", "profile", target.activityInfo.packageName, "start", trace.getPath()).start();
        }
    }

    private void endCollection(int idx, InputStream is, OutputStream os, ResolveInfo target) throws Exception {
        if (idx ==0) {
            //Collect the main log for rotation performance parsing and write to a file
            collectMainLog(is, os, target.activityInfo.name);
        } else {
            new ProcessBuilder("/system/bin/am", "profile", target.activityInfo.packageName, "stop").start();
        }
    }

    private void toLandscape() {
        Settings.System.putInt(mResolver, Settings.System.USER_ROTATION, Surface.ROTATION_90);
        Settings.System.putInt(mResolver, Settings.System.ACCELEROMETER_ROTATION, 0);
    }

    private void toPortrait() {
        Settings.System.putInt(mResolver, Settings.System.USER_ROTATION, Surface.ROTATION_0);
        Settings.System.putInt(mResolver, Settings.System.ACCELEROMETER_ROTATION, 1);
    }

    private void collectMainLog (InputStream is, OutputStream os, String target) throws Exception {
        int lines = 8192;
        java.lang.Process logcat = new ProcessBuilder("/system/bin/logcat", "-v", "threadtime", "-t", String.valueOf(lines), "-s", "WindowManager", "WindowStateAnimator").start();
        is = logcat.getInputStream();
        os = mActivity.openFileOutput("rot_perf_" + target, Context.MODE_WORLD_WRITEABLE);
        byte buf[] = new byte[1024];
        int length = 0;
        while ((length = is.read(buf)) > 0)
            os.write(buf, 0, length);
    }

    final class H extends Handler {
        public static final int ORIENTATION_TEST_TIMEOUT = 1;
        public H(){}

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ORIENTATION_TEST_TIMEOUT: {
                    String target = (String) msg.obj;
                    try {
                        Log.d(TAG, "Orientation test timeout on " + target + ", force stop ");
                        new ProcessBuilder("/system/bin/am", "force-stop", target).start();
                    } catch (Exception e) {
                        Log.e(TAG, "Error", e);
                    }
                }break;
            }
        }
    }

    //  The log pattern should be as following
    //  ************************************************************************************************************************************************
    //  Application tokens in Z order:
    //  App #11 AppWindowToken{44137878 token=Token{43ce1148 ActivityRecord{44136ba0 u0 com.mediatek.cts.window.stub/com.mediatek.cts.window.WindowTimeo
    //    windows=[Window{43ae8238 u0 ProgressDialog}, Window{439a8530 u0 com.mediatek.cts.window.stub/com.mediatek.cts.window.WindowTimeoutStubActivity}]
    //  ************************************************************************************************************************************************
    //  The implementation is to parse the "windows" content and identify if the order is right, WindowTimeoutStubActivity window should not be moved.
    private boolean checkWindowOrderInToken() {
        boolean isSuccessful = false;
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "window", "t").redirectErrorStream(true).start();
            InputStreamReader isr = new InputStreamReader(dumpsys.getInputStream());
            BufferedReader buf = new BufferedReader(isr);
            String s;
            Log.d(TAG, "start reading & parsing");
            while ((s = buf.readLine()) != null) {
                if (s.contains("Application tokens in Z order:")) {
                    Log.d(TAG, "Catch!! ");
                    Log.d(TAG, buf.readLine());
                    String target = buf.readLine();
                    String[] windowlist = target.split("=");
                    String[] tokens = windowlist[1].split(",");
                    Log.d(TAG, tokens[0]);
                    Log.d(TAG, tokens[1]);
                    if (tokens[0].contains("WindowTimeoutStubActivity"))
                        isSuccessful = true;
                }
            }
            Log.d(TAG, "finish parsing");
            isr.close();
        } catch (Exception e) {
        }
        return isSuccessful;
    }

    private boolean canParseLogPattern(String[] pattern) {
        boolean isSuccessful = false;
        try {
            int lines = 1024;
            java.lang.Process logcat = new ProcessBuilder("/system/bin/logcat", "-v", "time", "-t", String.valueOf(lines)).redirectErrorStream(true).start();
            InputStreamReader isr = new InputStreamReader(logcat.getInputStream());
            BufferedReader buf = new BufferedReader(isr);
            String s;
            String dumpString = "";
            int checkTimes = 0;
            Log.d(TAG, "start reading & parsing");
            boolean markEnterTimeStamp = false;
            while ((s = buf.readLine()) != null) {
                if (s.contains("Window") || !markEnterTimeStamp) {
                    dumpString += "   " + s + "\n";
                    markEnterTimeStamp = true;
                }
                if (s.contains(pattern[checkTimes])) {
                    Log.d(TAG, "Successfully parsing the "+ pattern[checkTimes] +" pattern!!");
                    Log.d(TAG, s);
                    checkTimes++;
                    if (checkTimes == pattern.length) {
                        isSuccessful = true;
                        break;
                    }
                }
            }
            if (!isSuccessful) {
                Log.d(TAG, "Parsing fail: print the captured log");
                Log.d(TAG, dumpString);
            }
            Log.d(TAG, "finish parsing");
            isr.close();
        } catch (Exception e) {
        }
        return isSuccessful;
    }
}
