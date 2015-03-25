package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.CheckBox;
import android.widget.TextView;

public class PreferenceFragmentActivity extends Activity {
	
	CheckBox prefCheckBox;
	TextView prefEditText;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.preference_fragment_main);
        
        prefCheckBox = (CheckBox)findViewById(R.id.prefCheckBox);
    	prefEditText = (TextView)findViewById(R.id.prefEditText);
    	
    	loadPref();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.preference_fragment, menu);
        return true;
    }

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		/*
		 * Because it's onlt ONE option in the menu.
		 * In order to make it simple, We always start SetPreferenceActivity
		 * without checking.
		 */
		
		Intent intent = new Intent();
        intent.setClass(PreferenceFragmentActivity.this, SetPreferenceActivity.class);
        startActivityForResult(intent, 0); 
		
        return true;
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		//super.onActivityResult(requestCode, resultCode, data);
		
		/*
		 * To make it simple, always re-load Preference setting.
		 */
		
		loadPref();
	}
    
	private void loadPref(){
		SharedPreferences mySharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
		
		boolean my_checkbox_preference = mySharedPreferences.getBoolean("checkbox_preference", false);
		prefCheckBox.setChecked(my_checkbox_preference);

		String my_edittext_preference = mySharedPreferences.getString("edittext_preference", "");
    	prefEditText.setText(my_edittext_preference);

	}
        
}
