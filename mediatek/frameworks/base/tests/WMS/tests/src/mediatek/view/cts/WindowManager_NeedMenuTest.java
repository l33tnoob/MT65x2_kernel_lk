package com.mediatek.cts.window;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.os.ServiceManager;
import android.test.ActivityInstrumentationTestCase2;
import android.test.AndroidTestCase;
import android.util.Slog;
import android.view.View;
import android.view.Window;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.PopupMenu;

import com.mediatek.cts.window.stub.R;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Stack;


public class WindowManager_NeedMenuTest extends ActivityInstrumentationTestCase2<WindowManagerNeedMenuActivity> {

    private Instrumentation mInstrumentation;
    private Context mContext;
    private WindowManagerNeedMenuActivity mActivity;
    private Window mWindow;

    public WindowManager_NeedMenuTest() {
        super(WindowManagerNeedMenuActivity.class);
    }

    public static class ActivityNeedMenu extends Activity {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // A hack to force NEED MENU KEY
            Window win = getWindow();
            win.getDecorView();
            win.addFlags(WindowManager.LayoutParams.FLAG_NEEDS_MENU_KEY);
        }
    }

    public static class ActivityNeedMenuAndNotFocusable extends Activity {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // A hack to force NEED MENU KEY
            Window win = getWindow();
            win.getDecorView();
            win.addFlags(
                WindowManager.LayoutParams.FLAG_NEEDS_MENU_KEY |
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
        }
    }

    public static class ActivitySmallNeedMenuAndNotFocusable extends Activity {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // A hack to force NEED MENU KEY
            Window win = getWindow();
            win.getDecorView();
            win.addFlags(
                WindowManager.LayoutParams.FLAG_NEEDS_MENU_KEY |
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
        }
    }

    public static class ActivitySmall extends Activity {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // A hack to force not NEED MENU KEY
            Window win = getWindow();
            win.getDecorView();
            WindowManager.LayoutParams attrs = win.getAttributes();
            attrs.flags &= ~WindowManager.LayoutParams.FLAG_NEEDS_MENU_KEY;
            attrs.privateFlags &= ~WindowManager.LayoutParams.PRIVATE_FLAG_SET_NEEDS_MENU_KEY;
        }
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mActivity = getActivity();
        mWindow = mActivity.getWindow();
    }

    private static class WindowManagerStateDumpper {
        private static final String DUMPSYS = "/system/bin/dumpsys";

        public boolean mLastFocusNeedsMenu;


        private void reset() {
            mLastFocusNeedsMenu = false;
        }

        public WindowManagerStateDumpper dump() throws IOException {

            reset();

            java.lang.Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-a").redirectErrorStream(true).start();
            InputStreamReader isr = new InputStreamReader(dumpsys.getInputStream());
            BufferedReader buf = new BufferedReader(isr);

            while (true) {
                String line = buf.readLine();
                if (line == null) {
                    break;
                }

                if (line.contains("mLastFocusNeedsMenu=true")) {
                    mLastFocusNeedsMenu = true;
                }
            }

            isr.close();

            return this;
        }
    }


    private Stack<Activity> mStartActivityList = new Stack<Activity>();

    private void startActivity(Class c) {
        final Context targetContext = mInstrumentation.getTargetContext();
        final Intent intent = new Intent(targetContext, c);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Activity a = mInstrumentation.startActivitySync(intent);

        mStartActivityList.push(a);
    }

    private void startActivityAndWait(Class c) {
        startActivity(c);
        mInstrumentation.waitForIdleSync();

        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
        }
    }

    private void finishAllActivities() {
        while (!mStartActivityList.isEmpty()) {
            Activity a = mStartActivityList.pop();
            a.finish();
        }
    }

    WindowManagerStateDumpper mDumpper = new WindowManagerStateDumpper();

    // Test FLAG_NEEDS_MENU_KEY
    public void testOptionMenuTarget1() {
        try {
            mInstrumentation.waitForIdleSync();
            assertFalse(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivityNeedMenu.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);
        } catch (IOException e) {
            assertTrue(false);
        } finally {
            finishAllActivities();
        }
    }

    // Test "Need Menu" + "Small"
    public void testOptionMenuTarget2() {
        try {
            startActivityAndWait(ActivityNeedMenu.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivitySmall.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);
        } catch (IOException e) {
            assertTrue(false);
        } finally {
            finishAllActivities();
        }
    }

    // Test "No Need Menu" + "Need Menu Not Focusable" + "Small"
    public void testOptionMenuTarget3() {
        try {
            startActivityAndWait(ActivityNeedMenuAndNotFocusable.class);
            assertFalse(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivitySmall.class);
            assertFalse(mDumpper.dump().mLastFocusNeedsMenu);
        } catch (IOException e) {
            assertTrue(false);
        } finally {
            finishAllActivities();
        }
    }

    // Test "Need Menu" + "Need Menu Not Focusable" + "Small"
    public void testOptionMenuTarget4() {
        try {
            startActivityAndWait(ActivityNeedMenu.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivityNeedMenuAndNotFocusable.class);
            //assertTrue(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivitySmall.class);
            assertFalse(mDumpper.dump().mLastFocusNeedsMenu);
        } catch (IOException e) {
            assertTrue(false);
        } finally {
            finishAllActivities();
        }
    }

    // Test "Need Menu" + "Small, Need Menu Not Focusable" + "Small"
    public void testOptionMenuTarget5() {
        try {
            startActivityAndWait(ActivityNeedMenu.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivitySmallNeedMenuAndNotFocusable.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);

            startActivityAndWait(ActivitySmall.class);
            assertTrue(mDumpper.dump().mLastFocusNeedsMenu);
        } catch (IOException e) {
            assertTrue(false);
        } finally {
            finishAllActivities();
        }
    }
}
