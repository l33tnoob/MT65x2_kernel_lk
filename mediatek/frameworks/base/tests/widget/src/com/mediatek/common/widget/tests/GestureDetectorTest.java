package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Fragment;
import android.app.FragmentManager;
import android.test.ActivityInstrumentationTestCase2;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.SearchView;
import android.widget.TextView;

@SuppressLint("NewApi")
@TargetApi(15)
public class GestureDetectorTest extends ActivityInstrumentationTestCase2<GestureDetectorActivity> {
	private GestureDetectorActivity mActivity;
	
	private ActionBarUtils mActionBarUtils;

	public GestureDetectorTest() {
		super("com.example.actionbaractivity", GestureDetectorActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (GestureDetectorActivity)getActivity();
		mActionBarUtils = new ActionBarUtils(mActivity, getInstrumentation());
	}
	
	@Override
	protected void tearDown() throws Exception {
		// TODO Auto-generated method stub
		super.tearDown();
		System.out.println("tearDown");
	}
	
	public void testPreConditions() {
	    assertTrue(mActivity != null);
	}
	
	public void testActionProvider() {
		ActionBar actionBar = mActivity.getActionBar();
		int cx, cy;
		
		DisplayMetrics metrics = new DisplayMetrics();
		mActivity.getWindowManager().getDefaultDisplay().getMetrics(metrics);

		cy = metrics.heightPixels / 2;
		cx = metrics.widthPixels / 2;
		
		mActionBarUtils.drag(cx+50, cx-50, cy, cy, 10);
		getInstrumentation().waitForIdleSync();
		
		mActionBarUtils.drag(cx-50, cx+50, cy, cy, 10);
		getInstrumentation().waitForIdleSync();
		
		assertTrue("", true);
	}
}
