/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package mediatek.app.cts;

import java.util.List;

import android.app.Activity;
import android.app.Application;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.app.Instrumentation.ActivityResult;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Point;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Debug;
import android.os.IBinder;
import android.os.SystemClock;
import android.test.InstrumentationTestCase;
import android.view.InputQueue;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.ViewGroup.LayoutParams;

import com.mediatek.cts.activity.stub.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;

public class InstrumentationTest extends InstrumentationTestCase {

    private static final int WAIT_TIME = 1000;
    private Instrumentation mInstrumentation;
    private InstrumentationTestActivity mActivity;
    private Intent mIntent;
    private boolean mRunOnMainSyncResult;
    private Context mContext;
    private MotionEvent mMotionEvent;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        final long downTime = SystemClock.uptimeMillis();
        final long eventTime = SystemClock.uptimeMillis();
        // use coordinates for MotionEvent that do not include the status bar
        // TODO: is there a more deterministic way to get these values
        final long x = 55;
        final long y = 55;
        final int metaState = 0;
        mMotionEvent = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, x, y,
                metaState);
        mIntent = new Intent(mContext, InstrumentationTestActivity.class);
        mIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivity = (InstrumentationTestActivity) mInstrumentation.startActivitySync(mIntent);
    }

    protected void tearDown() throws Exception {
        mInstrumentation = null;
        mIntent = null;
        if (mActivity != null) {
            mActivity.finish();
            mActivity = null;
        }
        super.tearDown();
    }

    public void testConstructor() throws Exception {
        new Instrumentation();
    }

    public void testMonitor() throws Exception {
        if (mActivity != null)
            mActivity.finish();
        ActivityResult result = new ActivityResult(Activity.RESULT_OK, new Intent());
        ActivityMonitor monitor = new ActivityMonitor(
                InstrumentationTestActivity.class.getName(), result, false);
        mInstrumentation.addMonitor(monitor);
        Intent intent = new Intent(mContext, InstrumentationTestActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
        Activity activity = mInstrumentation.waitForMonitorWithTimeout(monitor, WAIT_TIME);
        assertTrue(activity instanceof InstrumentationTestActivity);
        assertTrue(mInstrumentation.checkMonitorHit(monitor, 1));
        activity.finish();

        mInstrumentation.addMonitor(monitor);
        mInstrumentation.removeMonitor(monitor);
        Activity a = mInstrumentation.startActivitySync(intent);
        assertTrue(a instanceof InstrumentationTestActivity);
        activity = mInstrumentation.waitForMonitorWithTimeout(monitor, WAIT_TIME);
        assertNull(activity);
        a.finish();

        IntentFilter filter = new IntentFilter();
        ActivityMonitor am = mInstrumentation.addMonitor(filter, result, false);
        mContext.startActivity(intent);
        mInstrumentation.waitForIdleSync();
        activity = am.waitForActivity();
        assertTrue(activity instanceof InstrumentationTestActivity);
        activity.finish();
        mInstrumentation.removeMonitor(am);
        am = mInstrumentation
                .addMonitor(InstrumentationTestActivity.class.getName(), result, false);
        mContext.startActivity(intent);
        activity = am.waitForActivity();
        assertTrue(activity instanceof InstrumentationTestActivity);
        activity.finish();
        mInstrumentation.removeMonitor(am);
    }

    public void testCallActivityOnCreate() throws Throwable {
        mActivity.setOnCreateCalled(false);
        runTestOnUiThread(new Runnable() {
            public void run() {
                mInstrumentation.callActivityOnCreate(mActivity, new Bundle());
            }
        });
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnCreateCalled());
    }

    public void testAllocCounting() throws Exception {
        mInstrumentation.startAllocCounting();

        Bundle b = mInstrumentation.getAllocCounts();
        assertTrue(b.size() > 0);
        b = mInstrumentation.getBinderCounts();
        assertTrue(b.size() > 0);

        int globeAllocCount = Debug.getGlobalAllocCount();
        int globeAllocSize = Debug.getGlobalAllocSize();
        int globeExternalAllCount = Debug.getGlobalExternalAllocCount();
        int globeExternalAllSize = Debug.getGlobalExternalAllocSize();
        int threadAllocCount = Debug.getThreadAllocCount();

        assertTrue(Debug.getGlobalAllocCount() >= globeAllocCount);
        assertTrue(Debug.getGlobalAllocSize() >= globeAllocSize);
        assertTrue(Debug.getGlobalExternalAllocCount() >= globeExternalAllCount);
        assertTrue(Debug.getGlobalExternalAllocSize() >= globeExternalAllSize);
        assertTrue(Debug.getThreadAllocCount() >= threadAllocCount);

        mInstrumentation.stopAllocCounting();

        globeAllocCount = Debug.getGlobalAllocCount();
        globeAllocSize = Debug.getGlobalAllocSize();
        globeExternalAllCount = Debug.getGlobalExternalAllocCount();
        globeExternalAllSize = Debug.getGlobalExternalAllocSize();
        threadAllocCount = Debug.getThreadAllocCount();
        assertEquals(globeAllocCount, Debug.getGlobalAllocCount());
        assertEquals(globeAllocSize, Debug.getGlobalAllocSize());
        assertEquals(globeExternalAllCount, Debug.getGlobalExternalAllocCount());
        assertEquals(globeExternalAllSize, Debug.getGlobalExternalAllocSize());
        assertEquals(threadAllocCount, Debug.getThreadAllocCount());
    }

    public void testSendTrackballEventSync() throws Exception {
        mInstrumentation.sendTrackballEventSync(mMotionEvent);
        mInstrumentation.waitForIdleSync();

        MotionEvent motionEvent = mActivity.getMotionEvent();
        assertEquals(mMotionEvent.getMetaState(), motionEvent.getMetaState());
        assertEquals(mMotionEvent.getEventTime(), motionEvent.getEventTime());
        assertEquals(mMotionEvent.getDownTime(), motionEvent.getDownTime());
    }

    public void testCallApplicationOnCreate() throws Exception {
        InstrumentationTestStub ca = new InstrumentationTestStub();
        mInstrumentation.callApplicationOnCreate(ca);
        assertTrue(ca.mIsOnCreateCalled);
    }

    public void testContext() throws Exception {
        Context c1 = mInstrumentation.getContext();
        Context c2 = mInstrumentation.getTargetContext();
        assertNotSame(c1.getPackageName(), c2.getPackageName());
    }

    public void testInvokeMenuActionSync() throws Exception {
        final int resId = R.id.goto_menu_id;
        mInstrumentation.invokeMenuActionSync(mActivity, resId, 0);
        mInstrumentation.waitForIdleSync();

        assertEquals(resId, mActivity.getMenuID());
    }

    public void testCallActivityOnPostCreate() throws Throwable {
        mActivity.setOnPostCreate(false);
        runTestOnUiThread(new Runnable() {
            public void run() {
                mInstrumentation.callActivityOnPostCreate(mActivity, new Bundle());
            }
        });
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnPostCreate());
    }

    public void testCallActivityOnNewIntent() throws Throwable {
        mActivity.setOnNewIntentCalled(false);
        runTestOnUiThread(new Runnable() {
            public void run() {
                mInstrumentation.callActivityOnNewIntent(mActivity, null);
            }
        });
        mInstrumentation.waitForIdleSync();

        assertTrue(mActivity.isOnNewIntentCalled());
    }

    public void testCallActivityOnResume() throws Throwable {
        mActivity.setOnResume(false);
        runTestOnUiThread(new Runnable() {
            public void run() {
                mInstrumentation.callActivityOnResume(mActivity);
            }
        });
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnResume());
    }

    public void testMisc() throws Exception {
    }

    public void testPerformanceSnapshot() throws Exception {
        mInstrumentation.setAutomaticPerformanceSnapshots();
        mInstrumentation.startPerformanceSnapshot();
        mInstrumentation.endPerformanceSnapshot();
    }

    public void testProfiling() throws Exception {
        // by default, profiling was disabled. but after set the handleProfiling attribute in the
        // manifest file for this Instrumentation to true, the profiling was also disabled.
        assertFalse(mInstrumentation.isProfiling());

        mInstrumentation.startProfiling();
        mInstrumentation.stopProfiling();
    }

    public void testInvokeContextMenuAction() throws Exception {
        MockActivity activity = new MockActivity();
        final int id = 1;
        final int flag = 2;
        mInstrumentation.invokeContextMenuAction(activity, id, flag);
        mInstrumentation.waitForIdleSync();

        assertEquals(id, activity.mWindow.mId);
        assertEquals(flag, activity.mWindow.mFlags);
    }

    public void testSendStringSync() {
        final String text = "abcd";
        mInstrumentation.sendStringSync(text);
        mInstrumentation.waitForIdleSync();

        List<KeyEvent> keyUpList = mActivity.getKeyUpList();
        List<KeyEvent> keyDownList = mActivity.getKeyDownList();
        assertEquals(text.length(), keyDownList.size());
        assertEquals(text.length(), keyUpList.size());

        KeyCharacterMap kcm = KeyCharacterMap.load(KeyCharacterMap.BUILT_IN_KEYBOARD);
        KeyEvent[] keyEvents = kcm.getEvents(text.toCharArray());

        int i = 0;
        for (int j = 0; j < keyDownList.size(); j++) {
            assertEquals(keyEvents[i++].getKeyCode(), keyDownList.get(j).getKeyCode());
            assertEquals(keyEvents[i++].getKeyCode(), keyUpList.get(j).getKeyCode());
        }
    }

    //BEGIN mtk03923 [20120615][Framework Auto Regression Test fails on AMS]
    public void testCallActivityOnSaveInstanceState() throws Throwable {
        final Bundle bundle = new Bundle();
        mActivity.setOnSaveInstanceState(false);
        runTestOnUiThread(new Runnable() {
            public void run() {
                mInstrumentation.callActivityOnSaveInstanceState(mActivity, bundle);
            }
        });
        mInstrumentation.waitForIdleSync();

        assertTrue(mActivity.isOnSaveInstanceState());
        assertSame(bundle, mActivity.getBundle());
    }
    //END   mtk03923 [20120615][Framework Auto Regression Test fails on AMS]

    //BEGIN mtk03923 [20120615][Framework Auto Regression Test fails on AMS]
    public void testSendPointerSync() throws Exception {
        mInstrumentation.waitForIdleSync();
        mInstrumentation.setInTouchMode(true);
        mInstrumentation.waitForIdleSync();
        
        // Send a touch event to the middle of the activity.
        // We assume that the Activity is empty so there won't be anything in the middle
        // to handle the touch.  Consequently the Activity should receive onTouchEvent
        // because nothing else handled it.
        Point size = new Point();
        mActivity.getWindowManager().getDefaultDisplay().getSize(size);
        final int x = size.x / 2;
        final int y = size.y / 2;
        long now = SystemClock.uptimeMillis();
        MotionEvent orig = MotionEvent.obtain(now, now, MotionEvent.ACTION_DOWN,
                x, y, 0);
        mInstrumentation.sendPointerSync(orig);
        //mInstrumentation.sendPointerSync(mMotionEvent);
        
        assertTrue(mActivity.isOnTouchEventCalled());
        mActivity.setOnTouchEventCalled(false);
    }
    //END   mtk03923 [20120615][Framework Auto Regression Test fails on AMS]

    public void testGetComponentName() throws Exception {
        ComponentName com = getInstrumentation().getComponentName();
        assertNotNull(com.getPackageName());
        assertNotNull(com.getClassName());
        assertNotNull(com.getShortClassName());
    }

    public void testNewApplication() throws Exception {
        final String className = "mediatek.app.cts.MockApplication";
        ClassLoader cl = getClass().getClassLoader();

        Application app = mInstrumentation.newApplication(cl, className, mContext);
        assertEquals(className, app.getClass().getName());

        app = Instrumentation.newApplication(MockApplication.class, mContext);
        assertEquals(className, app.getClass().getName());
    }

    public void testRunOnMainSync() throws Exception {
        mRunOnMainSyncResult = false;
        mInstrumentation.runOnMainSync(new Runnable() {
            public void run() {
                mRunOnMainSyncResult = true;
            }
        });
        mInstrumentation.waitForIdleSync();
        assertTrue(mRunOnMainSyncResult);
    }

    public void testCallActivityOnPause() throws Exception {
        mActivity.setOnPauseCalled(false);
        mInstrumentation.callActivityOnPause(mActivity);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnPauseCalled());
    }

    //BEGIN mtk03923 [20120615][Framework Auto Regression Test fails on AMS]
    /*
    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "callActivityOnDestroy",
        args = {Activity.class}
    )
    public void testCallActivityOnDestroy() throws Exception {
        mActivity.setOnDestroyCalled(false);
        mInstrumentation.callActivityOnDestroy(mActivity);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnDestroyCalled());
    }
    */
    //END   mtk03923 [20120615][Framework Auto Regression Test fails on AMS]

    public void testSendKeyDownUpSync() throws Exception {
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_0);
        mInstrumentation.waitForIdleSync();
        assertEquals(1, mActivity.getKeyUpList().size());
        assertEquals(1, mActivity.getKeyDownList().size());
        assertEquals(KeyEvent.KEYCODE_0, mActivity.getKeyUpList().get(0).getKeyCode());
        assertEquals(KeyEvent.KEYCODE_0, mActivity.getKeyDownList().get(0).getKeyCode());
    }

    public void testNewActivity() throws Exception {
        Intent intent = new Intent();
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        ClassLoader cl = getClass().getClassLoader();
        Activity activity = mInstrumentation.newActivity(cl, InstrumentationTestActivity.class
                .getName(), intent);
        assertEquals(InstrumentationTestActivity.class.getName(), activity.getClass().getName());
        activity.finish();
        activity = null;

        intent = new Intent(mContext, InstrumentationTestActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        Activity father = new Activity();
        ActivityInfo info = new ActivityInfo();

        activity = mInstrumentation
                .newActivity(InstrumentationTestActivity.class, mContext, null, null, intent, info,
                        InstrumentationTestActivity.class.getName(), father, null, null);

        assertEquals(father, activity.getParent());
        assertEquals(InstrumentationTestActivity.class.getName(), activity.getClass().getName());
        activity.finish();
    }

    public void testCallActivityOnStart() throws Exception {
        mActivity.setOnStart(false);
        mInstrumentation.callActivityOnStart(mActivity);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnStart());
    }

    public void testWaitForIdle() throws Exception {
        MockRunnable mr = new MockRunnable();
        assertFalse(mr.isRunCalled());
        mInstrumentation.waitForIdle(mr);
        Thread.sleep(WAIT_TIME);
        assertTrue(mr.isRunCalled());
    }

    public void testSendCharacterSync() throws Exception {
        mInstrumentation.sendCharacterSync(KeyEvent.KEYCODE_0);
        mInstrumentation.waitForIdleSync();
        assertEquals(KeyEvent.KEYCODE_0, mActivity.getKeyDownCode());
        assertEquals(KeyEvent.KEYCODE_0, mActivity.getKeyUpCode());
    }

    public void testCallActivityOnRestart() throws Exception {
        mActivity.setOnRestart(false);
        mInstrumentation.callActivityOnRestart(mActivity);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnRestart());
    }

    public void testCallActivityOnStop() throws Exception {
        mActivity.setOnStop(false);
        mInstrumentation.callActivityOnStop(mActivity);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnStop());
    }

    public void testCallActivityOnUserLeaving() throws Exception {
        assertFalse(mActivity.isOnLeave());
        mInstrumentation.callActivityOnUserLeaving(mActivity);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnLeave());
    }

    public void testCallActivityOnRestoreInstanceState() throws Exception {
        mActivity.setOnRestoreInstanceState(false);
        mInstrumentation.callActivityOnRestoreInstanceState(mActivity, new Bundle());
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnRestoreInstanceState());
    }

    public void testSendKeySync() throws Exception {
        KeyEvent key = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_0);
        mInstrumentation.sendKeySync(key);
        mInstrumentation.waitForIdleSync();
        assertEquals(KeyEvent.KEYCODE_0, mActivity.getKeyDownCode());
    }

    private static class MockRunnable implements Runnable {
        private boolean mIsRunCalled ;

        public void run() {
            mIsRunCalled = true;
        }

        public boolean isRunCalled() {
            return mIsRunCalled;
        }
    }

    private class MockActivity extends Activity {
        MockWindow mWindow = new MockWindow(mContext);

        @Override
        public Window getWindow() {
            return mWindow;
        }

        private class MockWindow extends Window {

            public int mId;
            public int mFlags;

            public MockWindow(Context context) {
                super(context);
            }

            @Override
            public void addContentView(View view, LayoutParams params) {
            }

            @Override
            public void closeAllPanels() {
            }

            @Override
            public void closePanel(int featureId) {
            }

            @Override
            public View getCurrentFocus() {
                return null;
            }

            @Override
            public View getDecorView() {
                return null;
            }

            @Override
            public LayoutInflater getLayoutInflater() {
                return null;
            }

            @Override
            public int getVolumeControlStream() {
                return 0;
            }

            @Override
            public boolean isFloating() {
                return false;
            }

            @Override
            public boolean isShortcutKey(int keyCode, KeyEvent event) {
                return false;
            }

            @Override
            protected void onActive() {
            }

            @Override
            public void onConfigurationChanged(Configuration newConfig) {
            }

            @Override
            public void openPanel(int featureId, KeyEvent event) {
            }

            @Override
            public void alwaysReadCloseOnTouchAttr() {
            }

            @Override
            public View peekDecorView() {
                return null;
            }

            @Override
            public boolean performContextMenuIdentifierAction(int id, int flags) {
                mId = id;
                mFlags = flags;
                return false;
            }

            @Override
            public boolean performPanelIdentifierAction(int featureId, int id, int flags) {
                return false;
            }

            @Override
            public boolean performPanelShortcut(int featureId, int keyCode,
                    KeyEvent event, int flags) {
                return false;
            }

            @Override
            public void restoreHierarchyState(Bundle savedInstanceState) {
            }

            @Override
            public Bundle saveHierarchyState() {
                return null;
            }

            @Override
            public void setBackgroundDrawable(Drawable drawable) {
            }

            @Override
            public void setChildDrawable(int featureId, Drawable drawable) {
            }

            @Override
            public void setChildInt(int featureId, int value) {
            }

            @Override
            public void setContentView(int layoutResID) {
            }

            @Override
            public void setContentView(View view) {
            }

            @Override
            public void setContentView(View view, LayoutParams params) {
            }

            @Override
            public void setFeatureDrawable(int featureId, Drawable drawable) {
            }

            @Override
            public void setFeatureDrawableAlpha(int featureId, int alpha) {
            }

            @Override
            public void setFeatureDrawableResource(int featureId, int resId) {
            }

            @Override
            public void setFeatureDrawableUri(int featureId, Uri uri) {
            }

            @Override
            public void setFeatureInt(int featureId, int value) {
            }

            @Override
            public void setTitle(CharSequence title) {
            }

            @Override
            public void setTitleColor(int textColor) {
            }

            @Override
            public void setVolumeControlStream(int streamType) {
            }

            @Override
            public boolean superDispatchKeyEvent(KeyEvent event) {
                return false;
            }

            @Override
            public boolean superDispatchKeyShortcutEvent(KeyEvent event) {
                return false;
            }

            @Override
            public boolean superDispatchTouchEvent(MotionEvent event) {
                return false;
            }

            @Override
            public boolean superDispatchTrackballEvent(MotionEvent event) {
                return false;
            }

            @Override
            public boolean superDispatchGenericMotionEvent(MotionEvent event) {
                return false;
            }

            @Override
            public void takeKeyEvents(boolean get) {
            }

            @Override
            public void togglePanel(int featureId, KeyEvent event) {
            }

            @Override
            public void invalidatePanelMenu(int featureId) {
            }

            @Override
            public void takeSurface(SurfaceHolder.Callback2 callback) {
            }

            @Override
            public void takeInputQueue(InputQueue.Callback queue) {
            }
        }
    }

    private static class InstrumentationTestStub extends Application {
        boolean mIsOnCreateCalled = false;

        @Override
        public void onCreate() {
            super.onCreate();
            mIsOnCreateCalled = true;
        }
    }
}
