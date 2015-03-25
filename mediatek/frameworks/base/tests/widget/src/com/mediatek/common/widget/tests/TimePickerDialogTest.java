package com.mediatek.common.widget.tests;

import com.mediatek.common.widget.tests.TimePickerDialogActivity;

import android.app.Activity;
import android.app.TimePickerDialog;
import android.app.Instrumentation;
import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewConfiguration;
import android.widget.TimePicker;

public class TimePickerDialogTest extends ActivityInstrumentationTestCase2<TimePickerDialogActivity>{

    TimePickerDialogActivity mActivity;
    TimePickerDialog mTimePickerDialog;
    TimePicker mTimePicker;
  
    public TimePickerDialogTest() {
        super("android.app.activity.TimePickerDialogActivity", TimePickerDialogActivity.class);
        // TODO Auto-generated constructor stub
    }
  
    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        setActivityInitialTouchMode(false);
        mActivity = (TimePickerDialogActivity)getActivity();
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
		
        mTimePickerDialog = mActivity.getDialog();
        mTimePickerDialog.setCanceledOnTouchOutside(true);		
		
        if(!mTimePickerDialog.isShowing()) {
            mTimePickerDialog.show();
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
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, x, y, 0);
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
