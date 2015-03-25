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
public class ActionBarSplitActionBarTest extends ActivityInstrumentationTestCase2<ActionBarSplitActionBarActivity> {
	private ActionBarSplitActionBarActivity mActivity;
	private TextView mTextView;
	
	private ActionBarUtils mActionBarUtils;

	public ActionBarSplitActionBarTest() {
		super("com.mediatek.common.widget.tests", ActionBarSplitActionBarActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (ActionBarSplitActionBarActivity)getActivity();
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
	
	public void testSplitActionBar() {
		ActionBar actionBar = mActivity.getActionBar();

		mActionBarUtils.clickOnActionBarItem(R.id.simple_item);
		getInstrumentation().waitForIdleSync();
		assertTrue("the text is wrong", mTextView.getText().equals("option 1"));
		mActionBarUtils.clickOnActionBarItem(R.id.simple_item2);
		getInstrumentation().waitForIdleSync();
		assertTrue("the text is wrong", mTextView.getText().equals("option 2"));
		mActionBarUtils.clickOnActionBarItem(R.id.simple_item3);
		getInstrumentation().waitForIdleSync();
		assertTrue("the text is wrong", mTextView.getText().equals("option 3"));
	}
}
