package com.mediatek.common.widget.tests;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SimpleAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;;

public class SpinnerOnGlobalLayout extends Activity {

	private Spinner mSpinner;
	
	

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.spinner);
			    
	    mSpinner = (Spinner) findViewById(R.id.spinner);
	    
		List<String> list = new ArrayList<String>(); 
		list.add("list 1");
		list.add("list 2"); 
		list.add("list 3"); 
		ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this,
		        android.R.layout.simple_spinner_item, list);
		dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		mSpinner.setAdapter(dataAdapter);
				
		
		
		
	}

	public Spinner getSpinner() {
	    return mSpinner;	    
	}
	

}