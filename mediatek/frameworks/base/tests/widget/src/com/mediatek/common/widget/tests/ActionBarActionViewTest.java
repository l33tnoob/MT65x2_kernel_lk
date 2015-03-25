package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Fragment;
import android.app.FragmentManager;
import android.test.ActivityInstrumentationTestCase2;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.SearchView;
import android.widget.TextView;

@SuppressLint("NewApi")
@TargetApi(15)
public class ActionBarActionViewTest extends ActivityInstrumentationTestCase2<ActionBarActionViewActivity> {
	private ActionBarActionViewActivity mActivity;
	private TextView mTextView;
	
	private ActionBarUtils mActionBarUtils;

	public ActionBarActionViewTest() {
		super("com.mediatek.common.widget.tests", ActionBarActionViewActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (ActionBarActionViewActivity)getActivity();
		mActionBarUtils = new ActionBarUtils(mActivity, getInstrumentation());
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
	    assertTrue(mTextView != null);
	}
	
	public void testActionProvider() {
		ActionBar actionBar = mActivity.getActionBar();

		mActionBarUtils.clickOnActionBarItem(R.id.menu_search);
		getInstrumentation().waitForIdleSync();
		
		Menu menu = mActivity.getMenu();
		assertNotNull(menu);
		
		MenuItem searchMenuItem = menu.findItem(R.id.menu_search);
		assertNotNull(searchMenuItem);
		
		SearchView searchView = (SearchView)searchMenuItem.getActionView();
		assertNotNull(searchView);
		
		mActionBarUtils.clickOnActionBarHomeButton();
		getInstrumentation().waitForIdleSync();
		
		assertTrue("", true);
	}
}
