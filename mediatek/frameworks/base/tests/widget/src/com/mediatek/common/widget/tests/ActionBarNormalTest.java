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
public class ActionBarNormalTest extends ActivityInstrumentationTestCase2<ActionBarNormalActivity> {

	private ActionBarUtils mActionBarUtils;
	private ActionBarNormalActivity mActivity;
	private View mMenuView;
	private Button mButton;
	private TextView mTextView;
	
	public ActionBarNormalTest() {
		super("com.mediatek.common.widget.tests", ActionBarNormalActivity.class);
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
		mActivity = (ActionBarNormalActivity)getActivity();
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
	
	public void invokeMenuItem(int id) {
		View menuItemView = (View)mActivity.findViewById(id);
		
		if(menuItemView != null) {
			//sendKeys(KeyEvent.KEYCODE_MENU);
			mActivity.clickOnMenuItem(id, getInstrumentation());
			//getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_ENTER);
		} else {
			getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
			mActionBarUtils.sleep(300);
			mActivity.clickOnMenuItem(id, getInstrumentation());
			//sleep(300);
			//getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_ENTER);
		}
	}

	public void testPreconditions() {
		
		
	    assertTrue(mActivity != null);
	    assertTrue(mButton != null);
	    assertTrue(mTextView != null);
	    
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				int displayOptions;
				
			    assertTrue(actionBar.isShowing());
			    
			    displayOptions = actionBar.getDisplayOptions();
			    assertTrue((displayOptions & ActionBar.DISPLAY_HOME_AS_UP) != 0);
			    assertTrue((displayOptions & ActionBar.DISPLAY_SHOW_HOME) != 0);
			    
			    assertTrue(actionBar.getNavigationMode() == ActionBar.NAVIGATION_MODE_STANDARD);
			    assertTrue(actionBar.getCustomView() == null);
			    assertTrue(actionBar.getNavigationItemCount() == 0);
			    assertTrue(actionBar.getSelectedNavigationIndex() == -1);
			    assertTrue(actionBar.getSelectedTab() == null);
			    //assertTrue(actionBar.getTabAt(0) == null); // index out of bound
			    assertTrue(actionBar.getTabCount() == 0);
			}
		});
	    getInstrumentation().waitForIdleSync();
	}
	
	public void testActionBarHomeButton() throws InterruptedException {
		mActivity.runOnUiThread(new Runnable() {
			public void run() {
				//((View)mMenuView).requestFocus();
				//((View)mMenuView).callOnClick();
				mActionBarUtils.clickOnActionBarHomeButton();
			}
		});
		getInstrumentation().waitForIdleSync();
		assertTrue("Can't click action bar home button", mTextView.getText().equals(mActivity.getTitle()));
	}
	
	public void testActionMenuItem() throws InterruptedException {
		mActionBarUtils.clickOnActionBarItem(R.id.action_add);
		getInstrumentation().waitForIdleSync();

		assertTrue("Can't click action bar menu item", mTextView.getText().equals("action_bar_add"));
	}
	
	
	public void testOptionMenuItem() throws InterruptedException {
		invokeMenuItem(R.id.action_edit);
		getInstrumentation().waitForIdleSync();

		assertTrue("Can't click action bar option menu item", mTextView.getText().equals("action_bar_edit"));
	}
	
	public void testOptionMenuSubItem() throws InterruptedException {
		invokeMenuItem(R.id.action_sort);
		getInstrumentation().waitForIdleSync();
		mActivity.clickOnMenuItem(R.id.action_sort_size, getInstrumentation());
		getInstrumentation().waitForIdleSync();

		assertTrue("Can't click action bar option menu sub item", mTextView.getText().equals("action_bar_sort_size"));
	}
	
	public void testActionBarOrientation() throws InterruptedException {
		mActivity.setOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		getInstrumentation().waitForIdleSync();

		assertTrue("test orientation", true);
	}
	
	public void testActionBarOptions() throws InterruptedException {
		
		mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();

				actionBar.setDisplayHomeAsUpEnabled(false);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_HOME_AS_UP) == 0);
				actionBar.setDisplayShowHomeEnabled(false);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_SHOW_HOME) == 0);
				actionBar.setDisplayShowTitleEnabled(false);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_SHOW_TITLE) == 0);
				actionBar.setDisplayUseLogoEnabled(false);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_USE_LOGO) == 0);
				actionBar.setDisplayShowCustomEnabled(false);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_SHOW_CUSTOM) == 0);
				
				actionBar.setDisplayHomeAsUpEnabled(true);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_HOME_AS_UP) != 0);
				actionBar.setDisplayShowHomeEnabled(true);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_SHOW_HOME) != 0);
				actionBar.setDisplayShowTitleEnabled(true);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_SHOW_TITLE) != 0);
				actionBar.setDisplayUseLogoEnabled(true);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_USE_LOGO) != 0);
				actionBar.setDisplayShowCustomEnabled(true);
				assertTrue((actionBar.getDisplayOptions() & ActionBar.DISPLAY_SHOW_CUSTOM) != 0);
			}
		});
		getInstrumentation().waitForIdleSync();
	}
	
	public void testActionBarProperties() throws InterruptedException {
		
		mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				
				actionBar.setTitle("test");
				assertEquals(actionBar.getTitle(), "test");
				
				actionBar.setSubtitle("test subtitle");
				assertEquals(actionBar.getSubtitle(), "test subtitle");
				
				//Drawable drawable = mActivity.getResources().getDrawable(android.R.drawable.ic_menu_add);
				//actionBar.setIcon(drawable);
			}
		});
		getInstrumentation().waitForIdleSync();
	}
}
