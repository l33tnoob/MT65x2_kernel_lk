/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.test.emoticons;

import android.content.res.TypedArray;
import android.os.SystemClock;
import android.test.AndroidTestCase;
import android.text.SpannableStringBuilder;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.Scroller;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.emoticons.ScrollLayout;

import com.orangelabs.rcs.R;

import java.lang.reflect.Field;
import java.util.ArrayList;

/**
 * Defined to test the function of ScrollLayout
 */
public class ScrollLayoutTest extends AndroidTestCase {
    private static final String TAG = "ScrollLayoutTest";
    private ScrollLayout mLayout = null;
    private Scroller mScroller = null;
    private Integer mTouchState = null;
    private static final int CHILD_COUNT = 4;
    
    @Override
    protected void setUp() throws Exception, NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, InterruptedException {
        super.setUp();
        mLayout = new ScrollLayout(this.getContext(), null);
        assertNotNull(mLayout);
        mLayout.setMinimumWidth(400);
        mLayout.setMinimumHeight(200);
        for (int i = 0; i < CHILD_COUNT; i++) {
            ImageView image = new ImageView(this.getContext());
            image.setBackgroundResource(R.drawable.rcs_wallpaper_default);
            mLayout.addView(image);
        }
        Field fieldScroller = ScrollLayout.class.getDeclaredField("mScroller");
        fieldScroller.setAccessible(true);
        mScroller = (Scroller) fieldScroller.get(mLayout);
        assertNotNull(mScroller);
        
        Field touchState = ScrollLayout.class.getDeclaredField("mTouchState");
        touchState.setAccessible(true);
        mTouchState = (Integer) touchState.get(mLayout);
        assertNotNull(mTouchState);
    }

    /**
     * Test the computeScroll() method
     */
    public void testCase1_computeScroll() {
        Logger.d(TAG, "testCase1_computeScroll entry");
        mScroller.startScroll(0, 0, mLayout.getWidth() / 2, mLayout.getHeight() / 2);
        mLayout.computeScroll();
        assertEquals(mLayout.getWidth() / 2, mScroller.getCurrX());
        assertEquals(mLayout.getHeight() / 2, mScroller.getCurrY());
        Logger.d(TAG, "testCase1_computeScroll exit");
    }
    
    /**
     * Test the getCurScreen() method
     */
    public void testCase2_getCurScreen() {
        Logger.d(TAG, "testCase2_getCurScreen entry");
        assertEquals(0, mLayout.getCurScreen());
        Logger.d(TAG, "testCase2_getCurScreen exit");
    }
    
    /**
     * Test the setToScreen() method
     */
    public void testCase3_setToScreen() {
        Logger.d(TAG, "testCase3_setToScreen entry");
        int whichScreen = 1;
        mLayout.setToScreen(whichScreen);
        assertEquals(whichScreen, mLayout.getCurScreen());
        Logger.d(TAG, "testCase3_setToScreen exit");
    }

    /**
     * Test the onInterceptTouchEvent() method
     */
    public void testCase4_onInterceptTouchEvent() {
        Logger.d(TAG, "testCase4_onInterceptTouchEvent entry");
        
        // ACTION_MOVE
        mTouchState = Integer.valueOf(0);
        MotionEvent event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_MOVE, 20, 30, 0);
        assertTrue(mLayout.onInterceptTouchEvent(event));
        mTouchState = Integer.valueOf(1);
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_MOVE, 30, 30, 0);
        assertTrue(mLayout.onInterceptTouchEvent(event));
        
        mTouchState = Integer.valueOf(1);
        // ACTION_UP
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_UP, 40, 30, 0);
        assertFalse(mLayout.onInterceptTouchEvent(event));
        // ACTION_CANCEL
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_CANCEL, 50, 30, 0);
        assertFalse(mLayout.onInterceptTouchEvent(event));
        // ACTION_DOWN
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_DOWN, 60, 30, 0);
        assertFalse(mLayout.onInterceptTouchEvent(event));
        Logger.d(TAG, "testCase4_onInterceptTouchEvent exit");
    }

    /**
     * Test the onTouchEvent() method
     */
    public void testCase5_onTouchEvent() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase5_onTouchEvent entry");
        
        // ACTION_OUTSIDE
        Field fieldVelocityTracker = ScrollLayout.class.getDeclaredField("mVelocityTracker");
        fieldVelocityTracker.setAccessible(true);
        VelocityTracker tracker = null;
        fieldVelocityTracker.set(mLayout, tracker);
        MotionEvent event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_OUTSIDE, 20, 30, 0);
        assertTrue(mLayout.onTouchEvent(event));

        tracker = VelocityTracker.obtain();
        fieldVelocityTracker.set(mLayout, tracker);
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock.uptimeMillis(),
                MotionEvent.ACTION_OUTSIDE, 30, 30, 0);
        assertTrue(mLayout.onTouchEvent(event));

        // ACTION_CANCEL
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_CANCEL, 20, 30, 0);
        assertTrue(mLayout.onTouchEvent(event));
        assertEquals(Integer.valueOf(0),mTouchState);
        // ACTION_UP
        tracker = VelocityTracker.obtain();
        fieldVelocityTracker.set(mLayout, tracker);
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_CANCEL, 30, 30, 0);
        assertTrue(mLayout.onTouchEvent(event));
        assertEquals(Integer.valueOf(0),mTouchState);
        
        tracker = VelocityTracker.obtain();
        fieldVelocityTracker.set(mLayout, tracker);
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_CANCEL, 20, 20, 0);
        assertTrue(mLayout.onTouchEvent(event));
        assertEquals(Integer.valueOf(0),mTouchState);
        
        tracker = VelocityTracker.obtain();
        fieldVelocityTracker.set(mLayout, tracker);
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_CANCEL, 20, 20, 0);
        assertTrue(mLayout.onTouchEvent(event));
        assertEquals(Integer.valueOf(0),mTouchState);
        
        // ACTION_DOWN
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_DOWN, 40, 30, 0);
        assertTrue(mLayout.onTouchEvent(event));
        
        // ACTION_MOVE
        event = MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock
                .uptimeMillis(), MotionEvent.ACTION_MOVE, 40, 30, 0);
        assertTrue(mLayout.onTouchEvent(event));
        Logger.d(TAG, "testCase5_onTouchEvent exit");
    }
    
    /**
     * Test the snapToScreen() method
     */
    public void testCase6_snapToScreen() {
        Logger.d(TAG, "testCase6_snapToScreen entry");
        int whichScreen = 1;
        mLayout.setScrollX(5);
        mLayout.snapToScreen(whichScreen);
        assertEquals(whichScreen, mLayout.getCurScreen());
        Logger.d(TAG, "testCase6_snapToScreen exit");
    }
}
