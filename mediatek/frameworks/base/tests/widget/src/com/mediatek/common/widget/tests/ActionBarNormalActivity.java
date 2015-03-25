package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.pm.ActivityInfo;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

@TargetApi(11)
@SuppressLint("NewApi")
public class ActionBarNormalActivity extends Activity {
	private Menu mMenu;
	private TextView mTextView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.normal_main);
        
        mTextView = (TextView)findViewById(R.id.text);
        
        getActionBar().setHomeButtonEnabled(true);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setDisplayShowHomeEnabled(true);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.normal_main, menu);
        mMenu = menu;
        return true;
    }
    
    public Menu getMenu() {
    	return mMenu;
    }
    
    public void setOrientation(int requestedOrientation) {
    	//ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
    	this.setRequestedOrientation(requestedOrientation);
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

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
		if (item.getItemId() == android.R.id.home) {
			mTextView.setText(item.getTitle());
    		Toast.makeText(this, "Selected Item: " + item.getTitle(), Toast.LENGTH_SHORT).show();
    		return super.onOptionsItemSelected(item);
		} else {
			mTextView.setText(item.getTitle());
			Toast.makeText(this, "Selected Item: " + item.getTitle(), Toast.LENGTH_SHORT).show();
			return true;
		}
	}
	
	public void onSort(MenuItem item) {  
		mTextView.setText(item.getTitle());
		Toast.makeText(this, "Selected Item: " + item.getTitle(), Toast.LENGTH_SHORT).show();
    }  
}
