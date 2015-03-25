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

package com.mediatek.rcse.test.activity.widgets;

import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.test.ActivityInstrumentationTestCase2;
import android.util.DisplayMetrics;
import android.view.MotionEvent;

import com.mediatek.rcse.activities.SettingsActivity;
import com.mediatek.rcse.activities.widgets.TimeBar;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;

/**
 * This class is used to test TimeBar
 */
public class TimeBarTest extends ActivityInstrumentationTestCase2<SettingsActivity> {
    private static final String TAG = "TimeBarTest";
    private static final int MOCK_CURRENT_TIME = 5;
    private static final int MOCK_TOTAL_TIME = 10;
    private static final String MOCK_INFO_TEXT = "mock info text";
    private static final String MOCK_INFO_LONG_TEXT = "mock a long long long long long " +
    		"long long long long long info text";
    private static final int PROGRESS_BAR_LEFT = 0;
    private static final int PROGRESS_BAR_TOP = 10;
    private static final int PROGRESS_BAR_RIGHT = 5;
    private static final int PROGRESS_BAR_BOTTOM = 0;
    private SettingsActivity mActivity = null;
    private Context mContext = null;
    private TimeBar timeBar = null;
    private MockTimeBarListener mockTimeBarListener = null;

    public TimeBarTest() {
        super(SettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() enter");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        Intent intent = new Intent(mContext, SettingsActivity.class);
        setActivityIntent(intent);
        mActivity = getActivity();
        assertNotNull(mActivity);
        mockTimeBarListener = new MockTimeBarListener();
        timeBar = new TimeBar(mContext, mockTimeBarListener);
        Logger.v(TAG, "setUp() exit");
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() enter");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Test the function setTime()
     */
    public void testCase1_SetTime() throws Throwable {
        Logger.v(TAG, "testCase1_SetTime() enter");
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mActivity.setContentView(timeBar);
                timeBar.setTime(MOCK_CURRENT_TIME, MOCK_TOTAL_TIME);
            }
        });
        getInstrumentation().waitForIdleSync();
        Field fieldCurrentTime = Utils.getPrivateField(TimeBar.class, "mCurrentTime");
        int mCurrentTime = fieldCurrentTime.getInt(timeBar);
        Field fieldTotalTime = Utils.getPrivateField(TimeBar.class, "mTotalTime");
        int mTotalTime = fieldTotalTime.getInt(timeBar);
        Field fieldLastShowTime = Utils.getPrivateField(TimeBar.class, "mLastShowTime");
        int lastShowTime = fieldLastShowTime.getInt(timeBar);
        assertEquals(MOCK_CURRENT_TIME, mCurrentTime);
        assertEquals(MOCK_TOTAL_TIME, mTotalTime);
        int expectedLastShowTime = mTotalTime > mCurrentTime ? mTotalTime : mCurrentTime;
        assertEquals(expectedLastShowTime, lastShowTime);
        
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                timeBar.setTime(MOCK_CURRENT_TIME, 3600*1000);
            }
        });
        getInstrumentation().waitForIdleSync();
        Logger.v(TAG, "testCase1_SetTime() exit");
    }

    /**
     * Test the function resetTime()
     */
    public void testCase2_ResetTime() throws Throwable {
        Logger.v(TAG, "testCase2_ResetTime() enter");
        testCase1_SetTime();
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                timeBar.resetTime();
            }
        });
        getInstrumentation().waitForIdleSync();
        Field fieldCurrentTime = Utils.getPrivateField(TimeBar.class, "mCurrentTime");
        int mCurrentTime = fieldCurrentTime.getInt(timeBar);
        Field fieldTotalTime = Utils.getPrivateField(TimeBar.class, "mTotalTime");
        int mTotalTime = fieldTotalTime.getInt(timeBar);
        Field fieldLastShowTime = Utils.getPrivateField(TimeBar.class, "mLastShowTime");
        int lastShowTime = fieldLastShowTime.getInt(timeBar);
        assertEquals(0, mCurrentTime);
        assertEquals(0, mTotalTime);
        assertEquals(0, lastShowTime);
        Logger.v(TAG, "testCase2_ResetTime() exit");
    }

    /**
     * Test the view's visiable text
     */
    public void testCase3_VisibleText() throws Throwable {
        Logger.v(TAG, "testCase3_VisibleText() enter");
        // Test when mInfoText is null, mVisiableText is null
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mActivity.setContentView(timeBar);
                timeBar.resetTime();
                timeBar.setInfo(null);
                timeBar.setTime(MOCK_CURRENT_TIME, MOCK_TOTAL_TIME);
            }
        });
        getInstrumentation().waitForIdleSync();
        Field fieldVisibleText = Utils.getPrivateField(TimeBar.class, "mVisibleText");
        assertNull(fieldVisibleText.get(timeBar));

        // Test when mInfoText is not null
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                timeBar.resetTime();
                timeBar.setInfo(MOCK_INFO_TEXT);
                timeBar.setTime(MOCK_CURRENT_TIME, MOCK_TOTAL_TIME);
            }
        });
        getInstrumentation().waitForIdleSync();
        Field fieldVisibleTextShort = Utils.getPrivateField(TimeBar.class, "mVisibleText");
        String visibleTextShortString = (String) fieldVisibleTextShort.get(timeBar);
        assertNotNull(visibleTextShortString);
        assertTrue(visibleTextShortString.equals(MOCK_INFO_TEXT));
        
        // Test when mInfoText is long text
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                timeBar.resetTime();
                timeBar.setInfo(MOCK_INFO_LONG_TEXT);
                timeBar.setBufferPercent(-1);
                timeBar.setBufferPercent(10);
                timeBar.setTime(MOCK_CURRENT_TIME, MOCK_TOTAL_TIME);
            }
        });
        getInstrumentation().waitForIdleSync();
        
        
        Logger.v(TAG, "testCase3_VisibleText() exit");
    }

    /**
     * Test the function onTouchEvent, mock a lisener to get callback result
     */
    public void testCase4_OnTouchEvent() throws Throwable {
        Logger.v(TAG, "testCase4_OnTouchEvent() enter");
        Field fieldProgressBar = Utils.getPrivateField(TimeBar.class, "mProgressBar");
        Rect progressBar = (Rect) fieldProgressBar.get(timeBar);
        progressBar.set(PROGRESS_BAR_LEFT, PROGRESS_BAR_TOP, PROGRESS_BAR_RIGHT,
                PROGRESS_BAR_BOTTOM);
        getInstrumentation().waitForIdleSync();
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mActivity.setContentView(timeBar);
                timeBar.setShowScrubber(true);
                timeBar.setShowTimes(true);
                timeBar.setScrubbing(true);
                timeBar.setTime(MOCK_CURRENT_TIME, MOCK_TOTAL_TIME);
                
                MotionEvent eventDown = MotionEvent.obtain(0, 0, MotionEvent.ACTION_DOWN, 0, 0, 0);
                timeBar.onTouchEvent(eventDown);
                timeBar.setShowScrubber(false);

                timeBar.setShowScrubber(true);
                mockTimeBarListener.isOnScrubbingStartCalled = false;
                timeBar.onTouchEvent(eventDown);
                assertTrue(mockTimeBarListener.isOnScrubbingStartCalled);

                MotionEvent eventMove = MotionEvent.obtain(0, 0, MotionEvent.ACTION_MOVE, 0, 0, 0);
                mockTimeBarListener.isOnScrubbingMoveCalled = false;
                timeBar.onTouchEvent(eventMove);
                assertTrue(mockTimeBarListener.isOnScrubbingMoveCalled);

                MotionEvent eventUp = MotionEvent.obtain(0, 0, MotionEvent.ACTION_UP, 0, 0, 0);
                mockTimeBarListener.isOnScrubbingEndCalled = false;
                timeBar.onTouchEvent(eventUp);
                assertTrue(mockTimeBarListener.isOnScrubbingEndCalled);
            }
        });
        getInstrumentation().waitForIdleSync();
        Logger.v(TAG, "testCase4_OnTouchEvent() exit");
    }
    
    public void testCase5_testGetHeight() throws Throwable {
        Logger.v(TAG, "testCase5_testGetHeight() enter");
        DisplayMetrics metrics = mActivity.getResources().getDisplayMetrics();
        int paddingInPx = (int) (metrics.density * 30);
        int scrubberPadding = (int) (metrics.density * 10);
        int textPadding = scrubberPadding / 2;
        
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mActivity.setContentView(timeBar);
                timeBar.setInfo(MOCK_INFO_TEXT);
                timeBar.setTime(MOCK_CURRENT_TIME, MOCK_TOTAL_TIME);
            }
        });
        getInstrumentation().waitForIdleSync();
        
        assertTrue(timeBar.getPreferredHeight() >=(paddingInPx+scrubberPadding+textPadding));
        assertTrue(timeBar.getBarHeight() >=(paddingInPx+textPadding));
        
        Logger.v(TAG, "testCase5_testGetHeight() exit");
    }

    private class MockTimeBarListener implements TimeBar.Listener {
        public boolean isOnScrubbingStartCalled = false;
        public boolean isOnScrubbingMoveCalled = false;
        public boolean isOnScrubbingEndCalled = false;

        @Override
        public void onScrubbingStart() {
            isOnScrubbingStartCalled = true;
        }

        @Override
        public void onScrubbingMove(int time) {
            isOnScrubbingMoveCalled = true;
        }

        @Override
        public void onScrubbingEnd(int time) {
            isOnScrubbingEndCalled = true;
        }
    }
}
