package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Fragment;
import android.app.FragmentManager;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.SearchView;

@SuppressLint("NewApi")
@TargetApi(15)
public class TextureViewTest extends ActivityInstrumentationTestCase2<TextureViewActivity> {
	private TextureViewActivity mActivity;
	
	private ActionBarUtils mActionBarUtils;

	public TextureViewTest() {
		super("com.mediatek.common.widget.tests", TextureViewActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (TextureViewActivity)getActivity();
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
	    
	    getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
		getInstrumentation().waitForIdleSync();
		mActionBarUtils.sleep(2000);
	}
	
	public void testActionProvider() {
		ActionBar actionBar = mActivity.getActionBar();
		
		getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
		getInstrumentation().waitForIdleSync();
		
		assertTrue("", true);
		mActionBarUtils.sleep(2000);
	}
}
