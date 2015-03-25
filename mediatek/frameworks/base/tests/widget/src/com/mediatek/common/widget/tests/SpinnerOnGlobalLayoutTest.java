package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.widget.Spinner;
import com.mediatek.common.widget.tests.scroll.ButtonAboveTallInternalSelectionView;

import com.mediatek.common.widget.tests.SpinnerOnGlobalLayout;


public class SpinnerOnGlobalLayoutTest extends ActivityInstrumentationTestCase2<SpinnerOnGlobalLayout> {
    
    SpinnerOnGlobalLayout mActivity;
    Spinner mSpinner;
    
    public SpinnerOnGlobalLayoutTest(){
        super("com.mediatek.common.widget.tests", SpinnerOnGlobalLayout.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        
        mActivity = getActivity();
        mSpinner = mActivity.getSpinner();
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        
        
    }
    
    
    public void testPreconditions() {
        assertTrue(mActivity != null);
        assertTrue(mSpinner != null);
        
    }
    
    public void testOnGlobalLayoutChanged() {                
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        
        assertTrue("popup should be shown", mSpinner.isPopupShowing());
              
        mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }                 
        getInstrumentation().waitForIdleSync();
                
        
        assertTrue("popup should still be shown", mSpinner.isPopupShowing());
        
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSpinner.dismissPopup();
            }            
        });
        
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        } 
        
        getInstrumentation().waitForIdleSync();
        
        assertTrue("popup should still not be shown", !mSpinner.isPopupShowing());
        
        
    }
    
}
