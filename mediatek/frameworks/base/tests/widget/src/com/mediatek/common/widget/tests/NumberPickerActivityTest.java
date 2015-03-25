package com.mediatek.common.widget.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import android.view.MotionEvent;
import android.view.ViewConfiguration;
import android.widget.NumberPicker;

import com.mediatek.common.widget.tests.NumberPickerActivity;

public class NumberPickerActivityTest extends ActivityInstrumentationTestCase2<NumberPickerActivity> {

    Activity mActivity;
    NumberPicker mNumberPicker;

    public NumberPickerActivityTest() {
        super("android.widget.NumberPickerActivity", NumberPickerActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        setActivityInitialTouchMode(false);
        mActivity = getActivity();
        mNumberPicker = (NumberPicker)mActivity.findViewById(R.id.testNumberPicker);
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        super.tearDown();
        System.out.println("tearDown");
    }

    public void testLongPressIncrementBtn() {
        mActivity.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mNumberPicker.requestFocus();
            }
        });
        // generate a fake click event to test
        Instrumentation inst = getInstrumentation();

        int[] xy = new int[2];
        mNumberPicker.getLocationOnScreen(xy);

        final int viewWidth = mNumberPicker.getWidth();
        final int viewHeight = mNumberPicker.getHeight();

        final float x = xy[0] + (viewWidth / 2.0f);
        final float y = xy[1] + (viewHeight / 6.0f);

        long downTime = SystemClock.uptimeMillis();
        long eventTime = SystemClock.uptimeMillis();

        MotionEvent event = MotionEvent.obtain(downTime, eventTime,
                MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        eventTime = SystemClock.uptimeMillis();
        final int touchSlop = ViewConfiguration.get(mNumberPicker.getContext()).getScaledTouchSlop();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE,
                x , y , 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        try {
            Thread.sleep((long)(ViewConfiguration.getLongPressTimeout() * 3.0f));
            //Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        eventTime = SystemClock.uptimeMillis();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
	}
}
