package com.mediatek.common.widget.tests;

import java.lang.reflect.Constructor;

import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.drawable.Drawable;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

@SuppressLint({ "NewApi", "NewApi" })
public class ActionBarOrientationChangedTest extends ActivityInstrumentationTestCase2<ActionBarOrientationChangedActivity> {

	private ActionBarUtils mActionBarUtils;;
	private ActionBarOrientationChangedActivity mActivity;
	private View mMenuView;
	private Button mButton;
	private TextView mTextView;
	
	public ActionBarOrientationChangedTest() {
		super("com.mediatek.common.widget.tests", ActionBarOrientationChangedActivity.class);
	}
	/*
	public ActionBarTest(Class<ActionBarActivity> activityClass) {
		super(activityClass);
		// TODO Auto-generated constructor stub
	}
	*/

	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (ActionBarOrientationChangedActivity)getActivity();
		mActionBarUtils = new ActionBarUtils(mActivity, getInstrumentation());
		mMenuView = (View)mActivity.findViewById(R.id.action_add);
		mButton = (Button)mActivity.findViewById(R.id.button);
		mTextView = (TextView)mActivity.findViewById(R.id.text);
	}
	
	@Override
	protected void tearDown() throws Exception {
		// TODO Auto-generated method stub
		super.tearDown();
		System.out.println("tearDown");
	}

	public void testPreConditions() {
	    assertTrue(mActivity != null);
	    assertTrue(mButton != null);
	    assertTrue(mTextView != null);
	}
	
	
	public void testActionBarOrientation() throws InterruptedException {
		mActivity.setOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		getInstrumentation().waitForIdleSync();

		assertTrue("test orientation", true);
	}
}
