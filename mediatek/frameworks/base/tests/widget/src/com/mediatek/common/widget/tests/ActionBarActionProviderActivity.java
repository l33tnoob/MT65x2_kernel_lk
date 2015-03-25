package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.Intent;
import android.view.Menu;
import android.widget.ShareActionProvider;
import android.widget.ShareActionProvider.OnShareTargetSelectedListener;

@SuppressLint("NewApi")
public class ActionBarActionProviderActivity extends Activity {
	private Menu mMenu;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.action_provider_main);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		mMenu = menu;
		getMenuInflater().inflate(R.menu.action_provider, menu);
		ShareActionProvider mShareActionProvider = (ShareActionProvider) menu
				.findItem(R.id.menu_search).getActionProvider();
		mShareActionProvider
				.setShareHistoryFileName("custom_share_history.xml");
		mShareActionProvider.setShareIntent(getDefaultShareIntent());
		mShareActionProvider.setOnShareTargetSelectedListener(new OnShareTargetSelectedListener() {
			
			public boolean onShareTargetSelected(ShareActionProvider source,
					Intent intent) {
				return false;
			}
		});
		return true;
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
		mMenu = menu;
		return super.onPrepareOptionsMenu(menu);
	}
	
	@Override
	public void onOptionsMenuClosed(Menu menu) {
		// TODO Auto-generated method stub
		super.onOptionsMenuClosed(menu);
		mMenu = null;
	}

	private Intent getDefaultShareIntent() {
		Intent shareIntent = new Intent(Intent.ACTION_SEND);
		shareIntent.setType("text/plain");
		shareIntent.putExtra(Intent.EXTRA_TEXT, "http://www.google.com");
		return shareIntent;
	}

	public void clickOnMenuItem(int menuItemId, Instrumentation instruments) {
    	final Integer itemId = menuItemId;
    	instruments.runOnMainSync(new Runnable() {
    	    public void run() {
    	    	if(mMenu != null) {
    	    		mMenu.performIdentifierAction(itemId, 0);
    	    	}
    	    }
    	});
    	instruments.waitForIdleSync();
    }
}
