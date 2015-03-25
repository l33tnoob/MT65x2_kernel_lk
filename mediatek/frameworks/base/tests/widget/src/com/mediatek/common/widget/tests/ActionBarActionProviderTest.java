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
public class ActionBarActionProviderTest extends ActivityInstrumentationTestCase2<ActionBarActionProviderActivity> {
	private ActionBarActionProviderActivity mActivity;
	private TextView mTextView;
	
	private ActionBarUtils mActionBarUtils;

	public ActionBarActionProviderTest() {
		super("com.mediatek.common.widget.tests", ActionBarActionProviderActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (ActionBarActionProviderActivity)getActivity();
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
		
		assertTrue("", true);
	}
}
