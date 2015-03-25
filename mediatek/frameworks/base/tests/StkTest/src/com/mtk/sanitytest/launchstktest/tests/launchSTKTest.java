package com.mtk.sanitytest.launchstktest;

import android.app.Activity;
import android.content.ActivityNotFoundException;

import android.test.InstrumentationTestCase;

import junit.framework.TestCase;
import android.test.suitebuilder.annotation.LargeTest;
import android.app.Instrumentation;

import android.view.KeyEvent;
import android.view.View;
import android.widget.EditText;
import android.widget.Button;
import android.test.TouchUtils;

import android.os.SystemClock;
import android.view.MotionEvent;
import android.util.Log;

import java.lang.CharSequence;
import android.preference.Preference;

import android.preference.PreferenceActivity;
import android.net.Uri;
import android.content.Intent;

import android.os.Bundle;
import android.content.Context;
import android.view.MotionEvent;
import android.util.DisplayMetrics;
import android.test.ActivityInstrumentationTestCase2;
//import com.mtk.autotest.utils.AutoTestLog;

public class launchSTKTest extends
		ActivityInstrumentationTestCase2<com.mtk.sanitytest.launchstktest.launchSTK> {

	private Instrumentation mInst = null;
	private Context mContext = null;
	private Activity mActivity = null;

	private static final String TAG = "launchSTKTest_GEMINI";
    private static final int GEMINI_SIM_NUM = 2;
    private int mCurSimId = 0;

	//private AutoTestLog log = new AutoTestLog();

	private float mXRatio = 0.0f;
	private float mYRatio = 0.0f;
	int nXDelta = 20;
	int nYDelta = 20;
	int nXDirect = 1;
	int nYDirect = 1;

	public launchSTKTest() {
		super("com.mtk.sanitytest.launchstktest",
				com.mtk.sanitytest.launchstktest.launchSTK.class); 
	  Log.i(TAG, "launchSTKTest");
	}

	@Override
	protected void setUp() throws Exception {
		super.setUp();
		mInst = getInstrumentation();
		mContext = mInst.getContext();
		mActivity = getActivity();
	}

	@Override
	protected void tearDown() throws Exception {
		super.tearDown();

	}

	// @LargeTest
	public void testLaunchSTK() {

		SystemClock.sleep(5000);
		Log.i(TAG, "test STK1");
		mCurSimId = 0;
        launchStk4Gemini(mCurSimId);
        
        /* Ready to launch SIM3 STK */
        mCurSimId = 2;
        launchStk4Gemini(mCurSimId);
        /* Ready to launch SIM4 STK */
        mCurSimId = 3;
        launchStk4Gemini(mCurSimId);
	}

  public void testLaunchSTK2()
  {
      /* Ready to launch SIM2 STK */
      mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
      SystemClock.sleep(5000);
      mCurSimId = 1;
      launchStk4Gemini(mCurSimId);
  }
  
    private void launchStk4Gemini(int sim_id)
    {
        Log.i(TAG, "launchStk - start, sim_id: " + sim_id);
        String strTargetLoc = "com.android.stk";
        String strTargetClass = null;
        switch (sim_id)
        {
            case 1:
                strTargetClass = "com.android.stk.StkLauncherActivityII";
                break;
            case 2:
                strTargetClass = "com.android.stk.StkLauncherActivityIII";
                break;
            case 3:
                strTargetClass = "com.android.stk.StkLauncherActivityIV";
                break;
            default:
                strTargetClass = "com.android.stk.StkLauncherActivity";
                break;
        }
        Intent intent = new Intent();
        intent.setClassName(strTargetLoc, strTargetClass);
        try {
            mActivity.startActivity(intent);
        } catch(ActivityNotFoundException e) {
            Log.i(TAG, "ActivityNotFoundException happened");
            return;
        }

        clickMenu();
        SystemClock.sleep(2000);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        SystemClock.sleep(1000);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        SystemClock.sleep(1000);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        SystemClock.sleep(1000);
        Log.i(TAG, "launchStk - end");
    }
	// //helper functions///
	private void clickMenu() {

                mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
                //mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_ENTER);
                mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
                SystemClock.sleep(2000);
/*
		getDisplayMetrics();
		int x = (int) (mXRatio * 83 + nXDirect * nXDelta);
		int y = (int) (mYRatio * 67 + nYDirect * nYDelta);

		Log.i(TAG, "x =" + Integer.toString(x));
		Log.i(TAG, "y =" + Integer.toString(y));

		mInst.sendPointerSync(MotionEvent.obtain(SystemClock.uptimeMillis(),
				SystemClock.uptimeMillis(), MotionEvent.ACTION_DOWN, (float) x,
				(float) y, 0));
		SystemClock.sleep(50);
		mInst.sendPointerSync(MotionEvent.obtain(SystemClock.uptimeMillis(),
				SystemClock.uptimeMillis(), MotionEvent.ACTION_UP, (float) x,
				(float) y, 0));

		int DUR_WAIT_CHECK = 15000; // 15 seconds
		SystemClock.sleep(DUR_WAIT_CHECK);
*/
	}

	public void getDisplayMetrics() {
		DisplayMetrics dm = new DisplayMetrics();
		mActivity.getWindowManager().getDefaultDisplay().getMetrics(dm);
		int screenWidth = dm.widthPixels;
		int screenHeight = dm.heightPixels;
		if (screenWidth < screenHeight) {
			mXRatio = (float) (screenWidth / 240.00);
			mYRatio = (float) (screenHeight / 400.00);
		} else {
			mXRatio = (float) (screenWidth / 400.00);
			mYRatio = (float) (screenHeight / 240.00);
		}

		if (mXRatio < 1) {
			nXDirect = -1;
		}
		if (mYRatio < 1) {
			nYDirect = -1;
		}
	}
}
