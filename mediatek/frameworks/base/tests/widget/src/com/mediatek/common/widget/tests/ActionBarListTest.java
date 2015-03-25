package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Fragment;
import android.app.FragmentManager;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

@SuppressLint("NewApi")
@TargetApi(15)
public class ActionBarListTest extends ActivityInstrumentationTestCase2<ActionBarListActivity> {
	private ActionBarListActivity mActivity;

	public ActionBarListTest() {
		super("com.mediatek.common.widget.tests", ActionBarListActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (ActionBarListActivity)getActivity();
	}
	
	@Override
	protected void tearDown() throws Exception {
		// TODO Auto-generated method stub
		super.tearDown();
		System.out.println("tearDown");
	}
	
	public void testPreConditions() {
	    assertTrue(mActivity != null);
	    
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				assertTrue("navigation mode is not list", actionBar.getNavigationMode() == ActionBar.NAVIGATION_MODE_LIST);
			}
		});
	    getInstrumentation().waitForIdleSync();
	}
	
	public void testList() {
		FragmentManager fragmentManager = mActivity.getFragmentManager();
		
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();

				assertTrue("list item count is wrong", actionBar.getNavigationItemCount() == 3);
				FragmentManager fragmentManager = mActivity.getFragmentManager();
				Fragment fragment = fragmentManager.findFragmentByTag("test");
				View rootView = fragment.getView();
				assertTrue("root view is not TextView", rootView instanceof TextView);
				assertTrue("the content is wrong", ((TextView)rootView).getText().equals("Mercury"));
			}
		});
	    getInstrumentation().waitForIdleSync();
	    
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				actionBar.setSelectedNavigationItem(1);
			}
		});
	    getInstrumentation().waitForIdleSync();
		Fragment fragment = fragmentManager.findFragmentByTag("test");
		View rootView = fragment.getView();
		assertTrue("root view is not TextView", rootView instanceof TextView);
	    assertTrue("the content is wrong", ((TextView)rootView).getText().equals("Venus"));
	    
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				actionBar.setSelectedNavigationItem(2);
			}
		});
		getInstrumentation().waitForIdleSync();
		fragment = fragmentManager.findFragmentByTag("test");
		rootView = fragment.getView();
		assertTrue("root view is not TextView", rootView instanceof TextView);
		assertTrue("the content is wrong", ((TextView)rootView).getText().equals("Earth"));
	}
}
