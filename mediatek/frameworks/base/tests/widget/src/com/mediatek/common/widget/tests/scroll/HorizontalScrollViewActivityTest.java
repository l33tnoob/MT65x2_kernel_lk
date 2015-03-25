package com.mediatek.common.widget.tests.scroll;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.widget.HorizontalScrollView;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerProperties;
import android.view.MotionEvent.PointerCoords;
import android.view.View;
import android.view.Gravity;

import android.os.SystemClock;
import android.app.Instrumentation;

import com.mediatek.common.widget.tests.scroll.HorizontalScrollViewActivity;

public class HorizontalScrollViewActivityTest extends
        ActivityInstrumentationTestCase2<HorizontalScrollViewActivity> {

    HorizontalScrollViewActivity mActivity;
    HorizontalScrollView mHorizontalScrollView;

    public HorizontalScrollViewActivityTest() {
        super("com.mediatek.common.widget.tests",
                HorizontalScrollViewActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mActivity = getActivity();
        mHorizontalScrollView = mActivity.getHorizontalScrollView();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();

    }

    public void testPreconditions() {
        assertTrue(mActivity != null);
        assertTrue(mHorizontalScrollView != null);
    }

    public void testScroll() {

        /*
         * int halfWidth = (int)(mHorizontalScrollView.getWidth() / 2.0); int
         * halfHeight = (int)(mHorizontalScrollView.getHeight() / 2.0);
         * 
         * int[] location = new int[2];
         * mHorizontalScrollView.getLocationOnScreen(location);
         * 
         * TouchUtils.drag(this, location[0] + halfWidth + 50, location[0] +
         * halfWidth - 50, location[1] + halfHeight, location[1] + halfHeight,
         * 100);
         */

        View child = mActivity.getButton4();
        TouchUtils.dragViewBy(this, child, Gravity.CENTER_VERTICAL
                | Gravity.CENTER_HORIZONTAL, -100, 0);

        getInstrumentation().waitForIdleSync();

        assertTrue("ScrollX should not be zero after scrolling",
                mHorizontalScrollView.getScrollX() != 0);
    }

    public void testFlingAndDrag() {

        int halfWidth = (int) (mHorizontalScrollView.getWidth() / 2.0);
        int halfHeight = (int) (mHorizontalScrollView.getHeight() / 2.0);

        int[] location = new int[2];
        mHorizontalScrollView.getLocationOnScreen(location);

        Instrumentation inst = getInstrumentation();

        long downTime = SystemClock.uptimeMillis();
        long eventTime = SystemClock.uptimeMillis();

        float y = location[1] + halfHeight;
        float x = location[0] + halfWidth;

        MotionEvent event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        for (int i = 0; i < 2; ++i) {
            x -= 60;
            eventTime = SystemClock.uptimeMillis();
            event = MotionEvent.obtain(downTime, eventTime,
                    MotionEvent.ACTION_MOVE, x, y, 0);
            inst.sendPointerSync(event);
            inst.waitForIdleSync();
        }

        eventTime = SystemClock.uptimeMillis();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP,
                x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        x = location[0] + halfWidth;
        downTime = SystemClock.uptimeMillis();
        eventTime = SystemClock.uptimeMillis();

        event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        for (int i = 0; i < 30; ++i) {
            x += 1;
            eventTime = SystemClock.uptimeMillis();
            event = MotionEvent.obtain(downTime, eventTime,
                    MotionEvent.ACTION_MOVE, x, y, 0);
            inst.sendPointerSync(event);
            inst.waitForIdleSync();
        }

        eventTime = SystemClock.uptimeMillis();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP,
                x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        assertTrue("ScrollX should not be zero after scrolling",
                mHorizontalScrollView.getScrollX() != 0);
    }

    public void testMutiTouchScroll() {

        int halfWidth = (int) (mHorizontalScrollView.getWidth() / 2.0);
        int halfHeight = (int) (mHorizontalScrollView.getHeight() / 2.0);

        int[] location = new int[2];
        mHorizontalScrollView.getLocationOnScreen(location);
       
        float y = location[1] + halfHeight;
        float x = location[0] + halfWidth;
        
        float[] from1 = new float[2];
        float[] from2 = new float[2];
        float[] to1 = new float[2];
        float[] to2 = new float[2];
        
        from1[0] = x - 100;
        from1[1] = y;
        from2[0] = x + 100;
        from2[1] = y;
        to1[0] = x - 50;
        to1[1] = y;
        to2[0] = x + 20;
        to2[1] = y;
        
        generateZoomGesture(from1, from2, to1, to2, 50);
       

        assertTrue("ScrollX should not be zero after Zooming",
                mHorizontalScrollView.getScrollX() != 0);
    }

    private void generateZoomGesture(float[] from1, float[] from2, float[] to1, float[] to2, int steps) {

        Instrumentation inst = getInstrumentation();

        long eventTime = SystemClock.uptimeMillis();
        long downTime = SystemClock.uptimeMillis();
        MotionEvent event;
        float eventX1, eventY1, eventX2, eventY2;

        eventX1 = from1[0];
        eventY1 = from1[1];
        eventX2 = from2[0];
        eventY2 = from2[1];

        // specify the property for the two touch points
        PointerProperties[] properties = new PointerProperties[2];
        PointerProperties pp1 = new PointerProperties();
        pp1.id = 0;
        pp1.toolType = MotionEvent.TOOL_TYPE_FINGER;
        PointerProperties pp2 = new PointerProperties();
        pp2.id = 1;
        pp2.toolType = MotionEvent.TOOL_TYPE_FINGER;

        properties[0] = pp1;
        properties[1] = pp2;

        // specify the coordinations of the two touch points
        // NOTE: you MUST set the pressure and size value, or it doesn't work
        PointerCoords[] pointerCoords = new PointerCoords[2];
        PointerCoords pc1 = new PointerCoords();
        pc1.x = eventX1;
        pc1.y = eventY1;
        pc1.pressure = 1;
        pc1.size = 1;
        PointerCoords pc2 = new PointerCoords();
        pc2.x = eventX2;
        pc2.y = eventY2;
        pc2.pressure = 1;
        pc2.size = 1;
        pointerCoords[0] = pc1;
        pointerCoords[1] = pc2;

        // ////////////////////////////////////////////////////////////
        // events sequence of zoom gesture
        // 1. send ACTION_DOWN event of one start point
        // 2. send ACTION_POINTER_2_DOWN of two start points
        // 3. send ACTION_MOVE of two middle points
        // 4. repeat step 3 with updated middle points (x,y),
        // until reach the end points
        // 5. send ACTION_POINTER_2_UP of two end points
        // 6. send ACTION_UP of one end point
        // ////////////////////////////////////////////////////////////

        // step 1
        event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_DOWN, 1, properties, pointerCoords, 0, 0, 1,
                1, 0, 0, 0, 0);

        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        // step 2
        event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_POINTER_2_DOWN, 2, properties,
                pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);

        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        // step 3, 4
        if (true) {
            int moveEventNumber = 1;
            moveEventNumber = steps;

            float stepX1, stepY1, stepX2, stepY2;

            stepX1 = (to1[0] - from1[0]) / moveEventNumber;
            stepY1 = (to1[1] - from1[1]) / moveEventNumber;
            stepX2 = (to2[0] - from2[0]) / moveEventNumber;
            stepY2 = (to2[1] - from2[1]) / moveEventNumber;

            for (int i = 0; i < moveEventNumber; i++) {
                // update the move events
                eventTime = SystemClock.uptimeMillis();
                eventX1 += stepX1;
                eventY1 += stepY1;
                eventX2 += stepX2;
                eventY2 += stepY2;

                pc1.x = eventX1;
                pc1.y = eventY1;
                pc2.x = eventX2;
                pc2.y = eventY2;

                pointerCoords[0] = pc1;
                pointerCoords[1] = pc2;

                event = MotionEvent.obtain(downTime, eventTime,
                        MotionEvent.ACTION_MOVE, 2, properties, pointerCoords,
                        0, 0, 1, 1, 0, 0, 0, 0);

                inst.sendPointerSync(event);
                inst.waitForIdleSync();
            }
        }

        // step 5
        pc1.x = to1[0];
        pc1.y = to1[1];
        pc2.x = to2[0];
        pc2.y = to2[1];
        pointerCoords[0] = pc1;
        pointerCoords[1] = pc2;

        eventTime = SystemClock.uptimeMillis();
        event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_POINTER_2_UP, 2, properties, pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        // step 6
        eventTime = SystemClock.uptimeMillis();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP,
                1, properties, pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
    }

}