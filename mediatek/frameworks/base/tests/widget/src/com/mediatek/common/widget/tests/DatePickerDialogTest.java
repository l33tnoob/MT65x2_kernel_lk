package com.mediatek.common.widget.tests;

import com.mediatek.common.widget.tests.DatePickerDialogActivity;

import android.app.Activity;
import android.app.DatePickerDialog;
import android.app.Instrumentation;
import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewConfiguration;
import android.widget.DatePicker;

public class DatePickerDialogTest extends ActivityInstrumentationTestCase2<DatePickerDialogActivity>{

    DatePickerDialogActivity mActivity;
    DatePickerDialog mDatePickerDialog;
    DatePicker mDatePicker;
  
    public DatePickerDialogTest() {
        super("android.app.activity.DatePickerDialogActivity", DatePickerDialogActivity.class);
        // TODO Auto-generated constructor stub
    }
  
    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        setActivityInitialTouchMode(false);
        mActivity = (DatePickerDialogActivity)getActivity();
    }
  
    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        super.tearDown();
        System.out.println("tearDown");
    }
  
    public void testBackKeyHoldInitValue() {
        mActivity.runOnUiThread(new Runnable() {

        @Override
        public void run() {
            // TODO Auto-generated method stub
        } 
        });
    
        mDatePickerDialog = mActivity.getDialog();
        mDatePicker = mDatePickerDialog.getDatePicker();
    
        // generate a fake click event to test
        Instrumentation inst = getInstrumentation();
        int[] xy = new int[2];
        mDatePicker.getLocationOnScreen(xy);
    
        final int viewWidth = mDatePicker.getWidth();
        final int viewHeight = mDatePicker.getHeight();
        
        final float x = xy[0] + (viewWidth / 6.0f);
        final float y = xy[1] + (viewHeight / 6.0f);
        
        long downTime = SystemClock.uptimeMillis();
        long eventTime = SystemClock.uptimeMillis();
        
        MotionEvent event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
        
        eventTime = SystemClock.uptimeMillis();
        final int touchSlop = ViewConfiguration.get(mDatePicker.getContext()).getScaledTouchSlop();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, x , y , 0);
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
        
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }   

        this.sendKeys(KeyEvent.KEYCODE_BACK);
    
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
  
    public void testTouchOutSideHoldInitValue() {
	
        mActivity.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
            }	
        });
		
        mDatePickerDialog = mActivity.getDialog();
        mDatePickerDialog.setCanceledOnTouchOutside(true);
        mDatePicker = mDatePickerDialog.getDatePicker();		
		
        if(!mDatePickerDialog.isShowing()) {
            mDatePickerDialog.show();
        }
		
        // generate a fake click event to test
        Instrumentation inst = getInstrumentation();
		
        float x = 50.0f;
        float y = 50.0f;
		
        long downTime = SystemClock.uptimeMillis();
        long eventTime = SystemClock.uptimeMillis();
        
        MotionEvent event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
        
        eventTime = SystemClock.uptimeMillis();
        final int touchSlop = ViewConfiguration.get(mDatePicker.getContext()).getScaledTouchSlop();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, x + (touchSlop / 2.0f), y + (touchSlop / 2.0f), 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        eventTime = SystemClock.uptimeMillis();
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, x, y, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
        
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }    
    }
}
