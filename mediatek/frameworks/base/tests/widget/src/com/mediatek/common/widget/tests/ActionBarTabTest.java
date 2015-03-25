package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

@SuppressLint("NewApi")
@TargetApi(15)
public class ActionBarTabTest extends ActivityInstrumentationTestCase2<ActionBarTabActivity> {
	private ActionBarTabActivity mActivity;
	private Button mAddTabBtn;
	private Button mRemoveTabBtn;
	private Button mToggleTabsBtn;
	private Button mRemoveAllTabsBtn;

	public ActionBarTabTest() {
		super("com.mediatek.common.widget.tests", ActionBarTabActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (ActionBarTabActivity)getActivity();
		mAddTabBtn = (Button)mActivity.findViewById(R.id.btn_add_tab);
		mRemoveTabBtn = (Button)mActivity.findViewById(R.id.btn_remove_tab);
		mToggleTabsBtn = (Button)mActivity.findViewById(R.id.btn_toggle_tabs);
		mRemoveAllTabsBtn = (Button)mActivity.findViewById(R.id.btn_remove_all_tabs);
	}
	
	@Override
	protected void tearDown() throws Exception {
		// TODO Auto-generated method stub
		super.tearDown();
		System.out.println("tearDown");
	}
	
	public void testPreConditions() {
	    assertTrue(mActivity != null);
	    assertTrue(mAddTabBtn != null);
	    assertTrue(mRemoveTabBtn != null);
	    assertTrue(mToggleTabsBtn != null);
	    assertTrue(mRemoveAllTabsBtn != null);
	    
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				mToggleTabsBtn.requestFocus();
				mToggleTabsBtn.callOnClick();
			}
		});
	    getInstrumentation().waitForIdleSync();
	    
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				assertTrue("navigation mode is not tab", actionBar.getNavigationMode() == ActionBar.NAVIGATION_MODE_TABS);
			}
		});
	    getInstrumentation().waitForIdleSync();
	}
	
	public void testAddTabs() {
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();

				mToggleTabsBtn.requestFocus();
				mToggleTabsBtn.callOnClick();
				
				mAddTabBtn.requestFocus();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();
				
				assertTrue("tab count is wrong", actionBar.getTabCount() == 3);
				assertTrue("navigation count is wrong", actionBar.getNavigationItemCount() == actionBar.getTabCount());
				assertNotNull(actionBar.getTabAt(0));
				assertNotNull(actionBar.getTabAt(1));
				assertNotNull(actionBar.getTabAt(2));
				assertEquals(actionBar.getTabAt(0).getText(), "Tab "+ 0);
				assertEquals(actionBar.getTabAt(1).getText(), "Tab "+ 1);
				assertEquals(actionBar.getTabAt(2).getText(), "Tab "+ 2);
				
				actionBar.removeTabAt(1);
				assertTrue("tab count is wrong", actionBar.getTabCount() == 2);
				assertEquals(actionBar.getTabAt(0).getText(), "Tab "+ 0);
				assertEquals(actionBar.getTabAt(1).getText(), "Tab "+ 2);
			}
		});
	    getInstrumentation().waitForIdleSync();
	}
	
	public void testRemoveTabs() {
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();

				mToggleTabsBtn.requestFocus();
				mToggleTabsBtn.callOnClick();
				
				mAddTabBtn.requestFocus();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();
				
				mRemoveTabBtn.requestFocus();
				mRemoveTabBtn.callOnClick();
				assertTrue("tab count is wrong", actionBar.getTabCount() == 2);
				mRemoveTabBtn.callOnClick();
				assertTrue("tab count is wrong", actionBar.getTabCount() == 1);
				mRemoveTabBtn.callOnClick();
				assertTrue("tab count is wrong", actionBar.getTabCount() == 0);
				assertTrue("navigation count is wrong", actionBar.getNavigationItemCount() == actionBar.getTabCount());
				
				mAddTabBtn.requestFocus();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();
				
				mRemoveAllTabsBtn.requestFocus();
				mRemoveAllTabsBtn.callOnClick();
				assertTrue("tab count is wrong", actionBar.getTabCount() == 0);
			}
		});
	    getInstrumentation().waitForIdleSync();
	}
	
	public void testOthers() {
	    mActivity.runOnUiThread(new Runnable() {
			public void run() {
				ActionBar actionBar = mActivity.getActionBar();
				
				mToggleTabsBtn.requestFocus();
				mToggleTabsBtn.callOnClick();
				
				mAddTabBtn.requestFocus();
				mAddTabBtn.callOnClick();
				mAddTabBtn.callOnClick();

				assertTrue("select wrong tab", actionBar.getTabAt(0) == actionBar.getSelectedTab());
				actionBar.setSelectedNavigationItem(1);
				assertTrue("select wrong tab", actionBar.getTabAt(1) == actionBar.getSelectedTab());
				actionBar.selectTab(actionBar.getTabAt(0));
				assertTrue("select wrong tab", actionBar.getTabAt(0) == actionBar.getSelectedTab());
				
				actionBar.selectTab(null);
				assertNull(actionBar.getSelectedTab());
				
				// index out of bound
				//actionBar.selectTab(actionBar.getTabAt(0));
				//actionBar.setSelectedNavigationItem(Tab.INVALID_POSITION);
			
				
				
				//assertNull(actionBar.getSelectedTab());
			}
		});
	    getInstrumentation().waitForIdleSync();
	}
}
