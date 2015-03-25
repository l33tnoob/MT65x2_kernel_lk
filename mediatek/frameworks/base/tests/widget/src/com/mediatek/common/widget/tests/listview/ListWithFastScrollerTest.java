/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.common.widget.tests.listview;

import android.test.ActivityInstrumentationTestCase;
import android.test.suitebuilder.annotation.MediumTest;
import android.view.Gravity;
import android.widget.ListView;
import android.widget.ListAdapter;
import android.widget.ArrayAdapter;

import android.os.SystemClock;
import android.app.Instrumentation;
import android.view.MotionEvent;

import android.test.TouchUtils;
import android.view.View;

public class ListWithFastScrollerTest extends ActivityInstrumentationTestCase<ListWithFastScroller> {
    private ListWithFastScroller mActivity;
    private ListView mListView;

    public ListWithFastScrollerTest() {
        super("com.mediatek.common.widget.tests", ListWithFastScroller.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mActivity = getActivity();
        mListView = getActivity().getListView();
    }

    @MediumTest
    public void testTouchFastScroller() {
           
        TouchUtils.drag(this, mListView.getWidth() - 10, mListView.getWidth() - 10,
                100, 100 + 300, 300);
        
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        } 
        
        assertTrue("FastScroller should be enabled", mListView.isFastScrollEnabled());
    }
    
    @MediumTest
    public void testTouchOnWindowFocusChanged() {
        
        Instrumentation inst = getInstrumentation();

        long downTime = SystemClock.uptimeMillis();
        long eventTime = SystemClock.uptimeMillis();

        float y = 100;
        float x = mListView.getWidth() - 10;
        

        MotionEvent event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
        
        
        for (int i = 0; i < 2; ++i) {
            y += 5;
            eventTime = SystemClock.uptimeMillis();
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, x, y, 0);
            inst.sendPointerSync(event);
            inst.waitForIdleSync();
        }
            
        mActivity.finish();
        mActivity = null;
        
        assertTrue(mActivity == null);
    }

    
}
