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

package com.mediatek.common.widget.tests.listview.touch;

import android.test.ActivityInstrumentationTestCase;
import android.test.suitebuilder.annotation.LargeTest;
import android.test.suitebuilder.annotation.MediumTest;
import android.view.Gravity;
import android.view.View;
import android.view.ViewConfiguration;
import android.widget.ListView;

import com.mediatek.common.widget.tests.listview.ListOfTouchables;
import android.test.TouchUtils;


/// M: [ALPS00428138] get context for ViewConfiguration
import android.content.Context;

/**
 * Touch tests for a list where all of the items fit on the screen.
 */
public class ListOfTouchablesTest extends ActivityInstrumentationTestCase<ListOfTouchables> {
    private ListOfTouchables mActivity;
    private ListView mListView;

    /// M: [ALPS00428138] get context for ViewConfiguration
    private ViewConfiguration mViewConfig;

    public ListOfTouchablesTest() {
        super("com.mediatek.common.widget.tests", ListOfTouchables.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mActivity = getActivity();
        mListView = getActivity().getListView();
        
        /// M: [ALPS00428138] get context for ViewConfiguration
        final Context context = mActivity.getApplicationContext();
        mViewConfig = ViewConfiguration.get(context);
      
    }

    @MediumTest
    public void testPreconditions() {
        assertNotNull(mActivity);
        assertNotNull(mListView);
    }
    
    // TODO: needs to be adjusted to pass on non-HVGA displays
    // @LargeTest
    public void testShortScroll() {
        View firstChild = mListView.getChildAt(0);
        View lastChild = mListView.getChildAt(mListView.getChildCount() - 1);
        
        int firstTop = firstChild.getTop();
        
        /// M: [ALPS00428138] fix failure cases @{
        //TouchUtils.dragViewBy(this, lastChild, Gravity.TOP | Gravity.LEFT,
        //        0, -(ViewConfiguration.getTouchSlop() + 1 + 10));
        TouchUtils.dragViewBy(this, lastChild, Gravity.TOP | Gravity.LEFT,
                  0, -(mViewConfig.getScaledTouchSlop() + 10));
        
        /// @}
        
        
        View newFirstChild = mListView.getChildAt(0);
        
        assertEquals("View scrolled too early", firstTop, newFirstChild.getTop() + 10);
        assertEquals("Wrong view in first position", 0, newFirstChild.getId());
    }
    
    // TODO: needs to be adjusted to pass on non-HVGA displays
    // @LargeTest
    public void testLongScroll() {
        View lastChild = mListView.getChildAt(mListView.getChildCount() - 1);
        
        int lastTop = lastChild.getTop();
        
        int distance = TouchUtils.dragViewToY(this, lastChild, 
                Gravity.TOP | Gravity.LEFT, mListView.getTop());
        
        /// M: [ALPS00428138] fix failure cases @{
        //assertEquals("View scrolled to wrong position", 
        //        lastTop - (distance - ViewConfiguration.getTouchSlop() - 1), lastChild.getTop());
        assertEquals("View scrolled to wrong position", 
                  lastTop - (distance - mViewConfig.getScaledTouchSlop()), lastChild.getTop());
        
        /// @}
    } 

}
