package com.orangelabs.rcs.wizard;



import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;

public class RcsWizardActivity extends Activity {

	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);   	
		initRcsOobe();
	}
	
	
	
	//initialize the rcs oobe process
	private void initRcsOobe(){
		Intent rcsOobeMainIntent = new Intent(this, WizardMainActivity.class);
	    rcsOobeMainIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
	    startActivity(rcsOobeMainIntent);
	    finish();
	}

}
